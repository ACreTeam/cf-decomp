#pragma once

// d_sv_daily (DOL TU d_sv_daily.cpp, .text 801503E4..80150524, .sbss 8074E7E0): a dated town value that lapses at
// the next 6:00 day start (dSaveTown_c::mDaily). d_sv_proc requests a refresh and updates it. No RTTI: names
// inferred.

#include <types.h>
#include <game/game/d_date.hpp>

namespace dSvDaily {

enum {
    DAILY_NONE = 0x20, // mValue when unset or lapsed
    DAILY_INITIAL = 4, // mValue set on a requested refresh
};

struct Daily_c {
    void clear();                            // 801503E4: mValue = DAILY_NONE
    static void request();                   // 801503F0: the next update() sets DAILY_INITIAL for today
    void update();                           // 801503FC: also resets d_weather's timestamp (fn_801C95CC)
    void set(const dTime_c *time, u8 value); // 80150478: the date of time - 6h

    /* 0x0 */ dYMD_c mDate;
    /* 0x4 */ u8 mValue;
}; // size 0x6

extern u8 sRequest; // 8074E7E0

} // namespace dSvDaily
