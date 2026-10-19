#pragma once

#include <types.h>
#include <game/game/d_personal_id.hpp>

#define CATHERINE_CATEGORY_NUM 26 // two groups of 13; 12 and 25 are not used
#define CATHERINE_SUB_NUM 8

// Harriet's record of the town's hair trends (d_a_npc_sp_catherine, the Shampoodle hairdresser):
// dSaveShops_c::mCatherine. A category is a hairstyle (dPrivateData_c::mHair: 0..12 the boys', 13..25 the
// girls' styles; slot / group 0 = boys, 1 = girls), a sub-category a hair colour (mHairColor). mPersons /
// mCategory / mSub: the "masterpiece" she made for a player of a friend town; they come from the Wi-Fi
// message handled by fn_80178AFC (type 0xA). Source: src/dol/game/d_save_catherine.cpp.
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
