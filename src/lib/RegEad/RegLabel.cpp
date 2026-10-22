#include <lib/RegEad/RegNfa.hpp>

namespace RegEad {

// 801E90DC
RegLabel::RegLabel() : mType(TYPE_EPSILON), mChar(0x3B5) {} // char 0x03B5 = ε

// 801E9120
RegLabel::RegLabel(u16 ch) : mType(TYPE_CHAR), mChar(ch) {
}

// 801E9160
RegLabel::RegLabel(const CharSet &set) : mType(TYPE_SET), mChar(0x96C6), mSet(set) {} // char 0x96C6 = 集

// 801E91A8
RegLabel::RegLabel(int type) : mType(type) {
}

// 801E91E0
RegLabel::RegLabel(const RegLabel &other) {
    *this = other;
}

// 801E922C
RegLabel &RegLabel::operator=(const RegLabel &other) {
    mChar = other.mChar;
    mType = other.mType;
    mSet = other.mSet;
    return *this;
}

// 801E9274
bool RegLabel::matches(u16 ch) const {
    switch (mType) {
    case TYPE_EPSILON:
        return true;
    case TYPE_CHAR:
        return mChar == ch;
    case TYPE_SET:
        return mSet.contains(ch);
    case TYPE_LINE_START:
    case TYPE_LINE_END:
    case TYPE_WORD_BOUNDARY:
    case TYPE_NOT_WORD_BOUNDARY:
    case TYPE_INPUT_START:
    case TYPE_INPUT_END:
    case TYPE_INPUT_END_TERM:
        return false;
    }
    return false;
}

// 801E92D4
bool RegLabel::isAssertion() const {
    switch (mType) {
    case TYPE_LINE_START:
    case TYPE_LINE_END:
    case TYPE_WORD_BOUNDARY:
    case TYPE_NOT_WORD_BOUNDARY:
    case TYPE_INPUT_START:
    case TYPE_INPUT_END:
    case TYPE_INPUT_END_TERM:
        return true;
    }
    return false;
}

} // namespace RegEad
