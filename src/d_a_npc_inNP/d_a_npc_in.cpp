#include <game/game/d_a_npc_in.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/snd/snd_id.hpp>
#include <climits>
#include <game/game/d_a_player.hpp>

namespace dANpcIn {
static inline int getEventId(const dAnimalBlock_c *block) {
    return block->mEventId;
}
static inline BOOL isAnm(dAcNpc_c::anm_c *anm, int id) {
    return anm->mAnmId == id;
}
static inline float lengthXZ(const mVec3_c &v) {
    return EGG::Mathf::sqrt(v.x * v.x + v.z * v.z);
}
static inline float distanceXZ(const mVec3_c &a, const mVec3_c &b) {
    return lengthXZ(a - b);
}
// .text 0xCC
BASE_PROFILE(A_NPC_IN, dAcNpcIn_c);
} // namespace dANpcIn
// .text 0x260
bool dAcNpcIn_c::canInteract(dDemoActor_c *actor) {
    return dAcNpcNml_c::canInteract(actor) != FALSE;
}
// .text 0x28C
void dAcNpcIn_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START) {
            mState.change(this, dANpcIn::State_c::LOOK);
        } else if (permit == PERMIT_LOCAL_START) {
            if (mState.mResumeState < dANpcIn::State_c::COUNT) {
                mState.change(this, mState.mResumeState);
            } else {
                mState.change(this, dANpcIn::State_c::LOOK);
            }
            mState.mResumeState = dANpcIn::State_c::COUNT;
        }
        break;
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK:
        if (permit == PERMIT_REMOTE_TALK) {
            mState.change(this, dANpcIn::State_c::REMOTE_LOOK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mState.startTalk(this);
        }
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            mState.selectLocal(this);
            mState.enterSelected(this);
        } else if (permit == PERMIT_END_MULTI) {
            mState.selectRemote(this);
            mState.enterSelected(this);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            if (!dNet::isSceneOwner()) {
                mState.selectLocal(this);
            } else {
                mState.selectRemote(this);
            }
            mState.enterSelected(this);
        }
        break;
    }
    dAcNpc_c::onDemoCmd(cmd, member);
}
// .text 0x41C
int dAcNpcIn_c::removeRoomFtr(int obj, int x, int z) {
    dAnimal_c *animal = mpAnimal;
    if (animal) {
        u32 idx = getRoomFtrIdx(x, z);
        if (idx < ANIMAL_FTR_COUNT) {
            animal->clearFtr(idx);
            animal->clearFtrBoxed(idx);
            dItem::Item empty;
            dNpc::sendAnimalFtr(getNpcIdx(), &empty, idx);
        }
    }
    if (obj != INVALID_FTR_HANDLE && dFtrm_c::canTalk(obj)) {
        dFtrm_c::sendEvent(obj);
        dNpc::sendAnimalFtrTouch((u8)dScene::getCurrentScene(), (u8)x, (u8)z);
        return 1;
    }
    return 0;
}
// .text 0x4F4
int dAcNpcIn_c::preCreate() {
    return dAcNpcNml_c::preCreate() != FALSE;
}
// .text 0x520
int dAcNpcIn_c::create() {
    if (!dAcNpcNml_c::create()) {
        return 0;
    }
    if (dScene::isSceneAttr(dScene::getPrevScene(), dScene::SCENE_ATTR_TOWN) &&
        mpEntry) {
        dNpc::entry_c *entry = mpEntry;
        entry->mFlags.mEventTalked = FALSE;
        entry->mFlags.mFmarketSale = FALSE;
    }
    memcpy(&mMoveParam, &l_moveParamWalk, sizeof(mMoveParam));
    mFtrX = INVALID_UNIT_COORD;
    mFtrZ = INVALID_UNIT_COORD;
    mFtr.mId = dItem::ITEM_ID_NONE;
    if (isLocalOwner()) {
        mState.selectLocal(this);
        mState.enterSelected(this);
    } else {
        mState.selectRemote(this);
        mState.enterSelected(this);
    }
    return 1;
}
// .text 0x600
int dAcNpcIn_c::doDelete() {
    if (!dAcNpcNml_c::doDelete()) {
        return 0;
    }
    dMoney_c::offForceShow();
    return 1;
}
// .text 0x638
int dAcNpcIn_c::vtB0() {
    mState.execute(this);
    mMood.execute(this);
    return 1;
}
// .text 0x67C
BOOL dAcNpcIn_c::isSick() const {
    dAnimal_c *animal = getAnimal();
    if (!animal) {
        return FALSE;
    }
    const dAnmPersonalID_c *id = &animal->mID;
    if (!id->isValid()) {
        return FALSE;
    }
    if (dSaveData_c::getRaw()->mAnimals.mTown.isSickAnimal(id) == FALSE) {
        return FALSE;
    }
    return getSickness() < SICK_NONE;
}
// .text 0x71C
int dAcNpcIn_c::getSickness() const {
    dAnimal_c *animal = getAnimal();
    if (!animal) {
        return SICK_NONE;
    }
    dQuestVillager_c *quest = &animal->mQuest.mQuest;
    if (!quest->mBase.isActive() ||
        quest->mBase.getKind() != QUEST_KIND_REQUEST_5) {
        return SICK_NONE;
    }
    return quest->mMatchParam;
}
// .text 0x780
void dAcNpcIn_c::stopEmotion() {
    switch (mAction.mActionId) {
    default:
        return;
    case ACTION_EMOTION:
        if (mAction.mEmotion != dANpcIn::EMOTION_DEFAULT) {
            return stopManpu();
        }
    }
}

namespace dANpcIn {
// .text 0x7A0
State_c::State_c() : mTimer(0), mWaitTimer(0), mBlockedTimer(0) {
    mState = COUNT;
    mpState = NULL;
    mStep = STEP_START;
    mTimer = 0;
    mWaitTimer = 0;
    mpTalk = NULL;
    mResumeState = COUNT;
    mTalkState = COUNT;
    mSelectedState = IDLE;
    mBlockedTimer = 0;
}
// .text 0x848
State_c::~State_c() {
}
// .text 0x8C8
void State_c::setTalk(u32 resume, u32 talk,
                      const dAcNpcNml_c::talk_c::procSet_s *set) {
    mResumeState = resume;
    mTalkState = talk;
    mpTalk = set;
}
static const State_c::State_s l_states[] = {
    {&State_c::initIdle, &State_c::executeIdle},
    {&State_c::initTalk, &State_c::executeTalk},
    {&State_c::initLook, &State_c::executeLook},
    {&State_c::initRemote, &State_c::executeRemote},
    {&State_c::initRemoteLook, &State_c::executeRemoteLook},
    {&State_c::initFmarket, &State_c::executeFmarket},
    {&State_c::initFmarketRemote, &State_c::executeFmarketRemote},
    {&State_c::initSick, &State_c::executeSick},
    {&State_c::initChangeCloth, &State_c::executeChangeCloth}};
// Indoor wandering and illness timing/geometry parameters.
extern const int cWaitBase = 420;
extern const int cWaitRange = 600;
extern const float cWanderMaxDist = 128.0f;
extern const float cWanderMinDist = 48.0f;
extern const float cRoomX0 = 160.0f;
extern const float cRoomZ0 = 256.0f;
extern const float cRoomX1 = 352.0f;
extern const float cRoomZ1 = 448.0f;
extern const float cSickMaxDist = 80.0f;
extern const int cSickWaitBase = 180;
extern const int cSickWaitRange = 60;
extern const int cEmotionWaitBase = 60;
extern const int cEmotionWaitRange = 20;
extern const int cSickEmotionWaitRange = 120;
// .text 0x8D8
void State_c::change(dAcNpcIn_c *npc, u32 state) {
    if (state < COUNT) {
        mState = state;
        mpState = &l_states[state];
        mStep = STEP_START;
        if (mpState) {
            (this->*mpState->mInit)(npc);
        }
    }
}
// .text 0x928
u32 State_c::getEventState(dAcNpcIn_c *npc, u32 state) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!player || player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
        return COUNT;
    }
    if (!dEvent::isActive(EVENT_FLEA_MARKET)) {
        return COUNT;
    }
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    if (getEventId(block) != EVENT_FLEA_MARKET) {
        return COUNT;
    }
    dAnimal_c *animal = npc->mpAnimal;
    if (!animal || animal->isMoving() || !animal->mEvent.isInEvent()) {
        return COUNT;
    }
    if (block->isSickAnimal(&animal->mID)) {
        state = COUNT;
    }
    return state;
}
// .text 0xA18
u32 State_c::getLocalEventState(dAcNpcIn_c *npc) {
    return getEventState(npc, FMARKET);
}
// .text 0xA20
void State_c::selectLocal(dAcNpcIn_c *npc) {
    State_e state = COUNT;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player && !player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB) &&
        !dEvent::isActive(EVENT_COUNTDOWN) &&
        !dEvent::isActive(EVENT_HALLOWEEN) && npc->isSick()) {
        state = SICK;
    }
    if (state == COUNT) {
        state = (State_e)getLocalEventState(npc);
    }
    if (state == COUNT) {
        state = IDLE;
    }
    mSelectedState = state;
}
// .text 0xAE0
u32 State_c::getRemoteEventState(dAcNpcIn_c *npc) {
    return getEventState(npc, FMARKET_REMOTE);
}
// .text 0xAE8
void State_c::selectRemote(dAcNpcIn_c *npc) {
    State_e state = (State_e)getRemoteEventState(npc);
    if (state == COUNT) {
        state = REMOTE;
    }
    mSelectedState = state;
}
// .text 0xB24
void State_c::enterSelected(dAcNpcIn_c *npc) {
    change(npc, mSelectedState);
}
// .text 0xB2C
void State_c::startTalk(dAcNpcIn_c *npc) {
    u32 state = mTalkState;
    npc->mNmlTalk.start(
        npc, dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT),
        mpTalk);
    if (npc->mFtr != dItem::ITEM_ID_NONE) {
        npc->mNmlTalk.setRoomFtr(npc->mFtr, npc->mFtrX, npc->mFtrZ);
    }
    if (state < COUNT) {
        change(npc, state);
    } else {
        change(npc, TALK);
    }
    mTalkState = COUNT;
}
// .text 0xBDC
void State_c::execute(dAcNpcIn_c *npc) {
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
// .text 0xC38
void State_c::initIdle(dAcNpcIn_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
    npc->activate();
    npc->mMood.mShowEffects = TRUE;
    npc->mObjc.setDefWeight(COLLISION_WEIGHT);
    npc->mCheckGround = TRUE;
    mSelectedState = IDLE;
    setTalk(
        LOOK, TALK,
        dAcNpcNml_c::talk_c::getProcSet(dAcNpcNml_c::talk_c::TALK_PROC_ENTRY));
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    npc->mBusy = FALSE;
    mBlockedTimer = 0;
}
// .text 0xD38
BOOL State_c::avoidObstacles(dAcNpcIn_c *npc) {
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
// .text 0xDF8
BOOL State_c::stopIfBlocked(dAcNpcIn_c *npc) {
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
// .text 0xE98
BOOL State_c::checkMove(dAcNpcIn_c *npc) {
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
// .text 0xF14
void State_c::executeIdle(dAcNpcIn_c *npc) {
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
    if (!checkMove(npc) && npc->mAction.mCanChange &&
        (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT || !mWaitTimer)) {
        mVec3_c pos(npc->mPos);
        if (cM::rndInt(WANDER_ROLL_COUNT) < WANDER_MOVE_ROLL_COUNT &&
            dAcNpc_c::searchPosInRect(&pos, &npc->mPos, cRoomX0, cRoomZ0,
                                      cRoomX1, cRoomZ1, cWanderMinDist,
                                      cWanderMaxDist)) {
            mAng angle(0);
            if (npc->checkTargetAngle(&angle, &pos, &l_frontAngle) &&
                distanceXZ(pos, npc->mPos) > cWanderMinDist) {
                npc->mAction.requestWalk(PRIORITY_IDLE, pos, l_defaultAnmRate, l_walkTurnSpeed, 0,
                    l_moveParamWalk, cNpcMorphFrames);
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
// .text 0x1134
void State_c::initTalk(dAcNpcIn_c *npc) {
    mAng angle = npc->getTalkAngle();
    mAng speed = npc->getTalkTurnSpeed();
    npc->mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    npc->mBusy = FALSE;
}
// .text 0x11A8
void State_c::stepTalk(dAcNpcIn_c *npc) {
    if (!npc->mTalk.isBusy()) {
        npc->mMood.mTimerPending = TRUE;
        dDemo::finishTalk(npc);
        ++mStep;
    }
}
static const State_c::Proc l_talkSteps[] = {&State_c::stepTalk};
// .text 0x1208
void State_c::executeTalk(dAcNpcIn_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_talkSteps[mStep])(npc);
    }
}
// .text 0x1248
void State_c::initLook(dAcNpcIn_c *npc) {
    mAng angle = npc->getAngleYToPlayer(npc->_1B70);
    npc->mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                             cNpcMorphFrames);
}
// .text 0x12A8
void State_c::stepLook(dAcNpcIn_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}
static const State_c::Proc l_lookSteps[] = {&State_c::stepLook};
static const char l_sadEffect[] = "afm_mnp_sad_S";
// .text 0x130C
void State_c::executeLook(dAcNpcIn_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_lookSteps[mStep])(npc);
    }
}
// .text 0x134C
void State_c::initRemote(dAcNpcIn_c *npc) {
    npc->activate();
    npc->mBusy = FALSE;
    npc->mAcceptMoveReq = TRUE;
    npc->mObjc.setDefWeight(COLLISION_WEIGHT);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    setTalk(
        LOOK, TALK,
        dAcNpcNml_c::talk_c::getProcSet(dAcNpcNml_c::talk_c::TALK_PROC_ENTRY));
}
// .text 0x140C
void State_c::executeRemote(dAcNpcIn_c *npc) {
    if (npc->isLocalOwner()) {
        if (npc->isPermitSelf()) {
            npc->fn_80013474();
            startTalk(npc);
        } else if (npc->checkLocalOwner()) {
            npc->fn_800134A4();
            selectLocal(npc);
            enterSelected(npc);
        }
    } else if (npc->isClothChangeReady()) {
        if (npc->isDaubClothChange()) {
            change(npc, CHANGE_CLOTH);
        } else if (npc->isDaubClothChanged()) {
            npc->changeClothModel();
        }
    }
}
// .text 0x14F8
void State_c::initRemoteLook(dAcNpcIn_c *npc) {
    npc->activate();
    npc->mBusy = FALSE;
}
// .text 0x1530
void State_c::executeRemoteLook(dAcNpcIn_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_CHANGE_CLOTH2) {
        if (action->mCanChange) {
            npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep,
                                   l_lookYawStep, cNpcLookRange);
            action->requestWait(PRIORITY_IDLE, cNpcMorphFrames);
        }
    } else if (npc->isClothChangeReady()) {
        if (npc->isDaubClothChange()) {
            dAnimal_c *animal = npc->mpAnimal;
            action->requestChangeCloth2(PRIORITY_IDLE, animal->mCloth,
                                        (int)&animal->mClothDesign);
            npc->mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep,
                                  l_lookYawStep);
        } else if (npc->isDaubClothChanged()) {
            npc->changeClothModel();
        }
    } else if (npc->isLocalOwnerHost() && dNet::isSceneOwner()) {
        npc->fn_800134A4();
        selectLocal(npc);
        enterSelected(npc);
    }
}
// .text 0x1678
void State_c::initFmarket(dAcNpcIn_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
    npc->activate();
    npc->mMood.mShowEffects = TRUE;
    npc->mObjc.setDefWeight(COLLISION_WEIGHT);
    npc->mCheckGround = TRUE;
    mSelectedState = IDLE;
    setTalk(LOOK, TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_FMARKET));
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    npc->mBusy = FALSE;
    mSalePending = FALSE;
    npc->mFtrX = dAcNpcIn_c::INVALID_UNIT_COORD;
    npc->mFtrZ = dAcNpcIn_c::INVALID_UNIT_COORD;
    npc->mFtr.mId = dItem::ITEM_ID_NONE;
    if (dAcNpc_c::isMultiPlay()) {
        dFTR::disableHold();
    } else {
        dFTR::disableHoldOnly();
    }
    mBlockedTimer = 0;
}
// .text 0x17B0
BOOL State_c::checkFmarketMove(dAcNpcIn_c *npc) {
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
// .text 0x182C
void State_c::executeFmarket(dAcNpcIn_c *npc) {
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
    if (mSalePending) {
        if (dDemo::isType(dDemo::TYPE_ANY) || npc->isInUse(NULL) ||
            dPlayerMgr::getChairPosition(NULL,
                                         dPlayerMgr::PLAYER_INDEX_CURRENT) ||
            dPlayerMgr::getBedPosition(NULL,
                                       dPlayerMgr::PLAYER_INDEX_CURRENT)) {
            npc->mBusy = FALSE;
            mSalePending = FALSE;
            setTalk(LOOK, TALK,
                    dAcNpcNml_c::talk_c::getProcSet(
                        dAcNpcNml_c::talk_c::TALK_PROC_FMARKET));
        } else if (dDemo::requestSpeak(npc, NULL)) {
            npc->mBusy = TRUE;
        }
    } else if (npc->mpEntry && npc->mpEntry->mFlags.mFmarketSale) {
        int x = 0, z = 0;
        if (dAcNpc_c::getPointedUnit(&x, &z, &l_pointAngle, 0, l_pointRange,
                                     l_pointDist)) {
            dItem::Item item = dAcNpcNml_c::getRoomFtrAt(&x, &z, x, z);
            if (item.mId != dItem::ITEM_ID_NONE &&
                !dDemo::isType(dDemo::TYPE_ANY) && !npc->isInUse(NULL) &&
                !dPlayerMgr::getChairPosition(
                    NULL, dPlayerMgr::PLAYER_INDEX_CURRENT) &&
                !dPlayerMgr::getBedPosition(NULL,
                                            dPlayerMgr::PLAYER_INDEX_CURRENT) &&
                dDemo::requestSpeak(npc, NULL)) {
                setTalk(LOOK, TALK,
                        dAcNpcNml_c::talk_c::getProcSet(
                            dAcNpcNml_c::talk_c::TALK_PROC_SALE));
                npc->mFtr = item;
                npc->mFtrX = x;
                npc->mFtrZ = z;
                npc->mBusy = TRUE;
                mSalePending = TRUE;
                npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
            }
        }
    }
    if (!mSalePending && !checkFmarketMove(npc) && npc->mAction.mCanChange &&
        (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT || !mWaitTimer)) {
        mVec3_c pos(npc->mPos);
        if (cM::rndInt(WANDER_ROLL_COUNT) < WANDER_MOVE_ROLL_COUNT &&
            dAcNpc_c::searchPosInRect(&pos, &npc->mPos, cRoomX0, cRoomZ0,
                                      cRoomX1, cRoomZ1, cWanderMinDist,
                                      cWanderMaxDist)) {
            mAng angle(0);
            if (npc->checkTargetAngle(&angle, &pos, &l_frontAngle) &&
                distanceXZ(pos, npc->mPos) > cWanderMinDist) {
                npc->mAction.requestWalk(PRIORITY_IDLE, pos, l_defaultAnmRate, l_walkTurnSpeed, 0,
                    l_moveParamWalk, cNpcMorphFrames);
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
// .text 0x1C28
void State_c::initFmarketRemote(dAcNpcIn_c *npc) {
    npc->activate();
    npc->mBusy = FALSE;
    npc->mAcceptMoveReq = TRUE;
    npc->mObjc.setDefWeight(COLLISION_WEIGHT);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    mSalePending = FALSE;
    setTalk(LOOK, TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_FMARKET));
    npc->mFtrX = dAcNpcIn_c::INVALID_UNIT_COORD;
    npc->mFtrZ = dAcNpcIn_c::INVALID_UNIT_COORD;
    npc->mFtr.mId = dItem::ITEM_ID_NONE;
    if (dAcNpc_c::isMultiPlay()) {
        dFTR::disableHold();
    } else {
        dFTR::disableHoldOnly();
    }
}
// .text 0x1D24
void State_c::executeFmarketRemote(dAcNpcIn_c *npc) {
    if (npc->isLocalOwner()) {
        if (npc->isPermitSelf()) {
            npc->fn_80013474();
            startTalk(npc);
        } else if (npc->checkLocalOwner()) {
            npc->fn_800134A4();
            selectLocal(npc);
            enterSelected(npc);
        }
    } else if (mSalePending) {
        int player = dPlayerMgr::PLAYER_INDEX_CURRENT;
        if (npc->isInUse(&player) && player != dNet::getMyNo()) {
            npc->mBusy = FALSE;
            mSalePending = FALSE;
            setTalk(LOOK, TALK,
                    dAcNpcNml_c::talk_c::getProcSet(
                        dAcNpcNml_c::talk_c::TALK_PROC_FMARKET));
        } else if (dDemo::requestSpeak(npc, NULL)) {
            npc->mBusy = TRUE;
        }
    } else if (npc->mpEntry && npc->mpEntry->mFlags.mFmarketSale &&
               dAcNpc_c::isMultiPlay()) {
        int x = 0, z = 0;
        if (dAcNpc_c::getPointedUnit(&x, &z, &l_pointAngle, 0, l_pointRange,
                                     l_pointDist)) {
            dItem::Item item = dAcNpcNml_c::getRoomFtrAt(&x, &z, x, z);
            if (item.mId != dItem::ITEM_ID_NONE &&
                !dDemo::isType(dDemo::TYPE_ANY) && !npc->isInUse(NULL) &&
                dDemo::requestSpeak(npc, NULL)) {
                setTalk(LOOK, TALK,
                        dAcNpcNml_c::talk_c::getProcSet(
                            dAcNpcNml_c::talk_c::TALK_PROC_SALE));
                npc->mFtr = item;
                npc->mFtrX = x;
                npc->mFtrZ = z;
                npc->mBusy = TRUE;
                mSalePending = TRUE;
            }
        }
    }
}
static dAcNpc_c::moveParam_c l_moveParamSick(0.25f, 0.035f, 0.07f);
static dAcNpc_c::moveParam_c l_moveParamVerySick(0.2f, 0.03f, 0.07f);
// .text 0x1F44
void State_c::initSick(dAcNpcIn_c *npc) {
    BOOL sick = FALSE;
    int sickness = npc->getSickness();
    if (sickness == dAcNpcIn_c::SICK_WORSE ||
        sickness == dAcNpcIn_c::SICK_WORST) {
        sick = TRUE;
    }
    dAcNpc_c::anmSet_c *anm = &npc->mAnmSet;
    if (sick) {
        anm->setAnm0(ANM_SAD_WAIT);
        anm->setAnm1(ANM_SAD_WALK);
        memcpy(&npc->mMoveParam, &l_moveParamVerySick, sizeof(npc->mMoveParam));
    } else {
        anm->setAnm0(ANM_WAIT);
        anm->setAnm1(ANM_WALK);
        memcpy(&npc->mMoveParam, &l_moveParamSick, sizeof(npc->mMoveParam));
    }
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cSickWaitBase + cM::rndInt(cSickWaitRange);
    npc->activate();
    npc->mMood.mShowEffects = FALSE;
    npc->mMood.mDisabled = TRUE;
    npc->mObjc.setDefWeight(COLLISION_WEIGHT);
    npc->mCheckGround = TRUE;
    mSelectedState = SICK;
    setTalk(
        LOOK, TALK,
        dAcNpcNml_c::talk_c::getProcSet(dAcNpcNml_c::talk_c::TALK_PROC_ENTRY));
    npc->mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    npc->mBusy = FALSE;
    mBlockedTimer = 0;
}
// .text 0x20C4
void State_c::executeSick(dAcNpcIn_c *npc) {
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
    if (!checkMove(npc)) {
        dAcNpc_c::action_c *action = &npc->mAction;
        if ((action->mCanChange ||
             (action->mActionId == dAcNpc_c::ACTION_EMOTION && !mWaitTimer)) &&
            ((action->mActionId != dAcNpc_c::ACTION_WAIT &&
              action->mActionId != dAcNpc_c::ACTION_EMOTION) ||
             !mWaitTimer)) {
            mVec3_c pos(npc->mPos);
            if (cM::rndInt(WANDER_ROLL_COUNT) < WANDER_MOVE_ROLL_COUNT &&
                dAcNpc_c::searchPosInRect(&pos, &npc->mPos, cRoomX0, cRoomZ0,
                                          cRoomX1, cRoomZ1, cWanderMinDist,
                                          cSickMaxDist)) {
                npc->stopEmotion();
                mAng angle(0);
                if (npc->checkTargetAngle(&angle, &pos, &l_frontAngle) &&
                    distanceXZ(pos, npc->mPos) > cWanderMinDist) {
                    action->requestWalk(PRIORITY_IDLE, pos, l_defaultAnmRate, l_walkTurnSpeed, 0,
                        npc->mMoveParam, cNpcMorphFrames);
                } else {
                    action->requestTurnWalk(PRIORITY_IDLE, pos, angle,
                                            l_walkTurnSpeed, 0, npc->mMoveParam,
                                            l_defaultAnmRate, cNpcMorphFrames);
                }
            } else {
                int sickness = npc->getSickness();
                BOOL sick = FALSE;
                if (npc->isSickness(sickness, dAcNpcIn_c::SICK_WORSE) ||
                    npc->isSickness(sickness, dAcNpcIn_c::SICK_WORST)) {
                    sick = TRUE;
                }
                int chance = EMOTION_ROLL_COUNT;
                if (sick) {
                    chance = SICK_EMOTION_ROLL_COUNT;
                }
                if (action->mActionId != dAcNpc_c::ACTION_EMOTION &&
                    cM::rndInt(chance) == 0) {
                    mWaitTimer = cSickWaitBase + cM::rndInt(cSickWaitRange);
                    if (sickness == dAcNpcIn_c::SICK_WORSE ||
                        sickness >= dAcNpcIn_c::SICK_WORST) {
                        mWaitTimer = cEmotionWaitBase +
                                     cM::rndInt(cSickEmotionWaitRange);
                    } else {
                        mWaitTimer =
                            cEmotionWaitBase + cM::rndInt(cEmotionWaitRange);
                    }
                    action->requestEmotion(PRIORITY_IDLE, EMOTION_SICK,
                                           cNpcMorphFrames);
                } else {
                    mWaitTimer = cSickWaitBase + cM::rndInt(cSickWaitRange);
                    if (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT) {
                        npc->stopEmotion();
                        action->requestWait(PRIORITY_IDLE, cNpcMorphFrames);
                    }
                }
            }
        }
    }
    dAcNpc_c::anm_c *anm = &npc->getModel()->mAnm;
    if (isAnm(anm, ANM_SAD_WAIT) || isAnm(anm, ANM_SAD_WALK)) {
        if ((u8)(int)anm->getFrame() == 0) {
            npc->mAudioObj.startSound(SE_default_manpu_kanashii);
        }
        mEffect.create(l_sadEffect, &npc->mNodeMtx12, 0);
    }
}
// .text 0x2420
void State_c::initChangeCloth(dAcNpcIn_c *npc) {
    npc->activate();
    npc->mAcceptMoveReq = FALSE;
    dAnimal_c *animal = npc->mpAnimal;
    npc->mAction.requestChangeCloth2(PRIORITY_IDLE, animal->mCloth,
                                     (int)&animal->mClothDesign);
    npc->mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    npc->mBusy = TRUE;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = TRUE;
}
// .text 0x24D4
void State_c::stepChangeCloth(dAcNpcIn_c *npc) {
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_CHANGE_CLOTH2 &&
        npc->mAction.mCanChange) {
        selectRemote(npc);
        enterSelected(npc);
    }
}
static const State_c::Proc l_changeClothSteps[] = {&State_c::stepChangeCloth};
// .text 0x2530
void State_c::executeChangeCloth(dAcNpcIn_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_changeClothSteps[mStep])(npc);
    }
}
} // namespace dANpcIn
