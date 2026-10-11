#pragma once

// d_demo_sync.cpp (not decompiled). Only what other TUs use.

#include <types.h>

extern "C" {
void fn_8008205C(); // 8008205C: resets the four sync slots (805839D8 / 805839E8) and sets dPostOffice sync wait
void fn_8008239C(int member, const void *data); // 8008239C: net receive (d_net packet 0x2C)
}
