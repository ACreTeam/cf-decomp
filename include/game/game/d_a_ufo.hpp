#pragma once

// The UFO (REL d_a_ufoNP, profile A_UFO, RTTI daUfo_c). On the UFO visitor's
// day it flies over the town along a balloon route (dFdBase_c racco
// parameters); the slingshot shoots it down: it drops a part (dFgobjManager::
// dropUfoPart), explodes and flies off. Source: src/d_a_ufoNP/d_a_ufo.cpp.
// Names are inferred.

// The include order sets the vtable / RTTI order in .data (reverse class
// completion).
#include <game/mLib/m_3d.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_fltg.hpp>
#include <game/game/d_objc.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>

class daUfo_c : public dFltg::actor_c {
  public:
    enum STATE_e {
        STATE_HIDDEN, // waits for its minute (not in the dFltg list)
        STATE_ENTER,  // picks a route side and starts
        STATE_FLY,    // across the town
        STATE_TURN,   // waits at the route's end, then flies back
        STATE_SHOT,   // hit by the slingshot: drops a part
        STATE_LEAVE,  // flies off, exploding
        STATE_NUM,
    };

    typedef BOOL (daUfo_c::*InitFunc)();
    typedef void (daUfo_c::*ExecFunc)();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual int getKind() {
        return 1;
    }
    virtual int getRaccoIdx() {
        return mRaccoIdx;
    }

    static void *operator new(size_t size); // the static buffer
    static void operator delete(void *mem);

    void initWobble();
    void calcWobble();
    void calcExplosion();
    BOOL setState(int state);
    void executeState();

    BOOL initHidden();
    void executeHidden();
    BOOL initEnter();
    void executeEnter();
    BOOL initFly();
    void executeFly();
    BOOL initTurn();
    void executeTurn();
    BOOL initShot();
    void executeShot();
    BOOL initLeave();
    void executeLeave();

    static daUfo_c *s_instance;

    /* 0x0D0 */ m3d::mdl_c mMdl;
    /* 0x110 */ m3d::anmChr_c mAnmChr;
    /* 0x148 */ m3d::anmMatClr_c mAnmClr; // obj_ufo0_n, obj_ufo0_w when shot
    /* 0x174 */ int mRaccoIdx;
    /* 0x178 */ int mState;
    /* 0x17C */ u32 mFlyNum; // the flights across this time
    /* 0x180 */ f32 mEndX;   // the route's end
    /* 0x184 */ u8 _184[0x188 - 0x184];
    /* 0x188 */ f32 mRouteZ;
    /* 0x18C */ u8 mToRight; // flies toward +x
    /* 0x190 */ int mTurnWait;
    /* 0x194 */ dObjc::ac_c mObjc;
    /* 0x1D8 */ int mShotWait;
    /* 0x1DC */ u8 _1DC;
    /* 0x1DD */ u8 mExploding;
    /* 0x1E0 */ mVec3_c mHitOffset; // where the shot hit, from mPos
    /* 0x1EC */ mVec3_c mHitPos;    // the explosion's position (field)
    /* 0x1F8 */ dLevelEffect_c mExplosion;
    /* 0x28C */ f32 mRadius;
    /* 0x290 */ f32 mWobbleAmp;
    /* 0x294 */ s16 mWobblePhase;
    /* 0x296 */ s16 mWobbleSpeed;
    /* 0x298 */ s16 mBobSpeed;
    /* 0x29A */ s16 mBobPhase;
    /* 0x29C */ f32 mBobAmp;
    /* 0x2A0 */ bool mVisitorDay; // dEvent::isVisitorHere(VISITOR_UFO)
    /* 0x2A4 */ u32 mChance;      // of appearing at the next check (of 8)
    /* 0x2A8 */ u32 mLastMinute;
}; // size 0x2AC
