#pragma once

// dInsMusTarantula_c: the tarantula and the scorpion of the museum's insect rooms.

#include <game/game/d_insect_museum.hpp>
#include <game/snd/snd_id.hpp>

struct dInsMusTarantulaParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mWaitTimeMax;
    /* 0x10 */ f32 mWaitTimeMin;
    /* 0x14 */ f32 mWalkTimeMax; // unused: initWalk uses the wait times
    /* 0x18 */ f32 mWalkTimeMin;
    /* 0x1C */ f32 mWalkDist;
    /* 0x20 */ f32 mWalkSpeed;
    /* 0x24 */ f32 mWalkBrake;
    /* 0x28 */ f32 mWalkTurnRate;
    /* 0x2C */ f32 mWalkAnmRate;
    /* 0x30 */ f32 mTurnAnmRate;
    /* 0x34 */ f32 mTurnSpeed;
    /* 0x38 */ f32 mThreatAnmRate;
    /* 0x3C */ f32 mJumpSpeedY;
    /* 0x40 */ f32 mJumpSpeedZ;
    /* 0x44 */ f32 mJumpBrake;
    /* 0x48 */ f32 mRunAccel;
    /* 0x4C */ f32 mRunSpeedMax;
    /* 0x50 */ f32 mShadowSize;
    /* 0x54 */ f32 mScareRadius;
    /* 0x58 */ int mScareLimit;
    /* 0x5C */ int mArea;
}; // size 0x60

class dInsMusTarantula_c : public dInsMusBase_c {
public:
    typedef void (dInsMusTarantula_c::*StateFunc)();

    enum State_e {
        STATE_WAIT,
        STATE_WALK,
        STATE_TURN_TO_PLAYER,
        STATE_THREAT,
        STATE_TURN_BACK,
    };

    enum Pose_e {
        POSE_JUMP,
        POSE_RUN,
        POSE_CIRCLE,
        POSE_STAND,
    };

    enum {
        JUMP_SOUND_TIME = 5,
        RUN_TIME = 10,
        CIRCLE_TIME = 120,
        STAND_TIME = 120,
    };

    dInsMusTarantula_c();
    virtual ~dInsMusTarantula_c();
    virtual void calcMtx();
    virtual BOOL draw();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initWalk();
    void initTurnToPlayer();
    void initThreat();
    void initTurnBack();
    void initJump();
    void initRun();
    void initCircle();
    void initStand();
    void stateWait();
    void stateWalk();
    void stateThreat();
    void stateTurnToPlayer();
    void stateTurnBack();
    void stateJump();
    void stateRun();
    void stateCircle();
    void stateStand();
    mAng getWalkAngle();
    void getWalkTarget(mVec3_c *target);
    bool isTarantula();
    bool isPairDonated();

    void holdRunSound() {
        if (isTarantula() == true) {
            mSnd.holdSound(SE_TarantulaRun);
        } else {
            mSnd.holdSound(SE_ScorpionRun);
        }
    }

    /* 0x210 */ int mStateId;
    /* 0x214 */ StateFunc mState;
    /* 0x220 */ u16 mWaitTime;
    /* 0x222 */ u16 mWalkTime;
    /* 0x224 */ u16 mCircleTime;
    /* 0x226 */ u16 mStandTime;
    /* 0x228 */ f32 mRunSpeed;
    /* 0x22C */ mAng mBackAngle;
    /* 0x22E */ mAng mCircleAngle;
    /* 0x230 */ f32 mCircleRadius;
    /* 0x234 */ mVec3_c mCircleCenter;
}; // size 0x240
