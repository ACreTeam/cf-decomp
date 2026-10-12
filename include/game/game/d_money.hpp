#pragma once

// The pocket money display (DOL TU d_money.cpp, .text 8019BD78..8019C414; RTTI "dMoney_c"): the
// "Layout/money.arc" layout that shows the current player's pocket money in shops and while forced on
// (onForceShow), counting up or down to a new amount. Member and function names are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>

class dMoney_c : public fBase_c {
public:
    enum STATE_e {
        STATE_HIDDEN,
        STATE_SHOWN,
    };

    // The display flags (a file-local u32 in d_money.cpp).
    enum FLAG_e {
        FLAG_SHOW = 1,       // drawn
        FLAG_CALC = 2,       // the layout needs a calc
        FLAG_FORCE_SHOW = 4, // shown outside shops too (onForceShow)
        FLAG_DISABLE = 8,    // never shown (onDisable)
    };

    virtual int create();   // 8019BDF4
    virtual int doDelete(); // 8019BE9C
    virtual int execute();  // 8019BECC
    virtual int draw();     // 8019BF40
    virtual ~dMoney_c() {} // 8019C38C

    void stateShown();  // 8019BFA0: hides the display once isShow() is false
    void stateHidden(); // 8019BFE8: shows the display once isShow() is true

    static BOOL isShow(); // 8019C068
    static int getMoney(); // 8019C138: the current player's pocket money (0 without a player)
    void setNumber();      // 8019C16C: "T_money_00"
    void resetMoney();     // 8019C1B0: shows the current amount at once
    void updateMoney();    // 8019C1F0: counts the shown amount towards the current one

    static void onForceShow();  // 8019C34C (d_npc_talk_fmarket, d_npc_talk_quest_q09 trade)
    static void offForceShow(); // 8019C35C
    static void onDisable();    // 8019C36C
    static void offDisable();   // 8019C37C

    /* 0x064 */ m2d::ResAccLoader_c mResAcc;
    /* 0x138 */ m2d::SimpleEx_c mLyt;
    /* 0x1D0 */ m2d::AnmEx_c mAnm;
    /* 0x20C */ int mState;
    /* 0x210 */ int mDispMoney;  // the amount shown
    /* 0x214 */ int mCountTimer; // frames left of the count
    /* 0x218 */ int mCountStep;  // amount per frame
}; // size 0x21C
