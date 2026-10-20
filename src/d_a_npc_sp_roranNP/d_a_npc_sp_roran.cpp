#include <game/game/d_a_npc_sp_roran.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_sv_player_flag.hpp>

// Saharah. Functions are in target .text order.

namespace dANpcSpRoran {
// .text 0x5C
static BOOL isOldWallpaper(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_OLD_WALLPAPER));
}

// .text 0x9C: wallpaper she takes in trade (not her old wallpaper)
static BOOL isUsedWallpaper(const dItem::Item *item) {
    if (isOldWallpaper(item)) {
        return FALSE;
    }
    return item->getKind() == dItem::KIND_WALL;
}

// .text 0xEC
static BOOL isOldFlooring(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_OLD_FLOORING));
}

// .text 0x12C
static BOOL isUsedFlooring(const dItem::Item *item) {
    if (isOldFlooring(item)) {
        return FALSE;
    }
    return item->getKind() == dItem::KIND_CARPET;
}

// .text 0x17C
BASE_PROFILE(A_NPC_SP_RORAN, dAcNpcSpRoran_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpRoran

// .text 0x2EC
void dAcNpcSpRoran_c::onDemoCmd(int cmd, u8 member) {
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
            mRoranTalk.start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpRoran {
static const dAcNpcSpRoran_c::State_s l_states[] = {
    {&dAcNpcSpRoran_c::initIdle, &dAcNpcSpRoran_c::executeIdle},
    {&dAcNpcSpRoran_c::initLook, &dAcNpcSpRoran_c::executeLook},
    {&dAcNpcSpRoran_c::initTalk, &dAcNpcSpRoran_c::executeTalk},
    {&dAcNpcSpRoran_c::initRemote, &dAcNpcSpRoran_c::executeRemote},
    {&dAcNpcSpRoran_c::initRemoteTalk, &dAcNpcSpRoran_c::executeRemoteTalk},
};
} // namespace dANpcSpRoran

// .text 0x408
void dAcNpcSpRoran_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpRoran::l_states[state].mInit)();
}

// .text 0x440
int dAcNpcSpRoran_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mRoranTalk);
    return 1;
}

// .text 0x48C
int dAcNpcSpRoran_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mObjc.setDefWeight(0x80);
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        if (!dEvent::isVisitorHere(VISITOR_SAHARAH)) {
            mDaub.mVisiting = FALSE;
        } else {
            mDaub.mVisiting = TRUE;
        }
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (mDaub.mVisiting) {
        mIsActive = TRUE;
    } else {
        mIsActive = FALSE;
    }
    return 1;
}

// .text 0x570
int dAcNpcSpRoran_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x59C
int dAcNpcSpRoran_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (isLocalOwner() && dNet::isSceneOwner() &&
        mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) { // every member
        if (dEvent::isVisitorHere(VISITOR_SAHARAH)) {
            if (!mDaub.mVisiting) {
                mDaub.mVisiting = TRUE;
                setDaubData(&mDaub, sizeof(mDaub));
            }
        } else if (mDaub.mVisiting) {
            mDaub.mVisiting = FALSE;
            setDaubData(&mDaub, sizeof(mDaub));
        }
    }
    if (mDaub.mVisiting) {
        mIsActive = TRUE;
    } else {
        mIsActive = FALSE;
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpRoran::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x6C0
void dAcNpcSpRoran_c::initIdle() {
    initWander();
}

// .text 0x6C4
void dAcNpcSpRoran_c::executeIdle() {
    executeWander(NULL);
}

// .text 0x6D8
void dAcNpcSpRoran_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
    mStep = 0;
}

// .text 0x73C
void dAcNpcSpRoran_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep++;
    }
}

namespace dANpcSpRoran {
static const dAcNpcSpRoran_c::Proc l_lookSteps[] = {&dAcNpcSpRoran_c::stepLook};
} // namespace dANpcSpRoran

// .text 0x79C
void dAcNpcSpRoran_c::executeLook() {
    if (mStep < ARRAY_SIZE(dANpcSpRoran::l_lookSteps)) {
        (this->*dANpcSpRoran::l_lookSteps[mStep])();
    }
}

// .text 0x7DC
void dAcNpcSpRoran_c::initTalk() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep = 0;
}

// .text 0x874
void dAcNpcSpRoran_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpRoran {
static const dAcNpcSpRoran_c::Proc l_talkSteps[] = {&dAcNpcSpRoran_c::stepTalk};
} // namespace dANpcSpRoran

// .text 0x8C0
void dAcNpcSpRoran_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpRoran::l_talkSteps)) {
        (this->*dANpcSpRoran::l_talkSteps[mStep])();
    }
}

// .text 0x900
void dAcNpcSpRoran_c::initRemote() {
    resetLookTimer();
}

// .text 0x908
void dAcNpcSpRoran_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mRoranTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    executeRemoteWander(NULL);
}

// .text 0x9A8
void dAcNpcSpRoran_c::initRemoteTalk() {
}

// .text 0x9AC
void dAcNpcSpRoran_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0xA00
dAcNpcSpRoran_c::talk_c::talk_c(dAcNpcSpRoran_c *npc) : mpOwner(npc) {
    init();
}

// .text 0xA6C
dAcNpcSpRoran_c::talk_c::~talk_c() {
}

// .text 0xAC4
void dAcNpcSpRoran_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mItem = dItem::Item();
    mOldNum = 0;
}

// .text 0xB3C
void dAcNpcSpRoran_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (isPlayerFromThisTown()) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_SAHARAH_ASKED)) {
            mMsgNo = MSG_OFFER;
        } else {
            mMsgNo = MSG_INTRO;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_SAHARAH_ASKED);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setAllVillagerBits();
        }
    } else {
        mMsgNo = MSG_INTRO_VISITOR;
    }
    if (dSaveData_c::getTown()->mPlayerFlag.getState() ==
        dSvPlayerFlag::STATE_FLOORING) {
        mOldNum = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
            dANpcSpRoran::isOldFlooring, NULL);
    } else if (dSaveData_c::getTown()->mPlayerFlag.getState() ==
               dSvPlayerFlag::STATE_WALLPAPER) {
        mOldNum = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
            dANpcSpRoran::isOldWallpaper, NULL);
        mMsgNo += MSG_WALLPAPER;
    }
}

// .text 0xC48
void dAcNpcSpRoran_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_rollan";
    info->mCode = mMsgNo;
}

// .text 0xC60
void dAcNpcSpRoran_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_TAKE:
    case MSG_TAKE_OLD_ONLY:
        formatNumber(mOldNum, 0, 2, dScript::NUM_FORMAT_REGION);
        break;
    case MSG_WALLPAPER_TAKE:
    case MSG_WALLPAPER_TAKE_OLD_ONLY:
        formatNumber(mOldNum, 0, 2, dScript::NUM_FORMAT_REGION);
        break;
    case MSG_GIVEN:
    case MSG_GIVEN_RARE:
    case MSG_WALLPAPER_GIVEN:
    case MSG_WALLPAPER_GIVEN_RARE:
        setItemName(mItem, 0);
        break;
    }
}

// .text 0xCC0
void dAcNpcSpRoran_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_TAKE:
    case MSG_TAKE_OLD_ONLY:
        mItem = dItem::Item(dItem::ITEM_IDX_OLD_FLOORING);
        mOnMessageEnd = &talk_c::takeOld;
        mOnRequestEnd = &talk_c::removeOld;
        mpController->setNextMsgCode(MSG_GIVE);
        break;
    case MSG_GIVE:
        if (mOldNum >= RARE_COUNT) {
            dItemSel::Range_c range(dItem::KIND_CARPET, dItem::FROM_ROLAN);
            dItemSel::select(&mItem, 1, &range, 1, &dItemSel::sDefaultFilter,
                             NULL, 0, NULL);
        } else {
            dItemSel::Range_c range(dItem::KIND_CARPET, dItem::FROM_GROUP_ABC);
            dItemSel::select(&mItem, 1, &range, 1, &dItemSel::sDefaultFilter,
                             NULL, 0, NULL);
        }
        mOnMessageEnd = &talk_c::giveNew;
        mOnRequestEnd = &talk_c::pickUpNew;
        if (mOldNum >= RARE_COUNT) {
            mpController->setNextMsgCode(MSG_GIVEN_RARE);
        } else {
            mpController->setNextMsgCode(MSG_GIVEN);
        }
        break;
    case MSG_WALLPAPER_TAKE:
    case MSG_WALLPAPER_TAKE_OLD_ONLY:
        mItem = dItem::Item(dItem::ITEM_IDX_OLD_WALLPAPER);
        mOnMessageEnd = &talk_c::takeOld;
        mOnRequestEnd = &talk_c::removeOld;
        mpController->setNextMsgCode(MSG_WALLPAPER_GIVE);
        break;
    case MSG_WALLPAPER_GIVE:
        if (mOldNum >= RARE_COUNT) {
            dItemSel::Range_c range(dItem::KIND_WALL, dItem::FROM_ROLAN);
            dItemSel::select(&mItem, 1, &range, 1, &dItemSel::sDefaultFilter,
                             NULL, 0, NULL);
        } else {
            dItemSel::Range_c range(dItem::KIND_WALL, dItem::FROM_GROUP_ABC);
            dItemSel::select(&mItem, 1, &range, 1, &dItemSel::sDefaultFilter,
                             NULL, 0, NULL);
        }
        mOnMessageEnd = &talk_c::giveNew;
        mOnRequestEnd = &talk_c::pickUpNew;
        if (mOldNum >= RARE_COUNT) {
            mpController->setNextMsgCode(MSG_WALLPAPER_GIVEN_RARE);
        } else {
            mpController->setNextMsgCode(MSG_WALLPAPER_GIVEN);
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0xFE4
void dAcNpcSpRoran_c::talk_c::onAnswer(int arg) {
    int answer = mpController->getAnswer();
    switch (mMessageCode) {
    case MSG_OFFER:
    case MSG_INTRO_VISITOR:
        switch (answer) {
        case 0: // Here you go!
            if (mOldNum > 0) {
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                        dANpcSpRoran::isUsedFlooring, NULL) > 0) {
                    mpController->setNextMsgCode(MSG_TAKE_OLD_ONLY);
                } else {
                    mpController->setNextMsgCode(MSG_TAKE);
                }
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                           dANpcSpRoran::isUsedFlooring, NULL) > 0) {
                mpController->setNextMsgCode(MSG_NONE_NEW);
            } else {
                mpController->setNextMsgCode(MSG_NONE);
            }
            break;
        }
        break;
    case MSG_WALLPAPER_OFFER:
    case MSG_WALLPAPER_INTRO_VISITOR:
        switch (answer) {
        case 0: // Here you go!
            if (mOldNum > 0) {
                if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                        dANpcSpRoran::isUsedWallpaper, NULL) > 0) {
                    mpController->setNextMsgCode(MSG_WALLPAPER_TAKE_OLD_ONLY);
                } else {
                    mpController->setNextMsgCode(MSG_WALLPAPER_TAKE);
                }
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                           dANpcSpRoran::isUsedWallpaper, NULL) > 0) {
                mpController->setNextMsgCode(MSG_WALLPAPER_NONE_NEW);
            } else {
                mpController->setNextMsgCode(MSG_WALLPAPER_NONE);
            }
            break;
        }
        break;
    }
}

// .text 0x115C
void dAcNpcSpRoran_c::talk_c::preExecute() {
}

// .text 0x1160
void dAcNpcSpRoran_c::talk_c::onTalkEnd() {
}

// .text 0x1164
void dAcNpcSpRoran_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x11C8
void dAcNpcSpRoran_c::talk_c::takeOld() {
    requestItemActEx(6, &mItem, 0, 0, 0, 2);
}

// .text 0x11E4
void dAcNpcSpRoran_c::talk_c::giveNew() {
    requestItemAct(&mItem, 0, 0);
}

// .text 0x11F4
void dAcNpcSpRoran_c::talk_c::removeOld() {
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[i].isSame(mItem)) {
            dItem::Item empty;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&empty, i, 0);
        }
    }
}

// .text 0x1288
void dAcNpcSpRoran_c::talk_c::pickUpNew() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem, FALSE);
}

// .text 0x12C0
int dAcNpcSpRoran_c::getEarType() {
    return NPC_EAR_TYPE_SAHARAH;
}
