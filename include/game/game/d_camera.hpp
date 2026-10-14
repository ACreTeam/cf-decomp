#ifndef D_CAMERA_HPP
#define D_CAMERA_HPP

// Game camera. RTTI places dCamera_c at global scope; other names are inferred.
#include <game/framework/f_base.hpp>
#include <egg/gfxe/eggCamera.h>
#include <egg/gfxe/eggScreen.h>
#include <game/mLib/m_frustum.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>

class dAcNpc_c;

namespace dCamera {
// Constant table representation; the runtime pose uses vector operations.
struct Param_c {
    s16 mPitch, mYaw;
    f32 mDistance;
    Vec mOffset;
}; // 0x14

struct ViewParam_c {
    s16 mPitch, mYaw;
    f32 mDistance;
    mVec3_c mOffset;

    ViewParam_c &operator=(const Param_c &other) {
        mPitch = other.mPitch;
        mYaw = other.mYaw;
        mDistance = other.mDistance;
        mOffset.x = other.mOffset.x;
        mOffset.y = other.mOffset.y;
        mOffset.z = other.mOffset.z;
        return *this;
    }
}; // 0x14

struct Pose_c {
    /* 0x00 */ ViewParam_c mParam;
    /* 0x14 */ mVec3_c mTarget;
}; // 0x20

struct Transition_c {
    /* 0x00 */ f32 mStart;
    /* 0x04 */ f32 mEnd;
    /* 0x08 */ f32 mEaseIn;
    /* 0x0C */ f32 mEaseOut;
}; // 0x10

struct CafeShot_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mOrbitX;
    /* 0x10 */ f32 mOrbitZ;
    /* 0x14 */ f32 mDistance;
    /* 0x18 */ f32 mDistanceStep;
    /* 0x1C */ s16 mPitch;
    /* 0x1E */ s16 mYaw;
    /* 0x20 */ s16 mYawStep;
}; // 0x24

// The same 0x18 bytes are reused by mutually exclusive camera modes.
union ModeWork_c {
    dAcNpc_c *mNpc;
    struct { s16 mTimer, mIndex; Vec mPosition; u32 _10; f32 mSpeed; } mTitle;
    struct { s16 mBounceAngle, mRollAngle; u16 mBounceTimer, mRollTimer;
             u8 mBounceKind, mRollKind; } mBus;
    struct { s16 mOrbitAngle, mShot, mState, mHistory[2], mHistoryIndex, _0C, mPhase;
             u8 _10; } mCafe;
};
} // namespace dCamera

class dCamera_c : public fBase_c {
public:
    dCamera_c() : mBusRoll(0), mCullForeground(false) {}
    virtual int create();   // 80187180
    virtual int doDelete(); // 801873C0
    virtual int execute();  // 801873F0
    virtual int draw();     // 80187510
    virtual ~dCamera_c();   // 8018B9E4

    void changeMode(); // 801875CC
    void executeMode(); // 80187680
    void initBounds(); // 801876F0
    int getHorizontalEdge(const mVec3_c *target); // 801877E4
    BOOL clampTarget(mVec3_c *target); // 80187824
    void followTarget(const mVec3_c *target); // 80187904
    void followClampedTarget(const mVec3_c *target); // 801879B8
    void setClampedTarget(const mVec3_c *target); // 80187A58
    void initFollow(); // 80187B1C
    void execFollow(); // 80187D00
    void initPlayerView1(); // 80187F0C
    void execPlayerView1(); // 80187F6C
    void initPlayerView2(); // 80187FF8
    void execPlayerView2(); // 8018803C
    void initPlayerView3(); // 801880C8
    void execPlayerView3(); // 8018810C
    void initTitleSelect(); // 80188198
    void execTitleSelect(); // 801881DC
    void initTitleNpc(); // 8018835C
    void execTitleNpc(); // 8018841C
    void initTitleBuilding(); // 801884D4
    void execTitleBuilding(); // 8018863C
    void initTool(); // 80188820
    void execTool(); // 80188864
    void initPlayerView8(); // 80188984
    void execPlayerView8(); // 801889C8
    void initTalk(); // 80188A54
    void execTalk(); // 80188ABC
    void initShopWide(); // 80188B90
    void execShopWide(); // 80188BEC
    void initShopClose(); // 80188C30
    void execShopClose(); // 80188C8C
    void initPlayerSelect(); // 80188CD0
    void execPlayerSelect(); // 80188CD8
    void initPlayerView13(); // 80188D1C
    void execPlayerView13(); // 80188D60
    void setBusBounce(u8 kind); // 80188DEC
    void setBusRoll(u8 kind); // 80188E64
    f32 updateBusMotion(); // 80188EDC
    void initBus(); // 801890B0
    void execBus(); // 801890EC
    void initGate(); // 80189144
    void execGate(); // 8018914C
    void initFixedView16(); // 80189190
    void execFixedView16(); // 801891D4
    void initTwoTarget(); // 80189218
    void execTwoTarget(); // 8018925C
    void initPlayerView18(); // 80189358
    void execPlayerView18(); // 8018939C
    void initCafe(); // 80189428
    void execCafe(); // 801894B4
    void startCafeIntro(); // 80189690
    void setCafeShot(int index); // 80189708
    void startCafeOutro(); // 8018984C
    void initFixedView20(); // 801898E0
    void execFixedView20(); // 80189924
    void initPlayerView21(); // 80189968
    void execPlayerView21(); // 801899C8
    void initScriptedAction(); // 80189A54
    void execScriptedAction(); // 80189A98
    void initSoundTarget23(); // 80189BC4
    void execSoundTarget23(); // 80189C10
    void initSoundTarget24(); // 80189C9C
    void execSoundTarget24(); // 80189CE0
    void initFixedView25(); // 80189D74
    void execFixedView25(); // 80189DB8
    void initPlayerView26(); // 80189DFC
    void execPlayerView26(); // 80189E40
    void initCityIntro(); // 80189ECC
    void execCityIntro(); // 80189F04
    void initLookUp(); // 80189F9C
    void execLookUp(); // 80189FF4
    mVec3_c screenToWorld(const mVec2_c *screen); // 8018A080
    f32 calcTransitionWeight(f32 frame, f32 start, f32 end, f32 easeIn, f32 easeOut); // 8018A2D8
    void finishTransition(); // 8018A400
    void startTransition(); // 8018A40C
    f32 getTransitionFramesLeft() const; // 8018A460
    void updateTransition(); // 8018A470
    bool isTransitionActive() const; // 8018A748
    void updateView(); // 8018A764
    BOOL isLookingUp(); // 8018A938
    BOOL isFollowLookUp(); // 8018A978
    BOOL isControllable(); // 8018A9A0
    BOOL usesForegroundCulling(); // 8018A9E0
    BOOL isPastForeground(const mVec3_c *position, f32 depth); // 8018AA64
    BOOL isPastTarget(const mVec3_c *position, f32 depth); // 8018AAA4
    f32 calcTalkDistance(const mVec3_c *a, const mVec3_c *b, mVec3_c *center, f32 *zOffset); // 8018AAE4
    f32 calcThreeActorTalkDistance(const mVec3_c *a, const mVec3_c *b, const mVec3_c *c, mVec3_c *center, f32 *zOffset); // 8018ABD4
    void updateOutdoorTalk(); // 8018ABE0
    void updateIndoorTalk(); // 8018ACE0
    void setPreset(u8 preset); // 8018B0B4
    void blendPresets(int first, int second, f32 weight); // 8018B118
    void setTransition(u8 transition); // 8018B298
    f32 getAspect() const; // 8018B2EC
    static f32 getFovy(); // 8018B2FC
    static f32 getNear(); // 8018B308
    static f32 getFar(); // 8018B318
    void updateSecondaryProjection(); // 8018B358
    BOOL setRoomView(int yaw, int pitch); // 8018B404
    BOOL resetLookUpTimer(); // 8018B554
    void startTalkView(const mVec3_c *target); // 8018B5B8
    void startThreeActorTalkView(const mVec3_c *a, const mVec3_c *b); // 8018B67C
    void switchTalkTarget(const mVec3_c *target); // 8018B768
    void startGateView(); // 8018B7B4
    void startGateCloseUp(); // 8018B7F4
    void startTwoTargetView(const mVec3_c *target); // 8018B844
    void restoreModeSlow(); // 8018B868
    void restoreModeNormal(); // 8018B87C
    void restoreModeFast(); // 8018B890
    void startBusFrontView(); // 8018B8A4
    void startBusWideView(); // 8018B8F4
    void startBusSideView(); // 8018B944
    void startBusReturnView(); // 8018B994

    void getProjectionMtx(nw4r::math::MTX44 *mtx) const {
        if (mFrustum.mProjType == EGG::Frustum::PROJ_ORTHO) {
            mFrustum.GetOrthographicMtx(mtx);
        } else {
            mFrustum.GetPerspectiveMtx(mtx);
        }
    }
    // Non-const: a const version stores y before x.
    mVec3_c getEyePos() { return mVec3_c(mCamera.mPos); }
    mVec3_c getTargetPos() __attribute__((noinline)) { return mVec3_c(mCamera.mAt); }
    mAng getYaw() { return mCurrent.mParam.mYaw; }
    mAng getCurveAngle() { return mCurveAngle; }

    /* 0x064 */ EGG::Screen mFrustum;
    /* 0x0CC */ EGG::LookAtCamera mCamera;
    /* 0x154 */ dCamera::ModeWork_c mWork;
    /* 0x16C */ mMtx_c mViewMtx; // inverse view matrix
    /* 0x19C */ int mMode;
    /* 0x1A0 */ int mPreviousMode;
    /* 0x1A4 */ int _1A4; // default mode
    /* 0x1A8 */ int _1A8; // requested mode; 29 means no request
    /* 0x1AC */ dCamera::Pose_c mDesired;
    /* 0x1CC */ dCamera::Pose_c mCurrent;
    /* 0x1EC */ dCamera::Pose_c mPrevious;
    /* 0x20C */ mVec3_c mEye;
    /* 0x218 */ u8 _218[0xC];
    /* 0x224 */ int mLookUpTimer;
    /* 0x228 */ mVec3_c mTalkTarget;
    /* 0x234 */ mVec3_c mTalkTarget2;
    /* 0x240 */ f32 mTransitionFrame;
    /* 0x244 */ dCamera::Transition_c mTransition;
    /* 0x254 */ mFrustum_c mCullFrustum;
    /* 0x344 */ mAng mCurveAngle;
    /* 0x346 */ mAng mBusRoll;
    /* 0x348 */ int mRoomYaw;
    /* 0x34C */ int mRoomPitch;
    /* 0x350 */ int mTalkSide;
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
    /* 0x373 */ u8 mTalkTargetCount;
    /* 0x374 */ u8 mFixedTalkSide;
    /* 0x375 */ u8 mCullForeground;
}; // size 0x378

namespace dCamera {
const CafeShot_c *getCafeShot(int index); // 801870D0
BOOL isMuseumExhibit(u8 scene); // 80187AF8
int getShopIndex(); // 80188B14
const Param_c *getPreset(int index); // 8018B08C
const Transition_c *getTransition(int index); // 8018B0A0
void setSecondaryFovy(f32 fovy); // 8018B328
void resetSecondaryFovy(); // 8018B33C
void useMainFovy(); // 8018B34C
void worldToScreen(const dCamera_c *cam, f32 *x, f32 *y, const mVec3_c *pos); // 8018A16C
}

extern "C" {
extern mVec3_c lbl_80623FEC; // 80623FEC: field-space view center
extern mVec3_c lbl_80624004; // 80624004: field-space eye
extern dCamera_c *lbl_8074E9B0; // 8074E9B0
}

template <typename T>
T *getViewCenter() { return &lbl_80623FEC; }

#endif
