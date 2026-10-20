#include <game/game/d_a_npc_sp_jijyo.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shops.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_time_stamp.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/sLib/s_lib.hpp>

#ifdef MUST_MATCH
// Matching-only emission-order workaround (as in d_a_npc_sp_fuko); no evidence
// of an original helper. Queue d_a_npc.hpp's weak section before the implicit
// mVec3_c destructor (isDisplayPointed's l_displayPos) opens its own section.
// MWCC drops this unused body.
static void orderNpcWeakInlines(dAcNpc_c *npc) {
    npc->dAcNpc_c::getHoldItem();
}
#endif

// Labelle at GracieGrace. Functions are in target .text order.

namespace dANpcSpJijyo {
// .text 0xCC
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text 0xD8
BASE_PROFILE(A_NPC_SP_JIJYO, dAcNpcSpJijyo_c);
} // namespace dANpcSpJijyo

// .text 0x248
void dAcNpcSpJijyo_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mLabelleTalk.mKind = talk_c::KIND_TALK;
        mLabelleTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        mLabelleTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_WAIT);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpJijyo {
static const dAcNpcSpJijyo_c::State_s l_states[] = {
    {&dAcNpcSpJijyo_c::initWait, &dAcNpcSpJijyo_c::executeWait},
    {&dAcNpcSpJijyo_c::initTurn, &dAcNpcSpJijyo_c::executeTurn},
    {&dAcNpcSpJijyo_c::initApproach, &dAcNpcSpJijyo_c::executeApproach},
    {&dAcNpcSpJijyo_c::initLook, &dAcNpcSpJijyo_c::executeLook},
    {&dAcNpcSpJijyo_c::initTalk, &dAcNpcSpJijyo_c::executeTalk},
    {&dAcNpcSpJijyo_c::initGreet, &dAcNpcSpJijyo_c::executeGreet},
    {&dAcNpcSpJijyo_c::initSeeOff, &dAcNpcSpJijyo_c::executeSeeOff},
};
} // namespace dANpcSpJijyo

using namespace dANpcSpJijyo;

// .text 0x31C
void dAcNpcSpJijyo_c::changeState(int state) {
    mState = state;
    (this->*l_states[state].mInit)();
}

// .text 0x354
int dAcNpcSpJijyo_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mLabelleTalk);
    return 1;
}

// .text 0x3A0
int dAcNpcSpJijyo_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    for (int i = 0; i < STATE_NUM; i++) {
        mStep[i] = STEP_START;
    }
    dBGCF::lockNext(0);
    changeState(STATE_GREET);
    if (isPlayerFromThisTown() && !isMultiPlay() &&
        !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_DRAMA_LABELLE_HEARD) &&
        (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 2 ||
         dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 6 ||
         dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 9)) {
        mDrama = 1;
    } else {
        mDrama = 0;
    }
    mApproached = 0;
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    return 1;
}

// .text 0x498
int dAcNpcSpJijyo_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x4C4
int dAcNpcSpJijyo_c::vtB0() {
    if (mState < STATE_NUM) {
        (this->*l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x50C
void dAcNpcSpJijyo_c::initWait() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mApproached = 0;
}

// .text 0x550
void dAcNpcSpJijyo_c::executeWait() {
    if (dPlayerMgr::isSceneTransitionState()) {
        return;
    }
    if (!mDrama) {
        mVec3_c step = getRouteStep(
            dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
        f32 moveDist = getMoveDist();
        if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) >
                moveDist &&
            step != mPos) {
            changeState(STATE_APPROACH);
        } else if (sLib::distanceAngle(mAngle.y, getAngleYToPlayer(_1B70)) >
                   0) {
            changeState(STATE_TURN);
        }
    }
    if (isSeeOffTime()) {
        changeState(STATE_SEE_OFF);
    } else if (isDisplayPointed()) {
        mLabelleTalk.mKind = talk_c::KIND_DISPLAY;
        dDemo::requestSpeak(this, NULL);
    } else if (checkPointedItem()) {
        mLabelleTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x6C8
void dAcNpcSpJijyo_c::initTurn() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0, cNpcMorphFrames);
}

// .text 0x724
void dAcNpcSpJijyo_c::executeTurn() {
    if (isSeeOffTime()) {
        changeState(STATE_SEE_OFF);
    } else if (isDisplayPointed()) {
        mLabelleTalk.mKind = talk_c::KIND_DISPLAY;
        dDemo::requestSpeak(this, NULL);
    } else if (checkPointedItem()) {
        mLabelleTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    } else {
        mVec3_c playerPos =
            *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        mVec3_c step = getRouteStep(&playerPos);
        if (dPlayerMgr::isSceneTransitionState()) {
            changeState(STATE_WAIT);
        } else {
            f32 stopDist = getStopDist();
            if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) >
                    stopDist &&
                step != mPos) {
                changeState(STATE_APPROACH);
            } else if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
                changeState(STATE_WAIT);
            } else {
                mAction.setTargetAngle(getAngleYToPlayer(_1B70));
            }
        }
    }
}

namespace dANpcSpJijyo {
// The unit's parameters (.bss 0x8, built by __sinit).
struct param_s {
    param_s()
        : _0(0), _4(0), _8(0), mStopDist(88.0f), mMoveDist(118.0f),
          mWalkSpeed(0.8f), mWalkAccel(0.04f), mWalkDecel(0.06f) {
    }
    int _0; // unused
    int _4;
    int _8;
    f32 mStopDist;  // getStopDist
    f32 mMoveDist;  // getMoveDist
    f32 mWalkSpeed; // the walk over to the player (moveParam_c)
    f32 mWalkAccel;
    f32 mWalkDecel;
};
static param_s l_param;
} // namespace dANpcSpJijyo

// .text 0x8D0
void dAcNpcSpJijyo_c::initApproach() {
    mVec3_c pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    moveParam_c param(l_param.mWalkSpeed, l_param.mWalkAccel,
                      l_param.mWalkDecel);
    mAction.requestWalkFacing(PRIORITY_IDLE, pos, l_walkTurnSpeed, 0, param,
                              l_defaultAnmRate, cNpcMorphFrames);
    mApproached = 1;
}

// .text 0x970
void dAcNpcSpJijyo_c::executeApproach() {
    if (dPlayerMgr::isSceneTransitionState()) {
        changeState(STATE_WAIT);
    } else if (isSeeOffTime()) {
        changeState(STATE_SEE_OFF);
    } else if (isDisplayPointed()) {
        mLabelleTalk.mKind = talk_c::KIND_DISPLAY;
        dDemo::requestSpeak(this, NULL);
    } else if (checkPointedItem()) {
        mLabelleTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    } else {
        mVec3_c playerPos =
            *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        f32 dist = calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL);
        mVec3_c step = getRouteStep(&playerPos);
        if (dist <= getStopDist() || step == mPos) {
            changeState(STATE_WAIT);
        } else {
            mAction.setTargetPos(step);
        }
    }
}

// .text 0xAE8
void dAcNpcSpJijyo_c::initLook() {
    mStep[STATE_LOOK] = STEP_START;
}

// .text 0xAF4
void dAcNpcSpJijyo_c::stepLook() {
    ++mStep[STATE_LOOK];
}

namespace dANpcSpJijyo {
static const dAcNpcSpJijyo_c::Proc l_lookSteps[] = {&dAcNpcSpJijyo_c::stepLook};
} // namespace dANpcSpJijyo

// .text 0xB04
void dAcNpcSpJijyo_c::executeLook() {
    if (mStep[STATE_LOOK] < STEP_DONE) {
        (this->*l_lookSteps[mStep[STATE_LOOK]])();
    }
}

// .text 0xB44
void dAcNpcSpJijyo_c::initTalk() {
    dActor_c *target = mLabelleTalk.mpActActor;
    u16 angle = 0;
    if (target) {
        angle = getAngleYTo(target);
    }
    if (mLabelleTalk.mKind == talk_c::KIND_ITEM) {
        switch (mLabelleTalk.mItem.getKind()) {
        case dItem::KIND_CARPET:
            mTalk.requestTurnTalk(angle, getTalkTurnSpeed(), PRIORITY_DEMO);
            dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_SHOP_WIDE;
            break;
        case dItem::KIND_WALL:
            mTalk.requestTurnTalk(angle, getTalkTurnSpeed(), PRIORITY_DEMO);
            dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_SHOP_CLOSE;
            break;
        default:
            mTalk.requestTalk(false, PRIORITY_IDLE);
            break;
        }
    } else {
        mTalk.requestTurnTalk(angle, getTalkTurnSpeed(), PRIORITY_IDLE);
    }
    mStep[STATE_TALK] = STEP_START;
}

// .text 0xC68
void dAcNpcSpJijyo_c::stepTalk() {
    if (!mTalk.isBusy()) {
        if (!mLabelleTalk.mExitRequested) {
            dDemo::finishTalk(this);
            ++mStep[STATE_TALK];
        }
        return;
    }
    mVec3_c playerPos =
        *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    f32 dist = calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL);
    mVec3_c step = getRouteStep(&playerPos);
    if (dist <= getStopDist() || step == mPos || !mApproached) {
        if (mAction.isAction(ACTION_TURN)) {
            if (mAction.mCanChange) {
                int feel = mLabelleTalk.takePendingFeel();
                if (feel > 0) {
                    mAction.requestEmotion(PRIORITY_DEMO, feel,
                                           cNpcMorphFrames);
                } else {
                    mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
                }
            }
        } else if (mAction.isAction(ACTION_WALK_FACING) ||
                   mAction.isAction(ACTION_RUN_FACING)) {
            mAng angle = getAngleYToPlayer(_1B70);
            mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0,
                                cNpcMorphFrames);
        }
        mApproached = 0;
    } else {
        mAction.setTargetPos(step);
    }
}

namespace dANpcSpJijyo {
static const dAcNpcSpJijyo_c::Proc l_talkSteps[] = {&dAcNpcSpJijyo_c::stepTalk};
} // namespace dANpcSpJijyo

// .text 0xE54
void dAcNpcSpJijyo_c::executeTalk() {
    if (mStep[STATE_TALK] < STEP_DONE) {
        (this->*l_talkSteps[mStep[STATE_TALK]])();
    }
}

// .text 0xE94
void dAcNpcSpJijyo_c::initGreet() {
}

// .text 0xE98
void dAcNpcSpJijyo_c::executeGreet() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mLabelleTalk.mKind = talk_c::KIND_GREET;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0xEE4
void dAcNpcSpJijyo_c::initSeeOff() {
}

// .text 0xEE8
void dAcNpcSpJijyo_c::executeSeeOff() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mLabelleTalk.mKind = talk_c::KIND_SEE_OFF;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0xF34
BOOL dAcNpcSpJijyo_c::isSeeOffTime() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    return dBGCF::isNextLocked(
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
}

// .text 0xF74
BOOL dAcNpcSpJijyo_c::isDisplayPointed() {
    if (mPad::g_currentCore->getDpdValidFlag() && dPad::isATriggered()) {
        const dObjc::dpdHit_c *hit = dObjc::getDpdHit();
        if (hit == NULL) {
            return FALSE;
        }
        if (hit->mType != dObjc::TYPE_PLAYER) {
            if (hit->mType == dObjc::HIT_RMOBJ) {
                f32 range = l_pointRange;
                if (!dPlayerMgr::isATriggeredFacing(
                        &hit->mPos, l_pointAngle.mAngle, range)) {
                    return FALSE;
                }
            } else {
                return FALSE;
            }
        }
    }
    if (dPad::isConfirmTriggered()) {
        // The display (the mannequin) in front of the counter.
        static mVec3_c l_displayPos(384.0f, 28.0f, 24.0f);
        if (dField::getInfo(0) != NULL) {
            mVec3_c pos;
            static_cast<dActor_c *>(dScene::getScenePlayer())
                ->getOffsetPos(&pos, l_pointDist, 0);
            if (pos.z <= l_displayPos.z + 16.0f &&
                pos.x >= l_displayPos.x - 32.0f &&
                pos.x <= 32.0f + l_displayPos.x) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// .text 0x10FC
BOOL dAcNpcSpJijyo_c::checkPointedItem() {
    dItem::Item item;
    int unitX;
    int unitZ;
    if (!getPointedShopItem(&item, &unitX, &unitZ)) {
        return FALSE;
    }
    mLabelleTalk.setPointedItem(item, unitX, unitZ);
    if (mLabelleTalk.mItem.isValid()) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x1188
f32 dAcNpcSpJijyo_c::getStopDist() {
    return l_param.mStopDist;
}

// .text 0x1198
f32 dAcNpcSpJijyo_c::getMoveDist() {
    return l_param.mMoveDist;
}

// .text 0x11A8
dAcNpcSpJijyo_c::talk_c::talk_c(dAcNpcSpJijyo_c *npc) {
    mpOwner = npc;
    init();
    mItem = dItem::ITEM_ID_NONE;
    mUnitX = 0;
    mUnitZ = 0;
    mKind = KIND_TALK;
    mExitRequested = 0;
    mBuyCount = 0;
}

// .text 0x1250
dAcNpcSpJijyo_c::talk_c::~talk_c() {
}

// .text 0x12C4
void dAcNpcSpJijyo_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = MSG_NONE;
    mOnMessageStart = NULL;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mNextOnRequestEnd = NULL;
    mTrying = 0;
    mMiiMask = 0;
    mShopCamera = 0;
    mDelayFeel = 0;
    mPendingFeel = 0;
}

// GracieGrace's save data.
static inline dSaveShopGrace_c *getShopGrace() {
    return &dSaveData_c::getTown()->mShops.mShopGrace;
}

// .text 0x1388
void dAcNpcSpJijyo_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    switch (mKind) {
    case KIND_GREET:
        mMsgNo = MSG_WELCOME;
        break;
    case KIND_SEE_OFF:
        mMsgNo = MSG_SEE_OFF;
        break;
    case KIND_TALK:
        if (mpOwner->mDrama) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() ==
                2) {
                mMsgNo = DRAMA_MSG_SIGH;
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()
                           ->getGracieStep() == 6) {
                mMsgNo = DRAMA_MSG_SIGH_2;
            } else {
                mMsgNo = DRAMA_MSG_SIGH_3;
            }
        } else if (getShopGrace()->isSoldOut()) {
            mMsgNo = MSG_SOLD_OUT;
        } else {
            switch (getShopGrace()->getSaleStage()) {
            case GRACE_SALE_STAGE_1:
            case GRACE_SALE_STAGE_2: {
                dTimeStamp_c stamp(*dTime_c::getCurrent());
                stamp.toDayStart();
                switch (stamp.get().month) {
                case MONTH_JANUARY:
                    mMsgNo = MSG_SALE_WINTER;
                    break;
                case MONTH_APRIL:
                    mMsgNo = MSG_SALE_SPRING;
                    break;
                case MONTH_JULY:
                    mMsgNo = MSG_SALE_SUMMER;
                    break;
                case MONTH_OCTOBER:
                    mMsgNo = MSG_SALE_FALL;
                    break;
                default:
                    mMsgNo = MSG_NONE;
                    break;
                }
                break;
            }
            case GRACE_SALE_STAGE_3:
                mMsgNo = MSG_SALE_CLEARANCE;
                break;
            case GRACE_SALE_NONE:
                mMsgNo = MSG_MENU;
                break;
            }
        }
        break;
    case KIND_DISPLAY:
        if (dSaveData_c::getExtra()->mOutfit.isValid() &&
            dSaveData_c::getExtra()->mOutfitLand.isValid()) {
            mMsgNo = MSG_DISPLAY_TREND;
        } else {
            mMsgNo = MSG_DISPLAY;
        }
        break;
    case KIND_ITEM:
        if (mpOwner->mApproached) {
            mDelayFeel = 1;
        }
        switch (mItem.getKind()) {
        case dItem::KIND_WALL:
            if (mItem.getSeriesGroup() == dItem::SERIES_GROUP_BASIC) {
                mMsgNo = MSG_WALL_BASIC;
            } else {
                mMsgNo = MSG_WALL;
            }
            mShopCamera = 1;
            break;
        case dItem::KIND_CARPET:
            if (mItem.getSeriesGroup() == dItem::SERIES_GROUP_BASIC) {
                mMsgNo = MSG_CARPET_BASIC;
            } else {
                mMsgNo = MSG_CARPET;
            }
            mShopCamera = 1;
            break;
        case dItem::KIND_FTR:
            if (mItem.getSeriesGroup() == dItem::SERIES_GROUP_BASIC) {
                mMsgNo = MSG_FTR_BASIC;
            } else {
                mMsgNo = MSG_FTR;
            }
            break;
        case dItem::KIND_UMBRELLA:
            mMsgNo = MSG_UMBRELLA;
            break;
        case dItem::KIND_CLOTH:
            if (mItem.getFashion()) {
                mMsgNo = MSG_CLOTH_LOOK;
            } else {
                mMsgNo = MSG_CLOTH;
            }
            break;
        case dItem::KIND_CAP:
            if (mItem.getFashion()) {
                mMsgNo = MSG_CAP_LOOK;
            } else {
                mMsgNo = MSG_CAP;
            }
            break;
        case dItem::KIND_ACC:
            if (mItem.getFashion()) {
                mMsgNo = MSG_ACC_LOOK;
            } else {
                mMsgNo = MSG_ACC;
            }
            break;
        }
        break;
    }
}

// .text 0x1704
void dAcNpcSpJijyo_c::talk_c::getMsgInfo(msgInfo_s *info) {
    if (mpOwner->mDrama && mKind == KIND_TALK) {
        info->mLabel = "npc_SP/NPC_drama3";
        info->mCode = mMsgNo;
    } else {
        info->mLabel = "npc_SP/NPC_cotton";
        info->mCode = mMsgNo;
    }
}

// .text 0x1750
void dAcNpcSpJijyo_c::talk_c::onMessageStart(int arg) {
    if (!mpOwner->mDrama || mKind != KIND_TALK) {
        switch (mMessageCode) {
        case MSG_DISPLAY_TREND:
            setLandName(&dSaveData_c::getExtra()->mOutfitLand, 2);
            break;
        case MSG_EMPTY_0A:
            break;
        case MSG_CLOTH:
        case MSG_CLOTH_LOOK:
        case MSG_CAP:
        case MSG_CAP_LOOK:
        case MSG_ACC:
        case MSG_ACC_LOOK:
        case MSG_FTR:
        case MSG_FTR_BASIC:
        case MSG_CARPET:
        case MSG_CARPET_BASIC:
        case MSG_WALL:
        case MSG_WALL_BASIC:
        case MSG_UMBRELLA:
            setItemName(mItem, 0);
            break;
        case MSG_DECLINED:
        case MSG_NO_MONEY:
        case MSG_POCKETS_FULL:
        case MSG_THANKS:
        case MSG_THANKS_SLIP:
            if (mTrying) {
                // Take the tried-on item off again.
                mOnMessageStart = &talk_c::toggleTryOn;
                mOnRequestEnd = &talk_c::endTryOn;
            }
            break;
        case MSG_EMPTY_19:
        case MSG_TRIED_ON_OTHER:
        case MSG_EMPTY_2B:
            break;
        case MSG_PRICE:
            mPrice = mItem.getPrice();
            setNumber(mPrice, 0, 7, dScript::NUM_FORMAT_REGION);
            break;
        case MSG_PRICE_SALE: {
            int percent = 0;
            switch (getShopGrace()->getSaleStage()) {
            case GRACE_SALE_STAGE_1:
                percent = SALE_PERCENT_1;
                break;
            case GRACE_SALE_STAGE_2:
                percent = SALE_PERCENT_2;
                break;
            case GRACE_SALE_STAGE_3:
                percent = SALE_PERCENT_3;
                break;
            }
            mPrice = mItem.getPrice() - mItem.getPrice() * percent / 100;
            setNumber(mPrice, 0, 7, dScript::NUM_FORMAT_REGION);
            formatNumber(percent, 1, 2, dScript::NUM_FORMAT_REGION);
            break;
        }
        case MSG_PRICE_DISCOUNT:
            mPrice =
                mItem.getPrice() - mItem.getPrice() * DISCOUNT_PERCENT / 100;
            setNumber(mPrice, 0, 7, dScript::NUM_FORMAT_REGION);
            formatNumber(DISCOUNT_PERCENT, 1, 2, dScript::NUM_FORMAT_REGION);
            break;
        case MSG_SOLD_OUT: {
            int month = dTime_c::getCurrent()->month + 1;
            if (month >= MONTH_NUM) {
                month = MONTH_JANUARY;
            }
            setMonthName(month, 1);
            break;
        }
        }
    }
    if (mOnMessageStart) {
        (this->*mOnMessageStart)();
        mOnMessageStart = NULL;
    }
}

// .text 0x1A24
void dAcNpcSpJijyo_c::talk_c::onMessageEnd(int kind) {
    if (mpOwner->mDrama && mKind == KIND_TALK) {
        switch (mMessageCode) {
        case DRAMA_MSG_TOLD:
        case DRAMA_MSG_TOLD_2:
        case DRAMA_MSG_TOLD_3:
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_DRAMA_LABELLE_HEARD);
            break;
        }
    } else {
        switch (mMessageCode) {
        case MSG_THANKS:
        case MSG_THANKS_SLIP:
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_GRACE_BOUGHT);
            break;
        }
        switch (mKind) {
        case KIND_SEE_OFF:
            dMsg::Comp_c::mInstance->mBgmKeep = 1;
            break;
        }
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

static inline BOOL canAfford(int price) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketMoney() >= price;
}

// .text 0x1B20
void dAcNpcSpJijyo_c::talk_c::onAnswer(int arg) {
    dMsg::Comp_c *ctrl = getController();
    u8 answer = ctrl->getSelect()->mCursor;
    if (!mpOwner->mDrama || mKind != KIND_TALK) {
        // Show the wallpaper / carpet on the walls and floor.
        switch (mMessageCode) {
        case MSG_CARPET:
        case MSG_CARPET_BASIC:
        case MSG_WALL:
        case MSG_WALL_BASIC:
            switch (answer) {
            case ANSWER_BUY:
                break;
            case ANSWER_DECLINE:
                dCamera_c::sInstance->startTalkView(mpOwner->demoHook58());
                break;
            }
            break;
        case MSG_PRICE:
        case MSG_PRICE_SALE:
        case MSG_PRICE_DISCOUNT:
            switch (mItem.getKind()) {
            case dItem::KIND_WALL:
            case dItem::KIND_CARPET:
                dCamera_c::sInstance->startTalkView(mpOwner->demoHook58());
                break;
            }
            break;
        }
        if (mMessageCode >= MSG_CLOTH && mMessageCode <= MSG_UMBRELLA) {
            mDelayFeel = 0;
            mpOwner->mApproached = 0;
        }
        switch (mMessageCode) {
        case MSG_CLOTH:
        case MSG_CLOTH_LOOK:
        case MSG_CAP:
        case MSG_CAP_LOOK:
        case MSG_ACC:
        case MSG_ACC_LOOK:
            switch (answer) {
            case ANSWER_BUY:
                askPrice();
                break;
            case ANSWER_TRY_ON:
                mOnAnswer = &talk_c::tryOnAnswer;
                mOnRequestEnd = &talk_c::startTryOn;
                mItem.getStyle();
                switch (mItem.getKind()) {
                case dItem::KIND_CAP:
                    mpController->setNextMsgCode(MSG_TRIED_ON_CAP);
                    break;
                case dItem::KIND_ACC:
                    mpController->setNextMsgCode(MSG_TRIED_ON_ACC);
                    break;
                default:
                    getController()->setNextMsgCode(MSG_TRIED_ON_CLOTH +
                                                    mItem.getStyle());
                    break;
                }
                break;
            case ANSWER_NEVER_MIND:
                break;
            }
            break;
        case MSG_FTR:
        case MSG_FTR_BASIC:
        case MSG_CARPET:
        case MSG_CARPET_BASIC:
        case MSG_WALL:
        case MSG_WALL_BASIC:
        case MSG_UMBRELLA:
            switch (answer) {
            case ANSWER_BUY:
                askPrice();
                break;
            case ANSWER_DECLINE:
                break;
            }
            break;
        case MSG_TRIED_ON_CAP:
        case MSG_TRIED_ON_ACC:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_CUTE:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_COOL:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_SUBTLE:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_GAUDY:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_STRANGE:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_FUNKY:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_REFINED:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_FRESH:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_STYLISH:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_STRIKING:
        case MSG_TRIED_ON_CLOTH + dItem::LOOK_NONE:
            switch (answer) {
            case ANSWER_BUY:
                askPrice();
                break;
            case ANSWER_DECLINE:
                break;
            }
            break;
        case MSG_PRICE:
        case MSG_PRICE_SALE:
        case MSG_PRICE_DISCOUNT:
            switch (answer) {
            case ANSWER_BUY: {
                int free = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                    NULL, NULL);
                int price = mPrice;
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getCardSavings() >=
                    price) {
                    if (canAfford(price)) {
                        mpController->setNextMsgCode(MSG_PAY_HOW);
                    } else if (free < 1) {
                        mpController->setNextMsgCode(MSG_POCKETS_FULL);
                    } else {
                        mpController->setNextMsgCode(MSG_PAY_CARD);
                    }
                } else {
                    if (!canAfford(price)) {
                        mpController->setNextMsgCode(MSG_NO_MONEY);
                        return;
                    }
                    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(mPrice,
                                                                        TRUE)) {
                        dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem,
                                                                      FALSE);
                        onBought();
                    } else {
                        mpController->setNextMsgCode(MSG_POCKETS_FULL);
                    }
                }
                break;
            }
            case ANSWER_DECLINE:
                break;
            }
            break;
        case MSG_PAY_HOW:
            switch (answer) {
            case ANSWER_BUY:
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(mPrice,
                                                                    TRUE)) {
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem,
                                                                  FALSE);
                    onBought();
                } else {
                    mpController->setNextMsgCode(MSG_POCKETS_FULL);
                }
                break;
            case ANSWER_CARD:
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                        NULL, NULL) >= 1) {
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->subSavings(mPrice);
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem,
                                                                  FALSE);
                    onBought();
                } else {
                    mpController->setNextMsgCode(MSG_POCKETS_FULL);
                }
                break;
            default:
                mpController->setNextMsgCode(MSG_DECLINED);
                break;
            }
            break;
        case MSG_PAY_CARD:
            switch (answer) {
            case ANSWER_BUY:
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                        NULL, NULL) >= 1) {
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->subSavings(mPrice);
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem,
                                                                  FALSE);
                    onBought();
                } else {
                    mpController->setNextMsgCode(MSG_POCKETS_FULL);
                }
                break;
            default:
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

// .text 0x2018
void dAcNpcSpJijyo_c::talk_c::preExecute() {
}

// .text 0x201C
void dAcNpcSpJijyo_c::talk_c::onTalkEnd() {
    if (mpOwner->mDrama && mKind == KIND_TALK) {
        mpOwner->mDrama = 0;
    } else if (mKind == KIND_SEE_OFF) {
        getSceneChange()->requestExit(0, 2, 2);
        mExitRequested = 1;
    }
    mKind = KIND_TALK;
    mItem = dItem::ITEM_ID_NONE;
    if (mShopCamera == 1) {
        dCamera_c::sInstance->mRequestedMode =
            dCamera_c::sInstance->mDefaultMode;
    }
}

// .text 0x20C4
void dAcNpcSpJijyo_c::talk_c::setSpeakerFeel(u32 feel) {
    if (mDelayFeel) {
        mPendingFeel = feel;
    } else {
        setFeel(feel, mSpeaker);
    }
}

// .text 0x20EC
void dAcNpcSpJijyo_c::talk_c::onBought() {
    if (++mBuyCount >= 2) {
        if (getShopGrace()->getSaleStage() != GRACE_SALE_NONE) {
            dSaveTown_c *town = dSaveData_c::getTown();
            town->mVillagerMove.setPlayerValue(
                dPlayerMgr::getCurrentPlayerSlot(), 6);
        } else {
            dSaveTown_c *town = dSaveData_c::getTown();
            town->mVillagerMove.setPlayerValue(
                dPlayerMgr::getCurrentPlayerSlot(), 7);
        }
        mBuyCount = 2;
    }
    mpController->setNextMsgCode(MSG_BOUGHT);
}

// .text 0x21A0
void dAcNpcSpJijyo_c::talk_c::askPrice() {
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
            PRIVATE_FLAG1_GRACIE_DISCOUNT)) {
        mpController->setNextMsgCode(MSG_PRICE_DISCOUNT);
    } else if (getShopGrace()->getSaleStage() != GRACE_SALE_NONE) {
        mpController->setNextMsgCode(MSG_PRICE_SALE);
    } else {
        mpController->setNextMsgCode(MSG_PRICE);
    }
}

// .text 0x2220
void dAcNpcSpJijyo_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
        if (mNextOnRequestEnd) {
            mOnRequestEnd = mNextOnRequestEnd;
        }
        mNextOnRequestEnd = NULL;
    }
}

// .text 0x22C8
void dAcNpcSpJijyo_c::talk_c::toggleTryOn() {
    if (mTrying) {
        BOOL keepMask = FALSE;
        mEquip = mEquipOrig;
        if (mMiiMask && mItem.getKind() != dItem::KIND_TA_CLOTH &&
            mItem.getKind() != dItem::KIND_CLOTH) {
            keepMask = TRUE;
        }
        requestEquip(&mEquip, 4, keepMask);
    } else {
        mEquipOrig.setFromPlayer();
        mEquip = mEquipOrig;
        mMiiMask = dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiMask != 0;
        dFtrm_c::switchOn(dFTR::getUnitMap()->get(mUnitX, mUnitZ, 0));
        switch (mItem.getKind()) {
        case dItem::KIND_UMBRELLA:
            mEquip.mHeld = mItem;
            break;
        case dItem::KIND_CLOTH:
        case dItem::KIND_TA_CLOTH:
            mEquip.mShirt = mItem;
            break;
        case dItem::KIND_CAP:
            mEquip.mHat = mItem;
            if (!mEquip.isAccessoryVisible()) {
                mEquip.mAcc = dItem::ITEM_ID_NONE;
            }
            break;
        case dItem::KIND_ACC:
            mEquip.mAcc = mItem;
            if (!mEquip.isAccessoryVisible()) {
                mEquip.mHat = dItem::ITEM_ID_NONE;
            }
            break;
        }
        requestEquip(&mEquip, 4, 0);
    }
}

// .text 0x2488
void dAcNpcSpJijyo_c::talk_c::endTryOn() {
    if (mTrying) {
        mTrying = 0;
        dFtrm_c::switchOff(dFTR::getUnitMap()->get(mUnitX, mUnitZ, 0));
    } else {
        mTrying = 1;
    }
}

// .text 0x24E8
void dAcNpcSpJijyo_c::talk_c::tryOnAnswer() {
    if (getController()->mNextMsgCode == 0) {
        getController()->setNextMsgCode(0xFFFF);
    }
    setRequest1(0);
}

// .text 0x2540
void dAcNpcSpJijyo_c::talk_c::startTryOn() {
    toggleTryOn();
    mNextOnRequestEnd = &talk_c::finishTryOn;
}

// .text 0x2588
void dAcNpcSpJijyo_c::talk_c::finishTryOn() {
    endTryOn();
    reqMsgClose();
    mOnRequestEnd = &talk_c::doNothing2;
}

// .text 0x25D8
void dAcNpcSpJijyo_c::talk_c::doNothing2() {
}

// .text 0x25DC
int dAcNpcSpJijyo_c::getEarType() {
    return NPC_EAR_TYPE_LABELLE;
}
