#pragma once

// Saved positions (d_sv_npc_pos.cpp, .text 80150524..80150778). Not decompiled yet; only what other
// TUs call is declared, with the target's C names.

#include <types.h>
#include <game/mLib/m_vec.hpp>

extern "C" {
// 801506F8: the world position of a packed save position (d_npc reads dSaveTown_c +0x66745, the
// Harvest Festival spot that d_npc_talk_harvest remarks on).
mVec3_c fn_801506F8(const void *data);
// Packed save positions: dSaveTown_c +0x6673C (fn_80150544), +0x66741 (fn_801505FC), +0x66745 (fn_80150648).
void fn_80150544(void *data, u8 kind, const mVec3_c *pos); // 80150544
void fn_801505FC(void *data, u8 x, u8 z);                  // 801505FC
void fn_80150648(void *data, const mVec3_c *pos);          // 80150648
}
