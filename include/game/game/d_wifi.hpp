#pragma once

// WiiConnect24 state (TU d_wifi.cpp, not decompiled). Names are inferred.

#include <types.h>

extern "C" {
BOOL fn_80177D24(); // 80177D24: WiiConnect24 is on (and the save allows it)
BOOL fn_80177D6C(); // 80177D6C: idle (d_reset waits for it before a reset / quitting)
void fn_80177DAC(); // 80177DAC: (d_reset soft reset)
}
