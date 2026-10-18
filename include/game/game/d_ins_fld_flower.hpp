#pragma once

// The insects on flowers (dInsectType_e 15, 16, 29, 37: the mantises, the snail
// and the ladybug): sit and crawl around their flower, turn to face a player
// (the mantises), flee when scared (the snail drops off). REL d_insect_fieldNP
// .text 0xE398..0xF9B0.

#include <game/game/d_insect_field.hpp>
#include <game/game/d_net.hpp>

class dInsFldFlower_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldFlower_c::*StateFunc)();

    enum State_e {
        STATE_WAIT,   // sits
        STATE_WALK,   // crawls around the flower
        STATE_TURN,   // turns to the nearest player (the snail hides in its
                      // shell)
        STATE_LOOK,   // the stop anim (forward)
        STATE_UNLOOK, // the stop anim backwards
        STATE_ESCAPE, // runs off (the snail drops)
        STATE_LEAVE,  // released by a player

        STATE_NUM_MAX =
            14, // the net slot's state field: a larger value is a catch request
    };

    enum { TYPE_NUM = 4 }; // the types (sParam, getKindParam's type list)

    // The per-type behaviour (sParam, by getKindParam's type list).
    struct param_c {
        /* 0x00 */ f32 _00;
        /* 0x04 */ f32 _04;
        /* 0x08 */ f32 _08;
        /* 0x0C */ f32 mWaitTimeMax; // seconds
        /* 0x10 */ f32 mWaitTimeMin;
        /* 0x14 */ f32 mWalkTimeMax;
        /* 0x18 */ f32 mWalkTimeMin;
        /* 0x1C */ f32 mWalkDistMax;
        /* 0x20 */ f32 mWalkDistMin;
        /* 0x24 */ f32 mWalkSpeed;
        /* 0x28 */ f32 mWalkTurn; // dInsectUtil::moveTo
        /* 0x2C */ f32 mWalkAnmRate;
        /* 0x30 */ f32 mGravity;   // the snail's fall
        /* 0x34 */ f32 mTurnSpeed; // degrees per frame
        /* 0x38 */ f32 mLookTime;  // seconds before it looks away
        /* 0x3C */ f32 mEscapeSpeed;
        /* 0x40 */ f32 mEscapeAccelY;
        /* 0x44 */ f32 mLeaveAnmRate;
        /* 0x48 */ f32 mShadowSize; // while escaping / leaving
        /* 0x4C */ f32 _4C;
        /* 0x50 */ f32 _50;
        /* 0x54 */ int _54;
        /* 0x58 */ int _58;
    }; // size 0x5C

    dInsFldFlower_c();                                  // 0xE4C4
    virtual ~dInsFldFlower_c();                         // 0xE540
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0xE598
    virtual int draw();                                 // 0xE5D4
    virtual void onBecomeOwner();                       // 0xE5F8
    virtual void setGroundPos();                        // 0xE67C
    virtual void initState();                           // 0xE728
    virtual void executeState();                        // 0xE730
    virtual void onRelease(); // 0xEC64: also the STATE_LEAVE init

    static const param_c *getKindParam(int type); // 0xE398
    static int getEscapeSound(int type);          // 0xE40C
    static bool isSittingFlower(const dItem::Item *item,
                                const mVec3_c *pos); // 0xE47C

    void setState(int state);         // 0xE5FC
    void initWait();                  // 0xE7D8
    void initWalk();                  // 0xE8CC
    void initTurn();                  // 0xE990
    void initLook();                  // 0xEA58
    void initUnlook();                // 0xEAD4
    void initEscape();                // 0xEB64
    void executeWait();               // 0xED44
    void executeWalk();               // 0xED54
    void executeTurn();               // 0xEDCC
    void executeLook();               // 0xEE50
    void executeUnlook();             // 0xEE60
    void executeEscape();             // 0xEEA8
    void executeLeave();              // 0xEF10
    mAng getEscapeAngle();            // 0xEF78
    bool isSnail();                   // 0xF010
    bool isNotLadybug();              // 0xF024
    void fall();                      // 0xF03C: the snail drops off the flower
    mAng getWalkAngle() const;        // 0xF1EC
    f32 getWalkDist() const;          // 0xF250
    void getWalkTarget(mVec3_c *out); // 0xF280
    void getFlowerTarget(mVec3_c *out); // 0xF368: x / z on the flower's unit
    bool isPlayerNear();                // 0xF3D8
    bool isOnFlower();                  // 0xF450
    void checkState();                  // 0xF50C
    void startWalk();                   // 0xF8F0

    static param_c sParam[TYPE_NUM]; // .data 0xCF0
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sSounds[];
    static f32 sTurnAnmRate; // .data 0xCE8

    /* 0x318 */ int mPrevState; // mState before executeState's state function
    /* 0x31C */ int mState;     // State_e
    /* 0x320 */ mVec3_c mFlowerPos; // the unit center of mInitPos
    /* 0x32C */ u16 mWaitTime;
    /* 0x32E */ u16 mWalkTime;  // frames to the walk target
    /* 0x330 */ u16 mWalkTotal; // frames of STATE_WALK
    /* 0x332 */ u16 _332;       // 150
    /* 0x334 */ u16 mWalkTimer; // reset by startWalk (never counted up)
    /* 0x336 */ s16 mTargetAngle;
    /* 0x338 */ u8 mBounce; // fall: landings so far
}; // size 0x33C
