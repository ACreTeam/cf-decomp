#pragma once

// d_npc_talk_quest_q10.cpp (.text 800573D0..80058990, .rodata 8046CA18..8046CA90, .data 804A40F8..804A4268,
// .sdata 80749AB0, .sdata2 80750220..80750248). Not decompiled yet. Villager talk for hide and seek
// ("Q10_*" messages, QUEST_KIND_HIDE_AND_SEEK = dQuestPlayerAnimal_c at dSaveData_c+0x1E2FE): the
// offer (not in rain/snow, 6:00..21:59), the explanation and the game start, the hiders' "found" /
// "wait" / "time over" talks, the reward and the losing end. Friendship of unfound hiders drops when
// the game is lost (fn_800588BC).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slots, as
// seen from the d_a_npc_nml helpers (provisional):
//   EC   message select (setMsgProc / setProcSet, run by startMsg): fills the msgInfo_s through
//        setLooksMsg (label, code) and returns 1.
//   F8   one-shot hook (setHookProc / setProcSet, called once by fn_800313B0): no result.
//   104  wait/step procedure (setStepProc, called by fn_8003209C): BOOL; most only act once the
//        message controller is idle (Rcpt_c::mpController+0x6C84 == 0).
//   128 / 134  request-end procedures (stored directly at talk_c+0x128 / +0x134 = _110[2] / _110[3]).
//   choice  answer procedures registered with setChoiceProc (talk_c+0x214 table).
// getAnimal() is the npc's dAnimal_c. Parameter and return types are inferred from the asm.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_q10"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046CA18 .rodata 0x3C  "Q10_Explain", "Q10_Find", "Q10_Over", "Q10_Wait", "Q10_Visit" (merged by the split)
//   8046CA54 .rodata 0xD   "Q10_Continue"
//   8046CA64 .rodata 0x9   "Q10_Item"
//   8046CA70 .rodata 0xD   "Q10_ItemFull"
//   8046CA80 .rodata 0x9   "Q10_Lose" (+ pad to 8046CA90)
//   804A40F8..804A4170 .data 0xC each, PTMF {0,-1,fn}: fn_80053224, fn_800575AC, fn_80057624, fn_80057684,
//                          fn_8005776C, fn_80057A10, fn_800577C0, fn_800578A0, fn_80057918, fn_80057988,
//                          fn_80057A64
//   804A417C .data   0xB   "Q10_Badend" (label of fn_80057A90, via the 80749AB0 table)
//   804A4188 .data   0xC   PTMF {0,-1,fn_80057C64}
//   804A4194 .data   0xC   PTMF {0,-1,fn_80053224}
//   804A41A0 .data   0xC   PTMF {0,-1,fn_80057DB0}
//   804A41AC .data   0x50  PTMF x5: fn_80057E20, fn_80058388, fn_80058388, fn_80058114, fn_80053224; then
//                          "sys_STRING/STR_Unit" (merged literal)
//   804A41FC .data   0xC   PTMF {0,-1,fn_80058270}
//   804A4208 .data   0xC   PTMF {0,-1,fn_800582F8}
//   804A4214 .data   0x30  PTMF x4: fn_80058270, fn_800586BC, fn_80058780, fn_80058574
//   804A4244 .data   0xC   PTMF {0,-1,fn_80058754}
//   804A4250 .data   0xC   PTMF {0,-1,fn_80058754}
//   804A425C .data   0xC   PTMF {0,-1,fn_8005885C}
//   80749AB0 .sdata  0x8   label table {"Q10_Badend", NULL}? (read by nml getMsgLabel; probably this TU's)
//   80750220 .sdata2 0x8   "Q10_Req"
//   80750228 .sdata2 0x7   "Q10_OK"
//   80750230 .sdata2 0x8   "Q10_No"
//   80750238 .sdata2 0x8   0.0f
//   80750240 .sdata2 0x8   "Q10_End"
// Not this TU's: lbl_8074EBE8 (.sbss, weather manager, d_weather.hpp; fn_800573D0 reads its _5884).

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q10Labels[2]; // "Q10_Badend" (.sdata)
