#pragma once

// d_npc_talk_quest_q01.cpp (.text 8004FADC..80050F4C, .data 804A3780..804A38F0, .sdata
// 80749AA0..80749AA8, .sdata2 80750180..807501B8). The villager talk for the
// request quest of
// kind QUEST_KIND_REQUEST_INSECT (dQuestVillager_c at dAnimal_c+0x2BE6, mBase.mKind 0; messages
// "Q01_Req" / "Q01_Win" / "Q01_Con" / "Q01_Lose1/2" of the "Ai_Quest" group): the npc asks the
// players for an insect (several players can join), takes the insect from the item select, gives a
// reward (dAnimal_c::pickInsectFishReward: item or bells) and ends or keeps the quest.
// Q01..Q05 are the same file with the kind changed (Q02 fish, Q03 fossil, Q04 cloth, Q05 furniture).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook, 104 = wait/step procedure (BOOL; most act only once
// the message controller is idle), 128 / 134 = menu result procedures, choice = answer procedures
// (setChoice). Parameter and return types are inferred from the asm; unused incoming registers are
// left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_q01"): its procedures are stored in talk_c's member-function pointers.

// Data order: stepInsectAccept is the last function in .text, but its two .data objects (the PTMF of its EC and
// "sys_STRING/STR_Unit") come right after the offer function's PTMF. The .cpp defines them by name
// (a static msgFunc and l_strUnit) right after the offer function; see notes/d_npc_talk.txt.
// Data of the TU (file-local statics in the .cpp; split in splits.txt):
//   804A3780 .data   0xC   PTMF {0,-1,stepInsectAccept} (msgInsectOffer)
//   804A378C .data   0xC   PTMF {0,-1,msgInsectReq} (stepInsectAccept)
//   804A3798 .data   0x14  "sys_STRING/STR_Unit" (stepInsectAccept; msgInsectQuest as 804A3780+0x18)
//   804A37AC .data   0xC   PTMF {0,-1,nml stepRequestChoice} (msgInsectReq)
//   804A37B8 .data   0xC   "Q01_Lose2" (+2 padding)
//   804A37C4 .data   0xC   "Q01_Lose1" (+2 padding)
//   804A37D0 .data   0x5C  label table l_q01Labels (getMsgLabel) followed by the PTMF constants of msgInsectQuest (startQuestOffer
//                          hook/step, then its step procs; msgInsectQuest reads them as 804A3780+0x70..0xA0): 8 labels "Ai_Quest",
//                          "Ai_Quest", "Ai_Quest", "Q01_Lose2", "Q01_Lose1", "Ai_Quest", "Q01_Req", "Q01_Req";
//                          5 PTMFs endQuestCommon (another TU), stepInsectAccept, stepInsectChoice, stepInsectLose,
//                          stepInsectGiveChoice
//   804A382C..804A38D4 .data 0xC each, PTMF {0,-1,fn}: selInsectGive, nml selResumeTalk, resInsectGive,
//                          resInsectGiven, nml msgCancel, msgInsectWin, endInsectWin, stepInsectWin,
//                          msgInsectReward, stepInsectReward, msgInsectRewardEnd, selInsectCon, nml selResumeTalk,
//                          msgInsectCon, stepInsectCon
//   804A38E0 .data   0x10  PTMF {0,-1,msgInsectConPlayers} (+4 padding)
//   80749AA0 .sdata  0x8   "Q01_Req" (descriptor)
//   80750180 .sdata2 0x8   "Q01_Req"
//   80750188 .sdata2 0x4   100.0f (rndF range, msgInsectQuest)
//   8075018C .sdata2 0x4   70.0f (msgInsectQuest: no talk when rndF(100) < 70 and the player is not in the quest)
//   80750190 .sdata2 0x8   "Q01_Win"
//   80750198 .sdata2 0x8   "Q01_Win"
//   807501A0 .sdata2 0x8   "Q01_Win"
//   807501A8 .sdata2 0x8   "Q01_Con"
//   807501B0 .sdata2 0x8   "Q01_Con"

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q01Labels[8]; // "Ai_Quest" / "Q01_*" labels of the quest steps
