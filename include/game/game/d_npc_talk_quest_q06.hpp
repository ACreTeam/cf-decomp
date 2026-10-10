#pragma once

// Villager talk for errand quest 6. DOL TU d_npc_talk_quest_q06.cpp (.text 80056024..800573D0, .rodata
// 8046CA00..8046CA18, .data 804A3F88..804A40F8, .sdata2 80750208..80750220), decompiled (src/dol/game/d_npc_talk_quest_q06.cpp).
// QUEST_KIND_ERRAND_REQUEST (kind 7, labels "Q06_*", topic word "Errand"): a villager (dQuestErrand_c animal 0)
// asks the player to take an item to another villager (animal 1) ("Q06_Req", "Q_Yes"); the recipient takes it
// (d_npc_talk_quest_delivery), then the requester hears the report and pays ("Q06_Report", "Q_Item" /
// "Q_ItemFull", "Q06_End"); "Q06_Con" while the errand is open. Also holds getSceneChange (used by q10).

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>
#include <game/game/d_scene.hpp>

// Procedure names follow d_a_npc_nml's slots: msg* (EC message select), end* (F8 hook), step* (104 step),
// sel* (choice answer), res* (128 request result); see d_a_npc_nml.hpp and notes/d_a_npc_nml.txt.
// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_q06"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (split in splits.txt; file-local statics in the .cpp). The {0, -1, fn} records are the
// member-function pointer constants of the code above.
// .data 804A3F88..804A40F8:
//   804A3F88 0x0C PTMF {0, -1, stepErrandOffer}
//   804A3F94 0x0C PTMF {0, -1, msgErrandReq}
//   804A3FA0 0x0C PTMF {0, -1, stepErrandReq}
//   804A3FAC 0x0C PTMF {0, -1, selErrandYes}
//   804A3FB8 0x0C PTMF {0, -1, selNo}
//   804A3FC4 0x0C PTMF {0, -1, msgErrandYes}
//   804A3FD0 0x0C PTMF {0, -1, endErrandYes}
//   804A3FDC 0x0C PTMF {0, -1, stepErrandYes}
//   804A3FE8 0x14 string "sys_STRING/STR_Unit"
//   804A3FFC 0x0C PTMF {0, -1, msgYes}
//   804A4008 0x18 l_q06Labels by state {"Ai_Quest", "Q_Timeover", "Q_Timeover", "Ai_Quest" x3} (read by nml getMsgLabel(6, 0, state)),
//   804A4020 0x24 PTMF x3 (constants of msgErrandQuest, not a table): stepErrandChoice (open), stepErrandOver (time over), stepErrandReportChoice (done)
//   804A4044 0x0C PTMF {0, -1, selErrandReport}
//   804A4050 0x0C PTMF {0, -1, selResumeTalk}
//   804A405C 0x0C PTMF {0, -1, msgErrandReport}
//   804A4068 0x0C PTMF {0, -1, msgTimeover2}
//   804A4074 0x0C PTMF {0, -1, stepErrandReport}
//   804A4080 0x0C PTMF {0, -1, msgErrandRewardFull}
//   804A408C 0x0C PTMF {0, -1, msgErrandReward}
//   804A4098 0x0C PTMF {0, -1, endErrandReward}
//   804A40A4 0x0C PTMF {0, -1, stepErrandReward}
//   804A40B0 0x0C PTMF {0, -1, msgErrandEnd}
//   804A40BC 0x0C PTMF {0, -1, stepErrandRewardFull}
//   804A40C8 0x0C PTMF {0, -1, msgErrandEnd}
//   804A40D4 0x0C PTMF {0, -1, selErrandCon}
//   804A40E0 0x0C PTMF {0, -1, selResumeTalk}
//   804A40EC 0x0C PTMF {0, -1, msgErrandCon}
// .rodata 8046CA00..8046CA18:
//   8046CA00 0x0C s32[3] {0, 1, 2}: deadlines for dQuestBase_c::pickDeadline
//   8046CA0C 0x0B string "Q06_Report"
// .sdata2 80750208..80750220:
//   80750208 0x08 string "Q06_Req"
//   80750210 0x08 string "Q06_End"
//   80750218 0x08 string "Q06_Con"
// External: "Q_Yes" 8074FF30, "Q_Item" 8074FF40 (sdata2), "Q_ItemFull" 8046BD6C, l_talkEntrySets (nml),
// lbl_8059FF80 (d_item_sel .bss, item filter for fn_800F4608), gSceneChange.

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q06Labels[6]; // "Ai_Quest" / "Q06_*" labels of the quest steps
