#pragma once

// d_sv_activity (DOL TU d_sv_activity.cpp, .text 8014F8FC..80150140, not decompiled). Only what other TUs use.

#include <types.h>

// A town's jinx (called a "charm" in the English game): a superstition villagers pass on about a town,
// "If you <condition> in <town>... <outcome>!" (message labels FreeA_Jinx for another town, FreeG_JinxV for
// this one; condition text = message code 11 + condition, outcome text = 31 + outcome). Names below follow
// the English message text.
enum dJinxCondition_e {
    JINX_COND_HEART_LETTER,     // 0 send a heart-filled letter from the town hall
    JINX_COND_CUT_TREE,         // 1 cut down a single tree
    JINX_COND_PULL_WEEDS,       // 2 go on a weed-plucking spree (mValue weeds)
    JINX_COND_PLANT_TREES,      // 3 plant exactly mValue trees
    JINX_COND_PLANT_FLOWERS,    // 4 plant exactly mValue flowers
    JINX_COND_FOUR_LEAF_CLOVER, // 5 find a four-leaf clover
    JINX_COND_6,                // 6 no message text (unused)
    JINX_COND_RED_TURNIP,       // 7 plant a red turnip seed
    JINX_COND_CATCH_BUG,        // 8 catch a bug (mValue: index from ITEM_IDX_COMMON_BUTTERFLY)
    JINX_COND_CATCH_FISH,       // 9 catch a fish (mValue: index from ITEM_IDX_BITTERLING)
    JINX_COND_MONEY_TREE,       // 10 plant a money tree
    JINX_COND_NUM
};
enum dJinxOutcome_e {
    JINX_OUTCOME_NO_BEES,       // 0 all the bees will disappear from your own town
    JINX_OUTCOME_TOWN_IMPROVES, // 1 the condition of your own town will improve
    JINX_OUTCOME_GYROIDS,       // 2 you'll find a lot of gyroids
    JINX_OUTCOME_FOSSILS,       // 3 it will be easier to find fossils
    JINX_OUTCOME_TREE_LUCK,     // 4 happiness will fall from the trees
    JINX_OUTCOME_RARE_ITEM,     // 5 a chance to get a rare item
    JINX_OUTCOME_FRIENDSHIP,    // 6 your friendships will become stronger
    JINX_OUTCOME_FLOWERS,       // 7 flowers will become less likely to wilt
    JINX_OUTCOME_MONEY,         // 8 your financial outlook will improve
    JINX_OUTCOME_DROPPED_ITEMS, // 9 more things will be dropped in your town
    JINX_OUTCOME_NUM
};
// 2 bytes. Rolled by d_sv_activity; the town's own is dSaveTown_c::mJinx, other towns' are in mTownList.
struct dSaveJinx_c {
    /* 0x0 */ u8 mCondition : 4; // dJinxCondition_e
    /* 0x0 */ u8 mOutcome : 4;   // dJinxOutcome_e
    /* 0x1 */ u8 mValue;         // count (JINX_COND_PULL_WEEDS..PLANT_FLOWERS) or bug / fish (CATCH_BUG / CATCH_FISH)

    void clear(); // 8014F948: no jinx (JINX_COND_NUM, JINX_OUTCOME_NUM, value 0)
};

// The jinx results block at dSaveTown_c+0x5EC64 (dSaveTown_c::_05EC64; layout not recovered yet). The town's own
// jinx (dSaveTown_c::mJinx) is its +0x12; +0x8 and +0xC are bit sets indexed by dJinxOutcome_e.
extern "C" {
void fn_8014FBF0(void *results, BOOL done);                         // 8014FBF0: a visitor fulfilled this town's jinx
void fn_8014FCA0(void *results, int outcome, BOOL done, int player); // 8014FCA0: the player fulfilled a jinx (outcome)
void fn_80150080(void *results, u16 bits);                          // 80150080: sets +0xC
}
