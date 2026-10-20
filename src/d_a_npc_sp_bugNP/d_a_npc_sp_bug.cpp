#include <game/game/d_a_npc_sp_bug.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bug_off.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/snd/snd_id.hpp>

// Nat at the Bug-Off. Functions are in target .text order.

namespace dANpcSpBug {
static inline dBugOff_c *getBugOff() {
    dSaveTown_c *town = dSaveData_c::getTown();
    return &town->mBugOff;
}
// The current player holds the record.
static inline BOOL isPlayerLeader() {
    return *getBugOff()->getPlayer(0) ==
           dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID;
}
// .text 0x5C: a net of any grade.
static BOOL isNet(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_NET ||
           item->getKind() == dItem::KIND_SILVER_NET ||
           item->getKind() == dItem::KIND_GOLD_NET;
}

// .text 0xC4: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpBug_c *npc) {
    return FALSE;
}

// .text 0xCC
BASE_PROFILE(A_NPC_SP_BUG, dAcNpcSpBug_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpBug

// .text 0x248
void dAcNpcSpBug_c::onDemoCmd(int cmd, u8 member) {
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
            mBugTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpBug {
static const dAcNpcSpBug_c::State_s l_states[] = {
    {&dAcNpcSpBug_c::initIdle, &dAcNpcSpBug_c::executeIdle},
    {&dAcNpcSpBug_c::initLook, &dAcNpcSpBug_c::executeLook},
    {&dAcNpcSpBug_c::initTalk, &dAcNpcSpBug_c::executeTalk},
    {&dAcNpcSpBug_c::initRemote, &dAcNpcSpBug_c::executeRemote},
    {&dAcNpcSpBug_c::initRemoteTalk, &dAcNpcSpBug_c::executeRemoteTalk}};
} // namespace dANpcSpBug

// .text 0x364
void dAcNpcSpBug_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpBug::l_states[state].mInit)();
}

// .text 0x39C
int dAcNpcSpBug_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mBugTalk);
    return 1;
}

// .text 0x3E8
int dAcNpcSpBug_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mStep[STATE_IDLE] = 0;
    mStep[STATE_LOOK] = 0;
    mStep[STATE_TALK] = 0;
    mStep[STATE_REMOTE] = 0;
    mStep[STATE_REMOTE_TALK] = 0;
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        if (isBugOffTime()) {
            mDaub.mActive = 1;
        } else {
            mDaub.mActive = 0;
        }
        mDaub.mForceAdvance = 1;
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (mDaub.mActive) {
        mIsActive = 1;
    } else {
        mIsActive = 0;
    }
    dANpcSpBug::getBugOff()->mNoticePending = TRUE;

    dSceneCheck::fn_80081858(dANpcSpBug::entryDebugPage(this));
    if (mIsActive) {
        moveFtrToPoliceBox();
    }
    return 1;
}

// .text 0x514
int dAcNpcSpBug_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return 0;
    }
    dANpcSpBug::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return 1;
}

// .text 0x560
int dAcNpcSpBug_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (isLocalOwner() && dNet::isSceneOwner() &&
        mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) { // every member
        mDaub.mForceAdvance = 1;
        if (isBugOffTime()) {
            if (!mDaub.mActive) {
                mDaub.mActive = 1;
                setDaubData(&mDaub, sizeof(mDaub));
            }
        } else {
            if (mDaub.mActive) {
                mDaub.mActive = 0;
                setDaubData(&mDaub, sizeof(mDaub));
            }
        }
    }
    if (mDaub.mActive) {
        mIsActive = 1;
    } else {
        mIsActive = 0;
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpBug::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x690
void dAcNpcSpBug_c::initIdle() {
    mAction.requestTurn(PRIORITY_IDLE, mAng(0), l_turnSpeed, 0,
                        cNpcMorphFrames);
    mStep[STATE_IDLE] = IDLE_STEP_TURN;
}

// .text 0x6F0
void dAcNpcSpBug_c::stepIdleTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
        mStep[STATE_IDLE] = IDLE_STEP_WAIT;
    }
}

// .text 0x74C
void dAcNpcSpBug_c::stepIdleWait() {
}

namespace dANpcSpBug {
static const dAcNpcSpBug_c::Proc l_idleSteps[] = {&dAcNpcSpBug_c::stepIdleTurn,
                                                  &dAcNpcSpBug_c::stepIdleWait};
} // namespace dANpcSpBug

// .text 0x750
void dAcNpcSpBug_c::executeIdle() {
    if (mStep[STATE_IDLE] < IDLE_STEP_DONE) {
        (this->*dANpcSpBug::l_idleSteps[mStep[STATE_IDLE]])();
    }
}

// .text 0x790
void dAcNpcSpBug_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep[STATE_LOOK] = LOOK_STEP_TURN;
}

// .text 0x7F4
void dAcNpcSpBug_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep[STATE_LOOK]++;
    }
}

namespace dANpcSpBug {
static const dAcNpcSpBug_c::Proc l_lookSteps[] = {&dAcNpcSpBug_c::stepLook};
} // namespace dANpcSpBug

// .text 0x854
void dAcNpcSpBug_c::executeLook() {
    if (mStep[STATE_LOOK] < LOOK_STEP_DONE) {
        (this->*dANpcSpBug::l_lookSteps[mStep[STATE_LOOK]])();
    }
}

// .text 0x894
void dAcNpcSpBug_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep[STATE_TALK] = TALK_STEP_TURN;
}

// .text 0x900
void dAcNpcSpBug_c::stepTalkTurn() {
    if (!mTalk.isBusy()) {
        mAction.requestTurn(PRIORITY_DEMO, mAng(0), l_turnSpeed, 0,
                            cNpcMorphFrames);
        mStep[STATE_TALK]++;
    }
}

// .text 0x970
void dAcNpcSpBug_c::stepTalkTurnEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mStep[STATE_TALK]++;
    }
}

namespace dANpcSpBug {
static const dAcNpcSpBug_c::Proc l_talkSteps[] = {
    &dAcNpcSpBug_c::stepTalkTurn, &dAcNpcSpBug_c::stepTalkTurnEnd};
} // namespace dANpcSpBug

// .text 0x9C0
void dAcNpcSpBug_c::executeTalk() {
    if (mStep[STATE_TALK] < TALK_STEP_DONE) {
        (this->*dANpcSpBug::l_talkSteps[mStep[STATE_TALK]])();
    }
}

// .text 0xA00
void dAcNpcSpBug_c::initRemote() {
}

// .text 0xA04
void dAcNpcSpBug_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mBugTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
}

// .text 0xA98
void dAcNpcSpBug_c::initRemoteTalk() {
}

// .text 0xA9C
void dAcNpcSpBug_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0xAF0
bool dAcNpcSpBug_c::isBugOffOngoing() {
    return dEvent::isOngoing(EVENT_BUG_OFF);
}

// .text 0xB24
bool dAcNpcSpBug_c::isBugOffNotStarted() {
    return dEvent::isNotStarted(EVENT_BUG_OFF);
}

// .text 0xB58
bool dAcNpcSpBug_c::isBugOffOver() {
    return dEvent::isOver(EVENT_BUG_OFF);
}

// .text 0xB8C: from before the start until six hours after the end.
BOOL dAcNpcSpBug_c::isBugOffTime() {
    return isBugOffNotStarted() || isBugOffOngoing() ||
           dEvent::isOverWithin(EVENT_BUG_OFF, 6, 0);
}

// .text 0xC0C
BOOL dAcNpcSpBug_c::isForceAdvance() {
    return isBugOffOngoing() && mDaub.mForceAdvance;
}

// .text 0xC60
dAcNpcSpBug_c::talk_c::talk_c(dAcNpcSpBug_c *npc) : mpOwner(npc) {
    init();
}

// .text 0xCD0
dAcNpcSpBug_c::talk_c::~talk_c() {
}

// .text 0xD28
void dAcNpcSpBug_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgCode = 0;
    _0EC = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mNextOnRequestEnd = NULL;
    mBug = dItem::ITEM_ID_NONE;
    mPrize = dItem::ITEM_ID_NONE;
    mScore = 0;
    mGrade = 0;
    mNewRecord = 0;
}

namespace dANpcSpBug {
// Simulates the villager entries up to now and shares the standings.
static inline void advanceBugOff(dAcNpcSpBug_c *owner) {
    if (getBugOff()->advance(owner->isForceAdvance()) &&
        dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        dNet::beginPacket();
        dNet::addPacket(getBugOff(), sizeof(dBugOff_c));
        dNet::sendPacket(NET_PACKET_BUG_OFF, NET_MEMBER_NONE);
    }
}

} // namespace dANpcSpBug

// .text 0xDC8

void dAcNpcSpBug_c::talk_c::start(dActor_c *target) {
    dANpcSpBug::advanceBugOff(mpOwner);
    mpOwner->mDaub.mForceAdvance = 0;
    dANpcSpBug::getBugOff()->getItem(0);

    init();
    mpActActor = target;
    if (mpOwner->isBugOffNotStarted()) {
        if (dNpcInfo::bug_c::get()->isTalkedBeforeStart()) {
            mMsgCode = MSG_BEFORE_START_AGAIN;
        } else {
            mMsgCode = MSG_BEFORE_START;
        }
    } else if (mpOwner->isBugOffOver()) {
        if (*dANpcSpBug::getBugOff()->getPlayer(0) ==
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
            if (dAcNpc_c::isPlayerFromThisTown()) {
                if (dANpcSpBug::getBugOff()->mLettersSent) {
                    mMsgCode = MSG_RESULT_WON_SENT;
                } else {
                    mMsgCode = MSG_RESULT_WON;
                }
            } else {
                mMsgCode = MSG_RESULT_WON_VISITOR;
            }
        } else if (dANpcSpBug::getBugOff()->getPlayer(0)->isValid() &&
                   dANpcSpBug::getBugOff()->getPlayer(0)->land !=
                       *dSaveData_c::getTownLand()) {
            mMsgCode = MSG_RESULT_OTHER_TOWN;
        } else if (dANpcSpBug::getBugOff()->getAnimal(0)->isValid()) {
            mMsgCode = MSG_RESULT_VILLAGER;
        } else {
            mMsgCode = MSG_RESULT_PLAYER;
        }
        if (dNpcInfo::bug_c::get()->isResultHeard()) {
            mMsgCode++; // the short version
        }
    } else if (dNpcInfo::bug_c::get()->isJoined()) {
        mMsgCode = MSG_ASK_BUG;
    } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                   dANpcSpBug::isNet, NULL) > 0 ||
               dANpcSpBug::isNet(
                   &dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHeld)) {
        mMsgCode = MSG_GREETING;
    } else {
        mMsgCode = MSG_GREETING_NO_NET;
    }
}

// .text 0x1104
void dAcNpcSpBug_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_bug";
    info->mCode = mMsgCode;
}

// .text 0x111C
void dAcNpcSpBug_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_OFFER_NET: {
        dMoney_c::onForceShow();
        dItem::Item rod(dItem::ITEM_IDX_FISHING_ROD);
        setNumber(rod.getPrice(), 0, 7, dScript::NUM_FORMAT_REGION);
        break;
    }
    case MSG_EAT:
        mpOwner->mAudioObj.startSound(SE_Eat_Gokun);
        break;
    case MSG_PRIZE: {
        dItemSel::Range_c range(dItem::KIND_FTR, 5);
        dItemSel::select(&mPrize, 1, &range, 1, &dItemSel::sDefaultFilter, NULL,
                         0, NULL);
        break;
    }
    }
    if (dANpcSpBug::getBugOff()->getItem(0).isValid()) {
        switch (mMessageCode) {
        case MSG_BEAT_OWN:
        case MSG_LEAD_FROM_VILLAGER:
        case MSG_LEAD_FROM_PLAYER:
            setItemName(dANpcSpBug::getBugOff()->getItem(1), 0);
            break;
        default:
            setItemName(dANpcSpBug::getBugOff()->getItem(0), 0);
            break;
        }
    }

    if (mPrize.mId != dItem::ITEM_ID_NONE) {
        setItemName(mPrize, 7);
    }
    dItem::Item net(dItem::ITEM_IDX_NET);
    setNumber(net.getPrice(), 3, 7, dScript::NUM_FORMAT_REGION);
    if (mBug.mId != dItem::ITEM_ID_NONE) {
        setItemName(mBug, 4);
    }
    switch (mMessageCode) {
    case MSG_BEAT_OWN:
    case MSG_LEAD_FROM_VILLAGER:
    case MSG_LEAD_FROM_PLAYER:
        if (dANpcSpBug::getBugOff()->getPlayer(1)->isValid()) {
            setPersonalName(dANpcSpBug::getBugOff()->getPlayer(1), 2);
        } else if (dANpcSpBug::getBugOff()->getAnimal(1)->isValid()) {
            setAnmPersonalName(dANpcSpBug::getBugOff()->getAnimal(1), 2);
        }
        formatNumber(dANpcSpBug::getBugOff()->getScore(1), 1, 6,
                     dScript::NUM_FORMAT_REGION);
        break;
    default:
        if (dANpcSpBug::getBugOff()->getPlayer(0)->isValid()) {
            setPersonalName(dANpcSpBug::getBugOff()->getPlayer(0), 2);
            mpController->setLandName(
                8, &dANpcSpBug::getBugOff()->getPlayer(0)->land);
        } else if (dANpcSpBug::getBugOff()->getAnimal(0)->isValid()) {
            setAnmPersonalName(dANpcSpBug::getBugOff()->getAnimal(0), 2);
        }
        formatNumber(dANpcSpBug::getBugOff()->getScore(0), 1, 6,
                     dScript::NUM_FORMAT_REGION);
        break;
    }
    formatNumber(mScore, 5, 6, dScript::NUM_FORMAT_REGION);
}

namespace dANpcSpBug {
// The rank of a's score is below the rank of b's.
static inline bool isLowerRank(const dItem::Item &a, const dItem::Item &b) {
    int rankA = dBugOff_c::getRank(dInsectInfo::getBugOffScore(&a));
    return rankA < dBugOff_c::getRank(dInsectInfo::getBugOffScore(&b));
}
} // namespace dANpcSpBug

// .text 0x14DC
void dAcNpcSpBug_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_NET_NO_MONEY:
    case MSG_NET_POCKETS_FULL:
    case MSG_NET_DECLINED:
        dMoney_c::offForceShow();
        break;
    }
    switch (mMessageCode) {
    case MSG_NET_SOLD:
        mOnMessageEnd = &talk_c::sellNet;
        mOnRequestEnd = &talk_c::pickUpNet;
        mpController->setNextMsgCode(MSG_ABOUT);
        break;
    case MSG_EXPLAINED:
    case MSG_KNOW_DRILL:
        dNpcInfo::bug_c::get()->setJoined();
        break;
    case MSG_START_TIME:
        dNpcInfo::bug_c::get()->setTalkedBeforeStart();
        break;
    case MSG_RESULT_WON:
    case MSG_RESULT_WON_VISITOR:
    case MSG_RESULT_OTHER_TOWN:
    case MSG_RESULT_VILLAGER:
    case MSG_RESULT_PLAYER:
    case MSG_RESULT_WON_SENT:
        dNpcInfo::bug_c::get()->setResultHeard();
        break;
    case MSG_KEEP_HUNTING:
        dNpcInfo::bug_c::get()->setJudged();
        break;
    case MSG_NO_BUG_LEADER:
        if (dANpcSpBug::getBugOff()->getAnimal(0)->isValid()) {
            mpController->setNextMsgCode(MSG_LEADER_VILLAGER);
        } else {
            mpController->setNextMsgCode(MSG_LEADER_PLAYER);
        }
        break;
    case MSG_SHOW_BUG:
        mOnMessageEnd = &talk_c::openBugSelect;
        mOnRequestEnd = &talk_c::onBugSelected;
        break;
    case MSG_EXQUISITE:
        mpController->setNextMsgCode(MSG_ASK_KEEP);
        break;
    case MSG_SCORE:
        if (mNewRecord) {
            if (*dANpcSpBug::getBugOff()->getPlayer(1) ==
                dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
                mpController->setNextMsgCode(MSG_BEAT_OWN);
            } else if (dANpcSpBug::getBugOff()->getAnimal(1)->isValid()) {
                mpController->setNextMsgCode(MSG_LEAD_FROM_VILLAGER);
            } else {
                mpController->setNextMsgCode(MSG_LEAD_FROM_PLAYER);
            }
        } else if (dANpcSpBug::isPlayerLeader()) {
            mpController->setNextMsgCode(MSG_STILL_LEADING);
        } else if (dANpcSpBug::getBugOff()->getAnimal(0)->isValid()) {
            if (dANpcSpBug::isLowerRank(mBug,
                                        dANpcSpBug::getBugOff()->getItem(0))) {
                mpController->setNextMsgCode(MSG_LOWER_RANK_VILLAGER);
            } else {
                mpController->setNextMsgCode(MSG_NOT_BEATEN_VILLAGER);
            }
        } else {
            if (dANpcSpBug::isLowerRank(mBug,
                                        dANpcSpBug::getBugOff()->getItem(0))) {
                mpController->setNextMsgCode(MSG_LOWER_RANK_PLAYER);
            } else {
                mpController->setNextMsgCode(MSG_NOT_BEATEN_PLAYER);
            }
        }
        break;
    case MSG_EAT:
    case MSG_HEART_RACING:
        mpController->setNextMsgCode(MSG_PRIZE);
        mOnMessageEnd = &talk_c::handActD;
        mOnRequestEnd = &talk_c::setForceNext;
        break;
    case MSG_TAKE_BUG:
        if (dNpcInfo::bug_c::get()->isJudged()) {
            mpController->setNextMsgCode(MSG_KEEP_HUNTING_AGAIN);
        } else {
            mpController->setNextMsgCode(MSG_KEEP_HUNTING);
        }
        mOnMessageEnd = &talk_c::handActD;
        mOnRequestEnd = &talk_c::setForceNext;
        break;
    case MSG_PRIZE:
        mpController->setNextMsgCode(MSG_HUNT_MORE);
        mOnMessageEnd = &talk_c::givePrize;
        mOnRequestEnd = &talk_c::pickUpPrize;
        break;
    case MSG_KEEP_REFUSED:
        mpController->setNextMsgCode(MSG_CHANGE_MIND);
        mOnMessageEnd = &talk_c::handActE;
        mOnRequestEnd = &talk_c::setForceNext2;
        break;
    case MSG_CHANGE_MIND:
        if (dANpcSpBug::isPlayerLeader()) {
            mpController->setNextMsgCode(MSG_RECORD_YOURS);
        } else if (dANpcSpBug::getBugOff()->getAnimal(0)->isValid()) {
            mpController->setNextMsgCode(MSG_RECORD_VILLAGER);
        } else {
            mpController->setNextMsgCode(MSG_RECORD_PLAYER);
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

namespace dANpcSpBug {
// The player has the money (signed >= as a value: subfc / adde).
static inline BOOL canAfford(int price) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketMoney() >= price;
}
} // namespace dANpcSpBug

// .text 0x1B30
void dAcNpcSpBug_c::talk_c::onAnswer(int arg) {
    u8 answer = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_OFFER_NET:
        switch (answer) {
        case ANSWER_YES: {
            dItem::Item net(dItem::ITEM_IDX_NET);
            int price = net.getPrice();
            if (!dANpcSpBug::canAfford(price)) {
                mpController->setNextMsgCode(MSG_NET_NO_MONEY);
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(
                           0) < 0) {
                mpController->setNextMsgCode(MSG_NET_POCKETS_FULL);
            } else {
                mpController->setNextMsgCode(MSG_NET_SOLD);
            }
            break;
        }
        case ANSWER_NO:
            break;
        }
        break;
    case MSG_ASK_BUG:
        switch (answer) {
        case ANSWER_YES:
            break;
        case ANSWER_NO:
            if (*dANpcSpBug::getBugOff()->getPlayer(0) ==
                dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
                mpController->setNextMsgCode(MSG_NO_BUG_LEADING);
            } else {
                mpController->setNextMsgCode(MSG_NO_BUG);
            }
            break;
        }
        break;
    case MSG_ASK_KEEP:
        switch (answer) {
        case ANSWER_YES: {
            dItem::Item none;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(
                &none, getMenuSelSlot(), FALSE);
            if (dSaveData_c::getTown()->mBugOff.entryPlayer(
                    &dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID, &mBug,
                    &mScore, &mGrade)) {
                mNewRecord = 1;
                if (isMultiPlay()) {
                    dNet::beginPacket();
                    dNet::addPacket(dANpcSpBug::getBugOff(), sizeof(dBugOff_c));
                    dNet::sendPacket(NET_PACKET_BUG_OFF, NET_MEMBER_NONE);
                }
            }
            switch (mGrade) {
            case 0:
                mpController->setNextMsgCode(MSG_GRADE);
                break;
            case 1:
                mpController->setNextMsgCode(MSG_GRADE + 1);
                break;
            case 2:
                mpController->setNextMsgCode(MSG_GRADE + 2);
                break;
            }
            break;
        }
        case ANSWER_NO:
            mpController->setNextMsgCode(MSG_KEEP_REFUSED);
            break;
        }
        break;
    }
}

// .text 0x1DE4
void dAcNpcSpBug_c::talk_c::preExecute() {
}

// .text 0x1DE8
void dAcNpcSpBug_c::talk_c::onTalkEnd() {
}

// .text 0x1DEC (+ a dead blr at 0x1E1C)
void dAcNpcSpBug_c::talk_c::onCustomTag(int no) {
    if (no == 0 && !mpController->mSeq.mFast) {
        mpOwner->mAudioObj.startSound(SE_HyperboleScript);
    }
}

// .text 0x1E20
void dAcNpcSpBug_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
        if (mNextOnRequestEnd) {
            mOnRequestEnd = mNextOnRequestEnd;
        }
        mNextOnRequestEnd = NULL;
    }
}

// .text 0x1EC8
void dAcNpcSpBug_c::talk_c::takeBug() {
    requestItemActEx(7, &mBug, 0, 0, 0, 2);
}

// .text 0x1EE4
void dAcNpcSpBug_c::talk_c::handActD() {
    requestHandActD();
}

// .text 0x1EE8
void dAcNpcSpBug_c::talk_c::givePrize() {
    requestItemAct(&mPrize, 0, 0);
}

// .text 0x1EF8
void dAcNpcSpBug_c::talk_c::sellNet() {
    dItem::Item net(dItem::ITEM_IDX_NET);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(net.getPrice(), FALSE);
    requestItemAct(&net, 0, 0);
}

// .text 0x1F64
void dAcNpcSpBug_c::talk_c::startThinking() {
    requestHandAct11();
    mpOwner->mAudioObj.startSound(SE_Thinking);
}

namespace dANpcSpBug {
// .text 0x1FA8: the item select filter, only the insects can be picked.
static BOOL isNotInsect(void *pocket, int flag) {
    if (flag) {
        return TRUE;
    }
    switch (static_cast<dItem::Item *>(pocket)->getKind()) {
    case dItem::KIND_INSECT:
        return FALSE;
    default:
        return TRUE;
    }
}
} // namespace dANpcSpBug

// .text 0x1FF0
void dAcNpcSpBug_c::talk_c::openBugSelect() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(0xFFFF);
    }
    reqSelectItem(dANpcSpBug::isNotInsect, 0xE7, dMenuMng_c::MENU_ITEM_GIVE_INSECT, 1);
}

// .text 0x2058
void dAcNpcSpBug_c::talk_c::onBugSelected() {
    if (isMenuInvalid()) {
        if (*dANpcSpBug::getBugOff()->getPlayer(0) ==
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
            mpController->setNextMsgCode(MSG_NO_BUG_LEADING);
        } else {
            mpController->setNextMsgCode(MSG_NO_BUG);
        }
        reqMsgClose();
    } else {
        int slot = getMenuSelSlot();
        mBug = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[slot];
        switch (dBugOff_c::getRank(dInsectInfo::getBugOffScore(&mBug))) {
        case 1:
            mpController->setNextMsgCode(MSG_RANK);
            break;
        case 2:
            mpController->setNextMsgCode(MSG_RANK + 1);
            break;
        case 3:
            mpController->setNextMsgCode(MSG_RANK + 2);
            break;
        case 4:
            mpController->setNextMsgCode(MSG_RANK + 3);
            break;
        case 5:
        case 6:
            mpController->setNextMsgCode(MSG_RANK + 4);
            break;
        }
        takeBug();
        mNextOnRequestEnd = &talk_c::judgeBug;
    }
}

// .text 0x2214
void dAcNpcSpBug_c::talk_c::judgeBug() {
    startThinking();
    mNextOnRequestEnd = &talk_c::closeMsg;
}

// .text 0x225C
void dAcNpcSpBug_c::talk_c::setForceNext() {
    mpController->mForceNext = 1;
}

// .text 0x226C
void dAcNpcSpBug_c::talk_c::pickUpPrize() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mPrize, FALSE);
}

// .text 0x22A4
void dAcNpcSpBug_c::talk_c::pickUpNet() {
    dItem::Item net(dItem::ITEM_IDX_NET);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(net, FALSE);
    dNpcInfo::bug_c::get()->setJoined();
    dMoney_c::offForceShow();
}

// .text 0x22E8
void dAcNpcSpBug_c::talk_c::closeMsg() {
    reqMsgClose();
}

// .text 0x22EC
int dAcNpcSpBug_c::getEarType() {
    return 0;
}

// .text 0x22F4
void dAcNpcSpBug_c::talk_c::handActE() {
    requestHandActE();
}

// .text 0x22F8
void dAcNpcSpBug_c::talk_c::setForceNext2() {
    mpController->mForceNext = 1;
}
