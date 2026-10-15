#pragma once

// d_thunder (DOL TU d_thunder.cpp, .text 8016CEF8..8016D264, .ctors 8046576C..80465770): the lightning of a
// thunderstorm (dWeather_c + 0x28C). Namespace from the file name; class and member names inferred from
// what the code does. The first four stubs retain their target names. The fifth is the disabled
// dPerf thread-color lookup, kept in this split.

#include <types.h>
#include <nw4r/math.h>
#include <revolution/GX.h>

namespace EGG {
class Heap;
}

extern "C" {
void fn_8016CEF8(EGG::Heap *heap); // 8016CEF8: empty (d_sys, with the system heap)
void fn_8016CEFC();               // 8016CEFC: empty (d_sys)
void fn_8016CF00();               // 8016CF00: empty (d_sys)
void fn_8016CF04();               // 8016CF04: empty (d_sys)
}

namespace dPerf {
const GXColor *getThreadColor(int index); // 8016CF08: disabled in this build; returns NULL.
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
