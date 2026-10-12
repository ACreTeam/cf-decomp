#pragma once

// d_thunder (DOL TU d_thunder.cpp, .text 8016CEF8..8016D264, .ctors 8046576C..80465770): the lightning of a
// thunderstorm (dWeather_c + 0x28C). Namespace from the file name; class and member names inferred from
// what the code does. The five stubs at the start keep their target names (unknown purpose; d_sys and the
// thread creators call them).

#include <types.h>
#include <nw4r/math.h>

namespace EGG {
class Heap;
}

extern "C" {
void fn_8016CEF8(EGG::Heap *heap); // 8016CEF8: empty (d_sys, with the system heap)
void fn_8016CEFC();               // 8016CEFC: empty (d_sys)
void fn_8016CF00();               // 8016CF00: empty (d_sys)
void fn_8016CF04();               // 8016CF04: empty (d_sys)
int fn_8016CF08(int idx);         // 8016CF08: returns 0 (a per-thread value passed to fn_802B8D30)
}

namespace dThunder {

// A lightning strike: waits a random time, flashes (a growing glow above the view center, then the flash
// light color), fades out and plays the thunder sound.
class Thunder_c {
public:
    enum State_e {
        STATE_WAIT,
        STATE_FLASH,
        STATE_FADE,
    };

    void reset();               // 8016CF10
    void calc(BOOL outdoor);    // 8016CF84
    void draw(BOOL outdoor);    // 8016D188

    /* 0x00 */ nw4r::math::VEC3 mPos; // the glow's position
    /* 0x0C */ int mState;            // State_e
    /* 0x10 */ int mTimer;
    /* 0x14 */ f32 mGlow;             // the glow's size
}; // size 0x18

} // namespace dThunder
