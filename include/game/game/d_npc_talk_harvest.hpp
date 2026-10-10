#pragma once

// d_npc_talk_harvest.cpp (.text 80045154..8004664C; its data is still in the unsplit
// auto_07_804A2A90 .data / auto_06_8046C6C0 .rodata). Not decompiled yet. The villager talk on the
// Harvest Festival (EVENT_HARVEST_FESTIVAL, message file "Ev_Harvest"): the npc asks for an ingredient
// (item 0x44 in the pockets, item select), takes it, and remarks on where it is standing (fn_800464F8
// runs the 12 location checks of 8046C708: near an npc / player house, field block flags, the edge of
// the map, ...).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook, 104 = wait/step procedure (BOOL), 128 / 134 = menu
// result procedures, choice = answer procedures (setChoice). Parameter and return types are inferred
// from the asm; unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_harvest"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; not split yet):
//   8046C6F0 .rodata 0x18  int[6] field block flags for fn_80045F14: 2, 4, 0x400, 0x1000, 0x800, 0
//                          (5 used)
//   8046C708 .rodata 0x90  PTMF table, 12 x {0,-1,fn}: fn_80045A20, fn_80045B38, fn_80045C00,
//                          fn_80045D8C, fn_80045F14, fn_80045FC8, fn_80046098, fn_8004617C,
//                          fn_8004627C, fn_800462F8, fn_800463F0, fn_800464A8
//   804A2BB0 .data   0xB   "Ev_Harvest" (+1 padding)
//   804A2BBC .data   0xC   label table, 3 x const char *: "Ai_Quest", "Q_Cancel", "Ev_Harvest"
//                          (read by nml getMsgLabel kind 0xA)
//   804A2BC8..804A2C40 .data 0xC each, PTMF {0,-1,fn}: fn_80045544, fn_8004660C, fn_800455D0,
//                          fn_8004573C, fn_800456E0, fn_80045798, fn_8004581C, fn_80045874,
//                          fn_800458EC, fn_80045970, fn_800464F8
//   804A2C4C .data   0x14  "sys_STRING/STR_Unit" (fn_80045C00 / fn_80045D8C, word unit file)
//   80749A50 .sdata  0x4   dQuestEvent_e EVENT_HARVEST_FESTIVAL (0x16) (fn_80045154)
//   80749A54 .sdata  0x4   dQuestEvent_e EVENT_HARVEST_FESTIVAL (0x16) (fn_800464A8)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_harvestLabels[3]; // "Ai_Quest", "Q_Cancel", "Ev_Harvest"
