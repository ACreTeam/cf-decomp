#include <lib/RegEad/RegNfa.hpp>
#include <lib/RegEad/RegEadArray.inl>

namespace RegEad {

u8 gAllocFailed;

// 801E92FC
RegNfa::RegNfa() : mpToken(NULL), mFlags(0), mAccept(-1), mError(0) {
}

// 801E9364
RegNfa::~RegNfa() {
    reset();
}

// 801E93E4
bool RegNfa::compile(const u16 *pattern, u32 flags) {
    reset();
    mError = 0;
    gAllocFailed = 0;
    mFlags = flags;
    if (!(flags & FLAG_CASE_INSENSITIVE) && (flags & FLAG_UNICODE_CASE)) {
        mFlags = flags & ~FLAG_UNICODE_CASE;
    }
    tokenize(pattern);
    if (mError != 0) {
        return false;
    }
    {
        RegNfaState start;
        mStates.push_back(start);
    }
    if (gAllocFailed) {
        mError = ERR_NO_MEMORY;
        return false;
    }
    mpToken = mTokens.begin();
    mAccept = parseAlternation(0, 0);
    if (mError == 0) {
        return true;
    }
    reset();
    return false;
}

// 801E9518
void RegNfa::tokenize(const u16 *p) {
    bool quoted = false;
    while (*p != 0) {
        if (quoted) {
            if (*p == L'\\' && p[1] == L'E') {
                quoted = false;
                p += 2;
                continue;
            }
            RegToken *tok = mTokens.push_back(RegToken(RegToken::TYPE_CHAR));
            if (gAllocFailed) {
                mError = ERR_NO_MEMORY;
                return;
            }
            tok->mChar = *p++;
        } else if (*p == L'\\' && p[1] == L'Q') {
            quoted = true;
            p += 2;
        } else if (*p == L'\\' && p[1] == L'E') {
            mError = ERR_UNMATCHED_QUOTE_END;
            return;
        } else {
            p = tokenizeNext(p);
            if (mError != 0) {
                return;
            }
        }
    }
    mTokens.push_back(RegToken(RegToken::TYPE_END));
    if (gAllocFailed) {
        mError = ERR_NO_MEMORY;
    }
}

// 801E9664
const u16 *RegNfa::tokenizeNext(const u16 *p) {
    int type;
    u16 ch = 0;
    int min = 0;
    int max = 0;
    switch (*p) {
    case L'\\':
        p++;
        switch (*p) {
        case L'\\':
            type = RegToken::TYPE_CHAR;
            ch = L'\\';
            break;
        case L'r':
            type = RegToken::TYPE_CHAR;
            ch = L'\r';
            break;
        case L'n':
            type = RegToken::TYPE_CHAR;
            ch = L'\n';
            break;
        case L't':
            type = RegToken::TYPE_CHAR;
            ch = L'\t';
            break;
        case L'f':
            type = RegToken::TYPE_CHAR;
            ch = L'\f';
            break;
        case L'a':
            type = RegToken::TYPE_CHAR;
            ch = 7;
            break;
        case L'e':
            type = RegToken::TYPE_CHAR;
            ch = 0x1B;
            break;
        case L'd':
            type = RegToken::TYPE_CLASS_ESCAPE;
            min = CLASS_DIGIT;
            break;
        case L'D':
            type = RegToken::TYPE_CLASS_ESCAPE;
            min = CLASS_NOT_DIGIT;
            break;
        case L's':
            type = RegToken::TYPE_CLASS_ESCAPE;
            min = CLASS_SPACE;
            break;
        case L'S':
            type = RegToken::TYPE_CLASS_ESCAPE;
            min = CLASS_NOT_SPACE;
            break;
        case L'w':
            type = RegToken::TYPE_CLASS_ESCAPE;
            min = CLASS_WORD;
            break;
        case L'W':
            type = RegToken::TYPE_CLASS_ESCAPE;
            min = CLASS_NOT_WORD;
            break;
        case L'x':
            if (!parseHex(p + 1, 2, &ch)) {
                mError = ERR_BAD_HEX;
                return NULL;
            }
            type = RegToken::TYPE_CHAR;
            p += 2;
            break;
        case L'u':
            if (!parseHex(p + 1, 4, &ch)) {
                mError = ERR_BAD_HEX;
                return NULL;
            }
            type = RegToken::TYPE_CHAR;
            p += 4;
            break;
        case L'A':
            type = RegToken::TYPE_INPUT_START;
            break;
        case L'Z':
            type = RegToken::TYPE_INPUT_END_TERM;
            break;
        case L'z':
            type = RegToken::TYPE_INPUT_END;
            break;
        case L'b':
            type = RegToken::TYPE_WORD_BOUNDARY;
            break;
        case L'B':
            type = RegToken::TYPE_NOT_WORD_BOUNDARY;
            break;
        case L'c':
            p++;
            if (*p >= L'A' && *p <= L'_') {
                ch = *p - L'@';
                type = RegToken::TYPE_CHAR;
            } else {
                mError = ERR_BAD_CONTROL;
                return NULL;
            }
            break;
        case L'P':
        case L'p': {
            type = RegToken::TYPE_NEG_PROPERTY;
            if (*p == L'p') {
                type = RegToken::TYPE_CLASS_ESCAPE;
            }
            p++;
            if (*p == L'{') {
                p++;
                min = findChar(p, L'}');
                if (min != -1) {
                    int cls = lookupClassName(p, min, false);
                    if (cls == 0) {
                        mError = ERR_UNKNOWN_PROPERTY;
                        return NULL;
                    }
                    p += min;
                    min = cls;
                } else {
                    mError = ERR_UNCLOSED_PROPERTY;
                    return NULL;
                }
            } else {
                int cls = 0;
                min = 0;
                if (*p != 0 && p[1] != 0) {
                    cls = lookupClassName(p, 2, true);
                    if (cls != 0) {
                        min = 1;
                    }
                    if (cls == 0) {
                        cls = lookupClassName(p, 1, true);
                    }
                }
                if (cls != 0) {
                    p += min;
                    min = cls;
                } else {
                    mError = ERR_NO_PROPERTY_NAME;
                    return NULL;
                }
            }
            break;
        }
        case L'0':
            ch = p[1];
            if (ch >= L'0' && ch <= L'3') {
                ch = ch - L'0';
                p++;
                for (int digit = 0; digit < 2; digit++) {
                    p++;
                    if (*p >= L'0' && *p <= L'7') {
                        ch <<= 3;
                        ch |= (u16)(*p - L'0');
                    } else {
                        p--;
                        break;
                    }
                }
            } else if (ch >= L'4' && ch <= L'7') {
                p += 2;
                ch = ch - L'0';
                if (*p >= L'0' && *p <= L'7') {
                    ch <<= 3;
                    ch |= (u16)(*p - L'0');
                } else {
                    p--;
                }
            } else {
                mError = ERR_BAD_OCTAL;
                return NULL;
            }
            type = RegToken::TYPE_CHAR;
            break;
        case 0:
            mError = ERR_TRAILING_BACKSLASH;
            return NULL;
        default:
            if ((*p >= L'a' && *p <= L'z') || (*p >= L'A' && *p <= L'Z') ||
                (*p >= L'1' && *p <= L'9')) {
                mError = ERR_BAD_ESCAPE;
                return NULL;
            }
            ch = *p;
            type = RegToken::TYPE_CHAR;
            break;
        }
        break;
    case L'.':
        type = RegToken::TYPE_CLASS_ESCAPE;
        min = CLASS_ANY;
        break;
    case L'|':
        type = RegToken::TYPE_OR;
        break;
    case L'[':
        if (p[1] == '^') {
            p++;
            type = RegToken::TYPE_NEG_CLASS_OPEN;
        } else {
            type = RegToken::TYPE_CLASS_OPEN;
        }
        break;
    case L']':
        type = RegToken::TYPE_CLASS_CLOSE;
        break;
    case L'-':
        type = RegToken::TYPE_CLASS_DASH;
        break;
    case L'&':
        if (p[1] == L'&') {
            p++;
            type = RegToken::TYPE_CLASS_AND;
        } else {
            type = RegToken::TYPE_CHAR;
            ch = L'&';
        }
        break;
    case L'(':
        type = RegToken::TYPE_GROUP_OPEN;
        break;
    case L')':
        type = RegToken::TYPE_GROUP_CLOSE;
        break;
    case L'?':
        type = RegToken::TYPE_OPTIONAL;
        break;
    case L'*':
        type = RegToken::TYPE_STAR;
        break;
    case L'+':
        type = RegToken::TYPE_PLUS;
        break;
    case L'{':
        p++;
        type = countDigits(p);
        if (type == 0) {
            mError = ERR_BAD_REPEAT;
            return NULL;
        }
        min = parseDecimal(p, type);
        p += type;
        if (*p == L'}') {
            type = RegToken::TYPE_REPEAT_N;
        } else if (*p != L',') {
            mError = ERR_BAD_REPEAT;
            return NULL;
        } else {
            p++;
            if (*p == L'}') {
                type = RegToken::TYPE_REPEAT_N_OR_MORE;
            } else {
                const int &digits = countDigits(p);
                if (digits == 0) {
                    mError = ERR_BAD_REPEAT;
                    return NULL;
                }
                max = parseDecimal(p, digits);
                p += digits;
                if (*p != L'}') {
                    mError = ERR_BAD_REPEAT;
                    return NULL;
                }
                type = RegToken::TYPE_REPEAT_RANGE;
            }
        }
        break;
    case L'^':
        type = RegToken::TYPE_LINE_START;
        break;
    case L'$':
        type = RegToken::TYPE_LINE_END;
        break;
    default:
        type = RegToken::TYPE_CHAR;
        ch = *p;
        break;
    }
    RegToken *tok = mTokens.push_back(RegToken(type));
    if (gAllocFailed) {
        mError = ERR_NO_MEMORY;
        return NULL;
    }
    tok->mChar = ch;
    tok->mMin = min;
    tok->mMax = max;
    return p + 1;
}

// 801E9D90
bool RegNfa::parseHex(const u16 *p, int num, u16 *out) {
    *out = 0;
    for (u32 i = 0; i < num; i++) {
        u16 c = *p;
        if (c >= L'0' && c <= L'9') {
            *out |= c - L'0';
        } else if (c >= L'a' && c <= L'f') {
            *out |= c - L'a' + 10;
        } else if (c >= L'A' && c <= L'F') {
            *out |= c - L'A' + 10;
        } else {
            return false;
        }
        if (i != num - 1) {
            *out <<= 4;
            p++;
        }
    }
    return true;
}

// 801E9E4C
int RegNfa::countDigits(const u16 *p) {
    int n = 0;
    for (;;) {
        if (*p >= L'0' && *p <= L'9') {
            n++;
        } else {
            return n;
        }
        p++;
    }
}

// 801E9E7C
int RegNfa::parseDecimal(const u16 *p, int num) {
    int value = 0;
    for (u32 i = 0; i < num; i++) {
        value *= 10;
        value += *p++ - L'0';
    }
    return value;
}

// 801E9F64
int findChar(const u16 *s, u16 c) {
    for (int i = 0; *s != 0; s++, i++) {
        if (c == *s) {
            return i;
        }
    }
    return -1;
}

// 801E9F98
bool equalsName(const u16 *s, const u16 *name, int len) {
    const u16 *n = name;
    int i;
    for (i = 0; i < len; i++) {
        if (*n == 0) {
            return false;
        }
        if (*s != *n) {
            return false;
        }
        n++;
        s++;
    }
    return name[i] == 0;
}

// 801E9FF8
int RegNfa::parseAlternation(int from, int nested) {
    int last = parseSequence(from, nested);
    if (mError != 0) {
        return -1;
    }
    int end = newState();
    if (gAllocFailed) {
        mError = ERR_NO_MEMORY;
        return -1;
    }
    mStates[last].mEdges.push_back(RegNfaEdge(end));
    if (gAllocFailed) {
        mError = ERR_NO_MEMORY;
        return -1;
    }
    for (;;) {
        if (mpToken->mType != RegToken::TYPE_OR) {
            break;
        }
        mpToken++;
        last = parseSequence(from, nested);
        if (mError != 0) {
            return -1;
        }
        mStates[last].mEdges.push_back(RegNfaEdge(end));
        if (gAllocFailed) {
            mError = ERR_NO_MEMORY;
            return -1;
        }
    }
    return end;
}

// 801EA174
int RegNfa::parseSequence(int from, int nested) {
    for (;;) {
        int atom = parseAtom(from, nested);
        if (atom == -1) {
            if (gAllocFailed) {
                mError = ERR_NO_MEMORY;
                return -1;
            }
            return from;
        }
        from = parseQuantifier(from, atom);
        if (from == -1 && gAllocFailed) {
            mError = ERR_NO_MEMORY;
            return -1;
        }
    }
}

// 801EA23C
int RegNfa::parseAtom(int from, int nested) {
    switch (mpToken->mType) {
    case RegToken::TYPE_CHAR:
        return parseLiteral(from);
    case RegToken::TYPE_LINE_START:
        return addAssertionEdge(from, RegLabel::TYPE_LINE_START);
    case RegToken::TYPE_LINE_END:
        return addAssertionEdge(from, RegLabel::TYPE_LINE_END);
    case RegToken::TYPE_WORD_BOUNDARY:
        return addAssertionEdge(from, RegLabel::TYPE_WORD_BOUNDARY);
    case RegToken::TYPE_NOT_WORD_BOUNDARY:
        return addAssertionEdge(from, RegLabel::TYPE_NOT_WORD_BOUNDARY);
    case RegToken::TYPE_INPUT_START:
        return addAssertionEdge(from, RegLabel::TYPE_INPUT_START);
    case RegToken::TYPE_INPUT_END:
        return addAssertionEdge(from, RegLabel::TYPE_INPUT_END);
    case RegToken::TYPE_INPUT_END_TERM:
        return addAssertionEdge(from, RegLabel::TYPE_INPUT_END_TERM);
    case RegToken::TYPE_GROUP_OPEN:
        return parseGroup(from);
    case RegToken::TYPE_CLASS_OPEN:
        return addBracketClassEdge(from, false);
    case RegToken::TYPE_NEG_CLASS_OPEN:
        return addBracketClassEdge(from, true);
    case RegToken::TYPE_CLASS_ESCAPE:
        return addEscapeClassEdge(from, false);
    case RegToken::TYPE_NEG_PROPERTY:
        return addEscapeClassEdge(from, true);
    case RegToken::TYPE_CLASS_DASH: {
        int s = newState();
        if (gAllocFailed) {
            return -1;
        }
        mStates[from].mEdges.push_back(RegNfaEdge(s, (u16)'-'));
        if (gAllocFailed) {
            return -1;
        }
        mpToken++;
        return s;
    }
    case RegToken::TYPE_CLASS_AND: {
        int s;
        for (int i = 0; i < 2; i++) {
            s = newState();
            if (gAllocFailed) {
                return -1;
            }
            mStates[from].mEdges.push_back(RegNfaEdge(s, (u16)'&'));
            if (gAllocFailed) {
                return -1;
            }
            from = s;
        }
        mpToken++;
        return s;
    }
    case RegToken::TYPE_CLASS_CLOSE:
        mError = ERR_STRAY_CLASS_CLOSE;
        return -1;
    case RegToken::TYPE_GROUP_CLOSE:
        if (nested == 0) {
            mError = ERR_UNMATCHED_GROUP_CLOSE;
        }
        return -1;
    case RegToken::TYPE_OPTIONAL:
    case RegToken::TYPE_STAR:
    case RegToken::TYPE_REPEAT_N:
    case RegToken::TYPE_REPEAT_N_OR_MORE:
    case RegToken::TYPE_REPEAT_RANGE:
        mError = ERR_DANGLING_QUANTIFIER;
        return -1;
    }
    return -1;
}

// 801EA534
int RegNfa::parseLiteral(int from) {
    int s = newState();
    if (gAllocFailed) {
        mError = ERR_NO_MEMORY;
        return -1;
    }
    u16 ch = mpToken->getChar();
    mStates[from].mEdges.push_back(RegNfaEdge(s, ch));
    if (gAllocFailed) {
        return -1;
    }
    const u16 *set;
    if ((mFlags & FLAG_UNICODE_CASE) && (set = findCaseEquivSet(ch))) {
        int i = 0;
        while (true) {
            u16 other = set[i];
            if (other == 0) {
                break;
            }
            if (ch != other) {
                mStates[from].mEdges.push_back(RegNfaEdge(s, other));
                if (gAllocFailed) {
                    return -1;
                }
            }
            i++;
        }
    } else if ((mFlags & FLAG_UNICODE_CASE) ||
               (mFlags & FLAG_CASE_INSENSITIVE)) {
        const CaseMapEntry *entry =
            findCaseMapEntry(ch, !(mFlags & FLAG_UNICODE_CASE));
        if (entry) {
            mStates[from].mEdges.push_back(
                RegNfaEdge(s, (u16)(ch + entry->mDelta)));
            if (gAllocFailed) {
                return -1;
            }
        }
    }
    mpToken++;
    return s;
}

// 801EA6F4
int RegNfa::addAssertionEdge(int from, int type) {
    int s = newState();
    if (gAllocFailed) {
        return -1;
    }
    mStates[from].mEdges.push_back(RegNfaEdge(s, type));
    if (gAllocFailed) {
        return -1;
    }
    mpToken++;
    return s;
}

// 801EA7BC
int RegNfa::parseGroup(int from) {
    if (mpToken->mType != RegToken::TYPE_GROUP_OPEN) {
        return -1;
    }
    mpToken++;
    int result = parseAlternation(from, 1);
    if (gAllocFailed) {
        return -1;
    }
    if (mpToken->mType != RegToken::TYPE_GROUP_CLOSE) {
        mError = ERR_MISSING_GROUP_CLOSE;
        return -1;
    }
    mpToken++;
    return result;
}

// 801EA854
int RegNfa::addBracketClassEdge(int from, bool neg) {
    int s = newState();
    if (gAllocFailed) {
        return -1;
    }
    RegNfaEdge *edge = mStates[from].mEdges.push_back(RegNfaEdge(s));
    if (gAllocFailed) {
        return -1;
    }
    edge->mLabel.mType = RegLabel::TYPE_SET;
    if (neg) {
        parseNegBracketClass(edge->mLabel.mSet);
    } else {
        parseBracketClass(edge->mLabel.mSet);
    }
    return s;
}

// 801EA954
int RegNfa::addEscapeClassEdge(int from, bool neg) {
    int s = newState();
    if (gAllocFailed) {
        return -1;
    }
    RegNfaEdge *edge = mStates[from].mEdges.push_back(RegNfaEdge(s));
    if (gAllocFailed) {
        return -1;
    }
    edge->mLabel.mType = RegLabel::TYPE_SET;
    buildPredefinedClass(edge->mLabel.mSet, mpToken->getClassId());
    if (gAllocFailed) {
        return -1;
    }
    edge->mLabel.mSet.addCaseVariants(mFlags);
    if (gAllocFailed) {
        return -1;
    }
    if (neg) {
        edge->mLabel.mSet = edge->mLabel.mSet.complement();
        if (gAllocFailed) {
            return -1;
        }
    }
    mpToken++;
    return s;
}

// 801EAABC
int RegNfa::parseQuantifier(int start, int end) {
    switch (mpToken->mType) {
    case RegToken::TYPE_OPTIONAL:
        mpToken++;
        mStates[start].mEdges.push_back(RegNfaEdge(end));
        return gAllocFailed ? -1 : end;
    case RegToken::TYPE_PLUS:
        mpToken++;
        mStates[end].mEdges.push_back(RegNfaEdge(start));
        return gAllocFailed ? -1 : end;
    case RegToken::TYPE_STAR: {
        mpToken++;
        mStates[end].mEdges.push_back(RegNfaEdge(start));
        if (gAllocFailed) {
            return -1;
        }
        int s = newState();
        if (gAllocFailed) {
            return -1;
        }
        mStates[start].mEdges.push_back(RegNfaEdge(s));
        return gAllocFailed ? -1 : s;
    }
    case RegToken::TYPE_REPEAT_N:
        return parseRepeatN(start, end);
    case RegToken::TYPE_REPEAT_N_OR_MORE:
        return parseRepeatNOrMore(start, end);
    case RegToken::TYPE_REPEAT_RANGE:
        return parseRepeatRange(start, end);
    }
    return end;
}

// 801EACD0
int RegNfa::parseRepeatN(int start, int end) {
    if (mpToken->getMin() == 0) {
        Array<RegNfaEdge, 4> &edges = mStates[start].mEdges;
        while (edges.mSize != 0) {
            edges.erase(0);
        }
        edges.push_back(RegNfaEdge(end));
        if (gAllocFailed) {
            return -1;
        }
        mpToken++;
        return end;
    }
    int cur = end;
    for (int i = 1; i < mpToken->getMin(); i++) {
        RegNfaStateCopier copier(&mStates);
        if (copier.copy(start, end) == -1) {
            return -1;
        }
        mStates[cur].mEdges.push_back(RegNfaEdge(copier.lookup(start)));
        if (gAllocFailed) {
            return -1;
        }
        cur = copier.lookup(end);
    }
    mpToken++;
    return cur;
}

// 801EAE70
int RegNfa::parseRepeatNOrMore(int start, int end) {
    if (mpToken->getMin() == 0) {
        mStates[end].mEdges.push_back(RegNfaEdge(start));
        if (gAllocFailed) {
            return -1;
        }
        int s = newState();
        if (gAllocFailed) {
            return -1;
        }
        mStates[start].mEdges.push_back(RegNfaEdge(s));
        if (gAllocFailed) {
            return -1;
        }
        mpToken++;
        return s;
    }
    if (mpToken->getMin() == 1) {
        mpToken++;
        mStates[end].mEdges.push_back(RegNfaEdge(start));
        return gAllocFailed ? -1 : end;
    }
    int cur = end;
    for (int i = 1; i < mpToken->getMin(); i++) {
        RegNfaStateCopier copier(&mStates);
        if (copier.copy(start, end) == -1) {
            return -1;
        }
        int newStart = copier.lookup(start);
        int newEnd = copier.lookup(end);
        mStates[cur].mEdges.push_back(RegNfaEdge(newStart));
        if (gAllocFailed) {
            return -1;
        }
        cur = newEnd;
        if (i == mpToken->getMin() - 1) {
            mStates[newEnd].mEdges.push_back(RegNfaEdge(newStart));
            if (gAllocFailed) {
                return -1;
            }
        }
    }
    mpToken++;
    return cur;
}

// 801EB130
int RegNfa::parseRepeatRange(int start, int end) {
    int cur = end;
    int s = newState();
    if (gAllocFailed) {
        return -1;
    }
    if (mpToken->getMin() == 0) {
        if (mpToken->getMax() == 0) {
            Array<RegNfaEdge, 4> &edges = mStates[start].mEdges;
            while (edges.mSize != 0) {
                edges.erase(0);
            }
            edges.push_back(RegNfaEdge(end));
            if (gAllocFailed) {
                return -1;
            }
            mpToken++;
            return end;
        }
        mStates[start].mEdges.push_back(RegNfaEdge(s));
        if (gAllocFailed) {
            return -1;
        }
        mStates[end].mEdges.push_back(RegNfaEdge(s));
        if (gAllocFailed) {
            return -1;
        }
    } else if (mpToken->getMin() == 1) {
        mStates[end].mEdges.push_back(RegNfaEdge(s));
        if (gAllocFailed) {
            return -1;
        }
    }
    for (int i = 1; i < mpToken->getMax(); i++) {
        RegNfaStateCopier copier(&mStates);
        if (copier.copy(start, end) == -1) {
            return -1;
        }
        int newStart = copier.lookup(start);
        int newEnd = copier.lookup(end);
        mStates[cur].mEdges.push_back(RegNfaEdge(newStart));
        if (gAllocFailed) {
            return -1;
        }
        cur = newEnd;
        if (mpToken->getMin() - 1 <= i) {
            mStates[newEnd].mEdges.push_back(RegNfaEdge(s));
            if (gAllocFailed) {
                return -1;
            }
        }
    }
    mpToken++;
    return s;
}

// 801EB450
bool RegNfa::parseBracketClass(CharSet &out) {
    if (mpToken->mType != RegToken::TYPE_CLASS_OPEN) {
        return false;
    }
    mpToken++;
    parseClassBody(out);
    out.addCaseVariants(mFlags);
    if (mpToken->mType != RegToken::TYPE_CLASS_CLOSE) {
        mError = ERR_UNCLOSED_CLASS;
    }
    if (mError != 0) {
        return false;
    }
    mpToken++;
    return true;
}

// 801EB4FC
bool RegNfa::parseNegBracketClass(CharSet &out) {
    CharSet set;
    if (mpToken->mType != RegToken::TYPE_NEG_CLASS_OPEN) {
        return false;
    }
    mpToken++;
    parseClassBody(set);
    set.addCaseVariants(mFlags);
    if (mError != 0) {
        return false;
    }
    if (mpToken->mType != RegToken::TYPE_CLASS_CLOSE) {
        mError = ERR_UNCLOSED_CLASS;
        return false;
    }
    mpToken++;
    out = set.complement();
    return true;
}

// 801EB610
bool RegNfa::parseClassBody(CharSet &out) {
    int numItems = 0;
    do {
        switch (mpToken->mType) {
        case RegToken::TYPE_CLASS_CLOSE:
            if (numItems == 0) {
                mError = ERR_EMPTY_CLASS;
            } else {
                return true;
            }
            // fall through
        case RegToken::TYPE_CLASS_AND:
            mpToken++;
            if (numItems == 0) {
                if (mpToken->mType == RegToken::TYPE_CLASS_AND ||
                    mpToken->mType == RegToken::TYPE_CLASS_CLOSE) {
                    mError = ERR_EMPTY_CLASS;
                }
            } else if (mpToken->mType != RegToken::TYPE_CLASS_AND &&
                       mpToken->mType != RegToken::TYPE_CLASS_CLOSE) {
                CharSet rhs;
                if (!parseClassBody(rhs)) {
                    return false;
                }
                out.intersect(rhs);
                if (gAllocFailed) {
                    mError = ERR_NO_MEMORY;
                }
            }
            break;
        default:
            if (parseClassItem(out)) {
                numItems++;
            } else if (gAllocFailed) {
                mError = ERR_NO_MEMORY;
            }
            break;
        }
    } while (mError == 0);
    return false;
}

// 801EB788
bool RegNfa::parseClassItem(CharSet &out) {
    switch (mpToken->mType) {
    case RegToken::TYPE_CHAR:
        if (mpToken[1].mType == RegToken::TYPE_CLASS_DASH) {
            if (mpToken[2].mType == RegToken::TYPE_CHAR) {
                u16 lo = mpToken->getChar();
                u16 hi = mpToken[2].getChar();
                if (lo < hi) {
                    CharRange range(lo, hi);
                    out.add(range);
                    mpToken += 3;
                } else {
                    mError = ERR_BAD_RANGE;
                    return false;
                }
            } else {
                CharRange range(mpToken->getChar(), 0xFFFF);
                out.add(range);
                mpToken += 2;
            }
        } else {
            u16 ch = mpToken->getChar();
            CharRange range(ch, ch);
            out.add(range);
            mpToken++;
        }
        return !gAllocFailed;
    case RegToken::TYPE_CLASS_DASH: {
        CharRange range('-', '-');
        out.add(range);
        mpToken++;
        return !gAllocFailed;
    }
    case RegToken::TYPE_CLASS_ESCAPE:
        if (mpToken->getClassId() == CLASS_ANY) {
            CharRange range('.', '.');
            out.add(range);
        } else {
            CharSet set;
            buildPredefinedClass(set, mpToken->getClassId());
            if (gAllocFailed) {
                return false;
            }
            out.add(set);
        }
        mpToken++;
        return !gAllocFailed;
    case RegToken::TYPE_NEG_PROPERTY: {
        CharSet set;
        buildPredefinedClass(set, mpToken->getClassId());
        if (gAllocFailed) {
            return false;
        }
        set = set.complement();
        if (gAllocFailed) {
            return false;
        }
        out.add(set);
        if (gAllocFailed) {
            return false;
        }
        mpToken++;
        return true;
    }
    case RegToken::TYPE_CLASS_OPEN: {
        CharSet set;
        parseBracketClass(set);
        if (gAllocFailed) {
            return false;
        }
        out.add(set);
        if (gAllocFailed) {
            return false;
        }
        return true;
    }
    case RegToken::TYPE_NEG_CLASS_OPEN: {
        CharSet set;
        parseNegBracketClass(set);
        if (gAllocFailed) {
            return false;
        }
        out.add(set);
        if (gAllocFailed) {
            return false;
        }
        return true;
    }
    case RegToken::TYPE_LINE_START: {
        CharRange range('^', '^');
        out.add(range);
        mpToken++;
        return !gAllocFailed;
    }
    case RegToken::TYPE_LINE_END: {
        CharRange range('$', '$');
        out.add(range);
        mpToken++;
        return !gAllocFailed;
    }
    case RegToken::TYPE_PLUS: {
        CharRange range('+', '+');
        out.add(range);
        mpToken++;
        return !gAllocFailed;
    }
    case RegToken::TYPE_STAR: {
        CharRange range('*', '*');
        out.add(range);
        mpToken++;
        return !gAllocFailed;
    }
    case RegToken::TYPE_OPTIONAL: {
        CharRange range('?', '?');
        out.add(range);
        mpToken++;
        return !gAllocFailed;
    }
    case RegToken::TYPE_OR: {
        CharRange range('|', '|');
        out.add(range);
        mpToken++;
        return !gAllocFailed;
    }
    case RegToken::TYPE_GROUP_OPEN: {
        CharRange range('(', '(');
        out.add(range);
        mpToken++;
        return !gAllocFailed;
    }
    case RegToken::TYPE_GROUP_CLOSE: {
        CharRange range(')', ')');
        out.add(range);
        mpToken++;
        return !gAllocFailed;
    }
    case RegToken::TYPE_WORD_BOUNDARY:
    case RegToken::TYPE_NOT_WORD_BOUNDARY:
    case RegToken::TYPE_INPUT_START:
    case RegToken::TYPE_INPUT_END:
    case RegToken::TYPE_INPUT_END_TERM:
        mError = ERR_ASSERTION_IN_CLASS;
        return false;
    case RegToken::TYPE_END:
        mError = ERR_UNCLOSED_CLASS;
        return false;
    }
    return false;
}

// 801EBD4C
int RegNfa::newState() {
    {
        RegNfaState state;
        mStates.push_back(state);
    }
    return mStates.size() - 1;
}

// 801EBD9C
void RegNfa::reset() {
    mTokens.clear();
    mStates.clear();
    mAccept = -1;
    mpToken = NULL;
    mFlags = 0;
}

// 801EBDE8
inline RegNfaStateCopier::~RegNfaStateCopier() {
}

} // namespace RegEad
