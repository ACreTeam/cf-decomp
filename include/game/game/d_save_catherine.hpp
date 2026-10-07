#pragma once

#include <types.h>
#include <game/game/d_personal_id.hpp>

#define CATHERINE_CATEGORY_NUM 26 // two groups of 13; 12 and 25 are not used
#define CATHERINE_SUB_NUM 8

// Katie (d_a_npc_sp_catherine), who travels between towns over Wi-Fi: dSaveShops_c::mCatherine. What the
// categories / sub-categories stand for is not known yet; they come from the Wi-Fi message handled by
// fn_80178AFC (type 0xA). Source: src/dol/game/d_save_catherine.cpp.
class dSaveCatherine_c { // 0x80
public:
    void clear();                                            // 80147A48
    BOOL isValid(int slot);                                  // 80147ACC: a person and something counted
    void set(const dPersonalID_c *person, u8 category, u8 sub); // 80147B2C: slot 0 for categories 0..12, else 1
    void count(u8 category, u8 sub);                         // 80147D18: a full counter is halved instead
    u8 getTopCategory(int group);                            // 80147D88: of categories 0..12 / 13..25
    int getTopSub();                                         // 80147E08

    /* 0x00 */ u8 mCount[CATHERINE_CATEGORY_NUM];
    /* 0x1A */ u8 mCountSub[CATHERINE_SUB_NUM];
    /* 0x22 */ dPersonalID_c mPersons[2];
    /* 0x7A */ s8 mCategory[2]; // -1 = none
    /* 0x7C */ s8 mSub[2];      // -1 = none
    /* 0x7E */ u8 _7E;
    /* 0x7F */ s8 mCounted;
};
