#pragma once

// The walking leaf (INSECT_WALKING_LEAF): lies on the ground like a leaf until
// scared or an item is put on its unit, then gets up, walks away from the
// nearest player and fades out after a while.

#include <game/game/d_insect_field.hpp>

class dInsFldLeaf_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldLeaf_c::*StateFunc)();

    enum State_e {
        STATE_HIDE = 0,   // looks like a leaf
        STATE_REVEAL = 1, // gets up
        STATE_WALK = 2,
        STATE_RELEASE = 3, // released from the net
    };

    enum {
        REVEAL_END_FRAME = 59, // STATE_REVEAL: the get-up anim is done
        FADE_SPEED = 5,        // alpha per frame once mLifeTime is up
    };

    // The per-type parameters (.data 0x18D0, one entry).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mOffsetY;
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 mWalkDistMax;
        /* 0x10 */ f32 mWalkDistMin;
        /* 0x14 */ f32 mWalkSpeed;
        /* 0x18 */ f32 mWalkAccel; // fly()
        /* 0x1C */ f32 mWalkDrag;
        /* 0x20 */ f32 mWalkTurn;
        /* 0x24 */ f32 mWalkAnmRate;
        /* 0x28 */ f32 mLifeTime; // seconds walking, then it fades
        /* 0x2C */ f32 mGravity;
        /* 0x30 */ f32 _30;
        /* 0x34 */ f32 _34;
        /* 0x38 */ f32 _38;
        /* 0x3C */ int _3C;
    }; // size 0x40

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sTypes[];

    static int getParamIdx(int type);             // 0x25298
    static const param_c *getLeafParam(int type); // 0x252B8

    dInsFldLeaf_c();                  // 0x252E8
    virtual ~dInsFldLeaf_c();         // 0x2533C
    virtual BOOL doDelete();          // 0x25394
    virtual int draw();               // 0x253B8
    virtual void calcMtx();           // 0x253DC: setMtxFromAxis
    void setState(int state);         // 0x253E0
    virtual void setGroundPos();      // 0x25418
    virtual void initState();         // 0x25498
    virtual void executeState();      // 0x254A0
    void initHide();                  // 0x25594
    void initReveal();                // 0x2560C
    void initWalk();                  // 0x25690
    virtual void onRelease();         // 0x25780: = initRelease
    void executeHide();               // 0x25814
    void executeReveal();             // 0x25884
    void executeWalk();               // 0x258F4
    void executeRelease();            // 0x25A70
    mAng getWalkAngle();              // 0x25BF0: away from the nearest player
    f32 getWalkDist() const;          // 0x25C3C
    void getWalkTarget(mVec3_c *out); // 0x25C6C
    bool isItemPutHere(); // 0x25DD0: dInsectInfo::getPos() is on its unit
    void walk(f32 lift, f32 accel, f32 drag,
              f32 turn); // 0x25E74: fly(), upright on the ground

    /* 0x318 */ int mState;     // State_e
    /* 0x31C */ int mPrevState; // at the start of executeState
    /* 0x320 */ u16 mWalkFrames;
    /* 0x322 */ u16 mLifeTimer;
    /* 0x324 */ bool mLanded; // executeRelease: the landing sound played
}; // size 0x328
