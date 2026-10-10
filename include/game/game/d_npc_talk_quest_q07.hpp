#pragma once

// Villager talk for errand quest 7. DOL TU d_npc_talk_quest_q07.cpp (.text 800546F4..80056024), not decompiled.
// QUEST_KIND_ERRAND_REQUEST_FINAL (kind 8, labels "Q07_*"): like quest 6 ("Q07_Req", "Q07_Report",
// "Q07_End", "Q07_Con"), plus a 3-way answer on delivery ("Q07_Good" / "Q07_Normal" / "Q07_Bad") and the
// final errand reward.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// Member kinds (inferred from where each function is stored and how nml calls it):
// - int f(msgInfo_s *info): message procs (talk_c::_0EC, the d_a_npc_nml tables lbl_8046BF30 / lbl_8046BFD8
//   / lbl_804A0784, random-talk tables). info is filled by setLooksMsg (label, code); the result is used by
//   the table dispatchers and ignored for _0EC.
// - BOOL f(): step procs (_104, called by fn_8003209C; nonzero = done) and request-accepted procs.
// - void f(): _0F8 procs, choice procs (+0x214 list), action procs (_110[0]), menu procs (_128), +0x26C list procs.
// In the notes, "sets X=fn" means the function stores fn there (_0EC setMsgProc, _0F8 setHookProc,
// _104 setStepProc, _110[0] setActProc, choice setChoice / setChoiceProc, _128 direct store).
// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_q07"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (not split yet; file-local statics in the .cpp later). The {0, -1, fn} records are the
// member-function pointer constants of the code above.
// .data 804A3DA8..804A3F88:
//   804A3DA8 0x0C PTMF {0, -1, fn_8005473C}
//   804A3DB4 0x0C PTMF {0, -1, fn_800547B4}
//   804A3DC0 0x0C PTMF {0, -1, fn_80054814}
//   804A3DCC 0x0C PTMF {0, -1, fn_80054914}
//   804A3DD8 0x0C PTMF {0, -1, selNo}
//   804A3DE4 0x0C PTMF {0, -1, fn_80054968}
//   804A3DF0 0x0C PTMF {0, -1, fn_800549F4}
//   804A3DFC 0x0C PTMF {0, -1, fn_80054C48}
//   804A3E08 0x14 string "sys_STRING/STR_Unit"
//   804A3E1C 0x0C PTMF {0, -1, msgYes}
//   804A3E28 0x3C labels by state {"Ai_Quest", "Q_Timeover", "Q_Timeover", "Ai_Quest" x3} (read by nml getMsgLabel),
//                 then PTMF x3 by state: fn_80055D4C (open), stepErrandOver (time over), fn_800550B0 (done)
//   804A3E64 0x0C PTMF {0, -1, fn_80055284}
//   804A3E70 0x0C PTMF {0, -1, fn_80055CF8}
//   804A3E7C 0x0C PTMF {0, -1, msgTimeover2}
//   804A3E88 0x0C PTMF {0, -1, fn_80055348}
//   804A3E94 0x30 PTMF x4: fn_800553AC (_104 of Q07_Report), fn_800554C8, fn_8005551C, fn_80055570 (the 3 answers)
//   804A3EC4 0x0C PTMF {0, -1, fn_800555C4}
//   804A3ED0 0x0C PTMF {0, -1, fn_80055634}
//   804A3EDC 0x0C PTMF {0, -1, fn_800556A4}
//   804A3EE8 0x0C PTMF {0, -1, fn_80055710}
//   804A3EF4 0x0C PTMF {0, -1, fn_80055710}
//   804A3F00 0x0C PTMF {0, -1, fn_80055710}
//   804A3F0C 0x0C PTMF {0, -1, fn_80055B68}
//   804A3F18 0x0C PTMF {0, -1, fn_800557E8}
//   804A3F24 0x0C PTMF {0, -1, fn_80055874}
//   804A3F30 0x0C PTMF {0, -1, fn_80055A5C}
//   804A3F3C 0x0C PTMF {0, -1, fn_80055B3C}
//   804A3F48 0x0C PTMF {0, -1, fn_80055BCC}
//   804A3F54 0x0C PTMF {0, -1, fn_80055B3C}
//   804A3F60 0x0C PTMF {0, -1, fn_80055F20}
//   804A3F6C 0x0C PTMF {0, -1, fn_80055CF8}
//   804A3F78 0x10 PTMF {0, -1, fn_80055F74} + 4 bytes of padding
// .rodata 8046C9D0..8046CA00:
//   8046C9D0 0x0C s32[3] {0, 1, 2}: kinds for dQuestBase_c::pickKind
//   8046C9DC 0x0B string "Q07_Report"
//   8046C9E8 0x09 string "Q07_Good"
//   8046C9F4 0x0B string "Q07_Normal"
// .sdata2 807501D8..80750208:
//   807501D8 0x08 string "Q07_Req"
//   807501E0 0x08 string "Q07_Bad"
//   807501E8 0x08 u8[8] {0, 2, 1, 0, ...}: argument table of pickErrandFinalReward (fn_80055874)
//   807501F0 0x08 string "Q07_End"
//   807501F8 0x08 u8[8] {0, 2, 1, 0, ...}: argument table of completeErrandRequestFinal (fn_80055BCC)
//   80750200 0x08 string "Q07_Con"
// External: "Q_Yes" 8074FF30, "Q_Item" 8074FF40 (sdata2), "Q_ItemFull" 8046BD6C, lbl_804A0E48 (nml talk states).

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q07Labels[6]; // "Ai_Quest" / "Q07_*" labels of the quest steps
