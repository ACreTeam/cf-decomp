#pragma once

// d_npc_talk_halloween.cpp (.text 80043EBC..80045154, .rodata 8046C6D0..8046C6F0, .data
// 804A2AB0..804A2BB0, .sdata 80749A48..80749A50). The villager talk on
// Halloween (EVENT_HALLOWEEN, message file "Ev_Halloween"), in the villager's house: the npc asks for
// candy (item select, msgCandyAsk), takes it from the pockets, plays tricks when the player gives
// something else or has none (trickClothes: pumpkin head, moldy / patched shirt; trickPockets: a pocket
// item becomes a jack-in-the-box), reacts to the player's costume (isInCostume: hat or accessory with
// the shirt's fashion theme) and hands out a present (stepHalloweenPresent).
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

// Data of the TU (file-local statics in the .cpp; split in splits.txt):
//   8046C6D0 .rodata 0xD   "Ev_Halloween"
//   8046C6E0 .rodata 0xD   "Ev_Halloween" (second copy, used by msgHalloweenCostume)
//   804A2AB0..804A2AD4 .data 0xC each, PTMF {0,-1,fn}: endCandyAsk, stepCandyChoice, selCandyGive, selCandyNone
//   804A2AE0 .data   0x30  PTMF x4: resCandyGive, resCandyGiven, resCandyWrong, msgCandyNone (resCandyGive
//                          reads entries 1..3 off the 804A2AB0 base: probably separate constants)
//   804A2B10..804A2B94 .data 0xC each, PTMF {0,-1,fn}: msgCandyThanks, stepCandyThanks, msgCandyCostume,
//                          msgCandyWrong, stepCandyWrong, resCandyTrick, msgCandyTrick, stepCandyNone,
//                          msgCandyNoneTrick, msgCandyNone, endHalloweenCostume, stepHalloweenFirst
//   804A2BA0 .data   0x10  PTMF {0,-1,stepHalloweenPresent} + 4 bytes padding
//   80749A48 .sdata  0x8   dQuestEvent_e EVENT_HALLOWEEN (0x15) for dEvent::isOngoing (+4 padding)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char l_Ev_Halloween[13]; // "Ev_Halloween"
