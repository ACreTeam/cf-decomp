// The reset manager. .text 8010693C..80107A28. See include/game/game/d_reset.hpp.
#include <game/game/d_reset.hpp>
#include <lib/egg/core/eggColorFader.h>
#include <lib/egg/core/eggHeap.h>
#include <game/mLib/m_video.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_ds_dl.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_home_button.hpp>
#include <game/game/d_home_room_map.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_rumble.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_prc_mng.hpp>
#include <game/game/d_reset_save.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_base.hpp>
#include <game/game/d_screenshot.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_sys.hpp>
#include <game/game/d_wifi.hpp>
#include <game/game/d_wifi_err.hpp>
#include <revolution/DVD.h>
#include <revolution/GX.h>
#include <revolution/OS.h>
#include <revolution/VI.h>

namespace dReset {

// 8074E638
EGG::Color l_fadeColor(0, 0, 0, 255);

Manage_c *Manage_c::sInstance;

// 80475C30
const Manage_c::ModeFunc Manage_c::sModeInit[] = {
    &Manage_c::ModeInit_Normal,   &Manage_c::ModeInit_SoftReset,  &Manage_c::ModeInit_HbmWait,
    &Manage_c::ModeInit_DiskWait, &Manage_c::ModeInit_FatalError, &Manage_c::ModeInit_SafetyWait,
};

// 80475C78
const Manage_c::ModeFunc Manage_c::sModeProc[] = {
    &Manage_c::ModeProc_Normal,   &Manage_c::ModeProc_SoftReset,  &Manage_c::ModeProc_HbmWait,
    &Manage_c::ModeProc_DiskWait, &Manage_c::ModeProc_FatalError, &Manage_c::ModeProc_SafetyWait,
};

// NAND, screenshots and WiiConnect24 are idle.
static inline bool isSystemIdle() {
    bool idle = false;
    if (dNand::isReady() && dScreenshot::isIdle() && dWifi::isIdle()) {
        idle = true;
    }
    return idle;
}

// ... and DS download play too: quitting may go ahead.
static inline bool isAllIdle() {
    bool idle = false;
    if (isSystemIdle() && dDsDl::g_manager.finish()) {
        idle = true;
    }
    return idle;
}

// 8010693C
void Manage_c::CreateInstance(EGG::Heap *heap) {
    sInstance = new (heap, 4) Manage_c(heap);
}

// 80106988
Manage_c *Manage_c::GetInstance() {
    return sInstance;
}

// 80106990
Manage_c::Manage_c(EGG::Heap *heap) {
    mpHeap = heap;
    mpFader = NULL;
    mMode = MODE_NORMAL;
    mModeInit = MODE_NORMAL;
    mPrevMode = MODE_NORMAL;
    mExec = EXEC_NONE;
    mDiskCheck = DISK_CHECK_BUSY;
    mHbmReset = false;
    mHbmReturnMenu = false;
    mResetButton = false;
    mPowerButton = false;
    mErrorReset = false;
    mReturnMenu = false;
    mDataManager = false;
    mErrorResetType = 0;
    mHbmResetClose = HBM_CLOSE_NONE;
    mHbmPowerClose = HBM_CLOSE_NONE;
    mSoftResetStep = SOFT_RESET_FADE;
    mBootComplete = false;
    mSoftResetPending = false;
    mSoftResetRequest = false;
    mFadeOutRequest = false;
    mResetDisable = false;
    Configure();
}

// 80106A24
void Manage_c::Configure() {
    mpFader = new (mpHeap, 4) EGG::ColorFader(0.0f, 0.0f, mVideo::m_video->mRenderModeObj.fbWidth,
                                              mVideo::m_video->mRenderModeObj.efbHeight, l_fadeColor,
                                              EGG::Fader::HIDDEN);
    SetResetCallback();
    SetPowerCallback();
}

// 80106ACC
void Manage_c::Reinit() {
    mMode = MODE_NORMAL;
    mModeInit = MODE_NORMAL;
    mExec = EXEC_NONE;
    mDiskCheck = DISK_CHECK_BUSY;
    SetResetCallback();
    SetPowerCallback();
    mHbmReset = false;
    mHbmReturnMenu = false;
    mResetButton = false;
    mPowerButton = false;
    mErrorReset = false;
    mReturnMenu = false;
    mDataManager = false;
    mErrorResetType = 0;
    mHbmResetClose = HBM_CLOSE_NONE;
    mHbmPowerClose = HBM_CLOSE_NONE;
    mSoftResetStep = SOFT_RESET_FADE;
    mSoftResetPending = false;
    mSoftResetRequest = false;
    mFadeOutRequest = false;
    mResetDisable = false;
}

// 80106B5C
void Manage_c::Calculate() {
    BootCompleteCheck();
    ModeCalc();
    mpFader->calc();
}

// 80106BA4
void Manage_c::Draw() {
    mpFader->draw();
}

// 80106BB8
void Manage_c::BootComplete(bool complete) {
    mBootComplete = complete;
}

// 80106BC0
bool Manage_c::IsSafeToChangeScene() const {
    bool idle = false;
    bool safe = false;
    bool allIdle = false;
    if (!mFadeOutRequest && dNand::isReady() && dScreenshot::isIdle() && dWifi::isIdle()) {
        idle = true;
    }
    if (idle && dDsDl::g_manager.finish()) {
        allIdle = true;
    }
    if (allIdle && dHomeButton_c::getInstance()->mState != dHomeButton_c::STATE_OPEN) {
        safe = true;
    }
    return safe;
}

// 80106C74
void Manage_c::SetSoftResetFinish() {
    if (mMode == MODE_SOFT_RESET && mSoftResetStep == SOFT_RESET_SCENE) {
        fBase_c *parent = dScene::getSceneParent();
        u8 scene = dScene::getCurrentScene();
        if (parent == NULL || scene != dScene::SCENE_DM_LOAD) {
            mSoftResetStep = SOFT_RESET_FINISH;
        }
    }
}

// 80106CE4
bool Manage_c::SetSoftResetScene() {
    if (isSoftResetFade()) {
        if (IsSafeToChangeScene()) {
            if (mErrorResetType == 1) {
                dScene_c::m_nextScene = SCENE_PROF_ERROR;
                dScene_c::m_fadeInType = 2;
                dScene_c::m_fadeOutType = 2;
            } else if (dScene_c::m_nowScene != NULL && dScene_c::m_nowScene->getKind() == SCENE_KIND_BOOT) {
                if (mBootComplete) {
                    dScene::forceSceneChange(dScene::SCENE_DM_TITLE, 0, 0);
                } else {
                    dScene_c::m_nextScene = SCENE_PROF_BOOT;
                    dScene_c::m_fadeInType = 0;
                    dScene_c::m_fadeOutType = 0;
                }
            } else {
                dScene::forceSceneChange(dScene::SCENE_DM_LOAD, 0, 5);
            }
            mSoftResetStep = SOFT_RESET_SCENE;
            return true;
        }
        return false;
    }
    return false;
}

// 80106DF0
void Manage_c::PostDeleteScene() {
    if (mMode == MODE_SOFT_RESET && mSoftResetPending) {
        dDemo::reset();
        dNand::reset();
        dPrcMng_c::clear();
        dSvMgr_c::cancelRequest();
        fgMngProc_resetSync();
        dResetSave::clear();
        dSTR::resetLocks();
        dSTR::getBrresBank()->clear();
        clearAllRoomMaps();
        dFTR::resetHold();
        dSTR::busInfo_c *bus = dSTR::getBusInfo();
        bus->mType = 0;
        dNpcMng_c::reset();
        dSaveOption::reset(&dSaveData_c::sOption);
        dPlayerMgr::reset();
        dBgm::l_mgr.init();
        dWifi::reset();
        mSoftResetPending = false;
    }
}

// 80106E94
bool Manage_c::IsFadeOutDone() const {
    return mpFader->getStatus() == EGG::Fader::OPAQUE;
}

// 80106ECC
void Manage_c::SetHbmReturnMenu() {
    mHbmReturnMenu = true;
}

// 80106ED8
void Manage_c::SetHbmReset() {
    mHbmReset = true;
}

// 80106EE4
void Manage_c::SetHbmResetDone() {
    mHbmResetClose = HBM_CLOSE_DONE;
}

// 80106EF0
void Manage_c::SetHbmPowerDone() {
    mHbmPowerClose = HBM_CLOSE_DONE;
}

// 80106EFC
void Manage_c::RequestErrorReset(int type) {
    mErrorResetType = type;
    mErrorReset = true;
}

// 80106F0C
void Manage_c::RequestDataManager() {
    mDataManager = true;
}

// 80106F18
void Manage_c::SetResetDisable() {
    mResetDisable = true;
}

// 80106F24
void Manage_c::SetResetEnable() {
    mResetDisable = false;
}

// 80106F30
void Manage_c::FinalizeExec() {
    FinalizeGraphics();
    fn_8000E868();
    FinalizeCache();
}

// 80106F68
void Manage_c::Exec() const {
    if (mExec == EXEC_RESTART) {
        OSRestart(0);
    } else if (mExec == EXEC_REBOOT) {
        OSRebootSystem();
    } else if (mExec == EXEC_RETURN_MENU) {
        OSReturnToMenu();
    } else if (mExec == EXEC_DATA_MANAGER) {
        OSReturnToDataManager();
    } else if (mExec == EXEC_SHUTDOWN) {
        OSShutdownSystem();
    }
}

// 80106FB0
void Manage_c::BootCompleteCheck() {
    if (!mBootComplete && dSys::sBootComplete) {
        BootComplete(true);
    }
}

// 80106FD4
void Manage_c::DiskCheckReset() {
    bool bootScene = false;
    if (dScene_c::m_nowScene != NULL && dScene_c::m_nowScene->getKind() == SCENE_KIND_BOOT) {
        bootScene = true;
    }
    StartDiskCheck();
    while (true) {
        if (mDiskCheck == DISK_CHECK_FAIL) {
            SetSafetyWait(EXEC_RETURN_MENU);
            break;
        } else if (mDiskCheck == DISK_CHECK_OK) {
            if (mBootComplete || bootScene) {
                SetSoftReset();
            }
            break;
        }
        VIWaitForRetrace();
    }
}

// 80107088
void Manage_c::ModeLog(const char *name) const {}

// 8010708C
void Manage_c::ModeCalc() {
    bool fatal = IsFatalError();
    bool diskError = IsDiskError();
    if (fatal) {
        if (mMode != MODE_FATAL_ERROR) {
            SetNextMode(MODE_FATAL_ERROR);
        }
    } else if (diskError) {
        if (mMode != MODE_DISK_WAIT && mMode != MODE_SAFETY_WAIT) {
            SetNextMode(MODE_DISK_WAIT);
        }
    } else if (mMode == MODE_NORMAL && dHomeButton_c::getInstance()->mState == dHomeButton_c::STATE_OPEN) {
        SetNextMode(MODE_HBM_WAIT);
    }

    if (mModeInit != MODE_NONE) {
        mPrevMode = mMode;
        mMode = mModeInit;
        mModeInit = MODE_NONE;
        (this->*sModeInit[mMode])();
    }
    FadeOutCalc();
    (this->*sModeProc[mMode])();

    if (mResetButton) {
        SetResetCallback();
    }
    if (mPowerButton) {
        SetPowerCallback();
    }
    mHbmReset = false;
    mHbmReturnMenu = false;
    mResetButton = false;
    mPowerButton = false;
    mErrorReset = false;
    mReturnMenu = false;
    mDataManager = false;
}

// 801071EC
void Manage_c::SetNextMode(int mode) {
    mModeInit = mode;
}

// 801071F4
void Manage_c::SetSafetyWait(int exec) {
    mExec = exec;
    SetNextMode(MODE_SAFETY_WAIT);
    StopRumble();
}

// 80107230
void Manage_c::SetSoftReset() {
    mSoftResetRequest = true;
    SetNextMode(MODE_SOFT_RESET);
    StopRumble();
}

// 80107270
void Manage_c::ModeInit_Normal() {
    ModeLog("Normal");
    mExec = EXEC_NONE;
}

// 801072A8
void Manage_c::ModeProc_Normal() {
    if (mPowerButton) {
        SetSafetyWait(EXEC_SHUTDOWN);
    } else if (g_wifiErr.mActive && mResetButton) {
        g_wifiErr.mResetRequest = TRUE;
    } else if (mReturnMenu) {
        SetSafetyWait(EXEC_RETURN_MENU);
    } else if (mDataManager) {
        SetSafetyWait(EXEC_DATA_MANAGER);
    } else if (mResetButton || mErrorReset) {
        DiskCheckReset();
    }
}

// 80107330
void Manage_c::ModeInit_SoftReset() {
    ModeLog("SoftReset");
    dResetSave::request();
    if (mSoftResetRequest) {
        mSoftResetRequest = false;
        mSoftResetStep = SOFT_RESET_FADE;
        mSoftResetPending = true;
        RequestFadeOut();
    }
}

// 80107390
void Manage_c::ModeProc_SoftReset() {
    if (IsFadeOutDone() && mSoftResetStep == SOFT_RESET_FINISH) {
        mpFader->setStatus(EGG::Fader::HIDDEN);
        VISetBlack(FALSE);
        ResumeRumble();
        Reinit();
    }
}

// 80107400
void Manage_c::ModeInit_HbmWait() {
    ModeLog("HbmWait");
}

// 80107408
void Manage_c::ModeProc_HbmWait() {
    bool fade = false;
    if (mHbmPowerClose == HBM_CLOSE_NONE && mHbmResetClose == HBM_CLOSE_NONE) {
        if (mHbmReturnMenu) {
            SetSafetyWait(EXEC_RETURN_MENU);
        } else if (mHbmReset) {
            DiskCheckReset();
            fade = true;
        } else if (dHomeButton_c::getInstance()->mState != dHomeButton_c::STATE_OPEN) {
            SetNextMode(MODE_NORMAL);
        } else if (mPowerButton) {
            if (dHomeButton_c::getInstance()->requestClose(dHomeButton_c::CLOSE_POWER)) {
                mHbmPowerClose = HBM_CLOSE_REQUEST;
            }
        } else if (mResetButton) {
            if (dHomeButton_c::getInstance()->requestClose(dHomeButton_c::CLOSE_RESET)) {
                mHbmResetClose = HBM_CLOSE_REQUEST;
            }
        }
    } else if (mHbmPowerClose == HBM_CLOSE_DONE) {
        mHbmPowerClose = HBM_CLOSE_NONE;
        SetSafetyWait(EXEC_SHUTDOWN);
        fade = true;
    } else if (mHbmResetClose == HBM_CLOSE_DONE) {
        mHbmResetClose = HBM_CLOSE_NONE;
        DiskCheckReset();
        fade = true;
    }
    if (fade) {
        mpFader->setStatus(EGG::Fader::OPAQUE);
    }
}

// 80107560
void Manage_c::ModeInit_DiskWait() {
    ModeLog("DiskWait");
}

// 8010756C
void Manage_c::ModeProc_DiskWait() {
    if (IsDiskError()) {
        if (mPowerButton) {
            SetSafetyWait(EXEC_SHUTDOWN);
        } else if (mResetButton) {
            SetSafetyWait(EXEC_RETURN_MENU);
        }
    } else {
        SetNextMode(mPrevMode);
    }
}

// 801075E4
void Manage_c::ModeInit_FatalError() {
    ModeLog("FatalError");
}

// 801075F0
void Manage_c::ModeProc_FatalError() {}

// 801075F4
void Manage_c::ModeInit_SafetyWait() {
    ModeLog("SafetyWait");
    RequestFadeOut();
}

// 80107630
void Manage_c::ModeProc_SafetyWait() {
    if (IsFadeOutDone() && isAllIdle()) {
        FinalizeExec();
        Exec();
    }
}

// 801076DC
void Manage_c::SetResetCallback() {
    OSSetResetCallback(ResetCallback);
}

// 801076E8
void Manage_c::SetPowerCallback() {
    OSSetPowerCallback(PowerCallback);
}

// 801076F4
void Manage_c::StartDiskCheck() {
    DVDCheckDiskAsync(&mDiskCheckBlock, DiskCheckCallback);
}

// 80107704
void Manage_c::DiskCheckCallback(s32 result, DVDCommandBlock *block) {
    if (result == 0) {
        GetInstance()->mDiskCheck = DISK_CHECK_FAIL;
    } else {
        GetInstance()->mDiskCheck = DISK_CHECK_OK;
    }
}

// 8010774C
void Manage_c::ResetCallback() {
    GetInstance()->mResetButton = true;
}

// 8010777C
void Manage_c::PowerCallback() {
    GetInstance()->mPowerButton = true;
}

// 801077AC
void Manage_c::FinalizeGraphics() {
    VISetBlack(TRUE);
    VIFlush();
    VIWaitForRetrace();
    VIWaitForRetrace();
    OSThread *gxThread = GXGetCurrentGXThread();
    BOOL enabled = OSDisableInterrupts();
    if (gxThread != OSGetCurrentThread()) {
        OSCancelThread(gxThread);
        GXSetCurrentGXThread();
    }
    GXFlush();
    GXAbortFrame();
    GXDrawDone();
    OSRestoreInterrupts(enabled);
    GXSetDrawDoneCallback(NULL);
    VIWaitForRetrace();
}

// 80107834
void Manage_c::FinalizeCache() {
    LCDisable();
}

// 80107838
void Manage_c::StopRumble() {
    for (int i = 0; i < 4; i++) {
        mPad::getCore(i)->stopRumbleMgr();
    }
    dRumble::disable();
}

// 80107898
void Manage_c::ResumeRumble() {
    dRumble::enable();
}

// 8010789C
void Manage_c::RequestFadeOut() {
    mFadeOutRequest = true;
}

// 801078A8
void Manage_c::FadeOutCalc() {
    if (mFadeOutRequest && !mResetDisable && dNand::isReady() && dScreenshot::isIdle()) {
        mpFader->setFrame(30);
        mpFader->fadeOut();
        sndResetStart(30);
        dBgm::l_mgr.mute(0, 30, 0);
        mFadeOutRequest = false;
    }
}

// 80107948
bool Manage_c::IsFatalError() {
    s32 status = DVDGetDriveStatus();
    bool nandFatal = dNand::getResult() == 2;
    bool fatal = false;
    if (status == DVD_STATE_FATAL || nandFatal) {
        fatal = true;
    }
    return fatal;
}

static inline BOOL isDiskErrorStatus(s32 status) {
    return status == DVD_STATE_NO_DISK || status == DVD_STATE_WRONG_DISK_ID || status == DVD_STATE_DISK_ERROR;
}

// 8010799C
bool Manage_c::IsDiskError() {
    return isDiskErrorStatus(DVDGetDriveStatus());
}

} // namespace dReset
