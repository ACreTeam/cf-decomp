#pragma once

// RegEad NFA: the pattern compiler (src: lib/RegEad/RegNfa.cpp, .text
// 801E92FC..801ECD90, and lib/RegEad/RegNfaExec.cpp, .text 801ECD90..801F7FC4,
// the predefined character classes) and the transition label shared with the
// DFA (src: lib/RegEad/RegLabel.cpp, .text 801E90DC..801E92FC).

#include <lib/RegEad/RegEad.hpp>

namespace RegEad {

// Transition label: what an NFA edge / DFA transition consumes or asserts.
class RegLabel {
  public:
    enum Type {
        TYPE_EPSILON = 0,
        TYPE_CHAR = 1,
        TYPE_SET = 2,
        // Zero-width assertions (types 3..9)
        TYPE_LINE_START = 3,        // ^
        TYPE_LINE_END = 4,          // $
        TYPE_WORD_BOUNDARY = 5,     // \b (word = non-whitespace)
        TYPE_NOT_WORD_BOUNDARY = 6, // \B
        TYPE_INPUT_START = 7,       // \A
        TYPE_INPUT_END = 8,         // \z
        TYPE_INPUT_END_TERM = 9,    // \Z
    };

    RegLabel();                      // 801E90DC: epsilon (mChar = U+03B5)
    RegLabel(u16 ch);                // 801E9120
    RegLabel(const CharSet &set);    // 801E9160 (mChar = U+96C6)
    RegLabel(int type);              // 801E91A8
    RegLabel(const RegLabel &other); // 801E91E0
    RegLabel &operator=(const RegLabel &other); // 801E922C

    // Whether the label consumes ch (epsilon always does, assertions never do).
    bool matches(u16 ch) const; // 801E9274
    bool isAssertion() const;   // 801E92D4

    /* 0x0 */ int mType;
    /* 0x4 */ u16 mChar;
    /* 0x8 */ CharSet mSet;
}; // Size: 0x18

// Lexer token (RegNfa::mTokens).
class RegToken {
  public:
    enum Type {
        TYPE_CHAR = 1,           // literal mChar
        TYPE_OR = 2,             // |
        TYPE_CLASS_OPEN = 3,     // [
        TYPE_NEG_CLASS_OPEN = 4, // [^
        TYPE_CLASS_CLOSE = 5,    // ]
        TYPE_CLASS_DASH = 6,     // -
        TYPE_CLASS_AND = 7,      // &&
        // . \d \D \s \S \w \W \p{..}: getClassId() = CharClassId
        TYPE_CLASS_ESCAPE = 8,
        TYPE_NEG_PROPERTY = 9,       // \P{..}
        TYPE_GROUP_OPEN = 10,        // (
        TYPE_GROUP_CLOSE = 11,       // )
        TYPE_OPTIONAL = 12,          // ?
        TYPE_STAR = 13,              // *
        TYPE_PLUS = 14,              // +
        TYPE_REPEAT_N = 15,          // {n}
        TYPE_REPEAT_N_OR_MORE = 16,  // {n,}
        TYPE_REPEAT_RANGE = 17,      // {n,m}
        TYPE_LINE_START = 18,        // ^
        TYPE_LINE_END = 19,          // $
        TYPE_WORD_BOUNDARY = 20,     // \b
        TYPE_NOT_WORD_BOUNDARY = 21, // \B
        TYPE_INPUT_START = 22,       // \A
        TYPE_INPUT_END = 23,         // \z
        TYPE_INPUT_END_TERM = 24,    // \Z
        TYPE_END = 25,               // appended by RegNfa::tokenize
    };

    RegToken();                              // 801F7F50
    RegToken(int type);                      // 801F7F68
    RegToken &operator=(const RegToken &other); // 801F7F80

    // Two distinct accessors read +0x4 (801F7FAC / 801F7FBC); which is which is
    // a guess.
    u16 getChar() const; // 801F7FA4
    int getMin() const;  // 801F7FAC
    int getMax() const;  // 801F7FB4
    int getClassId() const; // 801F7FBC

    /* 0x0 */ u16 mChar;
    /* 0x4 */ int mMin; // {n,m} n, or the CharClassId
    /* 0x8 */ int mMax; // {n,m} m
    /* 0xC */ int mType;
}; // Size: 0x10
typedef Array<RegToken> RegTokenArray;

// Predefined character classes (RegNfa::buildPredefinedClass /
// lookupClassName).
enum CharClassId {
    CLASS_NONE = 0,
    CLASS_ANY = 1, // . (all but line terminators)
    CLASS_DIGIT = 2,
    CLASS_NOT_DIGIT = 3,
    CLASS_SPACE = 4,
    CLASS_NOT_SPACE = 5,
    CLASS_WORD = 6,
    CLASS_NOT_WORD = 7,
    CLASS_LOWER = 8,
    CLASS_UPPER = 9,
    CLASS_ASCII = 10, // also InBasicLatin
    CLASS_ALPHA = 11,
    CLASS_ALNUM = 12,
    CLASS_PUNCT = 13,
    CLASS_GRAPH = 14,
    CLASS_PRINT = 15,
    CLASS_BLANK = 16,
    CLASS_CNTRL = 17,
    CLASS_XDIGIT = 18,
    // Unicode blocks (\p{In..})
    CLASS_IN_LATIN_1_SUPPLEMENT = 19,
    CLASS_IN_LATIN_EXTENDED_A,
    CLASS_IN_IPA_EXTENSIONS,
    CLASS_IN_SPACING_MODIFIER_LETTERS,
    CLASS_IN_COMBINING_DIACRITICAL_MARKS,
    CLASS_IN_GREEK,
    CLASS_IN_CYRILLIC,
    CLASS_IN_ARMENIAN,
    CLASS_IN_HEBREW,
    CLASS_IN_ARABIC,
    CLASS_IN_SYRIAC,
    CLASS_IN_THAANA,
    CLASS_IN_DEVANAGARI,
    CLASS_IN_BENGALI,
    CLASS_IN_GURMUKHI,
    CLASS_IN_GUJARATI,
    CLASS_IN_ORIYA,
    CLASS_IN_TAMIL,
    CLASS_IN_TELUGU,
    CLASS_IN_KANNADA,
    CLASS_IN_MALAYALAM,
    CLASS_IN_SINHALA,
    CLASS_IN_THAI,
    CLASS_IN_LAO,
    CLASS_IN_TIBETAN,
    CLASS_IN_MYANMAR,
    CLASS_IN_GEORGIAN,
    CLASS_IN_HANGUL_JAMO,
    CLASS_IN_ETHIOPIC,
    CLASS_IN_CHEROKEE,
    CLASS_IN_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS,
    CLASS_IN_OGHAM,
    CLASS_IN_RUNIC,
    CLASS_IN_KHMER,
    CLASS_IN_MONGOLIAN,
    CLASS_IN_LATIN_EXTENDED_ADDITIONAL,
    CLASS_IN_GREEK_EXTENDED,
    CLASS_IN_GENERAL_PUNCTUATION,
    CLASS_IN_SUPERSCRIPTS_AND_SUBSCRIPTS,
    CLASS_IN_CURRENCY_SYMBOLS,
    CLASS_IN_LETTERLIKE_SYMBOLS,
    CLASS_IN_NUMBER_FORMS,
    CLASS_IN_ARROWS,
    CLASS_IN_MATHEMATICAL_OPERATORS,
    CLASS_IN_MISCELLANEOUS_TECHNICAL,
    CLASS_IN_CONTROL_PICTURES,
    CLASS_IN_OPTICAL_CHARACTER_RECOGNITION,
    CLASS_IN_ENCLOSED_ALPHANUMERICS,
    CLASS_IN_BOX_DRAWING,
    CLASS_IN_BLOCK_ELEMENTS,
    CLASS_IN_GEOMETRIC_SHAPES,
    CLASS_IN_MISCELLANEOUS_SYMBOLS,
    CLASS_IN_DINGBATS,
    CLASS_IN_BRAILLE_PATTERNS,
    CLASS_IN_CJK_RADICALS_SUPPLEMENT,
    CLASS_IN_KANGXI_RADICALS,
    CLASS_IN_IDEOGRAPHIC_DESCRIPTION_CHARACTERS,
    CLASS_IN_CJK_SYMBOLS_AND_PUNCTUATION,
    CLASS_IN_HIRAGANA,
    CLASS_IN_KATAKANA,
    CLASS_IN_BOPOMOFO,
    CLASS_IN_HANGUL_COMPATIBILITY_JAMO,
    CLASS_IN_KANBUN,
    CLASS_IN_BOPOMOFO_EXTENDED,
    CLASS_IN_ENCLOSED_CJK_LETTERS_AND_MONTHS,
    CLASS_IN_CJK_COMPATIBILITY,
    CLASS_IN_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A,
    CLASS_IN_CJK_UNIFIED_IDEOGRAPHS,
    CLASS_IN_YI_SYLLABLES,
    CLASS_IN_YI_RADICALS,
    CLASS_IN_HANGUL_SYLLABLES,
    CLASS_IN_HIGH_SURROGATES,
    CLASS_IN_HIGH_PRIVATE_USE_SURROGATES,
    CLASS_IN_LOW_SURROGATES,
    CLASS_IN_CJK_COMPATIBILITY_IDEOGRAPHS,
    CLASS_IN_ALPHABETIC_PRESENTATION_FORMS,
    CLASS_IN_ARABIC_PRESENTATION_FORMS_A,
    CLASS_IN_COMBINING_HALF_MARKS,
    CLASS_IN_CJK_COMPATIBILITY_FORMS,
    CLASS_IN_SMALL_FORM_VARIANTS,
    CLASS_IN_HALFWIDTH_AND_FULLWIDTH_FORMS,
    CLASS_IN_SPECIALS,
    // Unicode general categories
    CLASS_L = 101,
    CLASS_LL,
    CLASS_LU,
    CLASS_LT,
    CLASS_LM,
    CLASS_LO,
    CLASS_M,
    CLASS_MN,
    CLASS_MC,
    CLASS_ME,
    CLASS_Z,
    CLASS_ZS,
    CLASS_ZL,
    CLASS_ZP,
    CLASS_S,
    CLASS_SM,
    CLASS_SC,
    CLASS_SK,
    CLASS_SO,
    CLASS_N,
    CLASS_ND,
    CLASS_NL,
    CLASS_NO,
    CLASS_P,
    CLASS_PD,
    CLASS_PS,
    CLASS_PE,
    CLASS_PC,
    CLASS_PO,
    CLASS_C,
    CLASS_CC,
    CLASS_CF,
    CLASS_CO,
    CLASS_CN,
    CLASS_CS, // 135
};

struct RegNfaEdge {
    RegNfaEdge();                       // 801EC588
    RegNfaEdge(int target);             // 801EC5C4: epsilon
    RegNfaEdge(int target, u16 ch);     // 801EC5FC
    RegNfaEdge(int target, int type);   // 801EC638
    RegNfaEdge(int target, const RegLabel &label); // 801EC674
    RegNfaEdge &operator=(const RegNfaEdge &other); // 801EC6B0

    /* 0x0 */ int mTarget; // state index
    /* 0x4 */ RegLabel mLabel;
}; // Size: 0x1C
typedef Array<RegNfaEdge, 4> RegNfaEdgeArray;

class RegNfaState {
  public:
    RegNfaState();                                  // 801EC6F0
    virtual ~RegNfaState();                         // 801EC730
    RegNfaState &operator=(const RegNfaState &other); // 801EC78C

    /* 0x0 vtable */
    /* 0x4 */ RegNfaEdgeArray mEdges;
}; // Size: 0x14
typedef Array<RegNfaState> RegNfaStateArray;


// Pattern -> NFA compiler (recursive descent over the token list).
class RegNfa {
  public:
    // mError
    enum Error {
        ERR_NONE = 0,
        ERR_NO_MEMORY = 1,
        ERR_BAD_ESCAPE = 2, // unknown alphabetic escape or \1-\9
        ERR_TRAILING_BACKSLASH = 3,
        ERR_BAD_REPEAT = 4, // {..}
        ERR_BAD_HEX = 5,    // \x \u
        ERR_BAD_OCTAL = 6,
        ERR_BAD_CONTROL = 7, // \c
        ERR_NO_PROPERTY_NAME = 8,
        ERR_UNCLOSED_PROPERTY = 9, // \p{
        ERR_UNKNOWN_PROPERTY = 10,
        ERR_UNMATCHED_QUOTE_END = 11, // \E
        ERR_DANGLING_QUANTIFIER = 12,
        ERR_MISSING_GROUP_CLOSE = 13,
        ERR_UNMATCHED_GROUP_CLOSE = 14,
        ERR_UNCLOSED_CLASS = 15,
        ERR_STRAY_CLASS_CLOSE = 16,
        ERR_EMPTY_CLASS = 17,
        ERR_ASSERTION_IN_CLASS = 18,
        ERR_BAD_RANGE = 19,
    };

    RegNfa();          // 801E92FC
    virtual ~RegNfa(); // 801E9364

    virtual bool compile(const u16 *pattern, u32 flags); // 801E93E4
    virtual void reset(); // 801EBD9C (inline, emitted weak)
    virtual int getError() const {
        return mError;
    } // 801EBE44
    // Defined (inline) in RegDfa.cpp, its only user; the vtable here
    // resolves to RegDfa's weak copies.
    virtual const RegNfaStateArray &getStates() const; // 801E6724
    virtual int getAcceptState() const;                // 801E672C

    virtual void tokenize(const u16 *pattern);              // 801E9518
    virtual const u16 *tokenizeNext(const u16 *p);          // 801E9664
    virtual int countDigits(const u16 *p);                  // 801E9E4C
    virtual int parseDecimal(const u16 *p, int num);        // 801E9E7C
    virtual bool parseHex(const u16 *p, int num, u16 *out); // 801E9D90

    // These return the state the parsed fragment ends in (from = the state it
    // starts from).
    virtual int parseAlternation(int from, int nested); // 801E9FF8
    virtual int parseSequence(int from, int nested);    // 801EA174
    virtual int parseAtom(int from, int nested);        // 801EA23C
    virtual int parseLiteral(int from);                 // 801EA534
    virtual int parseGroup(int from);                   // 801EA7BC
    virtual int parseQuantifier(int start, int end);    // 801EAABC
    virtual int parseRepeatN(int start, int end);       // 801EACD0
    virtual int parseRepeatNOrMore(int start, int end); // 801EAE70
    virtual int parseRepeatRange(int start, int end);   // 801EB130

    virtual bool parseBracketClass(CharSet &out);    // 801EB450
    virtual bool parseNegBracketClass(CharSet &out); // 801EB4FC
    virtual bool parseClassBody(CharSet &out);       // 801EB610
    virtual bool parseClassItem(CharSet &out);       // 801EB788

    // Adds the ranges of a CharClassId to out (RegNfaExec.cpp).
    virtual void buildPredefinedClass(CharSet &out, u32 classId); // 801ECD90
    // \p{name} / \pX name -> CharClassId, 0 if unknown (RegNfaExec.cpp).
    virtual int lookupClassName(const u16 *name, int len,
                                bool shortForm); // 801F6A48

    virtual int newState(); // 801EBD4C (inline, emitted weak)
    virtual int addAssertionEdge(int from, int type);    // 801EA6F4
    virtual int addBracketClassEdge(int from, bool neg); // 801EA854
    virtual int addEscapeClassEdge(int from, bool neg);  // 801EA954

    /* 0x00 vtable */
    /* 0x04 */ RegTokenArray mTokens;
    /* 0x14 */ RegTokenArray::const_iterator mpToken; // parse cursor
    /* 0x18 */ RegNfaStateArray mStates; // state 0 is the start state
    /* 0x28 */ int mAccept;
    /* 0x2C */ u32 mFlags; // FLAG_*
    /* 0x30 */ int mError; // Error
}; // Size: 0x34

// Old -> new state index pair (RegNfaStateCopier::mMap).
struct RegIntPair {
    RegIntPair();                                  // 801ECBEC
    RegIntPair(int first, int second);             // 801ECBFC
    RegIntPair &operator=(const RegIntPair &other); // 801ECC08

    /* 0x0 */ int mFirst;
    /* 0x4 */ int mSecond;
}; // Size: 0x8

// Deep-copies an NFA sub-graph (for {n,m} repeats).
class RegNfaStateCopier {
  public:
    RegNfaStateCopier(RegNfaStateArray *states); // 801EC998
    virtual ~RegNfaStateCopier();                 // 801EBDE8

    // Copies the states reachable from start (stopping at end); returns the
    // copy of start or -1.
    int copy(int start, int end); // 801EC9E8
    int lookup(int from);   // 801ECBA0

    /* 0x00 vtable */
    /* 0x04 */ Array<RegIntPair> mMap;
    /* 0x14 */ RegNfaStateArray *mStates;
}; // Size: 0x18

// Index of c in the zero-terminated s, or -1.
int findChar(const u16 *s, u16 c); // 801E9F64
// True if s[0..len) equals the zero-terminated name exactly.
bool equalsName(const u16 *s, const u16 *name, int len); // 801E9F98

} // namespace RegEad
