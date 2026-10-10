#pragma once

// d_npc_talk_birthday.cpp (.text 8004902C..80049FDC; its data is still in the unsplit
// auto_07_804A2A90 .data / auto_06_8046C6C0 .rodata / auto_10_8074E188 .sbss). Not decompiled yet.
// Birthdays (message file "Ev_Birthday"):
// - talk procedures for the player's birthday (the npc congratulates, gives present item 0x8E0; the
//   Feb 29 birthday gets its own lines) and for the npc's own birthday;
// - the npc's birthday mood object (a member of the npc at dAcNpcNml_c+0x1FB0, size 0x1E0: state int
//   at +0x0, member-function pointer at +0x4, dNpcTimer_c at +0x10, three dLevelEffect_c at +0x20 /
//   +0xB4 / +0x148, flags at +0x1DC..+0x1DF). It swaps the wait / walk animations (8046C838 /
//   8046C84C: happy, sad, ...) and plays "afm_mnp_happy_walk_L/R", "afm_mnp_pun_S", "afm_mnp_sad_S".
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook, 104 = wait/step procedure (BOOL). The mood object's
// functions (800493EC..) do not run on a talk_c: they are members of that object's own (unnamed) class,
// kept here as static members with an explicit `self` until that class is named. Parameter and return
// types are inferred from the asm; unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_birthday"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; not split yet):
//   8046C828 .rodata 0xC   "Ev_Birthday" (+4 padding)
//   8046C838 .rodata 0x14  int[5] mood wait anm ids: 0, 0xC4, 0xC6, 0xC8, 0xC6
//   8046C84C .rodata 0x14  int[5] mood walk anm ids: 1, 0xC5, 0xC7, 0xC9, 0xC7
//   8046C860 .rodata 0x15  "afm_mnp_happy_walk_L" (+3)
//   8046C878 .rodata 0x15  "afm_mnp_happy_walk_R" (+3)
//   8046C890 .rodata 0xE   "afm_mnp_pun_S" (+2)
//   8046C8A0 .rodata 0xE   "afm_mnp_sad_S"
//   804A2F00 .data   0x10  PTMF {0,-1,fn_8004926C} (+4 padding)
//   804A2F10 .data   0x3C  function-local static PTMF[5] of fn_80049A64: null, fn_80049CA0,
//                          fn_80049D98, fn_80049F00, null (entries 0 and 4 copied from __ptmf_null on
//                          first use; guard byte 8074E1B8 .sbss)
//   804A2F4C .data   0xC   PTMF {0,-1,fn_80049CCC}
//   804A2F58 .data   0xC   PTMF {0,-1,fn_80049DC4}
//   804A2F64 .data   0xC   PTMF {0,-1,fn_80049F2C}
//   8074E1B8 .sbss   0x8   init guard of 804A2F10
