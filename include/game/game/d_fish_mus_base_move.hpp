#pragma once

// The plain museum fish: swims, waits and dodges the other fish.

#include <game/game/d_fish_mus_common_move.hpp>

class dFishMusBaseMove_c : public dFishMusCommonMove_c {
public:
    typedef void (dFishMusBaseMove_c::*StateFunc)();

    enum Mode_e {
        MODE_NORMAL,
        MODE_DANGEROUS, // other fish flee (dFishMusCommonMove_c::isHitByDangerous)
    };

    dFishMusBaseMove_c();
    virtual ~dFishMusBaseMove_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void initWait();
    virtual void initSwim();
    virtual bool isCornered(dFishMusBase_c *other);

    void startSwim() {
        mState = &dFishMusBaseMove_c::stateSwim;
        mSwimThroughRate = 100.0f;
    }

    void stateSwim();
    void setBottomHeight();
    void hitFish(dFishMusBase_c *other);
    void hitFishRiver(dFishMusBase_c *other, const dFishMusData_c *otherData);
    void hitFishPond(dFishMusBase_c *other, const dFishMusData_c *otherData);
    void hitFishSea(dFishMusBase_c *other, const dFishMusData_c *otherData);
    bool canEscape();
    void escapeUnder(const dFishMusData_c *otherData);
    void escape(dFishMusBase_c *other, const dFishMusData_c *otherData);
    void escapeFast(dFishMusBase_c *other, const dFishMusData_c *otherData);
    void init();
    void initNextAction();
    void stateWait();
    void turn();
    void stateSwimThrough();
    void stateStop();
    void checkHeight();
    void calcAnmRate();

    /* 0x418 */ int mMode; // Mode_e
    /* 0x41C */ StateFunc mState;
    /* 0x428 */ bool mInHeightRange;
    /* 0x42C */ f32 mAccelStep;
    /* 0x430 */ f32 mSwimThroughRate; // percent
}; // size 0x434
