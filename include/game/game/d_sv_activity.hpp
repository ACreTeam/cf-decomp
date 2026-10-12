#pragma once

// d_sv_activity: the town's jinx ("charm"): the monthly roll, its outcomes coming true and what is in effect.
// Source: src/dol/game/d_sv_activity.cpp (.text 8014F8FC..80150140). No RTTI; the names are inferred.

#include <types.h>
#include <game/game/d_date.hpp>

class dTime_c;

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
// 2 bytes. Rolled by d_sv_activity; the town's own is dSvActivity_c::mJinx, other towns' are in mTownList.
struct dSaveJinx_c {
    dSaveJinx_c() : mCondition(0), mOutcome(0), mValue(0) {}

    // Keep the packed condition/outcome byte together for member-wise copies.
    struct {
        /* 0x0 */ u8 mCondition : 4; // dJinxCondition_e
        /* 0x0 */ u8 mOutcome : 4;   // dJinxOutcome_e
    };
    /* 0x1 */ u8 mValue;         // count (JINX_COND_PULL_WEEDS..PLANT_FLOWERS) or bug / fish (CATCH_BUG / CATCH_FISH)

    void clear(); // 8014F948: no jinx (JINX_COND_NUM, JINX_OUTCOME_NUM, value 0)
    void copy(const dSaveJinx_c& other) { *this = other; }
};

// One day's jinx results: the outcomes that came true and, for JINX_OUTCOME_FRIENDSHIP / JINX_OUTCOME_MONEY, which
// players fulfilled them.
struct dSvActivityResult_c {
    void clear() {
        mOutcomes = 0;
        mFriendshipPlayers = 0;
        mMoneyPlayers = 0;
    }
    void removePlayer(int player); // 801500F8: also drops the outcome once no player is left

    /* 0x0 */ u16 mOutcomes;         // a bit per dJinxOutcome_e
    /* 0x2 */ u8 mFriendshipPlayers; // a bit per player
    /* 0x3 */ u8 mMoneyPlayers;
}; // size 0x4

// The town's jinx (dSaveTown_c::mActivity). A new one is rolled each month (updateMonth). Fulfilling it records the
// outcome in mToday, or in mPending before the day change ran; the day change (advanceDays) applies mToday's
// outcomes (mActive) and moves mPending into mToday. Only mDropDate and mJinx are constructed.
struct dSvActivity_c {
    void clear();                                           // 8014F96C
    void updateMonth(const dTime_c *now);                   // 8014F998: a new jinx each month
    void setVisitorDone(BOOL done);                         // 8014FBF0: a visitor fulfilled this town's jinx
    void setDone(int outcome, BOOL done, int player);       // 8014FCA0: the player fulfilled a jinx (outcome)
    void advanceDays(dTime_c last, dTime_c *now, int *days); // 8014FD48: the day change, days days after last
    void applyOutcome(const int &outcome, const dTime_c &time, const int &friendshipPlayers,
                      const int &moneyPlayers);             // 8014FF14
    void setTodayOutcomes(u16 outcomes);                    // 80150080 (dActivityLog restores it after travelling)
    void removePlayer(int player);                          // 80150088

    /* 0x00 */ dYMD_c mDropDate;              // JINX_OUTCOME_DROPPED_ITEMS: the day it came true (lasts a week)
    /* 0x04 */ int mYear;                     // the jinx's month (-1: none)
    /* 0x08 */ dSvActivityResult_c mPending;  // fulfilled before today's day change ran
    /* 0x0C */ dSvActivityResult_c mToday;    // applied by the next day change
    /* 0x10 */ u16 mActive;                   // the outcomes in effect, a bit per dJinxOutcome_e
    /* 0x12 */ dSaveJinx_c mJinx;             // this town's jinx (talk topic FreeG_JinxV)
    /* 0x14 */ u8 mMonth;
    /* 0x15 */ u8 mMoneyPlayers;              // JINX_OUTCOME_MONEY is in effect for these players
    /* 0x16 */ u8 _16[2];
}; // size 0x18
