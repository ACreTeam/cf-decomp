#pragma once

// dInsMusFlea_c: the flea of the museum's insect rooms.

#include <game/game/d_insect_museum.hpp>

struct dInsMusFleaParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mWaitTimeMax;
    /* 0x10 */ f32 mWaitTimeMin;
    /* 0x14 */ f32 mJumpSpeed;
    /* 0x18 */ f32 mJumpDistMax;
    /* 0x1C */ f32 mJumpDistMin;
    /* 0x20 */ f32 mGravity;
    /* 0x24 */ f32 mJumpHeightRate;
    /* 0x28 */ f32 mShadowSize;
    /* 0x2C */ int mArea;
}; // size 0x30

class dInsMusFlea_c : public dInsMusBase_c {
public:
    typedef void (dInsMusFlea_c::*StateFunc)();

    dInsMusFlea_c();
    virtual ~dInsMusFlea_c();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initJump();
    void stateWait();
    void stateJump();
    mAng getJumpAngle();
    f32 getJumpDist() const;
    void getJumpTarget(mVec3_c *target);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ u16 mWaitTime;
    /* 0x21E */ u16 mJumpTime;
}; // size 0x220
