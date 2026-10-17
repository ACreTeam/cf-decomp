#pragma once

// The ray: glides from side to side, rolling into its turns.

#include <game/game/d_fish_mus_common_move.hpp>

class dFishMusRay_c : public dFishMusCommonMove_c {
public:
    enum {
        SWIM_ANGLE_MIN = 0x160B, // ~31 degrees off the x axis
        SWIM_ANGLE_MAX = 0x16C1,
        WALL_ANGLE = 0x6889,     // ~147 degrees: turning away from a side wall
        ROLL_ANGLE = 0x31C7,     // ~70 degrees
        PITCH_MIN = 0xC17,       // ~17 degrees
        PITCH_MAX = 0xCCD,
    };

    dFishMusRay_c();
    virtual ~dFishMusRay_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitObj(dAquariumCorrectObj_c *obj, const mVec3_c &push);

    void init();
    void roll();
    void swim();
    void turnAtWall();
    void checkHeight();
    bool isSwimmingRight();

    void turnAtSideWallPlusZ() {
        if (isSwimmingRight()) {
            mSwimAngle = -WALL_ANGLE;
            mTargetAngleY = mSwimAngle + 0x4000;
            mTargetAngleZ = ROLL_ANGLE;
        } else {
            mSwimAngle = WALL_ANGLE;
            mTargetAngleY = mSwimAngle - 0x4000;
            mTargetAngleZ = -ROLL_ANGLE;
        }
    }
    void turnAtSideWall() {
        if (isSwimmingRight()) {
            mSwimAngle = WALL_ANGLE;
            mTargetAngleY = mSwimAngle + 0x4000;
            mTargetAngleZ = -ROLL_ANGLE;
        } else {
            mSwimAngle = -WALL_ANGLE;
            mTargetAngleY = mSwimAngle - 0x4000;
            mTargetAngleZ = ROLL_ANGLE;
        }
    }

    static bool isZero(f32 v) { return std::fabs(v) <= FLT_EPSILON; }

    /* 0x418 */ mVec3_c mWallPush; // push of the front / back glass this frame
    /* 0x424 */ mVec3_c mLastWallPush;
    /* 0x430 */ s16 mCurSwimAngle;
    /* 0x432 */ s16 mSwimAngle; // mTargetAngleY = mSwimAngle -+ 90 degrees
}; // size 0x434
