#pragma once

// d_npc_talk_quest_q12.cpp (.text 80050F4C..80051C68, .rodata 8046C8E0..8046C920, .data 804A38F0..804A3A20,
// .sdata 80749AA8). Not decompiled yet. Villager talk for the lost-key request ("Q12_*" messages,
// QUEST_KIND_REQUEST_6 = dQuestVillager_c kind 6 at dAnimal_c+0x2BE6): the player hands back the key
// the villager asked for (item select limited to BITM kind 0x35 = keys), gets a reward
// (dAnimal_c::pickLostItemReward), and the quest is closed (dLostQuest_c::setTime at
// dSaveData_c+0x1E2CA once no player is left on it).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slots, as
// seen from the d_a_npc_nml helpers (provisional):
//   EC   message select (setMsgProc / setProcSet, run by startMsg): fills the msgInfo_s through
//        setLooksMsg (label, code) and returns 1.
//   F8   one-shot hook (setHookProc / setProcSet, called once by fn_800313B0): no result.
//   104  wait/step procedure (setStepProc, called by fn_8003209C): BOOL; most only act once the
//        message controller is idle (Rcpt_c::mpController+0x6C84 == 0).
//   128 / 134  request-end procedures (stored directly at talk_c+0x128 / +0x134 = _110[2] / _110[3]).
//   choice  answer procedures registered with setChoiceProc / setChoice (talk_c+0x214 table).
// getAnimal() is the npc's dAnimal_c. Parameter and return types are inferred from the asm.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_q12"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046C8E0 .rodata 0xA   "Q12_KeyOK"
//   8046C8EC .rodata 0x9   "Q12_Item"
//   8046C8F8 .rodata 0x9   "Q12_Item"
//   8046C904 .rodata 0xA   "Q12_KeyNG"
//   8046C910 .rodata 0xB   "Q12_Cancel" (+ pad to 8046C920)
//   804A38F0 .data   0x9   "Q12_Con1"
//   804A38FC .data   0xC   "Q12_Report2"
//   804A3908 .data   0xC   "Q12_Report1"
//   804A3914 .data   0x9   "Q12_Con2"
//   804A3920 .data   0xA   "Q12_Visit"
//   804A392C .data   0x54  label table, 7 x const char *: "Q12_Con1", "Q12_Report2", "Q12_Report1",
//                          "Q12_Con2", "Q12_Con2", "Q12_Req" (80749AA8), "Q12_Visit"; then PTMF x3
//                          (104 procs of fn_80050F4C): fn_80051B38, fn_8005125C, fn_80051BF0; then
//                          "sys_STRING/STR_Unit" (merged literal, fn_801A5874 argument)
//   804A3980 .data   0xC   PTMF {0,-1,fn_800512F0}
//   804A398C .data   0x30  PTMF x4: fn_80051380, fn_800514E0, fn_80051A04, fn_80051B08
//   804A39BC..804A3A10 .data 0xC each, PTMF {0,-1,fn}: fn_8005153C, fn_800515A0, fn_8005162C,
//                          fn_800516B8, fn_80051810, fn_800519D4, fn_80051A60, fn_80051AC4 (+ pad to 804A3A20)
//   80749AA8 .sdata  0x8   "Q12_Req" (entry 5 of the label table; probably this TU's)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q12Labels[7]; // "Q12_Con1", "Q12_Report2", ... "Q12_Visit"
