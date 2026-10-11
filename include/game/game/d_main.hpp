#pragma once

// The main loop (DOL TU d_main.cpp, not decompiled). Only what other TUs use; names are inferred.

#include <types.h>

extern "C" {
// A mask of reasons the game is stopped (8074E4D8); the first one set also sets the low bits of 8074F030.
void fn_800CDD44(u32 mask); // 800CDD44: adds mask
BOOL fn_800CDD2C(u32 mask); // 800CDD2C: any bit of mask is set
void fn_800CDD68(u32 mask); // 800CDD68: removes mask
}
