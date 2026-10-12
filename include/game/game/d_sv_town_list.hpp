#pragma once

// d_sv_town_list (DOL TU d_sv_town_list.cpp, .text 80150140..801503E4, .sdata2 80750CD0..80750CD8): up to
// TOWN_LIST_NUM other towns the player knows about, with their jinxes (dSaveTown_c::mTownList; talk topic
// FreeA_Jinx). No RTTI: names inferred.

#include <types.h>
#include <game/game/d_land.hpp>
#include <game/game/d_sv_activity.hpp>

namespace dSvTownList {

enum {
    TOWN_LIST_NUM = 8,
};

// Another town (by land) and its jinx.
struct Entry_c {
    Entry_c() {
        mJinx.mCondition = 0;
        mJinx.mOutcome = 0;
        mJinx.mValue = 0;
    }

    dLandID_c& getLand() { return mLand; }
    dSaveJinx_c& getJinx() { return mJinx; }

    /* 0x00 */ dLandID_c mLand;
    /* 0x16 */ dSaveJinx_c mJinx;
}; // size 0x18

struct List_c {
    List_c();                                     // 80150140
    void set(const dLandID_c *land, dSaveJinx_c jinx); // 801501F0: land's entry, else a free one, else (half the
                                                  // time) a random one
    Entry_c *get(int idx);                        // 80150308: NULL if its land is invalid
    const Entry_c *get(int idx) const;            // 80150350
    // The callers reach get through these (an inline layer: d_npc_talk_free msgFreeJinx's registers need it).
    Entry_c *getTown(int idx) { return get(idx); }
    const Entry_c *getTown(int idx) const { return get(idx); }
    void clear();                                 // 80150398

    /* 0x00 */ Entry_c mTowns[TOWN_LIST_NUM];
}; // size 0xC0

} // namespace dSvTownList
