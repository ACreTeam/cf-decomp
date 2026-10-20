#include <game/game/d_a_npc_sp_fuusen.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_organ_balloon.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_heap.hpp>

// Phineas, the prize man. Functions are in target .text order.

namespace dANpcSpFuusen {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_FUUSEN, dAcNpcSpFuusen_c);

static const dAcNpcSpFuusen_c::State_s l_states[] = {
    {&dAcNpcSpFuusen_c::initIdle, &dAcNpcSpFuusen_c::executeIdle},
    {&dAcNpcSpFuusen_c::initLook, &dAcNpcSpFuusen_c::executeLook},
    {&dAcNpcSpFuusen_c::initTalk, &dAcNpcSpFuusen_c::executeTalk},
};
} // namespace dANpcSpFuusen

// .text 0x8C
dAcNpcSpFuusen_c::dAcNpcSpFuusen_c() : mPrizeTalk(this) {
    mpHeap = NULL;
    mpString = NULL;
}

// .text 0x274
void dAcNpcSpFuusen_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mPrizeTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0x31C
void dAcNpcSpFuusen_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpFuusen::l_states[state].mInit)();
}

// .text 0x354
int dAcNpcSpFuusen_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mPrizeTalk);
    return 1;
}

// .text 0x3A0
int dAcNpcSpFuusen_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    if (!dNpcMng_c::isWeekVisitDay()) {
        return FAILED; // not one of his visiting days
    }
    mAllocator_c *allocator = mAllocator;
    for (int i = 0; i < BALLOON_NUM; i++) {
        if (mpHeap == NULL) {
            mpHeap = mHeap::createFrmHeap(
                dHmnToolMng_c::getBankHeapSize(), dNpcMng_c::m_stageHeap_p,
                "風船おじさんdAcNpcSpFuusen_c::preCreate", 0x20,
                mHeap::OPT_NONE);
            allocator->attach(mpHeap, 0x20);
            mpString = dBalloonString_c::create(mpHeap);
        }
    }
    dItem::Item item(dItem::ITEM_IDX_BUNNY_R_BALLOON);
    if (!mBalloonRes[0].loadItem(item, mpHeap)) {
        return 0;
    }
    if (dOrganBalloon_c::s_instance == NULL) {
        return 0;
    }
    for (int i = 0; i < BALLOON_NUM; i++) {
        mBalloonMdl[i].create(
            nw4r::g3d::ResFile(mBalloonRes[i].getData()).GetResMdl(0),
            allocator, 0, 1, NULL);
        mBalloonClr[i].create(
            nw4r::g3d::ResFile(mBalloonRes[i].getData()).GetResMdl(0),
            nw4r::g3d::ResFile(mBalloonRes[i].getData()).GetResAnmClr(0),
            allocator, NULL, 1);
        mBalloonClr[i].setFrame(dOrganBalloon_c::s_instance->mColor, 0);
        mBalloonMdl[i].setAnm(mBalloonClr[i]);
    }
    mDrawCount = 0;
    mObjc.setDefWeight(0xFF);
    changeState(STATE_IDLE);
    getModel()->setRarmAnm(HMN_ANM_TANPOPO_WAIT1, m3d::PLAYMODE_INHERIT, 0.0f,
                           1.0f, 1.0f);
    return 1;
}

// .text 0x5D4
int dAcNpcSpFuusen_c::doDelete() {
    for (int i = 0; i < BALLOON_NUM; i++) {
        mBalloonMdl[i].remove();
        mBalloonClr[i].remove();
        if (!mBalloonRes[i].release()) {
            return 0;
        }
    }
    for (int i = 0; i < BALLOON_NUM; i++) {
        if (mpString != NULL) {
            mpString->destroy();
            mpString = NULL;
        }
        if (mpHeap != NULL) {
            mHeap::destroyFrmHeap(mpHeap);
            mpHeap = NULL;
        }
    }
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x698
int dAcNpcSpFuusen_c::draw() {
    if (mDrawCount != 0) {
        for (int i = 0; i < BALLOON_NUM; i++) {
            if (mpString != NULL) {
                mpString->entry();
                mBalloonMdl[i].entry();
            }
        }
    } else {
        mDrawCount++;
    }
    return dAcNpc_c::draw();
}

// .text 0x720
int dAcNpcSpFuusen_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpFuusen::l_states[mState].mExecute)();
        if (mDrawCount >= 1) {
            for (int i = 0; i < BALLOON_NUM; i++) {
                if (mpString == NULL) {
                    continue;
                }
                mMtx_c mtx;
                switch (i) {
                case 0:
                    mtx = *mpString->calc(&mNodeMtx0E);
                    break;
                case 1:
                    mtx = *mpString->calc(&mNodeMtx0B);
                    break;
                case 2:
                    mtx = *mpString->calc(&mNodeMtx12);
                    break;
                }
                mBalloonMdl[i].setLocalMtx(&mtx);
                mBalloonMdl[i].calc(false);
            }
            if (mDrawCount == 1) {
                mDrawCount++;
            }
        }
    }
    return 1;
}

// .text 0x950
void dAcNpcSpFuusen_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x964
void dAcNpcSpFuusen_c::executeIdle() {
}

// .text 0x968
void dAcNpcSpFuusen_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
    mStep = 0;
}

// .text 0x9CC
void dAcNpcSpFuusen_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep++;
    }
}

namespace dANpcSpFuusen {
static const dAcNpcSpFuusen_c::Proc l_lookSteps[] = {
    &dAcNpcSpFuusen_c::stepLook};
} // namespace dANpcSpFuusen

// .text 0xA2C
void dAcNpcSpFuusen_c::executeLook() {
    if (mStep < ARRAY_SIZE(dANpcSpFuusen::l_lookSteps)) {
        (this->*dANpcSpFuusen::l_lookSteps[mStep])();
    }
}

// .text 0xA6C
void dAcNpcSpFuusen_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep = 0;
}

// .text 0xAD8
void dAcNpcSpFuusen_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpFuusen {
static const dAcNpcSpFuusen_c::Proc l_talkSteps[] = {
    &dAcNpcSpFuusen_c::stepTalk};
} // namespace dANpcSpFuusen

// .text 0xB24
void dAcNpcSpFuusen_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpFuusen::l_talkSteps)) {
        (this->*dANpcSpFuusen::l_talkSteps[mStep])();
    }
}

// .text 0xB64
dAcNpcSpFuusen_c::talk_c::talk_c(dAcNpcSpFuusen_c *npc) : mpOwner(npc) {
    init();
}

// .text 0xBD0
dAcNpcSpFuusen_c::talk_c::~talk_c() {
}

// .text 0xC28
void dAcNpcSpFuusen_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageStart = NULL;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
}

// .text 0xCC0
void dAcNpcSpFuusen_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    mMsgNo = MSG_HELLO;
    mPrize = dItem::Item();
}

// .text 0xD1C
void dAcNpcSpFuusen_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_balloon";
    info->mCode = mMsgNo;
}

// .text 0xD34
void dAcNpcSpFuusen_c::talk_c::onMessageStart(int arg) {
    if (mOnMessageStart) {
        (this->*mOnMessageStart)();
        mOnMessageStart = NULL;
    }
}

// .text 0xD98
void dAcNpcSpFuusen_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_BALLOON:
    case MSG_BUNNY_BALLOON:
    case MSG_PINWHEEL:
    case MSG_BUBBLE_WAND:
        mOnMessageEnd = &talk_c::givePrize;
        switch (mMessageCode) {
        case MSG_BALLOON:
            mPrize.setFromIndex(dItem::ITEM_IDX_RED_BALLOON,
                                cM::rndInt(PRIZE_COLOR_NUM), FALSE);
            break;
        case MSG_BUNNY_BALLOON:
            mPrize.setFromIndex(dItem::ITEM_IDX_BUNNY_R_BALLOON,
                                cM::rndInt(PRIZE_COLOR_NUM), FALSE);
            break;
        case MSG_PINWHEEL:
            mPrize.setFromIndex(dItem::ITEM_IDX_RED_PINWHEEL,
                                cM::rndInt(PRIZE_COLOR_NUM), FALSE);
            break;
        case MSG_BUBBLE_WAND:
            mPrize.setFromIndex(dItem::ITEM_IDX_BUBBLE_WAND);
            break;
        }
        break;
    case MSG_BALLOON_GIVEN:
    case MSG_BUNNY_BALLOON_GIVEN:
    case MSG_PINWHEEL_GIVEN:
    case MSG_BUBBLE_WAND_GIVEN:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
            PRIVATE_FLAG1_PHINEAS_PRIZE);
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0xEE0
void dAcNpcSpFuusen_c::talk_c::onAnswer(int arg) {
    int answer = mpController->getAnswer();
    switch (mMessageCode) {
    case MSG_HELLO:
        if (answer == 0) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                    PRIVATE_FLAG1_PHINEAS_PRIZE)) {
                mpController->setNextMsgCode(MSG_GREEDY);
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(
                           0) < 0) {
                mpController->setNextMsgCode(MSG_POCKETS_FULL);
            } else {
                mpController->setNextMsgCode(MSG_CHOOSE);
            }
        }
        break;
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0xFB8
void dAcNpcSpFuusen_c::talk_c::preExecute() {
}

// .text 0xFBC
void dAcNpcSpFuusen_c::talk_c::onTalkEnd() {
}

// .text 0xFC0
void dAcNpcSpFuusen_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1024
void dAcNpcSpFuusen_c::talk_c::givePrize() {
    requestItemAct(&mPrize, 0, 0);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mPrize, FALSE);
}
