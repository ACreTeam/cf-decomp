#pragma once

// The honeybee and the fly (dInsectType_e 11, 59): buzz around, fly to a flower
// (the fly: to a rafflesia, trash or spoiled turnips), land and rest there. REL
// d_insect_fieldNP .text 0x146DC..0x16340.

#include <game/game/d_insect_field.hpp>

class dInsFldBee_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldBee_c::*StateFunc)();

    enum State_e {
        STATE_FLY,      // buzzes around (a new target every mFlyTime frames)
        STATE_APPROACH, // flies to the food mFoodSearch found
        STATE_LAND,     // settles on it
        STATE_REST,     // sits on it (the bee crawls around)
        STATE_LEAVE,    // released by a player
    };

    enum { TYPE_NUM = 2 }; // the types (sParam, getParamIdx's type list)

    // The per-type behaviour (sParam, one per type).
    struct param_c {
        /* 0x00 */ f32 mOfsX;   // setGroundPos
        /* 0x04 */ f32 mHeight; // flying height above the ground / water
        /* 0x08 */ f32 mOfsZ;   // setGroundPos
        /* 0x0C */ f32 mFlyDistMax;
        /* 0x10 */ f32 mFlyDistMin;
        /* 0x14 */ f32 mSpeed;    // units per frame to the target
        /* 0x18 */ f32 mFlyAccel; // fly() while flying
        /* 0x1C */ f32 mFlyDrag;
        /* 0x20 */ f32 mFlyTurn;
        /* 0x24 */ f32 mFlutterSpeed; // degrees per frame
        /* 0x28 */ f32 mFlutterAmpMin;
        /* 0x2C */ f32 mFlutterAmpMax;
        /* 0x30 */ f32 mFlyAnmRate;
        /* 0x34 */ f32 mPitchSpeed; // degrees per frame
        /* 0x38 */ f32 mPitchAmp;   // degrees
        /* 0x3C */ f32 mEscapeSpeed;
        /* 0x40 */ f32 mEscapeAccel; // fly() while scared
        /* 0x44 */ f32 mEscapeDrag;
        /* 0x48 */ f32 mEscapeTurn;
        /* 0x4C */ f32 mEscapeHeight; // x mHeight while scared
        /* 0x50 */ f32 mEscapeAnmRate;
        /* 0x54 */ f32 _54;
        /* 0x58 */ f32 _58;
        /* 0x5C */ f32 mApproachSpeed;
        /* 0x60 */ f32 mApproachAccel; // fly() while approaching
        /* 0x64 */ f32 mApproachDrag;
        /* 0x68 */ f32 mApproachTurn;
        /* 0x6C */ f32 mRestTimeMax; // seconds
        /* 0x70 */ f32 mRestTimeMin;
        /* 0x74 */ f32 mRestDistMax; // crawling on the food
        /* 0x78 */ f32 mRestDistMin;
        /* 0x7C */ f32 mRestSpeed; // dInsectUtil::flyTo
        /* 0x80 */ f32 mRestTurn;
        /* 0x84 */ f32
            mSearchDelayMax; // seconds before food is looked for again
        /* 0x88 */ f32 mSearchDelayMin;
        /* 0x8C */ f32 mRestAnmRate;
        /* 0x90 */ f32 mLeaveSpeed; // moveForward
        /* 0x94 */ f32 mLeaveAccelY;
        /* 0x98 */ f32 mLeaveAnmRate;
        /* 0x9C */ f32 _9C;
        /* 0xA0 */ f32 _A0;
        /* 0xA4 */ f32 _A4;
        /* 0xA8 */ int _A8;
    }; // size 0xAC

    dInsFldBee_c();                                     // 0x14834
    virtual ~dInsFldBee_c();                            // 0x148E0
    virtual void calcMtx();                             // 0x14AF8
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x14948
    virtual int execute();                              // 0x14A34
    virtual int draw();                                 // 0x14AD4
    virtual void setGroundPos();                        // 0x14B34
    virtual void initState();                           // 0x14BBC
    virtual void executeState();                        // 0x14BC4
    virtual void onRelease(); // 0x150B0: also the STATE_LEAVE init

    static bool isBeeFood(const dItem::Item *item,
                          const mVec3_c *pos); // 0x146DC
    static bool isFlyFood(const dItem::Item *item,
                          const mVec3_c *pos);    // 0x1472C
    static int getParamIdx(int type);             // 0x147D0
    static const param_c *getKindParam(int type); // 0x14804

    void setState(int state); // 0x14AFC
    void initFly();           // 0x14C94
    void initApproach();      // 0x14D9C
    void initLand();          // 0x14E90
    void initRest();          // 0x14FC4
    void executeFly();        // 0x15174
    void executeApproach();   // 0x15364
    void executeLand();       // 0x1550C
    void executeRest();       // 0x1569C
    void executeLeave();      // 0x15818
    void flyPitch(f32 accel, f32 drag, f32 turn,
                  mAng pitch);        // 0x1585C: fly(), the axis pitched
    void flutter();                   // 0x15908: bob up and down around mBaseY
    void flutterApproach(f32 rate);   // 0x15A64
    void crawl();                     // 0x15C1C: dInsectUtil::flyTo mHomePos
    bool canSearch();                 // 0x15C84
    void getFoodPos(mVec3_c *out);    // 0x15CA4: on the ground under the food
    mAng getFlyAngle() const;         // 0x15D30
    f32 getFlyDist() const;           // 0x15D94
    void getFlyTarget(mVec3_c *out);  // 0x15DEC
    f32 getBaseHeight();              // 0x15F90
    bool isFly();                     // 0x16048
    void getFoodPosFly(mVec3_c *out); // 0x1605C
    bool isOnFood();                  // 0x160E0
    void getAreaTarget(mVec3_c *out); // 0x161D0: x / z around mFlyCenter
    bool isFoodAway(); // 0x16294: the food found is at least 32 away

    static param_c sParam[TYPE_NUM]; // .data 0x11B8
    static const StateFunc sExecute[];
    static const StateFunc sInit[];

    /* 0x318 */ int mState; // State_e
    /* 0x31C */ f32 mBaseY; // the height the bob is around
    /* 0x320 */ mVec3_c mStartPos;
    /* 0x32C */ mAng mFlutter; // the bob phase
    /* 0x330 */ f32 mFlutterAmp;
    /* 0x334 */ u16 mSearchDelay; // frames before food is looked for
    /* 0x336 */ u16 mSearchTimer;
    /* 0x338 */ dInsectUtil::itemSearch_c mFoodSearch;
    /* 0x34C */ u16 mFlyTime;
    /* 0x34E */ u16 mApproachTime;
    /* 0x350 */ u16 mLandTime;
    /* 0x352 */ u16 mRestTime;
    /* 0x354 */ mAng mPitchPhase;
    /* 0x356 */ mAng mPitch;
    /* 0x358 */ mVec3_c mFlyCenter; // mInitPos
}; // size 0x364
