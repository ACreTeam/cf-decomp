#include <game/game/d_a_npc_sp_kappei.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str.hpp>
#include <game/cLib/c_math.hpp>
#include <game/snd/snd_system.hpp>

// Kapp'n driving the bus. Functions are in target .text order.

namespace dANpcSpKappei {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_KAPPEI, dAcNpcSpKappei_c);

// The parameters of the unit (.bss 0x8, built by __sinit).
struct param_s {
    param_s() : mSideViewDelay(120), mReturnViewDelay(35), mArriveDelay(90) {
    }
    int mSideViewDelay;   // frames until the side view (STATE_TALK)
    int mReturnViewDelay; // after the talk: frames until the camera turns back
    int mArriveDelay;     // after the talk: frames until the bus arrives
};
static param_s l_param;
} // namespace dANpcSpKappei

// .text 0x298
void dAcNpcSpKappei_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_WAIT);
        break;
    case DEMO_CMD_TALK:
        mBusTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        mBusTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_SPEAK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpKappei {
static const dAcNpcSpKappei_c::State_s l_states[] = {
    {&dAcNpcSpKappei_c::initIdle, &dAcNpcSpKappei_c::executeIdle},
    {&dAcNpcSpKappei_c::initWait, &dAcNpcSpKappei_c::executeWait},
    {&dAcNpcSpKappei_c::initTalk, &dAcNpcSpKappei_c::executeTalk},
    {&dAcNpcSpKappei_c::initDone, &dAcNpcSpKappei_c::executeDone},
    {&dAcNpcSpKappei_c::initSpeak, &dAcNpcSpKappei_c::executeSpeak},
};
} // namespace dANpcSpKappei

// .text 0x364
void dAcNpcSpKappei_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpKappei::l_states[state].mInit)();
}

// .text 0x39C
int dAcNpcSpKappei_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mBusTalk);
    mAnmSet.setAnm0(HMN_ANM_WIP_DRV1);
    mAnmSet.setAnm1(HMN_ANM_WIP_DRV1);
    mAnmSet.setAnm2(HMN_ANM_WIP_DRV1);
    return SUCCEEDED;
}

// .text 0x40C
int dAcNpcSpKappei_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    bool created = true;
    created &= mWheelMdl.create("obj_bus_handl", 0);
    if (!created) {
        return NOT_READY;
    }
    mWheelMdl.setAnm("obj_bus_handl_0", m3d::FORWARD_LOOP, 0.0f);
    dNpcInfo::kappei_c::get()->setLyleTalked(FALSE);
    mCheckGround = 0;
    if (dScene::getCurrentScene() != dScene::SCENE_DM_BUS_PL_CRT &&
        !(dSaveData_c::getRaw()->isBusNetEventPending() &&
          dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_TOWN)) {
        dBgm::l_mgr.mStgBus.setSceneFade(SoundSystem::SCENE_FADE_IN);
    }
    mSideViewTimer = 0;
    mReturnViewTimer = 0;
    mArriveTimer = 0;
    return SUCCEEDED;
}

// .text 0x4F0
int dAcNpcSpKappei_c::doDelete() {
    bool removed = true;
    removed &= mWheelMdl.remove();
    if (removed == false) {
        return NOT_READY;
    }
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x544
int dAcNpcSpKappei_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpKappei::l_states[mState].mExecute)();
    }
    if (mWheelMdl.isStop()) {
        switch (mBusTalk.mFeel) {
        case talk_c::FEEL_WHEEL_1:
            mWheelMdl.setAnm("obj_bus_handl_2", m3d::FORWARD_LOOP, 0.0f);
            break;
        case talk_c::FEEL_WHEEL_2:
            mWheelMdl.setAnm("obj_bus_handl_4", m3d::FORWARD_LOOP, 0.0f);
            break;
        case talk_c::FEEL_WHEEL_3:
            mWheelMdl.setAnm("obj_bus_handl_6", m3d::FORWARD_LOOP, 0.0f);
            break;
        }
    }
    mWheelMdl.play();
    return SUCCEEDED;
}

// .text 0x638
int dAcNpcSpKappei_c::draw() {
    mWheelMdl.entry();
    return dAcNpc_c::draw();
}

// .text 0x670
void dAcNpcSpKappei_c::initIdle() {
}

// .text 0x674: on a ride, ask to speak to the player.
void dAcNpcSpKappei_c::executeIdle() {
    if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_PL_CRT ||
        (dSaveData_c::getRaw()->isBusNetEventPending() &&
         dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_TOWN)) {
        return;
    }
    dDemo::requestSpeak(this, NULL);
}

// .text 0x6DC
void dAcNpcSpKappei_c::initWait() {
}

// .text 0x6E0
void dAcNpcSpKappei_c::executeWait() {
}

// .text 0x6E4
void dAcNpcSpKappei_c::initTalk() {
    mSideViewTimer = dANpcSpKappei::l_param.mSideViewDelay;
    mStep = TALK_STEP_SIDE_VIEW;
}

// .text 0x6FC
void dAcNpcSpKappei_c::stepSideView() {
    if (mSideViewTimer > 0 && --mSideViewTimer <= 0) {
        dCamera_c::sInstance->startBusSideView();
        dBgm::l_mgr.mStgBus.setSceneFade(SoundSystem::SCENE_FADE_NONE);
        mStep = TALK_STEP_START;
    }
}

// .text 0x764
void dAcNpcSpKappei_c::stepStartTalk() {
    if (!dCamera_c::sInstance->isTransitionActive()) {
        dBgm::l_mgr.mStgBus.setMute(FALSE);
        mTalk.requestTalk(true, TRUE);
        mStep = TALK_STEP_TALK;
    }
}

// .text 0x7CC
void dAcNpcSpKappei_c::stepTalk() {
    if (!mTalk.isBusy()) {
        mStep = TALK_STEP_ARRIVE;
        mReturnViewTimer = dANpcSpKappei::l_param.mReturnViewDelay;
        mArriveTimer = dANpcSpKappei::l_param.mArriveDelay;
    }
}

// .text 0x824
void dAcNpcSpKappei_c::stepArrive() {
    if (mReturnViewTimer > 0 && --mReturnViewTimer == 0) {
        dCamera_c::sInstance->startBusReturnView();
        dBgm::l_mgr.mStgBus.setSceneFade(SoundSystem::SCENE_FADE_IN);
    }
    if (mArriveTimer > 0 && --mArriveTimer == 0) {
        dDemo::finishTalk(this);
        if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_TOWN) {
            dSTR::requestBus(dSTR::getBusInfo(), FALSE, FALSE, false, FALSE);
        } else if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_LAND) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_BUS_RETURN_RIDDEN);
            dSTR::requestBus(dSTR::getBusInfo(), TRUE, FALSE, false, FALSE);
        }
        changeState(STATE_DONE);
    }
}

namespace dANpcSpKappei {
static const dAcNpcSpKappei_c::Proc l_talkSteps[] = {
    &dAcNpcSpKappei_c::stepSideView,
    &dAcNpcSpKappei_c::stepStartTalk,
    &dAcNpcSpKappei_c::stepTalk,
    &dAcNpcSpKappei_c::stepArrive,
};
} // namespace dANpcSpKappei

// .text 0x910
void dAcNpcSpKappei_c::executeTalk() {
    if (mStep < TALK_STEP_COUNT) {
        (this->*dANpcSpKappei::l_talkSteps[mStep])();
    }
}

// .text 0x950
void dAcNpcSpKappei_c::initDone() {
}

// .text 0x954
void dAcNpcSpKappei_c::executeDone() {
}

// .text 0x958
void dAcNpcSpKappei_c::initSpeak() {
    mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
    mStep = SPEAK_STEP_WAIT;
}

namespace dANpcSpKappei {
static const dAcNpcSpKappei_c::Proc l_speakSteps[] = {
    &dAcNpcSpKappei_c::stepSpeakWait,
};
} // namespace dANpcSpKappei

// .text 0x99C
void dAcNpcSpKappei_c::executeSpeak() {
    if (mStep < SPEAK_STEP_COUNT) {
        (this->*dANpcSpKappei::l_speakSteps[mStep])();
    }
}

// .text 0x9DC (+ a dead blr at 0x9F0)
void dAcNpcSpKappei_c::stepSpeakWait() {
    if (mAction.mActionId == ACTION_WAIT) {
        changeState(STATE_TALK);
    }
}

// .text 0x9F4
dAcNpcSpKappei_c::talk_c::talk_c(dAcNpcSpKappei_c *npc) : mpOwner(npc) {
    init();
}

// .text 0xA54
dAcNpcSpKappei_c::talk_c::~talk_c() {
}

// .text 0xAAC
void dAcNpcSpKappei_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mFeel = FEEL_NONE;
}

// .text 0xB18: the story of the ride.
void dAcNpcSpKappei_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_KAPPN_MET)) {
        if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_TOWN) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                    PRIVATE_FLAG0_CITY_VISITED)) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_BUS_RETURN_RIDDEN);
            }
            dPrivateData_c *player;
            do {
                mMsgNo = (u16)cM::rndInt(MSG_STORY_NUM) + MSG_TO_CITY;
                player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            } while (mMsgNo == player->getKappnCityMsg());
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setKappnCityMsg(mMsgNo);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_BUS_RETURN_RIDDEN)) {
            dPrivateData_c *player;
            do {
                mMsgNo = (u16)cM::rndInt(MSG_STORY_NUM) + MSG_TO_TOWN;
                player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            } while (mMsgNo == player->getKappnTownMsg());
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setKappnTownMsg(mMsgNo);
        } else {
            mMsgNo = MSG_RETURN_FORTUNES;
        }
    } else if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_TOWN) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                PRIVATE_FLAG0_CITY_VISITED)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_BUS_RETURN_RIDDEN);
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_BUS_RETURN_RIDDEN)) {
            mMsgNo = MSG_RODE_BEFORE;
        } else {
            mMsgNo = MSG_FIRST_RIDE;
        }
    } else {
        mMsgNo = MSG_FIRST_RETURN;
    }
}

// .text 0xCA0
void dAcNpcSpKappei_c::talk_c::getMsgInfo(msgInfo_s *info) {
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID.player.mGender ==
        GENDER_MALE) {
        info->mLabel = "npc_SP/NPC_kappeiM";
    } else {
        info->mLabel = "npc_SP/NPC_kappeiF";
    }
    info->mCode = mMsgNo;
}

// .text 0xD08
void dAcNpcSpKappei_c::talk_c::onMessageStart(int arg) {
    dBgm::l_mgr.mStgBus.setMute(FALSE);
}

// .text 0xD1C
void dAcNpcSpKappei_c::talk_c::onMessageEnd(int kind) {
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0xD80
void dAcNpcSpKappei_c::talk_c::onAnswer(int arg) {
}

// .text 0xD84
void dAcNpcSpKappei_c::talk_c::preExecute() {
}

// .text 0xD88
void dAcNpcSpKappei_c::talk_c::onTalkEnd() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_KAPPN_MET);
}

// .text 0xDB0: the feel tags also turn the wheel.
void dAcNpcSpKappei_c::talk_c::setSpeakerFeel(u32 feel) {
    mFeel = feel;
    setFeel(feel, mSpeaker, 10.0f);
    switch (feel) {
    case FEEL_NONE:
        mpOwner->mWheelMdl.setAnm("obj_bus_handl_0", m3d::FORWARD_LOOP, 10.0f);
        break;
    case FEEL_WHEEL_1:
        mpOwner->mWheelMdl.setAnm("obj_bus_handl_1", m3d::FORWARD_ONCE, 10.0f);
        break;
    case FEEL_WHEEL_2:
        mpOwner->mWheelMdl.setAnm("obj_bus_handl_3", m3d::FORWARD_ONCE, 10.0f);
        break;
    case FEEL_WHEEL_3:
        mpOwner->mWheelMdl.setAnm("obj_bus_handl_5", m3d::FORWARD_ONCE, 10.0f);
        break;
    }
}

// .text 0xEA4 (+ a dead blr at 0xEC4)
void dAcNpcSpKappei_c::talk_c::onKeyWait() {
    if (mFeel == FEEL_NONE) {
        dBgm::l_mgr.mStgBus.setMute(TRUE);
    }
}

// .text 0xEC8 (+ a dead blr at 0xEE8)
void dAcNpcSpKappei_c::talk_c::onPageWait() {
    if (mFeel == FEEL_NONE) {
        dBgm::l_mgr.mStgBus.setMute(TRUE);
    }
}

// .text 0xEEC (+ a dead blr at 0xF0C)
void dAcNpcSpKappei_c::talk_c::onTextEnd() {
    if (mFeel == FEEL_NONE) {
        dBgm::l_mgr.mStgBus.setMute(TRUE);
    }
}

// .text 0xF10 (+ a dead blr at 0xF30)
void dAcNpcSpKappei_c::talk_c::onSelectStart() {
    if (mFeel == FEEL_NONE) {
        dBgm::l_mgr.mStgBus.setMute(TRUE);
    }
}

// .text 0xF34
void dAcNpcSpKappei_c::talk_c::onTypeStart() {
    dBgm::l_mgr.mStgBus.setMute(FALSE);
}

// .text 0xF48
void dAcNpcSpKappei_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

#ifdef MUST_MATCH
// Matching-only emission order (the d_a_npc_sp_cafe / d_a_rmobj convention).
// There is no evidence for this helper in the original source. MWCC drops
// the unused function; its reference puts anmChr_c::getType before
// anmVis_c::getType among the retained weak inlines.
static void orderKappeiWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
