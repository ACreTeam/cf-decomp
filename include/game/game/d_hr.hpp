#pragma once

// The house rating (d_hr.cpp, .text 800AC260..800B4DA0). Not decompiled yet; only what other TUs
// call is declared. The functions keep the target's C names until the TU is split.

#include <types.h>

extern "C" {
// 800AC28C: the rating record (4 bytes) of player house home (< 4); home 4 = the current player's
// house (a zeroed static record when the player has none). Bytes 1 and 2 are read by d_npc_talk_rollan.
const u8 *fn_800AC28C(int home);
// 800B2480: rating of player house home (dHomeList_c index) into *rank (through fn_800AD1FC); returns
// layout flags (d_npc_talk_quest_q08 picks its "Q08_Layout" remark and present from them).
int fn_800B2480(int home, u32 *rank);
// 800B24BC: rating of villager animalIdx's house into *rank (through fn_800AD118); d_npc_talk_quest_q09
// clamps it to 1..5 to pick the present.
int fn_800B24BC(int animalIdx, u32 *rank);
}
