#include <game/game/d_a_npc_sp_ceremony_soncho.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_save_data.hpp>
#include <game/cLib/c_math.hpp>

// Tortimer at an opening ceremony. Functions are in target .text order.

namespace dANpcSpCeremonySoncho {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_CEREMONY_SONCHO, dAcNpcSpCeremonySoncho_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpCeremonySoncho

// .text 0x1CC
void dAcNpcSpCeremonySoncho_c::onDemoCmd(int cmd, u8 member) {
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
            mCeremonyTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpCeremonySoncho {
static const dAcNpcSpCeremonySoncho_c::State_s l_states[] = {
    {&dAcNpcSpCeremonySoncho_c::initIdle,
     &dAcNpcSpCeremonySoncho_c::executeIdle},
    {&dAcNpcSpCeremonySoncho_c::initLook,
     &dAcNpcSpCeremonySoncho_c::executeLook},
    {&dAcNpcSpCeremonySoncho_c::initTalk,
     &dAcNpcSpCeremonySoncho_c::executeTalk},
    {&dAcNpcSpCeremonySoncho_c::initRemote,
     &dAcNpcSpCeremonySoncho_c::executeRemote},
    {&dAcNpcSpCeremonySoncho_c::initRemoteTalk,
     &dAcNpcSpCeremonySoncho_c::executeRemoteTalk},
};
} // namespace dANpcSpCeremonySoncho

// .text 0x2E8
void dAcNpcSpCeremonySoncho_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpCeremonySoncho::l_states[state].mInit)();
}

// .text 0x320
int dAcNpcSpCeremonySoncho_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mCeremonyTalk);
    return 1;
}

// .text 0x36C
int dAcNpcSpCeremonySoncho_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mStep[STATE_IDLE] = 0;
    mStep[STATE_LOOK] = 0;
    mStep[STATE_TALK] = 0;
    mStep[STATE_REMOTE] = 0;
    mStep[STATE_REMOTE_TALK] = 0;
    if (dSaveData_c::getTown()->getNewConstruction() !=
        NEW_CONSTRUCTION_FOUNTAIN) {
        mObjc.setDefWeight(0x80);
    } else {
        moveFtrToPoliceBox();
    }
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        mDaub.mWander.mEmotion = 0;
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    mBlockX = (int)mPos.x >> 9;
    mBlockZ = (int)mPos.z >> 9;
    return 1;
}

// .text 0x478
int dAcNpcSpCeremonySoncho_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x4A4
int dAcNpcSpCeremonySoncho_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (dSaveData_c::getRaw()->getNewConstruction() ==
        NEW_CONSTRUCTION_FOUNTAIN) {
        mPos = mHomePos;
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpCeremonySoncho::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x544
void dAcNpcSpCeremonySoncho_c::initIdle() {
    initWander();
}

// .text 0x548
void dAcNpcSpCeremonySoncho_c::executeIdle() {
    if (executeWander(&mDaub.mWander)) {
        setDaubData(&mDaub, sizeof(mDaub));
    }
}

// .text 0x59C
void dAcNpcSpCeremonySoncho_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
    mStep[STATE_LOOK] = LOOK_STEP_TURN;
}

// .text 0x600
void dAcNpcSpCeremonySoncho_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep[STATE_LOOK]++;
    }
}

namespace dANpcSpCeremonySoncho {
static const dAcNpcSpCeremonySoncho_c::Proc l_lookSteps[] = {
    &dAcNpcSpCeremonySoncho_c::stepLook};
} // namespace dANpcSpCeremonySoncho

// .text 0x660
void dAcNpcSpCeremonySoncho_c::executeLook() {
    if (mStep[STATE_LOOK] < ARRAY_SIZE(dANpcSpCeremonySoncho::l_lookSteps)) {
        (this->*dANpcSpCeremonySoncho::l_lookSteps[mStep[STATE_LOOK]])();
    }
}

// .text 0x6A0
void dAcNpcSpCeremonySoncho_c::initTalk() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep[STATE_TALK] = TALK_STEP_TALK;
}

// .text 0x73C
void dAcNpcSpCeremonySoncho_c::stepTalk() {
    if (!mTalk.isBusy()) {
        if (dSaveData_c::getTown()->getNewConstruction() ==
            NEW_CONSTRUCTION_FOUNTAIN) {
            mAction.requestTurn(PRIORITY_DEMO, mAng(0), l_turnSpeed, 0);
            mStep[STATE_TALK] = TALK_STEP_TURN_END;
        } else {
            dDemo::finishTalk(this);
            mStep[STATE_TALK] = TALK_STEP_DONE;
        }
    }
}

// .text 0x7D0
void dAcNpcSpCeremonySoncho_c::stepTalkTurnEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mStep[STATE_TALK]++;
    }
}

namespace dANpcSpCeremonySoncho {
static const dAcNpcSpCeremonySoncho_c::Proc l_talkSteps[] = {
    &dAcNpcSpCeremonySoncho_c::stepTalk,
    &dAcNpcSpCeremonySoncho_c::stepTalkTurnEnd};
} // namespace dANpcSpCeremonySoncho

// .text 0x820
void dAcNpcSpCeremonySoncho_c::executeTalk() {
    if (mStep[STATE_TALK] < ARRAY_SIZE(dANpcSpCeremonySoncho::l_talkSteps)) {
        (this->*dANpcSpCeremonySoncho::l_talkSteps[mStep[STATE_TALK]])();
    }
}

// .text 0x860
void dAcNpcSpCeremonySoncho_c::initRemote() {
    resetLookTimer();
}

// .text 0x868
void dAcNpcSpCeremonySoncho_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mCeremonyTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    executeRemoteWander(&mDaub.mWander);
}

// .text 0x908
void dAcNpcSpCeremonySoncho_c::initRemoteTalk() {
}

// .text 0x90C
void dAcNpcSpCeremonySoncho_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0x960
BOOL dAcNpcSpCeremonySoncho_c::searchWanderPos(mVec3_c *pos) {
    if (dSaveData_c::getTown()->getNewConstruction() ==
        NEW_CONSTRUCTION_FOUNTAIN) {
        return FALSE;
    }
    mVec3_c center;
    f32 x = mHomePos.x;
    f32 z = mHomePos.z;
    dFdBase_c::getBlockCenterPos(&center, (int)x >> 9, (int)z >> 9);
    if (searchPosInCircle(pos, &center, &mPos, lbl_80750520 / 2,
                          64.0f + getWanderMinDist(), lbl_80750520 / 2)) {
        return TRUE;
    }
    return searchPosNear(pos, &mPos, getWanderMinDist(), getWanderMaxDist());
}

// .text 0xA84
BOOL dAcNpcSpCeremonySoncho_c::isWanderWalk() {
    BOOL ret = FALSE;
    if (mAction.mActionId == ACTION_WAIT) {
        if (cM::rndInt(100) < dNpcInfo::getWalkChance()) {
            ret = TRUE;
        }
    }
    return ret;
}

// .text 0xAE0
int dAcNpcSpCeremonySoncho_c::vtE4() {
    return 0;
}

// .text 0xAE8
dAcNpcSpCeremonySoncho_c::talk_c::talk_c(dAcNpcSpCeremonySoncho_c *npc)
    : mpOwner(npc) {
    init();
}

// .text 0xB48
dAcNpcSpCeremonySoncho_c::talk_c::~talk_c() {
}

// .text 0xBA0
void dAcNpcSpCeremonySoncho_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
}

// .text 0xBA4
void dAcNpcSpCeremonySoncho_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    switch (dSaveData_c::getTown()->getNewConstruction()) {
    case NEW_CONSTRUCTION_BRIDGE:
        mMsgNo = MSG_BRIDGE;
        break;
    case NEW_CONSTRUCTION_FOUNTAIN:
        mMsgNo = MSG_FOUNTAIN;
        break;
    case NEW_CONSTRUCTION_WINDMILL:
        mMsgNo = MSG_WINDMILL;
        break;
    case NEW_CONSTRUCTION_LIGHTHOUSE:
        mMsgNo = MSG_LIGHTHOUSE;
        break;
    default:
        mMsgNo = 0;
        break;
    }
    if (dNpcInfo::soncho_c::get()->testTalked()) {
        mMsgNo++;
    }
}

// .text 0xC74
void dAcNpcSpCeremonySoncho_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_soncho_0";
    info->mCode = mMsgNo;
}

// .text 0xC8C
void dAcNpcSpCeremonySoncho_c::talk_c::onMessageStart(int arg) {
}

// .text 0xC90
void dAcNpcSpCeremonySoncho_c::talk_c::onMessageEnd(int kind) {
}

// .text 0xC94
void dAcNpcSpCeremonySoncho_c::talk_c::onAnswer(int arg) {
}

// .text 0xC98
void dAcNpcSpCeremonySoncho_c::talk_c::preExecute() {
}

// .text 0xC9C
void dAcNpcSpCeremonySoncho_c::talk_c::onTalkEnd() {
}
