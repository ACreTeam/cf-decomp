#pragma once

// The save done on a reset (DOL TU d_reset_save.cpp, not decompiled). Only what other TUs use.

#include <types.h>

extern "C" {
void fn_80107CD4(); // 80107CD4: clears its state (d_reset soft reset)
BOOL fn_80107CF4(); // 80107CF4: (d_reset ModeInit_SoftReset)
}
