#include <game/game/d_a_npc_sp_sample.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_a_player.hpp>

namespace dANpcSpSample {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_SAMPLE, dAcNpcSpSample_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a word and a
// byte, both cleared.
struct unk_s {
    unk_s() : _0(0), _4(0) {
    }
    u32 _0;
    u8 _4;
};
static unk_s l_unk;
} // namespace dANpcSpSample

// .text 0x1CC
void dAcNpcSpSample_c::onDemoCmd(int cmd, u8 member) {
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
            mSampleTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpSample {
static const dAcNpcSpSample_c::State_s l_states[] = {
    {&dAcNpcSpSample_c::initIdle, &dAcNpcSpSample_c::executeIdle},
    {&dAcNpcSpSample_c::initFollow, &dAcNpcSpSample_c::executeFollow},
    {&dAcNpcSpSample_c::initLook, &dAcNpcSpSample_c::executeLook},
    {&dAcNpcSpSample_c::initTalk, &dAcNpcSpSample_c::executeTalk},
    {&dAcNpcSpSample_c::initRemote, &dAcNpcSpSample_c::executeRemote},
    {&dAcNpcSpSample_c::initRemoteTalk, &dAcNpcSpSample_c::executeRemoteTalk}};
} // namespace dANpcSpSample

// .text 0x2E8
void dAcNpcSpSample_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpSample::l_states[state].mInit)();
}
// .text 0x320
int dAcNpcSpSample_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mSampleTalk);
    return 1;
}
// .text 0x36C
int dAcNpcSpSample_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
    } else {
        changeState(STATE_REMOTE);
    }
    return 1;
}
// .text 0x3D8
int dAcNpcSpSample_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}
// .text 0x404
int dAcNpcSpSample_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpSample::l_states[mState].mExecute)();
    }
    return 1;
}
// .text 0x44C
void dAcNpcSpSample_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}
// .text 0x460
void dAcNpcSpSample_c::executeIdle() {
    if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) > 96.0f) {
        changeState(STATE_FOLLOW);
    }
}
// .text 0x4B0
void dAcNpcSpSample_c::initFollow() {
    mAction.requestWalk(
        PRIORITY_IDLE,
        *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT),
        l_walkTurnSpeed, 0, l_moveParamWalk, l_defaultAnmRate, cNpcMorphFrames);
}
// .text 0x514
void dAcNpcSpSample_c::executeFollow() {
    if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) < 64.0f) {
        changeState(STATE_IDLE);
    } else {
        mAction.setTargetPos(
            *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
    }
}
// .text 0x57C
void dAcNpcSpSample_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = STEP_START;
}
// .text 0x5E0
void dAcNpcSpSample_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}
namespace dANpcSpSample {
static const dAcNpcSpSample_c::Proc l_lookSteps[] = {
    &dAcNpcSpSample_c::stepLook};
} // namespace dANpcSpSample
// .text 0x640
void dAcNpcSpSample_c::executeLook() {
    if (mStep < STEP_DONE) {
        (this->*dANpcSpSample::l_lookSteps[mStep])();
    }
}
// .text 0x680
void dAcNpcSpSample_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    mStep = STEP_START;
}
// .text 0x6EC
void dAcNpcSpSample_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        ++mStep;
    }
}
namespace dANpcSpSample {
static const dAcNpcSpSample_c::Proc l_talkSteps[] = {
    &dAcNpcSpSample_c::stepTalk};
} // namespace dANpcSpSample
// .text 0x738
void dAcNpcSpSample_c::executeTalk() {
    if (mStep < STEP_DONE) {
        (this->*dANpcSpSample::l_talkSteps[mStep])();
    }
}
// .text 0x778
void dAcNpcSpSample_c::initRemote() {
}
// .text 0x77C
void dAcNpcSpSample_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mSampleTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
}
// .text 0x810
void dAcNpcSpSample_c::initRemoteTalk() {
}
// .text 0x814
void dAcNpcSpSample_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0x868
dAcNpcSpSample_c::talk_c::talk_c(dAcNpcSpSample_c *npc) : mpOwner(npc) {
    init();
}
// .text 0x8C8
dAcNpcSpSample_c::talk_c::~talk_c() {
}
// .text 0x920
void dAcNpcSpSample_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageStart = NULL;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
}
// .text 0x9B8
void dAcNpcSpSample_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (++mMsgNo > MSG_NUM) {
        mMsgNo = 1;
    }
}
// .text 0xA20
void dAcNpcSpSample_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_tanukichi";
    info->mCode = mMsgNo;
}
// .text 0xA38
void dAcNpcSpSample_c::talk_c::onMessageStart(int arg) {
    if (mOnMessageStart) {
        (this->*mOnMessageStart)();
        mOnMessageStart = NULL;
    }
}
// .text 0xA9C
void dAcNpcSpSample_c::talk_c::onMessageEnd(int kind) {
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
    }
}
// .text 0xAE4
void dAcNpcSpSample_c::talk_c::onAnswer(int arg) {
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}
// .text 0xB48
void dAcNpcSpSample_c::talk_c::preExecute() {
}
// .text 0xB4C
void dAcNpcSpSample_c::talk_c::onTalkEnd() {
}
// .text 0xB50
void dAcNpcSpSample_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}
