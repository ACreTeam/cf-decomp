#pragma once

// d_npc_talk_town.cpp (.text 800613BC..80061ADC, .rodata 8046CD60..8046CDB0, .data 804A4BB8..804A4C10,
// .sdata 80749B20..80749B50, .sdata2 807502E0..807502E8). The villager's small talk
// about the town (message files "Town_Rumor", "Town_Always", "Town_Theater", "Town_Grace") and the
// "3P_*" remarks about another villager (one label per personality).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers (records
// 17..20 of the d_a_npc_nml table 804A0784, slot EC). See d_npc_talk_arbeit.hpp for the slot
// conventions. Parameter and return types are inferred from the asm; unused incoming registers are left
// out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_town"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046CD60 .rodata 0xB   "Town_Rumor"
//   8046CD6C .rodata 0xC   "Town_Always"
//   8046CD78 .rodata 0x18  PTMF table, 2 x {0,-1,fn}: msgTownRumor, msgTownAlways (choices of msgTown)
//   8046CD90 .rodata 0xD   "Town_Theater"
//   8046CDA0 .rodata 0xB   "Town_Grace"
//   804A4BB8 .data   0x14  "sys_STRING/STR_Unit"
//   804A4BCC .data   0x28  jump table of msgTownRumor (switch on the memory kind 0..9)
//   804A4BF4 .data   0x4   padding (zero)
//   804A4BF8 .data   0x18  label table, 6 x const char *: "3P_Bo", "3P_Ha", "3P_Ko", "3P_Fu", "3P_Ge",
//                          "3P_Ta" (get3PLabel)
//   80749B20 .sdata  0x2E  the six "3P_*" strings (8 bytes apart)
//   807502E0 .sdata2 0x8   u8[2] {20, 80} odds of msgTown (+ padding)
