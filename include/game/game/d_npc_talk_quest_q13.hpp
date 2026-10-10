#pragma once

// d_npc_talk_quest_q13.cpp (.text 80058990..8005A3C4, .rodata 8046CA90..8046CAE8, .data 804A4268..804A4498,
// .sdata 80749AB8, .sdata2 80750248..80750270). Not decompiled yet. Villager talk for the style quest
// ("Q13_*" messages, QUEST_KIND_STYLE = dQuestPlayerPair_c at dSaveData_c+0x1E3EA): the villager asks
// the player to look at another player's style (dAnimalBlock_c::canStartStyle / pickStyleOtherPlayer /
// startStyle), later asks for the answer (typed into a text menu, compared with the topic text
// dQuestPlayerPair_c::mTopicText), and rewards a right answer (dAnimalBlock_c::pickStylePresent /
// sendStyleLetterTo).
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
// "d_npc_talk_quest_q13"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046CA90 .rodata 0x9   "Q13_Ask2"
//   8046CA9C .rodata 0xD   "Q13_AnswerOK"
//   8046CAAC .rodata 0xD   "Q13_AnswerOK"
//   8046CABC .rodata 0xA   "Q13_Thank"
//   8046CAC8 .rodata 0xD   "Q13_AnswerNG"
//   8046CAD8 .rodata 0xB   "Q13_Forget" (+ pad to 8046CAE8)
//   804A4268..804A4298 .data 0xC each, PTMF {0,-1,fn}: fn_80053224, fn_80058A98, fn_80058B10, fn_80058B98,
//                          fn_80058C98
//   804A42A4 .data   0x14  "sys_STRING/STR_Unit"
//   804A42B8 .data   0xC   PTMF {0,-1,fn_80058D98}
//   804A42C4 .data   0xC   PTMF {0,-1,selNo} (d_a_npc_nml)
//   804A42D0 .data   0xC   PTMF {0,-1,fn_80058DEC}
//   804A42DC .data   0xC   PTMF {0,-1,fn_80058E4C}
//   804A42E8 .data   0x9   "Q13_Ask2"
//   804A42F4 .data   0x9   "Q13_Item"
//   804A4300 .data   0xD   "Q13_ItemFull"
//   804A4310 .data   0x9   "Q13_Over"
//   804A431C .data   0x7C  label table, 7 x const char *: "Ai_Quest" (8046BD54), "Q13_Ask" (80749AB8) x2,
//                          "Q13_Ask2", "Q13_Item", "Q13_ItemFull", "Q13_Over"; then PTMF x8 (F8/104 of
//                          fn_80058F18): fn_80053224, fn_80059504, fn_80053224, fn_80059EF4, fn_8005A1EC,
//                          fn_8005A2A0, fn_8005A364, fn_8005A19C
//   804A4398..804A43EC .data 0xC each, PTMF {0,-1,fn}: fn_8005957C, fn_80059618, fn_800597EC, fn_80059E70,
//                          fn_80059838, fn_8005995C, fn_80059DBC, fn_800599C0
//   804A43F8 .data   0x3C  PTMF x5: fn_80059A38, fn_80059CE8, fn_80059D44, fn_80059B88, fn_80059C20
//   804A4434..804A447C .data 0xC each, PTMF {0,-1,fn}: fn_80059CB8, fn_80059CB8, fn_80059E20, fn_80059EC4,
//                          fn_8005A0C8, fn_8005A148, fn_8005A11C
//   804A4488 .data   0x10  PTMF {0,-1,fn_8005A338} (+ pad to 804A4498)
//   80749AB8 .sdata  0x8   "Q13_Ask" (label table entries 1/2; probably this TU's)
//   80750248 .sdata2 0x8   "Q13_Req"
//   80750250 .sdata2 0x8   "Q13_Yes"
//   80750258 .sdata2 0x8   "Q13_Ask"
//   80750260 .sdata2 0x8   "Q13_Con"
//   80750268 .sdata2 0x8   "Q13_End"

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q13Labels[7]; // "Ai_Quest" / "Q13_*" labels of the quest steps
