#pragma once

// The Wii Remote speaker (DOL TU d_pad_speaker.cpp, not decompiled). Only what other TUs use.

#include <types.h>

extern "C" {
void fn_800FABDC(); // 800FABDC: stops the speaker output (sets 8074E5D8)
void fn_800FABE8(); // 800FABE8: allows it again
void fn_800FAB60(); // 800FAB60: (d_wifi_err, opening its window; acts on mPad::g_core)
}
