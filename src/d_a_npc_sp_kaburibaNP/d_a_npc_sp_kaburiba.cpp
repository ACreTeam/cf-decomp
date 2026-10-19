#include <game/game/d_a_npc_sp_kaburiba.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shop_stalk_market.hpp>
#include <game/game/d_save_town.hpp>

// Joan. Functions are in target .text order.

namespace dANpcSpKaburiba {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_KABURIBA, dAcNpcSpKaburiba_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpKaburiba

// .text 0x1CC
void dAcNpcSpKaburiba_c::onDemoCmd(int cmd, u8 member) {
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
            mKaburibaTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpKaburiba {
static const dAcNpcSpKaburiba_c::State_s l_states[] = {
    {&dAcNpcSpKaburiba_c::initIdle, &dAcNpcSpKaburiba_c::executeIdle},
    {&dAcNpcSpKaburiba_c::initLook, &dAcNpcSpKaburiba_c::executeIdle},
    {&dAcNpcSpKaburiba_c::initTalk, &dAcNpcSpKaburiba_c::executeTalk},
    {&dAcNpcSpKaburiba_c::initRemote, &dAcNpcSpKaburiba_c::executeRemote},
    {&dAcNpcSpKaburiba_c::initRemoteTalk,
     &dAcNpcSpKaburiba_c::executeRemoteTalk},
};
} // namespace dANpcSpKaburiba

// .text 0x2E8
void dAcNpcSpKaburiba_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpKaburiba::l_states[state].mInit)();
}

// .text 0x320
int dAcNpcSpKaburiba_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mKaburibaTalk);
    return 1;
}

// .text 0x36C
int dAcNpcSpKaburiba_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mObjc.setDefWeight(0x80);
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        if (!dEvent::isVisitorHere(VISITOR_JOAN)) {
            mDaub.mVisiting = FALSE;
        } else {
            mDaub.mVisiting = TRUE;
        }
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (mDaub.mVisiting) {
        mIsActive = TRUE;
    } else {
        mIsActive = FALSE;
    }
    return 1;
}

// .text 0x450
int dAcNpcSpKaburiba_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x47C
int dAcNpcSpKaburiba_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (isLocalOwner() && dNet::isSceneOwner() &&
        mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) { // every member
        if (dEvent::isVisitorHere(VISITOR_JOAN)) {
            if (!mDaub.mVisiting) {
                mDaub.mVisiting = TRUE;
                setDaubData(&mDaub, sizeof(mDaub));
            }
        } else if (mDaub.mVisiting) {
            mDaub.mVisiting = FALSE;
            setDaubData(&mDaub, sizeof(mDaub));
        }
    }
    if (mDaub.mVisiting) {
        mIsActive = TRUE;
    } else {
        mIsActive = FALSE;
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpKaburiba::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x5A0
void dAcNpcSpKaburiba_c::initIdle() {
    initWander();
}

// .text 0x5A4
void dAcNpcSpKaburiba_c::executeIdle() {
    executeWander(NULL);
}

// .text 0x5B8
void dAcNpcSpKaburiba_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
    mStep = 0;
}

// .text 0x61C
void dAcNpcSpKaburiba_c::initTalk() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep = 0;
}

// .text 0x6B4
void dAcNpcSpKaburiba_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpKaburiba {
static const dAcNpcSpKaburiba_c::Proc l_talkSteps[] = {
    &dAcNpcSpKaburiba_c::stepTalk};
} // namespace dANpcSpKaburiba

// .text 0x700
void dAcNpcSpKaburiba_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpKaburiba::l_talkSteps)) {
        (this->*dANpcSpKaburiba::l_talkSteps[mStep])();
    }
}

// .text 0x740
void dAcNpcSpKaburiba_c::initRemote() {
    resetLookTimer();
}

// .text 0x748
void dAcNpcSpKaburiba_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mKaburibaTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    executeRemoteWander(NULL);
}

// .text 0x7E8
void dAcNpcSpKaburiba_c::initRemoteTalk() {
}

// .text 0x7EC
void dAcNpcSpKaburiba_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0x840
dAcNpcSpKaburiba_c::talk_c::talk_c(dAcNpcSpKaburiba_c *npc) : mpOwner(npc) {
    init();
    mItem = dItem::Item();
}

// .text 0x8BC
dAcNpcSpKaburiba_c::talk_c::~talk_c() {
}

// .text 0x914
void dAcNpcSpKaburiba_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mOnAnswer = NULL;
    mSeedBought = FALSE;
}

// .text 0x998
void dAcNpcSpKaburiba_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (!dNpcInfo::kaburiba_c::get()->isTalked()) {
        mMsgNo = MSG_GREETING;
    } else {
        mMsgNo = MSG_WELCOME_BACK;
    }
    mNextMsg = MSG_MENU;
    dMoney_c::onForceShow();
}

// .text 0xA10
void dAcNpcSpKaburiba_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_kaburiba";
    info->mCode = mMsgNo;
}

namespace dANpcSpKaburiba {
// Joan's price of a turnip this week.
static inline int getTurnipPrice() {
    return dSaveData_c::getTown()->mShops.mStalkMarket.mBuyPrice.get();
}
// The player has the money (signed >= as a value: subfc / adde).
static inline BOOL canAfford(int price) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketMoney() >= price;
}
} // namespace dANpcSpKaburiba

// .text 0xA28
void dAcNpcSpKaburiba_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_WHITE_PRICE:
        setNumber(dANpcSpKaburiba::getTurnipPrice(), 0, 3,
                  dScript::NUM_FORMAT_REGION);
        break;
    case MSG_CONFIRM:
        setNumber(fn_80029A50() * TURNIP_BUNCH, 1, 3,
                  dScript::NUM_FORMAT_REGION);
        setNumber(dANpcSpKaburiba::getTurnipPrice() *
                      (fn_80029A50() * TURNIP_BUNCH),
                  2, 6, dScript::NUM_FORMAT_REGION);
        break;
    }
}

// .text 0xAF0
void dAcNpcSpKaburiba_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_INTRO:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOAN_MET)) {
            mpController->setNextMsgCode(MSG_OFFER_LESSON);
        } else {
            mpController->setNextMsgCode(MSG_LESSON_TOPIC);
        }
        break;
    case MSG_LESSON_DECLINED:
    case MSG_LESSON_AGAIN:
        mpController->setNextMsgCode(MSG_MENU);
        break;
    case MSG_RED_LESSON_END:
        mpController->setNextMsgCode(mNextMsg);
        break;
    case MSG_HOW_MANY:
        mOnMessageEnd = &talk_c::openNumberMenu;
        mOnRequestEnd = &talk_c::onNumberEntered;
        break;
    case MSG_THANKS:
        mOnMessageEnd = &talk_c::giveItem;
        mOnRequestEnd = &talk_c::onItemGiven;
        if (mSeedBought) {
            mpController->setNextMsgCode(MSG_RED_SEED_SOLD);
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0xC64
void dAcNpcSpKaburiba_c::talk_c::onAnswer(int arg) {
    u8 answer = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_LESSON_TOPIC:
    case MSG_MENU:
        mNextMsg = MSG_RED_SEED_OFFER;
        switch (answer) {
        case ANSWER_WHITE:
            mpController->setNextMsgCode(MSG_WHITE_PRICE);
            break;
        case ANSWER_RED:
            if (!isPlayerFromThisTown()) {
                mpController->setNextMsgCode(MSG_RED_VISITOR);
            } else if (!dSaveData_c::getRaw()->isFlag(
                           SAVE_FLAG_TURNIPS_SPOILED)) {
                mpController->setNextMsgCode(MSG_RED_MENU);
            } else {
                mpController->setNextMsgCode(MSG_RED_SOLD_OUT);
            }
            break;
        case ANSWER_NEITHER:
            mpController->setNextMsgCode(MSG_NEITHER);
            break;
        }
        break;
    case MSG_WHITE_PRICE:
    case MSG_WHITE_PRICE_AGAIN:
        switch (answer) {
        case ANSWER_BUY:
            mpController->setNextMsgCode(MSG_HOW_MANY);
            break;
        case ANSWER_EXPLAIN:
            mpController->setNextMsgCode(MSG_WHITE_LESSON);
            break;
        case ANSWER_NEVER_MIND:
            mpController->setNextMsgCode(MSG_DECLINED);
            break;
        }
        break;
    case MSG_CONFIRM: {
        int num = fn_80029A50() * TURNIP_BUNCH;
        int pockets = (num + TURNIP_POCKET - 1) / TURNIP_POCKET;
        int empty =
            dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(NULL, NULL);
        switch (answer) {
        case ANSWER_YES:
            if (!dANpcSpKaburiba::canAfford(
                    num * dANpcSpKaburiba::getTurnipPrice())) {
                mpController->setNextMsgCode(MSG_NO_MONEY);
            } else if (empty < pockets) {
                if (empty <= 0) {
                    mpController->setNextMsgCode(MSG_NO_ROOM);
                } else {
                    mpController->setNextMsgCode(MSG_TOO_MANY);
                }
            } else {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(
                    num * dANpcSpKaburiba::getTurnipPrice(), FALSE);
                while (TRUE) {
                    if (num < TURNIP_POCKET) {
                        break;
                    }
                    dItem::Item turnips(dItem::ITEM_IDX_100_TURNIPS);
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&turnips,
                                                                  FALSE);
                    num -= TURNIP_POCKET;
                }
                if (num >= TURNIP_BUNCH) {
                    dItem::Item turnips(
                        dItem::ITEM_IDX_10_TURNIPS,
                        (num - (TURNIP_BUNCH - 1)) / TURNIP_BUNCH, FALSE);
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&turnips,
                                                                  FALSE);
                }
                mItem = dItem::Item(dItem::ITEM_IDX_100_TURNIPS);
                mpController->setNextMsgCode(MSG_THANKS);
            }
            break;
        case ANSWER_NO:
            mpController->setNextMsgCode(MSG_DECLINED);
            break;
        }
        break;
    }
    case MSG_TOO_MANY:
        switch (answer) {
        case ANSWER_YES:
            mOnAnswer = &talk_c::openNumberMenu;
            mOnRequestEnd = &talk_c::onNumberEntered;
            break;
        case ANSWER_NO:
            mpController->setNextMsgCode(MSG_DECLINED);
            break;
        }
        break;
    case MSG_RED_MENU:
        switch (answer) {
        case ANSWER_BUY:
            mpController->setNextMsgCode(MSG_RED_SEED_OFFER);
            break;
        case ANSWER_EXPLAIN:
            mpController->setNextMsgCode(MSG_RED_LESSON);
            break;
        case ANSWER_NEVER_MIND:
            mpController->setNextMsgCode(MSG_DECLINED);
            break;
        }
        break;
    case MSG_RED_SEED_OFFER: {
        int empty =
            dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(NULL, NULL);
        switch (answer) {
        case ANSWER_YES:
            if (!dANpcSpKaburiba::canAfford(RED_SEED_PRICE)) {
                mpController->setNextMsgCode(MSG_NO_MONEY);
            } else if (empty < 1) {
                mpController->setNextMsgCode(MSG_NO_ROOM);
            } else {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(RED_SEED_PRICE,
                                                                FALSE);
                mItem = dItem::Item(dItem::ITEM_IDX_RED_TURNIP_SEED);
                dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&mItem, FALSE);
                dSaveData_c::getTown()->mShops.mStalkMarket.invalidate();
                mpController->setNextMsgCode(MSG_THANKS);
                mSeedBought = TRUE;
            }
            break;
        case ANSWER_NO:
            mpController->setNextMsgCode(MSG_DECLINED);
            break;
        }
        break;
    }
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x1110
void dAcNpcSpKaburiba_c::talk_c::preExecute() {
}

// .text 0x1114
void dAcNpcSpKaburiba_c::talk_c::onTalkEnd() {
    dNpcInfo::kaburiba_c::get()->setTalked();
    mItem = dItem::Item();
    dMoney_c::offForceShow();
}

// .text 0x1154
void dAcNpcSpKaburiba_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x11B8
void dAcNpcSpKaburiba_c::talk_c::giveItem() {
    requestItemAct(&mItem, 0, 0);
}

// .text 0x11C8
void dAcNpcSpKaburiba_c::talk_c::onItemGiven() {
}

// .text 0x11CC
void dAcNpcSpKaburiba_c::talk_c::openNumberMenu() {
    reqMenu11(FALSE);
}

// .text 0x11D4
void dAcNpcSpKaburiba_c::talk_c::onNumberEntered() {
    if (isMenuInvalid()) {
        mpController->setNextMsgCode(MSG_DECLINED);
    } else {
        mpController->setNextMsgCode(MSG_CONFIRM);
    }
    dMoney_c::onForceShow();
}

// .text 0x1228
int dAcNpcSpKaburiba_c::getEarType() {
    return NPC_EAR_TYPE_JOAN;
}
