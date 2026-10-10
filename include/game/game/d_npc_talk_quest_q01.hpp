#pragma once

// d_npc_talk_quest_q01.cpp (.text 8004FADC..80050F4C; its data is still in the unsplit auto_07_804A2A90
// .data / auto_11 .sdata / .sdata2). Not decompiled yet. The villager talk for the request quest of
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

// Data of the TU (file-local statics in the .cpp; not split yet):
//   804A3780 .data   0xC   PTMF {0,-1,fn_80050D98} (fn_8004FADC)
//   804A378C .data   0xC   PTMF {0,-1,fn_8004FB24} (fn_80050D98)
//   804A3798 .data   0x14  "sys_STRING/STR_Unit" (fn_80050D98; fn_8004FB84 as 804A3780+0x18)
//   804A37AC .data   0xC   PTMF {0,-1,nml stepRequestChoice} (fn_8004FB24)
//   804A37B8 .data   0xC   "Q01_Lose2" (+2 padding)
//   804A37C4 .data   0xC   "Q01_Lose1" (+2 padding)
//   804A37D0 .data   0x5C  quest descriptor (nml 80033230 returns it for the kind; fn_8004FB84 reads its
//                          PTMFs as 804A3780+0x70..0xA0): 8 labels "Ai_Quest",
//                          "Ai_Quest", "Ai_Quest", "Q01_Lose2", "Q01_Lose1", "Ai_Quest", "Q01_Req", "Q01_Req";
//                          5 PTMFs fn_80053224 (another TU), fn_80050D98, fn_800509F8, fn_80050964,
//                          fn_80050094
//   804A382C..804A38D4 .data 0xC each, PTMF {0,-1,fn}: fn_80050268, nml selResumeTalk, fn_80050318,
//                          fn_80050448, nml msgCancel, fn_800504A4, fn_8005052C, fn_800506D0,
//                          fn_80050828, fn_80050888, fn_80050938, fn_80050BCC, nml selResumeTalk,
//                          fn_80050C20, fn_80050C80
//   804A38E0 .data   0x10  PTMF {0,-1,fn_80050CF8} (+4 padding)
//   80749AA0 .sdata  0x8   "Q01_Req" (descriptor)
//   80750180 .sdata2 0x8   "Q01_Req"
//   80750188 .sdata2 0x4   100.0f (rndF range, fn_8004FB84)
//   8075018C .sdata2 0x4   70.0f (fn_8004FB84: no talk when rndF(100) < 70 and the player is not in the quest)
//   80750190 .sdata2 0x8   "Q01_Win"
//   80750198 .sdata2 0x8   "Q01_Win"
//   807501A0 .sdata2 0x8   "Q01_Win"
//   807501A8 .sdata2 0x8   "Q01_Con"
//   807501B0 .sdata2 0x8   "Q01_Con"

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q01Labels[8]; // "Ai_Quest" / "Q01_*" labels of the quest steps
