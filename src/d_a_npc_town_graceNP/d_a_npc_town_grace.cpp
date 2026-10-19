// The guest villager in Gracie's shop. Functions are in target .text order.
#include <game/game/d_a_npc_town_grace.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_player_mgr.hpp>
#include <climits>

#define GRACE_LOOK_WAIT_MIN                                                    \
    120.0f // frames until the next look-around switch: at least ...
#define GRACE_LOOK_WAIT_RAND 600.0f // ... plus up to this many

namespace dANpcTownGrace {
// .text 0x5C
BASE_PROFILE(A_NPC_TOWN_GRACE, dAcNpcTownGrace_c);
} // namespace dANpcTownGrace

// .text 0x1D8
bool dAcNpcTownGrace_c::canInteract(dDemoActor_c *actor) {
    if (!dAcNpcNml_c::canInteract(actor)) {
        return false;
    }
    if (actor == NULL) {
        return false;
    }
    return !mTalk.isBusy();
}

// .text 0x240
void dAcNpcTownGrace_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        mState.change(this, dANpcTownGrace::State_c::STATE_LOOK);
        break;
    case DEMO_CMD_TALK: {
        const talk_c::procSet_s *set =
            talk_c::getProcSet(talk_c::TALK_PROC_TOWN_GRACE);
        mNmlTalk.start(
            this,
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT),
            set);
        mState.change(this, dANpcTownGrace::State_c::STATE_TALK);
        break;
    }
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        mState.change(this, dANpcTownGrace::State_c::STATE_IDLE);
        break;
    }
    dAcNpc_c::onDemoCmd(cmd, member);
}

// .text 0x318
int dAcNpcTownGrace_c::preCreate() {
    return dAcNpcTown_c::preCreate() != 0;
}

// .text 0x344
int dAcNpcTownGrace_c::create() {
    if (!dAcNpcNml_c::create()) {
        return 0;
    }
    mLookAt.setPlayer(dANpcTownGrace::PRIORITY_IDLE, true, l_lookPitchStep,
                      l_lookYawStep, cNpcLookRange);
    mState.change(this, dANpcTownGrace::State_c::STATE_IDLE);
    return 1;
}

// .text 0x3BC
int dAcNpcTownGrace_c::doDelete() {
    return dAcNpcNml_c::doDelete() != 0;
}

// .text 0x3E8
int dAcNpcTownGrace_c::vtB0() {
    mState.execute(this);
    return 1;
}

namespace dANpcTownGrace {
// .text 0x414
State_c::State_c() : mWaitTimer(0), mLookTimer(0), mBlockedTimer(0) {
    mState = STATE_NONE;
    mpState = NULL;
    mStep = STEP_START;
}

// .text 0x438
State_c::~State_c() {
}

static const State_c::State_s l_states[] = {
    {&State_c::initIdle, &State_c::executeIdle},
    {&State_c::initTalk, &State_c::executeTalk},
    {&State_c::initLook, &State_c::executeLook},
};

// Idle wander timing (frames).
extern const int cWaitBase = 120;
extern const int cWaitRange = 180;

// .text 0x478
void State_c::change(dAcNpcTownGrace_c *npc, u32 state) {
    if (state < STATE_COUNT) {
        mState = state;
        mpState = &l_states[state];
        mStep = STEP_START;
        if (mpState != NULL) {
            (this->*mpState->mInit)(npc);
        }
    }
}

// .text 0x4C8
void State_c::execute(dAcNpcTownGrace_c *npc) {
    if (mWaitTimer) {
        --mWaitTimer;
    }
    if (mpState != NULL) {
        (this->*mpState->mExecute)(npc);
    }
}

// .text 0x510
void State_c::initIdle(dAcNpcTownGrace_c *npc) {
    mWaitTimer = cWaitBase + cM::rndInt(cWaitRange);
    npc->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT);
    mLookTimer = GRACE_LOOK_WAIT_MIN + cM::rndF(GRACE_LOOK_WAIT_RAND);
    npc->mBusy = FALSE;
}

// .text 0x59C
BOOL State_c::avoidObstacles(dAcNpcTownGrace_c *npc) {
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

// .text 0x65C
BOOL State_c::stopIfBlocked(dAcNpcTownGrace_c *npc) {
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

// .text 0x6FC
BOOL State_c::checkMove(dAcNpcTownGrace_c *npc) {
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

// .text 0x778
void State_c::executeIdle(dAcNpcTownGrace_c *npc) {
    if (0.0f != npc->getSpeedF()) {
        if ((npc->mAcch.mPrevHitFlags &
             (dBGCF::HIT_WALL_FRONT | dBGCF::HIT_STEP_EXT_FRONT)) ||
            (npc->mAcch.mHitFlags &
             (dBGCF::HIT_WALL_FRONT | dBGCF::HIT_STEP_EXT_FRONT))) {
            if (mBlockedTimer < ULONG_MAX) {
                ++mBlockedTimer;
            }
        } else {
            mBlockedTimer = 0;
        }
    } else {
        mBlockedTimer = 0;
    }
    if (checkMove(npc)) {
        return;
    }
    dAcNpc_c::action_c *action = &npc->mAction;
    const mVec3_c *npcPos;
    int yawStep;
    if (action->mCanChange &&
        (action->mActionId != dAcNpc_c::ACTION_WAIT || !mWaitTimer)) {
        npcPos = &npc->mPos;
        mVec3_c pos(*npcPos);
        mAng sideAngle(0);
        BOOL moved = FALSE;
        if (cM::rndInt(WANDER_ROLL_COUNT) < WANDER_MOVE_ROLL_COUNT) {
            if (dAcNpc_c::searchPosNear(&pos, npcPos, l_8074FE14,
                                        l_8074FE10)) {
                mAng angle(0);
                if (npc->checkTargetAngle(&angle, &pos, &l_frontAngle)) {
                    action->requestWalk(PRIORITY_IDLE, pos, l_defaultAnmRate,
                                        l_walkTurnSpeed, 0, l_moveParamWalk,
                                        cNpcMorphFrames);
                    moved = TRUE;
                } else {
                    action->requestTurnWalk(PRIORITY_IDLE, pos, angle,
                                            l_turnSpeed, 0, l_moveParamWalk,
                                            l_defaultAnmRate, cNpcMorphFrames);
                    moved = TRUE;
                }
            } else if (dAcNpc_c::searchFreeSideAngle(&sideAngle, npcPos, 0)) {
                action->requestTurn(PRIORITY_IDLE, sideAngle, l_turnSpeed, 0,
                                    cNpcMorphFrames);
                moved = TRUE;
            }
        }
        if (!moved) {
            mWaitTimer = cWaitBase + (u32)cM::rndF(cWaitRange);
            if (action->mActionId != dAcNpc_c::ACTION_WAIT) {
                action->requestWait(PRIORITY_IDLE, cNpcMorphFrames);
            }
        }
    }
    if (mLookTimer) {
        --mLookTimer;
    }
    if (mLookTimer == 0) {
        dAcNpc_c::lookAt_c *look = &npc->mLookAt;
        if (look->mType != dAcNpc_c::lookAt_c::LOOK_AROUND) {
            yawStep = l_lookYawStep;
            npc->mLookAt.setAround(PRIORITY_IDLE, mAng(0),
                                   mAng(l_lookYawMax >> 1), l_lookPitchStep,
                                   mAng((yawStep >> 1) + cM::rndInt(yawStep)));
        } else {
            look->setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                            cNpcLookRange);
        }
        mLookTimer = GRACE_LOOK_WAIT_MIN + cM::rndF(GRACE_LOOK_WAIT_RAND);
    }
}

// .text 0xA84
void State_c::initTalk(dAcNpcTownGrace_c *npc) {
    mAng angle = npc->getTalkAngle();
    mAng speed = npc->getTalkTurnSpeed();
    npc->mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    npc->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
}

// .text 0xB18
void State_c::stepTalk(dAcNpcTownGrace_c *npc) {
    if (!npc->mTalk.isBusy()) {
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

static const State_c::Proc l_talkSteps[] = {&State_c::stepTalk};

// .text 0xB70
void State_c::executeTalk(dAcNpcTownGrace_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_talkSteps[mStep])(npc);
    }
}

// .text 0xBB0
void State_c::initLook(dAcNpcTownGrace_c *npc) {
    mAng angle = npc->getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
    npc->mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                             cNpcMorphFrames);
}

// .text 0xC10
void State_c::stepLook(dAcNpcTownGrace_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

static const State_c::Proc l_lookSteps[] = {&State_c::stepLook};

// .text 0xC74
void State_c::executeLook(dAcNpcTownGrace_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_lookSteps[mStep])(npc);
    }
}
} // namespace dANpcTownGrace
