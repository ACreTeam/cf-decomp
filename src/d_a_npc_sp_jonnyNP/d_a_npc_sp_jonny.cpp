#include <game/game/d_a_npc_sp_jonny.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <lib/egg/math/eggMath.h>

// Gulliver on the beach. Functions are in target .text order.

namespace dANpcSpJonny {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_JONNY, dAcNpcSpJonny_c);
} // namespace dANpcSpJonny

// .text 0x1CC
void dAcNpcSpJonny_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeState(STATE_LOOK);
        }
        break;
    case DEMO_CMD_TALK:
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mJonnyTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            changeStateIdle();
        } else if (permit == PERMIT_END_MULTI) {
            changeState(STATE_REMOTE);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            changeStateIdle();
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpJonny {
static const dAcNpcSpJonny_c::State_s l_states[] = {
    {&dAcNpcSpJonny_c::initSleep, &dAcNpcSpJonny_c::executeSleep},
    {&dAcNpcSpJonny_c::initRoam, &dAcNpcSpJonny_c::executeRoam},
    {&dAcNpcSpJonny_c::initLook, &dAcNpcSpJonny_c::executeLook},
    {&dAcNpcSpJonny_c::initTalk, &dAcNpcSpJonny_c::executeTalk},
    {&dAcNpcSpJonny_c::initRemote, &dAcNpcSpJonny_c::executeRemote},
    {&dAcNpcSpJonny_c::initRemoteTalk, &dAcNpcSpJonny_c::executeRemoteTalk},
};
} // namespace dANpcSpJonny

using namespace dANpcSpJonny;

// .text 0x320
void dAcNpcSpJonny_c::changeState(int state) {
    mState = state;
    (this->*l_states[state].mInit)();
}

// .text 0x358
int dAcNpcSpJonny_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mJonnyTalk);
    return 1;
}

// .text 0x3A4
int dAcNpcSpJonny_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    if (dSaveData_c::getTown()->isFlag(SAVE_FLAG_GULLIVER_AWAKE)) {
        mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
        mAwake = 1;
    } else {
        mObjc.setDefWeight(objc_c::WEIGHT_IMMOVABLE);
        mAwake = 0;
        mAnmSet.setAnm0(HMN_ANM_SEG_GETUP1);
        moveFtrToPoliceBox();
    }
    mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    if (isLocalOwner()) {
        changeStateIdle();
    } else {
        changeState(STATE_REMOTE);
    }
    // His block: 16 units of 32 (512) each.
    mBlockX = (int)mPos.x >> 9;
    mBlockZ = (int)mPos.z >> 9;
    dSaveData_c::getTown()->setFlag(SAVE_FLAG_UFO_STR_DRAW);
    return 1;
}

// .text 0x4D8
int dAcNpcSpJonny_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x504
int dAcNpcSpJonny_c::vtB0() {
    dSaveData_c::getTown()->setFlag(SAVE_FLAG_UFO_STR_DRAW);
    if (mState < STATE_NUM) {
        (this->*l_states[mState].mExecute)();
    }
    if (!mAwake && dSaveData_c::getTown()->isFlag(SAVE_FLAG_GULLIVER_AWAKE)) {
        // Woken up on another console.
        mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_SEG_GETUP2, 0.0f,
                           cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
        mAwake = 1;
        mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    }
    return 1;
}

// .text 0x5D0
void dAcNpcSpJonny_c::initSleep() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0x5E4
void dAcNpcSpJonny_c::executeSleep() {
}

// .text 0x5E8
void dAcNpcSpJonny_c::initRoam() {
    dAcNpcSp_c::initWander();
}

// .text 0x5EC
void dAcNpcSpJonny_c::executeRoam() {
    if (mWaitTimer != 0) {
        mWaitTimer--;
    }
    if (checkStuck()) {
        return;
    }
    if (!mAction.mCanChange) {
        return;
    }
    int blockX = (int)mPos.x >> 9;
    int blockZ = (int)mPos.z >> 9;
    if (mAction.mActionId == ACTION_WAIT && mWaitTimer != 0 &&
        blockX == mBlockX && blockZ == mBlockZ) {
        return;
    }
    if ((int)cM::rndF(3.0f) == 0 || blockX != mBlockX || blockZ != mBlockZ) {
        mVec3_c pos = getWanderPos();
        const mVec3_c *cur = &mPos;
        if (pos != *cur) {
            mAng angle = dActor_c::targetAngleY(cur, &pos);
            if (angle.labs() < 0x4000) {
                mAction.requestWalk(PRIORITY_IDLE, pos, l_defaultAnmRate,
                                    l_walkTurnSpeed, 0, l_moveParamWalk,
                                    cNpcMorphFrames);
            } else {
                mAction.requestTurnWalk(PRIORITY_IDLE, pos, angle, l_turnSpeed,
                                        0, l_moveParamWalk, l_defaultAnmRate,
                                        cNpcMorphFrames);
            }
        } else {
            u32 random = (u32)cM::rndF((u32)getWanderWaitRand());
            int base = getWanderWaitBase();
            int action = mAction.mActionId;
            mWaitTimer = base + random;
            if (action != ACTION_WAIT) {
                mAction.requestWait(PRIORITY_IDLE);
            }
        }
    } else {
        u32 random = (u32)cM::rndF((u32)getWanderWaitRand());
        int base = getWanderWaitBase();
        int action = mAction.mActionId;
        mWaitTimer = base + random;
        if (action != ACTION_WAIT) {
            mAction.requestWait(PRIORITY_IDLE);
        }
    }
}

// .text 0x8B8
mVec3_c dAcNpcSpJonny_c::getWanderPos() {
    BOOL ok;
    int baseX = mBlockX * UT_X_NUM;
    int baseZ = mBlockZ * UT_Z_NUM;
    int unitX = baseX + cM::rndInt(UT_X_NUM);
    int unitZ = baseZ + cM::rndInt(UT_Z_NUM);
    mVec3_c center;
    dFdBase_c::getUnitCenterPos(&center, unitX, unitZ);
    mVec3_c diff = center - mPos;
    f32 dist = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z);
    ok = FALSE;
    if (dist >= 64.0f) {
        dItem::Item none;
        if ((u32)dNpc::canPutItemOnUnit(unitX, unitZ, &none, NULL,
                                        dScene::SCENE_NUM, TRUE,
                                        TRUE) == TRUE) {
            ok = TRUE;
        }
    }
    if (ok) {
        return center;
    }
    return mPos;
}

// .text 0x9D8
void dAcNpcSpJonny_c::initLook() {
    if (mAwake) {
        mAng angle = getAngleYToPlayer(_1B70);
        mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                            cNpcMorphFrames);
        mStep = STEP_START;
    } else {
        mStep = STEP_DONE;
    }
}

// .text 0xA54
void dAcNpcSpJonny_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

namespace dANpcSpJonny {
static const dAcNpcSpJonny_c::Proc l_lookSteps[] = {&dAcNpcSpJonny_c::stepLook};
} // namespace dANpcSpJonny

// .text 0xAB4
void dAcNpcSpJonny_c::executeLook() {
    if (mStep < STEP_DONE) {
        (this->*l_lookSteps[mStep])();
    }
}

// .text 0xAF4
void dAcNpcSpJonny_c::initTalk() {
    if (mAwake) {
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    } else {
        mTalk.requestTalk(true, PRIORITY_IDLE);
    }
    mStep = STEP_START;
}

// .text 0xB80
void dAcNpcSpJonny_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        ++mStep;
    }
}

namespace dANpcSpJonny {
static const dAcNpcSpJonny_c::Proc l_talkSteps[] = {&dAcNpcSpJonny_c::stepTalk};
} // namespace dANpcSpJonny

// .text 0xBCC
void dAcNpcSpJonny_c::executeTalk() {
    if (mStep < STEP_DONE) {
        (this->*l_talkSteps[mStep])();
    }
}

// .text 0xC0C
void dAcNpcSpJonny_c::initRemote() {
    resetLookTimer();
}

// .text 0xC14
void dAcNpcSpJonny_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mJonnyTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeStateIdle();
        }
    }
    executeRemoteWander(NULL);
}

// .text 0xCD0
void dAcNpcSpJonny_c::initRemoteTalk() {
}

// .text 0xCD4
void dAcNpcSpJonny_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeStateIdle();
    }
}

// .text 0xD44
dAcNpcSpJonny_c::talk_c::talk_c(dAcNpcSpJonny_c *npc) {
    mpOwner = npc;
    init();
}

// .text 0xDA4
dAcNpcSpJonny_c::talk_c::~talk_c() {
}

// .text 0xDFC
void dAcNpcSpJonny_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mNextOnRequestEnd = NULL;
}

namespace dANpcSpJonny {
// .text 0xE7C: the countPockets filter.
static BOOL isSpaceshipPart(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_SPACESHIP_PART));
}
} // namespace dANpcSpJonny

// .text 0xEBC
void dAcNpcSpJonny_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    dPersonalID_c helper = dSaveData_c::getRaw()->mGulliverHelper;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!dSaveData_c::getRaw()->isFlag(SAVE_FLAG_GULLIVER_AWAKE)) {
        mMsgNo = MSG_SLEEP_TALK + cM::rndInt(MSG_SLEEP_TALK_NUM);
    } else if (helper.isValid()) {
        if (helper == *player->getPersonalID()) {
            mMsgNo = MSG_THANKS_HELPER;
        } else {
            mMsgNo = MSG_THANKS;
        }
    } else if ((!isPlayerFromThisTown() && !dNpcMng_c::sGulliverVisitorMet) ||
               (isPlayerFromThisTown() == TRUE &&
                !player->isFlag1(PRIVATE_FLAG1_GULLIVER_MET))) {
        if (!isPlayerFromThisTown()) {
            dNpcMng_c::sGulliverVisitorMet = 1;
        } else {
            player->setFlag1(PRIVATE_FLAG1_GULLIVER_MET);
        }
        mMsgNo = MSG_INTRO;
    } else {
        u16 mask;
        if (player->countPockets(isSpaceshipPart, &mask) > 0) {
            mMsgNo = MSG_FIND_PART_ASK;
        } else {
            mMsgNo = MSG_FIND_PART;
        }
    }
}

// .text 0x1118
void dAcNpcSpJonny_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_jonny";
    info->mCode = mMsgNo;
}

// .text 0x1130
void dAcNpcSpJonny_c::talk_c::onMessageStart(int arg) {
}

// .text 0x1134
void dAcNpcSpJonny_c::talk_c::onMessageEnd(int kind) {
    int code = mMessageCode;
    if (code >= MSG_SLEEP_TALK &&
        code <= MSG_SLEEP_TALK + MSG_SLEEP_TALK_NUM - 1) {
        // One time in ten he wakes up.
        if (cM::rndInt(100) <= 10) {
            getController()->setNextMsgCode(MSG_WAKE_UP +
                                            cM::rndInt(MSG_WAKE_UP_NUM));
            if (!isPlayerFromThisTown()) {
                dNpcMng_c::sGulliverVisitorMet = 1;
            } else {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                    PRIVATE_FLAG1_GULLIVER_MET);
            }
            mOnMessageEnd = &talk_c::playWakeUp;
            mOnRequestEnd = &talk_c::wakeUp;
            mNextOnRequestEnd = &talk_c::closeWakeUp;
        }
    } else if (code >= MSG_WAKE_UP &&
               code <= MSG_WAKE_UP + MSG_WAKE_UP_NUM - 1) {
        mpController->setNextMsgCode(MSG_PART_LOST);
    } else if (code == MSG_PART_FOUND) {
        mOnMessageEnd = &talk_c::handOver;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x12B4
void dAcNpcSpJonny_c::talk_c::onAnswer(int arg) {
    dMsg::Comp_c *ctrl = getController();
    u8 answer = ctrl->getSelect()->mCursor;
    switch (mMessageCode) {
    case MSG_FIND_PART_ASK:
        if (answer == ANSWER_HERE_YOU_GO) {
            mOnAnswer = &talk_c::openPartSelect;
        } else {
            ctrl->setNextMsgCode(MSG_PART_TOUGH);
        }
        break;
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x1364
void dAcNpcSpJonny_c::talk_c::preExecute() {
}

// .text 0x1368
void dAcNpcSpJonny_c::talk_c::onTalkEnd() {
}

// .text 0x136C
void dAcNpcSpJonny_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = mNextOnRequestEnd;
        mNextOnRequestEnd = NULL;
    }
}

// .text 0x13E8
void dAcNpcSpJonny_c::talk_c::playWakeUp() {
    mpOwner->mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    requestAct09(HMN_ANM_SEG_GETUP2, 0.0f, cNpcMorphFrames, 4);
}

namespace dANpcSpJonny {
// .text 0x1440: the item select filter, only the spaceship part can be
// picked.
static BOOL isNotSpaceshipPart(void *pocket, int flag) {
    return !static_cast<dItem::Item *>(pocket)->isSame(
        dItem::Item(dItem::ITEM_IDX_SPACESHIP_PART));
}
} // namespace dANpcSpJonny

// .text 0x1488
void dAcNpcSpJonny_c::talk_c::openPartSelect() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(0xFFFF);
    }
    reqSelectItem(isNotSpaceshipPart, 0xE7, 0x34, 1);
    mOnRequestEnd = &talk_c::onPartSelected;
}

// .text 0x150C
void dAcNpcSpJonny_c::talk_c::takePart() {
    dItem::Item part(dItem::ITEM_IDX_SPACESHIP_PART);
    requestItemActEx(7, &part, 0, 0, 0, 2);
}

// .text 0x1560
void dAcNpcSpJonny_c::talk_c::handOver() {
    requestHandActD();
    mOnRequestEnd = &talk_c::giveReward;
    mNextOnRequestEnd = &talk_c::closeReward;
}

// .text 0x15C4
void dAcNpcSpJonny_c::talk_c::wakeUp() {
    dSaveData_c::getTown()->setFlag(SAVE_FLAG_GULLIVER_AWAKE);
    if (isNetActive()) {
        dNet::beginPacket();
        dNet::sendPacket(NET_PACKET_GULLIVER_FLAG, 4);
    }
    mpOwner->mAwake = 1;
    mpOwner->mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mpOwner->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep,
                               l_lookYawStep, cNpcLookRange);
    setRequest1(0);
    if (mpActActor) {
        mAng angle = mpOwner->getAngleYTo(mpActActor);
        mpOwner->mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                                     cNpcMorphFrames);
    }
}

// .text 0x16AC
void dAcNpcSpJonny_c::talk_c::closeWakeUp() {
    reqMsgClose();
}

// .text 0x16B0
void dAcNpcSpJonny_c::talk_c::onPartSelected() {
    if (isMenuInvalid()) {
        mpController->setNextMsgCode(MSG_PART_WRONG);
        reqMsgClose();
        return;
    }
    mpController->setNextMsgCode(MSG_PART_FOUND);
    takePart();
    mNextOnRequestEnd = &talk_c::closePartTaken;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dSaveData_c::getTown()->mGulliverHelper.copy(&player->mPID);
    if (isNetActive()) {
        dNet::beginPacket();
        dNet::sendPacket(NET_PACKET_GULLIVER_PLAYER, 4);
    }
}

// .text 0x1764
void dAcNpcSpJonny_c::talk_c::closePartTaken() {
    reqMsgClose();
}

// .text 0x1768
void dAcNpcSpJonny_c::talk_c::giveReward() {
    dItem::Item reward;
    dItemSel::Range_c range(dItem::KIND_NONE, dItem::FROM_JONNY);
    dItemSel::select(&reward, 1, &range, 1, &dItemSel::sDefaultFilter, NULL, 0,
                     NULL);
    requestItemAct(&reward, 0, 0);
    int slot = getMenuSelSlot();
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&reward, slot, 0);
    mpController->setNextMsgCode(MSG_REWARD);
}

// .text 0x1818
void dAcNpcSpJonny_c::talk_c::closeReward() {
    reqMsgClose();
}
