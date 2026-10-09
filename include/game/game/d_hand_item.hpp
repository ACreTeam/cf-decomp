#pragma once

// The hand-item manager (TU d_hand_item.cpp, not decompiled; static instance lbl_8074E9F8): items
// handed between the players and npcs. Each function checks the instance and forwards to it.
// d_player_mgr.cpp still declares fn_80194A2C / fn_80194AA0 / fn_80194B30 locally (dPlayerActor_c *).

#include <types.h>

class dActor_c;
class dPlayerActor_c;
class mVec3_c;
namespace dItem {
class Item;
}

extern "C" {
BOOL fn_80194A2C(const dItem::Item *item, int a, int b, int c, dActor_c *actor, dPlayerActor_c *player,
                 f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6); // 80194A2C: start (forwards to fn_80191664)
BOOL fn_80194AA0(int type, dActor_c *actor, f32 radius); // 80194AA0: request (type, reach radius)
BOOL fn_80194AC8(dActor_c *actor);                       // 80194AC8: the actor is registered / holding
BOOL fn_80194AEC();                                      // 80194AEC: manager busy (state <= 12)
BOOL fn_80194B10(int state);                             // 80194B10
int fn_80194B30();                                       // 80194B30: use type (13 = no manager)
BOOL fn_80194B4C(int type, dActor_c *actor);             // 80194B4C: set the use type
BOOL fn_80194B74();                                      // 80194B74
BOOL fn_80194B8C(dActor_c *actor);                       // 80194B8C
BOOL fn_80194BCC(dActor_c *actor);                       // 80194BCC: the actor is close to the target position (xz)
void fn_80194CB0(dActor_c *actor);                       // 80194CB0: release the actor's hand item
BOOL fn_80194CEC(mVec3_c *pos);                          // 80194CEC: target position
extern const f32 lbl_80751600;                           // 16.0f: reach radius (.sdata2, no split yet)
}
