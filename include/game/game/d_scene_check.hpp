#pragma once

// d_scene_check (DOL TU d_scene_check.cpp, .text 80081618..80081868, .rodata
// 8046D5A0, .bss 80583648): the player house rooms opened to visitors in an
// online session (a flag per house 0..3 and room 0..2, set by d_net's "open
// house room" sync), the scene -> house / room lookups, and debug draw / boot
// timing hooks that are empty in this build. No RTTI: the names are inferred.

#include <types.h>

class mVec3_c;
struct mColor;
class mAng3_c;

namespace dSceneCheck {

enum {
    HOUSE_NUM = 4,
    ROOM_NUM = 3, // main floor, upstairs, basement
                  // (dHomeList_c::getHomeFromScene's room)
};

u8 isRoomOpen(int house, int room);      // 80081618
void setRoomOpen(int house, int room);   // 80081634: (d_net open house room)
void clearRoomOpen(int house, int room); // 80081654
u32 isHouseRoomScene(u8 scene); // 80081674: a room of player house 0..3 (TRUE /
                                // FALSE; unsigned: d_net compares with cmplwi)
int getSceneHouse(u8 scene);    // 80081704: its house, -1 if none
int getSceneRoom(u8 scene);     // 800817A0: its room, -1 if none

// Debug hooks (empty in this build).
BOOL fn_80081830(); // 80081830: (no callers)
// 80081838: a debug cylinder (dSTR::isAtFountainFront)
BOOL drawCylinder(const mVec3_c &pos, const mColor &color, int arg2,
                  const mAng3_c &angle, int arg4, f32 radius, f32 height);
// 80081840: a debug sphere (d_bgi_drawNP: the clock's sound position)
BOOL drawSphere(const mVec3_c &pos, const mColor &color, int arg2, int arg3,
                int arg4, f32 radius);
BOOL fn_80081848(); // 80081848: (no callers)
void fn_80081850(); // 80081850: (d_s_boot_static: before its setup calls)
void fn_80081854(); // 80081854: (no callers)
void fn_80081858(BOOL result); // 80081858: (d_s_boot_static: after each setup
                               // call, with its result)
void fn_8008185C();            // 8008185C: (no callers)
BOOL fn_80081860();            // 80081860: (d_s_boot_static)

extern u8 sRoomOpen[HOUSE_NUM][ROOM_NUM]; // 80583648

} // namespace dSceneCheck
