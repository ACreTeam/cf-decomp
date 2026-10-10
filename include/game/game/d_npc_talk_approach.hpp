#pragma once

// d_npc_talk_approach.cpp (.text 80036324..80038134, .rodata 8046C088..8046C178, .data 804A1BF0..804A1D58,
// .sdata 80749970..80749978, .sdata2 8074FF80..8074FFB0). The remarks a villager makes
// when it walks up to
// the player on its own ("approach", message labels "ApA_*" / "ApB_*" / "ApC_*" / "ApD_*"):
//   ApD_Moving   the villager is moving out; ApD_Fortune  remarks on the player's fortune (incl. a
//                lucky item for the player);
//   ApB_Habit / ApB_Hello / ApB_Nickname  greetings, new nickname for the player;
//   ApC_Present / ApC_Sell / ApC_Trade / ApC_Want  item offers (gift, sell, trade, buy);
//   ApA_Letter / ApA_Always  "got your letter?" and the default line.
// pickWantedPocketItem is also used by d_npc_talk_free.
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slots, as
// seen from the d_a_npc_nml helpers (provisional):
//   EC   message select (setMsgProc / setProcSet): fills the msgInfo_s through setLooksMsg
//        (label, code) and returns TRUE when it picked a message.
//   F8   hook called once by fn_800313B0 (setHookProc / setProcSet): no result.
//   104  wait/step procedure (setStepProc, called by fn_8003209C): BOOL; most only act once the
//        message controller is idle (Rcpt_c::mpController+0x6C84 == 0).
//   140  after-talk procedure (stored directly at talk_c+0x140, called with no argument).
//   choice  answer procedures registered with setChoiceProc (talk_c+0x214 table).
// The root select msgApproach and the F8 hook endApproach are the record at d_a_npc_nml's table
// 804A0784+0x240 ({EC, F8, 104} records of 0x24, copied by setProcSet).
// Parameter and return types are inferred from the asm; unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_private_data.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_approach"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046C088 .rodata 0x30  PTMF table, 4 x {0,-1,fn}: msgApMoving, msgApFortuneLove, msgApFortuneFriend,
//                          msgApFortuneItem (ApD checks of msgApD)
//   8046C0B8 .rodata 0x20  int[8] item kind per wish kind (dQuestWish_c::mKind): KIND_INSECT, KIND_FISH,
//                          KIND_FOSSIL, KIND_CLOTH, KIND_FTR, KIND_COUNT x3 (pickWantedPocketItem)
//   8046C0D8 .rodata 0x58  PTMF table, 7 x {0,-1,fn} (+4 pad): msgApHabit, msgApHello,
//                          msgApNickname, msgApPresent, msgApSell, msgApTrade, msgApWant
//                          (ApB/ApC checks of msgApBC)
//   8046C130 .rodata 0x18  PTMF table, 2 x {0,-1,fn}: msgApLetter, msgApAlways (msgApA)
//   8046C148 .rodata 0x30  PTMF table, 4 x {0,-1,fn}: msgApD, msgApBC, msgApPendingQuest,
//                          msgApA (msgApproach)
//   804A1BF0 .data   0x18  strings "ApD_Moving", "ApD_Fortune"
//   804A1C08 .data   0x10  const char *[4]: ApD_Moving, ApD_Fortune x3 (getApproachLabel group 0)
//   804A1C18 .data   0x58  strings "ApB_Habit", "ApB_Hello", "ApB_Nickname", "ApC_Present",
//                          "ApC_Sell", "ApC_Trade", "ApC_Want"
//   804A1C70 .data   0x1C  const char *[7] of those (getApproachLabel group 1)
//   804A1C8C .data   0x17  strings "ApA_Letter", "ApA_Always" (pointed to by .sdata 80749970)
//   804A1CA4 .data   0xC   PTMF {0,-1,reqApMoveOut}   804A1CB0 .data 0xC PTMF {0,-1,stepApFortune}
//   804A1CBC .data   0xC   PTMF {0,-1,stepApFortune}   804A1CC8 .data 0xC PTMF {0,-1,endApFortuneItem}
//   804A1CD4 .data   0xC   PTMF {0,-1,stepApNickname}   804A1CE0 .data 0xC PTMF {0,-1,endApPresent}
//   804A1CEC .data   0xC   PTMF {0,-1,endApSell}   804A1CF8 .data 0xC PTMF {0,-1,endApTrade}
//   804A1D04 .data   0xC   PTMF {0,-1,endApWant}   804A1D10 .data 0xC PTMF {0,-1,msgApFortuneGift}
//   804A1D1C .data   0xC   PTMF {0,-1,msgApFortuneFull}   804A1D28 .data 0xC PTMF {0,-1,selApNicknameNew}
//   804A1D34 .data   0xC   PTMF {0,-1,stepApNicknameChoice}   804A1D40 .data 0xC PTMF {0,-1,selApNicknameKeep}
//   804A1D4C .data   0xC   PTMF {0,-1,selApNicknameUndo}
//                          (the single PTMFs are local copies of member-function pointer constants)
//   8074FF80 .sdata2 0x30  100.0f, int->float magic double, 25.0f, 0.8f, 0.5f, 0.3f, unsigned
//                          int->float magic double, 0.25f
//   80749970 .sdata  0x8   const char *[2]: ApA_Letter, ApA_Always (getApproachLabel group 3); this TU's
//                          .sdata split
