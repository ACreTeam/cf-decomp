#pragma once

// d_sv_star_sign (DOL TU d_sv_star_sign.cpp, .text 80150A5C..801510A0): constellations (dSvStarSign). A player
// draws one with Celeste (dPrivateData_c::mStarSign, +0x5560); registering it copies it into the town's list
// of SIGN_NUM (dSaveTown_c::mStarSigns, +0x72E0A), which the night sky draws (d_star_draw). No RTTI: names
// inferred (the TU was split as d_sv_unk5560).

#include <types.h>
#include <game/game/d_personal_id.hpp>

namespace dSvStarSign {

enum {
    SIGN_LINE_NUM = 16, // lines per constellation
    SIGN_NAME_LEN = 16, // name characters (without the terminator)
    SIGN_NUM = 16,      // constellations in the town
    SIGN_LINE_NONE = 0xFFFF,
};

// A constellation: its owner, its lines (indices into dStarDraw_c::sLines) and its name.
struct Sign_c {
    Sign_c();                                 // 80150A5C
    ~Sign_c();                                // 80150A60
    Sign_c *copy(const Sign_c *other);        // 80150AA0: returns this
    void clear();                             // 80150AD4: no owner, no lines, no name
    void setName(const wchar_t *name);        // 80150B58: SIGN_NAME_LEN characters
    const wchar_t *getName() const;           // 80150B6C
    void setLine(int idx, u16 line);          // 80150B74
    void clearLine(int idx);                  // 80150B84

    /* 0x00 */ dPersonalID_c mOwner;
    /* 0x2C */ u16 mLines[SIGN_LINE_NUM];     // SIGN_LINE_NONE if unused
    /* 0x4C */ wchar_t mName[SIGN_NAME_LEN + 1];
}; // size 0x6E

// The town's constellations.
struct List_c {
    List_c();                                 // 80150B94
    void clear();                             // 80150BDC
    void set(int idx, const Sign_c *sign);    // 80150C28: also marks it new for every player
    void remove(int idx);                     // 80150CA4
    const Sign_c *get(int idx) const;         // 80150D24
    Sign_c *get(int idx);                     // 80150D30
    BOOL isValid(int idx) const;              // 80150D3C: it has an owner
    int findFree() const;                     // 80150D48: -1 if none
    int getRandom(int exclude) const;         // 80150DA8: a random valid one other than exclude, -1 if none
    void clearNew(int player);                // 80150E74
    // Adds the current player's constellation to the town's (unless one is linked to it or has its name) and
    // clears it.
    static void registerPlayerSign();         // 80150E88

    /* 0x000 */ Sign_c mSigns[SIGN_NUM];
    /* 0x6E0 */ u16 mNew[4];    // per player: 1 << idx of the constellations not seen yet
}; // size 0x6E8

} // namespace dSvStarSign
