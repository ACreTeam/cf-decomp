#include <game/game/d_a_rmobj_checkp.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_scene.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <nw4r/g3d.h>

// The gate of the check point. Functions are in target .text order.

// The debug parameters of the unit (RTTI "hostIO_c"): the frames of the gate's
// stop sounds.
class hostIO_c {
  public:
    hostIO_c() : mOpenStopFrame(211), mCloseStopFrame(224), _08(1) {
    }
    virtual ~hostIO_c() {
    }

    /* 0x04 */ s16 mOpenStopFrame;  // SE_GATE_OPEN_STOP
    /* 0x06 */ s16 mCloseStopFrame; // SE_GATE_CLOSE_STOP
    /* 0x08 */ u8 _08;
}; // size 0xC

namespace dARmobjCheckp {
// .text 0xCC
BASE_PROFILE(A_RMOBJ_CHECKP, daRmobjChkp_c);

static hostIO_c l_HIO;
} // namespace dARmobjCheckp

daRmobjChkp_c *daRmobjChkp_c::s_instance;

// .text 0x3B8
int daRmobjChkp_c::create() {
    setDemoID(0);
    if (mDoorMdl.create("obj_chk_door", 0)) {
        s_instance = this;
        switch (dScene::getCurrentScene()) {
        case dScene::SCENE_DM_CHKP_CNNCT:
        case dScene::SCENE_DM_CHKP_DCNNCT:
        case dScene::SCENE_DM_CHKP_MISS:
            changeState(STATE_OPEN);
            break;
        default:
            if (dNet::isSessionActive()) {
                changeState(STATE_OPEN);
            } else {
                changeState(STATE_CLOSED);
            }
            break;
        }
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x46C
int daRmobjChkp_c::execute() {
    executeState();
    updateBg();
    static mVec3_c l_sndPos(256.0f, 0.0f, 320.0f);
    mSnd.setPos(&l_sndPos);
    return SUCCEEDED;
}

// .text 0x510
int daRmobjChkp_c::draw() {
    mDoorMdl.entry();
    nw4r::g3d::ResFile file(mDoorMdl.getData());
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    mColor color;
    dEnvironment_c::calcColor(&color, dEnvironment_c::COLOR_8);
    int matId = m3d::getMatID(mdl, "m_light_anim");
    if (matId != -1) {
        nw4r::g3d::ScnMdl *scnMdl =
            nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(
                nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(
                    mDoorMdl.mMdl.getScn()));
        nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, matId);
        if (cma.IsValid()) {
            nw4r::g3d::ResMatTevColor tevColor = cma.GetResMatTevColor(false);
            tevColor.GXSetTevKColor(GX_KCOLOR2, color);
            tevColor.DCStore(false);
        } else {
            nw4r::g3d::ResMatTevColor tevColor =
                mdl.GetResMat("m_light_anim").GetResMatTevColor();
            tevColor.GXSetTevKColor(GX_KCOLOR2, color);
            tevColor.DCStore(false);
        }
    }
    return SUCCEEDED;
}

// .text 0x718
int daRmobjChkp_c::doDelete() {
    s_instance = NULL;
    if (mDoorMdl.remove()) {
        removeBg();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x770
void daRmobjChkp_c::setBgHeight(f32 height) {
    mBg.mHeight = height;
    mBg.mDirty = TRUE;
}

// .text 0x780
void daRmobjChkp_c::updateBg() {
    if (mBg.mMvbg.mActive) {
        dBGCF::getMvbgList()->remove(&mBg.mMvbg);
    }
    if (mBg.mDirty) {
        mVec3_c pos(256.0f, 0.0f, 256.0f + mBg.mHeight);
        dBGCF::getMvbgList()->add(128.0f, 0.0f, 32.0f, &mBg.mMvbg, pos, mAng(0),
                                  NULL);
        mBg.mDirty = FALSE;
    }
}

// .text 0x830
void daRmobjChkp_c::removeBg() {
    if (mBg.mMvbg.mActive) {
        dBGCF::getMvbgList()->remove(&mBg.mMvbg);
    }
}

namespace dARmobjCheckp {
static const daRmobjChkp_c::InitFunc l_stateInit[] = {
    &daRmobjChkp_c::initClosed,
    &daRmobjChkp_c::initOpening,
    &daRmobjChkp_c::initOpen,
    &daRmobjChkp_c::initClosing,
};
static const daRmobjChkp_c::ExecFunc l_stateExec[] = {
    &daRmobjChkp_c::executeClosed,
    &daRmobjChkp_c::executeOpening,
    &daRmobjChkp_c::executeOpen,
    &daRmobjChkp_c::executeClosing,
};
} // namespace dARmobjCheckp

// .text 0x870
BOOL daRmobjChkp_c::changeState(int state) {
    if (state < STATE_NUM) {
        if ((this->*dARmobjCheckp::l_stateInit[state])()) {
            mState = state;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// .text 0x8E4
void daRmobjChkp_c::executeState() {
    if (mState < STATE_NUM) {
        (this->*dARmobjCheckp::l_stateExec[mState])();
    }
    f32 frameMax = mDoorMdl.mAnmChr.mFrameMax;
    f32 ratio = mDoorMdl.mAnmChr.getFrame() / frameMax;
    if (mState == STATE_OPEN || mState == STATE_CLOSING) {
        ratio = 1.0f - ratio;
    }
    dEnvironment_c::setCheckPRatio(ratio);
}

// .text 0x970
BOOL daRmobjChkp_c::initClosed() {
    return mDoorMdl.setAnm(ANM_OPEN, m3d::FORWARD_ONCE, 0.0f);
}

// .text 0x988
void daRmobjChkp_c::executeClosed() {
    setBgHeight(0.0f);
}

// .text 0x994
BOOL daRmobjChkp_c::initOpening() {
    if (mDoorMdl.setAnm(ANM_OPEN, m3d::FORWARD_ONCE, 0.0f)) {
        mSnd.startSound(SE_GATE_OPEN);
        return TRUE;
    }
    return FALSE;
}

// .text 0x9F4
void daRmobjChkp_c::executeOpening() {
    if (mDoorMdl.isStop()) {
        changeState(STATE_OPEN);
    } else {
        if (mDoorMdl.mAnmChr.checkFrame(dARmobjCheckp::l_HIO.mOpenStopFrame)) {
            mSnd.startSound(SE_GATE_OPEN_STOP);
        }
        mDoorMdl.play();
    }
    f32 height = mBg.mHeight;
    sLib::addCalc2(&height, -32.0f, 0.005f, 1.0f);
    setBgHeight(height);
}

// .text 0xAC0
BOOL daRmobjChkp_c::initOpen() {
    return mDoorMdl.setAnm(ANM_CLOSE, m3d::FORWARD_ONCE, 0.0f);
}

// .text 0xAD8
void daRmobjChkp_c::executeOpen() {
    if (dNet::isSessionActive() && dNet::getMemberNum() >= 2) {
        setBgHeight(0.0f);
    } else {
        setBgHeight(-32.0f);
    }
}

// .text 0xB3C
BOOL daRmobjChkp_c::initClosing() {
    if (mDoorMdl.setAnm(ANM_CLOSE, m3d::FORWARD_ONCE, 0.0f)) {
        mSnd.startSound(SE_GATE_CLOSE);
        return TRUE;
    }
    return FALSE;
}

// .text 0xB9C
void daRmobjChkp_c::executeClosing() {
    if (mDoorMdl.isStop()) {
        changeState(STATE_CLOSED);
    } else {
        if (mDoorMdl.mAnmChr.checkFrame(dARmobjCheckp::l_HIO.mCloseStopFrame)) {
            mSnd.startSound(SE_GATE_CLOSE_STOP);
        }
        mDoorMdl.play();
    }
    f32 height = mBg.mHeight;
    sLib::addCalc(&height, 0.0f, 0.01f, 1.0f, 0.015f);
    setBgHeight(height);
}

namespace dARmobjCheckp {
// .text 0xC6C
BOOL isGateClosed() {
    if (daRmobjChkp_c::s_instance != NULL) {
        return daRmobjChkp_c::s_instance->getState() ==
               daRmobjChkp_c::STATE_CLOSED;
    }
    return FALSE;
}

// .text 0xC94
BOOL isGateOpen() {
    if (daRmobjChkp_c::s_instance != NULL) {
        return daRmobjChkp_c::s_instance->getState() ==
               daRmobjChkp_c::STATE_OPEN;
    }
    return FALSE;
}

// .text 0xCC0
BOOL openGate() {
    if (daRmobjChkp_c::s_instance != NULL) {
        return daRmobjChkp_c::s_instance->changeState(
            daRmobjChkp_c::STATE_OPENING);
    }
    return FALSE;
}

// .text 0xCE0
BOOL closeGate() {
    if (daRmobjChkp_c::s_instance != NULL) {
        return daRmobjChkp_c::s_instance->changeState(
            daRmobjChkp_c::STATE_CLOSING);
    }
    return FALSE;
}
} // namespace dARmobjCheckp

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
