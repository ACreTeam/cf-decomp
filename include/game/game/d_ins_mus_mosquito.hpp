#pragma once

// dInsMusMosquito_c: the mosquito of the museum's insect rooms, circling and biting the player.

#include <game/game/d_insect_museum.hpp>

struct dInsMusMosquitoParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mFlyDistMax;
    /* 0x10 */ f32 mFlyDistMin;
    /* 0x14 */ f32 mSpeed;
    /* 0x18 */ f32 mBobSpeed;
    /* 0x1C */ f32 mBobHeightMin;
    /* 0x20 */ f32 mBobHeightMax;
    /* 0x24 */ f32 mTurnDeg;
    /* 0x28 */ f32 mRollSpeed;
    /* 0x2C */ f32 mRollMax;
    /* 0x30 */ f32 mCloseRadius;
    /* 0x34 */ f32 mCircleSpeed;
    /* 0x38 */ f32 mShadowSize;
    /* 0x3C */ f32 mScareRadius;
}; // size 0x40

class dInsMusMosquito_c : public dInsMusBase_c {
public:
    typedef void (dInsMusMosquito_c::*StateFunc)();

    enum { BITE_WAIT_TIME = 180 };

    dInsMusMosquito_c();
    virtual ~dInsMusMosquito_c();
    virtual BOOL draw();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initFly();
    void initFollow();
    void initCircle();
    void stateFly();
    void stateFollow();
    void stateCircle();
    void bob();
    static mAng getFlyAngle();
    f32 getFlyDist() const;
    void getFlyTarget(mVec3_c *target);
    bool isPlayerNear();
    bool isPlayerClose();
    bool isCircleClockwise();

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ f32 mBaseY;
    /* 0x220 */ s16 mBobAngle;
    /* 0x224 */ f32 mBobHeight;
    /* 0x228 */ u16 mFlyTime;
    /* 0x22A */ s16 _22A;
    /* 0x22C */ s16 _22C;
    /* 0x22E */ bool mBitten;
    /* 0x230 */ mAng mCircleAngle;
    /* 0x234 */ mVec3_c mPlayerPos;
    /* 0x240 */ f32 mCircleRadius;
    /* 0x244 */ s16 mRollAngle;
    /* 0x246 */ bool mClockwise;
}; // size 0x248
