#pragma once

// d_npc_talk_countdown.cpp (.text 80043C34..80043EBC, .rodata 8046C6C0..8046C6D0, .data
// 804A2A90..804A2AB0, .sdata 80749A38..80749A48). The villager talk at the New
// Year's countdown
// (EVENT_COUNTDOWN, message file "Ev_Countdown"): the message depends on the time (23:00-23:29,
// 23:30-23:54, 23:55-23:58, 23:59, after midnight before / after 2:00), plus a once-per-villager
// message while the event runs (dAnimalMemory_c flag bit 0x01000000, fn_800F05C8).
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slot
// conventions (provisional, see d_npc_talk_arbeit.hpp): EC = message select (fills msgInfo_s via
// setLooksMsg, returns 1), F8 = end-of-talk hook. Parameter and return types are inferred from the asm;
// unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_countdown"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp; split in splits.txt):
//   8046C6C0 .rodata 0xD   "Ev_Countdown"
//   804A2A90 .data   0x20  const char *[7] (+4 padding): "Ev_Countdown" x7, message label per state
//                          (getMsgLabel kind 3)
//   80749A38 .sdata  0x4   dQuestEvent_e EVENT_COUNTDOWN (0x13) for dEvent::isOngoing
//   80749A3C .sdata  0x4   dQuestEvent_e EVENT_COUNTDOWN (0x13) for dEvent::isOver
//   80749A40 .sdata  0x8   dQuestEvent_e EVENT_COUNTDOWN (0x13) for dEvent::isOngoing (+4 padding)

// The countdown talk state of msgCountdown (label index; message code = state + 1).
enum countdownState_e {
    COUNTDOWN_STATE_FIRST,    // 0 first talk of the event (memory flag 0x01000000 not set), not 23:xx
    COUNTDOWN_STATE_2300,     // 1 23:00-23:29
    COUNTDOWN_STATE_2330,     // 2 23:30-23:54
    COUNTDOWN_STATE_2355,     // 3 23:55-23:58
    COUNTDOWN_STATE_2359,     // 4 23:59
    COUNTDOWN_STATE_NEW_YEAR, // 5 after midnight, before 2:00
    COUNTDOWN_STATE_OTHER,    // 6 other times of the event day
    COUNTDOWN_STATE_NONE      // 7 not in the event: no countdown talk
};

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_countdownLabels[COUNTDOWN_STATE_NONE]; // "Ev_Countdown" x7
