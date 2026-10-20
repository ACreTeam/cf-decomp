#include <game/game/d_a_npc_sp_catherine.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_rmobj_perm_machine.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shops.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_sv_villager_move.hpp>
#include <game/game/d_sys.hpp>
#include <game/cLib/c_lib.hpp>
#include <lib/revolution/RFL/RFL_System.h>

#ifdef MUST_MATCH
// Matching-only emission-order workaround (as in d_a_npc_sp_fuko / jijyo); no
// evidence of an original helper. Queue d_a_npc.hpp's weak section (from the
// moveParam_c destructor of l_moveParams on) before the implicit mVec3_c
// destructor (the registered walk targets) opens its own section. MWCC drops
// this unused body.
static void orderNpcWeakInlines() {
    dAcNpc_c::moveParam_c param;
}
#endif

// Harriet at Shampoodle. Functions are in target .text order.

// Tuning literals of the unit.
// A seated player at x <= CATHERINE_NEAR_SEAT_X (or, in STATE_TO_CHAIR, at
// z <= CATHERINE_NEAR_SEAT_Z) sits near her: she speaks without walking over.
#define CATHERINE_NEAR_SEAT_X 320.0f
#define CATHERINE_NEAR_SEAT_Z 352.0f
// She reached a walk target (xz distance).
#define CATHERINE_ARRIVE_DIST 16.0f
// STATE_SEAT: the player walks this far in -z to sit down.
#define CATHERINE_SEAT_STEP 32.0f
// HMN_ANM_POO_ON1: the frame the machine switch clicks.
#define CATHERINE_SWITCH_FRAME 44.0f

// The walk speeds of the debug menu (RTTI "dNpcCatherine_Calibrate_HIO_c";
// its vtable dtor is emitted before __sinit).
class dNpcCatherine_Calibrate_HIO_c {
  public:
    dNpcCatherine_Calibrate_HIO_c() {
        mSpeed = 0.8f;
        mAccel = 0.12f;
        _0C = 0;
    }
    virtual ~dNpcCatherine_Calibrate_HIO_c() {
    }

    /* 0x04 */ f32 mSpeed;
    /* 0x08 */ f32 mAccel;
    /* 0x0C */ u8 _0C;
}; // size 0x10

// The parameters of the unit (no RTTI: __sinit registers its implicit, weak
// dtor): the price of the services, plus the calibration above.
struct dNpcCatherineParam_c {
    dNpcCatherineParam_c() {
        _00 = 0;
        _04 = -1;
        mPrice = dAcNpcSpCatherine_c::talk_c::PRICE;
        _0C = 0;
        _10 = 0;
    }

    /* 0x00 */ int _00;
    /* 0x04 */ int _04;
    /* 0x08 */ int mPrice; // the hair styling and the makeover
    /* 0x0C */ int _0C;
    /* 0x10 */ int _10;
    /* 0x14 */ dNpcCatherine_Calibrate_HIO_c mCalibrate;
}; // size 0x24

namespace dANpcSpCatherine {
// .text 0xCC
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text 0xD8: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpCatherine_c *npc) {
    return FALSE;
}

// .text 0xE0
BASE_PROFILE(A_NPC_SP_CATHERINE, dAcNpcSpCatherine_c);
} // namespace dANpcSpCatherine

// .text 0x26C
void dAcNpcSpCatherine_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mCatherineTalk.mKind = talk_c::KIND_TALK;
        mCatherineTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        mCatherineTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        if (mCatherineTalk.mAccepted) {
            changeState(STATE_TO_CHAIR);
        } else if (mItemsRemoved || mTurnWalk) {
            changeState(STATE_TO_COUNTER);
        } else if (mServing) {
            changeState(STATE_SERVED);
        } else {
            changeState(STATE_IDLE);
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpCatherine {
static const dAcNpcSpCatherine_c::State_s l_states[] = {
    {&dAcNpcSpCatherine_c::initIdle, &dAcNpcSpCatherine_c::executeIdle},
    {&dAcNpcSpCatherine_c::initLook, &dAcNpcSpCatherine_c::executeLook},
    {&dAcNpcSpCatherine_c::initTalk, &dAcNpcSpCatherine_c::executeTalk},
    {&dAcNpcSpCatherine_c::initGreet, &dAcNpcSpCatherine_c::executeGreet},
    {&dAcNpcSpCatherine_c::initSeeOff, &dAcNpcSpCatherine_c::executeSeeOff},
    {&dAcNpcSpCatherine_c::initToChair, &dAcNpcSpCatherine_c::executeToChair},
    {&dAcNpcSpCatherine_c::initServed, &dAcNpcSpCatherine_c::executeServed},
    {&dAcNpcSpCatherine_c::initToCounter,
     &dAcNpcSpCatherine_c::executeToCounter},
    {&dAcNpcSpCatherine_c::initSeat, &dAcNpcSpCatherine_c::executeSeat},
};
} // namespace dANpcSpCatherine

using namespace dANpcSpCatherine;

// .text 0x398
void dAcNpcSpCatherine_c::changeState(int state) {
    mState = state;
    (this->*l_states[state].mInit)();
}

// .text 0x3D0
int dAcNpcSpCatherine_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mCatherineTalk);
    return 1;
}

// .text 0x41C
int dAcNpcSpCatherine_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    dBGCF::lockNext(0);
    changeState(STATE_GREET);
    dSceneCheck::fn_80081858(entryDebugPage(this));
    mAfterStyleMsg =
        cM::rndInt(talk_c::MSG_AFTER_STYLE_NUM) + talk_c::MSG_AFTER_STYLE;
    return 1;
}

// .text 0x48C
int dAcNpcSpCatherine_c::doDelete() {
    entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x4D0
int dAcNpcSpCatherine_c::vtB0() {
    if (mState < STATE_NUM) {
        (this->*l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x518
void dAcNpcSpCatherine_c::initIdle() {
    if (!mWalking) {
        mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    }
}

// .text 0x53C
void dAcNpcSpCatherine_c::executeIdle() {
    if (isPlayerAtExit()) {
        changeState(STATE_SEE_OFF);
    } else if (isPlayerSeated() &&
               dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT)->x <=
                   CATHERINE_NEAR_SEAT_X &&
               !dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                   PRIVATE_FLAG1_HARRIET_STYLED)) {
        mCatherineTalk.mKind = talk_c::KIND_SEATED;
        setDemoFlag8();
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x5E0
void dAcNpcSpCatherine_c::initLook() {
    if (!mWalking) {
        mAng angle = getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
        mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                            cNpcMorphFrames);
        mStep = LOOK_STEP_TURN;
    } else {
        mStep = LOOK_STEP_DONE;
    }
}

// .text 0x65C
void dAcNpcSpCatherine_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep++;
    }
}

namespace dANpcSpCatherine {
static const dAcNpcSpCatherine_c::Proc l_lookSteps[] = {
    &dAcNpcSpCatherine_c::stepLook,
};
} // namespace dANpcSpCatherine

// .text 0x6BC
void dAcNpcSpCatherine_c::executeLook() {
    if (mStep < LOOK_STEP_DONE) {
        (this->*l_lookSteps[mStep])();
    }
}

// .text 0x6FC
void dAcNpcSpCatherine_c::initTalk() {
    if (!mWalking) {
        dActor_c *target = mCatherineTalk.mpActActor;
        u16 angle = 0;
        if (target) {
            angle = getAngleYTo(target);
        }
        mTalk.requestTurnTalk(angle, mAng((u16)l_turnSpeed.mAngle),
                              PRIORITY_IDLE);
        mStep = TALK_STEP_TALK;
    } else {
        mWalking = 0;
        mStep = TALK_STEP_WALK;
    }
}

// .text 0x788
void dAcNpcSpCatherine_c::stepTalkWalk() {
    u8 turnWalk = mTurnWalk;
    // Initialized aggregates are promoted late, preserving the target
    // registers.
    BOOL walked[1] = {FALSE};
    if (!turnWalk && mAction.mActionId == ACTION_WALK) {
        walked[0] = TRUE;
    }
    BOOL turnWalked[1] = {FALSE};
    if (turnWalk && mAction.mActionId == ACTION_TURN_WALK) {
        turnWalked[0] = TRUE;
    }
    if ((walked[0] || turnWalked[0]) && mAction.mCanChange) {
        mTurnWalk = 0;
        dActor_c *target = mCatherineTalk.mpActActor;
        u16 angle = 0;
        if (target) {
            angle = getAngleYTo(target);
        }
        mTalk.requestTurnTalk(angle, mAng((u16)l_turnSpeed.mAngle),
                              PRIORITY_DEMO);
        dCamera_c::sInstance->startTalkView(demoHook58());
        mStep = TALK_STEP_TALK;
    }
}

// .text 0x87C
void dAcNpcSpCatherine_c::stepTalk() {
    if (!mTalk.isBusy() && !mCatherineTalk.mExitRequested) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpCatherine {
static const dAcNpcSpCatherine_c::Proc l_talkSteps[] = {
    &dAcNpcSpCatherine_c::stepTalk,
    &dAcNpcSpCatherine_c::stepTalkWalk,
};
} // namespace dANpcSpCatherine

// .text 0x8D4
void dAcNpcSpCatherine_c::executeTalk() {
    if (mStep < TALK_STEP_NUM) {
        (this->*l_talkSteps[mStep])();
    }
}

// .text 0x914
void dAcNpcSpCatherine_c::initGreet() {
}

// .text 0x918
void dAcNpcSpCatherine_c::executeGreet() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mCatherineTalk.mKind = talk_c::KIND_GREET;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x964
void dAcNpcSpCatherine_c::initSeeOff() {
}

// .text 0x968
void dAcNpcSpCatherine_c::executeSeeOff() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        mCatherineTalk.mKind = talk_c::KIND_SEE_OFF;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x9B4
void dAcNpcSpCatherine_c::initToChair() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mStep = TO_CHAIR_STEP_START;
}

namespace dANpcSpCatherine {
// The walk speeds (speed, accel, decel) of her walks; each goes over
// l_frontPos.
static dAcNpc_c::moveParam_c l_moveParams[4] = {
    dAcNpc_c::moveParam_c(1.0f, 0.15f, 0.07f), // STATE_TO_CHAIR
    dAcNpc_c::moveParam_c(0.8f, 0.12f, 0.07f), // talk: to the machine switch
    dAcNpc_c::moveParam_c(0.8f, 0.12f, 0.07f), // talk: back to the chair
    dAcNpc_c::moveParam_c(0.5f, 0.8f, 0.07f),  // STATE_TO_COUNTER
};
// The places she walks to.
static mVec3_c l_chairPos(317.5f, 0.0f, 335.0f);   // by the player's chair
static mVec3_c l_frontPos(274.0f, 0.0f, 335.0f);   // the way point
static mVec3_c l_counterPos(274.0f, 0.0f, 368.0f); // her place (the switch)
static dNpcCatherineParam_c l_param;
} // namespace dANpcSpCatherine

// .text 0x9F8
void dAcNpcSpCatherine_c::stepToChairStart() {
    if (!dDemo::isType(dDemo::TYPE_ANY) && isPlayerAtExit()) {
        mCatherineTalk.mKind = talk_c::KIND_SEE_OFF;
        dDemo::requestSpeak(this, NULL);
    }
    if (isPlayerSeated()) {
        if (dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT)->x <=
                CATHERINE_NEAR_SEAT_X ||
            dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT)->z <=
                CATHERINE_NEAR_SEAT_Z) {
            mCatherineTalk.mKind = talk_c::KIND_SEATED;
            setDemoFlag8();
            dDemo::requestSpeak(this, NULL);
        } else {
            mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
            dPlayerMgr::startDemoWait();
            mTargetPos = l_frontPos;
            if (mTargetPos.xzDistTo(mPos) < CATHERINE_ARRIVE_DIST) {
                mTurnWalk = 1;
            } else {
                mAng angle = targetAngleY(&mPos, &mTargetPos);
                moveParam_c param(l_moveParams[0].mSpeed,
                                  l_moveParams[0].mAccel,
                                  l_moveParams[0].mDecel);
                mAction.requestTurnWalk(PRIORITY_DEMO, mTargetPos, angle,
                                        l_turnSpeed, 0, param, l_defaultAnmRate,
                                        cNpcMorphFrames);
            }
            mStep = TO_CHAIR_STEP_WALK;
        }
    }
}

// .text 0xBB8
void dAcNpcSpCatherine_c::stepToChairWalk() {
    if (mPos.xzDistTo(mTargetPos) < CATHERINE_ARRIVE_DIST) {
        mTargetPos = l_chairPos;
        moveParam_c param(l_moveParams[0].mSpeed, l_moveParams[0].mAccel,
                          l_moveParams[0].mDecel);
        if (mTurnWalk) {
            mAng angle = targetAngleY(&mPos, &mTargetPos);
            mAction.requestTurnWalk(PRIORITY_DEMO, mTargetPos, angle,
                                    l_turnSpeed, 0, param, l_defaultAnmRate,
                                    cNpcMorphFrames);
        } else {
            mAction.requestWalk(PRIORITY_DEMO, mTargetPos, l_defaultAnmRate,
                                l_turnSpeed, 0, param, cNpcMorphFrames);
        }
        mWalking = 1;
        mCatherineTalk.mKind = talk_c::KIND_SERVE;
        setDemoFlag8();
        dPlayerMgr::endDemoWait();
        dDemo::requestSpeak(this, NULL);
    }
}

namespace dANpcSpCatherine {
static const dAcNpcSpCatherine_c::Proc l_toChairSteps[] = {
    &dAcNpcSpCatherine_c::stepToChairStart,
    &dAcNpcSpCatherine_c::stepToChairWalk,
};
} // namespace dANpcSpCatherine

// .text 0xD04
void dAcNpcSpCatherine_c::executeToChair() {
    if (mStep < TO_CHAIR_STEP_NUM) {
        (this->*l_toChairSteps[mStep])();
    }
}

// .text 0xD44
void dAcNpcSpCatherine_c::initServed() {
    dCamera_c::sInstance->restoreModeMedium();
    mAction.requestTurn(PRIORITY_IDLE, mAng(0), l_turnSpeed, 0,
                        cNpcMorphFrames);
    mStep = SERVED_STEP_TURN;
}

// .text 0xDB0
void dAcNpcSpCatherine_c::stepServedTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mServing = 0;
        changeState(STATE_IDLE);
    }
}

namespace dANpcSpCatherine {
static const dAcNpcSpCatherine_c::Proc l_servedSteps[] = {
    &dAcNpcSpCatherine_c::stepServedTurn,
};
} // namespace dANpcSpCatherine

// .text 0xDDC
void dAcNpcSpCatherine_c::executeServed() {
    if (mStep < SERVED_STEP_NUM) {
        (this->*l_servedSteps[mStep])();
    }
}

// .text 0xE1C
void dAcNpcSpCatherine_c::initToCounter() {
    dCamera_c::sInstance->restoreModeMedium();
    mTargetPos = l_frontPos;
    mAng angle = targetAngleY(&mPos, &mTargetPos);
    moveParam_c param(l_moveParams[3].mSpeed, l_moveParams[3].mAccel,
                      l_moveParams[3].mDecel);
    mAction.requestTurnWalk(PRIORITY_IDLE, mTargetPos, angle, l_turnSpeed, 0,
                            param, l_defaultAnmRate, cNpcMorphFrames);
    mTurnWalk = 0;
    mStep = TO_COUNTER_STEP_FRONT;
}

// .text 0xEE0
void dAcNpcSpCatherine_c::stepToCounterFront() {
    if (mPos.xzDistTo(mTargetPos) < CATHERINE_ARRIVE_DIST) {
        mTargetPos = l_counterPos;
        moveParam_c param(l_moveParams[3].mSpeed, l_moveParams[3].mAccel,
                          l_moveParams[3].mDecel);
        mAction.requestWalk(PRIORITY_IDLE, mTargetPos, l_defaultAnmRate,
                            l_turnSpeed, 0, param, cNpcMorphFrames);
        mItemsRemoved = 0;
        mStep = TO_COUNTER_STEP_TURN;
    }
}

// .text 0xFC0
void dAcNpcSpCatherine_c::stepToCounterTurn() {
    if (mAction.mActionId == ACTION_WALK && mAction.mCanChange) {
        mAction.requestTurn(PRIORITY_IDLE, mAng(0), l_turnSpeed, 0,
                            cNpcMorphFrames);
        mStep = TO_COUNTER_STEP_DONE;
    }
}

// .text 0x1034
void dAcNpcSpCatherine_c::stepToCounterDone() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        changeState(STATE_IDLE);
    }
}

namespace dANpcSpCatherine {
static const dAcNpcSpCatherine_c::Proc l_toCounterSteps[] = {
    &dAcNpcSpCatherine_c::stepToCounterFront,
    &dAcNpcSpCatherine_c::stepToCounterTurn,
    &dAcNpcSpCatherine_c::stepToCounterDone,
};
} // namespace dANpcSpCatherine

// .text 0x1058
void dAcNpcSpCatherine_c::executeToCounter() {
    if (mStep < TO_COUNTER_STEP_NUM) {
        (this->*l_toCounterSteps[mStep])();
    }
}

// .text 0x1098
void dAcNpcSpCatherine_c::initSeat() {
    mVec3_c pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    pos.z -= CATHERINE_SEAT_STEP;
    dPlayerMgr::moveTo(&pos, dPlayerMgr::PLAYER_INDEX_CURRENT,
                       dPlayerMgr::sDefaultMoveSpeed);
}

// .text 0x10F4
void dAcNpcSpCatherine_c::executeSeat() {
    if (!dPlayerMgr::isMovingTo(dPlayerMgr::PLAYER_INDEX_CURRENT) &&
        !mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0x1144
BOOL dAcNpcSpCatherine_c::isPlayerAtExit() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    if (dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return dBGCF::isNextLocked(
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
}

// .text 0x119C
BOOL dAcNpcSpCatherine_c::isPlayerSeated() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    if (dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return dPlayerMgr::getChairPosition(NULL, dPlayerMgr::PLAYER_INDEX_CURRENT);
}

// .text 0x11F4
dAcNpcSpCatherine_c::talk_c::talk_c(dAcNpcSpCatherine_c *npc) {
    mpOwner = npc;
    init();
    mFinishMsg = MSG_NONE;
    mKind = KIND_TALK;
    mMiiIndex = MII_INDEX_NONE;
    mExitRequested = 0;
    mAccepted = 0;
    mMakeover = 0;
    mSpecialMake = 0;
    mItemsRemoved = 0;
    mMiiMaskRemoved = 0;
    mAsked = 0;
    mAtMachineFront = 0;
    mAtChairFront = 0;
    mHair = 0;
    mHairColor = 0;
}

// .text 0x12AC
dAcNpcSpCatherine_c::talk_c::~talk_c() {
}

// .text 0x1320
void dAcNpcSpCatherine_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = MSG_NONE;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mProc = NULL;
}

// Her customer's news for the friend towns (dSvVillagerMove).
static inline void setPlayerValue() {
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mVillagerMove.setPlayerValue(dPlayerMgr::getCurrentPlayerSlot(),
                                       dSvVillagerMove::PLAYER_VALUE_HARRIET);
}

// Harriet's record of the town's trends and her masterpieces.
static inline dSaveCatherine_c *getCatherine() {
    return &dSaveData_c::getTown()->mShops.mCatherine;
}

// The hairstyle and colour of her masterpiece for this gender.
static inline u8 getMasterpieceHair(int gender) {
    dSaveTown_c *town = dSaveData_c::getTown();
    return town->mShops.mCatherine.mCategory[gender];
}
static inline u8 getMasterpieceColor(int gender) {
    dSaveTown_c *town = dSaveData_c::getTown();
    return town->mShops.mCatherine.mSub[gender];
}

// The player of a friend town she made the masterpiece for.
static inline dPersonalID_c *getMasterpiecePlayer(int gender) {
    dSaveTown_c *town = dSaveData_c::getTown();
    return &town->mShops.mCatherine.mPersons[gender];
}

// The current player has the price in the pockets.
static inline BOOL canPay(dPrivateData_c *player, int price) {
    return player->getPocketMoney() >= price;
}

// .text 0x13A0
void dAcNpcSpCatherine_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    mHairGender = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID.player.mGender;
    switch (mKind) {
    case KIND_GREET:
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_HARRIET_MET)) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                    PRIVATE_FLAG1_HARRIET_STYLED)) {
                mMsgNo = MSG_WELCOME_BACK;
            } else {
                mMsgNo = MSG_WELCOME;
            }
        } else {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_HARRIET_MET);
            mMsgNo = MSG_FIRST_VISIT;
        }
        break;
    case KIND_SEE_OFF:
        if (mAccepted) {
            if (mMakeover && !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                                 PRIVATE_FLAG0_HARRIET_MAKEOVER_DONE)) {
                mMsgNo = MSG_BYE_TRIAL;
            } else {
                mMsgNo = MSG_BYE_HURRY;
            }
        } else {
            mMsgNo = MSG_BYE;
        }
        break;
    case KIND_TALK:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_HARRIET_STYLED)) {
            mMsgNo = mpOwner->mAfterStyleMsg;
        } else if (mAccepted) {
            mMsgNo = MSG_GO_SIT;
        } else if (mAsked) {
            mMsgNo = MSG_ASK_SERVICE_AGAIN;
        } else {
            mAsked = 1;
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID.player.mGender ==
                GENDER_MALE) {
                mMsgNo = MSG_ASK_SERVICE;
            } else {
                mMsgNo = MSG_ASK_SERVICE_F;
            }
        }
        break;
    case KIND_SERVE: {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (mMakeover) {
            if (player->isFlag0(PRIVATE_FLAG0_HARRIET_MAKEOVER_DONE) &&
                !canPay(player, l_param.mPrice)) {
                mMsgNo = MSG_PAY_LATER;
            } else {
                mpOwner->mServing = 1;
                if (player->mMiiMask) {
                    mMsgNo = MSG_REMOVE_MII_MASK;
                } else if (player->mEquipment.mHat.getId() !=
                               dItem::ITEM_ID_NONE ||
                           player->mEquipment.mAcc.getId() !=
                               dItem::ITEM_ID_NONE) {
                    int worn = 0;
                    if (player->mEquipment.mHat.getId() !=
                        dItem::ITEM_ID_NONE) {
                        worn = 1;
                    }
                    if (player->mEquipment.mAcc.getId() !=
                        dItem::ITEM_ID_NONE) {
                        worn++;
                    }
                    if (player->countPockets(NULL, NULL) >= worn) {
                        mMsgNo = MSG_MAKEOVER_REMOVE_ITEMS;
                    } else {
                        mMsgNo = MSG_MAKEOVER_FORGOT;
                    }
                } else {
                    mMsgNo = MSG_PICK_MII;
                }
            }
        } else if (!canPay(player, l_param.mPrice)) {
            mMsgNo = MSG_PAY_LATER;
        } else if (player->mEquipment.mHat.getId() != dItem::ITEM_ID_NONE ||
                   player->mEquipment.mAcc.getId() != dItem::ITEM_ID_NONE ||
                   player->mMiiMask) {
            mMsgNo = MSG_REMOVE_ITEMS;
        } else {
            mpOwner->mServing = 1;
            int visits = player->mHarrietVisits;
            if (visits >= VISITS_GENDER_SWAP) {
                if (player->mPID.player.mGender == GENDER_MALE) {
                    mMsgNo = MSG_SWAP_TO_GIRL;
                } else {
                    mMsgNo = MSG_SWAP_TO_BOY;
                }
            } else if (visits >= 1) {
                if (dSaveData_c::getRaw()->mExtra.mHeader.mNetEnabled &&
                    getCatherine()->isValid(mHairGender)) {
                    mMsgNo = MSG_HAVE_IDEA;
                } else {
                    mMsgNo = MSG_HOW_HAIR;
                }
            } else {
                mMsgNo = MSG_HOW_HAIR;
            }
        }
        break;
    }
    case KIND_SEATED:
        if (mAccepted) {
            mMsgNo = MSG_SIT_AT_MACHINE;
        } else {
            mMsgNo = MSG_NO_WAIT;
        }
        break;
    }
    mFinishMsg = MSG_NONE;
}

// .text 0x1770
void dAcNpcSpCatherine_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_catherine";
    info->mCode = mMsgNo;
}

// .text 0x1788
void dAcNpcSpCatherine_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_MASTERPIECE_INTRO: {
        dPersonalID_c *person = getMasterpiecePlayer(mHairGender);
        mpController->setLandName(0, &person->land);
        mpController->setPersonalName(1, person);
        break;
    }
    case MSG_CONFIRM_MII: {
        dPlayerMgr::MiiData_c info;
        RFLGetCharInfo((RFLCharInfo *)&info, RFLDataSource_Official, NULL,
                       getMiiIndex());
        dString::Word_c word;
        word.set(info.mName, 0);
        mpController->setWord(0, &word);
        break;
    }
    case MSG_DONE:
    case MSG_DONE_SAME:
    case MSG_DONE_TREND:
    case MSG_DONE_OWN_COLOR:
    case MSG_DONE_MASTERPIECE:
    case MSG_DONE_MASTERPIECE_AGAIN:
    case MSG_MAKEOVER_DONE_TRIAL:
    case MSG_MAKEOVER_DONE:
        dBgm::l_mgr.mStgBarber.fadeUnmute();
        break;
    }
}

// .text 0x18C8
void dAcNpcSpCatherine_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_TAKE_SEAT:
        mpController->setNextMsgCode(MSG_TO_SEAT);
        break;
    case MSG_TO_SEAT:
        mAccepted = 1;
        mMakeover = 0;
        break;
    case MSG_REMOVE_ITEMS:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiMask) {
            mOnMessageEnd = &talk_c::removeMiiMask;
        } else {
            mOnMessageEnd = &talk_c::hideAccessories;
        }
        mOnRequestEnd = &talk_c::askHair;
        break;
    case MSG_NOT_SURE:
        dSaveTown_c *town = dSaveData_c::getTown();
        u8 hair = dPlayerMgr::Mgr_c::getCurrentPlayer()->mHair;
        if (hair != town->mShops.mCatherine.getTopCategory(mHairGender)) {
            mpController->setNextMsgCode(MSG_TRENDY);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mHair !=
                   getMasterpieceHair(mHairGender)) {
            mpController->setNextMsgCode(MSG_MASTERPIECE_INTRO);
        } else {
            mpController->setNextMsgCode(MSG_QUESTIONS);
        }
        break;
    case MSG_TREND_COLOR:
        mHair = getCatherine()->getTopCategory(mHairGender);
        break;
    case MSG_SAME_COLOR:
    case MSG_SAME_COLOR_2:
        mHair = getMasterpieceHair(mHairGender);
        break;
    case MSG_TREND_DONE:
        mFinishMsg = MSG_DONE_TREND;
        mHairColor = getCatherine()->getTopSub();
        break;
    case MSG_RETURN_ITEMS:
        if (mMiiMaskRemoved) {
            mOnMessageEnd = &talk_c::restoreMiiMask;
        } else {
            mOnMessageEnd = &talk_c::restoreEquip;
        }
        mOnRequestEnd = &talk_c::endReturnItems;
        break;
    case MSG_SAME_COLOR_YES:
        mFinishMsg = MSG_DONE_MASTERPIECE;
        mHairColor = getMasterpieceColor(mHairGender);
        break;
    case MSG_SAME_COLOR_YES_2:
        mFinishMsg = MSG_DONE_MASTERPIECE_AGAIN;
        mHairColor = getMasterpieceColor(mHairGender);
        break;
    case MSG_OWN_COLOR:
        mFinishMsg = MSG_DONE_OWN_COLOR;
        break;
    case MSG_PICK_COLOR:
        if (mMessedUp) {
            mFinishMsg = MSG_DONE_MASTERPIECE_AGAIN;
        } else {
            mFinishMsg = MSG_DONE_MASTERPIECE;
        }
        break;
    case MSG_MASTERPIECE:
        mMessedUp = 0;
        break;
    case MSG_MASTERPIECE_AGAIN:
        mMessedUp = 1;
        break;
    case MSG_DONE:
    case MSG_DONE_SAME:
    case MSG_DONE_TREND:
    case MSG_DONE_OWN_COLOR:
    case MSG_DONE_MASTERPIECE:
    case MSG_DONE_MASTERPIECE_AGAIN:
        if (mItemsRemoved) {
            mReturnMsg = MSG_RETURN_ITEMS;
            mOnMessageEnd = &talk_c::resumeTalk;
            mOnRequestEnd = &talk_c::startReturn;
        }
        dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(l_param.mPrice, FALSE);
        dPlayerMgr::startDemoWait();
        setPlayerValue();
        break;
    case MSG_SWAP_GIRL:
    case MSG_SWAP_BOY:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID.player.mGender ==
            GENDER_MALE) {
            mHairGender = HAIR_GENDER_FEMALE;
        } else {
            mHairGender = HAIR_GENDER_MALE;
        }
    case MSG_SWAP_SORRY:
        if (dSaveData_c::getRaw()->mExtra.mHeader.mNetEnabled &&
            getCatherine()->isValid(mHairGender)) {
            mpController->setNextMsgCode(MSG_HAVE_IDEA);
        } else {
            mpController->setNextMsgCode(MSG_HOW_HAIR_2);
        }
        break;
    case MSG_START_STYLE:
    case MSG_START_STYLE_2:
        mOnMessageEnd = &talk_c::resumeTalk;
        mOnRequestEnd = &talk_c::startStyling;
        break;
    case MSG_MAKEOVER_FIRST:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_HARRIET_MAKEOVER_TOLD);
        break;
    case MSG_MAKEOVER_SEAT:
        mAccepted = 1;
        mMakeover = 1;
        break;
    case MSG_REMOVE_MII_MASK:
        mOnMessageEnd = &talk_c::removeMiiMask;
        mOnRequestEnd = &talk_c::askMii;
        break;
    case MSG_MAKEOVER_REMOVE_ITEMS:
        mOnMessageEnd = &talk_c::takeOffAccessories;
        mOnRequestEnd = &talk_c::askMii;
        break;
    case MSG_MAKEOVER_FORGOT:
    case MSG_PAY_LATER:
        mKind = KIND_CANCELLED;
        break;
    case MSG_PICK_MII: {
        u16 index;
        if (!RFLSearchOfficialData(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiId,
                         &index)) {
            mMiiIndex = MII_INDEX_NONE;
        } else {
            mMiiIndex = getMiiIndex();
        }
    }
    case MSG_PICK_MII_AGAIN:
    case MSG_MIX_IT_UP:
        mOnMessageEnd = &talk_c::openMiiAlbum;
        mOnRequestEnd = &talk_c::confirmMii;
        break;
    case MSG_START_MAKEOVER:
        mOnMessageEnd = &talk_c::resumeTalk;
        mOnRequestEnd = &talk_c::startMakeover;
        break;
    case MSG_MAKEOVER_DONE_TRIAL:
        dPlayerMgr::startDemoWait();
        setPlayerValue();
        break;
    case MSG_MAKEOVER_DONE:
        dPlayerMgr::startDemoWait();
        dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(l_param.mPrice, FALSE);
        setPlayerValue();
        break;
    case MSG_BYE:
    case MSG_BYE_HURRY:
    case MSG_BYE_TRIAL_NO:
        dMsg::Comp_c::mInstance->mBgmKeep = 1;
        mExitRequested = 1;
        break;
    case MSG_QUESTIONNAIRE:
        if (mHairGender == HAIR_GENDER_FEMALE) {
            mpController->setNextMsgCode(MSG_QUESTION_GIRL);
        } else {
            mpController->setNextMsgCode(MSG_QUESTION_BOY);
        }
        break;
    case MSG_RESULT_GIRL_STAYING_POWER:
        mHair = 0x11;
        break;
    case MSG_RESULT_GIRL_NATURAL:
        mHair = 0x16;
        break;
    case MSG_RESULT_GIRL_YOUNG:
        mHair = 0x18;
        break;
    case MSG_RESULT_GIRL_INNOCENT:
        mHair = 0x0F;
        break;
    case MSG_RESULT_GIRL_ADORED:
        mHair = 0x17;
        break;
    case MSG_RESULT_GIRL_HEART:
        mHair = 0x12;
        break;
    case MSG_RESULT_GIRL_ADORATION:
        mHair = 0x0E;
        break;
    case MSG_RESULT_GIRL_STRONG:
        mHair = 0x15;
        break;
    case MSG_RESULT_GIRL_POP_STAR:
        mHair = 0x0D;
        break;
    case MSG_RESULT_GIRL_MODEL:
        mHair = 0x14;
        break;
    case MSG_RESULT_GIRL_DESIGNER:
        mHair = 0x10;
        break;
    case MSG_RESULT_GIRL_CHARISMA:
        mHair = 0x13;
        break;
    case MSG_RESULT_BOY_DYNAMIC:
        mHair = 0x09;
        break;
    case MSG_RESULT_BOY_EASY:
        mHair = 0x0B;
        break;
    case MSG_RESULT_BOY_YOUNG:
        mHair = 0x07;
        break;
    case MSG_RESULT_BOY_BOY:
        mHair = 0x06;
        break;
    case MSG_RESULT_BOY_BIG_BROTHER:
        mHair = 0x00;
        break;
    case MSG_RESULT_BOY_BIGHEARTED:
        mHair = 0x05;
        break;
    case MSG_RESULT_BOY_BRAVE:
        mHair = 0x01;
        break;
    case MSG_RESULT_BOY_QUICK:
        mHair = 0x02;
        break;
    case MSG_RESULT_BOY_MOVIE_STAR:
        mHair = 0x04;
        break;
    case MSG_RESULT_BOY_ROCK_STAR:
        mHair = 0x08;
        break;
    case MSG_RESULT_BOY_WORK:
        mHair = 0x03;
        break;
    case MSG_RESULT_BOY_CHARISMA:
        mHair = 0x0A;
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

static inline u8 getCurrentPlayerHair() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    return player->mHair;
}
static inline u8 getCurrentPlayerHairColor() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    return player->mHairColor;
}

// .text 0x2028
void dAcNpcSpCatherine_c::talk_c::onAnswer(int arg) {
    static const u8 l_warmColors[CATHERINE_SUB_NUM] = {1, 4, 2, 6};
    static const u8 l_coolColors[CATHERINE_SUB_NUM] = {0, 3, 5, 7};
    u8 answer = mpController->getSelect()->mCursor;
    u16 code = mMessageCode;
    switch (code) {
    case MSG_ASK_SERVICE:
    case MSG_ASK_SERVICE_F:
    case MSG_ASK_SERVICE_AGAIN:
        switch (answer) {
        case ANSWER_HAIR:
            break;
        case ANSWER_NEVER_MIND:
            break;
        case ANSWER_MAKEOVER:
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_HARRIET_MAKEOVER_TOLD)) {
                if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                        PRIVATE_FLAG0_HARRIET_MAKEOVER_DONE)) {
                    mpController->setNextMsgCode(MSG_MAKEOVER_FEE);
                } else {
                    mpController->setNextMsgCode(MSG_MAKEOVER_TRIAL);
                }
            } else {
                mpController->setNextMsgCode(MSG_MAKEOVER_FIRST);
            }
            break;
        }
        break;
    case MSG_PRICE:
        switch (answer) {
        case ANSWER_PAY:
            if (!canPay(dPlayerMgr::Mgr_c::getCurrentPlayer(),
                        l_param.mPrice)) {
                mpController->setNextMsgCode(MSG_NO_MONEY);
            } else {
                mpController->setNextMsgCode(MSG_TAKE_SEAT);
            }
            break;
        case ANSWER_NO:
            break;
        }
        break;
    case MSG_WARM_COLOR:
    case MSG_WARM_COLOR_2:
    case MSG_COOL_COLOR:
    case MSG_COOL_COLOR_2:
        switch (answer) {
        case 0:
        case 1:
        case 2:
        case 3:
            switch (code) {
            case MSG_WARM_COLOR:
            case MSG_WARM_COLOR_2:
                mHairColor = l_warmColors[answer];
                break;
            case MSG_COOL_COLOR:
            case MSG_COOL_COLOR_2:
                mHairColor = l_coolColors[answer];
                break;
            }
            mpController->setNextMsgCode(MSG_TO_DONE);
            if (mFinishMsg == MSG_NONE) {
                if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiMask &&
                    mHair == getCurrentPlayerHair() &&
                    mHairColor == getCurrentPlayerHairColor()) {
                    mFinishMsg = MSG_DONE_SAME;
                } else {
                    mFinishMsg = MSG_DONE;
                }
            }
            break;
        case ANSWER_COLOR_OTHER:
            break;
        }
        break;
    case MSG_TRENDY:
        switch (answer) {
        case ANSWER_YES:
            break;
        case ANSWER_NO:
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mHair !=
                getMasterpieceHair(mHairGender)) {
                mpController->setNextMsgCode(MSG_MASTERPIECE_INTRO);
            } else {
                mpController->setNextMsgCode(MSG_QUESTIONS);
            }
            break;
        }
        break;
    case MSG_MAKEOVER_OFFER:
    case MSG_MAKEOVER_OFFER_2:
    case MSG_MAKEOVER_TRIAL:
    case MSG_MAKEOVER_FEE:
        switch (answer) {
        case ANSWER_YES:
            if (dSys::isFaceError()) {
                mpController->setNextMsgCode(MSG_MII_BROKEN);
            } else if (!dPlayerMgr::canUseMiiMask()) {
                mpController->setNextMsgCode(MSG_NO_MII);
            } else if (mMessageCode == MSG_MAKEOVER_FEE &&
                       !canPay(dPlayerMgr::Mgr_c::getCurrentPlayer(),
                               l_param.mPrice)) {
                mpController->setNextMsgCode(MSG_NO_MONEY);
            } else {
                int worn = 0;
                dItem::Item hat =
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHat;
                dItem::Item acc =
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mAcc;
                if (hat.getId() != dItem::ITEM_ID_NONE) {
                    worn = 1;
                }
                if (acc.getId() != dItem::ITEM_ID_NONE) {
                    worn++;
                }
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                        NULL, NULL) < worn) {
                    mpController->setNextMsgCode(MSG_POCKETS_FULL);
                } else {
                    mpController->setNextMsgCode(MSG_MAKEOVER_SEAT);
                }
            }
            break;
        case ANSWER_NO:
            break;
        }
        break;
    case MSG_SAME_MII:
        switch (answer) {
        case ANSWER_MIX_IT_UP:
            break;
        case ANSWER_FINE:
            mMsgNo = MSG_START_MAKEOVER;
            mpController->setNextMsgCode(MSG_START_MAKEOVER);
            break;
        }
        break;
    }
}

// .text 0x23E0
void dAcNpcSpCatherine_c::talk_c::preExecute() {
    if (mProc) {
        (this->*mProc)();
    }
}

// .text 0x2428
void dAcNpcSpCatherine_c::talk_c::onTalkEnd() {
    switch (mKind) {
    case KIND_SERVE:
        if (mMakeover) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_HARRIET_MAKEOVER_DONE);
        } else {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            if (player->mHarrietVisits < VISITS_GENDER_SWAP) {
                player->mHarrietVisits++;
            }
        }
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
            PRIVATE_FLAG1_HARRIET_STYLED);
        mAccepted = 0;
        mpOwner->clearDemoFlag8();
        mpOwner->mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep,
                                   l_lookYawStep, cNpcLookRange);
        dPlayerMgr::changeSeatedPose(0, 0);
        break;
    case KIND_SEATED:
        mpOwner->clearDemoFlag8();
        dPlayerMgr::changeSeatedPose(0, 0);
        break;
    case KIND_CANCELLED:
        dCamera_c::sInstance->mRequestedMode =
            dCamera_c::sInstance->mDefaultMode;
        mAccepted = 0;
        mMakeover = 0;
        mpOwner->mTurnWalk = 1;
        mpOwner->mServing = 0;
        mpOwner->clearDemoFlag8();
        dPlayerMgr::changeSeatedPose(0, 0);
        break;
    }
    switch (mMessageCode) {
    case MSG_BYE:
    case MSG_BYE_HURRY:
    case MSG_BYE_TRIAL_NO:
        getSceneChange()->requestExit(0, 2, 2);
        break;
    case MSG_BYE_TRIAL_YES:
        mpOwner->changeState(STATE_SEAT);
        break;
    }
    mKind = KIND_TALK;
}

// .text 0x25BC
void dAcNpcSpCatherine_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x2620
void dAcNpcSpCatherine_c::talk_c::openMiiAlbum() {
    reqMenu0A(FALSE);
}

// .text 0x2628
void dAcNpcSpCatherine_c::talk_c::confirmMii() {
    u16 index = getMiiIndex();
    if (mMiiIndex == index) {
        mMsgNo = MSG_SAME_MII;
    } else {
        mMsgNo = MSG_CONFIRM_MII;
    }
    mpController->setNextMsgCode(mMsgNo);
}

// The window stays open for the request (a next message must follow).
static inline void holdMsg(dMsg::Comp_c *ctrl) {
    if (ctrl->mNextMsgCode == dMsg::MSG_CODE_NONE) {
        ctrl->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
}

// .text 0x2684
void dAcNpcSpCatherine_c::talk_c::resumeTalk() {
    holdMsg(mpController);
    setRequest1(0);
}

// .text 0x26DC
void dAcNpcSpCatherine_c::talk_c::startStyling() {
    dCamera_c::sInstance->mRequestedMode = dCamera_c::sInstance->mDefaultMode;
    dBgm::l_mgr.mStgBarber.mute();
    startToMachine();
}

// .text 0x272C
void dAcNpcSpCatherine_c::talk_c::startMakeover() {
    mSpecialMake = 1;
    dCamera_c::sInstance->mRequestedMode = dCamera_c::sInstance->mDefaultMode;
    dBgm::l_mgr.mStgBarber.mute();
    startToMachine();
}

// .text 0x2784
void dAcNpcSpCatherine_c::talk_c::stopProc() {
    mProc = NULL;
}

// .text 0x27A4
void dAcNpcSpCatherine_c::talk_c::takeOffAccessories() {
    holdMsg(mpController);
    dItem::Item hat = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHat;
    if (hat.getId() != dItem::ITEM_ID_NONE &&
        hat.getKind() != dItem::KIND_ORG_CAP) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(hat, FALSE);
    }
    dItem::Item acc = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mAcc;
    if (acc.getId() != dItem::ITEM_ID_NONE) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(acc, FALSE);
    }
    mEquipOrig.setFromPlayer();
    mEquip.mHeld = mEquipOrig.mHeld;
    mEquip.mShirt = mEquipOrig.mShirt;
    mEquip.mHat = dItem::ITEM_ID_NONE;
    mEquip.mAcc = dItem::ITEM_ID_NONE;
    requestEquip(&mEquip, 0, dPlayerMgr::EQUIP_CHANGE_NORMAL);
}

// .text 0x2884
void dAcNpcSpCatherine_c::talk_c::askMii() {
    mMsgNo = MSG_PICK_MII;
    mpController->setNextMsgCode(MSG_PICK_MII);
}

// .text 0x2898
void dAcNpcSpCatherine_c::talk_c::hideAccessories() {
    holdMsg(mpController);
    mEquipOrig.setFromPlayer();
    mEquip.mHeld = mEquipOrig.mHeld;
    mEquip.mShirt = mEquipOrig.mShirt;
    mEquip.mHat = dItem::ITEM_ID_NONE;
    mEquip.mAcc = dItem::ITEM_ID_NONE;
    requestEquip(&mEquip, 0, dPlayerMgr::EQUIP_CHANGE_NORMAL);
}

// .text 0x2920
void dAcNpcSpCatherine_c::talk_c::removeMiiMask() {
    holdMsg(mpController);
    mMiiMaskRemoved = 1;
    mEquipOrig.setFromPlayer();
    mEquip.mHeld = mEquipOrig.mHeld;
    mEquip.mShirt = mEquipOrig.mShirt;
    mEquip.mHat = dItem::ITEM_ID_NONE;
    mEquip.mAcc = dItem::ITEM_ID_NONE;
    requestEquip(&mEquip, 0, dPlayerMgr::EQUIP_CHANGE_MII_MASK_OFF);
}

// .text 0x29B0
void dAcNpcSpCatherine_c::talk_c::askHair() {
    mItemsRemoved = 1;
    mpOwner->mItemsRemoved = 1;
    int visits = dPlayerMgr::Mgr_c::getCurrentPlayer()->mHarrietVisits;
    if (visits >= VISITS_GENDER_SWAP) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID.player.mGender ==
            GENDER_MALE) {
            mMsgNo = MSG_SWAP_TO_GIRL;
        } else {
            mMsgNo = MSG_SWAP_TO_BOY;
        }
    } else if (visits >= 1) {
        if (dSaveData_c::getRaw()->mExtra.mHeader.mNetEnabled &&
            getCatherine()->isValid(mHairGender)) {
            mMsgNo = MSG_HAVE_IDEA;
        } else {
            mMsgNo = MSG_HOW_HAIR;
        }
    } else {
        mMsgNo = MSG_HOW_HAIR;
    }
    mpController->setNextMsgCode(mMsgNo);
}

// .text 0x2A88
void dAcNpcSpCatherine_c::talk_c::restoreEquip() {
    holdMsg(mpController);
    mEquip = mEquipOrig;
    requestEquip(&mEquip, 0, dPlayerMgr::EQUIP_CHANGE_NORMAL);
}

// .text 0x2B08
void dAcNpcSpCatherine_c::talk_c::restoreMiiMask() {
    holdMsg(mpController);
    mEquip = mEquipOrig;
    requestEquip(&mEquip, 0, dPlayerMgr::EQUIP_CHANGE_MII_MASK_ON);
}

// .text 0x2B88
void dAcNpcSpCatherine_c::talk_c::endReturnItems() {
    mItemsRemoved = 0;
    mMiiMaskRemoved = 0;
    mMsgNo = MSG_NONE;
    mpController->setNextMsgCode(MSG_NONE);
}

// .text 0x2BA4
void dAcNpcSpCatherine_c::talk_c::startReturn() {
    dCamera_c::sInstance->restoreModeMedium();
    startToChair();
}

// .text 0x2BE0
void dAcNpcSpCatherine_c::talk_c::startToMachine() {
    dAcNpcSpCatherine_c *owner = mpOwner;
    owner->mTargetPos = l_frontPos;
    mAng angle = targetAngleY(&owner->mPos, &owner->mTargetPos);
    moveParam_c param(l_moveParams[1].mSpeed, l_moveParams[1].mAccel,
                      l_moveParams[1].mDecel);
    owner->mAction.requestTurnWalk(PRIORITY_DEMO, owner->mTargetPos, angle,
                                   l_turnSpeed, 0, param, l_defaultAnmRate,
                                   cNpcMorphFrames);
    mProc = &talk_c::execToMachine;
}

// .text 0x2CB4
void dAcNpcSpCatherine_c::talk_c::execToMachine() {
    dAcNpcSpCatherine_c *owner = mpOwner;
    if (!mAtMachineFront) {
        if (owner->mPos.xzDistTo(owner->mTargetPos) < CATHERINE_ARRIVE_DIST) {
            owner->mTargetPos = l_counterPos;
            moveParam_c param(l_moveParams[1].mSpeed, l_moveParams[1].mAccel,
                              l_moveParams[1].mDecel);
            owner->mAction.requestWalk(PRIORITY_DEMO, owner->mTargetPos,
                                       l_defaultAnmRate, l_turnSpeed, 0, param,
                                       cNpcMorphFrames);
            mAtMachineFront = 1;
        }
    } else if (owner->mAction.mActionId == ACTION_WALK &&
               owner->mAction.mCanChange) {
        startSwitchOn();
    }
}

// .text 0x2DC4
void dAcNpcSpCatherine_c::talk_c::startSwitchOn() {
    dAcNpcSpCatherine_c *owner = mpOwner;
    mAng angle = owner->getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
    owner->mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                               cNpcMorphFrames);
    mProc = &talk_c::execTurnToMachine;
}

// .text 0x2E4C
void dAcNpcSpCatherine_c::talk_c::execTurnToMachine() {
    dAcNpcSpCatherine_c *owner = mpOwner;
    if (owner->mAction.mActionId == ACTION_TURN && owner->mAction.mCanChange) {
        owner->mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_POO_ON1, 0.0f,
                                  cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
        mProc = &talk_c::execSwitchOn;
    }
}

// .text 0x2ED4
void dAcNpcSpCatherine_c::talk_c::execSwitchOn() {
    dAcNpcSpCatherine_c *owner = mpOwner;
    daRmobjPermMachine_c *machine = daRmobjPermMachine_c::m_instance;
    if (owner->getModel()->mAnm.checkFrame(CATHERINE_SWITCH_FRAME)) {
        machine->startSwitchSound();
    }
    if (owner->mAction.mActionId == ACTION_ANM && owner->mAction.mCanChange) {
        owner->mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        if (mSpecialMake) {
            machine->setStep(daRmobjPermMachine_c::STEP_SPECIAL_MAKE);
            startMiiMask();
        } else {
            machine->setStep(daRmobjPermMachine_c::STEP_PERM);
            startHairChange();
        }
    }
}

// .text 0x2FB0
void dAcNpcSpCatherine_c::talk_c::startToChair() {
    dAcNpcSpCatherine_c *owner = mpOwner;
    owner->mTargetPos = l_frontPos;
    mAng angle = targetAngleY(&owner->mPos, &owner->mTargetPos);
    moveParam_c param(l_moveParams[2].mSpeed, l_moveParams[2].mAccel,
                      l_moveParams[2].mDecel);
    owner->mAction.requestTurnWalk(PRIORITY_DEMO, owner->mTargetPos, angle,
                                   l_turnSpeed, 0, param, l_defaultAnmRate,
                                   cNpcMorphFrames);
    mProc = &talk_c::execToChair;
}

// .text 0x3084
void dAcNpcSpCatherine_c::talk_c::execToChair() {
    dAcNpcSpCatherine_c *owner = mpOwner;
    if (!mAtChairFront) {
        if (owner->mPos.xzDistTo(owner->mTargetPos) < CATHERINE_ARRIVE_DIST) {
            owner->mTargetPos = l_chairPos;
            moveParam_c param(l_moveParams[2].mSpeed, l_moveParams[2].mAccel,
                              l_moveParams[2].mDecel);
            owner->mAction.requestWalk(PRIORITY_DEMO, owner->mTargetPos,
                                       l_defaultAnmRate, l_turnSpeed, 0, param,
                                       cNpcMorphFrames);
            mAtChairFront = 1;
        }
    } else {
        owner->mPos.xzDistTo(owner->mTargetPos);
        if (owner->mAction.mActionId == ACTION_WALK &&
            owner->mAction.mCanChange) {
            mAng angle =
                owner->getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
            owner->mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                                       cNpcMorphFrames);
            mProc = &talk_c::execTurnToPlayer;
        }
    }
}

// .text 0x320C
void dAcNpcSpCatherine_c::talk_c::execTurnToPlayer() {
    dAcNpcSpCatherine_c *owner = mpOwner;
    if (owner->mAction.mActionId == ACTION_TURN && owner->mAction.mCanChange) {
        reqMsgClose();
        mOnRequestEnd = &talk_c::stopProc;
        dCamera_c::sInstance->startTalkView(mpOwner->demoHook58());
        mMsgNo = mReturnMsg;
        mpController->setNextMsgCode(mMsgNo);
        mProc = NULL;
    }
}

// .text 0x32C4
void dAcNpcSpCatherine_c::talk_c::startMiiMask() {
    dPlayerMgr::changeMiiMask(getMiiIndex());
    mProc = &talk_c::execMakeover;
}

// .text 0x3314
void dAcNpcSpCatherine_c::talk_c::execMakeover() {
    if (daRmobjPermMachine_c::m_instance->isSpecialMakeDone() &&
        !dPlayerMgr::isChangingEquipment()) {
        reqMsgClose();
        mOnRequestEnd = &talk_c::stopProc;
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_HARRIET_MAKEOVER_DONE)) {
            mMsgNo = MSG_MAKEOVER_DONE;
        } else {
            mMsgNo = MSG_MAKEOVER_DONE_TRIAL;
        }
        mpController->setNextMsgCode(mMsgNo);
        mProc = NULL;
        dCamera_c::sInstance->startTwoTargetView(mpOwner->demoHook58());
    }
}

// .text 0x33F4
void dAcNpcSpCatherine_c::talk_c::startHairChange() {
    if (requestHairChange(mHair, mHairColor)) {
        mProc = &talk_c::execPerm;
    }
}

// .text 0x344C
void dAcNpcSpCatherine_c::talk_c::execPerm() {
    if (daRmobjPermMachine_c::m_instance->isPermDone()) {
        mProc = NULL;
        finishPerm();
    }
}

// .text 0x34AC
void dAcNpcSpCatherine_c::talk_c::finishPerm() {
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mVillagerMove.setCatherine(dPlayerMgr::getCurrentPlayerSlot(), mHair,
                                     mHairColor);
    reqMsgClose();
    mOnRequestEnd = &talk_c::stopProc;
    mMsgNo = mFinishMsg;
    mpController->setNextMsgCode(mMsgNo);
    mProc = NULL;
    dCamera_c::sInstance->startTwoTargetView(mpOwner->demoHook58());
}

// .text 0x3578
u16 dAcNpcSpCatherine_c::talk_c::getMiiIndex() {
    u16 index;
    if (RFLSearchOfficialData(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiId, &index) ==
        FALSE) {
        return MII_INDEX_NONE;
    }
    return index;
}

// .text 0x35C0
int dAcNpcSpCatherine_c::getEarType() {
    return NPC_EAR_TYPE_HARRIET;
}
