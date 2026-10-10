#pragma once

// The fishing float ("uki"). DOL TU d_uki.cpp (.text 8016DE60..801711EC), not decompiled. The class
// name is inferred from the TU name (no RTTI). Each player's float lives in their dHmnToolBank_c
// (d_hmn_tool_mng) and is also the slingshot's shot. Only what other TUs use; unnamed functions keep
// their addresses.

#include <types.h>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>

class dFishFldShadow_c;
namespace dItem {
class Item;
}

class dUki_c {
public:
    dUki_c();  // 8016DE60
    ~dUki_c(); // 8016DF98

    void fn_8016E0C0(int slot);                         // init
    void fn_8016E148();                                 // remove
    void fn_8016E1AC(int type, const dItem::Item *item); // create (0..2 rod grade, 3 slingshot)
    void fn_8016E37C();                                 // put away
    void fn_8016E380(const mMtx_c *mtx);                // calc at the rod tip
    void fn_8016E5D0();                                 // draw
    void fn_8016E680(const mMtx_c *mtx);                // set the matrix
    void fn_8016E7E4();                                 // change
    void fn_8016FC44(int state);
    void fn_8016FD60();                                 // the float bobs (ripple)
    BOOL fn_8016FED4() const;                           // the float just hit the water

    // The player's float while it is free for fish (or fish is on it), NULL if none.
    static dUki_c *fn_801710A4(int player, dFishFldShadow_c *fish);
    // Put fish on the player's float (NULL: off).
    static void fn_801710BC(int player, dFishFldShadow_c *fish);
    static int fn_801710F4(); // heap size of a float

    /* 0x000 */ u8 _000[0x270];
    /* 0x270 */ mVec3_c mPos;
    /* 0x27C */ u8 _27C[0x446 - 0x27C];
    /* 0x446 */ u8 mCastState; // 1, 2: in the water (2: cast long, a longer bite window)
    /* 0x447 */ u8 _447;       // 9: the fish escapes instead of biting (executeNibble)
    /* 0x448 */ u8 _448[0x450 - 0x448];
}; // size 0x450 (dHmnToolBank_c: 0x1C..0x46C)
