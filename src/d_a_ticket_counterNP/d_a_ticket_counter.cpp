#include <game/game/d_a_ticket_counter.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_rmobj_barricade.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_demo_sync.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_select.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_theater.hpp>
#include <game/game/d_theater_common.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>

// The theater's ticket counter. Functions are in target .text order.

// The pointer box (sizes x, z and height).
#define TICKET_COUNTER_BOX_SIZE 32.0f
#define TICKET_COUNTER_BOX_HEIGHT 48.0f
// canInteract: the player is within this xz distance and faces the counter
// (angle TICKET_COUNTER_FACE_ANGLE, within TICKET_COUNTER_FACE_RANGE).
#define TICKET_COUNTER_TALK_DIST 48.0f
#define TICKET_COUNTER_FACE_ANGLE -0x8000
#define TICKET_COUNTER_FACE_RANGE 0x2000
// movePlayer: how far the player is walked back (z) from a locked exit.
#define TICKET_COUNTER_PLAYER_STEP 32.0f
// The clerk's voice type (Rcpt_c::mVoiceType); getVoiceMode is 0 with it.
#define TICKET_COUNTER_VOICE_TYPE 1

// The counter's debug parameters (RTTI "daTicketCounterHIO_c"; nothing reads
// them in this build).
class daTicketCounterHIO_c {
  public:
    daTicketCounterHIO_c() {
        _04 = 1;
    }
    virtual ~daTicketCounterHIO_c() {
    }

    /* 0x4 */ int _04;
}; // size 0x8

namespace dATicketCounter {
static daTicketCounterHIO_c l_HIO;

// .text 0xCC
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text 0xD8
BASE_PROFILE(A_TICKET_COUNTER, daTicketCounter_c);
} // namespace dATicketCounter

// .text 0x16C
int daTicketCounter_c::create() {
    setDemoID(0);
    initParam();
    mBox.set(mPos, TICKET_COUNTER_BOX_SIZE, TICKET_COUNTER_BOX_SIZE,
             TICKET_COUNTER_BOX_HEIGHT, 0, dObjc::HIT_RMOBJ, this, 0);
    dHmnName::Word_c name;
    dString::loadString(&name, dNpcInfo::SP_NAME_CLERK,
                        "sys_STRING/STR_SPNpc_name");
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(),
        GENDER_OTHER);
    mVoiceType = TICKET_COUNTER_VOICE_TYPE;
    if (dTheater::g_common.mTicket) {
        dBGCF::lockNext(NEXT_STREET);
        dBGCF::unlockNext(NEXT_THEATER);
        dBGCF::unlockNext(NEXT_THEATER2);
    } else {
        dBGCF::unlockNext(NEXT_STREET);
        dBGCF::lockNext(NEXT_THEATER);
        dBGCF::lockNext(NEXT_THEATER2);
    }
    return SUCCEEDED;
}

// .text 0x274
int daTicketCounter_c::execute() {
    mBox.regist();
    executeState();
    return SUCCEEDED;
}

// .text 0x2B0
int daTicketCounter_c::draw() {
    return SUCCEEDED;
}

// .text 0x2B8
int daTicketCounter_c::doDelete() {
    if (mLeaving == TRUE) {
        dDemoSync::release();
    }
    return SUCCEEDED;
}

// .text 0x2E8
void daTicketCounter_c::initParam() {
    mLeaving = FALSE;
    mPrice = TICKET_PRICE;
}

namespace dATicketCounter {
static const daTicketCounter_c::InitProc l_initProcs[] = {
    &daTicketCounter_c::initWait,     &daTicketCounter_c::initTalk,
    &daTicketCounter_c::initTalkEnd,  &daTicketCounter_c::initStop,
    &daTicketCounter_c::initStopEnd,  &daTicketCounter_c::initLeave,
    &daTicketCounter_c::initLeaveEnd, &daTicketCounter_c::initWaitPlayer,
};
static const daTicketCounter_c::ExecuteProc l_executeProcs[] = {
    &daTicketCounter_c::executeWait,     &daTicketCounter_c::executeTalk,
    &daTicketCounter_c::executeTalkEnd,  &daTicketCounter_c::executeStop,
    &daTicketCounter_c::executeStopEnd,  &daTicketCounter_c::executeLeave,
    &daTicketCounter_c::executeLeaveEnd, &daTicketCounter_c::executeWaitPlayer,
};
} // namespace dATicketCounter

// .text 0x2FC
BOOL daTicketCounter_c::changeState(int state) {
    if (state < STATE_NUM && (this->*dATicketCounter::l_initProcs[state])()) {
        mState = state;
        return TRUE;
    }
    return FALSE;
}

// .text 0x368
void daTicketCounter_c::executeState() {
    if (mState < STATE_NUM) {
        (this->*dATicketCounter::l_executeProcs[mState])();
    }
}

// .text 0x3A8
BOOL daTicketCounter_c::initWait() {
    return TRUE;
}

// .text 0x3B0
void daTicketCounter_c::executeWait() {
    mVec3_c *player = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        mVec3_c pos = *player;
        int next = dBGCF::getLockedNext(&pos);
        if (next == NEXT_STREET || next == NEXT_THEATER ||
            next == NEXT_THEATER2) {
            dDemo::requestSpeak(this, NULL);
        }
    }
}

// .text 0x41C
BOOL daTicketCounter_c::initTalk() {
    attachDemoActor(this);
    dTheater::common_c *common = &dTheater::g_common;
    if (!daRmobjBarricade_c::isPlaced()) {
        if (common->mTicket) {
            setMessageLabel("npc_SP/NPC_theater");
            setMessageCode(MSG_GO_IN);
        } else {
            setMessageLabel("npc_SP/NPC_theater");
            setMessageCode(MSG_WELCOME);
        }
    } else {
        clearSpeakerName();
        setMessageLabel("sys_MESS/SYS_Door");
        setMessageCode(DOOR_MSG_CLOSED);
    }
    mpController->requestStart(0);
    return TRUE;
}

// .text 0x4F8
void daTicketCounter_c::executeTalk() {
    dMsg::Comp_c *ctrl = mpController;
    if (ctrl != NULL) {
        if (!ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            changeState(STATE_TALK_END);
        }
    }
}

// .text 0x56C
BOOL daTicketCounter_c::initTalkEnd() {
    return TRUE;
}

// .text 0x574
void daTicketCounter_c::executeTalkEnd() {
    dTheater::common_c *common = &dTheater::g_common;
    dMsg::Comp_c *ctrl = mpController;
    if (ctrl != NULL) {
        if (ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            if (common->mTicket) {
                dBGCF::lockNext(NEXT_STREET);
                dBGCF::unlockNext(NEXT_THEATER);
                dBGCF::unlockNext(NEXT_THEATER2);
            }
            dDemo::finishTalk(this);
        }
    }
}

// .text 0x630
BOOL daTicketCounter_c::initStop() {
    attachDemoActor(this);
    u8 prevScene = dScene::getPrevScene();
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour >= OPEN_HOUR && now.hour < CLOSE_HOUR) {
        if (prevScene == dScene::SCENE_RM_THEATER_SHOW) {
            setMessageLabel("npc_SP/NPC_theater");
            setMessageCode(MSG_NO_REENTRY);
        } else {
            setMessageLabel("npc_SP/NPC_theater");
            setMessageCode(MSG_NO_TICKET);
        }
    } else {
        setMessageLabel("npc_SP/NPC_theater");
        setMessageCode(MSG_CLOSED);
    }
    mpController->requestStart(0);
    return TRUE;
}

// .text 0x75C
void daTicketCounter_c::executeStop() {
    dMsg::Comp_c *ctrl = mpController;
    if (ctrl != NULL) {
        if (!ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            changeState(STATE_STOP_END);
        }
    }
}

// .text 0x7D0
BOOL daTicketCounter_c::initStopEnd() {
    return TRUE;
}

// .text 0x7D8
void daTicketCounter_c::executeStopEnd() {
    dMsg::Comp_c *ctrl = mpController;
    if (ctrl != NULL) {
        if (ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            movePlayer(STATE_STOP_END);
            changeState(STATE_WAIT_PLAYER);
        }
    }
}

// .text 0x870
BOOL daTicketCounter_c::initLeave() {
    attachDemoActor(this);
    setMessageLabel("npc_SP/NPC_theater");
    setMessageCode(MSG_LEAVE);
    mpController->requestStart(0);
    return TRUE;
}

// .text 0x8DC
void daTicketCounter_c::executeLeave() {
    dMsg::Comp_c *ctrl = mpController;
    if (ctrl != NULL) {
        if (!ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            changeState(STATE_LEAVE_END);
        }
    }
}

// .text 0x950
BOOL daTicketCounter_c::initLeaveEnd() {
    return TRUE;
}

// .text 0x958
void daTicketCounter_c::executeLeaveEnd() {
    dMsg::Comp_c *ctrl = mpController;
    if (ctrl != NULL) {
        if (ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            if (mLeaving == FALSE) {
                changeState(STATE_WAIT_PLAYER);
            }
        }
    }
}

// .text 0x9F0
BOOL daTicketCounter_c::initWaitPlayer() {
    return TRUE;
}

// .text 0x9F8
void daTicketCounter_c::executeWaitPlayer() {
    if (dPlayerMgr::isWaitingForDemo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        dDemo::finishTalk(this);
    }
}

// .text 0xA38
bool daTicketCounter_c::canInteract(dDemoActor_c *actor) {
    f32 range = TICKET_COUNTER_TALK_DIST;
    if (actor != NULL) {
        mVec3_c diff = mPos - actor->getPos();
        if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) < range &&
            (s16)sLib::distanceAngle(TICKET_COUNTER_FACE_ANGLE,
                                     actor->getAngle().y.mAngle) <
                TICKET_COUNTER_FACE_RANGE) {
            return true;
        }
    }
    return false;
}

// .text 0xAD8
void daTicketCounter_c::onDemoCmd(int cmd, u8 member) {
    dTheater::common_c *common = &dTheater::g_common;
    switch (cmd) {
    case DEMO_CMD_TALK:
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        if (common->mTicket) {
            changeState(STATE_LEAVE);
        } else {
            changeState(STATE_STOP);
        }
        break;
    case DEMO_CMD_END:
        changeState(STATE_WAIT);
        break;
    }
}

// .text 0xB34
void daTicketCounter_c::movePlayer(int state) {
    mVec3_c *player = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        mVec3_c pos = *player;
        if (state == STATE_STOP_END) {
            pos.z += TICKET_COUNTER_PLAYER_STEP;
        } else if (state == STATE_LEAVE_END) {
            pos.z -= TICKET_COUNTER_PLAYER_STEP;
        }
        dPlayerMgr::moveTo(&pos, dPlayerMgr::PLAYER_INDEX_CURRENT,
                           dPlayerMgr::sDefaultMoveSpeed);
    }
}

// .text 0xBCC
int daTicketCounter_c::getVoiceMode() {
    return mVoiceType != TICKET_COUNTER_VOICE_TYPE;
}

// .text 0xBE4
void daTicketCounter_c::onMessageStart(int arg) {
    dTheater::common_c *common = &dTheater::g_common;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (mMessageCode == MSG_SHOW_PRICE) {
        mpController->setStringWord(0, common->mProgram,
                                    "sys_STRING/STR_Reaction");
        common->setTicket(TRUE);
    } else if (mMessageCode == MSG_THANK_YOU) {
        player->payMoney(mPrice, FALSE);
    }
}

// .text 0xC68
void daTicketCounter_c::onMessageEnd(int kind) {
    dMsg::Comp_c *ctrl = getController();
    dSelect::Comp_c *sel = ctrl->getSelect();
    dSaveExtra_c *extra = dSaveData_c::getExtra();
    switch (mMessageCode) {
    case MSG_WHICH_SHOW:
        sel->setMessage(0, extra->mTheater.mPrograms[0],
                        "sys_STRING/STR_Reaction", MSG_SHOW_PRICE, NULL, 0, 0);
        sel->setMessage(1, extra->mTheater.mPrograms[1],
                        "sys_STRING/STR_Reaction", MSG_SHOW_PRICE, NULL, 0, 0);
        sel->setMessage(2, extra->mTheater.mPrograms[2],
                        "sys_STRING/STR_Reaction", MSG_SHOW_PRICE, NULL, 0, 0);
        sel->setMessage(3, extra->mTheater.mPrograms[3],
                        "sys_STRING/STR_Reaction", MSG_SHOW_PRICE, NULL, 0, 0);
        sel->setMessage(SHOW_ANSWER_NEVER_MIND, STR_REACTION_NEVER_MIND,
                        "sys_STRING/STR_Reaction", MSG_COME_AGAIN, NULL, 0, 0);
        sel->mCancel = SHOW_ANSWER_NEVER_MIND;
        sel->mNum = SHOW_ANSWER_NUM;
        ctrl->openSelect();
        break;
    case MSG_SHOW_PRICE:
        ctrl->setNextMsgCode(MSG_THANK_YOU);
        break;
    case MSG_FIRST_TIME:
        ctrl->setNextMsgCode(MSG_SPECIAL_SHOW);
        break;
    }
}

// .text 0xDD4
void daTicketCounter_c::onAnswer(int arg) {
    int price;
    dMsg::Comp_c *ctrl = mpController;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dTheater::common_c *common = &dTheater::g_common;
    dSaveExtra_c *extra = dSaveData_c::getExtra();
    int answer;
    if (ctrl != NULL && ctrl->getSelect() != NULL) {
        answer = ctrl->getSelect()->mCursor;
    }
    switch (mMessageCode) {
    case MSG_TICKETS:
        switch (answer) {
        case ANSWER_BUY:
            price = mPrice;
            BOOL enough = player->getPocketMoney() >= price;
            if (!enough) {
                ctrl->setNextMsgCode(MSG_NO_MONEY);
            } else if (!player->isFlag0(PRIVATE_FLAG0_THEATER_SEEN)) {
                ctrl->setNextMsgCode(MSG_FIRST_TIME);
                player->payMoney(mPrice, FALSE);
                common->setTicket(TRUE);
            } else {
                ctrl->setNextMsgCode(MSG_WHICH_SHOW);
            }
            break;
        case ANSWER_NO:
            ctrl->setNextMsgCode(MSG_COME_AGAIN);
            break;
        }
        break;
    case MSG_WHICH_SHOW:
        switch (answer) {
        case 0:
            common->setProgram(extra->mTheater.mPrograms[0]);
            break;
        case 1:
            common->setProgram(extra->mTheater.mPrograms[1]);
            break;
        case 2:
            common->setProgram(extra->mTheater.mPrograms[2]);
            break;
        case 3:
            common->setProgram(extra->mTheater.mPrograms[3]);
            break;
        }
        break;
    case MSG_LEAVE:
        switch (answer) {
        case ANSWER_GO_IN:
            movePlayer(STATE_LEAVE_END);
            mLeaving = FALSE;
            break;
        case ANSWER_GO_OUT:
            dATicketCounter::getSceneChange()->requestExit(0, 2, 2);
            mLeaving = TRUE;
            break;
        }
        break;
    }
}
