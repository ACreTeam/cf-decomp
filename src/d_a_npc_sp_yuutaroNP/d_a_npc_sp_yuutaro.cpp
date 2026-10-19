#include <game/game/d_a_npc_sp_yuutaro.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_sv_town_info.hpp>
#include <game/snd/snd_id.hpp>

// Wisp. Functions are in target .text order.

namespace dANpcSpYuutaro {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_YUUTARO, dAcNpcSpYuutaro_c);
} // namespace dANpcSpYuutaro

// .text 0x1D0
void dAcNpcSpYuutaro_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK:
        mWispTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_GUIDE);
        clearDemoFlag8();
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0x280: not while appearing or disappearing.
bool dAcNpcSpYuutaro_c::canInteract(dDemoActor_c *actor) {
    if (mGuideStep == GUIDE_STEP_APPEAR || mGuideStep == GUIDE_STEP_DISAPPEAR) {
        return false;
    }
    return dAcNpcSp_c::canInteract(actor);
}

namespace dANpcSpYuutaro {
static const dAcNpcSpYuutaro_c::State_s l_states[] = {
    {&dAcNpcSpYuutaro_c::initGuide, &dAcNpcSpYuutaro_c::executeGuide},
    {&dAcNpcSpYuutaro_c::initLook, &dAcNpcSpYuutaro_c::executeLook},
    {&dAcNpcSpYuutaro_c::initTalk, &dAcNpcSpYuutaro_c::executeTalk}};
} // namespace dANpcSpYuutaro

// .text 0x2A4
void dAcNpcSpYuutaro_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpYuutaro::l_states[state].mInit)();
}

// .text 0x2DC
int dAcNpcSpYuutaro_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mWispTalk);
    return SUCCEEDED;
}

// .text 0x328
int dAcNpcSpYuutaro_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    if (dNet::isSessionActive() == true ||
        dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
            PRIVATE_FLAG0_TRAVELLING) == TRUE ||
        !dEvent::isVisitorHere(VISITOR_WISP) ||
        dSaveData_c::getTown()->mTownInfo.findEndEvent(EVENT_LAMP_BURIED) < 0) {
        return FAILED;
    }
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
            PRIVATE_FLAG1_WISP_FOUND)) {
        mWispTalk.mMsgKind = talk_c::MSG_KIND_LAMP;
        if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) < 96.0f) {
            mGuideStep = GUIDE_STEP_SHOWN;
        } else {
            mGuideStep = GUIDE_STEP_HIDDEN;
        }
    } else {
        mWispTalk.mMsgKind = talk_c::MSG_KIND_CALL;
        mGuideStep = GUIDE_STEP_CALL;
    }
    changeState(STATE_GUIDE);
    mFootstep.mEnable = 0;
    moveFtrToPoliceBox();
    return SUCCEEDED;
}

// .text 0x43C
int dAcNpcSpYuutaro_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x468
int dAcNpcSpYuutaro_c::vtB0() {
    if (mState < STATE_NUM) {
        (this->*dANpcSpYuutaro::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x4B0
void dAcNpcSpYuutaro_c::initGuide() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0, cNpcMorphFrames);
    if (dSaveData_c::getTown()->mTownInfo.findEndEvent(EVENT_LAMP_SAME_DAY) >=
        0) {
        deleteRequest();
        mGuideStep = GUIDE_STEP_NUM;
        return;
    }
    if (mGuideStep != GUIDE_STEP_SHOWN) {
        hide();
    } else {
        if (!mIsActive) {
            mAudioObj.startSound(SE_YuutarouAppear);
        }
        show();
    }
    if (mGuideStep == GUIDE_STEP_SHOWN || mGuideStep == GUIDE_STEP_HIDDEN) {
        mWispTalk.mMsgKind = talk_c::MSG_KIND_LAMP;
    }
}

// .text 0x5C8
void dAcNpcSpYuutaro_c::stepCall() {
    if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) < 160.0f) {
        mWispTalk.mMsgKind = talk_c::MSG_KIND_CALL;
        dDemo::requestSpeak(this, NULL);
        mGuideStep = GUIDE_STEP_LEAD;
        setDemoFlag8();
    }
}

// .text 0x630
void dAcNpcSpYuutaro_c::stepLead() {
    f32 dist = calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL);
    if (dist < 64.0f) {
        mWispTalk.mMsgKind = talk_c::MSG_KIND_ARRIVED;
        dDemo::requestSpeak(this, NULL);
        mGuideStep = GUIDE_STEP_SHOWN;
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
            PRIVATE_FLAG1_WISP_FOUND);
        clearDemoFlag8();
    } else if (dist > 224.0f) {
        mWispTalk.mMsgKind = talk_c::MSG_KIND_WRONG_WAY;
        dDemo::requestSpeak(this, NULL);
        mGuideStep = GUIDE_STEP_WRONG_WAY;
        setDemoFlag8();
    }
}

// .text 0x6DC
void dAcNpcSpYuutaro_c::stepWrongWay() {
    f32 dist = calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL);
    if (dist < 160.0f) {
        mWispTalk.mMsgKind = talk_c::MSG_KIND_RIGHT_WAY;
        dDemo::requestSpeak(this, NULL);
        mGuideStep = GUIDE_STEP_LEAD;
        setDemoFlag8();
    } else if (dist >= 480.0f) {
        mWispTalk.mMsgKind = talk_c::MSG_KIND_CALL;
        mGuideStep = GUIDE_STEP_CALL;
    }
}

// .text 0x768
void dAcNpcSpYuutaro_c::stepAppear() {
    if (!mIsActive) {
        show();
        _1B68 = 0.0f;
    } else {
        _1B68 = getBodyScale();
        if (getModel()->mAnm.checkFrame(56.0f)) {
            mAng3_c angle(0, mAngle.y.mAngle, 0);
            dEffect::create("af_gst_apear", &mPos, &angle, NULL);
        }
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mGuideStep = GUIDE_STEP_SHOWN;
        mAng angle = getAngleYToPlayer(_1B70);
        mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                            cNpcMorphFrames);
        _1B68 = 1.0f;
    }
}

// .text 0x910
void dAcNpcSpYuutaro_c::stepShown() {
    if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) > 128.0f) {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_GST_DISAPPEAR1, 0.0f, 15.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        mGuideStep = GUIDE_STEP_DISAPPEAR;
        mLampAsked = 0;
    }
}

// .text 0x990
void dAcNpcSpYuutaro_c::stepDisappear() {
    _1B68 = getBodyScale();
    if (getModel()->mAnm.checkFrame(85.0f)) {
        mAng3_c angle(0, mAngle.y.mAngle, 0);
        dEffect::create("af_gst_disappear", &mPos, &angle, NULL);
    }
    if (getModel()->mAnm.checkFrame(31.0f)) {
        mAudioObj.startSound(SE_YuutarouDisappear);
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mGuideStep = GUIDE_STEP_HIDDEN;
        hide();
    }
}

// .text 0xB1C
void dAcNpcSpYuutaro_c::stepHidden() {
    if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) < 96.0f) {
        mGuideStep = GUIDE_STEP_APPEAR;
        mAngle.y = 0;
        mAngle3D.y = 0;
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_GST_APPEAR1, 0.0f, 0.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        mAudioObj.startSound(SE_YuutarouAppear);
    }
}

namespace dANpcSpYuutaro {
static const dAcNpcSpYuutaro_c::Proc l_guideSteps[] = {
    &dAcNpcSpYuutaro_c::stepCall,     &dAcNpcSpYuutaro_c::stepLead,
    &dAcNpcSpYuutaro_c::stepWrongWay, &dAcNpcSpYuutaro_c::stepAppear,
    &dAcNpcSpYuutaro_c::stepShown,    &dAcNpcSpYuutaro_c::stepDisappear,
    &dAcNpcSpYuutaro_c::stepHidden};
} // namespace dANpcSpYuutaro

// .text 0xBB0
void dAcNpcSpYuutaro_c::executeGuide() {
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return;
    }
    if (mGuideStep != GUIDE_STEP_APPEAR && mGuideStep != GUIDE_STEP_DISAPPEAR) {
        mAction.setTargetAngle(
            getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT));
    }
    if (mGuideStep < GUIDE_STEP_NUM) {
        (this->*dANpcSpYuutaro::l_guideSteps[mGuideStep])();
    }
}

// .text 0xC44
void dAcNpcSpYuutaro_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = LOOK_STEP_TURN;
}

// .text 0xCA8
void dAcNpcSpYuutaro_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

namespace dANpcSpYuutaro {
static const dAcNpcSpYuutaro_c::Proc l_lookSteps[] = {
    &dAcNpcSpYuutaro_c::stepLook};
} // namespace dANpcSpYuutaro

// .text 0xD08
void dAcNpcSpYuutaro_c::executeLook() {
    if (mStep < LOOK_STEP_NUM) {
        (this->*dANpcSpYuutaro::l_lookSteps[mStep])();
    }
}

// .text 0xD48
void dAcNpcSpYuutaro_c::initTalk() {
    if (mWispTalk.mMsgKind != talk_c::MSG_KIND_ARRIVED) {
        dActor_c *target = mWispTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        BOOL noCamera = FALSE;
        switch (mWispTalk.mMsgKind) {
        case talk_c::MSG_KIND_CALL:
        case talk_c::MSG_KIND_WRONG_WAY:
        case talk_c::MSG_KIND_RIGHT_WAY:
            noCamera = TRUE;
            setDemoFlag8();
            break;
        }
        turnTalk(angle, noCamera);
        mStep = TALK_STEP_TALK;
    } else {
        mAngle.y = 0;
        mAngle3D.y = 0;
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_GST_APPEAR1, 0.0f, 0.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        mStep = TALK_STEP_APPEAR;
        mAudioObj.startSound(SE_YuutarouAppear);
    }
}

// .text 0xE48
void dAcNpcSpYuutaro_c::stepTalkAppear() {
    if (!mIsActive) {
        show();
        _1B68 = 0.0f;
    } else {
        _1B68 = getBodyScale();
        if (getModel()->mAnm.checkFrame(56.0f)) {
            mAng3_c angle(0, mAngle.y.mAngle, 0);
            dEffect::create("af_gst_apear", &mPos, &angle, NULL);
        }
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        turnTalk(getTalkAngle(), FALSE);
        mStep = TALK_STEP_TALK;
        _1B68 = 1.0f;
    }
}

// .text 0xFF8
void dAcNpcSpYuutaro_c::stepTalk() {
    if (!mTalk.isBusy()) {
        if (mWispTalk.mLampTaken) {
            mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_GST_DISAPPEAR1, 0.0f,
                               15.0f, m3d::PLAYMODE_INHERIT, 0);
            mStep = TALK_STEP_DISAPPEAR;
        } else {
            dDemo::finishTalk(this);
            mStep = TALK_STEP_NUM;
        }
    }
}

// .text 0x1080
void dAcNpcSpYuutaro_c::stepTalkDisappear() {
    _1B68 = getBodyScale();
    if (getModel()->mAnm.checkFrame(85.0f)) {
        mAng3_c angle(0, mAngle.y.mAngle, 0);
        dEffect::create("af_gst_disappear", &mPos, &angle, NULL);
    }
    if (getModel()->mAnm.checkFrame(31.0f)) {
        mAudioObj.startSound(SE_YuutarouDisappear);
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mStep = TALK_STEP_NUM;
        hide();
    }
}

namespace dANpcSpYuutaro {
static const dAcNpcSpYuutaro_c::Proc l_talkSteps[] = {
    &dAcNpcSpYuutaro_c::stepTalkAppear, &dAcNpcSpYuutaro_c::stepTalk,
    &dAcNpcSpYuutaro_c::stepTalkDisappear};
} // namespace dANpcSpYuutaro

// .text 0x1214
void dAcNpcSpYuutaro_c::executeTalk() {
    if (mStep < TALK_STEP_NUM) {
        (this->*dANpcSpYuutaro::l_talkSteps[mStep])();
    }
}

// .text 0x1254
dAcNpcSpYuutaro_c::talk_c::talk_c(dAcNpcSpYuutaro_c *npc) : mpOwner(npc) {
    init();
}

// .text 0x12B4
dAcNpcSpYuutaro_c::talk_c::~talk_c() {
}

// .text 0x130C
void dAcNpcSpYuutaro_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageStart = NULL;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mLampTaken = 0;
}

namespace dANpcSpYuutaro {
// .text 0x13A8: pocket filter, Wisp's (empty) lamp.
static BOOL isEmptyLamp(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_EMPTY_LAMP));
}
} // namespace dANpcSpYuutaro

// .text 0x13E8
void dAcNpcSpYuutaro_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    switch (mMsgKind) {
    case MSG_KIND_CALL:
        mMsgNo = MSG_CALL;
        break;
    case MSG_KIND_WRONG_WAY:
        mMsgNo = MSG_WRONG_WAY;
        break;
    case MSG_KIND_RIGHT_WAY:
        mMsgNo = MSG_RIGHT_WAY;
        break;
    case MSG_KIND_ARRIVED:
        if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                PRIVATE_FLAG0_WISP_MET)) {
            mMsgNo = MSG_RELIEF;
        } else {
            mMsgNo = MSG_MEET_AGAIN;
        }
        break;
    case MSG_KIND_LAMP: {
        u16 mask;
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                dANpcSpYuutaro::isEmptyLamp, &mask) > 0) {
            mMsgNo = MSG_RIGHT_WAY_4; // overwritten below
            int slot = dSaveData_c::getTown()->mTownInfo.findEndEvent(
                EVENT_LAMP_BURIED);
            dSaveTown_c *town = dSaveData_c::getTown();
            town->mTownInfo.mEndEvents[slot] = TOWN_INFO_END_EVENT_NONE;
            dSaveData_c::getTown()->mTownInfo.addEndEvent(EVENT_LAMP_SAME_DAY);
            for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
                if (mask & (1 << i)) {
                    dItem::Item lamp(dItem::ITEM_IDX_MAGIC_LAMP);
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&lamp, i,
                                                                     FALSE);
                    mLampTaken = 1;
                    dMD_c *date = &dSaveData_c::getTown()->mTownInfo.mLampDate;
                    dTime_c *now = dTime_c::getCurrent();
                    u8 day = now->mday;
                    u8 month = now->month;
                    date->month = month;
                    date->day = day;
                    break;
                }
            }
            if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                    PRIVATE_FLAG0_WISP_LAMP_RETURNED)) {
                mMsgNo = MSG_LAMP_FOUND;
            } else {
                mMsgNo = MSG_LAMP_FOUND_AGAIN;
            }
        } else if (!mpOwner->mLampAsked) {
            mpOwner->mLampAsked = 1;
            if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                    PRIVATE_FLAG0_WISP_LAMP_LOST_TOLD)) {
                mMsgNo = MSG_ASK_LAMP;
            } else {
                mMsgNo = MSG_ASK_LAMP_AGAIN;
            }
        } else {
            mMsgNo = MSG_FIND_LAMP;
        }
        break;
    }
    }
}

// .text 0x1604
void dAcNpcSpYuutaro_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_yutarou";
    info->mCode = mMsgNo;
    setWispName();
}

// .text 0x161C
void dAcNpcSpYuutaro_c::talk_c::onMessageStart(int arg) {
    if (mOnMessageStart) {
        (this->*mOnMessageStart)();
        mOnMessageStart = NULL;
    }
}

// .text 0x1680
void dAcNpcSpYuutaro_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_RELIEF:
        mpController->setNextMsgCode(MSG_INTRO);
        break;
    case MSG_INTRO:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_WISP_MET);
        break;
    case MSG_LAMP_GONE:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_WISP_LAMP_LOST_TOLD);
        break;
    case MSG_LAMP_FOUND:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_WISP_LAMP_RETURNED);
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
    }
}

// .text 0x1738
void dAcNpcSpYuutaro_c::talk_c::onAnswer(int arg) {
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x179C
void dAcNpcSpYuutaro_c::talk_c::preExecute() {
}

// .text 0x17A0
void dAcNpcSpYuutaro_c::talk_c::onTalkEnd() {
}

// .text 0x17A4
void dAcNpcSpYuutaro_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1808: the speaker name: "?????" until he is met.
void dAcNpcSpYuutaro_c::talk_c::setWispName() {
    dString::Word_c name;
    switch (mMsgKind) {
    case MSG_KIND_CALL:
    case MSG_KIND_WRONG_WAY:
    case MSG_KIND_RIGHT_WAY:
        dString::loadString(&name, dNpcInfo::SP_NAME_UNKNOWN,
                            "sys_STRING/STR_SPNpc_name");
        setSpeakerName(
            (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(), 2);
        break;
    case MSG_KIND_ARRIVED:
    case MSG_KIND_LAMP:
        dString::loadString(&name, dNpcInfo::SP_NAME_WISP,
                            "sys_STRING/STR_SPNpc_name");
        setSpeakerName(
            (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(), 2);
        break;
    }
}

// .text 0x18DC
int dAcNpcSpYuutaro_c::getEarType() {
    return 0; // the first ear parameter preset (no NPC_EAR_TYPE name yet)
}
