#pragma once

// The museum's beetles, stags, walking stick and oak silk moth (dInsMusBeetle_c).

#include <game/game/d_insect_museum.hpp>
#include <lib/egg/math/eggQuat.h>

struct dInsMusBeetleParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mWaitTimeMax;
    /* 0x10 */ f32 mWaitTimeMin;
    /* 0x14 */ f32 mClimbTimeMax;
    /* 0x18 */ f32 mClimbTimeMin;
    /* 0x1C */ f32 mClimbSpeed;
    /* 0x20 */ f32 mClimbTop;
    /* 0x24 */ f32 mClimbBottom;
    /* 0x28 */ f32 mClimbAnmRate;
    /* 0x2C */ f32 mRestWaitTimeMax;
    /* 0x30 */ f32 mRestWaitTimeMin;
    /* 0x34 */ f32 mRestClimbTimeMax;
    /* 0x38 */ f32 mRestClimbTimeMin;
    /* 0x3C */ f32 mScale;
    /* 0x40 */ f32 mShadowSize;
    /* 0x44 */ f32 mScareRadius;
    /* 0x48 */ int mScareLimit;
}; // size 0x4C

class dInsMusBeetle_c : public dInsMusBase_c {
public:
    typedef void (dInsMusBeetle_c::*StateFunc)();

    enum {
        WIGGLE_STEP = 45,
        WIGGLE_MAX = 0xAAB,
        STUMP_AREA = 20,
        POSE_STILL_TIME = 240,
        POSE_END_TIME = 300,
    };

    dInsMusBeetle_c();
    virtual ~dInsMusBeetle_c();
    virtual void calcMtx();
    virtual BOOL draw();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initClimbUp();
    void initClimbDown();
    void stateWait();
    void stateClimbUp();
    void stateClimbDown();
    bool isWalkingStick();
    bool isOakSilkMoth();
    bool isGoldenStag();
    bool isHerculesBeetle();
    void wiggle();
    void initBrake();
    void initWalk();
    void initPose();
    void stateWalk();
    void stateBrake();
    void statePose();
    void setWalkTarget();
    void getWalkTarget(mVec3_c *target);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ bool mActive;
    /* 0x220 */ f32 mBaseY;
    /* 0x224 */ s16 mWiggleY;
    /* 0x226 */ s16 mWiggleZ;
    /* 0x228 */ bool mWiggleUp;
    /* 0x22C */ EGG::Quatf mWiggleRot;
    /* 0x23C */ u16 mWaitTime;
    /* 0x23E */ u16 mClimbUpTime;
    /* 0x240 */ u16 mClimbDownTime;
    /* 0x242 */ u16 mBrakeTime;
    /* 0x244 */ u16 mWalkTime;
    /* 0x248 */ f32 mAnmRate;
}; // size 0x24C
