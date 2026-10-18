#pragma once

// The pond skater (INSECT_POND_SKATER): waits on the water and now and then
// glides to a random spot nearby. Online, the owner's state and target are
// shared through the insect's net slot (dInsectCatchData_c).

#include <game/game/d_insect_field.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/game/d_net.hpp>

class dInsFldStrider_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldStrider_c::*StateFunc)();

    enum State_e {
        STATE_WAIT = 0,
        STATE_MOVE = 1,    // glide to mHomePos
        STATE_RELEASE = 2, // released from the net
    };

    // Anim frames of STATE_MOVE.
    enum {
        GLIDE_START_FRAME = 12, // the glide (and its ripple) starts
        MOVE_END_FRAME = 79,    // the anim stops here
        MOVE_TIME = 60,         // mMoveFrames
    };

    // The per-type parameters (.data 0x13DC, one entry).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mOffsetY; // above the water
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 _0C;
        /* 0x10 */ f32 _10;
        /* 0x14 */ f32 mMoveDistMax;
        /* 0x18 */ f32 mMoveDistMin;
        /* 0x1C */ f32 mMoveDecay;    // speed factor per frame
        /* 0x20 */ f32 mReleaseSpeed; // moveForward
        /* 0x24 */ f32 mReleaseAccelY;
        /* 0x28 */ f32 mReleaseAnmRate;
        /* 0x2C */ f32 _2C;
        /* 0x30 */ f32 _30;
    }; // size 0x34

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sTypes[];

    static const param_c *getStriderParam(int type); // 0x183B4

    dInsFldStrider_c();               // 0x183E8
    virtual ~dInsFldStrider_c();      // 0x1844C
    virtual int draw();               // 0x184A4
    void setState(int state);         // 0x184C8: and the owner's net slot
    virtual void setGroundPos();      // 0x185F8
    virtual void initState();         // 0x18674
    virtual void executeState();      // 0x1867C
    void initWait();                  // 0x186FC
    void initMove();                  // 0x18778
    virtual void onRelease();         // 0x188B0: = initRelease
    void executeWait();               // 0x18948
    void executeMove();               // 0x18958
    void executeRelease();            // 0x18A80
    mAng getMoveAngle();              // 0x18AC4
    f32 getMoveDist() const;          // 0x18B10
    void getMoveTarget(mVec3_c *out); // 0x18B40
    f32 getWaterY();                  // 0x18CB8
    bool isBlockedWater(const mVec3_c *pos); // 0x18D0C: the sea (BG_ATTR_SEA)
    void syncState(); // 0x18D60: the owner's state (net) or the timers

    /* 0x318 */ int mState;     // State_e
    /* 0x31C */ int mPrevState; // executeState: set again if changed
    /* 0x320 */ mVec3_c mStartPos;
    /* 0x32C */ u16 mWaitFrames;
    /* 0x32E */ u16 mMoveFrames;
}; // size 0x330
