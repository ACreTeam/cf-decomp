// Per-block (acre) data of a dFdBase_c (dFdBlock_c).
// .text 80080D7C..800811F4
#include <game/game/d_field_info.hpp>
#include <game/game/d_fg_item.hpp>
#include <lib/egg/core/eggHeap.h>
#include <string.h>

extern "C" {
void fn_800756F4(int blockX, int blockZ, void *data, int flag, int bg); // 800756F4
u32 fn_80081324(int type);                                             // 80081324: flags of a block type
BOOL fn_80081238(u16 *flags, int x, int z);                            // 80081238: set a unit's flag
BOOL fn_80081280(u16 *flags, int x, int z);                            // 80081280: clear a unit's flag
BOOL fn_80106464(u32 type);                                            // 80106464
}

static inline BOOL isUnitInBlock(int unitX, int unitZ) {
    return (u32)unitX < UT_X_NUM && (u32)unitZ < UT_Z_NUM;
}

// 80080D7C
dFdBlock_c::dFdBlock_c() {
    mType = 0xF1;
    memset(mItems, 0, sizeof(mItems));
    mBuried = NULL;
    mWatered = NULL;
    mBgData = NULL;
    mBlockX = 0;
    mBlockZ = 0;
    mFlag = 0;
}

// 80080DDC
u32 dFdBlock_c::getAllocSize(int num, int align) {
    int mask = align - 1;
    u32 size = num * sizeof(dFdBlock_c);
    return ~mask & (size + mask);
}

// 80080DF4
dFdBlock_c *dFdBlock_c::create(int num, EGG::Heap *heap, int align) {
    dFdBlock_c *blocks = (dFdBlock_c *)heap->alloc(num * sizeof(dFdBlock_c), align);
    if (blocks != NULL) {
        for (int i = 0; i < num; i++) {
            new (&blocks[i]) dFdBlock_c();
        }
    }
    return blocks;
}

// 80080E88
void dFdBlock_c::release(EGG::Heap *heap, BOOL items, BOOL data) {
    if (items) {
        for (int i = 0; i < 2; i++) {
            if (mItems[i] != NULL) {
                heap->free(mItems[i]);
                mItems[i] = NULL;
            }
        }
    }
    if (data && mBgData != NULL) {
        heap->free(mBgData);
        mBgData = NULL;
    }
}

// 80080F38
void dFdBlock_c::set(int type, dItem::Item *items0, dItem::Item *items1, u16 *buried, u16 *watered, void *bgData,
                     int blockX, int blockZ, int flag, int bg) {
    mType = type;
    mItems[0] = items0;
    mItems[1] = items1;
    mBuried = buried;
    mWatered = watered;
    mBgData = bgData;
    mBlockX = blockX;
    mBlockZ = blockZ;
    mFlag = flag;
    fn_800756F4(blockX, blockZ, bgData, flag, bg);
}

// 80080F80
BOOL dFdBlock_c::hasFlag(int mask) const {
    return (mask & fn_80081324(mType)) != 0;
}

// 80080FC0
BOOL dFdBlock_c::fn_80080FC0() const {
    return fn_80106464(mType);
}

// 80080FC8
BOOL dFdBlock_c::setItem(const dItem::Item *item, int unitX, int unitZ, int layer) {
    dItem::Item *dst = getItemP(unitX, unitZ, layer);
    if (dst != NULL) {
        *dst = *item;
        return TRUE;
    }
    return FALSE;
}

// 8008101C
dItem::Item *dFdBlock_c::getItemP(int unitX, int unitZ, int layer) {
    if (layer < 2 && mItems[layer] != NULL && isUnitInBlock(unitX, unitZ)) {
        return &mItems[layer][(unitZ << 4) + unitX];
    }
    return NULL;
}

// 80081074
dItem::Item *dFdBlock_c::getItemP(int unitX, int unitZ, int layer) const {
    if (layer < 2 && mItems[layer] != NULL && isUnitInBlock(unitX, unitZ)) {
        return &mItems[layer][(unitZ << 4) + unitX];
    }
    return NULL;
}

// 800810CC
dItem::Item *dFdBlock_c::getItem(int unitX, int unitZ, int layer) const {
    return getItemP(unitX, unitZ, layer);
}

// 800810D0
BOOL dFdBlock_c::setBuried(int unitX, int unitZ) {
    if (mBuried != NULL) {
        return fn_80081238(mBuried, unitX, unitZ);
    }
    return FALSE;
}

// 800810E8
BOOL dFdBlock_c::clearBuried(int unitX, int unitZ) {
    if (mBuried != NULL) {
        return fn_80081280(mBuried, unitX, unitZ);
    }
    return FALSE;
}

// 80081100
BOOL dFdBlock_c::isBuried(int unitX, int unitZ) const {
    if (mBuried != NULL) {
        if (isUnitInBlock(unitX, unitZ)) {
            u16 row = mBuried[unitZ];
            return (row & (1 << unitX)) != 0;
        }
        return FALSE;
    }
    return FALSE;
}

// 80081160
BOOL dFdBlock_c::setWatered(int unitX, int unitZ) {
    if (mWatered != NULL) {
        return fn_80081238(mWatered, unitX, unitZ);
    }
    return FALSE;
}

// 80081178
BOOL dFdBlock_c::isWatered(int unitX, int unitZ) const {
    if (mWatered != NULL) {
        if (isUnitInBlock(unitX, unitZ)) {
            u16 row = mWatered[unitZ];
            return (row & (1 << unitX)) != 0;
        }
        return FALSE;
    }
    return FALSE;
}

// 800811D8
void dFdBlock_c::clearWatered() {
    if (mWatered != NULL) {
        memset(mWatered, 0, UT_Z_NUM * sizeof(u16));
    }
}
