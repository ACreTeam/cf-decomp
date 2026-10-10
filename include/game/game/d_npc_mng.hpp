#pragma once

// The npc manager (TU d_npc_mng.cpp, not decompiled). Only what d_a_npc uses.

#include <types.h>

namespace EGG {
class FrmHeap;
}
class mVec3_c;

extern "C" {
extern EGG::FrmHeap *lbl_8074EABC; // the npc manager heap (801A658C); parent of dAcNpc_c::m_heap_p (no split yet)
extern int lbl_8074EAC8;          // theater message code (1..0x1D), set by a REL, cleared by 801AA56C (d_npc_talk_town "Town_Theater")
}

extern "C" {
// Hide-and-seek scene state (0 start, 1 running, 2 end; < 3) -> lbl_8074C180. FALSE while online
// (fn_800DCEDC) or out of range.
BOOL fn_801A6748(int state);                  // 801A6748
BOOL fn_801A8134(mVec3_c *pos, int a, int b); // 801A8134: adjusts the player's return position (q10 game start/end)
void fn_801AD168();                           // 801AD168: sets lbl_8074EACC
void fn_801A6564();                           // 801A6564: (d_reset soft reset) resets the hide-and-seek state
}

namespace dItem {
struct Item;
}
extern "C" {
// Special npcs in town (d_npc_talk_free FreeE_Snpc): the npc item is today's visitor of its kind
// (the item is passed by value).
BOOL fn_801AD174(dItem::Item npc); // 801AD174: an npc of kind 8 of the town list 8047DCCC
BOOL fn_801AD228(dItem::Item npc); // 801AD228: the list 8047DD90 (0xFFF1: any of its two npcs)
BOOL fn_801AD304(dItem::Item npc); // 801AD304: the list 8047E04C, or 0x8007 while visitor 3 is here
}
