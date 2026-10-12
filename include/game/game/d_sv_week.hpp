#pragma once

// d_sv_week (DOL TU d_sv_week.cpp, .text 80154080..80154358): the game week and the clock offset seen when the
// town last updated it (dSaveTown_c::mWeek, +0x735B7, unaligned: the fields are bytes copied with memcpy).
// update() runs on each day change (d_field_assessment) and flags a clock change within the same week
// (mTimeChanged, read by d_npc_mng fn_801AD3F0). No RTTI: names inferred.

#include <types.h>

namespace dSvWeek {

enum {
    WEEK_NONE = -1, // mWeek after init
};

struct Week_c {
    void init();                  // 80154080: no week, offset 0
    s64 getOffset() const;        // 801540CC
    s16 getWeek() const;          // 80154100
    void setOffset(s64 offset);   // 80154134
    void setWeek(int week);       // 80154180
    void update();                // 801541D8: weeks since the Monday before 2000-01-01, dTime_c::sOffset

    /* 0x0 */ u8 mOffset[8];      // s64: dSaveTimeOffset_c::mOffset when last updated
    /* 0x8 */ u8 mWeek[2];        // s16: WEEK_NONE or the week number
    /* 0xA */ u8 mTimeChanged;    // the clock offset changed during this week
}; // size 0xB

} // namespace dSvWeek
