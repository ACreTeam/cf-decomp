#pragma once

// Villager talk for appointment quest 8. DOL TU d_npc_talk_quest_q08.cpp (.text 8005C82C..8005E584), not
// decompiled. QUEST_KIND_APPOINTMENT_1 (kind 18, labels "Q08_*"): a villager asks to visit the player's house
// ("Q08_Req"), the player enters a time ("Q08_Reserve", time menu, "Q08_Reserved" / "Q08_Error1-3", "Q08_No");
// at that time the visit: "Q08_Call", "Q08_Door" (walk to the door), room talk in the player's house
// ("Q08_First", "Q_Roomtalk", "Q08_Furniture", "Q08_Layout"), "Q08_Wait", "Q08_Back", "Q08_Bye".

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>
#include <game/game/d_item.hpp>

// Member kinds (inferred from where each function is stored and how nml calls it):
// - int f(msgInfo_s *info): message procs (talk_c::_0EC, the d_a_npc_nml tables lbl_8046BF30 / lbl_8046BFD8
//   / lbl_804A0784, random-talk tables). info is filled by setLooksMsg (label, code); the result is used by
//   the table dispatchers and ignored for _0EC.
// - BOOL f(): step procs (_104, called by fn_8003209C; nonzero = done) and request-accepted procs.
// - void f(): _0F8 procs, choice procs (+0x214 list), action procs (_110[0]), menu procs (_128), +0x26C list procs.
// In the notes, "sets X=fn" means the function stores fn there (_0EC setMsgProc, _0F8 setHookProc,
// _104 setStepProc, _110[0] setActProc, choice setChoice / setChoiceProc, _128 direct store).
// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_quest_q08"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (not split yet; file-local statics in the .cpp later). The {0, -1, fn} records are the
// member-function pointer constants of the code above.
// .data 804A4708..804A4938:
//   804A4708 0x0C PTMF {0, -1, fn_80053224}
//   804A4714 0x0C PTMF {0, -1, fn_8005C950}
//   804A4720 0x0C PTMF {0, -1, fn_8005C9C8}
//   804A472C 0x0C PTMF {0, -1, fn_8005CA28}
//   804A4738 0x0C PTMF {0, -1, fn_8005CB28}
//   804A4744 0x0C PTMF {0, -1, fn_8005D3DC}
//   804A4750 0x0C PTMF {0, -1, fn_8005CB7C}
//   804A475C 0x0C PTMF {0, -1, fn_8005CBE0}
//   804A4768 0x54 PTMF x7: fn_8005CC50 (_128 of fn_8005CBE0), then the _0EC results of the time menu:
//                 fn_8005D314, fn_8005D0CC, fn_8005D314, fn_8005D2B0, fn_8005D378, fn_8005D314
//   804A47BC 0x0C PTMF {0, -1, fn_8005D130}
//   804A47C8 0x0C PTMF {0, -1, fn_8005CBE0}
//   804A47D4 0x0C PTMF {0, -1, fn_8005CBE0}
//   804A47E0 0x0C PTMF {0, -1, fn_8005CBE0}
//   804A47EC 0x0C PTMF {0, -1, fn_8005D430}
//   804A47F8 0x0A string "Q08_Leave"
//   804A4804 0x0C labels by state {"Ai_Quest", "Q08_Leave", "Q08_Leave"} (getMsgLabel)
//   804A4810 0x0C PTMF {0, -1, fn_8005D724}
//   804A481C 0x0C PTMF {0, -1, fn_8005D9CC}
//   804A4828 0x0C PTMF {0, -1, fn_8005D8F8}
//   804A4834 0x0C PTMF {0, -1, fn_8005D978}
//   804A4840 0x0C PTMF {0, -1, fn_8005D94C}
//   804A484C 0x0C PTMF {0, -1, fn_80053224}
//   804A4858 0x0C PTMF {0, -1, fn_8005DAA8}
//   804A4864 0x0C PTMF {0, -1, fn_8005DCE4}
//   804A4870 0x0C PTMF {0, -1, fn_8005DB48}
//   804A487C 0x0C PTMF {0, -1, fn_8005DB88}
//   804A4888 0x0C PTMF {0, -1, fn_8005DC38}
//   804A4894 0x0B string "Q_Roomtalk"
//   804A48A0 0x0C wchar_t[6] (0x2606 x4)
//   804A48AC 0x0C wchar_t[6] (0x2606 x5)
//   804A48B8 0x14 wchar_t *[5] {lbl_80749AC0, lbl_80749AC4, lbl_80749AD0, lbl_804A48A0, lbl_804A48AC}: words of fn_8005DFA0
//   804A48CC 0x0C PTMF {0, -1, fn_8005E2BC}
//   804A48D8 0x0C PTMF {0, -1, fn_80053224}
//   804A48E4 0x0C PTMF {0, -1, fn_80053224}
//   804A48F0 0x0C PTMF {0, -1, fn_8005E37C}
//   804A48FC 0x0C PTMF {0, -1, fn_8005E3F4}
//   804A4908 0x0C PTMF {0, -1, fn_8005E454}
//   804A4914 0x0C PTMF {0, -1, fn_80053224}
//   804A4920 0x0C PTMF {0, -1, fn_8005E50C}
//   804A492C 0x0C PTMF {0, -1, fn_8005E3F4}
// .rodata 8046CC60..8046CD1C:
//   8046CC60 0x0C string "Q08_Reserve"
//   8046CC6C 0x0D string "Q08_Reserved"
//   8046CC7C 0x0B string "Q08_Error1"
//   8046CC88 0x0B string "Q08_Error2"
//   8046CC94 0x0B string "Q08_Error3"
//   8046CCA0 0x09 string "Q08_Call"
//   8046CCAC 0x09 string "Q08_Door"
//   8046CCB8 0x0E string "Q08_Furniture"
//   8046CCC8 0x0B string "Q08_Layout"
//   8046CCD4 0x0A string "Q08_First"
//   8046CCE0 0x24 PTMF x3: random room talk fn_8005DD2C, fn_8005DE9C, fn_8005DFA0 (fn_8005E178)
//   8046CD04 0x09 string "Q08_Wait"
//   8046CD10 0x09 string "Q08_Back"
// .sdata2 807502A0..807502E0:
//   807502A0 0x08 string "Q08_Req"
//   807502A8 0x07 string "Q08_No"
//   807502B0 0x08 string "Q08_Con"
//   807502B8 0x04 f32 100.0 (fn_8005DD5C)
//   807502C0 0x08 f64 4503599627370496.0 (int->float magic, fn_8005DD5C)
//   807502C8 0x08 string "Q08_Bye"
//   807502D0 0x04 .float 48
//   807502D4 0x04 .float 96
//   807502D8 0x08 .4byte 0x00070007 .4byte 0x000A000A
// External: fn_80053224 (d_npc_talk_quest_delivery), lbl_804A0784 entries [1..4] (nml talk states), lbl_804A0E48,
// l_walkTargetPos (walk target), lbl_8074E9B0 (sbss pointer, fn_8005DC38).
// 807502D0..807502E0 (f32 48, f32 96, u16[4]) follow "Q08_Bye" but are not referenced here (owner unknown).

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q08Labels[3]; // "Ai_Quest" / "Q08_*" labels of the quest steps
