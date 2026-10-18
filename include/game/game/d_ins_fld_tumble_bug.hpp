#pragma once

// The dung beetle (INSECT_DUNG_BEETLE): only in winter, pushing one of the two
// snowballs (daSnowMan_c::sBall). It stands behind the nearest ball, walks
// it around and pushes it along walls; scared, it runs away.

#include <game/game/d_insect_field.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/game/d_a_snowman.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_net.hpp>

class dInsFldTumbleBug_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldTumbleBug_c::*StateFunc)();

    enum State_e {
        STATE_WAIT = 0,
        STATE_WALK = 1, // roll the ball to a random spot
        STATE_PUSH = 2, // push the ball along a wall
        STATE_ESCAPE = 3,
        STATE_RELEASE = 4, // released from the net
    };

    // mPushTimer: set to PUSH_START_TIME as STATE_PUSH starts and to
    // PUSH_COOLDOWN_TIME every STATE_PUSH frame (no new push this long after).
    enum {
        PUSH_START_TIME = 180,
        PUSH_COOLDOWN_TIME = 300,
    };

    // The per-type parameters (.data 0x14D0, one entry).
    struct param_c {
        /* 0x00 */ f32 _00;
        /* 0x04 */ f32 _04;
        /* 0x08 */ f32 _08;
        /* 0x0C */ f32 mWaitMax; // seconds
        /* 0x10 */ f32 mWaitMin;
        /* 0x14 */ f32 mWaitDecay;   // speed factor per frame
        /* 0x18 */ f32 mWalkTimeMax; // seconds
        /* 0x1C */ f32 mWalkTimeMin;
        /* 0x20 */ f32 mWalkDistMax;
        /* 0x24 */ f32 mWalkDistMin;
        /* 0x28 */ f32 mWalkSpeed;
        /* 0x2C */ f32 mWalkAccel; // fly(), scaled by the ball size
        /* 0x30 */ f32 mWalkDrag;
        /* 0x34 */ f32 mWalkTurn;
        /* 0x38 */ f32 mWalkAnmRate; // played backwards
        /* 0x3C */ f32 mEscapeSpeed; // moveForward
        /* 0x40 */ f32 mEscapeAccelY;
        /* 0x44 */ f32 mEscapeAnmRate;
        /* 0x48 */ f32 _48;
        /* 0x4C */ f32 _4C;
        /* 0x50 */ f32 _50;
        /* 0x54 */ int _54;
    }; // size 0x58

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sNoBall; // findNearestBall: no ball near
    static const int sTypes[];

    // The snowball it pushes (an index in daSnowMan_c::sBall).
    class ball_c {
      public:
        ball_c();  // 0x1CEAC
        ~ball_c(); // 0x1CEB8

        void set(int idx);              // 0x1CEF8
        bool isValid();                 // 0x1CF00
        BOOL canRoll();                 // 0x1CF24
        bool isFar(const mVec3_c *pos); // 0x1CF6C: pos is off the ball's edge
        bool getWallNormal(mVec3_c *normal);   // 0x1D030
        bool cannotPush();                     // 0x1D088
        f32 getRadius();                       // 0x1D0D8
        daSnowMan_c *get();                    // 0x1D130
        void getPos(mVec3_c *out);             // 0x1D180
        mAng getAngleFrom(const mVec3_c *pos); // 0x1D1E0

        /* 0x0 */ int mIdx;
    }; // size 0x4

    static int getParamIdx(int type);                  // 0x1BBB0
    static const param_c *getTumbleBugParam(int type); // 0x1BBD0

    dInsFldTumbleBug_c();                               // 0x1BC00
    virtual ~dInsFldTumbleBug_c();                      // 0x1BCA0
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x1BD08
    virtual int draw();                // 0x1BD84: turned around
    virtual BOOL doDelete();           // 0x1BDE4
    virtual void calcMtx();            // 0x1BE1C: setMtxFromAxis
    static BOOL isBallTaken(int ball); // 0x1BE20
    // 0x1BE30: the manager's spawn check (offline, the ball free, optionally
    // away from the players).
    static BOOL canSpawn(const mVec3_c *pos, int ball, void *awayFromPlayers);
    void setState(int state);         // 0x1BEF8
    virtual void setGroundPos();      // 0x1BF30: behind the nearest ball
    virtual void initState();         // 0x1C024
    virtual void executeState();      // 0x1C02C
    void initWait();                  // 0x1C11C
    void initWalk();                  // 0x1C1C4
    void initPush();                  // 0x1C27C
    void initEscape();                // 0x1C2EC
    virtual void onRelease();         // 0x1C3F4: = initRelease
    void executeWait();               // 0x1C4A8
    void executeWalk();               // 0x1C674
    void executePush();               // 0x1C878
    void executeEscape();             // 0x1CA5C
    void executeRelease();            // 0x1CAA0
    int findNearestBall();            // 0x1CAE4: sNoBall = none
    mAng getWalkAngle();              // 0x1CBBC
    f32 getWalkDist() const;          // 0x1CC20
    void getWalkTarget(mVec3_c *out); // 0x1CC50
    void setWalkTarget();             // 0x1CDC4
    void walk(f32 lift, f32 accel, f32 drag,
              f32 turn); // 0x1CE58: fly(), upright

    static u8
        sBallTaken[daSnowMan_c::BALL_NUM]; // .bss 0x1C (dtk's 0x10 object also
                                           // holds the 0x20 dtor chain node of
                                           // the firefly's l_glowColorA): a
                                           // dung beetle pushes ball i (set by
                                           // the manager's spawn)

    /* 0x318 */ int mState; // State_e
    /* 0x31C */ mVec3_c mWallNormal;
    /* 0x328 */ ball_c mBall;
    /* 0x32C */ u16 mWaitFrames;
    /* 0x32E */ u16 mMoveFrames;
    /* 0x330 */ u16 mWalkTimer;
    /* 0x332 */ u16 mWalkFrames;
    /* 0x334 */ u16 _334; // 480
    /* 0x336 */ s16 _336; // 0.05 degrees
    /* 0x338 */ s16
        mPushTimer; // no new push before it runs out; the bg check runs while 0
}; // size 0x33C
