#pragma once

// d_npc_talk_quest_q03.cpp (.text 8004CBCC..8004E31C; its data is still in the unsplit auto_07_804A2A90
// .data / auto_11 .sdata / .sdata2). Not decompiled yet. The villager talk for the request quest of
// kind QUEST_KIND_REQUEST_FOSSIL (dQuestVillager_c at dAnimal_c+0x2BE6, mBase.mKind 2; messages
// "Q03_Req" / "Q03_Win" / "Q03_Con" / "Q03_Lose1/2" / "Q03_Return" / "Q03_Comp" of the "Ai_Quest"
// group): the npc asks the players for a fossil (several players can join), takes it from the item
// select, gives a reward (dAnimal_c::pickFossilReward: item or bells), and has two more steps for
// the winner (quest state 2 "Return", 4 "Comp"). The same code as d_npc_talk_quest_q01 (insect) with
// the kind / ids changed, plus those two steps and the dAnimalMemory_c present.
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
// "d_npc_talk_quest_q03"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; not split yet):
//   804A3400 .data   0xC   PTMF {0,-1,fn_8004E150} (fn_8004CBCC)
//   804A340C .data   0xC   PTMF {0,-1,fn_8004CC14} (fn_8004E150)
//   804A3418 .data   0x14  "sys_STRING/STR_Unit" (fn_8004E150; fn_8004CCC8 as 804A3400+0x18)
//   804A342C .data   0xC   PTMF {0,-1,nml stepRequestChoice} (fn_8004CC14)
//   804A3438 .data   0xC   "Q03_Lose2" (+2 padding)
//   804A3444 .data   0xC   "Q03_Lose1" (+2 padding)
//   804A3450 .data   0xC   "Q03_Return" (+1 padding)
//   804A345C .data   0xC   "Q03_Comp" (+3 padding)
//   804A3468 .data   0x7C  quest descriptor (nml 80033230 returns it for the kind; fn_8004CCC8 reads its
//                          PTMFs as 804A3400+0x90..0xD8): 10 labels "Ai_Quest", "Ai_Quest", "Ai_Quest",
//                          "Q03_Lose2", "Q03_Lose1", "Ai_Quest", "Q03_Req", "Q03_Req", "Q03_Return",
//                          "Q03_Comp"; 7 PTMFs fn_80053224 (another TU), fn_8004E150, fn_8004DC88,
//                          fn_8004DBF4, fn_8004D2D0, fn_8004E088, fn_8004E0FC
//   804A34E4..804A358C .data 0xC each, PTMF {0,-1,fn}: fn_8004D4A4, nml selResumeTalk, fn_8004D548,
//                          fn_8004D688, nml msgCancel, fn_8004D6E4, fn_8004D76C, fn_8004D910,
//                          fn_8004DAB8, fn_8004DB18, fn_8004DBC8, fn_8004DE5C, nml selResumeTalk,
//                          fn_8004DEB0, fn_8004DF70
//   804A3598 .data   0x10  PTMF {0,-1,fn_8004DFE8} (+4 padding)
//   80749A90 .sdata  0x8   "Q03_Req" (descriptor)
//   80750100 .sdata2 0x8   "Q03_Req"
//   80750108 .sdata2 0x4   3.0f (rndF range of the message code, fn_8004CC14 / fn_8004DEB0)
//   8075010C .sdata2 0x4   100.0f (rndF range, fn_8004CCC8)
//   80750110 .sdata2 0x8   70.0f (+4 padding) (fn_8004CCC8: no talk when rndF(100) < 70 and the player is
//                          not in the quest)
//   80750118 .sdata2 0x8   "Q03_Win"
//   80750120 .sdata2 0x8   "Q03_Win"
//   80750128 .sdata2 0x8   "Q03_Win"
//   80750130 .sdata2 0x8   "Q03_Con"
//   80750138 .sdata2 0x8   "Q03_Con"

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q03Labels[10]; // "Ai_Quest" / "Q03_*" labels of the quest steps
