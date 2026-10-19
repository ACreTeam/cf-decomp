#include <game/game/d_a_npc_visit_npc_house.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <climits>
#include <game/game/d_a_player.hpp>

// How far the player steps back from the exit (STATE_HOLD_PLAYER).
#define VISIT_STEP_BACK_DIST 32.0f

// The villager at home on the player's visit. Functions are in target .text
// order.

namespace dANpcVisitNpcHouse {
// .text 0x5C
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}
static inline float lengthXZ(const mVec3_c &v) {
    return EGG::Mathf::sqrt(v.x * v.x + v.z * v.z);
}
static inline float distanceXZ(const mVec3_c &a, const mVec3_c &b) {
    return lengthXZ(a - b);
}
static inline dQuestPlayerItem_c *getAppointment() {
    return &dSaveData_c::getRaw()->mAnimals.mTown.mAppointment;
}
// .text 0x68
BASE_PROFILE(A_NPC_VISIT_NPC_HOUSE, dAcNpcVisitNpcHouse_c);
} // namespace dANpcVisitNpcHouse

// .text 0x1EC
bool dAcNpcVisitNpcHouse_c::canInteract(dDemoActor_c *actor) {
    if (!dAcNpcNml_c::canInteract(actor)) {
        return false;
    }
    if (actor == NULL) {
        return false;
    }
    return mTalk.isBusy() == FALSE;
}

// .text 0x254
void dAcNpcVisitNpcHouse_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        if (mState.mResumeState < dANpcVisitNpcHouse::State_c::STATE_COUNT) {
            mState.change(this, mState.mResumeState);
        } else {
            mState.change(this, dANpcVisitNpcHouse::State_c::STATE_LOOK);
        }
        mState.mResumeState = dANpcVisitNpcHouse::State_c::STATE_NONE;
        break;
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK: {
        u32 state = mState.mTalkState;
        mNmlTalk.start(
            this,
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT),
            mState.mpTalk);
        if (mFtr != dItem::ITEM_ID_NONE) {
            mNmlTalk.setRoomFtr(mFtr, mFtrX, mFtrZ);
        }
        if (state < dANpcVisitNpcHouse::State_c::STATE_COUNT) {
            mState.change(this, state);
        } else {
            mState.change(this, dANpcVisitNpcHouse::State_c::STATE_TALK);
        }
        mState.mTalkState = dANpcVisitNpcHouse::State_c::STATE_NONE;
        break;
    }
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        mState.change(this, dANpcVisitNpcHouse::State_c::STATE_IDLE);
        break;
    }
    dAcNpc_c::onDemoCmd(cmd, member);
}

// .text 0x38C
int dAcNpcVisitNpcHouse_c::preCreate() {
    return dAcNpcNml_c::preCreate() != FALSE;
}

// .text 0x3B8
int dAcNpcVisitNpcHouse_c::create() {
    if (!dAcNpcNml_c::create()) {
        return 0;
    }
    selectTradeFtrs();
    mState.change(this, dANpcVisitNpcHouse::State_c::STATE_WELCOME);
    mFtrX = INVALID_UNIT_COORD;
    mFtrZ = INVALID_UNIT_COORD;
    mFtr.mId = dItem::ITEM_ID_NONE;
    dBGCF::lockNext(0);
    dFTR::disableHoldOnly();
    return 1;
}

// .text 0x434
int dAcNpcVisitNpcHouse_c::doDelete() {
    if (!dAcNpcNml_c::doDelete()) {
        return 0;
    }
    dSaveData_c::getTown()->mAnimals.mTown.mAppointment.mBase.setState(
        QUEST_APPOINTMENT_DONE);
    dMoney_c::offForceShow();
    return 1;
}

// .text 0x47C
int dAcNpcVisitNpcHouse_c::vtB0() {
    mState.execute(this);
    return 1;
}

// .text 0x4A8
int dAcNpcVisitNpcHouse_c::removeRoomFtr(int obj, int x, int z) {
    dAnimal_c *animal = mpAnimal;
    if (animal) {
        u32 idx = getRoomFtrIdx(x, z);
        if (idx < ANIMAL_FTR_COUNT) {
            animal->clearFtr(idx);
        }
    }
    if (obj != INVALID_FTR_HANDLE && dFtrm_c::canTalk(obj)) {
        dFtrm_c::sendEvent(obj);
        return 1;
    }
    return 0;
}

namespace dANpcVisitNpcHouse {
// .text 0x530
State_c::State_c() : mTimer(0), mWaitTimer(0), mBlockedTimer(0) {
    mState = STATE_NONE;
    mpState = NULL;
    mStep = STEP_START;
    mTimer = 0;
    mWaitTimer = 0;
    mpTalk = NULL;
    mResumeState = STATE_NONE;
    mTalkState = STATE_NONE;
    _20 = 0;
    mBlockedTimer = 0;
}

// .text 0x564
State_c::~State_c() {
}

static const State_c::State_s l_states[] = {
    {&State_c::initIdle, &State_c::executeIdle},
    {&State_c::initOffScreen, &State_c::executeOffScreen},
    {&State_c::initTalk, &State_c::executeTalk},
    {&State_c::initLook, &State_c::executeLook},
    {&State_c::initWelcome, &State_c::executeWelcome},
    {&State_c::initHoldPlayer, &State_c::executeHoldPlayer},
    {&State_c::initSeeOff, &State_c::executeSeeOff},
};
// Room wandering timing and geometry parameters.
extern const int cWaitBase = 420;
extern const int cWaitRange = 600;
extern const float cWanderMaxDist = 128.0f;
extern const float cWanderMinDist = 48.0f;
extern const float cRoomX0 = 160.0f;
extern const float cRoomZ0 = 256.0f;
extern const float cRoomX1 = 352.0f;
extern const float cRoomZ1 = 448.0f;

// .text 0x5A4
void State_c::change(dAcNpcVisitNpcHouse_c *npc, u32 state) {
    if (state < STATE_COUNT) {
        mState = state;
        mpState = &l_states[state];
        mStep = STEP_START;
        if (mpState) {
            (this->*mpState->mInit)(npc);
        }
    }
}

// .text 0x5F4
void State_c::execute(dAcNpcVisitNpcHouse_c *npc) {
    if (mTimer) {
        --mTimer;
    }
    if (mWaitTimer) {
        --mWaitTimer;
    }
    if (mpState) {
        (this->*mpState->mExecute)(npc);
    }
}

// .text 0x650
void State_c::setTalk(u32 resume, u32 talk,
                      const dAcNpcNml_c::talk_c::procSet_s *set) {
    mResumeState = resume;
    mTalkState = talk;
    mpTalk = set;
}

// .text 0x660
void State_c::initIdle(dAcNpcVisitNpcHouse_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
    setTalk(STATE_LOOK, STATE_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_INVITE_FIRST));
    _20 = 0;
    mSpeakRequested = FALSE;
    mBlockedTimer = 0;
    npc->mBusy = FALSE;
}

// .text 0x6F0
BOOL State_c::avoidObstacles(dAcNpcVisitNpcHouse_c *npc) {
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

// .text 0x7B0
BOOL State_c::stopIfBlocked(dAcNpcVisitNpcHouse_c *npc) {
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

// .text 0x850
BOOL State_c::checkMove(dAcNpcVisitNpcHouse_c *npc) {
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

// .text 0x8CC: walk around; offer the furniture the player points at, and
// talk to the player when they walk to the exit.
void State_c::executeIdle(dAcNpcVisitNpcHouse_c *npc) {
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
    if (mSpeakRequested) {
        if (dDemo::isType(dDemo::TYPE_ANY) || npc->isInUse(NULL) ||
            dPlayerMgr::getChairPosition(NULL,
                                         dPlayerMgr::PLAYER_INDEX_CURRENT) ||
            dPlayerMgr::getBedPosition(NULL,
                                       dPlayerMgr::PLAYER_INDEX_CURRENT)) {
            npc->mBusy = FALSE;
            mSpeakRequested = FALSE;
            setTalk(STATE_LOOK, STATE_TALK,
                    dAcNpcNml_c::talk_c::getProcSet(
                        dAcNpcNml_c::talk_c::TALK_PROC_INVITE_FIRST));
        } else if (dDemo::requestSpeak(npc, NULL)) {
            npc->mBusy = TRUE;
        }
    } else {
        dQuestPlayerItem_c *appt = getAppointment();
        if (appt->isFlag(QUEST_INVITE_FLAG_TRADE_OFFERED) &&
            !appt->isFlag(QUEST_INVITE_FLAG_TRADED)) {
            int x = 0;
            int z = 0;
            if (dAcNpc_c::getPointedUnit(&x, &z, &l_pointAngle, 0, l_pointRange,
                                         l_pointDist)) {
                dItem::Item ftr = dAcNpcNml_c::getRoomFtrAt(&x, &z, x, z);
                if (ftr.mId != dItem::ITEM_ID_NONE &&
                    !dDemo::isType(dDemo::TYPE_ANY) && !npc->isInUse(NULL) &&
                    !dPlayerMgr::getChairPosition(
                        NULL, dPlayerMgr::PLAYER_INDEX_CURRENT) &&
                    !dPlayerMgr::getBedPosition(
                        NULL, dPlayerMgr::PLAYER_INDEX_CURRENT) &&
                    dDemo::requestSpeak(npc, NULL)) {
                    setTalk(STATE_LOOK, STATE_TALK,
                            dAcNpcNml_c::talk_c::getProcSet(
                                dAcNpcNml_c::talk_c::TALK_PROC_INVITE_TRADE));
                    npc->mFtr = ftr;
                    npc->mFtrX = x;
                    npc->mFtrZ = z;
                    npc->mBusy = TRUE;
                    mSpeakRequested = TRUE;
                    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
                }
            }
        }
        mVec3_c *pos =
            dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (!mSpeakRequested && pos && dBGCF::isNextLocked(pos) &&
            dDemo::requestSpeak(npc, NULL)) {
            if (appt->isFlag(QUEST_INVITE_FLAG_TALKED)) {
                setTalk(STATE_LOOK, STATE_SEE_OFF,
                        dAcNpcNml_c::talk_c::getProcSet(
                            dAcNpcNml_c::talk_c::TALK_PROC_INVITE_ANALOG));
                mSpeakRequested = TRUE;
                npc->mBusy = TRUE;
            } else {
                setTalk(STATE_LOOK, STATE_HOLD_PLAYER,
                        dAcNpcNml_c::talk_c::getProcSet(
                            dAcNpcNml_c::talk_c::TALK_PROC_INVITE_WAIT));
                mSpeakRequested = TRUE;
                npc->mBusy = TRUE;
            }
        }
    }
    if (!mSpeakRequested && !checkMove(npc) && npc->mAction.mCanChange &&
        (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT || !mWaitTimer)) {
        mVec3_c pos(npc->mPos);
        if (cM::rndInt(WANDER_ROLL_COUNT) < WANDER_MOVE_ROLL_COUNT &&
            dAcNpc_c::searchPosInRect(&pos, &npc->mPos, cRoomX0, cRoomZ0,
                                      cRoomX1, cRoomZ1, cWanderMinDist,
                                      cWanderMaxDist)) {
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
}

// .text 0xD90
void State_c::initOffScreen(dAcNpcVisitNpcHouse_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0xDA4
void State_c::executeOffScreen(dAcNpcVisitNpcHouse_c *npc) {
    if (!npc->mViewArea.isOutside(npc)) {
        change(npc, STATE_IDLE);
    }
}

// .text 0xDF8
void State_c::initTalk(dAcNpcVisitNpcHouse_c *npc) {
    mAng angle = npc->getTalkAngle();
    mAng speed = npc->getTalkTurnSpeed();
    npc->mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    npc->mBusy = FALSE;
}

// .text 0xE6C
void State_c::stepTalk(dAcNpcVisitNpcHouse_c *npc) {
    if (!npc->mTalk.isBusy()) {
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

static const State_c::Proc l_talkSteps[] = {&State_c::stepTalk};

// .text 0xEC4
void State_c::executeTalk(dAcNpcVisitNpcHouse_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_talkSteps[mStep])(npc);
    }
}

// .text 0xF04
void State_c::initLook(dAcNpcVisitNpcHouse_c *npc) {
    mAng angle = npc->getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
    npc->mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                             cNpcMorphFrames);
}

// .text 0xF64
void State_c::stepLook(dAcNpcVisitNpcHouse_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

static const State_c::Proc l_lookSteps[] = {&State_c::stepLook};

// .text 0xFC8
void State_c::executeLook(dAcNpcVisitNpcHouse_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_lookSteps[mStep])(npc);
    }
}

// .text 0x1008
void State_c::initWelcome(dAcNpcVisitNpcHouse_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x101C: speak to the player at the door until it starts.
void State_c::stepWelcome(dAcNpcVisitNpcHouse_c *npc) {
    if (dDemo::requestSpeak(npc, NULL)) {
        setTalk(STATE_LOOK, STATE_TALK,
                dAcNpcNml_c::talk_c::getProcSet(
                    dAcNpcNml_c::talk_c::TALK_PROC_INVITE_WELCOME));
    }
}

static const State_c::Proc l_welcomeSteps[] = {&State_c::stepWelcome};

// .text 0x1074
void State_c::executeWelcome(dAcNpcVisitNpcHouse_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_welcomeSteps[mStep])(npc);
    }
}

// .text 0x10B4
void State_c::initHoldPlayer(dAcNpcVisitNpcHouse_c *npc) {
    mAng angle = npc->getTalkAngle();
    mAng speed = npc->getTalkTurnSpeed();
    npc->mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
}

// .text 0x1120: after the talk the player steps back from the exit.
void State_c::stepHoldTalk(dAcNpcVisitNpcHouse_c *npc) {
    if (!npc->mTalk.isBusy()) {
        mVec3_c *pos =
            dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (pos) {
            mVec3_c target(*pos);
            target.z -= VISIT_STEP_BACK_DIST;
            dPlayerMgr::moveTo(&target, dPlayerMgr::PLAYER_INDEX_CURRENT,
                               dPlayerMgr::sDefaultMoveSpeed);
            ++mStep;
        }
    }
}

// .text 0x11AC
void State_c::stepHoldMove(dAcNpcVisitNpcHouse_c *npc) {
    if (!dPlayerMgr::isMovingTo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

static const State_c::Proc l_holdSteps[] = {&State_c::stepHoldTalk,
                                            &State_c::stepHoldMove};

// .text 0x1204
void State_c::executeHoldPlayer(dAcNpcVisitNpcHouse_c *npc) {
    if (mStep < HOLD_STEP_DONE) {
        (this->*l_holdSteps[mStep])(npc);
    }
}

// .text 0x1244
void State_c::initSeeOff(dAcNpcVisitNpcHouse_c *npc) {
    mAng angle = npc->getTalkAngle();
    mAng speed = npc->getTalkTurnSpeed();
    npc->mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
}

// .text 0x12B0: after the talk the player leaves the house.
void State_c::stepSeeOff(dAcNpcVisitNpcHouse_c *npc) {
    if (!npc->mTalk.isBusy()) {
        getSceneChange()->requestExit(0, 2, 2);
        ++mStep;
    }
}

static const State_c::Proc l_seeOffSteps[] = {&State_c::stepSeeOff};

// .text 0x1308
void State_c::executeSeeOff(dAcNpcVisitNpcHouse_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_seeOffSteps[mStep])(npc);
    }
}
} // namespace dANpcVisitNpcHouse
