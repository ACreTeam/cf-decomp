#include <game/game/d_a_npc_sp_franklin.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>

namespace dANpcSpFranklin {
// 0x5C
static BOOL isKnifeAndFork(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_KNIFE_AND_FORK;
}

// 0x88
static BOOL entryDebugPage(dAcNpcSpFranklin_c *npc) {
    return FALSE;
}

// 0x90
BASE_PROFILE(A_NPC_SP_FRANKLIN, dAcNpcSpFranklin_c);

// Unreferenced (.bss 0x8, built by __sinit).
struct unk_s {
    unk_s() : _0(0) {}
    u32 _0;
};
static unk_s l_unk;

static inline int countKnifeAndFork(u16 *mask) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(isKnifeAndFork, mask);
}
} // namespace dANpcSpFranklin

// 0x20C
void dAcNpcSpFranklin_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeState(STATE_LOOK);
        }
        break;
    case DEMO_CMD_SPEAK:
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mFranklinTalk.start(dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_TALK:
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mFranklinTalk.start(dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
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

namespace dANpcSpFranklin {
static const dAcNpcSpFranklin_c::State_s l_states[] = {
    {&dAcNpcSpFranklin_c::initIdle, &dAcNpcSpFranklin_c::executeIdle},
    {&dAcNpcSpFranklin_c::initLook, &dAcNpcSpFranklin_c::executeLook},
    {&dAcNpcSpFranklin_c::initTalk, &dAcNpcSpFranklin_c::executeTalk},
    {&dAcNpcSpFranklin_c::initRemote, &dAcNpcSpFranklin_c::executeRemote},
    {&dAcNpcSpFranklin_c::initRemoteTalk, &dAcNpcSpFranklin_c::executeRemoteTalk},
};
} // namespace dANpcSpFranklin

// 0x36C
void dAcNpcSpFranklin_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpFranklin::l_states[state].mInit)();
}

// 0x3A4
int dAcNpcSpFranklin_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mFranklinTalk);
    return SUCCEEDED;
}

// 0x3F0
int dAcNpcSpFranklin_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    mStep[STATE_IDLE] = 0;
    mStep[STATE_LOOK] = 0;
    mStep[STATE_TALK] = 0;
    mStep[STATE_REMOTE] = 0;
    mStep[STATE_REMOTE_TALK] = 0;
    mSpotTimer = 0;
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        if (isHarvestFestival()) {
            mDaub.mAtSpot = 1;
        } else {
            mDaub.mAtSpot = 0;
        }
        if (!dNpc::canPutItemAt(&mPos, mNpcItem, NULL, dScene::SCENE_NUM, FALSE, TRUE) ||
            !dNpc::isPosBlockedToSouth(&mPos)) {
            mDaub.mAtSpot = 0;
        }
        setDaubData(&mDaub, sizeof(mDaub));
        if (!dSaveData_c::getTown()->mHarvestSpot.mValid) {
            dSaveData_c::getTown()->mHarvestSpot.set(&mPos);
        }
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (mDaub.mAtSpot) {
        mIsActive = 1;
    } else {
        mIsActive = 0;
    }
    mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    dSceneCheck::fn_80081858(dANpcSpFranklin::entryDebugPage(this));
    return SUCCEEDED;
}

// 0x574
int dAcNpcSpFranklin_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return NOT_READY;
    }
    dANpcSpFranklin::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return SUCCEEDED;
}

// 0x5C0
int dAcNpcSpFranklin_c::vtB0() {
    if (mSpotTimer > 0) {
        mSpotTimer--;
    }
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (isLocalOwner() && dNet::isSceneOwner() && mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
        if (isHarvestFestival()) {
            if (!mDaub.mAtSpot) {
                dSaveTown_c *town = dSaveData_c::getTown();
                mVec3_c pos = town->mHarvestSpot.getPos();
                if (!dNpc::canPutItemAt(&pos, mNpcItem, NULL, dScene::SCENE_NUM, FALSE, TRUE) ||
                    !dNpc::isPosBlockedToSouth(&pos)) {
                    if (mSpotTimer <= 0) {
                        dNpcInfo::franklin_c::get()->setSpot(&pos, lbl_80750520);
                        mSpotTimer = SPOT_RETRY_FRAMES;
                    }
                } else if (mViewArea.isOutsideAll(NET_MEMBER_MAX, &pos)) {
                    mDaub.mAtSpot = 1;
                    mPos = mLastPos = mHomePos = pos;
                    mAngle3D.y = 0;
                    mAngle.y = 0;
                    stopManpu();
                    setDaubData(&mDaub, sizeof(mDaub));
                }
            }
        } else if (mDaub.mAtSpot) {
            mDaub.mAtSpot = 0;
            setDaubData(&mDaub, sizeof(mDaub));
        }
    }
    if (mDaub.mAtSpot) {
        mIsActive = 1;
    } else {
        mIsActive = 0;
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpFranklin::l_states[mState].mExecute)();
    }
    return 1;
}

// 0x7D0
int dAcNpcSpFranklin_c::canSpeak(dDemoActor_c *actor) {
    if (isInUse(NULL)) {
        return FALSE;
    }
    if (mIsActive == 0) { // not !mIsActive: branch order
        return FALSE;
    }
    return !mTalk.isBusy();
}

// 0x834
void dAcNpcSpFranklin_c::initIdle() {
    mAng front(0);
    mAction.requestTurn(PRIORITY_IDLE, front, l_turnSpeed, 0);
    mIdleTimer = cM::rndInt(IDLE_WAIT_RAND) + IDLE_WAIT_MIN;
}

// At the festival spot: fret while the spot is open to the south, else look around now and then.
inline void dAcNpcSpFranklin_c::executeAtSpot(BOOL remote) {
    if (!dNpc::isPosBlockedToSouth(&mPos)) {
        if (mAction.mActionId != ACTION_EMOTION || mAction.mEmotion != EMOTION_FRET) {
            stopManpu();
            mAction.requestEmotion(PRIORITY_IDLE, EMOTION_FRET, cNpcMorphFrames);
        }
    } else if (mAction.mActionId == ACTION_ANM) {
        if (mAction.mCanChange) {
            if (remote) {
                mAction.requestMoveDest(PRIORITY_IDLE);
            } else {
                mAction.requestWait(PRIORITY_IDLE);
            }
            mIdleTimer = cM::rndInt(IDLE_WAIT_RAND) + IDLE_WAIT_MIN;
        }
    } else if (mIdleTimer > 0) {
        mIdleTimer--;
    } else {
        mAction.requestAnm(PRIORITY_IDLE, selectFreeSide(), 0.0f, cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
    }
}

// 0x898
void dAcNpcSpFranklin_c::executeIdle() {
    if (dSaveData_c::getTown()->mHarvestSpot.mFlag && mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
        relocate();
    }
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_IDLE);
    } else if (mIsActive) {
        executeAtSpot(FALSE);
    }
}

// 0xA04
void dAcNpcSpFranklin_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
    mStep[STATE_LOOK] = LOOK_STEP_TURN;
}

// 0xA68
void dAcNpcSpFranklin_c::stepLookTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep[STATE_LOOK]++;
    }
}

namespace dANpcSpFranklin {
static const dAcNpcSpFranklin_c::Proc l_lookSteps[] = {&dAcNpcSpFranklin_c::stepLookTurn};
} // namespace dANpcSpFranklin

// 0xAC8
void dAcNpcSpFranklin_c::executeLook() {
    if (mStep[STATE_LOOK] < LOOK_STEP_DONE) {
        (this->*dANpcSpFranklin::l_lookSteps[mStep[STATE_LOOK]])();
    }
}

// 0xB08
void dAcNpcSpFranklin_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, 0);
    mStep[STATE_TALK] = TALK_STEP_TURN;
}

// 0xB74
void dAcNpcSpFranklin_c::stepTalkTurn() {
    if (!mTalk.isBusy()) {
        mAng front(0);
        mAction.requestTurn(PRIORITY_DEMO, front, l_turnSpeed, 0);
        mStep[STATE_TALK]++;
    }
}

// 0xBE4
void dAcNpcSpFranklin_c::stepTalkTurnEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mStep[STATE_TALK]++;
    }
}

namespace dANpcSpFranklin {
static const dAcNpcSpFranklin_c::Proc l_talkSteps[] = {
    &dAcNpcSpFranklin_c::stepTalkTurn,
    &dAcNpcSpFranklin_c::stepTalkTurnEnd,
};
} // namespace dANpcSpFranklin

// 0xC34
void dAcNpcSpFranklin_c::executeTalk() {
    if (mStep[STATE_TALK] < TALK_STEP_DONE) {
        (this->*dANpcSpFranklin::l_talkSteps[mStep[STATE_TALK]])();
    }
}

// 0xC74
void dAcNpcSpFranklin_c::initRemote() {
    mAction.requestMoveDest(PRIORITY_IDLE);
    mIdleTimer = 1;
}

// 0xCB0
void dAcNpcSpFranklin_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mFranklinTalk.start(dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    } else {
        int member;
        if (isInUse(&member) && member != dNet::getMyNo()) {
            mAcceptMoveReq = 1;
        } else {
            mAcceptMoveReq = 0;
            if (mIsActive) {
                executeAtSpot(TRUE);
            }
        }
    }
}

// 0xE58
void dAcNpcSpFranklin_c::initRemoteTalk() {}

// 0xE5C
void dAcNpcSpFranklin_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// 0xEB0
bool dAcNpcSpFranklin_c::isHarvestFestival() {
    return dEvent::isOngoing(EVENT_HARVEST_FESTIVAL);
}

// Unused (stripped by the linker); their const-ref event temporaries stay in .data.
bool dAcNpcSpFranklin_c::isHarvestFestivalNotStarted() {
    return dEvent::isNotStarted(EVENT_HARVEST_FESTIVAL);
}

bool dAcNpcSpFranklin_c::isHarvestFestivalOver() {
    return dEvent::isOver(EVENT_HARVEST_FESTIVAL);
}

// 0xEE4
void dAcNpcSpFranklin_c::relocate() {
    dNpcInfo::franklin_c::get()->relocate();
    dNpcInfo::franklin_c::get()->setSpot(&mPos, lbl_80750520);
    mDaub.mAtSpot = 0;
    setDaubData(&mDaub, sizeof(mDaub));
}

// 0xF44
dAcNpcSpFranklin_c::talk_c::talk_c(dAcNpcSpFranklin_c *npc) : mpOwner(npc) {
    init();
}

// 0xFB0
dAcNpcSpFranklin_c::talk_c::~talk_c() {}

// 0x1008
void dAcNpcSpFranklin_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mItem = dItem::Item();
    mTookKnifeFork = 0;
}

// 0x1080
void dAcNpcSpFranklin_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (!dNpcInfo::franklin_c::get()->isTalked()) {
        mMsgNo = MSG_FIRST;
    } else if (dNpcInfo::franklin_c::get()->mRelocated) {
        if (!dNpcInfo::franklin_c::get()->mTalkedHere) {
            mMsgNo = MSG_RAND_SCREAM;
        } else if (dNpcInfo::franklin_c::get()->mGaveHere) {
            mMsgNo = MSG_RAND_SHAKEN;
        } else if (dANpcSpFranklin::countKnifeAndFork(NULL) > 0) {
            if (dNpcInfo::franklin_c::get()->mHandedOver) {
                mMsgNo = MSG_ANOTHER;
            } else {
                mMsgNo = MSG_GO_AWAY;
            }
        } else {
            mMsgNo = MSG_RAND_COMPLAIN;
        }
    } else if (dNpcInfo::franklin_c::get()->mHandedOver) {
        mMsgNo = MSG_RAND_SHAKEN;
    } else if (dANpcSpFranklin::countKnifeAndFork(NULL) > 0) {
        mMsgNo = MSG_STILL_HERE;
    } else if (dNpcInfo::franklin_c::get()->mAskedForHelp) {
        mMsgNo = MSG_RAND_GO_AWAY;
    } else {
        mMsgNo = MSG_HANGING_AROUND;
    }
}

// 0x11E4
void dAcNpcSpFranklin_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_franklin";
    info->mCode = mMsgNo;
}

// 0x11FC
void dAcNpcSpFranklin_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_GIFT:
    case MSG_GIFT_2:
    case MSG_GIFT_3:
    case MSG_GIFT_4:
    case MSG_GIFT_HIDING:
    case MSG_GIFT_AGAIN:
    case MSG_GIFT_MANY: {
        dItemSel::Range_c range(dItem::KIND_NONE, dItem::FROM_HARVESTFESTIVAL);
        dItemSel::select(&mItem, 1, &range, 1, &dItemSel::sDefaultFilter, NULL, 0, NULL);
        setItemName(mItem, 0);
        break;
    }
    }
}

// 0x12C4
void dAcNpcSpFranklin_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_ANIMALS:
        if (dANpcSpFranklin::countKnifeAndFork(NULL) > 0) {
            mpController->setNextMsgCode(MSG_KNIVES);
        } else {
            mpController->setNextMsgCode(MSG_KNIVES_NONE);
        }
        break;
    case MSG_SECRET:
    case MSG_SECRET_2:
    case MSG_SECRET_HIDING:
        dNpcInfo::franklin_c::get()->setHandedOver();
        break;
    case MSG_HANGING_AROUND:
        dNpcInfo::franklin_c::get()->setAskedForHelp();
        break;
    case MSG_SWIPED:
        dNpcInfo::franklin_c::get()->setThanked();
        break;
    case MSG_HAND_OVER:
    case MSG_HAND_OVER_2:
    case MSG_HAND_OVER_3:
    case MSG_HAND_OVER_4:
    case MSG_HAND_OVER_HIDING:
    case MSG_HAND_OVER_AGAIN:
    case MSG_HAND_OVER_MANY:
        mItem = dItem::Item(ITEM_IDX_KNIFE_AND_FORK);
        mpController->setNextMsgCode(mMessageCode + 1);
        mOnMessageEnd = &talk_c::reqReceiveKnifeFork;
        mOnRequestEnd = &talk_c::takeKnifeFork;
        break;
    case MSG_GIFT:
    case MSG_GIFT_2:
    case MSG_GIFT_3:
    case MSG_GIFT_4:
    case MSG_GIFT_HIDING:
    case MSG_GIFT_MANY:
        mpController->setNextMsgCode(mMessageCode + 1);
        mOnMessageEnd = &talk_c::reqGiveGift;
        mOnRequestEnd = &talk_c::pickUpGift;
        break;
    case MSG_GIFT_AGAIN:
        if (dNpcInfo::franklin_c::get()->mThanked) {
            mpController->setNextMsgCode(MSG_FOR_ME);
        } else {
            mpController->setNextMsgCode(MSG_SWIPED);
        }
        mOnMessageEnd = &talk_c::reqGiveGift;
        mOnRequestEnd = &talk_c::pickUpGift;
        break;
    case MSG_JUST_YOU:
    case MSG_JUST_YOU + 2:
    case MSG_JUST_YOU + 4:
    case MSG_JUST_YOU + 6:
    case MSG_JUST_YOU + 8:
    case MSG_JUST_YOU + 10:
    case MSG_JUST_YOU + 12:
    case MSG_JUST_YOU + 14:
    case MSG_JUST_YOU + 16:
    case MSG_JUST_YOU + 18:
        if (dNpcInfo::franklin_c::get()->mGaveHere) {
            mpController->setNextMsgCode(MSG_RAND_SHAKEN);
        } else if (dANpcSpFranklin::countKnifeAndFork(NULL) > 0) {
            if (dNpcInfo::franklin_c::get()->mHandedOver) {
                mpController->setNextMsgCode(MSG_ANOTHER);
            } else {
                mpController->setNextMsgCode(MSG_GO_AWAY);
            }
        } else {
            mpController->setNextMsgCode(MSG_RAND_COMPLAIN);
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// 0x1560
void dAcNpcSpFranklin_c::talk_c::onAnswer(int arg) {}

// 0x1564
void dAcNpcSpFranklin_c::talk_c::preExecute() {}

// 0x1568
void dAcNpcSpFranklin_c::talk_c::onTalkEnd() {
    dNpcInfo::franklin_c::get()->setTalked();
    if (mTookKnifeFork == TRUE) {
        dNpcInfo::franklin_c::get()->setGaveKnifeFork();
        dSaveData_c::getTown()->mHarvestSpot.mFlags |= dSvNpcPos::SIDE_POS_FLAG_EVENT;
        if (dAcNpc_c::isMultiPlay()) {
            dNet::beginPacket();
            dNet::sendPacket(NET_PACKET_HARVEST_SPOT_FLAG, NET_MEMBER_NONE);
        }
    }
}

// 0x15DC
void dAcNpcSpFranklin_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// 0x1640
void dAcNpcSpFranklin_c::talk_c::reqReceiveKnifeFork() {
    requestItemActEx(6, &mItem, 0, 0, 0, 2);
}

// 0x165C
void dAcNpcSpFranklin_c::talk_c::reqGiveGift() {
    requestItemAct(&mItem, 0, 0);
}

// 0x166C
void dAcNpcSpFranklin_c::talk_c::takeKnifeFork() {
    u16 mask;
    dANpcSpFranklin::countKnifeAndFork(&mask);
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (mask & (1 << i)) {
            dItem::Item empty;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&empty, i, FALSE);
            break;
        }
    }
    mTookKnifeFork = 1;
}

// 0x1704
void dAcNpcSpFranklin_c::talk_c::pickUpGift() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem, FALSE);
}

// 0x173C
int dAcNpcSpFranklin_c::getEarType() {
    return NPC_EAR_TYPE_FRANKLIN;
}
