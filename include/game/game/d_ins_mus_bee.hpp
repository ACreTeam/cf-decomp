#pragma once

// The museum honeybee and fly.

#include <game/game/d_insect_museum.hpp>

struct dInsMusBeeParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mFlyDistMax;
    /* 0x10 */ f32 mFlyDistMin;
    /* 0x14 */ f32 mFlySpeed;
    /* 0x18 */ f32 mFlyAccel;
    /* 0x1C */ f32 mFlyBrake;
    /* 0x20 */ f32 mFlyTurnRate;
    /* 0x24 */ f32 mBobSpeed;
    /* 0x28 */ f32 mBobHeightMin;
    /* 0x2C */ f32 mBobHeightMax;
    /* 0x30 */ f32 mTakeOffAnmRate;
    /* 0x34 */ f32 mWobbleSpeed;
    /* 0x38 */ f32 mRollMax;
    /* 0x3C */ f32 mHoverAccel;
    /* 0x40 */ f32 mRestTimeMax;
    /* 0x44 */ f32 mRestTimeMin;
    /* 0x48 */ f32 mHoverTurnRate;
    /* 0x4C */ f32 mFlyTimeMax;
    /* 0x50 */ f32 mFlyTimeMin;
    /* 0x54 */ f32 mTakeOffSpeed;
    /* 0x58 */ f32 mTakeOffLift;
    /* 0x5C */ f32 mTakeOffBrake;
    /* 0x60 */ f32 mShadowSize;
    /* 0x64 */ f32 mScareRadius;
    /* 0x68 */ int mScareLimit;
    /* 0x6C */ Vec mRestOffset;
    /* 0x78 */ int mArea;
}; // size 0x7C

class dInsMusBee_c : public dInsMusBase_c {
public:
    typedef void (dInsMusBee_c::*StateFunc)();

    enum State_e {
        STATE_FLY,
        STATE_RETURN,
        STATE_LAND,
        STATE_REST,
        STATE_SLEEP,
        STATE_TAKE_OFF,
    };

    enum {
        TAKE_OFF_TIME = 70,
        LAND_EXTRA_TIME = 60,
    };

    dInsMusBee_c();
    virtual ~dInsMusBee_c();
    virtual BOOL create(nw4r::g3d::ResFile &res);
    virtual BOOL draw();
    virtual void calcMtx();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initFly();
    void initReturn();
    void initLand();
    void initRest();
    void initSleep();
    void initTakeOff();
    void stateFly();
    void stateReturn();
    void stateLand();
    void stateRest();
    void stateSleep();
    void stateTakeOff();
    bool isHouseFly();
    bool isFlyTimeOver();
    void fly(f32 accel, f32 brake, f32 turnRate, mAng roll);
    void hover();
    void bob();
    void bobFade(f32 rate);
    void updateBaseY();
    f32 getFlyY();
    mAng getFlyAngle();
    f32 getFlyDist() const;
    void getFlyTarget(mVec3_c *target);
    bool isAwayFromHome();
    bool isFacingTarget();

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ State_e mStateId;
    /* 0x220 */ f32 mBaseY;
    /* 0x224 */ f32 mBobHeight;
    /* 0x228 */ mAng mBobAngle;
    /* 0x22A */ u16 mFlyTimeLimit;
    /* 0x22C */ u16 mFlyTimer;
    /* 0x22E */ u16 mFlyTime;
    /* 0x230 */ u16 mReturnTime;
    /* 0x232 */ u16 mLandTime;
    /* 0x234 */ u16 mRestTime;
    /* 0x236 */ u16 mTakeOffTime;
    /* 0x238 */ mAng mTargetAngle;
    /* 0x23A */ mAng mWobbleAngle;
    /* 0x23C */ mAng mRoll;
}; // size 0x240
