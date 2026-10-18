#pragma once

// The snowballs and the snowmen built from them (RTTI daSnowMan_c). Names other than the class name are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_fd_bk_search_cand.hpp>
#include <game/game/d_bk_unit_search_cand.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_balloon_string.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <lib/egg/math/eggQuat.h>

namespace dDvd {

// A bank shared by its users: the first attach loads it, the last detach unloads it.
template <class T>
class CommonBank_c : public T {
public:
    CommonBank_c(const char *name) : mName(name), mNextId(0), mRefCount(0) {}
    virtual ~CommonBank_c() {}

    BOOL attach(u32 *id, void *heap) {
        if (*id != -1) {
            return TRUE;
        }
        if (mRefCount > 0 || this->load(mName, heap, 0)) {
            *id = mNextId;
            mNextId = (*id + 1) & 0x7FFFFFFF;
            mRefCount++;
            return TRUE;
        }
        return FALSE;
    }

    BOOL detach(u32 *id) {
        if (*id == -1) {
            return TRUE;
        }
        if (mRefCount > 1 || this->unload(FALSE)) {
            *id = -1;
            mRefCount--;
            return TRUE;
        }
        return FALSE;
    }

    /* 0x58 */ const char *mName;
    /* 0x5C */ int mNextId;
    /* 0x60 */ int mRefCount;
}; // size 0x64

} // namespace dDvd

class daSnowMan_c : public dDemoActor_c, private dMsg::Rcpt_c {
public:
    // mParam: BALL_HEAD / BALL_BODY for the snowballs, SNOWMAN + the town record index for a built snowman.
    enum {
        BALL_NUM = 2,
        SNOWMAN = 2,
    };

    // The field object of town record 0 (records 1, 2 follow).
    enum {
        FG_SNOWMAN = 0x7000,
    };

    enum STATE_e {
        STATE_ROLL,
        STATE_SINK,
        STATE_JUMP,
        STATE_SETTLE,
        STATE_BUILT,
        STATE_BASE,
        STATE_SNOWMAN,
        STATE_TALK,
        STATE_TALK_END,
        STATE_NUM,
    };

    typedef void (daSnowMan_c::*StateFunc)();
    typedef BOOL (*FindFunc)(const mVec3_c *pos, int idx, void *user);
    struct State_s {
        StateFunc mInit;
        StateFunc mExecute;
        StateFunc mFinal;
    };

    // A snowball placed for the day.
    struct ball_c {
        ball_c() : mActive(false) {}
        ~ball_c() {}

        void set(const mVec3_c &pos, f32 scale) {
            mActive = true;
            mScale = scale;
            mPos = pos;
        }

        /* 0x00 */ mVec3_c mPos;
        /* 0x0C */ f32 mScale;
        /* 0x10 */ bool mActive;
    }; // size 0x14

    daSnowMan_c() : mResId(-1), mStateOwner(this), mState(-1), mPrevState(-1), mDeleted(false) {}
    virtual ~daSnowMan_c() {}

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual bool canInteract(dDemoActor_c *actor);
    virtual void onDemoCmd(int cmd, u8 member);
    virtual mVec3_c *getDemoPosition();

    void changeState(int state) {
        if (mState != -1 && sStates[mState].mFinal) {
            (mStateOwner->*sStates[mState].mFinal)();
        }
        mPrevState = mState;
        mState = state;
        (mStateOwner->*sStates[mState].mInit)();
    }

    void executeState() { (mStateOwner->*sStates[mState].mExecute)(); }
    BOOL isState(int state) const { return mState == state; }

    static daSnowMan_c *getBall(int idx) { return sBall[idx]; }
    daSnowMan_c *getOther() const { return mParam < BALL_NUM ? sBall[mParam == 0] : NULL; }

    f32 getRadius() const { return 15.5f * mScale; }
    f32 getGravity() const;
    f32 getDepth(f32 waterY) const {
        return 1.25f * (15.5f * mScale) + (mPos.y + mSlopeY) - waterY;
    }
    void lerpPos(f32 t) { mPos = (1.0f - t) * mStartPos + t * mTargetPos; }
    static void rotate(EGG::Quatf *out, const EGG::Quatf *speed) {
        EGG::Quatf rotation;
        dBalloonString::multiplyQuaternion(&rotation, speed, out);
        *out = EGG::Quatf(rotation);
        out->normalise();
    }
    void calcRotation() {
        rotate(&mRotation, &mRotSpeed);
    }
    void registObjc(f32 radius) {
        mObjc.set(this, radius, 2.0f * radius, 0x80000010, 0x8040007F, mWeight, 0);
        mObjc.mParam = 0;
        mObjc.regist();
    }
    u16 getFgId() const {
        u16 id;
        switch (mSlot) {
        case 0:
            id = FG_SNOWMAN;
            break;
        case 1:
            id = FG_SNOWMAN + 1;
            break;
        case 2:
            id = FG_SNOWMAN + 2;
            break;
        }
        return id;
    }

    f32 getPushRate() const;
    f32 getRollRate() const;
    BOOL push(f32 speed, mVec3_c *pos, s16 *angle, f32 *speedF, mAng dir);
    BOOL canRoll();
    BOOL hit(const mVec3_c *pos);
    bool getWallNormal(mVec3_c *normal);

    void initRoll();
    void executeRoll();
    void finalRoll();
    void initSink();
    void executeSink();
    void finalSink();
    void initJump();
    void executeJump();
    void finalJump();
    void initSettle();
    void executeSettle();
    void finalSettle();
    void initBuilt();
    void executeBuilt();
    void finalBuilt();
    void initBase();
    void executeBase();
    void finalBase();
    void initSnowman();
    void executeSnowman();
    void finalSnowman();
    void initTalk();
    void executeTalk();
    void finalTalk();
    void initTalkEnd();
    void executeTalkEnd();
    void finalTalkEnd();

    BOOL checkGround();
    BOOL getGroundNormal(mVec3_c *normal);
    BOOL setFg();
    BOOL save(f32 headScale, f32 bodyScale);
    BOOL placeFg(int x, int z);
    BOOL placeFgAround(int x, int z);
    BOOL checkBreak(const mVec3_c *pos, f32 radius);
    BOOL breakBall();
    void doBreak();
    void setSplashEffect(const mVec3_c *pos);
    void calcMtx();

    static BOOL getWallNormal(dBGCF::acch_c *acch, mVec3_c *normal);
    static BOOL sendMail();
    BOOL canPlace(int x, int z);
    BOOL lose(const dItem::Item *item);
    static void splashCallback(nw4r::ef::Particle *ptcl, u32 kind);
    static BOOL checkUnit(int blockX, int blockZ, int x, int z, void *user);
    static BOOL pushAll(f32 speed, mVec3_c *pos, s16 *angle, f32 *speedF, mAng dir);
    static BOOL isAtUnit(int x, int z);
    static bool find(mVec3_c *pos, int *idx, void *user, FindFunc func);
    static BOOL checkBreakAll(const mVec3_c *pos, f32 radius);
    static BOOL placeBalls();
    static BOOL placeSnowmen();
    static void removeOld();
    static void clearBalls();
    static int getFace(f32 headScale, f32 bodyScale);

    static State_s sStates[STATE_NUM];
    static daSnowMan_c *sBall[BALL_NUM];
    static ball_c sBallInfo[BALL_NUM];

    /* 0x0BC dMsg::Rcpt_c */
    /* 0x120 */ m3d::smdl_c mBodyMdl;
    /* 0x12C */ m3d::smdl_c mFaceMdl;
    /* 0x138 */ m3d::smdl_c *mpMdl;
    /* 0x13C */ mAllocator_c mAllocator;
    /* 0x158 */ u32 mResId;
    /* 0x15C */ dObjc::ac_c mObjc;
    /* 0x1A0 */ dBGCF::acch_c mAcch;
    /* 0x1E8 */ dSndObjSimple_c mSound;
    /* 0x218 */ dLevelEffect_c mEffect;
    /* 0x2AC */ EGG::Quatf mRotation;
    /* 0x2BC */ EGG::Quatf mRotSpeed;
    /* 0x2CC */ daSnowMan_c *mStateOwner;
    /* 0x2D0 */ int mState;
    /* 0x2D4 */ int mPrevState;
    /* 0x2D8 */ mVec3_c mTargetPos;
    /* 0x2E4 */ mVec3_c mStartPos;
    /* 0x2F0 */ mVec3_c mSlopeNormal;
    /* 0x2FC */ mVec3_c mHitPos;
    /* 0x308 */ mVec3_c mGroundPos;
    /* 0x314 */ mVec3_c mWallNormal;
    /* 0x320 */ f32 mScale;
    /* 0x324 */ f32 mRadius;
    /* 0x328 */ f32 mSink;
    /* 0x32C */ f32 mRollDist;
    /* 0x330 */ f32 mGrowRate;
    /* 0x334 */ f32 mSlopeY;
    /* 0x338 */ f32 mWobble;
    /* 0x33C */ mAng mPushAngle;
    /* 0x33E */ mAng mWobbleAngle;
    /* 0x340 */ f32 mHeadScale;
    /* 0x344 */ f32 mBodyScale;
    /* 0x348 */ int mTimer;
    /* 0x34C */ int _34C;
    /* 0x350 */ int mFace;
    /* 0x354 */ int mSlot;
    /* 0x358 */ u8 mWeight;
    /* 0x359 */ u8 _359[3];
    /* 0x35C */ u8 mAlpha;
    /* 0x35D */ bool mKeep;
    /* 0x35E */ bool mInAir;
    /* 0x35F */ bool mPushed;
    /* 0x360 */ bool mWasPushed;
    /* 0x361 */ bool mPushStarted;
    /* 0x362 */ bool mHitting;
    /* 0x363 */ bool mJustBuilt;
    /* 0x364 */ u8 mFgSet;
    /* 0x365 */ bool mDeleted;
    /* 0x366 */ bool mOnSlope;
    /* 0x367 */ bool mOnWall;
    /* 0x368 */ bool mHit;
}; // size 0x36C
