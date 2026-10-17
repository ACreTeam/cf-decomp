#include <game/game/d_a_npc_sp_player_select.hpp>
#include <game/game/d_ds_player_data_layout.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_ds_dl.hpp>
#include <game/game/d_ds_player_data.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_home_button.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sv_auc.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_sv_proc.hpp>
#include <game/game/d_travel_data.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_fader.hpp>
#include <cstring>

namespace dANpcSpPlayerSelect {
// .text 0xCC
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}
// .text 0xD8
BASE_PROFILE(A_NPC_SP_PLAYER_SELECT, dAcNpcSpPlayerSelect_c);

// The label of the player select messages.
const char *l_msgLabel = "sys_MESS/SYS_Start";

// The Wild World save image received from the DS (DMA target: 32-byte aligned).
struct DsSaveBuf_s {
    u8 mData[DS_SAVE_BUFFER_SIZE] ALIGN(32);
};
static DsSaveBuf_s l_dsSaveBuf;
// A player travelling to or from the DS.
static dTravelData::Data_c l_travelData;
} // namespace dANpcSpPlayerSelect

// .text 0x264
int dAcNpcSpPlayerSelect_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mTalkPs);
    return 1;
}
// .text 0x2B0
int dAcNpcSpPlayerSelect_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    static const dState::mode_c<dAcNpcSpPlayerSelect_c>::Entry modes[] = {
        {"会話待機", &dAcNpcSpPlayerSelect_c::initTalkWait,
         &dAcNpcSpPlayerSelect_c::executeTalkWait},
        {"はじめてデモ", &dAcNpcSpPlayerSelect_c::initFirstDemo,
         &dAcNpcSpPlayerSelect_c::executeFirstDemo},
        {"デモ待機", &dAcNpcSpPlayerSelect_c::initDemoWait,
         &dAcNpcSpPlayerSelect_c::executeDemoWait},
        {"会話", &dAcNpcSpPlayerSelect_c::initTalk,
         &dAcNpcSpPlayerSelect_c::executeTalk},
        {"処理無し", &dAcNpcSpPlayerSelect_c::initNone,
         &dAcNpcSpPlayerSelect_c::executeNone},
    };
    dState::mode_c<dAcNpcSpPlayerSelect_c>::initialize(
        "dAcNpcSpPlayerSelect_c", modes, ARRAY_SIZE(modes));
    BOOL firstDemo = FALSE;
    dFTR::clearHomeFlags();
    if (!dSaveData_c::get()->isGood()) {
        sLoadErrorMessage = 0;
        firstDemo = TRUE;
    } else {
        switch (dNand::readAll()) {
        case dNand::RESULT_NOEXISTS:
            firstDemo = TRUE;
            break;
        case dNand::RESULT_OK:
            if (!dSaveData_c::getRaw()->isExtraGood(1)) {
                dSaveData_c::get()->clear();
                dSaveData_c::get()->initialize();
                sLoadErrorMessage = SV_MSG_NAND_ERROR;
            } else if (!dSaveData_c::get()->isGood()) {
                dSaveData_c::get()->clear();
                dSaveData_c::get()->initialize();
                sLoadErrorMessage = SV_MSG_NAND_ERROR;
            } else {
                dSaveData_c *raw = dSaveData_c::getRaw();
                raw->mSaveTime.get();
                sLoadErrorMessage = 0;
            }
            break;
        case dNand::RESULT_SHORT:
            dSaveData_c::get()->clear();
            dSaveData_c::get()->initialize();
            sLoadErrorMessage = SV_MSG_NAND_ERROR;
            break;
        case dNand::RESULT_CORRUPT:
            dSaveData_c::get()->clear();
            dSaveData_c::get()->initialize();
            sLoadErrorMessage = SV_MSG_NAND_DAMAGED;
            break;
        default:
            dSaveData_c::get()->clear();
            dSaveData_c::get()->initialize();
            sLoadErrorMessage = SV_MSG_NAND_ERROR;
            break;
        }
    }
    if (firstDemo) {
        dSaveData_c::get()->clear();
        dSaveData_c::get()->initialize();
        dSaveOption::setNewPlayerDefaults(&dSaveData_c::sOption);
        mAngle.set(0, -0x4000, 0);
        mAngle3D.set(0, -0x4000, 0);
        changeMode(&dAcNpcSpPlayerSelect_c::executeFirstDemo);
    }
    mHomePos = mPos;
    dNpcInfo::playerCreate_c *playerCreate = dNpcInfo::playerCreate_c::get();
    playerCreate->mFromDs = 0;
    dDsPlayerData::clearBuffer(dANpcSpPlayerSelect::l_dsSaveBuf.mData);
    mCheckGround = 0;
    *dSvAuc_c::getChanged() = 0;
    fn_800B468C();
    mSaveOnDelete = 0;
    return 1;
}
// .text 0x660
int dAcNpcSpPlayerSelect_c::doDelete() {
    if (dWifi::getThreadResult() == dWifi::THREAD_RESULT_BUSY) {
        return 0;
    }
    if (!dNand::isIdle()) {
        return 0;
    }
    if (mSaveOnDelete && !dSvProc::sProc.process(dSvProc_c::MODE_TITLE)) {
        return 0;
    }
    if (!dAcNpcSp_c::doDelete()) {
        return 0;
    }
    dBgm::l_mgr.mVolCtrl.mWait.cancel();
    dHomeButton_c::getInstance()->mFlags &= ~dHomeButton_c::FLAG_SAVING;
    return 1;
}
// .text 0x718
f32 dAcNpcSpPlayerSelect_c::calcFade(f32 dist, f32 start, f32 end) {
    f32 d = dist - start;
    f32 rate = 1.0f;
    if (d > 0.0f) {
        f32 range = end - start;
        f32 rest = range - d;
        if (rest > 0.0f) {
            rate = rest / range;
        } else {
            rate = 0.0f;
        }
    }
    return rate;
}
// .text 0x754
int dAcNpcSpPlayerSelect_c::vtB0() {
    if (mDsDlActive) {
        dDsDl::execute();
    }
    dState::base_c<dAcNpcSpPlayerSelect_c>::execute();
    f32 dist = EGG::Math<f32>::sqrt(PSVECSquareDistance(mHomePos, mPos));
    f32 rate = calcFade(dist, 8.0f, 40.0f);
    mShadowSize = calcFade(dist, 12.0f, 40.0f);
    mColor = 255.0f * rate;
    setModelColor(mColor);
    return 1;
}
// .text 0x84C
bool dAcNpcSpPlayerSelect_c::canInteract(dDemoActor_c *actor) {
    return actor == NULL ? false : !mTalk.isBusy();
}
// .text 0x888
int dAcNpcSpPlayerSelect_c::getSoundId() {
    return 0xFFFF;
}
// .text 0x894
void dAcNpcSpPlayerSelect_c::initTalkWait() {
    mTimer = 120;
}
// .text 0x8A0
void dAcNpcSpPlayerSelect_c::executeTalkWait() {
    if (mTimer != 0) {
        mTimer--;
    }
    if (mTimer == 0 && mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        mTalkPs.start(NULL);
        changeMode(&dAcNpcSpPlayerSelect_c::executeTalk);
    }
}
// .text 0xA50
void dAcNpcSpPlayerSelect_c::initFirstDemo() {
    static const stepFirstDemo_c::Entry steps[] = {
        {"フェード待ち", (stepFirstDemo_c::Method)&stepFirstDemo_c::waitFade},
        {"歩き初期化", (stepFirstDemo_c::Method)&stepFirstDemo_c::initWalk},
        {"歩き終了", (stepFirstDemo_c::Method)&stepFirstDemo_c::waitWalk},
        {"正面へターン初期化",
         (stepFirstDemo_c::Method)&stepFirstDemo_c::initTurnFront},
        {"正面へターン終了",
         (stepFirstDemo_c::Method)&stepFirstDemo_c::waitTurnFront},
        {"終了", (stepFirstDemo_c::Method)&stepFirstDemo_c::finish},
    };
    mStepFirstDemo.initialize(this, "dAcNpcSpPlayerSelect_c::stepFirstDemo_c",
                              steps, ARRAY_SIZE(steps));
}
// .text 0xB84
void dAcNpcSpPlayerSelect_c::executeFirstDemo() {
    mStepFirstDemo.execute();
}
// .text 0xBCC
void dAcNpcSpPlayerSelect_c::stepFirstDemo_c::waitFade() {
    mpOwner->mPos.x = 48.0f + mpOwner->mHomePos.x;
    mpOwner->mLastPos.x = mpOwner->mPos.x;
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        nextStep();
    }
}
// .text 0xCB0
void dAcNpcSpPlayerSelect_c::stepFirstDemo_c::initWalk() {
    mpOwner->mAction.requestWalk(0, mpOwner->mHomePos, l_walkTurnSpeed, 0,
                                 l_moveParamWalk, l_defaultAnmRate,
                                 cNpcMorphFrames);
    nextStep();
}
// .text 0xD80
void dAcNpcSpPlayerSelect_c::stepFirstDemo_c::waitWalk() {
    if (mpOwner->mAction.mActionId == dAcNpc_c::ACTION_WALK &&
        mpOwner->mAction.mCanChange) {
        nextStep();
    }
}
// .text 0xE34
void dAcNpcSpPlayerSelect_c::stepFirstDemo_c::initTurnFront() {
    mAng angle(0);
    mpOwner->mAction.requestTurn(0, angle, l_turnSpeed, 0, cNpcMorphFrames);
    nextStep();
}
// .text 0xEFC
void dAcNpcSpPlayerSelect_c::stepFirstDemo_c::waitTurnFront() {
    if (mpOwner->mAction.mActionId == dAcNpc_c::ACTION_TURN &&
        mpOwner->mAction.mCanChange) {
        nextStep();
    }
}
// .text 0xFB0
void dAcNpcSpPlayerSelect_c::stepFirstDemo_c::finish() {
    mpOwner->mTalkPs.start(NULL);
    mpOwner->changeMode(&dAcNpcSpPlayerSelect_c::executeTalk);
}
// .text 0x1128
void dAcNpcSpPlayerSelect_c::initDemoWait() {
}
// .text 0x112C
void dAcNpcSpPlayerSelect_c::executeDemoWait() {
}
// .text 0x1130
void dAcNpcSpPlayerSelect_c::initTalk() {
    mTalk.requestTalk(true, TRUE);
}
// .text 0x1140
void dAcNpcSpPlayerSelect_c::executeTalk() {
}
// .text 0x1144
void dAcNpcSpPlayerSelect_c::initNone() {
}
// .text 0x1148
void dAcNpcSpPlayerSelect_c::executeNone() {
}

// .text 0x114C
dAcNpcSpPlayerSelect_c::talk_c::talk_c(dAcNpcSpPlayerSelect_c *npc) : _24C(0) {
    static const dState::mode_c<talk_c>::Entry modes[] = {
        {"待機", &talk_c::initWait, &talk_c::executeWait},
        {"WC24ゲーム開始前の一括処理", &talk_c::initWC24Start,
         &talk_c::executeWC24Start},
        {"成長処理", &talk_c::initGrowth, &talk_c::executeGrowth},
        {"セーブ処理", &talk_c::initSave, &talk_c::executeSave},
        {"ファイル削除", &talk_c::initFileDelete, &talk_c::executeFileDelete},
        {"プレイヤ削除", &talk_c::initPlayerDelete,
         &talk_c::executePlayerDelete},
        {"DS引越し", &talk_c::initDSEmigrate, &talk_c::executeDSEmigrate},
        {"おでかけ：DSをサーチ ", &talk_c::initSearchDS,
         &talk_c::executeSearchDS},
        {"おでかけ：DSからの受け入れ", &talk_c::initAcceptDS,
         &talk_c::executeAcceptDS},
        {"おでかけ：DSからの受け入れ拒否", &talk_c::initRefuseDS,
         &talk_c::executeRefuseDS},
        {"おでかけ：DSからの受け入れ拒否チェック", &talk_c::initRefuseCheckDS,
         &talk_c::executeRefuseCheckDS},
        {"おでかけ：DS通信終了", &talk_c::initEndDS, &talk_c::executeEndDS},
    };
    mpOwner = npc;
    mMsgCode = 0;
    mSaveKind = SAVE_KIND_TODAY;
    dState::mode_c<talk_c>::initialize("dAcNpcSpPlayerSelect_c::talk_c", modes,
                                       ARRAY_SIZE(modes));
}
// .text 0x127C
void dAcNpcSpPlayerSelect_c::talk_c::preExecute() {
    dState::base_c<talk_c>::execute();
}
// .text 0x12CC
void dAcNpcSpPlayerSelect_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mRequestEndMethod = NULL;
    mMsgLabel = NULL;
    mMsgCode = 0;
    mVoiceMode = VOICE_MODE_NONE;
}

// .text 0x1328
void dAcNpcSpPlayerSelect_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    u16 error = sLoadErrorMessage;
    if (error != 0) {
        mMsgCode = error;
        mMsgLabel = dSvMgr_c::mMessageLabel;
    } else if (dSaveData_c::get()->isGood()) {
        mMsgCode = MSG_HELLO;
    } else {
        mMsgCode = MSG_FIRST_MEETING;
    }
    if (!mMsgLabel) {
        mMsgLabel = dANpcSpPlayerSelect::l_msgLabel;
    }
}
// .text 0x13D0
int dAcNpcSpPlayerSelect_c::talk_c::getForcedVoiceMode() {
    int mode = VOICE_MODE_NONE;
    switch (mVoiceMode) {
    case VOICE_MODE_ANIMALESE:
        mode = VOICE_MODE_ANIMALESE;
        break;
    case VOICE_MODE_BEBEBESE:
        mode = VOICE_MODE_BEBEBESE;
        break;
    case VOICE_MODE_SILENT:
        mode = VOICE_MODE_SILENT;
        break;
    }
    return mode;
}
// .text 0x1414
void dAcNpcSpPlayerSelect_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = mMsgLabel;
    info->mCode = mMsgCode;
}
// .text 0x1428
void dAcNpcSpPlayerSelect_c::talk_c::onMessageStart(int arg) {
    struct Entry {
        const char *label;
        int code;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_START,
         &talk_c::startDSEmigrate},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_START_WITH_TIME,
         &talk_c::setTimeWord},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DELETE_LAST_PLAYER,
         &talk_c::setFirstPlayerName},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_VISIT_WELCOME,
         &talk_c::finishTravelVisit},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_RETURN_WELCOME,
         &talk_c::finishTravelReturn},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_SAVING,
         &talk_c::disableHomeAndReset},
    };
    u16 code = getMessageCode();
    const char *label;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < 6; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code) {
            callMessageMethod(entry->method);
        }
    }
}
// .text 0x14F4
void dAcNpcSpPlayerSelect_c::talk_c::disableHomeAndReset() {
    dHomeButton_c::getInstance()->mFlags |= dHomeButton_c::FLAG_SAVING;
    dReset::Manage_c::GetInstance()->SetResetDisable();
}
// .text 0x1528
void dAcNpcSpPlayerSelect_c::talk_c::startDSEmigrate() {
    changeMode(&talk_c::executeDSEmigrate);
}
// .text 0x1684
void dAcNpcSpPlayerSelect_c::talk_c::setTimeWord() {
    dTime_c now = *dTime_c::getCurrent();
    dString::Word_c word;
    dScript::setAmPm(&word, now.hour);
    mpController->setWord(0, &word);
}
// .text 0x1730
void dAcNpcSpPlayerSelect_c::talk_c::setFirstPlayerName() {
    int idx = dPrivateData_c::getNthValid(dSaveData_c::getTown()->mPlayers, 0);
    setPersonalName(
        &dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, idx)
             ->mPID,
        2);
}
// .text 0x1798
void dAcNpcSpPlayerSelect_c::talk_c::finishTravelReturn() {
    dANpcSpPlayerSelect::l_travelData.finishReturn();
}
// .text 0x17A4
void dAcNpcSpPlayerSelect_c::talk_c::finishTravelVisit() {
    dANpcSpPlayerSelect::l_travelData.finishVisit();
}
// .text 0x17B0
void dAcNpcSpPlayerSelect_c::talk_c::onMessageEnd(int kind) {
    struct Entry {
        const char *label;
        int code;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpPlayerSelect::l_msgLabel, MSG_WC24_EXPLAIN,
         &talk_c::askNetSetting},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_PREPARATIONS,
         &talk_c::openTitleMenu},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_PLAYER_SELECT,
         &talk_c::openPlayerSelect},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_START_FRESH,
         &talk_c::finishNewPlayerKana},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_AWAY_PLAYER,
         &talk_c::setSaveKindTravel},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_SAVED,
         &talk_c::createDsPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_NEW_PLAYER_SAVED,
         &talk_c::createDefaultPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_START_FRESH_GO,
         &talk_c::createDefaultPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DELETE_PLAYER_SELECT,
         &talk_c::openDeletePlayerSelect},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_SAVING, &talk_c::startSave},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DELETING_PLAYER,
         &talk_c::startPlayerDelete},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DELETING_TOWN,
         &talk_c::startFileDelete},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DELETING_TOWN_AND_HOUSES,
         &talk_c::startFileDelete},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_DONE,
         &talk_c::checkDsSave},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_CANCEL_HINT,
         &talk_c::checkDsConnected},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_COMMUNICATING,
         &talk_c::checkDsEmigrateEnd},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_SELECT,
         &talk_c::openDsPlayerSelect},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_OTHER_TOWN,
         &talk_c::returnToStart},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_NO_TOWN,
         &talk_c::returnToStart},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_TOWN_CORRUPT,
         &talk_c::returnToStart},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_CARD_ERROR,
         &talk_c::returnToStart},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_NO_PLAYERS,
         &talk_c::returnToStart},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_NO_CARD,
         &talk_c::returnToStart},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_WRONG_CARD,
         &talk_c::returnToStart},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_SEARCHING,
         &talk_c::startSearchDS},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_RECEIVING,
         &talk_c::holdSearchDS},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_REFUSING,
         &talk_c::startRefuseCheckDS},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_VISIT_WELCOME,
         &talk_c::startAcceptVisit},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_RETURN_WELCOME,
         &talk_c::startAcceptReturn},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_RETURN_ALREADY_BACK,
         &talk_c::startRefuseDS},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_RETURN_NOT_AWAY,
         &talk_c::startRefuseDS},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_RETURN_LEFT_TOWN,
         &talk_c::startRefuseDS},
        {sNetErrorMessageLabel, NET_ERROR_MSG_RESTRICTED,
         &talk_c::returnFromNetError3},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_OFF,
         &talk_c::returnFromNetError3},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_NAND,
         &talk_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_UPDATE,
         &talk_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_NO_INTERNET,
         &talk_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_BUSY2,
         &talk_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_NAND2,
         &talk_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_OTHER,
         &talk_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_RESET,
         &talk_c::holdNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_RESET2,
         &talk_c::holdNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_BUSY,
         &talk_c::returnFromNetError2},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_BUSY3,
         &talk_c::returnFromNetError2},
    };
    u16 code = getMessageCode();
    const char *label;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < 44; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code) {
            callMessageMethod(entry->method);
        }
    }
    switch (kind) {
    case END_REQ_TIME_MENU:
        reqMenu06(0);
        break;
    case END_REQ_SYSTEM_SPEAKER:
        switchToSystemSpeaker();
        break;
    case END_REQ_GO_HOME:
        mEndAction = END_ACTION_GO_HOME;
        break;
    case END_REQ_NEW_PLAYER:
        mEndAction = END_ACTION_NEW_PLAYER;
        break;
    case END_REQ_TITLE:
        mEndAction = END_ACTION_TITLE;
        break;
    case END_REQ_NONE:
        break;
    }
}
// .text 0x1980
void dAcNpcSpPlayerSelect_c::talk_c::returnFromNetError() {
    switchToRoverSpeaker();
    requestMessage(MSG_START, dANpcSpPlayerSelect::l_msgLabel);
}
// .text 0x19C0
void dAcNpcSpPlayerSelect_c::talk_c::returnFromNetError2() {
    switchToRoverSpeaker();
    requestMessage(MSG_START, dANpcSpPlayerSelect::l_msgLabel);
}
// .text 0x1A00
void dAcNpcSpPlayerSelect_c::talk_c::holdNetError() {
    mpController->mLock = 1;
}
// .text 0x1A10
void dAcNpcSpPlayerSelect_c::talk_c::returnFromNetError3() {
    switchToRoverSpeaker();
    requestMessage(MSG_START, dANpcSpPlayerSelect::l_msgLabel);
}
// .text 0x1A50
void dAcNpcSpPlayerSelect_c::talk_c::askNetSetting() {
    if (!dSaveData_c::getRaw()->mExtra.mHeader.mNetEnabled) {
        requestMessage(MSG_WC24_OFF_AGAIN, dANpcSpPlayerSelect::l_msgLabel);
    } else {
        requestMessage(MSG_WC24_ON_AGAIN, dANpcSpPlayerSelect::l_msgLabel);
    }
}
// .text 0x1AB8
void dAcNpcSpPlayerSelect_c::talk_c::openTitleMenu() {
    dMsg::Comp_c *controller = mpController;
    dSelect::Comp_c *select = &controller->mSelect;
    select->setSysMessage(0, SEL_MSG_SOUND, MSG_VOICE, NULL, 0, 0);
    if (getLanguage() == LANGUAGE_JP) {
        select->setSysMessage(1, SEL_MSG_TEXT, MSG_KANA, NULL, 0, 0);
    } else {
        select->setSysMessage(1, SEL_MSG_TEXT, MSG_KEY_INPUT, NULL, 0, 0);
    }
    select->setSysMessage(2, SEL_MSG_MENU, MSG_TUTORIAL, NULL, 0, 0);
    int num = 3;
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUS_NET_EVENT_DONE)) {
        select->setSysMessage(3, SEL_MSG_WC24, dMsg::MSG_CODE_NONE, NULL, 0, 0);
        num = 4;
    }
    controller->mSelect.mCancel = num;
    select->setSysMessage(num, SEL_MSG_NEVER_MIND_PREPARATIONS, MSG_START, NULL,
                          0, 0);
    controller->mSelect.mNum = num + 1;
    controller->openSelect();
}
// .text 0x1BF4
void dAcNpcSpPlayerSelect_c::talk_c::openPlayerSelect() {
    dMsg::Comp_c *controller = mpController;
    dSelect::Comp_c *select = &controller->mSelect;
    int num = 0;
    for (int i = 0; i < 4; i++) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(i);
        if (player->mPID.isValid()) {
            int code;
            if (player->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
                code = MSG_AWAY_PLAYER;
            } else if (!dSaveData_c::getRaw()->isFlag(
                           SAVE_FLAG_ROVER_GREETED)) {
                dSaveData_c::getTown()->setFlag(SAVE_FLAG_ROVER_GREETED);
                player->setFlag0(PRIVATE_FLAG0_ROVER_GREETED);
                code = MSG_GREET_FIRST_IN_TOWN;
            } else {
                code = getGreetingMsg(player);
            }
            select->setPlayerName(num, &player->mPID, code, NULL, 0, 1);
            num++;
        }
    }
    if (num < 4) {
        select->setSysMessage(num, SEL_MSG_NEW_PLAYER, MSG_NEW_PLAYER, NULL, 0,
                              0);
        num++;
    }
    controller->mSelect.mCancel = num;
    select->setSysMessage(num, SEL_MSG_NEVER_MIND_START, MSG_START_AGAIN, NULL,
                          0, 0);
    controller->mSelect.mNum = num + 1;
    controller->openSelect();
}
// .text 0x1D40
void dAcNpcSpPlayerSelect_c::talk_c::openDeletePlayerSelect() {
    dMsg::Comp_c *controller = mpController;
    dSelect::Comp_c *select = &controller->mSelect;
    int num = 0;
    for (int i = 0; i < 4; i++) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(i);
        if (player->mPID.isValid()) {
            u16 code = MSG_DELETE_PLAYER_CONFIRM;
            if (player->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
                code = MSG_DELETE_AWAY_PLAYER;
            }
            select->setPlayerName(num, &player->mPID, code, NULL, 0, 1);
            num++;
        }
    }
    controller->mSelect.mCancel = num;
    select->setSysMessage(num, SEL_MSG_NEVER_MIND, MSG_DELETE_CANCELLED, NULL,
                          0, 0);
    controller->mSelect.mNum = num + 1;
    controller->openSelect();
}
// .text 0x1E18
void dAcNpcSpPlayerSelect_c::talk_c::startSave() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 0;
    controller->mWaitIcon = 1;
    switch (mSaveKind) {
    case SAVE_KIND_DAYS_PASSED:
    case SAVE_KIND_TODAY:
    case SAVE_KIND_AWAY_PLAYER:
    case SAVE_KIND_DS_RETURN:
        changeMode(&talk_c::executeWC24Start);
        break;
    case SAVE_KIND_NEW_PLAYER:
    case SAVE_KIND_DS_EMIGRATE:
        changeMode(&talk_c::executeSave);
        break;
    case SAVE_KIND_DS_VISIT:
        changeMode(&talk_c::executeGrowth);
        break;
    }
    switch (mSaveKind) {
    case SAVE_KIND_NEW_PLAYER:
        requestMessage(MSG_NEW_PLAYER_SAVED, dANpcSpPlayerSelect::l_msgLabel);
        break;
    case SAVE_KIND_DS_EMIGRATE:
        requestMessage(MSG_DS_EMIGRATE_SAVED, dANpcSpPlayerSelect::l_msgLabel);
        break;
    case SAVE_KIND_AWAY_PLAYER:
        requestMessage(MSG_SAVE_AWAY_PLAYER, dANpcSpPlayerSelect::l_msgLabel);
        break;
    case SAVE_KIND_DAYS_PASSED:
        requestMessage(MSG_SAVE_DAYS_PASSED, dANpcSpPlayerSelect::l_msgLabel);
        break;
    case SAVE_KIND_TODAY:
        requestMessage(MSG_SAVE_TODAY, dANpcSpPlayerSelect::l_msgLabel);
        break;
    }
}
// .text 0x22AC
void dAcNpcSpPlayerSelect_c::talk_c::startFileDelete() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 0;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executeFileDelete);
}
// .text 0x2420
void dAcNpcSpPlayerSelect_c::talk_c::startPlayerDelete() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 0;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executePlayerDelete);
}
// .text 0x2594
void dAcNpcSpPlayerSelect_c::talk_c::finishNewPlayerKana() {
    dSaveOption::setKana(TRUE);
    mSaveKind = SAVE_KIND_NEW_PLAYER;
}
// .text 0x25CC
void dAcNpcSpPlayerSelect_c::talk_c::setSaveKindTravel() {
    mSaveKind = SAVE_KIND_AWAY_PLAYER;
}
// .text 0x25D8
void dAcNpcSpPlayerSelect_c::talk_c::createDsPlayer() {
    const wchar_t *buffer;
    const u8 *dsPlayer = dDsPlayerData::getPlayer(
        dDsPlayerData::selectSave(dANpcSpPlayerSelect::l_dsSaveBuf.mData),
        mDsPlayer);
    dHmnName::Word_c name;
    dDsPlayerData::getPlayerName(dsPlayer, &name);
    int slot = dPrivateData_c::findEmpty(dSaveData_c::getTown()->mPlayers);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(slot);
    buffer = static_cast<dScript::Word_c &>(name).getBuffer();
    player->setup(buffer, dDsPlayerData::getPlayerId(dsPlayer),
                  dDsPlayerData::getGender(dsPlayer));
    player->mFace = dDsPlayerData::getFace(dsPlayer);
    player->mHair = dDsPlayerData::getHair(dsPlayer);
    player->mHairColor = dDsPlayerData::getHairColor(dsPlayer);
    player->mEquipment.mShirt =
        dPlayerMgr::getRandomShirt(dDsPlayerData::getGender(dsPlayer));
    if (dDsPlayerData::getGender(dsPlayer) == GENDER_MALE) {
        player->mShoeKind = 0;
        player->mShoeColor = 4;
    } else {
        player->mShoeKind = 1;
        player->mShoeColor = 1;
    }
    addCatalogItems((void *)dDsPlayerData::getCatalog(dsPlayer),
                    &player->mCatalog);
    dPlayerMgr::setPlayerSlot(0, slot);
    dSaveOption::setKana(dDsPlayerData::getOption(dsPlayer));
    dSaveOption::storeToCurrentPlayer();
    dNpcInfo::playerCreate_c *create = dNpcInfo::playerCreate_c::get();
    create->mFromDs = 1;
}
// .text 0x2754
void dAcNpcSpPlayerSelect_c::talk_c::createDefaultPlayer() {
    const wchar_t names[4][9] = {
        {0x30D7, 0x30EC, 0x30A4, 0x30E4, L'1'},
        {0x30D7, 0x30EC, 0x30A4, 0x30E4, L'2'},
        {0x30D7, 0x30EC, 0x30A4, 0x30E4, L'3'},
        {0x30D7, 0x30EC, 0x30A4, 0x30E4, L'4'},
    };
    int slot = dPrivateData_c::findEmpty(dSaveData_c::getTown()->mPlayers);
    dSaveData_c::setupPlayer(names[slot], GENDER_MALE, slot);
    dPlayerMgr::setPlayerSlot(0, slot);
    dSaveOption::storeToCurrentPlayer();
}
// .text 0x27DC
void dAcNpcSpPlayerSelect_c::talk_c::checkDsSave() {
    int code = getDsSaveMsg();
    requestMessage(code, dANpcSpPlayerSelect::l_msgLabel);
}
// .text 0x2820
void dAcNpcSpPlayerSelect_c::talk_c::checkDsConnected() {
    if (isCurrentMethod(&talk_c::executeDSEmigrate)) {
        if (dDsDl::g_manager.isConnected()) {
            requestMessage(MSG_DS_COMMUNICATING,
                           dANpcSpPlayerSelect::l_msgLabel);
            switchToSystemSpeaker();
        } else {
            dMsg::Comp_c *controller = mpController;
            controller->mLock = 1;
            controller->mWaitIconSe = 1;
            controller->mWaitIcon = 1;
        }
    } else {
        requestMessage(MSG_DS_EMIGRATE_NOT_FOUND,
                       dANpcSpPlayerSelect::l_msgLabel);
    }
}
// .text 0x28DC
void dAcNpcSpPlayerSelect_c::talk_c::checkDsEmigrateEnd() {
    if (isCurrentMethod(&talk_c::executeDSEmigrate)) {
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 1;
        controller->mWaitIconSe = 1;
        controller->mWaitIcon = 1;
    } else if (mpController->mNextMsgCode == 0) {
        requestMessage(MSG_DS_EMIGRATE_NOT_FOUND,
                       dANpcSpPlayerSelect::l_msgLabel);
    }
    mpController->mForceNext = 0;
}
// .text 0x2980
void dAcNpcSpPlayerSelect_c::talk_c::openDsPlayerSelect() {
    dMsg::Comp_c *controller = mpController;
    dSelect::Comp_c *select = &controller->mSelect;
    int num = 0;
    for (int i = 0; i < 4; i++) {
        const u8 *dsPlayer = dDsPlayerData::getPlayer(
            dDsPlayerData::selectSave(dANpcSpPlayerSelect::l_dsSaveBuf.mData),
            i);
        if (dDsPlayerData::isValidPlayer(dsPlayer)) {
            dHmnName::Word_c name;
            dDsPlayerData::getPlayerName(dsPlayer, &name);
            select->setWord(num, &name, MSG_DS_EMIGRATE_CONFIRM, NULL, 0, 1);
            num++;
        }
    }
    select->setSysMessage(num, SEL_MSG_NEVER_MIND, MSG_DS_EMIGRATE_QUIT, NULL,
                          0, 0);
    controller->mSelect.mNum = num + 1;
    controller->openSelect();
}
// .text 0x2A60
void dAcNpcSpPlayerSelect_c::talk_c::returnToStart() {
    if (dSaveData_c::get()->isGood()) {
        requestMessage(MSG_START_AGAIN, dANpcSpPlayerSelect::l_msgLabel);
    } else {
        requestMessage(MSG_START_OVER, dANpcSpPlayerSelect::l_msgLabel);
    }
}
// .text 0x2AC4
void dAcNpcSpPlayerSelect_c::talk_c::startSearchDS() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executeSearchDS);
}
// .text 0x2C34
void dAcNpcSpPlayerSelect_c::talk_c::holdSearchDS() {
    if (isCurrentMethod(&talk_c::executeSearchDS)) {
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 1;
        controller->mWaitIconSe = 1;
        controller->mWaitIcon = 1;
    }
}
// .text 0x2CA4
void dAcNpcSpPlayerSelect_c::talk_c::startRefuseCheckDS() {
    changeMode(&talk_c::executeRefuseCheckDS);
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
}
// .text 0x2E14
void dAcNpcSpPlayerSelect_c::talk_c::startAcceptReturn() {
    mSaveKind = SAVE_KIND_DS_RETURN;
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    if (dDsDl::g_manager.mError) {
        requestMessage(MSG_DS_ERROR_CONTINUE, dANpcSpPlayerSelect::l_msgLabel);
        changeMode(&talk_c::executeEndDS);
    } else {
        changeMode(&talk_c::executeAcceptDS);
    }
}
// .text 0x30DC
void dAcNpcSpPlayerSelect_c::talk_c::startAcceptVisit() {
    mSaveKind = SAVE_KIND_DS_VISIT;
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    if (dDsDl::g_manager.mError) {
        requestMessage(MSG_DS_ERROR_CONTINUE, dANpcSpPlayerSelect::l_msgLabel);
        changeMode(&talk_c::executeEndDS);
    } else {
        changeMode(&talk_c::executeAcceptDS);
    }
}
// .text 0x33A4
void dAcNpcSpPlayerSelect_c::talk_c::startRefuseDS() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    if (dDsDl::g_manager.mError) {
        changeMode(&talk_c::executeEndDS);
    } else {
        changeMode(&talk_c::executeRefuseDS);
    }
}
// .text 0x3650
void dAcNpcSpPlayerSelect_c::talk_c::switchToSystemSpeaker() {
    dString::Word_c speaker((const wchar_t *)getSpeakerName());
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GUIDE);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    if (name.isSame(&speaker)) {
        setRequest1(0);
        mRequestEndMethod = &talk_c::hideRoverSpeaker;
    }
}
// .text 0x36FC
void dAcNpcSpPlayerSelect_c::talk_c::switchToRoverSpeaker() {
    dString::Word_c speaker((const wchar_t *)getSpeakerName());
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GUIDE);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    if (!name.isSame(&speaker)) {
        setRequest1(0);
        mRequestEndMethod = &talk_c::showRoverSpeaker;
    }
}
// .text 0x37A8
void dAcNpcSpPlayerSelect_c::talk_c::onAnswer(int answer) {
    struct Entry {
        const char *label;
        int code;
        int answer;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpPlayerSelect::l_msgLabel, MSG_PREPARATIONS, 3,
         &talk_c::selectNetSetting},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_CONFIRM, 0,
         &talk_c::saveDsPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_PLAYER_SELECT, 0,
         &talk_c::selectPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_PLAYER_SELECT, 1,
         &talk_c::selectPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_PLAYER_SELECT, 2,
         &talk_c::selectPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_PLAYER_SELECT, 3,
         &talk_c::selectPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_VOICE, 0,
         &talk_c::selectVoiceMode0},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_VOICE, 1,
         &talk_c::selectVoiceMode1},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_VOICE, 2,
         &talk_c::selectVoiceMode2},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_VOICE_CONFIRM, 0,
         &talk_c::decideVoiceMode},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_VOICE_CONFIRM, 1,
         &talk_c::cancelVoiceMode},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_KANA, 1, &talk_c::setKanaOff},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_KANA, 0, &talk_c::setKanaOn},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_KEY_INPUT, 0,
         &talk_c::setKeySeOff},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_KEY_INPUT, 1,
         &talk_c::setKeySeOn},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_TUTORIAL, 0,
         &talk_c::setTutorialOn},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_TUTORIAL, 1,
         &talk_c::setTutorialOff},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DELETE_PLAYER_SELECT, 0,
         &talk_c::selectDeletePlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DELETE_PLAYER_SELECT, 1,
         &talk_c::selectDeletePlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DELETE_PLAYER_SELECT, 2,
         &talk_c::selectDeletePlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DELETE_PLAYER_SELECT, 3,
         &talk_c::selectDeletePlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_WC24_ON, 0, &talk_c::enableNet},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_WC24_ON, 1, &talk_c::disableNet},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_WC24_OFF, 0, &talk_c::enableNet},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_WC24_OFF, 1, &talk_c::disableNet},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_WC24_ON_AGAIN, 0,
         &talk_c::enableNet},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_WC24_ON_AGAIN, 1,
         &talk_c::disableNet},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_WC24_OFF_AGAIN, 0,
         &talk_c::enableNet},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_WC24_OFF_AGAIN, 1,
         &talk_c::disableNet},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_SELECT, 0,
         &talk_c::selectDsPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_SELECT, 1,
         &talk_c::selectDsPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_SELECT, 2,
         &talk_c::selectDsPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_SELECT, 3,
         &talk_c::selectDsPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_QUIT, 0,
         &talk_c::retryDsPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_EMIGRATE_QUIT, 2,
         &talk_c::cancelDsPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_AWAY_PLAYER, 0,
         &talk_c::answerSystemSpeaker},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_CONNECT, 0,
         &talk_c::answerSystemSpeaker},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_QUIT, 0,
         &talk_c::answerSystemSpeaker},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_WRONG_PLAYER, 0,
         &talk_c::answerSystemSpeaker},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_CONNECT_AGAIN, 0,
         &talk_c::answerSystemSpeaker},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_RETURN_CONFIRM, 0,
         &talk_c::checkTravelPlayer},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_RETURN_CONFIRM, 1,
         &talk_c::refuseTravel},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_VISIT_CONFIRM, 0,
         &talk_c::acceptTravel},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_VISIT_CONFIRM, 1,
         &talk_c::refuseTravel},
        {dANpcSpPlayerSelect::l_msgLabel, MSG_DS_REFUSE_ERROR, 1,
         &talk_c::checkTravelReturn},
    };
    u16 code = getMessageCode();
    const char *label;
    int selected = mpController->mSelect.mCursor;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < 45; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code &&
            selected == entry->answer) {
            callMessageMethod(entry->method);
        }
    }
}
// .text 0x3924
void dAcNpcSpPlayerSelect_c::talk_c::selectNetSetting() {
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUS_NET_EVENT_DONE)) {
        if (!dSaveData_c::getRaw()->mExtra.mHeader.mNetEnabled) {
            requestMessage(MSG_WC24_OFF, dANpcSpPlayerSelect::l_msgLabel);
        } else {
            requestMessage(MSG_WC24_ON, dANpcSpPlayerSelect::l_msgLabel);
        }
    }
}
// .text 0x39A0
void dAcNpcSpPlayerSelect_c::talk_c::saveDsPlayer() {
    if (dSaveData_c::get()->isGood()) {
        mSaveKind = SAVE_KIND_DS_EMIGRATE;
        requestMessage(MSG_SAVING, dANpcSpPlayerSelect::l_msgLabel);
    }
}
// .text 0x39F4
void dAcNpcSpPlayerSelect_c::talk_c::selectPlayer() {
    int passed;
    u32 cursor = mpController->mSelect.mCursor;
    dPrivateData_c *players = dSaveData_c::getTown()->mPlayers;
    int slot = dPrivateData_c::getNthValid(players, cursor);
    if (slot != -1) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(slot);
        if (player) {
            dPlayerMgr::setPlayerSlot(0, slot);
            dSaveOption::storeToCurrentPlayer();
            passed = player->mDaysPassed;
            int days = fgMngProc_getElapsedDays();
            days += passed;
            if (days == 0) {
                mSaveKind = SAVE_KIND_TODAY;
            } else {
                mSaveKind = SAVE_KIND_DAYS_PASSED;
            }
        }
    }
}
// .text 0x3A98
void dAcNpcSpPlayerSelect_c::talk_c::selectDeletePlayer() {
    u32 cursor = mpController->mSelect.mCursor;
    dPrivateData_c *players = dSaveData_c::getTown()->mPlayers;
    int slot = dPrivateData_c::getNthValid(players, cursor);
    if (slot != -1) {
        mPlayerIndex = slot;
        setPersonalName(&dPlayerMgr::Mgr_c::getPlayer(slot)->mPID, 2);
    }
}
// .text 0x3B04
void dAcNpcSpPlayerSelect_c::talk_c::selectDsPlayer() {
    u32 cursor;
    dMsg::Comp_c *controller = mpController;
    cursor = controller->mSelect.mCursor;
    const u8 *save =
        dDsPlayerData::selectSave(dANpcSpPlayerSelect::l_dsSaveBuf.mData);
    int idx = dDsPlayerData::findPlayer(save, cursor);
    if (idx != -1) {
        mDsPlayer = idx;
        const u8 *dsPlayer = dDsPlayerData::getPlayer(save, idx);
        if (dsPlayer) {
            dHmnName::Word_c name;
            dDsPlayerData::getPlayerName(dsPlayer, &name);
            controller->setWord(4, &name);
            dPersonalID_c pid;
            const wchar_t *buffer =
                static_cast<dScript::Word_c &>(name).getBuffer();
            pid.setPlayer(buffer, dDsPlayerData::getPlayerId(dsPlayer),
                          dDsPlayerData::getGender(dsPlayer));
            if (dPrivateData_c::contains(dSaveData_c::getTown()->mPlayers,
                                         &pid)) {
                requestMessage(MSG_DS_PLAYER_LIVES_HERE_SELECT,
                               dANpcSpPlayerSelect::l_msgLabel);
            }
        }
    }
}
// .text 0x3C28
void dAcNpcSpPlayerSelect_c::talk_c::cancelDsPlayer() {
    returnToStart();
}
// .text 0x3C2C
void dAcNpcSpPlayerSelect_c::talk_c::retryDsPlayer() {
    u32 num = dDsPlayerData::countPlayers(
        dDsPlayerData::selectSave(dANpcSpPlayerSelect::l_dsSaveBuf.mData));
    if (num == 1) {
        requestMessage(MSG_DS_EMIGRATE_CONFIRM,
                       dANpcSpPlayerSelect::l_msgLabel);
    } else {
        requestMessage(MSG_DS_EMIGRATE_SELECT, dANpcSpPlayerSelect::l_msgLabel);
    }
}
// .text 0x3C98
void dAcNpcSpPlayerSelect_c::talk_c::answerSystemSpeaker() {
    switchToSystemSpeaker();
}
// .text 0x3C9C
void dAcNpcSpPlayerSelect_c::talk_c::selectVoiceMode0() {
    mpController->setStringWord(1, 5, "sys_STRING/STR_Unit");
    mVoiceMode = VOICE_MODE_ANIMALESE;
}
// .text 0x3CE4
void dAcNpcSpPlayerSelect_c::talk_c::selectVoiceMode1() {
    mpController->setStringWord(1, 6, "sys_STRING/STR_Unit");
    mVoiceMode = VOICE_MODE_BEBEBESE;
}
// .text 0x3D2C
void dAcNpcSpPlayerSelect_c::talk_c::selectVoiceMode2() {
    mpController->setStringWord(1, 7, "sys_STRING/STR_Unit");
    mVoiceMode = VOICE_MODE_SILENT;
}
// .text 0x3D74
void dAcNpcSpPlayerSelect_c::talk_c::cancelVoiceMode() {
    mVoiceMode = VOICE_MODE_NONE;
}
// .text 0x3D80
void dAcNpcSpPlayerSelect_c::talk_c::decideVoiceMode() {
    dSaveOption::setVoiceMode(mVoiceMode);
    mVoiceMode = VOICE_MODE_NONE;
}
// .text 0x3DB8
void dAcNpcSpPlayerSelect_c::talk_c::setKanaOff() {
    dSaveOption::setKana(FALSE);
}
// .text 0x3DC0
void dAcNpcSpPlayerSelect_c::talk_c::setKanaOn() {
    dSaveOption::setKana(TRUE);
}
// .text 0x3DC8
void dAcNpcSpPlayerSelect_c::talk_c::setKeySeOff() {
    dSaveOption::setKeySe(FALSE);
}
// .text 0x3DD0
void dAcNpcSpPlayerSelect_c::talk_c::setKeySeOn() {
    dSaveOption::setKeySe(TRUE);
}
// .text 0x3DD8
void dAcNpcSpPlayerSelect_c::talk_c::setTutorialOn() {
    dSaveOption::setTutorialOff(FALSE);
}
// .text 0x3DE0
void dAcNpcSpPlayerSelect_c::talk_c::setTutorialOff() {
    dSaveOption::setTutorialOff(TRUE);
}
// .text 0x3DE8
void dAcNpcSpPlayerSelect_c::talk_c::enableNet() {
    if (fn_803AD87C()) {
        switchToSystemSpeaker();
        requestMessage(NET_ERROR_MSG_RESTRICTED, sNetErrorMessageLabel);
    } else if (dWifi::getWc24State() != dWifi::WC24_STATE_ON) {
        int message = dWifi::getErrorMessage();
        int error = dWifi::getErrorCode();
        u16 code = NET_ERROR_MSG_WC24_OTHER;
        if (message <= 9) {
            code = message + NET_ERROR_MSG_WC24_NAND;
        }
        switchToSystemSpeaker();
        mpController->formatNumber(0, error, 6, 6);
        requestMessage(code, sNetErrorMessageLabel);
    } else {
        dWifi::enableNet();
    }
}
// .text 0x3EC0
void dAcNpcSpPlayerSelect_c::talk_c::disableNet() {
    dWifi::disableNet();
}
// .text 0x3EC4
void dAcNpcSpPlayerSelect_c::talk_c::checkTravelPlayer() {
    int code = getTravelPlayerMsg();
    requestMessage(code, dANpcSpPlayerSelect::l_msgLabel);
}
// .text 0x3F08
void dAcNpcSpPlayerSelect_c::talk_c::acceptTravel() {
}
// .text 0x3F0C
void dAcNpcSpPlayerSelect_c::talk_c::refuseTravel() {
    if (dDsDl::g_manager.mError) {
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 1;
        controller->mWaitIconSe = 1;
        controller->mWaitIcon = 1;
        requestMessage(MSG_DS_REFUSE_ERROR, dANpcSpPlayerSelect::l_msgLabel);
        changeMode(&talk_c::executeEndDS);
    } else {
        switchToSystemSpeaker();
        requestMessage(MSG_DS_REFUSING, dANpcSpPlayerSelect::l_msgLabel);
    }
}
// .text 0x40BC
void dAcNpcSpPlayerSelect_c::talk_c::checkTravelReturn() {
    if (dANpcSpPlayerSelect::l_travelData.mPlayer.mPID.isFromTown()) {
        int code = getTravelPlayerMsg();
        requestMessage(code, dANpcSpPlayerSelect::l_msgLabel);
    } else {
        requestMessage(MSG_DS_VISIT_WELCOME, dANpcSpPlayerSelect::l_msgLabel);
    }
}
// .text 0x4138
void dAcNpcSpPlayerSelect_c::talk_c::onTalkEnd() {
    switch (mEndAction) {
    case END_ACTION_DS_VISIT: {
        dPrivateHost_c *host = &dANpcSpPlayerSelect::l_travelData.mPlayer.mHost;
        if (host->mFlagB &&
            host->mPID.land == dSaveData_c::getTown()->mLandID) {
            dScene::requestSceneChange(dScene::SCENE_DM_CHKP_MISS, 0, 0);
        } else {
            dScene::requestSceneChange(dScene::SCENE_DM_CHKP_CNNCT, 0, 0);
        }
        break;
    }
    case END_ACTION_DS_RETURN:
        dANpcSpPlayerSelect::getSceneChange()->requestExit(4, 0, 2);
        break;
    case END_ACTION_GO_HOME:
        dANpcSpPlayerSelect::getSceneChange()->requestExit(
            dSaveData_c::getTown()->mHomes.findCurrentPlayer(), 0, 2);
        break;
    case END_ACTION_TITLE:
        dScene::requestSceneChange(dScene::SCENE_DM_TITLE, 0, 2);
        break;
    case END_ACTION_NEW_PLAYER:
        dScene::requestSceneChange(dScene::SCENE_DM_BUS_PL_CRT, 0, 0);
        break;
    }
    mpOwner->changeMode(&dAcNpcSpPlayerSelect_c::executeNone);
}
// .text 0x43C8
int dAcNpcSpPlayerSelect_c::talk_c::getTravelPlayerMsg() {
    dPrivateData_c *travel = &dANpcSpPlayerSelect::l_travelData.mPlayer;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(
        dPlayerMgr::findPersonalIDSlot(&travel->mPID));
    if (!player->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
        return MSG_DS_RETURN_NOT_AWAY;
    }
    if (player->mTravelCount != travel->mTravelCount) {
        return MSG_DS_RETURN_ALREADY_BACK;
    }
    return MSG_DS_RETURN_WELCOME;
}
// .text 0x4444
int dAcNpcSpPlayerSelect_c::talk_c::getDsSaveMsg() {
    const wchar_t *buffer;
    const u8 *save =
        dDsPlayerData::selectSave(dANpcSpPlayerSelect::l_dsSaveBuf.mData);
    int code;
    if (!save) {
        code = MSG_DS_TOWN_CORRUPT;
    } else if (dDsPlayerData::isUsable(save)) {
        code = MSG_DS_NO_TOWN;
    } else {
        dLandNameWord_c town;
        dDsPlayerData::getTownName(save, &town);
        mpController->setWord(3, &town);
        u32 num = dDsPlayerData::countPlayers(save);
        if (num == 0) {
            code = MSG_DS_NO_PLAYERS;
        } else if (num == 1) {
            mDsPlayer = dDsPlayerData::findPlayer(save, 0);
            const u8 *dsPlayer = dDsPlayerData::getPlayer(save, mDsPlayer);
            dPersonalID_c pid;
            dHmnName::Word_c name;
            dDsPlayerData::getPlayerName(dsPlayer, &name);
            mpController->setWord(4, &name);
            buffer = static_cast<dScript::Word_c &>(name).getBuffer();
            pid.setPlayer(buffer, dDsPlayerData::getPlayerId(dsPlayer),
                          dDsPlayerData::getGender(dsPlayer));
            if (dPrivateData_c::contains(dSaveData_c::getTown()->mPlayers,
                                         &pid)) {
                code = MSG_DS_PLAYER_LIVES_HERE;
            } else {
                code = MSG_DS_EMIGRATE_CONFIRM;
            }
        } else {
            code = MSG_DS_EMIGRATE_SELECT;
        }
    }
    return code;
}
// .text 0x45C0
int dAcNpcSpPlayerSelect_c::talk_c::getGreetingMsg(dPrivateData_c *player) {
    int passed;
    u8 month;
    u8 day;
    dTime_c *now;
    BOOL isBirthday;
    int days;
    now = dTime_c::getCurrent();
    month = player->mBirthday.mMonth;
    day = player->mBirthday.mDay;
    passed = player->mDaysPassed;
    days = fgMngProc_getElapsedDays();
    days += passed;
    isBirthday = FALSE;
    if (!player->isFlag1(PRIVATE_FLAG1_BIRTHDAY_GREETED) &&
        player->isBirthday(*now)) {
        isBirthday = TRUE;
    }
    if (!player->isFlag0(PRIVATE_FLAG0_ROVER_GREETED)) {
        player->setFlag0(PRIVATE_FLAG0_ROVER_GREETED);
        return MSG_GREET_FIRST;
    }
    if (isBirthday) {
        player->setFlag1(PRIVATE_FLAG1_BIRTHDAY_GREETED);
        BOOL leapDay = FALSE;
        if (month == MONTH_FEBRUARY && day == 29) {
            leapDay = TRUE;
        }
        if (leapDay && !dTime_c::isLeapYear(now->year)) {
            return MSG_GREET_LEAP_BIRTHDAY;
        }
        return MSG_GREET_BIRTHDAY;
    }
    if (days >= 30) {
        return MSG_GREET_MONTH;
    }
    if (days >= 7) {
        return MSG_GREET_WEEK;
    }
    if (days >= 2) {
        return MSG_GREET_DAYS;
    }
    if (days >= 1) {
        return MSG_GREET_DAY;
    }
    if (days < 0) {
        return MSG_GREET_CLOCK_BACK;
    }
    return MSG_GREET_TODAY;
}
// .text 0x470C
void dAcNpcSpPlayerSelect_c::talk_c::initWait() {
}
// .text 0x4710
void dAcNpcSpPlayerSelect_c::talk_c::executeWait() {
}
// .text 0x4714
void dAcNpcSpPlayerSelect_c::talk_c::failWithNandError() {
    dMsg::Comp_c *controller = mpController;
    dSvRuntime::releaseMessage(controller);
    controller->setNextMsgCode(SV_MSG_NAND_DAMAGED);
    controller->setNextMsgLabel(dSvMgr_c::mMessageLabel);
    changeMode(&talk_c::executeWait);
}
// .text 0x48A0
void dAcNpcSpPlayerSelect_c::talk_c::initWC24Start() {
    if (!dWifi::startStartThread()) {
        changeMode(&talk_c::executeGrowth);
    }
}
// .text 0x4A08
void dAcNpcSpPlayerSelect_c::talk_c::executeWC24Start() {
    switch (dWifi::getThreadResult()) {
    case dWifi::THREAD_RESULT_FAILED:
    case dWifi::THREAD_RESULT_OK:
        if (dWifi::getNetErrorMessage() == SV_MSG_NAND_DAMAGED) {
            failWithNandError();
        } else {
            changeMode(&talk_c::executeGrowth);
        }
        break;
    }
}
// .text 0x4B94
void dAcNpcSpPlayerSelect_c::talk_c::initGrowth() {
    dSvProc::sProc.reset();
}
// .text 0x4BA0
void dAcNpcSpPlayerSelect_c::talk_c::executeGrowth() {
    if (dSvProc::sProc.process(dSvProc_c::MODE_LOAD)) {
        changeMode(&talk_c::executeSave);
    }
}
// .text 0x4D14
void dAcNpcSpPlayerSelect_c::talk_c::initSave() {
    static const stepSave_c::Entry steps[] = {
        {"DL BOX生成初期化",
         (stepSave_c::Method)&stepSave_c::initializeDownloadBox},
        {"DL BOX生成", (stepSave_c::Method)&stepSave_c::createDownloadBox},
        {"全体セーブ初期化",
         (stepSave_c::Method)&stepSave_c::initializeFullSave},
        {"全体セーブ開始", (stepSave_c::Method)&stepSave_c::startFullSave},
        {"全体セーブ終了", (stepSave_c::Method)&stepSave_c::waitFullSave},
        {"HBMとReset禁止OFF",
         (stepSave_c::Method)&stepSave_c::enableHomeAndReset},
        {"終了", (stepSave_c::Method)&stepSave_c::finish},
        {"セーブ失敗", (stepSave_c::Method)&stepSave_c::saveFailed},
    };
    mStepSave.initialize(this, "dAcNpcSpPlayerSelect_c::talk_c::stepSave_c",
                         steps, ARRAY_SIZE(steps));
}
// .text 0x4E80
void dAcNpcSpPlayerSelect_c::talk_c::executeSave() {
    mStepSave.execute();
}
// .text 0x4EC8
void dAcNpcSpPlayerSelect_c::talk_c::stepSave_c::finish() {
    if (mpOwner->mSaveKind == SAVE_KIND_DS_RETURN) {
        mpOwner->mEndAction = END_ACTION_DS_RETURN;
    } else if (mpOwner->mSaveKind == SAVE_KIND_DS_VISIT) {
        mpOwner->mEndAction = END_ACTION_DS_VISIT;
    } else {
        mpOwner->switchToRoverSpeaker();
    }
    dSvRuntime::releaseMessage(mpOwner->mpController);
    mpOwner->changeMode(&talk_c::executeWait);
}
// .text 0x5080
void dAcNpcSpPlayerSelect_c::talk_c::initFileDelete() {
    static const stepDelete_c::Entry steps[] = {
        {"HBMとReset禁止ON",
         (stepDelete_c::Method)&stepDelete_c::disableHomeAndReset},
        {"初期化", (stepDelete_c::Method)&stepDelete_c::initDelete},
        {"DL BOX削除初期化",
         (stepDelete_c::Method)&stepDelete_c::initializeDeleteDownloadBox},
        {"DL BOX削除", (stepDelete_c::Method)&stepDelete_c::deleteDownloadBox},
        {"セーブ削除初期化",
         (stepDelete_c::Method)&stepDelete_c::initializeDeleteSave},
        {"セーブ削除開始",
         (stepDelete_c::Method)&stepDelete_c::startDeleteSave},
        {"セーブ削除完了", (stepDelete_c::Method)&stepDelete_c::waitDeleteSave},
        {"HBMとReset禁止OFF",
         (stepDelete_c::Method)&stepDelete_c::enableHomeAndReset},
        {"終了", (stepDelete_c::Method)&stepDelete_c::finish},
        {"セーブ失敗", (stepDelete_c::Method)&stepDelete_c::saveFailed},
    };
    mStepDelete.initialize(this, "dAcNpcSpPlayerSelect_c::talk_c::stepDelete_c",
                           steps, ARRAY_SIZE(steps));
}
// .text 0x5258
void dAcNpcSpPlayerSelect_c::talk_c::executeFileDelete() {
    if (mStepDelete.mTimer != 0) {
        mStepDelete.mTimer--;
    }
    mStepDelete.execute();
}
// .text 0x52B4
void dAcNpcSpPlayerSelect_c::talk_c::stepDelete_c::initDelete() {
    mTimer = 300;
    mpOwner->requestMessage(MSG_TOWN_DELETED, dANpcSpPlayerSelect::l_msgLabel);
    mpOwner->mpOwner->mSaveOnDelete = 1;
    dSvProc::sProc.reset();
    dSaveData_c::get()->clear();
    dSaveData_c::get()->initialize();
    nextStep();
}
// .text 0x5394
void dAcNpcSpPlayerSelect_c::talk_c::stepDelete_c::finish() {
    if (mTimer == 0) {
        mpOwner->switchToRoverSpeaker();
        dSvRuntime::releaseMessage(mpOwner->mpController);
        mpOwner->changeMode(&talk_c::executeWait);
    }
}
// .text 0x5528
void dAcNpcSpPlayerSelect_c::talk_c::initPlayerDelete() {
    static const stepDeletePlayer_c::Entry steps[] = {
        {"HBMとReset禁止ON",
         (stepDeletePlayer_c::Method)&stepDeletePlayer_c::disableHomeAndReset},
        {"初期化", (stepDeletePlayer_c::Method)&stepDeletePlayer_c::initDelete},
        {"DL BOX生成初期化", (stepDeletePlayer_c::Method)&stepDeletePlayer_c::
                                 initializeDownloadBox},
        {"DL BOX生成",
         (stepDeletePlayer_c::Method)&stepDeletePlayer_c::createDownloadBox},
        {"全体セーブ初期化",
         (stepDeletePlayer_c::Method)&stepDeletePlayer_c::initializeFullSave},
        {"全体セーブ開始",
         (stepDeletePlayer_c::Method)&stepDeletePlayer_c::startFullSave},
        {"全体セーブ終了",
         (stepDeletePlayer_c::Method)&stepDeletePlayer_c::waitFullSave},
        {"HBMとReset禁止OFF",
         (stepDeletePlayer_c::Method)&stepDeletePlayer_c::enableHomeAndReset},
        {"終了", (stepDeletePlayer_c::Method)&stepDeletePlayer_c::finish},
        {"セーブ失敗",
         (stepDeletePlayer_c::Method)&stepDeletePlayer_c::saveFailed},
    };
    mStepDeletePlayer.initialize(
        this, "dAcNpcSpPlayerSelect_c::talk_c::stepDeletePlayer_c", steps,
        ARRAY_SIZE(steps));
}
// .text 0x5700
void dAcNpcSpPlayerSelect_c::talk_c::executePlayerDelete() {
    if (mStepDeletePlayer.mTimer != 0) {
        mStepDeletePlayer.mTimer--;
    }
    mStepDeletePlayer.execute();
}
// .text 0x575C
void dAcNpcSpPlayerSelect_c::talk_c::stepDeletePlayer_c::initDelete() {
    mTimer = 300;
    dSaveData_c::getTown()->deletePlayer(mpOwner->mPlayerIndex);
    dSaveData_c::getExtra()->clearDistMailReceived(mpOwner->mPlayerIndex);
    dPlayerMgr::Mgr_c::getPlayer(mpOwner->mPlayerIndex)->clear();
    mpOwner->requestMessage(MSG_PLAYER_DELETED,
                            dANpcSpPlayerSelect::l_msgLabel);
    nextStep();
}
// .text 0x5840
void dAcNpcSpPlayerSelect_c::talk_c::stepDeletePlayer_c::finish() {
    if (mTimer == 0) {
        mpOwner->switchToRoverSpeaker();
        dSvRuntime::releaseMessage(mpOwner->mpController);
        mpOwner->changeMode(&talk_c::executeWait);
    }
}
// .text 0x59D4
void dAcNpcSpPlayerSelect_c::talk_c::initDSEmigrate() {
    static const stepDSEmigrate_c::Entry steps[] = {
        {"DS引越しアプリ配信",
         (stepDSEmigrate_c::Method)&stepDSEmigrate_c::sendApp},
        {"DS引越し接続", (stepDSEmigrate_c::Method)&stepDSEmigrate_c::connect},
        {"DS引越しダウンロード",
         (stepDSEmigrate_c::Method)&stepDSEmigrate_c::download},
        {"DS引越し通信終了",
         (stepDSEmigrate_c::Method)&stepDSEmigrate_c::finish},
    };
    mStepDSEmigrate.initialize(
        this, "dAcNpcSpPlayerSelect_c::talk_c::stepDSEmigrate_c", steps,
        ARRAY_SIZE(steps));
}
// .text 0x5AD0
void dAcNpcSpPlayerSelect_c::talk_c::executeDSEmigrate() {
    mStepDSEmigrate.execute();
}
// .text 0x5B18
void dAcNpcSpPlayerSelect_c::talk_c::stepDSEmigrate_c::sendApp() {
    mTimer = 10800;
    dDsDl::g_manager.receiveFromDs(
        dANpcSpPlayerSelect::l_dsSaveBuf.mData,
        sizeof(dANpcSpPlayerSelect::l_dsSaveBuf.mData));
    mpOwner->mpOwner->mDsDlActive = 1;
    nextStep();
}
// .text 0x5BE0
void dAcNpcSpPlayerSelect_c::talk_c::stepDSEmigrate_c::connect() {
    dMsg::Comp_c *controller = mpOwner->mpController;
    if (dDsDl::g_manager.mError) {
        mpOwner->requestMessage(MSG_DS_EMIGRATE_NOT_FOUND,
                                dANpcSpPlayerSelect::l_msgLabel);
        endTransfer();
    } else if (dDsDl::g_manager.isConnected()) {
        if (controller->mLock) {
            dSvRuntime::releaseMessage(controller);
        } else {
            controller->mForceNext = 1;
        }
        mpOwner->requestMessage(MSG_DS_COMMUNICATING,
                                dANpcSpPlayerSelect::l_msgLabel);
        mpOwner->switchToSystemSpeaker();
        nextStep();
    } else if (controller->mLock) {
        if (mTimer != 0) {
            mTimer--;
        }
        if (mTimer == 0) {
            mpOwner->requestMessage(MSG_DS_EMIGRATE_NOT_FOUND,
                                    dANpcSpPlayerSelect::l_msgLabel);
            endTransfer();
        } else if (dPad::isCancelTriggered()) {
            if (!dSaveData_c::get()->isGood()) {
                mpOwner->requestMessage(MSG_DS_EMIGRATE_QUIT_NEW,
                                        dANpcSpPlayerSelect::l_msgLabel);
            } else {
                mpOwner->requestMessage(MSG_DS_EMIGRATE_QUIT_SAVED,
                                        dANpcSpPlayerSelect::l_msgLabel);
            }
            endTransfer();
        }
    }
}
// .text 0x5DA8
void dAcNpcSpPlayerSelect_c::talk_c::stepDSEmigrate_c::download() {
    if (dDsDl::g_manager.mError) {
        mpOwner->requestMessage(MSG_DS_EMIGRATE_NOT_FOUND,
                                dANpcSpPlayerSelect::l_msgLabel);
        endTransfer();
    } else if (dDsDl::g_manager.isTransferDone()) {
        switch (dDsDl::g_manager.mResult) {
        case dDsDl::RESULT_SUCCESS:
            mpOwner->requestMessage(MSG_DS_EMIGRATE_DONE,
                                    dANpcSpPlayerSelect::l_msgLabel);
            break;
        case dDsDl::RESULT_NO_ROM:
            mpOwner->requestMessage(MSG_DS_NO_CARD,
                                    dANpcSpPlayerSelect::l_msgLabel);
            break;
        case dDsDl::RESULT_INVALID:
            mpOwner->requestMessage(MSG_DS_WRONG_CARD,
                                    dANpcSpPlayerSelect::l_msgLabel);
            break;
        case dDsDl::RESULT_FAILED:
            mpOwner->requestMessage(MSG_DS_CARD_ERROR,
                                    dANpcSpPlayerSelect::l_msgLabel);
            break;
        default:
            mpOwner->requestMessage(MSG_DS_EMIGRATE_NOT_FOUND,
                                    dANpcSpPlayerSelect::l_msgLabel);
            break;
        }
        endTransfer();
    }
}
// .text 0x5EDC
void dAcNpcSpPlayerSelect_c::talk_c::stepDSEmigrate_c::endTransfer() {
    dDsDl::g_manager.cancel();
    mCurrentMethod = &stepDSEmigrate_c::finish;
}
// .text 0x5F2C
void dAcNpcSpPlayerSelect_c::talk_c::stepDSEmigrate_c::finish() {
    if (dDsDl::g_manager.isDone()) {
        mpOwner->mpOwner->mDsDlActive = 0;
        mpOwner->switchToRoverSpeaker();
        dSvRuntime::releaseMessage(mpOwner->mpController);
        mpOwner->changeMode(&talk_c::executeWait);
    }
}
// .text 0x60D4
void dAcNpcSpPlayerSelect_c::talk_c::initSearchDS() {
    static const stepSearchDSCarrier_c::Entry steps[] = {
        {"DSキャリア受け入れ開始",
         (stepSearchDSCarrier_c::Method)&stepSearchDSCarrier_c::startListen},
        {"DSキャリア接続",
         (stepSearchDSCarrier_c::Method)&stepSearchDSCarrier_c::connect},
        {"DSキャリアダウンロード",
         (stepSearchDSCarrier_c::Method)&stepSearchDSCarrier_c::download},
    };
    mStepSearchDSCarrier.initialize(
        this, "dAcNpcSpPlayerSelect_c::talk_c::stepSearchDSCarrier_c", steps,
        ARRAY_SIZE(steps));
}
// .text 0x61AC
void dAcNpcSpPlayerSelect_c::talk_c::executeSearchDS() {
    mStepSearchDSCarrier.execute();
}
// .text 0x61F4
void dAcNpcSpPlayerSelect_c::talk_c::stepSearchDSCarrier_c::startListen() {
    dANpcSpPlayerSelect::l_travelData.clear();
    dDsDl::g_manager.listenForDs(&dANpcSpPlayerSelect::l_travelData,
                                 sizeof(dANpcSpPlayerSelect::l_travelData));
    mpOwner->mpOwner->mDsDlActive = 1;
    mTimer = 10800;
    nextStep();
}
// .text 0x62C8
void dAcNpcSpPlayerSelect_c::talk_c::stepSearchDSCarrier_c::connect() {
    if (dDsDl::g_manager.mError) {
        mpOwner->requestMessage(MSG_DS_NOT_FOUND,
                                dANpcSpPlayerSelect::l_msgLabel);
        mpOwner->changeMode(&talk_c::executeEndDS);
    } else if (dDsDl::g_manager.isConnected()) {
        dSvRuntime::releaseMessage(mpOwner->mpController);
        mpOwner->requestMessage(MSG_DS_RECEIVING,
                                dANpcSpPlayerSelect::l_msgLabel);
        mCurrentMethod = &stepSearchDSCarrier_c::download;
    } else {
        if (mTimer != 0) {
            mTimer--;
        }
        if (mTimer == 0) {
            mpOwner->requestMessage(MSG_DS_NOT_FOUND,
                                    dANpcSpPlayerSelect::l_msgLabel);
            mpOwner->changeMode(&talk_c::executeEndDS);
        } else if (dPad::isCancelTriggered()) {
            mpOwner->requestMessage(MSG_DS_QUIT,
                                    dANpcSpPlayerSelect::l_msgLabel);
            mpOwner->changeMode(&talk_c::executeEndDS);
        }
    }
}
// .text 0x6768
void dAcNpcSpPlayerSelect_c::talk_c::stepSearchDSCarrier_c::download() {
    if (mpOwner->mpController->mLock) {
        if (dDsDl::g_manager.mError) {
            mpOwner->requestMessage(MSG_DS_ERROR_RETRY,
                                    dANpcSpPlayerSelect::l_msgLabel);
            mpOwner->changeMode(&talk_c::executeEndDS);
        } else if (dDsDl::g_manager.isReceived()) {
            dPrivateData_c *player = &dANpcSpPlayerSelect::l_travelData.mPlayer;
            dLandNameWord_c land;
            player->mPID.land.setWord(&land);
            dHmnName::Word_c name;
            player->mPID.setWord(&name);
            mpOwner->mpController->setWord(3, &land);
            mpOwner->mpController->setWord(4, &name);
            mpOwner->switchToRoverSpeaker();
            if (player->mPID.isFromTown()) {
                if (dPrivateData_c::find(dSaveData_c::getTown()->mPlayers,
                                         &player->mPID) == -1) {
                    mpOwner->requestMessage(MSG_DS_RETURN_LEFT_TOWN,
                                            dANpcSpPlayerSelect::l_msgLabel);
                    dSvRuntime::releaseMessage(mpOwner->mpController);
                    mpOwner->changeMode(&talk_c::executeWait);
                } else {
                    mpOwner->requestMessage(MSG_DS_RETURN_CONFIRM,
                                            dANpcSpPlayerSelect::l_msgLabel);
                    dSvRuntime::releaseMessage(mpOwner->mpController);
                    mpOwner->changeMode(&talk_c::executeWait);
                }
            } else {
                mpOwner->requestMessage(MSG_DS_VISIT_CONFIRM,
                                        dANpcSpPlayerSelect::l_msgLabel);
                dSvRuntime::releaseMessage(mpOwner->mpController);
                mpOwner->changeMode(&talk_c::executeWait);
            }
        }
    }
}
// .text 0x6DF0
void dAcNpcSpPlayerSelect_c::talk_c::initAcceptDS() {
    dDsDl::g_manager.shutdownListen();
}
// .text 0x6DFC
void dAcNpcSpPlayerSelect_c::talk_c::executeAcceptDS() {
    if (dDsDl::g_manager.isStopping()) {
        requestMessage(MSG_DS_TURN_OFF, dANpcSpPlayerSelect::l_msgLabel);
        changeMode(&talk_c::executeEndDS);
    } else if (dDsDl::g_manager.mError) {
        requestMessage(MSG_DS_ERROR_CONTINUE, dANpcSpPlayerSelect::l_msgLabel);
        changeMode(&talk_c::executeEndDS);
    }
}
// .text 0x70C0
void dAcNpcSpPlayerSelect_c::talk_c::initRefuseDS() {
    dDsDl::g_manager.closeListen();
}
// .text 0x70CC
void dAcNpcSpPlayerSelect_c::talk_c::executeRefuseDS() {
    if (dDsDl::g_manager.isStopping()) {
        changeMode(&talk_c::executeEndDS);
    } else if (dDsDl::g_manager.mError) {
        changeMode(&talk_c::executeEndDS);
    }
}
// .text 0x7370
void dAcNpcSpPlayerSelect_c::talk_c::initRefuseCheckDS() {
    dDsDl::g_manager.closeListen();
}
// .text 0x737C
void dAcNpcSpPlayerSelect_c::talk_c::executeRefuseCheckDS() {
    if (mpController->mLock) {
        if (dDsDl::g_manager.isStopping()) {
            requestMessage(MSG_DS_WRONG_PLAYER,
                           dANpcSpPlayerSelect::l_msgLabel);
            changeMode(&talk_c::executeEndDS);
        } else if (dDsDl::g_manager.mError) {
            requestMessage(MSG_DS_REFUSE_ERROR,
                           dANpcSpPlayerSelect::l_msgLabel);
            changeMode(&talk_c::executeEndDS);
        }
    }
}
// .text 0x7650
void dAcNpcSpPlayerSelect_c::talk_c::initEndDS() {
    if (mpOwner->mDsDlActive) {
        dDsDl::g_manager.cancel();
    }
}
// .text 0x7670
void dAcNpcSpPlayerSelect_c::talk_c::executeEndDS() {
    BOOL done;
    if (mpOwner->mDsDlActive) {
        done = dDsDl::g_manager.isDone();
    } else {
        done = TRUE;
    }
    if (done) {
        mpOwner->mDsDlActive = 0;
        switchToRoverSpeaker();
        dSvRuntime::releaseMessage(mpController);
        changeMode(&talk_c::executeWait);
    }
}

// .text 0x7830
void dAcNpcSpPlayerSelect_c::talk_c::onRequestEnd(int kind) {
    if (mRequestEndMethod) {
        Method method = mRequestEndMethod;
        callMessageMethod(method);
        if (method == mRequestEndMethod) {
            mRequestEndMethod = NULL;
        }
    }
}
// .text 0x78C8
void dAcNpcSpPlayerSelect_c::talk_c::hideRoverSpeaker() {
    dBgm::l_mgr.mVolCtrl.mWait.start();
    mpOwner->mTalk.mNoMouth = 1;
    clearSpeakerName();
    reqMsgClose();
}
// .text 0x791C
void dAcNpcSpPlayerSelect_c::talk_c::showRoverSpeaker() {
    dBgm::l_mgr.mVolCtrl.mWait.end();
    mpOwner->mTalk.mNoMouth = 0;
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GUIDE);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(), 2);
    reqMsgClose();
}
// .text 0x79BC
void dAcNpcSpPlayerSelect_c::talk_c::requestMessage(u16 code,
                                                    const char *label) {
    mpController->setNextMsgCode(code);
    mpController->setNextMsgLabel(label);
}
// .text 0x7A04
void dAcNpcSpPlayerSelect_c::setModelColor(u8 color) {
    nw4r::g3d::ResMdl mdl = getModel()->mMdl.getResMdl();
    setMatColor(color, &mdl);
}
// .text 0x7A60
void dAcNpcSpPlayerSelect_c::setMatColor(u8 color, nw4r::g3d::ResMdl *mdl) {
    if (mdl) {
        u32 num = mdl->GetResMatNumEntries();
        for (u32 i = 0; i < num; i++) {
            nw4r::g3d::ResMatChan chan = mdl->GetResMat(i).GetResMatChan();
            nw4r::ut::Color matColor;
            matColor.Set(color, color, color, 0xFF);
            chan.GXSetChanAmbColor(GX_COLOR0, matColor);
            matColor.Set(color, color, color, 0xFF);
            chan.GXSetChanMatColor(GX_COLOR0, matColor);
        }
    }
}
// .text 0x7B1C
int dAcNpcSpPlayerSelect_c::getEarType() {
    return 0x1F;
}
