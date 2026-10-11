#pragma once

// DOL TU d_net_wus.cpp (not decompiled; only what other TUs use).

#include <types.h>

// A session with the NWC24 friend list service (d_wifi's daily roster updates). Names are inferred.
class dNetWus_c {
public:
    dNetWus_c();  // 800EB158
    ~dNetWus_c(); // 800EB170
    BOOL open();  // 800EB1B0
    void close(); // 800EB2D4
    BOOL fn_800EB584(u64 userId, BOOL arg);                         // 800EB584
    // 800EB700: results[i] for keys[0, num).
    BOOL fn_800EB700(const u64 *keys, int num, int arg, u8 *results);

    /* 0x0 */ u32 _0;
    /* 0x4 */ u32 _4;
    /* 0x8 */ u32 _8; // 200
}; // size 0xC
