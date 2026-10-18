#pragma once

// The cicadas and the lantern fly (dInsectType_e 17..21): sit on tree trunks
// and sing, fly off when scared. The lantern fly flashes its wings instead of
// singing. REL d_insect_fieldNP .text 0xF9B0..0x10C4C.

#include <game/game/d_insect_field.hpp>
#include <game/mLib/m_quat.hpp>

class dInsFldCicada_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldCicada_c::*StateFunc)();

    enum State_e {
        STATE_WAIT,   // sings (the lantern fly: closes its wings)
        STATE_ALERT,  // stops singing (the lantern fly: opens its wings)
        STATE_ESCAPE, // flies off
        STATE_LEAVE,  // released by a player

        STATE_NUM_MAX =
            14, // the net slot's state field: a larger value is a catch request
    };

    enum { TYPE_NUM = 5 }; // the types (sParam, getParamIdx's type list)

    // The per-type behaviour (sParam, one per type).
    struct param_c {
        /* 0x00 */ f32
            mOfsX; // setGroundPos: from mInitPos (y: above the ground)
        /* 0x04 */ f32 mOfsY;
        /* 0x08 */ f32 mOfsZ;
        /* 0x0C */ f32 mWaitAnmRate;
        /* 0x10 */ f32 mSpeed; // moveForward
        /* 0x14 */ f32 mAccelY;
        /* 0x18 */ f32 mEscapeAnmRate;
        /* 0x1C */ f32 mObjcOfsX; // registObjc: from mPos
        /* 0x20 */ f32 mObjcOfsY;
        /* 0x24 */ f32 mObjcOfsZ;
        /* 0x28 */ f32 mShadowSize; // while escaping / leaving
        /* 0x2C */ f32 _2C;
        /* 0x30 */ f32 _30;
        /* 0x34 */ f32
            mScareAngle; // degrees in front of the insect a player scares it
        /* 0x38 */ int _38;
        /* 0x3C */ int _3C;
    }; // size 0x40

    dInsFldCicada_c();                                  // 0xFAA8
    virtual ~dInsFldCicada_c();                         // 0xFB74
    virtual void calcMtx();                             // 0xFD48
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0xFBCC
    virtual int draw();                                 // 0xFBF0
    virtual void onBecomeOwner();                       // 0xFDB0
    virtual void setGroundPos();                        // 0xFE34
    virtual void initState();                           // 0xFEA4
    virtual void executeState();                        // 0xFEAC
    virtual void onRelease(); // 0x10248: also the STATE_LEAVE init
    virtual void calcScare(); // 0x10A90

    static int getParamIdx(int type);             // 0xF9B0
    static const param_c *getKindParam(int type); // 0xFA20
    static int getSingSound(int type);            // 0xFA50

    void setState(int state);      // 0xFDB4
    void initWait();               // 0xFF68
    void initAlert();              // 0x10050
    void initEscape();             // 0x10120
    void executeWait();            // 0x103EC
    void executeAlert();           // 0x10524
    void executeEscape();          // 0x105FC
    void executeLeave();           // 0x10684
    void getRestRot(mQuat_c *out); // 0x106C8: facing down the trunk
    mAng getEscapeAngle();         // 0x107E0
    bool isLanternFly();           // 0x108A8
    bool isDisturbed();            // 0x108BC
    void checkState();             // 0x108C8

    static param_c sParam[TYPE_NUM]; // .data 0xE60
    static const StateFunc sExecute[];
    static const StateFunc sInit[];

    /* 0x318 */ int mState;       // State_e
    /* 0x31C */ int mPrevState;   // mState before executeState's state function
    /* 0x320 */ int mLastState;   // the same, read by initWait
    /* 0x324 */ mQuat_c mRotFrom; // escape: turned from
    /* 0x334 */ mQuat_c mRotTo;   // to
    /* 0x344 */ mQuat_c mRot;     // calcMtx
    /* 0x354 */ f32 mSlerp;       // mRotFrom -> mRotTo
}; // size 0x358
