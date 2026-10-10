#pragma once

// Villager talk for errand quest 7. DOL TU d_npc_talk_quest_q07.cpp (.text 800546F4..80056024, .rodata
// 8046C9D0..8046CA00, .data 804A3DA8..804A3F88, .sdata2 807501D8..80750208), decompiled (src/dol/game/d_npc_talk_quest_q07.cpp).
// QUEST_KIND_ERRAND_REQUEST_FINAL (kind 8, labels "Q07_*", topic word "ErrandFinal"): like quest 6 with
// clothing for the recipient ("Q07_Req", "Q07_Report", "Q07_End", "Q07_Con"), plus the player's 3-way answer
// on how the recipient liked it ("Q07_Good" / "Q07_Normal" / "Q07_Bad"; friendship +5 when it matches the
// errand state) and the final errand reward.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// Procedure names follow d_a_npc_nml's slots: msg* (EC message select), end* (F8 hook), step* (104 step),
// sel* (choice answer), res* (128 request result); see d_a_npc_nml.hpp and notes/d_a_npc_nml.txt.
// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_q07"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (split in splits.txt; file-local statics in the .cpp). The {0, -1, fn} records are the
// member-function pointer constants of the code above.
// .data 804A3DA8..804A3F88:
//   804A3DA8 0x0C PTMF {0, -1, stepErrandFinalOffer}
//   804A3DB4 0x0C PTMF {0, -1, msgErrandFinalReq}
//   804A3DC0 0x0C PTMF {0, -1, stepErrandFinalReq}
//   804A3DCC 0x0C PTMF {0, -1, selErrandFinalYes}
//   804A3DD8 0x0C PTMF {0, -1, selNo}
//   804A3DE4 0x0C PTMF {0, -1, msgErrandFinalYes}
//   804A3DF0 0x0C PTMF {0, -1, endErrandFinalYes}
//   804A3DFC 0x0C PTMF {0, -1, stepErrandFinalYes}
//   804A3E08 0x14 string "sys_STRING/STR_Unit"
//   804A3E1C 0x0C PTMF {0, -1, msgYes}
//   804A3E28 0x18 l_q07Labels by state {"Ai_Quest", "Q_Timeover", "Q_Timeover", "Ai_Quest" x3} (read by nml getMsgLabel),
//   804A3E40 0x24 PTMF x3 (constants of msgErrandFinalQuest, not a table): stepErrandFinalChoice (open), stepErrandOver (time over), stepErrandFinalReportChoice (done)
//   804A3E64 0x0C PTMF {0, -1, selErrandFinalReport}
//   804A3E70 0x0C PTMF {0, -1, selErrandFinalResume}
//   804A3E7C 0x0C PTMF {0, -1, msgTimeover2}
//   804A3E88 0x0C PTMF {0, -1, msgErrandFinalReport}
//   804A3E94 0x30 PTMF x4 (separate constants): stepErrandFinalReport (_104 of Q07_Report), selErrandFinalGood, selErrandFinalNormal, selErrandFinalBad (the 3 answers)
//   804A3EC4 0x0C PTMF {0, -1, msgErrandFinalGood}
//   804A3ED0 0x0C PTMF {0, -1, msgErrandFinalNormal}
//   804A3EDC 0x0C PTMF {0, -1, msgErrandFinalBad}
//   804A3EE8 0x0C PTMF {0, -1, stepErrandFinalAnswer}
//   804A3EF4 0x0C PTMF {0, -1, stepErrandFinalAnswer}
//   804A3F00 0x0C PTMF {0, -1, stepErrandFinalAnswer}
//   804A3F0C 0x0C PTMF {0, -1, msgErrandFinalRewardFull}
//   804A3F18 0x0C PTMF {0, -1, msgErrandFinalReward}
//   804A3F24 0x0C PTMF {0, -1, endErrandFinalReward}
//   804A3F30 0x0C PTMF {0, -1, stepErrandFinalReward}
//   804A3F3C 0x0C PTMF {0, -1, msgErrandFinalEnd}
//   804A3F48 0x0C PTMF {0, -1, stepErrandFinalRewardFull}
//   804A3F54 0x0C PTMF {0, -1, msgErrandFinalEnd}
//   804A3F60 0x0C PTMF {0, -1, selErrandFinalCon}
//   804A3F6C 0x0C PTMF {0, -1, selErrandFinalResume}
//   804A3F78 0x10 PTMF {0, -1, msgErrandFinalCon} + 4 bytes of padding
// .rodata 8046C9D0..8046CA00:
//   8046C9D0 0x0C s32[3] {0, 1, 2}: deadlines for dQuestBase_c::pickDeadline
//   8046C9DC 0x0B string "Q07_Report"
//   8046C9E8 0x09 string "Q07_Good"
//   8046C9F4 0x0B string "Q07_Normal"
// .sdata2 807501D8..80750208:
//   807501D8 0x08 string "Q07_Req"
//   807501E0 0x08 string "Q07_Bad"
//   807501E8 0x08 u8[8] {0, 2, 1, 0, ...}: argument table of pickErrandFinalReward (endErrandFinalReward)
//   807501F0 0x08 string "Q07_End"
//   807501F8 0x08 u8[8] {0, 2, 1, 0, ...}: argument table of completeErrandRequestFinal (stepErrandFinalRewardFull)
//   80750200 0x08 string "Q07_Con"
// External: "Q_Yes" 8074FF30, "Q_Item" 8074FF40 (sdata2), "Q_ItemFull" 8046BD6C, l_talkEntrySets (nml).

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q07Labels[6]; // "Ai_Quest" / "Q07_*" labels of the quest steps
