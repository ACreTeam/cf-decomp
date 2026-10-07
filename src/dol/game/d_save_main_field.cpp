// The town field in the save data (dSaveMainField_c). .text 80110634..80110F30.
#include <game/game/d_save_main_field.hpp>
#include <game/game/d_fg_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/cLib/c_math.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
BOOL fn_80106250(dSaveMainField_c *field, int arg);
BOOL fn_8010640C();
void *fn_800A835C();
BOOL fn_800A7B18(void *obj, int arg, int arg2);
BOOL fn_800A7B9C(void *obj);
int fn_80081514(int type); // the bridge variant of a block type (or the type itself)
void fn_8014F0A4(void *obj);
}

// 80110634
dSaveMainField_c::dSaveMainField_c() {
    mCreateStep = MAIN_FIELD_CREATE_CLEAR;
    mSeason = 0;
}

// 801106F4
void dSaveMainField_c::clear() {
    dSaveItemLayer_c *items = mItemLayer0[0];
    dFdUnitFlags_c *buried = mBuried[0];
    memset(this, 0, sizeof(dSaveMainField_c));
    for (int i = 0; i < FG_BLOCK_TOTAL_NUM; i++) {
        items->clear();
        buried->clear();
        items++;
        buried++;
    }
}

// 8011076C
BOOL dSaveMainField_c::createBlocks() {
    return TRUE;
}

// 80110774
BOOL dSaveMainField_c::createField(int arg) {
    if (mCreateStep == MAIN_FIELD_CREATE_CLEAR) {
        clear();
        mCreateStep = MAIN_FIELD_CREATE_BLOCKS;
    }
    if (mCreateStep == MAIN_FIELD_CREATE_BLOCKS) {
        mGrassType = cM::rndInt(3);
        if (!fn_80106250(this, 0)) {
            return FALSE;
        }
        if (!fn_8010640C()) {
            return FALSE;
        }
        if (!createBlocks()) {
            return FALSE;
        }
        mCreateStep = MAIN_FIELD_CREATE_ITEMS;
    }
    if (mCreateStep == MAIN_FIELD_CREATE_ITEMS) {
        dFdBlockId_c *blockId;
        int x, z;
        blockId = mFieldBlockData[0];
        for (z = 0; z < BLOCK_Z_NUM; z++) {
            for (x = 0; x < BLOCK_X_NUM; x++) {
                dItem::Item *items = getBlockItems(x, z);
                if (items != NULL) {
                    dFgData_getLayout0((u16 *)items, blockId->mId);
                }
                dFdUnitFlags_c *buried = getBlockBuried(x, z);
                if (buried != NULL) {
                    memset(buried, 0, sizeof(dFdUnitFlags_c));
                }
                dSaveGrassWear_c *grass = getBlockGrassWear(x, z);
                if (grass != NULL) {
                    memset(grass, 0, sizeof(dSaveGrassWear_c));
                }
                blockId++;
            }
        }
        dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
        if (fd != NULL) {
            ((dFdInfo_c *)fd)->updateTown();
        }
        dSaveData_c::getTown()->mBuilding.mList.create();
        mCreateStep = MAIN_FIELD_CREATE_STEP3;
    }
    if (mCreateStep == MAIN_FIELD_CREATE_STEP3) {
        if (!fn_800A7B18(fn_800A835C(), arg, 1)) {
            return FALSE;
        }
        mCreateStep = MAIN_FIELD_CREATE_STEP4;
    }
    if (mCreateStep == MAIN_FIELD_CREATE_STEP4) {
        if (!fn_800A7B9C(fn_800A835C())) {
            return FALSE;
        }
        mCreateStep = MAIN_FIELD_CREATE_FIELD_DONE;
    }
    return TRUE;
}

// 80110958
BOOL dSaveMainField_c::createFinish() {
    if (mCreateStep == MAIN_FIELD_CREATE_FIELD_DONE) {
        dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
        if (fd != NULL) {
            ((dFdInfo_c *)fd)->updateTown();
        }
        mCreateStep = MAIN_FIELD_CREATE_STEP6;
    }
    if (mCreateStep == MAIN_FIELD_CREATE_STEP6) {
        mCreateStep = MAIN_FIELD_CREATE_STEP7;
    }
    if (mCreateStep == MAIN_FIELD_CREATE_STEP7) {
        mCreateStep = MAIN_FIELD_CREATE_DONE;
    }
    return TRUE;
}

// 801109D4
void dSaveMainField_c::resetCreate() {
    if (mCreateStep == MAIN_FIELD_CREATE_DONE) {
        mCreateStep = MAIN_FIELD_CREATE_FIELD_DONE;
    }
}

// 801109EC
BOOL dSaveMainField_c::setBridgeBlock(int blockX, int blockZ) {
    bool done = false;
    if (mBridgeBlockX == -1 && mBridgeBlockZ == -1) {
        done = true;
    }
    if (!done && (mBridgeBlockX == 0 || mBridgeBlockZ == 0)) {
        if (isFieldBlock(blockX, blockZ)) {
            mBridgeBlockX = blockX;
            mBridgeBlockZ = blockZ;
            return TRUE;
        }
    }
    return FALSE;
}

// 80110A7C
BOOL dSaveMainField_c::buildBridge() {
    char z; // char (not s8 / int) for the match
    char x;
    x = mBridgeBlockX;
    if (x != 0) {
        z = mBridgeBlockZ;
        if (z != 0 && x != -1 && z != -1 && isFieldBlock(x, z)) {
            dFdBlockId_c *blockId = &mFieldBlockData[z][x];
            int type = blockId->mId;
            int newType = fn_80081514(type);
            blockId->mId = newType;
            if (newType != type) {
                dSaveTown_c *town = dSaveData_c::getTown();
                *(u32 *)&town->_05EC78[4] = 0;
                dSaveData_c::getTown()->setNewConstruction(NEW_CONSTRUCTION_BRIDGE, mBridgeBlockX, mBridgeBlockZ);
                mBridgeBlockX = mBridgeBlockZ = -1;
                fn_8014F0A4(&dSaveData_c::getTown()->_064078[0x50]);
                return TRUE;
            }
            return FALSE;
        }
    }
    return FALSE;
}

// 80110BA0
BOOL dSaveMainField_c::getSaveBlock(int *x, int *z, int blockX, int blockZ) {
    if (isFieldBlock(blockX, blockZ)) {
        *x = blockX - 1;
        *z = blockZ - 1;
        return TRUE;
    }
    return FALSE;
}

// 80110BF0
dItem::Item *dSaveMainField_c::getBlockItems(int blockX, int blockZ) {
    int x = 0;
    int z = 0;
    if (!getSaveBlock(&x, &z, blockX, blockZ)) {
        return NULL;
    }
    return mItemLayer0[z][x].getItems();
}

// 80110C70
dFdUnitFlags_c *dSaveMainField_c::getBlockBuried(int blockX, int blockZ) {
    int x = 0;
    int z = 0;
    if (!getSaveBlock(&x, &z, blockX, blockZ)) {
        return NULL;
    }
    return &mBuried[z][x];
}

// 80110CE4
dFdUnitFlags_c *dSaveMainField_c::getBlockWater(int blockX, int blockZ) {
    int x = 0;
    int z = 0;
    if (!getSaveBlock(&x, &z, blockX, blockZ)) {
        return NULL;
    }
    return &mWater[z][x];
}

// 80110D58
dSaveGrassWear_c *dSaveMainField_c::getBlockGrassWear(int blockX, int blockZ) {
    int x = 0;
    int z = 0;
    if (!getSaveBlock(&x, &z, blockX, blockZ)) {
        return NULL;
    }
    return &mGrassWear[z][x];
}

// 80110DCC
u8 *dSaveMainField_c::getBlockGrassWearUnits(int blockX, int blockZ) {
    dSaveGrassWear_c *grass = getBlockGrassWear(blockX, blockZ);
    return grass != NULL ? grass->mUnits : NULL;
}

// 80110DFC
dSaveItemLayer_c::dSaveItemLayer_c() {}

// 80110E44
dSaveItemLayer_c::~dSaveItemLayer_c() {}

// 80110EA8
void dSaveItemLayer_c::clear() {
    dItem::Item *item = getItems();
    dItem::Item *end = item + UT_TOTAL_NUM;
    for (; item != end; item++) {
        item->mId = dItem::ITEM_ID_NONE;
    }
}

// 80110EE8
dItem::Item *dSaveItemLayer_c::getItems() {
    return mItems;
}

// 80110EEC
dSaveGrassWear_c::dSaveGrassWear_c() {}

// 80110EF0
dSaveGrassWear_c::~dSaveGrassWear_c() {}
