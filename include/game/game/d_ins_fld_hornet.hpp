#pragma once

// The hornets (INSECT_BEE): the swarm that comes out of a fallen honeycomb
// (dInsectField_c::addHoneycombBees). They rise, chase the player and sting
// him (dPlayerMgr::requestBeeSting), or fly off when the player is safe.

#include <game/game/d_insect_field.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_shadow.hpp>

class dInsFldHornet_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldHornet_c::*StateFunc)();

    enum State_e {
        STATE_APPEAR = 0, // out of the honeycomb
        STATE_WAIT = 1,   // buzzing, the bee BGM starts after 120 frames
        STATE_CHASE = 2,  // after the player
        STATE_ESCAPE = 3, // the player is safe (demo, pitfall): fly away
        STATE_STING = 4,  // the sting hit: circle around the player, then leave
        STATE_RELEASE = 5, // released from the net
    };

    enum {
        WAIT_TIME = 120,     // STATE_WAIT: frames before the BGM and the chase
        WALL_TURN_TIME = 20, // initChase: frames against a wall before it turns
                             // off to the side
        STING_LEAVE_TIME = 195, // STATE_STING: may leave once behind the player
        STING_END_TIME = 300,   // STATE_STING: leaves
    };

    // The per-type parameters (.data 0x1310, one entry).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mOffsetY; // above the ground
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 mApproachSpeed; // initChase: frames = distance / speed
        /* 0x10 */ f32 mFlyAccel;      // fly()
        /* 0x14 */ f32 mFlyDrag;
        /* 0x18 */ f32 mFlyTurn;
        /* 0x1C */ f32 mNearRadius; // circles the player within this
        /* 0x20 */ f32 mObjcDelay;  // seconds before it can be caught
        /* 0x24 */ f32 mRiseSpeed;  // up to the player's height
        /* 0x28 */ f32 mEscapeTurn; // degrees per frame
        /* 0x2C */ f32 mEscapeSpeed;
        /* 0x30 */ f32 mCircleSpeed;  // degrees per frame
        /* 0x34 */ f32 mReleaseSpeed; // moveForward
        /* 0x38 */ f32 mReleaseAccelY;
        /* 0x3C */ f32 mReleaseAnmRate;
        /* 0x40 */ f32 _40;
        /* 0x44 */ f32 _44;
    }; // size 0x48

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sTypes[];

    static int getParamIdx(int type);               // 0x16340
    static const param_c *getHornetParam(int type); // 0x16360

    dInsFldHornet_c();                                  // 0x16390
    virtual ~dInsFldHornet_c();                         // 0x1646C
    static bool isBeesOut();                            // 0x16524: sBeeNum != 0
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x1653C
    virtual int draw();                                 // 0x16570
    virtual BOOL doDelete();                            // 0x16678
    virtual void calcMtx();          // 0x166B8: setMtxFromAxis
    virtual void onHold();           // 0x166BC: af_bug_bee_finish
    void setState(int state);        // 0x166FC
    virtual void setGroundPos();     // 0x16734
    virtual void initState();        // 0x167A4
    virtual void executeState();     // 0x167C0
    void initAppear();               // 0x16810
    void initWait();                 // 0x168D0
    void initChase();                // 0x168DC
    void initEscape();               // 0x16AEC
    void initSting();                // 0x16B5C
    virtual void onRelease();        // 0x16BF4: = initRelease
    void executeAppear();            // 0x16C9C
    void executeWait();              // 0x16CD8
    void executeChase();             // 0x16D34
    void executeEscape();            // 0x17160
    void executeSting();             // 0x17304
    void executeRelease();           // 0x1754C
    f32 getChaseHeight();            // 0x17590: 40 above the ground or water
    mAng getAngleToPlayer();         // 0x1760C
    f32 getTargetDist();             // 0x17618: 16
    void getTargetPos(mVec3_c *out); // 0x17624: a reachable spot in front
    void flyToHome(f32 lift, f32 accel, f32 drag, f32 turn); // 0x17798: fly()
    bool getTurnDir(); // 0x1779C: circle clockwise?

    static u8 sBeeNum; // .bss 0x14: the hornets out

    /* 0x318 */ dLevelEffect_c mGroupEffect; // af_bug_bee_group
    /* 0x3AC */ dLevelEffect_c mTailEffect;  // af_bug_bee_tail01
    /* 0x440 */ int mState;                  // State_e
    /* 0x444 */ bool mTurnDir;               // getTurnDir()
    /* 0x445 */ bool mFinished;              // onHold: af_bug_bee_finish done
    /* 0x446 */ bool mTurnDirSet;
    /* 0x448 */ mVec3_c mCenter; // initSting: the player position
    /* 0x454 */ f32 mRadius;
    /* 0x458 */ s16 mCircleAngle;
    /* 0x45A */ u16 _45A;         // 120 (initAppear)
    /* 0x45C */ u16 mChaseFrames; // frames to reach the target (initChase)
    /* 0x45E */ u16 mObjcTimer;   // up to mObjcDelay, then catchable
    /* 0x460 */ u16 mWallTimer;   // frames against a wall
}; // size 0x464
