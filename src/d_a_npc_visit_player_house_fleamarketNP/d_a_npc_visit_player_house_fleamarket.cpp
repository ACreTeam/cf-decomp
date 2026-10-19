// A villager visiting the player's flea market stall. Functions are in target
// .text order.
#include <game/game/d_a_npc_visit_player_house_fleamarket.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_a_ftr.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/snd/snd_id.hpp>
#include <climits>

namespace dANpcVisitPlayerHouseFleamarket {
// .text 0x5C
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text 0x68
BASE_PROFILE(A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET,
             dAcNpcVisitPlayerHouseFleamarket_c);

static inline float lengthXZ(const mVec3_c &v) {
    return EGG::Mathf::sqrt(v.x * v.x + v.z * v.z);
}
static inline float distanceXZ(const mVec3_c &a, const mVec3_c &b) {
    return lengthXZ(a - b);
}

// Stay, wait and wandering parameters (minutes, frames, units).
extern const int cStayMinutes = 30;
extern const int cWaitBase = 120;
extern const int cWaitRange = 600;
extern const int cOfferWaitBase = 600;
extern const int cOfferWaitRange = 600;
extern const int cCallWaitBase = 1200;
extern const int cCallWaitRange = 600;
extern const float cWanderMaxDist = 128.0f;
extern const float cWanderMinDist = 48.0f;
extern const int cWaitCallInterval = 300;
extern const int cFirstIdleFrameLimit = 1800;
extern const int cIdleFrameLimit = 36000;
extern const float cRoomX0 = 128.0f;
extern const float cRoomZ0 = 192.0f;
extern const float cRoomX1 = 384.0f;
extern const float cRoomZ1 = 448.0f;
} // namespace dANpcVisitPlayerHouseFleamarket

// .text 0x1EC
bool dAcNpcVisitPlayerHouseFleamarket_c::canInteract(dDemoActor_c *actor) {
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

// .text 0x268
void dAcNpcVisitPlayerHouseFleamarket_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        if (mState.mResumeState <
            dANpcVisitPlayerHouseFleamarket::State_c::STATE_COUNT) {
            mState.change(this, mState.mResumeState);
        } else {
            mState.change(this,
                          dANpcVisitPlayerHouseFleamarket::State_c::STATE_LOOK);
        }
        mState.mResumeState =
            dANpcVisitPlayerHouseFleamarket::State_c::STATE_NONE;
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
        if (state < dANpcVisitPlayerHouseFleamarket::State_c::STATE_COUNT) {
            mState.change(this, state);
        } else {
            mState.change(this,
                          dANpcVisitPlayerHouseFleamarket::State_c::STATE_TALK);
        }
        mState.mTalkState =
            dANpcVisitPlayerHouseFleamarket::State_c::STATE_NONE;
        break;
    }
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        mState.change(this,
                      dANpcVisitPlayerHouseFleamarket::State_c::STATE_IDLE);
        break;
    }
    dAcNpc_c::onDemoCmd(cmd, member);
}

// .text 0x3A0
BOOL dAcNpcVisitPlayerHouseFleamarket_c::isStayOver(int secs) const {
    dAnimal_c *animal = mpAnimal;
    if (animal == NULL) {
        return TRUE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player == NULL) {
        return TRUE;
    }
    dAnimalMemory_c *memory = animal->findMemory2(&player->mPID);
    if (memory == NULL) {
        return TRUE;
    }
    if (!memory->isEventFlag(dAnimalMemory_c::EVENT_FLAG_STALL_ARRIVED)) {
        return FALSE;
    }
    dNpc::entry_c *entry = mpEntry;
    if (entry == NULL) {
        return TRUE;
    }
    dTime_c *now = dTime_c::getCurrent();
    dTime_c end = entry->mTime;
    end.add(0, 0, dANpcVisitPlayerHouseFleamarket::cStayMinutes, secs);
    return dTime_c::isSameOrAfter(*now, end) != FALSE;
}

// .text 0x578
void dAcNpcVisitPlayerHouseFleamarket_c::endStaySoon() {
    dNpc::entry_c *entry = mpEntry;
    if (entry != NULL) {
        dTime_c time = *dTime_c::getCurrent();
        time.add(0, 0, -dANpcVisitPlayerHouseFleamarket::cStayMinutes,
                 dANpcVisitPlayerHouseFleamarket::STAY_END_SECS);
        entry->mTime = time;
    }
}

// .text 0x664
BOOL dAcNpcVisitPlayerHouseFleamarket_c::checkStayOver() {
    if (!isStayOver(-dANpcVisitPlayerHouseFleamarket::STAY_END_SECS)) {
        return FALSE;
    }
    endStaySoon();
    return TRUE;
}

// .text 0x6B0
BOOL dAcNpcVisitPlayerHouseFleamarket_c::checkRefusedTooOften() {
    if (isStayOver(-dANpcVisitPlayerHouseFleamarket::STAY_END_SECS)) {
        return FALSE;
    }
    dNpc::entry_c *entry = mpEntry;
    if (entry == NULL || entry->mStallRefuseCount <
                             dANpcVisitPlayerHouseFleamarket::STALL_COUNT_MAX) {
        return FALSE;
    }
    endStaySoon();
    return TRUE;
}

// .text 0x71C
int dAcNpcVisitPlayerHouseFleamarket_c::removeRoomFtr(int handle, int x,
                                                      int z) {
    if (handle != INVALID_FTR_HANDLE && dFtrm_c::canTalk(handle)) {
        dFtrm_c::sendEvent(handle);
        return 1;
    }
    return 0;
}

// .text 0x770
int dAcNpcVisitPlayerHouseFleamarket_c::preCreate() {
    return dAcNpcNml_c::preCreate() != FALSE;
}

// .text 0x79C
int dAcNpcVisitPlayerHouseFleamarket_c::create() {
    if (!dAcNpcNml_c::create()) {
        return 0;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dAnimalMemory_c *memory = mpAnimal != NULL && player != NULL
                                  ? mpAnimal->findMemory(&player->mPID)
                                  : NULL;
    BOOL arrived =
        memory != NULL
            ? memory->isEventFlag(dAnimalMemory_c::EVENT_FLAG_STALL_ARRIVED)
            : FALSE;
    BOOL looked =
        memory != NULL
            ? memory->isEventFlag(dAnimalMemory_c::EVENT_FLAG_STALL_LOOKED)
            : FALSE;
    dTime_c *now = dTime_c::getCurrent();
    if (arrived && !looked && isStayOver(0)) {
        if (memory != NULL) {
            memory->onEventFlag(dAnimalMemory_c::EVENT_FLAG_STALL_LOOKED);
        }
        looked = TRUE;
    }
    mState.mIdleFrameLimit =
        dANpcVisitPlayerHouseFleamarket::cFirstIdleFrameLimit;
    if (looked) {
        mState.change(this,
                      dANpcVisitPlayerHouseFleamarket::State_c::STATE_GONE);
    } else if (arrived) {
        mState.change(this,
                      dANpcVisitPlayerHouseFleamarket::State_c::STATE_IDLE);
        dBGCF::lockNext(0);
    } else {
        mState.change(this,
                      dANpcVisitPlayerHouseFleamarket::State_c::STATE_WAIT);
    }
    mFtrX = INVALID_UNIT_COORD;
    mFtrZ = INVALID_UNIT_COORD;
    mOfferUnitX = INVALID_UNIT_COORD;
    mOfferUnitZ = INVALID_UNIT_COORD;
    mFtr.mId = dItem::ITEM_ID_NONE;
    return 1;
}

// .text 0x930
int dAcNpcVisitPlayerHouseFleamarket_c::doDelete() {
    if (!dAcNpcNml_c::doDelete()) {
        return 0;
    }
    dSTR::clearPartyFlag();
    return 1;
}

// .text 0x968
int dAcNpcVisitPlayerHouseFleamarket_c::vtB0() {
    mState.execute(this);
    return 1;
}

namespace dANpcVisitPlayerHouseFleamarket {
// .text 0x994
State_c::State_c() : mTimer(0), mWaitTimer(0), mBlockedTimer(0) {
    mState = STATE_NONE;
    mResumeState = STATE_NONE;
    mTalkState = STATE_NONE;
    mpState = NULL;
    mStep = STEP_START;
    mTimer = 0;
    mWaitTimer = 0;
    mLookTimer = 0;
    mBlockedTimer = 0;
    mpTalk = NULL;
    mIdleFrames = 0;
}

// .text 0x9CC
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
    {&State_c::initOffer, &State_c::executeOffer},
};

// .text 0xA0C
void State_c::change(dAcNpcVisitPlayerHouseFleamarket_c *npc, u32 state) {
    if (state < STATE_COUNT) {
        mState = state;
        mpState = &l_states[state];
        mStep = STEP_START;
        if (mpState != NULL) {
            (this->*mpState->mInit)(npc);
        }
    }
}

// .text 0xA5C
void State_c::execute(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (mTimer) {
        --mTimer;
    }
    if (mWaitTimer) {
        --mWaitTimer;
    }
    if (mIdleFrames < ULONG_MAX) {
        ++mIdleFrames;
    }
    if (mpState != NULL && mpState->mExecute) {
        (this->*mpState->mExecute)(npc);
    }
}

// .text 0xB04
void State_c::setTalk(u32 resume, u32 talk,
                      const dAcNpcNml_c::talk_c::procSet_s *set) {
    mResumeState = resume;
    mTalkState = talk;
    mpTalk = set;
}

// .text 0xB14
void State_c::initIdle(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
    if (!(cM::rndInt(LOOK_ROLL_COUNT) & 1)) {
        npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep,
                               l_lookYawStep, cNpcLookRange);
    } else {
        npc->mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    }
    mLookTimer = cWaitBase + cM::rndF(cWaitRange);
    npc->mIsActive = TRUE;
    npc->mCheckGround = TRUE;
    dSTR::setPartyFlag();
    setTalk(STATE_LOOK, STATE_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_STALL_NO_ITEM));
    mBlockedTimer = 0;
}

// .text 0xC20
BOOL State_c::avoidObstacles(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
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

// .text 0xCE0
BOOL State_c::checkPlayerLeaving(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    mVec3_c *pos = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (pos != NULL && dBGCF::isNextLocked(pos) &&
        !dDemo::isType(dDemo::TYPE_ANY) && dDemo::requestSpeak(npc, NULL)) {
        change(npc, STATE_PLAYER_LEAVING);
        return TRUE;
    }
    return FALSE;
}

// .text 0xD70
BOOL State_c::checkGoHome(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if ((npc->isStayOver(0) || mIdleFrames > mIdleFrameLimit) &&
        !dDemo::isType(dDemo::TYPE_ANY) && dDemo::requestSpeak(npc, NULL)) {
        change(npc, STATE_GO_HOME);
        return TRUE;
    }
    return FALSE;
}

// .text 0xE08
BOOL State_c::stopIfBlocked(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
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

// .text 0xEA8
BOOL State_c::checkOffer(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    static const int unitOfs[4][2] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};
    static const u32 cUnitOfsNum = 4;
    dNpc::entry_c *entry = npc->mpEntry;
    if (entry == NULL || entry->mStallBuyCount >= STALL_COUNT_MAX ||
        entry->mStallRefuseCount >= STALL_COUNT_MAX) {
        return FALSE;
    }
    if (0.0f == npc->getSpeedF()) {
        int unitX = worldToUnit(npc->mPos.x);
        int unitZ = worldToUnit(npc->mPos.z);
        dItem::Item offer;
        int offerX = dAcNpcVisitPlayerHouseFleamarket_c::INVALID_UNIT_COORD;
        int offerZ = dAcNpcVisitPlayerHouseFleamarket_c::INVALID_UNIT_COORD;
        u32 count = 0;
        for (u32 i = 0; i < cUnitOfsNum; i++) {
            int x = unitX + unitOfs[i][0];
            int z = unitZ + unitOfs[i][1];
            if (x == npc->mFtrX || z == npc->mFtrZ) {
                continue;
            }
            mVec3_c pos = mVec3_c::Zero;
            dFdBase_c::getUnitCenterPos(&pos, x, z);
            dItem::Item item = dAcNpcNml_c::getRoomFtrAtPos(&x, &z, &pos);
            if (item == dItem::ITEM_ID_NONE) {
                continue;
            }
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
            if (bitm == NULL || bitm->getFtrFunc() == dItem::FTR_FUNC_NONE) {
                continue;
            }
            if (item.getPrice() <= 0 || bitm->m_noPurchase) {
                continue;
            }
            dFTR::alwaysAc_c *ftr = dFTR::getUnitMap()->get(&pos, 0);
            BOOL same = FALSE;
            if (ftr != NULL) {
                if (ftr->getItem().isSame(item)) {
                    same = TRUE;
                }
            }
            if (!same) {
                continue;
            }
            int handle = dFTR::getUnitMap()->getHandle(x, z, 0);
            if (handle ==
                    dAcNpcVisitPlayerHouseFleamarket_c::INVALID_FTR_HANDLE ||
                !dFtrm_c::canTalk(handle)) {
                continue;
            }
            f32 chance = OFFER_CHANCE_TOTAL / (count + 1);
            if (cM::rndF(OFFER_CHANCE_TOTAL) <= chance) {
                offer = item;
                offerX = x;
                offerZ = z;
            }
            count++;
        }
        if (offer != dItem::ITEM_ID_NONE) {
            npc->mFtr = offer;
            npc->mFtrX = offerX;
            npc->mFtrZ = offerZ;
            npc->mOfferUnitX = unitX;
            npc->mOfferUnitZ = unitZ;
            change(npc, STATE_OFFER);
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x1184
BOOL State_c::checkIdle(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (checkPlayerLeaving(npc)) {
        return TRUE;
    }
    if (checkGoHome(npc)) {
        return TRUE;
    }
    if (checkOffer(npc)) {
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

// .text 0x1254
void State_c::executeIdle(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
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
        mLookTimer = cWaitBase + cM::rndF(cWaitRange);
    }
}

// .text 0x1588
void State_c::initWait(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    npc->mIsActive = FALSE;
    mTimer = cWaitCallInterval;
}

// .text 0x15E0
void State_c::executeWait(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (mTimer == 0) {
        if (cM::rndInt(CALL_ROLL_COUNT) == 0) {
            change(npc, STATE_CALL);
        } else {
            mTimer = cWaitCallInterval;
        }
    }
}

// .text 0x164C
void State_c::initPlayerLeaving(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    npc->mIsActive = TRUE;
    npc->mCheckGround = TRUE;
    dSTR::setPartyFlag();
    setTalk(STATE_LOOK, STATE_SEE_OFF,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_STALL_WAIT));
}

// .text 0x1700
void State_c::executePlayerLeaving(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    mVec3_c *pos = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (pos != NULL && !dBGCF::isNextLocked(pos)) {
        change(npc, STATE_IDLE);
    } else {
        dDemo::requestSpeak(npc, NULL);
    }
}

// .text 0x1770
void State_c::initGoHome(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    npc->mIsActive = TRUE;
    npc->mCheckGround = TRUE;
    dSTR::setPartyFlag();
    setTalk(STATE_LOOK, STATE_LEAVE,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_STALL_BACK));
}

// .text 0x1824
void State_c::executeGoHome(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    dDemo::requestSpeak(npc, NULL);
}

// .text 0x1830
void State_c::initGone(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    npc->mIsActive = FALSE;
}

// .text 0x1874
void State_c::initCall(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mTimer = cM::rndInt(cCallWaitRange) + cCallWaitBase;
    npc->mIsActive = FALSE;
}

// .text 0x18D4
void State_c::executeCall(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (mTimer == 0 && isPlayerAwayFromDoor() &&
        dDemo::requestSpeak(npc, NULL)) {
        setTalk(STATE_GONE, STATE_ARRIVE,
                dAcNpcNml_c::talk_c::getProcSet(
                    dAcNpcNml_c::talk_c::TALK_PROC_STALL_CALL));
    }
}

// .text 0x1950
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

// .text 0x19E8
void State_c::initArrive(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    npc->mIsActive = FALSE;
    npc->mCheckGround = FALSE;
    npc->mAudioObj.startSound(SE_room_DoorOpenCloseWithKnock);
    mTimer = KNOCK_FRAMES;
}

// .text 0x1A70
void State_c::stepArriveKnock(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (mTimer == 0) {
        npc->mTalk.requestTalk(true, TRUE);
        mStep = ARRIVE_STEP_TALK;
    } else if (mTimer == KNOCK_ROOM_SE_FRAME) {
        dSTR::playRoomSe();
    }
}

// .text 0x1ACC
void State_c::stepArriveTalk(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (!npc->mTalk.isBusy()) {
        npc->mCheckGround = TRUE;
        dDemo::finishTalk(npc);
        dBGCF::lockNext(0);
        dCamera_c::sInstance->mRequestedMode =
            dCamera_c::sInstance->mDefaultMode;
        dNpc::entry_c *entry = npc->mpEntry;
        if (entry != NULL) {
            entry->mTime = *dTime_c::getCurrent();
            entry->mStallBuyCount = 0;
            entry->mStallRefuseCount = 0;
        }
        mIdleFrames = 0;
        ++mStep;
    }
}

static const State_c::Proc l_arriveSteps[] = {&State_c::stepArriveKnock,
                                              &State_c::stepArriveTalk};

// .text 0x1BB8
void State_c::executeArrive(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (mStep < ARRIVE_STEP_DONE) {
        (this->*l_arriveSteps[mStep])(npc);
    }
}

// .text 0x1BF8
void State_c::initTalk(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    u16 angleValue = 0;
    dActor_c *actor = npc->mNmlTalk.mpActActor;
    if (actor) {
        angleValue = npc->getAngleYTo(actor).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, FALSE);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
}

// .text 0x1C8C
void State_c::stepTalk(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (!npc->mTalk.isBusy()) {
        dDemo::finishTalk(npc);
        npc->checkStayOver();
        npc->checkRefusedTooOften();
        mIdleFrames = 0;
        mIdleFrameLimit = cIdleFrameLimit;
        ++mStep;
    }
}

static const State_c::Proc l_talkSteps[] = {&State_c::stepTalk};

// .text 0x1D08
void State_c::executeTalk(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_talkSteps[mStep])(npc);
    }
}

// .text 0x1D48
void State_c::initLook(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    mAng angle = npc->getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
    npc->mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                             cNpcMorphFrames);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
}

// .text 0x1DD0
void State_c::stepLook(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

static const State_c::Proc l_lookSteps[] = {&State_c::stepLook};

// .text 0x1E34
void State_c::executeLook(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_lookSteps[mStep])(npc);
    }
}

// .text 0x1E74
void State_c::initSeeOff(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    u16 angleValue = 0;
    dActor_c *actor = npc->mNmlTalk.mpActActor;
    if (actor) {
        angleValue = npc->getAngleYTo(actor).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, FALSE);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
}

// .text 0x1F08
void State_c::stepSeeOff(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (!npc->mTalk.isBusy()) {
        getSceneChange()->requestExit(EXIT_FRONT_DOOR, EXIT_WIPE, EXIT_WIPE);
        dSTR::clearPartyFlag();
        ++mStep;
    }
}

static const State_c::Proc l_seeOffSteps[] = {&State_c::stepSeeOff};

// .text 0x1F64
void State_c::executeSeeOff(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_seeOffSteps[mStep])(npc);
    }
}

// .text 0x1FA4
void State_c::initLeave(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    u16 angleValue = 0;
    dActor_c *actor = npc->mNmlTalk.mpActActor;
    if (actor) {
        angleValue = npc->getAngleYTo(actor).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, FALSE);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
}

// .text 0x2038
void State_c::stepLeave(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (!npc->mTalk.isBusy()) {
        getSceneChange()->setReturnExitHere(0.0f);
        getSceneChange()->requestExit(EXIT_RETURN, EXIT_WIPE, EXIT_WIPE);
        dSTR::clearPartyFlag();
        ++mStep;
    }
}

static const State_c::Proc l_leaveSteps[] = {&State_c::stepLeave};

// .text 0x20A4
void State_c::executeLeave(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_leaveSteps[mStep])(npc);
    }
}

// .text 0x20E4
void State_c::initOffer(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    mVec3_c pos = mVec3_c::Zero;
    dFdBase_c::getUnitCenterPos(&pos, npc->mFtrX, npc->mFtrZ);
    mAng angle = dActor_c::targetAngleY(&npc->mPos, &pos);
    if (npc->mAngle.y != angle) {
        npc->mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0,
                                 cNpcMorphFrames);
    } else {
        npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    }
    npc->mIsActive = TRUE;
    npc->mCheckGround = TRUE;
    dSTR::setPartyFlag();
    setTalk(STATE_LOOK, STATE_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_STALL_ITEM));
    mBlockedTimer = 0;
    mTimer = cM::rndInt(cOfferWaitRange) + cOfferWaitBase;
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
}

// .text 0x2210
void State_c::executeOffer(dAcNpcVisitPlayerHouseFleamarket_c *npc) {
    int unitX = worldToUnit(npc->mPos.x);
    int unitZ = worldToUnit(npc->mPos.z);
    if (unitX != npc->mOfferUnitX || unitZ != npc->mOfferUnitZ) {
        change(npc, STATE_IDLE);
        return;
    }
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN) {
        if (action->mCanChange) {
            action->requestWait(PRIORITY_IDLE, cNpcMorphFrames);
            mTimer = cM::rndInt(cOfferWaitRange) + cOfferWaitBase;
        }
    } else if (action->mActionId == dAcNpc_c::ACTION_WAIT && mTimer == 0) {
        change(npc, STATE_IDLE);
    }
}
} // namespace dANpcVisitPlayerHouseFleamarket
