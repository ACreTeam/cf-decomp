#pragma once

// NAND access (DOL TU d_nand.cpp, not decompiled). Only what other TUs use.

#include <types.h>

class dSceneChange_c;

extern "C" {
u32 fn_800D2840(); // 800D2840: size of the NAND library heap (0xA000)
BOOL fn_800D2360(); // 800D2360: no NAND access is running (d_reset waits for it before a reset / fade-out)
void fn_800D23A0(); // 800D23A0: (d_reset soft reset)
int fn_800D2404();  // 800D2404: (d_net_mgr execute: nonzero skips copying the friend list)
int fn_800D2A44();  // 800D2A44: the NAND error state (2: fatal, d_reset FatalError)
dSceneChange_c *fn_800D3584(); // 800D3584: &gSceneChange (d_net)
}
