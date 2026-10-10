#pragma once

// d_npc_talk_fmarket.cpp (.text 800476E0..8004902C, .rodata 8046C7B8..8046C828, .data 804A2CD0..804A2F00).
// The villager's flea market talk: as a seller (msgFmarket / msgSale*, "Ev_FmarketNPC*": the npc offers
// one of its boxed furniture items for a price, the player buys it: payMoney / pickUp, entry flags
// mEventTalked / mFmarketSale) and as a buyer at the player's stall (msgStall*, "Ev_FmarketPC": the player names a
// price with the number menu reqMenu21, the npc pays (dPrivateData_c::addMoney) and takes the item
// (dAnimal_c::addNewItem)), plus the "Q08_*" / "Q09_*" messages (call / wait / come back, trade yes /
// no) and the npc walking to the stall (actStallWalk: action_c::requestWalk / requestWait, camera).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers (records
// 21..27 of the d_a_npc_nml table l_talkProcSets 804A0784). See d_npc_talk_arbeit.hpp for the slot
// conventions. talk_c fields used: mFtrPosX / mFtrPosZ (the furniture of the deal, arguments of
// getRoomFtrIdx / fn_800A8F98), mFtrHandle (handle from fn_800A8F98), mItem0 (the item of the deal), mPrice.
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
//   804A2CD0 .data   0xE   "Ev_FmarketNPC" (the .data is addressed from this base in resStallPrice)
//   804A2CE0 .data   0xF   "Ev_FmarketNPC1"
//   804A2CF0 .data   0xF   "Ev_FmarketNPC2"
//   804A2D00 .data   0x20  label table, 8 x const char *: "Ev_FmarketNPC" x6, "Ev_FmarketNPC1",
//                          "Ev_FmarketNPC2" (msgFmarket)
//   804A2D20 .data   0xC   PTMF {0,-1,stepFmarketAsk}
//   804A2D2C .data   0x20  jump table of msgFmarket (switch on the case 0..7)
//   804A2D4C .data   0xC   PTMF {0,-1,selFmarketAsk}
//   804A2D58 .data   0xC   PTMF {0,-1,stepSaleChoice}
//   804A2D64 .data   0xC   PTMF {0,-1,selSaleYes}
//   804A2D70 .data   0xC   PTMF {0,-1,selSaleNo}
//   804A2D7C .data   0xC   PTMF {0,-1,msgSaleYes}
//   804A2D88 .data   0xC   PTMF {0,-1,endSaleYes}
//   804A2D94 .data   0xC   PTMF {0,-1,stepSaleYes}
//   804A2DA0 .data   0xC   PTMF {0,-1,actSaleWait}
//   804A2DAC .data   0xC   PTMF {0,-1,msgSaleNo}
//   804A2DB8 .data   0xC   PTMF {0,-1,endStallTalk}
//   804A2DC4 .data   0xC   PTMF {0,-1,stepStallCall}
//   804A2DD0 .data   0xC   PTMF {0,-1,msgStallComing}
//   804A2DDC .data   0xC   PTMF {0,-1,resStallCall}
//   804A2DE8 .data   0xC   PTMF {0,-1,actStallWalk}
//   804A2DF4 .data   0xC   PTMF {0,-1,actStallArrive}
//   804A2E00 .data   0xC   PTMF {0,-1,endStallTalk}
//   804A2E0C .data   0xC   PTMF {0,-1,stepStallWait}
//   804A2E18 .data   0xC   PTMF {0,-1,msgStallLook}
//   804A2E24 .data   0xC   PTMF {0,-1,endStallLook}
//   804A2E30 .data   0xC   PTMF {0,-1,endStallTalk}
//   804A2E3C .data   0xC   PTMF {0,-1,stepStallBack}
//   804A2E48 .data   0xC   PTMF {0,-1,msgStallLook}
//   804A2E54 .data   0xC   PTMF {0,-1,selStallPrice}
//   804A2E60 .data   0xC   PTMF {0,-1,selStallNo}
//   804A2E6C .data   0x30  PTMF table, 4 x {0,-1,fn}: resStallPrice, msgStallPriceOK, msgStallTooHigh, msgStallRefused
//   804A2E9C .data   0xC   PTMF {0,-1,stepStallBuy}
//   804A2EA8 .data   0xC   PTMF {0,-1,actStallBuyWait}
//   804A2EB4 .data   0xC   PTMF {0,-1,msgStallNoRoom}
//   804A2EC0 .data   0xC   PTMF {0,-1,msgStallBought}
//   804A2ECC .data   0xC   PTMF {0,-1,msgStallRefused}
//   804A2ED8 .data   0xC   PTMF {0,-1,stepStallTooHigh}
//   804A2EE4 .data   0xC   PTMF {0,-1,selStallPrice}
//   804A2EF0 .data   0x10  PTMF {0,-1,selStallNo} (+ 4 bytes padding)
// Globals used, not of this TU: l_walkTargetPos (.bss, walk target position), lbl_8074E9B0 (the camera),
// l_moveParamWalk / l_walkTurnSpeed / cNpcMorphFrames / l_8074FDE8 (d_a_npc).
