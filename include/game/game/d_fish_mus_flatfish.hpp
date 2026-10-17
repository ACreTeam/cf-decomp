#pragma once

// Dab and olive flounder: lie on the floor and swim short hops.

#include <game/game/d_fish_mus_common_move.hpp>

class dFishMusFlatfish_c : public dFishMusCommonMove_c {
public:
    typedef void (dFishMusFlatfish_c::*StateFunc)();

    dFishMusFlatfish_c();
    virtual ~dFishMusFlatfish_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void initWait();
    virtual void initSwim();
    virtual bool isCornered(dFishMusBase_c *other);

    void init();
    void stateWait();
    void stateSwim();
    void calcAnmRate();
    void fadeAlpha(int step, int target);

    /* 0x418 */ StateFunc mState;
    /* 0x424 */ int mAlpha; // TEV konst colour 1 alpha of "m0"
    /* 0x428 */ int _428;
}; // size 0x42C
