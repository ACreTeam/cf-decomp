// The scene base class (dScene_c) and the scene change. .text 801553E0..8015593C, .data 804EF650..804EF668,
// .sdata 8074B1B0..8074B1C0, .sbss 8074E800..8074E810. See include/game/game/d_scene_base.hpp.
#include <game/game/d_scene_base.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_fader.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_main.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_reset_save.hpp>
#include <game/game/d_root.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/mLib/m_perf.hpp>

ProfileName dScene_c::m_nextScene = SCENE_PROF_NONE;
bool dScene_c::m_sceneCreated = true;
u16 dScene_c::m_fadeInFrame = 30;
u16 dScene_c::m_fadeOutFrame = 30;
dScene_c *dScene_c::m_nowScene;
int dScene_c::m_fadeInType;
int dScene_c::m_fadeOutType;

// 801553E0
int dScene_c::preCreate() {
    m_nowScene = this;
    return SUCCEEDED;
}

// 801553EC
void dScene_c::postCreate(MAIN_STATE_e state) {
    if (state == SUCCESS) {
        dReset::Manage_c::GetInstance()->SetSoftResetFinish();
        dFgMngProc_c::execute();
        sndSceneStart();
        if (getKind() == SCENE_KIND_ONLINE) {
            dNet::fn_800DDE80();
        }
    }
    fBase_c::postCreate(state);
}

// 80155464
int dScene_c::preDelete() {
    return fBase_c::preDelete() != NOT_READY;
}

// 80155490
void dScene_c::postDelete(MAIN_STATE_e state) {
    if (state == SUCCESS) {
        m_sceneCreated = false;
        m_nowScene = NULL;
        sndSceneEnd();
        dNet::postNet();
        dResetSave::executeSceneDelete();
        dReset::Manage_c::GetInstance()->PostDeleteScene();
    }
    fBase_c::postDelete(state);
}

// 801554F8
int dScene_c::preExecute() {
    if (fBase_c::preExecute() == NOT_READY) {
        return NOT_READY;
    }

    if (m_nextScene != SCENE_PROF_NONE) {
        if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
            dFader::setFader(m_fadeOutType);
            mFader_c::mFader->setFrame(m_fadeOutFrame);
            mFader_c::mFader->fadeOut();
            sndSceneChange(m_fadeOutFrame);
        } else if (mFader_c::isStatus(mFaderBase_c::OPAQUE)) {
            int state = dNet::getSceneChangeState();
            if (state != NET_SCENE_STATE_NONE && !dReset::Manage_c::GetInstance()->isSoftResetFade()) {
                if (state == NET_SCENE_STATE_CLOSE) {
                    dNet::writeSceneClose(dNet::getMyNo());
                    dNet::setSceneChangeState(NET_SCENE_STATE_CLOSE_SEND);
                } else if (state == NET_SCENE_STATE_LEAVE) {
                    if (dNet::fn_800DD818()) {
                        dNet::writeSceneLeave(dNet::getMyNo());
                        dNet::setSceneChangeState(NET_SCENE_STATE_LEAVE_WRITTEN);
                    } else {
                        return NOT_READY;
                    }
                } else if (state == NET_SCENE_STATE_HANDOVER) {
                    if (dNet::fn_800DD8C4()) {
                        dNet::writeSceneHandover();
                        dNet::setSceneChangeState(NET_SCENE_STATE_HANDOVER_WRITTEN);
                    } else {
                        return NOT_READY;
                    }
                } else {
                    return NOT_READY;
                }
            }
            deleteRequest();
        }
        return NOT_READY;
    }

    if (isProcControlFlag(ROOT_DISABLE_EXECUTE)) {
        if (getChildProcessCreateState() == NULL) {
            if (dNet::getSceneChangeState() == NET_SCENE_STATE_NONE ||
                dReset::Manage_c::GetInstance()->isSoftResetFade()) {
                clearProcControlFlag(ROOT_DISABLE_EXECUTE);
                clearProcControlFlag(ROOT_DISABLE_DRAW);
            }
        }
        return NOT_READY;
    }

    if (mFader_c::isStatus(mFaderBase_c::OPAQUE)) {
        dFader::setFader(m_fadeInType);
        mFader_c::mFader->setFrame(m_fadeInFrame);
        mFader_c::mFader->fadeIn();
    }
    return SUCCEEDED;
}

// 80155744
void dScene_c::postExecute(MAIN_STATE_e state) {
    if (state != CANCELED) {
        dReset::Manage_c::GetInstance()->SetSoftResetScene();
    }
}

// 80155770
int dScene_c::preDraw() {
    return fBase_c::preDraw() != NOT_READY;
}

// 8015579C
void dScene_c::postDraw(MAIN_STATE_e state) {}

// 801557A0
void dScene_c::initNextScene() {
    m_nextScene = SCENE_PROF_BOOT;
    m_sceneCreated = false;
}

// 801557B4
fBase_c *dScene_c::createNextScene() {
    if (m_sceneCreated || m_nextScene == SCENE_PROF_NONE) {
        return NULL;
    }

    int state = dNet::getSceneChangeState();
    if (state != NET_SCENE_STATE_NONE && !dReset::Manage_c::GetInstance()->isSoftResetFade()) {
        if (state == NET_SCENE_STATE_CREATE) {
            dNet::enterLeaderScene();
            dNet::setSceneChangeState(NET_SCENE_STATE_CREATING);
        } else if (state != NET_SCENE_STATE_CREATING) {
            return NULL;
        }
    }

    if (dRoot_c::sInstance == NULL) {
        dBase_c::createRoot(0, 0, 0);
        return NULL;
    }

    fBase_c *scene = dBase_c::createBase(m_nextScene, dRoot_c::sInstance, 0, SCENE);
    if (scene != NULL) {
        if (state != NET_SCENE_STATE_NONE) {
            dNet::setSceneChangeState(NET_SCENE_STATE_CREATED);
        }
        m_nextScene = SCENE_PROF_NONE;
        m_sceneCreated = true;
        return scene;
    }
    return NULL;
}

// 801558D0
void dScene_c::createPerf(EGG::Heap *heap) {
    fn_802B8DE8(heap);
}

// 801558D4
void dScene_c::beginPerf() {
    fn_802B8DEC();
}

// 801558D8
void dScene_c::checkPerf() {
    fn_802B8DF0();
    if (fn_802B8DF8()) {
        if (dMain::isStopped(dMain::STOP_REASON_PERF)) {
            dMain::removeStop(dMain::STOP_REASON_PERF);
        }
    } else {
        if (!dMain::isStopped(dMain::STOP_REASON_PERF)) {
            dMain::addStop(dMain::STOP_REASON_PERF);
        }
    }
}

// 80155938
void dScene_c::endPerf() {
    fn_802B8DF4();
}
