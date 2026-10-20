#include <game/game/d_a_npc_sp_demo_resetsan.hpp>
#include <game/framework/f_manager.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_random.hpp>
#include <game/game/d_save_data.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_a_rmobj_reset.hpp>

// Mr. Resetti and Don in the Reset Surveillance Center. Functions are in
// target .text order.

namespace dANpcSpDemoResetsan {
// .text 0x5C, 0x1D0
CUSTOM_BASE_PROFILE(NPC_SP_DEMO_RESETSAN, dAcNpcSpDemoResetsan_c,
                    fProf::A_NPC_SP_DEMO_RESETSAN_099,
                    fProf::DRAW_ORDER::A_NPC_SP_DEMO_RESETSAN_099);
CUSTOM_BASE_PROFILE(NPC_SP_DEMO_RACKETSAN, dAcNpcSpDemoRacketsan_c,
                    fProf::A_NPC_SP_DEMO_RESETSAN_09A,
                    fProf::DRAW_ORDER::A_NPC_SP_DEMO_RESETSAN_09A);

// Tuning (.bss 0x8, built by __sinit).
struct param_s {
    param_s() : _0(0), mLookPitchMax(0x3FF), mLookYawMax(0xFFF), mBackWait(60) {
    }
    int _0;            // unused
    int mLookPitchMax; // head look limits (vtB0)
    int mLookYawMax;
    int mBackWait; // frames after the seat came back (procBack)
};
static param_s l_param;

// Resetti (procWaitCamera .. procBack run on Don's talk too).
inline dAcNpcSpDemoResetsanBase_c *getResetti() {
    return static_cast<dAcNpcSpDemoResetsanBase_c *>(
        fManager_c::searchBaseByProfName(fProf::A_NPC_SP_DEMO_RESETSAN_099,
                                         NULL));
}

// The machinery's state (STATE_NONE without it).
inline int getRoomState() {
    return daRmobjReset_c::s_instance ? daRmobjReset_c::s_instance->mState
                                      : daRmobjReset_c::STATE_NONE;
}
} // namespace dANpcSpDemoResetsan

// .text 0x344
void dAcNpcSpDemoResetsanBase_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_DEMO_WAIT);
        break;
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK:
        getTalk()->start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpDemoResetsan {
static const dAcNpcSpDemoResetsanBase_c::State_s l_states[] = {
    {&dAcNpcSpDemoResetsanBase_c::initIdle,
     &dAcNpcSpDemoResetsanBase_c::executeIdle},
    {&dAcNpcSpDemoResetsanBase_c::initDemoWait,
     &dAcNpcSpDemoResetsanBase_c::executeDemoWait},
    {&dAcNpcSpDemoResetsanBase_c::initTalk,
     &dAcNpcSpDemoResetsanBase_c::executeTalk},
    {&dAcNpcSpDemoResetsanBase_c::initPartnerTalk,
     &dAcNpcSpDemoResetsanBase_c::executePartnerTalk},
};
} // namespace dANpcSpDemoResetsan

// .text 0x3F8
void dAcNpcSpDemoResetsanBase_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpDemoResetsan::l_states[state].mInit)();
}

// .text 0x430
int dAcNpcSpDemoResetsanBase_c::preCreate() {
    if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_RESET_CENTER_SHOVEL)) {
        mVisit = VISIT_FIRST;
    } else if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_RESET_CENTER_VISIT2)) {
        mVisit = VISIT_SECOND;
    } else if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_RESET_CENTER_VISIT3)) {
        mVisit = VISIT_THIRD;
    } else if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_RESET_CENTER_VISIT4)) {
        mVisit = VISIT_THIRD;
    } else if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_RESET_CENTER_DON_VISIT)) {
        mVisit = VISIT_DON;
    } else {
        dRandom_c rnd(0x9D);
        dTime_c now = *dTime_c::getCurrent();
        rnd.initFromDateAndLandId(now, 0x232ACAC);
        int roll = rnd.rndF(100.0f);
        if (roll < 22) {
            mVisit = VISIT_DON_ONLY;
        } else if (roll < 34) {
            mVisit = VISIT_CHAT;
        } else if (roll < 56) {
            mVisit = VISIT_RESETTI;
        } else if (roll < 78) {
            mVisit = VISIT_CHAT2;
        } else {
            mVisit = VISIT_EMPTY;
        }
    }
    if (mVisit <= VISIT_FIRST) {
        mParam = SP_NPC_RESETTI_OFF;
    }
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    if (mVisit > VISIT_FIRST) {
        if (isResetti()) {
            mAnmSet.setAnm0(HMN_ANM_MOO_SITDOWN_WAIT1);
            mAnmSet.setAnm1(HMN_ANM_MOO_SITDOWN_WAIT1);
            mAnmSet.setAnm2(HMN_ANM_MOO_SITDOWN_WAIT1);
        } else {
            mAnmSet.setAnm0(HMN_ANM_MOC_SITDOWN_WAIT1);
            mAnmSet.setAnm1(HMN_ANM_MOC_SITDOWN_WAIT1);
            mAnmSet.setAnm2(HMN_ANM_MOC_SITDOWN_WAIT1);
        }
    }
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mVillagerMove.setPlayerValue(dPlayerMgr::getCurrentPlayerSlot(), 8);
    return SUCCEEDED;
}

// .text 0x6A8
int dAcNpcSpDemoResetsan_c::preCreate() {
    if (!dAcNpcSpDemoResetsanBase_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mResetTalk);
    return SUCCEEDED;
}

// .text 0x6F4
int dAcNpcSpDemoRacketsan_c::preCreate() {
    if (!dAcNpcSpDemoResetsanBase_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mRacketTalk);
    return SUCCEEDED;
}

// .text 0x740
int dAcNpcSpDemoResetsanBase_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    mTimer = 0;
    if (isDon()) {
        if (mVisit <= VISIT_THIRD || mVisit == VISIT_RESETTI ||
            mVisit == VISIT_EMPTY) {
            return FAILED;
        }
    } else if (mVisit == VISIT_DON_ONLY || mVisit == VISIT_EMPTY) {
        return FAILED;
    }
    changeState(STATE_IDLE);
    if (mVisit <= VISIT_FIRST) {
        mAngle3D.y = -0x2AAB;
        mAngle.y = -0x2AAB;
        mPos.z += 64.0f;
    } else {
        mCheckGround = 0;
    }
    return SUCCEEDED;
}

// .text 0x828
int dAcNpcSpDemoResetsanBase_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x854
int dAcNpcSpDemoResetsanBase_c::vtB0() {
    mLookAt.mPitch.mMax = dANpcSpDemoResetsan::l_param.mLookPitchMax;
    mLookAt.mYaw.mMax = dANpcSpDemoResetsan::l_param.mLookYawMax;
    if (mState < STATE_COUNT) {
        (this->*dANpcSpDemoResetsan::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x8B4
bool dAcNpcSpDemoResetsanBase_c::canInteract(dDemoActor_c *actor) {
    int visit = mVisit;
    if ((visit == VISIT_DON || visit == VISIT_CHAT || visit == VISIT_CHAT2) &&
        !getTalk()->mTalked) {
        return false;
    }
    if (visit >= VISIT_SECOND &&
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT)->z < mPos.z) {
        return false;
    }
    return dAcNpcSp_c::canInteract(actor);
}

// .text 0x950
int dAcNpcSpDemoResetsanBase_c::canSpeak(dDemoActor_c *actor) {
    if (mIsActive == 0) {
        return 0;
    }
    return mTalk.isBusy() == false;
}

// .text 0x990
int dAcNpcSpDemoResetsan_c::canSpeak(dDemoActor_c *actor) {
    return dAcNpcSpDemoResetsanBase_c::canSpeak(actor);
}

// .text 0x994
int dAcNpcSpDemoRacketsan_c::canSpeak(dDemoActor_c *actor) {
    return dAcNpcSpDemoResetsanBase_c::canSpeak(actor);
}

// .text 0x998: the player talks to one of the brothers while they chat.
BOOL dAcNpcSpDemoResetsanBase_c::checkInterrupt() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    if (dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    if (dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT)->z < mPos.z) {
        return FALSE;
    }
    if (getTalk()->mTalked) {
        return FALSE;
    }
    if (!((isResetti() && (mVisit == VISIT_CHAT || mVisit == VISIT_CHAT2)) ||
          (isDon() && mVisit == VISIT_DON))) {
        return FALSE;
    }
    if (dPad::isConfirmTriggered()) {
        dAcPlayer_c *player =
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
        dItem::Item resettiKey((u16)SP_NPC_RESETTI);
        dAcNpc_c *resetti =
            static_cast<dAcNpc_c *>(dNpcList::getSpByKey(resettiKey));
        if (resetti->mIsActive && resetti->checkInteractionRange(player) &&
            resetti->checkInteractionAngle(
                player, (s16)-dDemoActor_c::mInteractionAngle,
                dDemoActor_c::mInteractionAngle)) {
            return TRUE;
        }
        dItem::Item donKey((u16)SP_NPC_DON);
        dAcNpc_c *don = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(donKey));
        if (don->mIsActive && don->checkInteractionRange(player) &&
            don->checkInteractionAngle(player,
                                       (s16)-dDemoActor_c::mInteractionAngle,
                                       dDemoActor_c::mInteractionAngle)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0xB6C
void dAcNpcSpDemoResetsanBase_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0xB80
void dAcNpcSpDemoResetsanBase_c::executeIdle() {
    if (checkInterrupt()) {
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0xBC0
void dAcNpcSpDemoResetsanBase_c::initDemoWait() {
    mAction.requestWait(PRIORITY_DEMO);
}

// .text 0xBD4
void dAcNpcSpDemoResetsanBase_c::executeDemoWait() {
}

// .text 0xBD8
void dAcNpcSpDemoResetsanBase_c::initTalk() {
    startTalk();
}

// .text 0xBDC
void dAcNpcSpDemoResetsanBase_c::startTalk() {
    if (mVisit >= VISIT_SECOND) {
        mTalk.requestTalk(true, FALSE);
        mLookAt.setPlayerNo(PRIORITY_DEMO, dNet::getMyNo(), false,
                            l_lookPitchStep, l_lookYawStep, 0.0f);
    } else {
        getTalk()->start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
    mStep = TALK_STEP_TALK;
}

// .text 0xCC0
void dAcNpcSpDemoResetsanBase_c::stepTalk() {
    if (!mTalk.isBusy()) {
        if (getOwnTalk()->mScramble) {
            closeDoor();
        } else {
            dDemo::finishTalk(this);
            mStep = TALK_STEP_DONE;
            mLookAt.setPlayer(PRIORITY_DEMO, true, l_lookPitchStep,
                              l_lookYawStep, cNpcLookRange);
        }
    }
}

// .text 0xD5C
void dAcNpcSpDemoResetsanBase_c::closeDoor() {
    if (daRmobjReset_c::s_instance) {
        daRmobjReset_c::s_instance->changeState(
            daRmobjReset_c::STATE_DOOR_CLOSE);
    }
    dBgm::l_mgr.mStgReset.stop2();
    mStep = TALK_STEP_SCRAMBLE;
}

// .text 0xDB4
void dAcNpcSpDemoResetsanBase_c::stepScramble() {
    if (dANpcSpDemoResetsan::getRoomState() == daRmobjReset_c::STATE_END) {
        startTalkEnd();
    }
}

// .text 0xDE0
void dAcNpcSpDemoResetsanBase_c::startTalkEnd() {
    if (daRmobjReset_c::s_instance) {
        daRmobjReset_c::s_instance->lightOff();
    }
    mTimer = 60;
    mStep = TALK_STEP_END;
}

// .text 0xE2C
void dAcNpcSpDemoResetsanBase_c::stepTalkEnd() {
    if (mTimer > 0) {
        mTimer--;
    } else {
        dDemo::finishTalk(this);
        mStep = TALK_STEP_DONE;
        mLookAt.setPlayer(PRIORITY_DEMO, true, l_lookPitchStep, l_lookYawStep,
                          cNpcLookRange);
        dCamera_c::sInstance->mRequestedMode =
            dCamera_c::sInstance->mDefaultMode;
    }
}

namespace dANpcSpDemoResetsan {
static const dAcNpcSpDemoResetsanBase_c::Proc l_talkSteps[] = {
    &dAcNpcSpDemoResetsanBase_c::stepTalk,
    &dAcNpcSpDemoResetsanBase_c::stepScramble,
    &dAcNpcSpDemoResetsanBase_c::stepTalkEnd,
};
} // namespace dANpcSpDemoResetsan

// .text 0xEB0
void dAcNpcSpDemoResetsanBase_c::executeTalk() {
    if (mStep < TALK_STEP_DONE) {
        (this->*dANpcSpDemoResetsan::l_talkSteps[mStep])();
    }
}

// .text 0xEF0
void dAcNpcSpDemoResetsanBase_c::initPartnerTalk() {
}

// .text 0xEF4
void dAcNpcSpDemoResetsanBase_c::executePartnerTalk() {
}

// .text 0xEF8: the partner starts talking.
void dAcNpcSpDemoResetsanBase_c::vt80() {
    mLookAt.setPlayerNo(PRIORITY_IDLE, dNet::getMyNo(), false, l_lookPitchStep,
                        l_lookYawStep, 0.0f);
    changeState(STATE_PARTNER_TALK);
}

// .text 0xF5C: the partner's talk is over.
void dAcNpcSpDemoResetsanBase_c::vt84() {
    mLookAt.setPlayer(PRIORITY_DEMO, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    getOwnTalk()->setTalked();
    changeState(STATE_IDLE);
}

// .text 0xFD0
void dAcNpcSpDemoResetsanBase_c::lookFront() {
    mLookAt.setFront(PRIORITY_DEMO, l_lookPitchStep, l_lookYawStep);
}

// .text 0xFEC
dAcNpcSpDemoResetsanBase_c::talk_c::talk_c(dAcNpcSpDemoResetsanBase_c *npc)
    : mpOwner(npc) {
    mTalked = 0;
    mTalkedAgain = 0;
    mPocketsFullTold = 0;
    init();
}

// .text 0x105C
dAcNpcSpDemoResetsan_c::talkDemoReset_c::talkDemoReset_c(
    dAcNpcSpDemoResetsanBase_c *npc)
    : talk_c(npc) {
}

// .text 0x1098
dAcNpcSpDemoRacketsan_c::talkDemoRacket_c::talkDemoRacket_c(
    dAcNpcSpDemoResetsanBase_c *npc)
    : talk_c(npc) {
}

// .text 0x10D4
dAcNpcSpDemoResetsanBase_c::talk_c::~talk_c() {
}

// .text 0x112C
dAcNpcSpDemoResetsan_c::talkDemoReset_c::~talkDemoReset_c() {
}

// .text 0x1184
dAcNpcSpDemoRacketsan_c::talkDemoRacket_c::~talkDemoRacket_c() {
}

// .text 0x11DC
void dAcNpcSpDemoResetsanBase_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mProc = NULL;
    mScramble = 0;
    mTimer = 0;
    mFrame = 0;
}

// .text 0x1268
int dAcNpcSpDemoResetsanBase_c::talk_c::getMsgCode() const {
    switch (mpOwner->mVisit) {
    case VISIT_FIRST:
        if (!mTalked) {
            return MSG_FIRST;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_RESET_CENTER_SHOVEL) == FALSE) {
            return MSG_FIRST_POCKETS_FULL;
        }
        return MSG_FIRST_TALKED + (mTalkedAgain != 0);
    case VISIT_SECOND:
        return MSG_SECOND + (mTalked != 0);
    case VISIT_THIRD:
        return mTalked ? MSG_THIRD_TALKED : MSG_THIRD;
    case VISIT_DON:
        if (!mTalked) {
            return dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isDonAtCafe()
                       ? MSG_DON
                       : MSG_DON_FIRST;
        }
        return MSG_DON_TALKED + (mTalkedAgain != 0);
    case VISIT_DON_ONLY:
        return mTalked ? MSG_DON_ONLY_TALKED : MSG_DON_ONLY;
    case VISIT_CHAT:
        if (mTalked == 0) {
            return MSG_CHAT;
        }
        return MSG_CHAT_TALKED + (mTalkedAgain != 0);
    case VISIT_RESETTI:
        return mTalked ? MSG_RESETTI_TALKED : MSG_RESETTI;
    case VISIT_CHAT2:
        if (!mTalked) {
            return MSG_CHAT2;
        }
        return mpOwner->isResetti() ? MSG_CHAT2_RESETTI : MSG_CHAT2_DON;
    }
    return 0;
}

// .text 0x140C
void dAcNpcSpDemoResetsanBase_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    mpOwner->isResetti();
    mMsgNo = getMsgCode();
    setPartnerNpc();
}

// .text 0x1488
void dAcNpcSpDemoResetsanBase_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_reset_kanshi_center";
    info->mCode = mMsgNo;
}

// .text 0x14A0
void dAcNpcSpDemoResetsanBase_c::talk_c::onMessageStart(int arg) {
    switch (arg) {
    case TAG_SLAP:
        mpOwner->mAudioObj.startSound(SE_resetsan_viblslap);
        break;
    case TAG_THUNDER:
        mpOwner->mAudioObj.startSound(SE_resetsan_kaminari);
        dPlayerMgr::flinch();
        break;
    }
    switch (mMessageCode) {
    case MSG_SCRAMBLE:
    case MSG_SCRAMBLE_2:
    case MSG_SCRAMBLE_3:
        dBgm::l_mgr.mStgReset.play();
        dANpcSpDemoResetsan::getResetti()->lookFront();
        break;
    }
}

// .text 0x1560
void dAcNpcSpDemoResetsanBase_c::startSwitchAnm() {
    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_MOO_SWITCH1, 0.0f,
                       cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
}

// .text 0x1588
void dAcNpcSpDemoResetsanBase_c::startGearAnm() {
    mAnmSet.setAnm0(HMN_ANM_MOO_RANGER_WAIT1);
    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_MOO_GEAR1, 0.0f, cNpcMorphFrames,
                       m3d::PLAYMODE_INHERIT, 0);
}

// .text 0x15E4
void dAcNpcSpDemoResetsanBase_c::startBackAnm() {
    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_MOO_BACK1, 0.0f, cNpcMorphFrames,
                       m3d::FORWARD_ONCE, 0);
}

// .text 0x160C
void dAcNpcSpDemoResetsanBase_c::talk_c::onMessageEnd(int kind) {
    switch (kind) {
    case TAG_SLAP:
        mpOwner->mAudioObj.startSound(SE_resetsan_viblslap);
        break;
    case TAG_THUNDER:
        mpOwner->mAudioObj.startSound(SE_resetsan_kaminari);
        dPlayerMgr::flinch();
        break;
    }
    if (mMessageCode == MSG_RE_ASK) {
        dMsg::Comp_c *ctrl = getController();
        int code = getMsgCode();
        ctrl->setNextMsgCode(code);
    } else {
        switch (mpOwner->mVisit) {
        case VISIT_FIRST:
            switch (mMessageCode) {
            case MSG_FIRST_GIVE:
            case MSG_FIRST_POCKETS_FULL:
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                    0) {
                    mOnMessageEnd = &talk_c::giveShovel;
                    mOnRequestEnd = &talk_c::pickUpShovel;
                    getController()->setNextMsgCode(MSG_FIRST_GIVEN);
                } else if (!mPocketsFullTold) {
                    getController()->setNextMsgCode(MSG_POCKETS_FULL);
                } else {
                    getController()->setNextMsgCode(MSG_POCKETS_FULL_AGAIN);
                }
                break;
            case MSG_FIRST_GIVEN:
            case MSG_POCKETS_FULL_END:
                mPocketsFullTold = 1;
                break;
            }
            break;
        }
        switch (mMessageCode) {
        case MSG_SCRAMBLE:
        case MSG_SCRAMBLE_2:
        case MSG_SCRAMBLE_3:
            getController()->setNextMsgCode(mMessageCode + 1);
            mOnMessageEnd = &talk_c::holdMsg;
            mOnRequestEnd = &talk_c::startScramble;
            break;
        case MSG_RANGERS_GO:
        case MSG_RANGERS_GO_2:
        case MSG_RANGERS_GO_3:
            getController()->setNextMsgCode(mMessageCode + 1);
            mOnMessageEnd = &talk_c::holdMsg;
            mOnRequestEnd = &talk_c::startBack;
            break;
        case MSG_LAUNCHED:
        case MSG_LAUNCHED_2:
        case MSG_LAUNCHED_3:
            mScramble = 1;
            break;
        }
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x18CC
void dAcNpcSpDemoResetsanBase_c::talk_c::onCustomTag(int no) {
    switch (no) {
    case CUSTOM_SLAP:
        mpOwner->mAudioObj.startSound(SE_resetsan_viblslap);
        break;
    case CUSTOM_THUNDER:
        mpOwner->mAudioObj.startSound(SE_resetsan_kaminari);
        dPlayerMgr::flinch();
        break;
    case CUSTOM_LIGHT_ON:
        if (daRmobjReset_c::s_instance) {
            daRmobjReset_c::s_instance->lightOn();
        }
        break;
    }
}

// .text 0x195C
void dAcNpcSpDemoResetsanBase_c::talk_c::onAnswer(int arg) {
}

// .text 0x1960
void dAcNpcSpDemoResetsanBase_c::talk_c::preExecute() {
    if (mProc) {
        (this->*mProc)();
    }
}

// .text 0x19A8
void dAcNpcSpDemoResetsanBase_c::talk_c::setTalked() {
    if (!mTalked) {
        mTalked = 1;
    } else if (!mTalkedAgain) {
        mTalkedAgain = 1;
    }
}

// .text 0x19D8
void dAcNpcSpDemoResetsanBase_c::talk_c::onTalkEnd() {
    u8 talked = mTalked;
    setTalked();
    if (!talked) {
        switch (mpOwner->mVisit) {
        case VISIT_SECOND:
            if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_RESET_CENTER_VISIT2)) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_RESET_CENTER_VISIT2);
            }
            break;
        case VISIT_THIRD:
            if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_RESET_CENTER_VISIT3)) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_RESET_CENTER_VISIT3);
            } else if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                           PRIVATE_FLAG0_RESET_CENTER_VISIT4)) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_RESET_CENTER_VISIT4);
            }
            break;
        case VISIT_DON:
            if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_RESET_CENTER_DON_VISIT)) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_RESET_CENTER_DON_VISIT);
            }
            break;
        case VISIT_DON_ONLY:
            break;
        }
    }
    if (mpOwner->mVisit >= VISIT_DON) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_DON_MET);
    }
}

// .text 0x1AE8
void dAcNpcSpDemoResetsanBase_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1B4C
void dAcNpcSpDemoResetsanBase_c::talk_c::giveShovel() {
    dItem::Item shovel(dItem::ITEM_IDX_SILVER_SHOVEL);
    requestItemAct(&shovel, 0, 0);
}

// .text 0x1B94
void dAcNpcSpDemoResetsanBase_c::talk_c::pickUpShovel() {
    dItem::Item shovel(dItem::ITEM_IDX_SILVER_SHOVEL);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(shovel, 0);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
        PRIVATE_FLAG0_RESET_CENTER_SHOVEL);
}

// .text 0x1BD8
void dAcNpcSpDemoResetsan_c::talkDemoReset_c::setPartnerNpc() {
    dItem::Item key((u16)SP_NPC_DON);
    dAcNpc_c *don = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    if (don) {
        setPartner(don);
    }
}

// .text 0x1C28
void dAcNpcSpDemoRacketsan_c::talkDemoRacket_c::setPartnerNpc() {
    dItem::Item key((u16)SP_NPC_RESETTI);
    dAcNpc_c *resetti = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    if (resetti) {
        setPartner(resetti);
    }
}

// .text 0x1C78
void dAcNpcSpDemoResetsanBase_c::talk_c::holdMsg() {
    setRequest1(0);
}

// .text 0x1C80: after "RESET SCRAMBLE!"
void dAcNpcSpDemoResetsanBase_c::talk_c::startScramble() {
    dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_FIXED_VIEW_25;
    mProc = &talk_c::procWaitCamera;
}

// .text 0x1CB0
void dAcNpcSpDemoResetsanBase_c::talk_c::procWaitCamera() {
    if (!dCamera_c::sInstance->isTransitionActive()) {
        dANpcSpDemoResetsan::getResetti()->startSwitchAnm();
        mProc = &talk_c::procSwitch;
        mFrame = 0;
    }
}

// .text 0x1D20
void dAcNpcSpDemoResetsanBase_c::talk_c::procSwitch() {
    mFrame++;
    if (mFrame == 40 || mFrame == 58 || mFrame == 100) {
        mpOwner->mAudioObj.startSound(SE_ResetCenter_SwOn);
    }
    if (dANpcSpDemoResetsan::getResetti()->mAction.mActionId ==
            dAcNpc_c::ACTION_ANM &&
        dANpcSpDemoResetsan::getResetti()->mAction.mCanChange) {
        dANpcSpDemoResetsan::getResetti()->startGearAnm();
        if (daRmobjReset_c::s_instance) {
            daRmobjReset_c::s_instance->changeState(
                daRmobjReset_c::STATE_LAUNCH);
        }
        mProc = &talk_c::procLaunch;
    }
}

// .text 0x1DF8 (+ a dead blr at 0x1E3C)
void dAcNpcSpDemoResetsanBase_c::talk_c::procLaunch() {
    if (dANpcSpDemoResetsan::getRoomState() == daRmobjReset_c::STATE_WAIT) {
        mProc = NULL;
        reqMsgClose();
    }
}

// .text 0x1E40: after "RANGERS, GO!"
void dAcNpcSpDemoResetsanBase_c::talk_c::startBack() {
    mTimer = dANpcSpDemoResetsan::l_param.mBackWait;
    dANpcSpDemoResetsan::getResetti()->startBackAnm();
    if (daRmobjReset_c::s_instance) {
        daRmobjReset_c::s_instance->changeState(daRmobjReset_c::STATE_BACK);
    }
    mProc = &talk_c::procBack;
}

// .text 0x1EBC
void dAcNpcSpDemoResetsanBase_c::talk_c::procBack() {
    if (dANpcSpDemoResetsan::getResetti()->mAction.mActionId ==
            dAcNpc_c::ACTION_ANM &&
        dANpcSpDemoResetsan::getResetti()->mAction.mCanChange) {
        dANpcSpDemoResetsan::getResetti()->mIsActive = 0;
        if (--mTimer <= 0) {
            mProc = NULL;
            reqMsgClose();
        }
    }
}
