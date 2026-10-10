#pragma once

// d_npc_talk_bug.cpp (.text 8004664C..80046BD0; its data is still in the unsplit auto_07 .data /
// auto_09 .sdata / auto_11 .sdata2). Not decompiled yet. The villager talk during the Bug-Off
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

// Data of the TU (file-local statics in the .cpp; not split yet):
//   80750080 .sdata2 0x7   "Ev_Bug" (+1 padding)
//   804A2C60 .data   0x18  const char *[6]: "Ev_Bug" x6, message label per state (getMsgLabel kind 0xC)
//   804A2C78 .data   0x14  "sys_STRING/STR_Unit" (fn_801A5874, unit word of the leader's looks)
//   80749A58 .sdata  0x8   dQuestEvent_e EVENT_BUG_OFF (0xF) for dEvent::isOngoing (+4 padding)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_bugLabels[6]; // "Ev_Bug" x6, by state
