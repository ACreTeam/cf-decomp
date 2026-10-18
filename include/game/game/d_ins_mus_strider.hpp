#pragma once

// The museum pond skater.

#include <game/game/d_insect_museum.hpp>

struct dInsMusStriderParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mMoveAnmRate;
    /* 0x10 */ f32 mWaitAnmRate;
    /* 0x14 */ f32 mMoveSpeed;
    /* 0x18 */ f32 mMoveDistMax;
    /* 0x1C */ f32 mMoveDistMin;
    /* 0x20 */ f32 mShadowSize;
    /* 0x24 */ int mArea;
}; // size 0x28

class dInsMusStrider_c : public dInsMusBase_c {
public:
    typedef void (dInsMusStrider_c::*StateFunc)();

    enum {
        GLIDE_START_FRAME = 10,
        MOVE_END_FRAME = 79,
    };

    dInsMusStrider_c();
    virtual ~dInsMusStrider_c();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initMove();
    void stateWait();
    void stateMove();
    mAng getMoveAngle();
    f32 getMoveDist() const;
    void getMoveTarget(mVec3_c *target);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ bool mRipple;
    /* 0x21E */ u16 mWaitTime;
    /* 0x220 */ u16 mMoveTime;
}; // size 0x224
