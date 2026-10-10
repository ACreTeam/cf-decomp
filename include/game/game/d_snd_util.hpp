#pragma once

// Sound helpers (TU d_snd_util.cpp, not decompiled).

#include <types.h>

extern "C" {
void fn_8000FC1C(u32 id); // 8000FC1C: starts a sound through a function-local static player
BOOL fn_8000FBF4();                       // 8000FBF4: town tune state (talk_c::actPlayTune starts the tune while FALSE, then waits for TRUE, then FALSE)
void fn_8000FC98(u16 soundId, u8 *notes); // 8000FC98: plays a melody (the town tune) with a voice
}
