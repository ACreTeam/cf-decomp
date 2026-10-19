#include <game/game/d_a_rmobj_reset.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/snd/snd_id.hpp>
#include <nw4r/g3d.h>

// The Reset Surveillance Center's machinery. Functions are in target .text
// order.

// The debug parameters of the unit (RTTI "hostIO_c"): frames of the seat's
// sounds and the fire.
class hostIO_c {
  public:
    hostIO_c()
        : _04(0xFF), _06(0), mFireDelay(36), mSteamFrame(120), mLeverFrame(30),
          mSeatStartFrame(40) {
    }
    virtual ~hostIO_c() {
    }

    /* 0x04 */ s16 _04;
    /* 0x06 */ s16 _06;
    /* 0x08 */ int
        mFireDelay; // executeBack: frames from the sparks to the fire
    /* 0x0C */ int mSteamFrame; // SE_ResetCenter_SeatSteam
    /* 0x10 */ int mLeverFrame; // SE_ResetCenter_Lever
    /* 0x14 */ int
        mSeatStartFrame; // SE_ResetCenter_SeatStart from this frame on
}; // size 0x18

namespace dARmobjReset {
// .text 0xCC
BASE_PROFILE(A_RMOBJ_RESET, daRmobjReset_c);
} // namespace dARmobjReset

daRmobjReset_c *daRmobjReset_c::s_instance;

namespace dARmobjReset {
static hostIO_c l_HIO;
} // namespace dARmobjReset

// .text 0x308
int daRmobjReset_c::create() {
    mFireTimer = -1;
    setDemoID(0);
    void *tvData = dFtrm_c::sLoader.getFile("int_kob_oth_dummytv.brres");
    bool ok = true;
    ok &= mTvMdl.create("obj_tw_reset_TV", 0);
    ok &= mRoomMdl.create("obj_tw_reset_R", 0x204);
    nw4r::g3d::ResFile tvFile(tvData);
    if (ok && tvFile.IsValid()) {
        nw4r::g3d::ResFile file(mTvMdl.getData());
        file.Release();
        file.Bind(tvFile);
        file.Bind();
        changeState(STATE_NONE);
        s_instance = this;
        updateChair();
        dSaveData_c::getTown()->clearFlag(SAVE_FLAG_RESET_OPEN);
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x41C
int daRmobjReset_c::execute() {
    updateChair();
    mSnd.setPos(&mChairPos);
    static mVec3_c l_doorSndPos(272.0f, 0.0f, 288.0f);
    mDoorSnd.setPos(&l_doorSndPos);
    static mVec3_c l_tvSndPos(176.0f, 64.0f, 368.0f);
    mTvSnd.setPos(&l_tvSndPos);
    executeState();
    if (mLightOn) {
        holdSe(SE_ResetCenter_Alert);
    }
    mRoomMdl.play();
    if (!mLightOn) {
        if (mLightUsed) {
            mTvSnd.holdSound(SE_ResetKanshiCenterTV_Lev_ScrambleResstart);
        } else {
            mTvSnd.holdSound(SE_ResetKanshiCenterTV_Lev);
        }
    }
    if (mFireTimer > 0) {
        mFireTimer--;
    }
    if (mFireTimer == 0) {
        dEffect::create("af_obj_rstgear_fire", &mFirePos, NULL, NULL);
        mFireTimer = -1;
    }
    return SUCCEEDED;
}

// Matching-only storage workaround: the target retains 0x1C unreferenced
// bytes after execute's statics. A function-local mVec3_c and its guard /
// destructor node reproduce that storage. The bytes do not establish the
// original function, type, name or initializer; this function is synthetic
// and the linker strips its code. See notes/TUs/d_a_rmobj_audit.txt.
const mVec3_c *daRmobjReset_c::getUnusedSndPos() {
    static mVec3_c l_unusedSndPos(0.0f, 0.0f, 0.0f);
    return &l_unusedSndPos;
}

// .text 0x5A8
int daRmobjReset_c::draw() {
    mTvMdl.entry();
    mRoomMdl.entry();
    return SUCCEEDED;
}

// .text 0x5E4
int daRmobjReset_c::doDelete() {
    s_instance = NULL;
    bool ok = true;
    ok &= mTvMdl.remove();
    ok &= mRoomMdl.remove();
    return ok;
}

// .text 0x644
void daRmobjReset_c::lightOn() {
    if (mRoomMdl.mAnmSrt.IsBound()) {
        mRoomMdl.setAnmSrt("obj_tw_rst_light", m3d::FORWARD_LOOP);
        mLightOn = TRUE;
        mLightUsed = TRUE;
        if (dFixFarSwitchLight_c::sInstance != NULL) {
            dFixFarSwitchLight_c::sInstance->mOn = FALSE;
        }
        if (dResetCenterLight_c::sInstance != NULL) {
            dResetCenterLight_c::sInstance->mOn = TRUE;
        }
        if (dFixAmbSwitchLight_c::sInstance != NULL) {
            dFixAmbSwitchLight_c::sInstance->mOn = FALSE;
        }
    }
}

// .text 0x6E4
void daRmobjReset_c::lightOff() {
    if (mRoomMdl.mAnmSrt.IsBound()) {
        mRoomMdl.mAnmSrt.releaseAnm(0);
        mRoomMdl.mMdl.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_TEXSRT);
        mLightOn = FALSE;
        if (dFixFarSwitchLight_c::sInstance != NULL) {
            dFixFarSwitchLight_c::sInstance->mOn = TRUE;
        }
        if (dResetCenterLight_c::sInstance != NULL) {
            dResetCenterLight_c::sInstance->mOn = FALSE;
        }
        if (dFixAmbSwitchLight_c::sInstance != NULL) {
            dFixAmbSwitchLight_c::sInstance->mOn = TRUE;
        }
    }
}

namespace dARmobjReset {
static const daRmobjReset_c::InitFunc l_stateInit[] = {
    &daRmobjReset_c::initNone,      &daRmobjReset_c::initNone,
    &daRmobjReset_c::initLaunch,    &daRmobjReset_c::initWait,
    &daRmobjReset_c::initBack,      &daRmobjReset_c::initBackEnd,
    &daRmobjReset_c::initDoorClose, &daRmobjReset_c::initEnd,
};
static const daRmobjReset_c::ExecFunc l_stateExec[] = {
    &daRmobjReset_c::executeNone,      &daRmobjReset_c::executeNone,
    &daRmobjReset_c::executeLaunch,    &daRmobjReset_c::executeWait,
    &daRmobjReset_c::executeBack,      &daRmobjReset_c::executeBackEnd,
    &daRmobjReset_c::executeDoorClose, &daRmobjReset_c::executeEnd,
};
} // namespace dARmobjReset

// .text 0x784
BOOL daRmobjReset_c::changeState(int state) {
    if (state < STATE_NUM && (this->*dARmobjReset::l_stateInit[state])()) {
        mState = state;
        return TRUE;
    }
    return TRUE;
}

// .text 0x7F0
void daRmobjReset_c::executeState() {
    if (mState < STATE_NUM) {
        (this->*dARmobjReset::l_stateExec[mState])();
    }
}

// .text 0x830
BOOL daRmobjReset_c::initNone() {
    return TRUE;
}

// .text 0x838
void daRmobjReset_c::executeNone() {
}

// .text 0x83C
BOOL daRmobjReset_c::initLaunch() {
    if (mRoomMdl.setAnmChr("obj_tw_rst_gear", m3d::FORWARD_ONCE, 0.0f)) {
        mStartEffect = TRUE;
        return TRUE;
    }
    return FALSE;
}

// .text 0x89C
void daRmobjReset_c::executeLaunch() {
    if (mStartEffect) {
        dEffect::create("af_obj_rstgear_start", &mChairPos, NULL, NULL);
        mStartEffect = FALSE;
    }
    if (mRoomMdl.mAnmChr.checkFrame(dARmobjReset::l_HIO.mLeverFrame)) {
        mSnd.startSound(SE_ResetCenter_Lever);
    }
    if (mRoomMdl.mAnmChr.getFrame() >= dARmobjReset::l_HIO.mSeatStartFrame) {
        mSnd.holdSound(SE_ResetCenter_SeatStart);
    }
    if (mRoomMdl.mAnmChr.checkFrame(dARmobjReset::l_HIO.mSteamFrame)) {
        mSnd.startSound(SE_ResetCenter_SeatSteam);
    }
    if (mRoomMdl.mAnmChr.isStop()) {
        changeState(STATE_WAIT);
    }
}

// .text 0x9E8
BOOL daRmobjReset_c::initWait() {
    return mRoomMdl.setAnmChr("obj_tw_rst_wait", m3d::FORWARD_LOOP, 0.0f);
}

// .text 0xA04
void daRmobjReset_c::executeWait() {
    mSnd.holdSound(SE_ResetCenter_SeatStart);
}

// .text 0xA10
BOOL daRmobjReset_c::initBack() {
    if (mRoomMdl.setAnmChr("obj_tw_rst_back", m3d::FORWARD_ONCE, 0.0f)) {
        mSnd.startSound(SE_ResetCenter_ScrambleOut);
        mBackEffect = TRUE;
        return TRUE;
    }
    return FALSE;
}

// .text 0xA7C
void daRmobjReset_c::executeBack() {
    mSnd.holdSound(SE_ResetCenter_SeatStart);
    if (mBackEffect) {
        mBackEffect = FALSE;
        mFirePos = mChairPos;
        mFireTimer = dARmobjReset::l_HIO.mFireDelay;
        mSparkEffect.create("af_obj_rstgear_hibana", &mChairPos, NULL, NULL);
    } else {
        mSparkEffect.followEffect(&mChairPos, NULL, NULL);
    }
    if (mRoomMdl.mAnmChr.isStop()) {
        changeState(STATE_BACK_END);
    }
}

// .text 0xB3C
BOOL daRmobjReset_c::initBackEnd() {
    return TRUE;
}

// .text 0xB44
void daRmobjReset_c::executeBackEnd() {
    mSparkEffect.followEffect(&mChairPos, NULL, NULL);
}

// .text 0xB58
BOOL daRmobjReset_c::initDoorClose() {
    if (mRoomMdl.setAnmChr("obj_tw_rst_door1", m3d::FORWARD_ONCE, 0.0f)) {
        mDoorSnd.startSound(SE_ResetCenter_DoorClose);
        return TRUE;
    }
    return FALSE;
}

// .text 0xBBC
void daRmobjReset_c::executeDoorClose() {
    if (mRoomMdl.mAnmChr.isStop()) {
        changeState(STATE_END);
    }
}

// .text 0xC00
BOOL daRmobjReset_c::initEnd() {
    return TRUE;
}

// .text 0xC08
void daRmobjReset_c::executeEnd() {
}

// .text 0xC0C
void daRmobjReset_c::updateChair() {
    mRoomMdl.mMdl.calc(false);
    int node = m3d::getNodeID(
        nw4r::g3d::ResFile(mRoomMdl.getData()).GetResMdl(0), "chair0");
    if (node != -1) {
        mMtx_c mtx;
        mRoomMdl.mMdl.getNodeWorldMtx(node, &mtx);
        PSMTXMultVec(mtx, mVec3_c::Zero, mChairPos);
        mChairMtx = mtx;
    }
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
