#include <game/game/d_a_npc_sp_golditem_soncho.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/cLib/c_math.hpp>

// Tortimer with a golden tool. Functions are in target .text order.

namespace dANpcSpGolditemSoncho {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_GOLDITEM_SONCHO, dAcNpcSpGoldItemSoncho_c);
} // namespace dANpcSpGolditemSoncho

// .text 0x1CC
void dAcNpcSpGoldItemSoncho_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mGoldTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        fgMngProc_unblockDayChange();
        mGoldTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_WALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpGolditemSoncho {
static const dAcNpcSpGoldItemSoncho_c::State_s l_states[] = {
    {&dAcNpcSpGoldItemSoncho_c::initCall,
     &dAcNpcSpGoldItemSoncho_c::executeCall},
    {&dAcNpcSpGoldItemSoncho_c::initIdle,
     &dAcNpcSpGoldItemSoncho_c::executeIdle},
    {&dAcNpcSpGoldItemSoncho_c::initLook,
     &dAcNpcSpGoldItemSoncho_c::executeLook},
    {&dAcNpcSpGoldItemSoncho_c::initTalk,
     &dAcNpcSpGoldItemSoncho_c::executeTalk},
    {&dAcNpcSpGoldItemSoncho_c::initWalk,
     &dAcNpcSpGoldItemSoncho_c::executeWalk},
};
} // namespace dANpcSpGolditemSoncho

// .text 0x29C
void dAcNpcSpGoldItemSoncho_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpGolditemSoncho::l_states[state].mInit)();
}

// .text 0x2D4
int dAcNpcSpGoldItemSoncho_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mGoldTalk);
    return 1;
}

// .text 0x320
int dAcNpcSpGoldItemSoncho_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    changeState(STATE_CALL);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    u32 flag;
    if (!player->isFlag0(PRIVATE_FLAG0_GOLDEN_NET_GIVEN) &&
        player->mCatalog.isComplete(dItem::KIND_INSECT) == TRUE) {
        mNet = TRUE;
        flag = PRIVATE_FLAG0_GOLDEN_NET_GIVEN;
    } else {
        mNet = FALSE;
        flag = PRIVATE_FLAG0_GOLDEN_ROD_GIVEN;
    }
    dPrivateData_c *other;
    mFirstInTown = TRUE;
    for (int i = 0; i < PLAYER_NUM; i++) {
        other = dPlayerMgr::Mgr_c::getPlayer(i);
        if (other->mPID.isValid() && other->isFlag0(flag)) {
            mFirstInTown = FALSE;
            break;
        }
    }
    mObjc.setDefWeight(0x80);
    dNpcMng_c::_8074EACD = 1;
    mBlockX = (int)mPos.x >> 9;
    mBlockZ = (int)mPos.z >> 9;
    dBgm::l_mgr.mStgField.mute2C();
    fgMngProc_blockDayChange();
    return 1;
}

// .text 0x480
int dAcNpcSpGoldItemSoncho_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x4AC
int dAcNpcSpGoldItemSoncho_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpGolditemSoncho::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x4F4
void dAcNpcSpGoldItemSoncho_c::initCall() {
    setDemoFlag8();
    mCalling = TRUE;
    mFirstCall = TRUE;
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x540
void dAcNpcSpGoldItemSoncho_c::executeCall() {
    dDemo::requestSpeak(this, NULL);
}

// .text 0x548
void dAcNpcSpGoldItemSoncho_c::initIdle() {
    mFirstCall = FALSE;
    initWander();
}

// .text 0x554
void dAcNpcSpGoldItemSoncho_c::executeIdle() {
    executeWander(NULL);
}

// .text 0x568
BOOL dAcNpcSpGoldItemSoncho_c::searchWanderPos(mVec3_c *pos) {
    mVec3_c target;
    dFdBase_c::getBlockCenterPos(&target, mBlockX, mBlockZ);
    target.x += cM::rndCenterF(256.0f);
    target.y = 0.0f;
    dNpc::pickNearbyFreePos(&target, &target, 16);
    int unitX = (int)target.x >> 5;
    int blockZ = (int)target.z >> 9;
    mVec3_c freePos;
    freePos = calcFreePosToward(&target);
    if ((unitX >> 4) == mBlockX && blockZ == mBlockZ &&
        mPos.xzDistTo(freePos) >= 8.0f) {
        *pos = freePos;
        return TRUE;
    }
    return FALSE;
}

// .text 0x6A8
BOOL dAcNpcSpGoldItemSoncho_c::isWanderWalk() {
    BOOL ret = FALSE;
    if (mAction.mActionId == ACTION_WAIT) {
        if (cM::rndInt(100) < dNpcInfo::getWalkChance()) {
            ret = TRUE;
        }
    }
    return ret;
}

// .text 0x704
void dAcNpcSpGoldItemSoncho_c::initLook() {
    if (mCalling) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep = 1;
    } else {
        mAng angle = getAngleYToPlayer(_1B70);
        mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
        mStep = 0;
    }
}

// .text 0x794
void dAcNpcSpGoldItemSoncho_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep++;
    }
}

namespace dANpcSpGolditemSoncho {
static const dAcNpcSpGoldItemSoncho_c::Proc l_lookSteps[] = {
    &dAcNpcSpGoldItemSoncho_c::stepLook};
} // namespace dANpcSpGolditemSoncho

// .text 0x7F4
void dAcNpcSpGoldItemSoncho_c::executeLook() {
    if (mStep < ARRAY_SIZE(dANpcSpGolditemSoncho::l_lookSteps)) {
        (this->*dANpcSpGolditemSoncho::l_lookSteps[mStep])();
    }
}

// .text 0x834
void dAcNpcSpGoldItemSoncho_c::initTalk() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep = 0;
}

// .text 0x8CC
void dAcNpcSpGoldItemSoncho_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        if (mFirstCall) {
            dBgm::l_mgr.mStgField.fn_80078658();
        }
        mStep++;
    }
}

namespace dANpcSpGolditemSoncho {
static const dAcNpcSpGoldItemSoncho_c::Proc l_talkSteps[] = {
    &dAcNpcSpGoldItemSoncho_c::stepTalk};
} // namespace dANpcSpGolditemSoncho

// .text 0x934
void dAcNpcSpGoldItemSoncho_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpGolditemSoncho::l_talkSteps)) {
        (this->*dANpcSpGolditemSoncho::l_talkSteps[mStep])();
    }
}

// .text 0x974
void dAcNpcSpGoldItemSoncho_c::initWalk() {
    mVec3_c target;
    target = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    target.z = mPos.z;
    dPlayerMgr::moveTo(&target, dPlayerMgr::PLAYER_INDEX_CURRENT,
                       dPlayerMgr::sDefaultMoveSpeed);
    mStep = 0;
}

// .text 0x9E0
void dAcNpcSpGoldItemSoncho_c::executeWalk() {
    if (!dPlayerMgr::isMovingTo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        clearDemoFlag8();
        mCalling = FALSE;
        changeState(STATE_TALK);
    }
}

// .text 0xA34
dAcNpcSpGoldItemSoncho_c::talk_c::talk_c(dAcNpcSpGoldItemSoncho_c *npc)
    : mpOwner(npc) {
    init();
}

// .text 0xA94
dAcNpcSpGoldItemSoncho_c::talk_c::~talk_c() {
}

// .text 0xAEC
void dAcNpcSpGoldItemSoncho_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageStart = NULL;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
}

namespace dANpcSpGolditemSoncho {
// start: the call messages, then the ones after the player came back.
static const u16 l_msgs[2][dAcNpcSpGoldItemSoncho_c::talk_c::GIVE_NUM] = {
    {
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_POCKETS_FULL,
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_NET_HERO,
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_NET_NOT_FIRST,
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_ROD,
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_ROD_NOT_FIRST,
    },
    {
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_POCKETS_FULL_AGAIN,
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_NET_HERO_AGAIN,
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_NET_NOT_FIRST_AGAIN,
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_ROD_AGAIN,
        dAcNpcSpGoldItemSoncho_c::talk_c::MSG_ROD_NOT_FIRST_AGAIN,
    },
};
} // namespace dANpcSpGolditemSoncho

// .text 0xB84
void dAcNpcSpGoldItemSoncho_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    u8 net;
    u8 firstCall;
    dPrivateData_c *player;
    player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    net = mpOwner->mNet;
    firstCall = mpOwner->mFirstCall;
    u32 flag;
    int give;
    if (net) {
        flag = PRIVATE_FLAG0_GOLDEN_NET_GIVEN;
        give = GIVE_NET;
    } else {
        flag = PRIVATE_FLAG0_GOLDEN_ROD_GIVEN;
        give = GIVE_ROD;
    }
    if (!firstCall && player->isFlag0(flag)) {
        if (net) {
            mMsgNo = MSG_NET_GIVEN;
        } else {
            mMsgNo = MSG_ROD_GIVEN;
        }
        return;
    }
    if (player->findEmptyPocket(0) < 0) {
        give = GIVE_POCKETS_FULL;
    } else if (!mpOwner->mFirstInTown) {
        give++;
    }
    if (firstCall) {
        mMsgNo = dANpcSpGolditemSoncho::l_msgs[0][give];
    } else {
        mMsgNo = dANpcSpGolditemSoncho::l_msgs[1][give];
    }
}

// .text 0xC9C
void dAcNpcSpGoldItemSoncho_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_soncho_0";
    info->mCode = mMsgNo;
}

// .text 0xCB4
void dAcNpcSpGoldItemSoncho_c::talk_c::onMessageStart(int arg) {
    if (mOnMessageStart) {
        (this->*mOnMessageStart)();
        mOnMessageStart = NULL;
    }
}

// .text 0xD18
void dAcNpcSpGoldItemSoncho_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_NET_HERO:
    case MSG_NET_NOT_FIRST:
    case MSG_NET_HERO_AGAIN:
    case MSG_NET_NOT_FIRST_AGAIN:
    case MSG_ROD:
    case MSG_ROD_NOT_FIRST:
    case MSG_ROD_AGAIN:
    case MSG_ROD_NOT_FIRST_AGAIN:
        mOnMessageEnd = &talk_c::giveItem;
        mOnRequestEnd = &talk_c::thank;
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0xDDC
void dAcNpcSpGoldItemSoncho_c::talk_c::giveItem() {
    dItem::Item item;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (mpOwner->mNet) {
        item.setFromIndex(dItem::ITEM_IDX_GOLDEN_NET);
        player->setFlag0(PRIVATE_FLAG0_GOLDEN_NET_GIVEN);
    } else {
        item.setFromIndex(dItem::ITEM_IDX_GOLDEN_ROD);
        player->setFlag0(PRIVATE_FLAG0_GOLDEN_ROD_GIVEN);
    }
    requestItemAct(&item, 0, 0);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(item, FALSE);
}

// .text 0xE88
void dAcNpcSpGoldItemSoncho_c::talk_c::thank() {
    if (mpOwner->mNet) {
        if (mpOwner->mFirstInTown) {
            mpController->setNextMsgCode(MSG_NET_THANKS);
        } else {
            mpController->setNextMsgCode(MSG_NET_THANKS_NOT_FIRST);
        }
    } else if (mpOwner->mFirstInTown) {
        mpController->setNextMsgCode(MSG_ROD_THANKS);
    } else {
        mpController->setNextMsgCode(MSG_ROD_THANKS_NOT_FIRST);
    }
}

// .text 0xEE0
void dAcNpcSpGoldItemSoncho_c::talk_c::onAnswer(int arg) {
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0xF44
void dAcNpcSpGoldItemSoncho_c::talk_c::preExecute() {
}

// .text 0xF48
void dAcNpcSpGoldItemSoncho_c::talk_c::onTalkEnd() {
}

// .text 0xF4C
void dAcNpcSpGoldItemSoncho_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// Default construction is empty; retain the original empty TU initializer.
namespace dANpcSpGolditemSoncho {
static const mAng l_zeroAngle;
}
