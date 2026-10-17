#pragma once

// The crawfish and the lobster: walk on the tank floor.

#include <game/game/d_fish_mus_common_move.hpp>

class dFishMusLobster_c : public dFishMusCommonMove_c {
public:
    typedef void (dFishMusLobster_c::*StateFunc)();

    dFishMusLobster_c();
    virtual ~dFishMusLobster_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void initWait();
    virtual void initSwim();

    void init();
    void stateWait();
    void stateWalk();
    void turn();
    void walk();
    void calcAnmRate();
    void escape();

    /* 0x418 */ StateFunc mState;
    /* 0x424 */ mAng mWalkAngleY;
    /* 0x428 */ mVec3_c mWalkTarget;
    /* 0x434 */ mVec3_c mWalkMin;
    /* 0x440 */ mVec3_c mWalkMax;
}; // size 0x44C
