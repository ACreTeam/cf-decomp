#pragma once

// d_npc_talk_carnival.cpp (.text 8003F354..80043C34, .rodata 8046C630..8046C6C0, .data 804A2308..804A2A90,
// .sdata2 80750060..80750080). Not decompiled yet. The villager talk at the Festivale (EVENT_FESTIVALE,
// message files "Ev_Carnival" / "Ev_Carnival1".."Ev_Carnival5"): a villager in the event gives a player
// in costume a feather (item kind 0x45..0x48) on the first talk, and later asks for one of the player's
// feathers / an item / 500 bells as the stake of one of five mini games (picked at random), each played
// through choices: 1 a chance game, 2 rock-paper-scissors, 3 a two-round guessing game, 4 a three-round
// game with rising odds, 5 guess one of three / one of five. The prize is a feather, the loss takes the
// stake.
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook, 104 = wait/step procedure (BOOL; most act once the
// message controller is idle), choice = answer procedures (setChoiceProc). Parameter and return types are
// inferred from the asm; unused incoming registers are left out.
//
// talk_c fields used here (offsets in talk_c): +0x280 mItems[0] (the feather / stake item), +0x28C int
// (stake in bells: 500, else 0), +0x32B u8 (index of the chosen answer), +0x32F / +0x330 / +0x331 /
// +0x332 u8 (game round / win / loss counters, cleared by fn_800408A4).

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_carnival"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046C630 .rodata 0xC   "Ev_Carnival"
//   8046C63C..8046C67C .rodata 0x10 each (string + padding): "Ev_Carnival1".."Ev_Carnival5"
//   8046C68C .rodata 0x14  int[5] {4, 8, 0xC, 0x10, 0x14}: first state of each game (fn_8003F354, random)
//   8046C6A0 .rodata 0x10  int[4] {0x45, 0x46, 0x47, 0x48}: feather item indices (fn_8003F9D4)
//   8046C6B0 .rodata 0x10  f32[4] {80, 85, 90, 0}: game 4 win chance per round (fn_800425E8)
//   804A2308 .data   0x64  const char *[25]: message label per start state (getMsgLabel kind 4),
//                          "Ev_Carnival" x4, then "Ev_Carnival1".."Ev_Carnival5" x4 each, "Ev_Carnival"
//   804A236C .data   0x48  PTMF x6 (local initializers of fn_8003F354, 104 by state): fn_8003F758,
//                          fn_8003FE74, fn_800407DC, fn_80041644, fn_80042360, fn_80042E98
//   804A23B4 .data   0x64  switch jump table of fn_8003F354 (25 cases)
//   804A2418..804A2A84 .data  PTMF {0,-1,fn} records (0xC each), the static procedure constants and the
//                          local initializers of the procedures above, in order of use:
//                          fn_8003F83C, fn_8003FEB4, fn_8003FEFC, fn_8003FF3C, fn_8003FF7C, fn_8003FFF4,
//                          fn_80040060, fn_80040120 x2, fn_80040160, fn_800401D8, {fn_800403E4,
//                          fn_80040060, fn_80040408, fn_8004042C, fn_80040510}, fn_800404E0,
//                          fn_80040588, fn_80040708, fn_800407AC, fn_8004081C, fn_80040864,
//                          fn_800409CC, fn_80040968, {fn_800409CC, fn_80040AC8 x3}, {fn_80040B1C,
//                          fn_80040CC0, fn_80040DF0, fn_80040F20}, fn_80040D38, fn_80040408,
//                          fn_80040FFC, fn_80040E68, fn_800403E4, fn_80040FFC, fn_80040F98,
//                          fn_800409CC, {fn_80041074, fn_800408F0, fn_800411B8 x2, fn_80041398},
//                          fn_80041230, fn_800412CC, fn_80041368, fn_80041410, fn_80041590,
//                          fn_80041614, fn_80041684, fn_800416CC, fn_8004170C, fn_80041758,
//                          fn_80041818 x2, fn_80041858, {fn_800418D0, fn_80040408, fn_800403E4,
//                          fn_80041A24}, fn_80041AE4 x2, fn_80041B24, {fn_80041B9C, fn_80040408,
//                          fn_800403E4, fn_80041CF0}, {fn_80041D68, fn_80041E48, fn_80042094,
//                          fn_80041F30}, fn_80041EC0, fn_80041758, fn_80041FBC, fn_80042064,
//                          fn_8004210C, fn_8004228C, fn_80042330, fn_800423A0, fn_800423E8,
//                          fn_80042428, fn_80042470, fn_80042530 x2, fn_80042570, {fn_800425E8,
//                          fn_800403E4, fn_800427C8, fn_800428A4, fn_80040408, fn_80042BEC},
//                          fn_80042840, {fn_80042470, fn_800429CC x4}, fn_80042B08, fn_80042BBC,
//                          fn_80042C64, fn_80042DE4, fn_80042E68, fn_80042ED8, fn_80042F20,
//                          fn_80042F60, {fn_80042FA8, fn_800430A4 x3}, fn_800430E4, {fn_8004315C,
//                          fn_800403E4, fn_8004358C, fn_80040408, fn_800432C0}, fn_80043338,
//                          fn_800434B8, {fn_8004355C, fn_800436E0 x5}, fn_80043720, {fn_80043798,
//                          fn_800403E4, fn_800438EC, fn_80040408, fn_800432C0, fn_80043A14 x4},
//                          fn_80043B50, fn_80043C04 ({...} = one dtk object holding several records)
//   80750060 .sdata2 0x4   f32 100.0f (rndF range)
//   80750068 .sdata2 0x8   f64 4503599627370496.0 (u32 -> float conversion)
//   80750070 .sdata2 0x4   f32 85.0f
//   80750074 .sdata2 0x4   f32 70.0f
//   80750078 .sdata2 0x8   f32 20.0f (+4 padding)
// Referenced from outside the split ranges (still in the auto_ files):
//   80749A30 .sdata  0x8   dQuestEvent_e EVENT_FESTIVALE (0x18) for dEvent::isOngoing (+4 padding)
//   8074E198 / 8074E1A8 .sbss  init guards of the local statics of fn_800429CC / fn_80043A14
//   8074E1A0 / 8074E1B0 .sbss  dItem::Item[4] local statics of fn_800429CC / fn_80043A14
//   80565428 .bss    0x60  destructor-chain records of those local statics (4 x 0xC each)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_carnivalLabels[25]; // "Ev_Carnival", "Ev_Carnival1", "Ev_Carnival2", ...
