#pragma once

// The koi: swims like the plain fish, and now and then rises to the surface and leaps.

#include <game/game/d_fish_mus_base_move.hpp>

class dFishMusKoi_c : public dFishMusBaseMove_c {
public:
    typedef void (dFishMusKoi_c::*StateFunc)();

    dFishMusKoi_c();
    virtual ~dFishMusKoi_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void initWait();
    virtual void initSwim();

    void init();
    void initNextAction();
    void stateWait();
    void initRise();
    void stateRise();
    void initJump();
    void stateJump();
    void initFall();
    void stateFall();
    void initSettle();
    void stateSettle();
    void turn();
    void stateStop();
    void stateSwim();

    /* 0x434 */ StateFunc mState;
    /* 0x440 */ mVec3_c mPrevPos;
    /* 0x44C */ int mJumpNum;   // leaps left
    /* 0x450 */ int mJumpTimer;
}; // size 0x454
