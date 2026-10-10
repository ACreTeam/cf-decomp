#pragma once

// d_npc_talk_reaction.cpp (.text 8005E584..80060980, .data 804A4938..804A4B60, .bss 80565488..805654A0,
// .sdata 80749AD8..80749B20, .sbss 8074E1C0..8074E1D0, .sdata2 807502D0..807502D8). Decompiled (42/42 functions).
// The villager's "reactions" when the player starts a conversation: the 28 checks of the d_a_npc_nml
// PTMF table 8046BDE0 (tried in order by nml msgReaction, record 0 of 804A0E48, through getReactionMsg),
// one per "Re_*" label of 804A4A48 (moving out / in, first meeting, birthday, not seen for 30 / 7 days,
// tired / angry / sad, bee face, poison, holidays, ...).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. See
// d_npc_talk_arbeit.hpp for the slot conventions. The checks are called like EC procedures; with info ==
// NULL they only test whether the reaction applies (TRUE = yes), otherwise they also fill info
// (setLooksMsg). Index n in the comments = entry n of 8046BDE0 = label index of nml getMsgLabel(0, n).
// Parameter and return types are inferred from the asm; unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>


// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_reaction"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp, except the label table read by d_a_npc_nml):
//   804A4938 .data   0x110 the longer "Re_*" strings ("Re_Moveout", "Re_Fishing", "Ev_First",
//                          "Re_FirstA1" .. "Re_FirstV", "Re_Movein", "Re_Birthday", "Re_30days",
//                          "Re_7days", "Re_Anger", "Re_BeeFace", "Re_Poison", "Re_Newyear",
//                          "Re_Harvest", "Re_Halloween", "Re_Fireworks", "download")
//   804A4A48 .data   0x70  label table, 28 x const char * ("Re_Moveout" .. "download"); read by nml
//                          getMsgLabel(0, n), so not file-local
//   804A4AB8 .data   0xC   PTMF {0,-1,stepMoveout}
//   804A4AC4 .data   0xC   PTMF {0,-1,stepBeeFaceSeen}
//   804A4AD0 .data   0xC   PTMF {0,-1,stepReaction} (nml)
//   804A4ADC .data   0xC   PTMF {0,-1,stepReaction} (nml)
//   804A4AE8 .data   0xC   PTMF {0,-1,stepReaction} (nml)
//   804A4AF4 .data   0xC   PTMF {0,-1,stepBirthday}
//   804A4B00 .data   0xC   PTMF {0,-1,stepBeeFaceSeen}
//   804A4B0C .data   0xC   PTMF {0,-1,stepBeeFaceSeen}
//   804A4B18 .data   0xC   PTMF {0,-1,stepPoison}
//   804A4B24 .data   0x3C  5 x PTMF {0,-1,stepEventTalked} (804A4B24, 30, 3C, 48, 54)
//   80749AD8 .sdata  0x30  the short strings "Re_Cafe", "Re_Fall", "Re_Run", "Re_Tire", "Re_Sad",
//                          "Re_Xmas" (8 bytes apart)
//   80749B08 .sdata  0x18  const dQuestEvent_e 23, 20, 22, 21, 16 (arguments of dEvent::isOngoing /
//                          isOver in msgReXmas .. msgReFireworks; + padding)
//   8074E1C0 .sbss   0x10  guards and the two function-local static dItem::Item of findNearItemCb
//                          (ITEM_IDX_SCORPION at 8074E1C4, ITEM_IDX_TARANTULA at 8074E1C8)
//   80565488 .bss    0x18  two global destructor chain entries of those statics (0xC each)
//   807502D0 .sdata2 0x8   f32 48.0f (height range), 96.0f (distance range) of findNearItemCb / findNearItem

// The reactions: index into nml getReactionMsg's table (8046BDE0, tried in this order) and the label
// index of getMsgLabel(TALK_REACTION, idx) (l_reactionLabels).
enum reaction_e {
    REACTION_MOVEOUT,    // 0  "Re_Moveout"   msgReMoveout
    REACTION_CAFE,       // 1  "Re_Cafe"      msgReCafe
    REACTION_FISHING,    // 2  "Re_Fishing"   msgReFishing (never)
    REACTION_FALL,       // 3  "Re_Fall"      msgReFall
    REACTION_RUN,        // 4  "Re_Run"       msgReRun
    REACTION_EV_FIRST,   // 5  "Ev_First"     msgEvFirst
    REACTION_FIRST_A1,   // 6  "Re_FirstA1"   msgReFirstA1 .. msgReFirstV: msgFirstMeeting
    REACTION_FIRST_A2,   // 7  "Re_FirstA2"
    REACTION_FIRST_B1,   // 8  "Re_FirstB1"
    REACTION_FIRST_B2,   // 9  "Re_FirstB2"
    REACTION_FIRST_C1,   // 10 "Re_FirstC1"
    REACTION_FIRST_C2,   // 11 "Re_FirstC2"
    REACTION_FIRST_V,    // 12 "Re_FirstV"
    REACTION_MOVEIN,     // 13 "Re_Movein"    msgReMovein
    REACTION_BIRTHDAY,   // 14 "Re_Birthday"  msgReBirthday
    REACTION_30DAYS,     // 15 "Re_30days"    msgRe30days (msgNotSeen)
    REACTION_7DAYS,      // 16 "Re_7days"     msgRe7days (msgNotSeen)
    REACTION_TIRE,       // 17 "Re_Tire"      msgReTire
    REACTION_ANGER,      // 18 "Re_Anger"     msgReAnger
    REACTION_SAD,        // 19 "Re_Sad"       msgReSad
    REACTION_BEE_FACE,   // 20 "Re_BeeFace"   msgReBeeFace
    REACTION_POISON,     // 21 "Re_Poison"    msgRePoison
    REACTION_XMAS,       // 22 "Re_Xmas"      msgReXmas
    REACTION_NEW_YEAR,   // 23 "Re_Newyear"   msgReNewyear
    REACTION_HARVEST,    // 24 "Re_Harvest"   msgReHarvest
    REACTION_HALLOWEEN,  // 25 "Re_Halloween" msgReHalloween
    REACTION_FIREWORKS,  // 26 "Re_Fireworks" msgReFireworks
    REACTION_DOWNLOAD,   // 27 "download"     msgReDownload
    REACTION_NUM
};

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_reactionLabels[REACTION_NUM]; // "Re_Moveout", "Re_Cafe", ... one per reaction
