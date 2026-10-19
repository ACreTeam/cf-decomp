#include <game/game/d_a_npc_sp_cafe_guest.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_rmobj_cafe.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_cafe_guest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/cLib/c_math.hpp>

// The special npc guest at the Roost. Functions are in target .text order.

namespace dANpcSpCafeGuest {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_CAFE_GUEST, dAcNpcSpCafeGuest_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word; its identity is unknown.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpCafeGuest

// .text 0x1CC
void dAcNpcSpCafeGuest_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mGuestTalk.start(
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

namespace dANpcSpCafeGuest {
static const dAcNpcSpCafeGuest_c::State_s l_states[] = {
    {&dAcNpcSpCafeGuest_c::initWait, &dAcNpcSpCafeGuest_c::executeWait},
    {&dAcNpcSpCafeGuest_c::initLook, &dAcNpcSpCafeGuest_c::executeLook},
    {&dAcNpcSpCafeGuest_c::initTalk, &dAcNpcSpCafeGuest_c::executeTalk},
};
} // namespace dANpcSpCafeGuest

// .text 0x274
void dAcNpcSpCafeGuest_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpCafeGuest::l_states[state].mInit)();
}

// .text 0x2AC: today's guest of this time of day.
int dAcNpcSpCafeGuest_c::preCreate() {
    switch (dSaveData_c::getTown()->mShops.mCafeGuest.getCurrentGuest()) {
    case CAFE_GUEST_PELLY:
        mParam = SP_NPC_POST_GIRL;
        break;
    case CAFE_GUEST_PHYLLIS:
        mParam = SP_NPC_POST_GIRL2;
        break;
    case CAFE_GUEST_KAPPN:
        mParam = SP_NPC_KAPPN;
        break;
    case CAFE_GUEST_RESETTI:
        mParam = SP_NPC_RESETTI_OFF;
        break;
    case CAFE_GUEST_DON:
        mParam = SP_NPC_DON_OFF;
        break;
    }
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mGuestTalk);
    mAnmSet.setAnm0(HMN_ANM_CHAIR_SITDOWN_WAIT1);
    mAnmSet.setAnm1(HMN_ANM_CHAIR_SITDOWN_WAIT1);
    mAnmSet.setAnm2(HMN_ANM_CHAIR_SITDOWN_WAIT1);
    return SUCCEEDED;
}

// .text 0x3A8
int dAcNpcSpCafeGuest_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    if (mParam == SP_NPC_CAFE_GUEST) {
        return FAILED;
    }
    if (isNetActive()) {
        return FAILED;
    }
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
            PRIVATE_FLAG0_TRAVELLING)) {
        return FAILED;
    }
    changeState(STATE_WAIT);
    mCheckGround = 0;
    mLookAt.mPitch.mMax = LOOK_PITCH_MAX;
    mLookAt.mYaw.mMax = LOOK_YAW_MAX;
    return SUCCEEDED;
}

// .text 0x450
int dAcNpcSpCafeGuest_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x47C
int dAcNpcSpCafeGuest_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpCafeGuest::l_states[mState].mExecute)();
    }
    dARmobjCafe::g_spNpcCreated = TRUE;
    return SUCCEEDED;
}

// .text 0x4D0
int dAcNpcSpCafeGuest_c::draw() {
    return dAcNpc_c::draw();
}

// .text 0x4D4
void dAcNpcSpCafeGuest_c::initWait() {
    mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x52C
void dAcNpcSpCafeGuest_c::executeWait() {
}

// .text 0x530
void dAcNpcSpCafeGuest_c::initLook() {
    mLookAt.setPlayer(PRIORITY_IDLE, false, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
}

// .text 0x558
void dAcNpcSpCafeGuest_c::executeLook() {
}

// .text 0x55C
void dAcNpcSpCafeGuest_c::initTalk() {
    mLookAt.setPlayerNo(PRIORITY_IDLE, dNet::getMyNo(), false, l_lookPitchStep,
                        l_lookYawStep, cNpcLookRange);
    mTalk.requestTalk(true, FALSE);
    mStep = TALK_STEP_TALK;
}

// .text 0x5CC
void dAcNpcSpCafeGuest_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        ++mStep;
    }
}

namespace dANpcSpCafeGuest {
static const dAcNpcSpCafeGuest_c::Proc l_talkSteps[] = {
    &dAcNpcSpCafeGuest_c::stepTalk,
};
} // namespace dANpcSpCafeGuest

// .text 0x618
void dAcNpcSpCafeGuest_c::executeTalk() {
    if (mStep < TALK_STEP_COUNT) {
        (this->*dANpcSpCafeGuest::l_talkSteps[mStep])();
    }
}

// .text 0x658
dAcNpcSpCafeGuest_c::talk_c::talk_c(dAcNpcSpCafeGuest_c *npc) : mpOwner(npc) {
    init();
}

// .text 0x6B8
dAcNpcSpCafeGuest_c::talk_c::~talk_c() {
}

// .text 0x710
void dAcNpcSpCafeGuest_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
}

// .text 0x778: a greeting by the meetings at the Roost, or chat once
// greeted today.
void dAcNpcSpCafeGuest_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    BOOL talked = FALSE;
    u8 count = 0;
    switch (mpOwner->mParam) {
    case SP_NPC_POST_GIRL:
        talked = dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
            PRIVATE_FLAG1_CAFE_PELLY_TALKED);
        count = dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeCount(
            CAFE_GUEST_PELLY);
        break;
    case SP_NPC_POST_GIRL2:
        talked = dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
            PRIVATE_FLAG1_CAFE_PHYLLIS_TALKED);
        count = dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeCount(
            CAFE_GUEST_PHYLLIS);
        break;
    case SP_NPC_KAPPN:
        talked = dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
            PRIVATE_FLAG1_CAFE_KAPPN_TALKED);
        count = dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeCount(
            CAFE_GUEST_KAPPN);
        break;
    case SP_NPC_RESETTI_OFF:
        talked = dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
            PRIVATE_FLAG1_CAFE_RESETTI_TALKED);
        count = dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeCount(
            CAFE_GUEST_RESETTI);
        break;
    case SP_NPC_DON_OFF:
        talked = dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
            PRIVATE_FLAG1_CAFE_DON_TALKED);
        count =
            dPlayerMgr::Mgr_c::getCurrentPlayer()->getCafeCount(CAFE_GUEST_DON);
        break;
    }
    if (talked) {
        mMsgNo = cM::rndInt(MSG_CHAT_NUM) + MSG_CHAT;
    } else if (count == 0) {
        mMsgNo = MSG_FIRST;
    } else if (count >= AGAIN_MIN && count <= AGAIN_MAX) {
        mMsgNo = MSG_AGAIN;
    } else {
        mMsgNo = MSG_REGULAR;
    }
}

// .text 0x914
void dAcNpcSpCafeGuest_c::talk_c::getMsgInfo(msgInfo_s *info) {
    switch (mpOwner->mParam) {
    case SP_NPC_POST_GIRL:
        info->mLabel = "npc_SP/NPC_cafe_periko";
        break;
    case SP_NPC_POST_GIRL2:
        info->mLabel = "npc_SP/NPC_cafe_perimi";
        break;
    case SP_NPC_KAPPN:
        info->mLabel = "npc_SP/NPC_cafe_kappei";
        break;
    case SP_NPC_RESETTI_OFF:
        info->mLabel = "npc_SP/NPC_cafe_reset";
        break;
    case SP_NPC_DON_OFF:
        info->mLabel = "npc_SP/NPC_cafe_racket";
        break;
    }
    info->mCode = mMsgNo;
}

// .text 0x9BC
void dAcNpcSpCafeGuest_c::talk_c::onMessageStart(int arg) {
}

// .text 0x9C0
void dAcNpcSpCafeGuest_c::talk_c::onMessageEnd(int kind) {
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0xA24
void dAcNpcSpCafeGuest_c::talk_c::onAnswer(int arg) {
}

// .text 0xA28
void dAcNpcSpCafeGuest_c::talk_c::preExecute() {
}

// .text 0xA2C: the first talk of the day counts as a meeting.
void dAcNpcSpCafeGuest_c::talk_c::onTalkEnd() {
    switch (mpOwner->mParam) {
    case SP_NPC_POST_GIRL:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_CAFE_PELLY_TALKED)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_CAFE_PELLY_TALKED);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->incCafeCount(
                CAFE_GUEST_PELLY);
        }
        break;
    case SP_NPC_POST_GIRL2:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_CAFE_PHYLLIS_TALKED)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_CAFE_PHYLLIS_TALKED);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->incCafeCount(
                CAFE_GUEST_PHYLLIS);
        }
        break;
    case SP_NPC_KAPPN:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_CAFE_KAPPN_TALKED)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_CAFE_KAPPN_TALKED);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->incCafeCount(
                CAFE_GUEST_KAPPN);
        }
        break;
    case SP_NPC_RESETTI_OFF:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_CAFE_RESETTI_TALKED)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_CAFE_RESETTI_TALKED);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->incCafeCount(
                CAFE_GUEST_RESETTI);
        }
        break;
    case SP_NPC_DON_OFF:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_CAFE_DON_TALKED)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_CAFE_DON_TALKED);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->incCafeCount(CAFE_GUEST_DON);
        }
        break;
    }
}

// .text 0xB90
void dAcNpcSpCafeGuest_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0xBF4
int dAcNpcSpCafeGuest_c::getEarType() {
    int type = NPC_EAR_TYPE_NONE;
    switch (mParam) {
    case SP_NPC_POST_GIRL:
        type = NPC_EAR_TYPE_PELLY;
        break;
    case SP_NPC_POST_GIRL2:
        type = NPC_EAR_TYPE_PHYLLIS;
        break;
    }
    return type;
}
