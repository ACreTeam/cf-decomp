#pragma once

// d_npc_talk_halloween.cpp (.text 80043EBC..80045154; its data is still in the unsplit
// auto_07_804A2A90 .data / auto_06_8046C6C0 .rodata). Not decompiled yet. The villager talk on
// Halloween (EVENT_HALLOWEEN, message file "Ev_Halloween"): the npc asks for candy (item select),
// takes it from the pockets, plays tricks when the player has none (replaces a pocket item, changes the
// player's clothes), reacts to the player's costume (fn_800447EC) and hands out a present.
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook, 104 = wait/step procedure (BOOL), 128 / 134 = menu
// result procedures, choice = answer procedures (setChoice / setChoiceProc). Parameter and return types
// are inferred from the asm; unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_halloween"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; not split yet):
//   8046C6D0 .rodata 0xD   "Ev_Halloween"
//   8046C6E0 .rodata 0xD   "Ev_Halloween" (second copy, used by fn_80044B9C)
//   804A2AB0..804A2AD4 .data 0xC each, PTMF {0,-1,fn}: fn_80043F48, fn_80043FDC, fn_8004409C, fn_80044B48
//   804A2AE0 .data   0x30  PTMF x4: fn_80044120, fn_800442A8, fn_80044480, fn_80044A34 (fn_80044120
//                          reads entries 1..3 off the 804A2AB0 base: probably separate constants)
//   804A2B10..804A2B94 .data 0xC each, PTMF {0,-1,fn}: fn_80044304, fn_80044368, fn_800443EC,
//                          fn_800444DC, fn_80044540, fn_80044A00, fn_80044A04, fn_80044A98,
//                          fn_80044B18, fn_80044A34, fn_80044C50, fn_80044FBC
//   804A2BA0 .data   0x10  PTMF {0,-1,fn_80045038} + 4 bytes padding
//   80749A48 .sdata  0x8   dQuestEvent_e EVENT_HALLOWEEN (0x15) for dEvent::isOngoing (+4 padding)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char l_Ev_Halloween[13]; // "Ev_Halloween"
