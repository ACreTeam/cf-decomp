#pragma once

// The museum mole cricket.

#include <game/game/d_insect_museum.hpp>

struct dInsMusMoleCricketParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mChirpRadius;
    /* 0x10 */ f32 mJumpSpeed;
    /* 0x14 */ f32 mJumpBrake;
    /* 0x18 */ f32 mJumpSpeedY;
    /* 0x1C */ f32 mGravity;
    /* 0x20 */ f32 mJumpAnmRate;
    /* 0x24 */ f32 mWalkDistMax;
    /* 0x28 */ f32 mWalkDistMin;
    /* 0x2C */ f32 mWalkSpeed;
    /* 0x30 */ f32 mWalkAccel;
    /* 0x34 */ f32 mWalkBrake;
    /* 0x38 */ f32 mWalkTurnRate;
    /* 0x3C */ f32 mWalkAnmRate;
    /* 0x40 */ f32 mWalkTime;
    /* 0x44 */ f32 mHideTime;
    /* 0x48 */ f32 mScareRadius;
    /* 0x4C */ f32 mShadowSizeX;
    /* 0x50 */ f32 mShadowSizeZ;
    /* 0x54 */ int mScareLimit;
    /* 0x58 */ int mArea;
}; // size 0x5C

class dInsMusMoleCricket_c : public dInsMusBase_c {
public:
    typedef void (dInsMusMoleCricket_c::*StateFunc)();

    enum {
        JUMP_EFFECT_TIME = 4,
        JUMP_SHADOW_TIME = 20,
        JUMP_LAND_TIME = 30,
        JUMP_END_TIME = 90,
        DIG_TILT_TIME = 70,
    };

    dInsMusMoleCricket_c();
    virtual ~dInsMusMoleCricket_c();
    virtual void calcMtx();
    virtual BOOL draw();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initUnderground();
    void initJump();
    void initWalk();
    void initDig();
    void initHide();
    void stateUnderground();
    void stateJump();
    void stateWalk();
    void stateDig();
    void stateHide();
    mAng getWalkAngle() const;
    f32 getWalkDist() const;
    void getWalkTarget(mVec3_c *target);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ f32 mShadowSizeX;
    /* 0x220 */ f32 mShadowSizeZ;
    /* 0x224 */ u16 mWalkTime;
    /* 0x226 */ u16 mWalkTimer;
    /* 0x228 */ u16 mHideTime;
}; // size 0x22C
