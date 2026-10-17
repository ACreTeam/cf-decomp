#pragma once

// The puffer fish: puffs up when a big sea fish bumps into it.

#include <game/game/d_fish_mus_base_move.hpp>

class dFishMusPufferfish_c : public dFishMusBaseMove_c {
public:
    typedef void (dFishMusPufferfish_c::*StateFunc)();

    enum {
        MODE_PUFFED = MODE_DANGEROUS,
    };

    dFishMusPufferfish_c();
    virtual ~dFishMusPufferfish_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void initWait();
    virtual void initSwim();
    virtual bool isCornered(dFishMusBase_c *other);

    void init();
    void stateSwim();
    void calcAnm();
    bool isThreat(dFishMusBase_c *other);

    /* 0x434 */ StateFunc mState;
    /* 0x440 */ f32 mFrame;
    /* 0x444 */ f32 mLoopEnd;
    /* 0x448 */ f32 mLoopStart;
    /* 0x44C */ bool mPuffing;  // puffing up
    /* 0x44D */ bool mPuffed;   // puffed loop, then deflating
    /* 0x44E */ u8 mPuffLoops;  // loops left puffed
}; // size 0x450
