#pragma once

// Per-villager player flags in the town save (dSaveTown_c::_072CC0; d_sv_player_flag.cpp,
// .text 801510A0..801511B4). Not decompiled yet; the functions keep the target's C names.
// The current player's u16 at dPrivateData_c +0x7FA6 has one bit per villager (dAnimalBlock_c index).

#include <types.h>

extern "C" {
void fn_801510A0(void *obj);           // 801510A0: the state byte = 1 or 2 at random
void fn_801510EC(void *obj, int idx);  // 801510EC: clears villager idx's bit of the current player
int fn_8015112C(void *obj, int idx);   // 8015112C: the state byte when the current player is from this
                                       // town, has flag1 0x10 and villager idx's bit set, else 0
}
