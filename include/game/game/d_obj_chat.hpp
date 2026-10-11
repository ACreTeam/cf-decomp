#pragma once

// Chat object (TU d_obj_chat.cpp, not decompiled). Only what other TUs call is declared.

#include <types.h>

extern "C" {
void fn_801B12B0(const void *data, u32 size, int member); // 801B12B0: net receive (d_net packet 0x52)
void fn_801B13C8(const void *data, int member);           // 801B13C8: net receive (d_net packet 0x53)
}
