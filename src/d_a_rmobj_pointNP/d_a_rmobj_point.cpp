#include <game/game/d_a_rmobj_point.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_letter.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/sLib/s_lib.hpp>

// Nook's point machine. Functions are in target .text order.

namespace dARmobjPoint {
// .text 0xCC
BASE_PROFILE(A_RMOBJ_POINT, daRmobjPoint_c);
} // namespace dARmobjPoint

// .text 0x2EC
int daRmobjPoint_c::create() {
    mState = STATE_IDLE;
    mPrevState = STATE_NONE;
    mNextState = STATE_NONE;
    setDemoID(0);
    dBGCF::getMvbgList()->add(32.0f, 32.0f, 32.0f, &mBg, mPos, mAng(0), NULL);
    mBox.set(mPos, 32.0f, 32.0f, 32.0f, 0, dObjc::HIT_RMOBJ, this, 0);
    return SUCCEEDED;
}

namespace dARmobjPoint {
static const daRmobjPoint_c::State_s l_states[] = {
    {&daRmobjPoint_c::initIdle, &daRmobjPoint_c::executeIdle},
    {&daRmobjPoint_c::initTalk, &daRmobjPoint_c::executeTalk},
    {&daRmobjPoint_c::initSuspend, &daRmobjPoint_c::executeSuspend},
    {&daRmobjPoint_c::initMenu, &daRmobjPoint_c::executeMenu},
    {&daRmobjPoint_c::initResume, &daRmobjPoint_c::executeResume},
};
} // namespace dARmobjPoint

// .text 0x390
int daRmobjPoint_c::doDelete() {
    dBGCF::getMvbgList()->remove(&mBg);
    return SUCCEEDED;
}

// .text 0x3C8
int daRmobjPoint_c::execute() {
    mBox.regist();
    executeState();
    updateState();
    return SUCCEEDED;
}

// .text 0x40C
int daRmobjPoint_c::draw() {
    return SUCCEEDED;
}

// .text 0x414
void daRmobjPoint_c::initIdle() {
}

// .text 0x418
void daRmobjPoint_c::executeIdle() {
}

// .text 0x41C
void daRmobjPoint_c::initTalk() {
    if (mPrevState == STATE_IDLE) {
        u16 code;
        if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_NOOK_POINTS_TOLD)) {
            code = MSG_NO_CARD;
        } else {
            dLandID_c *land = dSaveData_c::getTown()->getLandID();
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            if (player->mPID.land == *land) {
                code = MSG_WELCOME_MEMBER;
            } else {
                code = MSG_WELCOME_VISITOR;
            }
        }
        startTalk(code);
    }
}

// .text 0x4E4
void daRmobjPoint_c::executeTalk() {
    dMsg::Comp_c *ctrl = mpController;
    if (ctrl != NULL) {
        if (ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
}

// .text 0x56C
void daRmobjPoint_c::initSuspend() {
    mpController->mSuspendReq = dMsg::SUSPEND_REQ_MENU;
}

// .text 0x57C
void daRmobjPoint_c::executeSuspend() {
    dMsg::Comp_c *ctrl = mpController;
    if (ctrl != NULL) {
        if (ctrl->isState(&dMsg::Comp_c::executeSuspend)) {
            mNextState = STATE_MENU;
        }
    }
}

// .text 0x5EC
void daRmobjPoint_c::initMenu() {
    dMenuMng_c::openMenu(dMenuMng_c::MENU_POINT_PRESENT);
}

// .text 0x5F4
void daRmobjPoint_c::executeMenu() {
    if (dMenuMng_c::isMenuEnd()) {
        if (lbl_8074EA7D != 1) {
            mpController->setNextMsgCode(MSG_MENU_CANCEL);
        } else {
            mpController->setNextMsgCode(MSG_PRESENT_SENT);
            static dMail_c mail;
            static u16 kind = 6;
            static u8 senderKind = MAIL_FROM_TNPS;
            static int paper = dItem::ITEM_IDX_NOOK_PAPER;
            dItem::Item present = dMenuMng_c::getMenuItem();
            dLetter::setItemName(0, present);
            mail.setupSystem(&kind, "MAIL_ETC_TPS", &senderKind,
                             &dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID,
                             &paper);
            mail.setPresent(present.mId, 0xFF);
            if (dPostOffice::add(&mail)) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                    PRIVATE_FLAG1_NOOK_POINT_PRESENT);
            } else {
                mpController->setNextMsgCode(MSG_BUSY);
            }
        }
        mNextState = STATE_RESUME;
    }
}

// .text 0x720
void daRmobjPoint_c::initResume() {
    mpController->requestStart(0);
}

// .text 0x72C
void daRmobjPoint_c::executeResume() {
    dMsg::Comp_c *ctrl = mpController;
    if (ctrl != NULL) {
        if (!ctrl->isState(&dMsg::Comp_c::executeSuspend)) {
            mNextState = STATE_TALK;
        }
    }
}

// .text 0x79C
void daRmobjPoint_c::updateState() {
    if (mNextState != STATE_NONE) {
        mPrevState = mState;
        mState = mNextState;
        if (dARmobjPoint::l_states[mState].mInit) {
            (this->*dARmobjPoint::l_states[mState].mInit)();
        }
        mNextState = STATE_NONE;
    }
}

// .text 0x820
void daRmobjPoint_c::executeState() {
    if (dARmobjPoint::l_states[mState].mExecute) {
        (this->*dARmobjPoint::l_states[mState].mExecute)();
    }
}

namespace dARmobjPoint {
static inline dSaveShop_c *getShop() {
    return &dSaveData_c::getTown()->mShops.mShop;
}
} // namespace dARmobjPoint

// .text 0x890
void daRmobjPoint_c::onMessageStart(int arg) {
    dSaveShop_c *shop = dARmobjPoint::getShop();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    getController()->setStringWord(0, shop->getRank(player) + STR_UNIT_RANK,
                                   "sys_STRING/STR_Unit");
    getController()->setLandName(1, &player->mPID.land);
    getController()->formatNumber(2, player->mNookPointsLifetime, 5,
                                  dScript::NUM_FORMAT_REGION);
    getController()->formatNumber(3, player->mNookPoints, 5,
                                  dScript::NUM_FORMAT_REGION);
    getController()->formatNumber(
        4, shop->getPointsToNextRank(player->mNookPointsLifetime), 5,
        dScript::NUM_FORMAT_REGION);
}

// .text 0x978
void daRmobjPoint_c::onMessageEnd(int kind) {
    int code = mMessageCode;
    dSaveShop_c *shop = &dSaveData_c::getTown()->mShops.mShop;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    switch (code) {
    case MSG_POINT_STATUS:
        if (shop->getRank(player) == RANK_PLATINUM) {
            mpController->setNextMsgCode(MSG_PLATINUM);
        } else {
            mpController->setNextMsgCode(MSG_NEXT_RANK);
        }
        break;
    case MSG_VISIT_POINTS:
    case MSG_VISIT_POINTS_FAR:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->addNookPointsForShop();
        break;
    }
}

// .text 0xA20
void daRmobjPoint_c::onAnswer(int arg) {
    u16 code = mMessageCode;
    int answer = mpController->mSelect.mCursor;
    if ((code == MSG_WELCOME_MEMBER || code == MSG_ANYTHING_ELSE) &&
        answer == ANSWER_PRESENT) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_NOOK_POINT_PRESENT)) {
            mpController->setNextMsgCode(MSG_PRESENT_TODAY);
        } else if (dPostOffice::isFull() == TRUE) {
            mpController->setNextMsgCode(MSG_BUSY);
        } else {
            if (mpController->mNextMsgCode == 0) {
                mpController->setNextMsgCode(0xFFFF);
            }
            mNextState = STATE_SUSPEND;
        }
    } else if (answer == ANSWER_VISITOR_POINTS) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                PRIVATE_FLAG1_NOOK_VISIT_POINTS)) {
            mpController->setNextMsgCode(MSG_VISITED_TODAY);
        } else {
            dLandID_c *land = dSaveData_c::getTown()->getLandID();
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            if (player->mPID.land == *land) {
                mpController->setNextMsgCode(MSG_VISIT_POINTS);
            } else {
                mpController->setNextMsgCode(MSG_VISIT_POINTS_FAR);
            }
        }
    }
}

// .text 0xB84
void daRmobjPoint_c::startTalk(u16 code) {
    attachDemoActor(this);
    dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
    setMessageLabel("NPC_sp/NPC_obj");
    setMessageCode(code);
    comp->requestStart(0);
}

// .text 0xC08
bool daRmobjPoint_c::canInteract(dDemoActor_c *actor) {
    if (actor != NULL) {
        mVec3_c diff = mPos - actor->getPos();
        if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) < 35.2f &&
            sLib::distanceAngle(mAngle.y.mAngle + 0x8000,
                                actor->getAngle().y.mAngle) < 0x2000) {
            return true;
        }
    }
    return false;
}

// .text 0xCA8
void daRmobjPoint_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        mNextState = STATE_TALK;
        break;
    case DEMO_CMD_END:
        mNextState = STATE_IDLE;
        break;
    }
}
