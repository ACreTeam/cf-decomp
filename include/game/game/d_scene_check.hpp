#pragma once

// d_scene_check.cpp (not decompiled). Only what other TUs use. The checked scenes are the player house
// rooms: house 0..3, room 0..2 (a flag per room at 80583648).

#include <types.h>

class mVec3_c;
struct mColor;
class mAng3_c;

extern "C" {
void fn_80081634(int house, int room); // 80081634: sets the room's flag
u32 fn_80081674(u8 scene);          // 80081674: 1 for a player house room
int fn_80081704(u8 scene);          // 80081704: its house, -1 if none
int fn_800817A0(u8 scene);          // 800817A0: its room, -1 if none
// 80081838: a debug cylinder (empty in this build: returns FALSE). (dSTR::isAtFountainFront)
BOOL fn_80081838(const mVec3_c &pos, const mColor &color, int arg2, const mAng3_c &angle, int arg4, f32 radius,
                 f32 height);
}
