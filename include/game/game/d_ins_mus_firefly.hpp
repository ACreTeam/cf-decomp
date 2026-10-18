#pragma once

// The museum firefly.

#include <game/game/d_insect_museum.hpp>
#include <game/game/d_effect.hpp>

struct dInsMusFireflyParam_c {
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
    /* 0x34 */ f32 mFlyTimeMax;
    /* 0x38 */ f32 mFlyTimeMin;
    /* 0x3C */ f32 mRestTimeMax;
    /* 0x40 */ f32 mRestTimeMin;
    /* 0x44 */ f32 mTakeOffSpeed;
    /* 0x48 */ f32 mTakeOffLift;
    /* 0x4C */ f32 mTakeOffBrake;
    /* 0x50 */ f32 mTakeOffAnmRate;
    /* 0x54 */ Vec mRestOffset;
    /* 0x60 */ f32 mShadowSize;
    /* 0x64 */ f32 mScareRadius;
    /* 0x68 */ int mScareLimit;
    /* 0x6C */ int mArea;
}; // size 0x70

class dInsMusFirefly_c : public dInsMusBase_c {
public:
    typedef void (dInsMusFirefly_c::*StateFunc)();

    enum State_e {
        STATE_FLY,
        STATE_RETURN,
        STATE_LAND,
        STATE_REST,
        STATE_TAKE_OFF,
    };

    enum {
        TAKE_OFF_TIME = 70,
        LAND_EXTRA_TIME = 60,
    };

    class glow_c {
    public:
        glow_c();
        ~glow_c();
        void init();
        void draw(const mVec3_c *pos);

        /* 0x0 */ int mPhase;
        /* 0x4 */ int mSpeed;
        /* 0x8 */ u8 mAlpha;
    }; // size 0xC

    dInsMusFirefly_c();
    virtual ~dInsMusFirefly_c();
    virtual void calcMtx();
    virtual BOOL create(nw4r::g3d::ResFile &res);
    virtual BOOL draw();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initFly();
    void initReturn();
    void initLand();
    void initRest();
    void initTakeOff();
    void stateFly();
    void stateReturn();
    void stateLand();
    void stateRest();
    void stateTakeOff();
    void bob();
    void bobFade(f32 rate);
    mAng getFlyAngle() const;
    f32 getFlyDist() const;
    void getFlyTarget(mVec3_c *target);
    bool isFlyTimeOver() const;
    void updateBaseY();
    f32 getFlyY();

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ State_e mStateId;
    /* 0x220 */ mAng mBobAngle;
    /* 0x224 */ f32 mBobHeight;
    /* 0x228 */ f32 mBaseY;
    /* 0x22C */ f32 _22C;
    /* 0x230 */ u16 mFlyTime;
    /* 0x232 */ u16 mReturnTime;
    /* 0x234 */ u16 mLandTime;
    /* 0x236 */ u16 mRestTime;
    /* 0x238 */ u16 mTakeOffTime;
    /* 0x23A */ u16 mFlyTimeLimit;
    /* 0x23C */ u16 mFlyTimer;
    /* 0x240 */ glow_c mGlow;
    /* 0x24C */ dLevelEffect_c mEffect;
}; // size 0x2E0
