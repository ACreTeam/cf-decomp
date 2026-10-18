#pragma once

// The moth (dInsectType_e 9): flutters around a light at night (a lit villager
// house, the town hall or the gate) and flies off when the light goes out or it
// strays too far. REL d_insect_fieldNP .text 0x13050..0x146DC.

#include <game/game/d_insect_field.hpp>
#include <game/game/d_animal.hpp>

class dInsFldMoth_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldMoth_c::*StateFunc)();

    enum State_e {
        STATE_FLY,    // flutters around the light (a new target every mFlyTime
                      // frames)
        STATE_GLIDE,  // drifts (60 frames)
        STATE_ESCAPE, // flies from the players / NPCs (or the light went out)
        STATE_AWAY,   // flies off (strayed too far from mInitPos)
        STATE_LEAVE,  // released by a player

        STATE_NUM_MAX =
            14, // the net slot's state field: a larger value is a catch request
    };

    enum {
        TYPE_NUM = 1,    // the types (sParam, getParamIdx's type list)
        GLIDE_TIME = 60, // STATE_GLIDE: frames before STATE_FLY
    };

    // mLight
    enum Light_e {
        LIGHT_HOUSE,     // a villager house (mHouse)
        LIGHT_TOWN_HALL, // the nearer of the town hall / the gate
        LIGHT_GATE,
    };

    // The behaviour (sParam).
    struct param_c {
        /* 0x00 */ f32 mOfsX;     // setGroundPos
        /* 0x04 */ f32 mHeight;   // flying height above the ground / water
        /* 0x08 */ f32 mOfsZ;     // setGroundPos
        /* 0x0C */ f32 mSpeed;    // units per frame to the target
        /* 0x10 */ f32 mFlyAccel; // fly() while flying
        /* 0x14 */ f32 mFlyDrag;
        /* 0x18 */ f32 mFlyTurn;
        /* 0x1C */ f32 mFlyAnmRate;
        /* 0x20 */ f32 _20;
        /* 0x24 */ f32 mEscapeAccel; // fly() while escaping
        /* 0x28 */ f32 mEscapeDrag;
        /* 0x2C */ f32 mEscapeTurn;
        /* 0x30 */ f32 _30;
        /* 0x34 */ f32 mFlyDistMax;
        /* 0x38 */ f32 mFlyDistMin;
        /* 0x3C */ f32 mFlutterSpeed; // degrees per frame
        /* 0x40 */ f32 mFlutterAmpMin;
        /* 0x44 */ f32 mFlutterAmpMax;
        /* 0x48 */ f32 mRange; // STATE_AWAY beyond this distance from mInitPos
        /* 0x4C */ f32 mLeaveSpeed; // moveForward
        /* 0x50 */ f32 mLeaveAccelY;
        /* 0x54 */ f32 mLeaveAnmRate;
        /* 0x58 */ f32 _58;
        /* 0x5C */ f32 _5C;
        /* 0x60 */ f32 _60;
        /* 0x64 */ int _64;
    }; // size 0x68

    dInsFldMoth_c();                                    // 0x130A0
    virtual ~dInsFldMoth_c();                           // 0x13114
    virtual void calcMtx();                             // 0x1316C
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x1332C
    virtual int draw();                                 // 0x133BC
    virtual void setGroundPos();                        // 0x13570
    virtual void initState();                           // 0x135E0
    virtual void executeState();                        // 0x135E8
    virtual void onRelease(); // 0x139B8: also the STATE_LEAVE init

    static int getParamIdx(int type);             // 0x13050
    static const param_c *getKindParam(int type); // 0x13070

    void setState(int state);        // 0x133E0
    void initFly();                  // 0x13744
    void initGlide();                // 0x137E4
    void initEscape();               // 0x1387C
    void initAway();                 // 0x13914
    void executeFly();               // 0x13A70
    void executeGlide();             // 0x13B10
    void executeEscape();            // 0x13BE0
    void executeAway();              // 0x13C80
    void executeLeave();             // 0x13CC4
    void flutter();                  // 0x13D08: bob up and down around mBaseY
    mAng getFlyAngle() const;        // 0x13E68
    f32 getFlyDist() const;          // 0x13F54
    void getFlyTarget(mVec3_c *out); // 0x13FA8
    f32 getBaseHeight();             // 0x14128
    bool isLightOff() const;         // 0x141A8: mHouse's light went out
    void setFlyTarget();             // 0x14288
    void setEscapeTarget();          // 0x1431C
    bool isTooFar();                 // 0x143BC
    mVec3_c getTownHallPos() const;  // 0x14434
    mVec3_c getGatePos() const;      // 0x144B8
    int getLight() const;            // 0x1453C: Light_e
    void flyAxis(f32 lift, f32 accel, f32 drag,
                 f32 turn);           // 0x14608: fly(), the axis kept upright
    void getHomeTarget(mVec3_c *out); // 0x1466C: x / z near mInitPos

    static param_c sParam[TYPE_NUM]; // .data 0x1150
    static const StateFunc sExecute[];
    static const StateFunc sInit[];

    /* 0x318 */ int mState;  // State_e
    /* 0x31C */ u8 mAtHouse; // spawned at a lit villager house
                             // (dInsectField::sLightHouseFound)
    /* 0x320 */ int
        mHouse; // its dSaveBuildingList_c npc house index, -1 = none
    /* 0x324 */ int mLight; // Light_e
    /* 0x328 */ f32 mBaseY; // the height the bob is around
    /* 0x32C */ f32 mFlutterAmp;
    /* 0x330 */ mAng mFlutter; // the bob phase
    /* 0x332 */ u16 mFlyTime;
    /* 0x334 */ u16 mEscapeTime;
    /* 0x336 */ s16 mSway; // calcMtx: the side to side sway phase
}; // size 0x338
