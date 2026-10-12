#pragma once

// d_fader (DOL TU d_fader.cpp, .text 8008B91C..8008BD2C): the scene change faders. Creates a color fader, a circle
// wipe (dWipeFader_c) and a side wipe (dSideWipeFader_c) and makes one of them mFader_c's current fader. No RTTI:
// the names are inferred.

#include <types.h>

namespace dFader {

// The fader types (setFader; dScene::dSceneChange_c's wipe in / out kinds).
enum TYPE_e {
    TYPE_BLACK,      // color fader, black
    TYPE_WHITE,      // color fader, white
    TYPE_CIRCLE,     // dWipeFader_c
    TYPE_SIDE_LEFT,  // dSideWipeFader_c centered at (0, 0.5)
    TYPE_SIDE_RIGHT, // dSideWipeFader_c centered at (1, 0.5)
    TYPE_CIRCLE2,    // dWipeFader_c
};

int getOutType();     // 8008B91C: a forced fade-out type for dScene::dSceneChange_c::request (0: none)
int getInType();      // 8008B924: a forced fade-in type (0: none)
BOOL create();        // 8008B930: creates the faders (d_sys)
void fn_8008BA48();   // 8008BA48: sets a flag nothing reads
BOOL setFader(int type); // 8008BA60: makes the fader of TYPE_e type current (d_sys, d_scene_base)

} // namespace dFader
