#pragma once

// The museum's locusts, crickets, bell cricket and grasshopper (dInsMusGrasshopper_c).

#include <game/game/d_insect_museum.hpp>

struct dInsMusGrasshopperParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mWaitTimeMax;
    /* 0x10 */ f32 mWaitTimeMin;
    /* 0x14 */ f32 mChirpAnmRate;
    /* 0x18 */ f32 mTurnSpeed;
    /* 0x1C */ f32 mTurnAnmRate;
    /* 0x20 */ f32 mJumpDistMax;
    /* 0x24 */ f32 mJumpDistMin;
    /* 0x28 */ f32 mJumpFriction;
    /* 0x2C */ f32 mGravity;
    /* 0x30 */ f32 mJumpAnmRate;
    /* 0x34 */ f32 mScale;
    /* 0x38 */ f32 mShadowSizeX;
    /* 0x3C */ f32 mShadowSizeZ;
    /* 0x40 */ f32 mScareRadius;
    /* 0x44 */ int mScareLimit;
    /* 0x48 */ int mArea;
}; // size 0x4C

class dInsMusGrasshopper_c : public dInsMusBase_c {
public:
    typedef void (dInsMusGrasshopper_c::*StateFunc)();

    enum { JUMP_TIME = 25 };

    dInsMusGrasshopper_c();
    virtual ~dInsMusGrasshopper_c();
    virtual BOOL draw();
    virtual void calcMtx();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initTurn();
    void initJump();
    void stateWait();
    void stateTurn();
    void stateJump();
    mAng getJumpAngle() const;
    f32 getJumpDist() const;
    void getJumpTarget(mVec3_c *target);
    bool isChirping();
    f32 getJumpSpeedY(f32 gravity, u16 time);
    f32 getJumpSpeedXZ(f32 dist, f32 friction);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ u16 mWaitTime;
    /* 0x21E */ u16 mJumpTime;
    /* 0x220 */ mAng mTargetAngle;
}; // size 0x224
