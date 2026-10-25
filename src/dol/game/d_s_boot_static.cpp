// The boot scene's static part. .text 80107F00..8010A120. See
// include/game/game/d_s_boot_static.hpp.
#include <game/game/d_s_boot_static.hpp>
#include <lib/egg/core/eggColorFader.h>
#include <lib/egg/core/eggHeap.h>
#include <revolution/sc.h>
#include <game/mLib/m_dvd.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/mLib/m_face.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_video.hpp>
#include <game/sLib/s_phase.hpp>
#include <game/game/d_activity_log.hpp>
#include <game/game/d_banner.hpp>
#include <game/game/d_bg.hpp>
#include <game/game/d_black_wait.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_fg_data.hpp>
#include <game/game/d_fg_mng_task.hpp>
#include <game/game/d_fg_obj.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_font.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_hmn_body_mng.hpp>
#include <game/game/d_hmn_face_anm_mng.hpp>
#include <game/game/d_hmn_face_tex_mng.hpp>
#include <game/game/d_hmn_head_mng.hpp>
#include <game/game/d_hmn_palette_mng.hpp>
#include <game/game/d_home_button.hpp>
#include <game/game/d_home_room_map.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_debug.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_mii_icon.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_mdl_mng.hpp>
#include <game/game/d_npc_setup.hpp>
#include <game/game/d_paper.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_prc_mng.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_shop_layout.hpp>
#include <game/game/d_shutter_fader.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_sv_auc.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_sv_proc.hpp>
#include <game/game/d_sys.hpp>
#include <game/game/d_uki.hpp>
#include <game/game/d_wifi.hpp>

static const char *l_fgItemName = "/FgObj/fgItem.brres";

inline BOOL dFgObj::alwaysBank_c::load(void *heap) {
    return bank_c::load(l_fgItemName, heap, 0);
}

// 80107F00
static sPhase_c::METHOD_RESULT_e loadHomeButton(void *) {
    sPhase_c::METHOD_RESULT_e result = sPhase_c::WAIT;
    if (dHomeButton_c::getInstance()->load()) {
        result = sPhase_c::OK;
    }
    return result;
}

// 80107F40
static bool phaseHomeButton() {
    static sPhase_c::phaseMethod methods[] = {loadHomeButton};
    static sPhase_c phase(methods, 1);
    if (phase.callMethod(NULL) == sPhase_c::DONE) {
        return true;
    }
    return false;
}

// 80107FA0
static sPhase_c::METHOD_RESULT_e loadBlackWait(void *) {
    sPhase_c::METHOD_RESULT_e result = sPhase_c::WAIT;
    if (dBlackWait::BlackWait_c::getInstance()->load()) {
        result = sPhase_c::OK;
    }
    return result;
}

// 80107FE0
static bool phaseBlackWait() {
    static sPhase_c::phaseMethod methods[] = {loadBlackWait};
    static sPhase_c phase(methods, 1);
    if (phase.callMethod(NULL) == sPhase_c::DONE) {
        return true;
    }
    return false;
}

// 80108040
static sPhase_c::METHOD_RESULT_e loadFace(void *) {
    static mDvd_toMainRam_c *sCmd;
    mDvd_toMainRam_c *cmd = sCmd;
    if (cmd == NULL) {
        cmd = sCmd = mDvd_toMainRam_c::create("/Face/RFLRes01.arc", 0,
                                              dHeap::faceResHeap_p);
        if (cmd == NULL) {
            return sPhase_c::WAIT;
        }
    }
    if (cmd->mDone != TRUE) {
        return sPhase_c::WAIT;
    }
    if (!mFace::initRes(sCmd->mpData, dHeap::faceResHeap_p, FALSE)) {
        dSys::sFaceError = 1;
    }
    sCmd->destroy();
    sCmd = NULL;
    dMiiIcon::create(dHeap::faceResHeap_p);
    return sPhase_c::OK;
}

// 801080E4
static bool phaseFace() {
    static sPhase_c::phaseMethod methods[] = {loadFace};
    static sPhase_c phase(methods, 1);
    if (phase.callMethod(NULL) == sPhase_c::DONE) {
        return true;
    }
    return false;
}

// 80108144
static sPhase_c::METHOD_RESULT_e loadNothing(void *) {
    return sPhase_c::OK;
}

// 8010814C
static bool phaseNothing() {
    static sPhase_c::phaseMethod methods[] = {loadNothing};
    static sPhase_c phase(methods, 1);
    if (phase.callMethod(NULL) == sPhase_c::DONE) {
        return true;
    }
    return false;
}

// 801081AC
static sPhase_c::METHOD_RESULT_e loadShutter(void *) {
    static mDvd_toMainRam_c *sCmd;
    mDvd_toMainRam_c *cmd = sCmd;
    if (cmd == NULL) {
        cmd = sCmd = mDvd_toMainRam_c::create("/Photo/shutter.brtex", 0,
                                              dHeap::localHeap_p);
        if (cmd == NULL) {
            return sPhase_c::WAIT;
        }
    }
    if (cmd->mDone != TRUE) {
        return sPhase_c::WAIT;
    }
    void *data = sCmd->mpData;
    dShutterFader_c::loadTexture(data);
    sCmd->destroy();
    sCmd = NULL;
    dHeap::localHeap_p->free(data);
    return sPhase_c::OK;
}

// 80108254
static bool phaseShutter() {
    static sPhase_c::phaseMethod methods[] = {loadShutter};
    static sPhase_c phase(methods, 1);
    if (phase.callMethod(NULL) == sPhase_c::DONE) {
        return true;
    }
    return false;
}

// 801082B4
static sPhase_c::METHOD_RESULT_e loadEffect(void *) {
    static mDvd_toMainRam_c *sBreffCmd;
    static mDvd_toMainRam_c *sBreftCmd;
    if (sBreffCmd == NULL) {
        sBreffCmd = mDvd_toMainRam_c::create("/Effect/wii_animal.breff", 0,
                                             dHeap::effectResHeap_p);
        if (sBreffCmd == NULL) {
            return sPhase_c::WAIT;
        }
    }
    if (sBreftCmd == NULL) {
        sBreftCmd = mDvd_toMainRam_c::create("/Effect/wii_animal.breft", 0,
                                             dHeap::effectResHeap_p);
        if (sBreftCmd == NULL) {
            return sPhase_c::WAIT;
        }
    }
    if (sBreffCmd->mDone != TRUE || sBreftCmd->mDone != TRUE) {
        return sPhase_c::WAIT;
    }
    dEffect::initialize();
    mEf::setResource(sBreffCmd->mpData, sBreftCmd->mpData);
    sBreffCmd->destroy();
    sBreffCmd = NULL;
    sBreftCmd->destroy();
    sBreftCmd = NULL;
    return sPhase_c::OK;
}

// 8010839C
static bool phaseEffect() {
    static sPhase_c::phaseMethod methods[] = {loadEffect};
    static sPhase_c phase(methods, 1);
    if (phase.callMethod(NULL) == sPhase_c::DONE) {
        return true;
    }
    return false;
}

static dScBootStatic_c l_bootStatic;

// 801083FC
static sPhase_c::METHOD_RESULT_e boot(void *) {
    BOOL done = TRUE;
    done &= phaseHomeButton();
    done &= phaseBlackWait();
    done &= phaseFace();
    done &= phaseNothing();
    done &= phaseShutter();
    done &= phaseEffect();
    done &= dFont::load();
    done &= dPrcMng_c::load();
    done &= dFgData_c::load();
    done &= dHmnAnm_c::load();
    done &= dHmnHeadMng_c::load();
    done &= dHmnPaletteMng_c::load();
    done &= dHmnFaceTexMng_c::load();
    done &= dHmnBodyMng_c::load();
    done &= dHmnFaceAnmMng_c::load();
    done &= dBG::getAlwaysBank()->load(dHeap::bgAlwaysHeap_p);
    done &= dSTR::getColBank()->load(dHeap::strColHeap_p);
    done &= dItem::infoBank_c::get()->load(dHeap::itemInfoHeap_p);
    done &= dItem::dsnPlttLoader_c::get()->load(dHeap::dsnPlttHeap_p);
    done &= dFTR::alwaysBank_c::get()->load(dHeap::ftrAlwaysHeap_p);
    done &= dFgObj::getAlwaysBank()->load(dHeap::fgAlwaysHeap_p);
    done &= dScript::loadBank(dScript::getBank(), dHeap::scriptHeap_p);
    done &= g_banner.loadArc(dHeap::bannerHeap_p);
    done &= dNpcMdlMng_c::load();
    done &= dNpcSetup::load();
    done &= dUki_c::loadRes();
    done &= l_bootStatic.mWc24Check.execute();
    if (!done) {
        return sPhase_c::WAIT;
    }
    dSaveData_c::create();
    done = TRUE;
    done &= dField::createSaveFields();
    done &= l_bootStatic.mLoadSave.execute();
    if (!done) {
        return sPhase_c::WAIT;
    }
    done = TRUE;
    done &= l_bootStatic.mPrcInit.execute();
    done &= l_bootStatic.mInitSave.execute();
    if (!done) {
        return sPhase_c::WAIT;
    }
    g_banner.build();
    dNpc::initDaubManagers();
    clearAllRoomMaps();
    dSceneCheck::fn_80081850();
    dSceneCheck::fn_80081858(dSceneCheck::fn_80081860());
    dSceneCheck::fn_80081858(dScript::fn_8015B618());
    dSceneCheck::fn_80081858(dItemDebug::loadRes());
    dSceneCheck::fn_80081858(dItemDebug::checkRes());
    dSceneCheck::fn_80081858(dBG::isBusy());
    dSceneCheck::fn_80081858(dItemSel::fn_800C769C());
    dSceneCheck::fn_80081858(fgMngTask_isBusy());
    dSceneCheck::fn_80081858(dActivityLog::isBusy());
    dSceneCheck::fn_80081858(dSTR::fn_8016911C());
    dSceneCheck::fn_80081858(fn_8015FD10());
    dSceneCheck::fn_80081858(fn_8015FD18());
    fn_800F59C8();
    dSceneCheck::fn_80081858(fn_8010DBFC());
    dSceneCheck::fn_80081858(fn_800F59CC());
    dSceneCheck::fn_80081858(fn_800F59D4());
    dSceneCheck::fn_80081858(dPlayerMgr::isBusy());
    dSceneCheck::fn_80081858(dSvAuc_c::isBusy());
    dSceneCheck::fn_80081858(dHR::isBusy());
    dSceneCheck::fn_80081858(dNand::fn_800D3574());
    dSceneCheck::fn_80081858(dNand::fn_800D357C());
    dSceneCheck::fn_80081858(dEvent::fn_8008AEE8());
    dSceneCheck::fn_80081858(FALSE);
    dSceneCheck::fn_80081858(FALSE);
    dPaper::fn_800FACBC();
    return sPhase_c::OK;
}

static sPhase_c::phaseMethod l_bootMethods[] = {boot};
static sPhase_c l_bootPhase(l_bootMethods, 1);

// 80108698
dScBootStatic_c *dScBootStatic_c::getInstance() {
    return &l_bootStatic;
}

// 801086A4
BOOL dScBootStatic_c::isBooting() {
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    if (l_bootStatic.mEsrb.mDone && l_bootStatic.mStrap.mDone) {
        return FALSE;
    }
#else
    if (l_bootStatic.mStrap.mDone) {
        return FALSE;
    }
#endif
    return TRUE;
}

// 801086D4
dScBootStatic_c::dScBootStatic_c() {
    mState = STATE_SHOW;
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    mScreen = SCREEN_STRAP;
    mpFader = NULL;
#endif
}

// 8010875C
BOOL dScBootStatic_c::create() {
    BOOL created = TRUE;
    if (!mStrap.create()) {
        created = FALSE;
    }
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    if (!mEsrb.create()) {
        created = FALSE;
    }
#endif
    if (!created) {
        return FALSE;
    }
    mState = STATE_SHOW;
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    mpFader = new (-4)
        EGG::ColorFader(0.0f, 0.0f, mVideo::m_video->mRenderModeObj.fbWidth,
                        mVideo::m_video->mRenderModeObj.efbHeight,
                        nw4r::ut::Color(0x000000FF), EGG::Fader::HIDDEN);
#endif
    return TRUE;
}

static inline BOOL isRemoved(BOOL esrbRemoved, bool strapRemoved) {
    return esrbRemoved && strapRemoved;
}

// 80108840
BOOL dScBootStatic_c::doDelete() {
    sPhase_c::METHOD_RESULT_e result = l_bootPhase.callMethod(NULL);
    BOOL strapResult = mStrap.remove();
    bool strapRemoved = false;
    if (strapResult) {
        strapRemoved = true;
    }
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    if (!isRemoved(mEsrb.remove(), strapRemoved)) {
        return FALSE;
    }
    if (mEsrb.mDone && mStrap.mDone) {
#else
    if (!strapRemoved) {
        return FALSE;
    }
    if (mStrap.mDone) {
#endif
        if (result != sPhase_c::DONE) {
            return FALSE;
        }
        dHomeButton_c::getInstance()->mFlags &= ~dHomeButton_c::FLAG_BOOT;
    }
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    if (mpFader != NULL) {
        mHeap::g_gameHeap->free(mpFader);
        mpFader = NULL;
    }
#endif
    return TRUE;
}

// 80108934
void dScBootStatic_c::preExecute() {
    l_bootPhase.callMethod(NULL);
}

// 80108940
void dScBootStatic_c::execute() {
    l_bootPhase.callMethod(NULL);
    if (mState == STATE_END) {
        dScene::requestSceneChange(dScene::SCENE_DM_TITLE, 0, 2);
    }
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    else if (mState == STATE_FADE_OUT) {
        mpFader->setFrame(30);
        mpFader->fadeOut();
        mState = STATE_FADING;
    } else if (mpFader->getStatus() == EGG::Fader::OPAQUE) {
        mScreen = SCREEN_ESRB;
        mpFader->setFrame(30);
        mpFader->fadeIn();
    }
    if (mScreen == SCREEN_STRAP) {
        mStrap.execute();
    }
    if (mScreen == SCREEN_ESRB) {
        mEsrb.execute();
    }
    mpFader->calc();
    if (mEsrb.mDone && mStrap.mDone && !dSys::sBootComplete) {
        dSys::sBootComplete = true;
    }
#else
    mStrap.execute();
    if (mStrap.mDone && !dSys::sBootComplete) {
        dSys::sBootComplete = true;
    }
#endif
}

// 80108A74
void dScBootStatic_c::draw() {
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    if (mScreen == SCREEN_STRAP) {
        mStrap.draw();
    } else {
        mEsrb.draw();
    }
    mpFader->draw();
#else
    mStrap.draw();
#endif
}

// 80108AD0
dScBootStatic_c::StrapScreen_c::StrapScreen_c() {
    mCreated = false;
    mDone = false;
    mTimer = 0;
    mPressed = false;
}

// 80108B24
BOOL dScBootStatic_c::StrapScreen_c::create() {
    if (mDone) {
        return TRUE;
    }
    if (!mCreated) {
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
        const char *arcName = NULL;
        const char *lytName = NULL;
        const char *anmName = NULL;
        switch (getLanguage()) {
        case LANGUAGE_US:
            arcName = "Strap/strapSampleUS.arc";
            lytName = "strap_00_U.brlyt";
            anmName = "strap_00_U_roop.brlan";
            break;
        case LANGUAGE_QC:
            arcName = "Strap/strapSampleFR.arc";
            lytName = "strap_00_F.brlyt";
            anmName = "strap_00_F_roop.brlan";
            break;
        case LANGUAGE_MX:
            arcName = "Strap/strapSampleSP.arc";
            lytName = "strap_00_S.brlyt";
            anmName = "strap_00_S_roop.brlan";
            break;
        }
#elif VERSION == VERSION_RUUP01_00 || VERSION == VERSION_RUUP01_01
        const char *arcName;
        const char *lytName;
        const char *anmName;
        switch (SCGetLanguage()) {
        case SC_LANG_ENGLISH:
            arcName = "Strap/strapSampleUK.arc";
            lytName = "strap_00_E.brlyt";
            anmName = "strap_00_E_roop.brlan";
            break;
        case SC_LANG_SPANISH:
            arcName = "Strap/strapSampleSP.arc";
            lytName = "strap_00_S.brlyt";
            anmName = "strap_00_S_roop.brlan";
            break;
        case SC_LANG_FRENCH:
            arcName = "Strap/strapSampleFR.arc";
            lytName = "strap_00_F.brlyt";
            anmName = "strap_00_F_roop.brlan";
            break;
        case SC_LANG_ITALIAN:
            arcName = "Strap/strapSampleIT.arc";
            lytName = "strap_00_I.brlyt";
            anmName = "strap_00_I_roop.brlan";
            break;
        case SC_LANG_GERMAN:
            arcName = "Strap/strapSampleGE.arc";
            lytName = "strap_00_G.brlyt";
            anmName = "strap_00_G_roop.brlan";
            break;
        case SC_LANG_DUTCH:
            arcName = "Strap/strapSampleDU.arc";
            lytName = "strap_00_D.brlyt";
            anmName = "strap_00_D_roop.brlan";
            break;
        default:
            arcName = "Strap/strapSampleUK.arc";
            lytName = "strap_00_E.brlyt";
            anmName = "strap_00_E_roop.brlan";
            break;
        }
#elif VERSION == VERSION_RUUJ01_01 || VERSION == VERSION_RUUJ01_02
        const char *arcName = "Strap/strapSample.arc";
        const char *lytName = "strap_00_J.brlyt";
        const char *anmName = "strap_00_J_roop.brlan";
#else
        const char *arcName = "Strap/strapSampleKR.arc";
        const char *lytName = "strap_00_K.brlyt";
        const char *anmName = "strap_00_K_roop.brlan";
#endif
        if (!mResAcc.load(arcName)) {
            return FALSE;
        }
        mLyt.mpResAcc = &mResAcc;
        mLyt.build(lytName, NULL);
        mAnm.create(anmName, &mResAcc);
        mLyt.bind(&mAnm);
        mAnm.play();
        mLyt.calc();
        mCreated = true;
    } else {
        mAnm.setFrame(0.0f);
        mAnm.play();
        mLyt.calc();
    }
    mTimer = 0;
    mPressed = false;
    return TRUE;
}

// 80108C94
BOOL dScBootStatic_c::StrapScreen_c::remove() {
    if (mCreated && mDone) {
        mResAcc.remove();
        mCreated = false;
    }
    return TRUE;
}

// 80108CE4
void dScBootStatic_c::StrapScreen_c::execute() {
    if (!mPressed && mPad::g_currentCore->downTrigger(
                         WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT |
                         WPAD_BUTTON_DOWN | WPAD_BUTTON_UP | WPAD_BUTTON_2 |
                         WPAD_BUTTON_1 | WPAD_BUTTON_B | WPAD_BUTTON_A)) {
        mPressed = true;
    }
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        mAnm.play();
        mLyt.calc();
        if (++mTimer > 1200) {
            mTimer = 1200;
        }
        if (mTimer >= 60 && !mDone) {
            mDone = true;
        }
        if (mTimer >= 1200 || (mDone && mPressed)) {
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
            dScBootStatic_c *boot = &l_bootStatic;
            if (boot->mState != STATE_FADE_OUT &&
                boot->mState != STATE_FADING) {
                boot->mState = STATE_FADE_OUT;
            }
#else
            if (l_bootStatic.mState != STATE_END) {
                l_bootStatic.mState = STATE_END;
            }
#endif
        }
    }
}

// 80108E08
void dScBootStatic_c::StrapScreen_c::draw() {
    mLyt.entry();
}

#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
// 80108E10
dScBootStatic_c::EsrbScreen_c::EsrbScreen_c() {
    mCreated = false;
    mDone = false;
    mTimer = 0;
}

// 80108E60
BOOL dScBootStatic_c::EsrbScreen_c::create() {
    if (mDone) {
        return TRUE;
    }
    if (!mCreated) {
        const char *arcName = NULL;
        const char *lytName = NULL;
        const char *anmName = NULL;
        switch (getLanguage()) {
        case LANGUAGE_US:
            arcName = "Strap/ESRB_US.arc";
            lytName = "ESRB_Engish.brlyt";
            anmName = "ESRB_Engish.brlan";
            break;
        case LANGUAGE_QC:
            arcName = "Strap/ESRB_FR.arc";
            lytName = "ESRB_French.brlyt";
            anmName = "ESRB_French_ESRBanim.brlan";
            break;
        case LANGUAGE_MX:
            arcName = "Strap/ESRB_SP.arc";
            lytName = "ESRB_Spanish.brlyt";
            anmName = "ESRB_Spanish_ESRBanim.brlan";
            break;
        }
        if (!mResAcc.load(arcName)) {
            return FALSE;
        }
        mLyt.mpResAcc = &mResAcc;
        mLyt.build(lytName, NULL);
        mAnm.create(anmName, &mResAcc);
        mLyt.bind(&mAnm);
        mAnm.play();
        mLyt.calc();
        mCreated = true;
    } else {
        mAnm.setFrame(0.0f);
        mAnm.play();
        mLyt.calc();
    }
    mTimer = 0;
    return TRUE;
}

// 80108FCC
BOOL dScBootStatic_c::EsrbScreen_c::remove() {
    if (mCreated && mDone) {
        mResAcc.remove();
        mCreated = false;
    }
    return TRUE;
}

// 8010901C
void dScBootStatic_c::EsrbScreen_c::execute() {
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        mAnm.play();
        mLyt.calc();
        if (++mTimer > 90) {
            mTimer = 90;
            if (!mDone) {
                mDone = true;
            }
        }
        if (mDone) {
            dScBootStatic_c *boot = &l_bootStatic;
            if (boot->mState != STATE_END) {
                boot->mState = STATE_END;
            }
        }
    }
}

// 801090E0
void dScBootStatic_c::EsrbScreen_c::draw() {
    mLyt.entry();
}

#endif

// 801090E8
bool dScBootStatic_c::LoadSaveState_c::isEnd() {
    return mStep.isStep(&stepLoad_c::finish);
}

// 80109134
void dScBootStatic_c::LoadSaveState_c::initStep() {
    static const stepLoad_c::Entry steps[] = {
        {"ロード前", (stepLoad_c::Method)&stepLoad_c::prepareLoad},
        {"ロード初期化", (stepLoad_c::Method)&stepLoad_c::initializeLoad},
        {"ロード開始", (stepLoad_c::Method)&stepLoad_c::startLoad},
        {"ロード完了？", (stepLoad_c::Method)&stepLoad_c::waitLoad},
        {"ロード終了", (stepLoad_c::Method)&stepLoad_c::processLoadedSave},
        {"終了", (stepLoad_c::Method)&stepLoad_c::finish},
    };
    mStep.initialize(this, "dScBootStatic_c::LoadSaveState_c::stepLoad_c",
                     steps, 6);
}

// 80109264
BOOL dScBootStatic_c::LoadSaveState_c::execute() {
    if (!mStepReady) {
        initStep();
        mStepReady = true;
    }
    mStep.execute();
    if (isEnd()) {
        return TRUE;
    }
    return FALSE;
}

// 801092E0
void dScBootStatic_c::LoadSaveState_c::stepLoad_c::prepareLoad() {
    mpOwner->mLoadFailed = true;
    if (l_bootStatic.mWc24Check.mNetError) {
        this->mCurrentMethod = &stepLoad_c::finish;
    } else {
        nextStep();
    }
}

// 801093B8
void dScBootStatic_c::LoadSaveState_c::stepLoad_c::processLoadedSave() {
    switch (dNand::getResult()) {
    case dNand::RESULT_OK:
        if (!dSaveData_c::getRaw()->isExtraGood(1)) {
            sLoadErrorMessage = 2;
        } else if (!dSaveData_c::get()->isGood()) {
            sLoadErrorMessage = 2;
        } else {
            mpOwner->mLoadFailed = false;
        }
        break;
    case dNand::RESULT_NOEXISTS:
        break;
    case dNand::RESULT_SHORT:
    case dNand::RESULT_RETRY:
        sLoadErrorMessage = 2;
        break;
    case dNand::RESULT_CORRUPT:
        sLoadErrorMessage = 0x0C;
        break;
    default:
        sLoadErrorMessage = 0x12;
        break;
    }
    nextStep();
}

// 801094EC
void dScBootStatic_c::LoadSaveState_c::stepLoad_c::finish() {
}

// 801094F0
BOOL dScBootStatic_c::Wc24CheckState_c::execute() {
    BOOL done = FALSE;
    if (!mStarted) {
        if (dWifi::startCheck()) {
            mStarted = true;
        } else {
            return FALSE;
        }
    }
    switch (dWifi::getThreadResult()) {
    case -1:
        break;
    case 0:
        if (dWifi::getNetErrorMessage() == 0x0C) {
            mNetError = true;
            done = TRUE;
            sLoadErrorMessage = 0x0C;
        } else {
            done = TRUE;
        }
        break;
    case 1:
        done = TRUE;
        break;
    }
    return done;
}

// 801095A4
BOOL dScBootStatic_c::PrcInitState_c::execute() {
    if (!mStarted) {
        if (dPrcMng_c::startSetup()) {
            mStarted = true;
        } else {
            return FALSE;
        }
    }
    if (dPrcMng_c::isDone()) {
        return TRUE;
    }
    return FALSE;
}

// 80109610
BOOL dScBootStatic_c::InitSaveState_c::execute() {
    if (!mStarted) {
        dSvProc::sProc.reset();
        if (l_bootStatic.mLoadSave.mLoadFailed) {
            dSaveData_c::get()->clear();
            dSaveData_c::get()->initialize();
        }
        mStarted = true;
    }
    if (l_bootStatic.mLoadSave.mLoadFailed) {
        return dSvProc::sProc.process(dSvProc_c::MODE_TITLE);
    }
    return dSvProc::sProc.process(dSvProc_c::MODE_GROWTH);
}
