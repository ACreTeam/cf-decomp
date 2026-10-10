#pragma once

// d_npc_talk_arbeit.cpp (.text 80038134..80039B38, .rodata 8046C178..8046C1C0, .data 804A1D58..804A1F20).
// The villager talk for the newcomer's first jobs ("arbeit", message file "Ev_Arbeit"; errands
// QUEST_KIND_FIRSTJOB_*, dQuestErrandList_c at dPrivateData_c+0x7FEE, private flag 0xD while the player
// is a newcomer): checks a running delivery when the player talks to the npc, takes the item, hands
// out the reward (the furniture delivery ends with the player's birthday entry, reqMenu1C), the
// newcomer progress talk (dPrivateData_c+0x8696), and the moving-in / moving-out remarks.
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
//   8046C178 .rodata 0x48  PTMF table, 6 x {0,-1,fn}: msgArbeitMoveIn, msgArbeitMoveOut, msgArbeitDeliverFtr,
//                          msgArbeitDeliverCarpet, msgArbeitDeliverCan, msgArbeitNewcomer (checks of msgArbeit)
//   804A1D58 .data   0x18  label table, 6 x const char *: "Ev_Arbeit", "Ev_Arbeit", "Ai_Quest" x3,
//                          "Ev_Arbeit" (strings in d_a_npc_nml; read by nml getMsgLabel)
//   804A1D70 .data   0xC   PTMF {0,-1,stepArbeitErrandChoice}
//   804A1D7C .data   0xC   PTMF {0,-1,stepArbeitNewcomerChoice}
//   804A1D88 .data   0xC   PTMF {0,-1,selArbeitGive}
//   804A1D94 .data   0xC   PTMF {0,-1,selArbeitOther}
//   804A1DA0 .data   0xC   PTMF {0,-1,resArbeitGive}
//   804A1DAC .data   0xC   PTMF {0,-1,resArbeitGiven}
//   804A1DB8 .data   0xC   PTMF {0,-1,msgArbeitCancel}
//   804A1DC4 .data   0x48  PTMF x6: msgArbeitGiven, stepArbeitFtrDone, stepArbeitCarpetReward, stepArbeitErrandNext,
//                          stepArbeitLetterMenu, stepArbeitErrandNext (msgArbeitGiven reads entries 1..5 off the
//                          804A1D58 base: probably separate constants merged by the split)
//   804A1E0C..804A1F14 .data 0xC each, PTMF {0,-1,fn}: msgArbeitFtrReward, stepArbeitFtrReward, msgArbeitBirthdayAsk,
//                          stepArbeitBirthdayMenu, resArbeitBirthday, msgArbeitBirthdayCheck, stepArbeitBirthdayChoice, selArbeitBirthdayYes,
//                          selArbeitBirthdayNo, msgArbeitBirthdayYes, msgArbeitBirthdayNo, stepArbeitBirthdayMenu, msgArbeitCarpetThanks,
//                          stepArbeitErrandDone, msgArbeitLetter, stepArbeitErrandNext, msgArbeitProgress, stepArbeitProgressNext,
//                          selArbeitProgress, selArbeitNo, msgArbeitProgressCheck, stepArbeitProgressNext, msgArbeitNo

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_arbeitLabels[6]; // "Ev_Arbeit" / "Ai_Quest" by state
