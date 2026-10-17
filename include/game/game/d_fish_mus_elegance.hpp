#pragma once

// Goldfish and angelfish: glide after a short acceleration.

#include <game/game/d_fish_mus_small.hpp>

class dFishMusElegance_c : public dFishMusSmall_c {
public:
    typedef void (dFishMusElegance_c::*StateFunc)();

    dFishMusElegance_c();
    virtual ~dFishMusElegance_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void initWait();
    virtual void initSwim();

    void stateSwim();

    /* 0x428 */ StateFunc mState;
}; // size 0x434
