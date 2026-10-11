#pragma once

// d_rumble (DOL TU d_rumble.cpp, .text 800FAB0C..800FABF4): Wii Remote rumble. Plays rumble patterns from a table
// on controller 0 through EGG's rumble manager. No RTTI: the names are inferred (the TU was called d_pad_speaker
// by the first split pass).

#include <types.h>

namespace dRumble {

// A rumble pattern: '*' rumbles for a frame, '-' pauses.
struct Pattern_s {
    /* 0x00 */ char mPattern[0x24];
    /* 0x24 */ int mFrames; // EGG::CoreController::startPatternRumble: 0 plays it once, -1 repeats it
}; // size 0x28

void create(u8 num);              // 800FAB0C: creates controller 0's rumble manager (in dHeap::rumbleHeap_p)
void stop();                      // 800FAB60: stops controller 0's rumble (d_wifi_err / d_caution opening)
void start(int pattern, bool force); // 800FAB6C: plays sPatterns[pattern] when rumble is on and not disabled
void disable();                   // 800FABDC: (d_reset)
void enable();                    // 800FABE8: (d_reset)

} // namespace dRumble
