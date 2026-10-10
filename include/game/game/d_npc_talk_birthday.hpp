#pragma once

// d_npc_talk_birthday.cpp (.text 8004902C..800493EC, .rodata 8046C828..8046C838, .data
// 804A2F00..804A2F10).
// Birthdays (message file "Ev_Birthday"): talk procedures for the player's birthday (the npc congratulates,
// gives present item 0x8E0; the Feb 29 birthday gets its own lines) and for the npc's own birthday.
// The npc's mood (dNpcMood_c, dAcNpcNml_c+0x1FB0) used to be listed here; it is the next TU, d_npc_talk_mood.
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook, 104 = wait/step procedure (BOOL). The mood object's
// functions (800493EC.., d_npc_talk_mood) do not run on a talk_c: they are members of dNpcMood_c (d_a_npc_nml.hpp). Parameter and return
// types are inferred from the asm; unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_birthday"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; split in splits.txt):
//   8046C828 .rodata 0xC   "Ev_Birthday" (+4 padding)
//   8046C838 .rodata 0x14  int[5] mood wait anm ids: 0, 0xC4, 0xC6, 0xC8, 0xC6
//   8046C84C .rodata 0x14  int[5] mood walk anm ids: 1, 0xC5, 0xC7, 0xC9, 0xC7
//   8046C860 .rodata 0x15  "afm_mnp_happy_walk_L" (+3)
//   8046C878 .rodata 0x15  "afm_mnp_happy_walk_R" (+3)
//   8046C890 .rodata 0xE   "afm_mnp_pun_S" (+2)
//   8046C8A0 .rodata 0xE   "afm_mnp_sad_S"
//   804A2F00 .data   0x10  PTMF {0,-1,msgPlayerBirthdayCake} (+4 padding)
//   804A2F10 .data   0x3C  function-local static PTMF[5] of fn_80049A64: null, fn_80049CA0,
//                          fn_80049D98, fn_80049F00, null (entries 0 and 4 copied from __ptmf_null on
//                          first use; guard byte 8074E1B8 .sbss)
//   804A2F4C .data   0xC   PTMF {0,-1,fn_80049CCC}
//   804A2F58 .data   0xC   PTMF {0,-1,fn_80049DC4}
//   804A2F64 .data   0xC   PTMF {0,-1,fn_80049F2C}
//   8074E1B8 .sbss   0x8   init guard of 804A2F10
