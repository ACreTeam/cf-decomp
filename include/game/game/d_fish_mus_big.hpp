#pragma once

// Base of the big museum fish that swim laps along waypoints around the tank.

#include <game/game/d_fish_mus_common_move.hpp>

class dFishMusBig_c : public dFishMusCommonMove_c {
public:
    dFishMusBig_c();
    virtual ~dFishMusBig_c();
    virtual void addPos(const mVec3_c &push);
    virtual void goToPoint5();
    virtual void goToPoint4();
    virtual void goToPoint3();
    virtual void goToPoint0();
    virtual void goToPoint1();
    virtual void goToPoint2();

    void initPoints();
    void goToPoint(u8 point);
    void setRandomHeight();
    void addOfs(mVec3_c &pos, f32 x, f32 y, f32 z);
    void calcAnmRate();

    /* 0x418 */ mVec3_c mPoint1Pos;
    /* 0x424 */ mVec3_c mPoint4Pos;
    /* 0x430 */ u8 mPoint;   // current waypoint
    /* 0x434 */ f32 mArriveDist; // a waypoint is reached closer than this
    /* 0x438 */ f32 mHeight; // y of the waypoints
    /* 0x43C */ f32 mWaitRate;   // percent: chance of initWait() at a waypoint
}; // size 0x440
