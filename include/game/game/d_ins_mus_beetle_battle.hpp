#pragma once

// dInsMusBeetleBattle_c: the hercules beetle and golden stag fighting on the museum's tree stump.

#include <game/game/d_insect_museum.hpp>
#include <lib/egg/core/eggFrmHeap.h>

struct dInsMusBattleBeetleParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mTurnRate;
    /* 0x10 */ f32 mAccel;
    /* 0x14 */ f32 mFriction;
    /* 0x18 */ f32 mLiftAnmRate;
    /* 0x1C */ f32 mThrowFriction;
    /* 0x20 */ f32 mBounceSpeed;
    /* 0x24 */ f32 mWalkAnmRate;
    /* 0x28 */ f32 mFallHeight;
    /* 0x2C */ f32 mShadowSize;
}; // size 0x30

namespace dInsectMuseum {

extern dInsMusBattleBeetleParam_c sBattleBeetleParam[];

} // namespace dInsectMuseum

class dInsMusBeetleBattle_c {
public:
    typedef void (dInsMusBeetleBattle_c::*StateFunc)();

    enum {
        BEETLE_HERCULES,
        BEETLE_STAG,
        BEETLE_NUM,
    };

    class beetle_c {
    public:
        typedef void (beetle_c::*StateFunc)();

        enum State_e {
            STATE_FACE,
            STATE_CHARGE,
            STATE_BACK,
            STATE_PUSH,
            STATE_LIFT,
            STATE_THROW,
            STATE_RECOVER,
            STATE_RETURN,
        };

        enum {
            BACK_TIME = 60,
            PUSH_TIME = 120,
            LIFT_END_FRAME = 423,
            THROW_TIME = 420,
            RECOVER_TIME = 300,
        };

        beetle_c(int idx);
        ~beetle_c();
        void init(const nw4r::g3d::ResFile &res, mAllocator_c *allocator);
        void execute();
        void draw();
        void remove();
        void createModel(const nw4r::g3d::ResFile &res, mAllocator_c *allocator);
        void setAnmChr(const nw4r::g3d::ResAnmChr &anm, f32 blend);
        void setAnmVis(const nw4r::g3d::ResAnmVis &anm);
        void setAnmRate(f32 rate);
        void setAnmFrame(f32 frame);
        void calcMtx();
        f32 getAnmFrame();
        void initFace();
        void initCharge();
        void initBack();
        void initPush();
        void initLift();
        void initThrow();
        void initRecover();
        void initReturn();
        void stateFace();
        void stateCharge();
        void stateBack();
        void statePush();
        void stateLift();
        void stateThrow();
        void stateRecover();
        void stateReturn();
        void getOpponentPos(mVec3_c *out);
        dInsectUtil::axis_c getOpponentAxis();
        f32 getOpponentAnmFrame();
        bool isAttacker();
        bool isHercules();
        mMtx_c getHeadMtx();
        mMtx_c getAxisMtx() const {
            mMtx_c mtx;
            dInsectUtil::getMtx(mtx, mAxis);
            return mtx;
        }
        const dInsMusBattleBeetleParam_c &getParam() const { return dInsectMuseum::sBattleBeetleParam[mIdx]; }

        /* 0x000 */ m3d::mdl_c mMdl;
        /* 0x040 */ m3d::anmChr_c mAnmChr;
        /* 0x078 */ m3d::anmVis_c mAnmVis;
        /* 0x0B0 */ nw4r::g3d::ResAnmChr mAnmChrWait;
        /* 0x0B4 */ nw4r::g3d::ResAnmChr mAnmChrStop;
        /* 0x0B8 */ nw4r::g3d::ResAnmVis mAnmVisWait;
        /* 0x0BC */ nw4r::g3d::ResAnmVis mAnmVisStop;
        /* 0x0C0 */ int mStateId;
        /* 0x0C4 */ StateFunc mState;
        /* 0x0D0 */ mVec3_c mHomePos;
        /* 0x0DC */ mVec3_c mPos;
        /* 0x0E8 */ mAng3_c mAngle;
        /* 0x0EE */ u16 mTimer;
        /* 0x0F0 */ int mIdx;
        /* 0x0F4 */ mMtx_c mMtx;
        /* 0x124 */ bool mIsHeld;
        /* 0x125 */ bool mIsFlipped;
        /* 0x128 */ dInsectUtil::axis_c mAxis;
        /* 0x14C */ mVec3_c mSpeed;
        /* 0x158 */ mVec3_c mReturnPos;
        /* 0x164 */ f32 mRadius;
        /* 0x168 */ s16 mTurnAngle;
        /* 0x16A */ bool mHitGround;
        /* 0x16B */ bool mCrushed;
        /* 0x16C */ dSndObjInsect_c mSnd;
        /* 0x19C */ f32 mHoldDist;
    }; // size 0x1A0

    dInsMusBeetleBattle_c();
    ~dInsMusBeetleBattle_c();
    BOOL create(const nw4r::g3d::ResFile &herculesRes, const nw4r::g3d::ResFile &stagRes);
    BOOL execute();
    BOOL draw();
    BOOL remove();
    void setHeap(EGG::FrmHeap *heap);
    void freeHeap();
    static dInsMusBeetleBattle_c *getInstance();
    void getBeetlePos(mVec3_c *out, int idx) const;
    dInsectUtil::axis_c getBeetleAxis(int idx) const;
    f32 getBeetleAnmFrame(int idx);
    void getAttackerMtx(mMtx_c *out) const;
    void setHeldMtx(mMtx_c mtx);
    void getStumpPos(mVec3_c *out) const;
    void setReady(int idx);
    mMtx_c getAttackerHeadMtx();
    void clearReady();
    void initFace();
    void initCharge();
    void initBack();
    void initPush();
    void initLift();
    void initThrow();
    void initFinish();
    void stateFace();
    void stateCharge();
    void stateBack();
    void statePush();
    void stateLift();
    void stateThrow();
    void stateFinish();

    /* 0x000 */ mAllocator_c mAllocator;
    /* 0x01C */ StateFunc mState;
    /* 0x028 */ beetle_c mHercules;
    /* 0x1C8 */ beetle_c mStag;
    /* 0x368 */ mVec3_c mStumpPos;
    /* 0x374 */ EGG::FrmHeap *m_heap_p;
    /* 0x378 */ u16 mTimer;
    /* 0x37C */ int mAttacker;
    /* 0x380 */ mMtx_c mHeldMtx;
    /* 0x3B0 */ bool mHerculesReady;
    /* 0x3B1 */ bool mStagReady;
    /* 0x3B2 */ bool mIsActive;
}; // size 0x3B4
