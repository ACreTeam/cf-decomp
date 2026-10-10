#pragma once

// d_npc_talk_bug.cpp (.text 8004664C..80046BD0, .data 804A2C60..804A2C90, .sdata
// 80749A58..80749A60, .sdata2 80750080..80750088). The villager talk during the
// Bug-Off
// (EVENT_BUG_OFF, message file "Ev_Bug"): before the npc's first talk of the event, then by who leads
// the contest (dBugOff_c rank 0: this npc / another villager / the current player / another player),
// with the leading bug (getItem), its score and the leader's name.
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook. Parameter and return types are inferred from the asm;
// unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_bug"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; split in splits.txt):
//   80750080 .sdata2 0x7   "Ev_Bug" (+1 padding)
//   804A2C60 .data   0x18  const char *[6]: "Ev_Bug" x6, message label per state (getMsgLabel kind 0xC)
//   804A2C78 .data   0x14  "sys_STRING/STR_Unit" (fn_801A5874, unit word of the leader's looks)
//   80749A58 .sdata  0x8   dQuestEvent_e EVENT_BUG_OFF (0xF) for dEvent::isOngoing (+4 padding)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
// The Bug-Off talk state of getBugOffTalkState (label index; message code = state + 1, + 2 from state 1).
enum bugOffState_e {
    BUGOFF_STATE_FIRST,             // 0 the npc's first talk of the event (memory event flag 0 not set)
    BUGOFF_STATE_NPC_LEADS,         // 1 this npc leads (sets getEntry() _28C.mEventTalked)
    BUGOFF_STATE_NPC_LEADS_AGAIN,   // 2 this npc leads, already said (mEventTalked set)
    BUGOFF_STATE_PLAYER_LEADS,      // 3 the current player leads
    BUGOFF_STATE_OTHER_PLAYER_LEADS, // 4 another player leads
    BUGOFF_STATE_OTHER_NPC_LEADS,   // 5 another villager leads
    BUGOFF_STATE_NONE               // 6 no Bug-Off talk (not in the event, or no leader yet)
};

extern const char *l_bugLabels[BUGOFF_STATE_NONE]; // "Ev_Bug" x6, by state
