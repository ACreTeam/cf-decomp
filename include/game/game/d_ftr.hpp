#pragma once

// Furniture / indoor floor (TU d_ftr.cpp, not decompiled). Other .cpp files declare fn_800A9058 locally.

#include <types.h>
#include <game/mLib/m_vec.hpp>

namespace dItem {
struct Item;
}
struct dNpcFtrShape_c;

extern "C" {
int fn_800A8F98(void *mgr, int x, int z, int layer);       // 800A8F98: handle of the furniture at unit (x, z), -1 = none
void *fn_800A93BC();                                       // 800A93BC: the furniture work (80596C80)
BOOL fn_800A9354(void *work, int handle);                  // 800A9354: furniture handle still busy
void *fn_800A900C(void *mgr, const mVec3_c *pos, int arg); // 800A900C: the furniture at pos (fn_800A8FC4 by unit)
void *fn_800A9058();                                       // 800A9058: the furniture manager
f32 fn_800AA7F0(int x, int z);                             // 800AA7F0: ground height of a 32-unit cell (indoors)
// Furniture footprint (dNpcFtrShape_c, d_npc.hpp): fill from an item, tile count, offset {x, z} of tile i.
void fn_800A8B28(dNpcFtrShape_c *shape, dItem::Item item); // 800A8B28
u32 fn_800A8BB8(dNpcFtrShape_c *shape);                    // 800A8BB8
const int *fn_800A8BE4(dNpcFtrShape_c *shape, u32 i);      // 800A8BE4
}
