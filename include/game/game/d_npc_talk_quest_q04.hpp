#pragma once

// d_npc_talk_quest_q04.cpp (.text 80049FDC..8004B75C; its data is still in the unsplit auto_06 .rodata /
// auto_07_804A2A90 .data / auto_11 .sdata / .sdata2). Not decompiled yet. The villager talk for the
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

// Data of the TU (file-local statics in the .cpp; not split yet):
//   8046C8B0 .rodata 0xC   "Q04_ConA" (+3 padding) (fn_8004B3D8)
//   8046C8BC .rodata 0xC   "Q04_ConB" (+3 padding) (fn_8004B3D8)
//   8046C8C8 .rodata 0xC   "Q04_ConA" (+3 padding) (fn_8004B4F4)
//   8046C8D4 .rodata 0xC   "Q04_ConB" (+3 padding) (fn_8004B4F4)
//   804A2F70 .data   0xC   PTMF {0,-1,fn_8004B5A8} (fn_80049FDC)
//   804A2F7C .data   0xC   PTMF {0,-1,fn_8004A024} (fn_8004B5A8)
//   804A2F88 .data   0x14  "sys_STRING/STR_Unit" (fn_8004B5A8; fn_8004A0D8 as 804A2F70+0x18)
//   804A2F9C .data   0xC   PTMF {0,-1,nml stepRequestChoice} (fn_8004A024)
//   804A2FA8 .data   0xC   "Q04_Lose2" (+2 padding)
//   804A2FB4 .data   0xC   "Q04_Lose1" (+2 padding)
//   804A2FC0 .data   0x7C  quest descriptor (nml 80033230 returns it for the kind; fn_8004A0D8 reads its
//                          PTMFs as 804A2F70+0x90..0xC0): 16 labels "Ai_Quest" x6, "Q04_Lose2", "Q04_Lose1",
//                          "Ai_Quest" x6, "Q04_Req", "Q04_Req"; 5 PTMFs fn_80053224 (another TU),
//                          fn_8004B5A8, fn_8004B1B0, fn_8004B11C, fn_8004A5F8
//   804A303C..804A3114 .data 0xC each, PTMF {0,-1,fn}: fn_8004A7CC, nml selResumeTalk, fn_8004A858,
//                          fn_8004A9E8, fn_8004AF30, nml msgCancel (804A3054 is one 0x30 symbol for four of
//                          them; fn_8004A858 reads them as 804A2F70+0xF0..0x108), fn_8004AA44, fn_8004AACC,
//                          fn_8004AC68, fn_8004ADF4, fn_8004ADBC, fn_8004AE54, fn_8004AF04, fn_8004AF8C,
//                          fn_8004B0D8, fn_8004B384, nml selResumeTalk, fn_8004B3D8, fn_8004B47C
//   804A3120 .data   0xC   PTMF {0,-1,fn_8004B4F4}; the symbol is 0x170 and runs on to 804A3290:
//                          804A312C..804A3208 zeros, then weak RTTI data: "dItem::nameLook_c", the base
//                          lists {dScript::Word_c, dString::WordBase_c, dString::Word_c},
//                          "dString::Word_c", {dScript::Word_c, dString::WordBase_c},
//                          "dString::WordBase_c", {dScript::Word_c}, "dScript::Word_c" (from the
//                          dItem::nameLook_c used by fn_8004AF8C; vtable at nml 804A1960)
//   80749A80 .sdata  0x8   "Q04_Req" (descriptor)
//   80750090 .sdata2 0x8   "Q04_Req"
//   80750098 .sdata2 0x4   3.0f (rndF range of the message code, fn_8004A024)
//   8075009C .sdata2 0x4   100.0f (rndF range, fn_8004A0D8)
//   807500A0 .sdata2 0x8   70.0f (+4 padding) (fn_8004A0D8: no talk when rndF(100) < 70 and the player is
//                          not in the quest)
//   807500A8 .sdata2 0x8   "Q04_Win"
//   807500B0 .sdata2 0x8   "Q04_Win"
//   807500B8 .sdata2 0x8   "Q04_Win"
//   807500C0 .sdata2 0x8   "Q04_NG" (+1 padding)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q04Labels[16]; // "Ai_Quest" / "Q04_*" labels of the quest steps
