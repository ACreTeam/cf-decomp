#pragma once

// d_obj_telop (DOL TU d_obj_telop.cpp, not decompiled). Only what other TUs use.

#include <types.h>

// fn_801B66F4 messages, by their users. Names inferred.
enum dTelopMsg_e {
    TELOP_TIMER_START = 1,       // d_clock: the timer started
    TELOP_TIMER_STOP = 2,        // d_clock: the timer was stopped
    TELOP_TIMER_END = 3,         // d_clock: the time is up
    TELOP_HIDE_AND_SEEK_END = 5, // d_clock: the hide and seek game's time is up
    TELOP_NET_SAVE = 6,          // d_net: 6..9 the online save step (mSaveKind)
    TELOP_NET_SAVE_END = 10,     // d_net
};

extern "C" {
void fn_801B66F4(int msg); // 801B66F4: shows telop message msg (dTelopMsg_e; lbl_80625820)
}
