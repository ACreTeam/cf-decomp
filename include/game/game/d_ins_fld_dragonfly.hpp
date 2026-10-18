#pragma once

// The dragonflies (dInsectType_e 22..25): hover, turn and dart to a new spot,
// land on a stone or a perch site (dInsectUtil::siteSearch_c) and rest there,
// dart away from players and NPCs. REL d_insect_fieldNP .text 0x10C4C..0x13050.

#include <game/game/d_insect_field.hpp>

class dInsFldDragonfly_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldDragonfly_c::*StateFunc)();

    enum State_e {
        STATE_TURN,        // hovers and turns to the next spot (mTargetAngle)
        STATE_TURN_PERCH,  // turns to the perch it found
        STATE_FLY,         // darts to the spot
        STATE_APPROACH,    // flies onto the perch
        STATE_REST,        // sits on the perch
        STATE_TAKE_OFF,    // rises from the perch
        STATE_ESCAPE,      // darts away
        STATE_ESCAPE_TURN, // turns away from the players / NPCs
        STATE_LEAVE,       // released by a player

        STATE_NUM_MAX =
            14, // the net slot's state field: a larger value is a catch request
    };

    enum {
        PARAM_NUM = 4, // sParam: one per dragonfly type
    };

    enum Perch_e {
        PERCH_SITE = 0, // a perch site (dInsectUtil::siteSearch_c)
        PERCH_ITEM = 1, // a stone (mPerchSearch)
    };

    // The per-type behaviour (sParam, one per type).
    struct param_c {
        /* 0x00 */ f32 mOfsX;      // setGroundPos
        /* 0x04 */ f32 mHeight;    // flying height above the ground / water
        /* 0x08 */ f32 mOfsZ;      // setGroundPos
        /* 0x0C */ f32 mTurnSpeed; // degrees per frame
        /* 0x10 */ f32 mHoverAmp;
        /* 0x14 */ f32 mHoverPeriod; // seconds
        /* 0x18 */ f32 mFlyDistMax;
        /* 0x1C */ f32 mFlyDistMin;
        /* 0x20 */ f32 mFlyDecay; // the forward speed's decay per frame
        /* 0x24 */ f32 mFlyGravity;
        /* 0x28 */ u16 mFlyTime; // frames (x 0.8)
        /* 0x2A */ u16 _2A;
        /* 0x2C */ f32 mEscapeDistMax;
        /* 0x30 */ f32 mEscapeDistMin;
        /* 0x34 */ f32 mEscapeDecay;
        /* 0x38 */ f32 mEscapeGravity;
        /* 0x3C */ u16 mEscapeTime; // frames (x 0.7)
        /* 0x3E */ u16 _3E;
        /* 0x40 */ f32 mRestTimeMax; // seconds
        /* 0x44 */ f32 mRestTimeMin;
        /* 0x48 */ f32
            mSearchDelayMax; // seconds before a perch is looked for again
        /* 0x4C */ f32 mSearchDelayMin;
        /* 0x50 */ f32 mLeaveSpeed; // moveForward
        /* 0x54 */ f32 mLeaveAccelY;
        /* 0x58 */ f32 mLeaveAnmRate;
        /* 0x5C */ f32 _5C;
        /* 0x60 */ f32 _60;
        /* 0x64 */ f32 _64;
        /* 0x68 */ int _68;
    }; // size 0x6C

    dInsFldDragonfly_c();                               // 0x10D54
    virtual ~dInsFldDragonfly_c();                      // 0x10E00
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x10E74
    virtual int execute();                              // 0x10EC0
    virtual int draw();                                 // 0x10F78
    virtual void onBecomeOwner();                       // 0x10F9C
    virtual void setGroundPos();                        // 0x11254
    virtual void initState();                           // 0x112C4
    virtual void executeState();                        // 0x112CC
    virtual void onRelease(); // 0x11B3C: also the STATE_LEAVE init

    static bool isPerchItem(const dItem::Item *item,
                            const mVec3_c *pos);  // 0x10C4C
    static int getParamIdx(int type);             // 0x10CC8
    static const param_c *getKindParam(int type); // 0x10D24

    void setState(int state); // 0x110C4
    void initTurn();          // 0x11340
    void initTurnPerch();     // 0x113F0
    void initFly();           // 0x114A0
    void initApproach();      // 0x11634
    void initRest();          // 0x116F0
    void initTakeOff();       // 0x11830
    void initEscape();        // 0x11904
    void initEscapeTurn();    // 0x11A98
    void executeTurn();       // 0x11BC8
    void executeTurnPerch();  // 0x11C54
    void executeFly();        // 0x11CE0
    void executeApproach();   // 0x11D8C
    void executeRest();       // 0x11DF4
    void executeTakeOff();    // 0x11EB4
    void executeEscape();     // 0x11F1C
    void executeEscapeTurn(); // 0x11FC8
    void executeLeave();      // 0x12040
    void hover();             // 0x12084: bob up and down around mBaseY
    bool isBig() const;       // 0x121AC: the banded / petaltail dragonflies
    bool isPerchFound();      // 0x121D0
    bool canSearch();         // 0x12230
    bool isPerchBelow();      // 0x12250
    mAng getFlyAngle() const; // 0x12304
    f32 getFlyDist() const;   // 0x12408
    void getFlyTarget(mVec3_c *out); // 0x12460
    void getPerchPos(mVec3_c *out);  // 0x12758
    f32 getBaseHeight();             // 0x129B0
    void checkState();               // 0x12A30
    u16 getSearchDelay() const;      // 0x12FFC

    static param_c sParam[PARAM_NUM]; // .data 0xFA0
    static const StateFunc sExecute[];
    static const StateFunc sInit[];

    /* 0x318 */ int mState;     // State_e
    /* 0x31C */ int mPrevState; // mState before executeState's state function
    /* 0x320 */ dInsectUtil::itemSearch_c mPerchSearch;
    /* 0x334 */ dInsectUtil::siteSearch_c mSiteSearch;
    /* 0x340 */ mVec3_c mStartPos;
    /* 0x34C */ f32 mBaseY;    // the height the hover is around
    /* 0x350 */ f32 mHoverAmp; // 0..1
    /* 0x354 */ mAng mHoverPhase;
    /* 0x356 */ u16 _356;
    /* 0x358 */ u16 mFlyTime;
    /* 0x35A */ u16 mEscapeTime;
    /* 0x35C */ u16 mApproachTime;
    /* 0x35E */ u16 mRestTime;
    /* 0x360 */ u16 mTakeOffTime;
    /* 0x362 */ u16 mSearchTimer;
    /* 0x364 */ u16 mSearchDelay;
    /* 0x366 */ s16 mTargetAngle;
}; // size 0x368
