#pragma once

// Pike, salmon, tuna-like fish: laps of the tank along the waypoints of dFishMusBig_c.

#include <game/game/d_fish_mus_big.hpp>

class dFishMusFast_c : public dFishMusBig_c {
public:
    typedef void (dFishMusFast_c::*StateFunc)();

    dFishMusFast_c();
    virtual ~dFishMusFast_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void initWait();
    virtual void goToPoint5();
    virtual void goToPoint4();
    virtual void goToPoint3();
    virtual void goToPoint0();
    virtual void goToPoint1();
    virtual void goToPoint2();

    void init();
    void stateWait();
    void stateSwim();
    void hitFish(dFishMusBase_c *other);
    void hitFishRiver(const dFishMusData_c *otherData);
    void hitFishPond(dFishMusBase_c *other, const dFishMusData_c *otherData);
    void escape(const dFishMusData_c *otherData);
    bool isApproaching();
    void setEscapeYaw(const dFishMusData_c *otherData);
    void escapeAhead(const dFishMusData_c *otherData);

    /* 0x440 */ StateFunc mState;
    /* 0x44C */ int mPointTimer; // goes on to the next waypoint when it runs out
}; // size 0x450
