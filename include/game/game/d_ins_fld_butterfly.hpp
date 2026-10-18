#pragma once

// The butterflies (dInsectType_e 0..8): flutter around their spot, fly to a
// flower, land and rest on it, take off again, flee from players and NPCs. REL
// d_insect_fieldNP .text 0x8268..0xA854.

#include <game/game/d_insect_field.hpp>

class dInsFldButterfly_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldButterfly_c::*StateFunc)();

    enum State_e {
        STATE_FLY,      // flutters around mHomePos (a new target every mFlyTime
                        // frames)
        STATE_APPROACH, // flies to the flower mFlowerSearch found
        STATE_LAND,     // settles on the flower
        STATE_REST,     // sits on the flower
        STATE_TAKE_OFF, // rises from the flower
        STATE_ESCAPE,   // flees from the players / NPCs
        STATE_LEAVE,    // flies away (released by a player)

        STATE_NUM_MAX =
            14, // the net slot's state field: a larger value is a catch request
    };

    enum {
        PARAM_NUM = 9,        // sParam: one per butterfly type
        LAND_EXTRA_TIME = 60, // initLand: frames added to the flight time
        TAKE_OFF_TIME = 70,
    };

    // The per-type behaviour (sParam, one per butterfly type).
    struct param_c {
        /* 0x00 */ f32 mOfsX;     // setGroundPos
        /* 0x04 */ f32 mHeight;   // flying height above the ground / water
        /* 0x08 */ f32 mOfsZ;     // setGroundPos
        /* 0x0C */ f32 mSpeed;    // units per frame to the target
        /* 0x10 */ f32 mFlyAccel; // fly() while flying
        /* 0x14 */ f32 mFlyDrag;
        /* 0x18 */ f32 mFlyTurn;
        /* 0x1C */ f32 mFlyDistMax; // the next target's distance
        /* 0x20 */ f32 mFlyDistMin;
        /* 0x24 */ f32 mFlutterSpeed;  // degrees per frame
        /* 0x28 */ f32 mFlutterAmpMin; // the up/down bob
        /* 0x2C */ f32 mFlutterAmpMax;
        /* 0x30 */ f32 mFlyAnmRate;
        /* 0x34 */ f32 mLandAnmRate;
        /* 0x38 */ f32 mRestTimeMax; // seconds on the flower
        /* 0x3C */ f32 mRestTimeMin;
        /* 0x40 */ f32 _40;
        /* 0x44 */ f32 mTakeOffAccel; // rising speed added per frame
        /* 0x48 */ f32 mTakeOffDrag;
        /* 0x4C */ f32 mTakeOffAnmRate;
        /* 0x50 */ f32 _50;
        /* 0x54 */ f32 mEscapeAccel; // fly() while escaping
        /* 0x58 */ f32 mEscapeDrag;
        /* 0x5C */ f32 mEscapeTurn;
        /* 0x60 */ f32 mEscapeDistMax;
        /* 0x64 */ f32 mEscapeDistMin;
        /* 0x68 */ f32 mEscapeFlutterSpeed;
        /* 0x6C */ f32 mEscapeFlutterAmpMin;
        /* 0x70 */ f32 mEscapeFlutterAmpMax;
        /* 0x74 */ f32 mEscapeAnmRate;
        /* 0x78 */ f32
            mSearchDelayMax; // seconds before a flower is looked for again
        /* 0x7C */ f32 mSearchDelayMin;
        /* 0x80 */ f32 mLeaveSpeed; // moveForward
        /* 0x84 */ f32 mLeaveAccelY;
        /* 0x88 */ f32 mLeaveAnmRate;
        /* 0x8C */ f32 _8C;
        /* 0x90 */ f32 mBgRadius; // the birdwing's mBgRadius (x 1.5)
        /* 0x94 */ f32 _94;
        /* 0x98 */ int _98;
    }; // size 0x9C

    dInsFldButterfly_c();                               // 0x8404
    virtual ~dInsFldButterfly_c();                      // 0x84A8
    virtual void calcMtx();                             // 0x8688
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x8510
    virtual int execute();                              // 0x85D4
    virtual int draw();                                 // 0x8664
    virtual void onBecomeOwner();                       // 0x8890
    virtual void setGroundPos();                        // 0x8B38
    virtual void initState();                           // 0x8BA8
    virtual void executeState();                        // 0x8BB0
    virtual void onRelease(); // 0x9164: also the STATE_LEAVE init

    static int getParamIdx(int type);             // 0x8268
    static const param_c *getKindParam(int type); // 0x8354
    static int getLeaveSound(int type);           // 0x8384
    static bool isLandingFlower(const dItem::Item *item,
                                const mVec3_c *pos); // 0x83B4

    void setState(int state);         // 0x89A8
    void initFly();                   // 0x8C24
    void initApproach();              // 0x8CF4
    void initLand();                  // 0x8DF8
    void initRest();                  // 0x8EF0
    void initTakeOff();               // 0x9000
    void initEscape();                // 0x9094
    void executeFly();                // 0x9228
    void executeApproach();           // 0x92AC
    void executeLand();               // 0x9380
    void executeRest();               // 0x93E8
    void executeTakeOff();            // 0x945C
    void executeEscape();             // 0x94DC
    void executeLeave();              // 0x9560
    void flutter();                   // 0x95A4: bob up and down around mBaseY
    void flutterApproach(f32 rate);   // 0x9804
    mAng getFlyAngle() const;         // 0x99BC
    f32 getFlyDist();                 // 0x9AA8
    void getNextTarget(mVec3_c *out); // 0x9B18
    void getFlowerPos(mVec3_c *out);  // 0x9C98
    f32 getBaseHeight();              // 0x9D88
    u16 getSearchDelay() const;       // 0x9E1C
    void checkState();                // 0x9E70
    void setFlyTarget();              // 0xA334
    void setEscapeTarget();           // 0xA510
    bool isFlowerFound();             // 0xA6F8
    bool canSearch();                 // 0xA700
    bool isOnFlower() const;          // 0xA720
    bool isBirdwing();                // 0xA798

    static param_c sParam[PARAM_NUM]; // .data 0x8
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sSounds[];

    /* 0x318 */ int mState;     // State_e
    /* 0x31C */ int mPrevState; // mState before executeState's state function
    /* 0x320 */ dInsectUtil::itemSearch_c mFlowerSearch;
    /* 0x334 */ mAng mFlutter;   // the bob phase
    /* 0x338 */ f32 mFlutterAmp; // the bob height
    /* 0x33C */ f32 mBaseY;      // the height the bob is around
    /* 0x340 */ mVec3_c mStartPos;
    /* 0x34C */ u16 mFlyTime;      // frames to the target (STATE_FLY)
    /* 0x34E */ u16 mApproachTime; // frames to the flower
    /* 0x350 */ u16 mLandTime;
    /* 0x352 */ u16 mRestTime;
    /* 0x354 */ u16 mTakeOffTime;
    /* 0x356 */ u16 mEscapeTime;
    /* 0x358 */ u16 mSearchDelay; // frames before a flower is looked for
    /* 0x35A */ u16 mSearchTimer;
    /* 0x35C */ mAng mSway; // calcMtx: the side to side sway phase
}; // size 0x360
