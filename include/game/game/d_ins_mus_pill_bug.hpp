#pragma once

// dInsMusPillBug_c: the pill bug and centipede of the museum's insect rooms.

#include <game/game/d_insect_museum.hpp>

struct dInsMusPillBugParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mWalkDistMax;
    /* 0x10 */ f32 mWalkDistMin;
    /* 0x14 */ f32 mWalkSpeed;
    /* 0x18 */ f32 mWalkAccel;
    /* 0x1C */ f32 mWalkBrake;
    /* 0x20 */ f32 mWalkTurnRate;
    /* 0x24 */ f32 mWalkAnmRate;
    /* 0x28 */ f32 mShadowSize;
    /* 0x2C */ f32 mScareRadius;
    /* 0x30 */ f32 mScareLimit; // f32 in the data
    /* 0x34 */ int mArea;
}; // size 0x38

class dInsMusPillBug_c : public dInsMusBase_c {
public:
    typedef void (dInsMusPillBug_c::*StateFunc)();

    enum {
        ROLL_END_FRAME = 150,
        UNROLL_END_FRAME = 249,
    };

    dInsMusPillBug_c();
    virtual ~dInsMusPillBug_c();
    virtual void calcMtx();
    virtual BOOL draw();
    virtual BOOL doDelete();
    virtual void createModel(nw4r::g3d::ResFile &res);
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWalk();
    void initRoll();
    void initUnroll();
    void stateWalk();
    void stateRoll();
    void stateUnroll();
    mAng getWalkAngle();
    f32 getWalkDist() const;
    void getWalkTarget(mVec3_c *target);
    bool isPillBug();
    void setAnmRate(f32 rate);

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ u16 mMoveTime;
    /* 0x220 */ nw4r::g3d::ResAnmTexSrt mResAnmTexSrt;
    /* 0x224 */ m3d::anmTexSrt_c mAnmTexSrt;
}; // size 0x250
