#include <game/game/d_a_rmobj_atm.hpp>
#include <egg/math/eggMath.h>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_select.hpp>
#include <game/sLib/s_lib.hpp>

// The ATM. Functions are in target .text order.

// The ATM's debug parameters (RTTI "hostIO_c"; empty).
class hostIO_c {
  public:
    hostIO_c() {
    }
    virtual ~hostIO_c() {
    }
}; // size 0x4

namespace dARmobjAtm {
static hostIO_c l_HIO;

// .text 0xCC
BASE_PROFILE(A_RMOBJ_ATM, daRmobjAtm_c);
} // namespace dARmobjAtm

// .text 0x254
int daRmobjAtm_c::create() {
    setDemoID(0);
    mDpdBox.set(mPos, 32.0f, 32.0f, 32.0f, mAngle.y, dObjc::HIT_RMOBJ, this, 0);
    setStep(STEP_WAIT);
    return SUCCEEDED;
}

// .text 0x2C0
int daRmobjAtm_c::execute() {
    executeStep();
    mDpdBox.regist();
    return SUCCEEDED;
}

// .text 0x2F8
int daRmobjAtm_c::draw() {
    return SUCCEEDED;
}

// .text 0x300
int daRmobjAtm_c::doDelete() {
    return SUCCEEDED;
}

// .text 0x308
bool daRmobjAtm_c::canInteract(dDemoActor_c *other) {
    if (other != NULL) {
        mVec3_c diff = mPos - other->getPos();
        if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) < 41.6f) {
            if (sLib::distanceAngle(mAngle.y.mAngle + 0x8000,
                                    other->getAngle().y.mAngle) < 0x2000) {
                return true;
            }
        }
    }
    return false;
}

// .text 0x3A8
void daRmobjAtm_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        setStep(STEP_TALK);
        break;
    case DEMO_CMD_END:
        setStep(STEP_WAIT);
        if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
            dMoney_c::offDisable();
        }
        break;
    }
}

typedef BOOL (daRmobjAtm_c::*initStep_t)();
typedef void (daRmobjAtm_c::*executeStep_t)();

namespace dARmobjAtm {
static const initStep_t l_initSteps[daRmobjAtm_c::STEP_NUM] = {
    &daRmobjAtm_c::initWait,    &daRmobjAtm_c::initTalk,
    &daRmobjAtm_c::initTalkEnd, &daRmobjAtm_c::initSuspend,
    &daRmobjAtm_c::initMenu,    &daRmobjAtm_c::initMenuEnd,
    &daRmobjAtm_c::initPaidOff, &daRmobjAtm_c::initPaidOffEnd,
};
static const executeStep_t l_executeSteps[daRmobjAtm_c::STEP_NUM] = {
    &daRmobjAtm_c::executeWait,    &daRmobjAtm_c::executeTalk,
    &daRmobjAtm_c::executeTalkEnd, &daRmobjAtm_c::executeSuspend,
    &daRmobjAtm_c::executeMenu,    &daRmobjAtm_c::executeMenuEnd,
    &daRmobjAtm_c::executePaidOff, &daRmobjAtm_c::executePaidOffEnd,
};
} // namespace dARmobjAtm

// .text 0x404
BOOL daRmobjAtm_c::setStep(int step) {
    if (step < STEP_NUM && (this->*dARmobjAtm::l_initSteps[step])()) {
        mStep = step;
        return TRUE;
    }
    return FALSE;
}

// .text 0x470
void daRmobjAtm_c::executeStep() {
    if (mStep < STEP_NUM) {
        (this->*dARmobjAtm::l_executeSteps[mStep])();
    }
}

// .text 0x4B0
BOOL daRmobjAtm_c::initWait() {
    return TRUE;
}

// .text 0x4B8
void daRmobjAtm_c::executeWait() {
}

// .text 0x4BC
BOOL daRmobjAtm_c::initTalk() {
    attachDemoActor(this);
    setMessageLabel("npc_SP/NPC_obj");
    BOOL city = dScene::isCityScene(dScene::getCurrentScene());
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (player != NULL) {
        mDebt = player->mDebt;
    }
    // MSG_WELCOME_MAIN_BRANCH : MSG_WELCOME_BRANCH (an enum-typed ternary
    // compiles to branches)
    u16 code = city ? 1 : 2;
    if (player != NULL && player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
        code = MSG_NOT_A_CLIENT;
    }
    setMessageCode(code);
    mpController->requestStart(0);
    return TRUE;
}

// .text 0x584
void daRmobjAtm_c::executeTalk() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setStep(STEP_TALK_END);
        }
    }
}

// .text 0x5F8
BOOL daRmobjAtm_c::initTalkEnd() {
    return TRUE;
}

// .text 0x600
void daRmobjAtm_c::executeTalkEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mDebt == 0 &&
                mDebt != 0) {
                if (!dScene::isCurrentSceneAttr(
                        dScene::SCENE_ATTR_CITY_FIELD)) {
                    dMoney_c::onDisable();
                }
                mDebt = 0;
                setStep(STEP_PAID_OFF);
            } else {
                detachDemoActor(this);
                dDemo::finishTalk(this);
                if (dScene::isCityScene(dScene::getCurrentScene())) {
                    dSaveTown_c *town = dSaveData_c::getTown();
                    town->mVillagerMove.setPlayerValue(
                        dPlayerMgr::getCurrentPlayerSlot(), 9);
                }
            }
        }
    }
}

// .text 0x704
BOOL daRmobjAtm_c::initSuspend() {
    mpController->mSuspendReq = dMsg::SUSPEND_REQ_MENU;
    return TRUE;
}

// .text 0x718
void daRmobjAtm_c::executeSuspend() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeSuspend)) {
            setStep(STEP_MENU);
        }
    }
}

// .text 0x78C
BOOL daRmobjAtm_c::initMenu() {
    dPlayerMgr::Mgr_c::getCurrentPlayer();
    switch (mMenu) {
    case MENU_PAY:
        return dMenuMng_c::openMenu(dMenuMng_c::MENU_BANK_PAY) != FALSE;
    case MENU_DEPOSIT:
        return dMenuMng_c::openMenu(dMenuMng_c::MENU_BANK_DEPOSIT) != FALSE;
    case MENU_WITHDRAW:
        return dMenuMng_c::openMenu(dMenuMng_c::MENU_BANK_WITHDRAW) != FALSE;
    }
    return FALSE;
}

// .text 0x82C
void daRmobjAtm_c::executeMenu() {
    if (dMenuMng_c::isMenuEnd()) {
        mpController->setNextMsgCode(mMenuMsgCode);
        mpController->requestStart(0);
        setStep(STEP_MENU_END);
    }
}

// .text 0x888
BOOL daRmobjAtm_c::initMenuEnd() {
    return TRUE;
}

// .text 0x890
void daRmobjAtm_c::executeMenuEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setStep(STEP_TALK_END);
        }
    }
}

// .text 0x904
BOOL daRmobjAtm_c::initPaidOff() {
    if (dPlayerMgr::reactDebtPaid()) {
        setMessageLabel("sys_MESS/SYS_Player");
        setMessageCode(MSG_PLAYER_DEBT_PAID);
        return TRUE;
    }
    return FALSE;
}

// .text 0x960
void daRmobjAtm_c::executePaidOff() {
    if (dPlayerMgr::isReactionFinished()) {
        mpController->requestStart(0);
        setStep(STEP_PAID_OFF_END);
    }
}

// .text 0x9AC
BOOL daRmobjAtm_c::initPaidOffEnd() {
    return TRUE;
}

// .text 0x9B4
void daRmobjAtm_c::executePaidOffEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setStep(STEP_TALK_END);
        }
    }
}

// .text 0xA28
void daRmobjAtm_c::onMessageStart(int arg) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL) {
        switch (mMessageCode) {
        case MSG_DEBT:
            mpController->setNumber(0, player->mDebt, 12, 9);
            break;
        case MSG_SAVINGS:
            mpController->setNumber(1, player->mSavings, 12, 9);
            break;
        }
    }
}

// .text 0xABC
void daRmobjAtm_c::onMessageEnd(int kind) {
    dSelect::Comp_c *sel = mpController->getSelect();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    u32 slot = dPlayerMgr::getCurrentPlayerSlot();
    switch (mMessageCode) {
    case MSG_WELCOME_MAIN_BRANCH:
    case MSG_WELCOME_BRANCH:
        if (player->mDebt > 0) {
            sel->setSysMessage(0, SEL_MSG_PAY, 0, NULL, 0, 0);
            mAnswers[0] = ANSWER_PAY;
            sel->setSysMessage(1, SEL_MSG_DEPOSIT, 0, NULL, 0, 0);
            mAnswers[1] = ANSWER_DEPOSIT;
            sel->setSysMessage(2, SEL_MSG_WITHDRAW, 0, NULL, 0, 0);
            mAnswers[2] = ANSWER_WITHDRAW;
            if (player->isCardMissing() && slot < PLAYER_NUM) {
                sel->setSysMessage(3, SEL_MSG_REISSUE_CARD, 10, NULL, 0, 0);
                mAnswers[3] = ANSWER_REISSUE_CARD;
                sel->setSysMessage(4, SEL_MSG_QUIT, 3, NULL, 0, 0);
                mAnswers[4] = ANSWER_QUIT;
                sel->mCancel = 4;
                sel->mNum = 5;
            } else {
                sel->setSysMessage(3, SEL_MSG_QUIT, 3, NULL, 0, 0);
                mAnswers[3] = ANSWER_QUIT;
                sel->mCancel = 3;
                sel->mNum = 4;
            }
            mpController->openSelect();
        } else {
            sel->setSysMessage(0, SEL_MSG_DEPOSIT, 0, NULL, 0, 0);
            mAnswers[0] = ANSWER_DEPOSIT;
            sel->setSysMessage(1, SEL_MSG_WITHDRAW, 0, NULL, 0, 0);
            mAnswers[1] = ANSWER_WITHDRAW;
            if (player->isCardMissing() && slot < PLAYER_NUM) {
                sel->setSysMessage(2, SEL_MSG_REISSUE_CARD, 10, NULL, 0, 0);
                mAnswers[2] = ANSWER_REISSUE_CARD;
                sel->setSysMessage(3, SEL_MSG_QUIT, 3, NULL, 0, 0);
                mAnswers[3] = ANSWER_QUIT;
                sel->mCancel = 3;
                sel->mNum = 4;
            } else {
                sel->setSysMessage(2, SEL_MSG_QUIT, 3, NULL, 0, 0);
                mAnswers[2] = ANSWER_QUIT;
                sel->mCancel = 2;
                sel->mNum = 3;
            }
            mpController->openSelect();
        }
        break;
    }
}

// .text 0xD6C
void daRmobjAtm_c::onAnswer(int arg) {
    dMsg::Comp_c *comp = mpController;
    int answer = comp != NULL && comp->getSelect() != NULL
                     ? comp->getSelect()->mCursor
                     : -1;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    switch (mAnswers[answer]) {
    case ANSWER_PAY:
        if (player->mDebt != 0) {
            mMenuMsgCode = MSG_DEBT;
            mMenu = MENU_PAY;
            setStep(STEP_SUSPEND);
        } else {
            mpController->setNextMsgCode(MSG_NO_DEBT);
        }
        break;
    case ANSWER_DEPOSIT:
        if (player->mSavings < PRIVATE_SAVINGS_MAX) {
            mMenuMsgCode = MSG_SAVINGS;
            mMenu = MENU_DEPOSIT;
            setStep(STEP_SUSPEND);
        } else {
            mpController->setNextMsgCode(MSG_SAVINGS_FULL);
        }
        break;
    case ANSWER_WITHDRAW:
        if (player->mSavings > 0) {
            mMenuMsgCode = MSG_SAVINGS;
            mMenu = MENU_WITHDRAW;
            setStep(STEP_SUSPEND);
        } else {
            mpController->setNextMsgCode(MSG_NO_SAVINGS);
        }
        break;
    case ANSWER_REISSUE_CARD:
        player->setFlag0(PRIVATE_FLAG0_CARD_REISSUE);
        break;
    }
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
