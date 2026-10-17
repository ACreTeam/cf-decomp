#pragma once

// The eel: a long body kept inside the tank by four of its nodes.

#include <game/game/d_fish_museum.hpp>

class dFishMusEel_c : public dFishMusBase_c {
public:
    typedef void (dFishMusEel_c::*StateFunc)();

    enum {
        NODE_NUM = 7,
        CORNER_NUM = 4,
    };

    dFishMusEel_c();
    virtual ~dFishMusEel_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual BOOL doDelete();
    virtual void addPos(const mVec3_c &push);

    mVec3_c getHitCorner(u32 i) { return mCorners[i & 3]; }

    void init();
    void initWait();
    void stateWait();
    void initSwim();
    void stateSwim();
    void setWaitTimer();
    void setSwimTimer();
    void turn();
    void calcMove();
    void calcAnmRate();
    BOOL isBent();
    void calcCorners();
    mVec3_c getWallPush(mVec3_c pos);
    mVec3_c getMaxPush(mVec3_c push0, mVec3_c push1, mVec3_c push2, mVec3_c push3);
    mVec3_c getMaxAbs(mVec3_c a, mVec3_c b);

    /* 0x360 */ StateFunc mState;
    /* 0x36C */ mVec3_c mTargetPos;
    /* 0x378 */ s16 mTargetAngleY;
    /* 0x37C */ mVec3_c mCorners[NODE_NUM]; // only CORNER_NUM used
}; // size 0x3D0
