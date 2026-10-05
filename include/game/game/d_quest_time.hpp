#pragma once

// 64-bit quest time and its helpers. The helpers live in the unsplit TU at 8014BD88..80153818
// along with other date code; they keep C linkage until that TU is split.

#include <types.h>
#include <game/game/d_date.hpp>

// An OSTime stored as bytes so it can sit at 2-byte-aligned offsets.
// INT64_MAX means "no limit".
struct dQuestTime_c;

extern "C" {
void fn_8014C468(dQuestTime_c *time);                              // 8014C468: reset to INT64_MAX
void fn_8014C5D0(dQuestTime_c *time, s64 value);                   // 8014C5D0: memcpy the value in
void fn_8014C538(dQuestTime_c *time, const dTime_c *cal);          // 8014C538: from a calendar time
s64 fn_8014C380(dQuestTime_c *time);                               // 8014C380: the raw value
BOOL fn_8014C384(dQuestTime_c *time);                              // 8014C384: value == INT64_MAX
void fn_8014C61C(dQuestTime_c *time);                              // 8014C61C: set to now
void fn_8014C6A4(dQuestTime_c *time, int days);                    // 8014C6A4: add days (?)
void fn_8014C818(dQuestTime_c *time);                              // 8014C818: back to the start of the day (6 AM)
int fn_8014C8E4(dQuestTime_c *time);                               // 8014C8E4: weekday
BOOL fn_8014C9C4(dQuestTime_c *time, s64 other, int, int);         // 8014C9C4
// fn_8014C2C4 (dTime_c from a dQuestTime_c) is declared by its users: d_sv_runtime.hpp
// still has an older void-pointer signature for it.
}

struct dQuestTime_c {
    dQuestTime_c(s64 value = 0x7FFFFFFFFFFFFFFFLL) { fn_8014C5D0(this, value); }
    dQuestTime_c(const dTime_c *cal) { fn_8014C538(this, cal); }

    u8 mData[8];
};
