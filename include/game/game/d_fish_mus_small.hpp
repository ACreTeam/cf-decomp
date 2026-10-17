#pragma once

// Small museum fish: short swims and waits.

#include <game/game/d_fish_mus_common_move.hpp>

class dFishMusSmall_c : public dFishMusCommonMove_c {
public:
    typedef void (dFishMusSmall_c::*StateFunc)();

    dFishMusSmall_c();
    virtual ~dFishMusSmall_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void initWait();
    virtual void initSwim();

    void init();
    void stateWait();
    void stateSwim();
    void calcAnmRate();

    /* 0x418 */ StateFunc mState;
    /* 0x424 */ bool mAccelerating; // used by dFishMusElegance_c
}; // size 0x428
