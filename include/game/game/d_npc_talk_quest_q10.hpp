#pragma once

// Villager talk for hide and seek. DOL TU d_npc_talk_quest_q10.cpp (.text 800573D0..80058990, .rodata
// 8046CA18..8046CA90, .data 804A40F8..804A4268, .sdata 80749AB0..80749AB8, .sdata2 80750220..80750248),
// decompiled (src/dol/game/d_npc_talk_quest_q10.cpp).
// QUEST_KIND_HIDE_AND_SEEK ("Q10_*" messages, topic word "Hide"; dQuestPlayerAnimal_c at dSaveData_c+0x1E2FE):
// the offer (not in rain/snow, 6:00..21:59), the explanation and the game start, the hiders' "found" /
// "wait" / "time over" talks, the reward and the losing end. Friendship of unfound hiders drops when
// the game is lost (addHiderFriendship).
//
// Procedure names follow d_a_npc_nml's slots: msg* (EC message select), end* (F8 hook), step* (104 step),
// sel* (choice answer), res* (128 request result); see d_a_npc_nml.hpp and notes/d_a_npc_nml.txt.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_q10"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046CA18 .rodata 0x3C  "Q10_Explain", "Q10_Find", "Q10_Over", "Q10_Wait", "Q10_Visit" (5 char arrays, 4-aligned)
//   8046CA54 .rodata 0xD   "Q10_Continue"
//   8046CA64 .rodata 0x9   "Q10_Item"
//   8046CA70 .rodata 0xD   "Q10_ItemFull"
//   8046CA80 .rodata 0x9   "Q10_Lose" (+ pad to 8046CA90)
//   804A40F8..804A4170 .data 0xC each, PTMF {0,-1,fn}: endQuestCommon, stepHideOffer, msgHideReq, stepHideReq,
//                          selHideYes, selHideNo, msgHideOK, endHideOK, stepHideOK, resHideSetup,
//                          msgHideNo
//   804A417C .data   0xB   "Q10_Badend" (label of msgHideQuest, via the 80749AB0 table)
//   804A4188 .data   0xC   PTMF {0,-1,stepHideBadend}
//   804A4194 .data   0xC   PTMF {0,-1,endQuestCommon}
//   804A41A0 .data   0xC   PTMF {0,-1,stepHideExplain}
//   804A41AC .data   0x50  PTMF x5: resHideStart, stepHideOver, stepHideOver, stepHideFind, endQuestCommon; then
//                          "sys_STRING/STR_Unit" (separate constants and literal)
//   804A41FC .data   0xC   PTMF {0,-1,resHideEnd}
//   804A4208 .data   0xC   PTMF {0,-1,msgHideContinue}
//   804A4214 .data   0x30  PTMF x4: resHideEnd, stepHideReward, stepHideRewardFull, endHideReward
//   804A4244 .data   0xC   PTMF {0,-1,msgHideEnd}
//   804A4250 .data   0xC   PTMF {0,-1,msgHideEnd}
//   804A425C .data   0xC   PTMF {0,-1,endHideLose}
//   80749AB0 .sdata  0x8   label table {"Q10_Badend", NULL}? (read by nml getMsgLabel; probably this TU's)
//   80750220 .sdata2 0x8   "Q10_Req"
//   80750228 .sdata2 0x7   "Q10_OK"
//   80750230 .sdata2 0x8   "Q10_No"
//   80750238 .sdata2 0x8   0.0f
//   80750240 .sdata2 0x8   "Q10_End"
// Not this TU's: lbl_8074EBE8 (.sbss, weather manager, d_weather.hpp; msgHideOffer reads its _5884).

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q10Labels[2]; // "Q10_Badend" (.sdata)
