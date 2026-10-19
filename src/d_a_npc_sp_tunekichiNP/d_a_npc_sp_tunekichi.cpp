#include <game/game/d_a_npc_sp_tunekichi.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shop_gallery.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shop_layout.hpp>
#include <game/game/d_sv_villager_move.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_lib.hpp>

// Crazy Redd in his tent. Functions are in target .text order.

namespace dANpcSpTunekichi {
// .text 0x5C
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text 0x68: countPockets filter.
static int isFakePicture(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_FAKE_PICTURE_AFTER;
}

// .text 0x94
BASE_PROFILE(A_NPC_SP_TUNEKICHI, dAcNpcSpTunekichi_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word and -1.
struct unk_s {
    unk_s() : _0(0), _4(-1) {
    }
    u32 _0;
    int _4;
};
static unk_s l_unk;
} // namespace dANpcSpTunekichi

// .text 0x204
void dAcNpcSpTunekichi_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mReddTalk.mKind = talk_c::KIND_TALK;
        mReddTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        mReddTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_END:
    case CMD_RELEASE_PERMIT:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpTunekichi {
static const dAcNpcSpTunekichi_c::State_s l_states[] = {
    {&dAcNpcSpTunekichi_c::initIdle, &dAcNpcSpTunekichi_c::executeIdle},
    {&dAcNpcSpTunekichi_c::initTurn, &dAcNpcSpTunekichi_c::executeTurn},
    {&dAcNpcSpTunekichi_c::initFollow, &dAcNpcSpTunekichi_c::executeFollow},
    {&dAcNpcSpTunekichi_c::initLook, &dAcNpcSpTunekichi_c::executeLook},
    {&dAcNpcSpTunekichi_c::initTalk, &dAcNpcSpTunekichi_c::executeTalk},
    {&dAcNpcSpTunekichi_c::initGreet, &dAcNpcSpTunekichi_c::executeGreet},
    {&dAcNpcSpTunekichi_c::initBye, &dAcNpcSpTunekichi_c::executeBye},
};
} // namespace dANpcSpTunekichi

// .text 0x2D8
void dAcNpcSpTunekichi_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpTunekichi::l_states[state].mInit)();
}

// .text 0x310
int dAcNpcSpTunekichi_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mReddTalk);
    return 1;
}

// .text 0x35C
int dAcNpcSpTunekichi_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    dBGCF::lockNext(0);
    changeState(STATE_GREET);
    mObjc.setDefWeight(0x80);
    mFollowing = 0;
    return 1;
}

// .text 0x3C4
int dAcNpcSpTunekichi_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x3F0
int dAcNpcSpTunekichi_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpTunekichi::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x438
void dAcNpcSpTunekichi_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0x44C
void dAcNpcSpTunekichi_c::executeIdle() {
    if (dPlayerMgr::isSceneTransitionState()) {
        return;
    }
    mVec3_c step =
        getRouteStep(dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
    if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) >
            getFollowDist() &&
        step != mPos) {
        changeState(STATE_FOLLOW);
    } else if (sLib::distanceAngle(mAngle.y, getAngleYToPlayer(_1B70)) > 0) {
        changeState(STATE_TURN);
    }
    if (isAtExit()) {
        changeState(STATE_BYE);
    } else if (checkPointedItem()) {
        mReddTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x590
void dAcNpcSpTunekichi_c::initTurn() {
    mAction.requestTurn(PRIORITY_IDLE, getAngleYToPlayer(_1B70), l_turnSpeed,
                        0);
}

// .text 0x5EC
void dAcNpcSpTunekichi_c::executeTurn() {
    if (isAtExit()) {
        changeState(STATE_BYE);
        return;
    }
    if (checkPointedItem()) {
        mReddTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
        return;
    }
    mVec3_c pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c step = getRouteStep(&pos);
    if (dPlayerMgr::isSceneTransitionState()) {
        changeState(STATE_IDLE);
        return;
    }
    if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) >
            getStopDist() &&
        step != mPos) {
        changeState(STATE_FOLLOW);
    } else if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        changeState(STATE_IDLE);
    } else {
        mAction.setTargetAngle(getAngleYToPlayer(_1B70));
    }
}

// .text 0x770
void dAcNpcSpTunekichi_c::initFollow() {
    mVec3_c pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mAction.requestWalkFacing(PRIORITY_IDLE, pos, l_walkTurnSpeed, 0,
                              l_moveParamWalk, l_defaultAnmRate,
                              cNpcMorphFrames);
    mFollowing = 1;
}

// .text 0x7F4
void dAcNpcSpTunekichi_c::executeFollow() {
    if (dPlayerMgr::isSceneTransitionState()) {
        changeState(STATE_IDLE);
        return;
    }
    if (isAtExit()) {
        changeState(STATE_BYE);
        return;
    }
    if (checkPointedItem()) {
        mReddTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
        return;
    }
    mVec3_c pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    f32 dist = calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL);
    mVec3_c step = getRouteStep(&pos);
    if (dist > getFollowDist()) {
        if (mAction.mActionId != ACTION_RUN &&
            mAction.mActionId != ACTION_RUN_FACING) {
            moveParam_c run(4.0f / 3.0f, 0.4f, 8.0f / 15.0f);
            mAction.requestRunFacing(PRIORITY_IDLE, step, l_runTurnSpeed, 0,
                                     run, l_8074FDEC, cNpcMorphFrames);
        }
    } else if (mAction.mActionId != ACTION_WALK &&
               mAction.mActionId != ACTION_WALK_FACING) {
        mAction.requestWalkFacing(PRIORITY_IDLE, step, l_walkTurnSpeed, 0,
                                  l_moveParamWalk, l_defaultAnmRate,
                                  cNpcMorphFrames);
    }
    if (dist <= getStopDist() || step == mPos) {
        changeState(STATE_IDLE);
    } else {
        mAction.setTargetPos(step);
    }
}

// .text 0xA0C
void dAcNpcSpTunekichi_c::initLook() {
    mAction.requestTurn(PRIORITY_DEMO, getAngleYToPlayer(_1B70), l_turnSpeed,
                        0);
    mStep = STEP_START;
}

// .text 0xA70
void dAcNpcSpTunekichi_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep++;
    }
}

namespace dANpcSpTunekichi {
static const dAcNpcSpTunekichi_c::Proc l_lookSteps[] = {
    &dAcNpcSpTunekichi_c::stepLook};
} // namespace dANpcSpTunekichi

// .text 0xAD0
void dAcNpcSpTunekichi_c::executeLook() {
    if (mStep < ARRAY_SIZE(dANpcSpTunekichi::l_lookSteps)) {
        (this->*dANpcSpTunekichi::l_lookSteps[mStep])();
    }
}

// .text 0xB10
void dAcNpcSpTunekichi_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep = STEP_START;
}

// .text 0xB7C
void dAcNpcSpTunekichi_c::stepTalk() {
    if (!mTalk.isBusy() && !mReddTalk.mExiting) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpTunekichi {
static const dAcNpcSpTunekichi_c::Proc l_talkSteps[] = {
    &dAcNpcSpTunekichi_c::stepTalk};
} // namespace dANpcSpTunekichi

// .text 0xBD4
void dAcNpcSpTunekichi_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpTunekichi::l_talkSteps)) {
        (this->*dANpcSpTunekichi::l_talkSteps[mStep])();
    }
}

// .text 0xC14
void dAcNpcSpTunekichi_c::initGreet() {
}

// .text 0xC18
void dAcNpcSpTunekichi_c::executeGreet() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mReddTalk.mKind = talk_c::KIND_GREET;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0xC64
void dAcNpcSpTunekichi_c::initBye() {
}

// .text 0xC68
void dAcNpcSpTunekichi_c::executeBye() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mReddTalk.mKind = talk_c::KIND_BYE;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0xCB4
BOOL dAcNpcSpTunekichi_c::isAtExit() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    return dBGCF::isNextLocked(
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
}

// .text 0xCF4
inline void dAcNpcSpTunekichi_c::setPointedItem(const dItem::Item & item, int unitX, int unitZ) {
    mReddTalk.mItem = item;
    mReddTalk.mUnitX = unitX;
    mReddTalk.mUnitZ = unitZ;
}

BOOL dAcNpcSpTunekichi_c::checkPointedItem() {
    dItem::Item item;
    int unitX;
    int unitZ;
    if (!getPointedShopItem(&item, &unitX, &unitZ)) {
        return FALSE;
    }
    setPointedItem(item, unitX, unitZ);
    if (mReddTalk.mItem != dItem::ITEM_ID_NONE) {
        return TRUE;
    }
    return FALSE;
}

// .text 0xD80
f32 dAcNpcSpTunekichi_c::getStopDist() {
    return 51.2f;
}

// .text 0xD8C
f32 dAcNpcSpTunekichi_c::getFollowDist() {
    if (mFollowing) {
        return 60.8f;
    }
    return 96.0f;
}

// .text 0xDB0
dAcNpcSpTunekichi_c::talk_c::talk_c(dAcNpcSpTunekichi_c *npc) : mpOwner(npc) {
    init();
    mItem = dItem::ITEM_ID_NONE;
    mBought = dItem::ITEM_ID_NONE;
    mUnitX = 0;
    mUnitZ = 0;
    mKind = KIND_TALK;
    mExiting = 0;
}

// .text 0xE4C
dAcNpcSpTunekichi_c::talk_c::~talk_c() {
    for (int i = 0; i < MSG_ITEM_PITCH_NUM; i++) {
        mPitched.mUsed[i] = 0;
    }
}

// .text 0xEC0
void dAcNpcSpTunekichi_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
}

namespace dANpcSpTunekichi {
// .text 0xF28: countPockets filter.
static int isInviteCard(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_INVITE_CARD));
}
} // namespace dANpcSpTunekichi

// .text 0xF68
void dAcNpcSpTunekichi_c::talk_c::start(dActor_c *target) {
    // The idle talks: after a purchase or not, with a fake picture or not.
    static const u16 l_boughtFakeMsgs[] = {MSG_LOOK_REST, MSG_BOUGHT_ITEM,
                                           MSG_SUPPLIERS, MSG_HAGGLE,
                                           MSG_FORGERIES};
    static const u16 l_boughtMsgs[] = {MSG_LOOK_REST, MSG_BOUGHT_ITEM,
                                       MSG_SUPPLIERS, MSG_HAGGLE};
    static const u16 l_fakeMsgs[] = {MSG_LOOK_TOUCH, MSG_SUPPLIERS, MSG_HAGGLE,
                                     MSG_FORGERIES};
    static const u16 l_msgs[] = {MSG_LOOK_TOUCH, MSG_SUPPLIERS, MSG_HAGGLE};

    init();
    mpActActor = target;
    switch (mKind) {
    case KIND_GREET:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_REDD_MEMBER)) {
            mMsgNo = MSG_JOIN_OFFER;
        } else if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                       PRIVATE_FLAG1_REDD_VISITED)) {
            mMsgNo = MSG_WELCOME;
        } else {
            mMsgNo = MSG_WELCOME_BACK;
        }
        break;
    case KIND_BYE:
        if (mBought == dItem::ITEM_ID_NONE) {
            mMsgNo = MSG_BYE_NO_PURCHASE;
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                       NULL, NULL) < 1) {
            mMsgNo = MSG_BYE;
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                       dANpcSpTunekichi::isInviteCard, NULL) > 0) {
            mMsgNo = MSG_BYE;
        } else {
            if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_REDD_REFERRAL_ASKED)) {
                mMsgNo = MSG_REFERRAL_ASK;
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                           PRIVATE_FLAG0_REDD_REFERRAL_PAID)) {
                mMsgNo = MSG_REFERRAL_MORE;
            } else {
                mMsgNo = MSG_REFERRAL_AGAIN;
            }
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_REDD_REFERRAL_ASKED);
            dItem::Item card(dItem::ITEM_IDX_INVITE_CARD);
            dItem::Item invite = card.makeVariant(0);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&invite, FALSE);
        }
        break;
    case KIND_TALK:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_REDD_MEMBER)) {
            mMsgNo = MSG_JOIN_DECIDED;
        } else if (dSaveData_c::getTown()->mShops.mShopGallery.getUnsoldNum() <=
                   0) {
            mMsgNo = MSG_ALL_SOLD;
        } else if (mBought != dItem::ITEM_ID_NONE) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                    dANpcSpTunekichi::isFakePicture, NULL)) {
                mMsgNo =
                    l_boughtFakeMsgs[cM::rndInt(ARRAY_SIZE(l_boughtFakeMsgs))];
            } else {
                mMsgNo = l_boughtMsgs[cM::rndInt(ARRAY_SIZE(l_boughtMsgs))];
            }
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                       dANpcSpTunekichi::isFakePicture, NULL)) {
            mMsgNo = l_fakeMsgs[cM::rndInt(ARRAY_SIZE(l_fakeMsgs))];
        } else {
            mMsgNo = l_msgs[cM::rndInt(ARRAY_SIZE(l_msgs))];
        }
        break;
    case KIND_ITEM: {
        int slot = dShopLayout_getSlot(mUnitX, mUnitZ);
        BOOL checked =
            dSaveData_c::getTown()->mShops.mShopGallery.isChecked(slot);
        dSaveData_c::getTown()->mShops.mShopGallery.check(slot);
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_REDD_MEMBER)) {
            if (checked) {
                mMsgNo = MSG_ITEM_AGAIN;
            } else {
                switch (mItem.getKind()) {
                case dItem::KIND_PICTURE:
                case dItem::KIND_FAKE_PICTURE_BEFORE:
                    mMsgNo =
                        MSG_PICTURE_PITCH + cM::rndInt(MSG_PICTURE_PITCH_NUM);
                    break;
                default:
                    if (mPitched.isAll()) {
                        mPitched.clear();
                    }
                    dItem::Item item = mItem;
                    dItemSel::getRarity(&item);
                    do {
                        mMsgNo =
                            cM::rndInt(MSG_ITEM_PITCH_NUM) + MSG_ITEM_PITCH;
                    } while (mPitched.mUsed[mMsgNo - MSG_ITEM_PITCH] == 1);
                    mPitched.mUsed[mMsgNo - MSG_ITEM_PITCH] = 1;
                    break;
                }
            }
        } else if (checked) {
            mMsgNo = MSG_ITEM_NOT_MEMBER_AGAIN;
        } else {
            mMsgNo = MSG_ITEM_NOT_MEMBER;
        }
        break;
    }
    }
}

// .text 0x13A8
void dAcNpcSpTunekichi_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_tunekichi";
    info->mCode = mMsgNo;
}

// .text 0x13C0
void dAcNpcSpTunekichi_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_ITEM_AGAIN:
    case MSG_ITEM_PITCH:
    case MSG_ITEM_PITCH + 1:
    case MSG_ITEM_PITCH + 2:
    case MSG_ITEM_PITCH + 3:
    case MSG_ITEM_PITCH + 4:
    case MSG_ITEM_PITCH + 5:
    case 0x08:
    case 0x09:
    case MSG_PICTURE_PITCH:
    case MSG_PICTURE_PITCH + 1:
        setItemName(mItem, 0);
        setNumber(mItem.getPrice() * 2, 0, 6, dScript::NUM_FORMAT_REGION);
        break;
    case MSG_REFERRAL_REWARD:
        setLandName(
            &dPlayerMgr::Mgr_c::getCurrentPlayer()->mLastWc24Sender.land, 2);
        setPersonalName(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mLastWc24Sender,
                        3);
        setUnitWord(dPlayerMgr::Mgr_c::getCurrentPlayer()
                        ->mLastWc24Sender.player.getGender(),
                    4);
        break;
    case MSG_JOIN_INVITER: {
        dMail_c *invite =
            dPlayerMgr::Mgr_c::getCurrentPlayer()->findAnsweredInvite();
        if (invite != NULL) {
            dPersonalID_c *player = invite->getFromPlayer();
            dAnmPersonalID_c *animal = invite->getFromAnimal();
            if (player != NULL) {
                setPersonalName(player, 1);
            } else if (animal != NULL) {
                setAnmPersonalName(animal, 1);
            }
        }
        break;
    }
    case MSG_BOUGHT_ITEM:
        setItemName(mBought, 0);
        break;
    }
}

// .text 0x1524
void dAcNpcSpTunekichi_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_BOUGHT:
    case MSG_BOUGHT_2:
    case MSG_BOUGHT_3:
        if (dSaveData_c::getTown()->getShopGallery()->getUnsoldNum() <= 0) {
            mpController->setNextMsgCode(MSG_SOLD_OUT);
        } else {
            mpController->setNextMsgCode(MSG_LOOK_AROUND);
        }
        break;
    case MSG_WELCOME:
    case MSG_WELCOME_BACK:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mLastWc24Sender.isValid()) {
            mpController->setNextMsgCode(MSG_REFERRAL_REWARD);
        }
        break;
    case MSG_REFERRAL_REWARD:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->addMoney(REFERRAL_REWARD);
        dPlayerMgr::Mgr_c::getCurrentPlayer()->mLastWc24Sender.clear();
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_REDD_REFERRAL_PAID);
        break;
    case MSG_JOINED: {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(JOIN_PRICE, FALSE);
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_REDD_MEMBER);
        dSaveData_c::getTown()->getShopGallery()->clearChecked();
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        dMail_c *invite = player->findAnsweredInvite();
        if (invite != NULL) {
            dPersonalID_c *from = invite->getFromPlayer();
            BOOL found = FALSE;
            if (from != NULL && from->isValid()) {
                for (int i = 0; i < PLAYER_NUM; i++) {
                    if (dPlayerMgr::Mgr_c::getPlayer(i)->mPID.isValid() &&
                        *from == dPlayerMgr::Mgr_c::getPlayer(i)->mPID) {
                        dPlayerMgr::Mgr_c::getPlayer(i)->mLastWc24Sender.copy(
                            &player->mPID);
                        found = TRUE;
                    }
                }
                if (!found) {
                    dSaveData_c::getPostSend()->set(from);
                }
            }
        }
        mpController->setNextMsgCode(MSG_MEMBERS_ONLY);
        break;
    }
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x1780
void dAcNpcSpTunekichi_c::talk_c::onAnswer(int arg) {
    int code = mMessageCode;
    int cursor = mpController->mSelect.mCursor;
    if (code < 0xC && code >= MSG_ITEM_AGAIN) {
        switch (cursor) {
        case 0:
            buy();
            break;
        case 1:
            break;
        }
    }
}

// .text 0x17BC
void dAcNpcSpTunekichi_c::talk_c::preExecute() {
}

// .text 0x17C0
void dAcNpcSpTunekichi_c::talk_c::onTalkEnd() {
    if (mKind == KIND_BYE) {
        dANpcSpTunekichi::getSceneChange()->requestExit(0, 2, 2);
        mExiting = 1;
    }
    mKind = KIND_TALK;
    mItem = dItem::ITEM_ID_NONE;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_REDD_VISITED);
}

// .text 0x1830
BOOL dAcNpcSpTunekichi_c::talk_c::buy() {
    int room = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(NULL, NULL);
    int price = mItem.getPrice() * 2;
    BOOL enough =
        dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketMoney() >= price;
    if (!enough) {
        mpController->setNextMsgCode(MSG_NO_MONEY);
        return FALSE;
    }
    if (room < 1) {
        mpController->setNextMsgCode(MSG_POCKETS_FULL);
        return FALSE;
    }
    dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(mItem.getPrice() * 2,
                                                    FALSE);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&mItem, FALSE);
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mVillagerMove.setPlayerValue(dPlayerMgr::getCurrentPlayerSlot(), 2);
    town = dSaveData_c::getTown();
    town->mShops.mShopGallery.sell(dShopLayout_getSlot(mUnitX, mUnitZ));
    int handle = dFTR::getUnitMap()->getHandle(mUnitX, mUnitZ, 0);
    if (handle != -1) {
        dFtrm_c::sendEvent(handle);
    } else {
        dItem::Item sign(dItem::ITEM_IDX_SOLD_OUT_SIGN_00);
        dField::getInfo(0)->setItem(&sign, 0, 0, mUnitX, mUnitZ, 0);
    }
    mBought = mItem;
    if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
            PRIVATE_FLAG1_REDD_BOUGHT_1)) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
            PRIVATE_FLAG1_REDD_BOUGHT_1);
        mpController->setNextMsgCode(MSG_BOUGHT);
    } else if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                   PRIVATE_FLAG1_REDD_BOUGHT_2)) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
            PRIVATE_FLAG1_REDD_BOUGHT_2);
        mpController->setNextMsgCode(MSG_BOUGHT_2);
    } else if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                   PRIVATE_FLAG1_REDD_BOUGHT_3)) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
            PRIVATE_FLAG1_REDD_BOUGHT_3);
        mpController->setNextMsgCode(MSG_BOUGHT_3);
    } else {
        mpController->setNextMsgCode(MSG_BOUGHT);
    }
    return TRUE;
}

// .text 0x1A40
void dAcNpcSpTunekichi_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1AA4
int dAcNpcSpTunekichi_c::getEarType() {
    return NPC_EAR_TYPE_REDD;
}
