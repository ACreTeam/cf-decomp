#pragma once

#include <types.h>
#include <game/mLib/m_wipe_fader.hpp>

// The side wipe fader (dSideWipeFader_c, a wipe fader centered on (mCenterX, mCenterY)) and the
// shutter fader (dShutterFader_c) helpers. Source: src/dol/game/d_side_wipe_fader.cpp
// (.text 80160AE4..80160EA4). The class names are from the RTTI; the others are inferred.

// A wipe fader whose wipe is centered on (mCenterX, mCenterY) instead of the screen center.
// Created by the fader setup at 8008B930, which then calls setDefaultTexture.
class dSideWipeFader_c : public mWipeFader_c {
public:
    dSideWipeFader_c(mColor col, mFaderBase_c::EStatus status); // 80160C60

    virtual void calcMtx(); // 80160D2C

    void setDefaultTexture(); // 80160CC0: the wipe texture of 8017CD3C

    /* 0x58 */ f32 mCenterX; // in texture coordinates
    /* 0x5C */ f32 mCenterY;
}; // size 0x60

// A wipe fader with its own texture (defined with the faders at 8015F748..80160AE4).
class dShutterFader_c : public mWipeFader_c {
public:
    dShutterFader_c(); // 801603A0
    void init(); // 80160460: its 0x80 x 0x80 texture
}; // size 0x58

BOOL createShutterFader(); // 80160C14
bool startShutterFadeOut(); // 80160AE4: when hidden
bool isShutterFaderHidden(); // 80160B34
void calcShutterFader(); // 80160B70: fades back in once opaque
void drawShutterFader(); // 80160BDC
