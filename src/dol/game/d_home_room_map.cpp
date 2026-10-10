// Per-room furniture state (active bits and gyroids) and the maps of the rooms that are not the
// current scene. .text 80110F30..80111A2C, .rodata 80475CE0..80475CF8, .bss 805EC5D0..805ECB58.
#include <game/game/d_home_room_map.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

// 80475CE0: house scene attribute per house (getSceneAttrTableIndex); same as d_home's table
static const u32 sHomeScenes[] = {
    SCENE_ATTR_HOUSE0 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE1 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE2 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE3 | SCENE_ATTR_PLAYER_HOUSE,
    0,
    0,
};

// 80110F30
void dHomeActiveFtr_c::clear() {
    for (int i = 0; i < 16; i++) {
        mRows[i] = 0xFFFF;
    }
}

// 80110F7C
BOOL dHomeActiveFtr_c::isSet(int x, int z) {
    return (mRows[z & 0xF] >> (x & 0xF)) & 1;
}

// 80110F94
void dHomeActiveFtr_c::set(int x, int z, BOOL on) {
    int bit = x & 0xF;
    int row = z & 0xF;
    if (on) {
        mRows[row] |= 1 << bit;
    } else {
        mRows[row] &= ~(1 << bit);
    }
}

// 80110FDC
void dHomeGyroids_c::clear() {
    for (int i = 0; i < 8; i++) {
        mPos[i].x = 0;
        mPos[i].z = 0;
    }
    mActive[0] = 0;
    for (int i = 0; i < 4; i++) {
        mValues[i] = 0;
    }
}

// 80111058
int dHomeGyroids_c::get(int x, int z) {
    for (u32 i = 0; i < 8; i++) {
        if (isActive(i) && x == mPos[i].x && z == mPos[i].z) {
            return getValue(i);
        }
    }
    return -1;
}

// 801111D8
BOOL dHomeGyroids_c::remove(int x, int z) {
    for (u32 i = 0; i < 8; i++) {
        u32 bit = i & 7;
        if (((mActive[i >> 3] >> bit) & 1) && x == mPos[i].x && z == mPos[i].z) {
            mPos[i].x = 0;
            mPos[i].z = 0;
            mActive[i >> 3] &= ~(1 << bit);
            mValues[i >> 1] &= ~(0xF << ((i & 1) * 4));
            return TRUE;
        }
    }
    return FALSE;
}

// 80111278
BOOL dHomeGyroids_c::add(int x, int z, int value) {
    if ((u32)value >= 16) {
        return FALSE;
    }
    if (get(x, z) == -1) {
        for (u32 i = 0; i < 8; i++) {
            u32 bit = i & 7;
            if (!((mActive[i >> 3] >> bit) & 1)) {
                mPos[i].x = x;
                mPos[i].z = z;
                // setValue spelled out: the inline here allocates registers differently
                u32 shift = (i & 1) * 4;
                mValues[i >> 1] = (u8)(mValues[i >> 1] & ~(0xF << shift)) | (value << shift);
                mActive[i >> 3] |= 1 << bit;
                return TRUE;
            }
        }
    } else {
        for (u32 i = 0; i < 8; i++) {
            if (isActive(i) && x == mPos[i].x && z == mPos[i].z) {
                setValue(i, value);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 801113F8
void dHomeRoomMap_c::clear() {
    for (u32 i = 0; i < HOME_LAYER_NUM; i++) {
        mActiveFtr[i].clear();
    }
    mGyroids.clear();
}

// 80111458
BOOL dHomeRoomMap_c::isSet(int x, int z, int layer) {
    return mActiveFtr[layer].isSet(x, z);
}

// 80111464
void dHomeRoomMap_c::set(int x, int z, int layer, BOOL on) {
    mActiveFtr[layer].set(x, z, on);
}

#define ANIMAL_HOME_MAP_NUM (SCENE_RM_NPC9 - SCENE_RM_NPC0 + 1) // 10
#define SHOP_MAP_NUM (SCENE_RM_SHOP3_F2 - SCENE_RM_SHOP0 + 1) // 5

// 805EC5D0..: the maps of the rooms without one in the save data
static dHomeRoomMap_c sAnimalHomeMaps[ANIMAL_HOME_MAP_NUM]; // SCENE_RM_NPC0..SCENE_RM_NPC9
static dHomeRoomMap_c sShopMaps[SHOP_MAP_NUM]; // SCENE_RM_SHOP0..SCENE_RM_SHOP3_F2
static dHomeRoomMap_c sTailorMap; // SCENE_RM_TAILOR
static dHomeRoomMap_c sGraceMap; // SCENE_RM_GRACE
static dHomeRoomMap_c sAuctionMap; // SCENE_RM_AUCTION

// 80111478
void clearAllRoomMaps() {
    for (u32 i = 0; i < ANIMAL_HOME_MAP_NUM; i++) {
        clearAnimalHomeRoomMap(i);
    }
    clearShopRoomMaps();
}

// 801114B8
void clearShopRoomMaps() {
    for (u32 i = 0; i < SHOP_MAP_NUM; i++) {
        sShopMaps[i].clear();
    }
    sTailorMap.clear();
    sGraceMap.clear();
    sAuctionMap.clear();
}

// 8011152C
BOOL clearRoomMap(u8 scene) {
    dHomeRoomMap_c *map = getRoomMap(scene);
    if (map != NULL) {
        map->clear();
        return TRUE;
    }
    return FALSE;
}

// 80111564
BOOL clearAnimalHomeRoomMap(int animal) {
    return clearRoomMap(animal + SCENE_RM_NPC0);
}

// 80111570
dHomeRoomMap_c *getRoomMap(u8 scene) {
    if (isSceneAttr(scene, SCENE_ATTR_PLAYER_HOUSE)) {
        int home;
        int room = getSceneAttrTableIndex(scene, sHomeScenes, &home);
        dHome_c *h = dSaveData_c::getTown()->mHomes.getHome(home);
        if (dHome_c::isValidRoom(room)) {
            dHomeRoom_c *r = h->getRoom(room);
            if (r != NULL) {
                return &r->mMap;
            }
        }
    } else if (isSceneAttr(scene, SCENE_ATTR_VILLAGER_HOUSE)) {
        return &sAnimalHomeMaps[getSceneAttrIndex(scene, SCENE_ATTR_VILLAGER_HOUSE)];
    } else if (isSceneAttr(scene, SCENE_ATTR_STORE_ROOM)) {
        return &sShopMaps[getSceneAttrIndex(scene, SCENE_ATTR_SHOP_ROOM)];
    } else if (scene == SCENE_RM_TAILOR) {
        return &sTailorMap;
    } else if (scene == SCENE_RM_GRACE) {
        return &sGraceMap;
    } else if (scene == SCENE_RM_AUCTION) {
        return &sAuctionMap;
    } else if (scene == SCENE_RM_HAPPY_MDL) {
        return &dSaveData_c::getTown()->mModelRoom.mMap;
    }
    return NULL;
}

// 801116C0
BOOL isRoomMapSet(int x, int z, int layer, u8 scene) {
    dHomeRoomMap_c *map = getRoomMap(scene);
    if (map != NULL) {
        return map->isSet(x, z, layer);
    }
    return FALSE;
}

// 80111728
void setRoomMap(int x, int z, int layer, BOOL on, u8 scene) {
    dHomeRoomMap_c *map = getRoomMap(scene);
    if (map != NULL) {
        map->set(x, z, layer, on);
    }
}

// 80111798
int getRoomGyroid(int x, int z, u8 scene) {
    dHomeRoomMap_c *map = getRoomMap(scene);
    if (map != NULL) {
        return map->mGyroids.get(x, z);
    }
    return -1;
}

// 801117F4
BOOL addRoomGyroid(int x, int z, int value, u8 scene) {
    dHomeRoomMap_c *map = getRoomMap(scene);
    if (map != NULL) {
        return map->mGyroids.add(x, z, value);
    }
    return FALSE;
}

// 80111860
BOOL removeRoomGyroid(int x, int z, u8 scene) {
    dHomeRoomMap_c *map = getRoomMap(scene);
    if (map != NULL) {
        return map->mGyroids.remove(x, z);
    }
    return FALSE;
}
