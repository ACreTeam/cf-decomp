#pragma once

// The waiting icon shown while the screen stays black (TU d_black_wait.cpp, .text 800805BC..80080D7C): after
// 2 s of a black (faded-out) screen, four textures "BlackWait/waitIcon_0N.brtex" cycle every 45 frames.
// Created / drawn / executed by d_sys, loaded by the boot scene. No RTTI: the namespace is from the file name,
// the class, function and member names are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>
#include <nw4r/g3d.h>

namespace EGG {
class Heap;
}

namespace dBlackWait {

#define BLACK_WAIT_ICON_NUM 4

class BlackWait_c {
public:
    enum State_e {
        STATE_LOAD, // textures not loaded yet (nothing to do)
        STATE_WAIT, // waiting for 2 s of black screen
        STATE_ANIM, // the icon is shown
    };

    BlackWait_c(EGG::Heap *heap); // 800809D0

    static void create(EGG::Heap *heap); // 800805BC
    static BlackWait_c *getInstance();   // 80080608

    BOOL load();    // 80080610: requests the textures; TRUE (and STATE_WAIT) once all are there
    void execute(); // 800806C8
    void draw();    // 80080700

    void stepLoad(); // 80080B54
    void stepWait(); // 80080B58
    void stepAnim(); // 80080C14
    BOOL isBlack();  // 80080C9C: the fader is opaque or dReset is faded out

    /* 0x00 */ EGG::Heap *mpHeap;
    /* 0x04 */ dDvd::loader_c mLoader[BLACK_WAIT_ICON_NUM];
    /* 0x54 */ nw4r::g3d::ResTex mTex[BLACK_WAIT_ICON_NUM];
    /* 0x64 */ int mState;     // State_e
    /* 0x68 */ u32 mStartTick; // OSGetTick() when the screen went black (0: not yet)
    /* 0x6C */ int mFrame;     // the icon shown, -1: none
    /* 0x70 */ int mTimer;     // frames on this icon
    /* 0x74 */ u8 mHide;       // never set here
}; // size 0x78

} // namespace dBlackWait
