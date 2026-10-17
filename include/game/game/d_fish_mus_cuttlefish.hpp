#pragma once

// The squid: swims in pulses towards random points, rolling while it accelerates.

#include <game/game/d_fish_mus_common_move.hpp>

class dFishMusCuttlefish_c : public dFishMusCommonMove_c {
public:
    dFishMusCuttlefish_c();
    virtual ~dFishMusCuttlefish_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void onHitObj(dAquariumCorrectObj_c *obj, const mVec3_c &push);
    virtual void initSwim();
    virtual bool isCornered(dFishMusBase_c *other);

    void init();
    void setRollAngle();
    void setPulseSpeed();
    void swim();
    void calcMove();
    void calcPulse();
    void setEscapeHeight(const dFishMusData_c *otherData);
    void escape(dFishMusBase_c *other, const dFishMusData_c *otherData);

    /* 0x418 */ f32 mPulseSpeed;
    /* 0x41C */ f32 mPulse;      // 0 .. mPulsePeriod, advanced by mAnmRateFast
    /* 0x420 */ f32 mPulsePeriod;
    /* 0x424 */ s16 mRollAngle;
}; // size 0x428
