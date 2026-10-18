#pragma once

// The mosquito (INSECT_MOSQUITO): wanders at night, chases a near player,
// circles around him and bites (dPlayerMgr::requestInsectBite).

#include <game/game/d_insect_field.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/game/d_player_mgr.hpp>

class dInsFldMosquito_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldMosquito_c::*StateFunc)();

    enum State_e {
        STATE_WANDER = 0,
        STATE_CHASE = 1,
        STATE_CIRCLE = 2,  // around the player, then bite
        STATE_RELEASE = 3, // released from the net (or after the bite)
    };

    enum { BITE_WAIT_TIME = 180 }; // STATE_CIRCLE frames before the bite

    // The per-type parameters (.data 0x1658, one entry).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mHeight;  // above the ground or water
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 mFlyDistMax;
        /* 0x10 */ f32 mFlyDistMin;
        /* 0x14 */ f32 mFlySpeed; // dInsectUtil::moveTo
        /* 0x18 */ f32 mFlyTurn;
        /* 0x1C */ f32 mBobSpeed; // degrees per frame
        /* 0x20 */ f32 mBobHeightMin;
        /* 0x24 */ f32 mBobHeightMax;
        /* 0x28 */ f32 mWingSpeed; // degrees per frame (mAngle.x swing)
        /* 0x2C */ f32 mWingAmp;   // degrees
        /* 0x30 */ f32 mBiteRadius;
        /* 0x34 */ f32 mCircleSpeed; // degrees per frame
        /* 0x38 */ f32 mCircleWingSpeed;
        /* 0x3C */ f32 mCircleWingAmp;
        /* 0x40 */ f32 mReleaseSpeed;
        /* 0x44 */ f32 mReleaseAccelY;
        /* 0x48 */ f32 _48;
        /* 0x4C */ f32 _4C;
    }; // size 0x50

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sTypes[];

    static int getParamIdx(int type);                 // 0x20340
    static const param_c *getMosquitoParam(int type); // 0x20360

    dInsFldMosquito_c();          // 0x20390
    virtual ~dInsFldMosquito_c(); // 0x20414
    virtual int draw();           // 0x2046C
    void setState(int state);     // 0x204A8
    virtual void setGroundPos();  // 0x204E0
    virtual void initState();     // 0x20554
    virtual void executeState();  // 0x2055C
    void initWander();            // 0x205B4
    void initChase();             // 0x20688
    void initCircle();            // 0x20770
    virtual void onRelease();     // 0x2085C: = initRelease
    void executeWander();         // 0x208FC
    void executeChase();          // 0x20A4C
    void executeCircle();         // 0x20C74
    void executeRelease();        // 0x20EF8
    void bob();                   // 0x21030
    mAng getTargetAngle() const;  // 0x21210
    f32 getTargetDist() const;    // 0x21278
    void getTarget(mVec3_c *out); // 0x212A8
    bool isPlayerNear();          // 0x2140C: within 320
    bool isBiteNear();            // 0x2145C
    f32 getFlyHeight();           // 0x214AC
    bool getTurnDir();            // 0x2152C: circle clockwise?

    /* 0x318 */ int mState; // State_e
    /* 0x31C */ f32 mBaseY; // follows getFlyHeight()
    /* 0x320 */ s16 mBobPhase;
    /* 0x324 */ f32 mBobHeight;
    /* 0x328 */ u16 mWanderFrames;
    /* 0x32A */ u16 mChaseFrames;
    /* 0x32C */ u16 mBiteTimer;
    /* 0x32E */ mAng mCircleAngle;
    /* 0x330 */ mVec3_c mCenter; // initCircle: the player position
    /* 0x33C */ f32 mRadius;
    /* 0x340 */ s16 mWingPhase;
    /* 0x342 */ u16 _342;
    /* 0x344 */ bool mTurnDir;
}; // size 0x348
