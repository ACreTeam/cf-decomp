#pragma once

// d_npc_talk_quest_delivery.cpp (.text 80052EB0..800546F4, .rodata 8046C958..8046C9D0,
// .data 804A3BC8..804A3DA8, .sdata2 807501C8..807501D8). Not decompiled yet. Villager talk when the
// player brings an errand delivery (dQuestErrandList_c at dPrivateData_c+0x7FEE, errand kinds 7 / 8):
// the player picks the package from the pockets, the villager receives it and reacts ("Q06_*" for
// kind 7, "Q07_*" for kind 8 = clothes: the villager wears them depending on
// dAnimal_c::getStyleMatch). Also holds fn_80053224, the common end-of-talk F8 hook used by many talk
// tables (d_a_npc_nml, quest q06..q10, q13 ...).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slots, as
// seen from the d_a_npc_nml helpers (provisional):
//   EC   message select (setMsgProc / setProcSet, run by startMsg): fills the msgInfo_s through
//        setLooksMsg (label, code) and returns 1.
//   F8   one-shot hook (setHookProc / setProcSet, called once by fn_800313B0): no result.
//   104  wait/step procedure (setStepProc, called by fn_8003209C): BOOL; most only act once the
//        message controller is idle (Rcpt_c::mpController+0x6C84 == 0).
//   128 / 134  request-end procedures (stored directly at talk_c+0x128 / +0x134 = _110[2] / _110[3]).
//   choice  answer procedures registered with setChoice (talk_c+0x214 table).
// getAnimal() is the npc's dAnimal_c. Parameter and return types are inferred from the asm.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_delivery"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046C958 .rodata 0x9   "Q06_Get0"
//   8046C964 .rodata 0x9   "Q06_Get1"
//   8046C970 .rodata 0x9   "Q06_Open"
//   8046C97C .rodata 0x9   "Q07_Get0"
//   8046C988 .rodata 0x9   "Q07_Get1"
//   8046C994 .rodata 0x9   "Q07_Get2"
//   8046C9A0 .rodata 0x9   "Q07_Get3"
//   8046C9AC .rodata 0xA   "Q07_Open1"
//   8046C9B8 .rodata 0xA   "Q07_Open2"
//   8046C9C4 .rodata 0xA   "Q07_Open3" (+ pad to 8046C9D0)
//   804A3BC8 .data   0x2C  PTMF x2 (104 of fn_80052EB0 by errand kind): fn_80053414, fn_80053BDC; then
//                          "sys_STRING/STR_Unit" (merged literal)
//   804A3BF4 .data   0xC   PTMF {0,-1,fn_800535E8}
//   804A3C00 .data   0xC   PTMF {0,-1,fn_800533C0}
//   804A3C0C .data   0x30  PTMF x4: fn_80053628, fn_80053774, fn_80053A30, fn_80053390
//   804A3C3C..804A3C9C .data 0xC each, PTMF {0,-1,fn}: fn_800537D0, fn_80053840, fn_800538C0, fn_80053924,
//                          fn_80053A04, fn_80053A8C, fn_80053AFC, fn_80053A04, fn_80053DB0
//   804A3CA8 .data   0xC   PTMF {0,-1,fn_800533C0}
//   804A3CB4 .data   0x30  PTMF x4: fn_80053DF0, fn_80053F44, fn_800544C0, fn_80053390
//   804A3CE4 .data   0xC   PTMF {0,-1,fn_80053FA0}
//   804A3CF0 .data   0x30  PTMF x4: fn_800540C8, fn_800541D0, fn_8005440C, fn_8005439C
//   804A3D20..804A3D44 .data 0xC each, PTMF {0,-1,fn}: fn_80054240, fn_80054370, fn_80054338, fn_80054240
//   804A3D50 .data   0x30  PTMF x4: fn_8005447C, fn_800545A4, fn_80054684, fn_80054614
//   804A3D80 .data   0xC   PTMF {0,-1,fn_80054240}
//   804A3D8C .data   0xC   PTMF {0,-1,fn_80054240}
//   804A3D98 .data   0x10  PTMF {0,-1,fn_8005447C} (+ pad to 804A3DA8)
//   807501C8 .sdata2 0x8   "Q06_Fin"
//   807501D0 .sdata2 0x8   "Q07_Fin"
