#pragma once

// The game camera dCamera_c (RTTI "dCamera_c"; profile execute order 0xA7, draw order 0xE4).

#include <types.h>
#include <egg/gfxe/eggCamera.h>
#include <egg/gfxe/eggScreen.h>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_frustum.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/math.h>
#include <revolution/MTX.h>

class dAcNpc_c;

class dCamera_c : public fBase_c {
public:
    enum MODE_e {
        MODE_FOLLOW,
        MODE_PLAYER_VIEW_1,
        MODE_PLAYER_VIEW_2,
        MODE_PLAYER_VIEW_3,
        MODE_TITLE_SELECT,
        MODE_TITLE_NPC,
        MODE_TITLE_BUILDING,
        MODE_TOOL,
        MODE_PLAYER_VIEW_8,
        MODE_TALK,
        MODE_SHOP_WIDE,
        MODE_SHOP_CLOSE,
        MODE_PLAYER_SELECT,
        MODE_PLAYER_VIEW_13,
        MODE_BUS,
        MODE_GATE,
        MODE_FIXED_VIEW_16,
        MODE_TWO_TARGET,
        MODE_PLAYER_VIEW_18,
        MODE_CAFE,
        MODE_FIXED_VIEW_20,
        MODE_PLAYER_VIEW_21,
        MODE_SCRIPTED_ACTION,
        MODE_SOUND_TARGET_23,
        MODE_SOUND_TARGET_24,
        MODE_FIXED_VIEW_25,
        MODE_PLAYER_VIEW_26,
        MODE_CITY_INTRO,
        MODE_LOOK_UP,
        MODE_NUM,
        MODE_NONE = MODE_NUM,
    };

    enum SIDE_e {
        SIDE_NONE,
        SIDE_RIGHT,
        SIDE_LEFT,
        SIDE_AUTO,
    };

    struct param_c {
        /* 0x00 */ s16 mPitch;
        /* 0x02 */ s16 mYaw;
        /* 0x04 */ f32 mDistance;
        /* 0x08 */ mVec3_c mOffset;
        /* 0x14 */ mVec3_c mTarget;
    }; // size 0x20

    struct transition_c {
        /* 0x00 */ f32 mStart;
        /* 0x04 */ f32 mEnd;
        /* 0x08 */ f32 mEaseIn;
        /* 0x0C */ f32 mEaseOut;
    }; // size 0x10

    struct title_c {
        /* 0x00 */ s16 mTimer;
        /* 0x02 */ s16 mIndex;
        /* 0x04 */ Vec mPosition;
        /* 0x10 */ u8 _10[4];
        /* 0x14 */ f32 mSpeed;
    };

    struct bus_c {
        /* 0x00 */ s16 mBounceAngle;
        /* 0x02 */ s16 mRollAngle;
        /* 0x04 */ u16 mBounceTimer;
        /* 0x06 */ u16 mRollTimer;
        /* 0x08 */ u8 mBounceKind;
        /* 0x09 */ u8 mRollKind;
    };

    struct cafe_c {
        /* 0x00 */ s16 mOrbitAngle;
        /* 0x02 */ s16 mShot;
        /* 0x04 */ s16 mState;
        /* 0x06 */ s16 mHistory[2];
        /* 0x0A */ s16 mHistoryIndex;
        /* 0x0C */ s16 _0C;
        /* 0x0E */ s16 mPhase;
        /* 0x10 */ u8 _10;
    };

    dCamera_c() {
        mBusRoll = 0;
        mCullForeground = 0;
    }

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    void changeMode();
    void executeMode();
    void initBounds();
    int getRoomRow(u8 scene);
    int getHorizontalEdge(const mVec3_c *pos);
    BOOL clampTarget(mVec3_c *pos);
    void followTarget(const mVec3_c *pos);
    void followClampedTarget(const mVec3_c *pos);
    void setClampedTarget(const mVec3_c *pos);

    void initFollow();
    void execFollow();
    void initPlayerView1();
    void execPlayerView1();
    void initPlayerView2();
    void execPlayerView2();
    void initPlayerView3();
    void execPlayerView3();
    void initTitleSelect();
    void execTitleSelect();
    void initTitleNpc();
    void execTitleNpc();
    void initTitleBuilding();
    void execTitleBuilding();
    void initTool();
    void execTool();
    void initPlayerView8();
    void execPlayerView8();
    void initTalk();
    void execTalk();
    void initShopWide();
    void execShopWide();
    void initShopClose();
    void execShopClose();
    void initPlayerSelect();
    void execPlayerSelect();
    void initPlayerView13();
    void execPlayerView13();
    void setBusBounce(u8 kind);
    void setBusRoll(u8 kind);
    f32 updateBusMotion();
    void initBus();
    void execBus();
    void initGate();
    void execGate();
    void initFixedView16();
    void execFixedView16();
    void initTwoTarget();
    void execTwoTarget();
    void initPlayerView18();
    void execPlayerView18();
    void initCafe();
    void execCafe();
    void startCafeIntro();
    void setCafeShot(int shot);
    void startCafeOutro();
    void initFixedView20();
    void execFixedView20();
    void initPlayerView21();
    void execPlayerView21();
    void initScriptedAction();
    void execScriptedAction();
    void initSoundTarget23();
    void execSoundTarget23();
    void initSoundTarget24();
    void execSoundTarget24();
    void initFixedView25();
    void execFixedView25();
    void initPlayerView26();
    void execPlayerView26();
    void initCityIntro();
    void execCityIntro();
    void initLookUp();
    void execLookUp();

    mVec3_c screenToWorld(const mVec2_c *screen) const;
    f32 calcTransitionWeight(f32 frame, f32 start, f32 end, f32 easeIn, f32 easeOut);
    void finishTransition();
    void startTransition();
    f32 getTransitionFramesLeft() const;
    void updateTransition();
    bool isTransitionActive() const;
    void updateView();
    BOOL isLookUpPreset();
    BOOL checkLookingUp();
    BOOL isLookingUp();
    BOOL isFollowLookUp();
    BOOL isControllable();
    u32 usesForegroundCulling();
    BOOL isPastForeground(const mVec3_c *pos, f32 depth);
    BOOL isPastTarget(const mVec3_c *pos, f32 depth);
    f32 calcTalkDistance(const mVec3_c *a, const mVec3_c *b, mVec3_c *center, f32 *zOffset);
    f32 calcThreeActorTalkDistance(const mVec3_c *a, const mVec3_c *b, const mVec3_c *c, mVec3_c *center,
                                   f32 *zOffset);
    void updateOutdoorTalk();
    void updateIndoorTalk();
    void setPreset(u8 preset);
    void blendPresets(u8 first, u8 second, f32 weight);
    void setTransition(u8 transition);
    f32 getAspect() const;
    static f32 getFovy();
    static f32 getNear();
    static f32 getFar();
    void updateSecondaryProjection();
    BOOL setRoomView(int yaw, int pitch);
    int resetLookUpTimer();
    void startTalkView(const mVec3_c *target);
    void startThreeActorTalkView(const mVec3_c *a, const mVec3_c *b);
    void switchTalkTarget(const mVec3_c *target);
    void startGateView();
    void startGateCloseUp();
    void startTwoTargetView(const mVec3_c *target);
    void restoreModeMedium();
    void restoreModeSlow();
    void restoreModeFast();
    void startBusFrontView();
    void startBusWideView();
    void startBusSideView();
    void startBusReturnView();

    void getProjectionMtx(nw4r::math::MTX44 *mtx) const {
        if (mFrustum.mProjType == EGG::Frustum::PROJ_ORTHO) {
            mFrustum.GetOrthographicMtx(mtx);
        } else {
            mFrustum.GetPerspectiveMtx(mtx);
        }
    }

    mVec3_c getEyePos() { return mVec3_c(mCamera.mPos); }
    mVec3_c getTargetPos() __attribute__((noinline)) { return mVec3_c(mCamera.mAt); }
    mAng getYaw() { return mCurrent.mYaw; }
    mAng getCurveAngle() { return mCurveAngle; }
    cafe_c *getCafe() { return &mWork.mCafe; }

    /* 0x064 */ EGG::Screen mFrustum;
    /* 0x0CC */ EGG::LookAtCamera mCamera;
    /* 0x154 */ union {
        dAcNpc_c *mpNpc;
        title_c mTitle;
        bus_c mBus;
        cafe_c mCafe;
    } mWork;
    /* 0x16C */ mMtx_c mViewMtx;
    /* 0x19C */ int mMode;
    /* 0x1A0 */ int mPreviousMode;
    /* 0x1A4 */ int mDefaultMode;
    /* 0x1A8 */ int mRequestedMode;
    /* 0x1AC */ param_c mDesired;
    /* 0x1CC */ param_c mCurrent;
    /* 0x1EC */ param_c mPrevious;
    /* 0x20C */ mVec3_c mEye;
    /* 0x218 */ u8 _218[0x224 - 0x218];
    /* 0x224 */ int mLookUpTimer;
    /* 0x228 */ mVec3_c mTalkTarget;
    /* 0x234 */ mVec3_c mTalkTarget2;
    /* 0x240 */ f32 mTransitionFrame;
    /* 0x244 */ transition_c mTransition;
    /* 0x254 */ mFrustum_c mCullFrustum;
    /* 0x344 */ mAng mCurveAngle;
    /* 0x346 */ s16 mBusRoll;
    /* 0x348 */ int mRoomYaw;
    /* 0x34C */ int mRoomPitch;
    /* 0x350 */ SIDE_e mTalkSide;
    /* 0x354 */ f32 mMinX;
    /* 0x358 */ f32 mMaxX;
    /* 0x35C */ f32 mMinZ;
    /* 0x360 */ f32 mMaxZ;
    /* 0x364 */ f32 mForegroundZ;
    /* 0x368 */ f32 mTargetZ;
    /* 0x36C */ f32 mSecondaryFovy;
    /* 0x370 */ u8 mPreset;
    /* 0x371 */ u8 mTransitionId;
    /* 0x372 */ u8 mPendingTransition;
    /* 0x373 */ u8 mHasSecondTalkTarget;
    /* 0x374 */ u8 mFixedTalkSide;
    /* 0x375 */ u8 mCullForeground;

    static dCamera_c *sInstance;
    static mVec3_c sCenter;
    static mVec3_c sEye;
}; // size 0x378

namespace dCamera {

void worldToScreen(const dCamera_c *cam, f32 *x, f32 *y, const mVec3_c *pos);
void setSecondaryFovy(f32 fovy);
void resetSecondaryFovy();
void useMainFovy();

} // namespace dCamera

template <typename T>
T *getViewCenter() {
    return &dCamera_c::sCenter;
}
