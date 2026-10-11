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
void fn_800A93C8(const void *data, int member);            // 800A93C8: net receive (d_net packet 0x33, 3 bytes)
void fn_800A9484(BOOL flag);                               // 800A9484: net receive (d_net packets 0x34 / 0x35)
void fn_800A949C(int, int);                                // 800A949C
void fn_800A95FC(const void *data);                        // 800A95FC: net receive (d_net packet 0x3C, 2 bytes)
void fn_800A9DD8(const void *data);                        // 800A9DD8: net receive (d_net packet 0x37, 4 bytes)
void fn_800AA028(const void *data);                        // 800AA028: net receive (d_net packet 0x38, 2 bytes)
void fn_800AA1D4(u8 scene, const void *data);              // 800AA1D4: net receive (d_net packet 0x3B, 4 bytes)
void fn_800AA3F8(u8 scene, const void *data);              // 800AA3F8: net receive (d_net packet 0x39, 4 bytes)
void fn_800AA68C(u8 scene, const void *data);              // 800AA68C: net receive (d_net packet 0x3A, 4 bytes)
void fn_800ABA90(int handle);                              // 800ABA90
void fn_800AB3CC();                                        // 800AB3CC: (d_reset soft reset)
f32 fn_800AA7F0(int x, int z);                             // 800AA7F0: ground height of a 32-unit cell (indoors)
// Furniture footprint (dNpcFtrShape_c, d_npc.hpp): fill from an item, tile count, offset {x, z} of tile i.
void fn_800A8B28(dNpcFtrShape_c *shape, dItem::Item item); // 800A8B28
u32 fn_800A8BB8(dNpcFtrShape_c *shape);                    // 800A8BB8
const int *fn_800A8BE4(dNpcFtrShape_c *shape, u32 i);      // 800A8BE4
}
