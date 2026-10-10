#pragma once

// Villager model data (TU d_npc_mdl_mng.cpp, not decompiled). Only what d_a_npc_nml uses.

#include <types.h>

u32 fn_800F9D8C();                                  // 800F9D8C: size of a villager's model data (0x7200)
BOOL fn_800F9D94(void *dst, u32 *size, u8 species); // 800F9D94: copies and binds the model of a species into dst
