#pragma once

// d_npc_talk_fireworks.cpp (.text 80046BD0..800470E4, .rodata 8046C798..8046C7A8, .data
// 804A2C90..804A2CA0, .sdata 80749A60..80749A78). The villager talk at the fireworks
// show
// (EVENT_FIREWORKS, message file "Ev_Fireworks"): the message depends on the time relative to the
// event (up to 30 min before the start, the first hour, the middle, the last hour).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook. Parameter and return types are inferred from the asm;
// unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_fireworks"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; split in splits.txt):
//   8046C798 .rodata 0xD   "Ev_Fireworks" (+3 padding)
//   804A2C90 .data   0x10  const char *[4]: "Ev_Fireworks" x4, message label per state (getMsgLabel kind 0xD)
//   80749A60 .sdata  0x4   dQuestEvent_e EVENT_FIREWORKS (0x10) for dEvent::isActive
//   80749A64 .sdata  0x4   dQuestEvent_e EVENT_FIREWORKS (0x10) for dEvent::isOver
//   80749A68 .sdata  0x4   dQuestEvent_e EVENT_FIREWORKS (0x10) for dEvent::getStartTime
//   80749A6C .sdata  0x4   dQuestEvent_e EVENT_FIREWORKS (0x10) for dEvent::isNotStarted
//   80749A70 .sdata  0x8   dQuestEvent_e EVENT_FIREWORKS (0x10) for dEvent::getEndTime (+4 padding)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
// The fireworks talk state of getFireworksTalkState (label index; message code = state + 1).
enum fireworksState_e {
    FIREWORKS_STATE_BEFORE,    // 0 not started yet, at most 30 minutes before the start
    FIREWORKS_STATE_FIRST_HOUR, // 1 the first hour of the show
    FIREWORKS_STATE_MIDDLE,    // 2 between the first and the last hour
    FIREWORKS_STATE_LAST_HOUR, // 3 the last hour before the end
    FIREWORKS_STATE_NONE       // 4 no fireworks talk
};

extern const char *l_fireworksLabels[FIREWORKS_STATE_NONE]; // "Ev_Fireworks" x4
