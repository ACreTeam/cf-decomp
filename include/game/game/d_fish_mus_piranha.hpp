#pragma once

// The piranha: charges at a nearby player and bounces off the glass.

#include <game/game/d_fish_mus_base_move.hpp>

class dFishMusPiranha_c : public dFishMusBaseMove_c {
public:
    typedef void (dFishMusPiranha_c::*StateFunc)();

    enum {
        MODE_ATTACK = MODE_DANGEROUS,
    };

    dFishMusPiranha_c();
    virtual ~dFishMusPiranha_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual BOOL draw();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void initWait();
    virtual void initSwim();

    static bool isZero(f32 v) { return std::fabs(v) <= FLT_EPSILON; }

    void init();
    void stateSwim();
    void checkPlayer();
    void stateAttack();
    void startBounce();
    void stateBounce();
    void stateAttackWait();
    void correctPos();
    BOOL isPlayerNear();
    BOOL isFacingPlayer(const mVec3_c &push);
    BOOL isPushedToPlayer(const mVec3_c &push);
    void calcAnm();

    /* 0x434 */ StateFunc mState;
    /* 0x440 */ dSndObjSimple_c mSnd;
    /* 0x470 */ f32 mFrame;
    /* 0x474 */ f32 mLoopEnd;
    /* 0x478 */ f32 mLoopStart;
    /* 0x47C */ f32 mFrameRate;
    /* 0x480 */ mVec3_c mPrevPos;
}; // size 0x48C
