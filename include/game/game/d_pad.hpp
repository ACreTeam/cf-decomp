#pragma once

// Game pad helpers (TU d_pad.cpp, not decompiled). d_player_mgr.cpp still declares fn_800FA724 locally.

#include <types.h>
#include <game/mLib/m_vec.hpp>

extern "C" {
void fn_800FA6C4(mVec2_c *pos); // 800FA6C4: the pointer position (lbl_8074E5D0)
void fn_800FA6DC(mVec2_c *dir); // 800FA6DC: the pointer direction (lbl_8074E5C8)
f32 fn_800FA704(); // 800FA704: the stick y (lbl_8074E5B4)
u16 fn_800FA714(); // 800FA714: the held buttons (lbl_8074E5BC)
BOOL fn_800FA724(); // 800FA724
BOOL fn_800FA748(); // 800FA748: bit 4 of lbl_8074E5BE
BOOL fn_800FA754(); // 800FA754: bit 4 of lbl_8074E5BC
BOOL fn_800FA760(); // 800FA760: bit 5 of lbl_8074E5BE
BOOL fn_800FA76C(); // 800FA76C: bit 5 of lbl_8074E5BC
}
