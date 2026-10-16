#include <game/framework/f_profile_name.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_debug00.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuDebug00 {
void *classInit();
// Profile 0xE6, with execute/draw priorities from the original profile.
fProf::fBaseProfile_c g_profile = {classInit, fProf::MENU_DEBUG00,
                                   fProf::DRAW_ORDER::MENU_DEBUG00};

// .text:0x5C
void *classInit() {
    return new dMenuDebug00_c;
}
} // namespace dMenuDebug00

// .text:0xB8
int dMenuDebug00_c::create() {
    if (!isFlag(FLAG_MAIL_READY)) {
        dMail_c mail;
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(0);
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        static u16 birthdayTextKind = 1;
        static int birthdayPaperIdx = 0x13C;
        mail.setupFromAnimal(&birthdayTextKind, "MAIL_BO_Birthday",
                             &animal->mID, &player->mPID, &birthdayPaperIdx);
        onFlag(FLAG_MAIL_READY);
        dMenuMng_c::setMail(&mail);
    }
    mPaper.setMail(dMenuMng_c::getMail(), FALSE);
    int loaded = mPaper.load();
    if (!(loaded & mButtons.load())) {
        return NOT_READY;
    }
    mPaper.create();
    mButtons.create();
    initButtons();
    startAnm(mPaper.mpLayout, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    mPaper.execute();
    onFlag(FLAG_DRAW);
    return SUCCEEDED;
}

// .text:0x1F0
int dMenuDebug00_c::doDelete() {
    if (!mButtons.remove()) {
        return NOT_READY;
    }
    if (!mPaper.remove()) {
        return NOT_READY;
    }
    return SUCCEEDED;
}
// .text:0x244
int dMenuDebug00_c::stateMain() {
    calcInput();
    executeSubState();
    executePaper();
    return SUCCEEDED;
}
// .text:0x284
int dMenuDebug00_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mPaper.draw();
        mButtons.draw();
    }
    return SUCCEEDED;
}
// .text:0x2D0
int dMenuDebug00_c::stateAnm() {
    bool done = calcAnm();
    mPaper.calcAnm();
    mButtons.setPos(mPaper.mpLayout, "N_all");
    if (done) {
        changeNextState();
    }
    return SUCCEEDED;
}
// .text:0x338
int dMenuDebug00_c::stateClose() {
    if (openNextMenu()) {
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}
// .text:0x380
int dMenuDebug00_c::execute() {
    int result = dMenuBase_c::execute();
    mButtons.execute();
    return result;
}
// .text:0x3C4
void dMenuDebug00_c::subStateMain() {
    setButtonSelect(mSelectedButton, FALSE);
    mSelectedButton = hitCheck();
    if (mSelectedButton != NO_BUTTON) {
        setButtonSelect(mSelectedButton, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            pushButton();
        }
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        offFlag(FLAG_DRAW);
        setSubState(SUB_REMOVE);
    }
}
// .text:0x490
void dMenuDebug00_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}
// .text:0x4CC
void dMenuDebug00_c::subStateLoad() {
    if (mPaper.load()) {
        mPaper.create();
        setSubState(SUB_MAIN);
        onFlag(FLAG_DRAW);
        onFlag(FLAG_UPDATE_PAPER);
    }
}
// .text:0x53C
void dMenuDebug00_c::subStateRemove() {
    if (mPaper.remove()) {
        setSubState(SUB_LOAD);
    }
}
// .text:0x58C
void dMenuDebug00_c::executeSubState() {
    typedef void (dMenuDebug00_c::*Proc)();
    static Proc subStateProcs[] = {
        &dMenuDebug00_c::subStateMain, &dMenuDebug00_c::subStatePush,
        &dMenuDebug00_c::subStateLoad, &dMenuDebug00_c::subStateRemove};
    (this->*subStateProcs[mSubState])();
}
// .text:0x5C4
void dMenuDebug00_c::calcInput() {
}
// .text:0x5C8
void dMenuDebug00_c::executePaper() {
    if (isFlag(FLAG_UPDATE_PAPER)) {
        offFlag(FLAG_UPDATE_PAPER);
        mPaper.execute();
    }
}
// .text:0x614
void dMenuDebug00_c::startCloseAnm() {
    startAnm(mPaper.mpLayout, "N_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
}
// .text:0x630
void dMenuDebug00_c::initButtons() {
    mButtons.setVisible(0, false);
    mButtons.setVisible(1, false);
    mButtons.setText(BUTTON_BACK, BACK_MESSAGE, TRUE);
}
// .text:0x68C
void dMenuDebug00_c::setButtonSelect(int button, BOOL select) {
    if (isButton(button)) {
        mButtons.setSelect(button, select, TRUE);
    }
}
// .text:0x6BC
int dMenuDebug00_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return NO_BUTTON;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int button = mButtons.hitCheck(&pos);
    if (button != NO_BUTTON) {
        return button;
    }
    return NO_BUTTON;
}
// .text:0x71C
void dMenuDebug00_c::pushButton() {
    switch (isButton(mSelectedButton)) {
    default:
        mButtons.push(mSelectedButton, FALSE);
        break;
    case false:
        return;
    }
    setSubState(SUB_PUSH);
}
// .text:0x794
bool dMenuDebug00_c::isPushEnd() {
    if (isButton(mSelectedButton)) {
        return !mButtons.isPushing();
    }
    return true;
}
// .text:0x7EC
void dMenuDebug00_c::finishPush() {
    if (isButton(mSelectedButton)) {
        startCloseAnm();
    } else {
        setSubState(SUB_MAIN);
    }
}
// .text:0x828
bool dMenuDebug00_c::openNextMenu() {
    int menu;
    switch (mMenuNo) {
    case 8:
        menu = 5;
        break;
    case 9:
        menu = 6;
        break;
    case 10:
        dAcUitag::openMenuA(9);
        return false;
    default:
        return false;
    }
    if (!dAcUitag::openMenuB(menu)) {
        return true;
    }
    return false;
}
