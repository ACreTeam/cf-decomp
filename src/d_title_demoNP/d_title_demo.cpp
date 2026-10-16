#include <game/game/d_title_demo.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_black_wait.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_sv_auc.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_sys.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/sLib/s_lib.hpp>
#include <revolution/NAND/NANDCheck.h>
#include <revolution/WPAD/WPAD.h>

namespace dTitleDemo {

// The message window has closed.
inline bool isMessageHidden(dMsg::Comp_c *controller) {
    return controller->isState(&dMsg::Comp_c::executeHidden);
}

// A button of the current controller was pressed this frame.
inline bool isTrigger(u32 button) {
    EGG::CoreController *core = mPad::g_currentCore;
    return core->downTrigger(button);
}

// The width of a word in a text box.
inline f32 calcWordWidth(nw4r::lyt::TextBox *textBox, dScript::Word_c *word) {
    const wchar_t *str = word->getBuffer();
    return m2d::calcTextWidth(textBox, str, word->getCapacity());
}

// .text:0x5C
void *create() {
    return new dTitleDemo_c;
}

fProf::fBaseProfile_c g_profile_TITLE_DEMO = {&create, fProf::TITLE_DEMO,
                                              fProf::DRAW_ORDER::TITLE_DEMO};

} // namespace dTitleDemo

// .text:0x8C
dTitleDemo_c::dTitleDemo_c() {
}

// .text:0x178
dTitleDemo_c::~dTitleDemo_c() {
}

// .text:0x230
int dTitleDemo_c::create() {
    static const dState::mode_c<dTitleDemo_c>::Entry modes[] = {
        {"フェードイン待ち", &dTitleDemo_c::initFadeIn,
         &dTitleDemo_c::executeFadeIn},
        {"ロゴイン待ち", &dTitleDemo_c::initLogoIn,
         &dTitleDemo_c::executeLogoIn},
        {"任天堂表示待ち", &dTitleDemo_c::initNintendo,
         &dTitleDemo_c::executeNintendo},
        {"待機", &dTitleDemo_c::initWait, &dTitleDemo_c::executeWait},
        {"メッセージ表示", &dTitleDemo_c::initMessage,
         &dTitleDemo_c::executeMessage},
        {"シーン切り替え", &dTitleDemo_c::initSceneChange,
         &dTitleDemo_c::executeSceneChange},
        {"プレイヤ選択画面へシーン切り替え",
         &dTitleDemo_c::initSceneChangeToPlayerSelect,
         &dTitleDemo_c::executeSceneChangeToPlayerSelect},
        {"似顔絵ライブラリエラーチェック", &dTitleDemo_c::initMiiErrorCheck,
         &dTitleDemo_c::executeMiiErrorCheck},
        {"似顔絵ライブラリエラーメッセージ", &dTitleDemo_c::initMiiErrorMessage,
         &dTitleDemo_c::executeMiiErrorMessage},
        {"NAND情報取得", &dTitleDemo_c::initNandInfo,
         &dTitleDemo_c::executeNandInfo},
        {"NANDチェック", &dTitleDemo_c::initNandCheck,
         &dTitleDemo_c::executeNandCheck},
        {"ファイル削除", &dTitleDemo_c::initFileDelete,
         &dTitleDemo_c::executeFileDelete},
        {"オープニングへ", &dTitleDemo_c::initToOpening,
         &dTitleDemo_c::executeToOpening},
        {"処理なし", &dTitleDemo_c::initNone, &dTitleDemo_c::executeNone},
    };
    if (!mRes.load("Layout/title_us.arc")) {
        return NOT_READY;
    }
    createLayout();
    dState::mode_c<dTitleDemo_c>::initialize("dTitleDemo_c", modes, 14, false);
    dSaveOption::reset(&dSaveData_c::sOption);
    dSaveOption::setNewPlayerDefaults(&dSaveData_c::sOption);
    dSvAuc_c::getChanged()->makeAllZero();
    return SUCCEEDED;
}

// .text:0x320
int dTitleDemo_c::doDelete() {
    if (dWifi::getThreadResult() == -1) {
        return NOT_READY;
    }
    if (!dNand::isIdle()) {
        return NOT_READY;
    }
    if (!mRes.remove()) {
        return NOT_READY;
    }
    dBgm::l_mgr.mStgTitle.stop();
    dBgm::l_mgr.mStgTitle.unmute();
    return SUCCEEDED;
}

// .text:0x3A4
int dTitleDemo_c::execute() {
    dState::base_c<dTitleDemo_c>::execute();
    calcLayout();
    return SUCCEEDED;
}

// .text:0x400
int dTitleDemo_c::draw() {
    drawLayout();
    return SUCCEEDED;
}

// .text:0x424
void dTitleDemo_c::requestMessage(u16 code) {
    dMsg::Comp_c *controller = dMsg::Comp_c::mInstance;
    setMessageLabel("sys_MESS/SYS_Save_and_Load");
    setMessageCode(code);
    controller->attachActor(this);
    controller->requestStart(0);
    mFlags &= ~FLAG_PRESS_A;
}

// .text:0x4B4
void dTitleDemo_c::onMessageStart(int arg) {
    u16 code = mMessageCode;
    dMsg::Comp_c *controller = mpController;
    int blocks = dSvMgr_c::getRequiredBlocks();
    if (code == 5) {
        controller->formatNumber(0, blocks, 12, 9);
    }
}

// .text:0x50C
void dTitleDemo_c::onMessageEnd(int kind) {
}

// .text:0x510
void dTitleDemo_c::onAnswer(int answer) {
    u16 code = mMessageCode;
    u8 selected = mpController->mSelect.mCursor;
    if (code == 2 && selected == 0) {
        changeMode(&dTitleDemo_c::executeFileDelete);
    }
    if (code == 7 && selected == 0) {
        dReset::Manage_c::GetInstance()->RequestDataManager();
        changeMode(&dTitleDemo_c::executeNone);
    }
}

// .text:0x7BC
void dTitleDemo_c::initFadeIn() {
    mFlags = 0;
    dBgm::l_mgr.mStgTitle.requestPlay();
}

// .text:0x7D4
void dTitleDemo_c::executeFadeIn() {
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        changeMode(&dTitleDemo_c::executeLogoIn);
    }
}

// .text:0x958
void dTitleDemo_c::initLogoIn() {
    mFlags |= FLAG_LOGO;
    mLogoAnm.setNext(LOGO_ANM_IN);
}

// .text:0x970
void dTitleDemo_c::executeLogoIn() {
    if (isSkipTriggered()) {
        mLogoAnm.setToEnd();
    }
    if (mLogoAnm.isEnd()) {
        changeMode(&dTitleDemo_c::executeNintendo);
    }
}

// .text:0xAF0
void dTitleDemo_c::initNintendo() {
    mFlags |= FLAG_NINTENDO;
    mLogoAnm.setNext(LOGO_ANM_WAIT);
    mTimer = 60;
}

// .text:0xB38
void dTitleDemo_c::executeNintendo() {
    if (sLib::calcTimer(&mTimer) == 0) {
        changeMode(&dTitleDemo_c::executeWait);
    }
}

// .text:0xCA4
void dTitleDemo_c::initWait() {
    mTimer = 3600;
    mFlags |= FLAG_PRESS_A;
    resetPressA();
    startLogoAnm((mFlags & FLAG_SMALL_LOGO) ? LOGO_ANM_S_WAIT : LOGO_ANM_WAIT);
}

// .text:0xD0C
void dTitleDemo_c::executeWait() {
    if (checkStart()) {
        return;
    }
    if (mFlags & FLAG_SMALL_LOGO) {
        if (sLib::calcTimer(&mTimer) == 0) {
            dScene::requestSceneChange(dScene::SCENE_DM_TITLE, 2, 2);
            changeMode(&dTitleDemo_c::executeSceneChange);
        }
    } else if (dBgm::l_mgr.mStgTitle.isEnd()) {
        startLogoAnm(LOGO_ANM_OUT);
    }
}

// .text:0xED4
void dTitleDemo_c::initFileDelete() {
    static const stepDelete_c::Entry steps[] = {
        {"初期化", (stepDelete_c::Method)&stepDelete_c::initDelete},
        {"DL BOX削除初期化",
         (stepDelete_c::Method)&stepDelete_c::initializeDeleteDownloadBox},
        {"DL BOX削除", (stepDelete_c::Method)&stepDelete_c::deleteDownloadBox},
        {"セーブ削除初期化",
         (stepDelete_c::Method)&stepDelete_c::initializeDeleteSave},
        {"セーブ削除開始",
         (stepDelete_c::Method)&stepDelete_c::startDeleteSave},
        {"セーブ削除完了", (stepDelete_c::Method)&stepDelete_c::waitDeleteSave},
        {"終了", (stepDelete_c::Method)&stepDelete_c::finish},
        {"セーブ失敗", (stepDelete_c::Method)&stepDelete_c::saveFailed},
    };
    mStepDelete.initialize(this, "dTitleDemo_c::stepDelete_c", steps, ARRAY_SIZE(steps));
}

// .text:0x1040
void dTitleDemo_c::executeFileDelete() {
    mStepDelete.execute();
}

// .text:0x1088
void dTitleDemo_c::stepDelete_c::initDelete() {
    mpOwner->lockMessage();
    nextStep();
}

// .text:0x113C
void dTitleDemo_c::stepDelete_c::finish() {
    dSvRuntime::releaseMessage(mpOwner->getController());
    mpOwner->changeMode(&dTitleDemo_c::executeToOpening);
}

// .text:0x12B8
void dTitleDemo_c::initToOpening() {
}

// .text:0x12BC
void dTitleDemo_c::executeToOpening() {
    dMsg::Comp_c *controller = dMsg::Comp_c::mInstance;
    if (dTitleDemo::isMessageHidden(controller)) {
        controller->detachActor();
        changeMode(&dTitleDemo_c::executeMiiErrorCheck);
    }
}

// .text:0x145C
void dTitleDemo_c::initMiiErrorCheck() {
}

// .text:0x1460
void dTitleDemo_c::executeMiiErrorCheck() {
    if (dSys::isFaceError()) {
        requestMessage(0x21);
        changeMode(&dTitleDemo_c::executeMiiErrorMessage);
    } else {
        changeMode(&dTitleDemo_c::executeSceneChangeToPlayerSelect);
    }
}

// .text:0x16FC
void dTitleDemo_c::initMiiErrorMessage() {
}

// .text:0x1700
void dTitleDemo_c::executeMiiErrorMessage() {
    dMsg::Comp_c *controller = dMsg::Comp_c::mInstance;
    if (dTitleDemo::isMessageHidden(controller)) {
        controller->detachActor();
        changeMode(&dTitleDemo_c::executeSceneChangeToPlayerSelect);
    }
}

// .text:0x18A0
void dTitleDemo_c::initSceneChangeToPlayerSelect() {
    dBgm::l_mgr.mStgTitle.mute();
    dBlackWait::BlackWait_c *blackWait = dBlackWait::BlackWait_c::getInstance();
    blackWait->mHide = 1;
    playSe(SE_Opening2NextScene);
    dScene::requestSceneChange(dScene::SCENE_DM_PL_SEL, 0, 0);
}

// .text:0x18F0
void dTitleDemo_c::executeSceneChangeToPlayerSelect() {
}

// .text:0x18F4
void dTitleDemo_c::initNone() {
}

// .text:0x18F8
void dTitleDemo_c::executeNone() {
}

// .text:0x18FC
void dTitleDemo_c::initMessage() {
}

// .text:0x1900
void dTitleDemo_c::executeMessage() {
    dMsg::Comp_c *controller = dMsg::Comp_c::mInstance;
    if (dTitleDemo::isMessageHidden(controller)) {
        controller->detachActor();
        changeMode(&dTitleDemo_c::executeWait);
    }
}

// .text:0x1AA0
void dTitleDemo_c::initSceneChange() {
    dBgm::l_mgr.mStgTitle.mute();
}

// .text:0x1AB0
void dTitleDemo_c::executeSceneChange() {
}

// .text:0x1AB4
void dTitleDemo_c::initNandInfo() {
}

// .text:0x1AB8
void dTitleDemo_c::executeNandInfo() {
    switch (dNand::getType(dBanner_c::sFileName)) {
    case dNand::RESULT_OK:
        changeMode(&dTitleDemo_c::executeMiiErrorCheck);
        break;
    case dNand::RESULT_NOEXISTS:
        changeMode(&dTitleDemo_c::executeNandCheck);
        break;
    default:
        requestMessage(0x12);
        changeMode(&dTitleDemo_c::executeMessage);
        break;
    }
}

// .text:0x1E9C
void dTitleDemo_c::initNandCheck() {
}

// .text:0x1EA0
void dTitleDemo_c::executeNandCheck() {
    u32 answer = 0;
    int blocks = dSvMgr_c::getRequiredInodes();
    int inodes = dSvMgr_c::getRequiredBlocks();
    int files = dSvMgr_c::getRequiredFiles();
    switch (dNand::check(blocks, files, &answer)) {
    case dNand::RESULT_OK:
        if (answer == 0) {
            changeMode(&dTitleDemo_c::executeMiiErrorCheck);
        } else if (answer & NAND_CHECK_TOO_MANY_APP_BLOCKS) {
            requestMessage(8);
            changeMode(&dTitleDemo_c::executeMessage);
        } else if (answer & NAND_CHECK_TOO_MANY_APP_FILES) {
            requestMessage(9);
            changeMode(&dTitleDemo_c::executeMessage);
        } else if (answer & NAND_CHECK_TOO_MANY_USER_BLOCKS) {
            requestMessage(5);
            changeMode(&dTitleDemo_c::executeMessage);
        } else if (answer & NAND_CHECK_TOO_MANY_USER_FILES) {
            requestMessage(6);
            changeMode(&dTitleDemo_c::executeMessage);
        }
        break;
    default:
        requestMessage(0x12);
        changeMode(&dTitleDemo_c::executeMessage);
        break;
    }
}

// .text:0x2668
bool dTitleDemo_c::isSkipTriggered() {
    return dTitleDemo::isTrigger(WPAD_BUTTON_B) ||
           dTitleDemo::isTrigger(WPAD_BUTTON_A) ||
           dTitleDemo::isTrigger(WPAD_BUTTON_FS_C) ||
           dTitleDemo::isTrigger(WPAD_BUTTON_FS_Z);
}

// .text:0x275C
BOOL dTitleDemo_c::checkStart() {
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A) ||
        dPad::isConfirmTriggered()) {
        if (sLoadErrorMessage != 0) {
            requestMessage(sLoadErrorMessage);
            changeMode(&dTitleDemo_c::executeMessage);
        } else {
            changeMode(&dTitleDemo_c::executeNandInfo);
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x2A34
void dTitleDemo_c::createLayout() {
    static const char *l_logoAnmNames[] = {
        "title_logo_in.brlan",     "title_logo_wait.brlan",
        "title_logo_out.brlan",    "title_logo_s_in.brlan",
        "title_logo_s_wait.brlan", "title_logo_s_out.brlan",
    };
    mLogoLyt.build(&mRes, "title_logo.brlyt");
    mLogoAnm.init(&mLogoLyt, &mRes, l_logoAnmNames, ARRAY_SIZE(l_logoAnmNames));
    mNintendoLyt.build(&mRes, "title_nintendo_00.brlyt");
    mNintendoLyt.calc();
    mPressLyt.build(&mRes, "title_Abtn_00.brlyt");
    mPressLyt.setAnm(&mPressAnm, "title_Abtn_00_Abtn_00.brlan", NULL);
    mPressAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;

    dString::Word_c word;
    dString::loadString(&word, 0x12B, "sys_2D/SYS_2D_menu");
    mPressLyt.setWord("T_pressA_00", &word);
    mPressLyt.setWord("T_pressA_sdw_00", &word);
    mPressLyt.setWord("T_pressA_01", &word);
    mPressLyt.setWord("T_pressA_sdw_01", &word);
    mPressLyt.calc();

    nw4r::lyt::TextBox *textBox = mPressLyt.findTextBox("T_pressA_00");
    mPressWidthL = dTitleDemo::calcWordWidth(textBox, &word);
    mPressWidthL *= textBox->GetScale().x;
    textBox = mPressLyt.findTextBox("T_pressA_01");
    mPressWidthS = dTitleDemo::calcWordWidth(textBox, &word);
    mPressWidthS *= textBox->GetScale().x;
    setBackScale();
}

// .text:0x2C1C
void dTitleDemo_c::calcLayout() {
    if (mFlags & FLAG_LOGO) {
        if (mLogoAnm.isEnd()) {
            switch (mLogoAnm.mCurIdx) {
            case LOGO_ANM_OUT:
                mLogoAnm.setNext(LOGO_ANM_S_IN);
                mFlags |= FLAG_SMALL_LOGO;
                mFlags &= ~FLAG_NINTENDO;
                break;
            case LOGO_ANM_S_IN:
                mLogoAnm.setNext(LOGO_ANM_S_WAIT);
                break;
            }
        }
        mLogoAnm.play();
        mLogoLyt.calc();
    }
    if (mFlags & FLAG_PRESS_A) {
        if (mPressAnm.mFrameCtrl.isStop()) {
            resetPressA();
        } else {
            mPressAnm.play();
        }
        mPressLyt.calc();
    }
}

// .text:0x2D08
void dTitleDemo_c::drawLayout() {
    if (mFlags & FLAG_LOGO) {
        mLogoLyt.entry();
    }
    if (mFlags & FLAG_NINTENDO) {
        mNintendoLyt.entry();
    }
    if (mFlags & FLAG_PRESS_A) {
        mPressLyt.entry();
    }
}

// .text:0x2D6C
void dTitleDemo_c::resetPressA() {
    mPressAnm.setFrame(0.0f);
    bool large = !(mFlags & FLAG_SMALL_LOGO);
    mPressLyt.setVisible("N_pressA_S_all", !large);
    mPressLyt.setVisible("N_pressA_L_all", large);
}

// .text:0x2DE8
void dTitleDemo_c::setBackScale() {
    f32 marginL = 80.0f;
    f32 scaleS = 60.0f; // the margin, then the scale
    f32 scaleL = (marginL + mPressWidthL) / 344.0f;
    nw4r::lyt::Pane *pane = mPressLyt.findPane("N_Abtn_L_Back");
    nw4r::math::VEC2 scale;
    scale.y = pane->GetScale().y;
    scale.x = scaleL;
    pane->SetScale(scale);
    scaleS = (scaleS + mPressWidthS) / 344.0f;
    pane = mPressLyt.findPane("N_Abtn_S_Back");
    scale = pane->GetScale();
    scale.x = scaleS;
    pane->SetScale(scale);
}

#ifdef MUST_MATCH
// .rodata 0x198: nothing in the REL references it. It follows setBackScale's
// literals, so it belonged to a function the link stripped: 344.0f
// (0x43AC0000), then 3600 and 60 (the wait and Nintendo-logo frame counts) as
// two s16.
extern const u8 lbl_126_rodata_198[8];
const u8 lbl_126_rodata_198[8] = {0x43, 0xAC, 0x00, 0x00,
                                  0x0E, 0x10, 0x00, 0x3C};
#endif
