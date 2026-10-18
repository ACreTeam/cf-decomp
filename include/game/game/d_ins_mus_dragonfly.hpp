#pragma once

// The museum's dragonflies (dInsMusDragonfly_c).

#include <game/game/d_insect_museum.hpp>

struct dInsMusDragonflyParam_c {
    /* 0x00 */ Vec mFlyOffset;
    /* 0x0C */ f32 mTurnSpeed;
    /* 0x10 */ f32 mHoverHeight;
    /* 0x14 */ f32 mHoverPeriod;
    /* 0x18 */ f32 mFlyDistMax;
    /* 0x1C */ f32 mFlyDistMin;
    /* 0x20 */ f32 mBrake;
    /* 0x24 */ f32 mGravity;
    /* 0x28 */ f32 mAnmRate;
    /* 0x2C */ f32 mRestTime;
    /* 0x30 */ f32 mFlyTimeMax;
    /* 0x34 */ f32 mFlyTimeMin;
    /* 0x38 */ f32 mTakeOffHeight;
    /* 0x3C */ f32 mShadowSizeX;
    /* 0x40 */ f32 mShadowSizeZ;
    /* 0x44 */ f32 mScareRadius;
    /* 0x48 */ int mScareLimit;
    /* 0x4C */ Vec mRestOffset;
    /* 0x58 */ int mArea;
}; // size 0x5C

class dInsMusDragonfly_c : public dInsMusBase_c {
public:
    typedef void (dInsMusDragonfly_c::*StateFunc)();

    enum State_e {
        STATE_TURN,
        STATE_FLY,
        STATE_TURN_HOME,
        STATE_LAND,
        STATE_REST,
        STATE_TAKE_OFF,
    };

    enum {
        FLY_TIME_BIG = 100,
        FLY_TIME = 150,
    };

    dInsMusDragonfly_c();
    virtual ~dInsMusDragonfly_c();
    virtual BOOL create(nw4r::g3d::ResFile &res);
    virtual BOOL draw();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initTurn();
    void initFly();
    void initTurnHome();
    void initLand();
    void initRest();
    void initTakeOff();
    void stateTurn();
    void stateFly();
    void stateTurnHome();
    void stateLand();
    void stateRest();
    void stateTakeOff();
    void updateHover();
    mAng getFlyAngle() const;
    f32 getFlyDist() const;
    void getFlyTarget(mVec3_c *target);
    bool isBig() const;
    bool isDarner() const;
    bool isPetaltail() const;
    bool isFlyTimeOver();
    bool isAwayFromHome();
    bool isFarFromHome();
    f32 getFlyY();

    mAng getAngleY() const { return mAngle.y; }

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ int mStateId;
    /* 0x220 */ int mPrevStateId;
    /* 0x224 */ f32 mBaseY;
    /* 0x228 */ f32 mHoverRate;
    /* 0x22C */ mAng mHoverSpeed;
    /* 0x22E */ mAng mHoverPhase;
    /* 0x230 */ s16 _230;
    /* 0x232 */ u16 mFlyTime;
    /* 0x234 */ u16 mLandTime;
    /* 0x236 */ u16 mRestTime;
    /* 0x238 */ u16 mFlyTimer;
    /* 0x23A */ u16 mFlyTimeLimit;
    /* 0x23C */ s16 mTargetAngle;
}; // size 0x240
