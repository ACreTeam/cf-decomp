// The HOME button menu (dHomeButton_c). See include/game/game/d_home_button.hpp.
#include <game/game/d_home_button.hpp>
#include <game/game/d_caution.hpp>
#include <game/game/d_ds_dl.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_wifi_err.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/mLib/m_pad.hpp>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/gfxe/eggDrawGX.h>
#include <lib/egg/gfxe/eggScreen.h>
#include <revolution/DVD.h>
#include <revolution/OS.h>
#include <revolution/SC.h>
#include <string.h>

#define HBM_MEM_SIZE 0x80000
#define HBM_SOUND_MEM_SIZE 0x18700
#define ICON_HALF_SIZE 28.0f
#define ICON_FADE_IN_END 250.0f // ms
#define ICON_FADE_OUT_START 1250.0f
#define ICON_END 1500.0f
#define ICON_FADE_RATE (1.0f / 250.0f)

static dHomeButton_c *sInstance;

static inline f32 clampAlpha(f32 alpha) {
    return alpha < 0.0f ? 0.0f : (alpha > 1.0f ? 1.0f : alpha);
}

// 8017D89C
void dHomeButton_c::create(EGG::Heap *heap) {
    sInstance = new (heap, 4) dHomeButton_c(heap);
}

// 8017D8E8
dHomeButton_c *dHomeButton_c::getInstance() {
    return sInstance;
}

// 8017D8F0
dHomeButton_c::dHomeButton_c(EGG::Heap *heap) : mHeap(heap) {
    mSoundData = NULL;
    mSoundBuf = NULL;
    mLayoutName = NULL;
    mSpeakerSeName = NULL;
    mMsgName = NULL;
    mConfigName = NULL;
    mIconName = NULL;
    mSoundName = NULL;
    mState = STATE_LOAD;
    mCloseType = CLOSE_NONE;
    mFlags = FLAG_BOOT;
    mIconTick = 0;
    mIconAlpha = 0.0f;
    initDataInfo();
    initControllerData();
    mCoreStatus.init();
    setLayoutName();
    setSpeakerSeName();
    setMsgName();
    setConfigName();
    setIconName();
    setSoundName();
    mSoundBuf = new (heap, 0x20) u8[HBM_SOUND_MEM_SIZE];
}

// 8017DA00
BOOL dHomeButton_c::load() {
    if (mDataInfo.layoutBuf == NULL) {
        mDataInfo.layoutBuf = mLayoutLoader.request(mLayoutName, 0, mHeap);
    }
    if (mDataInfo.spkSeBuf == NULL) {
        mDataInfo.spkSeBuf = mSpeakerSeLoader.request(mSpeakerSeName, 0, mHeap);
    }
    if (mDataInfo.msgBuf == NULL) {
        mDataInfo.msgBuf = mMsgLoader.request(mMsgName, 0, mHeap);
    }
    if (mDataInfo.configBuf == NULL) {
        mDataInfo.configBuf = mConfigLoader.request(mConfigName, 0, mHeap);
        mDataInfo.configBufSize = mConfigLoader.getStatus();
    }
    if (!mIconTex.IsValid()) {
        void *data = mIconLoader.request(mIconName, 0, mHeap);
        if (data != NULL) {
            nw4r::g3d::ResFile file(data);
            mIconTex = file.GetResTex(0);
            mIconTex.Init();
        }
    }
    if (mSoundData == NULL) {
        mSoundData = mSoundLoader.request(mSoundName, 0, mHeap);
    }
    BOOL loaded = FALSE;
    if (mDataInfo.layoutBuf != NULL && mDataInfo.spkSeBuf != NULL && mDataInfo.msgBuf != NULL &&
        mDataInfo.configBuf != NULL && mIconTex.IsValid() && mSoundData != NULL) {
        loaded = TRUE;
    }
    if (loaded) {
        HBMCreate(&mDataInfo);
        HBMCreateSound(mSoundData, mSoundBuf, HBM_SOUND_MEM_SIZE);
        HBMSetAdjustFlag(TRUE);
        HBMSetVIBlack(FALSE);
        HBMSetBlackOutColor(dReset::l_fadeColor.r, dReset::l_fadeColor.g, dReset::l_fadeColor.b);
        mState = STATE_READY;
    }
    return loaded;
}

// 8017DBD4
void dHomeButton_c::execute() {
    if (mState == STATE_READY) {
        updateFlags();
        if (isHomeTrigger()) {
            bool showIcon = canShowIcon();
            if (mFlags == 0) {
                open();
            } else if (showIcon) {
                startIcon();
            }
        }
    }
    if (mState == STATE_OPEN) {
        DVDGetDriveStatus();
        if (!dCaution::g_caution.mActive) {
            calcController();
            calcMenu();
        }
    }
    calcIcon();
}

// 8017DC8C
void dHomeButton_c::draw() {
    if (mState == STATE_OPEN) {
        setScreen();
        HBMDraw();
    }
}

// 8017DCBC
void dHomeButton_c::setScreen() {
    EGG::Screen screen;
    screen.SetCanvasMode(EGG::Frustum::CANVASMODE_0);
    screen.mProjType = EGG::Frustum::PROJ_ORTHO;
    screen.mNearZ = -10000.0f;
    screen.mFarZ = 10000.0f;
    screen.SetProjectionGX();
}

// 8017DD20
void dHomeButton_c::drawIcon() {
    if (mIconAlpha > 0.0f) {
        GXColor color;
        color.r = 0xFF;
        color.g = 0xFF;
        color.b = 0xFF;
        color.a = (u32)(255.5f * mIconAlpha);
        EGG::Screen screen;
        screen.SetCanvasMode(EGG::Frustum::CANVASMODE_0);
        screen.mProjType = EGG::Frustum::PROJ_ORTHO;
        screen.mNearZ = -10000.0f;
        screen.mFarZ = 10000.0f;
        screen.SetProjectionGX();

        nw4r::math::VEC2 pos(0.0f, 0.0f);
        EGG::DrawGX::BeginDrawScreen(false, true, false);
        EGG::DrawGX::LoadTexture(mIconTex, GX_TEXMAP0);

        f32 bottom = pos.y - ICON_HALF_SIZE;
        f32 top = pos.y + ICON_HALF_SIZE;
        f32 left = pos.x - ICON_HALF_SIZE;
        f32 right = pos.x + ICON_HALF_SIZE;
        nw4r::math::MTX34 mtx;
        screen.CalcMatrixForDrawQuad(&mtx, left, top, nw4r::math::FAbs(right - left), nw4r::math::FAbs(bottom - top));
        EGG::DrawGX::DrawDL(EGG::DrawGX::DL_16, mtx, color);
    }
}

// 8017DE40
void dHomeButton_c::initDataInfo() {
    memset(&mDataInfo, 0, sizeof(mDataInfo));
    mDataInfo.layoutBuf = NULL;
    mDataInfo.spkSeBuf = NULL;
    mDataInfo.msgBuf = NULL;
    mDataInfo.configBuf = NULL;
    mDataInfo.mem = new (mHeap, 4) u8[HBM_MEM_SIZE];
    mDataInfo.memSize = HBM_MEM_SIZE;
    mDataInfo.pAllocator = NULL;
    mDataInfo.sound_callback = NULL;
    mDataInfo.backFlag = FALSE;
    mDataInfo.region = 0;
    mDataInfo.cursor = 0;
    mDataInfo.messageFlag = 0;
    mDataInfo.configBufSize = 0;
    mDataInfo.frameDelta = 1.0f;
    mDataInfo.adjust.x = (f32)EGG::Screen::sTVModeInfo[EGG::Screen::sTVMode].width /
                         (f32)EGG::Screen::sTVModeInfo[EGG::Screen::TV_MODE_4_3].width;
    mDataInfo.adjust.y = 1.0f;
}

// 8017DF20
void dHomeButton_c::initControllerData() {
    memset(&mControllerData, 0, sizeof(mControllerData));
}

// 8017DF30
void dHomeButton_c::setLayoutName() {
    u8 lang = SCGetLanguage();
    const char *jpn = "HomeButton2/homeBtn.arc";
    const char *eng = "HomeButton2/homeBtn_ENG.arc";
    const char *ger = "HomeButton2/homeBtn_GER.arc";
    const char *fra = "HomeButton2/homeBtn_FRA.arc";
    const char *spa = "HomeButton2/homeBtn_SPA.arc";
    const char *ita = "HomeButton2/homeBtn_ITA.arc";
    const char *ned = "HomeButton2/homeBtn_NED.arc";
    mDataInfo.region = lang;
    if (lang == SC_LANG_JP) {
        mLayoutName = jpn;
    } else if (lang == SC_LANG_EN) {
        mLayoutName = eng;
    } else if (lang == SC_LANG_DE) {
        mLayoutName = ger;
    } else if (lang == SC_LANG_FR) {
        mLayoutName = fra;
    } else if (lang == SC_LANG_SP) {
        mLayoutName = spa;
    } else if (lang == SC_LANG_IT) {
        mLayoutName = ita;
    } else if (lang == SC_LANG_NL) {
        mLayoutName = ned;
    } else {
        mLayoutName = jpn;
        mDataInfo.region = SC_LANG_JP;
    }
}

// 8017E008
void dHomeButton_c::setSpeakerSeName() {
    mSpeakerSeName = "HomeButton2/SpeakerSe.arc";
}

// 8017E018
void dHomeButton_c::setMsgName() {
    mMsgName = "HomeButton2/home.csv";
}

// 8017E028
void dHomeButton_c::setConfigName() {
    mConfigName = "HomeButton2/config.txt";
}

// 8017E038
void dHomeButton_c::setIconName() {
    mIconName = "HomeButton2/HomeBtnIcon.brtex";
}

// 8017E048
void dHomeButton_c::setSoundName() {
    mSoundName = "HomeButton2/HomeButtonSe.arc";
}

// 8017E058
void dHomeButton_c::calcController() {
    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        HBMKPadData *pad = getKPadData(i);
        u32 type;
        s32 result = WPADProbe(i, &type);
        if (type == WPAD_DEV_CLASSIC) {
            pad->use_devtype = WPAD_DEV_CORE;
        } else {
            pad->use_devtype = type;
        }
        if (result == WPAD_ERR_NO_CONTROLLER) {
            pad->kpad = NULL;
        } else {
            EGG::CoreController *core = mPad::getCore(i);
            EGG::CoreStatus *status = core->getCoreStatus();
            if (core->mKPADReadLength > 0) {
                mCoreStatus = *status;
                pad->kpad = &mCoreStatus;
                if (status->dpd_valid_fg > 0) {
                    EGG::Vector2f pos = status->getPosition();
                    pad->pos.x = pos.x;
                    pad->pos.y = pos.y;
                }
            }
        }
    }
}

// 8017E250
bool dHomeButton_c::isHomeTrigger() {
    return mPad::g_currentCore->downTrigger(WPAD_BUTTON_HOME);
}

// 8017E26C
void dHomeButton_c::open() {
    HBMInit();
    stopRumble();
    resetIcon();
    sndStopReverb();
    mState = STATE_OPEN;
}

// 8017E2B0
void dHomeButton_c::calcMenu() {
    HBMUpdateSound();
    int select = HBMCalc(&mControllerData);
    if (select >= HBM_SELECT_HOMEBTN) {
        BOOL closed = FALSE;
        if (mCloseType != CLOSE_NONE) {
            if (mCloseType == CLOSE_RESET) {
                dReset::Manage_c::GetInstance()->SetHbmResetDone();
            } else {
                dReset::Manage_c::GetInstance()->SetHbmPowerDone();
            }
            mCloseType = CLOSE_NONE;
            closed = TRUE;
        } else if (select == HBM_SELECT_HOMEBTN) {
            sndStartReverb();
            closed = TRUE;
        } else if (select == HBM_SELECT_BTN1) {
            dReset::Manage_c::GetInstance()->SetHbmReturnMenu();
            closed = TRUE;
        } else if (select == HBM_SELECT_BTN2) {
            dReset::Manage_c::GetInstance()->SetHbmReset();
            closed = TRUE;
        }
        if (closed) {
            mState = STATE_READY;
        }
    }
}

// 8017E374
void dHomeButton_c::startIcon() {
    if (mIconTick == 0) {
        mIconTick = OSGetTick();
        mIconAlpha = 0.0f;
    }
}

// 8017E3B8
void dHomeButton_c::calcIcon() {
    if (mIconTick != 0) {
        f32 time = OS_TICKS_TO_MSEC(OSGetTick() - mIconTick);
        BOOL end = FALSE;
        if (time < 0.0f) {
            end = TRUE;
        } else if (time < ICON_FADE_IN_END) {
            mIconAlpha = ICON_FADE_RATE * time;
            mIconAlpha = clampAlpha(mIconAlpha);
        } else if (time < ICON_FADE_OUT_START) {
            mIconAlpha = 1.0f;
        } else if (time < ICON_END) {
            mIconAlpha = 1.0f - ICON_FADE_RATE * (time - ICON_FADE_OUT_START);
            mIconAlpha = clampAlpha(mIconAlpha);
        } else {
            end = TRUE;
        }
        if (end) {
            resetIcon();
        }
    }
}

// 8017E500
void dHomeButton_c::resetIcon() {
    mIconTick = 0;
    mIconAlpha = 0.0f;
}

// 8017E514
void dHomeButton_c::stopRumble() {
    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mPad::getCore(i)->stopRumbleMgr();
    }
}

// 8017E570
BOOL dHomeButton_c::requestClose(Close_e type) {
    BOOL ok = FALSE;
    if (mState == STATE_OPEN && mCloseType == CLOSE_NONE) {
        ok = TRUE;
    }
    if (ok) {
        mCloseType = type;
        HBMStartBlackOut();
    }
    return ok;
}

// 8017E5C8
void dHomeButton_c::updateFlags() {
    s32 driveStatus = DVDGetDriveStatus();
    BOOL discError = FALSE;
    if (driveStatus == DVD_STATE_FATAL || dNand::getResult() == dNand::RESULT_CORRUPT) {
        discError = TRUE;
    }
    setFlag(FLAG_DISC_ERROR, discError);
    setFlag(FLAG_CAUTION, dCaution::g_caution.mActive);
    setFlag(FLAG_RESETTING, dReset::Manage_c::GetInstance()->isResetMode());
    setFlag(FLAG_FADER, !mFader_c::isStatus(mFaderBase_c::HIDDEN));
    setFlag(FLAG_NAND_WRITING, dNand::isWriting());
    BOOL network = FALSE;
    if (dNet::isNetMgrRunning() || g_wifiErr.mActive) {
        network = TRUE;
    }
    setFlag(FLAG_NETWORK, network);
    setFlag(FLAG_DS_DOWNLOAD, (u32)dDsDl::g_manager.mStep < dDsDl::STEP_DONE);
}
