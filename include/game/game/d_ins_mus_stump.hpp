#pragma once

// dInsMusStump_c: the longhorn and violin beetles on the museum's tree stumps.

#include <game/game/d_insect_museum.hpp>

struct dInsMusStumpParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mWaitTimeMax;
    /* 0x10 */ f32 mWaitTimeMin;
    /* 0x14 */ f32 mWalkSpeed;
    /* 0x18 */ f32 mWalkAccel;
    /* 0x1C */ f32 mWalkBrake;
    /* 0x20 */ f32 mWalkTurnRate;
    /* 0x24 */ f32 mWalkAnmRate;
    /* 0x28 */ f32 mWalkDistMax;
    /* 0x2C */ f32 mWalkDistMin;
    /* 0x30 */ f32 mWalkTimeMax;
    /* 0x34 */ f32 mWalkTimeMin;
    /* 0x38 */ f32 mScale;
    /* 0x3C */ f32 mShadowSize;
    /* 0x40 */ f32 mScareRadius;
    /* 0x44 */ int mScareLimit;
    /* 0x48 */ int mArea;
}; // size 0x4C

class dInsMusStump_c : public dInsMusBase_c {
public:
    typedef void (dInsMusStump_c::*StateFunc)();

    dInsMusStump_c();
    virtual ~dInsMusStump_c();
    virtual BOOL draw();
    virtual void calcMtx();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initWalk();
    void initSleep();
    void stateWait();
    void stateWalk();
    void stateSleep();
    mAng getWalkAngle();
    f32 getWalkDist() const;
    void getWalkTarget(mVec3_c *target);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ u16 mWaitTime;
    /* 0x21E */ u16 mMoveTime;
    /* 0x220 */ u16 mSleepTime;
    /* 0x222 */ u16 mWalkTime;
    /* 0x224 */ u16 mWalkTimer;
}; // size 0x228
