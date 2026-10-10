#pragma once

// d_sv_town_info: per-town state kept with the town field (dSaveTown_c::mTownInfo, +0x68372): the last
// day change, the native fruit, the day-end events, the money tree and lamp days and the perfect-town
// streak. TU d_sv_town_info.cpp (.text 8014D0BC..8014D900). The class name is inferred (no RTTI).

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_time_stamp.hpp>

#define TOWN_INFO_END_EVENT_NUM 4      // mEndEvents
#define TOWN_INFO_END_EVENT_NONE 0xFFFF // an empty mEndEvents slot

class dSaveTownInfo_c {
public:
    dSaveTownInfo_c();                          // 8014D0BC
    void init();                                // 8014D27C: a new town (mFruit unset): day, fruit, field setup
    void updatePerfectDays(int rank, int days); // 8014D3F4: mPerfectDays after a day change's assessment
    void plantFruitTrees();                     // 8014D42C: every fruit tree (not palms) bears mFruit
    void removeSouthCedars();                   // 8014D5C4: cedars below the cedar rows become trees
    void setMoneyTreeRolled();                  // 8014D69C: the current player's money tree chance is used today
    BOOL canRollMoneyTree();                    // 8014D740: the current player's money tree chance isn't used today
    void clearEndEvents();                      // 8014D828
    int findEndEvent(int id);                   // 8014D844: slot of event id, -1 if absent
    int addEndEvent(int id);                    // 8014D89C: into a free slot unless already there; its slot or -1

    /* 0x00 */ dTimeStamp_c mLastDay;                   // the last day change (6:00 of that game day)
    /* 0x08 */ dPersonalID_c mEggPlayer;                // Bunny Day: the player credited with the found eggs
    /* 0x34 */ dYMD_c mMoneyTreeDates[PLAYER_NUM];      // the day each player last had the money tree chance
    /* 0x44 */ dYMD_c mEventDay;                        // the last day dFgMngProc_c::procDay handled
    /* 0x48 */ u16 mEndEvents[TOWN_INFO_END_EVENT_NUM]; // EVENT_* to end at the next day change
    /* 0x50 */ u16 mFruit;                              // the native fruit's item id (0 / ITEM_ID_NONE: no town yet)
    /* 0x52 */ dMD_c mLampDate;                         // Wisp's lamp day (month 12, day 0: none)
    /* 0x54 */ s8 mPerfectDays;                         // days the town has been TOWN_RANK_PERFECT, -1: not perfect
}; // size 0x56
