#include <game/game/d_a_npc_sp_perio.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_string.hpp>
#include <game/cLib/c_math.hpp>

// Pete the mail carrier. Functions are in target .text order.

namespace dANpcSpPerio {
static inline dPrivateData_c *getPlayer() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer();
}
static inline dPrivateData_c *getPlayerRaw() {
    return dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
}
// The villager throwing the current player's birthday party (-1: none).
static inline int getBirthdayHostIdx() {
    dSaveData_c *save = dSaveData_c::getRaw();
    return save->mAnimals.mTown.getBirthdayHostIdx(
        dPlayerMgr::getCurrentPlayerSlot(), TRUE, TRUE, TRUE);
}

// .text 0x5C
BASE_PROFILE(A_NPC_SP_PERIO, dAcNpcSpPerio_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpPerio

// .text 0x1D8
void dAcNpcSpPerio_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeState(STATE_LOOK);
        }
        break;
    case DEMO_CMD_SPEAK:
        mPerioTalk.mKind = talk_c::KIND_DELIVER;
        mPerioTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_DELIVER_WALK);
        break;
    case DEMO_CMD_TALK:
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mPerioTalk.mKind = talk_c::KIND_TALK;
            mPerioTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpPerio {
static const dAcNpcSpPerio_c::State_s l_states[] = {
    {&dAcNpcSpPerio_c::initDeliver, &dAcNpcSpPerio_c::executeDeliver},
    {&dAcNpcSpPerio_c::initIdle, &dAcNpcSpPerio_c::executeIdle},
    {&dAcNpcSpPerio_c::initLook, &dAcNpcSpPerio_c::executeLook},
    {&dAcNpcSpPerio_c::initDeliverWalk, &dAcNpcSpPerio_c::executeDeliverWalk},
    {&dAcNpcSpPerio_c::initTalk, &dAcNpcSpPerio_c::executeTalk},
    {&dAcNpcSpPerio_c::initRemote, &dAcNpcSpPerio_c::executeRemote},
    {&dAcNpcSpPerio_c::initRemoteTalk, &dAcNpcSpPerio_c::executeRemoteTalk},
};
} // namespace dANpcSpPerio

// .text 0x328
void dAcNpcSpPerio_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpPerio::l_states[state].mInit)();
}

// .text 0x360
int dAcNpcSpPerio_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mPerioTalk);
    return SUCCEEDED;
}

// .text 0x3AC
int dAcNpcSpPerio_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    if (isNetActive() || dPlayerMgr::getCurrentPlayerSlot() >= PLAYER_NUM) {
        return FAILED;
    }
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mDelivering = 0;
    mFieldMuted = 0;
    if (isLocalOwner()) {
        if (dScene::getPrevScene() == dScene::SCENE_DM_PL_SEL &&
            !dNpcMng_c::_8074EACF &&
            dANpcSpPerio::getPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_DIST_MAIL_PENDING) &&
            dANpcSpPerio::getBirthdayHostIdx() < 0) {
            dANpcSpPerio::getPlayer()->clearFlag0(
                PRIVATE_FLAG0_DIST_MAIL_PENDING);
            mDelivering = 1;
            mFieldMuted = 1;
            setDemoFlag8();
            changeState(STATE_DELIVER);
            dBgm::l_mgr.mStgField.mute2D();
            fgMngProc_blockDayChange();
        } else {
            dTime_c now = *dTime_c::getCurrent();
            if ((now.hour != ROUND_HOUR_MORNING &&
                 now.hour != ROUND_HOUR_EVENING) ||
                now.min >= ROUND_MINUTES) {
                mIsActive = FALSE;
            }
            changeState(STATE_IDLE);
        }
    } else {
        changeState(STATE_REMOTE);
    }
    return SUCCEEDED;
}

// .text 0x578
int dAcNpcSpPerio_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x5A4
int dAcNpcSpPerio_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpPerio::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x5EC
void dAcNpcSpPerio_c::initDeliver() {
}

// .text 0x5F0
void dAcNpcSpPerio_c::executeDeliver() {
    dDemo::requestSpeak(this, NULL);
}

// .text 0x5F8
void dAcNpcSpPerio_c::initIdle() {
    initWander();
}

// .text 0x5FC
void dAcNpcSpPerio_c::executeIdle() {
    executeWander(NULL);
}

// .text 0x610
void dAcNpcSpPerio_c::initLook() {
    if (mDelivering) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep = LOOK_STEP_DONE;
    } else {
        mAng angle = getAngleYToPlayer(_1B70);
        mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                            cNpcMorphFrames);
        mStep = LOOK_STEP_TURN;
    }
}

// .text 0x6A0
void dAcNpcSpPerio_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep++;
    }
}

namespace dANpcSpPerio {
static const dAcNpcSpPerio_c::Proc l_lookSteps[] = {&dAcNpcSpPerio_c::stepLook};
} // namespace dANpcSpPerio

// .text 0x700
void dAcNpcSpPerio_c::executeLook() {
    if (mStep < ARRAY_SIZE(dANpcSpPerio::l_lookSteps)) {
        (this->*dANpcSpPerio::l_lookSteps[mStep])();
    }
}

// .text 0x740: the player walks up to him (in his z).
void dAcNpcSpPerio_c::initDeliverWalk() {
    mVec3_c pos;
    pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    pos.z = mPos.z;
    dPlayerMgr::moveTo(&pos, dPlayerMgr::PLAYER_INDEX_CURRENT,
                       dPlayerMgr::sDefaultMoveSpeed);
}

// .text 0x7A4
void dAcNpcSpPerio_c::executeDeliverWalk() {
    if (!dPlayerMgr::isMovingTo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        clearDemoFlag8();
        changeState(STATE_TALK);
    }
}

// .text 0x7F0
void dAcNpcSpPerio_c::initTalk() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep = TALK_STEP_TALK;
}

// .text 0x888
void dAcNpcSpPerio_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        if (mFieldMuted) {
            dBgm::l_mgr.mStgField.fadeUnmute2D();
        }
        mFieldMuted = 0;
        mStep++;
    }
}

namespace dANpcSpPerio {
static const dAcNpcSpPerio_c::Proc l_talkSteps[] = {&dAcNpcSpPerio_c::stepTalk};
} // namespace dANpcSpPerio

// .text 0x8F8
void dAcNpcSpPerio_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpPerio::l_talkSteps)) {
        (this->*dANpcSpPerio::l_talkSteps[mStep])();
    }
}

// .text 0x938
void dAcNpcSpPerio_c::initRemote() {
    resetLookTimer();
}

// .text 0x940
void dAcNpcSpPerio_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mPerioTalk.mKind = talk_c::KIND_TALK;
            mPerioTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    executeRemoteWander(NULL);
}

// .text 0x9E8
void dAcNpcSpPerio_c::initRemoteTalk() {
}

// .text 0x9EC
void dAcNpcSpPerio_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0xA40
dAcNpcSpPerio_c::talk_c::talk_c(dAcNpcSpPerio_c *npc) : mpOwner(npc) {
    mKind = KIND_TALK;
    init();
}

// .text 0xAA8
dAcNpcSpPerio_c::talk_c::~talk_c() {
}

// .text 0xB00
void dAcNpcSpPerio_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
}

// .text 0xB68
void dAcNpcSpPerio_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (mpOwner->mDelivering) {
        if (mKind == KIND_DELIVER) {
            if (dPostOffice::getDistMail()) {
                mMsgNo = MSG_LETTER;
            } else {
                mMsgNo = MSG_LETTER_FULL;
            }
        } else {
            mMsgNo = cM::rndInt(MSG_BUSY_NUM) + MSG_BUSY;
        }
    } else if (!dANpcSpPerio::getPlayerRaw()->isFlag0(PRIVATE_FLAG0_PETE_MET)) {
        mMsgNo = MSG_FIRST_MEETING;
    } else if (!dANpcSpPerio::getPlayer()->isFlag1(PRIVATE_FLAG1_PETE_TALKED)) {
        mMsgNo = cM::rndInt(MSG_CHAT_NUM) + MSG_CHAT;
    } else {
        mMsgNo = cM::rndInt(MSG_BUSY_NUM) + MSG_BUSY;
    }
}

// .text 0xC5C
void dAcNpcSpPerio_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_perio";
    info->mCode = mMsgNo;
}

// .text 0xC74: the letter's sender is word 0.
void dAcNpcSpPerio_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_LETTER_FULL:
    case MSG_LETTER: {
        dString::Word_c sender;
        dPostOffice::getDistMailSender(&sender);
        mpController->setWord(0, &sender);
        break;
    }
    }
}

// .text 0xCE0
void dAcNpcSpPerio_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_LETTER:
        mOnMessageEnd = &talk_c::giveLetter;
        mOnRequestEnd = &talk_c::onLetterGiven;
        mpController->setNextMsgCode(MSG_LETTER_DONE);
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0xD98
void dAcNpcSpPerio_c::talk_c::onAnswer(int arg) {
}

// .text 0xD9C
void dAcNpcSpPerio_c::talk_c::preExecute() {
}

// .text 0xDA0
void dAcNpcSpPerio_c::talk_c::onTalkEnd() {
    if (!mpOwner->mDelivering) {
        dANpcSpPerio::getPlayer()->setFlag0(PRIVATE_FLAG0_PETE_MET);
        dANpcSpPerio::getPlayer()->setFlag1(PRIVATE_FLAG1_PETE_TALKED);
    } else {
        fgMngProc_unblockDayChange();
    }
    mKind = KIND_TALK;
}

// .text 0xE00
void dAcNpcSpPerio_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0xE64
void dAcNpcSpPerio_c::talk_c::giveLetter() {
    dItem::Item letter(dItem::ITEM_IDX_LETTER);
    requestItemAct(&letter, 0, 0);
}

// .text 0xEAC
void dAcNpcSpPerio_c::talk_c::onLetterGiven() {
}

// .text 0xEB0
int dAcNpcSpPerio_c::getEarType() {
    return NPC_EAR_TYPE_PETE;
}
