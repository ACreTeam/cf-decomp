#pragma once

// The boot scene's static part (DOL TU d_s_boot_static.cpp, not decompiled). Only what other TUs use.

#include <types.h>

extern "C" {
extern bool lbl_8074E890; // set once the boot sequence is done (in an unsplit .sbss); d_reset BootCompleteCheck
}
