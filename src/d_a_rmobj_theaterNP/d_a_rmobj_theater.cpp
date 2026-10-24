#include <game/game/d_a_rmobj_theater.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_environment.hpp>
#include <game/mLib/m_color.hpp>

// The theater stage. Functions are in target .text order.

// The light material's settings (RTTI "hostIO_c"): the konst alpha of
// "m_light".
class hostIO_c {
  public:
    hostIO_c() : mAlphaOn(0xFF), mAlphaOff(0) {
    }
    virtual ~hostIO_c() {
    }

    /* 0x04 */ s16 mAlphaOn;
    /* 0x06 */ s16 mAlphaOff;
};

namespace dARmobjTheater {
// .text 0xCC
BASE_PROFILE(A_RMOBJ_THEATER, daRmobjTheater_c);

hostIO_c l_hio;
} // namespace dARmobjTheater

daRmobjTheater_c *daRmobjTheater_c::sInstance;

// .text 0x2C8
int daRmobjTheater_c::create() {
    setDemoID(0);
    bool ok = true;
    ok &= mCurtain.create("obj_tw_theater_dontyou", 0);
    ok &= mLight.create("obj_tw_theater_light", 0);
    if (ok) {
        mLight.setAnm("obj_tw_tht_light_off", m3d::FORWARD_LOOP, 0.0f);
        sInstance = this;
        changeState(STATE_CLOSED);
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x380
int daRmobjTheater_c::execute() {
    executeState();
    mLight.playSrt();
    nw4r::g3d::ResFile file(mLight.getData());
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    nw4r::g3d::ResMatTevColor tev =
        mdl.GetResMat("m_light").GetResMatTevColor();
    tev.GXSetTevColor(GX_TEVREG0,
                       mLightOn
                           ? mColor(0, 0, 0, dARmobjTheater::l_hio.mAlphaOn)
                           : mColor(0, 0, 0, dARmobjTheater::l_hio.mAlphaOff));
    tev.DCStore(false);
    return SUCCEEDED;
}

// .text 0x4A4
int daRmobjTheater_c::draw() {
    mCurtain.entry();
    mLight.entry();
    return SUCCEEDED;
}

// .text 0x4E0
int daRmobjTheater_c::doDelete() {
    sInstance = NULL;
    if (mCurtain.remove() && mLight.remove()) {
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x540
BOOL daRmobjTheater_c::changeState(int state) {
    static const InitProc l_init[] = {
        &daRmobjTheater_c::initClosed,
        &daRmobjTheater_c::initOpening,
        &daRmobjTheater_c::initOpen,
        &daRmobjTheater_c::initClosing,
    };
    if (state < STATE_NUM) {
        if ((this->*l_init[state])()) {
            mState = state;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// .text 0x5B4
void daRmobjTheater_c::executeState() {
    static const ExecuteProc l_execute[] = {
        &daRmobjTheater_c::executeClosed,
        &daRmobjTheater_c::executeOpening,
        &daRmobjTheater_c::executeOpen,
        &daRmobjTheater_c::executeClosing,
    };
    if (mState < STATE_NUM) {
        (this->*l_execute[mState])();
    }
}

// .text 0x5F4
BOOL daRmobjTheater_c::initClosed() {
    return mCurtain.setAnm(CURTAIN_ANM_CLOSE, m3d::REVERSE_ONCE, 0.0f);
}

// .text 0x60C
void daRmobjTheater_c::executeClosed() {
    dFixSpotTheaterLight_c *light = dFixSpotTheaterLight_c::sInstance;
    if (light != NULL) {
        light->mValue = 0.0f;
        light->mMode = 0;
    }
}

// .text 0x634
BOOL daRmobjTheater_c::initOpening() {
    if (mCurtain.setAnm(CURTAIN_ANM_OPEN, m3d::FORWARD_ONCE, 0.0f) &&
        mLight.setAnm("obj_tw_tht_light_on", m3d::FORWARD_LOOP, 0.0f)) {
        mLightOn = true;
        return TRUE;
    }
    return FALSE;
}

// .text 0x6B8
void daRmobjTheater_c::executeOpening() {
    if (mCurtain.isStop()) {
        changeState(STATE_OPEN);
    } else {
        mCurtain.play();
    }
    dFixSpotTheaterLight_c *light = dFixSpotTheaterLight_c::sInstance;
    if (light != NULL) {
        f32 frameMax = mCurtain.mAnmChr.mFrameMax;
        light->mValue = mCurtain.mAnmChr.getFrame() / frameMax;
        light->mMode = 1;
    }
}

// .text 0x74C
BOOL daRmobjTheater_c::initOpen() {
    return mCurtain.setAnm(CURTAIN_ANM_OPEN, m3d::REVERSE_ONCE, 0.0f);
}

// .text 0x764
void daRmobjTheater_c::executeOpen() {
    dFixSpotTheaterLight_c *light = dFixSpotTheaterLight_c::sInstance;
    if (light != NULL) {
        light->mValue = 1.0f;
        light->mMode = 1;
    }
}

// .text 0x78C
BOOL daRmobjTheater_c::initClosing() {
    return mCurtain.setAnm(CURTAIN_ANM_CLOSE, m3d::FORWARD_ONCE, 0.0f);
}

// .text 0x7A4
void daRmobjTheater_c::executeClosing() {
    if (mCurtain.isStop()) {
        changeState(STATE_CLOSED);
    } else {
        mCurtain.play();
    }
    dFixSpotTheaterLight_c *light = dFixSpotTheaterLight_c::sInstance;
    if (light != NULL) {
        f32 frameMax = mCurtain.mAnmChr.mFrameMax;
        light->mValue = 1.0f - mCurtain.mAnmChr.getFrame() / frameMax;
        light->mMode = 0;
    }
}

// .text 0x844
BOOL daRmobjTheater_c::isClosed() {
    if (sInstance != NULL) {
        return sInstance->mState == STATE_CLOSED;
    }
    return FALSE;
}

// .text 0x86C
BOOL daRmobjTheater_c::isOpen() {
    if (sInstance != NULL) {
        return sInstance->mState == STATE_OPEN;
    }
    return FALSE;
}

// .text 0x898
BOOL daRmobjTheater_c::open() {
    if (sInstance != NULL) {
        return sInstance->changeState(STATE_OPENING);
    }
    return FALSE;
}

// .text 0x8B8
BOOL daRmobjTheater_c::close() {
    if (sInstance != NULL) {
        return sInstance->changeState(STATE_CLOSING);
    }
    return FALSE;
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
