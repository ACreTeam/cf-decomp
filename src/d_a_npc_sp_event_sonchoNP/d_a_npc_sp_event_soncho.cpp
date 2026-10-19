#include <game/game/d_a_npc_sp_event_soncho.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_time_stamp.hpp>
#include <game/cLib/c_math.hpp>

// Tortimer at the town events. Functions are in target .text order.

namespace dANpcSpEventSoncho {
// .text 0x5C: countPockets filters.
static int isKnifeAndFork(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_KNIFE_AND_FORK;
}
// .text 0x88
static int isSparkler(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_SPARKLER));
}
// .text 0xC8
static int isRomanCandle(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_ROMAN_CANDLE));
}

// .text 0x108..0x2344
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO_CD, dAcNpcSpEvSonchoCd_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_07C,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_07C);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO_NY, dAcNpcSpEvSonchoNy_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_07D,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_07D);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO_HF, dAcNpcSpEvSonchoHf_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_07E,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_07E);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO_HT, dAcNpcSpEvSonchoHt_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_07F,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_07F);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO01, dAcNpcSpEvSoncho01_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_080,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_080);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO02, dAcNpcSpEvSoncho02_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_081,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_081);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO03, dAcNpcSpEvSoncho03_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_082,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_082);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO04, dAcNpcSpEvSoncho04_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_083,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_083);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO05, dAcNpcSpEvSoncho05_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_084,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_084);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO06, dAcNpcSpEvSoncho06_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_085,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_085);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO07, dAcNpcSpEvSoncho07_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_086,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_086);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO08, dAcNpcSpEvSoncho08_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_087,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_087);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO09, dAcNpcSpEvSoncho09_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_088,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_088);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO10, dAcNpcSpEvSoncho10_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_089,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_089);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO11, dAcNpcSpEvSoncho11_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_08A,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_08A);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO12, dAcNpcSpEvSoncho12_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_08B,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_08B);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO13, dAcNpcSpEvSoncho13_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_08C,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_08C);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO14, dAcNpcSpEvSoncho14_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_08D,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_08D);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO15, dAcNpcSpEvSoncho15_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_08E,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_08E);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO16, dAcNpcSpEvSoncho16_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_08F,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_08F);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO17, dAcNpcSpEvSoncho17_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_090,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_090);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO18, dAcNpcSpEvSoncho18_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_091,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_091);
CUSTOM_BASE_PROFILE(NPC_SP_EV_SONCHO19, dAcNpcSpEvSoncho19_c,
                    fProf::A_NPC_SP_EVENT_SONCHO_092,
                    fProf::DRAW_ORDER::A_NPC_SP_EVENT_SONCHO_092);

// Tuning (.bss 0x8, built by __sinit).
struct param_s {
    param_s()
        : _0(0), mPopperRate(50), mPresentRate(50), mCountdownMin(55),
          mDropRange(4) {
    }
    int _0;            // unused
    int mPopperRate;   // % chance of a party popper before the countdown
    int mPresentRate;  // % chance of a firework before the show
    int mCountdownMin; // 23:mm: the countdown is close
    int mDropRange;    // units searched for dropped knives and forks
};
static param_s l_param;
} // namespace dANpcSpEventSoncho

// .text 0x2344
void dAcNpcSpEvSoncho_c::onDemoCmd(int cmd, u8 member) {
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
            getTalk()->start(dPlayerMgr::getNpcTargetActor(
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

namespace dANpcSpEventSoncho {
static const dAcNpcSpEvSoncho_c::State_s l_states[] = {
    {&dAcNpcSpEvSoncho_c::initIdle, &dAcNpcSpEvSoncho_c::executeIdle},
    {&dAcNpcSpEvSoncho_c::initLook, &dAcNpcSpEvSoncho_c::executeLook},
    {&dAcNpcSpEvSoncho_c::initTalk, &dAcNpcSpEvSoncho_c::executeTalk},
    {&dAcNpcSpEvSoncho_c::initRemote, &dAcNpcSpEvSoncho_c::executeRemote},
    {&dAcNpcSpEvSoncho_c::initRemoteTalk,
     &dAcNpcSpEvSoncho_c::executeRemoteTalk},
};
} // namespace dANpcSpEventSoncho

// .text 0x246C
void dAcNpcSpEvSoncho_c::changeState(int state) {
    if (resetWanderDaub(&mDaub.mWander)) {
        setDaubData(&mDaub, sizeof(mDaub));
    }
    mState = state;
    (this->*dANpcSpEventSoncho::l_states[state].mInit)();
}

// .text 0x24E0
int dAcNpcSpEvSoncho_c::preCreate() {
    return dAcNpcSp_c::preCreate() != 0;
}

// .text 0x250C
int dAcNpcSpEvSonchoSpecialEvent_c::preCreate() {
    return dAcNpcSpEvSoncho_c::preCreate() != 0;
}

// .text 0x2538
int dAcNpcSpEvSonchoCd_c::preCreate() {
    if (!dAcNpcSpEvSonchoSpecialEvent_c::preCreate()) {
        return 0;
    }
    setRecept(&mCdTalk);
    return 1;
}

// .text 0x2584
int dAcNpcSpEvSonchoNy_c::preCreate() {
    if (!dAcNpcSpEvSonchoSpecialEvent_c::preCreate()) {
        return 0;
    }
    setRecept(&mNyTalk);
    return 1;
}

// .text 0x25D0
int dAcNpcSpEvSonchoHf_c::preCreate() {
    if (!dAcNpcSpEvSonchoSpecialEvent_c::preCreate()) {
        return 0;
    }
    setRecept(&mHfTalk);
    return 1;
}

// .text 0x261C
int dAcNpcSpEvSonchoHt_c::preCreate() {
    if (!dAcNpcSpEvSonchoSpecialEvent_c::preCreate()) {
        return 0;
    }
    setRecept(&mHtTalk);
    return 1;
}

// .text 0x2668
int dAcNpcSpEvSonchoNormalEvent_c::preCreate() {
    if (!dAcNpcSpEvSoncho_c::preCreate()) {
        return 0;
    }
    setRecept(&mNormalTalk);
    return 1;
}

// .text 0x26B4
int dAcNpcSpEvSoncho_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mStep[STATE_IDLE] = 0;
    mStep[STATE_LOOK] = 0;
    mStep[STATE_TALK] = 0;
    mStep[STATE_REMOTE] = 0;
    mStep[STATE_REMOTE_TALK] = 0;
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        if (!isEventTime()) {
            mDaub.mActive = 0;
        } else {
            mDaub.mActive = 1;
        }
        mDaub.mWander.mEmotion = 0;
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (mDaub.mActive) {
        mIsActive = 1;
    } else {
        mIsActive = 0;
    }
    if (getEvent() != EVENT_COUNTDOWN) {
        mObjc.setDefWeight(0x80);
    } else {
        moveFtrToPoliceBox();
    }
    return 1;
}

// .text 0x27F0
int dAcNpcSpEvSoncho_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x281C
int dAcNpcSpEvSoncho_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (isLocalOwner() && dNet::isSceneOwner() &&
        mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) { // every member
        if (isEventTime()) {
            if (!mDaub.mActive) {
                mDaub.mActive = 1;
                setDaubData(&mDaub, sizeof(mDaub));
            }
        } else {
            if (mDaub.mActive) {
                mDaub.mActive = 0;
                setDaubData(&mDaub, sizeof(mDaub));
            }
        }
    }
    if (mDaub.mActive) {
        mIsActive = 1;
    } else {
        mIsActive = 0;
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpEventSoncho::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x294C
int dAcNpcSpEvSonchoHf_c::vtB0() {
    if (mViewArea.isOutsideAll(dNet::getMyNo(), this)) {
        dNpcInfo::soncho_c::get()->resetHfLostAsked();
    }
    return dAcNpcSpEvSoncho_c::vtB0();
}

// .text 0x29A0
void dAcNpcSpEvSoncho_c::initIdle() {
    if (getEvent() == EVENT_COUNTDOWN) {
        mAction.requestTurn(PRIORITY_IDLE, mAng(0), l_turnSpeed, 0);
    } else {
        initWander();
    }
}

// .text 0x2A18
void dAcNpcSpEvSoncho_c::executeIdle() {
    if (getEvent() == EVENT_COUNTDOWN) {
        if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
            mAction.requestWait(PRIORITY_IDLE);
        }
    } else if (executeWander(&mDaub.mWander)) {
        setDaubData(&mDaub, sizeof(mDaub));
    }
}

// .text 0x2AB8
void dAcNpcSpEvSoncho_c::initLook() {
    mAction.requestTurn(PRIORITY_DEMO, getAngleYToPlayer(_1B70), l_turnSpeed,
                        0);
    mStep[STATE_LOOK] = LOOK_STEP_TURN;
}

// .text 0x2B1C
void dAcNpcSpEvSoncho_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep[STATE_LOOK]++;
    }
}

namespace dANpcSpEventSoncho {
static const dAcNpcSpEvSoncho_c::Proc l_lookSteps[] = {
    &dAcNpcSpEvSoncho_c::stepLook};
} // namespace dANpcSpEventSoncho

// .text 0x2B7C
void dAcNpcSpEvSoncho_c::executeLook() {
    if (mStep[STATE_LOOK] < ARRAY_SIZE(dANpcSpEventSoncho::l_lookSteps)) {
        (this->*dANpcSpEventSoncho::l_lookSteps[mStep[STATE_LOOK]])();
    }
}

// .text 0x2BBC
void dAcNpcSpEvSoncho_c::initTalk() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep[STATE_TALK] = TALK_STEP_TURN;
}

// .text 0x2C58
void dAcNpcSpEvSoncho_c::stepTalkTurn() {
    if (!mTalk.isBusy()) {
        if (getEvent() == EVENT_COUNTDOWN) {
            mAction.requestTurn(PRIORITY_DEMO, mAng(0), l_turnSpeed, 0);
            mStep[STATE_TALK] = TALK_STEP_TURN_END;
        } else {
            dDemo::finishTalk(this);
            mStep[STATE_TALK] = TALK_STEP_DONE;
        }
    }
}

// .text 0x2CF4
void dAcNpcSpEvSoncho_c::stepTalkTurnEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mStep[STATE_TALK]++;
    }
}

namespace dANpcSpEventSoncho {
static const dAcNpcSpEvSoncho_c::Proc l_talkSteps[] = {
    &dAcNpcSpEvSoncho_c::stepTalkTurn, &dAcNpcSpEvSoncho_c::stepTalkTurnEnd};
} // namespace dANpcSpEventSoncho

// .text 0x2D44
void dAcNpcSpEvSoncho_c::executeTalk() {
    if (mStep[STATE_TALK] < ARRAY_SIZE(dANpcSpEventSoncho::l_talkSteps)) {
        (this->*dANpcSpEventSoncho::l_talkSteps[mStep[STATE_TALK]])();
    }
}

// .text 0x2D84
void dAcNpcSpEvSoncho_c::initRemote() {
    resetLookTimer();
}

// .text 0x2D8C
void dAcNpcSpEvSoncho_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            getTalk()->start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    executeRemoteWander(&mDaub.mWander);
}

// .text 0x2E38
void dAcNpcSpEvSoncho_c::initRemoteTalk() {
}

// .text 0x2E3C
void dAcNpcSpEvSoncho_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0x2E90
bool dAcNpcSpEvSoncho_c::isEventOngoing() {
    return dEvent::isOngoing(getEvent());
}

// .text 0x2ED4
bool dAcNpcSpEvSoncho_c::isEventNotStarted() {
    return dEvent::isNotStarted(getEvent());
}

// .text 0x2F18
bool dAcNpcSpEvSoncho_c::isEventOver() {
    return dEvent::isOver(getEvent());
}

// .text 0x2F5C: from the start until an hour after the end.
BOOL dAcNpcSpEvSoncho_c::isEventTime() {
    return isEventOngoing() || dEvent::isOverWithin(getEvent(), 1, 0);
}

// .text 0x2FDC: the whole day until an hour after the end.
BOOL dAcNpcSpEvSonchoHt_c::isEventTime() {
    BOOL today = FALSE;
    BOOL ret = TRUE;
    if (isEventNotStarted() || isEventOngoing()) {
        today = TRUE;
    }
    if (!today) {
        if (!dEvent::isOverWithin(getEvent(), 1, 0)) {
            ret = FALSE;
        }
    }
    return ret;
}

// .text 0x3084: the whole day.
BOOL dAcNpcSpEvSonchoCd_c::isEventTime() {
    return dEvent::isActive(getEvent()) != 0;
}

// .text 0x30C8: the whole day until three hours after the end.
BOOL dAcNpcSpEvSonchoHf_c::isEventTime() {
    BOOL today = FALSE;
    BOOL ret = TRUE;
    if (isEventNotStarted() || isEventOngoing()) {
        today = TRUE;
    }
    if (!today) {
        if (!dEvent::isOverWithin(getEvent(), 3, 0)) {
            ret = FALSE;
        }
    }
    return ret;
}

// .text 0x3170
dItem::Item dAcNpcSpEvSoncho_c::getPresent() {
    dItem::Item present;
    if (dNpcInfo::soncho_c::get()->getPresent()->mId == dItem::ITEM_ID_NONE) {
        dItemSel::Range_c range(dItem::KIND_NONE, getPresentFrom());
        dItemSel::select(&present, 1, &range, 1, &dItemSel::sDefaultFilter,
                         NULL, 0, NULL);
        dNpcInfo::soncho_c::get()->setPresent(present);
    } else {
        present = *dNpcInfo::soncho_c::get()->getPresent();
    }
    return present;
}

// .text 0x322C
dItem::Item dAcNpcSpEvSonchoHt_c::getPresent() {
    BOOL hasSparkler = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                           dANpcSpEventSoncho::isSparkler, NULL) > 0;
    BOOL hasCandle = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                         dANpcSpEventSoncho::isRomanCandle, NULL) > 0;
    if (hasSparkler && hasCandle) {
        return dItem::Item();
    }
    if (hasSparkler) {
        return dItem::Item(dItem::ITEM_IDX_ROMAN_CANDLE);
    }
    if (hasCandle) {
        return dItem::Item(dItem::ITEM_IDX_SPARKLER);
    }
    dItem::Item present;
    dItemSel::Range_c range(dItem::KIND_NONE, getPresentFrom());
    dItemSel::select(&present, 1, &range, 1, &dItemSel::sDefaultFilter, NULL, 0,
                     NULL);
    dNpcInfo::soncho_c::get()->setPresent(present);
    return present;
}

// .text 0x3360
dItem::Item dAcNpcSpEvSonchoNy_c::getPresent() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dTime_c now = *dTime_c::getCurrent();
    int year = now.year - TIME_YEAR_MIN; // one shirt per year from 2000
    if (year < 0) {
        year = 0;
    } else if (year > dItem::ITEM_IDX_NEW_YEARS_SHIRT_35 -
                          dItem::ITEM_IDX_NEW_YEARS_SHIRT_00) {
        year = dItem::ITEM_IDX_NEW_YEARS_SHIRT_35 -
               dItem::ITEM_IDX_NEW_YEARS_SHIRT_00;
    }
    return dItem::Item(dItem::ITEM_IDX_NEW_YEARS_SHIRT_00, year, TRUE);
}

// .text 0x3414
dItem::Item dAcNpcSpEvSonchoHf_c::getPresent() {
    return dItem::Item(dItem::ITEM_IDX_KNIFE_AND_FORK);
}

// .text 0x341C: a wander target near pos.
BOOL dAcNpcSpEvSoncho_c::adjustWanderPos(mVec3_c *pos) {
    mVec3_c target = *pos;
    if (mPos.xzDistTo(target) < 8.0f) {
        return FALSE;
    }
    if (mPos.xzDistTo(target) > 0.5f * lbl_80750520) {
        return searchPosInCircle(pos, &mPos, &mPos, 0.5f * lbl_80750520,
                                 getWanderMinDist(), 0.5f * lbl_80750520);
    }
    mVec3_c freePos;
    freePos = calcFreePosToward(&target);
    if (mPos.xzDistTo(freePos) >= 8.0f) {
        *pos = freePos;
        return TRUE;
    }
    if (searchPosInCircle(pos, &target, &mPos, 0.5f * lbl_80750520,
                          getWanderMinDist(), 0.5f * lbl_80750520)) {
        return TRUE;
    }
    return searchPosNear(pos, &mPos, getWanderMinDist(), getWanderMaxDist());
}

// .text 0x363C: near the town hall.
BOOL dAcNpcSpEvSonchoNormalEvent_c::searchWanderPos(mVec3_c *pos) {
    dItem::Item hall((u16)BUILDING_TOWN_HALL);
    int unitX;
    int unitZ;
    if (!dSaveBuildingList_c::get()->getPos(&unitX, &unitZ, hall, 1)) {
        return FALSE;
    }
    unitX = unitX - 1 - cM::rndInt(3);
    unitZ += 6;
    dFdBase_c::getUnitGroundPos(pos, unitX, unitZ);
    if (dNpc::getPlayerOnUnit(unitX, unitZ)) {
        return FALSE;
    }
    return adjustWanderPos(pos);
}

// .text 0x36F8
BOOL dAcNpcSpEvSonchoHt_c::searchWanderPos(mVec3_c *pos) {
    dItem::Item hall((u16)BUILDING_TOWN_HALL);
    int unitX;
    int unitZ;
    if (!dSaveBuildingList_c::get()->getPos(&unitX, &unitZ, hall, 1)) {
        return FALSE;
    }
    unitX -= cM::rndInt(2);
    unitZ = unitZ + cM::rndInt(2) + 4;
    dFdBase_c::getUnitGroundPos(pos, unitX, unitZ);
    if (dNpc::getPlayerOnUnit(unitX, unitZ)) {
        return FALSE;
    }
    return adjustWanderPos(pos);
}

// .text 0x37C0
BOOL dAcNpcSpEvSonchoNy_c::searchWanderPos(mVec3_c *pos) {
    dItem::Item hall((u16)BUILDING_TOWN_HALL);
    int unitX;
    int unitZ;
    if (!dSaveBuildingList_c::get()->getPos(&unitX, &unitZ, hall, 1)) {
        return FALSE;
    }
    unitX -= cM::rndInt(2);
    unitZ = unitZ + cM::rndInt(2) + 4;
    dFdBase_c::getUnitGroundPos(pos, unitX, unitZ);
    if (dNpc::getPlayerOnUnit(unitX, unitZ)) {
        return FALSE;
    }
    return adjustWanderPos(pos);
}

// .text 0x3888
BOOL dAcNpcSpEvSonchoHf_c::searchWanderPos(mVec3_c *pos) {
    dItem::Item hall((u16)BUILDING_TOWN_HALL);
    int unitX;
    int unitZ;
    if (!dSaveBuildingList_c::get()->getPos(&unitX, &unitZ, hall, 1)) {
        return FALSE;
    }
    if (cM::rndInt(2)) {
        unitX += 2;
    }
    unitZ += 6;
    dFdBase_c::getUnitGroundPos(pos, unitX, unitZ);
    if (dNpc::getPlayerOnUnit(unitX, unitZ)) {
        return FALSE;
    }
    return adjustWanderPos(pos);
}

// .text 0x394C: he stays home.
BOOL dAcNpcSpEvSonchoCd_c::searchWanderPos(mVec3_c *pos) {
    *pos = mHomePos;
    return FALSE;
}

// .text 0x396C
int dAcNpcSpEvSoncho_c::vtE4() {
    return 0;
}

// .text 0x3974
dAcNpcSpEvSoncho_c::talk_c::talk_c(dAcNpcSpEvSoncho_c *npc) : mpOwner(npc) {
    init();
}

// .text 0x39D4
dAcNpcSpEvSonchoNormalEvent_c::talkNormal_c::talkNormal_c(
    dAcNpcSpEvSoncho_c *npc)
    : talk_c(npc) {
}

// .text 0x3A10
dAcNpcSpEvSonchoSpecialEvent_c::talkSpecial_c::talkSpecial_c(
    dAcNpcSpEvSoncho_c *npc)
    : talk_c(npc) {
}

// .text 0x3A4C
dAcNpcSpEvSoncho_c::talk_c::~talk_c() {
}

// .text 0x3AA4
dAcNpcSpEvSonchoNormalEvent_c::talkNormal_c::~talkNormal_c() {
}

// .text 0x3AFC
dAcNpcSpEvSonchoSpecialEvent_c::talkSpecial_c::~talkSpecial_c() {
}

// .text 0x3B54
void dAcNpcSpEvSoncho_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
}

// .text 0x3BB4
BOOL dAcNpcSpEvSonchoNy_c::talkNewyear_c::isWearingNewYearShirt() {
    dItem::Item shirt =
        dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mShirt;
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(shirt);
    BOOL ret = bitm != NULL;
    if (ret) {
        ret = dItem::clampField(bitm->m_from, dItem::FROM_COUNT,
                                dItem::FROM_NONE) == dItem::FROM_NEWYEAR;
    }
    return ret;
}

// .text 0x3C20
BOOL dAcNpcSpEvSonchoNy_c::talkNewyear_c::isWearingPresent() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dAcNpcSpEvSoncho_c *owner = mpOwner;
    dItem::Item shirt = player->mEquipment.mShirt;
    return shirt.isSame(owner->getPresent());
}

// .text 0x3C78
int dAcNpcSpEvSonchoNy_c::talkNewyear_c::getMsgCode() {
    if (mpOwner->isEventOver()) {
        return MSG_OVER;
    }
    if (dNpcInfo::soncho_c::get()->isTalked()) {
        if (dNpcInfo::soncho_c::get()->isGiven()) {
            if (isWearingNewYearShirt()) {
                return isWearingPresent() ? (int)MSG_WEARING_PRESENT
                                          : (int)MSG_WEARING_OTHER_YEAR;
            }
            return MSG_TALKED;
        }
        return MSG_GOALS_AGAIN;
    }
    return dAcNpc_c::isPlayerFromThisTown() ? (int)MSG_GREETING
                                            : (int)MSG_VISITOR_GREETING;
}

// .text 0x3D28
int dAcNpcSpEvSonchoCd_c::talkCountdown_c::getMsgCode() {
    BOOL notStarted = mpOwner->isEventNotStarted();
    BOOL ongoing = mpOwner->isEventOngoing();
    BOOL over = mpOwner->isEventOver();
    if (!dNpcInfo::soncho_c::get()->testTalked()) {
        return dAcNpc_c::isPlayerFromThisTown() ? (int)MSG_GREETING
                                                : (int)MSG_VISITOR_GREETING;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (mpOwner->isEventOver() ||
        (mpOwner->isEventOngoing() && now.month != MONTH_DECEMBER)) {
        if (!dNpcInfo::soncho_c::get()->testTalkedAfter()) {
            return MSG_HAPPY_NEW_YEAR;
        }
        return mpOwner->isEventOver() ? (int)MSG_TALKED_OVER
                                      : (int)MSG_TALKED_AFTER;
    }
    if (now.hour < 23 || now.min < dANpcSpEventSoncho::l_param.mCountdownMin) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0 &&
            cM::rndInt(100) < dANpcSpEventSoncho::l_param.mPopperRate &&
            !dNpcInfo::soncho_c::get()->isPopperGiven()) {
            dNpcInfo::soncho_c::get()->setPopperGiven();
            return MSG_POPPER;
        }
        dNpcInfo::soncho_c::get()->resetPopperGiven();
        if (!dNpcInfo::soncho_c::get()->testFullTold()) {
            if (now.hour < 12) {
                return MSG_MORNING;
            }
            if (now.hour < 18) {
                return MSG_AFTERNOON;
            }
            if (now.hour < 23) {
                return MSG_EVENING;
            }
            if (now.min < 30) {
                return dAcNpc_c::isPlayerFromThisTown() ? (int)MSG_LATE
                                                        : (int)MSG_LATE_VISITOR;
            }
            return dAcNpc_c::isPlayerFromThisTown() ? (int)MSG_LATER
                                                    : (int)MSG_LATER_VISITOR;
        }
        if (now.hour < 12) {
            return MSG_TALKED_MORNING;
        }
        if (now.hour < 18) {
            return MSG_TALKED_AFTERNOON;
        }
        if (now.hour < 23) {
            return MSG_TALKED_EVENING;
        }
        return now.min < 30 ? (int)MSG_TALKED_LATE : (int)MSG_TALKED_LATER;
    }
    if (dNpcInfo::soncho_c::get()->testFullTold()) {
        return now.min < 59 ? (int)MSG_TALKED_LAST_MINUTES
                            : (int)MSG_TALKED_LAST_MINUTE;
    }
    if (now.min < 59) {
        return dAcNpc_c::isPlayerFromThisTown() ? (int)MSG_LAST_MINUTES
                                                : (int)MSG_LAST_MINUTES_VISITOR;
    }
    return dAcNpc_c::isPlayerFromThisTown() ? (int)MSG_LAST_MINUTE
                                            : (int)MSG_LAST_MINUTE_VISITOR;
}

// .text 0x3FF4
int dAcNpcSpEvSonchoHf_c::talkHarvestFestival_c::getMsgCode() {
    if (mpOwner->isEventOngoing()) {
        if (!dNpcInfo::soncho_c::get()->isHfGreeted()) {
            return dAcNpc_c::isPlayerFromThisTown() ? (int)MSG_GREETING
                                                    : (int)MSG_GREETING;
        }
        if (!dNpcInfo::soncho_c::get()->isTalked()) {
            return MSG_TALK;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                dANpcSpEventSoncho::isKnifeAndFork, NULL) > 0) {
            return dNpcInfo::soncho_c::get()->isHfLostAsked()
                       ? (int)MSG_CARRYING_LOST
                       : (int)MSG_CARRYING;
        }
        if (mAnimalIdx >= ANIMAL_NUM) {
            if (dNpcInfo::soncho_c::get()->isHfLostAsked()) {
                return MSG_LOST_AGAIN;
            }
            dNpcInfo::soncho_c::get()->setHfLostAsked();
            return MSG_NO_VILLAGER;
        }
        return dNpcInfo::soncho_c::get()->isFullTold()
                   ? (int)MSG_HAVING_FUN_AGAIN
                   : (int)MSG_HAVING_FUN;
    }
    if (mpOwner->isEventNotStarted()) {
        if (!dNpcInfo::soncho_c::get()->testHfTalkedBeforeStart()) {
            return dAcNpc_c::isPlayerFromThisTown()
                       ? (int)MSG_BEFORE_GREETING
                       : (int)MSG_BEFORE_VISITOR_GREETING;
        }
        return MSG_BEFORE_TALKED;
    }
    return dNpcInfo::soncho_c::get()->testHfTalkedAfter()
               ? (int)MSG_AFTER_TALKED
               : (int)MSG_AFTER;
}

// .text 0x4140
int dAcNpcSpEvSonchoHt_c::talkHanabi_c::getMsgCode() {
    if (mpOwner->isEventOver()) {
        if (!dNpcInfo::soncho_c::get()->testTalkedAfter()) {
            dTime_c today = *dEvent::getToday();
            int show = (today.mday - 1) / 7 + 1;
            int shows = dTime_c::getNthWeekday(today.year, MONTH_AUGUST,
                                               WEEKDAY_SUNDAY, 5) >= 0
                            ? 5
                            : 4;
            if (show == 1) {
                return MSG_OVER_FIRST;
            }
            if (show == 2) {
                return MSG_OVER_SECOND;
            }
            if (show == shows) {
                return MSG_OVER_LAST;
            }
            return show == shows - 1 ? (int)MSG_OVER_NEXT_LAST : (int)MSG_OVER;
        }
        return MSG_TALKED_OVER;
    }
    if (!dNpcInfo::soncho_c::get()->testTalked()) {
        return dAcNpc_c::isPlayerFromThisTown() ? (int)MSG_GREETING
                                                : (int)MSG_VISITOR_GREETING;
    }
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0 &&
        cM::rndInt(100) < dANpcSpEventSoncho::l_param.mPresentRate &&
        dNpcInfo::soncho_c::get()->getPresent()->mId != dItem::ITEM_ID_NONE) {
        return MSG_PRESENT;
    }
    dTime_c start;
    dTime_c end;
    dEvent::getStartTime(EVENT_FIREWORKS, &start);
    dEvent::getEndTime(EVENT_FIREWORKS, &end);
    if (mpOwner->isEventOngoing()) {
        dTimeStamp_c hourAgo(*dTime_c::getCurrent());
        hourAgo.add(0, -1, 0, 0);
        dTimeStamp_c hourLater(*dTime_c::getCurrent());
        hourLater.add(0, 1, 0, 0);
        if (!dTime_c::isSameOrAfter(hourAgo.get(), start)) {
            return MSG_STARTED;
        }
        return dTime_c::isSameOrAfter(hourLater.get(), end) ? (int)MSG_ENDING
                                                            : (int)MSG_SHOW;
    }
    dTimeStamp_c soon(*dTime_c::getCurrent());
    soon.add(0, 0, 30, 0);
    return !dTime_c::isSameOrAfter(soon.get(), start) ? (int)MSG_BEFORE
                                                      : (int)MSG_BEFORE_SOON;
}

// .text 0x4600
int dAcNpcSpEvSonchoSpecialEvent_c::talkSpecial_c::getMsgCode() {
    return 0;
}

// .text 0x4608
int dAcNpcSpEvSonchoNormalEvent_c::talkNormal_c::getMsgCode() {
    if (!mpOwner->isEventOngoing()) {
        return (u16)(getEventMsgBase() + MSG_OVER);
    }
    if (dNpcInfo::soncho_c::get()->isTalked()) {
        if (dNpcInfo::soncho_c::get()->isGiven()) {
            return (u16)(getEventMsgBase() + MSG_TALKED);
        }
        return (u16)(getEventMsgBase() + MSG_ABOUT_AGAIN);
    }
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return (u16)(getEventMsgBase() + MSG_GREETING);
    }
    return (u16)(getEventMsgBase() + MSG_VISITOR_GREETING);
}

// .text 0x4708
void dAcNpcSpEvSoncho_c::talk_c::start(dActor_c *target) {
    init();
    setPresent(mpOwner->getPresent());
    mpActActor = target;
    onStart();
    mMsgNo = getMsgCode();
}

// .text 0x47A0
void dAcNpcSpEvSonchoCd_c::talkCountdown_c::getMsgInfo(msgInfo_s *info) {
    static char label[] = "npc_SP/NPC_soncho_2";
    info->mLabel = label;
    info->mCode = mMsgNo;
}

// .text 0x47B8
void dAcNpcSpEvSonchoNy_c::talkNewyear_c::getMsgInfo(msgInfo_s *info) {
    static char label[] = "npc_SP/NPC_soncho_2";
    info->mLabel = label;
    info->mCode = mMsgNo;
}

// .text 0x47D0
void dAcNpcSpEvSonchoHf_c::talkHarvestFestival_c::getMsgInfo(msgInfo_s *info) {
    static char label[] = "npc_SP/NPC_soncho_3";
    info->mLabel = label;
    info->mCode = mMsgNo;
}

// .text 0x47E8
void dAcNpcSpEvSonchoHt_c::talkHanabi_c::getMsgInfo(msgInfo_s *info) {
    static char label[] = "npc_SP/NPC_soncho_4";
    info->mLabel = label;
    info->mCode = mMsgNo;
}

// .text 0x4800
void dAcNpcSpEvSonchoNormalEvent_c::talkNormal_c::getMsgInfo(msgInfo_s *info) {
    static char label[] = "npc_SP/NPC_soncho_1";
    info->mLabel = label;
    info->mCode = mMsgNo;
}

// .text 0x4818
void dAcNpcSpEvSonchoHf_c::talkHarvestFestival_c::onStart() {
    mAnimalIdx = dSaveData_c::getAnimalBlock()->pickHarvestFestivalAnimal();
}

// .text 0x4854
void dAcNpcSpEvSoncho_c::talk_c::onMessageStart(int arg) {
    if (dNpcInfo::soncho_c::get()->getPresent()->mId != dItem::ITEM_ID_NONE) {
        setItemName(dNpcInfo::soncho_c::get()->getPresent(), 0);
    }
}

// .text 0x48A8
void dAcNpcSpEvSonchoHf_c::talkHarvestFestival_c::onMessageStart(int arg) {
    if (mAnimalIdx < ANIMAL_NUM) {
        dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
        setAnmPersonalName(&block->getAnimal(mAnimalIdx)->mID, 1);
    }
    talk_c::onMessageStart(arg);
}

// .text 0x491C
void dAcNpcSpEvSonchoNy_c::talkNewyear_c::selectNextMsg() {
    switch (mMessageCode) {
    case MSG_GREETING:
    case MSG_VISITOR_GREETING:
        mpController->setNextMsgCode(MSG_NEW_YEAR);
        break;
    case MSG_NEW_YEAR:
        mpController->setNextMsgCode(MSG_GOALS);
        break;
    case MSG_GOALS:
    case MSG_GOALS_AGAIN:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            mpController->setNextMsgCode(MSG_GIVE);
        } else if (dNpcInfo::soncho_c::get()->isTalked()) {
            mpController->setNextMsgCode(MSG_POCKETS_FULL_AGAIN);
        } else {
            mpController->setNextMsgCode(MSG_POCKETS_FULL);
        }
        break;
    case MSG_GIVE:
        mpController->setNextMsgCode(MSG_GIVEN);
        setGivePresent();
        break;
    case MSG_GIVEN:
        dNpcInfo::soncho_c::get()->setGiven();
        break;
    }
}

// .text 0x4A2C
void dAcNpcSpEvSonchoCd_c::talkCountdown_c::selectNextMsg() {
    switch (mMessageCode) {
    case MSG_GREETING:
    case MSG_VISITOR_GREETING:
        getController()->setNextMsgCode(getMsgCode());
        break;
    case MSG_POPPER_INTRO_1:
    case MSG_POPPER_INTRO_2:
    case MSG_POPPER_INTRO_3:
    case MSG_POPPER_INTRO_4:
    case MSG_POPPER_INTRO_5:
        setGivePresent();
        mpController->setNextMsgCode(MSG_POPPER_GIVE);
        break;
    }
}

// .text 0x4ADC
void dAcNpcSpEvSonchoHf_c::talkHarvestFestival_c::selectNextMsg() {
    switch (mMessageCode) {
    case MSG_GREETING:
        dNpcInfo::soncho_c::get()->setHfGreeted();
        break;
    case MSG_FEAST:
    case MSG_TALK:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            mpController->setNextMsgCode(MSG_GIVE);
        } else if (dNpcInfo::soncho_c::get()->testHfFullTold()) {
            mpController->setNextMsgCode(MSG_POCKETS_FULL_AGAIN);
        } else {
            mpController->setNextMsgCode(MSG_POCKETS_FULL);
        }
        break;
    case MSG_GIVE:
        setGivePresent();
        mpController->setNextMsgCode(MSG_GIVEN);
        break;
    case MSG_GIVEN:
        dNpcInfo::soncho_c::get()->setTalked();
        break;
    case MSG_NO_GUEST:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            mpController->setNextMsgCode(MSG_DELIVER);
        } else {
            mpController->setNextMsgCode(MSG_DELIVER_POCKETS_FULL);
        }
        break;
    case MSG_DELIVER:
        setGivePresent();
        if (dAcNpc_c::isPlayerFromThisTown()) {
            mpController->setNextMsgCode(MSG_DELIVER_GIVEN);
        } else {
            mpController->setNextMsgCode(MSG_DELIVER_GIVEN_VISITOR);
        }
        break;
    case MSG_DELIVER_MORE:
        setGivePresent();
        if (dAcNpc_c::isPlayerFromThisTown()) {
            mpController->setNextMsgCode(MSG_DELIVER_MORE_GIVEN);
        } else {
            mpController->setNextMsgCode(MSG_DELIVER_MORE_GIVEN_VISITOR);
        }
        break;
    case MSG_DELIVER_GIVEN:
    case MSG_DELIVER_GIVEN_VISITOR:
    case MSG_DELIVER_MORE_GIVEN:
    case MSG_DELIVER_MORE_GIVEN_VISITOR:
        dSaveData_c::getTown()->mAnimals.mTown.setHarvestFestivalDone(
            mAnimalIdx);
        dNpcInfo::soncho_c::get()->setFullTold();
        break;
    case MSG_WAIT_GUEST:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            mpController->setNextMsgCode(MSG_DELIVER_MORE);
        } else {
            mpController->setNextMsgCode(MSG_DELIVER_MORE_POCKETS_FULL);
        }
        break;
    case MSG_LOST:
    case MSG_LOST_AGAIN:
        if (isDroppedNear()) {
            if (dNpcInfo::soncho_c::get()->testTalkedAfter()) {
                mpController->setNextMsgCode(MSG_DROPPED_AGAIN);
            } else {
                mpController->setNextMsgCode(MSG_DROPPED);
            }
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                   0) {
            mpController->setNextMsgCode(MSG_REGIVE);
        } else {
            mpController->setNextMsgCode(MSG_REGIVE_POCKETS_FULL);
        }
        break;
    case MSG_REGIVE:
        setGivePresent();
        if (dAcNpc_c::isPlayerFromThisTown()) {
            if (dNpcInfo::soncho_c::get()->testPopperGiven()) {
                mpController->setNextMsgCode(MSG_REGIVEN_AGAIN);
            } else {
                mpController->setNextMsgCode(MSG_REGIVEN);
            }
        } else if (dNpcInfo::soncho_c::get()->testHfRegiveTold()) {
            mpController->setNextMsgCode(MSG_REGIVEN_VISITOR_AGAIN);
        } else {
            mpController->setNextMsgCode(MSG_REGIVEN_VISITOR);
        }
        break;
    }
}

// .text 0x4E48
void dAcNpcSpEvSonchoHt_c::talkHanabi_c::selectNextMsg() {
    switch (mMessageCode) {
    case MSG_GREETING:
    case MSG_VISITOR_GREETING: {
        dTime_c today = *dEvent::getToday();
        int show = (today.mday - 1) / 7 + 1;
        int shows = dTime_c::getNthWeekday(today.year, MONTH_AUGUST,
                                           WEEKDAY_SUNDAY, 5) >= 0
                        ? 5
                        : 4;
        if (show == 1) {
            mpController->setNextMsgCode(MSG_FIRST_SHOW);
        } else if (show == shows) {
            mpController->setNextMsgCode(MSG_LAST_SHOW);
        } else {
            mpController->setNextMsgCode(MSG_TODAY_SHOW);
        }
        break;
    }
    case MSG_GIVE:
    case MSG_GIVE_2:
        setGivePresent();
        mpController->setNextMsgCode(MSG_PRESENT_GIVE);
        break;
    }
}

// .text 0x4FC0
void dAcNpcSpEvSonchoSpecialEvent_c::talkSpecial_c::selectNextMsg() {
}

// .text 0x4FC4
void dAcNpcSpEvSonchoNormalEvent_c::talkNormal_c::selectNextMsg() {
    int code = mMessageCode;
    switch (code) {
    case MSG_BASE_SETSUBUN + MSG_ABOUT:
    case MSG_BASE_SETSUBUN + MSG_ABOUT_AGAIN:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            mpController->setNextMsgCode(MSG_BASE_SETSUBUN + MSG_EXTRA_1);
        } else if (dNpcInfo::soncho_c::get()->isFullTold()) {
            mpController->setNextMsgCode(MSG_BASE_SETSUBUN +
                                         MSG_POCKETS_FULL_AGAIN);
        } else {
            mpController->setNextMsgCode(MSG_BASE_SETSUBUN + MSG_POCKETS_FULL);
            dNpcInfo::soncho_c::get()->setFullTold();
        }
        break;
    case MSG_BASE_SETSUBUN + MSG_EXTRA_1:     // red or blue (onAnswerSelect)
    case MSG_BASE_NAUGHTY_OR_NICE + MSG_GIVE: // naughty or nice
    case MSG_BASE_APRIL_FOOLS + MSG_EXTRA_2:
        break;
    case MSG_BASE_NAUGHTY_OR_NICE + MSG_EXTRA_1: // nice
    case MSG_BASE_NAUGHTY_OR_NICE + MSG_EXTRA_2: // naughty
        mpController->setNextMsgCode(MSG_BASE_NAUGHTY_OR_NICE + MSG_GIVEN);
        setGivePresent();
        dNpcInfo::soncho_c::get()->setGiven();
        break;
    case MSG_BASE_APRIL_FOOLS + MSG_GIVE: // "...a special present today!"
        mpController->setNextMsgCode(MSG_BASE_APRIL_FOOLS + MSG_GIVEN);
        setPresent(dItem::Item());
        setGivePresent();
        break;
    case MSG_BASE_APRIL_FOOLS + MSG_GIVEN: // "April Fool's!"
        mpController->setNextMsgCode(MSG_BASE_APRIL_FOOLS + MSG_EXTRA_1);
        break;
    case MSG_BASE_APRIL_FOOLS + MSG_EXTRA_1: // "JUST KIDDING!"
        mpController->setNextMsgCode(MSG_BASE_APRIL_FOOLS + MSG_EXTRA_2);
        setPresent(mpOwner->getPresent());
        setGivePresent();
        dNpcInfo::soncho_c::get()->setGiven();
        break;
    default:
        switch (code % MSG_PER_EVENT) {
        case 0:
            break;
        case MSG_GREETING:
        case MSG_VISITOR_GREETING:
            if (dEvent::getActiveOfKind(EVENT_KIND_TOWN) < EVENT_NUM) {
                mpController->setNextMsgCode(getEventMsgBase() +
                                             MSG_TODAY_IS_ALSO);
            } else {
                mpController->setNextMsgCode(getEventMsgBase() + MSG_TODAY_IS);
            }
            break;
        case MSG_TODAY_IS:
        case MSG_TODAY_IS_ALSO:
            mpController->setNextMsgCode(getEventMsgBase() + MSG_ABOUT);
            break;
        case MSG_ABOUT:
        case MSG_ABOUT_AGAIN:
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                0) {
                mpController->setNextMsgCode(getEventMsgBase() + MSG_GIVE);
            } else if (dNpcInfo::soncho_c::get()->isFullTold()) {
                mpController->setNextMsgCode(getEventMsgBase() +
                                             MSG_POCKETS_FULL_AGAIN);
            } else {
                mpController->setNextMsgCode(getEventMsgBase() +
                                             MSG_POCKETS_FULL);
                dNpcInfo::soncho_c::get()->setFullTold();
            }
            break;
        case MSG_GIVE:
            mpController->setNextMsgCode(getEventMsgBase() + MSG_GIVEN);
            setGivePresent();
            dNpcInfo::soncho_c::get()->setGiven();
            break;
        }
        break;
    }
}

// .text 0x53F0
void dAcNpcSpEvSoncho_c::talk_c::onMessageEnd(int kind) {
    selectNextMsg();
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x5464
void dAcNpcSpEvSoncho_c::talk_c::setPresent(dItem::Item item) {
    dNpcInfo::soncho_c::get()->setPresent(item);
}

// .text 0x5498: Setsubun: the red or the blue ogre mask.
void dAcNpcSpEvSonchoNormalEvent_c::talkNormal_c::onAnswerSelect() {
    int answer = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_BASE_SETSUBUN + MSG_EXTRA_1: // "...red or blue?"
        switch (answer) {
        case ANSWER_RED:
            setPresent(dItem::Item(dItem::ITEM_IDX_RED_OGRE_MASK));
            break;
        case ANSWER_BLUE:
        default:
            setPresent(dItem::Item(dItem::ITEM_IDX_BLUE_OGRE_MASK));
            break;
        }
        mpController->setNextMsgCode(MSG_BASE_SETSUBUN + MSG_GIVE);
        break;
    }
}

// .text 0x5530
void dAcNpcSpEvSoncho_c::talk_c::onAnswer(int arg) {
    onAnswerSelect();
}

// .text 0x5540
void dAcNpcSpEvSoncho_c::talk_c::preExecute() {
}

// .text 0x5544
void dAcNpcSpEvSoncho_c::talk_c::onTalkEnd() {
    dNpcInfo::soncho_c::get()->setTalked();
}

// .text 0x5568
void dAcNpcSpEvSonchoHf_c::talkHarvestFestival_c::onTalkEnd() {
}

// .text 0x556C
void dAcNpcSpEvSoncho_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x55D0
void dAcNpcSpEvSoncho_c::talk_c::giveItem() {
    requestItemAct(dNpcInfo::soncho_c::get()->getPresent(), 0, 0);
}

// .text 0x5614
void dAcNpcSpEvSoncho_c::talk_c::pickUpPresent() {
    dItem::Item *present = dNpcInfo::soncho_c::get()->getPresent();
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(present, 0);
}

// .text 0x5654: a knife and fork dropped near him.
BOOL dAcNpcSpEvSonchoHf_c::talkHarvestFestival_c::isDroppedNear() {
    mVec3_c pos = mpOwner->getPos();
    int unitX = worldToUnit(pos.x);
    int unitZ = worldToUnit(pos.z);
    dItem::Item knifeAndFork(dItem::ITEM_IDX_KNIFE_AND_FORK);
    for (int dx = -dANpcSpEventSoncho::l_param.mDropRange;
         dx <= dANpcSpEventSoncho::l_param.mDropRange; dx++) {
        for (int dz = -dANpcSpEventSoncho::l_param.mDropRange;
             dz <= dANpcSpEventSoncho::l_param.mDropRange; dz++) {
            dItem::Item *item =
                getCurrentField()->getItem(unitX + dx, unitZ + dz, 0);
            if (item && item->isSame(knifeAndFork) &&
                !getCurrentField()->isBuried(unitX + dx, unitZ + dz)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// .text 0x5760
BOOL dAcNpcSpEvSoncho_c::isWanderWalk() {
    BOOL walk = FALSE;
    if (mAction.mActionId == ACTION_WAIT) {
        int chance = dNpcInfo::getWalkChance();
        if (cM::rndInt(100) < chance) {
            walk = TRUE;
        }
    }
    return walk;
}
