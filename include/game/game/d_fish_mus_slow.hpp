#pragma once

// Arowana, gar, arapaima: laps of the tank through 10 waypoints.

#include <game/game/d_fish_mus_big.hpp>

class dFishMusSlow_c : public dFishMusBig_c {
public:
    typedef void (dFishMusSlow_c::*StateFunc)();

    dFishMusSlow_c();
    virtual ~dFishMusSlow_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void initWait();

    void init();
    void stateWait();
    void stateSwim();
    void swimToPoint(u8 point);
    void swimToPoint0();
    void swimToPoint1();
    void swimToPoint2();
    void swimToPoint3();
    void swimToPoint4();
    void swimToPoint5();
    void swimToPoint6();
    void swimToPoint7();
    void swimToPoint8();
    void swimToPoint9();
    void escape();

    /* 0x440 */ StateFunc mState;
    /* 0x44C */ mVec3_c mWayPoint0;
    /* 0x458 */ mVec3_c mWayPoint1;
    /* 0x464 */ mVec3_c mWayPoint2;
    /* 0x470 */ mVec3_c mWayPoint3;
    /* 0x47C */ mVec3_c mWayPoint4;
    /* 0x488 */ mVec3_c mWayPoint5;
    /* 0x494 */ mVec3_c mWayPoint6;
    /* 0x4A0 */ mVec3_c mWayPoint7;
    /* 0x4AC */ mVec3_c mWayPoint8;
    /* 0x4B8 */ mVec3_c mWayPoint9;
}; // size 0x4C4
