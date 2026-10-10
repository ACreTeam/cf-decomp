#pragma once

// Villager model data (TU d_npc_mdl_mng.cpp, not decompiled). Only what d_a_npc_nml / d_a_npc_sp use.

#include <types.h>

extern "C" u32 fn_800F9D5C();                       // 800F9D5C: size of the npc model heap
u32 fn_800F9D8C();                                  // 800F9D8C: size of a villager's model data (0x7200)
BOOL fn_800F9D94(void *dst, u32 *size, u8 species); // 800F9D94: copies and binds the model of a species into dst
u32 fn_800F9E40();                                  // 800F9E40: size of a special npc's model file (0xD780)
