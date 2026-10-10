#pragma once

// Villager talk for errand quest 6. DOL TU d_npc_talk_quest_q06.cpp (.text 80056024..800573D0), not decompiled.
// QUEST_KIND_ERRAND_REQUEST (kind 7, labels "Q06_*"): a villager asks the player to take an item to another
// villager ("Q06_Req", "Q_Yes"); the recipient takes it and pays ("Q06_Report", "Q_Item" / "Q_ItemFull",
// "Q06_End"); "Q06_Con" while the errand is open.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>
#include <game/game/d_scene.hpp>

// Member kinds (inferred from where each function is stored and how nml calls it):
// - int f(msgInfo_s *info): message procs (talk_c::_0EC, the d_a_npc_nml tables lbl_8046BF30 / lbl_8046BFD8
//   / lbl_804A0784, random-talk tables). info is filled by setLooksMsg (label, code); the result is used by
//   the table dispatchers and ignored for _0EC.
// - BOOL f(): step procs (_104, called by fn_8003209C; nonzero = done) and request-accepted procs.
// - void f(): _0F8 procs, choice procs (+0x214 list), action procs (_110[0]), menu procs (_128), +0x26C list procs.
// In the notes, "sets X=fn" means the function stores fn there (_0EC setMsgProc, _0F8 setHookProc,
// _104 setStepProc, _110[0] setActProc, choice setChoice / setChoiceProc, _128 direct store).
// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_q06"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (not split yet; file-local statics in the .cpp later). The {0, -1, fn} records are the
// member-function pointer constants of the code above.
// .data 804A3F88..804A40F8:
//   804A3F88 0x0C PTMF {0, -1, fn_8005606C}
//   804A3F94 0x0C PTMF {0, -1, fn_800560E4}
//   804A3FA0 0x0C PTMF {0, -1, fn_80056144}
//   804A3FAC 0x0C PTMF {0, -1, fn_80056244}
//   804A3FB8 0x0C PTMF {0, -1, selNo}
//   804A3FC4 0x0C PTMF {0, -1, fn_80056298}
//   804A3FD0 0x0C PTMF {0, -1, fn_80056324}
//   804A3FDC 0x0C PTMF {0, -1, fn_80056534}
//   804A3FE8 0x14 string "sys_STRING/STR_Unit"
//   804A3FFC 0x0C PTMF {0, -1, msgYes}
//   804A4008 0x3C labels by state {"Ai_Quest", "Q_Timeover", "Q_Timeover", "Ai_Quest" x3} (read by nml getMsgLabel(6, 0, state)),
//                 then PTMF x3 by state: fn_80057170 (open), stepErrandOver (time over), fn_80056928 (done)
//   804A4044 0x0C PTMF {0, -1, fn_80056AFC}
//   804A4050 0x0C PTMF {0, -1, selResumeTalk}
//   804A405C 0x0C PTMF {0, -1, fn_80056BCC}
//   804A4068 0x0C PTMF {0, -1, msgTimeover2}
//   804A4074 0x0C PTMF {0, -1, fn_80056C30}
//   804A4080 0x0C PTMF {0, -1, fn_80057000}
//   804A408C 0x0C PTMF {0, -1, fn_80056D14}
//   804A4098 0x0C PTMF {0, -1, fn_80056DA0}
//   804A40A4 0x0C PTMF {0, -1, fn_80056EF4}
//   804A40B0 0x0C PTMF {0, -1, fn_80056FD4}
//   804A40BC 0x0C PTMF {0, -1, fn_80057064}
//   804A40C8 0x0C PTMF {0, -1, fn_80056FD4}
//   804A40D4 0x0C PTMF {0, -1, fn_80057344}
//   804A40E0 0x0C PTMF {0, -1, selResumeTalk}
//   804A40EC 0x0C PTMF {0, -1, fn_80057398}
// .rodata 8046CA00..8046CA18:
//   8046CA00 0x0C s32[3] {0, 1, 2}: kinds for dQuestBase_c::pickKind
//   8046CA0C 0x0B string "Q06_Report"
// .sdata2 80750208..80750220:
//   80750208 0x08 string "Q06_Req"
//   80750210 0x08 string "Q06_End"
//   80750218 0x08 string "Q06_Con"
// External: "Q_Yes" 8074FF30, "Q_Item" 8074FF40 (sdata2), "Q_ItemFull" 8046BD6C, lbl_804A0E48 (nml talk states),
// lbl_8059FF80 (d_animal .bss, item filter for fn_800F4608), gSceneChange.

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q06Labels[6]; // "Ai_Quest" / "Q06_*" labels of the quest steps
