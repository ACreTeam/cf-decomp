#pragma once

// Villager talk for appointment quest 9. DOL TU d_npc_talk_quest_q09.cpp (.text 8005A3C4..8005C82C,
// .rodata 8046CAE8..8046CC60, .data 804A4498..804A4708, .sdata2 80750270..807502A0), not
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

// Data of the TU (split in splits.txt; file-local statics in the .cpp). The {0, -1, fn} records are the
// member-function pointer constants of the code above.
// .data 804A4498..804A4708:
//   804A4498 0x0C PTMF {0, -1, endQuestCommon}
//   804A44A4 0x0C PTMF {0, -1, stepInviteOffer}
//   804A44B0 0x0C PTMF {0, -1, msgInviteReq}
//   804A44BC 0x0C PTMF {0, -1, stepInviteChoice}
//   804A44C8 0x0C PTMF {0, -1, selInviteYes}
//   804A44D4 0x0C PTMF {0, -1, selInviteNo}
//   804A44E0 0x0C PTMF {0, -1, msgInviteReserve}
//   804A44EC 0x0C PTMF {0, -1, stepInviteTimeMenu}
//   804A44F8 0x54 PTMF x7: resInviteTime (_128 of stepInviteTimeMenu), then the _0EC results of the time menu:
//                 msgInviteTooLate, msgInviteReserved, msgInviteTooLate, msgInviteTooSoon, msgInviteSleeping, msgInviteTooLate
//   804A454C 0x0C PTMF {0, -1, endInviteReserved}
//   804A4558 0x0C PTMF {0, -1, stepInviteTimeMenu}
//   804A4564 0x0C PTMF {0, -1, stepInviteTimeMenu}
//   804A4570 0x0C PTMF {0, -1, stepInviteTimeMenu}
//   804A457C 0x0C PTMF {0, -1, msgInviteNo}
//   804A4588 0x0A string "Q09_Leave"
//   804A4598 0x10 labels by state {"Ai_Quest", "Q09_Leave" x3} (getMsgLabel)
//   804A45A8 0x0C PTMF {0, -1, stepInviteTalkMenu}
//   804A45B4 0x0C PTMF {0, -1, stepInviteClear}
//   804A45C0 0x0C PTMF {0, -1, selInviteCon}
//   804A45CC 0x0C PTMF {0, -1, selInviteResume}
//   804A45D8 0x0C PTMF {0, -1, msgInviteCon}
//   804A45E4 0x0C PTMF {0, -1, endQuestCommon}
//   804A45F0 0x0C PTMF {0, -1, stepInviteWelcome}
//   804A45FC 0x0B string "Q_Roomtalk"
//   804A4608 0x30 PTMF x4: stepInviteTradeOffer, endInviteFirst, endQuestCommon, endQuestCommon
//   804A4638 0x0C PTMF {0, -1, selInviteTradeOffer}
//   804A4644 0x0C PTMF {0, -1, stepInviteTradeChoice}
//   804A4650 0x0C PTMF {0, -1, endQuestCommon}
//   804A465C 0x0C PTMF {0, -1, selInviteTradeYes}
//   804A4668 0x0C PTMF {0, -1, selInviteTradeNo}
//   804A4674 0x0C PTMF {0, -1, msgInviteTradeYes}
//   804A4680 0x0C PTMF {0, -1, endInviteTradeYes}
//   804A468C 0x0C PTMF {0, -1, stepInviteTradeYes}
//   804A4698 0x0C PTMF {0, -1, actInviteTradeWait}
//   804A46A4 0x0C PTMF {0, -1, msgInviteTradeNo}
//   804A46B0 0x0C PTMF {0, -1, endQuestCommon}
//   804A46BC 0x0C PTMF {0, -1, endQuestCommon}
//   804A46C8 0x0C PTMF {0, -1, stepInvitePresentChoice}
//   804A46D4 0x0C PTMF {0, -1, selInvitePresent}
//   804A46E0 0x0C PTMF {0, -1, msgInvitePresent}
//   804A46EC 0x0C PTMF {0, -1, stepInviteBye}
//   804A46F8 0x10 PTMF {0, -1, msgInviteBye} + 4 bytes of padding
// .rodata 8046CAE8..8046CC60:
//   8046CAE8 0x0C string "Q09_Reserve"
//   8046CAF4 0x0D string "Q09_Reserved"
//   8046CB04 0x0B string "Q09_Error1"
//   8046CB10 0x0B string "Q09_Error2"
//   8046CB1C 0x0B string "Q09_Error3"
//   8046CB28 0x0C string "Q09_Welcome"
//   8046CB34 0x0E string "Q09_Furniture"
//   8046CB44 0x48 char[3][12] {"Q09_Trade1", "Q09_First", "Q09_Trade2"}, then PTMF x3 (random room talk):
//                 msgInviteRoomtalk, msgInviteFurniture, msgInviteTradeOffer
//   8046CB8C 0x0B string "Q09_Trade3"
//   8046CB98 0x0B string "Q09_Trade4"
//   8046CBA4 0x0D string "Q09_TradeYes"
//   8046CBB4 0x0C string "Q09_TradeNo"
//   8046CBC0 0x09 string "Q09_Wait"
//   8046CBCC 0x0B string "Q09_Analog"
//   8046CBD8 0x28 u8[0x28] flag table for pickAppointmentPresent2 (selInvitePresent)
//   8046CC00 0x0B string "Q09_Analog"
//   8046CC10 0x50 u16[0x28] message numbers for "Q09_Analog" (msgInvitePresent)
// .sdata2 80750270..807502A0:
//   80750270 0x08 string "Q09_Req"
//   80750278 0x07 string "Q09_No"
//   80750280 0x08 string "Q09_Con"
//   80750288 0x04 f32 100.0 (pickHouseFtrMsg)
//   80750290 0x08 f64 4503599627370496.0 (int->float magic, pickHouseFtrMsg)
//   80750298 0x08 string "Q09_Bye"
// External: endQuestCommon (d_npc_talk_quest_delivery), lbl_804A0784 entries [5..9] (nml talk states), lbl_804A0E48;
// getSellPrice / getBuyPrice are called from d_npc_talk_fmarket.

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_q09Labels[4]; // "Ai_Quest" / "Q09_*" labels of the quest steps
