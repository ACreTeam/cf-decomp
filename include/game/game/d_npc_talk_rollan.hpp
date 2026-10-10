#pragma once

// d_npc_talk_rollan.cpp (.text 80060980..800613BC, .rodata 8046CD20..8046CD5F, .data 804A4B60..804A4BB7,
// .sdata2 807502D8..807502DF). Not decompiled yet. The villager's "Ev_Rollan" talk: the npc offers the
// player a present (item 0x36C / 0x313 into talk_c+0x280) as one answer of a two-choice menu, with a
// chance that grows with two values of the player's home (dHomeList_c::findOwner, fn_800AC28C).
// fn_80060980 is also used by the quest talk TUs (d_npc_talk_quest_*) to add the Rollan answer to
// their own choice menus. Per-npc state at +0x72CC0 of the save (fn_8015112C on getRaw(),
// fn_801510EC on getTown()).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers (record 7
// of the d_a_npc_nml table 804A0E48 = {fn_80060C44, fn_80060D28, fn_80060DB8}). See
// d_npc_talk_arbeit.hpp for the slot conventions. Parameter and return types are inferred from the asm;
// unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_rollan"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046CD20 .rodata 0xA   "Ev_Rollan"
//   8046CD30 .rodata 0x30  PTMF table, 4 x {0,-1,fn}: fn_80061034, fn_800611A4, fn_800611F8, fn_80061368
//                          (answers of fn_80060980)
//   804A4B60 .data   0xC   PTMF {0,-1,selResumeTalk} (nml; second choice of fn_80060DB8)
//   804A4B6C .data   0xC   PTMF {0,-1,fn_800610C4}
//   804A4B78 .data   0xC   PTMF {0,-1,fn_80061140}
//   804A4B84 .data   0xC   PTMF {0,-1,fn_80061140}
//   804A4B90 .data   0xC   PTMF {0,-1,fn_80061288}
//   804A4B9C .data   0xC   PTMF {0,-1,fn_80061304}
//   804A4BA8 .data   0x10  PTMF {0,-1,fn_80061304} (+ 4 bytes padding)
//   807502D8 .sdata2 0x8   u16[4] {7, 7, 10, 10} menu codes of the 4 answers
