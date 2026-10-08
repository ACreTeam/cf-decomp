#pragma once

#include <types.h>
#include <cstring>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_field_info.hpp>

// The town field in the save data (NH ::Game::SaveMainField): the 7 x 7 block (acre) types and,
// for the 5 x 5 usable field blocks, their items, buried spots, watered units and grass wear. In the
// save data at dSaveData_c::mMainField. Source: src/dol/game/d_save_main_field.cpp (.text 80110634..80110F30).
// Names are inferred.

// The steps of building a new town (dSaveMainField_c::mCreateStep).
enum {
    MAIN_FIELD_CREATE_CLEAR,      // clear the field
    MAIN_FIELD_CREATE_BLOCKS,     // pick the grass type and lay out the blocks
    MAIN_FIELD_CREATE_ITEMS,      // the blocks' initial items
    MAIN_FIELD_CREATE_STEP3,
    MAIN_FIELD_CREATE_STEP4,
    MAIN_FIELD_CREATE_FIELD_DONE, // 5: done with createField
    MAIN_FIELD_CREATE_STEP6,
    MAIN_FIELD_CREATE_STEP7,
    MAIN_FIELD_CREATE_DONE,       // 8: done with createFinish
};

// One bit per unit of a block, a u16 row per unit z (dFdBlock_c::mBuried / mWatered). Its ctor and
// dtor are with dFdBlock_c's functions.
class dFdUnitFlags_c { // 0x20
public:
    dFdUnitFlags_c(); // 800811F4
    ~dFdUnitFlags_c(); // 800811F8

    void clear() { memset(this, 0, sizeof(dFdUnitFlags_c)); }

    /* 0x00 */ u16 mRows[UT_Z_NUM];
};

// The items of one block (NH "ItemLayer0").
class dSaveItemLayer_c { // 0x200
public:
    dSaveItemLayer_c(); // 80110DFC
    ~dSaveItemLayer_c(); // 80110E44
    void clear(); // 80110EA8: every unit to ITEM_ID_NONE
    dItem::Item *getItems(); // 80110EE8

    /* 0x000 */ dItem::Item mItems[UT_TOTAL_NUM];
};

// Grass wear of one block, a byte per unit.
class dSaveGrassWear_c { // 0x100
public:
    dSaveGrassWear_c(); // 80110EEC
    ~dSaveGrassWear_c(); // 80110EF0

    /* 0x000 */ u8 mUnits[UT_TOTAL_NUM];
};

class dSaveMainField_c { // 0x51A8
public:
    dSaveMainField_c(); // 80110634
    void clear(); // 801106F4
    BOOL createBlocks(); // 8011076C: nothing to do
    BOOL createField(EGG::Heap *heap); // 80110774: steps mCreateStep up to MAIN_FIELD_CREATE_FIELD_DONE
    BOOL createFinish(); // 80110958: up to MAIN_FIELD_CREATE_DONE
    void resetCreate(); // 801109D4: back to MAIN_FIELD_CREATE_FIELD_DONE
    BOOL setBridgeBlock(int blockX, int blockZ); // 801109EC: where the extra bridge goes, unless already set or built
    BOOL buildBridge(); // 80110A7C: the block at mBridgeBlockX/Z to its bridge variant (BLOCK_KIND_FLAG_BRIDGE)
    static BOOL getSaveBlock(int *x, int *z, int blockX, int blockZ); // 80110BA0: field block -> 0..4
    dItem::Item *getBlockItems(int blockX, int blockZ); // 80110BF0
    dFdUnitFlags_c *getBlockBuried(int blockX, int blockZ); // 80110C70
    dFdUnitFlags_c *getBlockWater(int blockX, int blockZ); // 80110CE4: watered flowers / red turnips
    dSaveGrassWear_c *getBlockGrassWear(int blockX, int blockZ); // 80110D58
    u8 *getBlockGrassWearUnits(int blockX, int blockZ); // 80110DCC

    void setSeason(u8 season) { mSeason = season; }

    // A usable field block (not a border block).
    static bool isFieldBlock(int blockX, int blockZ) {
        bool ok = false;
        if (blockX > 0 && blockX < BLOCK_X_NUM - 1 && blockZ > 0 && blockZ < BLOCK_Z_NUM - 1) {
            ok = true;
        }
        return ok;
    }

    /* 0x0000 */ dFdBlockId_c mFieldBlockData[BLOCK_Z_NUM][BLOCK_X_NUM];
    /* 0x0062 */ dSaveItemLayer_c mItemLayer0[FG_BLOCK_Z_NUM][FG_BLOCK_X_NUM];
    /* 0x3262 */ dFdUnitFlags_c mBuried[FG_BLOCK_Z_NUM][FG_BLOCK_X_NUM];
    /* 0x3582 */ dFdUnitFlags_c mWater[FG_BLOCK_Z_NUM][FG_BLOCK_X_NUM];
    /* 0x38A2 */ dSaveGrassWear_c mGrassWear[FG_BLOCK_Z_NUM][FG_BLOCK_X_NUM];
    /* 0x51A2 */ u8 mCreateStep; // MAIN_FIELD_CREATE_*
    /* 0x51A3 */ u8 mGrassType; // 0..2
    /* 0x51A4 */ u8 mSeason;
    // volatile: setBridgeBlock reloads them on every read.
    /* 0x51A5 */ volatile s8 mBridgeBlockX; // the extra bridge (public works); 0: none, -1: built
    /* 0x51A6 */ volatile s8 mBridgeBlockZ;
}; // size 0x51A8
