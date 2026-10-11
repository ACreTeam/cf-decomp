#pragma once

#include <types.h>
#include <game/mLib/m_wipe_fader.hpp>

// The circle wipe fader (dWipeFader_c) and its wipe texture. Source: src/dol/game/d_wipe_fader.cpp
// (.text 8017CD3C..8017D09C). The class name is from the RTTI; the others are inferred.

// A wipe fader whose circle is centered on (mCenterX, mCenterY). Created by the fader setup at 8008B930.
class dWipeFader_c : public mWipeFader_c {
public:
    dWipeFader_c(mColor col, mFaderBase_c::EStatus status); // 8017CD58

    virtual void calcMtx(); // 8017CF80

    void init(); // 8017CDC4: draws the wipe texture (a circle) and uses it

    // HACK: only calcMtx uses it, to match its load order (see notes/d_wipe_fader.txt).
    f32 getCenterY() const { return mCenterY; }

    /* 0x58 */ f32 mCenterX; // in texture coordinates
    /* 0x5C */ f32 mCenterY;
}; // size 0x60

// The wipe texture: 4-bit (I4), a transparent circle of radius 63 on an opaque square. Also used by
// dSideWipeFader_c.
void *getWipeTexture(); // 8017CD3C
int getWipeTextureWidth(); // 8017CD48
int getWipeTextureHeight(); // 8017CD50
