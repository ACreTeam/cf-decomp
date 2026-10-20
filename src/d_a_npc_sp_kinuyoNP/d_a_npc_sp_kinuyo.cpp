#include <game/game/d_a_npc_sp_kinuyo.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_pattern_copy.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shops.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shop_layout.hpp>
#include <game/sLib/s_lib.hpp>

// Mabel at the Able Sisters. Functions are in target .text order.

// npc_SP/NPC_kinuyo message codes, named from the English text.
enum {
    MSG_WELCOME = 0x01,       // "Hi there! Welcome to ... the Able Sisters!"
    MSG_WELCOME_AGAIN = 0x02, // "Ohh, welcome!" (talked to today)
    MSG_SEE_OFF = 0x03,       // "Thanks! You come back, now!"
    MSG_PRICE_CLOTH = 0x04,   // "That's <item>. ... costs just <n> Bells."
    MSG_PRICE_CAP = 0x05,
    MSG_PRICE_ACC = 0x06,
    MSG_PRICE_UMBRELLA = 0x07, // no "Try it on."
    MSG_DECLINED = 0x08,       // "Ohh, really? Don't be shy."
    MSG_TRIED_ON = 0x09,       // "...that looks absolutely perfect on you!"
    MSG_THANKS = 0x0A,         // "Thanks!" (bought)
    MSG_PAID = 0x0B,           // "It's <n> Bells, OK? Thanks so much!"
    MSG_NO_MONEY = 0x0C,       // "...you don't have enough money..."
    MSG_POCKETS_FULL = 0x0D,   // "...your pockets are full..."
    MSG_DESIGN_MINE = 0x0E,    // "...that's a design called <name>." (the
                               // player's own)
    MSG_DESIGN_TOWN = 0x0F,    // "...that design was made by <player>."
    MSG_DESIGN_OTHER = 0x10,   // "...made by <player> from <town>."
    MSG_DISPLAY_CHOOSE = 0x13, // "OK, go ahead and choose the design you want
                               // to display."
    MSG_TAKE_CHOOSE = 0x15,   // "OK, please select where you want to store it."
    MSG_SWAP_CHOOSE = 0x16,   // "Oh, OK, please choose a design to swap it
                              // with."
    MSG_DESIGN_CANCEL = 0x17, // "Ohhh, OK then."
    MSG_DESIGN_TAKEN = 0x18,  // "Enjoy your new design!"
    MSG_DESIGN_DISPLAYED = 0x19, // "Ohhh, wow, this design could make it BIG."
    MSG_DESIGN_SWAPPED = 0x1C,   // "OK, I switched them out!"
    MSG_BUSY = 0x1D,             // "...I'm a little busy right now."
    MSG_MENU = 0x1F,             // "What can I do for you?" / "Make a Pro
                                 // design." / "See design storage." / ...
    MSG_MENU_VISITOR = 0x23,     // "You can try on anything you like."
    MSG_PRO_CHOOSE = 0x2C,       // "OK, first decide where you want to store
                                 // the design..."
    MSG_PRO_CANCEL = 0x2D,       // "...you want to quit?"
    MSG_PRO_NAMED = 0x2E,        // "Ohhh, so it's called <name>!" (how worn)
    MSG_PRO_WEARER = 0x2F,       // "What sort of person do you picture wearing
                                 // this design?"
    MSG_PRO_DONE = 0x30,         // "...that'll be 350 Bells!"
    MSG_PRICE_CLOTH_LOOK = 0x31, // MSG_PRICE_* with the fashion theme
    MSG_PRICE_CAP_LOOK = 0x32,
    MSG_PRICE_ACC_LOOK = 0x33,
    MSG_STORED = 0x34,            // "We'll take good care of it."
    MSG_STORE_CANCEL = 0x35,      // "Let me know if you need anything else"
    MSG_MENU_AGAIN = 0x36,        // "Well, OK.... What do you want?" (after the
                                  // drama)
    MSG_NOT_FROM_TOWN = 0x38,     // "We can only take designs for people from
                                  // this town."
    MSG_NEWCOMER_DESIGN = 0x39,   // "That <name> is a pretty neat design..."
    MSG_NEWCOMER_UMBRELLA = 0x3A, // "That <name> umbrella... Nice, isn't it?"
    MSG_NEWCOMER_ITEM = 0x3B,     // "That's <item>. Nice, right?"
    MSG_NEWCOMER = 0x3C, // "Ohh, hello! We haven't seen your face here before!"
};

// npc_SP/NPC_drama1 message codes (her sister's story,
// dPrivateData_c::getGracieStep).
enum {
    DRAMA_MSG_SIGH = 0x01,         // "Sigh.... / What's wrong? / Back to work!"
    DRAMA_MSG_BACK_TO_WORK = 0x02, // "I must have totally spaced out..."
    DRAMA_MSG_TOLD = 0x03,         // "...a letter from someone from my past."
    DRAMA_MSG_SIGH_2 = 0x05,
    DRAMA_MSG_BACK_TO_WORK_2 = 0x06,
    DRAMA_MSG_TOLD_2 = 0x07,
    DRAMA_MSG_SIGH_3 = 0x0B,
    DRAMA_MSG_BACK_TO_WORK_3 = 0x0C,
    DRAMA_MSG_TOLD_3 = 0x0D,
    DRAMA_MSG_SIGH_4 = 0x10,
    DRAMA_MSG_BACK_TO_WORK_4 = 0x11,
    DRAMA_MSG_TOLD_4 = 0x12,
};

#define PRO_DESIGN_PRICE 350 // Bells for the materials
#define PRO_STYLE_END 10     // ends a l_proStyles list

namespace dANpcSpKinuyo {
// .text 0x5C
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text 0x68
BASE_PROFILE(A_NPC_SP_KINUYO, dAcNpcSpKinuyo_c);
} // namespace dANpcSpKinuyo

// .text 0x1D8
void dAcNpcSpKinuyo_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeState(STATE_LOOK);
        }
        break;
    case DEMO_CMD_TALK:
        if (member == NET_MEMBER_NONE) {
            mMabelTalk.mKind = talk_c::KIND_TALK;
        }
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mMabelTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_SPEAK:
        if (isMultiPlay() && member == NET_MEMBER_NONE) {
            mMabelTalk.mKind = talk_c::KIND_ITEM;
        }
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mMabelTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            if (isMultiPlay()) {
                changeState(STATE_MULTI_WAIT);
            } else {
                changeState(STATE_WAIT);
            }
        } else if (permit == PERMIT_END_MULTI) {
            changeState(STATE_REMOTE);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            if (isMultiPlay()) {
                changeState(STATE_MULTI_WAIT);
            } else {
                changeState(STATE_WAIT);
            }
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpKinuyo {
static const dAcNpcSpKinuyo_c::State_s l_states[] = {
    {&dAcNpcSpKinuyo_c::initWait, &dAcNpcSpKinuyo_c::executeWait},
    {&dAcNpcSpKinuyo_c::initTurn, &dAcNpcSpKinuyo_c::executeTurn},
    {&dAcNpcSpKinuyo_c::initApproach, &dAcNpcSpKinuyo_c::executeApproach},
    {&dAcNpcSpKinuyo_c::initLook, &dAcNpcSpKinuyo_c::executeLook},
    {&dAcNpcSpKinuyo_c::initTalk, &dAcNpcSpKinuyo_c::executeTalk},
    {&dAcNpcSpKinuyo_c::initGreet, &dAcNpcSpKinuyo_c::executeGreet},
    {&dAcNpcSpKinuyo_c::initSeeOff, &dAcNpcSpKinuyo_c::executeSeeOff},
    {&dAcNpcSpKinuyo_c::initMultiWait, &dAcNpcSpKinuyo_c::executeMultiWait},
    {&dAcNpcSpKinuyo_c::initWatch, &dAcNpcSpKinuyo_c::executeWatch},
    {&dAcNpcSpKinuyo_c::initRemote, &dAcNpcSpKinuyo_c::executeRemote},
    {&dAcNpcSpKinuyo_c::initRemoteTalk, &dAcNpcSpKinuyo_c::executeRemoteTalk},
};
} // namespace dANpcSpKinuyo

using namespace dANpcSpKinuyo;

// .text 0x3A8
void dAcNpcSpKinuyo_c::changeState(int state) {
    mState = state;
    (this->*l_states[state].mInit)();
}

// .text 0x3E0
int dAcNpcSpKinuyo_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mMabelTalk);
    return 1;
}

// .text 0x42C
int dAcNpcSpKinuyo_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mApproached = 0;
    if (isPlayerFromThisTown() && !isMultiPlay() &&
        !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_DRAMA_MABEL_HEARD) &&
        (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 1 ||
         dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 4 ||
         dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 7 ||
         dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 10)) {
        mDrama = 1;
    } else {
        mDrama = 0;
    }
    if (isMultiPlay()) {
        if (isLocalOwner()) {
            changeState(STATE_MULTI_WAIT);
        } else {
            changeState(STATE_REMOTE);
        }
        mPos.set(224.0f, 0.0f, 400.0f);
        mLastPos = mPos;
    } else {
        mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
        dBGCF::lockNext(0);
        if (dScene::getPrevScene() != dScene::SCENE_FIELD) {
            changeState(STATE_WAIT);
            mSoundPlayed = 1;
        } else {
            changeState(STATE_GREET);
        }
    }
    return 1;
}

// .text 0x5A8
int dAcNpcSpKinuyo_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x5D4
int dAcNpcSpKinuyo_c::vtB0() {
    if (mState < STATE_NUM) {
        (this->*l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x61C
int dAcNpcSpKinuyo_c::canSpeak(dDemoActor_c *actor) {
    if (isInUse(NULL)) {
        return 0;
    }
    return !mTalk.isBusy();
}

// .text 0x66C
void dAcNpcSpKinuyo_c::initWait() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x680
void dAcNpcSpKinuyo_c::executeWait() {
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
    } else if (isClosingTime()) {
        changeState(STATE_SEE_OFF);
    } else if (checkPointedItem()) {
        mMabelTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x7F0
void dAcNpcSpKinuyo_c::initTurn() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0, cNpcMorphFrames);
}

// .text 0x84C
void dAcNpcSpKinuyo_c::executeTurn() {
    if (isSeeOffTime()) {
        changeState(STATE_SEE_OFF);
    } else if (isClosingTime()) {
        changeState(STATE_SEE_OFF);
    } else if (checkPointedItem()) {
        mMabelTalk.mKind = talk_c::KIND_ITEM;
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

// .text 0x9F0
void dAcNpcSpKinuyo_c::initApproach() {
    mVec3_c pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mAction.requestWalkFacing(PRIORITY_IDLE, pos, l_walkTurnSpeed, 0,
                              l_moveParamWalk, l_defaultAnmRate,
                              cNpcMorphFrames);
    mApproached = 1;
}

// .text 0xA74
void dAcNpcSpKinuyo_c::executeApproach() {
    if (dPlayerMgr::isSceneTransitionState()) {
        changeState(STATE_WAIT);
    } else if (isSeeOffTime()) {
        changeState(STATE_SEE_OFF);
    } else if (isClosingTime()) {
        changeState(STATE_SEE_OFF);
    } else if (checkPointedItem()) {
        mMabelTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    } else {
        mVec3_c playerPos =
            *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        f32 dist = calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL);
        mVec3_c step = getRouteStep(&playerPos);
        if (dist > getMoveDist()) {
            if (mAction.mActionId != ACTION_RUN &&
                mAction.mActionId != ACTION_RUN_FACING) {
                moveParam_c param(1.0666667f, 0.26666668f, 0.4f);
                mAction.requestRunFacing(PRIORITY_IDLE, step, l_runTurnSpeed, 0,
                                         param, l_8074FDEC, cNpcMorphFrames);
            }
        } else {
            if (mAction.mActionId != ACTION_WALK &&
                mAction.mActionId != ACTION_WALK_FACING) {
                mAction.requestWalkFacing(PRIORITY_IDLE, step, l_walkTurnSpeed,
                                          0, l_moveParamWalk, l_defaultAnmRate,
                                          cNpcMorphFrames);
            }
        }
        if (dist <= getStopDist() || step == mPos) {
            changeState(STATE_WAIT);
        } else {
            mAction.setTargetPos(step);
        }
    }
}

// .text 0xCAC
void dAcNpcSpKinuyo_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = STEP_START;
}

// .text 0xD10
void dAcNpcSpKinuyo_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

namespace dANpcSpKinuyo {
static const dAcNpcSpKinuyo_c::Proc l_lookSteps[] = {
    &dAcNpcSpKinuyo_c::stepLook};
} // namespace dANpcSpKinuyo

// .text 0xD70
void dAcNpcSpKinuyo_c::executeLook() {
    if (mStep < STEP_DONE) {
        (this->*l_lookSteps[mStep])();
    }
}

// .text 0xDB0
void dAcNpcSpKinuyo_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    mStep = STEP_START;
}

// .text 0xE1C
void dAcNpcSpKinuyo_c::stepTalk() {
    if (!mTalk.isBusy() && !mMabelTalk.mExitRequested &&
        dPatternCopy::isCopyAcked() && dPatternCopy::isSwapAcked()) {
        dDemo::finishTalk(this);
        ++mStep;
    }
}

namespace dANpcSpKinuyo {
static const dAcNpcSpKinuyo_c::Proc l_talkSteps[] = {
    &dAcNpcSpKinuyo_c::stepTalk};
} // namespace dANpcSpKinuyo

// .text 0xE8C
void dAcNpcSpKinuyo_c::executeTalk() {
    if (mStep < STEP_DONE) {
        (this->*l_talkSteps[mStep])();
    }
}

// .text 0xECC
void dAcNpcSpKinuyo_c::initGreet() {
}

// .text 0xED0
void dAcNpcSpKinuyo_c::executeGreet() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mMabelTalk.mKind = talk_c::KIND_GREET;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0xF1C
void dAcNpcSpKinuyo_c::initSeeOff() {
}

// .text 0xF20
void dAcNpcSpKinuyo_c::executeSeeOff() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mMabelTalk.mKind = talk_c::KIND_SEE_OFF;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0xF6C
void dAcNpcSpKinuyo_c::initMultiWait() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0xF80
void dAcNpcSpKinuyo_c::executeMultiWait() {
    if (!dPlayerMgr::isSceneTransitionState() && checkPointedItem()) {
        mMabelTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0xFD8
void dAcNpcSpKinuyo_c::initWatch() {
    if (sLib::distanceAngle(mAngle.y, getAngleYToPlayer(_1B70)) > 0) {
        mAng angle = getAngleYToPlayer(_1B70);
        mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0,
                            cNpcMorphFrames);
    } else {
        mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    }
}

// .text 0x1070
void dAcNpcSpKinuyo_c::executeWatch() {
}

// .text 0x1074
void dAcNpcSpKinuyo_c::initRemote() {
}

// .text 0x1078
void dAcNpcSpKinuyo_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mMabelTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
            return;
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_MULTI_WAIT);
            return;
        }
    }
    if (!dPlayerMgr::isSceneTransitionState() && checkPointedItem()) {
        mMabelTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x1140
void dAcNpcSpKinuyo_c::initRemoteTalk() {
}

// .text 0x1144
void dAcNpcSpKinuyo_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_MULTI_WAIT);
    }
}

// .text 0x1198
BOOL dAcNpcSpKinuyo_c::isSeeOffTime() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    if (dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return dBGCF::isNextLocked(
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
}

// .text 0x11F0
BOOL dAcNpcSpKinuyo_c::checkPointedItem() {
    dItem::Item item;
    int unitX;
    int unitZ;
    if (!getPointedShopItem(&item, &unitX, &unitZ)) {
        return FALSE;
    }
    mMabelTalk.setPointedItem(item, unitX, unitZ);
    if (mMabelTalk.mItem.isValid()) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x127C
BOOL dAcNpcSpKinuyo_c::isClosingTime() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    return isShopClosed();
}

// .text 0x12C4: reads the time, but the shop never closes.
BOOL dAcNpcSpKinuyo_c::isShopClosed() {
    dTime_c::getCurrent();
    return FALSE;
}

// .text 0x12E8
f32 dAcNpcSpKinuyo_c::getStopDist() {
    return 51.2f;
}

// .text 0x12F4
f32 dAcNpcSpKinuyo_c::getMoveDist() {
    if (mApproached) {
        return 64.32f;
    }
    return 96.0f;
}

// .text 0x1318
void dAcNpcSpKinuyo_c::vt80() {
    changeState(STATE_WATCH);
}

// .text 0x1320
void dAcNpcSpKinuyo_c::vt84() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    changeState(STATE_WAIT);
}

// .text 0x137C
dAcNpcSpKinuyo_c::talk_c::talk_c(dAcNpcSpKinuyo_c *npc) {
    mpOwner = npc;
    init();
    mItem = dItem::ITEM_ID_NONE;
    mDesignIdx = -1;
    mUnitX = 0;
    mUnitZ = 0;
    mDesignMenu = DESIGN_MENU_NONE;
    mKind = KIND_TALK;
    mExitRequested = 0;
}

// .text 0x1430
dAcNpcSpKinuyo_c::talk_c::~talk_c() {
}

// .text 0x14A4
void dAcNpcSpKinuyo_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mDesignMsg = 0;
    mOnMessageStart = NULL;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mNextOnRequestEnd = NULL;
    mTrying = 0;
    mBought = 0;
    mMiiMask = 0;
    mProSlot = -1;
    mWearAnswer = -1;
}

// The current player's house (NULL: a newcomer without one yet).
static inline dHome_c *getPlayerHome() {
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    return homes->getHome(homes->findCurrentPlayer());
}

// The shop's design mDesignIdx.
static inline dDesign_c *getShopDesign(int idx) {
    return dSaveData_c::getTown()->mShops.mShopTailor.getDesign(idx);
}

// .text 0x1570
void dAcNpcSpKinuyo_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    switch (mKind) {
    case KIND_GREET:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_MABEL_TALKED)) {
            mMsgNo = MSG_WELCOME;
        } else {
            mMsgNo = MSG_WELCOME_AGAIN;
        }
        break;
    case KIND_SEE_OFF:
        mMsgNo = MSG_SEE_OFF;
        break;
    case KIND_TALK:
        if (isPlayerFromThisTown() && getPlayerHome() == NULL) {
            mMsgNo = MSG_NEWCOMER;
        } else if (mpOwner->mDrama) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() ==
                1) {
                mMsgNo = DRAMA_MSG_SIGH;
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()
                           ->getGracieStep() == 4) {
                mMsgNo = DRAMA_MSG_SIGH_2;
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()
                           ->getGracieStep() == 7) {
                mMsgNo = DRAMA_MSG_SIGH_3;
            } else {
                mMsgNo = DRAMA_MSG_SIGH_4;
            }
        } else if (isMultiPlay() && !isPlayerFromThisTown()) {
            mMsgNo = MSG_MENU_VISITOR;
        } else {
            mMsgNo = MSG_MENU;
        }
        break;
    case KIND_ITEM:
        if (isPlayerFromThisTown() && getPlayerHome() == NULL) {
            switch (mItem.getKind()) {
            case dItem::KIND_TA_CLOTH:
                for (mDesignIdx = 0;
                     mDesignIdx <
                     ARRAY_SIZE(
                         dSaveData_c::getTown()->mShops.mShopTailor.mDesigns);
                     mDesignIdx++) {
                    if (mItem.getId() ==
                        dItem::Item(dItem::ITEM_IDX_TA_CLOTH_00, mDesignIdx,
                                    FALSE)
                            .getId()) {
                        break;
                    }
                }
                if (mDesignIdx < 4) {
                    mMsgNo = MSG_NEWCOMER_UMBRELLA;
                } else {
                    mMsgNo = MSG_NEWCOMER_DESIGN;
                }
                break;
            default:
                mMsgNo = MSG_NEWCOMER_ITEM;
                break;
            }
        } else {
            switch (mItem.getKind()) {
            case dItem::KIND_TA_CLOTH:
                for (mDesignIdx = 0;
                     mDesignIdx <
                     ARRAY_SIZE(
                         dSaveData_c::getTown()->mShops.mShopTailor.mDesigns);
                     mDesignIdx++) {
                    if (mItem.getId() ==
                        dItem::Item(dItem::ITEM_IDX_TA_CLOTH_00, mDesignIdx,
                                    FALSE)
                            .getId()) {
                        break;
                    }
                }
                if (*getShopDesign(mDesignIdx)->getCreator() ==
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID) {
                    mMsgNo = MSG_DESIGN_MINE;
                } else if (getShopDesign(mDesignIdx)->mCreator.land ==
                           dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID.land) {
                    mMsgNo = MSG_DESIGN_TOWN;
                } else {
                    mMsgNo = MSG_DESIGN_OTHER;
                }
                break;
            case dItem::KIND_UMBRELLA:
                mMsgNo = MSG_PRICE_UMBRELLA;
                break;
            case dItem::KIND_CLOTH:
                if (mItem.getFashion()) {
                    mMsgNo = MSG_PRICE_CLOTH_LOOK;
                } else {
                    mMsgNo = MSG_PRICE_CLOTH;
                }
                break;
            case dItem::KIND_CAP:
                if (mItem.getFashion()) {
                    mMsgNo = MSG_PRICE_CAP_LOOK;
                } else {
                    mMsgNo = MSG_PRICE_CAP;
                }
                break;
            case dItem::KIND_ACC:
                if (mItem.getFashion()) {
                    mMsgNo = MSG_PRICE_ACC_LOOK;
                } else {
                    mMsgNo = MSG_PRICE_ACC;
                }
                break;
            default:
                mMsgNo = MSG_PRICE_CLOTH;
                break;
            }
        }
        break;
    }
    if (!isMultiPlay()) {
        dDemoActor_c *partner =
            dNpcList::getSpByKey(dItem::Item((u16)SP_NPC_SABLE));
        if (partner != NULL) {
            setPartner(static_cast<dAcNpc_c *>(partner));
        }
    }
}

// .text 0x1A14
void dAcNpcSpKinuyo_c::talk_c::getMsgInfo(msgInfo_s *info) {
    if (mpOwner->mDrama && mKind == KIND_TALK) {
        info->mLabel = "npc_SP/NPC_drama1";
        info->mCode = mMsgNo;
    } else {
        info->mLabel = "npc_SP/NPC_kinuyo";
        info->mCode = mMsgNo;
    }
}

// .text 0x1A60
void dAcNpcSpKinuyo_c::talk_c::onMessageStart(int arg) {
    if (!mpOwner->mDrama || mKind != KIND_TALK) {
        switch (mMessageCode) {
        case MSG_PRICE_CLOTH:
        case MSG_PRICE_CAP:
        case MSG_PRICE_ACC:
        case MSG_PRICE_UMBRELLA:
        case MSG_PAID:
        case MSG_PRICE_CLOTH_LOOK:
        case MSG_PRICE_CAP_LOOK:
        case MSG_PRICE_ACC_LOOK:
        case MSG_NEWCOMER_ITEM:
            setItemName(mItem, 0);
            setNumber(mItem.getPrice(), 0, 7, dScript::NUM_FORMAT_REGION);
            if (mItem.getFashion()) {
                dItem::nameFashion_c fashionName(mItem);
                getController()->setWord(4, &fashionName);
            }
            break;
        case MSG_DECLINED:
        case MSG_THANKS:
        case MSG_NO_MONEY:
        case MSG_POCKETS_FULL:
            if (mTrying) {
                // Take the tried-on item off again.
                mOnMessageStart = &talk_c::toggleTryOn;
                mOnRequestEnd = &talk_c::endTryOn;
            }
            break;
        case MSG_DESIGN_MINE:
        case MSG_DESIGN_TOWN:
        case MSG_DESIGN_OTHER:
        case MSG_NEWCOMER_DESIGN:
        case MSG_NEWCOMER_UMBRELLA: {
            const dPersonalID_c *creator = &getShopDesign(mDesignIdx)->mCreator;
            dDesign_c *design = getShopDesign(mDesignIdx);
            dString::Word_c name(getShopDesign(mDesignIdx)->mName);
            getController()->setPersonalWord(1, &name, creator);
            getController()->setPersonalName(2, creator);
            getController()->setLandName(3, &design->mCreator.land);
            break;
        }
        case MSG_PRO_NAMED: {
            dString::Word_c name;
            name.set(dPlayerMgr::Mgr_c::getCurrentPlayer()
                         ->getOrgDesign(mProSlot)
                         ->mName,
                     0);
            getController()->setWord(1, &name);
            break;
        }
        }
    }
    if (mOnMessageStart) {
        (this->*mOnMessageStart)();
        mOnMessageStart = NULL;
    }
}

// .text 0x1D0C
void dAcNpcSpKinuyo_c::talk_c::onMessageEnd(int kind) {
    if (mpOwner->mDrama && mKind == KIND_TALK) {
        switch (mMessageCode) {
        case DRAMA_MSG_BACK_TO_WORK:
        case DRAMA_MSG_BACK_TO_WORK_2:
        case DRAMA_MSG_BACK_TO_WORK_3:
        case DRAMA_MSG_BACK_TO_WORK_4:
            // A copy of the label in this function's .data (not merged with
            // getMsgInfo's literal).
            static char label[] = "npc_SP/NPC_kinuyo";
            getController()->setNextMsgLabel(label);
            getController()->setNextMsgCode(MSG_MENU_AGAIN);
            mpOwner->mDrama = 0;
            break;
        case DRAMA_MSG_TOLD:
        case DRAMA_MSG_TOLD_2:
        case DRAMA_MSG_TOLD_3:
        case DRAMA_MSG_TOLD_4:
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_DRAMA_MABEL_HEARD);
            break;
        }
    } else {
        if (mMessageCode == MSG_SWAP_CHOOSE) {
            mDesignMenu = DESIGN_MENU_SWAP;
        }
        switch (mMessageCode) {
        case MSG_DISPLAY_CHOOSE:
        case MSG_TAKE_CHOOSE:
        case MSG_SWAP_CHOOSE:
            mOnMessageEnd = &talk_c::openDesignMenu;
            mOnRequestEnd = &talk_c::onDesignMenuEnd;
            break;
        case MSG_PRO_CHOOSE:
            mOnMessageEnd = &talk_c::openProDesignMenu;
            mOnRequestEnd = &talk_c::onProDesignMenuEnd;
            break;
        case MSG_PRO_DONE:
            dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(PRO_DESIGN_PRICE,
                                                            FALSE);
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

namespace dANpcSpKinuyo {
// The styles (dDesign_c::setStyle) a Pro design may get, by the two analog
// answers (how worn, by whom; two directions each); PRO_STYLE_END ends a
// list.
static const int l_proStyles[4][4][4] = {
    {{3, 9, 4, 10}, {3, 9, 4, 5}, {3, 9, 4, 5}, {3, 9, 4, 10}},
    {{5, 8, 10, 10}, {5, 8, 10, 10}, {0, 1, 8, 10}, {0, 1, 8, 10}},
    {{0, 1, 7, 10}, {0, 1, 7, 10}, {2, 7, 10, 10}, {2, 7, 10, 10}},
    {{6, 2, 10, 10}, {6, 2, 10, 10}, {6, 2, 10, 10}, {6, 2, 10, 10}},
};
} // namespace dANpcSpKinuyo

// .text 0x1EEC
void dAcNpcSpKinuyo_c::talk_c::onAnswer(int arg) {
    dMsg::Comp_c *ctrl = getController();
    u8 answer = ctrl->getSelect()->mCursor;
    if (!mpOwner->mDrama || mKind != KIND_TALK) {
        switch (mMessageCode) {
        case MSG_PRICE_CLOTH:
        case MSG_PRICE_CAP:
        case MSG_PRICE_ACC:
        case MSG_PRICE_CLOTH_LOOK:
        case MSG_PRICE_CAP_LOOK:
        case MSG_PRICE_ACC_LOOK:
            switch (answer) {
            case 0:
                buy();
                break;
            case 1:
                mOnAnswer = &talk_c::tryOnAnswer;
                mOnRequestEnd = &talk_c::startTryOn;
                break;
            case 2:
                break;
            }
            break;
        case MSG_PRICE_UMBRELLA:
            switch (answer) {
            case 0:
                buy();
                break;
            case 1:
                break;
            }
            break;
        case MSG_TRIED_ON:
            switch (answer) {
            case 0:
                buy();
                break;
            case 1:
                break;
            }
            break;
        case MSG_DESIGN_MINE:
        case MSG_DESIGN_TOWN:
        case MSG_DESIGN_OTHER:
            switch (answer) {
            case 0:
                mDesignMenu = DESIGN_MENU_DISPLAY;
                mDesignMsg = MSG_DESIGN_DISPLAYED;
                break;
            case 1:
                mDesignMenu = DESIGN_MENU_TAKE;
                mDesignMsg = MSG_DESIGN_TAKEN;
                break;
            case 2:
                mDesignMenu = DESIGN_MENU_SWAP;
                mDesignMsg = MSG_DESIGN_SWAPPED;
                break;
            }
            break;
        case MSG_MENU:
        case MSG_MENU_AGAIN:
            switch (answer) {
            case 1:
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                        PRIVATE_FLAG0_TRAVELLING)) {
                    getController()->setNextMsgCode(MSG_NOT_FROM_TOWN);
                } else if (isMultiPlay()) {
                    getController()->setNextMsgCode(MSG_BUSY);
                } else {
                    getController()->setNextMsgCode(0xFFFF);
                    mOnAnswer = &talk_c::openStoreMenu;
                    mOnRequestEnd = &talk_c::onStoreMenuEnd;
                }
                break;
            }
            break;
        case MSG_PRO_NAMED:
            mWearAnswer = ctrl->getAnalogSelect()->mCursor;
            break;
        case MSG_PRO_WEARER: {
            u8 wearer = ctrl->getAnalogSelect()->mCursor;
            int num;
            for (num = 0; num < 4; num++) {
                if (l_proStyles[mWearAnswer / 2][wearer / 2][num] ==
                    PRO_STYLE_END) {
                    break;
                }
            }
            int style =
                l_proStyles[mWearAnswer / 2][wearer / 2][cM::rndInt(num)];
            dPlayerMgr::Mgr_c::getCurrentPlayer()
                ->getOrgDesign(mProSlot)
                ->setStyle(style);
            break;
        }
        }
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x2228
void dAcNpcSpKinuyo_c::talk_c::preExecute() {
}

// .text 0x222C
void dAcNpcSpKinuyo_c::talk_c::onTalkEnd() {
    if (mKind == KIND_SEE_OFF) {
        getSceneChange()->requestExit(0, 2, 2);
        mExitRequested = 1;
    } else if (mKind == KIND_TALK) {
        mpOwner->mDrama = 0;
    }
    mKind = KIND_TALK;
    mItem = dItem::ITEM_ID_NONE;
    mDesignIdx = -1;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_MABEL_TALKED);
    mDesignMenu = DESIGN_MENU_NONE;
}

// .text 0x22C4
BOOL dAcNpcSpKinuyo_c::talk_c::buy() {
    int free = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(NULL, NULL);
    int price = mItem.getPrice();
    BOOL enough =
        dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketMoney() >= price;
    if (!enough) {
        getController()->setNextMsgCode(MSG_NO_MONEY);
        return FALSE;
    }
    if (free < 1) {
        getController()->setNextMsgCode(MSG_POCKETS_FULL);
        return FALSE;
    }
    dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(mItem.getPrice(), FALSE);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem, FALSE);
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mShops.mShopTailor.sellAndNotify(
        dScene::getCurrentScene(), dShopLayout_getSlot(mUnitX, mUnitZ),
        mUnitX, mUnitZ, mItem.getPrice(), !isPlayerFromThisTown());
    mBought = 1;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_ABLE_BOUGHT);
    getController()->setNextMsgCode(MSG_THANKS);
    return TRUE;
}

// .text 0x2414
void dAcNpcSpKinuyo_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
        if (mNextOnRequestEnd) {
            mOnRequestEnd = mNextOnRequestEnd;
        }
        mNextOnRequestEnd = NULL;
    }
}

// .text 0x24BC
void dAcNpcSpKinuyo_c::talk_c::openDesignMenu() {
    switch (mDesignMenu) {
    case DESIGN_MENU_DISPLAY:
        reqMenu13(0);
        break;
    case DESIGN_MENU_TAKE:
        reqMenu12(0);
        break;
    case DESIGN_MENU_SWAP:
        reqMenu14(0);
        break;
    }
}

// .text 0x2500
void dAcNpcSpKinuyo_c::talk_c::openStoreMenu() {
    reqMenu18(0);
}

// .text 0x2508
void dAcNpcSpKinuyo_c::talk_c::toggleTryOn() {
    if (mTrying) {
        BOOL keepMask = FALSE;
        if (mMiiMask && mItem.getKind() != dItem::KIND_TA_CLOTH &&
            mItem.getKind() != dItem::KIND_CLOTH) {
            keepMask = TRUE;
        }
        mEquip = mEquipOrig;
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

// .text 0x26C8
BOOL dAcNpcSpKinuyo_c::talk_c::isDesignEquipped(u32 slot) {
    return dPlayerMgr::isDesignEquipped((u8)slot);
}

// .text 0x26D0
void dAcNpcSpKinuyo_c::talk_c::onDesignMenuEnd() {
    if (isMenuInvalid()) {
        getController()->setNextMsgCode(MSG_DESIGN_CANCEL);
        return;
    }
    switch (mDesignMenu) {
    case DESIGN_MENU_DISPLAY:
        getController()->setNextMsgCode(mDesignMsg);
        dPatternCopy::copy(dPatternCopy::PATTERN_MY_PLAYER, fn_80029A84(),
                           dPatternCopy::PATTERN_TAILOR, mDesignIdx, TRUE);
        break;
    case DESIGN_MENU_TAKE:
        getController()->setNextMsgCode(mDesignMsg);
        dPatternCopy::copy(dPatternCopy::PATTERN_TAILOR, mDesignIdx,
                           dPatternCopy::PATTERN_MY_PLAYER, fn_80029A84(),
                           TRUE);
        if (isDesignEquipped(fn_80029A84())) {
            mOnMessageStart = &talk_c::refreshEquip;
            mNextOnRequestEnd = &talk_c::doNothing;
        }
        dPlayerMgr::refreshEquippedUmbrellaDesign();
        break;
    case DESIGN_MENU_SWAP:
        getController()->setNextMsgCode(mDesignMsg);
        dPatternCopy::swap(dPatternCopy::PATTERN_TAILOR, mDesignIdx,
                           dPatternCopy::PATTERN_MY_PLAYER, fn_80029A84(),
                           TRUE);
        if (isDesignEquipped(fn_80029A84())) {
            mOnMessageStart = &talk_c::refreshEquip;
            mNextOnRequestEnd = &talk_c::doNothing;
        }
        dPlayerMgr::refreshEquippedUmbrellaDesign();
        break;
    }
}

// .text 0x2864
void dAcNpcSpKinuyo_c::talk_c::onStoreMenuEnd() {
    if (isMenuInvalid()) {
        getController()->setNextMsgCode(MSG_STORE_CANCEL);
        return;
    }
    getController()->setNextMsgCode(MSG_STORED);
    if (dMenuMng_c::isPickedShirt() || dMenuMng_c::isPickedHat()) {
        mOnMessageStart = &talk_c::refreshEquip;
        mNextOnRequestEnd = &talk_c::doNothing;
    }
    dPlayerMgr::refreshEquippedUmbrellaDesign();
}

// .text 0x2908
void dAcNpcSpKinuyo_c::talk_c::refreshEquip() {
    mEquip.setFromPlayer();
    requestEquip(&mEquip, 5, 0);
}

// .text 0x294C
void dAcNpcSpKinuyo_c::talk_c::endTryOn() {
    if (mTrying) {
        mTrying = 0;
        if (!mBought) {
            dFtrm_c::switchOff(dFTR::getUnitMap()->get(mUnitX, mUnitZ, 0));
        }
    } else {
        mTrying = 1;
    }
}

// .text 0x29B8
void dAcNpcSpKinuyo_c::talk_c::doNothing() {
}

// .text 0x29BC
void dAcNpcSpKinuyo_c::talk_c::openProDesignMenu() {
    reqMenu15(0);
}

// .text 0x29C4
void dAcNpcSpKinuyo_c::talk_c::onProDesignMenuEnd() {
    if (isMenuInvalid()) {
        getController()->setNextMsgCode(MSG_PRO_CANCEL);
        return;
    }
    mProSlot = fn_80029A84();
    getController()->setNextMsgCode(MSG_PRO_NAMED);
    if (dMenuMng_c::isPickedShirt() || dMenuMng_c::isPickedHat()) {
        mOnMessageStart = &talk_c::refreshEquip;
        mNextOnRequestEnd = &talk_c::doNothing;
    }
    dPlayerMgr::refreshEquippedUmbrellaDesign();
}

// .text 0x2A70
void dAcNpcSpKinuyo_c::talk_c::tryOnAnswer() {
    if (getController()->mNextMsgCode == 0) {
        getController()->setNextMsgCode(0xFFFF);
    }
    setRequest1(0);
}

// .text 0x2AC8
void dAcNpcSpKinuyo_c::talk_c::startTryOn() {
    toggleTryOn();
    mNextOnRequestEnd = &talk_c::finishTryOn;
}

// .text 0x2B10
void dAcNpcSpKinuyo_c::talk_c::finishTryOn() {
    endTryOn();
    reqMsgClose();
    mOnRequestEnd = &talk_c::doNothing2;
}

// .text 0x2B60
void dAcNpcSpKinuyo_c::talk_c::doNothing2() {
}

namespace dANpcSpKinuyo {
// The unit's parameters (.bss 0x8, built by __sinit).
struct param_s {
    param_s() : _0(0), mTurnFrame(0x2FF) {
    }
    int _0;         // unused
    int mTurnFrame; // the player's turn to a talk partner (getTurnFrame)
};
static param_s l_param;
} // namespace dANpcSpKinuyo

// .text 0x2B64
int dAcNpcSpKinuyo_c::talk_c::getTurnFrame() {
    return (s16)l_param.mTurnFrame;
}

// .text 0x2B78
int dAcNpcSpKinuyo_c::getEarType() {
    return NPC_EAR_TYPE_MABEL;
}
