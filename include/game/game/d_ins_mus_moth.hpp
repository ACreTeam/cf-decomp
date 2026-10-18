#pragma once

// The museum moth.

#include <game/game/d_insect_museum.hpp>

struct dInsMusMothParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mFlySpeed;
    /* 0x10 */ f32 mFlyAccel;
    /* 0x14 */ f32 mFlyBrake;
    /* 0x18 */ f32 mFlyTurnRate;
    /* 0x1C */ f32 mFlyDistMax;
    /* 0x20 */ f32 mFlyDistMin;
    /* 0x24 */ f32 mBobSpeed;
    /* 0x28 */ f32 mBobHeightMin;
    /* 0x2C */ f32 mBobHeightMax;
    /* 0x30 */ f32 mFlyAnmRate;
    /* 0x34 */ f32 mShadowSize;
    /* 0x38 */ f32 mScareRadius;
    /* 0x3C */ int mScareLimit;
    /* 0x40 */ f32 mScale;
    /* 0x44 */ Vec mRestOffset;
    /* 0x50 */ int mArea;
}; // size 0x54

class dInsMusMoth_c : public dInsMusBase_c {
public:
    typedef void (dInsMusMoth_c::*StateFunc)();

    dInsMusMoth_c();
    virtual ~dInsMusMoth_c();
    virtual void calcMtx();
    virtual BOOL create(nw4r::g3d::ResFile &res);
    virtual BOOL draw();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initFly();
    void initRest();
    void stateFly();
    void stateRest();
    void fly(f32 accel, f32 brake, f32 turnRate);
    void bob();
    void updateBaseY();
    f32 getFlyY();
    mAng getFlyAngle();
    f32 getFlyDist() const;
    void getFlyTarget(mVec3_c *target);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ BOOL mIsResting;
    /* 0x220 */ mAng mBobAngle;
    /* 0x224 */ f32 mBobHeight;
    /* 0x228 */ f32 mBaseY;
    /* 0x22C */ u16 mFlyTime;
    /* 0x22E */ u16 _22E;
    /* 0x230 */ mAng mFlutterAngle;
}; // size 0x234
