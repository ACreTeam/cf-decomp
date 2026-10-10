#pragma once

// d_npc_talk_quest_q02.cpp (.text 8004B75C..8004CBCC; its data is still in the unsplit auto_07_804A2A90
// .data / auto_11 .sdata / .sdata2). Not decompiled yet. The villager talk for the request quest of
// kind QUEST_KIND_REQUEST_FISH (dQuestVillager_c at dAnimal_c+0x2BE6, mBase.mKind 1; messages
// "Q02_Req" / "Q02_Win" / "Q02_Con" / "Q02_Lose1/2" of the "Ai_Quest" group): the npc asks the
// players for a fish (several players can join), takes the fish from the item select, gives a
// reward (dAnimal_c::pickInsectFishReward: item or bells) and ends or keeps the quest.
// Function for function the same code as d_npc_talk_quest_q01 (insect) with the kind / ids changed.
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
// "d_npc_talk_quest_q02"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; not split yet):
//   804A3290 .data   0xC   PTMF {0,-1,fn_8004CA18} (fn_8004B75C)
//   804A329C .data   0xC   PTMF {0,-1,fn_8004B7A4} (fn_8004CA18)
//   804A32A8 .data   0x14  "sys_STRING/STR_Unit" (fn_8004CA18; fn_8004B804 as 804A3290+0x18)
//   804A32BC .data   0xC   PTMF {0,-1,nml stepRequestChoice} (fn_8004B7A4)
//   804A32C8 .data   0xC   "Q02_Lose2" (+2 padding)
//   804A32D4 .data   0xC   "Q02_Lose1" (+2 padding)
//   804A32E0 .data   0x5C  quest descriptor (nml 80033230 returns it for the kind; fn_8004B804 reads its
//                          PTMFs as 804A3290+0x70..0xA0): 8 labels "Ai_Quest",
//                          "Ai_Quest", "Ai_Quest", "Q02_Lose2", "Q02_Lose1", "Ai_Quest", "Q02_Req", "Q02_Req";
//                          5 PTMFs fn_80053224 (another TU), fn_8004CA18, fn_8004C678, fn_8004C5E4,
//                          fn_8004BD14
//   804A333C..804A33E4 .data 0xC each, PTMF {0,-1,fn}: fn_8004BEE8, nml selResumeTalk, fn_8004BF98,
//                          fn_8004C0C8, nml msgCancel, fn_8004C124, fn_8004C1AC, fn_8004C350,
//                          fn_8004C4A8, fn_8004C508, fn_8004C5B8, fn_8004C84C, nml selResumeTalk,
//                          fn_8004C8A0, fn_8004C900
//   804A33F0 .data   0x10  PTMF {0,-1,fn_8004C978} (+4 padding)
//   80749A88 .sdata  0x8   "Q02_Req" (descriptor)
//   807500C8 .sdata2 0x8   "Q02_Req"
//   807500D0 .sdata2 0x4   100.0f (rndF range, fn_8004B804)
//   807500D4 .sdata2 0x4   70.0f (fn_8004B804: no talk when rndF(100) < 70 and the player is not in the quest)
//   807500D8 .sdata2 0x8   "Q02_Win"
//   807500E0 .sdata2 0x8   "Q02_Win"
//   807500E8 .sdata2 0x8   "Q02_Win"
//   807500F0 .sdata2 0x8   "Q02_Con"
//   807500F8 .sdata2 0x8   "Q02_Con"

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q02Labels[8]; // "Ai_Quest" / "Q02_*" labels of the quest steps
