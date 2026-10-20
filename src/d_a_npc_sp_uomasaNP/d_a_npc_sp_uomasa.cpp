#include <game/game/d_a_npc_sp_uomasa.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_ev_fishing.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_fish_info.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/snd/snd_id.hpp>

// Chip at the fishing tourney. Functions are in target .text order.

namespace dANpcSpUomasa {
static inline dEvFishing_c *getFishing() {
    dSaveTown_c *town = dSaveData_c::getTown();
    return &town->mFishing;
}
// The current player holds the record.
static inline BOOL isPlayerLeader() {
    return *getFishing()->getPlayer(0) ==
           dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID;
}
// The current player held the record before.
static inline BOOL isPlayerRunnerUp() {
    return *getFishing()->getPlayer(1) ==
           dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID;
}
// The fish's index in its kind (MSG_DISH + n, dFishInfo).
static inline int getFishType(const dItem::Item &fish) {
    return dItem::seeker_c::get()->findLike(fish);
}

// .text 0x5C: a fishing rod of any grade.
static BOOL isRod(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_FISHINGROD ||
           item->getKind() == dItem::KIND_SILVER_FISHINGROD ||
           item->getKind() == dItem::KIND_GOLD_FISHINGROD;
}

// .text 0xC4: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpUomasa_c *npc) {
    return FALSE;
}

// .text 0xCC
BASE_PROFILE(A_NPC_SP_UOMASA, dAcNpcSpUomasa_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpUomasa

// .text 0x248
void dAcNpcSpUomasa_c::onDemoCmd(int cmd, u8 member) {
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
            mUomasaTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpUomasa {
static const dAcNpcSpUomasa_c::State_s l_states[] = {
    {&dAcNpcSpUomasa_c::initIdle, &dAcNpcSpUomasa_c::executeIdle},
    {&dAcNpcSpUomasa_c::initLook, &dAcNpcSpUomasa_c::executeLook},
    {&dAcNpcSpUomasa_c::initTalk, &dAcNpcSpUomasa_c::executeTalk},
    {&dAcNpcSpUomasa_c::initRemote, &dAcNpcSpUomasa_c::executeRemote},
    {&dAcNpcSpUomasa_c::initRemoteTalk, &dAcNpcSpUomasa_c::executeRemoteTalk}};
} // namespace dANpcSpUomasa

// .text 0x364
void dAcNpcSpUomasa_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpUomasa::l_states[state].mInit)();
}

// .text 0x39C
int dAcNpcSpUomasa_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mUomasaTalk);
    return 1;
}

// .text 0x3E8
int dAcNpcSpUomasa_c::create() {
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
        if (isTourneyTime()) {
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
    dANpcSpUomasa::getFishing()->mNoticePending = TRUE;

    dSceneCheck::fn_80081858(dANpcSpUomasa::entryDebugPage(this));
    if (mIsActive) {
        moveFtrToPoliceBox();
    }
    return 1;
}

// .text 0x514
int dAcNpcSpUomasa_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return 0;
    }
    dANpcSpUomasa::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return 1;
}

// .text 0x560
int dAcNpcSpUomasa_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (isLocalOwner() && dNet::isSceneOwner() &&
        mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) { // every member
        mDaub.mForceAdvance = 1;
        if (isTourneyTime()) {
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
        (this->*dANpcSpUomasa::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x690
void dAcNpcSpUomasa_c::initIdle() {
    mAction.requestTurn(PRIORITY_IDLE, mAng(0), l_turnSpeed, 0,
                        cNpcMorphFrames);
    mStep[STATE_IDLE] = IDLE_STEP_TURN;
}

// .text 0x6F0
void dAcNpcSpUomasa_c::stepIdleTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
        mStep[STATE_IDLE] = IDLE_STEP_WAIT;
    }
}

// .text 0x74C
void dAcNpcSpUomasa_c::stepIdleWait() {
}

namespace dANpcSpUomasa {
static const dAcNpcSpUomasa_c::Proc l_idleSteps[] = {
    &dAcNpcSpUomasa_c::stepIdleTurn, &dAcNpcSpUomasa_c::stepIdleWait};
} // namespace dANpcSpUomasa

// .text 0x750
void dAcNpcSpUomasa_c::executeIdle() {
    if (mStep[STATE_IDLE] < IDLE_STEP_DONE) {
        (this->*dANpcSpUomasa::l_idleSteps[mStep[STATE_IDLE]])();
    }
}

// .text 0x790
void dAcNpcSpUomasa_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep[STATE_LOOK] = LOOK_STEP_TURN;
}

// .text 0x7F4
void dAcNpcSpUomasa_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep[STATE_LOOK]++;
    }
}

namespace dANpcSpUomasa {
static const dAcNpcSpUomasa_c::Proc l_lookSteps[] = {
    &dAcNpcSpUomasa_c::stepLook};
} // namespace dANpcSpUomasa

// .text 0x854
void dAcNpcSpUomasa_c::executeLook() {
    if (mStep[STATE_LOOK] < LOOK_STEP_DONE) {
        (this->*dANpcSpUomasa::l_lookSteps[mStep[STATE_LOOK]])();
    }
}

// .text 0x894
void dAcNpcSpUomasa_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep[STATE_TALK] = TALK_STEP_TURN;
}

// .text 0x900
void dAcNpcSpUomasa_c::stepTalkTurn() {
    if (!mTalk.isBusy()) {
        mAction.requestTurn(PRIORITY_DEMO, mAng(0), l_turnSpeed, 0,
                            cNpcMorphFrames);
        mStep[STATE_TALK]++;
    }
}

// .text 0x970
void dAcNpcSpUomasa_c::stepTalkTurnEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mStep[STATE_TALK]++;
    }
}

namespace dANpcSpUomasa {
static const dAcNpcSpUomasa_c::Proc l_talkSteps[] = {
    &dAcNpcSpUomasa_c::stepTalkTurn, &dAcNpcSpUomasa_c::stepTalkTurnEnd};
} // namespace dANpcSpUomasa

// .text 0x9C0
void dAcNpcSpUomasa_c::executeTalk() {
    if (mStep[STATE_TALK] < TALK_STEP_DONE) {
        (this->*dANpcSpUomasa::l_talkSteps[mStep[STATE_TALK]])();
    }
}

// .text 0xA00
void dAcNpcSpUomasa_c::initRemote() {
}

// .text 0xA04
void dAcNpcSpUomasa_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mUomasaTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
}

// .text 0xA98
void dAcNpcSpUomasa_c::initRemoteTalk() {
}

// .text 0xA9C
void dAcNpcSpUomasa_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0xAF0
bool dAcNpcSpUomasa_c::isTourneyOngoing() {
    return dEvent::isOngoing(EVENT_FISHING_TOURNEY);
}

// .text 0xB24
bool dAcNpcSpUomasa_c::isTourneyNotStarted() {
    return dEvent::isNotStarted(EVENT_FISHING_TOURNEY);
}

// .text 0xB58
bool dAcNpcSpUomasa_c::isTourneyOver() {
    return dEvent::isOver(EVENT_FISHING_TOURNEY);
}

// .text 0xB8C: from before the start until six hours after the end.
BOOL dAcNpcSpUomasa_c::isTourneyTime() {
    return isTourneyNotStarted() || isTourneyOngoing() ||
           dEvent::isOverWithin(EVENT_FISHING_TOURNEY, 6, 0);
}

// .text 0xC0C
BOOL dAcNpcSpUomasa_c::isForceAdvance() {
    return isTourneyOngoing() && mDaub.mForceAdvance;
}

// .text 0xC60
dAcNpcSpUomasa_c::talk_c::talk_c(dAcNpcSpUomasa_c *npc) : mpOwner(npc) {
    init();
}

// .text 0xCD0
dAcNpcSpUomasa_c::talk_c::~talk_c() {
}

// .text 0xD28
void dAcNpcSpUomasa_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgCode = 0;
    mAfterDish = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mNextOnRequestEnd = NULL;
    mFish = dItem::ITEM_ID_NONE;
    mPrize = dItem::ITEM_ID_NONE;
    mSize = 0;
    mNewRecord = 0;
    mVoiceMode = VOICE_MODE_ANIMALESE;
}

namespace dANpcSpUomasa {
// Simulates the villager entries up to now and shares the standings.
static inline void advanceFishing(dAcNpcSpUomasa_c *owner) {
    if (getFishing()->advance(owner->isForceAdvance()) &&
        dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        dNet::beginPacket();
        dNet::addPacket(getFishing(), sizeof(dEvFishing_c));
        dNet::sendPacket(NET_PACKET_FISHING_RECORD, NET_MEMBER_NONE);
    }
}
} // namespace dANpcSpUomasa

// .text 0xDC8
void dAcNpcSpUomasa_c::talk_c::start(dActor_c *target) {
    dANpcSpUomasa::advanceFishing(mpOwner);
    mpOwner->mDaub.mForceAdvance = 0;
    dANpcSpUomasa::getFishing()->getItem(0);

    init();
    mpActActor = target;
    if (mpOwner->isTourneyNotStarted()) {
        if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
            mMsgCode = MSG_BEFORE_START_ANY;
        } else {
            mMsgCode = MSG_BEFORE_START;
        }
        if (dNpcInfo::uomasa_c::get()->isTalkedBeforeStart()) {
            mMsgCode++; // the short version
        }
    } else if (mpOwner->isTourneyOver()) {
        if (*dANpcSpUomasa::getFishing()->getPlayer(0) ==
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
            if (dAcNpc_c::isPlayerFromThisTown()) {
                if (dANpcSpUomasa::getFishing()->mLettersSent) {
                    if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
                        mMsgCode = MSG_RESULT_WON_SENT_ANY;
                    } else {
                        mMsgCode = MSG_RESULT_WON_SENT;
                    }
                } else {
                    if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
                        mMsgCode = MSG_RESULT_WON_ANY;
                    } else {
                        mMsgCode = MSG_RESULT_WON;
                    }
                }
            } else {
                if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
                    mMsgCode = MSG_RESULT_WON_VISITOR_ANY;
                } else {
                    mMsgCode = MSG_RESULT_WON_VISITOR;
                }
            }
        } else if (dANpcSpUomasa::getFishing()->getPlayer(0)->isValid() &&
                   dANpcSpUomasa::getFishing()->getPlayer(0)->land !=
                       *dSaveData_c::getTownLand()) {
            if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
                mMsgCode = MSG_RESULT_OTHER_TOWN_ANY;
            } else {
                mMsgCode = MSG_RESULT_OTHER_TOWN;
            }
        } else if (dANpcSpUomasa::getFishing()->getAnimal(0)->isValid()) {
            if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
                mMsgCode = MSG_RESULT_VILLAGER_ANY;
            } else {
                mMsgCode = MSG_RESULT_VILLAGER;
            }
        } else {
            if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
                mMsgCode = MSG_RESULT_PLAYER_ANY;
            } else {
                mMsgCode = MSG_RESULT_PLAYER;
            }
        }
        if (dNpcInfo::uomasa_c::get()->isResultHeard()) {
            mMsgCode++; // the short version
        }
    } else if (dNpcInfo::uomasa_c::get()->isJoined()) {
        if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
            mMsgCode = MSG_ASK_CATCH_ANY;
        } else {
            mMsgCode = MSG_ASK_CATCH;
        }
    } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                   dANpcSpUomasa::isRod, NULL) > 0 ||
               dANpcSpUomasa::isRod(
                   &dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHeld)) {
        if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
            mMsgCode = MSG_GREETING_ANY;
        } else {
            mMsgCode = MSG_GREETING;
        }
    } else {
        if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
            mMsgCode = MSG_OFFER_ROD_ANY;
        } else {
            mMsgCode = MSG_OFFER_ROD;
        }
    }
}

// .text 0x12E4
void dAcNpcSpUomasa_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_uomasa";
    info->mCode = mMsgCode;
}

// .text 0x12FC
void dAcNpcSpUomasa_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_OFFER_ROD:
    case MSG_OFFER_ROD_ANY: {
        dMoney_c::onForceShow();
        dItem::Item rod(dItem::ITEM_IDX_FISHING_ROD);
        setNumber(rod.getPrice(), WORD_TOURNEY_FISH, 7,
                  dScript::NUM_FORMAT_REGION);
        break;
    }
    case MSG_NEW_HOME:
    case MSG_NEW_HOME_ANY: {
        dItemSel::Range_c range(dItem::KIND_FTR, 5);
        dItemSel::select(&mPrize, 1, &range, 1, &dItemSel::sDefaultFilter, NULL,
                         0, NULL);
        break;
    }
    }
    if (dANpcSpUomasa::getFishing()->getFish().isValid()) {
        setItemName(dANpcSpUomasa::getFishing()->getFish(), WORD_TOURNEY_FISH);
    } else {
        if (dANpcSpUomasa::getFishing()->getItem(0).isValid()) {
            setItemName(dANpcSpUomasa::getFishing()->getItem(0),
                        WORD_LEADER_FISH);
        }
        if (dANpcSpUomasa::getFishing()->getItem(1).isValid()) {
            setItemName(dANpcSpUomasa::getFishing()->getItem(1),
                        WORD_RUNNER_UP_FISH);
        }
    }
    if (mPrize.isValid()) {
        setItemName(mPrize, WORD_PRIZE);
    }
    if (mFish.isValid()) {
        setItemName(mFish, WORD_FISH);
    }
    if (dANpcSpUomasa::getFishing()->getPlayer(0)->isValid()) {
        setPersonalName(dANpcSpUomasa::getFishing()->getPlayer(0),
                        WORD_LEADER_NAME);
        setLandName(&dANpcSpUomasa::getFishing()->getPlayer(0)->land,
                    WORD_LEADER_TOWN);
    } else if (dANpcSpUomasa::getFishing()->getAnimal(0)->isValid()) {
        setAnmPersonalName(dANpcSpUomasa::getFishing()->getAnimal(0),
                           WORD_LEADER_NAME);
    }
    setDecimal(
        dEvFishing_c::toDisplaySize(dANpcSpUomasa::getFishing()->getSize(0)),
        WORD_LEADER_SIZE, 1);
    switch (mMessageCode) {
    case MSG_MEASURE:
    case MSG_MEASURE_ANY:
        setDecimal(dEvFishing_c::toDisplaySize(mSize), WORD_SIZE, 1);
        break;
    case MSG_RECORD_OWN:
    case MSG_RECORD_VILLAGER:
    case MSG_RECORD_PLAYER:
    case MSG_RECORD_OWN_ANY:
    case MSG_RECORD_VILLAGER_ANY:
    case MSG_RECORD_PLAYER_ANY:
        setDecimal(dEvFishing_c::toDisplaySize(
                       dANpcSpUomasa::getFishing()->getSize(1)),
                   WORD_RUNNER_UP_SIZE, 1);
        if (dANpcSpUomasa::getFishing()->getPlayer(1)->isValid()) {
            setPersonalName(dANpcSpUomasa::getFishing()->getPlayer(1),
                            WORD_RUNNER_UP_NAME);
        } else if (dANpcSpUomasa::getFishing()->getAnimal(1)->isValid()) {
            setAnmPersonalName(dANpcSpUomasa::getFishing()->getAnimal(1),
                               WORD_RUNNER_UP_NAME);
        }
        break;
    }
}

// .text 0x1760
void dAcNpcSpUomasa_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_ROD_POCKETS_FULL:
    case MSG_ROD_NO_MONEY:
    case MSG_ROD_DECLINED:
    case MSG_ROD_POCKETS_FULL_ANY:
    case MSG_ROD_NO_MONEY_ANY:
    case MSG_ROD_DECLINED_ANY:
        dMoney_c::offForceShow();
        break;
    }
    switch (mMessageCode) {
    case MSG_ROD_SOLD:
    case MSG_ROD_SOLD_ANY:
        mOnMessageEnd = &talk_c::sellRod;
        mOnRequestEnd = &talk_c::pickUpRod;
        mpController->setNextMsgCode(mMessageCode + 1);
        break;
    case MSG_NO_CATCH_LEADING:
    case MSG_NO_CATCH_LEADING_ANY:
        mpController->setNextMsgCode(mMessageCode + 1);
        break;
    case MSG_RULES_DONE:
    case MSG_NO_RULES:
    case MSG_ENOUGH_RULES:
    case MSG_RULES_DONE_ANY:
    case MSG_NO_RULES_ANY:
        dNpcInfo::uomasa_c::get()->setJoined();
        break;
    case MSG_BEFORE_START:
    case MSG_BEFORE_START_ANY:
        dNpcInfo::uomasa_c::get()->setTalkedBeforeStart();
        break;
    case MSG_RESULT_WON:
    case MSG_RESULT_WON_VISITOR:
    case MSG_RESULT_OTHER_TOWN:
    case MSG_RESULT_VILLAGER:
    case MSG_RESULT_WON_ANY:
    case MSG_RESULT_WON_VISITOR_ANY:
    case MSG_RESULT_OTHER_TOWN_ANY:
    case MSG_RESULT_VILLAGER_ANY:
    case MSG_RESULT_PLAYER:
    case MSG_RESULT_PLAYER_ANY:
    case MSG_RESULT_WON_SENT:
    case MSG_RESULT_WON_SENT_ANY:
        dNpcInfo::uomasa_c::get()->setResultHeard();
        break;
    case MSG_NO_CATCH:
        if (dANpcSpUomasa::getFishing()->getAnimal(0)->isValid()) {
            mpController->setNextMsgCode(MSG_LEADER_VILLAGER);
        } else {
            mpController->setNextMsgCode(MSG_LEADER_PLAYER);
        }
        break;
    case MSG_NO_CATCH_ANY:
        if (dANpcSpUomasa::getFishing()->getAnimal(0)->isValid()) {
            mpController->setNextMsgCode(MSG_LEADER_VILLAGER_ANY);
        } else {
            mpController->setNextMsgCode(MSG_LEADER_PLAYER_ANY);
        }
        break;
    case MSG_SHOW_FISH:
    case MSG_SHOW_FISH_ANY:
        mOnMessageEnd = &talk_c::openFishSelect;
        mOnRequestEnd = &talk_c::onFishSelected;
        break;
    case MSG_MEASURE:
    case MSG_MEASURE_ANY:
        mOnMessageEnd = &talk_c::startMeasure;
        mOnRequestEnd = &talk_c::onMeasured;
        break;
    case MSG_SIZE:
        if (mNewRecord) {
            if (*dANpcSpUomasa::getFishing()->getPlayer(1) ==
                dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
                mpController->setNextMsgCode(MSG_RECORD_OWN);
            } else if (dANpcSpUomasa::getFishing()->getAnimal(1)->isValid()) {
                mpController->setNextMsgCode(MSG_RECORD_VILLAGER);
            } else {
                mpController->setNextMsgCode(MSG_RECORD_PLAYER);
            }
        } else if (dANpcSpUomasa::isPlayerLeader()) {
            mpController->setNextMsgCode(MSG_STILL_LEADING);
        } else if (dANpcSpUomasa::getFishing()->getAnimal(0)->isValid()) {
            mpController->setNextMsgCode(MSG_NOT_BEATEN_VILLAGER);
        } else {
            mpController->setNextMsgCode(MSG_NOT_BEATEN_PLAYER);
        }
        break;
    case MSG_SIZE_ANY:
        if (mNewRecord) {
            if (dANpcSpUomasa::isPlayerRunnerUp()) {
                mpController->setNextMsgCode(MSG_RECORD_OWN_ANY);
            } else if (dANpcSpUomasa::getFishing()->getAnimal(1)->isValid()) {
                mpController->setNextMsgCode(MSG_RECORD_VILLAGER_ANY);
            } else {
                mpController->setNextMsgCode(MSG_RECORD_PLAYER_ANY);
            }
        } else if (dANpcSpUomasa::isPlayerLeader()) {
            mpController->setNextMsgCode(MSG_STILL_LEADING_ANY);
        } else if (dANpcSpUomasa::getFishing()->getAnimal(0)->isValid()) {
            mpController->setNextMsgCode(MSG_NOT_BEATEN_VILLAGER_ANY);
        } else {
            mpController->setNextMsgCode(MSG_NOT_BEATEN_PLAYER_ANY);
        }
        break;
    case MSG_WRONG_FISH:
    case MSG_RECORD_OWN_ANY:
    case MSG_RECORD_VILLAGER_ANY:
    case MSG_STILL_LEADING_ANY:
    case MSG_NOT_BEATEN_VILLAGER_ANY:
    case MSG_RECORD_PLAYER_ANY:
    case MSG_NOT_BEATEN_PLAYER_ANY:
        // His recipe of the fish first.
        switch (mMessageCode) {
        case MSG_WRONG_FISH:
            mAfterDish = MSG_ASK_KEEP;
            break;
        case MSG_RECORD_OWN_ANY:
        case MSG_RECORD_VILLAGER_ANY:
        case MSG_RECORD_PLAYER_ANY:
            mAfterDish = MSG_NEW_HOME_ANY;
            break;
        case MSG_STILL_LEADING_ANY:
        case MSG_NOT_BEATEN_VILLAGER_ANY:
        case MSG_NOT_BEATEN_PLAYER_ANY:
            mAfterDish = MSG_TAKE_FISH_ANY;
            break;
        }
        mpController->setNextMsgCode(dANpcSpUomasa::getFishType(mFish) +
                                     MSG_DISH);
        break;
    case MSG_RECORD_OWN:
    case MSG_RECORD_VILLAGER:
    case MSG_RECORD_PLAYER:
        mpController->setNextMsgCode(MSG_NEW_HOME);
        break;
    case MSG_NEW_HOME:
        if (dFishInfo::isLarge(dANpcSpUomasa::getFishType(mFish))) {
            mpController->setNextMsgCode(MSG_RARE_PRIZE);
            mOnMessageEnd = &talk_c::keepFish;
            mOnRequestEnd = &talk_c::setForceNextKeep;
        } else {
            mpController->setNextMsgCode(MSG_EAT_PRIZE);
            mOnMessageEnd = &talk_c::eatFish;
            mOnRequestEnd = &talk_c::setForceNextEat;
        }
        break;
    case MSG_STILL_LEADING:
    case MSG_NOT_BEATEN_VILLAGER:
    case MSG_NOT_BEATEN_PLAYER:
        mpController->setNextMsgCode(MSG_TAKE_FISH);
        break;
    case MSG_TAKE_FISH:
        if (dFishInfo::isLarge(dANpcSpUomasa::getFishType(mFish))) {
            mpController->setNextMsgCode(MSG_RARE_TAKEN);
            mOnMessageEnd = &talk_c::keepFish;
            mOnRequestEnd = &talk_c::setForceNextKeep;
        } else {
            mpController->setNextMsgCode(MSG_EAT);
            mOnMessageEnd = &talk_c::eatFish;
            mOnRequestEnd = &talk_c::setForceNextEat;
        }
        break;
    case MSG_KEEP_OK:
        mpController->setNextMsgCode(mMessageCode + 1);
        mOnMessageEnd = &talk_c::eatFish;
        mOnRequestEnd = &talk_c::setForceNextEat;
        break;
    case MSG_NEW_HOME_ANY:
        if (dFishInfo::isLarge(dANpcSpUomasa::getFishType(mFish))) {
            mpController->setNextMsgCode(MSG_RARE_PRIZE_ANY);
            mOnMessageEnd = &talk_c::keepFish;
            mOnRequestEnd = &talk_c::setForceNextKeep;
        } else {
            mpController->setNextMsgCode(MSG_EAT_PRIZE_ANY);
            mOnMessageEnd = &talk_c::eatFish;
            mOnRequestEnd = &talk_c::setForceNextEat;
        }
        break;
    case MSG_TAKE_FISH_ANY:
        if (dFishInfo::isLarge(dANpcSpUomasa::getFishType(mFish))) {
            mpController->setNextMsgCode(MSG_RARE_TAKEN_ANY);
            mOnMessageEnd = &talk_c::keepFish;
            mOnRequestEnd = &talk_c::setForceNextKeep;
        } else {
            mpController->setNextMsgCode(MSG_EAT_ANY);
            mOnMessageEnd = &talk_c::eatFish;
            mOnRequestEnd = &talk_c::setForceNextEat;
        }
        break;
    case MSG_KEEP_REFUSED:
        mpController->setNextMsgCode(mMessageCode + 1);
        mOnMessageEnd = &talk_c::returnFish;
        mOnRequestEnd = &talk_c::pickUpFish;
        break;
    case MSG_KEEP_RARE:
        mpController->setNextMsgCode(mMessageCode + 1);
        mOnMessageEnd = &talk_c::keepFish;
        mOnRequestEnd = &talk_c::setForceNextKeep;
        break;
    case MSG_EAT_PRIZE:
    case MSG_RARE_PRIZE:
    case MSG_EAT_PRIZE_ANY:
    case MSG_RARE_PRIZE_ANY:
        mpController->setNextMsgCode(mMessageCode + 1);
        mOnMessageEnd = &talk_c::givePrize;
        mOnRequestEnd = &talk_c::pickUpPrize;
        break;
    case MSG_EAT:
    case MSG_EAT_ANY:
    case MSG_RARE_TAKEN_ANY:
        mpController->setNextMsgCode(mMessageCode + 1);
        break;
    case MSG_RARE_TAKEN:
        mpController->setNextMsgCode(MSG_KEEP_TRYING_RARE);
        break;
    }
    if (mMessageCode >= MSG_DISH && mMessageCode <= MSG_DISH_LAST) {
        mpController->setNextMsgCode(mAfterDish);
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

namespace dANpcSpUomasa {
// The player has the money (signed >= as a value: subfc / adde).
static inline BOOL canAfford(int price) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketMoney() >= price;
}
} // namespace dANpcSpUomasa

// .text 0x21EC
void dAcNpcSpUomasa_c::talk_c::onAnswer(int arg) {
    u8 answer = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_OFFER_ROD:
    case MSG_OFFER_ROD_ANY:
        switch (answer) {
        case ANSWER_BUY: {
            dItem::Item rod(dItem::ITEM_IDX_FISHING_ROD);
            int price = rod.getPrice();
            if (!dANpcSpUomasa::canAfford(price)) {
                mpController->setNextMsgCode(
                    mMessageCode + (MSG_ROD_NO_MONEY - MSG_OFFER_ROD));
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(
                           0) < 0) {
                mpController->setNextMsgCode(
                    mMessageCode + (MSG_ROD_POCKETS_FULL - MSG_OFFER_ROD));
            } else {
                mpController->setNextMsgCode(mMessageCode +
                                             (MSG_ROD_SOLD - MSG_OFFER_ROD));
            }
            break;
        }
        case ANSWER_DECLINE:
            break;
        }
        break;
    case MSG_ASK_CATCH:
    case MSG_ASK_CATCH_ANY:
        switch (answer) {
        case ANSWER_CAUGHT:
            break;
        case ANSWER_NOTHING:
            if (*dANpcSpUomasa::getFishing()->getPlayer(0) ==
                dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
                mpController->setNextMsgCode(
                    mMessageCode + (MSG_NO_CATCH_LEADING - MSG_ASK_CATCH));
            } else {
                mpController->setNextMsgCode(mMessageCode +
                                             (MSG_NO_CATCH - MSG_ASK_CATCH));
            }
            break;
        }
        break;
    case MSG_ASK_KEEP:
        switch (answer) {
        case ANSWER_MIND:
            break;
        case ANSWER_GO_AHEAD:
            if (dFishInfo::isLarge(dANpcSpUomasa::getFishType(mFish))) {
                mpController->setNextMsgCode(MSG_KEEP_RARE);
            } else {
                mpController->setNextMsgCode(MSG_KEEP_OK);
            }
            break;
        }
        break;
    }
}

// .text 0x2424
void dAcNpcSpUomasa_c::talk_c::preExecute() {
}

// .text 0x2428
void dAcNpcSpUomasa_c::talk_c::onTalkEnd() {
}

// .text 0x242C
void dAcNpcSpUomasa_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
        if (mNextOnRequestEnd) {
            mOnRequestEnd = mNextOnRequestEnd;
        }
        mNextOnRequestEnd = NULL;
    }
}

// .text 0x24D4
void dAcNpcSpUomasa_c::talk_c::takeFish() {
    requestItemActEx(7, &mFish, 0, 0, 0, 2);
}

// .text 0x24F0
void dAcNpcSpUomasa_c::talk_c::returnFish() {
    requestHandActE();
}

// .text 0x24F4
void dAcNpcSpUomasa_c::talk_c::eatFish() {
    requestHandAct10(0);
    mpOwner->mAudioObj.startSound(SE_Eat_Uomasa);
}

// .text 0x253C
void dAcNpcSpUomasa_c::talk_c::keepFish() {
    requestHandActD();
}

// .text 0x2540
void dAcNpcSpUomasa_c::talk_c::givePrize() {
    requestItemAct(&mPrize, 0, 0);
}

// .text 0x2550
void dAcNpcSpUomasa_c::talk_c::sellRod() {
    dItem::Item rod(dItem::ITEM_IDX_FISHING_ROD);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(rod.getPrice(), FALSE);
    requestItemAct(&rod, 0, 0);
}

// .text 0x25BC
void dAcNpcSpUomasa_c::talk_c::startMeasure() {
    requestHandAct11();
    mpOwner->mAudioObj.startSound(SE_Thinking);
}

namespace dANpcSpUomasa {
// .text 0x2600: the item select filter, only the fish can be picked.
static BOOL isNotFish(void *pocket, int flag) {
    if (flag) {
        return TRUE;
    }
    switch (static_cast<dItem::Item *>(pocket)->getKind()) {
    case dItem::KIND_FISH:
        return FALSE;
    default:
        return TRUE;
    }
}
} // namespace dANpcSpUomasa

// .text 0x2648
void dAcNpcSpUomasa_c::talk_c::openFishSelect() {
    if (mpController->mNextMsgCode == dMsg::MSG_CODE_NONE) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    reqSelectItem(dANpcSpUomasa::isNotFish, ITEM_SEL_MSG_GIVE, dMenuMng_c::MENU_ITEM_GIVE_FISH,
                  TRUE);
}

// .text 0x26B0
void dAcNpcSpUomasa_c::talk_c::onFishSelected() {
    if (isMenuInvalid()) {
        if (*dANpcSpUomasa::getFishing()->getPlayer(0) ==
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
            if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
                mpController->setNextMsgCode(MSG_NO_CATCH_LEADING_ANY);
            } else {
                mpController->setNextMsgCode(MSG_NO_CATCH_LEADING);
            }
        } else {
            if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
                mpController->setNextMsgCode(MSG_NO_CATCH_ANY);
            } else {
                mpController->setNextMsgCode(MSG_NO_CATCH);
            }
        }
        reqMsgClose();
    } else {
        int slot = getMenuSelSlot();
        mFish = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[slot];
        if (!dANpcSpUomasa::getFishing()->getFish().isValid()) {
            mpController->setNextMsgCode(MSG_MEASURE_ANY);
        } else if (dANpcSpUomasa::getFishing()->getFish().isSame(mFish)) {
            mpController->setNextMsgCode(MSG_MEASURE);
        } else {
            mpController->setNextMsgCode(MSG_WRONG_FISH);
        }
        if (dSaveData_c::getTown()->mFishing.entryPlayer(
                &dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID, &mFish, &mSize)) {
            mNewRecord = 1;
            if (isMultiPlay()) {
                dNet::beginPacket();
                dNet::addPacket(dANpcSpUomasa::getFishing(),
                                sizeof(dEvFishing_c));
                dNet::sendPacket(NET_PACKET_FISHING_RECORD, NET_MEMBER_NONE);
            }
        }
        takeFish();
        mNextOnRequestEnd = &talk_c::removeFish;
    }
}

// .text 0x2928
void dAcNpcSpUomasa_c::talk_c::removeFish() {
    dItem::Item none;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&none, getMenuSelSlot(),
                                                     FALSE);
    reqMsgClose();
}

// .text 0x2988
void dAcNpcSpUomasa_c::talk_c::pickUpFish() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mFish, FALSE);
}

// .text 0x29C0
void dAcNpcSpUomasa_c::talk_c::setForceNextEat() {
    mpController->mForceNext = 1;
}

// .text 0x29D0
void dAcNpcSpUomasa_c::talk_c::setForceNextKeep() {
    mpController->mForceNext = 1;
}

// .text 0x29E0
void dAcNpcSpUomasa_c::talk_c::pickUpPrize() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mPrize, FALSE);
}

// .text 0x2A18
void dAcNpcSpUomasa_c::talk_c::pickUpRod() {
    dItem::Item rod(dItem::ITEM_IDX_FISHING_ROD);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(rod, FALSE);
    dNpcInfo::uomasa_c::get()->setJoined();
    dMoney_c::offForceShow();
}

// .text 0x2A5C
void dAcNpcSpUomasa_c::talk_c::onMeasured() {
}

// .text 0x2A60
int dAcNpcSpUomasa_c::getEarType() {
    return NPC_EAR_TYPE_CHIP;
}

// .text 0x2A68: tag 0 is his burp (silent while he eats), tag 1 its end.
void dAcNpcSpUomasa_c::talk_c::onCustomTag(int no) {
    switch (no) {
    case 0:
        mpOwner->mAudioObj.startSound(SE_Eat_Gefu);
        mVoiceMode = VOICE_MODE_SILENT;
        break;
    case 1:
        mVoiceMode = VOICE_MODE_ANIMALESE;
        break;
    }
}

// .text 0x2AD4
int dAcNpcSpUomasa_c::talk_c::getVoiceMode() {
    mVoiceMode = VOICE_MODE_ANIMALESE;
    return VOICE_MODE_ANIMALESE;
}
