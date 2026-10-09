#pragma once

// Ground attributes (TU d_ground_attr.cpp, not decompiled).

#include <types.h>
#include <game/mLib/m_vec.hpp>

extern "C" {
u32 fn_800A8648(const mVec3_c *pos, u8 *attr); // 800A8648: footstep sound id of the ground at pos
}
