#include <game/game/d_a_rmobj_bus.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_weather.hpp>
#include <game/snd/snd_id.hpp>

// The bus interior. Functions are in target .text order.

// The bus's debug parameters (RTTI "hostIO_c").
class hostIO_c {
  public:
    hostIO_c() {
        mWiperMode = -1;
    }
    virtual ~hostIO_c() {
    }

    /* 0x4 */ s16 mWiperMode; // -1: with rain or snow, 1: always
}; // size 0x8

namespace dARmobjBus {
static hostIO_c l_HIO;
} // namespace dARmobjBus

daRmobjBus_c *daRmobjBus_c::m_instance;

namespace dARmobjBus {
// .text 0xCC
BASE_PROFILE(A_RMOBJ_BUS, daRmobjBus_c);
} // namespace dARmobjBus

// .text 0x364
BOOL daRmobjBus_c::startTunnelExit() {
    if (m_instance != NULL) {
        return m_instance->setStep(STEP_TUNNEL_EXIT);
    }
    return FALSE;
}

// .text 0x384
BOOL daRmobjBus_c::isTunnelExitStarted() {
    if (m_instance != NULL) {
        return m_instance->mTunnelExitStarted;
    }
    return FALSE;
}

// .text 0x3A4
void daRmobjBus_c::reloadSeason() {
    if (m_instance != NULL) {
        dScene::updateSaveSeason();
        m_instance->mReloadSeason = true;
    }
}

// .text 0x3E8
BOOL daRmobjBus_c::isSeasonReloaded() {
    if (m_instance != NULL) {
        return m_instance->mReloadSeason == false;
    }
    return FALSE;
}

// .text 0x410
int daRmobjBus_c::create() {
    mIsPlayerCreate = dScene::getCurrentScene() == dScene::SCENE_DM_BUS_PL_CRT;
    mTunnelExitStarted = true;
    mIsSaveGood = dSaveData_c::get()->isGood();
    setDemoID(0);
    bool ok = true;
    ok &= mBgModel.create("obj_bus_bg", 0);
    ok &= mWiperModel.create("obj_bus_wiper", 0);
    if (mIsPlayerCreate && mIsSaveGood) {
        ok &= mTunnelModel.create("obj_bus_bg_tunnel", 0);
        ok &= mPackBank.load(dHeap::localHeap_p);
        mBusRatio = 0.0f;
    } else if (mIsPlayerCreate) {
        ok &= mTunnelModel.create("obj_bus_bg_tunnel", 0);
        mBusRatio = 0.0f;
    } else {
        ok &= mPackBank.load(dHeap::localHeap_p);
        mBusRatio = 1.0f;
    }
    if (ok) {
        mWiperModel.setAnmChr("obj_bus_wiper", m3d::FORWARD_LOOP, 0.0f);
        mWiperModel.mMdl.setPriorityDraw(2, 8);
        if (mIsPlayerCreate) {
            setStep(STEP_TUNNEL);
        } else {
            setStep(STEP_NONE);
        }
        if (mIsPlayerCreate) {
            mTunnelModel.setAnm(ANM_TUNNEL, m3d::FORWARD_LOOP, 0.0f);
        }
        if (!mIsPlayerCreate || (mIsPlayerCreate && mIsSaveGood)) {
            nw4r::g3d::ResFile(mBgModel.getData()).Release();
            nw4r::g3d::ResFile(mBgModel.getData())
                .Bind(nw4r::g3d::ResFile(mPackBank.getData()));
            nw4r::g3d::ResFile(mBgModel.getData()).Bind();
            mBgModel.setAnmSrt("obj_bus_bg", m3d::FORWARD_LOOP);
        }
        m_instance = this;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x688
int daRmobjBus_c::execute() {
    dWeather_c *weather = dWeather_c::sInstance;
    if (weather != NULL && mStep != STEP_TUNNEL) {
        mIsRain = dWeather_c::isRainKind(weather->mWeather);
        mIsSnow = dWeather_c::isSnowKind(weather->mWeather);
    }
    if (mReloadSeason && mPackBank.load(dHeap::localHeap_p)) {
        nw4r::g3d::ResFile(mBgModel.getData()).Release();
        nw4r::g3d::ResFile(mBgModel.getData())
            .Bind(nw4r::g3d::ResFile(mPackBank.getData()));
        nw4r::g3d::ResFile(mBgModel.getData()).Bind();
        mBgModel.setAnmSrt("obj_bus_bg", m3d::FORWARD_LOOP);
        mReloadSeason = false;
    }
    mBgModel.playSrt();
    if (((mIsRain || mIsSnow) && dARmobjBus::l_HIO.mWiperMode == -1) ||
        dARmobjBus::l_HIO.mWiperMode == 1) {
        mWiperModel.playMdl();
        static mVec3_c pos(256.0f, 32.0f, 288.0f);
        mSnd.setPos(&pos);
        if (mWiperModel.mAnmChr.checkFrame(2.0f)) {
            mSnd.startSound(SE_wiper1);
        } else if (mWiperModel.mAnmChr.checkFrame(62.0f)) {
            mSnd.startSound(SE_wiper2);
        }
        if (mIsRain || dARmobjBus::l_HIO.mWiperMode == 1) {
            holdSe(SE_BusRunRain);
            holdSe(SE_BusRain);
        } else {
            holdSe(SE_BusRun);
        }
    } else {
        holdSe(SE_BusRun);
    }
    executeStep();
    dEnvironment_c::setBusRatio(mBusRatio);
    return SUCCEEDED;
}

// .text 0x8E0
int daRmobjBus_c::draw() {
    mBgModel.entry();
    mWiperModel.entry();
    if (mIsPlayerCreate && mStep != STEP_NONE) {
        mTunnelModel.entry();
    }
    return SUCCEEDED;
}

// .text 0x93C
int daRmobjBus_c::doDelete() {
    m_instance = NULL;
    if (mBgModel.remove() && mPackBank.unload() && mWiperModel.remove() &&
        mTunnelModel.remove()) {
        return SUCCEEDED;
    }
    return NOT_READY;
}

typedef BOOL (daRmobjBus_c::*initStep_t)();
typedef void (daRmobjBus_c::*executeStep_t)();

namespace dARmobjBus {
static const initStep_t l_initSteps[daRmobjBus_c::STEP_NUM] = {
    &daRmobjBus_c::initNone,
    &daRmobjBus_c::initTunnel,
    &daRmobjBus_c::initTunnelExit,
};
static const executeStep_t l_executeSteps[daRmobjBus_c::STEP_NUM] = {
    &daRmobjBus_c::executeNone,
    &daRmobjBus_c::executeTunnel,
    &daRmobjBus_c::executeTunnelExit,
};
} // namespace dARmobjBus

// .text 0x9BC
BOOL daRmobjBus_c::setStep(int step) {
    if (step < STEP_NUM) {
        if ((this->*dARmobjBus::l_initSteps[step])()) {
            mStep = step;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// .text 0xA30
void daRmobjBus_c::executeStep() {
    if (mStep < STEP_NUM) {
        (this->*dARmobjBus::l_executeSteps[mStep])();
    }
}

// .text 0xA70
BOOL daRmobjBus_c::initNone() {
    return TRUE;
}

// .text 0xA78
void daRmobjBus_c::executeNone() {
}

// .text 0xA7C
BOOL daRmobjBus_c::initTunnel() {
    mTunnelModel.setAnm(ANM_TUNNEL, m3d::FORWARD_LOOP, 0.0f);
    return TRUE;
}

// .text 0xAB4
void daRmobjBus_c::executeTunnel() {
    mTunnelModel.play();
}

// .text 0xABC
BOOL daRmobjBus_c::initTunnelExit() {
    mTunnelExitStarted = false;
    mBgModel.setAnmSrt("obj_bus_bg", m3d::FORWARD_LOOP);
    return TRUE;
}

// .text 0xAF8
void daRmobjBus_c::executeTunnelExit() {
    if (!mTunnelExitStarted) {
        mTunnelModel.play();
        if (mTunnelModel.mAnmSrt.IsBound() &&
            mTunnelModel.mAnmSrt.checkFrame(0.0f, 0)) {
            mTunnelModel.setAnm("obj_bus_bg_tunnel1", m3d::FORWARD_ONCE, 0.0f);
            mTunnelExitStarted = true;
        }
    } else {
        mTunnelModel.play();
        f32 frames = 10.0f;
        nw4r::g3d::ResFile res(mTunnelModel.getData());
        if (res.GetResAnmTexSrtNumEntries() >= 2) {
            frames = res.GetResAnmTexSrt(1).GetNumFrame();
        }
        mBusRatio = (mTunnelModel.mAnmSrt.getFrame(0) + 1.0f) / frames;
        if (mTunnelModel.isStop()) {
            setStep(STEP_NONE);
        }
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
