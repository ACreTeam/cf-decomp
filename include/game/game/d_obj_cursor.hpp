#pragma once

// The pointer cursor (DOL TU d_obj_cursor.cpp, .text 801B3380..801B3E54, not decompiled). Only what other TUs use.

#include <types.h>
#include <game/mLib/m_vec.hpp>

extern "C" {
void fn_801B3D58(mVec2_c *pos); // 801B3D58: the pointer position (lbl_8074EB10)
BOOL fn_801B3D70(); // 801B3D70: 8074EB18 > 80751910 (d_money hides the money while it holds)
}
