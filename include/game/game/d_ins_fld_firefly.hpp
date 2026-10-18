#pragma once

// The firefly (INSECT_FIREFLY): bobs over the water at night, glowing (a
// dEffectLight_c glow plus af_ins_hotaru_glow), and drifts to random spots.
// Online, the owner's state and target are shared through the net slot.

#include <game/game/d_insect_field.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_shadow.hpp>
#include <game/mLib/m_color.hpp>

class dInsFldFirefly_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldFirefly_c::*StateFunc)();

    enum State_e {
        STATE_FLY = 0,
        STATE_RELEASE = 1,  // released from the net
        STATE_NUM_NET = 14, // syncState: higher net values are ignored
    };

    // The per-type parameters (.data 0x1528, one entry).
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
        /* 0x28 */ f32 mScaredSpeed;
        /* 0x2C */ f32 mScaredTurn;
        /* 0x30 */ f32 mScaredHeight; // mHeight factor
        /* 0x34 */ f32 mReleaseSpeed; // moveForward
        /* 0x38 */ f32 mReleaseAccelY;
        /* 0x3C */ f32 mReleaseAnmRate;
        /* 0x40 */ f32 _40;
        /* 0x44 */ f32 _44;
        /* 0x48 */ f32 _48;
        /* 0x4C */ int _4C;
    }; // size 0x50

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sTypes[];

    // Its dEffectLight_c glow (pulsing between l_glowColorA and l_glowColorB).
    class glow_c {
      public:
        glow_c();  // 0x1E1F8
        ~glow_c(); // 0x1E210

        void init();                   // 0x1E250
        void draw(const mVec3_c *pos); // 0x1E2E4

        /* 0x0 */ int mPhase; // an angle
        /* 0x4 */ int mSpeed; // per frame; new at each zero crossing
        /* 0x8 */ u8 mAlpha;
    }; // size 0xC

    static const param_c *getFireflyParam(int type); // 0x1D24C

    dInsFldFirefly_c();                                 // 0x1D280
    virtual ~dInsFldFirefly_c();                        // 0x1D328
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x1D3C0
    virtual int draw(); // 0x1D400: glow + shadow only
    virtual void
    onBecomeOwner(); // 0x1D4A0: back to the net slot's spot if under ground
    void setState(int state);        // 0x1D5B8: and the owner's net slot
    virtual void setGroundPos();     // 0x1D748
    virtual void initState();        // 0x1D7B8
    virtual void executeState();     // 0x1D7C0
    void initFly();                  // 0x1D834
    virtual void onRelease();        // 0x1D8C4: = initRelease
    void executeFly();               // 0x1D95C
    void executeRelease();           // 0x1DA08
    void bob();                      // 0x1DA98
    mAng getFlyAngle();              // 0x1DBF8
    f32 getFlyDist() const;          // 0x1DC5C
    void getFlyTarget(mVec3_c *out); // 0x1DC8C
    f32 getFlyHeight();              // 0x1DDF4
    void syncState();    // 0x1DE98: the owner's state (net) or the timer
    void setFlyTarget(); // 0x1E004: and the owner's net slot

    /* 0x318 */ int mState;     // State_e
    /* 0x31C */ int mPrevState; // executeState: set again if changed
    /* 0x320 */ glow_c mGlow;
    /* 0x32C */ f32 mBobHeight;
    /* 0x330 */ f32 mBaseY; // follows getFlyHeight()
    /* 0x334 */ f32 _334;
    /* 0x338 */ u16 mMoveFrames;
    /* 0x33A */ s16 mBobPhase;
    /* 0x33C */ dLevelEffect_c mGlowEffect; // af_ins_hotaru_glow
}; // size 0x3D0
