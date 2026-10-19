#include <game/game/d_a_npc_sp_sisyo.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_rmobj_theater.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_theater_common.hpp>
#include <game/snd/snd_id.hpp>

// Dr. Shrunk at the Marquee. Functions are in target .text order.

namespace dANpcSpSisyo {
static inline dPrivateData_c *getPlayer() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer();
}
static inline dPrivateData_c *getPlayerRaw() {
    return dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
}

// .text 0x5C: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpSisyo_c *npc) {
    return FALSE;
}

// .text 0x64
BASE_PROFILE(A_NPC_SP_SISYO, dAcNpcSpSisyo_c);
} // namespace dANpcSpSisyo

// .text 0x1E0
void dAcNpcSpSisyo_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_SPEAK:
        changeState(STATE_SHOW);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpSisyo {
static const dAcNpcSpSisyo_c::State_s l_states[] = {
    {&dAcNpcSpSisyo_c::initIdle, &dAcNpcSpSisyo_c::executeIdle},
    {&dAcNpcSpSisyo_c::initLook, &dAcNpcSpSisyo_c::executeLook},
    {&dAcNpcSpSisyo_c::initShow, &dAcNpcSpSisyo_c::executeShow},
};

// The parameters of the unit (.bss 0x8, built by __sinit).
struct param_s {
    param_s() {
        _00 = 0;
        _04 = 0;
        mCurtainDelay = 30;
        mBuzzerDelay = 25;
    }

    /* 0x00 */ int _00;
    /* 0x04 */ int _04;
    /* 0x08 */ int mCurtainDelay; // the joke's end .. the curtain falls
    /* 0x0C */ int mBuzzerDelay;  // the lights go down .. SE_buzzer
}; // size 0x10

static param_s l_param;
} // namespace dANpcSpSisyo

// .text 0x270
void dAcNpcSpSisyo_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpSisyo::l_states[state].mInit)();
}

// .text 0x2A8
int dAcNpcSpSisyo_c::preCreate() {
    if (dANpcSpSisyo::getPlayerRaw()->isFlag0(PRIVATE_FLAG0_THEATER_SEEN) &&
        dTheater::g_common.isKyosyoProgram()) {
        mParam = SP_NPC_KYOSYO;
    } else {
        mParam = SP_NPC_SISYO;
    }
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mSisyoTalk);
    return 1;
}

// .text 0x33C
int dAcNpcSpSisyo_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    dSceneCheck::fn_80081858(dANpcSpSisyo::entryDebugPage(this));
    mSoundPlayed = 1;
    mLookAt.setFront(2, l_lookPitchStep, l_lookYawStep);
    mEnvNoise = 1;
    mTimer = 0;
    mBuzzerTimer = 0;
    return 1;
}

// .text 0x3C4
int dAcNpcSpSisyo_c::doDelete() {
    dANpcSpSisyo::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x408
int dAcNpcSpSisyo_c::vtB0() {
    if (mBuzzerTimer > 0 && --mBuzzerTimer <= 0) {
        mAudioObj.startSound(SE_buzzer);
        mEnvNoise = 0;
    }
    if (mEnvNoise) {
        holdSe(SE_Theater_EnvNoise);
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpSisyo::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x4A8
int dAcNpcSpSisyo_c::getEarType() {
    return mNpcItem.isSame(SP_NPC_KYOSYO) ? NPC_EAR_TYPE_NONE
                                          : NPC_EAR_TYPE_SHRUNK;
}

// .text 0x4EC
void dAcNpcSpSisyo_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x500
void dAcNpcSpSisyo_c::executeIdle() {
    if (isAudienceSeated()) {
        setDemoFlag8();
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x548
void dAcNpcSpSisyo_c::initLook() {
    mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
    mStep = SHOW_STEP_OPENING;
}

// .text 0x58C
void dAcNpcSpSisyo_c::executeLook() {
}

// .text 0x590
void dAcNpcSpSisyo_c::initShow() {
    initOpening();
}

// .text 0x594: the emcee's announcement.
void dAcNpcSpSisyo_c::initOpening() {
    mStep = SHOW_STEP_OPENING;
    mEmcee = 1;
    mSisyoTalk.mType = talk_c::TYPE_OPENING;
    mSisyoTalk.start(
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
    mTalk.requestTalk(false, TRUE);
    dPlayerMgr::lookAt(NULL, 0x7F, FALSE, 0xFF);
    dBgm::l_mgr.mStgTheater.mute();
}

// .text 0x618
void dAcNpcSpSisyo_c::executeOpening() {
    if (!mTalk.isBusy() || mSisyoTalk.mDone) {
        initDim();
    }
}

// .text 0x664: the lights go down.
void dAcNpcSpSisyo_c::initDim() {
    mStep = SHOW_STEP_DIM;
    mTimer = DIM_TIME;
    if (dFixFarSwitch2Light_c::sInstance) {
        dFixFarSwitch2Light_c::sInstance->mOn = 0;
    }
    dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_FIXED_VIEW_16;
    mBuzzerTimer = dANpcSpSisyo::l_param.mBuzzerDelay;
}

// .text 0x6B0
void dAcNpcSpSisyo_c::executeDim() {
    if (mTimer > 0) {
        mTimer--;
    } else {
        initRaiseWait();
    }
}

// .text 0x6D0
void dAcNpcSpSisyo_c::initRaiseWait() {
    mStep = SHOW_STEP_RAISE_WAIT;
    mTimer = RAISE_WAIT_TIME;
}

// .text 0x6E4: the player turns to the stage.
void dAcNpcSpSisyo_c::executeRaiseWait() {
    if (mTimer > 0) {
        if (--mTimer == LOOK_TIME) {
            dPlayerMgr::lookAt(&mPos, 0x7F, FALSE, 0xFF);
        }
    } else {
        initRaise();
    }
}

// .text 0x71C: the curtain rises with the show BGM.
void dAcNpcSpSisyo_c::initRaise() {
    mStep = SHOW_STEP_RAISE;
    daRmobjTheater_c::open();
    if (mNpcItem.isSame(SP_NPC_KYOSYO)) {
        dBgm::l_mgr.mStgTheater.playShow(dBgm::StgTheater_c::SHOW_TYPE_KYOSYO);
    } else {
        dBgm::l_mgr.mStgTheater.playShow(dBgm::StgTheater_c::SHOW_TYPE_SISYO);
    }
}

// .text 0x79C
void dAcNpcSpSisyo_c::executeRaise() {
    if (daRmobjTheater_c::isOpen()) {
        initPerform();
    }
}

// .text 0x7D8: the joke.
void dAcNpcSpSisyo_c::initPerform() {
    mStep = SHOW_STEP_PERFORM;
    mEmcee = 0;
    mSisyoTalk.mType = talk_c::TYPE_SHOW;
    mSisyoTalk.start(
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
}

// .text 0x82C
void dAcNpcSpSisyo_c::executePerform() {
    if (!mTalk.isBusy() || mSisyoTalk.mDone) {
        initPerformed();
    }
}

// .text 0x878
void dAcNpcSpSisyo_c::initPerformed() {
    mStep = SHOW_STEP_PERFORMED;
}

// .text 0x884: the first show has an encore.
void dAcNpcSpSisyo_c::executePerformed() {
    if (!dANpcSpSisyo::getPlayerRaw()->isFlag0(PRIVATE_FLAG0_THEATER_SEEN)) {
        initEncoreWait();
    } else {
        initLightsWait();
    }
}

// .text 0x8D4
void dAcNpcSpSisyo_c::initEncoreWait() {
    mStep = SHOW_STEP_ENCORE_WAIT;
    mTimer = ENCORE_WAIT_TIME;
}

// .text 0x8E8
void dAcNpcSpSisyo_c::executeEncoreWait() {
    if (mTimer > 0) {
        if (--mTimer == 0) {
            daRmobjTheater_c::open();
            dBgm::l_mgr.mStgTheater.playEncore();
        }
    } else if (daRmobjTheater_c::isOpen()) {
        initEncore();
    }
}

// .text 0x954
void dAcNpcSpSisyo_c::initEncore() {
    mStep = SHOW_STEP_ENCORE;
    mEmcee = 0;
    mSisyoTalk.mType = talk_c::TYPE_ENCORE;
    mSisyoTalk.start(
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
}

// .text 0x9A8
void dAcNpcSpSisyo_c::executeEncore() {
    if (!mTalk.isBusy() || mSisyoTalk.mDone) {
        initEncoreEnd();
    }
}

// .text 0x9F4
void dAcNpcSpSisyo_c::initEncoreEnd() {
    mStep = SHOW_STEP_ENCORE_END;
    daRmobjTheater_c::close();
    dBgm::l_mgr.mStgTheater.playEncoreEnd();
}

// .text 0xA2C
void dAcNpcSpSisyo_c::executeEncoreEnd() {
    if (daRmobjTheater_c::isClosed()) {
        initLightsWait();
    }
}

// .text 0xA68
void dAcNpcSpSisyo_c::initLightsWait() {
    mStep = SHOW_STEP_LIGHTS_WAIT;
    mTimer = LIGHTS_WAIT_TIME;
}

// .text 0xA7C: the player looks away from the stage.
void dAcNpcSpSisyo_c::executeLightsWait() {
    if (mTimer > 0) {
        if (--mTimer == LOOK_TIME) {
            dPlayerMgr::lookAt(NULL, 0x7F, FALSE, 0xFF);
        }
    } else {
        initLightsUp();
    }
}

// .text 0xAB4
void dAcNpcSpSisyo_c::initLightsUp() {
    mStep = SHOW_STEP_LIGHTS_UP;
    mTimer = LIGHTS_UP_TIME;
}

// .text 0xAC8: the lights come back on; then the player learns the emotion.
void dAcNpcSpSisyo_c::executeLightsUp() {
    if (mTimer > 0) {
        if (--mTimer == 0) {
            if (dFixFarSwitch2Light_c::sInstance) {
                dFixFarSwitch2Light_c::sInstance->mOn = 1;
            }
            dCamera_c::sInstance->restoreModeSlow();
        }
    } else if (!dCamera_c::sInstance->isTransitionActive()) {
        mEnvNoise = 1;
        if (!dANpcSpSisyo::getPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_THEATER_SEEN)) {
            initEnding();
        } else {
            BOOL swap = TRUE;
            if (dANpcSpSisyo::getPlayerRaw()->mEmotions.findEmpty() >= 0) {
                swap = FALSE;
            } else {
                for (int i = 0; i < dPrivateEmotions_c::SLOT_NUM; i++) {
                    if (dANpcSpSisyo::getPlayer()->mEmotions.isValid(i) &&
                        dANpcSpSisyo::getPlayer()->mEmotions.get(i) ==
                            mSisyoTalk.getEmotion()) {
                        swap = FALSE;
                        break;
                    }
                }
            }
            if (swap) {
                initSwap();
            } else {
                initEnding();
            }
        }
    }
}

// .text 0xC1C: four emotions already: forget one.
void dAcNpcSpSisyo_c::initSwap() {
    mEmcee = 1;
    mSisyoTalk.mType = talk_c::TYPE_SWAP;
    mSisyoTalk.start(
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
    mStep = SHOW_STEP_SWAP;
}

// .text 0xC70
void dAcNpcSpSisyo_c::executeSwap() {
    if (!mTalk.isBusy() || mSisyoTalk.mDone) {
        initEnding();
    }
}

// .text 0xCBC: the emcee's goodbye.
void dAcNpcSpSisyo_c::initEnding() {
    mEmcee = 1;
    mSisyoTalk.mType = talk_c::TYPE_ENDING;
    mSisyoTalk.start(
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
    mStep = SHOW_STEP_ENDING;
}

// .text 0xD10
void dAcNpcSpSisyo_c::executeEnding() {
    if (!mTalk.isBusy() || mSisyoTalk.mDone) {
        dSaveTown_c *town = dSaveData_c::getTown();
        town->mVillagerMove.setPlayerValue(dPlayerMgr::getCurrentPlayerSlot(),
                                           4);
        dPlayerMgr::changeSeatedPose(0, 0);
        clearDemoFlag8();
        dDemo::finishTalk(this);
        dBgm::l_mgr.mStgTheater.unmute();
        mStep = SHOW_STEP_DONE;
    }
}

namespace dANpcSpSisyo {
static const dAcNpcSpSisyo_c::Proc l_showSteps[] = {
    &dAcNpcSpSisyo_c::executeOpening,    &dAcNpcSpSisyo_c::executeDim,
    &dAcNpcSpSisyo_c::executeRaiseWait,  &dAcNpcSpSisyo_c::executeRaise,
    &dAcNpcSpSisyo_c::executePerform,    &dAcNpcSpSisyo_c::executePerformed,
    &dAcNpcSpSisyo_c::executeEncoreWait, &dAcNpcSpSisyo_c::executeEncore,
    &dAcNpcSpSisyo_c::executeEncoreEnd,  &dAcNpcSpSisyo_c::executeSwap,
    &dAcNpcSpSisyo_c::executeLightsWait, &dAcNpcSpSisyo_c::executeLightsUp,
    &dAcNpcSpSisyo_c::executeEnding,
};
} // namespace dANpcSpSisyo

// .text 0xDB0
void dAcNpcSpSisyo_c::executeShow() {
    if (mStep < ARRAY_SIZE(dANpcSpSisyo::l_showSteps)) {
        (this->*dANpcSpSisyo::l_showSteps[mStep])();
    }
}

// .text 0xDF0: no show is on and the player sits in the audience.
BOOL dAcNpcSpSisyo_c::isAudienceSeated() {
    if (dTheater::g_common.mOpen) {
        return FALSE;
    }
    return dPlayerMgr::getChairPosition(NULL, dPlayerMgr::PLAYER_INDEX_CURRENT);
}

// .text 0xE1C
void dAcNpcSpSisyo_c::getName(dHmnName::Word_c *name, int len) const {
    if (mEmcee) {
        dString::Word_c emcee(dNpcInfo::SP_NAME_EMCEE,
                              "sys_STRING/STR_SPNpc_name");
        name->copy(&emcee, 0);
    } else {
        dAcNpcSp_c::getName(name, len);
    }
}

// .text 0xE88
int dAcNpcSpSisyo_c::getVoiceType() const {
    if (mEmcee) {
        return 1;
    }
    return dAcNpcSp_c::getVoiceType();
}

// .text 0xEA4
dAcNpcSpSisyo_c::talk_c::talk_c(dAcNpcSpSisyo_c *npc)
    : mpOwner(npc), mType(TYPE_NONE) {
    dAcNpc_c::recept_c::init();
    init();
    mEmotion = 0;
    mDone = 0;
}

// .text 0xF20
dAcNpcSpSisyo_c::talk_c::~talk_c() {
}

// .text 0xF78
void dAcNpcSpSisyo_c::talk_c::init() {
    mMsgCode = 0;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mProc = NULL;
    mTimer = 0;
    mDone = 0;
    mShowEnded = 0;
}

// .text 0xFF4
void dAcNpcSpSisyo_c::talk_c::start(dActor_c *target) {
    if (mType == TYPE_OPENING) {
        dAcNpc_c::recept_c::init();
    }
    mpActActor = target;
    init();
    switch (mType) {
    case TYPE_OPENING:
        mEmotion = 0;
        if (dANpcSpSisyo::getPlayerRaw()->isFlag0(PRIVATE_FLAG0_THEATER_SEEN)) {
            mEmotion = dTheater::g_common.mProgram;
        }
        if (mEmotion == 0) {
            mEmotion = dTheater::g_common.drawSisyoProgram();
        }
        dNpcMng_c::sTheaterMsg = mEmotion;
        if (!dANpcSpSisyo::getPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_THEATER_SEEN)) {
            mMsgCode = MSG_THEATER_FIRST;
        } else {
            switch (mpOwner->mParam) {
            case SP_NPC_KYOSYO:
                mMsgCode = MSG_THEATER_FRILLARD;
                break;
            case SP_NPC_SISYO:
                mMsgCode = MSG_THEATER_SHRUNK;
                break;
            }
        }
        break;
    case TYPE_SHOW:
        if (dANpcSpSisyo::getPlayerRaw()->isFlag0(PRIVATE_FLAG0_THEATER_SEEN)) {
            switch (mpOwner->mParam) {
            case SP_NPC_KYOSYO:
                mMsgCode = MSG_SHOW_FRILLARD;
                break;
            case SP_NPC_SISYO:
                mMsgCode = MSG_SHOW_SHRUNK;
                break;
            }
        } else {
            mMsgCode = MSG_SHOW_FIRST;
        }
        break;
    case TYPE_ENCORE:
        mMsgCode = MSG_ENCORE;
        break;
    case TYPE_SWAP:
        mMsgCode = MSG_SWAP_ASK;
        break;
    case TYPE_ENDING:
        mMsgCode = MSG_THEATER_ENDING;
        break;
    }
    if (mType != TYPE_OPENING) {
        dHmnName::Word_c name;
        msgInfo_s info;
        info.mLabel = NULL;
        info.mCode = 0;
        switch (mType) {
        case TYPE_SWAP:
            clearSpeakerName();
            break;
        default:
            mpOwner->getName(&name, 10);
            dAcNpcSpSisyo_c *owner = mpOwner;
            const u16 *str =
                (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer();
            setSpeakerName(str, owner->getNameKind());
            break;
        }
        setVoiceType(mpOwner->getVoiceType());
        getMsgInfo(&info);
        mpController->setNextMsgLabel(info.mLabel);
        mpController->setNextMsgCode(info.mCode);
        reqMsgClose();
        mOnRequestEnd = &talk_c::onRequestEndNone;
    }
}

// .text 0x12DC
int dAcNpcSpSisyo_c::talk_c::getVoiceMode() {
    switch (mType) {
    case TYPE_SWAP:
        return 1;
    }
    return dAcNpc_c::recept_c::getVoiceMode();
}

// .text 0x12FC
void dAcNpcSpSisyo_c::talk_c::getMsgInfo(msgInfo_s *info) {
    switch (mType) {
    case TYPE_SHOW:
    case TYPE_ENCORE:
        info->mLabel = "npc_SP/NPC_shisho";
        info->mCode = mMsgCode;
        break;
    case TYPE_OPENING:
    case TYPE_ENDING:
        info->mLabel = "npc_SP/NPC_theater";
        info->mCode = mMsgCode;
        break;
    case TYPE_SWAP:
        info->mLabel = "sys_MESS/SYS_Player";
        info->mCode = mMsgCode;
        break;
    }
}

// .text 0x136C: the program's emotion is word 0 (the title) / 2 (the swap).
void dAcNpcSpSisyo_c::talk_c::onMessageStart(int arg) {
    switch (mType) {
    case TYPE_SWAP:
        switch (mMessageCode) {
        case MSG_SWAP_ASK:
            mpController->setStringWord(2, mEmotion, "sys_STRING/STR_Reaction");
            break;
        }
        break;
    case TYPE_OPENING:
        switch (mMessageCode) {
        case MSG_THEATER_SHRUNK:
        case MSG_THEATER_FRILLARD:
        case MSG_THEATER_FIRST:
            mpController->setStringWord(0, mEmotion, "sys_STRING/STR_Reaction");
            break;
        }
        break;
    }
}

// .text 0x13F0
void dAcNpcSpSisyo_c::talk_c::onMessageEnd(int kind) {
    switch (mType) {
    case TYPE_OPENING:
        break;
    case TYPE_SHOW:
        switch (mMessageCode) {
        case MSG_FRILLARD_SHTICK:
        case MSG_AHEM:
            mpController->setNextMsgCode(mEmotion + MSG_JOKE);
            break;
        default:
            if (mMessageCode >= MSG_JOKE_FIRST &&
                mMessageCode <= MSG_JOKE_LAST) {
                mOnMessageEnd = &talk_c::endJoke;
                mOnRequestEnd = &talk_c::startCurtainClose;
            }
            break;
        }
        break;
    case TYPE_SWAP:
        switch (mMessageCode) {
        case MSG_SWAP_WHICH:
            for (int i = 0; i < dPrivateEmotions_c::SLOT_NUM; i++) {
                mpController->getSelect()->setMessage(
                    i, dANpcSpSisyo::getPlayer()->mEmotions.get(i),
                    "sys_STRING/STR_Reaction", 0, NULL, 0, 0);
            }
            mpController->getSelect()->setMessage(
                SWAP_CHOICE_CANCEL, STR_REACTION_NEVER_MIND,
                "sys_STRING/STR_Reaction", 0, NULL, 0, 0);
            mpController->mSelect.mCancel = SWAP_CHOICE_CANCEL;
            mpController->mSelect.mNum = SWAP_CHOICE_NUM;
            mpController->openSelect();
            break;
        }
        break;
    }
    switch (mType) {
    case TYPE_OPENING:
    case TYPE_ENCORE:
        if (mpController->mNextMsgCode == 0) {
            mOnMessageEnd = &talk_c::closeMsg;
            mOnRequestEnd = &talk_c::setDone;
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x1624
void dAcNpcSpSisyo_c::talk_c::onAnswer(int arg) {
    int answer = mpController->getAnswer();
    switch (mType) {
    case TYPE_SWAP:
        switch (mMessageCode) {
        case MSG_SWAP_ASK:
            switch (answer) {
            case ANSWER_YES:
                mpController->setNextMsgCode(MSG_SWAP_WHICH);
                break;
            default:
                mOnAnswer = &talk_c::closeMsg;
                mOnRequestEnd = &talk_c::setDone;
                break;
            }
            break;
        case MSG_SWAP_WHICH:
            switch (answer) {
            case 0:
            case 1:
            case 2:
            case 3:
                dANpcSpSisyo::getPlayer()->mEmotions.set(
                    answer, dPrivateEmotions_c::EMPTY);
                break;
            }
            mOnAnswer = &talk_c::closeMsg;
            mOnRequestEnd = &talk_c::setDone;
            break;
        }
        break;
    case TYPE_ENCORE:
        break;
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x1794
void dAcNpcSpSisyo_c::talk_c::preExecute() {
    if (mProc) {
        (this->*mProc)();
    }
    if (mShowEnded) {
        dMsg::Comp_c *controller = mpController;
        if (!controller->isState(&dMsg::Comp_c::executeComplete)) {
            dBgm::l_mgr.mStgTheater.playShowEnd();
            mShowEnded = 0;
        }
    }
}

// .text 0x1838: the ending teaches the program's emotion.
void dAcNpcSpSisyo_c::talk_c::onTalkEnd() {
    switch (mType) {
    case TYPE_ENDING: {
        int slot = -1;
        for (int i = 0; i < dPrivateEmotions_c::SLOT_NUM; i++) {
            if (!dANpcSpSisyo::getPlayer()->mEmotions.isValid(i)) {
                slot = i;
                break;
            }
            if (mEmotion == dANpcSpSisyo::getPlayer()->mEmotions.get(i)) {
                break;
            }
        }
        if (slot >= 0) {
            dANpcSpSisyo::getPlayer()->mEmotions.set(slot, mEmotion);
        }
        dANpcSpSisyo::getPlayer()->setFlag0(PRIVATE_FLAG0_THEATER_SEEN);
        dTheater::g_common.setOpen(1);
        mEmotion = 0;
        break;
    }
    case TYPE_SHOW:
        break;
    case TYPE_ENCORE:
        break;
    }
    mType = TYPE_NONE;
}

// .text 0x1934
void dAcNpcSpSisyo_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1998
void dAcNpcSpSisyo_c::talk_c::initCurtainClose() {
    mTimer = dANpcSpSisyo::l_param.mCurtainDelay;
    mProc = &talk_c::executeCurtainClose;
}

// .text 0x19C8
void dAcNpcSpSisyo_c::talk_c::executeCurtainClose() {
    if (mTimer > 0) {
        if (--mTimer <= 0) {
            daRmobjTheater_c::close();
        }
    } else if (daRmobjTheater_c::isClosed()) {
        setSpeakerFeel(0);
        mDone = 1;
        mProc = NULL;
    }
}

// .text 0x1A58
void dAcNpcSpSisyo_c::talk_c::closeMsg() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    setRequest1(0);
}

// .text 0x1AB0
void dAcNpcSpSisyo_c::talk_c::endJoke() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    setRequest1(0);
    dBgm::l_mgr.mStgTheater.stopShow();
    mShowEnded = 1;
}

// .text 0x1B20
void dAcNpcSpSisyo_c::talk_c::startCurtainClose() {
    initCurtainClose();
}

// .text 0x1B24
void dAcNpcSpSisyo_c::talk_c::setDone() {
    mDone = 1;
}

// .text 0x1B30
void dAcNpcSpSisyo_c::talk_c::onRequestEndNone() {
}
