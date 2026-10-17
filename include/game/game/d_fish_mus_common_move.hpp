#pragma once

// Base of the free-swimming museum fish: target point and angles, acceleration, avoiding the other fish.

#include <game/game/d_fish_museum.hpp>

class dFishMusCommonMove_c : public dFishMusBase_c {
public:
    struct TurnParam_c {
        /* 0x0 */ s16 mScale;
        /* 0x4 */ f32 mMaxStep; // degrees
        /* 0x8 */ f32 mMinStep; // degrees
    };

    dFishMusCommonMove_c();
    virtual ~dFishMusCommonMove_c();
    virtual void initWait();
    virtual void initSwim();
    virtual bool isCornered(dFishMusBase_c *other);

    // mFishIdx compared as an s32: `isFishKind(A) || isFishKind(A + 1)` stays two compares as in the target
    // (`mFishIdx == A || mFishIdx == A + 1` becomes a range check). Comparisons of mHitFishIdx are direct.
    bool isFishKind(s32 kind) const { return mFishIdx == kind; }

    bool isRiverBottomKind() { return mFishIdx == FISH_DONKO || mFishIdx == FISH_DOJOU || mFishIdx == FISH_NAMAZU; }
    bool isLurkKind() {
        return mFishIdx == FISH_DONKO || mFishIdx == FISH_DOJOU || mFishIdx == FISH_NAMAZU || mFishIdx == FISH_RAIGYO;
    }
    // FISH_SHIIRA is tested twice in the original: with only two tests the compiler drops the target's bit-mask form.
    bool isSeaBottomKind() { return mFishIdx == FISH_ANKOU || mFishIdx == FISH_SHIIRA || mFishIdx == FISH_SHIIRA; }

    void initMove();
    void initNextAction();
    void resetAccel();
    void accelerate(f32 maxSpeed, f32 accelStep, f32 ratio);
    void setRandomPos();
    void setWaitTimer();
    void setSwimTimer();
    void setTargetAngleX();
    void setTargetOfs();
    void turnX(s16 scale, f32 maxStep, f32 minStep);
    void turnY(s16 scale, f32 maxStep, f32 minStep);
    void turnZ(s16 scale, f32 maxStep, f32 minStep);
    void calcMove();
    bool isLurkFish();
    bool isBottomFish();
    void setHitInfo(dFishMusBase_c *other);
    void setEscapeAngleY(dFishMusBase_c *other, const dFishMusData_c *otherData);
    void setEscapeAngleX(const dFishMusData_c *otherData, mAng maxAngle);
    void setEscapeTarget(const dFishMusData_c *otherData);
    void clampTarget(f32 marginX, f32 marginZ);
    bool isHitByDangerous(dFishMusBase_c *other);

    /* 0x360 */ mVec3_c mPoint0Pos; // waypoints of dFishMusBig_c (also used by dFishMusKeepSwimming_c)
    /* 0x36C */ mVec3_c mPoint2Pos;
    /* 0x378 */ mVec3_c mPoint5Pos;
    /* 0x384 */ mVec3_c mPoint3Pos;
    /* 0x390 */ mVec3_c mTargetPos;
    /* 0x39C */ mVec3_c mTargetOfs;
    /* 0x3A8 */ mAng mTargetAngleX;
    /* 0x3AC */ TurnParam_c mTurnX; // turnX() parameters
    /* 0x3B8 */ mAng mTargetAngleY;
    /* 0x3BC */ TurnParam_c mTurnY; // turnY() parameters
    /* 0x3C8 */ mAng mTargetAngleZ;
    /* 0x3CC */ f32 mTurnBackRate; // percent: chance that setTargetOfs turns back after a wall
    /* 0x3D0 */ mAng mTargetYaw;   // yaw offset picked by setTargetOfs
    /* 0x3D4 */ f32 mAccel;
    /* 0x3D8 */ u8 mNearWall;
    /* 0x3D9 */ u8 mNextPoint;     // next waypoint (dFishMusBig_c::goToPoint)
    /* 0x3DC */ mVec3_c mHitCenter;      // setHitInfo: our center
    /* 0x3E8 */ mVec3_c mHitDir;         // setHitInfo: from our center to the other's
    /* 0x3F4 */ mVec3_c mHitOtherCenter; // setHitInfo: the other's center
    /* 0x400 */ mVec3_c mHitOtherVel;    // setHitInfo: the other's velocity
    /* 0x40C */ s16 mEscapeAngleY;
    /* 0x410 */ f32 mHitApproach; // our velocity . mHitDir (>= 0: swimming into the other)
    /* 0x414 */ f32 mHitVelDot;   // our velocity . the other's velocity
}; // size 0x418
