#include <game/game/d_a_npc_sp_player_create.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_rmobj_bus.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_sv_proc.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/snd/snd_system.hpp>
#include <game/sLib/s_lib.hpp>
#include <cstring>

// Rover on the bus. Functions are in target .text order.

// Frames before the bus leaves the scene once the talk is over
// (initNextSceneWait).
#define PC_NEXT_SCENE_WAIT 145
// Frames between the fade-in and the talk in the bus demo (stepDemoBus_c).
#define PC_BUS_DEMO_WAIT 180

namespace dANpcSpPlayerCreate {
// The label of the bus messages.
const char *l_msgLabel = "sys_MESS/SYS_Bus";

// .text 0x5C
BASE_PROFILE(A_NPC_SP_PLAYER_CREATE, dAcNpcSpPlayerCreate_c);

// The player's looks per look answer (SYS_Bus 0x1F..0x26) and gender.
struct LookSet_s {
    /* 0x0 */ u8 mFace;
    /* 0x1 */ u8 mHair;
    /* 0x2 */ u8 mHairColor;
    /* 0x4 */ int mShirt; // item index
    /* 0x8 */ u8 mShoeKind;
    /* 0x9 */ u8 mShoeColor;
}; // size 0xC
} // namespace dANpcSpPlayerCreate

// .text 0x1E4
int dAcNpcSpPlayerCreate_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mTalkPc);
    mAnmSet.setAnm0(HMN_ANM_XCT_SITDOWN_WAIT1);
    mAnmSet.setAnm1(HMN_ANM_XCT_SITDOWN_WAIT1);
    return SUCCEEDED;
}

// .text 0x248
int dAcNpcSpPlayerCreate_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    if (dScene::getCurrentScene() != dScene::SCENE_DM_BUS_PL_CRT &&
        !(dSaveData_c::getRaw()->isBusNetEventPending() &&
          dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_TOWN)) {
        return FAILED;
    }
    static const dState::mode_c<dAcNpcSpPlayerCreate_c>::Entry modes[] = {
        {"待機", &dAcNpcSpPlayerCreate_c::initWait,
         &dAcNpcSpPlayerCreate_c::executeWait},
        {"デモ待機", &dAcNpcSpPlayerCreate_c::initDemoWait,
         &dAcNpcSpPlayerCreate_c::executeDemoWait},
        {"会話", &dAcNpcSpPlayerCreate_c::initTalk,
         &dAcNpcSpPlayerCreate_c::executeTalk},
        {"トンネル抜け始め調査", &dAcNpcSpPlayerCreate_c::initTunnelExitCheck,
         &dAcNpcSpPlayerCreate_c::executeTunnelExitCheck},
        {"次シーンへの待ち", &dAcNpcSpPlayerCreate_c::initNextSceneWait,
         &dAcNpcSpPlayerCreate_c::executeNextSceneWait},
        {"処理無し", &dAcNpcSpPlayerCreate_c::initNone,
         &dAcNpcSpPlayerCreate_c::executeNone},
        {"バスデモ", &dAcNpcSpPlayerCreate_c::initBusDemo,
         &dAcNpcSpPlayerCreate_c::executeBusDemo},
    };
    dState::mode_c<dAcNpcSpPlayerCreate_c>::initialize(
        "dAcNpcSpPlayerCreate_c", modes, ARRAY_SIZE(modes));
    mCheckGround = 0;
    return SUCCEEDED;
}

// .text 0x340
int dAcNpcSpPlayerCreate_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return NOT_READY;
    }
    return SUCCEEDED;
}

// .text 0x36C
int dAcNpcSpPlayerCreate_c::vtB0() {
    dState::base_c<dAcNpcSpPlayerCreate_c>::execute();
    return SUCCEEDED;
}

// .text 0x3C0
bool dAcNpcSpPlayerCreate_c::canInteract(dDemoActor_c *actor) {
    return actor == NULL ? false : !mTalk.isBusy();
}

// .text 0x3FC
void dAcNpcSpPlayerCreate_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeMode(&dAcNpcSpPlayerCreate_c::executeDemoWait);
        break;
    case DEMO_CMD_TALK:
        mTalkPc.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpPlayerCreate_c::executeTalk);
        break;
    case DEMO_CMD_SPEAK:
        mTalkPc.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpPlayerCreate_c::executeBusDemo);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeMode(&dAcNpcSpPlayerCreate_c::executeWait);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0x930
void dAcNpcSpPlayerCreate_c::initWait() {
}

// .text 0x934
void dAcNpcSpPlayerCreate_c::executeWait() {
    dDemo::requestSpeak(this, NULL);
}

// .text 0x93C
void dAcNpcSpPlayerCreate_c::initDemoWait() {
}

// .text 0x940
void dAcNpcSpPlayerCreate_c::executeDemoWait() {
}

// .text 0x944
void dAcNpcSpPlayerCreate_c::initTalk() {
    mTalk.requestTalk(true, TRUE);
}

// .text 0x954
void dAcNpcSpPlayerCreate_c::executeTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0x994
void dAcNpcSpPlayerCreate_c::initTunnelExitCheck() {
    if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_PL_CRT) {
        daRmobjBus_c::startTunnelExit();
    }
}

// .text 0x9C4
void dAcNpcSpPlayerCreate_c::executeTunnelExitCheck() {
    if (dScene::getCurrentScene() != dScene::SCENE_DM_BUS_PL_CRT ||
        daRmobjBus_c::isTunnelExitStarted()) {
        changeMode(&dAcNpcSpPlayerCreate_c::executeNextSceneWait);
    }
}

// .text 0xB3C
void dAcNpcSpPlayerCreate_c::initNextSceneWait() {
    mTimer = PC_NEXT_SCENE_WAIT;
    dCamera_c::sInstance->startBusWideView();
    dBgm::l_mgr.mStgBus.setSceneFade(SoundSystem::SCENE_FADE_IN);
    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_XCT_SITDOWN_TURN1, 0.0f,
                       cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
}

// .text 0xBB4
void dAcNpcSpPlayerCreate_c::executeNextSceneWait() {
    if (sLib::calcTimer(&mTimer) == 0) {
        changeMode(&dAcNpcSpPlayerCreate_c::executeNone);
        if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_PL_CRT) {
            dSTR::requestBus(dSTR::getBusInfo(), TRUE, TRUE, true, TRUE);
        } else if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_TOWN) {
            dSTR::requestBus(dSTR::getBusInfo(), FALSE, FALSE, true, FALSE);
            dSaveData_c::getTown()->setFlag(SAVE_FLAG_BUS_NET_EVENT_DONE);
        } else if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_LAND) {
        }
    }
}

// .text 0xD88
void dAcNpcSpPlayerCreate_c::initNone() {
}

// .text 0xD8C
void dAcNpcSpPlayerCreate_c::executeNone() {
}

// .text 0xD90
void dAcNpcSpPlayerCreate_c::initBusDemo() {
    static const stepDemoBus_c::Entry steps[] = {
        {"初期化", (stepDemoBus_c::Method)&stepDemoBus_c::init},
        {"フェードイン終了待ち",
         (stepDemoBus_c::Method)&stepDemoBus_c::waitFadeIn},
        {"タイマー開始", (stepDemoBus_c::Method)&stepDemoBus_c::startTimer},
        {"タイマー終了待ち", (stepDemoBus_c::Method)&stepDemoBus_c::waitTimer},
        {"終了", (stepDemoBus_c::Method)&stepDemoBus_c::finish},
    };
    mStepDemoBus.initialize(this, "dAcNpcSpPlayerCreate_c::stepDemoBus_c",
                            steps, ARRAY_SIZE(steps));
}

// .text 0xEA8
void dAcNpcSpPlayerCreate_c::executeBusDemo() {
    mStepDemoBus.execute();
}

// .text 0xEF0
void dAcNpcSpPlayerCreate_c::stepDemoBus_c::init() {
    dBgm::l_mgr.mStgBus.setMute(1);
    dBgm::l_mgr.mStgBus.setSceneFade(SoundSystem::SCENE_FADE_IN);
    nextStep();
}

// .text 0xFA8
void dAcNpcSpPlayerCreate_c::stepDemoBus_c::waitFadeIn() {
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        nextStep();
    }
}

// .text 0x1068
void dAcNpcSpPlayerCreate_c::stepDemoBus_c::startTimer() {
    mTimer = PC_BUS_DEMO_WAIT;
    nextStep();
}

// .text 0x1108
void dAcNpcSpPlayerCreate_c::stepDemoBus_c::waitTimer() {
    if (mTimer != 0) {
        mTimer--;
    }
    if (mTimer == 0) {
        nextStep();
    }
}

// .text 0x11C0
void dAcNpcSpPlayerCreate_c::stepDemoBus_c::finish() {
    mpOwner->changeMode(&dAcNpcSpPlayerCreate_c::executeTalk);
}

// .text 0x1324
dAcNpcSpPlayerCreate_c::talk_c::talk_c(dAcNpcSpPlayerCreate_c *npc)
    : _128(0), mpOwner(npc) {
    init();
    static const dState::mode_c<talk_c>::Entry modes[] = {
        {"待機", &talk_c::initWait, &talk_c::executeWait},
        {"セーブデータ作成処理", &talk_c::initCreateSave,
         &talk_c::executeCreateSave},
        {"カメラ", &talk_c::initCamera, &talk_c::executeCamera},
        {"アナウンス用頭回転", &talk_c::initAnnounceTurn,
         &talk_c::executeAnnounceTurn},
        {"頭回転", &talk_c::initHeadTurn, &talk_c::executeHeadTurn},
    };
    dState::mode_c<talk_c>::initialize("dAcNpcSpPlayerCreate_c::talk_c", modes,
                                       ARRAY_SIZE(modes));
}

// .text 0x1408
void dAcNpcSpPlayerCreate_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mRequestEndMethod = NULL;
    mMsgCode = 0;
}

// .text 0x1458
void dAcNpcSpPlayerCreate_c::talk_c::start(dActor_c *target) {
    mpActActor = target;
    if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_PL_CRT) {
        mMsgCode = MSG_JUST_MADE_IT;
    } else {
        mMsgCode = MSG_RECOGNIZE;
    }
}

// .text 0x14A8
void dAcNpcSpPlayerCreate_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "sys_MESS/SYS_Bus";
    info->mCode = mMsgCode;
}

// .text 0x14C0
void dAcNpcSpPlayerCreate_c::talk_c::onMessageStart(int arg) {
    struct Entry {
        const char *label;
        int code;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpPlayerCreate::l_msgLabel, MSG_TIME_CHECK, &talk_c::setTimeWord},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_TIME_CHECK_AGAIN,
         &talk_c::setTimeWord},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_NEXT_STOP, &talk_c::unmuteBusBgm},
    };
    u16 code = getMessageCode();
    const char *label;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < 3; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code) {
            callMessageMethod(entry->method);
        }
    }
}

// .text 0x1580
void dAcNpcSpPlayerCreate_c::talk_c::setTimeWord() {
    dTime_c now = *dTime_c::getCurrent();
    dString::Word_c word;
    dScript::setAmPm(&word, now.hour);
    mpController->setWord(0, &word);
}

// .text 0x162C
void dAcNpcSpPlayerCreate_c::talk_c::unmuteBusBgm() {
    dBgm::l_mgr.mStgBus.setMute(0);
}

// .text 0x1640
void dAcNpcSpPlayerCreate_c::talk_c::onMessageEnd(int kind) {
    struct Entry {
        const char *label;
        int code;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpPlayerCreate::l_msgLabel, MSG_JUST_MADE_IT,
         &talk_c::startIntro},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_INTRO, &talk_c::askFirstQuestion},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_RECOGNIZE,
         &talk_c::startRecognize},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_TIME_OK,
         &talk_c::askNameAfterTime},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_TIME_THANKS,
         &talk_c::askNameAfterTime},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_JP_03, &talk_c::askNameOrTime},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_JP_04, &talk_c::askNameOrTime},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_SORRY_BOY, &talk_c::askTown},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_SORRY_GIRL, &talk_c::askTown},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_ENVY_DS,
         &talk_c::startCreateSave},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_HOUSING_BOOM,
         &talk_c::startCreateSave},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_ENVY, &talk_c::startCreateSave},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_TOWN_EXCITED,
         &talk_c::startCreateSave},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_LOOK_FIRST + 0,
         &talk_c::setPlayerLook},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_LOOK_FIRST + 1,
         &talk_c::setPlayerLook},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_LOOK_FIRST + 2,
         &talk_c::setPlayerLook},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_LOOK_FIRST + 3,
         &talk_c::setPlayerLook},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_LOOK_FIRST + 4,
         &talk_c::setPlayerLook},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_LOOK_FIRST + 5,
         &talk_c::setPlayerLook},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_LOOK_FIRST + 6,
         &talk_c::setPlayerLook},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_LOOK_LAST,
         &talk_c::setPlayerLook},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_MAP_DS_GIVEN,
         &talk_c::requestDriverSpeaker},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_MAP_GIVEN,
         &talk_c::requestDriverSpeaker},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_NEXT_STOP,
         &talk_c::startAnnounceTurn},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_ABOUT_TO_STOP,
         &talk_c::startHeadTurn},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_WC24_ON, &talk_c::enableNet},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_WC24_OFF, &talk_c::disableNet},
    };
    u16 code = getMessageCode();
    const char *label;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < 27; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code) {
            callMessageMethod(entry->method);
        }
    }
    switch (kind) {
    case END_REQ_TIME_MENU:
        reqMenu06(0);
        mRequestEndMethod = &talk_c::reloadSeason;
        break;
    case END_REQ_TOWN_MENU:
        reqMenu0C(0);
        break;
    case END_REQ_NAME_MENU:
        reqMenu0B(0);
        mRequestEndMethod = &talk_c::checkName;
        break;
    case END_REQ_GIVE_MAP:
        if (isCurrentMethod(&talk_c::executeCreateSave)) {
            mpController->mLock = 1;
        }
        reqMenu05(0);
        break;
    }
}

// .text 0x183C
void dAcNpcSpPlayerCreate_c::talk_c::enableNet() {
    dWifi::enableNet();
}

// .text 0x1840
void dAcNpcSpPlayerCreate_c::talk_c::disableNet() {
    dWifi::disableNet();
}

// .text 0x1844
void dAcNpcSpPlayerCreate_c::talk_c::startIntro() {
    mpController->setNextMsgCode(MSG_INTRO);
    setRequest1(0);
    mRequestEndMethod = &talk_c::startFrontView;
}

// .text 0x18A0
void dAcNpcSpPlayerCreate_c::talk_c::askFirstQuestion() {
    int language = getLanguage();
    if (dNpcInfo::playerCreate_c::get()->mFromDs) {
        if (dSaveData_c::get()->isGood()) {
            mpController->setNextMsgCode(MSG_ENVY_DS);
        } else {
            mpController->setNextMsgCode(MSG_TIME_CHECK);
        }
    } else if (language == LANGUAGE_JP) {
        mpController->setNextMsgCode(MSG_INTRO_JP);
    } else {
        askNameOrTime();
    }
}

// .text 0x1938
void dAcNpcSpPlayerCreate_c::talk_c::startRecognize() {
    setRequest1(0);
    mRequestEndMethod = &talk_c::startFrontView;
}

// .text 0x1984
void dAcNpcSpPlayerCreate_c::talk_c::askNameOrTime() {
    if (dSaveData_c::get()->isGood()) {
        mpController->setNextMsgCode(MSG_ASK_NAME);
    } else {
        mpController->setNextMsgCode(MSG_TIME_CHECK);
    }
}

// .text 0x19D8
void dAcNpcSpPlayerCreate_c::talk_c::askNameAfterTime() {
    if (dNpcInfo::playerCreate_c::get()->mFromDs) {
        mpController->setNextMsgCode(MSG_ASK_TOWN_DS);
    } else {
        mpController->setNextMsgCode(MSG_ASK_NAME);
    }
}

// .text 0x1A2C
void dAcNpcSpPlayerCreate_c::talk_c::askTown() {
    if (dSaveData_c::get()->isGood()) {
        mpController->setNextMsgCode(MSG_ENVY);
    } else {
        mpController->setNextMsgCode(MSG_ASK_TOWN);
    }
}

// .text 0x1A80
void dAcNpcSpPlayerCreate_c::talk_c::setPlayerLook() {
    static const dANpcSpPlayerCreate::LookSet_s
        lookSets[MSG_LOOK_LAST - MSG_LOOK_FIRST + 1][2] = {
            {{4, 1, 0, 0x376, 0, 4}, {14, 13, 0, 0x3A8, 1, 1}},
            {{5, 7, 1, 0x391, 0, 4}, {13, 16, 6, 0x3F2, 1, 1}},
            {{6, 1, 0, 0x384, 0, 4}, {10, 16, 0, 0x3D6, 1, 1}},
            {{7, 7, 1, 0x37C, 0, 4}, {8, 13, 6, 0x37D, 1, 1}},
            {{2, 7, 1, 0x3A6, 0, 4}, {11, 16, 0, 0x44B, 1, 1}},
            {{3, 1, 1, 0x383, 0, 4}, {15, 16, 1, 0x3AF, 1, 1}},
            {{0, 1, 0, 0x410, 0, 4}, {12, 13, 1, 0x44D, 1, 1}},
            {{1, 7, 0, 0x425, 0, 4}, {9, 13, 1, 0x3C2, 1, 1}},
        };
    int idx = getMessageCode() - MSG_LOOK_FIRST;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int gender = player->mPID.player.mGender;
    player->mFace = lookSets[idx][gender].mFace;
    player->mHair = lookSets[idx][gender].mHair;
    player->mHairColor = lookSets[idx][gender].mHairColor;
    player->mEquipment.mShirt = dItem::Item(lookSets[idx][gender].mShirt);
    player->mShoeKind = lookSets[idx][gender].mShoeKind;
    player->mShoeColor = lookSets[idx][gender].mShoeColor;
    player->mCatalog.registerDefaults(-1);
}

// .text 0x1B44
void dAcNpcSpPlayerCreate_c::talk_c::startCreateSave() {
    changeMode(&talk_c::executeCreateSave);
}

// .text 0x1CA0
void dAcNpcSpPlayerCreate_c::talk_c::requestDriverSpeaker() {
    setRequest1(0);
    mRequestEndMethod = &talk_c::showDriverSpeaker;
}

// .text 0x1CEC
void dAcNpcSpPlayerCreate_c::talk_c::startAnnounceTurn() {
    dBgm::l_mgr.mStgBus.setMute(1);
    changeMode(&talk_c::executeAnnounceTurn);
}

// .text 0x1E5C
void dAcNpcSpPlayerCreate_c::talk_c::startHeadTurn() {
    changeMode(&talk_c::executeHeadTurn);
}

// .text 0x1FB8
void dAcNpcSpPlayerCreate_c::talk_c::onAnswer(int answer) {
    struct Entry {
        const char *label;
        int code;
        int answer;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpPlayerCreate::l_msgLabel, MSG_INTRO_JP, 1, &talk_c::setKanaOff},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_INTRO_JP, 0, &talk_c::setKanaOn},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_NAME_BOY, 0, &talk_c::setMale},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_NAME_BOY, 1, &talk_c::setFemale},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_NAME_GIRL, 0, &talk_c::setFemale},
        {dANpcSpPlayerCreate::l_msgLabel, MSG_NAME_GIRL, 1, &talk_c::setMale},
    };
    u16 code = getMessageCode();
    const char *label;
    int selected = mpController->mSelect.mCursor;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < 6; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code &&
            selected == entry->answer) {
            callMessageMethod(entry->method);
        }
    }
}

// .text 0x2098
void dAcNpcSpPlayerCreate_c::talk_c::setKanaOff() {
    dSaveOption::setKana(FALSE);
}

// .text 0x20A0
void dAcNpcSpPlayerCreate_c::talk_c::setKanaOn() {
    dSaveOption::setKana(TRUE);
}

// .text 0x20A8
void dAcNpcSpPlayerCreate_c::talk_c::setMale() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    player->mPID.player.mGender = GENDER_MALE;
    if (mpController->mSelect.mCursor == 0) {
        askTown();
    }
}

// .text 0x20F4
void dAcNpcSpPlayerCreate_c::talk_c::setFemale() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    player->mPID.player.mGender = GENDER_FEMALE;
    if (mpController->mSelect.mCursor == 0) {
        askTown();
    }
}

// .text 0x2140
void dAcNpcSpPlayerCreate_c::talk_c::preExecute() {
    dState::base_c<talk_c>::execute();
}

// .text 0x2190
void dAcNpcSpPlayerCreate_c::talk_c::initWait() {
}

// .text 0x2194
void dAcNpcSpPlayerCreate_c::talk_c::executeWait() {
}

// .text 0x2198
void dAcNpcSpPlayerCreate_c::talk_c::initCreateSave() {
    if (dNpcInfo::playerCreate_c::get()->mFromDs) {
        if (!dSaveData_c::get()->isGood()) {
            mSaveMode = dSvProc_c::MODE_NEW_VISITOR_TOWN;
        } else {
            mSaveMode = dSvProc_c::MODE_NEW_VISITOR_PLAYER;
        }
    } else if (!dSaveData_c::get()->isGood()) {
        mSaveMode = dSvProc_c::MODE_NEW_TOWN;
    } else {
        mSaveMode = dSvProc_c::MODE_NEW_PLAYER;
    }
    dSvProc::sProc.reset();
}

// .text 0x2228
void dAcNpcSpPlayerCreate_c::talk_c::executeCreateSave() {
    if (dSvProc::sProc.process(mSaveMode)) {
        mpController->mLock = 0;
        changeMode(&talk_c::executeWait);
    }
}

// .text 0x23A8
void dAcNpcSpPlayerCreate_c::talk_c::initCamera() {
}

// .text 0x23AC
void dAcNpcSpPlayerCreate_c::talk_c::executeCamera() {
    if (!dCamera_c::sInstance->isTransitionActive()) {
        reqMsgClose();
        changeMode(&talk_c::executeWait);
    }
}

// .text 0x2524
void dAcNpcSpPlayerCreate_c::talk_c::initAnnounceTurn() {
    mpController->mLock = 1;
    mpOwner->mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_XCT_SITDOWN_TURN1, 0.0f,
                                cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
}

// .text 0x255C
void dAcNpcSpPlayerCreate_c::talk_c::executeAnnounceTurn() {
    if (mpOwner->mAction.mCanChange &&
        mpOwner->getModel()->mAnm.mAnmId == HMN_ANM_XCT_SITDOWN_TURN1) {
        mpOwner->mAnmSet.setAnm0(HMN_ANM_XCT_SITDOWN_WAIT2);
        mpOwner->mAnmSet.setAnm1(HMN_ANM_XCT_SITDOWN_WAIT2);
        mpOwner->mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mpController->mLock = 0;
        setRequest1(0);
        mRequestEndMethod = &talk_c::showRoverSpeaker;
        changeMode(&talk_c::executeWait);
    }
}

// .text 0x2754
void dAcNpcSpPlayerCreate_c::talk_c::initHeadTurn() {
    mpController->mLock = 1;
    mpOwner->mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_XCT_SITDOWN_TURN2, 0.0f,
                                cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
}

// .text 0x278C
void dAcNpcSpPlayerCreate_c::talk_c::executeHeadTurn() {
    if (daRmobjBus_c::isSeasonReloaded() && mpOwner->mAction.mCanChange &&
        mpOwner->getModel()->mAnm.mAnmId == HMN_ANM_XCT_SITDOWN_TURN2) {
        mpOwner->mAnmSet.setAnm0(HMN_ANM_XCT_SITDOWN_WAIT1);
        mpOwner->mAnmSet.setAnm1(HMN_ANM_XCT_SITDOWN_WAIT1);
        mpOwner->mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mpController->mLock = 0;
        changeMode(&talk_c::executeWait);
    }
}

// .text 0x2964
void dAcNpcSpPlayerCreate_c::talk_c::onTalkEnd() {
    mpOwner->changeMode(&dAcNpcSpPlayerCreate_c::executeTunnelExitCheck);
}

// .text 0x2AC8
void dAcNpcSpPlayerCreate_c::talk_c::onRequestEnd(int kind) {
    if (mRequestEndMethod) {
        Method method = mRequestEndMethod;
        callMessageMethod(method);
        if (method == mRequestEndMethod) {
            mRequestEndMethod = NULL;
        }
    }
}

// .text 0x2B60
void dAcNpcSpPlayerCreate_c::talk_c::startFrontView() {
    dCamera_c::sInstance->startBusFrontView();
    dBgm::l_mgr.mStgBus.setSceneFade(SoundSystem::SCENE_FADE_OUT);
    changeMode(&talk_c::executeCamera);
}

// .text 0x2CDC
void dAcNpcSpPlayerCreate_c::talk_c::showDriverSpeaker() {
    mpOwner->mTalk.mNoMouth = 1;
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_KAPPN);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(),
        GENDER_OTHER);
    reqMsgClose();
}

// .text 0x2D6C
void dAcNpcSpPlayerCreate_c::talk_c::showRoverSpeaker() {
    mpOwner->mTalk.mNoMouth = 0;
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GUIDE);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(),
        GENDER_OTHER);
    reqMsgClose();
}

// .text 0x2DFC
void dAcNpcSpPlayerCreate_c::talk_c::reloadSeason() {
    daRmobjBus_c::reloadSeason();
}

// .text 0x2E00
void dAcNpcSpPlayerCreate_c::talk_c::checkName() {
    dMsg::Comp_c *controller = mpController;
    u16 code = getMessageCode();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dHmnName::Word_c name;
    player->mPID.setWord(&name);
    BOOL same = FALSE;
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *other = dPlayerMgr::Mgr_c::getPlayer(i);
        if (other != player) {
            dHmnName::Word_c otherName;
            other->mPID.setWord(&otherName);
            if (name.isSame(&otherName)) {
                same = TRUE;
                break;
            }
        }
    }
    if (!same) {
        if (code == MSG_ASK_NAME) {
            controller->setNextMsgCode(MSG_NAME_CHECK);
        } else {
            controller->setNextMsgCode(MSG_NAME_CHECK_AGAIN);
        }
    } else {
        controller->setNextMsgCode(MSG_NAME_SAME);
    }
}

// .text 0x2F08
int dAcNpcSpPlayerCreate_c::getEarType() {
    return NPC_EAR_TYPE_GUIDE;
}
