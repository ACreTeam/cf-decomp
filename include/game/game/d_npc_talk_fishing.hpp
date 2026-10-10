#pragma once

// d_npc_talk_fishing.cpp (.text 800470E4..800476E0, .rodata 8046C7A8..8046C7B8, .data
// 804A2CA0..804A2CD0, .sdata 80749A78..80749A80, .sdata2 80750088..80750090). The
// villager talk during the
// fishing tourney (EVENT_FISHING_TOURNEY, message file "Ev_Fishing"): before the npc's first talk of
// the event (with / without a fish entered yet), then by who leads the contest (rank 0: this npc /
// another villager / the current player / another player), with the leading fish, its size and the
// leader's name. The tourney record is at dSaveData_c::getRaw() + 0x632F0 (class not named yet; its
// accessors are fn_801533C8 player, fn_801533E0 villager, fn_801533F8 size, fn_80153410 / fn_8015341C
// item, fn_8015343C size conversion).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook. Parameter and return types are inferred from the asm;
// unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_fishing"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; split in splits.txt):
//   8046C7A8 .rodata 0xB   "Ev_Fishing" (+5 padding)
//   804A2CA0 .data   0x1C  const char *[7]: "Ev_Fishing" x7, message label per state (getMsgLabel kind 0xB)
//   804A2CBC .data   0x14  "sys_STRING/STR_Unit" (fn_801A5874, unit word of the leader's looks)
//   80749A78 .sdata  0x8   dQuestEvent_e EVENT_FISHING_TOURNEY (0xE) for dEvent::isOngoing (+4 padding)
//   80750088 .sdata2 0x8   f64 4503601774854144.0 (s32 -> float conversion of the fish size)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
// The fishing tourney talk state of getFishingTalkState (label index; message code = state + 1).
enum fishingState_e {
    FISHING_STATE_FIRST,             // 0 the npc's first talk of the event (memory event flag 0 not set), no fish yet
    FISHING_STATE_FIRST_FISH,        // 1 the same, a fish is recorded
    FISHING_STATE_NPC_LEADS,         // 2 this npc leads (sets getEntry() _28C.mEventTalked)
    FISHING_STATE_NPC_LEADS_AGAIN,   // 3 this npc leads, already said (mEventTalked set)
    FISHING_STATE_PLAYER_LEADS,      // 4 the current player leads
    FISHING_STATE_OTHER_PLAYER_LEADS, // 5 another player leads
    FISHING_STATE_OTHER_NPC_LEADS,   // 6 another villager leads
    FISHING_STATE_NONE               // 7 no tourney talk (not in the event, or no leader yet)
};

extern const char *l_fishingLabels[FISHING_STATE_NONE]; // "Ev_Fishing" x7
