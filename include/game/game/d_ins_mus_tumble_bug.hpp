#pragma once

// dInsMusTumbleBug_c: the dung beetle of the museum's insect rooms, pushing its ball around.

#include <game/game/d_insect_museum.hpp>
#include <lib/egg/math/eggQuat.h>

struct dInsMusTumbleBugParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mWaitTimeMax;
    /* 0x10 */ f32 mWaitTimeMin;
    /* 0x14 */ f32 mPushTimeMax;
    /* 0x18 */ f32 mPushTimeMin;
    /* 0x1C */ f32 mWalkDistMax;
    /* 0x20 */ f32 mWalkDistMin;
    /* 0x24 */ f32 mSpeedMax;
    /* 0x28 */ f32 mAccel;
    /* 0x2C */ f32 mBrake;
    /* 0x30 */ f32 mTurnDeg;
    /* 0x34 */ f32 mAnmRate;
    /* 0x38 */ f32 mShadowSize;
    /* 0x3C */ int mArea;
}; // size 0x40

class dInsMusTumbleBug_c : public dInsMusBase_c {
public:
    typedef void (dInsMusTumbleBug_c::*StateFunc)();

    class ball_c {
    public:
        enum { RADIUS = 5 };

        ball_c();
        ~ball_c();
        void create(nw4r::g3d::ResFile res, mAllocator_c *allocator);
        void calc();
        void draw();
        void remove();
        void setPos(const mVec3_c &pos);
        void roll(const mVec3_c &pos);

        /* 0x00 */ mVec3_c mPos;
        /* 0x0C */ EGG::Quatf mRot;
        /* 0x1C */ m3d::smdl_c mMdl;
    }; // size 0x28

    enum {
        TILT_ANGLE = DEG_TO_ANGLE(30),
        BALL_DIST = 8,
    };

    dInsMusTumbleBug_c();
    virtual ~dInsMusTumbleBug_c();
    virtual BOOL create(nw4r::g3d::ResFile &res);
    virtual BOOL draw();
    virtual BOOL doDelete();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initPush();
    void stateWait();
    void statePush();
    mAng getWalkAngle();
    f32 getWalkDist() const;
    void getWalkTarget(mVec3_c *target);
    void setWalkTarget();

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ ball_c mBall;
    /* 0x244 */ f32 mPushSpeed;
    /* 0x248 */ u16 mWaitTime;
    /* 0x24A */ u16 mMoveTime;
    /* 0x24C */ u16 mPushTime;
    /* 0x24E */ u16 mPushTimer;
}; // size 0x250
