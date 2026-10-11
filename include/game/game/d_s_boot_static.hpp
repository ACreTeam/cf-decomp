#pragma once

// The boot scene's static part (DOL TU d_s_boot_static.cpp, not decompiled). Only what other TUs use.

#include <types.h>

extern "C" {
extern bool lbl_8074E890; // set once the boot sequence is done (in an unsplit .sbss); d_reset BootCompleteCheck
BOOL fn_801086A4(); // 801086A4: FALSE when lbl_805EBDA0 +0x333 and +0x19C are both set (d_wifi: idle / reset)
}
