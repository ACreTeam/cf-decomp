#include <game/game/d_a_npc_sp_goddess.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/snd/snd_id.hpp>

// Timeline of the rise (mAppearTimer counts down from GODDESS_RISE_LEN + the
// wait) and of the sinking (mLeaveTimer, from GODDESS_SINK_LEN + the wait).
#define GODDESS_RISE_LEN 241        // the timer start (+ l_param.mAppearWait)
#define GODDESS_RISE_EFFECT 240     // the effect and SE_MegamiAppear
#define GODDESS_RISE_ANM 120        // HMN_ANM_GDS_APPEAR1; shown from here on
#define GODDESS_RISE_FADE 60        // the last frames: mTevAlpha 255 -> 0
#define GODDESS_SINK_LEN 196        // the timer start (+ l_param.mLeaveWait)
#define GODDESS_SINK_ANM 195        // HMN_ANM_GDS_DISAPPEAR1
#define GODDESS_SINK_FADE_START 165 // mTevAlpha 0 -> 255 over 60 frames
#define GODDESS_SINK_EFFECT 120     // the effect and SE_MegamiDisAppear
#define GODDESS_SINK_FADE_END 105

namespace dANpcSpGoddess {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_GODDESS, dAcNpcSpGoddess_c);

// The parameters of the unit (.bss 0x8, built by __sinit): timer lengths and
// the greeting chances.
struct dNpcGoddessParam_c {
    dNpcGoddessParam_c() {
        _00 = 0;
        mAppearWait = 60;
        mLeaveWait = 60;
        mLeaveEndWait = 60;
        mAxeQuestionRate = 40;
        mHowDoILookRate = 40;
    }

    /* 0x00 */ int _00;
    /* 0x04 */ int mAppearWait;      // added to the rise length
    /* 0x08 */ int mLeaveWait;       // added to the sink length
    /* 0x0C */ int mLeaveEndWait;    // frames after sinking until the talk ends
    /* 0x10 */ int mAxeQuestionRate; // percent: MSG_AXE_QUESTION (met before)
    /* 0x14 */ int
        mHowDoILookRate; // percent: MSG_HOW_DO_I_LOOK (else MSG_SIGH)
}; // size 0x18

static dNpcGoddessParam_c l_param;
} // namespace dANpcSpGoddess

// .text 0x1CC
void dAcNpcSpGoddess_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK:
        changeState(STATE_APPEAR);
        break;
    case DEMO_CMD_START: // nothing to do: she turns to the player herself
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_WAIT);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpGoddess {
static const dAcNpcSpGoddess_c::State_s l_states[] = {
    {&dAcNpcSpGoddess_c::initWait, &dAcNpcSpGoddess_c::executeWait},
    {&dAcNpcSpGoddess_c::initAppear, &dAcNpcSpGoddess_c::executeAppear},
    {&dAcNpcSpGoddess_c::initTalk, &dAcNpcSpGoddess_c::executeTalk}};
} // namespace dANpcSpGoddess

// .text 0x250
void dAcNpcSpGoddess_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpGoddess::l_states[state].mInit)();
}
// .text 0x288
int dAcNpcSpGoddess_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mGoddessTalk);
    return 1;
}
// .text 0x2D4
int dAcNpcSpGoddess_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mIsActive = 0;
    mShow = 0;
    mTevAlpha = 0xFF;
    clearDemoState();
    mCheckGround = 0;
    return 1;
}
// .text 0x33C
int dAcNpcSpGoddess_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}
// .text 0x368
int dAcNpcSpGoddess_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpGoddess::l_states[mState].mExecute)();
    }
    if (mShow) {
        mIsActive = 1;
    } else {
        mIsActive = 0;
    }
    return 1;
}
// .text 0x3DC
int dAcNpcSpGoddess_c::draw() {
    nw4r::g3d::ResMdl mdl = getModel()->mMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat("e");
    if (mat.IsValid()) {
        nw4r::g3d::ResMatTevColor tev = mat.GetResMatTevColor();
        if (tev.IsValid()) {
            GXColor color;
            tev.GXGetTevColor(GX_TEVPREV, &color);
            color.a = mTevAlpha;
            tev.GXSetTevColor(GX_TEVPREV, color);
            tev.DCStore(false);
        }
    }
    mat = mdl.GetResMat("b");
    if (mat.IsValid()) {
        nw4r::g3d::ResMatTevColor tev = mat.GetResMatTevColor();
        if (tev.IsValid()) {
            GXColor color;
            tev.GXGetTevColor(GX_TEVPREV, &color);
            color.a = mTevAlpha;
            tev.GXSetTevColor(GX_TEVPREV, color);
            tev.DCStore(false);
        }
    }
    return dAcNpc_c::draw();
}
// .text 0x514
void dAcNpcSpGoddess_c::initWait() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}
// .text 0x528
void dAcNpcSpGoddess_c::executeWait() {
    if (dPlayerMgr::isAxeInFountain() && !dDemo::isType(dDemo::TYPE_ANY)) {
        dDemo::requestSpeak(this, NULL);
    }
}
// .text 0x578
void dAcNpcSpGoddess_c::initAppear() {
    startAppear();
}
// .text 0x57C
void dAcNpcSpGoddess_c::startAppear() {
    dPlayerMgr::receiveGoddessItem();
    mStep = APPEAR_STEP_WAIT_DEMO;
}
// .text 0x5B0
void dAcNpcSpGoddess_c::stepAppearWaitDemo() {
    if (dPlayerMgr::isWaitingForDemo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        startRise();
    }
}
// .text 0x5F0
void dAcNpcSpGoddess_c::startRise() {
    mShow = 0;
    mTevAlpha = 0xFF;
    mAppearTimer = dANpcSpGoddess::l_param.mAppearWait + GODDESS_RISE_LEN;
    mStep = APPEAR_STEP_RISE;
    mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
}
// .text 0x630
void dAcNpcSpGoddess_c::stepAppearRise() {
    if (mAppearTimer <= 0) {
        startTurn();
    } else {
        mAppearTimer--;
    }
    if (mAppearTimer == GODDESS_RISE_EFFECT) {
        dEffect::create("af_gds_appear", &mPos, NULL, NULL);
        mAudioObj.startSound(SE_MegamiAppear);
    }
    if (mAppearTimer == GODDESS_RISE_ANM) {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_GDS_APPEAR1, 0.0f, 0.0f,
                           m3d::PLAYMODE_INHERIT, 0);
    }
    if (mAppearTimer <= GODDESS_RISE_ANM) {
        mShow = 0xFF;
    }
    if (mAppearTimer <= GODDESS_RISE_FADE - 1) {
        mTevAlpha = 255.0f * mAppearTimer / 60.0f;
    }
}
// .text 0x748
void dAcNpcSpGoddess_c::startTurn() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = APPEAR_STEP_TURN;
}
// .text 0x7AC
void dAcNpcSpGoddess_c::stepAppearTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        changeState(STATE_TALK);
    }
}
namespace dANpcSpGoddess {
static const dAcNpcSpGoddess_c::Proc l_appearSteps[] = {
    &dAcNpcSpGoddess_c::stepAppearWaitDemo, &dAcNpcSpGoddess_c::stepAppearRise,
    &dAcNpcSpGoddess_c::stepAppearTurn};
} // namespace dANpcSpGoddess
// .text 0x7D0
void dAcNpcSpGoddess_c::executeAppear() {
    if (mStep < APPEAR_STEP_NUM) {
        (this->*dANpcSpGoddess::l_appearSteps[mStep])();
    }
}
// .text 0x810
void dAcNpcSpGoddess_c::initTalk() {
    startTalk();
}
// .text 0x814
void dAcNpcSpGoddess_c::startTalk() {
    mGoddessTalk.start(
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    mStep = TALK_STEP_TALK;
}
// .text 0x898
void dAcNpcSpGoddess_c::stepTalk() {
    if (!mTalk.isBusy()) {
        startLeave();
    }
}
// .text 0x8D8
void dAcNpcSpGoddess_c::startLeave() {
    mStep = TALK_STEP_LEAVE;
    mLeaveTimer = dANpcSpGoddess::l_param.mLeaveWait + GODDESS_SINK_LEN;
    mLeaveEndTimer = dANpcSpGoddess::l_param.mLeaveEndWait;
    mShow = 0xFF;
    mTevAlpha = 0;
}
// .text 0x910
void dAcNpcSpGoddess_c::stepLeave() {
    if (mLeaveTimer <= 0) {
        if (mLeaveEndTimer > 0) {
            mLeaveEndTimer--;
        }
        if (mLeaveEndTimer <= 0) {
            dDemo::finishTalk(this);
            dPlayerMgr::canGoddessLeave();
            mStep = TALK_STEP_NUM;
        }
        mShow = 0;
    } else {
        mLeaveTimer--;
    }
    if (mLeaveTimer == GODDESS_SINK_EFFECT) {
        dEffect::create("af_gds_appear", &mPos, NULL, NULL);
        mAudioObj.startSound(SE_MegamiDisAppear);
    }
    if (mLeaveTimer == GODDESS_SINK_ANM) {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_GDS_DISAPPEAR1, 0.0f, 10.0f,
                           m3d::PLAYMODE_INHERIT, 0);
    }
    if (mLeaveTimer <= GODDESS_SINK_FADE_END) {
        mTevAlpha = 0xFF;
    } else if (mLeaveTimer <= GODDESS_SINK_FADE_START) {
        mTevAlpha = 255.0f * (GODDESS_SINK_FADE_START - mLeaveTimer) / 60.0f;
    }
}
namespace dANpcSpGoddess {
static const dAcNpcSpGoddess_c::Proc l_talkSteps[] = {
    &dAcNpcSpGoddess_c::stepTalk, &dAcNpcSpGoddess_c::stepLeave};
} // namespace dANpcSpGoddess
// .text 0xA64
void dAcNpcSpGoddess_c::executeTalk() {
    if (mStep < TALK_STEP_NUM) {
        (this->*dANpcSpGoddess::l_talkSteps[mStep])();
    }
}

// .text 0xAA4
dAcNpcSpGoddess_c::talk_c::talk_c(dAcNpcSpGoddess_c *npc) : mpOwner(npc) {
    init();
}
// .text 0xB10
dAcNpcSpGoddess_c::talk_c::~talk_c() {
}
// .text 0xB68
void dAcNpcSpGoddess_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mOnPreExecute = NULL;
    mItem = dItem::ITEM_ID_NONE;
}
// .text 0xBF4
void dAcNpcSpGoddess_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_GODDESS_MET)) {
        mMsgNo = MSG_AXE_QUESTION;
    } else {
        int rnd = cM::rndInt(100);
        if (rnd < dANpcSpGoddess::l_param.mAxeQuestionRate) {
            mMsgNo = MSG_AXE_QUESTION;
        } else if (rnd < dANpcSpGoddess::l_param.mAxeQuestionRate +
                             dANpcSpGoddess::l_param.mHowDoILookRate) {
            mMsgNo = MSG_HOW_DO_I_LOOK;
        } else {
            mMsgNo = MSG_SIGH;
        }
    }
}
// .text 0xCA8
void dAcNpcSpGoddess_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_goddess";
    info->mCode = mMsgNo;
}
// .text 0xCC0
void dAcNpcSpGoddess_c::talk_c::onMessageStart(int arg) {
}
// .text 0xCC4
void dAcNpcSpGoddess_c::talk_c::onMessageEnd(int kind) {
    switch (kind) {
    case END_GIVE_GOLD_AXE:
        mItem.setFromIndex(dItem::ITEM_IDX_GOLDEN_AXE);
        dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHeld = mItem;
        dPlayerMgr::changeHeldItemForDemo(&mItem);
        break;
    case END_GIVE_SILVER_AXE:
        mItem.setFromIndex(dItem::ITEM_IDX_SILVER_AXE);
        dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHeld = mItem;
        dPlayerMgr::changeHeldItemForDemo(&mItem);
        break;
    case END_RETURN_AXE:
        mItem = dNpcInfo::goddess_c::get()->getItem();
        dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHeld = mItem;
        dPlayerMgr::changeHeldItemForDemo(&mItem);
        break;
    case END_RETURN_AXE_ANGRY:
        mItem = dNpcInfo::goddess_c::get()->getItem();
        dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHeld = mItem;
        dPlayerMgr::changeHeldItemForDemo(&mItem);
        break;
    case END_LEAVE:
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}
// .text 0xDF8
void dAcNpcSpGoddess_c::talk_c::onAnswer(int arg) {
}
// .text 0xDFC
void dAcNpcSpGoddess_c::talk_c::preExecute() {
    if (mOnPreExecute) {
        (this->*mOnPreExecute)();
    }
}
// .text 0xE44
void dAcNpcSpGoddess_c::talk_c::onTalkEnd() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_GODDESS_MET);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_FOUNTAIN_AXE);
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHeld.getId() ==
        dItem::ITEM_ID_NONE) {
        dPlayerMgr::changeHeldItemForDemo(
            &dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mHeld);
    }
}
// .text 0xE94
void dAcNpcSpGoddess_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0xEF8
int dAcNpcSpGoddess_c::getEarType() {
    return NPC_EAR_TYPE_GODDESS;
}
