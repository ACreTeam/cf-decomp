#include <game/game/d_a_npc_sp_jingle.hpp>
#include <game/game/d_a_npc_sp_jingle_inline.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_animal.hpp>
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

// Jingle the black-nosed reindeer on Toy Day. Functions are in target .text
// order.

namespace dANpcSpJingle {
// .text 0x5C
static BOOL entryDebugPage(dAcNpcSpJingle_c *npc) {
    return FALSE;
}

// .text 0x64
BASE_PROFILE(A_NPC_SP_JINGLE, dAcNpcSpJingle_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word; its identity is unknown.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpJingle

// .text 0x1E0
void dAcNpcSpJingle_c::onDemoCmd(int cmd, u8 member) {
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
            mJingleTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpJingle {
static const dAcNpcSpJingle_c::State_s l_states[] = {
    {&dAcNpcSpJingle_c::initIdle, &dAcNpcSpJingle_c::executeIdle},
    {&dAcNpcSpJingle_c::initLook, &dAcNpcSpJingle_c::executeLook},
    {&dAcNpcSpJingle_c::initTalk, &dAcNpcSpJingle_c::executeTalk},
    {&dAcNpcSpJingle_c::initRemote, &dAcNpcSpJingle_c::executeRemote},
    {&dAcNpcSpJingle_c::initRemoteTalk, &dAcNpcSpJingle_c::executeRemoteTalk},
};
} // namespace dANpcSpJingle

// .text 0x2FC
void dAcNpcSpJingle_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpJingle::l_states[state].mInit)();
}

// .text 0x334
int dAcNpcSpJingle_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mJingleTalk);
    return SUCCEEDED;
}

// .text 0x380
int dAcNpcSpJingle_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    mSpotTimer = 0;
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        if (dScene::getCurrentScene() == dScene::SCENE_FIELD &&
            dSaveData_c::getTown()->mNpcSpotA.getKind() == dScene::SCENE_NUM) {
            dSaveTown_c *town = dSaveData_c::getTown();
            town->mNpcSpotA.update((u8)dScene::getCurrentScene(), &mPos);
        }
        u8 kind = dSaveData_c::getTown()->mNpcSpotA.mKind;
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(
            kind - dScene::SCENE_RM_NPC0);
        if (animal != NULL && animal->mID.isValid() &&
            animal->mPlace != dAnimal_c::PLACE_OUTSIDE) {
            relocate();
        }
        if (dSaveData_c::getTown()->mNpcSpotA.getKind() ==
                dScene::getCurrentScene() &&
            isToyDay()) {
            mDaub.mAtSpot = 1;
        } else {
            mDaub.mAtSpot = 0;
        }
        setDaubData(&mDaub, sizeof(mDaub));
        if (!dSaveData_c::getTown()->mNpcSpotA.mValid) {
            dSaveTown_c *town = dSaveData_c::getTown();
            town->mNpcSpotA.set(dScene::getCurrentScene(), &mPos);
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
    mObjc.setDefWeight(0x80);
    dSceneCheck::fn_80081858(dANpcSpJingle::entryDebugPage(this));
    return SUCCEEDED;
}

// .text 0x560: when the spot was used, he moves to a new one.
int dAcNpcSpJingle_c::doDelete() {
    dANpcSpJingle::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    if (!isNetActive() && dSaveData_c::getTown()->mNpcSpotA.mFlag &&
        dSaveData_c::getTown()->mNpcSpotA.getKind() ==
            dScene::getCurrentScene()) {
        dNpcInfo::jingle_c::get()->move();
        dNpcInfo::jingle_c::get()->setSpot();
    }
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x5F8
int dAcNpcSpJingle_c::vtB0() {
    if (mSpotTimer > 0) {
        mSpotTimer--;
    }
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (isLocalOwner() && dNet::isSceneOwner()) {
        if (isToyDay()) {
            if (!mDaub.mAtSpot && dSaveData_c::getTown()->mNpcSpotA.getKind() ==
                                      dScene::getCurrentScene()) {
                dSaveTown_c *town = dSaveData_c::getTown();
                mVec3_c pos = town->mNpcSpotA.getPos();
                if (!dNpc::canPutItemAt(&pos, mNpcItem, NULL, dScene::SCENE_NUM,
                                        TRUE, TRUE)) {
                    if (mSpotTimer <= 0) {
                        dNpcInfo::jingle_c::get()->setSpot();
                        mSpotTimer = SPOT_RETRY_FRAMES;
                    }
                } else if (mViewArea.isOutsideAll(NET_MEMBER_MAX, &pos)) {
                    if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
                        mPos = mLastPos = mHomePos = pos;
                    }
                    mDaub.mAtSpot = 1;
                    setDaubData(&mDaub, sizeof(mDaub));
                }
            }
        } else if (mDaub.mAtSpot &&
                   mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
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
        (this->*dANpcSpJingle::l_states[mState].mExecute)();
    }
    if (mViewArea.isOutsideAll(dNet::getMyNo(), this)) {
        dNpcInfo::jingle_c::get()->resetEncounter();
    }
    return SUCCEEDED;
}

// .text 0x830
void dAcNpcSpJingle_c::initIdle() {
    initWander();
}

// .text 0x834
void dAcNpcSpJingle_c::executeIdle() {
    if (dSaveData_c::getTown()->mNpcSpotA.mFlag &&
        dSaveData_c::getTown()->mNpcSpotA.getKind() ==
            dScene::getCurrentScene() &&
        mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
        relocate();
    } else {
        executeWander(NULL);
    }
}

// .text 0x8D0
void dAcNpcSpJingle_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = STEP_START;
}

// .text 0x934
void dAcNpcSpJingle_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

namespace dANpcSpJingle {
static const dAcNpcSpJingle_c::Proc l_lookSteps[] = {
    &dAcNpcSpJingle_c::stepLook,
};
} // namespace dANpcSpJingle

// .text 0x994
void dAcNpcSpJingle_c::executeLook() {
    if (mStep < STEP_DONE) {
        (this->*dANpcSpJingle::l_lookSteps[mStep])();
    }
}

// .text 0x9D4
void dAcNpcSpJingle_c::initTalk() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    mStep = STEP_START;
}

// .text 0xA6C
void dAcNpcSpJingle_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        ++mStep;
    }
}

namespace dANpcSpJingle {
static const dAcNpcSpJingle_c::Proc l_talkSteps[] = {
    &dAcNpcSpJingle_c::stepTalk,
};
} // namespace dANpcSpJingle

// .text 0xAB8
void dAcNpcSpJingle_c::executeTalk() {
    if (mStep < STEP_DONE) {
        (this->*dANpcSpJingle::l_talkSteps[mStep])();
    }
}

// .text 0xAF8
void dAcNpcSpJingle_c::initRemote() {
    resetLookTimer(NULL);
}

// .text 0xB00
void dAcNpcSpJingle_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mJingleTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    executeRemoteWander(NULL);
}

// .text 0xBA0
void dAcNpcSpJingle_c::initRemoteTalk() {
}

// .text 0xBA4
void dAcNpcSpJingle_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0xBF8: around the center of his home block, else near him.
BOOL dAcNpcSpJingle_c::searchWanderPos(mVec3_c *pos) {
    mVec3_c center;
    int blockX = worldToBlock(mHomePos.x);
    int blockZ = worldToBlock(mHomePos.z);
    dFdBase_c::getBlockCenterPos(&center, blockX, blockZ);
    if (searchPosInCircle(pos, &center, &mPos, lbl_80750520 / 2,
                          64.0f + getWanderMinDist(), lbl_80750520 / 2)) {
        return TRUE;
    }
    return searchPosNear(pos, &mPos, getWanderMinDist(), getWanderMaxDist());
}

// .text 0xD00
bool dAcNpcSpJingle_c::isToyDay() {
    return dEvent::isOngoing(EVENT_TOY_DAY);
}

// .text 0xD34
void dAcNpcSpJingle_c::relocate() {
    dNpcInfo::jingle_c::get()->move();
    dNpcInfo::jingle_c::get()->setSpot();
    mDaub.mAtSpot = 0;
    setDaubData(&mDaub, sizeof(mDaub));
}

// .text 0xD88
dAcNpcSpJingle_c::talk_c::talk_c(dAcNpcSpJingle_c *npc) : mpOwner(npc) {
    init();
}

// .text 0xDE8
dAcNpcSpJingle_c::talk_c::~talk_c() {
}

// .text 0xE40
void dAcNpcSpJingle_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mMeet = MEET_NONE;
}

namespace dANpcSpJingle {
static const u16 l_limitMsgs[] = {dAcNpcSpJingle_c::talk_c::MSG_LIMIT,
                                  dAcNpcSpJingle_c::talk_c::MSG_LIMIT + 1,
                                  dAcNpcSpJingle_c::talk_c::MSG_LIMIT + 2,
                                  dAcNpcSpJingle_c::talk_c::MSG_LIMIT + 2};
static const u16 l_noMoreMsgs[] = {dAcNpcSpJingle_c::talk_c::MSG_NO_MORE,
                                   dAcNpcSpJingle_c::talk_c::MSG_NO_MORE + 1,
                                   dAcNpcSpJingle_c::talk_c::MSG_NO_MORE + 2,
                                   dAcNpcSpJingle_c::talk_c::MSG_NO_MORE + 2};
} // namespace dANpcSpJingle

// .text 0xEB0: at his new spot after a present.
void dAcNpcSpJingle_c::talk_c::startMoved() {
    if (dNpcInfo::jingle_c::get()->isGivenHere()) {
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR |
                                       dScene::SCENE_ATTR_MY_TOWN)) {
            startTown();
        } else {
            startElsewhere();
        }
        return;
    }
    mMeet = MEET_MOVED;
    dItem::Item hat = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHat;
    dItem::Item acc = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mAcc;
    if (dNpcInfo::jingle_c::get()->contains(hat, acc)) {
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR |
                                       dScene::SCENE_ATTR_MY_TOWN)) {
            if (!dNpcInfo::jingle_c::get()->isTalked()) {
                mMsgNo = MSG_FOOL_ME;
                dNpcInfo::jingle_c::get()->meet();
            } else {
                mMsgNo = dANpcSpJingle::l_limitMsgs[dNpcInfo::jingle_c::get()
                                                        ->getCount()];
                dNpcInfo::jingle_c::get()->incCount();
            }
        } else if (!dNpcInfo::jingle_c::get()->isTalked()) {
            mMsgNo = MSG_SPOTTED;
            dNpcInfo::jingle_c::get()->meet();
        } else {
            mMsgNo = dANpcSpJingle::l_noMoreMsgs[dNpcInfo::jingle_c::get()
                                                     ->getCount()];
            dNpcInfo::jingle_c::get()->incCount();
        }
    } else if (!dNpcInfo::jingle_c::get()->isGreetedNewLook()) {
        mMsgNo = MSG_NEW_LOOK;
        dNpcInfo::jingle_c::get()->meet();
        dNpcInfo::jingle_c::get()->setGreetedNewLook();
    } else {
        mMsgNo = MSG_OH_HEY;
    }
}

namespace dANpcSpJingle {
static const u16 l_limitMsgs2[] = {dAcNpcSpJingle_c::talk_c::MSG_LIMIT_2,
                                   dAcNpcSpJingle_c::talk_c::MSG_LIMIT_2 + 1,
                                   dAcNpcSpJingle_c::talk_c::MSG_LIMIT_2 + 2,
                                   dAcNpcSpJingle_c::talk_c::MSG_LIMIT_2 + 2};
} // namespace dANpcSpJingle

// .text 0x102C: in the town.
void dAcNpcSpJingle_c::talk_c::startTown() {
    mMeet = MEET_TOWN;
    dItem::Item hat = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHat;
    dItem::Item acc = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mAcc;
    if (dNpcInfo::jingle_c::get()->contains(hat, acc)) {
        if (dNpcInfo::jingle_c::get()->getCount() == 0 &&
            dNpcInfo::jingle_c::get()->isFirstGift()) {
            mMsgNo = MSG_BUSY;
        } else {
            mMsgNo = dANpcSpJingle::l_limitMsgs2[dNpcInfo::jingle_c::get()
                                                     ->getCount()];
        }
        dNpcInfo::jingle_c::get()->incCount();
    } else if (!dNpcInfo::jingle_c::get()->isRefusedNewLook()) {
        mMsgNo = MSG_NEW_LOOK;
        dNpcInfo::jingle_c::get()->setRefusedNewLook();
    } else {
        mMsgNo = MSG_SCHEDULE;
    }
}

namespace dANpcSpJingle {
static const u16 l_noMoreMsgs2[] = {dAcNpcSpJingle_c::talk_c::MSG_NO_MORE_2,
                                    dAcNpcSpJingle_c::talk_c::MSG_NO_MORE_2 + 1,
                                    dAcNpcSpJingle_c::talk_c::MSG_NO_MORE_2 + 2,
                                    dAcNpcSpJingle_c::talk_c::MSG_NO_MORE_2 +
                                        2};
} // namespace dANpcSpJingle

// .text 0x1114: in a house.
void dAcNpcSpJingle_c::talk_c::startElsewhere() {
    mMeet = MEET_ELSEWHERE;
    dItem::Item hat = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHat;
    dItem::Item acc = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mAcc;
    if (dNpcInfo::jingle_c::get()->contains(hat, acc)) {
        if (dNpcInfo::jingle_c::get()->getCount() == 0 &&
            dNpcInfo::jingle_c::get()->isFirstGift()) {
            mMsgNo = MSG_RUN_ALONG;
        } else {
            mMsgNo = dANpcSpJingle::l_noMoreMsgs2[dNpcInfo::jingle_c::get()
                                                      ->getCount()];
        }
        dNpcInfo::jingle_c::get()->incCount();
    } else if (!dNpcInfo::jingle_c::get()->isRefusedNewLook()) {
        mMsgNo = MSG_SPOTTED_2;
        dNpcInfo::jingle_c::get()->setRefusedNewLook();
    } else {
        mMsgNo = MSG_LATE;
    }
}

// .text 0x11FC
void dAcNpcSpJingle_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (!dNpcInfo::jingle_c::get()->isMet()) {
        mMsgNo = MSG_FIRST;
        dNpcInfo::jingle_c::get()->meet();
    } else if (!dNpcInfo::jingle_c::get()->isGiven()) {
        mMsgNo = MSG_AGAIN;
    } else if (!dNpcInfo::jingle_c::get()->isMoved()) {
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR |
                                       dScene::SCENE_ATTR_MY_TOWN)) {
            startTown();
        } else {
            startElsewhere();
        }
    } else {
        startMoved();
    }
}

// .text 0x12C4
void dAcNpcSpJingle_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_jingle";
    info->mCode = mMsgNo;
}

// .text 0x12DC
void dAcNpcSpJingle_c::talk_c::onMessageStart(int arg) {
}

// .text 0x12E0
void dAcNpcSpJingle_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_FIRST:
    case MSG_AGAIN:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            mpController->setNextMsgCode(MSG_QUIZ);
        } else {
            mpController->setNextMsgCode(MSG_NO_ROOM);
        }
        break;
    case MSG_NEVER_MIND:
    case MSG_OH_HEY:
        switch (mMeet) {
        case MEET_MOVED:
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                0) {
                mpController->setNextMsgCode(MSG_QUIZ_2);
            } else {
                mpController->setNextMsgCode(MSG_NO_ROOM_2);
            }
            break;
        case MEET_TOWN:
            mpController->setNextMsgCode(MSG_LOOKALIKE);
            break;
        case MEET_ELSEWHERE:
            mpController->setNextMsgCode(MSG_LOOKALIKE_2);
            break;
        }
        break;
    case MSG_GIFT_CARPET:
    case MSG_GIFT_FTR:
    case MSG_GIFT_WALL:
    case MSG_GIFT_FTR_2:
    case MSG_GIFT_CARPET_2:
    case MSG_GIFT_FTR_3:
    case MSG_GIFT_WALL_2:
    case MSG_GIFT_FTR_4:
        mOnMessageEnd = &talk_c::giveGift;
        mOnRequestEnd = &talk_c::onGiftEnd;
        mpController->setNextMsgCode(mMessageCode + 1);
        break;
    case MSG_GIFT_CARPET + 1:
    case MSG_GIFT_FTR + 1:
    case MSG_GIFT_WALL + 1:
    case MSG_GIFT_FTR_2 + 1:
    case MSG_GIFT_CARPET_2 + 1:
    case MSG_GIFT_FTR_3 + 1:
    case MSG_GIFT_WALL_2 + 1:
    case MSG_GIFT_FTR_4 + 1: {
        dItem::Item hat =
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHat;
        dItem::Item acc =
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mAcc;
        dNpcInfo::jingle_c::get()->push(hat, acc);
        break;
    }
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x1494
void dAcNpcSpJingle_c::talk_c::onAnswer(int arg) {
}

// .text 0x1498
void dAcNpcSpJingle_c::talk_c::preExecute() {
}

// .text 0x149C
void dAcNpcSpJingle_c::talk_c::onTalkEnd() {
}

// .text 0x14A0
void dAcNpcSpJingle_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1504: the present of the quiz answers (carpet, wallpaper or
// furniture).
void dAcNpcSpJingle_c::talk_c::giveGift() {
    dItem::Item gift;
    dItemSel::Range_c range(dItem::KIND_FTR, dItem::FROM_JINGLE);
    switch (mMessageCode) {
    case MSG_GIFT_CARPET:
    case MSG_GIFT_CARPET_2:
        range.mKind = dItem::KIND_CARPET;
        break;
    case MSG_GIFT_WALL:
    case MSG_GIFT_WALL_2:
        range.mKind = dItem::KIND_WALL;
        break;
    default:
        range.mKind = dItem::KIND_FTR;
        break;
    }
    dItemSel::select(&gift, 1, &range, 1, &dItemSel::sDefaultFilter, NULL, 0,
                     NULL);
    requestItemAct(&gift, 1, 0);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&gift, TRUE);
}

// .text 0x15E8
void dAcNpcSpJingle_c::talk_c::onGiftEnd() {
}

// .text 0x15EC
int dAcNpcSpJingle_c::getEarType() {
    return NPC_EAR_TYPE_JINGLE;
}
