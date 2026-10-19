#include <game/game/d_a_npc_sp_pumpking.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/snd/snd_id.hpp>

// Jack on Halloween. Functions are in target .text order.

namespace dANpcSpPumpking {
// 0x5C
static BOOL entryDebugPage(dAcNpcSpPumpking_c *npc) {
    return FALSE;
}

// 0x64
BASE_PROFILE(A_NPC_SP_PUMPKING, dAcNpcSpPumpking_c);

// 0x1E0
static BOOL isCandy(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_CANDY;
}

// .bss 0x8 (built by __sinit): the morph frames of the disappear animation.
struct anmParam_s {
    anmParam_s() : _0(0), mMorph(4.0f) {
    }
    u32 _0;
    f32 mMorph;
};
static anmParam_s l_anmParam;
} // namespace dANpcSpPumpking

// 0x20C
void dAcNpcSpPumpking_c::onDemoCmd(int cmd, u8 member) {
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
            mPumpkingTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpPumpking {
static const dAcNpcSpPumpking_c::State_s l_states[] = {
    {&dAcNpcSpPumpking_c::initIdle, &dAcNpcSpPumpking_c::executeIdle},
    {&dAcNpcSpPumpking_c::initLook, &dAcNpcSpPumpking_c::executeLook},
    {&dAcNpcSpPumpking_c::initTalk, &dAcNpcSpPumpking_c::executeTalk},
    {&dAcNpcSpPumpking_c::initRemote, &dAcNpcSpPumpking_c::executeRemote},
    {&dAcNpcSpPumpking_c::initRemoteTalk,
     &dAcNpcSpPumpking_c::executeRemoteTalk},
};
} // namespace dANpcSpPumpking

// 0x328
void dAcNpcSpPumpking_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpPumpking::l_states[state].mInit)();
}

// 0x360
int dAcNpcSpPumpking_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mPumpkingTalk);
    return SUCCEEDED;
}

// 0x3AC
int dAcNpcSpPumpking_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mStep[STATE_IDLE] = 0;
    mStep[STATE_LOOK] = 0;
    mStep[STATE_TALK] = 0;
    mStep[STATE_REMOTE] = 0;
    mStep[STATE_REMOTE_TALK] = 0;
    mRelocateTimer = 0;
    mSpotTimer = 0;
    mLeaveTimer = 0;
    mWasTalking = 0;
    mDisappearing = 0;
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        if (isHalloween()) {
            mDaub.mPresent = 1;
        } else {
            mDaub.mPresent = 0;
        }
        if (!dNpc::canPutItemAt(&mPos, mNpcItem, NULL, dScene::SCENE_NUM, TRUE,
                                TRUE)) {
            mDaub.mPresent = 0;
        }
        mDaub.mTalking = 0;
        mDaub.mLeaving = 0;
        setDaubData(&mDaub, sizeof(mDaub));
        if (!dSaveData_c::getTown()->mNpcSpotB.mValid) {
            int unitX = worldToUnit(mPos.x);
            int unitZ = worldToUnit(mPos.z);
            dSaveTown_c *town = dSaveData_c::getTown();
            town->mNpcSpotB.set(unitX, unitZ);
        }
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (mDaub.mPresent) {
        mIsActive = 1;
    } else {
        mIsActive = 0;
    }
    dSceneCheck::fn_80081858(dANpcSpPumpking::entryDebugPage(this));
    return SUCCEEDED;
}

// 0x568
int dAcNpcSpPumpking_c::doDelete() {
    dANpcSpPumpking::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return dAcNpcSp_c::doDelete() != 0;
}

// 0x5AC
int dAcNpcSpPumpking_c::vtB0() {
    if (mSpotTimer > 0) {
        mSpotTimer--;
    }
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (isLocalOwner() && dNet::isSceneOwner()) {
        if (isHalloween()) {
            if (!mDaub.mPresent) {
                dSaveTown_c *town = dSaveData_c::getTown();
                mVec3_c pos = town->mNpcSpotB.getPos();
                if (!dNpc::canPutItemAt(&pos, mNpcItem, NULL, dScene::SCENE_NUM,
                                        TRUE, TRUE)) {
                    if (mSpotTimer <= 0) {
                        dNpcInfo::pumpking_c::get()->setSpot(&pos,
                                                             lbl_80750520);
                        mSpotTimer = SPOT_RETRY_FRAMES;
                    }
                } else if (mViewArea.isOutsideAll(NET_MEMBER_MAX, &pos)) {
                    mDaub.mPresent = 1;
                    mPos = mLastPos = mHomePos = pos;
                    setDaubData(&mDaub, sizeof(mDaub));
                }
            }
        } else if (mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
            mDaub.mPresent = 0;
            setDaubData(&mDaub, sizeof(mDaub));
        }
    }
    if (mDaub.mTalking) {
        mWasTalking = 1;
    } else if (mWasTalking) {
        mWasTalking = 0;
        mAcceptMoveReq = 0;
        mDisappearing = 1;
        mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_PKN_DISAPPEAR1, 0.0f,
                           dANpcSpPumpking::l_anmParam.mMorph,
                           m3d::PLAYMODE_INHERIT, 0);
    } else if (mDisappearing) {
        if (mAction.mActionId != ACTION_ANM || mAction.mCanChange) {
            mDisappearing = 0;
            mAudioObj.startSound(SE_Disappear);
            dEffect::create("af_pkn_disappear", &mPos, &mAngle, NULL);
            mIsActive = 0;
        }
    }
    if (!mDaub.mLeaving) {
        if (mDaub.mPresent) {
            if (!mIsActive) {
                dEffect::create("af_pkn_appear", &mPos, &mAngle, NULL);
            }
            mIsActive = 1;
            mAcceptMoveReq = 1;
        } else {
            mIsActive = 0;
        }
    }
    mLookAt.setFront(PRIORITY_DEMO, l_lookPitchStep, l_lookYawStep);
    if (mState < STATE_COUNT) {
        (this->*dANpcSpPumpking::l_states[mState].mExecute)();
    }
    return 1;
}

// 0x8AC
void dAcNpcSpPumpking_c::initIdle() {
    initWander();
    mRelocateTimer = RELOCATE_FRAMES;
}

// 0x8E0
void dAcNpcSpPumpking_c::executeIdle() {
    if (dSaveData_c::getTown()->mNpcSpotB.mFlag) {
        if (mRelocateTimer > 0) {
            mRelocateTimer--;
        } else {
            relocate();
        }
    } else {
        mRelocateTimer = RELOCATE_FRAMES;
    }
    executeWander(NULL);
}

// 0x960
void dAcNpcSpPumpking_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep[STATE_LOOK] = LOOK_STEP_TURN;
}

// 0x9C4
void dAcNpcSpPumpking_c::stepLookTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep[STATE_LOOK]++;
    }
}

namespace dANpcSpPumpking {
static const dAcNpcSpPumpking_c::Proc l_lookSteps[] = {
    &dAcNpcSpPumpking_c::stepLookTurn};
} // namespace dANpcSpPumpking

// 0xA24
void dAcNpcSpPumpking_c::executeLook() {
    if (mStep[STATE_LOOK] < LOOK_STEP_DONE) {
        (this->*dANpcSpPumpking::l_lookSteps[mStep[STATE_LOOK]])();
    }
}

// 0xA64
void dAcNpcSpPumpking_c::initTalk() {
    mLookAt.setPlayer(PRIORITY_DEMO, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, 0);
    mStep[STATE_TALK] = TALK_STEP_TALK;
    mDaub.mTalking = 1;
    mDaub.mLeaving = 1;
    setDaubData(&mDaub, sizeof(mDaub));
}

// 0xB18
void dAcNpcSpPumpking_c::stepTalk() {
    if (!mTalk.isBusy()) {
        mLookAt.setFront(PRIORITY_DEMO, l_lookPitchStep, l_lookYawStep);
        mDaub.mTalking = 0;
        setDaubData(&mDaub, sizeof(mDaub));
        mStep[STATE_TALK]++;
    }
}

// 0xB94
void dAcNpcSpPumpking_c::stepTalkDisappear() {
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mLeaveTimer = LEAVE_FRAMES;
        dSaveTown_c *town = dSaveData_c::getTown();
        town->mNpcSpotB.mFlag = 1;
        if (dAcNpc_c::isMultiPlay()) {
            dNet::beginPacket();
            dNet::sendPacket(NET_PACKET_NPC_SPOT_B_FLAG, NET_MEMBER_NONE);
        }
        mStep[STATE_TALK]++;
    }
}

// 0xC14
void dAcNpcSpPumpking_c::stepTalkLeave() {
    if (mLeaveTimer > 0) {
        if (--mLeaveTimer == LEAVE_HIDE_FRAME) {
            mDaub.mLeaving = 0;
            mDaub.mPresent = 0;
            setDaubData(&mDaub, sizeof(mDaub));
        }
    } else {
        dDemo::finishTalk(this);
        mStep[STATE_TALK]++;
    }
}

namespace dANpcSpPumpking {
static const dAcNpcSpPumpking_c::Proc l_talkSteps[] = {
    &dAcNpcSpPumpking_c::stepTalk,
    &dAcNpcSpPumpking_c::stepTalkDisappear,
    &dAcNpcSpPumpking_c::stepTalkLeave,
};
} // namespace dANpcSpPumpking

// 0xC88
void dAcNpcSpPumpking_c::executeTalk() {
    if (mStep[STATE_TALK] < TALK_STEP_DONE) {
        (this->*dANpcSpPumpking::l_talkSteps[mStep[STATE_TALK]])();
    }
}

// 0xCC8
void dAcNpcSpPumpking_c::initRemote() {
    resetLookTimer();
}

// 0xCD0
void dAcNpcSpPumpking_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mPumpkingTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    executeRemoteWander(NULL);
}

// 0xD70
void dAcNpcSpPumpking_c::initRemoteTalk() {
}

// 0xD74
void dAcNpcSpPumpking_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// 0xDC8: around the center of his acre first.
BOOL dAcNpcSpPumpking_c::searchWanderPos(mVec3_c *pos) {
    mVec3_c center;
    int blockX = worldToBlock(mHomePos.x);
    int blockZ = worldToBlock(mHomePos.z);
    dFdBase_c::getBlockCenterPos(&center, blockX, blockZ);
    if (searchPosInCircle(pos, &center, &mPos, lbl_80750520 / 2,
                          64.0f + getWanderMinDist(), 1.5f * lbl_80750520)) {
        return TRUE;
    }
    return searchPosNear(pos, &mPos, getWanderMinDist(), getWanderMaxDist());
}

// 0xEDC
bool dAcNpcSpPumpking_c::isHalloween() {
    return dEvent::isOngoing(EVENT_HALLOWEEN);
}

// 0xF10
void dAcNpcSpPumpking_c::relocate() {
    dNpcInfo::pumpking_c::get()->setSpot(&mPos, lbl_80750520);
}

// 0xF4C
dAcNpcSpPumpking_c::talk_c::talk_c(dAcNpcSpPumpking_c *npc) {
    mpOwner = npc;
    init();
}

// 0xFC4
dAcNpcSpPumpking_c::talk_c::~talk_c() {
}

// 0x102C
void dAcNpcSpPumpking_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mNextRequestEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mNextRequestEnd = NULL;
    mItem = dItem::Item();
    mGift = dItem::Item();
}

// 0x10EC
void dAcNpcSpPumpking_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    mMsgNo = MSG_TRICK_OR_TREAT;
}

// 0x113C
void dAcNpcSpPumpking_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_pumpking";
    info->mCode = mMsgNo;
}

// 0x1154
void dAcNpcSpPumpking_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_CANDY_GIFT:
    case MSG_CANDY_GIFT_2:
        setItemName(mGift, 0);
        break;
    }
}

// 0x117C
void dAcNpcSpPumpking_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_TRICK_OR_TREAT:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                dANpcSpPumpking::isCandy, NULL) > 0) {
            mpController->setNextMsgCode(MSG_ASK_CANDY);
        } else {
            mpController->setNextMsgCode(MSG_ASK_CANDY_NONE);
        }
        break;
    case MSG_CANDY_THANKS:
    case MSG_CANDY_THANKS_2:
        mpController->setNextMsgCode(mMessageCode + 1);
        mOnMessageEnd = &talk_c::reqWait;
        mOnRequestEnd = &talk_c::reqThanksPose;
        break;
    case MSG_CANDY_PRESENT:
    case MSG_CANDY_PRESENT_2: {
        mpController->setNextMsgCode(mMessageCode + 1);
        dItemSel::Range_c range(dItem::KIND_NONE, dItem::FROM_HALLOWEEN);
        dItemSel::select(&mGift, 1, &range, 1, &dItemSel::sDefaultFilter, NULL,
                         0, NULL);
        mOnMessageEnd = &talk_c::reqGiveGift;
        mOnRequestEnd = &talk_c::pickUpGift;
        break;
    }
    case MSG_NO_CANDY:
    case MSG_NO_CANDY_2:
    case MSG_REFUSED:
    case MSG_REFUSED_2:
        mpController->setNextMsgCode(mMessageCode + 1);
        mOnMessageEnd = &talk_c::trickClothes;
        mOnRequestEnd = &talk_c::onTrickDone;
        break;
    case MSG_NOT_CANDY:
    case MSG_NOT_CANDY_2:
        mpController->setNextMsgCode(mMessageCode + 1);
        mOnMessageEnd = &talk_c::reqTrickPose;
        mOnRequestEnd = &talk_c::trickAfterPose;
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// 0x13B4
void dAcNpcSpPumpking_c::talk_c::onAnswer(int arg) {
    u8 answer = mpController->getAnswer();
    switch (mMessageCode) {
    case MSG_ASK_CANDY:
    case MSG_GIVE_CANDY:
        switch (answer) {
        case ANSWER_YES:
            mOnAnswer = &talk_c::reqSelectCandy;
            mOnRequestEnd = &talk_c::onCandySelected;
            break;
        case ANSWER_NO:
            if (!dNpcInfo::pumpking_c::get()->isRefused()) {
                dNpcInfo::pumpking_c::get()->setRefused();
                mpController->setNextMsgCode(MSG_REFUSED);
            } else {
                mpController->setNextMsgCode(MSG_REFUSED_2);
            }
            break;
        }
        break;
    case MSG_ASK_CANDY_NONE:
        switch (answer) {
        case ANSWER_YES:
            mpController->setNextMsgCode(MSG_GIVE_CANDY);
            break;
        case ANSWER_NO:
            if (!dNpcInfo::pumpking_c::get()->isNoCandyTold()) {
                dNpcInfo::pumpking_c::get()->setNoCandyTold();
                mpController->setNextMsgCode(MSG_NO_CANDY);
            } else {
                mpController->setNextMsgCode(MSG_NO_CANDY_2);
            }
            break;
        }
        break;
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// 0x152C
void dAcNpcSpPumpking_c::talk_c::preExecute() {
}

// 0x1530
void dAcNpcSpPumpking_c::talk_c::onTalkEnd() {
}

// 0x1534
void dAcNpcSpPumpking_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
        if (mNextRequestEnd) {
            mOnRequestEnd = mNextRequestEnd;
        }
        mNextRequestEnd = NULL;
    }
}

// 0x15DC
void dAcNpcSpPumpking_c::talk_c::reqSelectCandy() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(0xFFFF);
    }
    reqSelectItem((u16)0, 0xE7, 0x2E, TRUE);
}

// 0x1640
void dAcNpcSpPumpking_c::talk_c::reqReceiveItem() {
    requestItemActEx(7, &mItem, 0, 0, 0, 2);
}

// 0x165C
void dAcNpcSpPumpking_c::talk_c::reqWait() {
    setRequest1(0);
}

// 0x1664
void dAcNpcSpPumpking_c::talk_c::reqTrickPose() {
    requestHandActD();
}

// 0x1668
void dAcNpcSpPumpking_c::talk_c::reqGiveGift() {
    requestItemAct(&mGift, 0, 0);
}

// 0x1678
namespace dANpcSpPumpking {
static inline BOOL isEmpty(const dItem::Item &item) {
    return item.isSame(dItem::Item());
}
} // namespace dANpcSpPumpking

void dAcNpcSpPumpking_c::talk_c::trickClothes() {
    mEquip.setFromPlayer();
    if (dNpcInfo::isSameFashion(&mEquip.mHat, &mEquip.mShirt) ||
        dNpcInfo::isSameFashion(&mEquip.mAcc, &mEquip.mShirt)) {
        trickPockets();
    } else {
        BOOL ok =
            mEquip.mHat.isNotSame(dItem::Item(dItem::ITEM_IDX_PUMPKIN_HEAD));
        if (ok) {
            ok = dANpcSpPumpking::isEmpty(mEquip.mHat);
            if (!ok) {
                ok = !dItem::infoBank_c::get()
                          ->getBITM(mEquip.mHat)
                          ->m_noPurchase;
            }
        }
        if (ok) {
            ok = dANpcSpPumpking::isEmpty(mEquip.mAcc);
            if (!ok) {
                ok = !dItem::infoBank_c::get()
                          ->getBITM(mEquip.mAcc)
                          ->m_noPurchase;
            }
        }
        if (ok) {
            mEquip.mHat = dItem::Item(dItem::ITEM_IDX_PUMPKIN_HEAD);
            mEquip.mAcc = dItem::Item();
        } else {
            ok = mEquip.mShirt.isNotSame(
                dItem::Item(dItem::ITEM_IDX_MOLDY_SHIRT));
            if (ok) {
                ok = mEquip.mShirt.isNotSame(
                    dItem::Item(dItem::ITEM_IDX_PATCHED_SHIRT));
            }
            if (ok) {
                ok = dANpcSpPumpking::isEmpty(mEquip.mShirt);
                if (!ok) {
                    ok = !dItem::infoBank_c::get()
                              ->getBITM(mEquip.mShirt)
                              ->m_noPurchase;
                }
            }
            if (ok) {
                if (cM::rndInt(2) == 0) {
                    mEquip.mShirt = dItem::Item(dItem::ITEM_IDX_MOLDY_SHIRT);
                } else {
                    mEquip.mShirt = dItem::Item(dItem::ITEM_IDX_PATCHED_SHIRT);
                }
            } else {
                trickPockets();
            }
        }
    }
    requestEquip(&mEquip, 6, 0);
}

// 0x18B0
void dAcNpcSpPumpking_c::talk_c::onCandySelected() {
    if (isMenuInvalid()) {
        if (!dNpcInfo::pumpking_c::get()->isRefused()) {
            dNpcInfo::pumpking_c::get()->setRefused();
            mpController->setNextMsgCode(MSG_REFUSED);
        } else {
            mpController->setNextMsgCode(MSG_REFUSED_2);
        }
        reqMsgClose();
        return;
    }
    int slot = getMenuSelSlot();
    mItem = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[slot];
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mItem);
    if (mItem.getKind() == dItem::KIND_CANDY) {
        if (!dNpcInfo::pumpking_c::get()->isCandyGiven()) {
            dNpcInfo::pumpking_c::get()->setCandyGiven();
            mpController->setNextMsgCode(MSG_CANDY_THANKS);
        } else {
            mpController->setNextMsgCode(MSG_CANDY_THANKS_2);
        }
    } else if (!dNpcInfo::pumpking_c::get()->isOtherGiven()) {
        dNpcInfo::pumpking_c::get()->setOtherGiven();
        mpController->setNextMsgCode(MSG_NOT_CANDY);
    } else {
        mpController->setNextMsgCode(MSG_NOT_CANDY_2);
    }
    reqReceiveItem();
    mNextRequestEnd = &talk_c::removeItem;
}

// 0x19F8
void dAcNpcSpPumpking_c::talk_c::removeItem() {
    reqMsgClose();
    dItem::Item empty;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&empty, getMenuSelSlot(),
                                                     FALSE);
}

// 0x1A48
void dAcNpcSpPumpking_c::talk_c::reqThanksPose() {
    requestHandAct10(2);
    mNextRequestEnd = &talk_c::closeThanks;
}

// 0x1A94
void dAcNpcSpPumpking_c::talk_c::closeThanks() {
    reqMsgClose();
    mNextRequestEnd = &talk_c::onThanksDone;
}

// 0x1ADC
void dAcNpcSpPumpking_c::talk_c::trickAfterPose() {
    trickClothes();
    mNextRequestEnd = &talk_c::onTrickDone;
}

// 0x1B24
void dAcNpcSpPumpking_c::talk_c::onThanksDone() {
}

// 0x1B28
void dAcNpcSpPumpking_c::talk_c::pickUpGift() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&mGift, FALSE);
}

// 0x1B60
void dAcNpcSpPumpking_c::talk_c::onTrickDone() {
}

namespace dANpcSpPumpking {
// 0x1B64: an item that can be bought (not a jack-in-the-box target otherwise).
static BOOL isTrickable(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    return !bitm->m_noPurchase;
}
} // namespace dANpcSpPumpking

// 0x1BB4
void dAcNpcSpPumpking_c::talk_c::trickPockets() {
    u16 mask = 0;
    int pick = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
        dANpcSpPumpking::isTrickable, &mask);
    if (pick > 0) {
        pick = cM::rndInt(pick);
        for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if (mask & (1 << i)) {
                if (pick > 0) {
                    pick--;
                } else {
                    dItem::Item jackInTheBox(dItem::ITEM_IDX_JACK_IN_THE_BOX);
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(
                        &jackInTheBox, i, FALSE);
                    break;
                }
            }
        }
    }
}
