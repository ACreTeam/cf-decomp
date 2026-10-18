#pragma once

// dInsMusDivingBeetle_c: the diving beetle of the museum's pond.

#include <game/game/d_insect_museum.hpp>

struct dInsMusDivingBeetleParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mWaitTimeMax;
    /* 0x10 */ f32 mWaitTimeMin;
    /* 0x14 */ f32 mSwimDistMax;
    /* 0x18 */ f32 mSwimDistMin;
    /* 0x1C */ f32 mSwimBrake;
    /* 0x20 */ f32 mSwimAnmRateStart;
    /* 0x24 */ f32 mSwimAnmRateEnd;
    /* 0x28 */ f32 mShadowSize;
    /* 0x2C */ int mArea;
}; // size 0x30

class dInsMusDivingBeetle_c : public dInsMusBase_c {
public:
    typedef void (dInsMusDivingBeetle_c::*StateFunc)();

    enum { SWIM_TIME = 200 };

    dInsMusDivingBeetle_c();
    virtual ~dInsMusDivingBeetle_c();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initSwim();
    void stateWait();
    void stateSwim();
    void swim();
    mAng getSwimAngle();
    f32 getSwimDist() const;
    void getSwimTarget(mVec3_c *target);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ u16 mWaitTime;
    /* 0x21E */ u16 mSwimTime;
}; // size 0x220
