#pragma once

// d_npc_talk_fmarket.cpp (.text 800476E0..8004902C, .rodata 8046C7B8..8046C827, .data 804A2CD0..804A2EFF).
// Not decompiled yet. The villager's flea market talk: as a seller ("Ev_FmarketNPC*": the npc offers
// one of its boxed furniture items for a price, the player buys it: payMoney / pickUp, npc timer flags)
// and as a buyer at the player's stall ("Ev_FmarketPC": the player names a price with the number menu
// reqMenu21, the npc pays (dPrivateData_c::addMoney) and takes the item (dAnimal_c::addNewItem)), plus
// the "Q08_*" / "Q09_*" messages (call / wait / come back, trade yes / no) and the npc walking to the
// stall (action_c::requestWalk / requestWait, camera).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers (records
// 21..27 of the d_a_npc_nml table 804A0784). See d_npc_talk_arbeit.hpp for the slot conventions. talk_c
// fields used: +0x1F8 / +0x1FC (the furniture of the deal, arguments of getRoomFtrIdx / fn_800A8F98),
// +0x200 (handle from fn_800A8F98), +0x280 (mItems[0], the item of the deal), +0x28C (int price).
// Parameter and return types are inferred from the asm; unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_fmarket"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046C7B8 .rodata 0xF   "Ev_FmarketNPC3"
//   8046C7C8 .rodata 0xF   "Ev_FmarketNPC4"
//   8046C7D8 .rodata 0xD   "Q09_TradeYes"
//   8046C7E8 .rodata 0xC   "Q09_TradeNo"
//   8046C7F4 .rodata 0xD   "Ev_FmarketPC"
//   8046C804 .rodata 0x9   "Q08_Call"
//   8046C810 .rodata 0x9   "Q08_Wait"
//   8046C81C .rodata 0x9   "Q08_Back"
//   804A2CD0 .data   0xE   "Ev_FmarketNPC" (the .data is addressed from this base in fn_800489D4)
//   804A2CE0 .data   0xF   "Ev_FmarketNPC1"
//   804A2CF0 .data   0xF   "Ev_FmarketNPC2"
//   804A2D00 .data   0x20  label table, 8 x const char *: "Ev_FmarketNPC" x6, "Ev_FmarketNPC1",
//                          "Ev_FmarketNPC2" (fn_800476E0)
//   804A2D20 .data   0xC   PTMF {0,-1,fn_80047A6C}
//   804A2D2C .data   0x20  jump table of fn_800476E0 (switch on the case 0..7)
//   804A2D4C .data   0xC   PTMF {0,-1,fn_80047B00}
//   804A2D58 .data   0xC   PTMF {0,-1,fn_80047CD4}
//   804A2D64 .data   0xC   PTMF {0,-1,fn_80047DAC}
//   804A2D70 .data   0xC   PTMF {0,-1,fn_8004805C}
//   804A2D7C .data   0xC   PTMF {0,-1,fn_80047E00}
//   804A2D88 .data   0xC   PTMF {0,-1,fn_80047E8C}
//   804A2D94 .data   0xC   PTMF {0,-1,fn_80047FA0}
//   804A2DA0 .data   0xC   PTMF {0,-1,fn_80047FE4}
//   804A2DAC .data   0xC   PTMF {0,-1,fn_800480B0}
//   804A2DB8 .data   0xC   PTMF {0,-1,fn_8004816C}
//   804A2DC4 .data   0xC   PTMF {0,-1,fn_800481FC}
//   804A2DD0 .data   0xC   PTMF {0,-1,fn_80048438}
//   804A2DDC .data   0xC   PTMF {0,-1,fn_8004829C}
//   804A2DE8 .data   0xC   PTMF {0,-1,fn_800482DC}
//   804A2DF4 .data   0xC   PTMF {0,-1,fn_8004838C}
//   804A2E00 .data   0xC   PTMF {0,-1,fn_8004816C}
//   804A2E0C .data   0xC   PTMF {0,-1,fn_800484F4}
//   804A2E18 .data   0xC   PTMF {0,-1,fn_8004856C}
//   804A2E24 .data   0xC   PTMF {0,-1,fn_800485D0}
//   804A2E30 .data   0xC   PTMF {0,-1,fn_8004816C}
//   804A2E3C .data   0xC   PTMF {0,-1,fn_80048674}
//   804A2E48 .data   0xC   PTMF {0,-1,fn_8004856C}
//   804A2E54 .data   0xC   PTMF {0,-1,fn_80048988}
//   804A2E60 .data   0xC   PTMF {0,-1,fn_80048E44}
//   804A2E6C .data   0x30  PTMF table, 4 x {0,-1,fn}: fn_800489D4, fn_80048B0C, fn_80048EF0, fn_80048E98
//   804A2E9C .data   0xC   PTMF {0,-1,fn_80048B70}
//   804A2EA8 .data   0xC   PTMF {0,-1,fn_80048D3C}
//   804A2EB4 .data   0xC   PTMF {0,-1,fn_80048E14}
//   804A2EC0 .data   0xC   PTMF {0,-1,fn_80048DE4}
//   804A2ECC .data   0xC   PTMF {0,-1,fn_80048E98}
//   804A2ED8 .data   0xC   PTMF {0,-1,fn_80048F54}
//   804A2EE4 .data   0xC   PTMF {0,-1,fn_80048988}
//   804A2EF0 .data   0x10  PTMF {0,-1,fn_80048E44} (+ 4 bytes padding)
// Globals used, not of this TU: l_walkTargetPos (.bss, walk target position), lbl_8074E9B0 (the camera),
// l_moveParamWalk / l_walkTurnSpeed / cNpcMorphFrames / l_8074FDE8 (d_a_npc).
