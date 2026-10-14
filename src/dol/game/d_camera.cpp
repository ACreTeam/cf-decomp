// The game camera (dCamera_c).
#include <game/game/d_camera.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_search_cand_base.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/sLib/s_lib.hpp>

struct cafeShot_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mOrbitX;
    /* 0x10 */ f32 mOrbitZ;
    /* 0x14 */ f32 mDistance;
    /* 0x18 */ f32 mDistanceStep;
    /* 0x1C */ s16 mPitch;
    /* 0x1E */ s16 mYaw;
    /* 0x20 */ s16 mYawStep;
};

static cafeShot_c l_cafeShots[] = {
    {{248.0f, 32.0f, 272.0f}, 0.0f, 0.0f, 184.0f, -0.025f, -2729, 1364, 0},
    {{256.0f, 32.0f, 272.0f}, -16.0f, 0.0f, 176.0f, 0.0f, -1364, 6144, 4},
    {{256.0f, 28.0f, 272.0f}, -16.0f, 0.0f, 192.0f, 0.0f, -1364, -6144, -4},
    {{264.0f, 32.0f, 272.0f}, -24.0f, 0.0f, 256.0f, -0.025f, -5460, 5460, -6},
    {{264.0f, 24.0f, 272.0f}, -24.0f, 0.0f, 288.0f, -0.025f, -5460, -5460, 6},
    {{304.0f, 0.0f, 336.0f}, 0.0f, 0.0f, 512.0f, 0.0f, -10921, -1364, 4},
    {{276.0f, 24.0f, 339.0f}, 0.0f, 0.0f, 384.0f, 0.0f, -4096, 6825, -6},
    {{276.0f, 24.0f, 339.0f}, 0.0f, 0.0f, 384.0f, 0.0f, -4096, -6825, -6},
    {{248.0f, 24.0f, 342.0f}, 0.0f, 0.0f, 296.0f, 0.1f, -2729, 28215, -4},
    {{248.0f, 24.0f, 342.0f}, 0.0f, 0.0f, 336.0f, 0.1f, -2729, 4550, -4},
    {{256.0f, 24.0f, 412.0f}, -0.0f, 0.0f, 224.0f, 0.025f, -2729, 28215, -6},
    {{256.0f, 24.0f, 412.0f}, 0.0f, 0.0f, 320.0f, -0.025f, -4096, -28215, 4},
    {{392.0f, 32.0f, 304.0f}, 16.0f, 0.0f, 128.0f, 0.025f, -4096, -10921, 6},
    {{384.0f, 32.0f, 304.0f}, -24.0f, 0.0f, 256.0f, 0.025f, -4096, -13652, -4},
};

// 801870D0
static const cafeShot_c *getCafeShot(int shot) {
    return &l_cafeShots[shot];
}

// 801870E4
CUSTOM_BASE_PROFILE(CAMERA, dCamera_c, 0xA7, 0xE4);

struct preset_c {
    /* 0x00 */ s16 mPitch;
    /* 0x02 */ s16 mYaw;
    /* 0x04 */ f32 mDistance;
    /* 0x08 */ Vec mOffset;
};

static const preset_c l_presets[] = {
    {-9466, 0, 332.0f, {0.0f, 0.0f, 20.0f}},
    {-7918, 0, 200.0f, {0.0f, 0.0f, 0.0f}},
    {-5461, -3586, 270.0f, {0.0f, 0.0f, 0.0f}},
    {-5461, 0, 270.0f, {0.0f, 0.0f, 0.0f}},
    {-5461, 3586, 270.0f, {0.0f, 0.0f, 0.0f}},
    {-6371, -3586, 350.0f, {0.0f, 0.0f, 0.0f}},
    {-6371, 0, 350.0f, {0.0f, 0.0f, 0.0f}},
    {-6371, 3586, 350.0f, {0.0f, 0.0f, 0.0f}},
    {-12288, -3586, 450.0f, {0.0f, 0.0f, 0.0f}},
    {-12288, 0, 450.0f, {0.0f, 0.0f, 0.0f}},
    {-12288, 3586, 450.0f, {0.0f, 0.0f, 0.0f}},
    {-2730, -3586, 370.0f, {0.0f, 16.0f, -24.0f}},
    {-2730, 0, 370.0f, {0.0f, 16.0f, -24.0f}},
    {-2730, 3586, 370.0f, {0.0f, 16.0f, -24.0f}},
    {-7054, -3586, 480.0f, {0.0f, -8.0f, 12.0f}},
    {-7054, 0, 480.0f, {0.0f, -8.0f, 12.0f}},
    {-7054, 3586, 480.0f, {0.0f, -8.0f, 12.0f}},
    {-6371, 0, 280.0f, {0.0f, -30.0f, 15.0f}},
    {-3185, 0, 330.0f, {0.0f, -30.5f, 15.0f}},
    {-8192, 0, 240.0f, {0.0f, 4.0f, 15.0f}},
    {-4004, 0, 370.0f, {0.0f, 24.0f, -64.0f}},
    {-5461, 0, 200.0f, {0.0f, 0.0f, -16.0f}},
    {-7509, 0, 248.0f, {0.0f, 0.0f, 24.0f}},
    {-4096, 0, 264.0f, {0.0f, -4.0f, 16.0f}},
    {-6826, 0, 280.0f, {256.0f, 64.0f, 494.0f}},
    {-6826, 0, 280.0f, {128.0f, 64.0f, 448.0f}},
    {-6826, 0, 280.0f, {128.0f, 64.0f, 320.0f}},
    {-6826, 0, 280.0f, {128.0f, 128.0f, 320.0f}},
    {-6826, 0, 280.0f, {160.0f, 64.0f, 256.0f}},
    {-2730, 0, 240.0f, {256.0f, 64.0f, 448.0f}},
    {-2730, 0, 240.0f, {128.0f, 64.0f, 448.0f}},
    {-2730, 0, 240.0f, {128.0f, 64.0f, 320.0f}},
    {-2730, 0, 240.0f, {128.0f, 128.0f, 320.0f}},
    {-2730, 0, 280.0f, {160.0f, 64.0f, 288.0f}},
    {-2730, 0, 179.0f, {256.0f, 26.0f, 494.0f}},
    {-8192, 0, 240.0f, {0.0f, -4.0f, 15.0f}},
    {0, 1365, 32.0f, {292.0f, 40.0f, 440.0f}},
    {-1137, 17294, 40.0f, {330.0f, 34.0f, 420.0f}},
    {-1137, 5461, 24.0f, {304.0f, 40.0f, 440.0f}},
    {-6371, 0, 350.0f, {256.0f, 20.0f, 288.0f}},
    {-1365, 0, 350.0f, {256.0f, 24.0f, 256.0f}},
    {-6371, 0, 350.0f, {256.0f, 20.0f, 390.0f}},
    {-3640, 0, 350.0f, {256.0f, 20.0f, 256.0f}},
    {-2548, 6571, 220.0f, {35.0f, 5.0f, 15.0f}},
    {-7918, 0, 200.0f, {0.0f, 0.0f, 0.0f}},
    {-4278, 0, 400.0f, {304.0f, 30.0f, 370.0f}},
    {-546, 0, 150.0f, {254.0f, 42.0f, 295.0f}},
    {-4004, 6007, 196.0f, {342.0f, 112.0f, 478.0f}},
    {-6371, 0, 280.0f, {-60.0f, -30.0f, 15.0f}},
    {-6371, 0, 280.0f, {-8.0f, -30.0f, 15.0f}},
    {-5097, 0, 240.0f, {0.0f, -4.0f, 15.0f}},
    {-8192, 0, 346.0f, {0.0f, 0.0f, 10.0f}},
    {-4096, 0, 384.0f, {0.0f, 28.0f, -88.0f}},
    {-8192, -3586, 350.0f, {0.0f, 0.0f, 0.0f}},
    {-8192, 0, 350.0f, {0.0f, 0.0f, 0.0f}},
    {-8192, 3586, 350.0f, {0.0f, 0.0f, 0.0f}},
    {-6371, 0, 350.0f, {272.0f, 20.0f, 360.0f}},
    {-6189, 0, 390.0f, {768.0f, 76.0f, 480.0f}},
    {-8192, 0, 346.0f, {768.0f, 48.0f, 794.0f}},
    {-2730, 0, 240.0f, {0.0f, -8.0f, 16.0f}},
    {-9466, 0, 332.0f, {0.0f, 0.0f, -24.0f}},
};

static const dCamera_c::transition_c l_transitions[] = {
    {0.0f, 32.0f, 8.0f, 16.0f},
    {0.0f, 30.0f, 5.0f, 20.0f},
    {0.0f, 130.0f, 10.0f, 50.0f},
    {0.0f, 42.0f, 4.0f, 20.0f},
    {0.0f, 130.0f, 10.0f, 50.0f},
    {0.0f, 130.0f, 20.0f, 90.0f},
    {0.0f, 150.0f, 30.0f, 120.0f},
    {0.0f, 30.0f, 5.0f, 20.0f},
    {0.0f, 180.0f, 60.0f, 90.0f},
    {0.0f, 60.0f, 20.0f, 30.0f},
    {0.0f, 900.0f, 60.0f, 120.0f},
    {0.0f, 180.0f, 0.0f, 120.0f},
    {0.0f, 90.0f, 20.0f, 70.0f},
    {0.0f, 105.0f, 40.0f, 65.0f},
    {0.0f, 48.0f, 18.0f, 30.0f},
    {0.0f, 90.0f, 35.0f, 55.0f},
    {0.0f, 45.0f, 12.0f, 20.0f},
    {0.0f, 50.0f, 12.0f, 20.0f},
    {0.0f, 130.0f, 20.0f, 90.0f},
    {0.0f, 28.0f, 12.0f, 16.0f},
    {0.0f, 32.0f, 8.0f, 16.0f},
    {60.0f, 360.0f, 180.0f, 120.0f},
};

struct titleBuilding_c {
    /* 0x00 */ u32 mBuilding;
    /* 0x04 */ nw4r::math::_VEC3 mOffset;
};

static const titleBuilding_c l_titleBuildings[] = {
    {1, {0.0f, 0.0f, 16.0f}},
    {21, {0.0f, 0.0f, 16.0f}},
    {23, {16.0f, 0.0f, 16.0f}},
    {19, {16.0f, 0.0f, 16.0f}},
    {20, {32.0f, 0.0f, 16.0f}},
};

static void (dCamera_c::*const l_modeFuncs[dCamera_c::MODE_NUM][2])() = {
    {&dCamera_c::initFollow, &dCamera_c::execFollow},
    {&dCamera_c::initPlayerView1, &dCamera_c::execPlayerView1},
    {&dCamera_c::initPlayerView2, &dCamera_c::execPlayerView2},
    {&dCamera_c::initPlayerView3, &dCamera_c::execPlayerView3},
    {&dCamera_c::initTitleSelect, &dCamera_c::execTitleSelect},
    {&dCamera_c::initTitleNpc, &dCamera_c::execTitleNpc},
    {&dCamera_c::initTitleBuilding, &dCamera_c::execTitleBuilding},
    {&dCamera_c::initTool, &dCamera_c::execTool},
    {&dCamera_c::initPlayerView8, &dCamera_c::execPlayerView8},
    {&dCamera_c::initTalk, &dCamera_c::execTalk},
    {&dCamera_c::initShopWide, &dCamera_c::execShopWide},
    {&dCamera_c::initShopClose, &dCamera_c::execShopClose},
    {&dCamera_c::initPlayerSelect, &dCamera_c::execPlayerSelect},
    {&dCamera_c::initPlayerView13, &dCamera_c::execPlayerView13},
    {&dCamera_c::initBus, &dCamera_c::execBus},
    {&dCamera_c::initGate, &dCamera_c::execGate},
    {&dCamera_c::initFixedView16, &dCamera_c::execFixedView16},
    {&dCamera_c::initTwoTarget, &dCamera_c::execTwoTarget},
    {&dCamera_c::initPlayerView18, &dCamera_c::execPlayerView18},
    {&dCamera_c::initCafe, &dCamera_c::execCafe},
    {&dCamera_c::initFixedView20, &dCamera_c::execFixedView20},
    {&dCamera_c::initPlayerView21, &dCamera_c::execPlayerView21},
    {&dCamera_c::initScriptedAction, &dCamera_c::execScriptedAction},
    {&dCamera_c::initSoundTarget23, &dCamera_c::execSoundTarget23},
    {&dCamera_c::initSoundTarget24, &dCamera_c::execSoundTarget24},
    {&dCamera_c::initFixedView25, &dCamera_c::execFixedView25},
    {&dCamera_c::initPlayerView26, &dCamera_c::execPlayerView26},
    {&dCamera_c::initCityIntro, &dCamera_c::execCityIntro},
    {&dCamera_c::initLookUp, &dCamera_c::execLookUp},
};

static const u8 l_roomPresets[][3] = {
    {2, 3, 4},
    {5, 6, 7},
    {8, 9, 10},
    {11, 12, 13},
    {53, 54, 55},
    {14, 15, 16},
};

struct busMotion_c {
    /* 0x00 */ u16 mFrames;
    /* 0x02 */ u16 mRandomFrames;
    /* 0x04 */ s16 mSpeed;
    /* 0x08 */ f32 mAmplitude;
};

static const busMotion_c l_busMotions[] = {
    {0, 60, 0, 0.0f},
    {30, 3, 2730, 0.01f},
    {30, 3, 2730, 0.024f},
    {30, 3, 2730, 0.036f},
};

class dCameraHIO_c {
public:
    dCameraHIO_c() {
        mFovy = 22.5f;
        mSecondaryFovy = 30.0f;
        mNear = 1.0f;
        mFar = 2000.0f;
        mFollowRadius = 16.0f;
        mSideMargin = 64.0f;
        mTalkCullOffset = -16.0f;
        mLookUpDelay = 180;
    }

    /* 0x00 */ f32 mFovy;
    /* 0x04 */ f32 mSecondaryFovy;
    /* 0x08 */ f32 mNear;
    /* 0x0C */ f32 mFar;
    /* 0x10 */ f32 mFollowRadius;
    /* 0x14 */ f32 mSideMargin;
    /* 0x18 */ f32 mTalkCullOffset;
    /* 0x1C */ s16 mLookUpDelay;
};

static dCameraHIO_c l_hostIO;

static inline f32 getTalkCullOffset() {
    return l_hostIO.mTalkCullOffset;
}
mVec3_c dCamera_c::sCenter;
mVec3_c dCamera_c::sEye;
dCamera_c *dCamera_c::sInstance;
static f32 l_viewDistance;

// 80187180
int dCamera_c::create() {
    sInstance = this;
    dScene::setSceneE848((u32)this);
    mFrustum.SetFovy(l_hostIO.mFovy);
    mFrustum.mNearZ = l_hostIO.mNear;
    mFrustum.mFarZ = l_hostIO.mFar;
    mSecondaryFovy = l_hostIO.mSecondaryFovy;
    switch (dScene::getCurrentScene()) {
    case dScene::SCENE_DM_TITLE:
        if (dNpcMng_c::sInstance == NULL) {
            return NOT_READY;
        }
        mDefaultMode = MODE_TITLE_SELECT;
        break;
    case dScene::SCENE_DM_PL_SEL:
        mDefaultMode = MODE_PLAYER_SELECT;
        break;
    case dScene::SCENE_DM_BUS_PL_CRT:
    case dScene::SCENE_DM_BUS_TO_TOWN:
    case dScene::SCENE_DM_BUS_TO_LAND:
        mDefaultMode = MODE_BUS;
        break;
    case dScene::SCENE_DM_CHKP_CNNCT:
    case dScene::SCENE_DM_CHKP_DCNNCT:
    case dScene::SCENE_DM_CHKP_MISS:
        mDefaultMode = MODE_GATE;
        break;
    case dScene::SCENE_TOWN: {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
        if (player != NULL && !player->isFlag0(0x70)) {
            mDefaultMode = MODE_CITY_INTRO;
        } else {
            mDefaultMode = MODE_FOLLOW;
        }
        break;
    }
    default:
        mDefaultMode = MODE_FOLLOW;
        break;
    }
    mRequestedMode = mDefaultMode;
    mPendingTransition = 22;
    initBounds();
    changeMode();
    finishTransition();
    updateTransition();
    updateView();
    execute();
    draw();
    mMtx_c mtx;
    PSMTXIdentity(mtx);
    m3d::setCurrentCamera(1);
    nw4r::g3d::Camera camera = m3d::getCamera(1);
    camera.SetCameraMtxDirectly(mtx);
    EGG::Screen screen;
    screen.SetFovy(mSecondaryFovy);
    f32 nearZ = l_hostIO.mNear;
    f32 farZ = l_hostIO.mFar;
    screen.mNearZ = nearZ;
    screen.mFarZ = farZ;
    screen.CopyToG3D(camera);
    return SUCCEEDED;
}

// 801873C0
int dCamera_c::doDelete() {
    sInstance = NULL;
    dScene::setSceneE848(0);
    return SUCCEEDED;
}

// 801873F0
int dCamera_c::execute() {
    changeMode();
    executeMode();
    nw4r::g3d::Camera camera = m3d::getCurrentCamera();
    mCamera.doUpdateMatrix();
    mViewMtx = mCamera.getViewMatrix();
    PSMTXInverse(mViewMtx, mViewMtx);
    mCamera.setG3DCamera(camera);
    const nw4r::math::MTX34 &view = mCamera.getViewMatrix();
    f32 aspect = mFrustum.mSize.x / mFrustum.mSize.y;
    f32 farZ = l_hostIO.mFar;
    f32 nearZ = l_hostIO.mNear;
    mCullFrustum.set(l_hostIO.mFovy, aspect, nearZ, farZ, view);
    return SUCCEEDED;
}

// 80187510
int dCamera_c::draw() {
    nw4r::g3d::Camera camera = m3d::getCurrentCamera();
    mFrustum.SetFovy(l_hostIO.mFovy);
    mFrustum.mNearZ = l_hostIO.mNear;
    mFrustum.mFarZ = l_hostIO.mFar;
    mFrustum.CopyToG3D(camera);
    updateSecondaryProjection();
    return SUCCEEDED;
}

// 801875CC
void dCamera_c::changeMode() {
    int next = mRequestedMode;
    if (next != MODE_NONE) {
        if (mMode != next) {
            mPreviousMode = mMode;
        }
        mMode = mRequestedMode;
        if (l_modeFuncs[mRequestedMode][0]) {
            (this->*l_modeFuncs[mRequestedMode][0])();
        }
        mRequestedMode = MODE_NONE;
        if (mPendingTransition != 22) {
            setTransition(mPendingTransition);
            mPendingTransition = 22;
            startTransition();
        }
    }
}

// 80187680
void dCamera_c::executeMode() {
    if (l_modeFuncs[mMode][1]) {
        (this->*l_modeFuncs[mMode][1])();
    }
}

// 801876F0
void dCamera_c::initBounds() {
    if (dScene::getCurrentScene() == dScene::SCENE_TOWN) {
        mMinX = 112.0f;
        mMaxX = 1424.0f;
    } else if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM) ||
               dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
        dBGCF::getArea(&mMinX, &mMaxX, &mMinZ, &mMaxZ);
        mRoomPitch = 1;
        mRoomYaw = 1;
    } else if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
        nw4r::math::VEC3 min;
        nw4r::math::VEC3 max;
        dFdBase_c::getUnitPos(&min, 1, 1, 0, 0);
        dFdBase_c::getUnitPos(&max, 6, 6, 15, 15);
        mMinX = min.x;
        mMaxX = mMinX + 2560.0f;
    } else {
        mMinX = -NW4R_MATH_FLT_MAX;
        mMaxX = NW4R_MATH_FLT_MAX;
    }
}

// 801877E4
int dCamera_c::getHorizontalEdge(const mVec3_c *pos) {
    if (pos->x < 80.0f + mMinX) {
        return 1;
    }
    if (pos->x > mMaxX - 80.0f) {
        return 2;
    }
    return 0;
}

// 80187824
BOOL dCamera_c::clampTarget(mVec3_c *pos) {
    BOOL clamped = FALSE;
    if (mMaxX - mMinX <= 160.0f) {
        pos->x = 0.5f * (mMaxX + mMinX);
        clamped = TRUE;
    } else if (pos->x < 80.0f + mMinX) {
        pos->x = 80.0f + mMinX;
        clamped = TRUE;
    } else if (pos->x > mMaxX - 80.0f) {
        pos->x = mMaxX - 80.0f;
        clamped = TRUE;
    }
    if (mMaxZ - mMinZ <= 112.0f) {
        pos->z = 0.5f * (mMaxZ + mMinZ);
        clamped = TRUE;
    } else if (pos->z < 32.0f + mMinZ) {
        pos->z = 32.0f + mMinZ;
        clamped = TRUE;
    } else if (pos->z > mMaxZ - 80.0f) {
        pos->z = mMaxZ - 80.0f;
        clamped = TRUE;
    }
    return clamped;
}

// 80187904
void dCamera_c::followTarget(const mVec3_c *pos) {
    mVec3_c diff = mDesired.mTarget - *pos;
    f32 dist = PSVECMag(diff);
    if (dist > l_hostIO.mFollowRadius) {
        f32 over = dist - l_hostIO.mFollowRadius;
        if (diff.normalizeRS()) {
            mDesired.mTarget.x -= diff.x * over;
            mDesired.mTarget.y -= diff.y * over;
            mDesired.mTarget.z -= diff.z * over;
        }
    }
}

// 801879B8
void dCamera_c::followClampedTarget(const mVec3_c *pos) {
    mVec3_c p(*pos);
    f32 margin = l_hostIO.mSideMargin;
    if (mMaxX - mMinX <= 2.0f * margin) {
        p.x = 0.5f * (mMaxX + mMinX);
    } else if (p.x < mMinX + margin) {
        p.x = mMinX + margin;
    } else if (p.x > mMaxX - margin) {
        p.x = mMaxX - margin;
    }
    followTarget(&p);
}

// 80187A58
void dCamera_c::setClampedTarget(const mVec3_c *pos) {
    mVec3_c p(*pos);
    f32 margin = l_hostIO.mSideMargin;
    if (mMaxX - mMinX <= 2.0f * margin) {
        p.x = 0.5f * (mMaxX + mMinX);
    } else if (p.x < mMinX + margin) {
        p.x = mMinX + margin;
    } else if (p.x > mMaxX - margin) {
        p.x = mMaxX - margin;
    }
    mDesired.mTarget = p;
}

// 80187AF8
static bool isMuseumExhibit(u8 scene) {
    return scene == dScene::SCENE_RM_MM_FISH || scene == dScene::SCENE_RM_MM_INS ||
           scene == dScene::SCENE_RM_MM_FOSSIL0 || scene == dScene::SCENE_RM_MM_FOSSIL1 ||
           scene == dScene::SCENE_RM_MM_PIC0 || scene == dScene::SCENE_RM_MM_PIC1;
}

inline int dCamera_c::getRoomRow(u8 scene) {
    int row = mRoomPitch;
    if (isMuseumExhibit(scene)) {
        if (row == 0) {
            row = 3;
        } else if (row == 2) {
            row = 5;
        } else if (row == 1 && scene == dScene::SCENE_RM_MM_FOSSIL1) {
            row = 4;
        }
    }
    return row;
}

static inline void getPlayerTarget(mVec3_c *pos) {
    dActor_c *player = (dActor_c *)dScene::getScenePlayer();
    if (player != NULL) {
        *pos = *player->getPosP();
        pos->y += 20.0f;
    }
}

static inline void getSoundTarget(mVec3_c *pos) {
    mVec3_c *sound = dPlayerMgr::getCurrentSoundPositionRaw();
    if (sound != NULL) {
        *pos = *sound;
        pos->y += 20.0f;
    }
}

// 80187B1C
void dCamera_c::initFollow() {
    if (dWorld::isCurve()) {
        if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN)) {
            setPreset(0);
        } else if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM)) {
            setPreset(6);
        } else {
            setPreset(51);
        }
        mLookUpTimer = l_hostIO.mLookUpDelay;
        setTransition(0);
    } else {
        u8 scene = dScene::getCurrentScene();
        setPreset(l_roomPresets[getRoomRow(scene)][mRoomYaw]);
        setTransition(0);
    }
    if (mLifecycleState == CREATING) {
        if (dScene::getCurrentScene() == dScene::SCENE_DM_LOAD || dScene::getCurrentScene() == dScene::SCENE_DM_SAVE) {
            mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
        } else {
            mDesired.mTarget = *dScene::getScenePlayerPos();
            dBGCF::groundChk_c chk(&mDesired.mTarget, 0, 0, 0);
            mDesired.mTarget.y = 20.0f + chk.getHeight(FALSE);
        }
        finishTransition();
    } else {
        startTransition();
    }
}

// 80187D00
void dCamera_c::execFollow() {
    mVec3_c pos(mDesired.mTarget);
    mVec3_c *sound = dPlayerMgr::getCurrentSoundPositionRaw();
    if (sound != NULL) {
        pos = *sound;
        pos.y += 20.0f;
    }
    u8 scene = dScene::getCurrentScene();
    if (!dWorld::isCurve()) {
        mDesired.mTarget = pos;
        if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_FTR_ROOM)) {
            if (getRoomRow(scene) != 0) {
                clampTarget(&mDesired.mTarget);
            }
        } else {
            clampTarget(&mDesired.mTarget);
        }
    } else {
        if (!dPlayerMgr::isIdle(4) && !dPlayerMgr::isWaitingForDemo(4)) {
            mLookUpTimer = l_hostIO.mLookUpDelay;
            if (!dPlayerMgr::isStartingDemo(4) && mPreset == 20) {
                startTransition();
                setPreset(0);
                setTransition(7);
            }
        } else if (!dScene::isCityScene(dScene::getCurrentScene()) && mLookUpTimer == 0 && mPreset != 20) {
            startTransition();
            setPreset(20);
            setTransition(6);
        }
        followClampedTarget(&pos);
    }
    updateTransition();
    updateView();
}

// 80187F0C
void dCamera_c::initPlayerView1() {
    startTransition();
    if (dWorld::isCurve()) {
        setPreset(17);
    } else {
        setPreset(18);
    }
    setTransition(1);
}

// 80187F6C
void dCamera_c::execPlayerView1() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    getPlayerTarget(&pos);
    mDesired.mTarget = pos;
    updateTransition();
    updateView();
}

// 80187FF8
void dCamera_c::initPlayerView2() {
    startTransition();
    setPreset(19);
    setTransition(0);
}

// 8018803C
void dCamera_c::execPlayerView2() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    getPlayerTarget(&pos);
    mDesired.mTarget = pos;
    updateTransition();
    updateView();
}

// 801880C8
void dCamera_c::initPlayerView3() {
    startTransition();
    setPreset(1);
    setTransition(1);
}

// 8018810C
void dCamera_c::execPlayerView3() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    getPlayerTarget(&pos);
    mDesired.mTarget = pos;
    updateTransition();
    updateView();
}

// 80188198
void dCamera_c::initTitleSelect() {
    finishTransition();
    setPreset(0);
    setTransition(0);
}

static long l_titleNpcIndex = -1;
static long l_titleNpcRemaining = 10;
static u8 l_titleNpcWrapped;

// 801881DC
void dCamera_c::execTitleSelect() {
    dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
    if (l_titleNpcIndex == -1) {
        l_titleNpcIndex = cM::rndInt(10);
    }
    dDemoActor_c *npc = NULL;
    long idx = l_titleNpcIndex;
    if (!l_titleNpcWrapped) {
        while (npc == NULL && l_titleNpcRemaining > 0) {
            dAnimal_c *animal = block->getAnimalConst(idx);
            if (animal != NULL && animal->mID.isValid() && animal->mPlace == 0) {
                dItem::Item key = block->getAnimalKey(&animal->mID);
                npc = dNpcList::getNmlByKey(key);
                if (npc != NULL) {
                    l_titleNpcIndex = idx;
                }
            }
            if (sLib::chase(&idx, 10, 1)) {
                idx = 0;
            }
            l_titleNpcRemaining--;
        }
    }
    if (npc != NULL) {
        mWork.mpNpc = (dAcNpc_c *)npc;
        mDefaultMode = MODE_TITLE_NPC;
        if (sLib::chase(&l_titleNpcIndex, 10, 1)) {
            l_titleNpcIndex = 0;
            l_titleNpcWrapped = TRUE;
        }
    } else {
        mDefaultMode = MODE_TITLE_BUILDING;
        l_titleNpcWrapped = FALSE;
        l_titleNpcRemaining = 10;
    }
    mRequestedMode = mDefaultMode;
    changeMode();
    executeMode();
}

// 8018835C
void dCamera_c::initTitleNpc() {
    setPreset(60);
    setTransition(0);
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    if (mWork.mpNpc != NULL) {
        pos = *mWork.mpNpc->getPosP();
        dBGCF::groundChk_c chk(&pos, 0, 0, 0);
        pos.y = 20.0f + chk.getHeight(FALSE);
    }
    mDesired.mTarget = pos;
    finishTransition();
}

// 8018841C
void dCamera_c::execTitleNpc() {
    if (mWork.mpNpc != NULL && mWork.mpNpc->mIsActive) {
        mVec3_c pos(0.0f, 0.0f, 0.0f);
        pos = *mWork.mpNpc->getPosP();
        dBGCF::groundChk_c chk(&pos, 0, 0, 0);
        pos.y = 20.0f + chk.getHeight(FALSE);
        setClampedTarget(&pos);
    }
    updateTransition();
    updateView();
}

// 801884D4
void dCamera_c::initTitleBuilding() {
    mWork.mTitle.mIndex = 0;
    dItem::Item id = dSaveBuildingList_c::getId(l_titleBuildings[mWork.mTitle.mIndex].mBuilding);
    int x;
    int z;
    dSaveTown_c *town = dSaveData_c::getRaw();
    if (town->mBuilding.mList.getPos(&x, &z, &id, 1)) {
        mVec3_c pos;
        pos.x = x * 32;
        pos.z = z * 32;
        pos.y = 16.0f + dBGCF::getUnitY(&pos);
        mWork.mTitle.mPosition.x = pos.x;
        mWork.mTitle.mPosition.y = pos.y;
        mWork.mTitle.mPosition.z = pos.z;
        mWork.mTitle.mSpeed = 0.0f;
        mWork.mTitle.mTimer = 180;
        mDesired.mTarget = pos;
        mDesired.mTarget += mVec3_c(l_titleBuildings[mWork.mTitle.mIndex].mOffset);
    }
    setPreset(0);
}

// 8018863C
void dCamera_c::execTitleBuilding() {
    title_c *title = &mWork.mTitle;
    mVec3_c pos(title->mPosition.x, title->mPosition.y, title->mPosition.z);
    pos += mVec3_c(l_titleBuildings[title->mIndex].mOffset);
    sLib::chase(&title->mSpeed, 1.0f, 0.0012f);
    cLib::addCalcPos(&mDesired.mTarget, pos, 0.03125f, title->mSpeed, 0.002f);
    if (mDesired.mTarget.distTo(pos) < 0.002f && sLib::calcTimer(&title->mTimer) == 0) {
        int x;
        int z;
        while (true) {
            if (sLib::chase(&title->mIndex, 5, 1)) {
                title->mIndex = 0;
            }
            dItem::Item id = dSaveBuildingList_c::getId(l_titleBuildings[title->mIndex].mBuilding);
            dSaveTown_c *town = dSaveData_c::getRaw();
            if (town->mBuilding.mList.getPos(&x, &z, &id, 1)) {
                break;
            }
        }
        mVec3_c p;
        p.x = x * 32;
        p.z = z * 32;
        title->mPosition.x = p.x;
        title->mPosition.y = p.y;
        title->mPosition.z = p.z;
        title->mSpeed = 0.0f;
        title->mTimer = 180;
        title->mPosition.y = 16.0f + dBGCF::getUnitY(&p);
    }
    updateTransition();
    updateView();
}

// 80188820
void dCamera_c::initTool() {
    startTransition();
    setPreset(0);
    setTransition(2);
}

// 80188864
void dCamera_c::execTool() {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    mVec3_c tool(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = (dActor_c *)dScene::getScenePlayer();
    if (actor != NULL) {
        player = *actor->getPosP();
        player.y += 20.0f;
        target = player;
        mVec3_c *toolPos = dPlayerMgr::getToolOrPlayerPosition(4);
        if (toolPos != NULL) {
            mVec3_c diff(*toolPos);
            diff -= player;
            diff.y *= 0.5f;
            diff *= 0.75f;
            target += diff;
        }
    }
    followTarget(&target);
    updateTransition();
    updateView();
}

// 80188984
void dCamera_c::initPlayerView8() {
    startTransition();
    setPreset(21);
    setTransition(5);
}

// 801889C8
void dCamera_c::execPlayerView8() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    getPlayerTarget(&pos);
    mDesired.mTarget = pos;
    updateTransition();
    updateView();
}

// 80188A54
void dCamera_c::initTalk() {
    startTransition();
    if (dWorld::isCurve()) {
        setPreset(22);
    } else {
        mTalkSide = SIDE_AUTO;
        setPreset(23);
    }
    setTransition(0);
}

// 80188ABC
void dCamera_c::execTalk() {
    if (dWorld::isCurve()) {
        updateOutdoorTalk();
    } else {
        updateIndoorTalk();
    }
    updateTransition();
    updateView();
}

// 80188B14
static int getShopIndex() {
    int idx = 0;
    switch (dScene::getCurrentScene()) {
    case dScene::SCENE_RM_SHOP0:
        idx = 0;
        break;
    case dScene::SCENE_RM_SHOP1:
        idx = 1;
        break;
    case dScene::SCENE_RM_SHOP2:
        idx = 2;
        break;
    case dScene::SCENE_RM_SHOP3_F2:
        idx = 3;
        break;
    case dScene::SCENE_RM_GRACE:
        idx = 4;
        break;
    }
    return idx;
}

// 80188B90
void dCamera_c::initShopWide() {
    int idx = getShopIndex();
    startTransition();
    setPreset(idx + 24);
    setTransition(0);
}

// 80188BEC
void dCamera_c::execShopWide() {
    mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
    updateTransition();
    updateView();
}

// 80188C30
void dCamera_c::initShopClose() {
    int idx = getShopIndex();
    startTransition();
    setPreset(idx + 29);
    setTransition(0);
}

// 80188C8C
void dCamera_c::execShopClose() {
    mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
    updateTransition();
    updateView();
}

// 80188CD0
void dCamera_c::initPlayerSelect() {
    setPreset(34);
}

// 80188CD8
void dCamera_c::execPlayerSelect() {
    mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
    updateTransition();
    updateView();
}

// 80188D1C
void dCamera_c::initPlayerView13() {
    startTransition();
    setPreset(35);
    setTransition(0);
}

// 80188D60
void dCamera_c::execPlayerView13() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    getPlayerTarget(&pos);
    mDesired.mTarget = pos;
    updateTransition();
    updateView();
}

// 80188DEC
void dCamera_c::setBusBounce(u8 kind) {
    mWork.mBus.mBounceKind = kind;
    if (mWork.mBus.mBounceKind >= 4) {
        mWork.mBus.mBounceKind = 0;
    }
    mWork.mBus.mBounceTimer = l_busMotions[mWork.mBus.mBounceKind].mFrames;
    mWork.mBus.mBounceTimer += cM::rndInt(l_busMotions[mWork.mBus.mBounceKind].mRandomFrames);
    mWork.mBus.mBounceAngle = 0;
}

// 80188E64
void dCamera_c::setBusRoll(u8 kind) {
    mWork.mBus.mRollKind = kind;
    if (mWork.mBus.mRollKind >= 4) {
        mWork.mBus.mRollKind = 0;
    }
    mWork.mBus.mRollTimer = l_busMotions[mWork.mBus.mRollKind].mFrames;
    mWork.mBus.mRollTimer += cM::rndInt(l_busMotions[mWork.mBus.mRollKind].mRandomFrames);
    mWork.mBus.mRollAngle = 0;
}

// 80188EDC
f32 dCamera_c::updateBusMotion() {
    if (sLib::calcTimer(&mWork.mBus.mBounceTimer) == 0 && abs(mWork.mBus.mBounceAngle) < 256) {
        if (mWork.mBus.mBounceKind == 0) {
            if (cM::rndInt(100) < 80) {
                mWork.mBus.mBounceKind = 1;
            } else {
                mWork.mBus.mBounceKind = 2;
            }
        } else if (mWork.mBus.mBounceKind == 2) {
            mWork.mBus.mBounceKind = 1;
        } else {
            mWork.mBus.mBounceKind = 0;
        }
        setBusBounce(mWork.mBus.mBounceKind);
    }
    if (sLib::calcTimer(&mWork.mBus.mRollTimer) == 0 && abs(mWork.mBus.mRollAngle) < 256) {
        if (mWork.mBus.mRollKind == 0) {
            mWork.mBus.mRollKind = 1;
        } else if (mWork.mBus.mRollKind == 2) {
            mWork.mBus.mRollKind = 1;
        } else {
            mWork.mBus.mRollKind = 0;
        }
        setBusRoll(mWork.mBus.mRollKind);
    }
    if (l_busMotions[mWork.mBus.mRollKind].mSpeed != 0) {
        mWork.mBus.mRollAngle += l_busMotions[mWork.mBus.mRollKind].mSpeed;
        f32 s = mAng(mWork.mBus.mRollAngle).sin();
        mBusRoll = s * l_busMotions[mWork.mBus.mRollKind].mAmplitude;
    }
    if (l_busMotions[mWork.mBus.mBounceKind].mSpeed != 0) {
        mWork.mBus.mBounceAngle += l_busMotions[mWork.mBus.mBounceKind].mSpeed;
        f32 s = mAng(mWork.mBus.mBounceAngle).sin();
        return 16.0f * (s * l_busMotions[mWork.mBus.mBounceKind].mAmplitude);
    }
    return 0.0f;
}

// 801890B0
void dCamera_c::initBus() {
    setPreset(36);
    setBusBounce(0);
}

// 801890EC
void dCamera_c::execBus() {
    mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
    mDesired.mTarget.y += updateBusMotion();
    updateTransition();
    updateView();
}

// 80189144
void dCamera_c::initGate() {
    setPreset(39);
}

// 8018914C
void dCamera_c::execGate() {
    mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
    updateTransition();
    updateView();
}

// 80189190
void dCamera_c::initFixedView16() {
    startTransition();
    setPreset(42);
    setTransition(8);
}

// 801891D4
void dCamera_c::execFixedView16() {
    mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
    updateTransition();
    updateView();
}

// 80189218
void dCamera_c::initTwoTarget() {
    startTransition();
    setPreset(43);
    setTransition(16);
}

// 8018925C
void dCamera_c::execTwoTarget() {
    mVec3_c pos;
    dActor_c *player = (dActor_c *)dScene::getScenePlayer();
    if (player != NULL) {
        pos = *player->getPosP();
    } else {
        pos = mTalkTarget;
    }
    mDesired.mTarget = pos + mTalkTarget;
    mDesired.mTarget *= 0.5f;
    mDesired.mTarget.z += (int)__fabs(0.5f * (pos.z - mTalkTarget.z));
    updateTransition();
    updateView();
}

// 80189358
void dCamera_c::initPlayerView18() {
    startTransition();
    setPreset(44);
    setTransition(0);
}

// 8018939C
void dCamera_c::execPlayerView18() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    getPlayerTarget(&pos);
    mDesired.mTarget = pos;
    updateTransition();
    updateView();
}

// 80189428
void dCamera_c::initCafe() {
    mWork.mCafe.mOrbitAngle = 0;
    mWork.mCafe.mShot = -1;
    mWork.mCafe.mState = 0;
    mWork.mCafe.mHistory[0] = -1;
    mWork.mCafe.mHistory[1] = -1;
    mWork.mCafe.mHistoryIndex = 0;
    mWork.mCafe.mPhase = -1;
    mWork.mCafe._10 = 0;
    startTransition();
    mDesired.mTarget = mVec3_c::Zero;
    setPreset(45);
    setTransition(0);
}

// 801894B4
void dCamera_c::execCafe() {
    dBgm::StgCafe_c *cafe = &dBgm::l_mgr.mStgCafe;
    if (cafe != NULL && cafe->isKKPlaying()) {
        switch (mWork.mCafe.mState) {
        case 0:
            startCafeIntro();
            break;
        case 1:
        case 2:
            if (mWork.mCafe.mPhase != cafe->getKKInfo4()) {
                mWork.mCafe.mPhase = cafe->getKKInfo4();
                if (mWork.mCafe.mPhase == 2) {
                    startCafeOutro();
                } else {
                    setCafeShot(-1);
                }
            }
            break;
        case 3:
            break;
        }
    }
    if (mWork.mCafe.mState == 2) {
        const cafeShot_c *shot = getCafeShot(mWork.mCafe.mShot);
        mDesired.mOffset.x = shot->mOffset.x;
        mDesired.mOffset.y = shot->mOffset.y;
        mDesired.mOffset.z = shot->mOffset.z;
        int minDist = 20;
        if (mDesired.mDistance > minDist) {
            mDesired.mDistance += shot->mDistanceStep;
        }
        mDesired.mYaw += shot->mYawStep;
        mWork.mCafe.mOrbitAngle += shot->mYawStep;
        mVec3_c move(shot->mOrbitX, 0.0f, shot->mOrbitZ);
        mDesired.mOffset += move;
        move.rotY(mWork.mCafe.mOrbitAngle);
        mDesired.mOffset -= move;
    }
    updateTransition();
    updateView();
}

// 80189690
void dCamera_c::startCafeIntro() {
    if (mMode == MODE_CAFE) {
        mDesired.mTarget = mVec3_c::Zero;
        mWork.mCafe.mState = 1;
        startTransition();
        setPreset(46);
        setTransition(10);
    }
}

// 80189708
void dCamera_c::setCafeShot(int shot) {
    if (mMode == MODE_CAFE) {
        cafe_c *cafe = getCafe();
        cafe->mOrbitAngle = 0;
        if (shot < 0) {
            dSearchCand_c<14> cand;
            cand.setAll();
            for (int i = 0; i < 2; i++) {
                if (cafe->mHistory[i] != -1) {
                    cand.remove(cafe->mHistory[i]);
                }
            }
            shot = cand.getRandom();
            cafe->mHistory[cafe->mHistoryIndex] = shot;
            cafe->mHistoryIndex ^= 1;
        }
        cafe->mShot = shot;
        cafe->mState = 2;
        const cafeShot_c *data = getCafeShot(cafe->mShot);
        mDesired.mTarget = mVec3_c::Zero;
        mDesired.mOffset.x = data->mOffset.x;
        mDesired.mOffset.y = data->mOffset.y;
        mDesired.mOffset.z = data->mOffset.z;
        mDesired.mYaw = data->mYaw;
        mDesired.mPitch = data->mPitch;
        mDesired.mDistance = data->mDistance;
        finishTransition();
    }
}

// 8018984C
void dCamera_c::startCafeOutro() {
    if (mMode == MODE_CAFE) {
        mDesired.mTarget = mVec3_c::Zero;
        mWork.mCafe.mState = 3;
        finishTransition();
        setPreset(47);
        updateTransition();
        startTransition();
        setPreset(45);
        setTransition(11);
    }
}

// 801898E0
void dCamera_c::initFixedView20() {
    startTransition();
    setPreset(41);
    setTransition(0);
}

// 80189924
void dCamera_c::execFixedView20() {
    mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
    updateTransition();
    updateView();
}

// 80189968
void dCamera_c::initPlayerView21() {
    startTransition();
    if (dWorld::isCurve()) {
        setPreset(17);
    } else {
        setPreset(18);
    }
    setTransition(1);
}

// 801899C8
void dCamera_c::execPlayerView21() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    getPlayerTarget(&pos);
    mDesired.mTarget = pos;
    updateTransition();
    updateView();
}

// 80189A54
void dCamera_c::initScriptedAction() {
    startTransition();
    setPreset(0);
    setTransition(4);
}

// 80189A98
void dCamera_c::execScriptedAction() {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = (dActor_c *)dScene::getScenePlayer();
    if (actor != NULL) {
        player = *actor->getPosP();
        player.y += 20.0f;
        target = player;
        mVec3_c action;
        if (dPlayerMgr::getScriptedActionPosition(&action)) {
            mVec3_c diff = action - player;
            diff.y *= 0.5f;
            diff *= 0.75f;
            target += diff;
        } else {
            target = mDesired.mTarget;
        }
    }
    followClampedTarget(&target);
    updateTransition();
    updateView();
}

// 80189BC4
void dCamera_c::initSoundTarget23() {
    setPreset(52);
    setTransition(19);
    mLookUpTimer = 0;
    startTransition();
}

// 80189C10
void dCamera_c::execSoundTarget23() {
    mVec3_c pos(mDesired.mTarget);
    mVec3_c *sound = dPlayerMgr::getCurrentSoundPositionRaw();
    if (sound != NULL) {
        pos = *sound;
        pos.y += 20.0f;
    }
    followClampedTarget(&pos);
    updateTransition();
    updateView();
}

// 80189C9C
void dCamera_c::initSoundTarget24() {
    setPreset(50);
    setTransition(0);
    startTransition();
}

// 80189CE0
void dCamera_c::execSoundTarget24() {
    mVec3_c pos(mDesired.mTarget);
    getSoundTarget(&pos);
    mDesired.mTarget = pos;
    updateTransition();
    updateView();
}

// 80189D74
void dCamera_c::initFixedView25() {
    setPreset(56);
    setTransition(20);
    startTransition();
}

// 80189DB8
void dCamera_c::execFixedView25() {
    mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
    updateTransition();
    updateView();
}

// 80189DFC
void dCamera_c::initPlayerView26() {
    startTransition();
    setPreset(59);
    setTransition(0);
}

// 80189E40
void dCamera_c::execPlayerView26() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    getPlayerTarget(&pos);
    mDesired.mTarget = pos;
    updateTransition();
    updateView();
}

// 80189ECC
void dCamera_c::initCityIntro() {
    setPreset(57);
    finishTransition();
}

// 80189F04
void dCamera_c::execCityIntro() {
    mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
    updateTransition();
    updateView();
    if (mPreset == 57 && mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        mDefaultMode = MODE_FOLLOW;
        mRequestedMode = MODE_FOLLOW;
        mPendingTransition = 21;
        dBgm::l_mgr.mStgTown.play1AB7();
    }
}

// 80189F9C
void dCamera_c::initLookUp() {
    mLookUpTimer = 0;
    if (mPreset != 20) {
        setPreset(20);
        setTransition(6);
        startTransition();
    }
}

// 80189FF4
void dCamera_c::execLookUp() {
    mVec3_c pos(mDesired.mTarget);
    mVec3_c *sound = dPlayerMgr::getCurrentSoundPositionRaw();
    if (sound != NULL) {
        pos = *sound;
        pos.y += 20.0f;
    }
    followClampedTarget(&pos);
    updateTransition();
    updateView();
}

// 8018A080
mVec3_c dCamera_c::screenToWorld(const mVec2_c *screen) const {
    mMtx_c mtx = mViewMtx;
    f32 tanHalf = 1.0f / mFrustum.mTanFovY;
    f32 nearZ = mFrustum.mNearZ;
    f32 aspect = mFrustum.mSize.x / mFrustum.mSize.y;
    f32 height = 2.0f * nearZ * tanHalf;
    f32 width = aspect * height;
    mVec3_c p(0.5f * (screen->x * width), 0.5f * (-screen->y * height), -nearZ);
    mVec3_c pos;
    PSMTXMultVec(mtx, p, pos);
    return pos;
}

// 8018A16C
void dCamera::worldToScreen(const dCamera_c *cam, f32 *x, f32 *y, const mVec3_c *pos) {
    mVec3_c view;
    PSMTXMultVec(const_cast<dCamera_c *>(cam)->mCamera.getViewMatrix(), *pos, view);
    nw4r::math::MTX44 proj;
    dCamera_c::sInstance->getProjectionMtx(&proj);
    f32 rate;
    f32 sx = proj._00 * view.x + proj._01 * view.y + proj._02 * view.z + proj._03;
    f32 sy = proj._10 * view.x + proj._11 * view.y + proj._12 * view.z + proj._13;
    f32 w = proj._30 * view.x + proj._31 * view.y + proj._32 * view.z + proj._33;
    rate = 1.0f / (2.0f * w);
    m3d::getCurrentCamera();
    *x = 0.5f + sx * rate;
    *y = 0.5f - sy * rate;
}

// 8018A2D8
f32 dCamera_c::calcTransitionWeight(f32 time, f32 start, f32 end, f32 easeIn, f32 easeOut) {
    if (time >= end) {
        return 1.0f;
    }
    if (time <= start) {
        return 0.0f;
    }
    f32 len = end - start;
    f32 t = time - start;
    if (len < easeIn + easeOut) {
        return 0.0f;
    }
    f32 speed = 1.0f / (2.0f * len - easeIn - easeOut);
    f32 rate = 0.0f;
    if (easeIn != 0.0f) {
        if (t <= easeIn) {
            rate = speed * (t * t) / easeIn;
            if (rate > 1.0f) {
                return 1.0f;
            }
            return rate;
        }
        rate = speed * easeIn;
    }
    if (t <= len - easeOut) {
        rate += 2.0f * (speed * (t - easeIn));
        if (rate > 1.0f) {
            return 1.0f;
        }
        return rate;
    }
    rate += 2.0f * (speed * (len - easeIn - easeOut));
    if (easeOut != 0.0f) {
        rate += speed * easeOut;
        if (t < len) {
            rate -= speed * ((len - t) * (len - t)) / easeOut;
        }
    }
    if (rate > 1.0f) {
        return 1.0f;
    }
    return rate;
}

// 8018A400
void dCamera_c::finishTransition() {
    mTransitionFrame = mTransition.mEnd;
}

// 8018A40C
void dCamera_c::startTransition() {
    mTransitionFrame = 0.0f;
    mPrevious = mCurrent;
}

// 8018A460
f32 dCamera_c::getTransitionFramesLeft() const {
    return mTransition.mEnd - mTransitionFrame;
}

static inline f32 scale(f32 diff, f32 rate) {
    diff *= rate;
    return diff;
}

// 8018A470
void dCamera_c::updateTransition() {
    u32 cull = usesForegroundCulling();
    if (isTransitionActive()) {
        f32 rate = calcTransitionWeight(mTransitionFrame, mTransition.mStart, mTransition.mEnd, mTransition.mEaseIn,
                                        mTransition.mEaseOut);
        mVec3_c diff = mDesired.mTarget - mPrevious.mTarget;
        diff *= rate;
        mCurrent.mTarget = mPrevious.mTarget + diff;
        mVec3_c ofs = mDesired.mOffset - mPrevious.mOffset;
        ofs *= rate;
        mCurrent.mOffset = mPrevious.mOffset + ofs;
        mCurrent.mPitch = mPrevious.mPitch + scale(mDesired.mPitch - mPrevious.mPitch, rate);
        mCurrent.mYaw = mPrevious.mYaw + scale(mDesired.mYaw - mPrevious.mYaw, rate);
        mCurrent.mDistance = mPrevious.mDistance + scale(mDesired.mDistance - mPrevious.mDistance, rate);
        mTransitionFrame += 1.0f;
        if (rate > 0.5f && cull != mCullForeground) {
            mCullForeground = cull;
        }
    } else {
        mCurrent = mDesired;
        mPrevious = mDesired;
        if (cull != mCullForeground) {
            mCullForeground = cull;
        }
    }
    if (mCullForeground && cull) {
        if (mMode == MODE_TALK) {
            mForegroundZ = mDesired.mTarget.z + getTalkCullOffset();
        } else {
            mForegroundZ = mDesired.mTarget.z;
        }
        mTargetZ = mDesired.mTarget.z;
    }
}

// 8018A748
bool dCamera_c::isTransitionActive() const {
    return mTransitionFrame <= mTransition.mEnd;
}

// 8018A764
void dCamera_c::updateView() {
    mVec3_c &center = sCenter;
    center = mCurrent.mTarget;
    center += mCurrent.mOffset;
    mVec3_c eye(0.0f, 0.0f, mCurrent.mDistance);
    eye.rotX(mCurrent.mPitch);
    eye.rotY(mCurrent.mYaw);
    mEye = center + eye;
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c at(0.0f, 0.0f, -1000.0f);
    mVec3_c up(mVec3_c::Ey);
    mAng angle = dWorld::curvePosition(&at, &center);
    mCurveAngle = dWorld::curvePosition(&pos, &mEye);
    up.rotX(angle);
    sEye = mEye;
    l_viewDistance = PSVECMag(center - sEye);
    mCamera.mPos = pos;
    mCamera.mAt = at;
    mCamera.mUp = up;
}

inline BOOL dCamera_c::isLookUpPreset() {
    BOOL wide = FALSE;
    if (mMode == MODE_FOLLOW && mPreset == 20) {
        wide = TRUE;
    }
    return wide;
}

inline BOOL dCamera_c::checkLookingUp() {
    BOOL wide;
    BOOL ret = TRUE;
    wide = FALSE;
    if (mMode == MODE_FOLLOW && mPreset == 20) {
        wide = TRUE;
    }
    if (!wide && mMode != MODE_LOOK_UP) {
        ret = FALSE;
    }
    return ret;
}

// 8018A938
BOOL dCamera_c::isLookingUp() {
    return checkLookingUp();
}

// 8018A978
BOOL dCamera_c::isFollowLookUp() {
    return isLookUpPreset();
}

// 8018A9A0
BOOL dCamera_c::isControllable() {
    if (mMode != MODE_CITY_INTRO && !isTransitionActive()) {
        return TRUE;
    }
    return FALSE;
}

// 8018A9E0
u32 dCamera_c::usesForegroundCulling() {
    if (isLookingUp() || mMode == MODE_PLAYER_VIEW_3 || mMode == MODE_TALK || mMode == MODE_PLAYER_VIEW_8 || mMode == MODE_PLAYER_VIEW_1 ||
        mMode == MODE_PLAYER_VIEW_21 || mMode == MODE_PLAYER_VIEW_18 || mMode == MODE_PLAYER_VIEW_13 || mMode == MODE_SOUND_TARGET_23) {
        return TRUE;
    }
    return FALSE;
}

// 8018AA64
BOOL dCamera_c::isPastForeground(const mVec3_c *pos, f32 size) {
    if (mCullForeground && pos->z - 0.5f * size > mForegroundZ - 32.0f) {
        return TRUE;
    }
    return FALSE;
}

// 8018AAA4
BOOL dCamera_c::isPastTarget(const mVec3_c *pos, f32 size) {
    if (mCullForeground && pos->z - 0.5f * size > mTargetZ - 32.0f) {
        return TRUE;
    }
    return FALSE;
}

// 8018AAE4
f32 dCamera_c::calcTalkDistance(const mVec3_c *a, const mVec3_c *b, mVec3_c *center, f32 *zOffset) {
    mVec3_c diff = *a - *b;
    mVec3_c d;
    d.y = diff.y;
    d.x = diff.x;
    d.z = 1.5f * diff.z;
    f32 dist = PSVECMag(d);
    if (center != NULL) {
        *center = 0.5f * (*a + *b);
        if (zOffset != NULL) {
            *zOffset = nw4r::math::FAbs(0.5f * diff.z);
        }
    }
    return dist;
}

// 8018ABD4
f32 dCamera_c::calcThreeActorTalkDistance(const mVec3_c *a, const mVec3_c *b, const mVec3_c *c, mVec3_c *center,
                                           f32 *zOffset) {
    return calcTalkDistance(a, b, center, zOffset);
}

// 8018ABE0
void dCamera_c::updateOutdoorTalk() {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    dActor_c *actor = (dActor_c *)dScene::getScenePlayer();
    if (actor != NULL) {
        player = *actor->getPosP();
        player.y += 20.0f;
    }
    f32 zOffset;
    f32 dist;
    if (mHasSecondTalkTarget) {
        dist = calcThreeActorTalkDistance(&player, &mTalkTarget, &mTalkTarget2, &mDesired.mTarget, &zOffset);
    } else {
        dist = calcTalkDistance(&player, &mTalkTarget, &mDesired.mTarget, &zOffset);
    }
    mDesired.mTarget.z += zOffset;
    f32 clamped;
    if (dist < 72.0f) {
        clamped = 72.0f;
    } else if (dist > 176.0f) {
        clamped = 176.0f;
    } else {
        clamped = dist;
    }
    blendPresets(22, 0, (clamped - 72.0f) / 104.0f);
}

static inline s16 minAngle(int a, int b) {
    return a > b ? b : a;
}

// 8018ACE0
void dCamera_c::updateIndoorTalk() {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    dActor_c *actor = (dActor_c *)dScene::getScenePlayer();
    if (actor != NULL) {
        player = *actor->getPosP();
        player.y += 20.0f;
    }
    mVec3_c front;
    f32 zOffset;
    f32 dist = calcTalkDistance(&player, &mTalkTarget, &mDesired.mTarget, &zOffset);
    f32 clamped;
    if (dist < 72.0f) {
        clamped = 72.0f;
    } else if (dist > 176.0f) {
        clamped = 176.0f;
    } else {
        clamped = dist;
    }
    f32 rate = (clamped - 72.0f) / 104.0f;
    blendPresets(23, 6, rate);
    if (player.z > mTalkTarget.z) {
        front = player;
    } else {
        front = mTalkTarget;
    }
    int areaSide = getHorizontalEdge(&mDesired.mTarget);
    f32 dx = 0.0f;
    s16 yaw = cLib::targetAngleY(mDesired.mTarget, front);
    static s16 maxYaw = 19.7f * mAng::DegreeToAngleCoefficient;
    static s16 halfMaxYaw = maxYaw / 2;
    s16 turn = 8192.0f - nw4r::math::FAbs(yaw);
    if (turn < 0) {
        turn = 0;
    }
    s16 ang = minAngle((s16)(turn / 8192.0f * maxYaw), maxYaw);
    if (ang > 0 && !mFixedTalkSide) {
        if (mTalkSide == SIDE_AUTO) {
            SIDE_e side = SIDE_LEFT;
            if (yaw > 0) {
                side = SIDE_RIGHT;
            }
            if (areaSide == 1 && side == SIDE_RIGHT) {
                if (nw4r::math::FAbs(yaw) > halfMaxYaw) {
                    side = SIDE_NONE;
                } else {
                    side = SIDE_LEFT;
                }
            }
            if (areaSide == 2 && side == SIDE_LEFT) {
                if (nw4r::math::FAbs(yaw) > halfMaxYaw) {
                    side = SIDE_NONE;
                } else {
                    side = SIDE_RIGHT;
                }
            }
            mTalkSide = side;
        }
        if (mTalkSide == SIDE_RIGHT) {
            ang = -ang;
        }
        if (mTalkSide != SIDE_NONE) {
            mDesired.mYaw = ang * (1.0f - rate);
            dx = zOffset * mAng(mDesired.mYaw).sin();
        }
    }
    mDesired.mTarget.z += zOffset;
    mDesired.mTarget.x += dx;
}

// 8018B08C
static const preset_c *getPreset(int idx) {
    return &l_presets[idx];
}

// 8018B0A0
static const dCamera_c::transition_c *getTransition(int idx) {
    return &l_transitions[idx];
}

// 8018B0B4
void dCamera_c::setPreset(u8 idx) {
    mPreset = idx;
    const preset_c *data = getPreset(mPreset);
    mDesired.mPitch = data->mPitch;
    mDesired.mYaw = data->mYaw;
    mDesired.mDistance = data->mDistance;
    mDesired.mOffset.x = data->mOffset.x;
    mDesired.mOffset.y = data->mOffset.y;
    mDesired.mOffset.z = data->mOffset.z;
}

// 8018B118
void dCamera_c::blendPresets(u8 a, u8 b, f32 rate) {
    mPreset = a;
    const preset_c *da = getPreset(a);
    const preset_c *db = getPreset(b);
    f32 inv = 1.0f - rate;
    mDesired.mPitch = (s16)(da->mPitch * inv) + (s16)(db->mPitch * rate);
    mDesired.mYaw = (s16)(da->mYaw * inv) + (s16)(db->mYaw * rate);
    mDesired.mDistance = da->mDistance * inv + db->mDistance * rate;
    mDesired.mOffset.x = da->mOffset.x * inv + db->mOffset.x * rate;
    mDesired.mOffset.y = da->mOffset.y * inv + db->mOffset.y * rate;
    mDesired.mOffset.z = da->mOffset.z * inv + db->mOffset.z * rate;
}

// 8018B298
void dCamera_c::setTransition(u8 idx) {
    mTransitionId = idx;
    const transition_c *data = getTransition(mTransitionId);
    mTransition.mStart = data->mStart;
    mTransition.mEnd = data->mEnd;
    mTransition.mEaseIn = data->mEaseIn;
    mTransition.mEaseOut = data->mEaseOut;
}

// 8018B2EC
f32 dCamera_c::getAspect() const {
    return mFrustum.mSize.x / mFrustum.mSize.y;
}

// 8018B2FC
f32 dCamera_c::getFovy() {
    return l_hostIO.mFovy;
}

// 8018B308
f32 dCamera_c::getNear() {
    return l_hostIO.mNear;
}

// 8018B318
f32 dCamera_c::getFar() {
    return l_hostIO.mFar;
}

// 8018B328
void dCamera::setSecondaryFovy(f32 fovy) {
    if (dCamera_c::sInstance != NULL) {
        dCamera_c::sInstance->mSecondaryFovy = fovy;
    }
}

// 8018B33C
void dCamera::resetSecondaryFovy() {
    setSecondaryFovy(l_hostIO.mSecondaryFovy);
}

// 8018B34C
void dCamera::useMainFovy() {
    setSecondaryFovy(l_hostIO.mFovy);
}

// 8018B358
void dCamera_c::updateSecondaryProjection() {
    nw4r::g3d::Camera camera = m3d::getCamera(1);
    EGG::Screen screen;
    screen.SetFovy(mSecondaryFovy);
    f32 nearZ = l_hostIO.mNear;
    f32 farZ = l_hostIO.mFar;
    screen.mNearZ = nearZ;
    screen.mFarZ = farZ;
    screen.CopyToG3D(camera);
}

// 8018B404
BOOL dCamera_c::setRoomView(int x, int level) {
    u8 scene = dScene::getCurrentScene();
    if (!dScene::isSceneAttr(scene, dScene::SCENE_ATTR_FTR_ROOM)) {
        return FALSE;
    }
    if (mTransitionFrame < 0.5f * mTransition.mEnd) {
        return FALSE;
    }
    if (mRoomYaw != x || mRoomPitch != level) {
        mRoomYaw = x;
        if (mRoomPitch < level) {
            playSe(0x16DE);
        } else {
            playSe(0x16DD);
        }
        mRoomPitch = level;
        startTransition();
        setPreset(l_roomPresets[getRoomRow(scene)][mRoomYaw]);
        return TRUE;
    }
    return FALSE;
}

// 8018B554
int dCamera_c::resetLookUpTimer() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN) && mMode == MODE_FOLLOW &&
        mPreset != 20) {
        mLookUpTimer = 0;
    }
    return 0;
}

// 8018B5B8
void dCamera_c::startTalkView(const mVec3_c *target) {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    dActor_c *actor = (dActor_c *)dScene::getScenePlayer();
    if (actor != NULL) {
        player = *actor->getPosP();
        player.y += 20.0f;
    }
    if (!(calcTalkDistance(&player, target, NULL, NULL) >= 176.0f)) {
        mHasSecondTalkTarget = FALSE;
        mTalkTarget = *target;
        mRequestedMode = MODE_TALK;
    }
}

// 8018B67C
void dCamera_c::startThreeActorTalkView(const mVec3_c *a, const mVec3_c *b) {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    dActor_c *actor = (dActor_c *)dScene::getScenePlayer();
    if (actor != NULL) {
        player = *actor->getPosP();
        player.y += 20.0f;
    }
    if (!(calcThreeActorTalkDistance(&player, a, b, NULL, NULL) >= 176.0f)) {
        mHasSecondTalkTarget = TRUE;
        mTalkTarget = *a;
        mTalkTarget2 = *b;
        mRequestedMode = MODE_TALK;
    }
}

// 8018B768
void dCamera_c::switchTalkTarget(const mVec3_c *target) {
    mTalkTarget = *target;
    startTransition();
    mTalkSide = SIDE_AUTO;
}

// 8018B7B4
void dCamera_c::startGateView() {
    startTransition();
    setTransition(0);
    mRequestedMode = MODE_GATE;
}

// 8018B7F4
void dCamera_c::startGateCloseUp() {
    if (mMode == MODE_GATE) {
        startTransition();
        setTransition(18);
        setPreset(40);
    }
}

// 8018B844
void dCamera_c::startTwoTargetView(const mVec3_c *pos) {
    mTalkTarget = *pos;
    mRequestedMode = MODE_TWO_TARGET;
}

// 8018B868
void dCamera_c::restoreModeMedium() {
    mRequestedMode = mDefaultMode;
    mPendingTransition = 17;
}

// 8018B87C
void dCamera_c::restoreModeSlow() {
    mRequestedMode = mDefaultMode;
    mPendingTransition = 9;
}

// 8018B890
void dCamera_c::restoreModeFast() {
    mRequestedMode = mDefaultMode;
    mPendingTransition = 3;
}

// 8018B8A4
void dCamera_c::startBusFrontView() {
    if (mMode == MODE_BUS) {
        startTransition();
        setTransition(12);
        setPreset(37);
    }
}

// 8018B8F4
void dCamera_c::startBusWideView() {
    if (mMode == MODE_BUS) {
        startTransition();
        setTransition(13);
        setPreset(36);
    }
}

// 8018B944
void dCamera_c::startBusSideView() {
    if (mMode == MODE_BUS) {
        startTransition();
        setTransition(14);
        setPreset(38);
    }
}

// 8018B994
void dCamera_c::startBusReturnView() {
    if (mMode == MODE_BUS) {
        startTransition();
        setTransition(15);
        setPreset(36);
    }
}
