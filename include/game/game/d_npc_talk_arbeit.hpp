#pragma once

// d_npc_talk_arbeit.cpp (.text 80038134..80039B38, .rodata 8046C178..8046C1C0, .data 804A1D58..804A1F20).
// Not decompiled yet. The villager talk for errands ("arbeit", message file "Ev_Arbeit"): the npc
// asks the player to run an errand (deliver an item / a letter, dQuestErrandList_c at
// dPrivateData_c+0x7FEE), checks a running errand when the player talks to the sender or the
// receiver, hands out the reward, and the moving-in / moving-out remarks of the "Ai_Quest" group.
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slots, as
// seen from the d_a_npc_nml helpers (provisional):
//   EC   message select (set by setMsgProc / setProcSet, called by startMsg): fills the
//        msgInfo_s through setLooksMsg (label, code) and returns 1.
//   F8   end-of-talk hook (setHookProc / setProcSet, called once by fn_800313B0): no result.
//   104  wait/step procedure (setStepProc, called by fn_8003209C): BOOL; most only act once the
//        message controller is idle (Rcpt_c::mpController+0x6C84 == 0).
//   128 / 134  menu result procedures (stored directly at talk_c+0x128 / +0x134).
//   choice  answer procedures registered with setChoice / setChoiceProc (talk_c+0x214 table).
// Parameter and return types are inferred from the asm; unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_arbeit"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046C178 .rodata 0x48  PTMF table, 6 x {0,-1,fn}: fn_80038134, fn_80038238, fn_80038570,
//                          fn_80038580, fn_80038590, fn_800385A0 (checks of fn_800386A0)
//   804A1D58 .data   0x18  label table, 6 x const char *: "Ev_Arbeit", "Ev_Arbeit", "Ai_Quest" x3,
//                          "Ev_Arbeit" (strings in d_a_npc_nml; read by nml getMsgLabel)
//   804A1D70 .data   0xC   PTMF {0,-1,fn_800387BC}
//   804A1D7C .data   0xC   PTMF {0,-1,fn_800397DC}
//   804A1D88 .data   0xC   PTMF {0,-1,fn_80038894}
//   804A1D94 .data   0xC   PTMF {0,-1,fn_800396EC}
//   804A1DA0 .data   0xC   PTMF {0,-1,fn_80038928}
//   804A1DAC .data   0xC   PTMF {0,-1,fn_80038A94}
//   804A1DB8 .data   0xC   PTMF {0,-1,fn_800396BC}
//   804A1DC4 .data   0x48  PTMF x6: fn_80038AF0, fn_80038D4C, fn_8003941C, fn_80039564,
//                          fn_800395BC, fn_80039564 (fn_80038AF0 reads entries 1..5 off the
//                          804A1D58 base: probably separate constants merged by the split)
//   804A1E0C..804A1F14 .data 0xC each, PTMF {0,-1,fn}: fn_80038DCC, fn_80038EB4, fn_80038F40,
//                          fn_80038FA4, fn_80039014, fn_80039068, fn_80039134, fn_8003920C,
//                          fn_80039364, fn_80039278, fn_800393B8, fn_80038FA4, fn_800394A8,
//                          fn_8003950C, fn_80039658, fn_80039564, fn_80039740, fn_80039A44,
//                          fn_800398F4, fn_80039A94, fn_80039948, fn_80039A44, fn_80039AE8

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_arbeitLabels[6]; // "Ev_Arbeit" / "Ai_Quest" by state
