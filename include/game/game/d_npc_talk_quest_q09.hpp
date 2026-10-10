#pragma once

// Villager talk for appointment quest 9. DOL TU d_npc_talk_quest_q09.cpp (.text 8005A3C4..8005C82C), not
// decompiled. QUEST_KIND_APPOINTMENT_0 (kind 17, labels "Q09_*"): a villager invites the player to their house
// ("Q09_Req"), the player enters a time ("Q09_Reserve", time menu, "Q09_Reserved" / "Q09_Error1-3", "Q09_No");
// then in the villager's house: "Q09_Welcome", room talk ("Q09_First", "Q_Roomtalk", "Q09_Furniture"), a
// furniture trade ("Q09_Trade1-4", "Q09_TradeYes" / "Q09_TradeNo"), a present ("Q09_Analog"), "Q09_Wait",
// "Q09_Bye".

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
// "d_npc_talk_quest_q09"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (not split yet; file-local statics in the .cpp later). The {0, -1, fn} records are the
// member-function pointer constants of the code above.
// .data 804A4498..804A4708:
//   804A4498 0x0C PTMF {0, -1, fn_80053224}
//   804A44A4 0x0C PTMF {0, -1, fn_8005A4E8}
//   804A44B0 0x0C PTMF {0, -1, fn_8005A560}
//   804A44BC 0x0C PTMF {0, -1, fn_8005A5C0}
//   804A44C8 0x0C PTMF {0, -1, fn_8005A6C0}
//   804A44D4 0x0C PTMF {0, -1, fn_8005AF74}
//   804A44E0 0x0C PTMF {0, -1, fn_8005A714}
//   804A44EC 0x0C PTMF {0, -1, fn_8005A778}
//   804A44F8 0x54 PTMF x7: fn_8005A7E8 (_128 of fn_8005A778), then the _0EC results of the time menu:
//                 fn_8005AEAC, fn_8005AC64, fn_8005AEAC, fn_8005AE48, fn_8005AF10, fn_8005AEAC
//   804A454C 0x0C PTMF {0, -1, fn_8005ACC8}
//   804A4558 0x0C PTMF {0, -1, fn_8005A778}
//   804A4564 0x0C PTMF {0, -1, fn_8005A778}
//   804A4570 0x0C PTMF {0, -1, fn_8005A778}
//   804A457C 0x0C PTMF {0, -1, fn_8005AFC8}
//   804A4588 0x0A string "Q09_Leave"
//   804A4598 0x10 labels by state {"Ai_Quest", "Q09_Leave" x3} (getMsgLabel)
//   804A45A8 0x0C PTMF {0, -1, fn_8005B2E4}
//   804A45B4 0x0C PTMF {0, -1, fn_8005B58C}
//   804A45C0 0x0C PTMF {0, -1, fn_8005B4B8}
//   804A45CC 0x0C PTMF {0, -1, fn_8005B538}
//   804A45D8 0x0C PTMF {0, -1, fn_8005B50C}
//   804A45E4 0x0C PTMF {0, -1, fn_80053224}
//   804A45F0 0x0C PTMF {0, -1, fn_8005B674}
//   804A45FC 0x0B string "Q_Roomtalk"
//   804A4608 0x30 PTMF x4: fn_8005BC04, fn_8005BBD0, fn_80053224, fn_80053224
//   804A4638 0x0C PTMF {0, -1, fn_8005BC98}
//   804A4644 0x0C PTMF {0, -1, fn_8005BFC0}
//   804A4650 0x0C PTMF {0, -1, fn_80053224}
//   804A465C 0x0C PTMF {0, -1, fn_8005C098}
//   804A4668 0x0C PTMF {0, -1, fn_8005C340}
//   804A4674 0x0C PTMF {0, -1, fn_8005C0EC}
//   804A4680 0x0C PTMF {0, -1, fn_8005C178}
//   804A468C 0x0C PTMF {0, -1, fn_8005C284}
//   804A4698 0x0C PTMF {0, -1, fn_8005C2C8}
//   804A46A4 0x0C PTMF {0, -1, fn_8005C394}
//   804A46B0 0x0C PTMF {0, -1, fn_80053224}
//   804A46BC 0x0C PTMF {0, -1, fn_80053224}
//   804A46C8 0x0C PTMF {0, -1, fn_8005C4B4}
//   804A46D4 0x0C PTMF {0, -1, fn_8005C544}
//   804A46E0 0x0C PTMF {0, -1, fn_8005C698}
//   804A46EC 0x0C PTMF {0, -1, fn_8005C788}
//   804A46F8 0x10 PTMF {0, -1, fn_8005C800} + 4 bytes of padding
// .rodata 8046CAE8..8046CC60:
//   8046CAE8 0x0C string "Q09_Reserve"
//   8046CAF4 0x0D string "Q09_Reserved"
//   8046CB04 0x0B string "Q09_Error1"
//   8046CB10 0x0B string "Q09_Error2"
//   8046CB1C 0x0B string "Q09_Error3"
//   8046CB28 0x0C string "Q09_Welcome"
//   8046CB34 0x0E string "Q09_Furniture"
//   8046CB44 0x48 char[3][12] {"Q09_Trade1", "Q09_First", "Q09_Trade2"}, then PTMF x3 (random room talk):
//                 fn_8005B6C8, fn_8005B858, fn_8005B940
//   8046CB8C 0x0B string "Q09_Trade3"
//   8046CB98 0x0B string "Q09_Trade4"
//   8046CBA4 0x0D string "Q09_TradeYes"
//   8046CBB4 0x0C string "Q09_TradeNo"
//   8046CBC0 0x09 string "Q09_Wait"
//   8046CBCC 0x0B string "Q09_Analog"
//   8046CBD8 0x28 u8[0x28] flag table for pickAppointmentPresent2 (fn_8005C544)
//   8046CC00 0x0B string "Q09_Analog"
//   8046CC10 0x50 u16[0x28] message numbers for "Q09_Analog" (fn_8005C698)
// .sdata2 80750270..807502A0:
//   80750270 0x08 string "Q09_Req"
//   80750278 0x07 string "Q09_No"
//   80750280 0x08 string "Q09_Con"
//   80750288 0x04 f32 100.0 (fn_8005B6F8)
//   80750290 0x08 f64 4503599627370496.0 (int->float magic, fn_8005B6F8)
//   80750298 0x08 string "Q09_Bye"
// External: fn_80053224 (d_npc_talk_quest_delivery), lbl_804A0784 entries [5..9] (nml talk states), lbl_804A0E48;
// fn_8005BCCC / fn_8005BD4C are called from d_npc_talk_fmarket.

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q09Labels[4]; // "Ai_Quest" / "Q09_*" labels of the quest steps
