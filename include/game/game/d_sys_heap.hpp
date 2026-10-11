#pragma once

// d_sys_heap (DOL TU d_sys_heap.cpp, .text 800AC154..800AC260, not decompiled). Only what other TUs use.

#include <types.h>

struct dNetPlayerInfo_c;

extern "C" {
void fn_800AC154(dNetPlayerInfo_c *info); // 800AC154: memset 0x66
void fn_800AC160(dNetPlayerInfo_c *info); // 800AC160: the current player's ids and name (dPlayerMgr_c::getCurrentPlayer())
}
