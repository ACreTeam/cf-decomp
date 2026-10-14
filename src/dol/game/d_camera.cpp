// Game camera: follow, talk and scene-specific views. Names are inferred unless supplied by RTTI.
#include <game/game/d_camera.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_search_cand_base.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_world.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/sLib/s_lib.hpp>
#include <cmath>

dCamera_c *lbl_8074E9B0;

namespace dCamera {

class HostIO_c {
public:
    HostIO_c() {
        mFovy = 22.5f;
        mSecondaryFovy = 30.0f;
        mNear = 1.0f;
        mFar = 2000.0f;
        mFollowRadius = 16.0f;
        mSideMargin = 64.0f;
        mTalkCullOffset = -16.0f;
        mLookUpDelay = 180;
    }
    f32 mFovy, mSecondaryFovy, mNear, mFar;
    f32 mFollowRadius, mSideMargin, mTalkCullOffset;
    s16 mLookUpDelay;
};

static HostIO_c l_hostIO;
static f32 l_viewDistance;
static bool l_titleNpcWrapped;

static const Param_c l_presets[] = {
    {-9466, 0, 332.0f, {0.0f, 0.0f, 2e+01f}},
    {-7918, 0, 2e+02f, {0.0f, 0.0f, 0.0f}},
    {-5461, -3586, 2.7e+02f, {0.0f, 0.0f, 0.0f}},
    {-5461, 0, 2.7e+02f, {0.0f, 0.0f, 0.0f}},
    {-5461, 3586, 2.7e+02f, {0.0f, 0.0f, 0.0f}},
    {-6371, -3586, 3.5e+02f, {0.0f, 0.0f, 0.0f}},
    {-6371, 0, 3.5e+02f, {0.0f, 0.0f, 0.0f}},
    {-6371, 3586, 3.5e+02f, {0.0f, 0.0f, 0.0f}},
    {-12288, -3586, 4.5e+02f, {0.0f, 0.0f, 0.0f}},
    {-12288, 0, 4.5e+02f, {0.0f, 0.0f, 0.0f}},
    {-12288, 3586, 4.5e+02f, {0.0f, 0.0f, 0.0f}},
    {-2730, -3586, 3.7e+02f, {0.0f, 16.0f, -24.0f}},
    {-2730, 0, 3.7e+02f, {0.0f, 16.0f, -24.0f}},
    {-2730, 3586, 3.7e+02f, {0.0f, 16.0f, -24.0f}},
    {-7054, -3586, 4.8e+02f, {0.0f, -8.0f, 12.0f}},
    {-7054, 0, 4.8e+02f, {0.0f, -8.0f, 12.0f}},
    {-7054, 3586, 4.8e+02f, {0.0f, -8.0f, 12.0f}},
    {-6371, 0, 2.8e+02f, {0.0f, -3e+01f, 15.0f}},
    {-3185, 0, 3.3e+02f, {0.0f, -30.5f, 15.0f}},
    {-8192, 0, 2.4e+02f, {0.0f, 4.0f, 15.0f}},
    {-4004, 0, 3.7e+02f, {0.0f, 24.0f, -64.0f}},
    {-5461, 0, 2e+02f, {0.0f, 0.0f, -16.0f}},
    {-7509, 0, 248.0f, {0.0f, 0.0f, 24.0f}},
    {-4096, 0, 264.0f, {0.0f, -4.0f, 16.0f}},
    {-6826, 0, 2.8e+02f, {256.0f, 64.0f, 494.0f}},
    {-6826, 0, 2.8e+02f, {128.0f, 64.0f, 448.0f}},
    {-6826, 0, 2.8e+02f, {128.0f, 64.0f, 3.2e+02f}},
    {-6826, 0, 2.8e+02f, {128.0f, 128.0f, 3.2e+02f}},
    {-6826, 0, 2.8e+02f, {1.6e+02f, 64.0f, 256.0f}},
    {-2730, 0, 2.4e+02f, {256.0f, 64.0f, 448.0f}},
    {-2730, 0, 2.4e+02f, {128.0f, 64.0f, 448.0f}},
    {-2730, 0, 2.4e+02f, {128.0f, 64.0f, 3.2e+02f}},
    {-2730, 0, 2.4e+02f, {128.0f, 128.0f, 3.2e+02f}},
    {-2730, 0, 2.8e+02f, {1.6e+02f, 64.0f, 288.0f}},
    {-2730, 0, 179.0f, {256.0f, 26.0f, 494.0f}},
    {-8192, 0, 2.4e+02f, {0.0f, -4.0f, 15.0f}},
    {0, 1365, 32.0f, {292.0f, 4e+01f, 4.4e+02f}},
    {-1137, 17294, 4e+01f, {3.3e+02f, 34.0f, 4.2e+02f}},
    {-1137, 5461, 24.0f, {304.0f, 4e+01f, 4.4e+02f}},
    {-6371, 0, 3.5e+02f, {256.0f, 2e+01f, 288.0f}},
    {-1365, 0, 3.5e+02f, {256.0f, 24.0f, 256.0f}},
    {-6371, 0, 3.5e+02f, {256.0f, 2e+01f, 3.9e+02f}},
    {-3640, 0, 3.5e+02f, {256.0f, 2e+01f, 256.0f}},
    {-2548, 6571, 2.2e+02f, {35.0f, 5.0f, 15.0f}},
    {-7918, 0, 2e+02f, {0.0f, 0.0f, 0.0f}},
    {-4278, 0, 4e+02f, {304.0f, 3e+01f, 3.7e+02f}},
    {-546, 0, 1.5e+02f, {254.0f, 42.0f, 295.0f}},
    {-4004, 6007, 196.0f, {342.0f, 112.0f, 478.0f}},
    {-6371, 0, 2.8e+02f, {-6e+01f, -3e+01f, 15.0f}},
    {-6371, 0, 2.8e+02f, {-8.0f, -3e+01f, 15.0f}},
    {-5097, 0, 2.4e+02f, {0.0f, -4.0f, 15.0f}},
    {-8192, 0, 346.0f, {0.0f, 0.0f, 1e+01f}},
    {-4096, 0, 384.0f, {0.0f, 28.0f, -88.0f}},
    {-8192, -3586, 3.5e+02f, {0.0f, 0.0f, 0.0f}},
    {-8192, 0, 3.5e+02f, {0.0f, 0.0f, 0.0f}},
    {-8192, 3586, 3.5e+02f, {0.0f, 0.0f, 0.0f}},
    {-6371, 0, 3.5e+02f, {272.0f, 2e+01f, 3.6e+02f}},
    {-6189, 0, 3.9e+02f, {768.0f, 76.0f, 4.8e+02f}},
    {-8192, 0, 346.0f, {768.0f, 48.0f, 794.0f}},
    {-2730, 0, 2.4e+02f, {0.0f, -8.0f, 16.0f}},
    {-9466, 0, 332.0f, {0.0f, 0.0f, -24.0f}},
};

static const Transition_c l_transitions[] = {
    {0.0f, 32.0f, 8.0f, 16.0f},
    {0.0f, 3e+01f, 5.0f, 2e+01f},
    {0.0f, 1.3e+02f, 1e+01f, 5e+01f},
    {0.0f, 42.0f, 4.0f, 2e+01f},
    {0.0f, 1.3e+02f, 1e+01f, 5e+01f},
    {0.0f, 1.3e+02f, 2e+01f, 9e+01f},
    {0.0f, 1.5e+02f, 3e+01f, 1.2e+02f},
    {0.0f, 3e+01f, 5.0f, 2e+01f},
    {0.0f, 1.8e+02f, 6e+01f, 9e+01f},
    {0.0f, 6e+01f, 2e+01f, 3e+01f},
    {0.0f, 9e+02f, 6e+01f, 1.2e+02f},
    {0.0f, 1.8e+02f, 0.0f, 1.2e+02f},
    {0.0f, 9e+01f, 2e+01f, 7e+01f},
    {0.0f, 105.0f, 4e+01f, 65.0f},
    {0.0f, 48.0f, 18.0f, 3e+01f},
    {0.0f, 9e+01f, 35.0f, 55.0f},
    {0.0f, 45.0f, 12.0f, 2e+01f},
    {0.0f, 5e+01f, 12.0f, 2e+01f},
    {0.0f, 1.3e+02f, 2e+01f, 9e+01f},
    {0.0f, 28.0f, 12.0f, 16.0f},
    {0.0f, 32.0f, 8.0f, 16.0f},
    {6e+01f, 3.6e+02f, 1.8e+02f, 1.2e+02f},
};

struct TitleBuilding_c { u32 mBuilding; Vec mOffset; };
static const TitleBuilding_c l_titleBuildings[] = {
    {1, {0.0f, 0.0f, 16.0f}},
    {21, {0.0f, 0.0f, 16.0f}},
    {23, {16.0f, 0.0f, 16.0f}},
    {19, {16.0f, 0.0f, 16.0f}},
    {20, {32.0f, 0.0f, 16.0f}},
};

struct ModeProc_c { void (dCamera_c::*mInit)(); void (dCamera_c::*mExec)(); };
static const ModeProc_c l_modes[] = {
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

static const u8 l_roomPresets[8][3] = {
    {2, 3, 4},
    {5, 6, 7},
    {8, 9, 10},
    {11, 12, 13},
    {53, 54, 55},
    {14, 15, 16},
    {0, 0, 0},
    {0, 0, 0},
};

struct BusMotion_c { u16 mFrames, mRandomFrames; s16 mSpeed; f32 mAmplitude; };
static const BusMotion_c l_busMotions[] = {
    {0, 60, 0, 0.0f},
    {30, 3, 2730, 0.01f},
    {30, 3, 2730, 0.024f},
    {30, 3, 2730, 0.036f},
};

static CafeShot_c l_cafeShots[] = {
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
    {{256.0f, 24.0f, 412.0f}, 0.0f, 0.0f, 3.2e+02f, -0.025f, -4096, -28215, 4},
    {{392.0f, 32.0f, 304.0f}, 16.0f, 0.0f, 128.0f, 0.025f, -4096, -10921, 6},
    {{384.0f, 32.0f, 304.0f}, -24.0f, 0.0f, 256.0f, 0.025f, -4096, -13652, -4},
};

} // namespace dCamera

mVec3_c lbl_80623FEC;
mVec3_c lbl_80624004;

using namespace dCamera;

// 801870D0
const dCamera::CafeShot_c *dCamera::getCafeShot(int index) {
    return &l_cafeShots[index];
}

// 801870E4
CUSTOM_BASE_PROFILE(CAMERA, dCamera_c, 0xA7, 0xE4);

namespace dCamera {
static long l_titleNpcIndex = -1;
static int l_titleNpcRemaining = 10;
}

// 80187180
int dCamera_c::create() {
    lbl_8074E9B0 = this;
    dScene::setSceneE848((u32)this);
    mFrustum.SetFovY(l_hostIO.mFovy);
    mFrustum.mNearZ = l_hostIO.mNear;
    mFrustum.mFarZ = l_hostIO.mFar;
    mSecondaryFovy = l_hostIO.mSecondaryFovy;
    switch (dScene::getCurrentScene()) {
    case dScene::SCENE_DM_TITLE:
        if (dNpcMng_c::sInstance == NULL) return 0;
        _1A4 = 4;
        break;
    case dScene::SCENE_DM_PL_SEL:
        _1A4 = 12;
        break;
    case dScene::SCENE_DM_BUS_PL_CRT:
    case dScene::SCENE_DM_BUS_TO_TOWN:
    case dScene::SCENE_DM_BUS_TO_LAND:
        _1A4 = 14;
        break;
    case dScene::SCENE_DM_CHKP_CNNCT:
    case dScene::SCENE_DM_CHKP_DCNNCT:
    case dScene::SCENE_DM_CHKP_MISS:
        _1A4 = 15;
        break;
    case dScene::SCENE_TOWN: {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
        if (player != NULL && !player->isFlag0(0x70)) _1A4 = 27;
        else _1A4 = 0;
        break;
    }
    default:
        _1A4 = 0;
        break;
    }
    mPendingTransition = 22;
    _1A8 = _1A4;
    initBounds();
    changeMode();
    finishTransition();
    updateTransition();
    updateView();
    execute();
    draw();
    mMtx_c identity;
    PSMTXIdentity(identity);
    m3d::setCurrentCamera(1);
    nw4r::g3d::Camera camera = m3d::getCamera(1);
    camera.SetCameraMtxDirectly(identity);
    EGG::Screen screen;
    screen.SetFovY(mSecondaryFovy);
    screen.mNearZ = l_hostIO.mNear;
    screen.mFarZ = l_hostIO.mFar;
    screen.CopyToG3D(camera);
    return 1;
}

// 801873C0
int dCamera_c::doDelete() {
    lbl_8074E9B0 = NULL;
    dScene::setSceneE848(0);
    return 1;
}

// 801873F0
int dCamera_c::execute() {
    changeMode();
    executeMode();
    nw4r::g3d::Camera camera = m3d::getCurrentCamera();
    mCamera.updateMatrix();
    static_cast<nw4r::math::MTX34 &>(mViewMtx) = mCamera.getViewMatrix();
    PSMTXInverse(mViewMtx, mViewMtx);
    mCamera.CopyToG3D(camera);
    mCullFrustum.set(l_hostIO.mFovy, mFrustum.mSize.x / mFrustum.mSize.y,
                    l_hostIO.mNear, l_hostIO.mFar,
                    static_cast<const mMtx_c &>(mCamera.getViewMatrix()));
    return 1;
}

// 80187510
int dCamera_c::draw() {
    nw4r::g3d::Camera camera = m3d::getCurrentCamera();
    mFrustum.SetFovY(l_hostIO.mFovy);
    mFrustum.mNearZ = l_hostIO.mNear;
    mFrustum.mFarZ = l_hostIO.mFar;
    mFrustum.CopyToG3D(camera);
    updateSecondaryProjection();
    return 1;
}

// 801875CC
void dCamera_c::changeMode() {
    if (_1A8 != 29) {
        if (mMode != _1A8) mPreviousMode = mMode;
        mMode = _1A8;
        if (l_modes[_1A8].mInit) (this->*l_modes[_1A8].mInit)();
        _1A8 = 29;
        if (mPendingTransition != 22) {
            setTransition(mPendingTransition);
            mPendingTransition = 22;
            startTransition();
        }
    }
}

// 80187680
void dCamera_c::executeMode() {
    if (l_modes[mMode].mExec) (this->*l_modes[mMode].mExec)();
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
        mVec3_c first, last;
        dFdBase_c::getUnitPos(&first, 1, 1, 0, 0);
        dFdBase_c::getUnitPos(&last, 6, 6, 15, 15);
        mMinX = first.x;
        mMaxX = mMinX + 2560.0f;
    } else {
        mMinX = -3.402823466e38f;
        mMaxX = 3.402823466e38f;
    }
}

// 801877E4
int dCamera_c::getHorizontalEdge(const mVec3_c *target) {
    if (target->x < mMinX + 80.0f) return 1;
    if (target->x > mMaxX - 80.0f) return 2;
    return 0;
}

// 80187824
BOOL dCamera_c::clampTarget(mVec3_c *target) {
    BOOL changed = FALSE;
    if (mMaxX - mMinX <= 160.0f) {
        target->x = (mMaxX + mMinX) * 0.5f;
        changed = TRUE;
    } else {
        if (target->x < mMinX + 80.0f) {
            target->x = mMinX + 80.0f;
            changed = TRUE;
        } else if (target->x > mMaxX - 80.0f) {
            target->x = mMaxX - 80.0f;
            changed = TRUE;
        }
    }
    if (mMaxZ - mMinZ <= 112.0f) {
        target->z = (mMaxZ + mMinZ) * 0.5f;
        changed = TRUE;
    } else {
        if (target->z < mMinZ + 32.0f) {
            target->z = mMinZ + 32.0f;
            changed = TRUE;
        } else if (target->z > mMaxZ - 80.0f) {
            target->z = mMaxZ - 80.0f;
            changed = TRUE;
        }
    }
    return changed;
}

// 80187904
void dCamera_c::followTarget(const mVec3_c *target) {
    mVec3_c delta = mDesired.mTarget - *target;
    f32 distance = PSVECMag(delta);
    if (distance > l_hostIO.mFollowRadius) {
        f32 step = distance - l_hostIO.mFollowRadius;
        if (delta.normalizeRS()) {
            mDesired.mTarget.x -= delta.x * step;
            mDesired.mTarget.y -= delta.y * step;
            mDesired.mTarget.z -= delta.z * step;
        }
    }
}

// 801879B8
void dCamera_c::followClampedTarget(const mVec3_c *target) {
    mVec3_c position = *target;
    if (mMaxX - mMinX <= l_hostIO.mSideMargin * 2.0f) {
        position.x = (mMaxX + mMinX) * 0.5f;
    } else {
        if (position.x < mMinX + l_hostIO.mSideMargin) position.x = mMinX + l_hostIO.mSideMargin;
        else if (position.x > mMaxX - l_hostIO.mSideMargin) position.x = mMaxX - l_hostIO.mSideMargin;
    }
    followTarget(&position);
}

// 80187A58
void dCamera_c::setClampedTarget(const mVec3_c *target) {
    mVec3_c position = *target;
    if (mMaxX - mMinX <= l_hostIO.mSideMargin * 2.0f) {
        position.x = (mMaxX + mMinX) * 0.5f;
    } else {
        if (position.x < mMinX + l_hostIO.mSideMargin) position.x = mMinX + l_hostIO.mSideMargin;
        else if (position.x > mMaxX - l_hostIO.mSideMargin) position.x = mMaxX - l_hostIO.mSideMargin;
    }
    mDesired.mTarget = position;
}

// 80187AF8
BOOL dCamera::isMuseumExhibit(u8 scene) {
    return (u8)(scene - dScene::SCENE_RM_MM_FISH) <= 5;
}

// 80187B1C
void dCamera_c::initFollow() {
    if (!dWorld::isCurve()) {
        u8 scene = dScene::getCurrentScene();
        int row = mRoomPitch;
        if (isMuseumExhibit(scene)) {
            if (row == 0) row = 3;
            else if (row == 2) row = 5;
            else if (row == 1 && scene == dScene::SCENE_RM_MM_FOSSIL1) row = 4;
        }
        setPreset(l_roomPresets[row][mRoomYaw]);
        setTransition(0);
    } else {
        if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN)) setPreset(0);
        else if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM)) setPreset(6);
        else setPreset(51);
        mLookUpTimer = l_hostIO.mLookUpDelay;
        setTransition(0);
    }
    if (mLifecycleState == CREATING) {
        if (dScene::getCurrentScene() == dScene::SCENE_DM_LOAD || dScene::getCurrentScene() == dScene::SCENE_DM_SAVE) {
            mDesired.mTarget.set(0.0f, 0.0f, 0.0f);
        } else {
            mDesired.mTarget = *dScene::getScenePlayerPos();
            dBGCF::groundChk_c ground(&mDesired.mTarget, 0, 0, 0);
            mDesired.mTarget.y = ground.getHeight(FALSE) + 20.0f;
        }
        finishTransition();
    } else {
        startTransition();
    }
}

// 80187D00
void dCamera_c::execFollow() {
    mVec3_c target = mDesired.mTarget;
    mVec3_c *position = dPlayerMgr::getCurrentSoundPositionRaw();
    if (position != NULL) {
        target = *position;
        target.y += 20.0f;
    }
    u8 scene = dScene::getCurrentScene();
    if (!dWorld::isCurve()) {
        mDesired.mTarget = target;
        if (!dScene::isSceneAttr(scene, dScene::SCENE_ATTR_FTR_ROOM)) clampTarget(&mDesired.mTarget);
        else {
            int row = mRoomPitch;
            if (isMuseumExhibit(scene)) {
                if (row == 0) row = 3;
                else if (row == 2) row = 5;
                else if (row == 1 && scene == dScene::SCENE_RM_MM_FOSSIL1) row = 4;
            }
            if (row != 0) clampTarget(&mDesired.mTarget);
        }
    } else {
        if (dPlayerMgr::isIdle(4) || dPlayerMgr::isWaitingForDemo(4)) {
            if (!dScene::isCityScene(dScene::getCurrentScene()) && mLookUpTimer == 0 && mPreset != 20) {
                startTransition();
                setPreset(20);
                setTransition(6);
            }
        } else {
            mLookUpTimer = l_hostIO.mLookUpDelay;
            if (!dPlayerMgr::isStartingDemo(4) && mPreset == 20) {
                startTransition();
                setPreset(0);
                setTransition(7);
            }
        }
        followClampedTarget(&target);
    }
    updateTransition();
    updateView();
}

// 80187F0C
void dCamera_c::initPlayerView1() {
    startTransition();
    if (dWorld::isCurve()) setPreset(17);
    else setPreset(18);
    setTransition(1);
}

// 80187F6C
void dCamera_c::execPlayerView1() {
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
    }
    mDesired.mTarget = target;
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
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
    }
    mDesired.mTarget = target;
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
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
    }
    mDesired.mTarget = target;
    updateTransition();
    updateView();
}

// 80188198
void dCamera_c::initTitleSelect() {
    finishTransition();
    setPreset(0);
    setTransition(0);
}

// 801881DC
void dCamera_c::execTitleSelect() {
    dAnimalBlock_c *animals = dSaveData_c::getAnimalBlock();
    if (l_titleNpcIndex == -1) l_titleNpcIndex = cM::rndInt(10);
    dAcNpc_c *npc = NULL;
    long index = l_titleNpcIndex;
    if (!l_titleNpcWrapped) {
        for (; npc == NULL && l_titleNpcRemaining > 0; l_titleNpcRemaining--) {
            dAnimal_c *animal = animals->getAnimalConst(index);
            if (animal != NULL && animal->mID.isValid() && animal->mPlace == 0) {
                dItem::Item key = animals->getAnimalKey(&animal->mID);
                npc = static_cast<dAcNpc_c *>(dNpcList::getNmlByKey(key));
                if (npc != NULL) l_titleNpcIndex = index;
            }
            if (sLib::chase(&index, 10, 1)) index = 0;
        }
    }
    if (npc != NULL) {
        mWork.mNpc = npc;
        _1A4 = 5;
        if (sLib::chase(&l_titleNpcIndex, 10, 1)) {
            l_titleNpcIndex = 0;
            l_titleNpcWrapped = true;
        }
    } else {
        _1A4 = 6;
        l_titleNpcWrapped = false;
        l_titleNpcRemaining = 10;
    }
    _1A8 = _1A4;
    changeMode();
    executeMode();
}

// 8018835C
void dCamera_c::initTitleNpc() {
    setPreset(60);
    setTransition(0);
    mVec3_c target(0.0f, 0.0f, 0.0f);
    if (mWork.mNpc != NULL) {
        target = *mWork.mNpc->getPosP();
        dBGCF::groundChk_c ground(&target, 0, 0, 0);
        target.y = ground.getHeight(FALSE) + 20.0f;
    }
    mDesired.mTarget = target;
    finishTransition();
}

// 8018841C
void dCamera_c::execTitleNpc() {
    if (mWork.mNpc != NULL && mWork.mNpc->mIsActive) {
        mVec3_c target = *mWork.mNpc->getPosP();
        dBGCF::groundChk_c ground(&target, 0, 0, 0);
        target.y = ground.getHeight(FALSE) + 20.0f;
        setClampedTarget(&target);
    }
    updateTransition();
    updateView();
}

// 801884D4
void dCamera_c::initTitleBuilding() {
    mWork.mTitle.mIndex = 0;
    u32 building = l_titleBuildings[0].mBuilding;
    dItem::Item item((u16)(building < 69 ? building + 0xD000 : 0xD000));
    int x, z;
    if (dSaveData_c::getRaw()->mBuilding.mList.getPos(&x, &z, &item, 1)) {
        mVec3_c position;
        position.x = x << 5;
        position.z = z << 5;
        position.y = dBGCF::getUnitY(&position) + 16.0f;
        mWork.mTitle.mPosition = *static_cast<const Vec *>(position);
        mVec3_c offset(l_titleBuildings[mWork.mTitle.mIndex].mOffset);
        mWork.mTitle.mSpeed = 0.0f;
        mWork.mTitle.mTimer = 180;
        mDesired.mTarget = position;
        mDesired.mTarget += offset;
    }
    setPreset(0);
}

// 8018863C
void dCamera_c::execTitleBuilding() {
    mVec3_c target(mWork.mTitle.mPosition);
    mVec3_c offset(l_titleBuildings[mWork.mTitle.mIndex].mOffset);
    target += offset;
    sLib::chase(&mWork.mTitle.mSpeed, 1.0f, 0.0012f);
    cLib::addCalcPos(&mDesired.mTarget, target, 0.03125f, mWork.mTitle.mSpeed, 0.002f);
    if (mDesired.mTarget.distTo(target) < 0.002f && sLib::calcTimer(&mWork.mTitle.mTimer) == 0) {
        int x, z;
        do {
            if (sLib::chase(&mWork.mTitle.mIndex, 5, 1)) mWork.mTitle.mIndex = 0;
            u32 building = l_titleBuildings[mWork.mTitle.mIndex].mBuilding;
            dItem::Item item((u16)(building < 69 ? building + 0xD000 : 0xD000));
            if (dSaveData_c::getRaw()->mBuilding.mList.getPos(&x, &z, &item, 1)) break;
        } while (true);
        mVec3_c position;
        position.x = x << 5;
        position.z = z << 5;
        mWork.mTitle.mPosition = *static_cast<const Vec *>(position);
        mWork.mTitle.mSpeed = 0.0f;
        mWork.mTitle.mTimer = 180;
        mWork.mTitle.mPosition.y = dBGCF::getUnitY(&position) + 16.0f;
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
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
        player = target;
        mVec3_c *position = dPlayerMgr::getToolOrPlayerPosition(4);
        if (position != NULL) {
            target.y += (position->y - player.y) * 0.5f * 0.75f;
            target.z += (position->z - player.z) * 0.75f;
            target.x += (position->x - player.x) * 0.75f;
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
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
    }
    mDesired.mTarget = target;
    updateTransition();
    updateView();
}

// 80188A54
void dCamera_c::initTalk() {
    startTransition();
    if (!dWorld::isCurve()) {
        mTalkSide = 3;
        setPreset(23);
    } else {
        setPreset(22);
    }
    setTransition(0);
}

// 80188ABC
void dCamera_c::execTalk() {
    if (dWorld::isCurve()) updateOutdoorTalk();
    else updateIndoorTalk();
    updateTransition();
    updateView();
}

// 80188B14
int dCamera::getShopIndex() {
    int index = 0;
    switch (dScene::getCurrentScene()) {
    case dScene::SCENE_RM_SHOP0: index = 0; break;
    case dScene::SCENE_RM_SHOP1: index = 1; break;
    case dScene::SCENE_RM_SHOP2: index = 2; break;
    case dScene::SCENE_RM_SHOP3_F2: index = 3; break;
    case dScene::SCENE_RM_GRACE: index = 4; break;
    }
    return index;
}

// 80188B90
void dCamera_c::initShopWide() {
    int index = getShopIndex();
    startTransition();
    setPreset(index + 24);
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
    int index = getShopIndex();
    startTransition();
    setPreset(index + 29);
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
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
    }
    mDesired.mTarget = target;
    updateTransition();
    updateView();
}

// 80188DEC
void dCamera_c::setBusBounce(u8 kind) {
    mWork.mBus.mBounceKind = kind;
    if (kind > 3) mWork.mBus.mBounceKind = 0;
    const BusMotion_c *motion = &l_busMotions[mWork.mBus.mBounceKind];
    mWork.mBus.mBounceTimer = motion->mFrames;
    mWork.mBus.mBounceTimer += cM::rndInt(motion->mRandomFrames);
    mWork.mBus.mBounceAngle = 0;
}

// 80188E64
void dCamera_c::setBusRoll(u8 kind) {
    mWork.mBus.mRollKind = kind;
    if (kind > 3) mWork.mBus.mRollKind = 0;
    const BusMotion_c *motion = &l_busMotions[mWork.mBus.mRollKind];
    mWork.mBus.mRollTimer = motion->mFrames;
    mWork.mBus.mRollTimer += cM::rndInt(motion->mRandomFrames);
    mWork.mBus.mRollAngle = 0;
}

// 80188EDC
f32 dCamera_c::updateBusMotion() {
    if (sLib::calcTimer(&mWork.mBus.mBounceTimer) == 0 && ::abs(mWork.mBus.mBounceAngle) < 256) {
        if (mWork.mBus.mBounceKind == 0) {
            if (cM::rndInt(100) < 80) mWork.mBus.mBounceKind = 1;
            else mWork.mBus.mBounceKind = 2;
        } else if (mWork.mBus.mBounceKind == 2) mWork.mBus.mBounceKind = 1;
        else mWork.mBus.mBounceKind = 0;
        setBusBounce(mWork.mBus.mBounceKind);
    }
    if (sLib::calcTimer(&mWork.mBus.mRollTimer) == 0 && ::abs(mWork.mBus.mRollAngle) < 256) {
        if (mWork.mBus.mRollKind == 0) mWork.mBus.mRollKind = 1;
        else if (mWork.mBus.mRollKind == 2) mWork.mBus.mRollKind = 1;
        else mWork.mBus.mRollKind = 0;
        setBusRoll(mWork.mBus.mRollKind);
    }
    if (l_busMotions[mWork.mBus.mRollKind].mSpeed != 0) {
        mWork.mBus.mRollAngle += l_busMotions[mWork.mBus.mRollKind].mSpeed;
        mBusRoll = l_busMotions[mWork.mBus.mRollKind].mAmplitude * mAng(mWork.mBus.mRollAngle).sin();
    }
    f32 offset;
    if (l_busMotions[mWork.mBus.mBounceKind].mSpeed != 0) {
        mWork.mBus.mBounceAngle += l_busMotions[mWork.mBus.mBounceKind].mSpeed;
        offset = l_busMotions[mWork.mBus.mBounceKind].mAmplitude * mAng(mWork.mBus.mBounceAngle).sin() * 16.0f;
    } else offset = 0.0f;
    return offset;
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
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    mVec3_c player;
    if (actor != NULL) player = *actor->getPosP();
    else player = mTalkTarget;
    mDesired.mTarget = player + mTalkTarget;
    mDesired.mTarget *= 0.5f;
    int depth = std::fabs((player.z - mTalkTarget.z) * 0.5f);
    mDesired.mTarget.z += depth;
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
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
    }
    mDesired.mTarget = target;
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
    if (dBgm::l_mgr.mStgCafe.isKKPlaying()) {
        switch (mWork.mCafe.mState) {
        case 0:
            startCafeIntro();
            break;
        case 1:
        case 2:
            if (mWork.mCafe.mPhase != dBgm::l_mgr.mStgCafe.getKKInfo4()) {
                mWork.mCafe.mPhase = dBgm::l_mgr.mStgCafe.getKKInfo4();
                if (mWork.mCafe.mPhase == 2) startCafeOutro();
                else setCafeShot(-1);
            }
            break;
        case 3:
            break;
        }
    }
    if (mWork.mCafe.mState == 2) {
        const CafeShot_c *shot = getCafeShot(mWork.mCafe.mShot);
        mDesired.mParam.mOffset.set(shot->mOffset.x, shot->mOffset.y, shot->mOffset.z);
        if (mDesired.mParam.mDistance > 20.0f) mDesired.mParam.mDistance += shot->mDistanceStep;
        mDesired.mParam.mYaw += shot->mYawStep;
        mWork.mCafe.mOrbitAngle += shot->mYawStep;
        mVec3_c orbit(shot->mOrbitX, 0.0f, shot->mOrbitZ);
        mAng angle = mWork.mCafe.mOrbitAngle;
        mDesired.mParam.mOffset.x += orbit.x;
        mDesired.mParam.mOffset.y += orbit.y;
        mDesired.mParam.mOffset.z += orbit.z;
        orbit.rotY(angle);
        mDesired.mParam.mOffset.x -= orbit.x;
        mDesired.mParam.mOffset.y -= orbit.y;
        mDesired.mParam.mOffset.z -= orbit.z;
    }
    updateTransition();
    updateView();
}

// 80189690
void dCamera_c::startCafeIntro() {
    if (mMode == 19) {
        mDesired.mTarget = mVec3_c::Zero;
        mWork.mCafe.mState = 1;
        startTransition();
        setPreset(46);
        setTransition(10);
    }
}

// 80189708
void dCamera_c::setCafeShot(int index) {
    if (mMode == 19) {
        mWork.mCafe.mOrbitAngle = 0;
        if (index < 0) {
            dSearchCand_c<14> candidates;
            candidates.setAll();
            for (int i = 0; i < 2; i++) {
                if (mWork.mCafe.mHistory[i] != -1) candidates.remove(mWork.mCafe.mHistory[i]);
            }
            index = candidates.getRandom();
            mWork.mCafe.mHistory[mWork.mCafe.mHistoryIndex] = index;
            mWork.mCafe.mHistoryIndex ^= 1;
        }
        mWork.mCafe.mShot = index;
        mWork.mCafe.mState = 2;
        const CafeShot_c *shot = getCafeShot(mWork.mCafe.mShot);
        mDesired.mTarget = mVec3_c::Zero;
        mDesired.mParam.mOffset.set(shot->mOffset.x, shot->mOffset.y, shot->mOffset.z);
        mDesired.mParam.mYaw = shot->mYaw;
        mDesired.mParam.mPitch = shot->mPitch;
        mDesired.mParam.mDistance = shot->mDistance;
        finishTransition();
    }
}

// 8018984C
void dCamera_c::startCafeOutro() {
    if (mMode == 19) {
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
    if (dWorld::isCurve()) setPreset(17);
    else setPreset(18);
    setTransition(1);
}

// 801899C8
void dCamera_c::execPlayerView21() {
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
    }
    mDesired.mTarget = target;
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
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
        player = target;
        mVec3_c position;
        if (dPlayerMgr::getScriptedActionPosition(&position)) {
            mVec3_c delta = position - player;
            delta.x *= 0.75f;
            delta.y = delta.y * 0.5f * 0.75f;
            delta.z *= 0.75f;
            target += delta;
        } else target = mDesired.mTarget;
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
    mVec3_c target = mDesired.mTarget;
    mVec3_c *position = dPlayerMgr::getCurrentSoundPositionRaw();
    if (position != NULL) {
        target = *position;
        target.y += 20.0f;
    }
    followClampedTarget(&target);
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
    mVec3_c target = mDesired.mTarget;
    mVec3_c *position = dPlayerMgr::getCurrentSoundPositionRaw();
    if (position != NULL) {
        target = *position;
        target.y += 20.0f;
    }
    mDesired.mTarget = target;
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
    mVec3_c target(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        target = *actor->getPosP();
        target.y += 20.0f;
    }
    mDesired.mTarget = target;
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
    if (mPreset == 57 && mFader_c::mFader->getStatus() == mFaderBase_c::HIDDEN) {
        _1A4 = 0;
        _1A8 = 0;
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
    mVec3_c target = mDesired.mTarget;
    mVec3_c *position = dPlayerMgr::getCurrentSoundPositionRaw();
    if (position != NULL) {
        target = *position;
        target.y += 20.0f;
    }
    followClampedTarget(&target);
    updateTransition();
    updateView();
}

// 8018A080
mVec3_c dCamera_c::screenToWorld(const mVec2_c *screen) {
    mMtx_c inverse = mViewMtx;
    f32 height = mFrustum.mNearZ * 2.0f * (1.0f / mFrustum.mTanFovY);
    f32 width = (mFrustum.mSize.x / mFrustum.mSize.y) * height;
    mVec3_c local(screen->x * width * 0.5f, -screen->y * height * 0.5f, -mFrustum.mNearZ);
    mVec3_c world;
    PSMTXMultVec(inverse, local, world);
    return world;
}

// 8018A16C
void dCamera::worldToScreen(const dCamera_c *cam, f32 *x, f32 *y, const mVec3_c *pos) {
    mVec3_c view;
    PSMTXMultVec(const_cast<EGG::LookAtCamera &>(cam->mCamera).getViewMatrix(), *pos, view);
    nw4r::math::MTX44 projection;
    lbl_8074E9B0->getProjectionMtx(&projection);
    f32 clipW = projection._33 + (projection._32 * view.z +
                                 (projection._30 * view.x + projection._31 * view.y));
    f32 scale = 1.0f / (clipW * 2.0f);
    f32 clipX = projection._03 + (projection._02 * view.z +
                                 (projection._00 * view.x + projection._01 * view.y));
    f32 clipY = projection._13 + (projection._12 * view.z +
                                 (projection._10 * view.x + projection._11 * view.y));
    m3d::getCurrentCamera();
    *x = clipX * scale + 0.5f;
    *y = 0.5f - clipY * scale;
}

// 8018A2D8
f32 dCamera_c::calcTransitionWeight(f32 frame, f32 start, f32 end, f32 easeIn, f32 easeOut) {
    if (frame >= end) return 1.0f;
    if (frame <= start) return 0.0f;
    f32 duration = end - start;
    f32 elapsed = frame - start;
    if (duration < easeIn + easeOut) return 0.0f;
    f32 weight = 0.0f;
    f32 speed = 1.0f / (duration * 2.0f - easeIn - easeOut);
    if (easeIn != 0.0f) {
        if (elapsed <= easeIn) {
            weight = speed * (elapsed * elapsed) / easeIn;
            return weight > 1.0f ? 1.0f : weight;
        }
        weight = speed * easeIn;
    }
    if (elapsed <= duration - easeOut) {
        weight += speed * (elapsed - easeIn) * 2.0f;
        return weight > 1.0f ? 1.0f : weight;
    }
    weight += speed * (duration - easeIn - easeOut) * 2.0f;
    if (easeOut != 0.0f) {
        weight += speed * easeOut;
        if (elapsed < duration) weight -= speed * ((duration - elapsed) * (duration - elapsed)) / easeOut;
    }
    return weight > 1.0f ? 1.0f : weight;
}

// 8018A400
void dCamera_c::finishTransition() { mTransitionFrame = mTransition.mEnd; }

// 8018A40C
void dCamera_c::startTransition() {
    mTransitionFrame = 0.0f;
    mPrevious = mCurrent;
}

// 8018A460
f32 dCamera_c::getTransitionFramesLeft() const { return mTransition.mEnd - mTransitionFrame; }

// 8018A470
void dCamera_c::updateTransition() {
    u32 cull = usesForegroundCulling();
    if (isTransitionActive()) {
        f32 weight = calcTransitionWeight(mTransitionFrame, mTransition.mStart, mTransition.mEnd,
                                         mTransition.mEaseIn, mTransition.mEaseOut);
        mVec3_c delta = mDesired.mTarget - mPrevious.mTarget;
        delta *= weight;
        mCurrent.mTarget = mPrevious.mTarget + delta;
        mVec3_c offset = mDesired.mParam.mOffset - mPrevious.mParam.mOffset;
        offset *= weight;
        mCurrent.mParam.mOffset = mPrevious.mParam.mOffset + offset;
        mCurrent.mParam.mPitch = mPrevious.mParam.mPitch + (mDesired.mParam.mPitch - mPrevious.mParam.mPitch) * weight;
        mCurrent.mParam.mYaw = mPrevious.mParam.mYaw + (mDesired.mParam.mYaw - mPrevious.mParam.mYaw) * weight;
        mCurrent.mParam.mDistance = mPrevious.mParam.mDistance + (mDesired.mParam.mDistance - mPrevious.mParam.mDistance) * weight;
        mTransitionFrame += 1.0f;
        if (weight > 0.5f && cull != mCullForeground) mCullForeground = cull;
    } else {
        mCurrent = mDesired;
        mPrevious = mDesired;
        if (cull != mCullForeground) mCullForeground = cull;
    }
    if (mCullForeground && cull) {
        if (mMode == 9) mForegroundZ = mDesired.mTarget.z + l_hostIO.mTalkCullOffset;
        else mForegroundZ = mDesired.mTarget.z;
        mTargetZ = mDesired.mTarget.z;
    }
}

// 8018A748
bool dCamera_c::isTransitionActive() const { return mTransitionFrame <= mTransition.mEnd; }

// 8018A764
void dCamera_c::updateView() {
    lbl_80623FEC = mCurrent.mTarget + mCurrent.mParam.mOffset;
    mVec3_c offset(0.0f, 0.0f, mCurrent.mParam.mDistance);
    offset.rotX(mCurrent.mParam.mPitch);
    offset.rotY(mCurrent.mParam.mYaw);
    mEye = lbl_80623FEC + offset;
    mVec3_c eye(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, -1000.0f);
    mVec3_c up = mVec3_c::Ey;
    mAng targetAngle = dWorld::curvePosition(&target, &lbl_80623FEC);
    mCurveAngle = dWorld::curvePosition(&eye, &mEye);
    up.rotX(targetAngle);
    lbl_80624004 = mEye;
    mVec3_c direction = lbl_80623FEC - lbl_80624004;
    l_viewDistance = PSVECMag(direction);
    mCamera.mPos = eye;
    mCamera.mAt = target;
    mCamera.mUp = up;
}

// 8018A938
BOOL dCamera_c::isLookingUp() {
    BOOL result = TRUE;
    bool follow = false;
    if (mMode == 0 && mPreset == 20) follow = true;
    if (!follow && mMode != 28) result = FALSE;
    return result;
}

// 8018A978
BOOL dCamera_c::isFollowLookUp() {
    BOOL result = FALSE;
    if (mMode == 0 && mPreset == 20) result = TRUE;
    return result;
}

// 8018A9A0
BOOL dCamera_c::isControllable() {
    if (mMode != 27 && !isTransitionActive()) return TRUE;
    return FALSE;
}

// 8018A9E0
BOOL dCamera_c::usesForegroundCulling() {
    if (isLookingUp() || mMode == 3 || mMode == 9 || mMode == 8 || mMode == 1 ||
        mMode == 21 || mMode == 18 || mMode == 13 || mMode == 23) return TRUE;
    return FALSE;
}

// 8018AA64
BOOL dCamera_c::isPastForeground(const mVec3_c *position, f32 depth) {
    if (mCullForeground && position->z - depth * 0.5f > mForegroundZ - 32.0f) return TRUE;
    return FALSE;
}

// 8018AAA4
BOOL dCamera_c::isPastTarget(const mVec3_c *position, f32 depth) {
    if (mCullForeground && position->z - depth * 0.5f > mTargetZ - 32.0f) return TRUE;
    return FALSE;
}

// 8018AAE4
f32 dCamera_c::calcTalkDistance(const mVec3_c *a, const mVec3_c *b, mVec3_c *center, f32 *zOffset) {
    mVec3_c difference = *a - *b;
    mVec3_c scaled(difference.x, difference.y, difference.z * 1.5f);
    f32 distance = PSVECMag(scaled);
    if (center != NULL) {
        *center = (*a + *b) * 0.5f;
        if (zOffset != NULL) *zOffset = std::fabs(difference.z * 0.5f);
    }
    return distance;
}

// 8018ABD4
f32 dCamera_c::calcThreeActorTalkDistance(const mVec3_c *a, const mVec3_c *b, const mVec3_c *c, mVec3_c *center, f32 *zOffset) {
    return calcTalkDistance(a, b, center, zOffset);
}

// 8018ABE0
void dCamera_c::updateOutdoorTalk() {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        player.x = actor->getPosP()->x;
        player.y = actor->getPosP()->y + 20.0f;
        player.z = actor->getPosP()->z;
    }
    f32 zOffset;
    f32 distance;
    if (mTalkTargetCount == 0) distance = calcTalkDistance(&player, &mTalkTarget, &mDesired.mTarget, &zOffset);
    else distance = calcThreeActorTalkDistance(&player, &mTalkTarget, &mTalkTarget2, &mDesired.mTarget, &zOffset);
    mDesired.mTarget.z += zOffset;
    if (distance < 72.0f) distance = 72.0f;
    else if (distance > 176.0f) distance = 176.0f;
    blendPresets(22, 0, (distance - 72.0f) / 104.0f);
}

// 8018ACE0
void dCamera_c::updateIndoorTalk() {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        player.x = actor->getPosP()->x;
        player.y = actor->getPosP()->y + 20.0f;
        player.z = actor->getPosP()->z;
    }
    f32 zOffset;
    f32 distance = calcTalkDistance(&player, &mTalkTarget, &mDesired.mTarget, &zOffset);
    if (distance < 72.0f) distance = 72.0f;
    else if (distance > 176.0f) distance = 176.0f;
    f32 weight = (distance - 72.0f) / 104.0f;
    blendPresets(23, 6, weight);
    mVec3_c front;
    if (player.z > mTalkTarget.z) front = player;
    else front = mTalkTarget;
    int edge = getHorizontalEdge(&mDesired.mTarget);
    f32 xOffset = 0.0f;
    s16 angle = cLib::targetAngleY(mDesired.mTarget, front);
    static mAng maxAngle = mAng::fromDegree(19.7f);
    static mAng halfAngle = (s16)maxAngle / 2;
    s16 yaw = 8192.0f - std::fabs((f32)angle);
    if (yaw < 0) yaw = 0;
    yaw = ((f32)yaw / 8192.0f) * (f32)(s16)maxAngle;
    if (yaw > (s16)maxAngle) yaw = maxAngle;
    if (yaw > 0 && !mFixedTalkSide) {
        if (mTalkSide == 3) {
            int side = 2;
            if (angle > 0) side = 1;
            if (edge == 1 && side == 1) {
                if (std::fabs((f32)angle) <= (f32)(s16)halfAngle) side = 2;
                else side = 0;
            }
            if (edge == 2 && side == 2) {
                if (std::fabs((f32)angle) <= (f32)(s16)halfAngle) side = 1;
                else side = 0;
            }
            mTalkSide = side;
        }
        if (mTalkSide == 1) yaw = -yaw;
        if (mTalkSide != 0) {
            yaw = yaw * (1.0f - weight);
            mDesired.mParam.mYaw = yaw;
            xOffset = zOffset * mAng(yaw).sin();
        }
    }
    mDesired.mTarget.x += xOffset;
    mDesired.mTarget.z += zOffset;
}

// 8018B08C
const dCamera::Param_c *dCamera::getPreset(int index) { return &l_presets[index]; }

// 8018B0A0
const dCamera::Transition_c *dCamera::getTransition(int index) { return &l_transitions[index]; }

// 8018B0B4
void dCamera_c::setPreset(u8 preset) {
    mPreset = preset;
    mDesired.mParam = *getPreset(mPreset);
}

// 8018B118
void dCamera_c::blendPresets(int first, int second, f32 weight) {
    mPreset = first;
    const Param_c *a = getPreset(first);
    const Param_c *b = getPreset(second);
    f32 inverse = 1.0f - weight;
    mDesired.mParam.mPitch = (int)(a->mPitch * inverse) + (int)(b->mPitch * weight);
    mDesired.mParam.mYaw = (int)(a->mYaw * inverse) + (int)(b->mYaw * weight);
    mDesired.mParam.mDistance = a->mDistance * inverse + b->mDistance * weight;
    mDesired.mParam.mOffset.x = a->mOffset.x * inverse + b->mOffset.x * weight;
    mDesired.mParam.mOffset.y = a->mOffset.y * inverse + b->mOffset.y * weight;
    mDesired.mParam.mOffset.z = a->mOffset.z * inverse + b->mOffset.z * weight;
}

// 8018B298
void dCamera_c::setTransition(u8 transition) {
    mTransitionId = transition;
    mTransition = *getTransition(mTransitionId);
}

// 8018B2EC
f32 dCamera_c::getAspect() const { return mFrustum.mSize.x / mFrustum.mSize.y; }

// 8018B2FC
f32 dCamera_c::getFovy() { return l_hostIO.mFovy; }

// 8018B308
f32 dCamera_c::getNear() { return l_hostIO.mNear; }

// 8018B318
f32 dCamera_c::getFar() { return l_hostIO.mFar; }

// 8018B328
void dCamera::setSecondaryFovy(f32 fovy) { if (lbl_8074E9B0) lbl_8074E9B0->mSecondaryFovy = fovy; }

// 8018B33C
void dCamera::resetSecondaryFovy() { setSecondaryFovy(l_hostIO.mSecondaryFovy); }

// 8018B34C
void dCamera::useMainFovy() { setSecondaryFovy(l_hostIO.mFovy); }

// 8018B358
void dCamera_c::updateSecondaryProjection() {
    nw4r::g3d::Camera camera = m3d::getCamera(1);
    EGG::Screen screen;
    screen.SetFovY(mSecondaryFovy);
    screen.mNearZ = l_hostIO.mNear;
    screen.mFarZ = l_hostIO.mFar;
    screen.CopyToG3D(camera);
}

// 8018B404
BOOL dCamera_c::setRoomView(int yaw, int pitch) {
    u8 scene = dScene::getCurrentScene();
    if (!dScene::isSceneAttr(scene, dScene::SCENE_ATTR_FTR_ROOM)) return FALSE;
    if (mTransitionFrame < mTransition.mEnd * 0.5f) return FALSE;
    if (mRoomYaw == yaw && mRoomPitch == pitch) return FALSE;
    mRoomYaw = yaw;
    if (mRoomPitch < pitch) playSe(0x16DE);
    else playSe(0x16DD);
    mRoomPitch = pitch;
    startTransition();
    int row = mRoomPitch;
    if (isMuseumExhibit(scene)) {
        if (row == 0) row = 3;
        else if (row == 2) row = 5;
        else if (row == 1 && scene == dScene::SCENE_RM_MM_FOSSIL1) row = 4;
    }
    setPreset(l_roomPresets[row][mRoomYaw]);
    return TRUE;
}

// 8018B554
BOOL dCamera_c::resetLookUpTimer() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN) && mMode == 0 && mPreset != 20)
        mLookUpTimer = 0;
    return FALSE;
}

// 8018B5B8
void dCamera_c::startTalkView(const mVec3_c *target) {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        player.x = actor->getPosP()->x;
        player.y = actor->getPosP()->y + 20.0f;
        player.z = actor->getPosP()->z;
    }
    if (calcTalkDistance(&player, target, NULL, NULL) < 176.0f) {
        mTalkTargetCount = 0;
        mTalkTarget = *target;
        _1A8 = 9;
    }
}

// 8018B67C
void dCamera_c::startThreeActorTalkView(const mVec3_c *a, const mVec3_c *b) {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    dActor_c *actor = static_cast<dActor_c *>(dScene::getScenePlayer());
    if (actor != NULL) {
        player.x = actor->getPosP()->x;
        player.y = actor->getPosP()->y + 20.0f;
        player.z = actor->getPosP()->z;
    }
    if (calcThreeActorTalkDistance(&player, a, b, NULL, NULL) < 176.0f) {
        mTalkTargetCount = 1;
        mTalkTarget = *a;
        mTalkTarget2 = *b;
        _1A8 = 9;
    }
}

// 8018B768
void dCamera_c::switchTalkTarget(const mVec3_c *target) {
    mTalkTarget = *target;
    startTransition();
    mTalkSide = 3;
}

// 8018B7B4
void dCamera_c::startGateView() {
    startTransition();
    setTransition(0);
    _1A8 = 15;
}

// 8018B7F4
void dCamera_c::startGateCloseUp() {
    if (mMode == 15) {
        startTransition();
        setTransition(18);
        setPreset(40);
    }
}

// 8018B844
void dCamera_c::startTwoTargetView(const mVec3_c *target) {
    mTalkTarget = *target;
    _1A8 = 17;
}

// 8018B868
void dCamera_c::restoreModeSlow() { _1A8 = _1A4; mPendingTransition = 17; }

// 8018B87C
void dCamera_c::restoreModeNormal() { _1A8 = _1A4; mPendingTransition = 9; }

// 8018B890
void dCamera_c::restoreModeFast() { _1A8 = _1A4; mPendingTransition = 3; }

// 8018B8A4
void dCamera_c::startBusFrontView() {
    if (mMode == 14) {
        startTransition();
        setTransition(12);
        setPreset(37);
    }
}

// 8018B8F4
void dCamera_c::startBusWideView() {
    if (mMode == 14) {
        startTransition();
        setTransition(13);
        setPreset(36);
    }
}

// 8018B944
void dCamera_c::startBusSideView() {
    if (mMode == 14) {
        startTransition();
        setTransition(14);
        setPreset(38);
    }
}

// 8018B994
void dCamera_c::startBusReturnView() {
    if (mMode == 14) {
        startTransition();
        setTransition(15);
        setPreset(36);
    }
}

// 8018B9E4
dCamera_c::~dCamera_c() {}




