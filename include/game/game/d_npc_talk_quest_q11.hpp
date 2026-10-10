#pragma once

// d_npc_talk_quest_q11.cpp (.text 80051C68..80052EB0, .rodata 8046C920..8046C958, .data 804A3A20..804A3BC8,
// .sdata2 807501B8..807501C8). Not decompiled yet. Villager talk for the sick-villager request
// ("Q11_*" messages, QUEST_KIND_REQUEST_5 = dQuestVillager_c kind 5 at dAnimal_c+0x2BE6, with
// dQuestSick_c at dSaveData_c+0x1E284): the player gives medicine (dQuestSick_c::getMedicine, item
// select), the villager thanks/rewards (dAnimal_c::pickSickReward / sendSickReward), visiting players
// are recorded (dQuestSick_c::setPlayer, dQuestVillager_c::addPlayer). Message codes are
// dAnimal_c+0x2C64 * 3 + rnd(3) + 1.
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
// "d_npc_talk_quest_q11"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046C920 .rodata 0xD   "Q11_Medicine"
//   8046C930 .rodata 0x9   "Q11_Cure"
//   8046C93C .rodata 0xB   "Q11_Cancel"
//   8046C948 .rodata 0x9   "Q11_Talk" (+ pad to 8046C958)
//   804A3A20 .data   0xB   "Q11_GreetA"
//   804A3A2C .data   0xB   "Q11_GreetB"
//   804A3A38 .data   0x9   "Q11_Item"
//   804A3A44 .data   0x9   "Q11_Full"
//   804A3A50 .data   0xC   "Q11_Report1"
//   804A3A5C .data   0xC   "Q11_Report2"
//   804A3A68 .data   0xA8  label table, 13 x const char *: GreetA x3, GreetB x3, GreetA x3, "Q11_Item",
//                          "Q11_Full", "Q11_Report1", "Q11_Report2"; then PTMF x8 (F8/104 of fn_80051C68):
//                          fn_80052818, fn_80052A34, fn_80052190, fn_80052AAC, fn_80052B48, fn_80052C88,
//                          fn_80052D04, fn_80052E04; then "sys_STRING/STR_Unit" (merged literal)
//   804A3B10..804A3BB8 .data 0xC each, PTMF {0,-1,fn}: fn_80052268, fn_8005289C, fn_800522BC, fn_8005237C,
//                          fn_80052440, fn_80052578, fn_8005274C, fn_800525D4, fn_80052694, fn_80052818,
//                          fn_800528F0, fn_800529B0, fn_800528F0, fn_80052C5C, fn_80052C5C (+ pad to 804A3BC8)
//   807501B8 .sdata2 0x8   3.0f (rnd range of the message code)
//   807501C0 .sdata2 0x8   "Q11_End"

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q11Labels[13]; // "Q11_GreetA", "Q11_GreetB", "Q11_Item", ...
