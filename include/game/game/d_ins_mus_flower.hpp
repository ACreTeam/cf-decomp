#pragma once

// The museum insects on the flowers: the mantises, the snail and the ladybug.

#include <game/game/d_insect_museum.hpp>
#include <float.h>
#include <cmath>

struct dInsMusFlowerParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mWaitTimeMax;
    /* 0x10 */ f32 mWaitTimeMin;
    /* 0x14 */ f32 mWalkDistMax;
    /* 0x18 */ f32 mWalkDistMin;
    /* 0x1C */ f32 mWalkSpeed;
    /* 0x20 */ f32 mWalkTurnRate;
    /* 0x24 */ f32 mGravity;
    /* 0x28 */ f32 mTurnSpeed;
    /* 0x2C */ f32 mThreatTime;
    /* 0x30 */ f32 mWalkAnmRate;
    /* 0x34 */ f32 mShadowSize;
    /* 0x38 */ f32 mScareRadius;
    /* 0x3C */ int mScareLimit;
}; // size 0x40

class dInsMusFlower_c : public dInsMusBase_c {
public:
    typedef void (dInsMusFlower_c::*StateFunc)();

    enum {
        AREA_SNAIL = 1,
        AREA_LADYBUG = 11,
        SNAIL_HIDE_END_FRAME = 58,
        MANTIS_PAUSE_TIME = 600,
        MANTIS_SWAY_TIME = 300,
    };

    dInsMusFlower_c();
    virtual ~dInsMusFlower_c();
    virtual BOOL draw();
    virtual void calcMtx();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initRest();
    void initWait();
    void initWalk();
    void initTurn();
    void initThreat();
    void stateRest();
    void stateWait();
    void stateWalk();
    void stateTurn();
    void stateThreat();
    bool isMantis();
    bool isSnail();
    bool isLadybug();
    mAng getWalkAngle();
    f32 getWalkDist() const;
    void getWalkTarget(mVec3_c *target);

    bool isAnmStopped() { return std::fabs(mAnmChr.getRate()) < FLT_EPSILON; }

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ mVec3_c mUnitCenter;
    /* 0x228 */ u16 mRestTime;
    /* 0x22A */ u16 mWaitTime;
    /* 0x22C */ u16 mWalkTime;
    /* 0x22E */ u16 _22E;
    /* 0x230 */ u16 _230;
    /* 0x232 */ mAng mTargetAngle;
}; // size 0x234
