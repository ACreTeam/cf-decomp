#include <game/game/d_a_npc_sp_haniwa.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_ftr.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sv_auc.hpp>

// Lloid at the auction house. Functions are in target .text order.

namespace dANpcSpHaniwa {
// .text 0x5C
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text 0x68
static dSvAucItem_c *getAucItem(u32 slot) {
    return dSvAuc_c::get()->getItem(slot);
}

// .text 0x9C
BASE_PROFILE(A_NPC_SP_HANIWA, dAcNpcSpHaniwa_c);
} // namespace dANpcSpHaniwa

// .text 0x20C
void dAcNpcSpHaniwa_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mTalkHaniwa.mMsgKind = talk_c::MSG_KIND_TALK;
        mTalkHaniwa.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        mTalkHaniwa.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpHaniwa {
static const dAcNpcSpHaniwa_c::State_s l_states[] = {
    {&dAcNpcSpHaniwa_c::initIdle, &dAcNpcSpHaniwa_c::executeIdle},
    {&dAcNpcSpHaniwa_c::initLook, &dAcNpcSpHaniwa_c::executeLook},
    {&dAcNpcSpHaniwa_c::initTalk, &dAcNpcSpHaniwa_c::executeTalk},
    {&dAcNpcSpHaniwa_c::initGreet, &dAcNpcSpHaniwa_c::executeGreet},
    {&dAcNpcSpHaniwa_c::initExit, &dAcNpcSpHaniwa_c::executeExit},
    {&dAcNpcSpHaniwa_c::initItem, &dAcNpcSpHaniwa_c::executeItem}};
} // namespace dANpcSpHaniwa

// .text 0x2E0
void dAcNpcSpHaniwa_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpHaniwa::l_states[state].mInit)();
    mAnmRate = 1.0f;
}

// .text 0x330: the closer the player, the faster the dance.
void dAcNpcSpHaniwa_c::calcAnmRate() {
    f32 dist = calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL);
    f32 rate;
    if (dist >= 96.0f) {
        rate = 0.5f;
    } else if (dist <= 32.0f) {
        rate = 1.5f;
    } else {
        rate = 1.5f - (dist - 32.0f) / 64.0f;
    }
    mAnmRate = rate;
}

// .text 0x3BC
int dAcNpcSpHaniwa_c::getEarType() {
    return NPC_EAR_TYPE_NONE;
}

// .text 0x3C4
int dAcNpcSpHaniwa_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mTalkHaniwa);
    mAnmSet.setAnm0(HMN_ANM_HNW_WAIT1);
    mAnmSet.setAnm1(HMN_ANM_HNW_WAIT1);
    mAnmSet.setAnm2(HMN_ANM_HNW_WAIT1);
    mAnmRate = 1.0f;
    return 1;
}

// .text 0x440
int dAcNpcSpHaniwa_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    dBGCF::lockNext(0);
    changeState(STATE_GREET);
    mFootstep.mEnable = 0;
    return 1;
}

// .text 0x49C
int dAcNpcSpHaniwa_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x4C8
int dAcNpcSpHaniwa_c::vtB0() {
    dSvAuc_c::updateOnline();
    if (mState < STATE_NUM) {
        (this->*dANpcSpHaniwa::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x524
void dAcNpcSpHaniwa_c::initIdle() {
    mAng angle = getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0, cNpcMorphFrames);
}

// .text 0x580
void dAcNpcSpHaniwa_c::executeIdle() {
    if (dPlayerMgr::isSceneTransitionState()) {
        return;
    }
    if (isAtExit()) {
        changeState(STATE_EXIT);
        return;
    }
    mAction.setTargetAngle(getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT));
    calcAnmRate();
    if (checkPointedItem()) {
        changeState(STATE_ITEM);
    }
}

// .text 0x618
void dAcNpcSpHaniwa_c::initLook() {
    mAng angle = getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = STEP_START;
}

// .text 0x67C
void dAcNpcSpHaniwa_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

namespace dANpcSpHaniwa {
static const dAcNpcSpHaniwa_c::Proc l_lookSteps[] = {
    &dAcNpcSpHaniwa_c::stepLook};
} // namespace dANpcSpHaniwa

// .text 0x6DC
void dAcNpcSpHaniwa_c::executeLook() {
    if (mStep < STEP_DONE) {
        (this->*dANpcSpHaniwa::l_lookSteps[mStep])();
    }
    calcAnmRate();
}

// .text 0x730
void dAcNpcSpHaniwa_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    mStep = STEP_START;
}

// .text 0x79C
void dAcNpcSpHaniwa_c::stepTalk() {
    if (!mTalk.isBusy() && !mTalkHaniwa.mExiting) {
        dDemo::finishTalk(this);
        ++mStep;
    }
}

namespace dANpcSpHaniwa {
static const dAcNpcSpHaniwa_c::Proc l_talkSteps[] = {
    &dAcNpcSpHaniwa_c::stepTalk};
} // namespace dANpcSpHaniwa

// .text 0x7F4
void dAcNpcSpHaniwa_c::executeTalk() {
    if (mStep < STEP_DONE) {
        (this->*dANpcSpHaniwa::l_talkSteps[mStep])();
    }
    calcAnmRate();
}

// .text 0x848
void dAcNpcSpHaniwa_c::initGreet() {
}

// .text 0x84C
void dAcNpcSpHaniwa_c::executeGreet() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mTalkHaniwa.mMsgKind = talk_c::MSG_KIND_GREET;
        dDemo::requestSpeak(this, NULL);
    }
    calcAnmRate();
}

// .text 0x8A0
void dAcNpcSpHaniwa_c::initExit() {
}

// .text 0x8A4
void dAcNpcSpHaniwa_c::executeExit() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mTalkHaniwa.mMsgKind = talk_c::MSG_KIND_EXIT;
        dDemo::requestSpeak(this, NULL);
    }
    calcAnmRate();
}

// .text 0x8F8
void dAcNpcSpHaniwa_c::initItem() {
}

// .text 0x8FC
void dAcNpcSpHaniwa_c::executeItem() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mTalkHaniwa.mMsgKind = talk_c::MSG_KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    }
    calcAnmRate();
}

// .text 0x950: the player stands in the (locked) exit area.
BOOL dAcNpcSpHaniwa_c::isAtExit() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    return dBGCF::isNextLocked(
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
}

// .text 0x990: the player points at a displayed item (a wall furniture of
// the auction).
BOOL dAcNpcSpHaniwa_c::checkPointedItem() {
    mTalkHaniwa.mSlot = -1;
    mTalkHaniwa.mFtrHandle = -1;
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    int unitX;
    int unitZ;
    mAng angle(0x2000);
    if (getPointedUnit(&unitX, &unitZ, &angle, 0, 128.0f, l_pointDist)) {
        daFTR_c *ftr = dFTR::getUnitMap()->get(unitX, unitZ, 0);
        if (ftr != NULL) {
            mTalkHaniwa.mFtrHandle =
                dFTR::getUnitMap()->getHandle(unitX, unitZ, 0);
            mTalkHaniwa.mSlot = ftr->getWallSlot();
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0xA68
dAcNpcSpHaniwa_c::talk_c::talk_c(dAcNpcSpHaniwa_c *npc) : mpOwner(npc) {
    init();
    mExiting = 0;
}

// .text 0xADC
dAcNpcSpHaniwa_c::talk_c::~talk_c() {
}

// .text 0xB34
void dAcNpcSpHaniwa_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageStart = NULL;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
}

// .text 0xBCC
void dAcNpcSpHaniwa_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    switch (mMsgKind) {
    case MSG_KIND_GREET:
        mMsgNo = MSG_WELCOME;
        break;
    case MSG_KIND_EXIT:
        mMsgNo = MSG_COME_AGAIN;
        break;
    case MSG_KIND_TALK:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_AUCTION_VISITED)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_AUCTION_VISITED);
            mMsgNo = MSG_FIRST_VISIT;
            mFirstVisit = 1;
        } else {
            mMsgNo = MSG_MENU;
            mFirstVisit = 0;
        }
        break;
    case MSG_KIND_ITEM: {
        if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_AUCTION_VISITED)) {
            mMsgNo = MSG_FIRST_VISIT_ITEM;
            break;
        }
        dSvAucItem_c item = *dANpcSpHaniwa::getAucItem(mSlot);
        if (item.isMyUnsold()) {
            mMsgNo = MSG_UNSOLD;
        } else if (item.isMyWon()) {
            mMsgNo = MSG_WON;
        } else if (item.getSeller() == NULL ||
                   *item.getSeller() !=
                       dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
            mMsgNo = MSG_ITEM_INFO;
        } else {
            mMsgNo = MSG_OWN_ITEM;
        }
        mOnMessageStart = &talk_c::setItemWords;
        break;
    }
    }
    mItem = dItem::ITEM_ID_NONE;
}

// .text 0xF7C
void dAcNpcSpHaniwa_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_auction";
    info->mCode = mMsgNo;
}

// .text 0xF94
void dAcNpcSpHaniwa_c::talk_c::onMessageStart(int arg) {
    if (mOnMessageStart) {
        (this->*mOnMessageStart)();
        mOnMessageStart = NULL;
    }
}

// .text 0xFF8
void dAcNpcSpHaniwa_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_WELCOME:
        switch (dSvAuc_c::getTodayType()) {
        case AUC_DAY_DISPLAY:
            if (dSaveData_c::getTown()->mAuction.countEmpty() == 0) {
                mpController->setNextMsgCode(MSG_DISPLAY_FULL);
            } else {
                mpController->setNextMsgCode(MSG_DISPLAY_OPEN);
            }
            break;
        case AUC_DAY_WAIT_BID:
        case AUC_DAY_AFTER_BID:
        case AUC_DAY_WAIT_DISPLAY: {
            dTime_c now = *dTime_c::getCurrent();
            if (now.wday != WEEKDAY_SUNDAY && now.wday != WEEKDAY_SATURDAY) {
                now.add(WEEKDAY_SATURDAY - now.wday, 0, 0, 0);
            }
            now.hour = TIME_DAY_START_HOUR;
            if (dSvAuc_c::getDayType(now) == AUC_DAY_DISPLAY) {
                mpController->setNextMsgCode(MSG_PREPARING_DISPLAY);
            } else {
                mpController->setNextMsgCode(MSG_PREPARING_BIDDING);
            }
            setMonthName(now.month, 3);
            setDayName((u8)now.mday, 4);
            break;
        }
        case AUC_DAY_BID:
            mpController->setNextMsgCode(MSG_BIDDING_OPEN);
            break;
        }
        break;
    case MSG_ABOUT_DISPLAY:
    case MSG_ABOUT_BID:
        if (mFirstVisit) {
            mpController->setNextMsgCode(MSG_ANYTHING_ELSE);
        } else {
            mpController->setNextMsgCode(MSG_ANYTHING_ELSE_CHECKROOM);
        }
        break;
    case MSG_WHICH_ITEM:
        mOnMessageEnd = &talk_c::selectItem;
        break;
    case MSG_DISPLAY_CANCELED:
        mItem = dItem::ITEM_ID_NONE;
        break;
    case MSG_ASK_RESERVE:
        mOnMessageEnd = &talk_c::openPriceMenu;
        break;
    case MSG_DISPLAYED: {
        int slot = dSvAuc_c::get()->findSlot(&mItem);
        int price = fn_80029B20();
        dSvAuc_c::get()->list(slot, &mItem, price, NULL);
        dItem::Item none;
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(
            &none, getMenuSelSlot(), FALSE);
        dFtrm_c::createWallFtr(slot, mItem);
        break;
    }
    case MSG_UNSOLD:
    case MSG_WON:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) < 0) {
            mpController->setNextMsgCode(MSG_POCKETS_FULL);
        } else {
            mpController->setNextMsgCode(MSG_ASK_TAKE_HOME);
        }
        break;
    case MSG_TAKE_IT: {
        dSvAucItem_c *item = dSvAuc_c::get()->getItem(mSlot);
        dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(item->mItem, FALSE);
        item->take();
        dFTR::sendEvent(mFtrHandle);
        break;
    }
    case MSG_ITEM_INFO: {
        dSvAucItem_c item = *dANpcSpHaniwa::getAucItem(mSlot);
        switch (dSvAuc_c::getTodayType()) {
        case AUC_DAY_BID:
            if (item.isOpenToday() == TRUE) {
                if (item.hasMyBid()) {
                    mpController->setNextMsgCode(MSG_MY_BID);
                } else if (dSvAuc_c::get()->hasMyBidToday()) {
                    mpController->setNextMsgCode(MSG_BID_ELSEWHERE);
                } else {
                    mpController->setNextMsgCode(MSG_ASK_BID);
                }
            } else {
                mpController->setNextMsgCode(MSG_BIDDING_ENDED);
            }
            break;
        case AUC_DAY_DISPLAY:
        case AUC_DAY_WAIT_BID:
            if (item.isNotToday() == TRUE) {
                mpController->setNextMsgCode(MSG_BIDDING_ENDED);
            } else {
                int year, month, day;
                dSvAuc_c::getNextCloseDate(&year, &month, &day);
                setMonthName(month, 3);
                setDayName((u8)day, 4);
                mpController->setNextMsgCode(MSG_BIDDING_LATER);
            }
            break;
        case AUC_DAY_AFTER_BID:
        case AUC_DAY_WAIT_DISPLAY:
        default:
            mpController->setNextMsgCode(MSG_BIDDING_ENDED);
            break;
        }
        break;
    }
    case MSG_BIDDING_ENDED:
        if (dANpcSpHaniwa::getAucItem(mSlot)->hasMyBid()) {
            mpController->setNextMsgCode(MSG_CALCULATING);
        } else {
            mpController->setNextMsgCode(dMsg::MSG_CODE_NONE);
        }
        break;
    case MSG_DISPLAY_OPEN:
    case MSG_DISPLAY_FULL:
    case MSG_BIDDING_OPEN:
    case MSG_PREPARING_DISPLAY:
    case MSG_PREPARING_BIDDING:
        if (dSvAuc_c::get()->hasMyReturns()) {
            mpController->setNextMsgCode(MSG_RETURN_ITEM);
        } else {
            mpController->setNextMsgCode(dMsg::MSG_CODE_NONE);
        }
        break;
    case MSG_CHECKROOM:
        mOnMessageEnd = &talk_c::openCheckroom;
        break;
    }
    switch (mMsgKind) {
    case MSG_KIND_EXIT:
        dMsg::Comp_c::mInstance->mBgmKeep = 1;
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
    }
}

// .text 0x16E0
void dAcNpcSpHaniwa_c::talk_c::onAnswer(int arg) {
    dMsg::Comp_c *ctrl = mpController;
    switch (mMessageCode) {
    case MSG_MENU:
    case MSG_MENU_AGAIN:
        switch (ctrl->mSelect.mCursor) {
        case MENU_DISPLAY:
            startDisplay();
            break;
        case MENU_BID:
            startBid();
            break;
        case MENU_CHECKROOM:
            ctrl->setNextMsgCode(MSG_CHECKROOM);
            break;
        case MENU_WHATS_THIS:
        case MENU_NEVER_MIND:
            break;
        }
        break;
    case MSG_ASK_TAKE_HOME:
        switch (ctrl->mSelect.mCursor) {
        case 0:
            ctrl->setNextMsgCode(MSG_TAKE_IT);
            break;
        case 1:
            ctrl->setNextMsgCode(MSG_SEND_LATER);
            break;
        }
        break;
    case MSG_ASK_BID:
        switch (ctrl->mSelect.mCursor) {
        case 0:
            mOnAnswer = &talk_c::openBidMenu;
            break;
        case 1:
            ctrl->setNextMsgCode(MSG_NO_BID);
            break;
        }
        break;
    case MSG_CONFIRM_BID:
        switch (ctrl->mSelect.mCursor) {
        case 0:
            placeBid(mBid, FALSE);
            break;
        case 1:
            ctrl->setNextMsgCode(MSG_NO_BID);
            break;
        }
        break;
    case MSG_CASH_OR_CARD:
        switch (ctrl->mSelect.mCursor) {
        case 0:
            placeBid(mBid, FALSE);
            break;
        case 1:
            placeBid(mBid, TRUE);
            break;
        case 2:
            ctrl->setNextMsgCode(MSG_NO_BID);
            break;
        }
        break;
    case MSG_CARD_INSTEAD:
        switch (ctrl->mSelect.mCursor) {
        case 0:
            placeBid(mBid, TRUE);
            break;
        case 1:
            ctrl->setNextMsgCode(MSG_NO_BID);
            break;
        }
        break;
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x1940
void dAcNpcSpHaniwa_c::talk_c::preExecute() {
}

// .text 0x1944
void dAcNpcSpHaniwa_c::talk_c::onTalkEnd() {
    if (mMsgKind == MSG_KIND_EXIT) {
        dANpcSpHaniwa::getSceneChange()->requestExit(0, 2, 2);
        mExiting = 1;
    }
    mMsgKind = MSG_KIND_TALK;
}

// .text 0x199C
void dAcNpcSpHaniwa_c::talk_c::startDisplay() {
    if (dSvAuc_c::getTodayType() != AUC_DAY_DISPLAY) {
        int year, month, day;
        dSvAuc_c::getNextOpenDate(&year, &month, &day);
        setMonthName(month, 3);
        setDayName((u8)day, 4);
        mpController->setNextMsgCode(MSG_NEXT_DISPLAY);
    } else if (dSvAuc_c::get()->hasMyReturns()) {
        mpController->setNextMsgCode(MSG_TAKE_RETURN_FIRST);
    } else if (dSaveData_c::getTown()->mAuction.countEmpty() == 0) {
        mpController->setNextMsgCode(MSG_NO_ROOM);
    } else if (dSvAuc_c::get()->hasListedToday(NULL)) {
        mpController->setNextMsgCode(MSG_ONE_DISPLAY);
    } else {
        mpController->setNextMsgCode(MSG_WHICH_ITEM);
    }
}

// .text 0x1AC0
void dAcNpcSpHaniwa_c::talk_c::startBid() {
    if (dSvAuc_c::getTodayType() != AUC_DAY_BID) {
        int year, month, day;
        dSvAuc_c::getNextCloseDate(&year, &month, &day);
        setMonthName(month, 3);
        setDayName((u8)day, 4);
        mpController->setNextMsgCode(MSG_NEXT_BIDDING);
    } else if (dSvAuc_c::get()->hasMyBidToday()) {
        mpController->setNextMsgCode(MSG_ONE_BID);
    } else if (dSaveData_c::getTown()->mAuction.hasOpenToday() == TRUE) {
        mpController->setNextMsgCode(MSG_LOOK_AROUND);
    } else {
        mpController->setNextMsgCode(MSG_NOTHING_ON_DISPLAY);
    }
}

// .text 0x1B8C
void dAcNpcSpHaniwa_c::talk_c::placeBid(int bid, BOOL card) {
    if (!card) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(bid, FALSE);
    } else {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->subSavings(bid);
    }
    dSvAucItem_c *item = dSvAuc_c::get()->getItem(mSlot);
    mpController->setNextMsgCode(MSG_BID_PLACED);
    item->bid(bid);
}

// .text 0x1C18
int dAcNpcSpHaniwa_c::talk_c::checkMoney(int bid) {
    int savings = dPlayerMgr::Mgr_c::getCurrentPlayer()->getCardSavings();
    int cash = dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketMoney();
    int pay = PAY_NONE;
    if (cash >= bid) {
        pay = PAY_CASH;
        if (savings >= bid) {
            pay = PAY_BOTH;
        }
    } else if (savings >= bid) {
        pay = PAY_CARD;
    }
    return pay;
}

// .text 0x1C88
void dAcNpcSpHaniwa_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

namespace dANpcSpHaniwa {
// .text 0x1CEC: the item-select filter: only furniture can be displayed
// (TRUE = not selectable).
static BOOL isNotDisplayable(void *item, int flag) {
    if (flag) {
        return TRUE;
    }
    u16 id = *(u16 *)item;
    if (dItem::isRealItemId(id)) {
        const dItem::BITM *bitm = dItem::getBITM(id);
        if (bitm == NULL || !bitm->isFurnitureLike()) {
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}
} // namespace dANpcSpHaniwa

// .text 0x1D78
void dAcNpcSpHaniwa_c::talk_c::selectItem() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    reqSelectItem(dANpcSpHaniwa::isNotDisplayable, 0xED, 0x2C, TRUE);
    mOnRequestEnd = &talk_c::endSelectItem;
    mOnMessageEnd = NULL;
}

// .text 0x1E18
void dAcNpcSpHaniwa_c::talk_c::openPriceMenu() {
    reqMenu30(TRUE);
    mOnRequestEnd = &talk_c::endPriceMenu;
    mOnMessageEnd = NULL;
}

// .text 0x1E80
void dAcNpcSpHaniwa_c::talk_c::openBidMenu() {
    dSvAucItem_c item = *dANpcSpHaniwa::getAucItem(mSlot);
    reqMenu31(item.mMinPrice, TRUE);
    mOnRequestEnd = &talk_c::endBidMenu;
    mOnMessageEnd = NULL;
}

// .text 0x2184
void dAcNpcSpHaniwa_c::talk_c::openCheckroom() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    reqMenu0D(dMenuMng_c::MENU_STORAGE, TRUE);
    mOnRequestEnd = &talk_c::endCheckroom;
    mOnMessageEnd = NULL;
}

// .text 0x2218
void dAcNpcSpHaniwa_c::talk_c::endSelectItem() {
    if (isMenuInvalid()) {
        mpController->setNextMsgCode(MSG_DISPLAY_CANCELED);
    } else {
        int slot = getMenuSelSlot();
        mItem = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[slot];
        if (dSaveData_c::getTown()->mAuction.findSlot(&mItem) < 0) {
            mpController->setNextMsgCode(MSG_TOO_BIG);
        } else {
            mpController->setNextMsgCode(MSG_CONFIRM_ITEM);
            setItemName(mItem, 0);
        }
    }
    reqMsgClose();
}

// .text 0x22D0
void dAcNpcSpHaniwa_c::talk_c::endPriceMenu() {
    if (lbl_8074EA7D == 1) {
        mpController->setNextMsgCode(MSG_CONFIRM_RESERVE);
        setNumber(fn_80029B20(), 0, 12, dScript::NUM_FORMAT_REGION);
    } else {
        mpController->setNextMsgCode(MSG_DISPLAY_CANCELED);
    }
    reqMsgClose();
}

// .text 0x2348
void dAcNpcSpHaniwa_c::talk_c::setItemWords() {
    mBid = 0;
    dSvAucItem_c item = *dANpcSpHaniwa::getAucItem(mSlot);
    setItemName(item.mItem, 0);
    setPersonalName(&item.mSeller, 2);
    setLandName(&item.mSeller.land, 1);
    setNumber(item.mMinPrice, 0, 12, dScript::NUM_FORMAT_REGION);
    setNumber(item.getMyBid(), 1, 12, dScript::NUM_FORMAT_REGION);
}

// .text 0x2598
void dAcNpcSpHaniwa_c::talk_c::endBidMenu() {
    if (lbl_8074EA7D == 1) {
        setNumber(fn_80029B20(), 1, 12, dScript::NUM_FORMAT_REGION);
        mBid = fn_80029B20();
        switch (checkMoney(mBid)) {
        case PAY_NONE:
            mpController->setNextMsgCode(MSG_NOT_ENOUGH);
            break;
        case PAY_CASH:
            mpController->setNextMsgCode(MSG_CONFIRM_BID);
            break;
        case PAY_CARD:
            mpController->setNextMsgCode(MSG_CARD_INSTEAD);
            break;
        case PAY_BOTH:
            mpController->setNextMsgCode(MSG_CASH_OR_CARD);
            break;
        }
    } else {
        mpController->setNextMsgCode(MSG_NO_BID);
    }
    reqMsgClose();
}

// .text 0x267C
void dAcNpcSpHaniwa_c::talk_c::endCheckroom() {
    mpController->setNextMsgCode(MSG_CHECKROOM_THANKS);
    reqMsgClose();
}
