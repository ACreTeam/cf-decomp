#pragma once

// RegEad character classes (src: lib/RegEad/RegEad.cpp, .text
// 801E39B0..801E4EF4). See RegEadArray.hpp.

#include <lib/RegEad/RegEadArray.hpp>

namespace RegEad {

// Inclusive UTF-16 code unit range.
struct CharRange {
    CharRange() : mLo(0), mHi(0) {
    }
    CharRange(u16 lo, u16 hi) : mLo(lo), mHi(hi) {
    } // 801F6A10 (weak copy in RegNfaExec)

    u16 getHi() const {
        return mHi;
    }

    bool operator<(const CharRange &other) const {
        return mLo < other.mLo;
    }
    bool contains(u16 ch) const {
        return mLo <= ch && ch <= mHi;
    }
    bool contains(const CharRange &other) const {
        return mLo <= other.mLo && other.mHi <= mHi;
    }
    bool overlaps(const CharRange &other) const {
        return contains(other.mLo) || contains(other.mHi) || other.contains(mLo) || other.contains(mHi);
    }

    /* 0x0 */ u16 mLo;
    /* 0x2 */ u16 mHi;
}; // Size: 0x4

// Simple case mapping: chars in [mLo, mHi] map to ch + mDelta.
struct CaseMapEntry {
    /* 0x0 */ u16 mLo;
    /* 0x2 */ u16 mHi;
    /* 0x4 */ s32 mDelta;
}; // Size: 0x8

// Case-folding flags (RegNfa::mFlags, CharSet::addCaseVariants).
enum {
    FLAG_CASE_INSENSITIVE = (1 << 0), // ASCII case folding
    // Unicode case folding (ignored without FLAG_CASE_INSENSITIVE)
    FLAG_UNICODE_CASE = (1 << 1),
};

extern CaseMapEntry sAsciiCaseMap[2];       // 80503DA0
extern CaseMapEntry sUnicodeCaseMap[0x310]; // 80503DB0
// Zero-terminated lists of characters that are all case variants of each other
// (e.g. I i U+0130 U+0131).
extern const u16 *sCaseEquivSets[19]; // 80505654

// Set of characters, kept as sorted non-overlapping ranges.
class CharSet {
  public:
    Array<CharRange> &getRanges() {
        return mRanges;
    }

    u32 getRangeCount() const {
        return mRanges.mSize;
    }

    CharSet();                                // 801E39B0
    ~CharSet();                               // 801E39E0
    CharSet &operator=(const CharSet &other); // 801E3CA8

    bool containsRange(const CharRange &range) const; // 801E3A38
    bool contains(u16 ch) const;                      // 801E3A9C
    bool intersects(const CharSet &other) const;      // 801E3B0C
    bool intersects(const CharRange &range) const;    // 801E3B94

    // Union-inserts the range; returns true if the set changed.
    bool add(const CharRange &range); // 801E3CD8
    bool addChar(u16 ch) {            // 801F6A1C (weak copy in RegNfaExec)
        CharRange range(ch, ch);
        return add(range);
    }
    // Adds the case variants of every character in the set (flags: FLAG_*).
    void addCaseVariants(u32 flags); // 801E3EF8
    bool addMappedRanges(const CharRange &range, const CaseMapEntry *map,
                         int num);               // 801E3FCC
    bool addEquivalents(const CharRange &range); // 801E417C
    bool add(const CharSet &other);              // 801E4264
    CharSet &intersect(const CharSet &other);    // 801E42F0
    CharSet complement() const;                  // over 0..0xFFFF 801E44EC
    CharSet &remove(u16 ch);                     // 801E4658
    CharSet &remove(const CharRange &range);     // 801E4750
    CharSet &subtract(const CharSet &other);     // 801E4934

    /* 0x0 */ Array<CharRange> mRanges;
}; // Size: 0x10

// Binary search of the ASCII or Unicode case map; NULL if ch has no mapping.
const CaseMapEntry *findCaseMapEntry(u32 ch, bool ascii); // 801E4E0C
// The sCaseEquivSets entry containing ch, or NULL.
const u16 *findCaseEquivSet(u32 ch); // 801E4E84

} // namespace RegEad
