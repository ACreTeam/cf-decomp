#pragma once

// d_npc_talk_countdown.cpp (.text 80043C34..80043EBC; its data is still in the unsplit auto_06 .rodata /
// auto_07 .data / auto_09 .sdata). Not decompiled yet. The villager talk at the New Year's countdown
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

// Data of the TU (file-local statics in the .cpp; not split yet):
//   8046C6C0 .rodata 0xD   "Ev_Countdown"
//   804A2A90 .data   0x20  const char *[7] (+4 padding): "Ev_Countdown" x7, message label per state
//                          (getMsgLabel kind 3)
//   80749A38 .sdata  0x4   dQuestEvent_e EVENT_COUNTDOWN (0x13) for dEvent::isOngoing
//   80749A3C .sdata  0x4   dQuestEvent_e EVENT_COUNTDOWN (0x13) for dEvent::isOver
//   80749A40 .sdata  0x8   dQuestEvent_e EVENT_COUNTDOWN (0x13) for dEvent::isOngoing (+4 padding)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_countdownLabels[7]; // "Ev_Countdown" x7
