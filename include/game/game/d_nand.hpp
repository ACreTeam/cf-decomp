#pragma once

// NAND access (DOL TU d_nand.cpp, not decompiled). Only what other TUs use.

#include <types.h>

extern "C" {
u32 fn_800D2840(); // 800D2840: size of the NAND library heap (0xA000)
}
