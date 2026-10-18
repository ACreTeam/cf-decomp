#pragma once

// The violin beetle and the longhorn beetle (INSECT_VIOLIN_BEETLE,
// INSECT_LONGHORN_BEETLE): sit on a tree stump and wander around on it,
// swaying; they fly off when someone comes near or the stump is gone.
// Online, the owner's flight is shared through the net slot.

#include <game/game/d_insect_field.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_net.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_quat.hpp>

class dInsFldStump_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldStump_c::*StateFunc)();

    enum State_e {
        STATE_WAIT = 0,
        STATE_WALK = 1,
        STATE_FLY_AWAY = 2,
        STATE_RELEASE = 3,  // released from the net
        STATE_NUM_NET = 14, // syncState: higher net values are ignored
    };

    // The sway while walking (mSwayY / mSwayZ).
    enum {
        SWAY_SPEED = ANGLE_PER_DEGREE / 2, // per frame
        SWAY_MAX = DEG_TO_ANGLE(10),       // mSwayY turns back here
    };

    // The per-type parameters (.data 0x1848: violin beetle, longhorn beetle).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mOffsetY; // on the stump
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 mWaitMax; // seconds
        /* 0x10 */ f32 mWaitMin;
        /* 0x14 */ f32 mWalkDistMax;
        /* 0x18 */ f32 mWalkDistMin;
        /* 0x1C */ f32 mWalkSpeed; // dInsectUtil::moveTo
        /* 0x20 */ f32 mWalkTurn;
        /* 0x24 */ f32 mWalkAnmRate;
        /* 0x28 */ f32 mFlySpeed; // moveForward
        /* 0x2C */ f32 mFlyAccelY;
        /* 0x30 */ f32 mReleaseAnmRate;
        /* 0x34 */ f32 _34;
        /* 0x38 */ f32 _38;
        /* 0x3C */ f32 _3C;
        /* 0x40 */ int mScareLimit;
    }; // size 0x44

    static param_c sParam[2]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];

    static bool isStump(const dItem::Item *item,
                        const mVec3_c *pos); // 0x24280: itemSearch_c filter
    static int getParamIdx(int type);        // 0x2421C
    static const param_c *getStumpParam(int type); // 0x24250

    dInsFldStump_c();                                   // 0x242B8
    virtual ~dInsFldStump_c();                          // 0x2434C
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x243A4
    virtual int draw();                                 // 0x24420
    virtual void calcMtx();      // 0x24444: with the sway (mRot)
    void setState(int state);    // 0x24518: and the owner's net slot (flight)
    virtual void setGroundPos(); // 0x245AC
    virtual void initState();    // 0x2461C
    virtual void executeState(); // 0x24624
    void initWait();             // 0x24698
    void initWalk();             // 0x2475C
    void initFlyAway();          // 0x24884
    virtual void onRelease();    // 0x24934: = initRelease
    void executeWait();          // 0x249CC
    void executeWalk();          // 0x249DC
    void executeFlyAway();       // 0x24BDC
    void executeRelease();       // 0x24C20
    mAng getFlyAngle();          // 0x24C64: away from the nearest player / NPC
    mAng getWalkAngle();         // 0x24CFC
    f32 getWalkDist() const;     // 0x24D60
    void getWalkTarget(mVec3_c *out); // 0x24D90
    void getStumpPos(mVec3_c *out);   // 0x24E80: a random spot on the stump
    bool isSomeoneNear();             // 0x24F30
    BOOL isOnStump();                 // 0x24FA8
    void syncState();                 // 0x25010

    /* 0x318 */ int mState;     // State_e
    /* 0x31C */ int mPrevState; // executeState: set again if changed
    /* 0x320 */ u16 mWaitFrames;
    /* 0x322 */ u16 mWalkFrames;
    /* 0x324 */ u16 _324;
    /* 0x328 */ mVec3_c mUnitCenter; // the stump's unit
    /* 0x334 */ s16 mSwayY;
    /* 0x336 */ s16 mSwayZ;
    /* 0x338 */ bool mSwayDir;
    /* 0x33C */ mQuat_c mRot; // the sway
}; // size 0x34C
