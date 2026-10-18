#pragma once

// The diving beetle (INSECT_DIVING_BEETLE): swims in ponds and rivers, now
// and then surfacing (catchable) and diving again. Online, the owner's state
// and target are shared through the insect's net slot.

#include <game/game/d_insect_field.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/game/d_net.hpp>

class dInsFldDivingBeetle_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldDivingBeetle_c::*StateFunc)();

    enum State_e {
        STATE_WAIT = 0,
        STATE_SWIM = 1,     // under the water
        STATE_SURFACE = 2,  // swim up into view (mVisAlpha up)
        STATE_DIVE = 3,     // swim down out of view
        STATE_RELEASE = 4,  // released from the net
        STATE_NUM_NET = 14, // syncState: higher net values are ignored
    };

    enum {
        SWIM_TIME = 150,        // mMoveFrames of STATE_SWIM / SURFACE / DIVE
        CATCHABLE_ALPHA = 0x80, // registObjc catchable from this mVisAlpha
    };

    // The per-type parameters (.data 0x1424, one entry).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mOffsetY; // below the water surface
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 mWaitMax; // seconds
        /* 0x10 */ f32 mWaitMin;
        /* 0x14 */ f32 mMoveDistMax;
        /* 0x18 */ f32 mMoveDistMin;
        /* 0x1C */ f32 mSwimAnmRateStart;
        /* 0x20 */ f32 mSwimAnmRateEnd;
        /* 0x24 */ f32 mSwimChance; // above this random value: STATE_SWIM, else
                                    // STATE_SURFACE
        /* 0x28 */ int mFadeFrames; // mVisAlpha 0 <-> 255
        /* 0x2C */ f32 mMoveDecay;  // speed factor per frame
        /* 0x30 */ f32 mReleaseSpeed; // moveForward
        /* 0x34 */ f32 mReleaseAccelY;
        /* 0x38 */ f32 mReleaseAnmRate;
        /* 0x3C */ f32 _3C;
        /* 0x40 */ f32 _40;
    }; // size 0x44

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sTypes[];

    static int getParamIdx(int type);                     // 0x18E6C
    static const param_c *getDivingBeetleParam(int type); // 0x18E8C

    dInsFldDivingBeetle_c();                            // 0x18EBC
    virtual ~dInsFldDivingBeetle_c();                   // 0x18F24
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x18F7C
    virtual int draw();                                 // 0x18FDC
    virtual void
    onBecomeOwner(); // 0x19000: back to the net slot's spot if under ground
    void setState(int state);         // 0x19130: and the owner's net slot
    virtual void setGroundPos();      // 0x192C0
    virtual void initState();         // 0x1933C
    virtual void executeState();      // 0x19344
    void initWait();                  // 0x193D0
    void initSwim();                  // 0x19490
    void initSurface();               // 0x195D0
    void initDive();                  // 0x19710
    virtual void onRelease();         // 0x19850: = initRelease
    void executeWait();               // 0x198F4
    void executeSwim();               // 0x19920
    void executeSurface();            // 0x19960
    void executeDive();               // 0x19A08
    void executeRelease();            // 0x19AA8
    mAng getMoveAngle();              // 0x19AEC
    f32 getMoveDist() const;          // 0x19B38
    void getMoveTarget(mVec3_c *out); // 0x19B68
    f32 getWaterY();                  // 0x19CFC
    void setMatAlpha(u8 alpha); // 0x19D50: TEV KColor 1 alpha of material "m0"
    void swim();                // 0x19E6C
    bool isBlockedWater(const mVec3_c *pos); // 0x19F88: the sea (BG_ATTR_SEA)
    void syncState(); // 0x19FDC: the owner's state (net) or the timers

    /* 0x318 */ int mState;     // State_e
    /* 0x31C */ int mPrevState; // executeState: set again if changed
    /* 0x320 */ mVec3_c mStartPos;
    /* 0x32C */ u16 mWaitFrames;
    /* 0x32E */ u16 mMoveFrames;
    /* 0x330 */ s16 mVisAlpha; // 255: surfaced
}; // size 0x334
