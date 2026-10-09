#pragma once

// Furniture / indoor floor (TU d_ftr.cpp, not decompiled). Other .cpp files declare fn_800A9058 locally.

#include <types.h>
#include <game/mLib/m_vec.hpp>

extern "C" {
void *fn_800A900C(void *mgr, const mVec3_c *pos, int arg); // 800A900C: the furniture at pos (fn_800A8FC4 by unit)
void *fn_800A9058();                                       // 800A9058: the furniture manager
f32 fn_800AA7F0(int x, int z);                             // 800AA7F0: ground height of a 32-unit cell (indoors)
}
