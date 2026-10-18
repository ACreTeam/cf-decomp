#pragma once

// The tarantula and the scorpion (INSECT_TARANTULA, INSECT_SCORPION): roam at
// night; disturbed, they turn to the player and rear up, and when he has his
// net out they charge and knock him out (dPlayerMgr::requestFaint), or run.

#include <game/game/d_insect_field.hpp>
#include <game/game/d_player_mgr.hpp>

class dInsFldTarantula_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldTarantula_c::*StateFunc)();

    enum State_e {
        STATE_WAIT = 0,
        STATE_WALK = 1,
        STATE_TURN = 2,   // to the player
        STATE_THREAT = 3, // rear up
        STATE_TURN_AWAY = 4,
        STATE_ATTACK = 5,  // charge the player
        STATE_STOP = 6,    // the player is down (or safe)
        STATE_ESCAPE = 7,  // runs off and fades
        STATE_RELEASE = 8, // released from the net
    };

    // The per-type parameters (.data 0x1710: tarantula, scorpion).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mOffsetY;
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 mWaitMax; // seconds
        /* 0x10 */ f32 mWaitMin;
        /* 0x14 */ f32 mWaitDecay;   // speed factor per frame
        /* 0x18 */ f32 mWalkTimeMax; // seconds
        /* 0x1C */ f32 mWalkTimeMin;
        /* 0x20 */ f32 mWalkSpeed;
        /* 0x24 */ f32 mWalkAccel; // fly()
        /* 0x28 */ f32 mWalkDrag;
        /* 0x2C */ f32 mWalkTurn;
        /* 0x30 */ f32 mWalkAnmRate;
        /* 0x34 */ f32 _34;
        /* 0x38 */ f32 mAttackAccel; // fly()
        /* 0x3C */ f32 mAttackDrag;
        /* 0x40 */ f32 mAttackTurn;
        /* 0x44 */ f32 mAttackAnmRate;
        /* 0x48 */ f32 mEscapeTime; // seconds, then it fades
        /* 0x4C */ f32 mEscapeSpeed;
        /* 0x50 */ f32 mEscapeAccel; // fly()
        /* 0x54 */ f32 mEscapeDrag;
        /* 0x58 */ f32 mEscapeTurn;
        /* 0x5C */ f32 mEscapeAnmRate;
        /* 0x60 */ f32 mTurnRate; // front blend per frame
        /* 0x64 */ f32 _64;
        /* 0x68 */ f32 mGravity;
        /* 0x6C */ f32 mReleaseAnmRate;
        /* 0x70 */ f32 _70;
        /* 0x74 */ f32 _74;
        /* 0x78 */ f32
            mSafeRadius; // STATE_STOP when the player is this near and safe
        /* 0x7C */ int _7C;
        /* 0x80 */ int _80;
    }; // size 0x84

    static param_c sParam[2]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static f32 sAttackJump[2]; // .data 0x1708: initAttack's jump speed ([1] is
                               // not used)

    static int getParamIdx(int type);                  // 0x2250C
    static const param_c *getTarantulaParam(int type); // 0x22540

    dInsFldTarantula_c();             // 0x22570
    virtual ~dInsFldTarantula_c();    // 0x225D4
    virtual int draw();               // 0x2262C
    virtual void calcMtx();           // 0x22650: setMtxFromAxis
    void setState(int state);         // 0x22654
    virtual void setGroundPos();      // 0x2268C
    virtual void initState();         // 0x22718
    virtual void executeState();      // 0x22720
    void initWait();                  // 0x227F4
    void initWalk();                  // 0x228CC
    void initTurn();                  // 0x22980
    void initThreat();                // 0x229FC
    void initTurnAway();              // 0x22A94
    void initAttack();                // 0x22B10
    void initStop();                  // 0x22BD4
    void initEscape();                // 0x22C78
    virtual void onRelease();         // 0x22D10: = initRelease
    void executeWait();               // 0x22DC8
    void executeWalk();               // 0x22F8C
    void executeThreat();             // 0x231D0
    void executeTurn();               // 0x23334
    void executeTurnAway();           // 0x23548
    void executeAttack();             // 0x236A0
    void executeStop();               // 0x238F4
    void executeEscape();             // 0x23A2C
    void executeRelease();            // 0x23C00
    mAng getMoveAngle() const;        // 0x23DAC
    void getMoveTarget(mVec3_c *out); // 0x23E88
    bool isTarantula();               // 0x23FBC
    bool isInWater() const;           // 0x23FD0
    void setWalkTarget();             // 0x24054
    void setEscapeTarget();           // 0x240E8
    void walk(f32 lift, f32 accel, f32 drag,
              f32 turn);  // 0x2417C: fly(), upright on the ground
    void alignToGround(); // 0x241B0

    /* 0x318 */ int mState;     // State_e
    /* 0x31C */ int mPrevState; // at the start of executeState
    /* 0x320 */ u16 mWaitFrames;
    /* 0x322 */ u16 mWalkFrames;
    /* 0x324 */ u16 mEscapeFrames;
    /* 0x326 */ u16 mEscapeTimer;
    /* 0x328 */ u16 mWalkTimer;
    /* 0x32A */ u16 mWalkTime;
    /* 0x32C */ f32 mSavedScare; // initStop: mScare.mValue, kept while stopped
    /* 0x330 */ bool mLanded;    // executeRelease: the landing sound played
}; // size 0x334
