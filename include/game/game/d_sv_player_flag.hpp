#pragma once

// d_sv_player_flag (DOL TU d_sv_player_flag.cpp, .text 801510A0..801511B4): a town state byte (1 or 2, rolled when
// the town is made) with per-villager player flags: the current player's u16 at dPrivateData_c +0x7FA6 has one bit
// per villager (dAnimalBlock_c index). Used by Rollan's talk (d_npc_talk_rollan). No RTTI: names inferred.

#include <types.h>

namespace dSvPlayerFlag {

struct Flag_c {
    void init();         // 801510A0: the state = 1 or 2 at random
    void clear(int idx); // 801510EC: clears villager idx's bit of the current player
    int get(int idx);    // 8015112C: the state when the current player is from this town, has flag1 0x10 and
                         // villager idx's bit set, else 0

    /* 0x0 */ u8 mState;
}; // size 0x1

} // namespace dSvPlayerFlag
