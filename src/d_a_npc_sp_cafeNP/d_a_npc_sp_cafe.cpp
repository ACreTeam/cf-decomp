#include <game/game/d_a_npc_sp_cafe.hpp>
#include <game/framework/f_manager.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_string.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>

extern "C" long labs(long);

// Brewster at the Roost. Functions are in target .text order.

// The serving speeds of the debug menu (RTTI "dNpcCafe_Calibrate_HIO_c"; its
// vtable dtor is emitted before __sinit).
class dNpcCafe_Calibrate_HIO_c {
  public:
    dNpcCafe_Calibrate_HIO_c() {
        _04 = 0;
        _30 = 0;
        mMorph = cNpcMorphFrames;
        for (int i = 0; i < dAcNpcSpCafe_c::talk_c::MODE_NUM; i++) {
            if (i == dAcNpcSpCafe_c::talk_c::MODE_POUR) {
                mTurnFrames[i] = 25.0f;
            } else {
                mTurnFrames[i] = 50.0f;
            }
        }
    }
    virtual ~dNpcCafe_Calibrate_HIO_c() {
    }

    /* 0x04 */ s16 _04;
    /* 0x08 */ f32
        mTurnFrames[dAcNpcSpCafe_c::talk_c::MODE_NUM]; // the frames of a turn
                                                       // (startTurn)
    /* 0x30 */ u8 _30;
    /* 0x34 */ f32 mMorph;
}; // size 0x38

// The parameters of the unit (no RTTI: __sinit registers its implicit, weak
// dtor): the chances (percent) of the coffee talk, plus the calibration above.
struct dNpcCafeParam_c {
    dNpcCafeParam_c() {
        _00 = 0;
        _04 = -1;
        mNearDayRate = 75;
        mWeekRate = 50;
        mMilkRate = 30;
        mKKMorph = 48;
        mCameraFrame = 258.0f;
    }

    /* 0x00 */ int _00;
    /* 0x04 */ int _04;
    /* 0x08 */ int mNearDayRate; // a coffee 2..3 days from the last one
    /* 0x0C */ int mWeekRate;    // ... 4..6 days
    /* 0x10 */ int mMilkRate;    // he offers pigeon milk
    /* 0x14 */ int mKKMorph;     // the morph frames of his K.K. animation
    /* 0x18 */ f32 mCameraFrame; // the drinking frame of the camera change
    /* 0x1C */ dNpcCafe_Calibrate_HIO_c mCalibrate;
}; // size 0x54

namespace dANpcSpCafe {
// .text 0xCC: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpCafe_c *npc) {
    return FALSE;
}

static mAng l_lookFov(0x4000);
static mAng l_lookYawMax(0x27FF);

// The morph frames of the serving animations (l_anmMorph), in service order.
enum AnmMorph_e {
    MORPH_TURN_START,     // startTurn: MODE_START
    MORPH_TAKE_CUP,       // initTakeCup
    MORPH_TURN_TAKE_CUP,  // startTurn: MODE_TAKE_CUP
    MORPH_POUR,           // initPour
    MORPH_TURN_POUR,      // startTurn: MODE_POUR
    MORPH_TURN_POUR_MILK, // startTurn: MODE_POUR_MILK
    MORPH_ADD_MILK,       // initAddMilk
    MORPH_TURN_ADD_MILK,  // startTurn: MODE_ADD_MILK
    MORPH_SERVE,          // initServe
    MORPH_SERVE_MILK,     // initServeMilk
    MORPH_SERVE_WAIT,     // execServe / execServeMilk: the wait after serving
    MORPH_CLEAR,          // initClear
    MORPH_TURN_CLEAR,     // startTurn: MODE_CLEAR
    MORPH_PUT_AWAY,       // initPutAway
    MORPH_TURN_PUT_AWAY,  // startTurn: MODE_PUT_AWAY
    MORPH_POLISH,         // initPolish
    MORPH_NUM
};

static f32 l_anmMorph[MORPH_NUM] = {
    cNpcMorphFrames, 15.0f,           cNpcMorphFrames, 33.0f,
    cNpcMorphFrames, cNpcMorphFrames, cNpcMorphFrames, cNpcMorphFrames,
    15.0f,           cNpcMorphFrames, 30.0f,           15.0f,
    cNpcMorphFrames, cNpcMorphFrames, cNpcMorphFrames, cNpcMorphFrames,
};

// .text 0xD4
BASE_PROFILE(A_NPC_SP_CAFE, dAcNpcSpCafe_c);

static dNpcCafeParam_c l_param;
} // namespace dANpcSpCafe

// .text 0x628
void dAcNpcSpCafe_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeState(STATE_LOOK);
        }
        break;
    case DEMO_CMD_TALK:
        mTalkCafe.mMsgKind = talk_c::MSG_KIND_TALK;
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mTalkCafe.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_SPEAK:
        mTalkCafe.mMsgKind = talk_c::MSG_KIND_SPEAK;
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mTalkCafe.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_END:
        dMoney_c::offForceShow();
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

namespace dANpcSpCafe {
static const dAcNpcSpCafe_c::State_s l_states[] = {
    {&dAcNpcSpCafe_c::initIdle, &dAcNpcSpCafe_c::executeIdle},
    {&dAcNpcSpCafe_c::initLook, &dAcNpcSpCafe_c::executeLook},
    {&dAcNpcSpCafe_c::initTalk, &dAcNpcSpCafe_c::executeTalk},
    {&dAcNpcSpCafe_c::initRemote, &dAcNpcSpCafe_c::executeRemote},
    {&dAcNpcSpCafe_c::initRemoteTalk, &dAcNpcSpCafe_c::executeRemoteTalk}};
} // namespace dANpcSpCafe

// .text 0x7A8
void dAcNpcSpCafe_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpCafe::l_states[state].mInit)();
}

// .text 0x7E0
int dAcNpcSpCafe_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mTalkCafe);
    mAnmSet.setAnm0(HMN_ANM_PGE_WAIT1);
    mAnmSet.setAnm1(HMN_ANM_PGE_WALK1);
    return 1;
}

// .text 0x844
int dAcNpcSpCafe_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    bool created = true;
    created &= mPolishCupMdl.create("obj_cafe1", 0);
    created &= mClothMdl.create("obj_cafe2", 0);
    created &= mCupMdl.create("obj_cafe3", 0);
    created &= mSaucerMdl.create("obj_cafe4", 0);
    created &= mMakerMdl.create("obj_cafe5", 0);
    created &= mCup2Mdl.create("obj_cafe6", 0);
    if (!created) {
        return 0;
    }
    if (isMultiPlay()) {
        mLookAt.setFront(PRIORITY_DEMO, l_lookPitchStep, l_lookYawStep);
    } else {
        mLookAt.setPlayerNo(PRIORITY_DEMO, dNet::getMyNo(), true,
                            l_lookPitchStep, l_lookYawStep, 112.0f);
        mLookAt.mFov = dANpcSpCafe::l_lookFov.labs();
        mLookAt.mYaw.mMax = dANpcSpCafe::l_lookYawMax.mAngle;
    }
    mLookedAt = 0;
    mPolishCupMdl.setAnmChr("obj_cafe1_0", m3d::FORWARD_LOOP, 0.0f);
    mClothMdl.setAnmChr("obj_cafe2_0", m3d::FORWARD_LOOP, 0.0f);
    mCupMdl.setAnmChr("obj_cafe3_0", m3d::FORWARD_LOOP, 0.0f);
    mVec3_c zero(0.0f, 0.0f, 0.0f);
    mCupMtx.trans(zero);
    mSaucerMdl.setAnmChr("obj_cafe4_0", m3d::FORWARD_LOOP, 0.0f);
    mSaucerMtx.trans(zero);
    mMakerMdl.setAnmChr("obj_cafe5_0", m3d::FORWARD_ONCE, 0.0f);
    mMakerMdl.setRateChr(0.0f);
    mMakerMtx.trans(mBasePos);
    mMakerMtx.YrotM(-0xAAB);
    mCup2Mdl.setAnmChr("obj_cafe3_0", m3d::FORWARD_LOOP, 0.0f);
    mCup2Mtx.trans(zero);
    setInteractionRadius(80.0f);
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        mDaub.mBusy = 0;
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    mKKAnm = 0;
    dSceneCheck::fn_80081858(dANpcSpCafe::entryDebugPage(this));
    return 1;
}

// .text 0xB54
int dAcNpcSpCafe_c::doDelete() {
    dANpcSpCafe::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    bool removed = true;
    removed &= mPolishCupMdl.remove();
    removed &= mClothMdl.remove();
    removed &= mCupMdl.remove();
    removed &= mSaucerMdl.remove();
    removed &= mMakerMdl.remove();
    removed &= mCup2Mdl.remove();
    if (!removed) {
        return 0;
    }
    if (!dAcNpcSp_c::doDelete()) {
        return 0;
    }
    return 1;
}

// .text 0xC30
int dAcNpcSpCafe_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (dBgm::l_mgr.mStgCafe.isKKPlaying()) {
        if (mDaub.mBusy) {
            mAcceptMoveReq = 0;
        } else if (isInUse(NULL) || mTalk.isBusy()) {
            mAcceptMoveReq = 1;
        } else {
            mAcceptMoveReq = 0;
            if (dBgm::l_mgr.mStgCafe.getKKInfo3() >= 1) {
                if (mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_PGE_WAIT1, 0.0f,
                                       cNpcMorphFrames, m3d::PLAYMODE_INHERIT,
                                       0)) {
                    mKKAnm = 0;
                }
            } else if (mKKAnm) {
                f32 frame = dBgm::l_mgr.mStgCafe.getKKInfo10();
                getModel()->setFrame(frame, FALSE);
            } else {
                if (mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_PGE_WAIT1,
                                       dBgm::l_mgr.mStgCafe.getKKInfo10(),
                                       dANpcSpCafe::l_param.mKKMorph,
                                       m3d::PLAYMODE_INHERIT, 1)) {
                    mKKAnm = 1;
                }
            }
        }
    } else {
        if (mDaub.mBusy) {
            mAcceptMoveReq = 0;
        } else {
            mAcceptMoveReq = 1;
        }
        mKKAnm = 0;
        if (!mTalk.isBusy() && mAction.mActionId != ACTION_WAIT) {
            mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
        }
    }
    if (isMultiPlay()) {
        mLookAt.setFront(PRIORITY_DEMO, l_lookPitchStep, l_lookYawStep);
    } else if (mTalk.isBusy()) {
        mLookedAt = 1;
    } else if (!mLookedAt && mLookAt.isLooking()) {
        mLookedAt = 1;
        mLookAt.setFront(PRIORITY_DEMO, l_lookPitchStep, l_lookYawStep);
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpCafe::l_states[mState].mExecute)();
    }
    if (mPolishing) {
        mPolishCupMtx = mNodeMtx0B;
        mClothMtx = mNodeMtx0E;
    }
    mPolishCupMdl.mMdl.setLocalMtx(&mPolishCupMtx);
    mPolishCupMdl.playMdl();
    mClothMdl.mMdl.setLocalMtx(&mClothMtx);
    mClothMdl.playMdl();
    mCupMdl.mMdl.setLocalMtx(&mCupMtx);
    mCupMdl.playMdl();
    mSaucerMdl.mMdl.setLocalMtx(&mSaucerMtx);
    mSaucerMdl.playMdl();
    mMakerMdl.mMdl.setLocalMtx(&mMakerMtx);
    mMakerMdl.playMdl();
    mCup2Mdl.mMdl.setLocalMtx(&mCup2Mtx);
    mCup2Mdl.playMdl();
    return 1;
}

// .text 0x1034
int dAcNpcSpCafe_c::draw() {
    if (mDrawPolish) {
        mPolishCupMdl.entry();
        mClothMdl.entry();
    }
    if (mDrawCup) {
        if (mUseCup2) {
            mCup2Mdl.entry();
        } else {
            mCupMdl.entry();
        }
        mSaucerMdl.entry();
    }
    mMakerMdl.entry();
    return dAcNpc_c::draw();
}

// .text 0x10BC
int dAcNpcSpCafe_c::canSpeak(dDemoActor_c *actor) {
    if (isInUse(NULL)) {
        return FALSE;
    }
    return !mTalk.isBusy();
}

// .text 0x110C
void dAcNpcSpCafe_c::initIdle() {
    mAction.requestTurn(PRIORITY_IDLE, mAng(-0x4000), l_turnSpeed, 0,
                        cNpcMorphFrames);
}

// .text 0x1154
void dAcNpcSpCafe_c::executeIdle() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    } else if (isPlayerSeated()) {
        setDemoFlag8();
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x11D0
void dAcNpcSpCafe_c::initLook() {
    mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
    mStep = LOOK_STEP_DONE;
}

// .text 0x1214
void dAcNpcSpCafe_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

namespace dANpcSpCafe {
static const dAcNpcSpCafe_c::Proc l_lookSteps[] = {&dAcNpcSpCafe_c::stepLook};
} // namespace dANpcSpCafe

// .text 0x1274
void dAcNpcSpCafe_c::executeLook() {
    if (mStep < LOOK_STEP_DONE) {
        (this->*dANpcSpCafe::l_lookSteps[mStep])();
    }
}

// .text 0x12B4
void dAcNpcSpCafe_c::initTalk() {
    if (mTalkCafe.mMsgKind == talk_c::MSG_KIND_SPEAK) {
        mTalk.requestTalk(false, FALSE);
    } else {
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
    mLookAt.setFront(PRIORITY_DEMO, l_lookPitchStep, l_lookYawStep);
    mStep = TALK_STEP_TURN;
}

// .text 0x135C
void dAcNpcSpCafe_c::stepTalkTurn() {
    if (!mTalk.isBusy()) {
        mAction.requestTurn(PRIORITY_DEMO, mAng(-0x4000), l_turnSpeed, 0,
                            cNpcMorphFrames);
        ++mStep;
    }
}

// .text 0x13CC
void dAcNpcSpCafe_c::stepTalkEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        if (mTalkCafe.mMsgKind == talk_c::MSG_KIND_SPEAK) {
            dPlayerMgr::changeSeatedPose(2, 0);
            clearDemoFlag8();
        }
        dDemo::finishTalk(this);
        ++mStep;
    }
}

namespace dANpcSpCafe {
static const dAcNpcSpCafe_c::Proc l_talkSteps[] = {
    &dAcNpcSpCafe_c::stepTalkTurn, &dAcNpcSpCafe_c::stepTalkEnd};
} // namespace dANpcSpCafe

// .text 0x1440
void dAcNpcSpCafe_c::executeTalk() {
    if (mStep < TALK_STEP_DONE) {
        (this->*dANpcSpCafe::l_talkSteps[mStep])();
    }
}

// .text 0x1480
void dAcNpcSpCafe_c::initRemote() {
}

// .text 0x1484
void dAcNpcSpCafe_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mTalkCafe.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
            return;
        }
        if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
            return;
        }
    }
    if (isPlayerSeated()) {
        setDemoFlag8();
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x1540
void dAcNpcSpCafe_c::initRemoteTalk() {
}

// .text 0x1544
void dAcNpcSpCafe_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0x1598
BOOL dAcNpcSpCafe_c::isPlayerSeated() {
    if (dPlayerMgr::getChairPosition(NULL, dPlayerMgr::PLAYER_INDEX_CURRENT) &&
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT)->z < 352.0f) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x15F0
void dAcNpcSpCafe_c::setBusy() {
    if (isLocalOwner()) {
        mDaub.mBusy = 1;
        setDaubData(&mDaub, sizeof(mDaub));
    }
}

// .text 0x1640
void dAcNpcSpCafe_c::clearBusy() {
    if (isLocalOwner()) {
        mDaub.mBusy = 0;
        setDaubData(&mDaub, sizeof(mDaub));
    }
}

// .text 0x1690
BOOL dAcNpcSpCafe_c::isGyroid(const dItem::Item *item, int flag) {
    if (item->mId != dItem::ITEM_ID_NONE && flag == 0) {
        const dItem::BITM *info = dItem::infoBank_c::get()->getBITM(*item);
        if (info != NULL && info->getKind() == dItem::KIND_HANIWA) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x1704
dAcNpcSpCafe_c::talk_c::talk_c(dAcNpcSpCafe_c *npc) {
    mpOwner = npc;
    init();
    mMsgKind = MSG_KIND_TALK;
    mItem = dItem::ITEM_ID_NONE;
    mServeMsg = 0;
    mReactMsg = 0;
    mThanksMsg = 0;
    mGyroidStored = 0;
    mMode = MODE_START;
}

// .text 0x17C4
dAcNpcSpCafe_c::talk_c::~talk_c() {
}

// .text 0x1850
void dAcNpcSpCafe_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgCode = 0;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mProc = NULL;
    mItem = dItem::ITEM_ID_NONE;
    mValentine = 0;
    mMilk = 0;
    mServed = 0;
    mGyroidStored = 0;
}

// .text 0x1904
void dAcNpcSpCafe_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    dTime_c now = *dTime_c::getCurrent();
    BOOL valentine = FALSE;
    BOOL valentineDay = FALSE;
    BOOL february = FALSE;
    BOOL regular = FALSE;
    if (dAcNpc_c::isPlayerFromThisTown() &&
        dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeVisits() >= 4) {
        regular = TRUE;
    }
    if (regular && now.month == MONTH_FEBRUARY) {
        february = TRUE;
    }
    if (february && now.mday == 14) {
        valentineDay = TRUE;
    }
    if (valentineDay && now.hour >= TIME_DAY_START_HOUR) {
        valentine = TRUE;
    }
    mValentine = valentine;
    mMilk = 0;
    mServed = 0;
    dHomeList_c *speakHomes;
    dPrivateGyroids_c *gyroids;
    dHomeList_c *talkHomes;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dPrivateData_c *rawSpeakPlayer;
    dPrivateData_c *rawTalkPlayer;
    switch (mMsgKind) {
    case MSG_KIND_TALK: {
        gyroids = &player->mGyroids;
        u16 mask = 0;
        int gyroidNum = player->countPocketsFlag(isGyroid, &mask);
        if (dAcNpc_c::isPlayerFromThisTown()) {
            talkHomes = &dSaveData_c::getTown()->mHomes;
            if (talkHomes->getHome(talkHomes->findCurrentPlayer()) == NULL) {
                mMsgCode = MSG_NO_HOME;
            } else if (dNpcInfo::totakeke_c::get()->mPerforming) {
                mMsgCode = MSG_SHOW_QUIET;
            } else if (dNpcInfo::totakeke_c::get()->isPlaying()) {
                mMsgCode = MSG_SHOW_SOON;
            } else if (player->getCafeVisits() < 7 ||
                       (player->getCafeVisits() >= 7 && gyroids->count() == 0 &&
                        gyroidNum == 0)) {
                if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                        PRIVATE_FLAG1_CAFE_TALKED)) {
                    mMsgCode = (dPlayerMgr::Mgr_c::getCurrentPlayer()
                                    ->getCafeVisits() +
                                1) /
                                   2 +
                               MSG_GREETING;
                } else if (isKKOnStage()) {
                    mMsgCode = MSG_PERFORMANCE;
                } else {
                    rawTalkPlayer = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
                    if (rawTalkPlayer->isFlag1(PRIVATE_FLAG1_CAFE_SERVED)) {
                        if (rawTalkPlayer->isFlag1(
                                PRIVATE_FLAG1_CAFE_VALENTINE)) {
                            mMsgCode = MSG_SPECIAL_DAY;
                        } else {
                            mMsgCode = (dPlayerMgr::Mgr_c::getCurrentPlayer()
                                            ->getCafeVisits() +
                                        2) /
                                           3 +
                                       MSG_REGULAR;
                        }
                    } else {
                        mMsgCode = (dPlayerMgr::Mgr_c::getCurrentPlayer()
                                        ->getCafeVisits() +
                                    2) /
                                       3 +
                                   MSG_SEAT;
                    }
                }
            } else if (gyroids->count() > 0) {
                if (gyroidNum != 0) {
                    mMsgCode = cM::rndInt(3) + MSG_GYROID_NICE;
                } else {
                    mMsgCode = cM::rndInt(3) + MSG_GYROID_THANKS;
                }
            } else if (gyroidNum != 0) {
                if (!player->isFlag0(PRIVATE_FLAG0_GYROID_ASKED)) {
                    mMsgCode = MSG_GYROID_ASK;
                    player->setFlag0(PRIVATE_FLAG0_GYROID_ASKED);
                } else {
                    mMsgCode = MSG_GYROID_STORE;
                }
            }
        } else if (!dNpcInfo::cafe_c::get()->mTalked) {
            mMsgCode = MSG_VISITOR_WELCOME;
        } else if (isKKOnStage()) {
            if (dNpcInfo::totakeke_c::get()->mPerforming) {
                mMsgCode = MSG_SHOW_QUIET;
            } else if (dNpcInfo::totakeke_c::get()->isPlaying()) {
                mMsgCode = MSG_SHOW_SOON;
            } else {
                mMsgCode = MSG_PERFORMANCE;
            }
        } else if (dNpcInfo::cafe_c::get()->mServed) {
            mMsgCode = cM::rndInt(5) + MSG_VISITOR_AGAIN;
        } else {
            mMsgCode = MSG_VISITOR_SIT;
        }
        break;
    }
    case MSG_KIND_SPEAK:
        if (dAcNpc_c::isPlayerFromThisTown()) {
            speakHomes = &dSaveData_c::getTown()->mHomes;
            if (speakHomes->getHome(speakHomes->findCurrentPlayer()) == NULL) {
                mMsgCode = MSG_NO_HOME;
            } else if (dNpcInfo::totakeke_c::get()->mPerforming) {
                mMsgCode = MSG_SHOW_QUIET;
            } else if (dNpcInfo::totakeke_c::get()->isPlaying()) {
                mMsgCode = MSG_SHOW_SOON;
            } else {
                rawSpeakPlayer = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
                if (rawSpeakPlayer->isFlag1(PRIVATE_FLAG1_CAFE_SERVED)) {
                    if (rawSpeakPlayer->isFlag1(PRIVATE_FLAG1_CAFE_VALENTINE)) {
                        mMsgCode = MSG_SPECIAL_DAY;
                    } else {
                        mMsgCode = (dPlayerMgr::Mgr_c::getCurrentPlayer()
                                        ->getCafeVisits() +
                                    2) /
                                       3 +
                                   MSG_REGULAR;
                    }
                } else if (mValentine) {
                    mMsgCode = MSG_ASK_VALENTINE;
                } else {
                    mMsgCode = (dPlayerMgr::Mgr_c::getCurrentPlayer()
                                    ->getCafeVisits() +
                                2) /
                                   3 +
                               MSG_ASK_COFFEE;
                }
            }
        } else if (dNpcInfo::totakeke_c::get()->mPerforming) {
            mMsgCode = MSG_SHOW_QUIET;
        } else if (dNpcInfo::totakeke_c::get()->isPlaying()) {
            mMsgCode = MSG_SHOW_SOON;
        } else if (dNpcInfo::cafe_c::get()->mServed) {
            mMsgCode = cM::rndInt(5) + MSG_VISITOR_AGAIN;
        } else {
            mMsgCode = MSG_VISITOR_ASK;
        }
        break;
    }
    mServeMsg = 0;
    mReactMsg = 0;
    mThanksMsg = 0;
    mMode = MODE_START;
}

// .text 0x1EC4
int dAcNpcSpCafe_c::talk_c::getVoiceMode() {
    return mMode == MODE_DRINK;
}

// .text 0x1ED8
void dAcNpcSpCafe_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_master";
    info->mCode = mMsgCode;
}

// .text 0x1EF0
void dAcNpcSpCafe_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_ASK_COFFEE:
    case MSG_ASK_COFFEE + 1:
    case MSG_ASK_COFFEE + 2:
    case MSG_ASK_COFFEE + 3:
    case MSG_ASK_COFFEE + 4:
    case MSG_ASK_COFFEE + 5:
    case MSG_ASK_VALENTINE:
    case MSG_VISITOR_ASK:
        dMoney_c::onForceShow();
        break;
    case MSG_VISITOR_TOWN:
        setLandName(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID.land, 0);
        break;
    case MSG_GYROID_STORED:
    case MSG_GYROID_HAVE:
    case MSG_RETURNED:
    case MSG_RETURNED + 1:
    case MSG_RETURNED + 2:
        setItemName(mItem, 0);
        break;
    }
}

// .text 0x1FB0
void dAcNpcSpCafe_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_ALL_RIGHT:
        mServeMsg = MSG_SERVE_VALENTINE;
        mOnMessageEnd = &talk_c::closeCoffeeMsg;
        mOnRequestEnd = &talk_c::startServe;
        break;
    case MSG_THANKS:
    case MSG_THANKS + 1:
    case MSG_THANKS + 2:
    case MSG_THANKS + 3:
    case MSG_THANKS + 4:
    case MSG_THANKS + 5:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeVisits() >= 5 &&
            cM::rndInt(100) < dANpcSpCafe::l_param.mMilkRate) {
            mpController->setNextMsgCode(MSG_OFFER_MILK);
        } else {
            mServeMsg =
                (u16)(dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeVisits() +
                      2) /
                    3 +
                MSG_SERVE;
            mOnMessageEnd = &talk_c::closeCoffeeMsg;
            mOnRequestEnd = &talk_c::startServe;
        }
        break;
    case MSG_MILK_YES:
        mMilk = 1;
        mServeMsg =
            (u16)(dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeVisits() + 2) /
                3 +
            MSG_SERVE;
        mOnMessageEnd = &talk_c::closeCoffeeMsg;
        mOnRequestEnd = &talk_c::startServe;
        break;
    case MSG_MILK_NO:
        mServeMsg =
            (u16)(dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeVisits() + 2) /
                3 +
            MSG_SERVE;
        mOnMessageEnd = &talk_c::closeCoffeeMsg;
        mOnRequestEnd = &talk_c::startServe;
        break;
    case MSG_VISITOR_SERVE:
        dNpcInfo::cafe_c::get()->setServed();
        mServeMsg = MSG_VISITOR_HERE;
        mOnMessageEnd = &talk_c::closeCoffeeMsg;
        mOnRequestEnd = &talk_c::startServe;
        dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(COFFEE_PRICE, 0);
        break;
    case MSG_REACT_MILK:
    case MSG_REACT:
    case MSG_REACT + 1:
    case MSG_REACT + 2:
    case MSG_REACT + 3:
    case MSG_REACT + 4:
    case MSG_REACT + 5:
    case MSG_REACT + 6:
    case MSG_REACT + 7:
    case MSG_REACT + 8:
    case MSG_REACT + 9:
    case MSG_REACT + 10:
        mThanksMsg = MSG_THANK_YOU;
        mOnMessageEnd = &talk_c::closeCoffeeMsg;
        mOnRequestEnd = &talk_c::startReaction;
        break;
    case MSG_REACT_CHOCOLATE:
        mThanksMsg = MSG_HAPPY_VALENTINE;
        mOnMessageEnd = &talk_c::closeCoffeeMsg;
        mOnRequestEnd = &talk_c::startReaction;
        break;
    case MSG_VISITOR_REACT:
    case MSG_VISITOR_REACT + 1:
    case MSG_VISITOR_REACT + 2:
    case MSG_VISITOR_REACT + 3:
    case MSG_VISITOR_REACT + 4:
        mOnMessageEnd = &talk_c::closeCoffeeMsg;
        mOnRequestEnd = &talk_c::startReaction;
        mThanksMsg = MSG_VISITOR_THANKS;
        break;
    case MSG_BUY:
        mServed = 1;
        if (mValentine) {
            mpController->setNextMsgCode(MSG_ALL_RIGHT);
        } else {
            mpController->setNextMsgCode(
                (u16)(dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeVisits() +
                      2) /
                    3 +
                MSG_THANKS);
        }
        dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(COFFEE_PRICE, 0);
        break;
    case MSG_GYROID_STORED:
    case MSG_GYROID_HAVE: {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        u16 mask = 0;
        int gyroidNum = player->countPocketsFlag(isGyroid, &mask);
        if (gyroidNum != 0) {
            mpController->setNextMsgCode(MSG_GYROID_MORE);
        } else if (gyroidNum == 0) {
            if (mGyroidStored) {
                mMsgCode = cM::rndInt(3) + MSG_GYROID_SAFE;
                mpController->setNextMsgCode(mMsgCode);
            } else {
                mMsgCode = MSG_GYROID_LATER;
                mpController->setNextMsgCode(MSG_GYROID_LATER);
            }
        }
        break;
    }
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x249C
void dAcNpcSpCafe_c::talk_c::onAnswer(int arg) {
    u8 answer = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_ASK_COFFEE:
    case MSG_ASK_COFFEE + 1:
    case MSG_ASK_COFFEE + 2:
    case MSG_ASK_COFFEE + 3:
    case MSG_ASK_COFFEE + 4:
    case MSG_ASK_COFFEE + 5:
    case MSG_ASK_VALENTINE:
        switch (answer) {
        case 0:
            break;
        case 1:
            mpController->setNextMsgCode(
                dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeVisits() / 4 +
                MSG_DECLINED);
            break;
        }
        break;
    case MSG_SERVE:
    case MSG_SERVE + 1:
    case MSG_SERVE + 2:
    case MSG_SERVE + 3:
    case MSG_SERVE + 4:
    case MSG_SERVE + 5:
    case MSG_TOO_HOT:
    case MSG_TOO_HOT + 1:
    case MSG_TOO_HOT + 2:
    case MSG_TOO_HOT + 3:
    case MSG_TOO_HOT + 4:
    case MSG_TOO_HOT + 5:
        switch (answer) {
        case 0:
            if (mMilk) {
                mReactMsg = MSG_REACT_MILK;
            } else {
                mReactMsg = cM::rndInt(11) + MSG_REACT;
            }
            mOnAnswer = &talk_c::closeMsg;
            mpOwner->mTalk.mNoMouth = 1;
            mOnRequestEnd = &talk_c::startDrink;
            break;
        case 1:
            break;
        }
        break;
    case MSG_SERVE_VALENTINE:
    case MSG_TOO_HOT_VALENTINE:
        switch (answer) {
        case 0:
            mReactMsg = MSG_REACT_CHOCOLATE;
            mOnAnswer = &talk_c::closeMsg;
            mpOwner->mTalk.mNoMouth = 1;
            mOnRequestEnd = &talk_c::startDrink;
            break;
        case 1:
            break;
        }
        break;
    case MSG_VISITOR_HERE:
    case MSG_VISITOR_TOO_HOT:
        switch (answer) {
        case 0:
            mReactMsg = cM::rndInt(5) + MSG_VISITOR_REACT;
            mOnAnswer = &talk_c::closeMsg;
            mpOwner->mTalk.mNoMouth = 1;
            mOnRequestEnd = &talk_c::startDrink;
            break;
        case 1:
            break;
        }
        break;
    case MSG_GYROID_ASK:
    case MSG_GYROID_STORE:
        switch (answer) {
        case 0:
            mOnAnswer = &talk_c::selectGyroid;
            mOnRequestEnd = &talk_c::storeGyroid;
            break;
        case 1:
            break;
        }
        break;
    case MSG_GYROID_MORE:
        switch (answer) {
        case 0:
            mOnAnswer = &talk_c::selectGyroid;
            mOnRequestEnd = &talk_c::storeGyroid;
            break;
        case 1:
            if (mGyroidStored) {
                mMsgCode = cM::rndInt(3) + MSG_GYROID_SAFE;
                mpController->setNextMsgCode(mMsgCode);
            } else {
                mMsgCode = MSG_GYROID_LATER;
                mpController->setNextMsgCode(MSG_GYROID_LATER);
            }
            break;
        }
        break;
    case MSG_GYROID_NICE:
    case MSG_GYROID_NICE + 1:
    case MSG_GYROID_NICE + 2:
        switch (answer) {
        case 0:
            mOnAnswer = &talk_c::selectGyroid;
            mOnRequestEnd = &talk_c::storeGyroid;
            break;
        case 1:
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                    NULL, NULL) == 0) {
                mMsgCode = MSG_POCKETS_FULL;
                mpController->setNextMsgCode(MSG_POCKETS_FULL);
            } else {
                mOnAnswer = &talk_c::openGyroidMenu;
                mOnRequestEnd = &talk_c::returnGyroid;
            }
            break;
        case 2: // "No, thanks" (MSG_GYROID_NICE + 1)
            break;
        }
        break;
    case MSG_GYROID_THANKS:
    case MSG_GYROID_THANKS + 1:
    case MSG_GYROID_THANKS + 2:
        switch (answer) {
        case 0:
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                    NULL, NULL) == 0) {
                mMsgCode = MSG_POCKETS_FULL;
                mpController->setNextMsgCode(MSG_POCKETS_FULL);
            } else {
                mOnAnswer = &talk_c::openGyroidMenu;
                mOnRequestEnd = &talk_c::returnGyroid;
            }
            break;
        case 1:
            break;
        }
        break;
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x2984
void dAcNpcSpCafe_c::talk_c::preExecute() {
    if (mProc) {
        (this->*mProc)();
    }
}

// .text 0x29CC
void dAcNpcSpCafe_c::talk_c::onTalkEnd() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        if (mServed == 1) {
            BOOL special = FALSE;
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_CAFE_SERVED)) {
                dTime_c now = *dTime_c::getCurrent();
                dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
                now.add(0, -TIME_DAY_START_HOUR, 0, 0);
                int days = player->mCafeDate.diffDays(&now, FALSE);
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeVisits() <=
                    0) {
                    special = TRUE;
                } else if (days <= 1 && days >= -1) {
                    special = TRUE;
                } else if (days <= 3 && days >= -3) {
                    if (cM::rndInt(100) < dANpcSpCafe::l_param.mNearDayRate) {
                        special = TRUE;
                    }
                } else if (days <= 6 && days >= -6) {
                    if (cM::rndInt(100) < dANpcSpCafe::l_param.mWeekRate) {
                        special = TRUE;
                    }
                }
            } else {
                special = TRUE;
            }
            if (special) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_CAFE_VISIT);
            }
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mCafeDate.setToday(TRUE);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_CAFE_SERVED);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_CAFE_SERVED);
            if (mValentine) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                    PRIVATE_FLAG1_CAFE_VALENTINE);
            }
        }
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
            PRIVATE_FLAG1_CAFE_TALKED);
    } else {
        dNpcInfo::cafe_c::get()->setTalked();
    }
}

// .text 0x2BD4
void dAcNpcSpCafe_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x2C38
void dAcNpcSpCafe_c::talk_c::closeCoffeeMsg() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(0xFFFF);
    }
    setRequest1(0);
    mpOwner->mTalk.mNoMouth = 0;
}

// .text 0x2C9C
void dAcNpcSpCafe_c::talk_c::closeMsg() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(0xFFFF);
    }
    setRequest1(0);
}

// .text 0x2CF4
void dAcNpcSpCafe_c::talk_c::selectGyroid() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(0xFFFF);
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    u16 mask = 0;
    player->countPocketsFlag(isGyroid, &mask);
    mask = ~mask;
    reqSelectItem(mask, 0x22, dMenuMng_c::MENU_ITEM_GYROID, 0);
}

// .text 0x2D80
void dAcNpcSpCafe_c::talk_c::openGyroidMenu() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(0xFFFF);
    }
    reqMenu0D(dMenuMng_c::MENU_HANIWA, 0);
}

// .text 0x2DDC
void dAcNpcSpCafe_c::talk_c::startServe() {
    dMoney_c::offForceShow();
    mpOwner->setBusy();
    startTurn();
}

// .text 0x2E18
void dAcNpcSpCafe_c::talk_c::startDrink() {
    clearSpeakerName();
    startDrinkAnm();
}

// .text 0x2E4C
void dAcNpcSpCafe_c::talk_c::startReaction() {
    dString::Word_c name;
    dString::loadString(&name, dNpcInfo::SP_NAME_BREWSTER,
                        "sys_STRING/STR_SPNpc_name");
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(), 2);
    dCamera_c::sInstance->startTalkView(mpOwner->demoHook58());
    startReturnCup();
}

// .text 0x2EEC
void dAcNpcSpCafe_c::talk_c::storeGyroid() {
    mMsgCode = MSG_GYROID_CHANGE;
    if (!isMenuInvalid()) {
        dPrivateGyroids_c *gyroids;
        int slot = getMenuSelSlot();
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        gyroids = &player->mGyroids;
        dItem::Item item = player->mPockets[slot];
        if (gyroids->isSet(item)) {
            mMsgCode = MSG_GYROID_HAVE;
        } else {
            dItem::Item none;
            player->setPocket(&none, slot, FALSE);
            gyroids->set(item);
            mGyroidStored = 1;
            mMsgCode = MSG_GYROID_STORED;
        }
        mItem = item;
    }
    mpController->setNextMsgCode(mMsgCode);
}

// .text 0x2FD8
void dAcNpcSpCafe_c::talk_c::returnGyroid() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    mMsgCode = MSG_RETURN_LATER;
    if (!isMenuInvalid()) {
        dItem::Item item = getMenuItem();
        player->mGyroids.reset(item);
        mMsgCode = cM::rndInt(3) + MSG_RETURNED;
        mItem = item;
    }
    mpController->setNextMsgCode(mMsgCode);
}

// .text 0x3060
void dAcNpcSpCafe_c::talk_c::stopProc() {
    mProc = NULL;
}

// .text 0x3080
void dAcNpcSpCafe_c::talk_c::startSteam() {
    mProc = &talk_c::execSteam;
}

// .text 0x30A0
void dAcNpcSpCafe_c::talk_c::nextServeMsg() {
    reqMsgClose();
    mOnRequestEnd = &talk_c::startSteam;
    mMsgCode = mServeMsg;
    mpController->setNextMsgCode(mMsgCode);
}

// .text 0x30FC
void dAcNpcSpCafe_c::talk_c::startTurn() {
    dAcNpcSpCafe_c *owner = mpOwner;
    int anm;
    f32 morph;
    if (mMode == MODE_TAKE_CUP) {
        mTurnAngle = -0x4AAB;
        morph = dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_TURN_TAKE_CUP];
        anm = HMN_ANM_PGE_WALK2;
        owner->mCupMdl.setAnmChr("obj_cafe3_2", m3d::FORWARD_LOOP, 0.0f);
        owner->mSaucerMdl.setAnmChr("obj_cafe4_2", m3d::FORWARD_LOOP, 0.0f);
    } else if (mMode == MODE_POUR) {
        mTurnAngle = -0x4000;
        morph = dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_TURN_POUR];
        anm = HMN_ANM_PGE_WALK2;
        owner->mCupMdl.setAnmChr("obj_cafe3_4", m3d::FORWARD_LOOP, 0.0f);
        owner->mSaucerMdl.setAnmChr("obj_cafe4_2", m3d::FORWARD_LOOP, 0.0f);
    } else if (mMode == MODE_POUR_MILK) {
        mTurnAngle = 0;
        morph = dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_TURN_POUR_MILK];
        anm = HMN_ANM_PGE_WALK2;
        owner->mCupMdl.setAnmChr("obj_cafe3_4", m3d::FORWARD_LOOP, 0.0f);
        owner->mSaucerMdl.setAnmChr("obj_cafe4_2", m3d::FORWARD_LOOP, 0.0f);
    } else if (mMode == MODE_ADD_MILK) {
        mTurnAngle = -0x4000;
        morph = dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_TURN_ADD_MILK];
        anm = HMN_ANM_PGE_WALK2;
        owner->mCup2Mdl.setAnmChr("obj_cafe3_4", m3d::FORWARD_LOOP, 0.0f);
        owner->mSaucerMdl.setAnmChr("obj_cafe4_2", m3d::FORWARD_LOOP, 0.0f);
    } else if (mMode == MODE_CLEAR) {
        mTurnAngle = 0;
        anm = HMN_ANM_PGE_WALK2;
        morph = dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_TURN_CLEAR];
        if (mMilk) {
            owner->mCup2Mdl.setAnmChr("obj_cafe3_2", m3d::FORWARD_ONCE, 0.0f);
        } else {
            owner->mCupMdl.setAnmChr("obj_cafe3_2", m3d::FORWARD_ONCE, 0.0f);
        }
        owner->mSaucerMdl.setAnmChr("obj_cafe4_2", m3d::FORWARD_ONCE, 0.0f);
    } else if (mMode == MODE_PUT_AWAY) {
        mTurnAngle = -0x4000;
        morph = dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_TURN_PUT_AWAY];
        anm = HMN_ANM_PGE_WALK1;
        owner->mPolishCupMdl.setAnmChr("obj_cafe1_1", m3d::FORWARD_ONCE, 0.0f);
        owner->mClothMdl.setAnmChr("obj_cafe2_1", m3d::FORWARD_ONCE, 0.0f);
    } else {
        anm = HMN_ANM_PGE_WALK1;
        mTurnAngle = 0;
        morph = dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_TURN_START];
    }
    owner->mAction.requestAnm(PRIORITY_DEMO, anm, 0.0f, morph,
                              m3d::PLAYMODE_INHERIT, 0);
    owner->mAngle3D.y = owner->mAngle.y;
    mProc = &talk_c::execTurn;
}

// .text 0x3398
void dAcNpcSpCafe_c::talk_c::execTurn() {
    dAcNpcSpCafe_c *owner = mpOwner;
    s16 angle = owner->mAngle.y.mAngle;
    s16 rest = dANpcSpCafe::l_param.mCalibrate.mTurnFrames[mMode] -
               owner->getModel()->mAnm.getFrame();
    if (rest <= 0) {
        angle = mTurnAngle;
    } else {
        sLib::addCalcAngle(&angle, mTurnAngle, rest, 0x7FFF);
    }
    owner->mAngle.y = angle;
    owner->mAngle3D.y = owner->mAngle.y;
    if (mMode == MODE_TAKE_CUP) {
        owner->mCupMtx = owner->mSaucerMtx = owner->mNodeMtx0B;
    } else if (mMode == MODE_CLEAR) {
        if (mMilk) {
            owner->mCup2Mtx = owner->mNodeMtx0B;
        } else {
            owner->mCupMtx = owner->mNodeMtx0B;
        }
        owner->mSaucerMtx = owner->mNodeMtx0B;
    } else if (mMode == MODE_POUR || mMode == MODE_POUR_MILK) {
        owner->mCupMtx = owner->mSaucerMtx = owner->mNodeMtx0B;
        mVec3_c *pos = owner->mCupMdl.getNodePos("cup2_mdl");
        mEffect.create("af_obj_coffee", pos, NULL, NULL, 0);
    } else if (mMode == MODE_ADD_MILK) {
        owner->mCup2Mtx = owner->mSaucerMtx = owner->mNodeMtx0B;
        mEffect.create("af_obj_coffee", owner->mCup2Mdl.getNodePos("cup2_mdl"),
                       NULL, NULL, 0);
    }
    if (rest <= 0) {
        if (mMode == MODE_TAKE_CUP) {
            initPour();
        } else if (mMode == MODE_POUR) {
            initServe();
        } else if (mMode == MODE_POUR_MILK) {
            initAddMilk();
        } else if (mMode == MODE_ADD_MILK) {
            initServeMilk();
        } else if (mMode == MODE_CLEAR) {
            initPutAway();
        } else if (mMode == MODE_PUT_AWAY) {
            initPolish();
        } else {
            initTakeCup();
        }
    }
}

// .text 0x38E0
void dAcNpcSpCafe_c::talk_c::initTakeCup() {
    mMode = MODE_TAKE_CUP;
    mpOwner->mAction.requestAnm(
        PRIORITY_DEMO, HMN_ANM_PGE_CLS1, 0.0f,
        dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_TAKE_CUP],
        m3d::PLAYMODE_INHERIT, 0);
    mProc = &talk_c::execTakeCup;
}

// .text 0x395C
void dAcNpcSpCafe_c::talk_c::execTakeCup() {
    dAcNpcSpCafe_c *owner = mpOwner;
    f32 frame = owner->getModel()->mAnm.getFrame();
    if (owner->getModel()->mAnm.checkFrame(18.0f)) {
        owner->mAudioObj.startSound(SE_cafe_CupToridasu_master);
        owner->mPolishCupMdl.setAnmChr("obj_cafe1_1", m3d::FORWARD_LOOP, 0.0f);
        owner->mClothMdl.setAnmChr("obj_cafe2_1", m3d::FORWARD_LOOP, 0.0f);
        owner->mDrawPolish = 0;
        owner->mPolishing = 0;
    } else if (52.5f < frame) {
        if (owner->getModel()->mAnm.checkFrame(54.0f)) {
            owner->mCupMdl.setAnmChr("obj_cafe3_0", m3d::FORWARD_ONCE, 0.0f);
            owner->mSaucerMdl.setAnmChr("obj_cafe4_0", m3d::FORWARD_ONCE, 0.0f);
            owner->mDrawCup = 1;
        }
        owner->mCupMtx = owner->mSaucerMtx = owner->mNodeMtx0B;
    }
    if (owner->mAction.mActionId == ACTION_ANM && owner->mAction.mCanChange) {
        mProc = &talk_c::execTakeCupEnd;
    }
}

// .text 0x3BB0
void dAcNpcSpCafe_c::talk_c::execTakeCupEnd() {
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mCupMtx = owner->mSaucerMtx = owner->mNodeMtx0B;
    if (owner->mAction.mActionId == ACTION_ANM && owner->mAction.mCanChange) {
        startTurn();
    }
}

// .text 0x3C94
void dAcNpcSpCafe_c::talk_c::initPour() {
    if (mMilk) {
        mMode = MODE_POUR_MILK;
    } else {
        mMode = MODE_POUR;
    }
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_PGE_POUR1, 0.0f,
                              dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_POUR],
                              m3d::PLAYMODE_INHERIT, 0);
    owner->mCupMdl.setAnmChr("obj_cafe3_3", m3d::FORWARD_ONCE, 0.0f);
    owner->mSaucerMdl.setAnmChr("obj_cafe4_3", m3d::FORWARD_ONCE, 0.0f);
    owner->mMakerMdl.setAnmChr("obj_cafe5_0", m3d::FORWARD_ONCE, 0.0f);
    owner->mMakerMdl.setRateChr(1.0f);
    mProc = &talk_c::execPour;
}

// .text 0x3D90
void dAcNpcSpCafe_c::talk_c::execPour() {
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mCupMtx = owner->mSaucerMtx = owner->mNodeMtx0B;
    owner->getModel()->mAnm.getFrame();
    if (owner->getModel()->mAnm.checkFrame(35.0f)) {
        owner->mAudioObj.startSound(SE_cafe_CafeMakerPickUp_master);
        owner->mMakerMtx = owner->mNodeMtx0E;
    } else if (owner->getModel()->mAnm.checkFrame(221.0f)) {
        owner->mAudioObj.startSound(SE_cafe_CafeMakerOku_master);
        owner->mMakerMtx.trans(owner->mBasePos);
        owner->mMakerMtx.YrotM(-0xAAB);
    } else if (35.0f < owner->getModel()->mAnm.getFrame() &&
               owner->getModel()->mAnm.getFrame() < 221.0f) {
        if (owner->getModel()->mAnm.checkFrame(101.0f)) {
            owner->mAudioObj.startSound(SE_cafe_Sosogu_master);
        }
        owner->mMakerMtx = owner->mNodeMtx0E;
    }
    if (owner->getModel()->mAnm.getFrame() > 110.0f) {
        mEffect.create("af_obj_coffee", owner->mCupMdl.getNodePos("cup2_mdl"),
                       NULL, NULL, 0);
    }
    if (owner->mAction.mActionId == ACTION_ANM && owner->mAction.mCanChange) {
        startTurn();
    }
}

// .text 0x4128
void dAcNpcSpCafe_c::talk_c::initAddMilk() {
    mMode = MODE_ADD_MILK;
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mCupMtx = owner->mSaucerMtx = owner->mNodeMtx0B;
    owner->mAction.requestAnm(
        PRIORITY_DEMO, HMN_ANM_PGE_MILK1, 0.0f,
        dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_ADD_MILK],
        m3d::PLAYMODE_INHERIT, 0);
    owner->mCupMdl.setAnmChr("obj_cafe3_10", m3d::FORWARD_ONCE, 0.0f);
    owner->mCup2Mdl.setAnmChr("obj_cafe3_10", m3d::FORWARD_ONCE, 0.0f);
    owner->mSaucerMdl.setAnmChr("obj_cafe4_7", m3d::FORWARD_ONCE, 0.0f);
    mProc = &talk_c::execAddMilk;
}

// .text 0x42BC
void dAcNpcSpCafe_c::talk_c::execAddMilk() {
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mSaucerMtx = owner->mNodeMtx0B;
    mEffect.create("af_obj_coffee", owner->mCup2Mdl.getNodePos("cup2_mdl"),
                   NULL, NULL, 0);
    f32 frame = owner->getModel()->mAnm.getFrame();
    if (frame < 164.0f) {
        owner->mCupMtx = owner->mSaucerMtx = owner->mCup2Mtx =
            owner->mNodeMtx0B;
    } else if (owner->getModel()->mAnm.checkFrame(164.0f)) {
        owner->mCup2Mtx = owner->mNodeMtx0B;
        owner->mCupMtx.trans(mVec3_c(0.0f, 0.0f, 0.0f));
        owner->mUseCup2 = 1;
    } else if (frame > 164.0f) {
        owner->mCup2Mtx = owner->mNodeMtx0B;
    }
    if (owner->getModel()->mAnm.checkFrame(23.0f)) {
        owner->mAudioObj.startSound(SE_cafe_PigeonMilk_CupOku);
    } else if (owner->getModel()->mAnm.checkFrame(53.0f)) {
        owner->mAudioObj.startSound(SE_cafe_PigeonMilk_Sosogu);
    } else if (owner->getModel()->mAnm.checkFrame(135.0f)) {
        owner->mAudioObj.startSound(SE_cafe_PigeonMilk_Mazeru);
    } else if (owner->getModel()->mAnm.checkFrame(186.0f)) {
        owner->mAudioObj.startSound(SE_cafe_PigeonMilk_CupUp);
    }
    if (owner->mAction.mActionId == ACTION_ANM && owner->mAction.mCanChange) {
        startTurn();
    }
}

// .text 0x473C
void dAcNpcSpCafe_c::talk_c::initServe() {
    mMode = MODE_SERVE;
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mCupMtx = owner->mSaucerMtx = owner->mNodeMtx0B;
    owner->mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_PGE_HAND1, 0.0f,
                              dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_SERVE],
                              m3d::PLAYMODE_INHERIT, 0);
    owner->mCupMdl.setAnmChr("obj_cafe3_6", m3d::FORWARD_ONCE, 0.0f);
    mEffect.create("af_obj_coffee", owner->mCupMdl.getNodePos("cup2_mdl"), NULL,
                   NULL, 0);
    owner->mSaucerMdl.setAnmChr("obj_cafe4_4", m3d::FORWARD_ONCE, 0.0f);
    mProc = &talk_c::execServe;
}

// .text 0x48E4
void dAcNpcSpCafe_c::talk_c::execServe() {
    dAcNpcSpCafe_c *owner = mpOwner;
    mEffect.create("af_obj_coffee", owner->mCupMdl.getNodePos("cup2_mdl"), NULL,
                   NULL, 0);
    if (owner->getModel()->mAnm.getFrame() < 48.0f) {
        owner->mCupMtx = owner->mSaucerMtx = owner->mNodeMtx0B;
    } else if (owner->getModel()->mAnm.checkFrame(48.0f)) {
        owner->mAudioObj.startSound(SE_cafe_CupOku_master);
        mVec3_c zero(0.0f, 0.0f, 0.0f);
        owner->mCupMtx.trans(zero);
        owner->mSaucerMtx.trans(zero);
    } else if (owner->getModel()->mAnm.checkFrame(99.0f)) {
        owner->mAudioObj.startSound(SE_cafe_CupSlide_master);
    }
    if (owner->mAction.mActionId == ACTION_ANM && owner->mAction.mCanChange) {
        owner->mCupMdl.setRateChr(0.0f);
        owner->mSaucerMdl.setRateChr(0.0f);
        owner->mAnmSet.setAnm0(HMN_ANM_PGE_WAIT2);
        owner->mAction.requestWait(
            PRIORITY_IDLE,
            dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_SERVE_WAIT]);
        nextServeMsg();
    }
}

// .text 0x4B5C
void dAcNpcSpCafe_c::talk_c::initServeMilk() {
    mMode = MODE_SERVE_MILK;
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mCup2Mtx = owner->mSaucerMtx = owner->mNodeMtx0B;
    owner->mAction.requestAnm(
        PRIORITY_DEMO, HMN_ANM_PGE_HAND1, 0.0f,
        dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_SERVE_MILK],
        m3d::PLAYMODE_INHERIT, 0);
    owner->mCup2Mdl.setAnmChr("obj_cafe3_6", m3d::FORWARD_ONCE, 0.0f);
    mEffect.create("af_obj_coffee", owner->mCup2Mdl.getNodePos("cup2_mdl"),
                   NULL, NULL, 0);
    owner->mSaucerMdl.setAnmChr("obj_cafe4_4", m3d::FORWARD_ONCE, 0.0f);
    mProc = &talk_c::execServeMilk;
}

// .text 0x4D04
void dAcNpcSpCafe_c::talk_c::execServeMilk() {
    dAcNpcSpCafe_c *owner = mpOwner;
    mEffect.create("af_obj_coffee", owner->mCup2Mdl.getNodePos("cup2_mdl"),
                   NULL, NULL, 0);
    if (owner->getModel()->mAnm.getFrame() < 48.0f) {
        owner->mCup2Mtx = owner->mSaucerMtx = owner->mNodeMtx0B;
    } else if (owner->getModel()->mAnm.checkFrame(48.0f)) {
        owner->mAudioObj.startSound(SE_cafe_CupOku_master);
        mVec3_c zero(0.0f, 0.0f, 0.0f);
        owner->mCup2Mtx.trans(zero);
        owner->mSaucerMtx.trans(zero);
    } else if (owner->getModel()->mAnm.checkFrame(99.0f)) {
        owner->mAudioObj.startSound(SE_cafe_CupSlide_master);
    }
    if (owner->mAction.mActionId == ACTION_ANM && owner->mAction.mCanChange) {
        owner->mCup2Mdl.setRateChr(0.0f);
        owner->mSaucerMdl.setRateChr(0.0f);
        owner->mAnmSet.setAnm0(HMN_ANM_PGE_WAIT2);
        owner->mAction.requestWait(
            PRIORITY_IDLE,
            dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_SERVE_WAIT]);
        nextServeMsg();
    }
}

// .text 0x4F7C
void dAcNpcSpCafe_c::talk_c::execSteam() {
    dAcNpcSpCafe_c *owner = mpOwner;
    if (mMilk) {
        mEffect.create("af_obj_coffee", owner->mCup2Mdl.getNodePos("cup2_mdl"),
                       NULL, NULL, 0);
    } else {
        mEffect.create("af_obj_coffee", owner->mCupMdl.getNodePos("cup2_mdl"),
                       NULL, NULL, 0);
    }
}

// .text 0x5018
void dAcNpcSpCafe_c::talk_c::startDrinkAnm() {
    dAcNpcSpCafe_c *owner = mpOwner;
    dPlayerMgr::drinkCoffee();
    if (mMilk) {
        owner->mCup2Mdl.setAnmChr("obj_cafe3_7", m3d::FORWARD_ONCE, 0.0f);
        mEffect.create("af_obj_coffee", owner->mCup2Mdl.getNodePos("cup2_mdl"),
                       NULL, NULL, 0);
    } else {
        owner->mCupMdl.setAnmChr("obj_cafe3_7", m3d::FORWARD_ONCE, 0.0f);
        mEffect.create("af_obj_coffee", owner->mCupMdl.getNodePos("cup2_mdl"),
                       NULL, NULL, 0);
    }
    owner->mSaucerMdl.setAnmChr("obj_cafe4_5", m3d::FORWARD_ONCE, 0.0f);
    mProc = &talk_c::execDrink;
}

// .text 0x5124
void dAcNpcSpCafe_c::talk_c::execDrink() {
    f32 frame = dPlayerMgr::getCoffeeAnimationFrame();
    if (frame < 70.0f) {
        if (mMilk) {
            mEffect.create("af_obj_coffee",
                           mpOwner->mCup2Mdl.getNodePos("cup2_mdl"), NULL, NULL,
                           0);
        } else {
            mEffect.create("af_obj_coffee",
                           mpOwner->mCupMdl.getNodePos("cup2_mdl"), NULL, NULL,
                           0);
        }
    }
    if (dANpcSpCafe::l_param.mCameraFrame - 1.0f < frame &&
        frame < 1.0f + dANpcSpCafe::l_param.mCameraFrame) {
        dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_SOUND_TARGET_24;
    }
    if (dPlayerMgr::isCoffeeAnimationFinished()) {
        mMode = MODE_DRINK;
        reqMsgClose();
        mOnRequestEnd = &talk_c::stopProc;
        mMsgCode = mReactMsg;
        mpController->setNextMsgCode(mMsgCode);
    }
}

// .text 0x5274
void dAcNpcSpCafe_c::talk_c::startReturnCup() {
    dAcNpcSpCafe_c *owner = mpOwner;
    dPlayerMgr::finishCoffee();
    if (mMilk) {
        owner->mCup2Mdl.setAnmChr("obj_cafe3_8", m3d::FORWARD_ONCE, 0.0f);
    } else {
        owner->mCupMdl.setAnmChr("obj_cafe3_8", m3d::FORWARD_ONCE, 0.0f);
    }
    owner->mSaucerMdl.setAnmChr("obj_cafe4_6", m3d::FORWARD_ONCE, 0.0f);
    mProc = &talk_c::execWaitReturn;
}

// .text 0x5330
void dAcNpcSpCafe_c::talk_c::execWaitReturn() {
    if (dPlayerMgr::isWaitingForDemo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        initClear();
    }
}

// .text 0x5370
void dAcNpcSpCafe_c::talk_c::initClear() {
    mMode = MODE_CLEAR;
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_PGE_HAND1, 0.0f,
                              dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_CLEAR],
                              m3d::REVERSE_ONCE, 0);
    if (mMilk) {
        owner->mCup2Mdl.setAnmChr("obj_cafe3_9", m3d::REVERSE_ONCE, 0.0f);
    } else {
        owner->mCupMdl.setAnmChr("obj_cafe3_9", m3d::REVERSE_ONCE, 0.0f);
    }
    owner->mSaucerMdl.setAnmChr("obj_cafe4_4", m3d::REVERSE_ONCE, 0.0f);
    mProc = &talk_c::execClear;
}

// .text 0x5458
void dAcNpcSpCafe_c::talk_c::execClear() {
    f32 takeFrame = 48.0f;
    dAcNpcSpCafe_c *owner = mpOwner;
    if (owner->getModel()->mAnm.checkFrame(131.0f)) {
        owner->mAudioObj.startSound(SE_cafe_CupSlideReturn_master);
    } else if (owner->getModel()->mAnm.checkFrame(1.0f + takeFrame)) {
        owner->mAudioObj.startSound(SE_cafe_CupToru_master);
        if (mMilk) {
            owner->mCup2Mtx = owner->mNodeMtx0B;
        } else {
            owner->mCupMtx = owner->mNodeMtx0B;
        }
        owner->mSaucerMtx = owner->mNodeMtx0B;
    } else if (owner->getModel()->mAnm.getFrame() < 1.0f + takeFrame) {
        if (mMilk) {
            owner->mCup2Mtx = owner->mNodeMtx0B;
        } else {
            owner->mCupMtx = owner->mNodeMtx0B;
        }
        owner->mSaucerMtx = owner->mNodeMtx0B;
    }
    if (owner->mAction.mActionId == ACTION_ANM && owner->mAction.mCanChange) {
        startTurn();
    }
}

// .text 0x57E8
void dAcNpcSpCafe_c::talk_c::initPutAway() {
    mMode = MODE_PUT_AWAY;
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mAction.requestAnm(
        PRIORITY_DEMO, HMN_ANM_PGE_CLS1, 0.0f,
        dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_PUT_AWAY], m3d::REVERSE_ONCE,
        0);
    owner->mPolishCupMdl.setAnmChr("obj_cafe1_1", m3d::REVERSE_ONCE, 0.0f);
    owner->mClothMdl.setAnmChr("obj_cafe2_1", m3d::REVERSE_ONCE, 0.0f);
    if (mMilk) {
        owner->mCup2Mdl.setAnmChr("obj_cafe3_0", m3d::REVERSE_ONCE, 0.0f);
    } else {
        owner->mCupMdl.setAnmChr("obj_cafe3_0", m3d::REVERSE_ONCE, 0.0f);
    }
    owner->mSaucerMdl.setAnmChr("obj_cafe4_0", m3d::REVERSE_ONCE, 0.0f);
    mProc = &talk_c::execPutAway;
}

// .text 0x58F8
void dAcNpcSpCafe_c::talk_c::execPutAway() {
    dAcNpcSpCafe_c *owner = mpOwner;
    f32 frame = owner->getModel()->mAnm.getFrame();
    f32 putFrame = 50.0f;
    if (owner->getModel()->mAnm.checkFrame(putFrame)) {
        owner->mAudioObj.startSound(SE_cafe_Cupkatazukeru_master);
        owner->mDrawPolish = 1;
        owner->mDrawCup = 0;
    } else if (frame > putFrame) {
        if (mMilk) {
            owner->mCup2Mtx = owner->mNodeMtx0B;
        } else {
            owner->mCupMtx = owner->mNodeMtx0B;
        }
        owner->mSaucerMtx = owner->mNodeMtx0B;
    } else if (putFrame > frame) {
        owner->mPolishCupMtx = owner->mNodeMtx0B;
        owner->mClothMtx = owner->mNodeMtx0E;
        mpOwner->mPolishing = 1;
    }
    if (owner->mAction.mActionId == ACTION_ANM && owner->mAction.mCanChange) {
        owner->mUseCup2 = 0;
        startTurn();
    }
}

// .text 0x5C00
void dAcNpcSpCafe_c::talk_c::initPolish() {
    dAcNpcSpCafe_c *owner = mpOwner;
    owner->mAnmSet.setAnm0(HMN_ANM_PGE_WAIT1);
    owner->mAction.requestWait(
        PRIORITY_DEMO, dANpcSpCafe::l_anmMorph[dANpcSpCafe::MORPH_POLISH]);
    owner->mPolishCupMdl.setAnmChr("obj_cafe1_0", m3d::REVERSE_LOOP, 0.0f);
    owner->mClothMdl.setAnmChr("obj_cafe2_0", m3d::REVERSE_LOOP, 0.0f);
    mProc = &talk_c::finishServe;
}

// .text 0x5CB4
void dAcNpcSpCafe_c::talk_c::finishServe() {
    reqMsgClose();
    mOnRequestEnd = &talk_c::stopProc;
    mMsgCode = mThanksMsg;
    mpController->setNextMsgCode(mMsgCode);
    mpOwner->clearBusy();
}

// .text 0x5D18
BOOL dAcNpcSpCafe_c::talk_c::isKKOnStage() {
    dAcNpc_c *kk = (dAcNpc_c *)fManager_c::searchBaseByProfName(
        fProf::A_NPC_SP_TOTAKEKE, NULL);
    if (kk != NULL) {
        return kk->mIsActive;
    }
    return FALSE;
}

#ifdef MUST_MATCH
// Matching-only emission order, following d_a_str's existing convention.
// There is no evidence for this helper in the original source. MWCC drops
// the unused function; its references order the retained weak inlines.
static void orderCafeWeakInlines(m3d::anmChr_c *anm, dDebugPage_c *page) {
    anm->m3d::anmChr_c::getType();
    page->dDebugPage_c::isActive();
}
#endif
