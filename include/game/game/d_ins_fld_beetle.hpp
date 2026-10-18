#pragma once

// The insects on tree trunks (dInsectType_e 10, 35, 42..55: the oak silk moth,
// the walking stick and the beetles): sit or crawl on the trunk, drop and run
// away when scared. The walking stick fades out instead. REL d_insect_fieldNP
// .text 0xAB8C..0xC5A8.

#include <game/game/d_insect_field.hpp>
#include <game/mLib/m_quat.hpp>

class dInsFldBeetle_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldBeetle_c::*StateFunc)();

    enum State_e {
        STATE_WAIT,   // sits still
        STATE_WALK,   // crawls (wobbles)
        STATE_ESCAPE, // runs off
        STATE_HIDE,   // the walking stick: fades out
        STATE_LEAVE,  // released by a player: walks off (the walking stick
                      // falls)

        STATE_NUM_MAX =
            14, // the net slot's state field: a larger value is a catch request
    };

    enum {
        TYPE_NUM = 16,      // the types (sParam, getKindParam's type list)
        WOBBLE_MAX = 0x889, // wobble(): mWobbleY turns at +-0x889 (~12 degrees)
    };

    // The per-type behaviour (sParam, by getKindParam's type list).
    struct param_c {
        /* 0x00 */ f32
            mOfsX; // setGroundPos: from mInitPos (y: above the ground)
        /* 0x04 */ f32 mOfsY;
        /* 0x08 */ f32 mOfsZ;
        /* 0x0C */ f32 mWaitTimeMax; // seconds
        /* 0x10 */ f32 mWaitTimeMin;
        /* 0x14 */ f32 _14;
        /* 0x18 */ f32 _18;
        /* 0x1C */ f32 mWalkAnmRate;
        /* 0x20 */ f32 mLeaveAngleX; // degrees
        /* 0x24 */ f32 mSpeed;       // moveForward
        /* 0x28 */ f32 mAccelY;
        /* 0x2C */ f32 mEscapeAnmRate;
        /* 0x30 */ f32 mObjcOfsX; // registObjc: from mPos
        /* 0x34 */ f32 mObjcOfsY;
        /* 0x38 */ f32 mObjcOfsZ;
        /* 0x3C */ f32 mShadowSize; // while escaping / leaving
        /* 0x40 */ f32 _40;
        /* 0x44 */ f32 _44;
        /* 0x48 */ f32
            mScareAngle; // degrees in front of the insect a player scares it
        /* 0x4C */ int _4C;
    }; // size 0x50

    dInsFldBeetle_c();            // 0xAD70
    virtual ~dInsFldBeetle_c();   // 0xAE74
    virtual void calcMtx();       // 0xB024
    virtual int draw();           // 0xAECC
    virtual void onBecomeOwner(); // 0xB0A8
    virtual void setGroundPos();  // 0xB148
    virtual void initState();     // 0xB1B8
    virtual void executeState();  // 0xB1C0
    virtual void onRelease();     // 0xB658: also the STATE_LEAVE init
    virtual void calcScare();     // 0xC134

    static const param_c *getKindParam(int type); // 0xAB8C
    static int getEscapeSound(int type);          // 0xAC80

    void setState(int state);      // 0xB0AC
    void initWait();               // 0xB27C
    void initWalk();               // 0xB3A0
    void initEscape();             // 0xB494
    void initHide();               // 0xB5C0
    void executeWait();            // 0xB774
    void executeWalk();            // 0xB834
    void executeEscape();          // 0xB910
    void executeHide();            // 0xB9BC
    void executeLeave();           // 0xB9D4
    void getRestRot(mQuat_c *out); // 0xBD64: facing down the trunk
    mAng getEscapeAngle();         // 0xBE7C
    bool isOakSilkMoth();          // 0xBF44
    bool isWalkingStick();         // 0xBF58
    bool isDisturbed();            // 0xBF6C
    u8 getHideAlpha();             // 0xBF78
    void checkState();             // 0xBFC0
    void wobble();                 // 0xC2F0

    static param_c sParam[TYPE_NUM]; // .data 0x588
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sSounds[];

    /* 0x318 */ int mState;       // State_e
    /* 0x31C */ int mPrevState;   // mState before executeState's state function
    /* 0x320 */ mQuat_c mRotFrom; // escape: turned from
    /* 0x330 */ mQuat_c mRotTo;   // to
    /* 0x340 */ mQuat_c mRot;     // calcMtx
    /* 0x350 */ mQuat_c mWobble;  // calcMtx, after mRot
    /* 0x360 */ u8 mWobbleDir;
    /* 0x362 */ mAng mWobbleY;
    /* 0x364 */ mAng mWobbleZ;
    /* 0x368 */ f32 mSlerp; // mRotFrom -> mRotTo
    /* 0x36C */ u16 mWaitTime;
    /* 0x36E */ u16 mWalkTime;
}; // size 0x370
