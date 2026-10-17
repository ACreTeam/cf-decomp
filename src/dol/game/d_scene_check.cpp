// The open player house rooms and the scene -> house / room lookups
// (dSceneCheck). .text 80081618..80081868, .rodata 8046D5A0..8046D5C0, .bss
// 80583648..80583658. See include/game/game/d_scene_check.hpp.
#include <game/game/d_scene_check.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_scene.hpp>

namespace dSceneCheck {

u8 sRoomOpen[HOUSE_NUM][ROOM_NUM];

// 80081618
u32 isRoomOpen(int house, int room) {
    return sRoomOpen[house][room];
}

// 80081634
void setRoomOpen(int house, int room) {
    sRoomOpen[house][room] = TRUE;
}

// 80081654
void clearRoomOpen(int house, int room) {
    sRoomOpen[house][room] = FALSE;
}

// 80081674
u32 isHouseRoomScene(u8 scene) {
    if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_HOUSE0 |
                                       dScene::SCENE_ATTR_PLAYER_HOUSE) ||
        dScene::isSceneAttr(scene, dScene::SCENE_ATTR_HOUSE1 |
                                       dScene::SCENE_ATTR_PLAYER_HOUSE) ||
        dScene::isSceneAttr(scene, dScene::SCENE_ATTR_HOUSE2 |
                                       dScene::SCENE_ATTR_PLAYER_HOUSE) ||
        dScene::isSceneAttr(scene, dScene::SCENE_ATTR_HOUSE3 |
                                       dScene::SCENE_ATTR_PLAYER_HOUSE)) {
        return TRUE;
    }
    return FALSE;
}

// 80081704
int getSceneHouse(u8 scene) {
    const u32 attrs[HOUSE_NUM] = {
        dScene::SCENE_ATTR_HOUSE0 | dScene::SCENE_ATTR_PLAYER_HOUSE,
        dScene::SCENE_ATTR_HOUSE1 | dScene::SCENE_ATTR_PLAYER_HOUSE,
        dScene::SCENE_ATTR_HOUSE2 | dScene::SCENE_ATTR_PLAYER_HOUSE,
        dScene::SCENE_ATTR_HOUSE3 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    };
    int house = -1;
    for (int i = 0; i < HOUSE_NUM; i++) {
        if (dScene::isSceneAttr(scene, attrs[i])) {
            house = i;
        }
    }
    return house;
}

// 800817A0
int getSceneRoom(u8 scene) {
    const int rooms[ROOM_NUM] = {0, 1, 2};
    int result = -1;
    int room;
    if (dHomeList_c::getHomeFromScene(&room, scene) != -1) {
        for (int i = 0; i < ROOM_NUM; i++) {
            if (room == rooms[i]) {
                result = i;
            }
        }
    }
    return result;
}

// 80081830
BOOL fn_80081830() {
    return FALSE;
}

// 80081838
BOOL drawCylinder(const mVec3_c &pos, const mColor &color, int arg2,
                  const mAng3_c &angle, int arg4, f32 radius, f32 height) {
    return FALSE;
}

// 80081840
BOOL drawSphere(const mVec3_c &pos, const mColor &color, int arg2, int arg3,
                int arg4, f32 radius) {
    return FALSE;
}

// 80081848
BOOL fn_80081848() {
    return FALSE;
}

// 80081850
void fn_80081850() {
}

// 80081854
void fn_80081854() {
}

// 80081858
void fn_80081858(BOOL result) {
}

// 8008185C
void fn_8008185C() {
}

// 80081860
BOOL fn_80081860() {
    return FALSE;
}

} // namespace dSceneCheck
