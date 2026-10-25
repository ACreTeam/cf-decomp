#pragma once

// The pill bug and the centipede (INSECT_PILL_BUG, INSECT_CENTIPEDE): hide
// under a rock until a player hits it, then jump out away from that player and
// walk around (the pill bug curls up when scared). They fade out after a while.
// Online, the owner's state and target are shared through the net slot.

#include <game/game/d_insect_field.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_shadow.hpp>

class dInsFldPillBug_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldPillBug_c::*StateFunc)();

    enum State_e {
        STATE_HIDE = 0, // under the rock
        STATE_JUMP_OUT = 1,
        STATE_WALK = 2,
        STATE_CURL = 3,     // scared (not the centipede)
        STATE_RELEASE = 4,  // released from the net
        STATE_NUM_NET = 14, // syncState: higher net values are ignored
    };

    enum {
        JUMP_NO_BG_TIME = 10, // STATE_JUMP_OUT: no bg check for these frames
        AROUND_SIZE = 3, // isSurrounded / getEscapePos: the 3x3 units around it
        SURROUNDED_NUM = 7, // isSurrounded: columns on this many of the 8 units
    };

    // The per-type parameters (.data 0x15B0: pill bug, centipede).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mOffsetY;
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ u8 mHitFrames; // after the rock is hit
        /* 0x10 */ f32 mJumpSpeedY;
        /* 0x14 */ f32 mGravity;
        /* 0x18 */ f32 mJumpTime; // seconds
        /* 0x1C */ f32 mWalkDistMax;
        /* 0x20 */ f32 mWalkDistMin;
        /* 0x24 */ f32 mWalkSpeed;
        /* 0x28 */ f32 mWalkAccel; // fly()
        /* 0x2C */ f32 mWalkDrag;
        /* 0x30 */ f32 mWalkTurn;
        /* 0x34 */ f32 mLifeTime; // seconds out, then it fades
        /* 0x38 */ f32 mWalkAnmRate;
        /* 0x3C */ f32 mReleaseSpeed;
        /* 0x40 */ f32 mReleaseAnmRate;
        /* 0x44 */ f32 _44;
        /* 0x48 */ f32 _48;
        /* 0x4C */ f32 _4C;
        /* 0x50 */ f32 _50;
    }; // size 0x54

    static param_c sParam[2]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];

    static int getParamIdx(int type);                // 0x1E910
    static const param_c *getPillBugParam(int type); // 0x1E944

    dInsFldPillBug_c();          // 0x1E974
    virtual ~dInsFldPillBug_c(); // 0x1EA2C
    virtual int execute();       // 0x1EAB8: and the fade out
    virtual int draw();          // 0x1EB80
    virtual void calcMtx();      // 0x1EC3C: setMtxFromAxis
    // 0x1EC40 (its trailing dead blr is dtk's fn_116_1EC50)
    virtual void onBecomeOwner();
    void setState(int state);         // 0x1EC54: and the owner's net slot
    virtual void setGroundPos();      // 0x1EDE4
    virtual void initState();         // 0x1EE70
    virtual void executeState();      // 0x1EE78
    void initHide();                  // 0x1EEF0
    void initJumpOut();               // 0x1EF54
    void initWalk();                  // 0x1F1AC
    void initCurl();                  // 0x1F2A4
    virtual void onRelease();         // 0x1F334: = initRelease
    void executeHide();               // 0x1F3F4
    void executeJumpOut();            // 0x1F4C4
    void executeWalk();               // 0x1F688
    void executeCurl();               // 0x1F840
    void executeRelease();            // 0x1F93C
    mAng getWalkAngle();              // 0x1FACC: away from the nearest player
    f32 getWalkDist() const;          // 0x1FB18
    void getWalkTarget(mVec3_c *out); // 0x1FB48
    void walk(f32 lift, f32 accel, f32 drag,
              f32 turn);  // 0x1FCB0: fly(), upright on the ground
    void syncState();     // 0x1FD20: the owner's state (net) or the timers
    void setWalkTarget(); // 0x1FED8: and the owner's net slot
    bool isCentipede();   // 0x200A8
    bool isSurrounded();  // 0x200BC: columns on the 8 units around
    mVec3_c
    getEscapePos() const; // 0x20184: the free unit farthest from the hitter
    bool wasUnderWater(); // 0x202E0: mOldPos

    /* 0x318 */ int mState;      // State_e
    /* 0x31C */ int mPrevState;  // at the start of executeState
    /* 0x320 */ int mStateCheck; // executeState: set again if changed
    /* 0x324 */ bool mHit;       // a player hit its rock
    /* 0x328 */ f32 _328;
    /* 0x32C */ bool mOut; // jumped out: the life timer runs
    /* 0x32E */ u16 mLifeTimer;
    /* 0x330 */ u16 mMoveFrames;
    /* 0x332 */ u16 mHitTimer;
    /* 0x334 */ bool mLanded; // executeRelease: the landing sound played
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    /* 0x335 */ bool mAlive;
#endif
    /* 0x338 */ dLevelEffect_c mEffect;
    /* 0x3CC */ int mHitter; // the player that hit the rock (4: none)
}; // size 0x3D0
