#pragma once

// The mole cricket (INSECT_MOLE_CRICKET): chirps underground (heard when a
// player is near) until a player digs at its spot, then jumps out, walks away
// from the nearest player and burrows again (or fades out where it cannot
// dig). Online, the owner's state, target and the player it flees are shared
// through the insect's net slot.

#include <game/game/d_insect_field.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_shadow.hpp>

class dInsFldMoleCricket_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldMoleCricket_c::*StateFunc)();

    enum State_e {
        STATE_HIDE = 0,     // underground, chirping
        STATE_JUMP_OUT = 1, // dug up
        STATE_LAND = 2,
        STATE_WALK = 3,
        STATE_BURROW = 4,
        STATE_FADE = 5,     // cannot dig here: fade out
        STATE_RELEASE = 6,  // released from the net
        STATE_NUM_NET = 14, // syncState: higher net values are ignored
    };

    enum {
        DIG_TILT_TIME = 70, // STATE_BURROW tilts down for this many frames
    };

    // The per-type parameters (.data 0x1468, one entry).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mOffsetY;
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 mChirpRadius; // its chirp is heard this close
        /* 0x10 */ u8 mDigFrames;    // frames of digging that bring it out
        /* 0x14 */ f32 mJumpSpeed;
        /* 0x18 */ f32 mJumpSpeedY;
        /* 0x1C */ f32 mGravity;
        /* 0x20 */ f32 mLandTime; // seconds
        /* 0x24 */ f32 mWalkDistMax;
        /* 0x28 */ f32 mWalkDistMin;
        /* 0x2C */ f32 mWalkSpeed;
        /* 0x30 */ f32 mWalkAccel; // fly()
        /* 0x34 */ f32 mWalkDrag;
        /* 0x38 */ f32 mWalkTurn;
        /* 0x3C */ f32 mWalkTime; // seconds before it burrows
        /* 0x40 */ f32 mWalkAnmRate;
        /* 0x44 */ f32 mReleaseSpeed;
        /* 0x48 */ f32 mReleaseAnmRate;
        /* 0x4C */ f32 _4C;
        /* 0x50 */ f32 _50;
    }; // size 0x54

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sTypes[];

    static int getParamIdx(int type);                    // 0x1A184
    static const param_c *getMoleCricketParam(int type); // 0x1A1A4

    dInsFldMoleCricket_c();          // 0x1A1D4
    virtual ~dInsFldMoleCricket_c(); // 0x1A234
    virtual int draw();              // 0x1A28C: not while hidden
    virtual void calcMtx();          // 0x1A394: setMtxFromAxis
    // 0x1A398 (its trailing dead blr is dtk's fn_116_1A3B0)
    virtual void onBecomeOwner();
    void setState(int state);         // 0x1A3B4: and the owner's net slot
    virtual void setGroundPos();      // 0x1A544
    virtual void initState();         // 0x1A5C4
    virtual void executeState();      // 0x1A5CC
    void initHide();                  // 0x1A644
    void initJumpOut();               // 0x1A6B4
    void initLand();                  // 0x1A7F4
    void initWalk();                  // 0x1A894
    void initBurrow();                // 0x1A994
    void initFade();                  // 0x1AA58
    virtual void onRelease();         // 0x1AADC: = initRelease
    void executeHide();               // 0x1AB84
    void executeJumpOut();            // 0x1AC54
    void executeLand();               // 0x1ADE8
    void executeWalk();               // 0x1AF90
    void executeBurrow();             // 0x1B148
    void executeFade();               // 0x1B1F0
    void executeRelease();            // 0x1B3A4
    mAng getWalkAngle();              // 0x1B528: away from the nearest player
    f32 getWalkDist() const;          // 0x1B574
    void getWalkTarget(mVec3_c *out); // 0x1B5A4
    void walk(f32 lift, f32 accel, f32 drag,
              f32 turn);  // 0x1B70C: fly(), upright on the ground
    bool canDig();        // 0x1B77C
    void setWalkTarget(); // 0x1B7BC: and the owner's net slot
    void syncState();     // 0x1B994: the owner's state (net) or the timers
    bool wasUnderWater(); // 0x1BB50: mOldPos

    /* 0x318 */ int mState;      // State_e
    /* 0x31C */ int mPrevState;  // at the start of executeState
    /* 0x320 */ int mStateCheck; // executeState: set again if changed
    /* 0x324 */ bool mDug;       // a player dug at its spot
    /* 0x326 */ u16 mLandFrames;
    /* 0x328 */ u16 mMoveFrames;
    /* 0x32A */ u16 mWalkTimer;
    /* 0x32C */ u16 mDigTimer;
}; // size 0x330
