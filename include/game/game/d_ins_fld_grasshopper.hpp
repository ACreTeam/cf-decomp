#pragma once

// The hopping insects (dInsectType_e 13, 14, 30..32: the locusts, the crickets
// and the grasshopper): sit (the crickets chirp), turn to a spot nearby and hop
// there, hop away from players and NPCs. REL d_insect_fieldNP .text
// 0xC5A8..0xE2E0.

#include <game/game/d_insect_field.hpp>
#include <game/game/d_net.hpp>

class dInsFldGrasshopper_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldGrasshopper_c::*StateFunc)();

    enum State_e {
        STATE_WAIT,        // sits (the crickets chirp)
        STATE_TURN,        // turns to the next spot (mTargetAngle)
        STATE_JUMP,        // hops there
        STATE_ESCAPE_TURN, // turns away from the players / NPCs
        STATE_ESCAPE_JUMP, // hops away
        STATE_LEAVE,       // released by a player: hops off

        STATE_NUM_MAX =
            14, // the net slot's state field: a larger value is a catch request
    };

    enum {
        TYPE_NUM = 5,   // the types (sParam, getParamIdx's type list)
        JUMP_TIME = 25, // onRelease: the hop's frames (getJumpSpeedY)
    };

    // The per-type behaviour (sParam, one per type).
    struct param_c {
        /* 0x00 */ f32 mOfsX; // setGroundPos
        /* 0x04 */ f32 mOfsY;
        /* 0x08 */ f32 mOfsZ;
        /* 0x0C */ f32 mWaitTimeMax; // seconds
        /* 0x10 */ f32 mWaitTimeMin;
        /* 0x14 */ f32 mChirpAnmRate;
        /* 0x18 */ f32 mTurnAnmRate;
        /* 0x1C */ f32 mJumpDistMin;
        /* 0x20 */ f32 mJumpDistMax;
        /* 0x24 */ f32 mJumpDecay; // the forward speed's decay per frame
        /* 0x28 */ f32 mTurnSpeed; // degrees per frame
        /* 0x2C */ f32 mJumpGravity;
        /* 0x30 */ f32 mJumpAnmRate;
        /* 0x34 */ s16 _34; // 30
        /* 0x36 */ s16 _36;
        /* 0x38 */ f32 mEscapeDistMin;
        /* 0x3C */ f32 mEscapeDistMax;
        /* 0x40 */ f32 mEscapeDecay;
        /* 0x44 */ f32 mEscapeTurnSpeed;
        /* 0x48 */ f32 mEscapeGravity;
        /* 0x4C */ f32 mEscapeAnmRate;
        /* 0x50 */ s16 _50; // 40
        /* 0x52 */ s16 _52;
        /* 0x54 */ f32 mLeaveDecay;
        /* 0x58 */ f32 mLeaveGravity;
        /* 0x5C */ f32 _5C;
        /* 0x60 */ f32 _60;
        /* 0x64 */ f32 _64;
        /* 0x68 */ int _68;
        /* 0x6C */ int _6C;
    }; // size 0x70

    dInsFldGrasshopper_c();          // 0xC6A0
    virtual ~dInsFldGrasshopper_c(); // 0xC6FC
    virtual void calcMtx();          // 0xC778
    virtual int draw();              // 0xC754
    virtual void onBecomeOwner();    // 0xC77C
    virtual void setGroundPos();     // 0xCA38
    virtual void initState();        // 0xCAC4
    virtual void executeState();     // 0xCACC
    virtual void onRelease();        // 0xD138: also the STATE_LEAVE init

    static int getParamIdx(int type);             // 0xC5A8
    static const param_c *getKindParam(int type); // 0xC618
    static int getChirpSound(int type);           // 0xC648

    void setState(int state);         // 0xC8A8
    void initWait();                  // 0xCBE8
    void initTurn();                  // 0xCCFC
    void initJump();                  // 0xCDA4
    void initEscapeTurn();            // 0xCF10
    void initEscapeJump();            // 0xCFCC
    void executeWait();               // 0xD2F4
    void executeTurn();               // 0xD460
    void executeJump();               // 0xD5D4
    void executeEscapeTurn();         // 0xD790
    void executeEscapeJump();         // 0xD8F0
    void executeLeave();              // 0xDAAC
    mAng getJumpAngle() const;        // 0xDC00
    f32 getJumpDist();                // 0xDCE8
    void getJumpTarget(mVec3_c *out); // 0xDD3C
    bool isChirper(); // 0xDEDC: the crickets and the grasshopper
    mVec3_c getGroundNormal(const mVec3_c *pos); // 0xDF20
    void checkState();                           // 0xDF78
    f32 getJumpSpeedY(f32 gravity,
                      u16 frames); // 0xE230: up at the start, down at the end
    f32 getJumpSpeed(f32 dist,
                     f32 decay); // 0xE268: the start speed that decays to dist
    bool isOldPosInWater();      // 0xE280

    static param_c sParam[TYPE_NUM]; // .data 0xAB8
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static mAng sJumpAngleRange; // .bss 0x8 (0x2AAB, set in __sinit)

    /* 0x318 */ int mState;     // State_e
    /* 0x31C */ int mPrevState; // mState before executeState's state function
    /* 0x320 */ u8 mHasJumped;  // onRelease: hops up only if it hopped before
    /* 0x322 */ u16 mWaitTime;
    /* 0x324 */ u16 mJumpTime;
    /* 0x326 */ u16 mEscapeTime;
    /* 0x328 */ s16 mTargetAngle;
}; // size 0x32C
