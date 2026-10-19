#include <game/game/d_a_npc_sp_fuko.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_select.hpp>
#include <game/game/d_star_draw.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// Queue d_a_npc.hpp before the implicit vector destructor's section is opened.
// MWCC drops this unused body. See notes/TUs/d_a_npc_sp_fuko.txt.
static void orderNpcWeakInlines(dAcNpc_c *npc) {
    npc->dAcNpc_c::getHoldItem();
}
#endif

// Celeste at the observatory. Functions are in target .text order.

namespace dANpcSpFuko {
// .text 0xCC: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpFuko_c *npc) {
    return FALSE;
}

// .text 0xD4
BASE_PROFILE(A_NPC_SP_FUKO, dAcNpcSpFuko_c);

// Tuning (.bss 0x8, built by __sinit).
struct param_s {
    param_s() : _4(0), mPointDist(64.0f), mPointAngle(0x1000) {
        mSleepMorph = 90.0f;
    }
    f32 mSleepMorph; // the morph frames of her dozing animation
    int _4;
    f32 mPointDist; // isATriggeredFacing: the telescope picked by pointer
    mAng mPointAngle;
};
static param_s l_param;
} // namespace dANpcSpFuko

// .text 0x2A0
void dAcNpcSpFuko_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeState(STATE_LOOK);
        }
        break;
    case DEMO_CMD_TALK:
        mTalkFuko.mMsgKind = talk_c::MSG_KIND_TALK;
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mTalkFuko.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_SPEAK:
        mTalkFuko.mMsgKind = talk_c::MSG_KIND_SPEAK;
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mTalkFuko.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            changeState(STATE_IDLE);
        } else if (permit == PERMIT_END_MULTI) {
            changeState(STATE_REMOTE);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            changeState(STATE_IDLE);
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpFuko {
static const dAcNpcSpFuko_c::State_s l_states[] = {
    {&dAcNpcSpFuko_c::initIdle, &dAcNpcSpFuko_c::executeIdle},
    {&dAcNpcSpFuko_c::initLook, &dAcNpcSpFuko_c::executeLook},
    {&dAcNpcSpFuko_c::initTalk, &dAcNpcSpFuko_c::executeTalk},
    {&dAcNpcSpFuko_c::initRemote, &dAcNpcSpFuko_c::executeRemote},
    {&dAcNpcSpFuko_c::initRemoteTalk, &dAcNpcSpFuko_c::executeRemoteTalk}};
} // namespace dANpcSpFuko

// .text 0x410
void dAcNpcSpFuko_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpFuko::l_states[state].mInit)();
}

// .text 0x448
int dAcNpcSpFuko_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mTalkFuko);
    return 1;
}

// .text 0x494
int dAcNpcSpFuko_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mSleepTimer = 0;
    if (isLocalOwner()) {
        mDaub.mAsleep = isSleepTime() != FALSE;
        changeState(STATE_IDLE);
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    dSceneCheck::fn_80081858(dANpcSpFuko::entryDebugPage(this));
    return 1;
}

// .text 0x550
int dAcNpcSpFuko_c::doDelete() {
    dANpcSpFuko::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x594
int dAcNpcSpFuko_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    isLocalOwner();
    if (mState < STATE_COUNT) {
        (this->*dANpcSpFuko::l_states[mState].mExecute)();
    }
    if (!isInUse(NULL) && !mTalk.isBusy()) {
        if (mDaub.mAsleep) {
            mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
        } else {
            mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep,
                              l_lookYawStep, cNpcLookRange);
        }
    }
    return 1;
}

// .text 0x688
int dAcNpcSpFuko_c::canSpeak(dDemoActor_c *actor) {
    if (isInUse(NULL)) {
        return FALSE;
    }
    return !mTalk.isBusy();
}

// .text 0x6D8
void dAcNpcSpFuko_c::initIdle() {
    if (mDaub.mAsleep) {
        fallAsleep();
    } else {
        wakeUp();
    }
}

// .text 0x6EC
void dAcNpcSpFuko_c::wakeUp() {
    mDaub.mAsleep = 0;
    setDaubData(&mDaub, sizeof(mDaub));
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mStep = IDLE_STEP_AWAKE;
    mSleepTimer = SLEEP_WAIT_FRAMES;
}

// .text 0x75C
void dAcNpcSpFuko_c::stepIdleAwake() {
    if (isSleepTime()) {
        if (mLookAt.searchPlayer(NET_MEMBER_NONE, this) != NET_MEMBER_NONE) {
            mSleepTimer = SLEEP_WAIT_FRAMES;
        } else if (mSleepTimer != 0) {
            mSleepTimer--;
        } else {
            fallAsleep();
        }
    } else {
        mSleepTimer = SLEEP_WAIT_FRAMES;
    }
}

// .text 0x7E0
void dAcNpcSpFuko_c::fallAsleep() {
    mDaub.mAsleep = 1;
    setDaubData(&mDaub, sizeof(mDaub));
    mAnmSet.setAnm0(HMN_ANM_OWL_SLP1);
    mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_OWL_SLP1, 0.0f,
                       dANpcSpFuko::l_param.mSleepMorph, m3d::PLAYMODE_INHERIT,
                       0);
    mStep = IDLE_STEP_ASLEEP;
}

// .text 0x85C
void dAcNpcSpFuko_c::stepIdleAsleep() {
    if (!isSleepTime()) {
        wakeUp();
    }
    mSleepEffect.create("afm_owl_sleep", &mNodeMtx12, 0);
    if (isTelescopePressed()) {
        dDemo::requestSpeak(this, NULL);
    }
}

namespace dANpcSpFuko {
static const dAcNpcSpFuko_c::Proc l_idleSteps[] = {
    &dAcNpcSpFuko_c::stepIdleAwake, &dAcNpcSpFuko_c::stepIdleAsleep};
} // namespace dANpcSpFuko

// .text 0x8CC
void dAcNpcSpFuko_c::executeIdle() {
    if (mStep < IDLE_STEP_DONE) {
        (this->*dANpcSpFuko::l_idleSteps[mStep])();
    }
    if (isTelescopePressed()) {
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x934
void dAcNpcSpFuko_c::initLook() {
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    if (mDaub.mAsleep) {
        initLookAsleep();
    } else {
        initLookTurn();
    }
}

// .text 0x988
void dAcNpcSpFuko_c::initLookAsleep() {
    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_NML_WAIT1, 0.0f, cNpcMorphFrames,
                       m3d::PLAYMODE_INHERIT, 0);
    mStep = LOOK_STEP_WAKE;
}

// .text 0x9E0
void dAcNpcSpFuko_c::stepLookWake() {
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep = LOOK_STEP_DONE;
    }
}

// .text 0xA3C
void dAcNpcSpFuko_c::initLookTurn() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = LOOK_STEP_TURN;
}

// .text 0xAA0
void dAcNpcSpFuko_c::stepLookTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep = LOOK_STEP_DONE;
    }
}

namespace dANpcSpFuko {
static const dAcNpcSpFuko_c::Proc l_lookSteps[] = {
    &dAcNpcSpFuko_c::stepLookWake, &dAcNpcSpFuko_c::stepLookTurn};
} // namespace dANpcSpFuko

// .text 0xAFC
void dAcNpcSpFuko_c::executeLook() {
    if (mStep < LOOK_STEP_DONE) {
        (this->*dANpcSpFuko::l_lookSteps[mStep])();
    }
}

// .text 0xB3C
void dAcNpcSpFuko_c::initTalk() {
    if (mDaub.mAsleep) {
        mTalk.requestTalk(true, FALSE);
    } else {
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
    mStep = TALK_STEP_TURN;
    mDaub.mAsleep = 0;
    setDaubData(&mDaub, sizeof(mDaub));
}

// .text 0xBE4
void dAcNpcSpFuko_c::stepTalkTurn() {
    if (!mTalk.isBusy()) {
        mAction.requestTurn(PRIORITY_DEMO, mAng(0), l_turnSpeed, 0,
                            cNpcMorphFrames);
        mStep++;
    }
}

// .text 0xC54
void dAcNpcSpFuko_c::stepTalkTurnEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpFuko {
static const dAcNpcSpFuko_c::Proc l_talkSteps[] = {
    &dAcNpcSpFuko_c::stepTalkTurn, &dAcNpcSpFuko_c::stepTalkTurnEnd};
} // namespace dANpcSpFuko

// .text 0xCA4
void dAcNpcSpFuko_c::executeTalk() {
    if (mStep < TALK_STEP_DONE) {
        (this->*dANpcSpFuko::l_talkSteps[mStep])();
    }
}

// .text 0xCE4
void dAcNpcSpFuko_c::initRemote() {
    if (mDaub.mAsleep) {
        initRemoteAsleep();
    } else {
        initRemoteAwake();
    }
}

// .text 0xCF8
void dAcNpcSpFuko_c::initRemoteAwake() {
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mStep = REMOTE_STEP_AWAKE;
}

// .text 0xD48
void dAcNpcSpFuko_c::stepRemoteAwake() {
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mAcceptMoveReq = 1;
    }
    if (mDaub.mAsleep) {
        initRemoteAsleep();
    }
}

// .text 0xD7C
void dAcNpcSpFuko_c::initRemoteAsleep() {
    mAnmSet.setAnm0(HMN_ANM_OWL_SLP1);
    mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_OWL_SLP1, 0.0f, cNpcMorphFrames,
                       m3d::PLAYMODE_INHERIT, 1);
    mStep = REMOTE_STEP_ASLEEP;
}

// .text 0xDE0
void dAcNpcSpFuko_c::stepRemoteAsleep() {
    if (!mDaub.mAsleep) {
        initRemoteAwake();
    }
    mSleepEffect.create("afm_owl_sleep", &mNodeMtx12, 0);
}

namespace dANpcSpFuko {
static const dAcNpcSpFuko_c::Proc l_remoteSteps[] = {
    &dAcNpcSpFuko_c::stepRemoteAwake, &dAcNpcSpFuko_c::stepRemoteAsleep};
} // namespace dANpcSpFuko

// .text 0xE30
void dAcNpcSpFuko_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
            fn_80013474();
            mTalkFuko.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
            return;
        }
        if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
            return;
        }
    }
    if (isTelescopePressed()) {
        dDemo::requestSpeak(this, NULL);
    } else if (mStep < REMOTE_STEP_DONE) {
        (this->*dANpcSpFuko::l_remoteSteps[mStep])();
    }
}

// .text 0xF1C
void dAcNpcSpFuko_c::initRemoteTalk() {
}

// .text 0xF20
void dAcNpcSpFuko_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0xF74
BOOL dAcNpcSpFuko_c::isSleepTime() {
    dTime_c now = *dTime_c::getCurrent();
    BOOL sleep = FALSE;
    if (now.hour >= SLEEP_START_HOUR && now.hour < SLEEP_END_HOUR) {
        sleep = TRUE;
    }
    return sleep;
}

// .text 0x1008
int dAcNpcSpFuko_c::countNewSigns() {
    int count = 0;
    for (int i = 0; i < dSvStarSign::SIGN_NUM; i++) {
        if (dSaveData_c::getRaw()->getStarSignNew(
                dPlayerMgr::getCurrentPlayerSlot()) &
            (1 << i)) {
            count++;
        }
    }
    return count;
}

// .text 0x108C
BOOL dAcNpcSpFuko_c::isTelescopePressed() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    if (dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    if (checkInteractionRange(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT)) &&
        checkInteractionAngle(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT),
            (s16)-dDemoActor_c::mInteractionAngle,
            dDemoActor_c::mInteractionAngle)) {
        return FALSE;
    }
    static mVec3_c l_pos(264.0f, 0.0f, 352.0f);
    mVec3_c pos(l_pos);
    pos.z -= 32.0f;
    mTelescopeBox.set(pos, 48.0f, 32.0f, 96.0f, 0, dObjc::HIT_RMOBJ, NULL, 0);
    mTelescopeBox.regist();
    if (mPad::g_currentCore->getDpdValidFlag() && dPad::isATriggered()) {
        const dObjc::dpdHit_c *hit = dObjc::getDpdHit();
        if (hit == NULL) {
            return FALSE;
        }
        if (hit->mType == dObjc::TYPE_PLAYER) {
        } else if (hit->mType == dObjc::HIT_RMOBJ) {
            f32 dist = dANpcSpFuko::l_param.mPointDist;
            if (!dPlayerMgr::isATriggeredFacing(
                    &hit->mPos, dANpcSpFuko::l_param.mPointAngle.mAngle,
                    dist)) {
                return FALSE;
            }
        } else {
            return FALSE;
        }
    }
    if (dPad::isConfirmTriggered() && dField::getInfo(FD_ID_CURRENT) != NULL) {
        mVec3_c playerPos;
        ((dActor_c *)dScene::getScenePlayer())
            ->getOffsetPos(&playerPos, 16.0f, 0);
        if (playerPos.z <= l_pos.z && playerPos.x >= l_pos.x - 8.0f &&
            playerPos.x <= l_pos.x + 8.0f) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x12EC
void dAcNpcSpFuko_c::turnToPlayer() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
}

// .text 0x1348
dAcNpcSpFuko_c::talk_c::talk_c(dAcNpcSpFuko_c *npc) : mpOwner(npc) {
    mMsgKind = MSG_KIND_TALK;
    mAsleep = 0;
    init();
}

// .text 0x13B4
dAcNpcSpFuko_c::talk_c::~talk_c() {
}

// .text 0x140C
void dAcNpcSpFuko_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mNextOnRequestEnd = NULL;
    mProc = NULL;
    mMode = MODE_NONE;
    for (int i = 0; i < SEL_LIST_NUM + 1; i++) {
        mSelList[i] = SEL_LIST_NONE;
    }
    mSelSign = SEL_LIST_NONE;
    mListEnd = 0;
}

// .text 0x14E4
void dAcNpcSpFuko_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    mAsleep = mpOwner->mDaub.mAsleep;
    if (mAsleep) {
        mMsgNo = MSG_HOOT;
    } else {
        mMsgNo = getGreetingMsg();
    }
}

// .text 0x1558
void dAcNpcSpFuko_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_hu-ko";
    info->mCode = mMsgNo;
}

// .text 0x1570
void dAcNpcSpFuko_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_NEW_SIGN:
        for (int i = 0; i < dSvStarSign::SIGN_NUM; i++) {
            if (dSaveData_c::getRaw()->getStarSignNew(
                    dPlayerMgr::getCurrentPlayerSlot()) &
                (1 << i)) {
                dString::Word_c name(
                    dSaveData_c::getRaw()->getStarSigns()->get(i)->getName());
                mpController->setWord(3, &name);
                break;
            }
        }
        break;
    case MSG_RENAMED:
    case MSG_REGISTERED: {
        dString::Word_c name(dSaveData_c::getRaw()
                                 ->getStarSigns()
                                 ->get(fn_80029CDC())
                                 ->getName());
        mpController->setWord(0, &name);
        break;
    }
    case MSG_ASK_LOG: {
        dTime_c date = *dTime_c::getCurrent();
        dStarDraw_c::getSignDate(
            dSaveData_c::getRaw()->getStarSigns()->get(fn_80029CDC()), &date);
        setMonthName(date.month, 4);
        setDayName((u8)date.mday, 5);
        break;
    }
    case MSG_VIEW_TIME:
    case MSG_VIEW_TOMORROW: {
        dString::Word_c name(
            dSaveData_c::getRaw()->getStarSigns()->get(mSelSign)->getName());
        mpController->setWord(2, &name);
        dTime_c time = *dTime_c::getCurrent();
        dStarDraw_c::getSignTime(
            dSaveData_c::getRaw()->getStarSigns()->get(mSelSign), &time);
        setTime(time.hour, 6, 0);
        setMinute(time.min, 1);
        break;
    }
    case MSG_BEST_EVENING: {
        dTime_c date = *dTime_c::getCurrent();
        dStarDraw_c::getSignDate(
            dSaveData_c::getRaw()->getStarSigns()->get(mSelSign), &date);
        setMonthName(date.month, 4);
        setDayName((u8)date.mday, 5);
        break;
    }
    case MSG_ASK_ERASE: {
        dString::Word_c name(
            dSaveData_c::getRaw()->getStarSigns()->get(mSelSign)->getName());
        mpController->setWord(1, &name);
        break;
    }
    case MSG_VIEW_NOW: {
        dString::Word_c name(
            dSaveData_c::getRaw()->getStarSigns()->get(mSelSign)->getName());
        mpController->setWord(2, &name);
        break;
    }
    case MSG_LIKED: {
        dString::Word_c name(
            dSaveData_c::getRaw()->getStarSigns()->get(mSelSign)->getName());
        mpController->setWord(2, &name);
        break;
    }
    }
}

// .text 0x19C8
void dAcNpcSpFuko_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_WAKE:
    case MSG_WELCOME:
    case MSG_WAKE_AGAIN:
    case MSG_WELCOME_AGAIN:
        if (!dAcNpc_c::isPlayerFromThisTown()) {
            mpController->setNextMsgCode(MSG_VISITOR);
        } else if (mpOwner->countNewSigns() > 0) {
            if (mpOwner->countNewSigns() == 1) {
                mpController->setNextMsgCode(MSG_NEW_SIGN);
            } else {
                mpController->setNextMsgCode(MSG_NEW_SIGNS);
            }
        } else if (dAcNpc_c::isMultiPlay()) {
            mpController->setNextMsgCode(MSG_TELESCOPE);
        } else {
            mpController->setNextMsgCode(MSG_ASSIST);
        }
        break;
    case MSG_NEW_SIGN:
    case MSG_NEW_SIGNS:
        for (int i = 0; i < dSvStarSign::SIGN_NUM; i++) {
            dSaveData_c::getTown()->getStarSignNew(
                dPlayerMgr::getCurrentPlayerSlot()) &= ~(1 << i);
        }
        break;
    case MSG_HOOT:
        mOnMessageEnd = &talk_c::setRequest;
        mOnRequestEnd = &talk_c::startTurn;
        {
            dMsg::Comp_c *ctrl = getController();
            int msg = getGreetingMsg();
            ctrl->setNextMsgCode(msg);
        }
        break;
    case MSG_NEVER_DO: {
        dMsg::Comp_c *ctrl = getController();
        int msg = getGreetingMsg();
        ctrl->setNextMsgCode(msg);
    } break;
    case MSG_ENJOY_MAKE:
        mOnMessageEnd = &talk_c::setRequest;
        mOnRequestEnd = &talk_c::startMake;
        break;
    case MSG_ENJOY_VIEW:
        mOnMessageEnd = &talk_c::setRequest;
        mOnRequestEnd = &talk_c::startView;
        break;
    case MSG_WHICH_CHANGE:
    case MSG_WHICH_LEARN:
    case MSG_WHICH_ERASE:
    case MSG_PAGE_2:
    case MSG_PAGE_3:
    case MSG_PAGE_4:
    case MSG_WHICH_LIKED: {
        BOOL more = FALSE;
        int skip = 0;
        switch (mMessageCode) {
        case MSG_PAGE_2:
            skip = SEL_LIST_NUM;
            break;
        case MSG_PAGE_3:
            skip = SEL_LIST_NUM * 2;
            break;
        case MSG_PAGE_4:
            skip = SEL_LIST_NUM * 3;
            break;
        }
        for (int i = 0; i < SEL_LIST_NUM + 1; i++) {
            mSelList[i] = SEL_LIST_NONE;
        }
        mSelSign = SEL_LIST_NONE;
        int num = 0;
        for (int i = 0; i < dSvStarSign::SIGN_NUM; i++) {
            if (dSaveData_c::getRaw()->getStarSigns()->isValid(i)) {
                if (skip > 0) {
                    skip--;
                } else if (num >= SEL_LIST_NUM) {
                    more = TRUE;
                } else {
                    dString::Word_c name(dSaveData_c::getRaw()
                                             ->getStarSigns()
                                             ->get(i)
                                             ->getName());
                    mpController->mSelect.setWord(num, &name, 0, NULL, 0, 1);
                    mSelList[num] = i;
                    num++;
                }
            }
        }
        mListEnd = 1;
        for (int i = mSelList[num - 1] + 1; i < dSvStarSign::SIGN_NUM; i++) {
            if (dSaveData_c::getRaw()->getStarSigns()->isValid(i)) {
                mListEnd = 0;
                break;
            }
        }
        if (more) {
            mSelList[num] = SEL_LIST_NEXT;
            mpController->mSelect.setSysMessage(num, SEL_MSG_NEXT, 0, NULL, 0,
                                                0);
        } else {
            mpController->mSelect.setSysMessage(num, SEL_MSG_QUIT, 0, NULL, 0,
                                                0);
        }
        mpController->mSelect.mCancel = num;
        mpController->mSelect.mNum = num + 1;
        mpController->openSelect();
        break;
    }
    case MSG_ASK_NAME:
        mOnMessageEnd = &talk_c::openNameMenu;
        mOnRequestEnd = &talk_c::checkName;
        break;
    case MSG_RENAME:
    case MSG_NAME_TAKEN:
        mOnMessageEnd = &talk_c::openRenameMenu;
        mOnRequestEnd = &talk_c::checkRename;
        break;
    case MSG_NEW_ERASED:
        dSaveData_c::getTown()->getStarSignList()->remove(fn_80029CDC());
        break;
    case MSG_ERASED:
        dSaveData_c::getTown()->mStarSigns.remove(mSelSign);
        break;
    }
    switch (kind) {
    case END_REQ_USE_AGAIN:
        getController()->setNextMsgCode(getGreetingMsg() +
                                        (MSG_USE_AGAIN - MSG_USE));
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x1ED4
void dAcNpcSpFuko_c::talk_c::onAnswer(int arg) {
    int answer = mpController->getAnswer();
    switch (mMessageCode) {
    case MSG_USE:
    case MSG_USE_AGAIN:
        switch (answer) {
        case 0:
            mMode = MODE_MAKE;
            break;
        case 1:
            mMode = MODE_VIEW;
            break;
        case 2:
            mMode = MODE_NONE;
            break;
        }
        break;
    case MSG_USE_ERASE:
    case MSG_USE_AGAIN_ERASE:
        switch (answer) {
        case 0:
            mMode = MODE_MAKE;
            break;
        case 1:
            mMode = MODE_VIEW;
            break;
        case 2:
            mMode = MODE_ERASE;
            break;
        case 3:
            mMode = MODE_NONE;
            break;
        }
        break;
    case MSG_USE_VISITOR:
    case MSG_USE_VISITOR_SIGNS:
    case MSG_USE_AGAIN_VISITOR:
    case MSG_USE_AGAIN_VISITOR_SIGNS:
    // The answer switch is missing here: the answers' labels are cases of
    // the message switch (code 0 / 1 never come).
    case 0:
        mMode = MODE_VIEW;
        break;
    case 1:
        mMode = MODE_NONE;
        break;
    case MSG_ASK_MAKE:
        switch (answer) {
        case 0:
            if (dSaveData_c::getTown()->mStarSigns.findFree() >= 0) {
                mpController->setNextMsgCode(MSG_ENJOY_MAKE);
            } else {
                mpController->setNextMsgCode(MSG_TOO_MANY);
            }
            break;
        case 1:
            mMode = MODE_CHANGE;
            break;
        }
        break;
    case MSG_ASK_RENAME:
        switch (answer) {
        case 0:
            mpController->setNextMsgCode(MSG_RENAME);
            break;
        case 1:
            mpController->setNextMsgCode(MSG_CHANGE_CANCELLED);
            break;
        }
        break;
    case MSG_WHICH_CHANGE:
    case MSG_WHICH_LEARN:
    case MSG_WHICH_ERASE:
    case MSG_PAGE_2:
    case MSG_PAGE_3:
    case MSG_PAGE_4:
    case MSG_WHICH_LIKED: {
        int sign = mSelList[answer];
        mSelSign = sign;
        switch (mMode) {
        case MODE_CHANGE:
            if (sign >= 0) {
                mOnAnswer = &talk_c::setRequest;
                mOnRequestEnd = &talk_c::startChange;
            } else if (sign == SEL_LIST_NONE) {
                mpController->setNextMsgCode(MSG_CHANGE_CANCELLED);
            } else if (sign == SEL_LIST_NEXT) {
                if (mListEnd) {
                    mpController->setNextMsgCode(MSG_FROM_BEGINNING);
                } else {
                    switch (mMessageCode) {
                    case MSG_WHICH_CHANGE:
                    case MSG_WHICH_LEARN:
                    case MSG_WHICH_ERASE:
                    case MSG_WHICH_LIKED:
                        mpController->setNextMsgCode(MSG_PAGE_2);
                        break;
                    case MSG_PAGE_2:
                        mpController->setNextMsgCode(MSG_PAGE_3);
                        break;
                    case MSG_PAGE_3:
                        mpController->setNextMsgCode(MSG_PAGE_4);
                        break;
                    case MSG_PAGE_4:
                        break;
                    }
                }
            }
            break;
        case MODE_VIEW:
            if (sign >= 0) {
                dTime_c time = *dTime_c::getCurrent();
                dStarDraw_c::getSignTime(
                    dSaveData_c::getRaw()->getStarSigns()->get(mSelSign),
                    &time);
                const dSvStarSign::List_c *signs =
                    dSaveData_c::getRaw()->getStarSigns();
                if (dStarDraw_c::isSignVisible(signs->get(mSelSign),
                                               dTime_c::getCurrent())) {
                    mpController->setNextMsgCode(MSG_VIEW_NOW);
                } else if (dTime_c::isSameOrAfter(*dTime_c::getCurrent(),
                                                  time)) {
                    mpController->setNextMsgCode(MSG_VIEW_TOMORROW);
                } else {
                    mpController->setNextMsgCode(MSG_VIEW_TIME);
                }
            } else if (sign == SEL_LIST_NONE) {
                mpController->setNextMsgCode(MSG_VIEW_CANCELLED);
            } else if (sign == SEL_LIST_NEXT) {
                switch (mMessageCode) {
                case MSG_WHICH_CHANGE:
                case MSG_WHICH_LEARN:
                case MSG_WHICH_ERASE:
                case MSG_WHICH_LIKED:
                    mpController->setNextMsgCode(MSG_PAGE_2);
                    break;
                case MSG_PAGE_2:
                    mpController->setNextMsgCode(MSG_PAGE_3);
                    break;
                case MSG_PAGE_3:
                    mpController->setNextMsgCode(MSG_PAGE_4);
                    break;
                case MSG_PAGE_4:
                    break;
                }
            }
            break;
        case MODE_FAVORITE:
            if (sign >= 0) {
                mpController->setNextMsgCode(MSG_LIKED);
                const dSvStarSign::List_c *signs =
                    dSaveData_c::getRaw()->getStarSigns();
                dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
                player->mStarSign.copy(signs->get(mSelSign));
            } else if (sign == SEL_LIST_NONE) {
                mpController->setNextMsgCode(MSG_LIKED_CANCELLED);
            } else if (sign == SEL_LIST_NEXT) {
                switch (mMessageCode) {
                case MSG_WHICH_CHANGE:
                case MSG_WHICH_LEARN:
                case MSG_WHICH_ERASE:
                case MSG_WHICH_LIKED:
                    mpController->setNextMsgCode(MSG_PAGE_2);
                    break;
                case MSG_PAGE_2:
                    mpController->setNextMsgCode(MSG_PAGE_3);
                    break;
                case MSG_PAGE_3:
                    mpController->setNextMsgCode(MSG_PAGE_4);
                    break;
                case MSG_PAGE_4:
                    break;
                }
            }
            break;
        case MODE_ERASE:
            if (sign >= 0) {
                mpController->setNextMsgCode(MSG_ASK_ERASE);
            } else if (sign == SEL_LIST_NONE) {
                mpController->setNextMsgCode(MSG_ERASE_CANCELLED);
            } else if (sign == SEL_LIST_NEXT) {
                switch (mMessageCode) {
                case MSG_WHICH_CHANGE:
                case MSG_WHICH_LEARN:
                case MSG_WHICH_ERASE:
                case MSG_WHICH_LIKED:
                    mpController->setNextMsgCode(MSG_PAGE_2);
                    break;
                case MSG_PAGE_2:
                    mpController->setNextMsgCode(MSG_PAGE_3);
                    break;
                case MSG_PAGE_3:
                    mpController->setNextMsgCode(MSG_PAGE_4);
                    break;
                case MSG_PAGE_4:
                    break;
                }
            }
            break;
        case MODE_NONE:
            break;
        }
        break;
    }
    case MSG_ASK_LOG:
        switch (answer) {
        case 0:
            mpController->setNextMsgCode(MSG_ASK_NAME);
            break;
        case 1:
            mpController->setNextMsgCode(MSG_ASK_ERASE_NEW);
            break;
        }
        break;
    case MSG_ASK_ERASE_NEW:
        switch (answer) {
        case 0:
            mpController->setNextMsgCode(MSG_NEW_ERASED);
            break;
        case 1:
            mpController->setNextMsgCode(MSG_ASK_NAME);
            break;
        }
        break;
    case MSG_ASK_ERASE:
        switch (answer) {
        case 0:
            mpController->setNextMsgCode(MSG_ERASED);
            break;
        case 1:
            mpController->setNextMsgCode(MSG_ERASE_CANCELLED);
            break;
        }
        break;
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x2660
void dAcNpcSpFuko_c::talk_c::preExecute() {
    if (mProc) {
        (this->*mProc)();
    }
}

// The player's own constellations are not new to them.
void dAcNpcSpFuko_c::talk_c::onTalkEnd() {
    dNpcInfo::fuko_c::get()->setTalked();

    for (int i = 0; i < dSvStarSign::SIGN_NUM; i++) {
        const dPersonalID_c *pid =
            dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getPersonalID();
        if (dSaveData_c::getTown()->getStarSign(i)->mOwner == *pid) {
            dSaveData_c::getTown()->getStarSignNew(
                dPlayerMgr::getCurrentPlayerSlot()) &= ~(1 << i);
        }
    }

    mMsgKind = MSG_KIND_TALK;
}

// .text 0x27A0
int dAcNpcSpFuko_c::talk_c::getGreetingMsg() const {
    switch (mMsgKind) {
    case MSG_KIND_TALK:
        if (dAcNpc_c::isPlayerFromThisTown()) {
            dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
            if (homes->getHome(homes->findCurrentPlayer()) == NULL) {
                if (mAsleep) {
                    return -(dNpcInfo::fuko_c::get()->isTalked() == 0) +
                           MSG_NEWCOMER_DOZING_AGAIN;
                }
                return -(dNpcInfo::fuko_c::get()->isTalked() == 0) +
                       MSG_NEWCOMER_AGAIN;
            }
        }
        if (!dNpcInfo::fuko_c::get()->isTalked()) {
            return mAsleep ? (int)MSG_WAKE : (int)MSG_WELCOME;
        }
        return mAsleep ? (int)MSG_WAKE_AGAIN : (int)MSG_WELCOME_AGAIN;
    case MSG_KIND_SPEAK: {
        if (dAcNpc_c::isPlayerFromThisTown()) {
            dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
            if (homes->getHome(homes->findCurrentPlayer()) == NULL) {
                if (mAsleep) {
                    return -(dNpcInfo::fuko_c::get()->isTalked() == 0) +
                           MSG_NEWCOMER_DOZING_AGAIN;
                }
                return -(dNpcInfo::fuko_c::get()->isTalked() == 0) +
                       MSG_NEWCOMER_AGAIN;
            }
        }
        BOOL any = FALSE;
        for (int i = 0; i < dSvStarSign::SIGN_NUM; i++) {
            if (dSaveData_c::getRaw()->getStarSigns()->isValid(i)) {
                any = TRUE;
                break;
            }
        }
        if (mMessageCode == MSG_HOOT) {
            return MSG_NEVER_DO;
        }
        if (dAcNpc_c::isPlayerFromThisTown() && !dAcNpc_c::isMultiPlay()) {
            return (any != 0) + MSG_USE;
        }
        if (any) {
            return MSG_USE_VISITOR_SIGNS;
        }
        return MSG_USE_VISITOR;
    }
    }
    return 0;
}

// .text 0x29B8
void dAcNpcSpFuko_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
        if (mNextOnRequestEnd) {
            mOnRequestEnd = mNextOnRequestEnd;
        }
        mNextOnRequestEnd = NULL;
    }
}

// .text 0x2A60
void dAcNpcSpFuko_c::talk_c::setRequest() {
    setRequest1(0);
}

// .text 0x2A68
void dAcNpcSpFuko_c::talk_c::playerTurn() {
    requestPlayerTurn();
}

// .text 0x2A6C
void dAcNpcSpFuko_c::talk_c::openMakeMenu() {
    reqMenu28(0);
}

// .text 0x2A74
void dAcNpcSpFuko_c::talk_c::openChangeMenu() {
    reqMenu29(mSelSign, 0);
}

// .text 0x2A80
void dAcNpcSpFuko_c::talk_c::openViewMenu() {
    reqMenu2A(0);
}

// .text 0x2A88
void dAcNpcSpFuko_c::talk_c::openNameMenu() {
    reqMenu2B(0);
}

// .text 0x2A90
void dAcNpcSpFuko_c::talk_c::openRenameMenu() {
    reqMenu2C(0);
}

// .text 0x2A98
void dAcNpcSpFuko_c::talk_c::startMake() {
    playerTurn();
    mNextOnRequestEnd = &talk_c::startMakeMenu;
}

// .text 0x2AE0
void dAcNpcSpFuko_c::talk_c::startChange() {
    playerTurn();
    mNextOnRequestEnd = &talk_c::startChangeMenu;
}

// .text 0x2B28
void dAcNpcSpFuko_c::talk_c::startView() {
    playerTurn();
    mNextOnRequestEnd = &talk_c::startViewMenu;
}

// .text 0x2B70
void dAcNpcSpFuko_c::talk_c::startMakeMenu() {
    openMakeMenu();
    mNextOnRequestEnd = &talk_c::endMake;
}

// .text 0x2BB8
void dAcNpcSpFuko_c::talk_c::startChangeMenu() {
    openChangeMenu();
    mNextOnRequestEnd = &talk_c::endChange;
}

// .text 0x2C00
void dAcNpcSpFuko_c::talk_c::startViewMenu() {
    openViewMenu();
    mNextOnRequestEnd = &talk_c::endView;
}

// .text 0x2C48
void dAcNpcSpFuko_c::talk_c::endMake() {
    dPlayerMgr::beginDemoWait();
    if (isMenuInvalid()) {
        mpController->setNextMsgCode(MSG_MAKE_CANCELLED);
    } else {
        mpController->setNextMsgCode(MSG_ASK_LOG);
    }
}

// .text 0x2C9C
void dAcNpcSpFuko_c::talk_c::endChange() {
    dPlayerMgr::beginDemoWait();
    if (isMenuInvalid()) {
        mpController->setNextMsgCode(MSG_CHANGE_CANCELLED);
    } else {
        mpController->setNextMsgCode(MSG_ASK_RENAME);
    }
}

// .text 0x2CF0
void dAcNpcSpFuko_c::talk_c::endView() {
    dPlayerMgr::beginDemoWait();
    BOOL any = FALSE;
    for (int i = 0; i < dSvStarSign::SIGN_NUM; i++) {
        if (dSaveData_c::getRaw()->getStarSigns()->isValid(i)) {
            any = TRUE;
            break;
        }
    }
    if (any && !dAcNpc_c::isPlayerFromThisTown()) {
        mMode = MODE_FAVORITE;
        mpController->setNextMsgCode(MSG_ASK_LIKED);
    } else {
        mpController->setNextMsgCode(MSG_ALL_DONE);
    }
}

// .text 0x2D9C
void dAcNpcSpFuko_c::talk_c::checkName() {
    if (isNameTaken(fn_80029CDC())) {
        mpController->setNextMsgCode(MSG_NAME_TAKEN);
    } else {
        mpController->setNextMsgCode(MSG_REGISTERED);
    }
}

// .text 0x2DF8
void dAcNpcSpFuko_c::talk_c::checkRename() {
    if (isNameTaken(fn_80029CDC())) {
        mpController->setNextMsgCode(MSG_NAME_TAKEN);
    } else if (mMode == MODE_CHANGE) {
        mpController->setNextMsgCode(MSG_RENAMED);
    } else {
        mpController->setNextMsgCode(MSG_REGISTERED);
    }
}

// .text 0x2E70
BOOL dAcNpcSpFuko_c::talk_c::isNameTaken(int idx) {
    dString::Word_c name;
    name.set(dSaveData_c::getRaw()->getStarSigns()->get(idx)->getName(), 0);
    for (int i = 0; i < dSvStarSign::SIGN_NUM; i++) {
        if (i != idx && dSaveData_c::getRaw()->getStarSigns()->isValid(i)) {
            dString::Word_c other;
            other.set(dSaveData_c::getRaw()->getStarSigns()->get(i)->getName(),
                      0);
            if (other.isSame(&name)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// .text 0x2F84
void dAcNpcSpFuko_c::talk_c::startTurn() {
    mpOwner->turnToPlayer();
    mProc = &talk_c::execTurn;
}

// .text 0x2FD0 (+ a dead blr at 0x300C)
void dAcNpcSpFuko_c::talk_c::execTurn() {
    dAcNpcSpFuko_c *owner = mpOwner;
    if (owner->mAction.mActionId == ACTION_TURN && owner->mAction.mCanChange) {
        mProc = NULL;
        reqMsgClose();
    }
}

// .text 0x3010
int dAcNpcSpFuko_c::getEarType() {
    return NPC_EAR_TYPE_CELESTE;
}
