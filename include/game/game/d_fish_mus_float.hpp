#pragma once

// Sea angel, jellyfish, seahorse: drift down and swim up in strokes.

#include <game/game/d_fish_museum.hpp>

class dFishMusFloat_c : public dFishMusBase_c {
public:
    typedef void (dFishMusFloat_c::*StateFunc)();

    dFishMusFloat_c();
    virtual ~dFishMusFloat_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual BOOL draw();
    virtual void addPos(const mVec3_c &push);

    bool isFishKind(s32 kind) const { return mFishIdx == kind; }

    void init();
    void initDrift();
    void stateDrift();
    void sway();
    void initDriftLong();
    void stateDriftLong();
    void initSwim();
    void stateSwim();
    void initSwimLong();
    void stateSwimLong();
    void setWaitTimer();
    void setSwimTimer();
    void setSwayAngle();
    void setTargetAngleY();
    void turnX(f32 maxStep, f32 minStep);
    void turnY();
    void calcMove();
    void calcAnmFrame();

    /* 0x360 */ StateFunc mState;
    /* 0x36C */ f32 mSwimSpeed;
    /* 0x370 */ f32 mAnmFrame;
    /* 0x374 */ f32 mStrokeFrames; // one stroke of the animation
    /* 0x378 */ f32 mSwayDeg;
    /* 0x37C */ mAng mPrevSway;
    /* 0x37E */ mAng mTargetAngleX;
    /* 0x380 */ mAng mSway; // pitch
    /* 0x384 */ f32 mTurnMaxStep; // degrees
    /* 0x388 */ f32 mTurnMinStep; // degrees
    /* 0x38C */ mAng mYawStep;
    /* 0x38E */ mAng mTargetAngleY;
    /* 0x390 */ u8 mSwayWait;
    /* 0x391 */ u8 mSwayPause;
}; // size 0x394
