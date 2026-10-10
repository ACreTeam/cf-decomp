#pragma once

// d_npc_talk_carnival.cpp (.text 8003F354..80043C34, .rodata 8046C630..8046C6C0, .data 804A2308..804A2A90,
// .bss 80565428..80565488, .sdata 80749A30..80749A38, .sbss 8074E198..8074E1B8, .sdata2
// 80750060..80750080). The villager talk at the Festivale (EVENT_FESTIVALE,
// message files "Ev_Carnival" / "Ev_Carnival1".."Ev_Carnival5"): a villager in the event gives a player
// in costume a candy (ITEM_IDX_BLUE_CANDY..ITEM_IDX_GREEN_CANDY, BITM kind KIND_CANDY) on the first talk,
// and later asks for one of the player's candies / an item / 500 bells as the stake of one of five mini
// games (picked at random), each played through choices: 1 win two rounds in a row, 2 rock-paper-scissors
// (best of three), 3 a two-round game with a replay on a draw, 4 a three-round game with rising odds,
// 5 guess one of three / one of five. The prize is a candy, a loss takes the stake.
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook, 104 = wait/step procedure (BOOL; most act once the
// message controller is idle), choice = answer procedures (setChoiceProc). Parameter and return types are
// inferred from the asm; unused incoming registers are left out.
//
// talk_c fields used here (offsets in talk_c): +0x280 mItem0 (the candy prize / stake item), +0x28C mPrice
// (stake in bells: 500, else 0), +0x32B mAnswer (index of the chosen answer), mGameCount[4] / mGameShown[2] u8 game
// counters: games 1, 4, 5 use mGameCount[0] as the round (game 1: won once); games 2 and 3 count losses in
// mGameCount[0] and wins in mGameCount[1]; game 3 marks shown message variants in mGameCount[2..3] / mGameShown[0..1].

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// msgCarnival's state (index into l_carnivalLabels; each game owns four states that share its label).
enum carnivalState_e {
    CARNIVAL_NO_COSTUME = 0, // first talk at the event, the player isn't in costume
    CARNIVAL_GIFT = 1,       // first talk: gives a random candy (stepCarnivalGift)
    CARNIVAL_GIFT_FULL = 2,  // first talk, the pockets are full
    CARNIVAL_FULL = 3,       // the pockets are full: no game
    CARNIVAL_GAME1 = 4,      // games 1..5 ("Ev_Carnival1".."Ev_Carnival5"), picked at random
    CARNIVAL_GAME2 = 8,
    CARNIVAL_GAME3 = 12,
    CARNIVAL_GAME4 = 16,
    CARNIVAL_GAME5 = 20,
    CARNIVAL_PLAYED = 24,    // a game was already offered (getEntry() _28C.mEventTalked)
    CARNIVAL_STATE_NUM = 25  // no Festivale talk
};

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_carnival"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046C630 .rodata 0xC   "Ev_Carnival"
//   8046C63C..8046C67C .rodata 0x10 each (string + padding): "Ev_Carnival1".."Ev_Carnival5"
//   8046C68C .rodata 0x14  int[5] {4, 8, 0xC, 0x10, 0x14}: first state of each game (msgCarnival, random)
//   8046C6A0 .rodata 0x10  int[4] {0x45, 0x46, 0x47, 0x48}: candy item indices l_candyIdx (setStakeMsg)
//   8046C6B0 .rodata 0x10  f32[4] {80, 85, 90, 0}: game 4 win chance per round (msgCarnival4Result)
//   804A2308 .data   0x64  const char *[25]: message label per start state (getMsgLabel kind 4),
//                          "Ev_Carnival" x4, then "Ev_Carnival1".."Ev_Carnival5" x4 each, "Ev_Carnival"
//   804A236C .data   0x48  PTMF x6 (local initializers of msgCarnival, 104 by state): stepCarnivalGift,
//                          stepCarnival1Start, stepCarnival2Start, stepCarnival3Start, stepCarnival4Start,
//                          stepCarnival5Start
//   804A23B4 .data   0x64  switch jump table of msgCarnival (25 cases)
//   804A2418..804A2A84 .data  PTMF {0,-1,fn} records (0xC each), the static procedure constants and the
//                          local initializers of the procedures above, in order of use:
//                          msgCarnivalGift, msgCarnival1Stake, stepCarnival1Stake, selCarnival1Accept,
//                          stepCarnival1Accept, msgCarnival1Play, stepCarnival1Choice, selCarnival1Answe  x,
//                          stepCarnival1Answer, msgCarnival1Result, {endCarnivalWin, stepCarnival1Choice,
//                          endCarnivalLose, stepCarnival1Prize, stepCarnival1Lost}, msgCarnival1Prize,
//                          msgCarnival1Pay, stepCarnival1Pay, msgCarnival1Paid, msgCarnival2Stake,
//                          stepCarnival2Stake, stepCarnival2Choice, msgCarnival2Next, {stepCarnival2Choice,
//                          selCarnival2Han  x}, {msgCarnival2Hand, stepCarnival2Lose, stepCarnival2Win,
//                          stepCarnival2Draw}, msgCarnival2Lose, endCarnivalLose, stepCarnival2Score,
//                          msgCarnival2Win, endCarnivalWin, stepCarnival2Score, msgCarnival2Draw,
//                          stepCarnival2Choice, {msgCarnival2Score, stepCarnival2Next, stepCarnival2Wo  x,
//                          stepCarnival2Lost}, msgCarnival2Won, stepCarnival2Prize, msgCarnival2Prize,
//                          msgCarnival2Pay, stepCarnival2Pay, msgCarnival2Paid, msgCarnival3Stake,
//                          stepCarnival3Stake, selCarnival3Accept, stepCarnival3Choice,
//                          selCarnival3Answe  x, stepCarnival3Answer, {msgCarnival3Round1, endCarnivalLose,
//                          endCarnivalWin, stepCarnival3Choice2}, selCarnival3Answer  x,
//                          stepCarnival3Answer2, {msgCarnival3Round2, endCarnivalLose, endCarnivalWin,
//                          stepCarnival3Score}, {msgCarnival3Score, stepCarnival3Draw, stepCarnival3Lost,
//                          stepCarnival3Won}, msgCarnival3Draw, stepCarnival3Choice, stepCarnival3Prize,
//                          msgCarnival3Prize, msgCarnival3Pay, stepCarnival3Pay, msgCarnival3Paid,
//                          msgCarnival4Stake, stepCarnival4Stake, selCarnival4Accept, stepCarnival4Choice,
//                          selCarnival4Answe  x, stepCarnival4Answer, {msgCarnival4Result, endCarnivalWin,
//                          stepCarnival4Next, stepCarnival4Color, endCarnivalLose, stepCarnival4Lost},
//                          msgCarnival4Next, {stepCarnival4Choice, selCarnival4Colo  x}, stepCarnival4Prize,
//                          msgCarnival4Prize, msgCarnival4Pay, stepCarnival4Pay, msgCarnival4Paid,
//                          msgCarnival5Stake, stepCarnival5Stake, selCarnival5Accept, {stepCarnival5Choice,
//                          selCarnival5Answe  x}, stepCarnival5Answer, {msgCarnival5Result, endCarnivalWin,
//                          stepCarnival5Choice2, endCarnivalLose, stepCarnival5Lost}, msgCarnival5Pay,
//                          stepCarnival5Pay, {msgCarnival5Paid, selCarnival5Answer  x},
//                          stepCarnival5Answer2, {msgCarnival5Result2, endCarnivalWin, stepCarnival5Color,
//                          endCarnivalLose, stepCarnival5Lost, selCarnival5Colo  x}, stepCarnival5Prize,
//                          msgCarnival5Prize ({...} = one dtk object holding several records)
//   80750060 .sdata2 0x4   f32 100.0f (rndF range)
//   80750068 .sdata2 0x8   f64 4503599627370496.0 (u32 -> float conversion)
//   80750070 .sdata2 0x4   f32 85.0f
//   80750074 .sdata2 0x4   f32 70.0f
//   80750078 .sdata2 0x8   f32 20.0f (+4 padding)
// Referenced from outside the split ranges (still in the auto_ files):
//   80749A30 .sdata  0x8   dQuestEvent_e EVENT_FESTIVALE (0x18) for dEvent::isOngoing (+4 padding)
//   8074E198 / 8074E1A8 .sbss  init guards of the local statics of selCarnival4Color / selCarnival5Color
//   8074E1A0 / 8074E1B0 .sbss  dItem::Item[4] local statics of selCarnival4Color / selCarnival5Color
//   80565428 .bss    0x60  destructor-chain records of those local statics (4 x 0xC each)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_carnivalLabels[CARNIVAL_STATE_NUM]; // "Ev_Carnival", "Ev_Carnival1", "Ev_Carnival2", ...
