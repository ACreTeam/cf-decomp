#pragma once

// d_npc_talk_rollan.cpp (.text 80060980..800613BC, .rodata 8046CD20..8046CD60, .data 804A4B60..804A4BB8,
// .sdata2 807502D8..807502E0). The villager's "Ev_Rollan" talk: the npc offers the player a present
// (old flooring / old wallpaper into mItem0, by the npc's state 1 / 2 in the save) as one answer of a
// two-choice menu, with a chance that grows with two bytes of the player's house rating record
// (dHomeList_c::findOwner, fn_800AC28C).
// getRollanChoice is also used by the quest talk TUs (d_npc_talk_quest_*) to add the Rollan answer to
// their own choice menus. Per-npc state at +0x72CC0 of the save (fn_8015112C on getRaw(),
// fn_801510EC on getTown()).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers (record 7
// of the d_a_npc_nml table 804A0E48 = {msgRollan, endRollan, stepRollan}). See
// d_npc_talk_arbeit.hpp for the slot conventions. Parameter and return types are inferred from the asm;
// unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_rollan"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046CD20 .rodata 0xA   "Ev_Rollan"
//   8046CD30 .rodata 0x30  PTMF table, 4 x {0,-1,fn}: selRollanFloor, selRollanFloorFull, selRollanWall, selRollanWallFull
//                          (answers of getRollanChoice)
//   804A4B60 .data   0xC   PTMF {0,-1,selResumeTalk} (nml; second choice of stepRollan)
//   804A4B6C .data   0xC   PTMF {0,-1,msgRollanFloorGift}
//   804A4B78 .data   0xC   PTMF {0,-1,msgRollanFloorNone}
//   804A4B84 .data   0xC   PTMF {0,-1,msgRollanFloorNone}
//   804A4B90 .data   0xC   PTMF {0,-1,msgRollanWallGift}
//   804A4B9C .data   0xC   PTMF {0,-1,msgRollanWallNone}
//   804A4BA8 .data   0x10  PTMF {0,-1,msgRollanWallNone} (+ 4 bytes padding)
//   807502D8 .sdata2 0x8   u16[4] {7, 7, 10, 10} menu codes of the 4 answers
