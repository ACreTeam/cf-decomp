#include <game/game/d_a_npc_sp_dancer.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/snd/snd_id.hpp>

namespace dANpcSpDancer {
// .text 0x5C: pocket filter, any candy.
static BOOL isCandy(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_CANDY;
}

// .text 0x88: pocket filter, the candy of the day's colour.
static BOOL isTodaysCandy(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_BLUE_CANDY,
                                    dSaveData_c::getTown()->mDailyRand.mValue,
                                    FALSE));
}

// .text 0xD8
BASE_PROFILE(A_NPC_SP_DANCER, dAcNpcSpDancer_c);

// .bss 0x8, built by __sinit: a word and the range around the npc that keeps
// the encounter (vtB0).
struct unk_s {
    unk_s() : _0(0), mNearDist(128.0f) {
    }
    u32 _0;
    f32 mNearDist;
};
static unk_s l_unk;
} // namespace dANpcSpDancer

// .text 0x248
void dAcNpcSpDancer_c::onDemoCmd(int cmd, u8 member) {
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
            mDancerTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpDancer {
static const dAcNpcSpDancer_c::State_s l_states[] = {
    {&dAcNpcSpDancer_c::initIdle, &dAcNpcSpDancer_c::executeIdle},
    {&dAcNpcSpDancer_c::initLook, &dAcNpcSpDancer_c::executeLook},
    {&dAcNpcSpDancer_c::initTalk, &dAcNpcSpDancer_c::executeTalk},
    {&dAcNpcSpDancer_c::initRemote, &dAcNpcSpDancer_c::executeRemote},
    {&dAcNpcSpDancer_c::initRemoteTalk, &dAcNpcSpDancer_c::executeRemoteTalk}};
} // namespace dANpcSpDancer

// .text 0x364
void dAcNpcSpDancer_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpDancer::l_states[state].mInit)();
}
// .text 0x39C
int dAcNpcSpDancer_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mDancerTalk);
    mAnmSet.setAnm0(HMN_ANM_PCK_DANCE1);
    return 1;
}
// .text 0x3F4
int dAcNpcSpDancer_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    for (int i = 0; i < STATE_NUM; i++) {
        mStep[i] = 0;
    }
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
    } else {
        changeState(STATE_REMOTE);
    }
    mFacedAway = 0;
    moveFtrToPoliceBox();
    return 1;
}
// .text 0x488
int dAcNpcSpDancer_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}
// .text 0x4B4
int dAcNpcSpDancer_c::vtB0() {
    if (mState < STATE_NUM) {
        (this->*dANpcSpDancer::l_states[mState].mExecute)();
    }
    if (!isPlayerNear(dNet::getMyNo(), FALSE, NULL,
                      dANpcSpDancer::l_unk.mNearDist)) {
        dNpcInfo::dancer_c::get()->resetEncounter();
    }
    mAudioObj.holdSound(SE_CarnivalDance);
    dBgm::l_mgr.mVolCtrl.mFocus.set(0, mPos);
    return 1;
}
// .text 0x56C
void dAcNpcSpDancer_c::initIdle() {
    mAnmSet.setAnm0(HMN_ANM_PCK_DANCE1);
    mAction.requestTurn(PRIORITY_IDLE, mAng(0), l_turnSpeed, 0,
                        cNpcMorphFrames);
    mStep[STATE_IDLE] = IDLE_STEP_TURN;
}
// .text 0x5D8
void dAcNpcSpDancer_c::stepIdleTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
        mStep[STATE_IDLE] = IDLE_STEP_DANCE;
    }
}
// .text 0x634
void dAcNpcSpDancer_c::stepIdleDance() {
}
namespace dANpcSpDancer {
static const dAcNpcSpDancer_c::Proc l_idleSteps[] = {
    &dAcNpcSpDancer_c::stepIdleTurn, &dAcNpcSpDancer_c::stepIdleDance};
} // namespace dANpcSpDancer
// .text 0x638
void dAcNpcSpDancer_c::executeIdle() {
    if (mStep[STATE_IDLE] < IDLE_STEP_NUM) {
        (this->*dANpcSpDancer::l_idleSteps[mStep[STATE_IDLE]])();
    }
}
// .text 0x678
void dAcNpcSpDancer_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    if (labs((s16)(mAngle.y - angle)) >= 0x6000) {
        mFacedAway = 1;
    } else {
        mFacedAway = 0;
    }
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep[STATE_LOOK] = LOOK_STEP_TURN;
}
// .text 0x70C
void dAcNpcSpDancer_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep[STATE_LOOK];
    }
}
namespace dANpcSpDancer {
static const dAcNpcSpDancer_c::Proc l_lookSteps[] = {
    &dAcNpcSpDancer_c::stepLook};
} // namespace dANpcSpDancer
// .text 0x76C
void dAcNpcSpDancer_c::executeLook() {
    if (mStep[STATE_LOOK] < LOOK_STEP_NUM) {
        (this->*dANpcSpDancer::l_lookSteps[mStep[STATE_LOOK]])();
    }
}
// .text 0x7AC
void dAcNpcSpDancer_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    mStep[STATE_TALK] = TALK_STEP_TALK;
}
// .text 0x818
void dAcNpcSpDancer_c::stepTalk() {
    if (!mTalk.isBusy()) {
        mAction.requestTurn(PRIORITY_DEMO, mAng(0), l_turnSpeed, 0,
                            cNpcMorphFrames);
        ++mStep[STATE_TALK];
    }
}
// .text 0x888
void dAcNpcSpDancer_c::stepTalkTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        dDemo::finishTalk(this);
        ++mStep[STATE_TALK];
    }
}
namespace dANpcSpDancer {
static const dAcNpcSpDancer_c::Proc l_talkSteps[] = {
    &dAcNpcSpDancer_c::stepTalk, &dAcNpcSpDancer_c::stepTalkTurn};
} // namespace dANpcSpDancer
// .text 0x8D8
void dAcNpcSpDancer_c::executeTalk() {
    if (mStep[STATE_TALK] < TALK_STEP_NUM) {
        (this->*dANpcSpDancer::l_talkSteps[mStep[STATE_TALK]])();
    }
}
// .text 0x918
void dAcNpcSpDancer_c::initRemote() {
    mAnmSet.setAnm0(HMN_ANM_PCK_DANCE1);
}
// .text 0x924
void dAcNpcSpDancer_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mDancerTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
}
// .text 0x9B8
void dAcNpcSpDancer_c::initRemoteTalk() {
    mAnmSet.setAnm0(HMN_ANM_PCK_DANCE1);
}
// .text 0x9C4
void dAcNpcSpDancer_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0xA18
dAcNpcSpDancer_c::talk_c::talk_c(dAcNpcSpDancer_c *npc) : mpOwner(npc) {
    init();
}
// .text 0xA88
dAcNpcSpDancer_c::talk_c::~talk_c() {
}
// .text 0xAE0
void dAcNpcSpDancer_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mCandy = dItem::Item();
    mGift = dItem::Item();
}
// .text 0xB58
void dAcNpcSpDancer_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    mCandy.setFromIndex(dItem::ITEM_IDX_BLUE_CANDY,
                        dSaveData_c::getTown()->mDailyRand.mValue, FALSE);
    if (!dNpcInfo::dancer_c::get()->isTalked()) {
        mMsgNo = MSG_FIRST_TALK;
    } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                   dANpcSpDancer::isCandy, NULL) <= 0) {
        if (dNpcInfo::dancer_c::get()->mNoCandyTold) {
            mMsgNo = MSG_NO_CANDY_AGAIN;
        } else {
            mMsgNo = MSG_NO_CANDY;
            dNpcInfo::dancer_c::get()->setNoCandyTold();
        }
    } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                   dANpcSpDancer::isTodaysCandy, NULL) <= 0) {
        if (dNpcInfo::dancer_c::get()->mWrongCandyTold) {
            mMsgNo = MSG_WRONG_CANDY_AGAIN;
        } else {
            mMsgNo = MSG_WRONG_CANDY;
            dNpcInfo::dancer_c::get()->setWrongCandyTold();
        }
    } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                   dANpcSpDancer::isTodaysCandy, NULL) < CANDY_NUM) {
        if (dNpcInfo::dancer_c::get()->mTooFewTold) {
            mMsgNo = MSG_TOO_FEW_AGAIN;
        } else {
            mMsgNo = MSG_TOO_FEW;
            dNpcInfo::dancer_c::get()->setTooFewTold();
        }
    } else {
        dNpcInfo::dancer_c::get()->resetCandyHints();
        if (dNpcInfo::dancer_c::get()->mGiftGiven) {
            mMsgNo = MSG_GIVE_AGAIN;
        } else {
            mMsgNo = MSG_GIVE;
        }
    }
}
// .text 0xCF0
void dAcNpcSpDancer_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_berliner";
    info->mCode = mMsgNo;
}
// .text 0xD08
void dAcNpcSpDancer_c::talk_c::onMessageStart(int arg) {
    if (mCandy.mId != dItem::ITEM_ID_NONE) {
        setItemName(mCandy, 0);
    }
    if (mGift.mId != dItem::ITEM_ID_NONE) {
        setItemName(mGift, 1);
    }
    switch (arg) {
    case START_REQ_DANCE:
        mpOwner->mAnmSet.setAnm0(HMN_ANM_PCK_DANCE2);
        break;
    case START_REQ_STOP_DANCE:
        mpOwner->mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
        break;
    }
    switch (mMessageCode) {
    case MSG_EAT:
        mpOwner->mAudioObj.startSound(SE_Eat_Gokun);
        break;
    }
}
// .text 0xDD4
void dAcNpcSpDancer_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_TRY_AGAIN:
        if (dNpcInfo::isPlayerFashionMatching()) {
            mpController->setNextMsgCode(MSG_COSTUME);
        } else {
            mpController->setNextMsgCode(MSG_NO_DANCER);
        }
        break;
    case MSG_COSTUME:
        mpController->setNextMsgCode(MSG_NO_DANCER);
        break;
    case MSG_NEED_KIND:
    case MSG_NO_CANDY_ANY: {
        // two messages per colour
        int color = dSaveData_c::getTown()->mDailyRand.mValue;
        mpController->setNextMsgCode(mMessageCode + color * 2 + 1);
        break;
    }
    case MSG_NO_CANDY_AGAIN:
    case MSG_WRONG_CANDY_SORRY:
    case MSG_WRONG_CANDY_AGAIN: {
        int color = dSaveData_c::getTown()->mDailyRand.mValue;
        mpController->setNextMsgCode(mMessageCode + color + 1);
        break;
    }
    case MSG_GIVE_TAKE:
    case MSG_GIVE_AGAIN_TAKE:
        mpController->setNextMsgCode(MSG_EAT);
        mOnMessageEnd = &talk_c::takeCandy;
        mOnRequestEnd = &talk_c::removeCandy;
        break;
    case MSG_TASTY: {
        dItemSel::Range_c range(dItem::KIND_NONE, dItem::FROM_CARNIVAL);
        dItemSel::Filter_c filter;
        dItemSel::select(&mGift, 1, &range, 1, &filter, NULL, 0, NULL);
        if (dNpcInfo::dancer_c::get()->mGiftGiven) {
            mpController->setNextMsgCode(MSG_GIFT_AGAIN);
        } else {
            mpController->setNextMsgCode(MSG_GIFT);
            dNpcInfo::dancer_c::get()->setGiftGiven();
        }
        break;
    }
    case MSG_GIFT:
    case MSG_GIFT_AGAIN:
        mOnMessageEnd = &talk_c::giveGift;
        mOnRequestEnd = &talk_c::receiveGift;
        break;
    case MSG_NEXT_CANDY:
    case MSG_NEXT_CANDY + 4:
    case MSG_NEXT_CANDY + 8:
    case MSG_NEXT_CANDY + 12:
        dNpcInfo::dancer_c::get()->setCandyAsked();
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}
// .text 0x100C
void dAcNpcSpDancer_c::talk_c::onAnswer(int arg) {
}
// .text 0x1010
void dAcNpcSpDancer_c::talk_c::preExecute() {
}
// .text 0x1014
void dAcNpcSpDancer_c::talk_c::onTalkEnd() {
    dNpcInfo::dancer_c::get()->setTalked();
}
// .text 0x1038
void dAcNpcSpDancer_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}
// .text 0x109C
void dAcNpcSpDancer_c::talk_c::takeCandy() {
    requestItemActEx(10, &mCandy, 0, 0, 0, 2);
}
// .text 0x10B8
void dAcNpcSpDancer_c::talk_c::giveGift() {
    requestItemAct(&mGift, 0, 0);
}
// .text 0x10C8
void dAcNpcSpDancer_c::talk_c::removeCandy() {
    u16 mask;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
        dANpcSpDancer::isTodaysCandy, &mask);
    int n = 0;
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (mask & (1 << i)) {
            dItem::Item empty;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&empty, i, FALSE);
            if (++n >= CANDY_NUM) {
                break;
            }
        }
    }
}
// .text 0x116C
void dAcNpcSpDancer_c::talk_c::receiveGift() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mGift, FALSE);
    dSaveData_c::getTown()->mDailyRand.change();
    if (dNpcInfo::dancer_c::get()->isCandyAsked()) {
        mpController->setNextMsgCode(
            dSaveData_c::getTown()->mDailyRand.mValue * 4 + MSG_FULL_AGAIN);
    } else {
        mpController->setNextMsgCode(
            dSaveData_c::getTown()->mDailyRand.mValue * 4 + MSG_FULL);
    }
}

// .text 0x1208
int dAcNpcSpDancer_c::getEarType() {
    return NPC_EAR_TYPE_PAVE;
}
