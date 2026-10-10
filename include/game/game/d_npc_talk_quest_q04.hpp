#pragma once

// d_npc_talk_quest_q04.cpp (.text 80049FDC..8004B75C, .rodata 8046C8B0..8046C8E0, .data
// 804A2F70..804A3290, .sdata 80749A80..80749A88, .sdata2 80750090..807500C8). The
// villager talk for the
// request quest of kind QUEST_KIND_REQUEST_CLOTH (dQuestVillager_c at dAnimal_c+0x2BE6, mBase.mKind 3;
// messages "Q04_Req" / "Q04_Win" / "Q04_NG" / "Q04_ConA" / "Q04_ConB" / "Q04_Lose1/2" of the
// "Ai_Quest" group): the npc asks the players for a shirt of some look (several players can join),
// checks the selected shirt (dAnimal_c::isClothRequestMatch; "Q04_NG" and handing it back when it does
// not match), puts it on (dAnimal_c::setCloth), and gives a reward (dAnimal_c::pickClothReward: item
// or bells). The same frame as d_npc_talk_quest_q01 (insect) with the kind / ids changed.
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
// "d_npc_talk_quest_q04"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; split in splits.txt):
//   8046C8B0 .rodata 0xC   "Q04_ConA" (+3 padding) (msgClothCon)
//   8046C8BC .rodata 0xC   "Q04_ConB" (+3 padding) (msgClothCon)
//   8046C8C8 .rodata 0xC   "Q04_ConA" (+3 padding) (msgClothCon2)
//   8046C8D4 .rodata 0xC   "Q04_ConB" (+3 padding) (msgClothCon2)
//   804A2F70 .data   0xC   PTMF {0,-1,stepClothAccept} (msgClothOffer)
//   804A2F7C .data   0xC   PTMF {0,-1,msgClothReq} (stepClothAccept)
//   804A2F88 .data   0x14  "sys_STRING/STR_Unit" (stepClothAccept; msgClothQuest as 804A2F70+0x18)
//   804A2F9C .data   0xC   PTMF {0,-1,nml stepRequestChoice} (msgClothReq)
//   804A2FA8 .data   0xC   "Q04_Lose2" (+2 padding)
//   804A2FB4 .data   0xC   "Q04_Lose1" (+2 padding)
//   804A2FC0 .data   0x40  l_q04Labels: "Ai_Quest" x6, "Q04_Lose2", "Q04_Lose1", "Ai_Quest" x6, "Q04_Req" x2
//   804A3000 .data   0x3C  the 5 PTMF constants of msgClothQuest (read as 804A2F70+0x90..0xC0):
//                          endQuestCommon (another TU), stepClothAccept, stepClothTalkChoice, stepClothOver,
//                          stepClothGiveChoice
//   804A303C..804A3114 .data 0xC each, PTMF {0,-1,fn}: selClothGive, nml selResumeTalk, resClothSelect,
//                          resClothMatch, resClothMismatch, nml msgCancel (804A3054 is one 0x30 symbol for four of
//                          them; resClothSelect reads them as 804A2F70+0xF0..0x108), msgClothWin, endClothReward,
//                          stepClothWin, msgClothWin2, resClothWear, stepClothWin2, msgClothWin3, msgClothNG,
//                          stepClothNG, selClothCon, nml selResumeTalk, msgClothCon, stepClothCon
//   804A3120 .data   0xC   PTMF {0,-1,msgClothCon2}; the symbol is 0x170 and runs on to 804A3290:
//                          804A312C..804A3208 zeros, then weak RTTI data: "dItem::nameLook_c", the base
//                          lists {dScript::Word_c, dString::WordBase_c, dString::Word_c},
//                          "dString::Word_c", {dScript::Word_c, dString::WordBase_c},
//                          "dString::WordBase_c", {dScript::Word_c}, "dScript::Word_c" (from the
//                          dItem::nameLook_c used by msgClothNG; vtable at nml 804A1960)
//   80749A80 .sdata  0x8   "Q04_Req" (l_q04Labels)
//   80750090 .sdata2 0x8   "Q04_Req"
//   80750098 .sdata2 0x4   3.0f (rndF range of the message code, msgClothReq)
//   8075009C .sdata2 0x4   100.0f (rndF range, msgClothQuest)
//   807500A0 .sdata2 0x8   70.0f (+4 padding) (msgClothQuest: no talk when rndF(100) < 70 and the player is
//                          not in the quest)
//   807500A8 .sdata2 0x8   "Q04_Win"
//   807500B0 .sdata2 0x8   "Q04_Win"
//   807500B8 .sdata2 0x8   "Q04_Win"
//   807500C0 .sdata2 0x8   "Q04_NG" (+1 padding)

// Message label table of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (global).
extern const char *l_q04Labels[16]; // "Ai_Quest" / "Q04_*" labels of the quest steps
