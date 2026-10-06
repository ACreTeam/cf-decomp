#pragma once

#include <types.h>
#include <game/game/d_home.hpp>

// Furniture state (active bits and gyroids) of the rooms that are not the current scene, by
// scene. Defined in src/dol/game/d_home_room_map.cpp (.text 80110F30..80111A2C); the classes
// (dHomeActiveFtr_c, dHomeGyroids_c, dHomeRoomMap_c) are in d_home.hpp.
//   - player house rooms (SCENE_RM_HS0_F1..SCENE_RM_HS3_LOFT): the room's own map (dHomeRoom_c::mMap)
//   - villager houses (SCENE_RM_NPC0..9, SCENE_ATTR_VILLAGER_HOUSE): 10 maps kept in .bss
//   - Nook's store (SCENE_RM_SHOP0..SCENE_RM_SHOP3_F2, SCENE_ATTR_STORE_ROOM, one per stage/floor):
//     5 maps in .bss
//   - the tailor, Gracie's and the auction house (SCENE_RM_TAILOR, SCENE_RM_GRACE, SCENE_RM_AUCTION):
//     one map each in .bss
//   - the model room (SCENE_RM_HAPPY_MDL): dSaveData_c::_0636F0's map
// The .bss maps other than the villager houses are cleared every day (clearShopRoomMaps).

void clearAllRoomMaps(); // 80111478
void clearShopRoomMaps(); // 801114B8: from the daily update
BOOL clearRoomMap(u8 scene); // 8011152C
BOOL clearAnimalHomeRoomMap(int animal); // 80111564
dHomeRoomMap_c *getRoomMap(u8 scene); // 80111570: NULL for scenes without one
BOOL isRoomMapSet(int x, int z, int layer, u8 scene); // 801116C0
void setRoomMap(int x, int z, int layer, BOOL on, u8 scene); // 80111728
int getRoomGyroid(int x, int z, u8 scene); // 80111798: -1 if none
BOOL addRoomGyroid(int x, int z, int value, u8 scene); // 801117F4
BOOL removeRoomGyroid(int x, int z, u8 scene); // 80111860
