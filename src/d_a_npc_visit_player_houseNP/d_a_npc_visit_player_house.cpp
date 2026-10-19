// A villager visiting the player's house (appointment quest q08). Functions are
// in target .text order.
#include <game/game/d_a_npc_visit_player_house.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/snd/snd_id.hpp>
#include <climits>

namespace dANpcVisitPlayerHouse {
// .text 0x5C
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text 0x68
BASE_PROFILE(A_NPC_VISIT_PLAYER_HOUSE, dAcNpcVisitPlayerHouse_c);

static inline float lengthXZ(const mVec3_c &v) {
    return EGG::Mathf::sqrt(v.x * v.x + v.z * v.z);
}
static inline float distanceXZ(const mVec3_c &a, const mVec3_c &b) {
    return lengthXZ(a - b);
}
} // namespace dANpcVisitPlayerHouse

// .text 0x1E4
bool dAcNpcVisitPlayerHouse_c::canInteract(dDemoActor_c *actor) {
    if (!dAcNpcNml_c::canInteract(actor)) {
        return false;
    }
    if (actor == NULL) {
        return false;
    }
    if (!mIsActive) {
        return false;
    }
    if (mTalk.isBusy()) {
        return false;
    }
    return true;
}

// .text 0x260
void dAcNpcVisitPlayerHouse_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        if (mState.mResumeState < dANpcVisitPlayerHouse::State_c::STATE_COUNT) {
            mState.change(this, mState.mResumeState);
        } else {
            mState.change(this, dANpcVisitPlayerHouse::State_c::STATE_LOOK);
        }
        mState.mResumeState = dANpcVisitPlayerHouse::State_c::STATE_NONE;
        break;
    case DEMO_CMD_SPEAK: {
        u32 state = mState.mTalkState;
        mNmlTalk.start(
            this,
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT),
            mState.mpTalk);
        if (state < dANpcVisitPlayerHouse::State_c::STATE_COUNT) {
            mState.change(this, state);
        } else {
            mState.change(this, dANpcVisitPlayerHouse::State_c::STATE_TALK);
        }
        mState.mTalkState = dANpcVisitPlayerHouse::State_c::STATE_NONE;
        break;
    }
    case DEMO_CMD_TALK: {
        const talk_c::procSet_s *set =
            talk_c::getProcSet(talk_c::TALK_PROC_VISIT_FIRST);
        mNmlTalk.start(
            this,
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT),
            set);
        mState.change(this, dANpcVisitPlayerHouse::State_c::STATE_TALK);
        break;
    }
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        mState.change(this, dANpcVisitPlayerHouse::State_c::STATE_IDLE);
        break;
    }
    dAcNpc_c::onDemoCmd(cmd, member);
}

// .text 0x3B4
int dAcNpcVisitPlayerHouse_c::preCreate() {
    return dAcNpcNml_c::preCreate() != FALSE;
}

// .text 0x3E0
int dAcNpcVisitPlayerHouse_c::create() {
    if (!dAcNpcNml_c::create()) {
        return 0;
    }
    dQuestPlayerItem_c *appointment =
        &dSaveData_c::getTown()->mAnimals.mTown.mAppointment;
    BOOL arrived = appointment->isFlag(QUEST_VISIT_FLAG_ARRIVED);
    BOOL talked = appointment->isFlag(QUEST_VISIT_FLAG_TALKED);
    dTime_c *now = dTime_c::getCurrent();
    dTime_c meet = appointment->getMeetTime();
    dTime_c limit = meet;
    limit.add(0, dANpcVisitPlayerHouse::MEET_EXPIRE_HOURS, 0, 0);
    if (dTime_c::isSameOrAfter(*now, limit)) {
        appointment->mBase.mState = QUEST_APPOINTMENT_DONE;
        mState.change(this, dANpcVisitPlayerHouse::State_c::STATE_GONE);
    } else if (arrived || talked) {
        mState.change(this, dANpcVisitPlayerHouse::State_c::STATE_IDLE);
        mState.setStayTime(talked);
        dBGCF::lockNext(0);
        mPos = l_walkTargetPos;
    } else {
        mState.change(this, dANpcVisitPlayerHouse::State_c::STATE_WAIT);
    }
    return 1;
}

// .text 0x630
int dAcNpcVisitPlayerHouse_c::doDelete() {
    if (!dAcNpcNml_c::doDelete()) {
        return 0;
    }
    dSTR::clearPartyFlag();
    return 1;
}

// .text 0x668
int dAcNpcVisitPlayerHouse_c::vtB0() {
    mState.execute(this);
    return 1;
}

namespace dANpcVisitPlayerHouse {
// .text 0x694
State_c::State_c() : mTimer(0), mWaitTimer(0), mBlockedTimer(0) {
    mState = STATE_NONE;
    mResumeState = STATE_NONE;
    mTalkState = STATE_NONE;
    mpState = NULL;
    mStep = STEP_START;
    mTimer = 0;
    mWaitTimer = 0;
    mpTalk = NULL;
    mFirstStayTimer = 0;
    mStayTimer = 0;
    mTalkCount = 0;
    mBlockedTimer = 0;
    mLookTimer = 0;
}

// .text 0x6D4
State_c::~State_c() {
}

static State_c::State_s l_states[] = {
    {&State_c::initIdle, &State_c::executeIdle},
    {&State_c::initWait, &State_c::executeWait},
    {&State_c::initGone, NULL},
    {&State_c::initCall, &State_c::executeCall},
    {&State_c::initArrive, &State_c::executeArrive},
    {&State_c::initTalk, &State_c::executeTalk},
    {&State_c::initLook, &State_c::executeLook},
    {&State_c::initSeeOff, &State_c::executeSeeOff},
    {&State_c::initLeave, &State_c::executeLeave},
    {&State_c::initPlayerLeaving, &State_c::executePlayerLeaving},
    {&State_c::initGoHome, &State_c::executeGoHome},
};

// Room wandering and stay timing (frames, units).
extern const int cWaitBase = 420;
extern const int cWaitRange = 600;
extern const int cCallWaitBase = 1200;
extern const int cCallWaitRange = 600;
extern const float cWanderMaxDist = 128.0f;
extern const float cWanderMinDist = 48.0f;
extern const float cRoomX0 = 128.0f;
extern const float cRoomZ0 = 192.0f;
extern const float cRoomX1 = 384.0f;
extern const float cRoomZ1 = 448.0f;

// .text 0x714
void State_c::change(dAcNpcVisitPlayerHouse_c *npc, u32 state) {
    if (state < STATE_COUNT) {
        mState = state;
        mpState = &l_states[state];
        mStep = STEP_START;
        if (mpState != NULL) {
            (this->*mpState->mInit)(npc);
        }
    }
}

// .text 0x764
void State_c::execute(dAcNpcVisitPlayerHouse_c *npc) {
    if (mTimer) {
        --mTimer;
    }
    if (mWaitTimer) {
        --mWaitTimer;
    }
    if (mFirstStayTimer) {
        --mFirstStayTimer;
    }
    if (mStayTimer) {
        --mStayTimer;
    }
    if (mpState != NULL && mpState->mExecute) {
        (this->*mpState->mExecute)(npc);
    }
}

// .text 0x81C
void State_c::setTalk(u32 resume, u32 talk,
                      const dAcNpcNml_c::talk_c::procSet_s *set) {
    mResumeState = resume;
    mTalkState = talk;
    mpTalk = set;
}

// .text 0x82C
BOOL State_c::isStayOver() {
    dQuestPlayerItem_c *appointment =
        &dSaveData_c::getRaw()->mAnimals.mTown.mAppointment;
    if (appointment->isFlag(QUEST_VISIT_FLAG_TALKED)) {
        if (mStayTimer == 0) {
            return TRUE;
        }
    } else if (appointment->isFlag(QUEST_VISIT_FLAG_ARRIVED) &&
               mFirstStayTimer == 0) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x8BC
void State_c::setStayTime(BOOL resumed) {
    if (resumed) {
        mTalkCount = cM::rndInt(STAY_TALK_RANGE) + STAY_TALK_BASE;
        mStayTimer = cM::rndInt(STAY_TIME_RANGE) + STAY_TIME_BASE;
    } else {
        mTalkCount = cM::rndInt(STAY_TALK_RANGE) + FIRST_STAY_TALK_BASE;
        mStayTimer = cM::rndInt(STAY_TIME_RANGE) + FIRST_STAY_TIME_BASE;
        mFirstStayTimer = FIRST_STAY_FRAMES;
    }
}

// .text 0x938
void State_c::initIdle(dAcNpcVisitPlayerHouse_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
    npc->mIsActive = TRUE;
    npc->mCheckGround = TRUE;
    dSTR::setPartyFlag();
    mBlockedTimer = 0;
    mLookTimer = LOOK_WAIT_BASE + cM::rndF(cWaitRange);
}

// .text 0x9D4
BOOL State_c::avoidObstacles(dAcNpcVisitPlayerHouse_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    BOOL result = FALSE;
    if (!action->isArrived(npc, TRUE)) {
        mVec3_c pos;
        switch (npc->getSidePos(&pos)) {
        case SIDE_BLOCKED:
            action->requestWait(PRIORITY_IDLE, cNpcMorphFrames);
            result = TRUE;
            break;
        case SIDE_DETOUR:
            action->setGoalPos(pos);
            break;
        }
    } else if (action->isDetour()) {
        action->resetGoal();
    }
    return result;
}

// .text 0xA94
BOOL State_c::stopIfBlocked(dAcNpcVisitPlayerHouse_c *npc) {
    if (0.0f != npc->getSpeedF()) {
        BOOL stop = FALSE;
        if (mBlockedTimer > BLOCKED_FRAME_LIMIT) {
            stop = TRUE;
        }
        if (npc->mPushTimer > BLOCKED_FRAME_LIMIT) {
            stop = TRUE;
        }
        if (stop) {
            npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
            npc->mPushTimer = 0;
            mBlockedTimer = 0;
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0xB34
BOOL State_c::checkPlayerLeaving(dAcNpcVisitPlayerHouse_c *npc) {
    mVec3_c *pos = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (pos != NULL && dBGCF::isNextLocked(pos) &&
        !dDemo::isType(dDemo::TYPE_ANY) && dDemo::requestSpeak(npc, NULL)) {
        change(npc, STATE_PLAYER_LEAVING);
        return TRUE;
    }
    return FALSE;
}

// .text 0xBC4
BOOL State_c::checkGoHome(dAcNpcVisitPlayerHouse_c *npc) {
    if (isStayOver() && !dDemo::isType(dDemo::TYPE_ANY) &&
        dDemo::requestSpeak(npc, NULL)) {
        change(npc, STATE_GO_HOME);
        return TRUE;
    }
    return FALSE;
}

// .text 0xC44
BOOL State_c::checkIdle(dAcNpcVisitPlayerHouse_c *npc) {
    if (checkPlayerLeaving(npc)) {
        return TRUE;
    }
    if (checkGoHome(npc)) {
        return TRUE;
    }
    if (0.0f != npc->getSpeedF()) {
        if (avoidObstacles(npc)) {
            return TRUE;
        }
        if (stopIfBlocked(npc)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0xCF8
void State_c::executeIdle(dAcNpcVisitPlayerHouse_c *npc) {
    const mVec3_c *cur;
    int yawStep;
    BOOL found;
    if (0.0f != npc->getSpeedF()) {
        if (npc->mAcch.mHitFlags &
            (dBGCF::HIT_WALL_FRONT | dBGCF::HIT_STEP_EXT_FRONT)) {
            if (mBlockedTimer < ULONG_MAX) {
                ++mBlockedTimer;
            }
        } else {
            mBlockedTimer = 0;
        }
    } else {
        mBlockedTimer = 0;
    }
    if (checkIdle(npc)) {
        return;
    }
    if (npc->mAction.mCanChange &&
        (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT || !mWaitTimer)) {
        mVec3_c pos = npc->mPos;
        cur = &npc->mPos;
        found = FALSE;
        if (cM::rndInt(WANDER_ROLL_COUNT) < WANDER_MOVE_ROLL_COUNT) {
            found = dAcNpc_c::searchPosInRect(&pos, cur, cRoomX0, cRoomZ0,
                                              cRoomX1, cRoomZ1, cWanderMinDist,
                                              cWanderMaxDist);
        }
        if (found) {
            mAng angle(0);
            if (npc->checkTargetAngle(&angle, &pos, &l_frontAngle) &&
                distanceXZ(pos, npc->mPos) > cWanderMinDist) {
                npc->mAction.requestWalk(PRIORITY_IDLE, pos, l_defaultAnmRate,
                                         l_walkTurnSpeed, 0, l_moveParamWalk,
                                         cNpcMorphFrames);
            } else {
                npc->mAction.requestTurnWalk(PRIORITY_IDLE, pos, angle,
                                             l_turnSpeed, 0, l_moveParamWalk,
                                             l_defaultAnmRate, cNpcMorphFrames);
            }
        } else {
            mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
            if (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT) {
                npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
            }
        }
    }
    if (mLookTimer != 0) {
        mLookTimer--;
    }
    if (mLookTimer == 0) {
        if (npc->mLookAt.mType != dAcNpc_c::lookAt_c::LOOK_AROUND &&
            !(cM::rndInt(LOOK_ROLL_COUNT) & 1)) {
            yawStep = l_lookYawStep;
            npc->mLookAt.setAround(PRIORITY_IDLE,
                                   mAng(cM::rndInt(l_lookPitchMax >> 4)),
                                   mAng(l_lookYawMax >> 1), l_lookPitchStep,
                                   mAng((yawStep >> 1) + cM::rndInt(yawStep)));
        } else if (!(cM::rndInt(LOOK_ROLL_COUNT) & 1)) {
            npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep,
                                   l_lookYawStep, cNpcLookRange);
        } else {
            npc->mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep,
                                  l_lookYawStep);
        }
        mLookTimer = LOOK_WAIT_BASE + cM::rndF(cWaitRange);
    }
}

// .text 0x102C
void State_c::initWait(dAcNpcVisitPlayerHouse_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    npc->mIsActive = FALSE;
}

// .text 0x1070
void State_c::executeWait(dAcNpcVisitPlayerHouse_c *npc) {
    dSaveData_c *save = dSaveData_c::getRaw();
    dTime_c *now = dTime_c::getCurrent();
    dTime_c meet = save->mAnimals.mTown.mAppointment.getMeetTime();
    dTime_c limit = meet;
    limit.add(0, 0, MEET_LATE_MINUTES, 0);
    if (dTime_c::isSameOrAfter(*now, limit)) {
        change(npc, STATE_GONE);
    } else if (dTime_c::isSameOrAfter(*now, meet)) {
        change(npc, STATE_CALL);
    }
}

// .text 0x12E0
void State_c::initGone(dAcNpcVisitPlayerHouse_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    npc->mIsActive = FALSE;
}

// .text 0x1324
void State_c::initCall(dAcNpcVisitPlayerHouse_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mTimer = cM::rndInt(cCallWaitRange) + cCallWaitBase;
    npc->mIsActive = FALSE;
}

// .text 0x1384
void State_c::executeCall(dAcNpcVisitPlayerHouse_c *npc) {
    dSaveData_c *save = dSaveData_c::getRaw();
    dTime_c *now = dTime_c::getCurrent();
    dTime_c meet = save->mAnimals.mTown.mAppointment.getMeetTime();
    dTime_c limit = meet;
    limit.add(0, 0, MEET_LATE_MINUTES, 0);
    if (dTime_c::isSameOrAfter(*now, limit)) {
        change(npc, STATE_GONE);
    } else if (mTimer == 0 && isPlayerAwayFromDoor() &&
               dDemo::requestSpeak(npc, NULL)) {
        setTalk(STATE_GONE, STATE_ARRIVE,
                dAcNpcNml_c::talk_c::getProcSet(
                    dAcNpcNml_c::talk_c::TALK_PROC_VISIT_CALL));
    }
}

// .text 0x157C
BOOL State_c::isPlayerAwayFromDoor() {
    mVec3_c *pos = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (pos == NULL) {
        return FALSE;
    }
    mVec3_c door = l_walkTargetPos;
    if (distanceXZ(l_walkTargetPos, *pos) <= VISIT_DOOR_NEAR_DIST) {
        return FALSE;
    }
    return TRUE;
}

// .text 0x1614
void State_c::initArrive(dAcNpcVisitPlayerHouse_c *npc) {
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    npc->mIsActive = FALSE;
    npc->mCheckGround = FALSE;
    npc->mAudioObj.startSound(SE_room_DoorOpenCloseWithKnock);
    mTimer = KNOCK_FRAMES;
}

// .text 0x169C
void State_c::stepArriveKnock(dAcNpcVisitPlayerHouse_c *npc) {
    if (mTimer == 0) {
        npc->mTalk.requestTalk(true, TRUE);
        mStep = ARRIVE_STEP_TALK;
    } else if (mTimer == KNOCK_ROOM_SE_FRAME) {
        dSTR::playRoomSe();
    }
}

// .text 0x16F8
void State_c::stepArriveTalk(dAcNpcVisitPlayerHouse_c *npc) {
    if (!npc->mTalk.isBusy()) {
        npc->mCheckGround = TRUE;
        dDemo::finishTalk(npc);
        setStayTime(FALSE);
        dBGCF::lockNext(0);
        dCamera_c::sInstance->mRequestedMode =
            dCamera_c::sInstance->mDefaultMode;
        ++mStep;
    }
}

static const State_c::Proc l_arriveSteps[] = {&State_c::stepArriveKnock,
                                              &State_c::stepArriveTalk};

// .text 0x177C
void State_c::executeArrive(dAcNpcVisitPlayerHouse_c *npc) {
    if (mStep < ARRIVE_STEP_DONE) {
        (this->*l_arriveSteps[mStep])(npc);
    }
}

// .text 0x17BC
void State_c::initTalk(dAcNpcVisitPlayerHouse_c *npc) {
    u16 angleValue = 0;
    dActor_c *actor = npc->mNmlTalk.mpActActor;
    if (actor) {
        angleValue = npc->getAngleYTo(actor).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, FALSE);
}

// .text 0x1828
void State_c::stepTalk(dAcNpcVisitPlayerHouse_c *npc) {
    if (!npc->mTalk.isBusy()) {
        dDemo::finishTalk(npc);
        if (mTalkCount) {
            --mTalkCount;
        } else {
            mStayTimer = cM::rndInt(cCallWaitRange) + cCallWaitBase;
        }
        ++mStep;
    }
}

static const State_c::Proc l_talkSteps[] = {&State_c::stepTalk};

// .text 0x18A8
void State_c::executeTalk(dAcNpcVisitPlayerHouse_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_talkSteps[mStep])(npc);
    }
}

// .text 0x18E8
void State_c::initLook(dAcNpcVisitPlayerHouse_c *npc) {
    mAng angle = npc->getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
    npc->mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                             cNpcMorphFrames);
}

// .text 0x1948
void State_c::stepLook(dAcNpcVisitPlayerHouse_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

static const State_c::Proc l_lookSteps[] = {&State_c::stepLook};

// .text 0x19AC
void State_c::executeLook(dAcNpcVisitPlayerHouse_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_lookSteps[mStep])(npc);
    }
}

// .text 0x19EC
void State_c::initSeeOff(dAcNpcVisitPlayerHouse_c *npc) {
    u16 angleValue = 0;
    dActor_c *actor = npc->mNmlTalk.mpActActor;
    if (actor) {
        angleValue = npc->getAngleYTo(actor).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, FALSE);
}

// .text 0x1A58
void State_c::stepSeeOff(dAcNpcVisitPlayerHouse_c *npc) {
    if (!npc->mTalk.isBusy()) {
        getSceneChange()->requestExit(EXIT_FRONT_DOOR, EXIT_WIPE, EXIT_WIPE);
        dSTR::clearPartyFlag();
        ++mStep;
    }
}

static const State_c::Proc l_seeOffSteps[] = {&State_c::stepSeeOff};

// .text 0x1AB4
void State_c::executeSeeOff(dAcNpcVisitPlayerHouse_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_seeOffSteps[mStep])(npc);
    }
}

// .text 0x1AF4
void State_c::initLeave(dAcNpcVisitPlayerHouse_c *npc) {
    u16 angleValue = 0;
    dActor_c *actor = npc->mNmlTalk.mpActActor;
    if (actor) {
        angleValue = npc->getAngleYTo(actor).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, FALSE);
}

// .text 0x1B60
void State_c::stepLeave(dAcNpcVisitPlayerHouse_c *npc) {
    if (!npc->mTalk.isBusy()) {
        getSceneChange()->setReturnExitHere(0.0f);
        getSceneChange()->requestExit(EXIT_RETURN, EXIT_WIPE, EXIT_WIPE);
        dSTR::clearPartyFlag();
        ++mStep;
    }
}

static const State_c::Proc l_leaveSteps[] = {&State_c::stepLeave};

// .text 0x1BCC
void State_c::executeLeave(dAcNpcVisitPlayerHouse_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_leaveSteps[mStep])(npc);
    }
}

// .text 0x1C0C
void State_c::initPlayerLeaving(dAcNpcVisitPlayerHouse_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    npc->mIsActive = TRUE;
    npc->mCheckGround = TRUE;
    dSTR::setPartyFlag();
    setTalk(STATE_LOOK, STATE_SEE_OFF,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_VISIT_WAIT));
}

// .text 0x1CC0
void State_c::executePlayerLeaving(dAcNpcVisitPlayerHouse_c *npc) {
    mVec3_c *pos = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (pos != NULL && !dBGCF::isNextLocked(pos)) {
        change(npc, STATE_IDLE);
    } else {
        dDemo::requestSpeak(npc, NULL);
    }
}

// .text 0x1D30
void State_c::initGoHome(dAcNpcVisitPlayerHouse_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    npc->mIsActive = TRUE;
    npc->mCheckGround = TRUE;
    dSTR::setPartyFlag();
    setTalk(STATE_LOOK, STATE_LEAVE,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_VISIT_BACK));
}

// .text 0x1DE4
void State_c::executeGoHome(dAcNpcVisitPlayerHouse_c *npc) {
    dDemo::requestSpeak(npc, NULL);
}
} // namespace dANpcVisitPlayerHouse
