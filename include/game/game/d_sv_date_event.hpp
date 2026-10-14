#pragma once

// d_sv_date_event (DOL TU d_sv_date_event.cpp, .text 80150778..80150A5C, .sdata2 80750CE0..80750CF8): a dated
// town record with two scales; its only user is the snowman (d_a_snowman), which keeps SNOWMAN_NUM of them at
// dSaveTown_c::mSnowmen (+0x683E0): the day it was built and the sizes of its two snowballs. Dates are game days
// (they start at 6:00). No RTTI: names inferred (the TU name is the split's).

#include <types.h>
#include <game/game/d_date.hpp>

namespace dSvDateEvent {

enum {
    SNOWMAN_NUM = 3,
};

// The range of the scales (Event_c::set clamps to it).
extern const f32 cScaleMin;
extern const f32 cScaleMax;

struct Event_c {
    void clear();                            // 80150778
    int diffDays(const dTime_c *time) const; // 80150784: time's game day - mDate
    int diffDaysAdjusted(const dTime_c *time) const; // 8015082C: the same with dYMD_c::diffDays's adjust
    void set(f32 scale1, f32 scale2);        // 801508D4: today (game day), both scales, valid
    void setScale1(f32 scale);               // 801509AC
    void setScale2(f32 scale);               // 80150A04

    /* 0x0 */ dYMD_c mDate;
    /* 0x4 */ u8 mScale1; // (scale - 0.5) clamped to 0..0.75, as 0..255
    /* 0x5 */ u8 mScale2;
    /* 0x6 */ u8 mValid;
}; // size 0x8

} // namespace dSvDateEvent
