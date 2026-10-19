#include <game/game/d_a_npc_sp_resetsan.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_shutdown_fader.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/cLib/c_math.hpp>
#include <game/snd/snd_id.hpp>

// Mr. Resetti after a reset. Functions are in target .text order.

namespace dANpcSpResetsan {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_RESETSAN, dAcNpcSpResetsan_c);

// The parameters of the unit (.bss 0x8, built by __sinit).
struct param_s {
    param_s() : _0(0), mAwayTime(120), mFadeOutFrames(210), mFadeInFrames(30) {
    }
    int _0;             // unused
    int mAwayTime;      // "SCRAM!": frames until he digs back up
    int mFadeOutFrames; // the fake reset (dShutdownFader)
    int mFadeInFrames;
};
static param_s l_param;
} // namespace dANpcSpResetsan

// .text 0x298
void dAcNpcSpResetsan_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_WAIT);
        break;
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK:
        mResetTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_REPORT:
        changeState(STATE_APPEAR);
        break;
    case DEMO_CMD_END:
        changeState(STATE_DELETE);
        break;
    case CMD_RELEASE_PERMIT:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpResetsan {
static const dAcNpcSpResetsan_c::State_s l_states[] = {
    {&dAcNpcSpResetsan_c::initIdle, &dAcNpcSpResetsan_c::executeIdle},
    {&dAcNpcSpResetsan_c::initWait, &dAcNpcSpResetsan_c::executeWait},
    {&dAcNpcSpResetsan_c::initTalk, &dAcNpcSpResetsan_c::executeTalk},
    {&dAcNpcSpResetsan_c::initAppear, &dAcNpcSpResetsan_c::executeAppear},
    {&dAcNpcSpResetsan_c::initDisappear, &dAcNpcSpResetsan_c::executeDisappear},
    {&dAcNpcSpResetsan_c::initDelete, &dAcNpcSpResetsan_c::executeDelete},
};
} // namespace dANpcSpResetsan

// .text 0x358
void dAcNpcSpResetsan_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpResetsan::l_states[state].mInit)();
}

// .text 0x390
int dAcNpcSpResetsan_c::preCreate() {
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getLastResetCount() ==
        RESET_COUNT_DON) {
        mParam = SP_NPC_RACKETSAN;
    }
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mResetTalk);
    mAnmSet.setAnm0(HMN_ANM_MOL_WAIT1);
    mAnmSet.setAnm1(HMN_ANM_MOL_WAIT1);
    mAnmSet.setAnm2(HMN_ANM_MOL_WAIT1);
    return 1;
}

// .text 0x424
int dAcNpcSpResetsan_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    if (!mMoleMdl.create("obj_mol", 0)) {
        return 0;
    }
    dNpcInfo::resetsan_c::get()->setSorryType(talk_c::SORRY_NONE);
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getLastResetCount() ==
            RESET_COUNT_MAX &&
        cM::rndInt(2) == 0) {
        if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                PRIVATE_FLAG0_RESETTI_SORRY_1)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_RESETTI_SORRY_1);
            dNpcInfo::resetsan_c::get()->setSorryType(talk_c::SORRY_1);
        } else if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                       PRIVATE_FLAG0_RESETTI_SORRY_2)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_RESETTI_SORRY_2);
            dNpcInfo::resetsan_c::get()->setSorryType(talk_c::SORRY_2);
        } else if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                       PRIVATE_FLAG0_RESETTI_SORRY_3)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_RESETTI_SORRY_3);
            dNpcInfo::resetsan_c::get()->setSorryType(talk_c::SORRY_3);
        } else {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
                PRIVATE_FLAG0_RESETTI_SORRY_1);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
                PRIVATE_FLAG0_RESETTI_SORRY_2);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
                PRIVATE_FLAG0_RESETTI_SORRY_3);
            dNpcInfo::resetsan_c::get()->setSorryType(talk_c::SORRY_4);
        }
    }
    dPlayerMgr::Mgr_c::getCurrentPlayer()->incResetCount();
    mMoleVisible = 0;
    changeState(STATE_IDLE);
    mDrawShadow = 0;
    clearDemoState();
    fgMngProc_blockDayChange();
    if (isDon()) {
        dBgm::l_mgr.mStgField.set30(1);
    } else {
        dBgm::l_mgr.mStgField.set30(0);
    }
    mLookAt.setFront(PRIORITY_DEMO, l_lookPitchStep, l_lookYawStep);
    mHideSndTimer = 0;
    return 1;
}

// .text 0x618
int dAcNpcSpResetsan_c::doDelete() {
    if (mMoleMdl.remove() == false) {
        return 0;
    }
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x66C
int dAcNpcSpResetsan_c::vtB0() {
    if (mHideSndTimer > 0 && --mHideSndTimer == 0) {
        mAudioObj.startSound(SE_reset_san_moguru);
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpResetsan::l_states[mState].mExecute)();
    }
    if (mMoleVisible) {
        makeMtx(&mMoleMtx, &mPos, mAng(0));
        mMoleMdl.mMdl.setLocalMtx(&mMoleMtx);
        mMoleMdl.playMdl();
    }
    return 1;
}

// .text 0x728
int dAcNpcSpResetsan_c::draw() {
    if (mMoleVisible && mIsActive) {
        mMoleMdl.entry();
    }
    return dAcNpc_c::draw();
}

// .text 0x778
void dAcNpcSpResetsan_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mIsActive = 0;
    dPlayerMgr::lock();
}

// .text 0x7C0: once the player stands in front, dig up beside them.
void dAcNpcSpResetsan_c::executeIdle() {
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActorRaw(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        deleteRequest();
        return;
    }
    int playerX = worldToUnit(player->mPos.x);
    int playerZ = worldToUnit(player->mPos.z);
    int unitX = worldToUnit(mPos.x);
    int unitZ = worldToUnit(mPos.z);
    if (playerZ < unitZ) {
        return;
    }
    if (playerX == unitX - 2 && playerZ == unitZ) {
        return;
    }
    if (playerX >= unitX - 1) {
        unitX -= 4;
    }
    dFdBase_c::getUnitGroundPos(&mPos, unitX, unitZ);
    dPlayerMgr::unlock();
    dDemo::requestReport(this, NULL);
}

// .text 0x894
void dAcNpcSpResetsan_c::initWait() {
    mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
}

// .text 0x8A8
void dAcNpcSpResetsan_c::executeWait() {
}

// .text 0x8AC
void dAcNpcSpResetsan_c::initTalk() {
    mResetTalk.start(
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
    mTalk.requestTalk(true, FALSE);
    mStep = 0;
}

// .text 0x900
void dAcNpcSpResetsan_c::executeTalk() {
}

// .text 0x904
void dAcNpcSpResetsan_c::initAppear() {
    mAng angle;
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getLastResetCount() ==
        RESET_COUNT_FIRST) {
        angle = 0;
    } else {
        dAcPlayer_c *player =
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (player != NULL) {
            angle = getAngleYTo(player);
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getLastResetCount() ==
            RESET_COUNT_BACK_TURNED) {
            angle = angle.mAngle + DEG_TO_ANGLE(180);
        }
    }
    mAngle.y = angle;
    mAngle3D.y = angle;
    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_MOL_APP1, 0.0f, 0.0f,
                       m3d::PLAYMODE_INHERIT, 0);
    dEffect::create("af_npc_reset_app", &mPos, NULL, NULL);
    mAudioObj.startSound(SE_reset_san_detekuru);
    dBgm::l_mgr.mStgField.play34();
    mMoleMdl.setAnmChr("obj_mol_0", m3d::FORWARD_ONCE, 0.0f);
    mMoleVisible = 1;
}

// .text 0xA24
void dAcNpcSpResetsan_c::executeAppear() {
    int action = mAction.mActionId;
    mIsActive = 1;
    if (action == ACTION_ANM && mAction.mCanChange) {
        changeState(STATE_TALK);
    }
}

// .text 0xA50
void dAcNpcSpResetsan_c::initDisappear() {
    mStep = 0;
}

// .text 0xA5C
void dAcNpcSpResetsan_c::executeDisappear() {
    if (mStep == 0) {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_MOL_GND1, 0.0f,
                           cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
        mMoleMdl.setAnmChr("obj_mol_1", m3d::FORWARD_ONCE, 0.0f);
        dEffect::create("af_npc_reset_dis", &mPos, NULL, NULL);
        mHideSndTimer = HIDE_SND_DELAY;
        mStep++;
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange &&
        !mTalk.isBusy()) {
        mMoleVisible = 0;
        dDemo::finishTalk(this);
    }
}

// .text 0xB3C
void dAcNpcSpResetsan_c::initDelete() {
}

// .text 0xB40
void dAcNpcSpResetsan_c::executeDelete() {
    dNpcInfo::resetsan_c::get()->set(0);
    deleteRequest();
}

// .text 0xB7C
void dAcNpcSpResetsan_c::turnToPlayer() {
    mAction.requestTurn(PRIORITY_DEMO, getAngleYToPlayer(_1B70),
                        mAng(1.5f * l_turnSpeed), 0, cNpcMorphFrames);
}

// .text 0xC18
void dAcNpcSpResetsan_c::startHideAnm() {
    mMoleMdl.setAnmChr("obj_mol_1", m3d::FORWARD_ONCE, 0.0f);
}

// .text 0xC34
void dAcNpcSpResetsan_c::startAppearAnm() {
    mMoleMdl.setAnmChr("obj_mol_0", m3d::FORWARD_ONCE, 0.0f);
}

// .text 0xC50
BOOL dAcNpcSpResetsan_c::isResetti() {
    return mParam == SP_NPC_RESETSAN;
}

// .text 0xC6C
BOOL dAcNpcSpResetsan_c::isDon() {
    return mParam == SP_NPC_RACKETSAN;
}

// .text 0xC88
dAcNpcSpResetsan_c::talk_c::talk_c(dAcNpcSpResetsan_c *npc) : mpOwner(npc) {
    init();
}

// .text 0xCE8
dAcNpcSpResetsan_c::talk_c::~talk_c() {
}

// .text 0xD40
void dAcNpcSpResetsan_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mNextOnRequestEnd = NULL;
    mProc = NULL;
    mTimer = 0;
    mRetry = 0;
    mSometime = 0;
    mPressCount = 0;
    mPhrase = cM::rndInt(PHRASE_NUM);
}

// .text 0xDF4
void dAcNpcSpResetsan_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    switch (dPlayerMgr::Mgr_c::getCurrentPlayer()->getLastResetCount()) {
    case 0:
        mMsgNo = MSG_LECTURE_0;
        break;
    case 1:
        mMsgNo = MSG_LECTURE_1;
        break;
    case 2:
        mMsgNo = MSG_LECTURE_2;
        break;
    case 3:
        mMsgNo = MSG_LECTURE_3;
        break;
    case 4:
        mMsgNo = MSG_LECTURE_4;
        break;
    case 5:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isDonAtCafe()) {
            mMsgNo = MSG_DON;
        } else {
            mMsgNo = MSG_DON_FIRST;
        }
        break;
    case 6:
        mMsgNo = MSG_LECTURE_6;
        break;
    case 7:
        mMsgNo = MSG_LECTURE_7;
        break;
    case 8:
        mMsgNo = MSG_LECTURE_8;
        break;
    case 9:
        mMsgNo = MSG_LECTURE_9;
        break;
    case 10:
        mMsgNo = MSG_LECTURE_10;
        break;
    }
}

// .text 0xF04
void dAcNpcSpResetsan_c::talk_c::getMsgInfo(msgInfo_s *info) {
    if (mpOwner->isDon()) {
        info->mLabel = "npc_SP/NPC_racket";
    } else {
        info->mLabel = "npc_SP/NPC_reset";
    }
    info->mCode = mMsgNo;
}

// .text 0xF78
void dAcNpcSpResetsan_c::talk_c::onMessageStart(int arg) {
    switch (arg) {
    case TAG_SLAP:
        mpOwner->mAudioObj.startSound(SE_resetsan_viblslap);
        break;
    case TAG_THUNDER:
        mpOwner->mAudioObj.startSound(SE_resetsan_kaminari);
        dPlayerMgr::flinch();
        break;
    }
    if (!mpOwner->isDon()) {
        switch (mMessageCode) {
        case MSG_PRESS_COUNT:
            formatNumber(mPressCount, 1, 7, dScript::NUM_FORMAT_REGION);
            break;
        case MSG_SAY_THIS_1:
        case MSG_SAY_THIS_2:
            mpController->setStringWord(0, mPhrase + PHRASE_FIRST,
                                        "sys_STRING/STR_Unit");
            break;
        }
    }
}

// .text 0x1070
void dAcNpcSpResetsan_c::talk_c::onMessageEnd(int kind) {
    switch (kind) {
    case TAG_SLAP:
        mpOwner->mAudioObj.startSound(SE_resetsan_viblslap);
        break;
    case TAG_THUNDER:
        mpOwner->mAudioObj.startSound(SE_resetsan_kaminari);
        dPlayerMgr::flinch();
        break;
    }
    if (!mpOwner->isDon()) {
        switch (mMessageCode) {
        case MSG_INTRO:
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_JOINED_AS_VISITOR)) {
                mpController->setNextMsgCode(MSG_THANKS);
            } else {
                mpController->setNextMsgCode(MSG_THANKS_VISITOR);
            }
            break;
        case MSG_THANKS:
        case MSG_THANKS_VISITOR:
            mOnMessageEnd = &talk_c::holdMsg;
            mOnRequestEnd = &talk_c::startTurn;
            break;
        case MSG_SORRY_ABOUT_THAT:
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_JOINED_AS_VISITOR)) {
                mpController->setNextMsgCode(MSG_RIGHT_WAY);
            } else {
                mpController->setNextMsgCode(MSG_RIGHT_WAY_VISITOR);
            }
            break;
        case MSG_SCRAM:
            mpController->setNextMsgCode(MSG_HELD_BACK);
            mOnMessageEnd = &talk_c::holdMsg;
            mOnRequestEnd = &talk_c::startDigAway;
            break;
        case MSG_SAY_THIS_1:
        case MSG_SAY_THIS_2:
            mOnMessageEnd = &talk_c::startWordInput;
            mOnRequestEnd = &talk_c::checkWord;
            break;
        case MSG_WHERE_AT:
            mOnMessageEnd = &talk_c::holdMsg;
            mOnRequestEnd = &talk_c::startTurn;
            break;
        case MSG_SEE_YA:
            mpController->setNextMsgCode(MSG_GOTCHA);
            mOnMessageEnd = &talk_c::holdMsg;
            mOnRequestEnd = &talk_c::startFakeReset;
            break;
        case MSG_I_HEAR_THAT:
            mpController->setNextMsgCode(MSG_ANTSYPANTS);
            break;
        case MSG_SOMETIME:
            mSometime = 1;
            break;
        case MSG_GOT_IT_3_YES:
            if (mSometime) {
                mpController->setNextMsgCode(MSG_MEAN_IT);
            } else {
                mpController->setNextMsgCode(MSG_LUNCH);
            }
            break;
        case MSG_NO_REGRET:
            if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_RESET_COUNT_DROPPED)) {
                mpController->setNextMsgCode(MSG_HERE_WE_GO);
            } else {
                mpController->setNextMsgCode(MSG_HERE_WE_GO_AGAIN);
            }
            break;
        case MSG_HERE_WE_GO:
        case MSG_HERE_WE_GO_AGAIN:
            mpController->setNextMsgCode(MSG_FRIEND);
            break;
        case MSG_FRIEND:
            if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_RESET_COUNT_DROPPED)) {
                mpController->setNextMsgCode(MSG_FORGET_IT);
            } else {
                mpController->setNextMsgCode(MSG_ENOUGH);
            }
            break;
        case MSG_FORGET_IT:
            mpController->setNextMsgCode(MSG_NICE_TALK);
            break;
        case MSG_ENOUGH:
            mpController->setNextMsgCode(MSG_NICE_TALK_AGAIN);
            break;
        case MSG_NICE_TALK:
            mpController->setNextMsgCode(MSG_CREEPED_OUT);
            break;
        case MSG_NICE_TALK_AGAIN:
            mpController->setNextMsgCode(MSG_CREEPED_OUT);
            break;
        case MSG_SORRY_ASK:
            if (dNpcInfo::resetsan_c::get()->mSorryType >= SORRY_1) {
                mpController->setNextMsgCode(MSG_SHOW_SORRY);
            } else {
                mpController->setNextMsgCode(MSG_LAST_TIME);
            }
            break;
        case MSG_SHOW_SORRY:
            switch (dNpcInfo::resetsan_c::get()->mSorryType) {
            case SORRY_1:
                mpController->setNextMsgCode(MSG_HOW_SORRY);
                break;
            case SORRY_2:
                mpController->setNextMsgCode(MSG_HOW_SORRY + SORRY_2);
                break;
            case SORRY_3:
                mpController->setNextMsgCode(MSG_HOW_SORRY + SORRY_3);
                break;
            case SORRY_4:
                mpController->setNextMsgCode(MSG_HOW_SORRY + SORRY_4);
                break;
            }
            break;
        case MSG_SORRY_FAKE:
        case MSG_SORRY_MIDDLE:
        case MSG_SORRY_NOT:
            switch (dNpcInfo::resetsan_c::get()->mSorryType) {
            case SORRY_1:
                mpController->setNextMsgCode(MSG_HOW_SORRY_AGAIN);
                break;
            case SORRY_2:
                mpController->setNextMsgCode(MSG_HOW_SORRY_AGAIN + SORRY_2);
                break;
            case SORRY_3:
                mpController->setNextMsgCode(MSG_HOW_SORRY_AGAIN + SORRY_3);
                break;
            case SORRY_4:
                mpController->setNextMsgCode(MSG_HOW_SORRY_AGAIN + SORRY_4);
                break;
            }
            break;
        case MSG_SORRY_BELIEVED:
            break;
        }
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x15EC
void dAcNpcSpResetsan_c::talk_c::onAnswer(int arg) {
    int answer = mpController->getAnswer();
    if (!mpOwner->isDon()) {
        switch (mMessageCode) {
        case MSG_GOT_IT_1:
            switch (answer) {
            case ANSWER_YES:
                mpController->setNextMsgCode(MSG_GOT_IT_1_YES);
                break;
            case ANSWER_NO:
                switch (mRetry) {
                case 0:
                    mpController->setNextMsgCode(MSG_GOT_IT_1_NO);
                    mRetry++;
                    break;
                case 1:
                    mpController->setNextMsgCode(MSG_GOT_IT_1_NO_AGAIN);
                    mRetry++;
                    break;
                default:
                    mpController->setNextMsgCode(MSG_GOT_IT_1_NOT_HEARD);
                    break;
                }
                break;
            }
            break;
        case MSG_GOT_IT_2:
            switch (answer) {
            case ANSWER_YES:
                mpController->setNextMsgCode(MSG_GOT_IT_2_YES);
                break;
            case ANSWER_NO:
                switch (mRetry) {
                case 0:
                    mpController->setNextMsgCode(MSG_GOT_IT_2_NO);
                    mRetry++;
                    break;
                case 1:
                    mpController->setNextMsgCode(MSG_GOT_IT_2_NO_AGAIN);
                    mRetry++;
                    break;
                default:
                    mpController->setNextMsgCode(MSG_GOT_IT_2_NOT_HEARD);
                    break;
                }
                break;
            }
            break;
        case MSG_GOT_IT_3:
            switch (answer) {
            case ANSWER_YES:
                mpController->setNextMsgCode(MSG_GOT_IT_3_YES);
                break;
            case ANSWER_NO:
                switch (mRetry) {
                case 0:
                    mpController->setNextMsgCode(MSG_GOT_IT_3_NO);
                    mRetry++;
                    break;
                case 1:
                    mpController->setNextMsgCode(MSG_GOT_IT_3_NO_AGAIN);
                    mRetry++;
                    break;
                default:
                    mpController->setNextMsgCode(MSG_GOT_IT_3_NOT_HEARD);
                    break;
                }
                break;
            }
            break;
        }
    }
}

// .text 0x1808
void dAcNpcSpResetsan_c::talk_c::preExecute() {
    if (mProc) {
        (this->*mProc)();
    }
    if (mMessageCode == MSG_CLICKING) {
        if (dPad::isConfirmTriggered()) {
            mPressCount++;
        }
        if (dPad::isCancelTriggered()) {
            mPressCount++;
        }
    }
}

// .text 0x188C
void dAcNpcSpResetsan_c::talk_c::onTalkEnd() {
    mpOwner->changeState(STATE_DISAPPEAR);
    dBgm::l_mgr.mStgField.fn_80078850();
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getLastResetCount() >=
        RESET_COUNT_DON) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_DON_MET);
    }
    fgMngProc_unblockDayChange();
}

// .text 0x18E8
void dAcNpcSpResetsan_c::talk_c::onCustomTag(int no) {
    switch (no) {
    case CUSTOM_SLAP:
        mpOwner->mAudioObj.startSound(SE_resetsan_viblslap);
        break;
    case CUSTOM_THUNDER:
        mpOwner->mAudioObj.startSound(SE_resetsan_kaminari);
        dPlayerMgr::flinch();
        break;
    }
}

// .text 0x1954
void dAcNpcSpResetsan_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
        if (mNextOnRequestEnd) {
            mOnRequestEnd = mNextOnRequestEnd;
        }
        mNextOnRequestEnd = NULL;
    }
}

// .text 0x19FC: the word input menu for the phrase.
void dAcNpcSpResetsan_c::talk_c::startWordInput() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    reqMenu1A(0);
}

// .text 0x1A54
void dAcNpcSpResetsan_c::talk_c::holdMsg() {
    setRequest1(0);
}

// .text 0x1A5C: the typed words against the phrase.
void dAcNpcSpResetsan_c::talk_c::checkWord() {
    if (!isMenuInvalid()) {
        dString::Word_c word;
        word.set((const wchar_t *)getMenuWork(), 0);
        if (word.isSame(mpController->getWord(0))) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getLastResetCount() ==
                RESET_COUNT_PHRASE) {
                mpController->setNextMsgCode(MSG_SAID_IT_1);
            } else {
                mpController->setNextMsgCode(MSG_SAID_IT_2);
            }
        } else {
            mRetry++;
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getLastResetCount() ==
                RESET_COUNT_PHRASE) {
                if (mRetry % 2) {
                    mpController->setNextMsgCode(MSG_SAID_WRONG_1);
                } else {
                    mpController->setNextMsgCode(MSG_SAID_WRONG_1_AGAIN);
                }
            } else {
                if (mRetry % 2) {
                    mpController->setNextMsgCode(MSG_SAID_WRONG_2);
                } else {
                    mpController->setNextMsgCode(MSG_SAID_WRONG_2_AGAIN);
                }
            }
        }
    }
}

// .text 0x1B98: "SCRAM!": he digs away for a while.
void dAcNpcSpResetsan_c::talk_c::startDigAway() {
    requestAct09(HMN_ANM_MOL_GND1, 0.0f, cNpcMorphFrames,
                 m3d::PLAYMODE_INHERIT);
    mpOwner->startHideAnm();
    dEffect::create("af_npc_reset_dis", &mpOwner->mPos, NULL, NULL);
    mpOwner->mHideSndTimer = HIDE_SND_DELAY;
    dBgm::l_mgr.mStgField.mute38();
    mNextOnRequestEnd = &talk_c::startAway;
}

// .text 0x1C38
void dAcNpcSpResetsan_c::talk_c::startAway() {
    mProc = &talk_c::procAway;
    mTimer = dANpcSpResetsan::l_param.mAwayTime;
    mpOwner->mIsActive = 0;
}

// .text 0x1C74
void dAcNpcSpResetsan_c::talk_c::procAway() {
    if (--mTimer <= 0) {
        mpOwner->mIsActive = 1;
        requestAct09(HMN_ANM_MOL_APP1, 0.0f, 0.0f, m3d::PLAYMODE_INHERIT);
        mpOwner->startAppearAnm();
        dEffect::create("af_npc_reset_app", &mpOwner->mPos, NULL, NULL);
        mpOwner->mAudioObj.startSound(SE_reset_san_detekuru);
        mOnRequestEnd = &talk_c::closeMsg;
        mProc = NULL;
    }
}

// .text 0x1D44
void dAcNpcSpResetsan_c::talk_c::closeMsg() {
    reqMsgClose();
    dBgm::l_mgr.mStgField.unmute38();
}

// .text 0x1D74
void dAcNpcSpResetsan_c::talk_c::startTurn() {
    mpOwner->turnToPlayer();
    mProc = &talk_c::procTurn;
}

// .text 0x1DC0 (+ a dead blr at 0x1DFC)
void dAcNpcSpResetsan_c::talk_c::procTurn() {
    if (mpOwner->mAction.mActionId == dAcNpc_c::ACTION_TURN &&
        mpOwner->mAction.mCanChange) {
        mProc = NULL;
        reqMsgClose();
    }
}

// .text 0x1E00: "SEE YA, KID!": the screen goes dark as if reset.
void dAcNpcSpResetsan_c::talk_c::startFakeReset() {
    dShutdownFader::startFadeOut(dANpcSpResetsan::l_param.mFadeOutFrames,
                                 dANpcSpResetsan::l_param.mFadeInFrames);
    dBgm::l_mgr.mStgField.mute38();
    mpOwner->mAudioObj.startSound(SE_NiseReset);
    sndFadeOut();
    mProc = &talk_c::procFakeReset;
}

// .text 0x1E8C
void dAcNpcSpResetsan_c::talk_c::procFakeReset() {
    if (dShutdownFader::isHidden()) {
        dBgm::l_mgr.mStgField.unmute38();
        sndFadeIn();
        mProc = NULL;
        reqMsgClose();
    }
}

#ifdef MUST_MATCH
// Matching-only emission order (the d_a_npc_sp_cafe / d_a_rmobj convention).
// There is no evidence for this helper in the original source. MWCC drops
// the unused function; its reference puts anmChr_c::getType before
// anmVis_c::getType among the retained weak inlines.
static void orderResetsanWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
