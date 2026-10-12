#pragma once

// d_sys (DOL TU d_sys.cpp, .text 8016B894..8016C720, not decompiled): the system setup and the frame loop. Only
// what other TUs use.

#include <types.h>
#include <lib/egg/core/eggSystem.h>

extern EGG::TSystem<>::Configuration *lbl_8074E880; // 8074E880: the system configuration (80600750)

extern "C" {
void fn_8016B9FC(u8 value); // 8016B9FC: sets a byte (+8) of the object at lbl_8074E880 +0x4C, if any (d_sys boot, d_s_boot create)
}

extern "C" {
void fn_8016BA7C();          // 8016BA7C: the system setup (heaps, graphics, faders, ...) (d_main)
BOOL fn_8016B97C(u32 value); // 8016B97C: (d_main, after the setup: 0)
void fn_8016C3C0();          // 8016C3C0: one frame (d_main's loop)
}
