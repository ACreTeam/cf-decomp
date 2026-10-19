#include <game/game/d_a_npc_sp_honma.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_item_name.hpp>
#include <game/game/d_model_room.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_msg_comp.hpp>

// Lyle of the Happy Room Academy. Functions are in target .text order.

namespace dANpcSpHonma {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_HONMA, dAcNpcSpHonma_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word; its identity is unknown.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpHonma

// .text 0x1CC
void dAcNpcSpHonma_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mHonmaTalk.start(
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

namespace dANpcSpHonma {
static const dAcNpcSpHonma_c::State_s l_states[] = {
    {&dAcNpcSpHonma_c::initIdle, &dAcNpcSpHonma_c::executeIdle},
    {&dAcNpcSpHonma_c::initFollow, &dAcNpcSpHonma_c::executeFollow},
    {&dAcNpcSpHonma_c::initLook, &dAcNpcSpHonma_c::executeLook},
    {&dAcNpcSpHonma_c::initTalk, &dAcNpcSpHonma_c::executeTalk},
};
} // namespace dANpcSpHonma

// .text 0x274
void dAcNpcSpHonma_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpHonma::l_states[state].mInit)();
}

// .text 0x2AC
int dAcNpcSpHonma_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mHonmaTalk);
    return SUCCEEDED;
}

// .text 0x2F8: active in the Academy's office hours.
int dAcNpcSpHonma_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour >= HRA_OPEN_HOUR && now.hour < HRA_CLOSE_HOUR) {
        mIsActive = TRUE;
    } else {
        mIsActive = FALSE;
    }
    mObjc.setDefWeight(0x80);
    dHR::updateRemarks();
    return SUCCEEDED;
}

// .text 0x3C0
int dAcNpcSpHonma_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x3EC
int dAcNpcSpHonma_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpHonma::l_states[mState].mExecute)();
    }
    return SUCCEEDED;
}

// .text 0x434
void dAcNpcSpHonma_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x448
void dAcNpcSpHonma_c::executeIdle() {
    if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) > 96.0f) {
        changeState(STATE_FOLLOW);
    }
}

// .text 0x498
void dAcNpcSpHonma_c::initFollow() {
    mAction.requestWalk(
        PRIORITY_IDLE,
        *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT),
        l_defaultAnmRate, l_walkTurnSpeed, 0, l_moveParamWalk, cNpcMorphFrames);
}

// .text 0x4FC
void dAcNpcSpHonma_c::executeFollow() {
    if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) < 64.0f) {
        changeState(STATE_IDLE);
    } else {
        mAction.setTargetPos(
            *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
    }
}

// .text 0x564
void dAcNpcSpHonma_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = STEP_START;
}

// .text 0x5C8
void dAcNpcSpHonma_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

namespace dANpcSpHonma {
static const dAcNpcSpHonma_c::Proc l_lookSteps[] = {
    &dAcNpcSpHonma_c::stepLook,
};
} // namespace dANpcSpHonma

// .text 0x628
void dAcNpcSpHonma_c::executeLook() {
    if (mStep < STEP_DONE) {
        (this->*dANpcSpHonma::l_lookSteps[mStep])();
    }
}

// .text 0x668
void dAcNpcSpHonma_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    mStep = STEP_START;
}

// .text 0x6D4
void dAcNpcSpHonma_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        ++mStep;
    }
}

namespace dANpcSpHonma {
static const dAcNpcSpHonma_c::Proc l_talkSteps[] = {
    &dAcNpcSpHonma_c::stepTalk,
};
} // namespace dANpcSpHonma

// .text 0x720
void dAcNpcSpHonma_c::executeTalk() {
    if (mStep < STEP_DONE) {
        (this->*dANpcSpHonma::l_talkSteps[mStep])();
    }
}

// .text 0x760
dAcNpcSpHonma_c::talk_c::talk_c(dAcNpcSpHonma_c *npc) : mpOwner(npc) {
    init();
}

// .text 0x7C0
dAcNpcSpHonma_c::talk_c::~talk_c() {
}

// .text 0x818
void dAcNpcSpHonma_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mRemark = REMARK_NONE;
}

// .text 0x884: the introduction, the request to join, or the questions.
void dAcNpcSpHonma_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_LYLE_MET)) {
        mMsgNo = MSG_FIRST;
    } else if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_HRA_RATED)) {
        if (dNpcInfo::kappei_c::get()->mLyleTalked == FALSE) {
            mMsgNo = MSG_ASK_AGAIN;
        } else {
            mMsgNo = MSG_ASK_AGAIN_SULK;
        }
    } else if (dNpcInfo::kappei_c::get()->mLyleTalked == FALSE) {
        mMsgNo = MSG_WELCOME;
    } else {
        mMsgNo = MSG_QUESTION;
    }
}

// .text 0x94C
void dAcNpcSpHonma_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_honma";
    info->mCode = mMsgNo;
}

namespace dANpcSpHonma {
// Inline helpers of the talk (the argument is loaded before the nested call;
// see notes/TUs/d_a_npc_sp_honma.txt).
// The words of one remark of the rating (word 0 its series, word 1 its item).
static inline void setRemarkWords(dMsg::Comp_c *ctrl,
                                  dHR::remark_c dHR::remarks_c::*remark) {
    dHR::remarks_c *remarks = dHR::getRemarks();
    (remarks->*remark).setWords(ctrl);
}
// Word 0: the name of a furniture series.
static inline void setSeriesWord(dMsg::Rcpt_c *rcpt, u32 series) {
    dItem::nameSeries_c name;
    setFurnitureName(&name, series);
    rcpt->getController()->setWord(dAcNpcSpHonma_c::talk_c::WORD_SERIES, &name);
}
// Is this month's model room the room of pid?
static inline BOOL isModelRoomOwner(const dPersonalID_c &pid) {
    return *dSaveData_c::getTown()->mModelRoom.getPlayerID() == pid;
}
} // namespace dANpcSpHonma

// .text 0x964: the words of the message.
void dAcNpcSpHonma_c::talk_c::onMessageStart(int arg) {
    switch (mRemark) {
    case REMARK_THEME:
        dANpcSpHonma::setRemarkWords(mpController, &dHR::remarks_c::mTheme);
        break;
    case REMARK_SERIES:
        dANpcSpHonma::setRemarkWords(mpController, &dHR::remarks_c::mSeries);
        break;
    case REMARK_PARTS:
        dANpcSpHonma::setRemarkWords(mpController, &dHR::remarks_c::mParts);
        break;
    case REMARK_NOTABLE:
        dANpcSpHonma::setRemarkWords(mpController, &dHR::remarks_c::mNotable);
        break;
    case REMARK_EXTRA:
        dANpcSpHonma::setRemarkWords(mpController, &dHR::remarks_c::mExtra);
        break;
    }
    if (mMessageCode < MSG_SERIES_WORD_END) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
        if (player != NULL) {
            dANpcSpHonma::setSeriesWord(this, player->mHRAInfo.mLastSeries);
        }
    }
    switch (mMessageCode) {
    case MSG_EVALUATION:
        formatNumber(dPlayerMgr::Mgr_c::getCurrentPlayer()->mHRAInfo.mLastScore,
                     WORD_SCORE, SCORE_DIGITS, dScript::NUM_FORMAT_REGION);
        break;
    case MSG_WINNER_PLAYER:
        setPersonalName(dSaveData_c::getTown()->mModelRoom.getPlayerID(),
                        WORD_MODEL_ROOM_PLAYER);
        setLandName(&dSaveData_c::getTown()->mModelRoom.getPlayerID()->land,
                    WORD_MODEL_ROOM_TOWN);
        break;
    case MSG_WINNER_VILLAGER:
        setAnmPersonalName(dSaveData_c::getTown()->mModelRoom.getAnimalID(),
                           WORD_MODEL_ROOM_VILLAGER);
        setLandName(&dSaveData_c::getTown()->mModelRoom.getAnimalID()->mLand,
                    WORD_MODEL_ROOM_TOWN);
        break;
    }
}

// .text 0xCC4: the remarks one after another, then the next message.
void dAcNpcSpHonma_c::talk_c::onMessageEnd(int kind) {
    dHR::remarks_c *remarks = dHR::getRemarks();
    switch (kind) {
    case END_REMARK_SERIES:
        if (remarks->mSeries.mMsg != 0) {
            mpController->setNextMsgCode(remarks->mSeries.mMsg);
            mRemark = REMARK_SERIES;
            break;
        }
    case END_REMARK_PARTS:
        if (remarks->mParts.mMsg != 0) {
            mpController->setNextMsgCode(remarks->mParts.mMsg);
            mRemark = REMARK_PARTS;
            break;
        }
    case END_REMARK_NOTABLE:
        if (remarks->mNotable.mMsg != 0) {
            mpController->setNextMsgCode(remarks->mNotable.mMsg);
            mRemark = REMARK_NOTABLE;
            break;
        }
    case END_REMARK_EXTRA:
        if (remarks->mExtra.mMsg != 0) {
            mpController->setNextMsgCode(remarks->mExtra.mMsg);
            mRemark = REMARK_EXTRA;
            break;
        }
    default:
        mRemark = REMARK_NONE;
        if ((mMessageCode >= HR_REMARK_MESSY_ITEM &&
             mMessageCode <= HR_REMARK_LAYOUT_PACKED) ||
            (mMessageCode >= HR_REMARK_THEME_NONE &&
             mpController->mNextMsgCode == 0)) {
            if (dSaveData_c::getTown()->mModelRoom.getPlayerID() != NULL &&
                dANpcSpHonma::isModelRoomOwner(
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID)) {
                mpController->setNextMsgCode(MSG_ON_DISPLAY_YOURS);
            } else {
                mpController->setNextMsgCode(MSG_MAKE_ROOMS);
            }
        } else {
            switch (mMessageCode) {
            case MSG_LETTERS_UNCHANGED:
            case MSG_NO_EVALUATION:
            case MSG_EVALUATION_END:
            case MSG_ON_DISPLAY_YOURS:
            case MSG_MAKE_ROOMS:
            case MSG_NO_WINNER:
                mpController->setNextMsgCode(MSG_ANYTHING_ELSE);
                break;
            case MSG_JOINED:
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_HRA_RATED);
                dHR::onJoinAcademy();
                break;
            case MSG_LETTERS_STOPPED:
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_HRA_NO_LETTER);
                mpController->setNextMsgCode(MSG_ANYTHING_ELSE);
                break;
            case MSG_LETTERS_RESUMED:
                dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
                    PRIVATE_FLAG0_HRA_NO_LETTER);
                mpController->setNextMsgCode(MSG_ANYTHING_ELSE);
                break;
            case MSG_EVALUATION: {
                dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
                mpController->setNextMsgCode(player->mHRAInfo.mLastRemark +
                                             MSG_ADVICE);
            } break;
            case MSG_THEME + MODEL_ROOM_THEME_SCORE:
            case MSG_THEME + MODEL_ROOM_THEME_FENG_SHUI:
            case MSG_THEME + MODEL_ROOM_THEME_ADULT:
            case MSG_THEME + MODEL_ROOM_THEME_KIDDY:
            case MSG_THEME + MODEL_ROOM_THEME_NEW:
            case MSG_THEME + MODEL_ROOM_THEME_OLD:
                if (dSaveData_c::getTown()->mModelRoom.getPlayerID() != NULL) {
                    if (dANpcSpHonma::isModelRoomOwner(
                            dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID)) {
                        mpController->setNextMsgCode(MSG_WINNER_YOU);
                    } else {
                        mpController->setNextMsgCode(MSG_WINNER_PLAYER);
                    }
                } else if (dSaveData_c::getTown()->mModelRoom.getAnimalID() !=
                           NULL) {
                    mpController->setNextMsgCode(MSG_WINNER_VILLAGER);
                } else {
                    mpController->setNextMsgCode(MSG_NO_WINNER);
                }
                break;
            case MSG_WINNER_YOU:
            case MSG_WINNER_PLAYER:
            case MSG_WINNER_VILLAGER:
                remarks = dHR::getRemarks();
                if (remarks->mTheme.mMsg != 0) {
                    mpController->setNextMsgCode(remarks->mTheme.mMsg);
                    mRemark = REMARK_THEME;
                }
                break;
            }
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x1280: the question menus.
void dAcNpcSpHonma_c::talk_c::onAnswer(int arg) {
    u16 code = mMessageCode;
    u8 answer = mpController->mSelect.mCursor;
    switch (code) {
    case MSG_WELCOME:
    case MSG_QUESTION:
    case MSG_ANYTHING_ELSE:
        switch (answer) {
        case ANSWER_MODEL_ROOM:
            switch (getModelRoomTheme()) {
            case MODEL_ROOM_THEME_SCORE:
                mpController->setNextMsgCode(MSG_THEME +
                                             MODEL_ROOM_THEME_SCORE);
                break;
            case MODEL_ROOM_THEME_FENG_SHUI:
                mpController->setNextMsgCode(MSG_THEME +
                                             MODEL_ROOM_THEME_FENG_SHUI);
                break;
            case MODEL_ROOM_THEME_ADULT:
                mpController->setNextMsgCode(MSG_THEME +
                                             MODEL_ROOM_THEME_ADULT);
                break;
            case MODEL_ROOM_THEME_KIDDY:
                mpController->setNextMsgCode(MSG_THEME +
                                             MODEL_ROOM_THEME_KIDDY);
                break;
            case MODEL_ROOM_THEME_NEW:
                mpController->setNextMsgCode(MSG_THEME + MODEL_ROOM_THEME_NEW);
                break;
            case MODEL_ROOM_THEME_OLD:
                mpController->setNextMsgCode(MSG_THEME + MODEL_ROOM_THEME_OLD);
                break;
            }
            break;
        case ANSWER_EVALUATION:
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_HRA_HAS_RESULT)) {
                mpController->setNextMsgCode(MSG_EVALUATION);
            } else {
                mpController->setNextMsgCode(MSG_NO_EVALUATION);
            }
            break;
        case ANSWER_LETTERS:
            if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_HRA_NO_LETTER)) {
                mpController->setNextMsgCode(MSG_LETTERS_STOP_ASK);
            } else {
                mpController->setNextMsgCode(MSG_LETTERS_RESUME_ASK);
            }
            break;
        }
        break;
    }
}

// .text 0x13E8
void dAcNpcSpHonma_c::talk_c::preExecute() {
}

// .text 0x13EC
void dAcNpcSpHonma_c::talk_c::onTalkEnd() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_LYLE_MET);
    dNpcInfo::kappei_c::get()->setLyleTalked(TRUE);
}

// .text 0x1420
void dAcNpcSpHonma_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1484
int dAcNpcSpHonma_c::getEarType() {
    return NPC_EAR_TYPE_LYLE;
}
