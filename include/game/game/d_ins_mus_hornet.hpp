#pragma once

// dInsMusHornet_c: the wasp of the museum's insect rooms.

#include <game/game/d_insect_museum.hpp>

struct dInsMusHornetParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mFlyDistMax;
    /* 0x10 */ f32 mFlyDistMin;
    /* 0x14 */ f32 mFlyAccelMinRate;
    /* 0x18 */ f32 mFlySpeed;
    /* 0x1C */ f32 mFlyAccel;
    /* 0x20 */ f32 mFlyBrake;
    /* 0x24 */ f32 mFlyTurnRate;
    /* 0x28 */ f32 mFlyAnmRate;
    /* 0x2C */ f32 mWobbleSpeed;
    /* 0x30 */ f32 mRollMax;
    /* 0x34 */ f32 mBobSpeed;
    /* 0x38 */ f32 mBobHeightMin;
    /* 0x3C */ f32 mBobHeightMax;
    /* 0x40 */ f32 mWaitBrake;
    /* 0x44 */ f32 mWaitTimeMax;
    /* 0x48 */ f32 mWaitTimeMin;
    /* 0x4C */ f32 mShadowSize;
    /* 0x50 */ f32 mScareRadius;
    /* 0x54 */ int mScareLimit;
    /* 0x58 */ int mArea;
}; // size 0x5C

class dInsMusHornet_c : public dInsMusBase_c {
public:
    typedef void (dInsMusHornet_c::*StateFunc)();

    dInsMusHornet_c();
    virtual ~dInsMusHornet_c();
    virtual BOOL create(nw4r::g3d::ResFile &res);
    virtual BOOL draw();
    virtual void calcMtx();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initFly();
    void initWait();
    void stateFly();
    void stateWait();
    void fly(f32 accel, f32 brake, f32 turnRate, mAng roll);
    void bob();
    mAng getFlyAngle();
    f32 getFlyDist() const;
    void getFlyTarget(mVec3_c *target);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ f32 mFlyAccel;
    /* 0x220 */ f32 mBaseY;
    /* 0x224 */ f32 mBobHeight;
    /* 0x228 */ s16 mBobAngle;
    /* 0x22A */ u16 mFlyTime;
    /* 0x22C */ u16 mWaitTime;
    /* 0x22E */ mAng mWobbleAngle;
    /* 0x230 */ mAng mRoll;
}; // size 0x234
