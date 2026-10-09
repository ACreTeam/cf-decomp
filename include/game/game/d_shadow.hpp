#pragma once

// Simple actor shadows (TU d_shadow.cpp, not decompiled).

#include <types.h>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_angle.hpp>

extern "C" {
// 801C8518: draws a round shadow (dAcNpc_c::draw: (pos, 0, &angle, _1B64, _1B68, mShadowSize, 0.0f, 1.0f)).
void fn_801C8518(const mVec3_c *pos, int arg, const mAng *angle, f32 f1, f32 f2, f32 size, f32 f4, f32 f5);
}
