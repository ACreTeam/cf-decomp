#pragma once

// Big sea fish that keep swimming laps around the tank.

#include <game/game/d_fish_mus_common_move.hpp>

class dFishMusKeepSwimming_c : public dFishMusCommonMove_c {
public:
    typedef void (dFishMusKeepSwimming_c::*StateFunc)();

    dFishMusKeepSwimming_c();
    virtual ~dFishMusKeepSwimming_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void initSwim();
    virtual bool isCornered(dFishMusBase_c *other);

    void init();
    void setTargetRightLow();
    void setTargetLeftLow();
    void setTargetRightHigh();
    void setTargetLeftHigh();
    void brake();
    void setTargetRoll();
    void roll();
    void stateSwim();
    bool isFacingRight();
    bool isFacingLeft();
    bool isNearRight();
    bool isNearLeft();
    void hitFish(dFishMusBase_c *other);
    void hitFishAji(dFishMusBase_c *other, const dFishMusData_c *otherData);
    void hitFishBig(dFishMusBase_c *other, const dFishMusData_c *otherData);
    void setEscapeHeight(const dFishMusData_c *otherData);
    void setEscapeTargetFar();

    /* 0x418 */ StateFunc mState;
    /* 0x424 */ f32 mRightX;
    /* 0x428 */ f32 mLeftX;
    /* 0x42C */ f32 mCornerOfsX;
    /* 0x430 */ f32 mHeightOfs;
    /* 0x434 */ u8 mLap;      // % 4: right low, right high, left low, left high
    /* 0x435 */ bool mEscaped; // hit while swimming: picks the side again at the next target
    /* 0x436 */ s16 mTurnXScale;
    /* 0x438 */ f32 mTurnXMaxStep;
    /* 0x43C */ f32 mTurnYMaxStep;
    /* 0x440 */ s16 mRollDist;
    /* 0x444 */ f32 mRollDeg;
    /* 0x448 */ f32 mRollStep;
    /* 0x44C */ f32 mRollBackStep;
    /* 0x450 */ f32 mBrakeDist;
    /* 0x454 */ f32 mBrakeStep;
    /* 0x458 */ f32 mAccelRatio;
    /* 0x45C */ int _45C;
}; // size 0x460
