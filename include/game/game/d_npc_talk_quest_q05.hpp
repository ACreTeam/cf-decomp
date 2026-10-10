#pragma once

// d_npc_talk_quest_q05.cpp (.text 8004E31C..8004FADC; its data is still in the unsplit auto_07_804A2A90
// .data / auto_11 .sdata / .sdata2). Not decompiled yet. The villager talk for the request quest of
// kind QUEST_KIND_REQUEST_FTR (dQuestVillager_c at dAnimal_c+0x2BE6, mBase.mKind 4; messages
// "Q05_Req" / "Q05_Win" / "Q05_NG" / "Q05_Con" / "Q05_Lose1/2" / "Q05_Return" / "Q05_Comp" of the
// "Ai_Quest" group): the npc asks the players for a piece of furniture (several players can join),
// checks the selected item (dAnimal_c::checkFtrRequest; "Q05_NG" and handing it back when it does not
// match), keeps it, gives a reward (dAnimal_c::pickFtrReward: item or bells), and has two more steps
// for the winner (quest state 2 "Return", 4 "Comp"). The same frame as d_npc_talk_quest_q01 (insect)
// with the kind / ids changed, the item check of d_npc_talk_quest_q04 and the extra steps of
// d_npc_talk_quest_q03.
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
// "d_npc_talk_quest_q05"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; not split yet):
//   804A35A8 .data   0xC   PTMF {0,-1,fn_8004F930} (fn_8004E31C)
//   804A35B4 .data   0xC   PTMF {0,-1,fn_8004E364} (fn_8004F930)
//   804A35C0 .data   0x14  "sys_STRING/STR_Unit" (fn_8004F930; fn_8004E3C4 as 804A35A8+0x18)
//   804A35D4 .data   0xC   PTMF {0,-1,nml stepRequestChoice} (fn_8004E364)
//   804A35E0 .data   0xC   "Q05_Lose2" (+2 padding)
//   804A35EC .data   0xC   "Q05_Lose1" (+2 padding)
//   804A35F8 .data   0xC   "Q05_Return" (+1 padding)
//   804A3604 .data   0xC   "Q05_Comp" (+3 padding)
//   804A3610 .data   0x88  quest descriptor (nml 80033230 returns it for the kind; fn_8004E3C4 reads its
//                          PTMFs as 804A35A8+0x9C..0xE4): 13 labels "Ai_Quest" x3, "Q05_Lose2", "Q05_Lose1",
//                          "Ai_Quest" x4, "Q05_Req", "Q05_Req", "Q05_Return", "Q05_Comp"; 7 PTMFs fn_80053224
//                          (another TU), fn_8004F930, fn_8004F4D8, fn_8004F444, fn_8004E990, fn_8004F868,
//                          fn_8004F8DC
//   804A3698..804A3764 .data 0xC each, PTMF {0,-1,fn}: fn_8004EB64, nml selResumeTalk, fn_8004EBF0,
//                          fn_8004ED80, fn_8004F2E4, nml msgCancel (804A36B0 is one 0x30 symbol for four of
//                          them; fn_8004EBF0 reads them as 804A35A8+0x114..0x12C), fn_8004EDDC, fn_8004EE64,
//                          fn_8004F000, fn_8004F1A8, fn_8004F208, fn_8004F2B8, fn_8004F340, fn_8004F400,
//                          fn_8004F6AC, nml selResumeTalk, fn_8004F700, fn_8004F760
//   804A3770 .data   0x10  PTMF {0,-1,fn_8004F7D8} (+4 padding)
//   80749A98 .sdata  0x8   "Q05_Req" (descriptor)
//   80750140 .sdata2 0x8   "Q05_Req"
//   80750148 .sdata2 0x4   100.0f (rndF range, fn_8004E3C4)
//   8075014C .sdata2 0x4   70.0f (fn_8004E3C4: no talk when rndF(100) < 70 and the player is not in the quest)
//   80750150 .sdata2 0x8   "Q05_Win"
//   80750158 .sdata2 0x8   "Q05_Win"
//   80750160 .sdata2 0x8   "Q05_Win"
//   80750168 .sdata2 0x8   "Q05_NG" (+1 padding)
//   80750170 .sdata2 0x8   "Q05_Con"
//   80750178 .sdata2 0x8   "Q05_Con"

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q05Labels[13]; // "Ai_Quest" / "Q05_*" labels of the quest steps
