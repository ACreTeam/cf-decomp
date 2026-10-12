#pragma once

// d_sv_daily_rand (DOL TU d_sv_daily_rand.cpp, .text 80153818..801538F8): a random value 0..3 redrawn each day and
// shared with net visitors (dSaveTown_c::mDailyRand; d_npc_talk_carnival reads it). No RTTI: names inferred.

#include <types.h>

namespace dSvDailyRand {

#define DAILY_RAND_NUM 4 // values 0..3; DAILY_RAND_NUM: none yet

struct Rand_c {
    Rand_c() {}
    Rand_c(u8 value) : mValue(value) {}

    void init();   // 80153818: a random value (d_sv_proc)
    void reset();  // 8015384C: none, then change (d_field_assessment)
    void change(); // 80153858: a different random value, sent to the net visitors (NET_PACKET_DAILY_RAND)

    /* 0x0 */ u8 mValue;
}; // size 0x1

} // namespace dSvDailyRand
