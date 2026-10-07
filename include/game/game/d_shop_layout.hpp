#pragma once

// Shop item layouts: which item kind each unit of a shop room displays, and the mapping between a
// shop's display units and its stock slots in the town save. See notes/d_shop_layout.txt.

#include <types.h>
#include <game/game/d_field_info.hpp>

// One shop room's layout: an item kind (dItem::Kind) per unit. KIND_NONE (0x29) is no display;
// KIND_COUNT (0x57) marks a few special units.
typedef int dShopLayout_t[UT_Z_NUM][UT_X_NUM];

// A sale to apply to the town's shop stock (copied out of an 8-byte message by fn_800DFFB4).
struct dShopSale_c {
    /* 0x0 */ u32 mSlot : 8;   // stock slot, 0xFF = none
    /* 0x0 */ u32 mPrice : 23; // added to the shop's sales
    /* 0x0 */ u32 mFlag : 1;
    /* 0x4 */ u32 mUnitX : 8;
    /* 0x4 */ u32 mUnitZ : 8;
    /* 0x4 */ u32 mScene : 8;
    /* 0x4 */ u32 _4_0 : 8;
}; // size 0x8

const dShopLayout_t *dShopLayout_get(u8 scene);                // 8015F748
BOOL dShopLayout_findFreeUnit(int kind, int *unitX, int *unitZ); // 8015F870
int dShopLayout_getSlot(int unitX, int unitZ);                  // 8015F930: -1 when none
BOOL dShopLayout_isSlotSold(int unitX, int unitZ);              // 8015FBF8
BOOL fn_8015FD10();                                             // 8015FD10: always FALSE
BOOL fn_8015FD18();                                             // 8015FD18: always FALSE
void dShopLayout_applySale(dShopSale_c sale);                    // 8015FD20
