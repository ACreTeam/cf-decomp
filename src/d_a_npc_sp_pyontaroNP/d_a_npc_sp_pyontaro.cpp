#include <game/game/d_a_npc_sp_pyontaro.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/snd/snd_id.hpp>

// Zipper T. Bunny. Functions are in target .text order.

namespace dANpcSpPyontaro {
// .text 0x5C: an opened egg with a prize ticket.
static BOOL isFoil(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_EGG_BINGO_AFTER;
}

// .text 0x88: an unopened egg.
static BOOL isEgg(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_EGG_FAKE_BEFORE ||
           item->getKind() == dItem::KIND_EGG_BINGO_BEFORE;
}

// .text 0xE0: the first pocket with a foil, -1 if none.
static int getFoilPocket() {
    u16 mask;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(isFoil, &mask);
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (mask & (1 << i)) {
            return i;
        }
    }
    return -1;
}

// .text 0x190: a shovel of any grade.
static BOOL isShovel(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_SCOOP ||
           item->getKind() == dItem::KIND_SILVER_SCOOP ||
           item->getKind() == dItem::KIND_GOLD_SCOOP;
}

// .text 0x1F8
BASE_PROFILE(A_NPC_SP_PYONTARO, dAcNpcSpPyontaro_c);

// Tuning values (.bss 0x8, built by __sinit).
struct param_s {
    param_s() {
        _00 = 0;
        mDanceDist = 200.0f;
        mStopDist = 280.0f;
        mBackAngle = 0x5000;
        mRestMorph = 30.0f;
        mAnmMorph = 50.0f;
    }

    /* 0x00 */ int _00;
    /* 0x04 */ int
        mBackAngle; // the player is behind him from here (isPlayerBehind)
    /* 0x08 */ f32
        mDanceDist; // he starts dancing when a player comes this close
    /* 0x0C */ f32
        mStopDist; // and stops when they are farther; the talk counts reset
    /* 0x10 */ f32 mRestMorph; // requestEmotion morph of the rest
    /* 0x14 */ f32 mAnmMorph; // requestAnm morph of the dance / rest animations
}; // size 0x18
static param_s l_param;
} // namespace dANpcSpPyontaro

// .text 0x368
void dAcNpcSpPyontaro_c::onDemoCmd(int cmd, u8 member) {
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
            mPyontaroTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpPyontaro {
static const dAcNpcSpPyontaro_c::State_s l_states[] = {
    {&dAcNpcSpPyontaro_c::initIdle, &dAcNpcSpPyontaro_c::executeIdle},
    {&dAcNpcSpPyontaro_c::initLook, &dAcNpcSpPyontaro_c::executeLook},
    {&dAcNpcSpPyontaro_c::initTalk, &dAcNpcSpPyontaro_c::executeTalk},
    {&dAcNpcSpPyontaro_c::initRemote, &dAcNpcSpPyontaro_c::executeRemote},
    {&dAcNpcSpPyontaro_c::initRemoteTalk,
     &dAcNpcSpPyontaro_c::executeRemoteTalk},
};
} // namespace dANpcSpPyontaro

// .text 0x484
void dAcNpcSpPyontaro_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpPyontaro::l_states[state].mInit)();
}

// .text 0x4BC
int dAcNpcSpPyontaro_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mPyontaroTalk);
    return 1;
}

// .text 0x508
int dAcNpcSpPyontaro_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mStep[STATE_IDLE] = 0;
    mStep[STATE_LOOK] = 0;
    mStep[STATE_TALK] = 0;
    mStep[STATE_REMOTE] = 0;
    mStep[STATE_REMOTE_TALK] = 0;
    mCreating = TRUE;
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        mDaub.mDancing = FALSE;
        mDaub.mResting = TRUE;
        setDaubData(&mDaub, sizeof(mDaub));
        changeState(STATE_IDLE);
    } else {
        getDaubData(&mDaub, sizeof(mDaub));
        changeState(STATE_REMOTE);
    }
    mLookAt.setPlayer(PRIORITY_DEMO, true, mAng(0), mAng(0x180),
                      dANpcSpPyontaro::l_param.mDanceDist);
    mLookAt.mPitch.mMax = 0;
    mLookAt.mYaw.mMax = 0x2BFF;
    mLookAt.mFov = labs(0x4000);
    mAnmSet.setAnm1(HMN_ANM_PYN_DANCE);
    mAnmSet.setAnm2(HMN_ANM_PYN_DANCE);
    moveFtrToPoliceBox();
    mCreating = FALSE;
    return 1;
}

// .text 0x648
int dAcNpcSpPyontaro_c::doDelete() {
    dNpcInfo::pyontaro_c::get()->resetTalks();
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x68C
int dAcNpcSpPyontaro_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    isLocalOwner();
    if (mState < STATE_COUNT) {
        (this->*dANpcSpPyontaro::l_states[mState].mExecute)();
    }
    if (mDaub.mDancing) {
        mAudioObj.holdSound(SE_PyontarouDance);
    }
    dBgm::l_mgr.mVolCtrl.mFocus.set(1, mPos);
    return 1;
}

// .text 0x744
void dAcNpcSpPyontaro_c::initIdle() {
    mAnmSet.setAnm0(HMN_ANM_PYN_DANCE);
    mAction.requestTurn(PRIORITY_IDLE, mAng(0), l_turnSpeed, 0,
                        cNpcMorphFrames);
}

// .text 0x7A4
void dAcNpcSpPyontaro_c::executeIdle() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        if (mDaub.mDancing) {
            mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_PYN_DANCE, 0.0f,
                               dANpcSpPyontaro::l_param.mAnmMorph,
                               m3d::PLAYMODE_INHERIT, 0);
        } else {
            mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_MR_TAME2, 0.0f,
                               dANpcSpPyontaro::l_param.mAnmMorph,
                               m3d::PLAYMODE_INHERIT, 0);
        }
    } else if (isDancing()) {
        if (!isPlayerNear(dPlayerMgr::PLAYER_INDEX_CURRENT, FALSE, NULL,
                          dANpcSpPyontaro::l_param.mStopDist)) {
            mAction.requestEmotion(PRIORITY_IDLE, action_c::EMOTION_SIGH,
                                   dANpcSpPyontaro::l_param.mRestMorph);
            mDaub.mDancing = FALSE;
            mDaub.mResting = TRUE;
            setDaubData(&mDaub, sizeof(mDaub));
            mLookAt.setPlayer(PRIORITY_DEMO, true, mAng(0), mAng(0x180),
                              dANpcSpPyontaro::l_param.mDanceDist);
        }
    } else if (isPlayerNear(dPlayerMgr::PLAYER_INDEX_CURRENT, FALSE, NULL,
                            dANpcSpPyontaro::l_param.mDanceDist)) {
        mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_PYN_DANCE, 0.0f,
                           dANpcSpPyontaro::l_param.mAnmMorph,
                           m3d::PLAYMODE_INHERIT, 0);
        mDaub.mDancing = TRUE;
        mDaub.mResting = FALSE;
        setDaubData(&mDaub, sizeof(mDaub));
        mLookAt.setPlayer(PRIORITY_DEMO, true, mAng(0), mAng(0x180),
                          dANpcSpPyontaro::l_param.mStopDist);
    }
    if (!isPlayerNear(dNet::getMyNo(), FALSE, NULL,
                      dANpcSpPyontaro::l_param.mStopDist)) {
        dNpcInfo::pyontaro_c::get()->resetTalks();
    }
}

// .text 0x9C0
void dAcNpcSpPyontaro_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep[STATE_LOOK] = 0;
}

// .text 0xA24
void dAcNpcSpPyontaro_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep[STATE_LOOK]++;
    }
}

namespace dANpcSpPyontaro {
static const dAcNpcSpPyontaro_c::Proc l_lookSteps[] = {
    &dAcNpcSpPyontaro_c::stepLook};
} // namespace dANpcSpPyontaro

// .text 0xA84
void dAcNpcSpPyontaro_c::executeLook() {
    if (mStep[STATE_LOOK] < ARRAY_SIZE(dANpcSpPyontaro::l_lookSteps)) {
        (this->*dANpcSpPyontaro::l_lookSteps[mStep[STATE_LOOK]])();
    }
}

// .text 0xAC4
void dAcNpcSpPyontaro_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep[STATE_TALK] = 0;
    mDaub.mDancing = TRUE;
    mDaub.mResting = FALSE;
    setDaubData(&mDaub, sizeof(mDaub));
}

// .text 0xB50
void dAcNpcSpPyontaro_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        mStep[STATE_TALK]++;
    }
}

namespace dANpcSpPyontaro {
static const dAcNpcSpPyontaro_c::Proc l_talkSteps[] = {
    &dAcNpcSpPyontaro_c::stepTalk};
} // namespace dANpcSpPyontaro

// .text 0xB9C
void dAcNpcSpPyontaro_c::executeTalk() {
    if (mStep[STATE_TALK] < ARRAY_SIZE(dANpcSpPyontaro::l_talkSteps)) {
        (this->*dANpcSpPyontaro::l_talkSteps[mStep[STATE_TALK]])();
    }
}

// .text 0xBDC
void dAcNpcSpPyontaro_c::initRemote() {
    if (isDancing()) {
        initRemoteDance();
    } else if (isResting()) {
        initRemoteRest();
    } else {
        initRemoteIdle();
    }
}

// .text 0xC40
void dAcNpcSpPyontaro_c::initRemoteIdle() {
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mStep[STATE_REMOTE] = REMOTE_STEP_IDLE;
}

// .text 0xC90
void dAcNpcSpPyontaro_c::stepRemoteIdle() {
    if (isResting()) {
        initRemoteRest();
    } else if (isDancing()) {
        initRemoteDance();
    }
}

// .text 0xCE8
void dAcNpcSpPyontaro_c::initRemoteRest() {
    mAcceptMoveReq = FALSE;
    mLookAt.setPlayer(PRIORITY_DEMO, true, mAng(0), mAng(0x180),
                      dANpcSpPyontaro::l_param.mDanceDist);
    if (mCreating) {
        mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_MR_TAME2, 0.0f,
                           dANpcSpPyontaro::l_param.mAnmMorph,
                           m3d::PLAYMODE_INHERIT, 0);
    } else {
        mAction.requestEmotion(PRIORITY_IDLE, action_c::EMOTION_SIGH,
                               dANpcSpPyontaro::l_param.mRestMorph);
    }
    mStep[STATE_REMOTE] = REMOTE_STEP_REST;
}

// .text 0xDA0
void dAcNpcSpPyontaro_c::stepRemoteRest() {
    if (isDancing()) {
        initRemoteDance();
    } else if (!isResting()) {
        initRemoteIdle();
    }
}

// .text 0xDF8
void dAcNpcSpPyontaro_c::initRemoteDance() {
    mAcceptMoveReq = TRUE;
    mLookAt.setPlayer(PRIORITY_DEMO, true, mAng(0), mAng(0x180),
                      dANpcSpPyontaro::l_param.mStopDist);
    mAnmSet.setAnm0(HMN_ANM_PYN_DANCE);
    mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_PYN_DANCE, 0.0f,
                       dANpcSpPyontaro::l_param.mAnmMorph,
                       m3d::PLAYMODE_INHERIT, 1);
    mStep[STATE_REMOTE] = REMOTE_STEP_DANCE;
}

// .text 0xE9C
void dAcNpcSpPyontaro_c::stepRemoteDance() {
    if (isResting()) {
        initRemoteRest();
    } else if (!isDancing()) {
        initRemoteIdle();
    }
}

namespace dANpcSpPyontaro {
static const dAcNpcSpPyontaro_c::Proc l_remoteSteps[] = {
    &dAcNpcSpPyontaro_c::stepRemoteIdle,
    &dAcNpcSpPyontaro_c::stepRemoteRest,
    &dAcNpcSpPyontaro_c::stepRemoteDance,
};
} // namespace dANpcSpPyontaro

// .text 0xEF4
void dAcNpcSpPyontaro_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
            fn_80013474();
            mPyontaroTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    if (!isPlayerNear(dNet::getMyNo(), FALSE, NULL,
                      dANpcSpPyontaro::l_param.mStopDist)) {
        dNpcInfo::pyontaro_c::get()->resetTalks();
    }
    if (mStep[STATE_REMOTE] < ARRAY_SIZE(dANpcSpPyontaro::l_remoteSteps)) {
        (this->*dANpcSpPyontaro::l_remoteSteps[mStep[STATE_REMOTE]])();
    }
}

// .text 0xFF0
void dAcNpcSpPyontaro_c::initRemoteTalk() {
    mAnmSet.setAnm0(HMN_ANM_PYN_DANCE);
}

// .text 0xFFC
void dAcNpcSpPyontaro_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0x1050: the current player stands behind him.
BOOL dAcNpcSpPyontaro_c::isPlayerBehind() const {
    return getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT).labs() >=
           dANpcSpPyontaro::l_param.mBackAngle;
}

// .text 0x109C
BOOL dAcNpcSpPyontaro_c::isDancing() const {
    return mDaub.mDancing;
}

// .text 0x10A8
BOOL dAcNpcSpPyontaro_c::isResting() const {
    return mDaub.mResting;
}

// .text 0x10B4
dAcNpcSpPyontaro_c::talk_c::talk_c(dAcNpcSpPyontaro_c *npc) : mpOwner(npc) {
    init();
}

// .text 0x1120
dAcNpcSpPyontaro_c::talk_c::~talk_c() {
}

// .text 0x1178
void dAcNpcSpPyontaro_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mItem = dItem::Item();
}

namespace dANpcSpPyontaro {
// .text 0x11EC: an egg is still buried somewhere in the field.
static BOOL isEggBuried() {
    dItem::Item egg(dItem::ITEM_IDX_BUNNY_EGG_00);
    dItem::Item fakeEgg(dItem::ITEM_IDX_BUNNY_EGG_FAKE);
    for (int x = 0; x < getCurrentField()->mUnitW; x++) {
        for (int z = 0; z < getCurrentField()->mUnitH; z++) {
            dItem::Item *item = getCurrentField()->getItem(x, z, 0);
            if (item != NULL && (item->isSame(egg) || item->isSame(fakeEgg)) &&
                getCurrentField()->isBuried(x, z)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}
} // namespace dANpcSpPyontaro

// .text 0x12E8
void dAcNpcSpPyontaro_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (mpOwner->isPlayerBehind()) {
        dNpcInfo::pyontaro_c::get()->incBackTalks();
        if (dNpcInfo::pyontaro_c::get()->getBackTalks() <= 1) {
            mMsgNo = MSG_BACK;
        } else {
            mMsgNo = MSG_BACK_AGAIN;
        }
    } else if (!dNpcInfo::pyontaro_c::get()->isIntroduced()) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                dANpcSpPyontaro::isShovel, NULL) > 0 ||
            dANpcSpPyontaro::isShovel(
                &dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHeld)) {
            mMsgNo = MSG_GREETING;
        } else {
            if (!dNpcInfo::pyontaro_c::get()->isShovelOffered()) {
                mMsgNo = MSG_NO_SHOVEL;
            } else {
                mMsgNo = MSG_NO_SHOVEL_AGAIN;
            }
            dNpcInfo::pyontaro_c::get()->setShovelOffered();
        }
    } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                   dANpcSpPyontaro::isFoil, NULL) > 0) {
        dNpcInfo::pyontaro_c::get()->incFoilTalks();
        if (dNpcInfo::pyontaro_c::get()->mFoilTalks <= 1) {
            mMsgNo = MSG_FOIL;
        } else {
            mMsgNo = MSG_FOIL_AGAIN;
        }
    } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                   dANpcSpPyontaro::isEgg, NULL) > 0) {
        dNpcInfo::pyontaro_c::get()->incEggTalks();
        int talks = dNpcInfo::pyontaro_c::get()->mEggTalks;
        if (talks <= 1) {
            mMsgNo = MSG_EGG;
        } else if (talks <= 2) {
            mMsgNo = MSG_EGG_2;
        } else if (talks <= 3) {
            mMsgNo = MSG_EGG_3;
        } else if (talks <= 4) {
            mMsgNo = MSG_EGG_4;
        } else {
            mMsgNo = MSG_EGG_5;
        }
    } else if (dANpcSpPyontaro::isEggBuried()) {
        dNpcInfo::pyontaro_c::get()->incEggsLeftTalks();
        int talks = dNpcInfo::pyontaro_c::get()->mEggsLeftTalks;
        if (talks <= 1) {
            mMsgNo = MSG_EGGS_LEFT;
        } else if (talks <= 2) {
            mMsgNo = MSG_EGGS_LEFT_2;
        } else if (talks <= 3) {
            mMsgNo = MSG_EGGS_LEFT_3;
        } else {
            mMsgNo = MSG_EGGS_LEFT_4;
        }
    } else {
        dNpcInfo::pyontaro_c::get()->incNoEggsTalks();
        if (dNpcInfo::pyontaro_c::get()->mNoEggsTalks <= 1) {
            mMsgNo = MSG_EGGS_LEFT;
        } else {
            mMsgNo = MSG_NO_EGGS;
        }
    }
}

// .text 0x1550
void dAcNpcSpPyontaro_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_pyontarou";
    info->mCode = mMsgNo;
}

namespace dANpcSpPyontaro {
// The bell price of a shovel (MSG_OFFER_SHOVEL).
static inline int getShovelPrice() {
    return dItem::Item(dItem::ITEM_IDX_SHOVEL).getPrice();
}
// The player has the money (signed >= as a value: subfc / adde).
static inline BOOL canAfford(int price) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketMoney() >= price;
}
} // namespace dANpcSpPyontaro

// .text 0x1568
void dAcNpcSpPyontaro_c::talk_c::onMessageStart(int arg) {
    if (mItem.mId != dItem::ITEM_ID_NONE) {
        setItemName(mItem, 0);
    }
    switch (mMessageCode) {
    case MSG_GREETING:
        dNpcInfo::pyontaro_c::get()->setIntroduced();
        dMoney_c::offForceShow();
        break;
    case MSG_OFFER_SHOVEL:
        dMoney_c::onForceShow();
        setNumber(dANpcSpPyontaro::getShovelPrice(), 1, 7,
                  dScript::NUM_FORMAT_REGION);
        break;
    }
    switch (arg) {
    case START_DANCE:
        mpOwner->mAnmSet.setAnm0(HMN_ANM_PYN_DANCE);
        break;
    case START_STAND:
        mpOwner->mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
        break;
    }
}

namespace dANpcSpPyontaro {
// The prize for each foil (by its index in KIND_EGG_BINGO_AFTER).
static const int l_prizes[] = {
    dItem::ITEM_IDX_EGG_WALL,   dItem::ITEM_IDX_EGG_FLOOR,
    dItem::ITEM_IDX_EGG_BED,    dItem::ITEM_IDX_EGG_DRESSER,
    dItem::ITEM_IDX_EGG_STEREO, dItem::ITEM_IDX_EGG_TOY_SET,
    dItem::ITEM_IDX_EGG_BENCH,  dItem::ITEM_IDX_EGG_WARDROBE,
    dItem::ITEM_IDX_EGG_CHAIR,  dItem::ITEM_IDX_EGG_LAMP,
    dItem::ITEM_IDX_EGG_TABLE,  dItem::ITEM_IDX_EGG_CLOCK,
};
} // namespace dANpcSpPyontaro

// .text 0x164C
void dAcNpcSpPyontaro_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_SHOVEL_DECLINED:
    case MSG_SHOVEL_NO_MONEY:
    case MSG_SHOVEL_POCKETS_FULL:
        dMoney_c::offForceShow();
        break;
    case MSG_SHOVEL_SOLD:
        mOnMessageEnd = &talk_c::giveShovel;
        mOnRequestEnd = &talk_c::buyShovel;
        mpController->setNextMsgCode(MSG_SHOVEL_RESTART);
        break;
    }
    switch (kind) {
    case END_REQ_TAKE_FOIL: {
        mpController->setNextMsgCode(mMessageCode + 1);
        int pocket = dANpcSpPyontaro::getFoilPocket();
        mItem = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[pocket];
        mOnMessageEnd = &talk_c::takeFoil;
        mOnRequestEnd = &talk_c::removeFoil;
        break;
    }
    case END_REQ_GIVE_PRIZE:
        switch (mMessageCode) {
        case MSG_SHOVEL_SOLD:
            break;
        default:
            mpController->setNextMsgCode(mMessageCode + 1);
            mItem = dItem::Item(
                dANpcSpPyontaro::l_prizes[dItem::seeker_c::get()->findLike(
                    mItem)]);
            mOnMessageEnd = &talk_c::givePrize;
            mOnRequestEnd = &talk_c::pickUpPrize;
            break;
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x1840
void dAcNpcSpPyontaro_c::talk_c::onAnswer(int arg) {
    u8 answer = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_OFFER_SHOVEL:
        switch (answer) {
        case ANSWER_YES:
            if (!dANpcSpPyontaro::canAfford(
                    dANpcSpPyontaro::getShovelPrice())) {
                mpController->setNextMsgCode(MSG_SHOVEL_NO_MONEY);
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(
                           0) < 0) {
                mpController->setNextMsgCode(MSG_SHOVEL_POCKETS_FULL);
            } else {
                mpController->setNextMsgCode(MSG_SHOVEL_SOLD);
            }
            break;
        case ANSWER_NO:
            break;
        }
        break;
    }
}

// .text 0x1914
void dAcNpcSpPyontaro_c::talk_c::preExecute() {
}

// .text 0x1918
void dAcNpcSpPyontaro_c::talk_c::onTalkEnd() {
}

// .text 0x191C
void dAcNpcSpPyontaro_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1980
void dAcNpcSpPyontaro_c::talk_c::takeFoil() {
    requestItemActEx(6, &mItem, 0, 0, 0, 2);
}

// .text 0x199C
void dAcNpcSpPyontaro_c::talk_c::givePrize() {
    requestItemAct(&mItem, 0, 0);
}

// .text 0x19AC
void dAcNpcSpPyontaro_c::talk_c::giveShovel() {
    dItem::Item shovel(dItem::ITEM_IDX_SHOVEL);
    requestItemAct(&shovel, 0, 0);
}

// .text 0x19F4
void dAcNpcSpPyontaro_c::talk_c::removeFoil() {
    u16 mask;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(dANpcSpPyontaro::isFoil,
                                                        &mask);
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (mask & (1 << i)) {
            dItem::Item none;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&none, i, FALSE);
            break;
        }
    }
}

// .text 0x1A78
void dAcNpcSpPyontaro_c::talk_c::pickUpPrize() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem, FALSE);
}

// .text 0x1AB0
void dAcNpcSpPyontaro_c::talk_c::buyShovel() {
    dItem::Item shovel(dItem::ITEM_IDX_SHOVEL);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(shovel, FALSE);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(shovel.getPrice(), FALSE);
}
