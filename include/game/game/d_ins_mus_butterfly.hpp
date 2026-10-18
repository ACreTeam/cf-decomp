#pragma once

// The museum's butterflies (dInsMusButterfly_c).

#include <game/game/d_insect_museum.hpp>

struct dInsMusButterflyParam_c {
    /* 0x00 */ Vec mFlyOffset;
    /* 0x0C */ f32 mFlySpeed;
    /* 0x10 */ f32 mFlyAccel;
    /* 0x14 */ f32 mFlyBrake;
    /* 0x18 */ f32 mTurnRate;
    /* 0x1C */ f32 mFlyDistMax;
    /* 0x20 */ f32 mFlyDistMin;
    /* 0x24 */ f32 mBobSpeed;
    /* 0x28 */ f32 mBobHeightMin;
    /* 0x2C */ f32 mBobHeightMax;
    /* 0x30 */ f32 mFlyAnmRate;
    /* 0x34 */ f32 mLandAnmRate;
    /* 0x38 */ f32 mTakeOffSpeed;
    /* 0x3C */ f32 mTakeOffLift;
    /* 0x40 */ f32 mTakeOffBrake;
    /* 0x44 */ f32 mTakeOffAnmRate;
    /* 0x48 */ f32 mFlyTimeMax;
    /* 0x4C */ f32 mFlyTimeMin;
    /* 0x50 */ f32 mRestTimeMax;
    /* 0x54 */ f32 mRestTimeMin;
    /* 0x58 */ f32 mShadowSizeX;
    /* 0x5C */ f32 mShadowSizeZ;
    /* 0x60 */ f32 mScareRadius;
    /* 0x64 */ int mScareLimit;
    /* 0x68 */ f32 mScale;
    /* 0x6C */ Vec mRestOffset;
    /* 0x78 */ int mArea;
}; // size 0x7C

class dInsMusButterfly_c : public dInsMusBase_c {
public:
    typedef void (dInsMusButterfly_c::*StateFunc)();

    enum State_e {
        STATE_FLY,
        STATE_APPROACH,
        STATE_LAND,
        STATE_REST,
        STATE_TAKE_OFF,
    };

    enum {
        TAKE_OFF_TIME = 70,
        LAND_EXTRA_TIME = 60,
    };

    dInsMusButterfly_c();
    virtual ~dInsMusButterfly_c();
    virtual void calcMtx();
    virtual BOOL create(nw4r::g3d::ResFile &res);
    virtual BOOL draw();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initFly();
    void initApproach();
    void initLand();
    void initRest();
    void initTakeOff();
    void stateFly();
    void stateApproach();
    void stateLand();
    void stateRest();
    void stateTakeOff();
    void calcBob();
    void calcBob(f32 rate);
    void calcBaseY();
    f32 getBaseTargetY();
    mAng getFlyAngle();
    f32 getFlyDist() const;
    void getFlyTarget(mVec3_c *target);
    BOOL isFlyTimeOver();
    bool isNearRestSpot();

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ int mStateId;
    /* 0x220 */ s16 mBobAngle;
    /* 0x224 */ f32 mBobHeight;
    /* 0x228 */ f32 mBaseY;
    /* 0x22C */ u16 mFlyTime;
    /* 0x22E */ u16 mApproachTime;
    /* 0x230 */ u16 mLandTime;
    /* 0x232 */ u16 mRestTime;
    /* 0x234 */ u16 mTakeOffTime;
    /* 0x236 */ u16 mFlyTimeLimit;
    /* 0x238 */ u16 mFlyTimer;
    /* 0x23A */ s16 mFlapAngle;
}; // size 0x23C
