#include <game/game/d_menu_paper.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuPaper {
// .text:0x5C
BASE_PROFILE(MENU_PAPER, dMenuPaper_c)
} // namespace dMenuPaper

// .text:0xB8
int dMenuPaper_c::create() {
    mPaper.setMail(dMenuMng_c::getMail(), FALSE);
    BOOL loaded = mPaper.load();
    loaded &= mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    mPaper.create();
    mBtn.create();
    initButtons();
    startAnm(mPaper.mpLayout, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    mPaper.execute();
    onFlag(FLAG_DRAW);
    return SUCCEEDED;
}

// .text:0x170
int dMenuPaper_c::doDelete() {
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    bool removed = mPaper.remove();
    return removed;
}

// .text:0x1C4
int dMenuPaper_c::stateMain() {
    if (handleCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    calcPaper();
    return SUCCEEDED;
}

// .text:0x214
int dMenuPaper_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mPaper.draw();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x260
int dMenuPaper_c::stateAnm() {
    bool done = calcAnm();
    mPaper.calcAnm();
    mBtn.setPos(mPaper.mpLayout, "N_all");
    if (done) {
        changeNextState();
    }
    return SUCCEEDED;
}

// .text:0x2C8
int dMenuPaper_c::stateClose() {
    if (isFlag(FLAG_QUIT)) {
        deleteRequest();
        return SUCCEEDED;
    }
    if (openNextMenu()) {
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x334
int dMenuPaper_c::execute() {
    int result = dMenuBase_c::execute();
    mBtn.execute();
    return result;
}

// .text:0x378
void dMenuPaper_c::subStateMain() {
    setSelect(mCursor, FALSE);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        mCursor = btn;
        setSelect(btn, TRUE);
        pushCursor();
    } else {
        int cursor = hitCheck();
        mCursor = cursor;
        if (cursor != CURSOR_NONE) {
            setSelect(cursor, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                pushCursor();
            }
        }
    }
}

// .text:0x434
void dMenuPaper_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x470
BOOL dMenuPaper_c::onCloseMain() {
    startCloseAnm();
    onFlag(FLAG_QUIT);
    dAcUitag::closeTab();
    dAcUitag::setHideAfterMenu();
    return TRUE;
}

// .text:0x4B4
BOOL dMenuPaper_c::handleCloseRequest() {
    if (mMenuNo != dMenuMng_c::MENU_READ_LETTER) {
        return FALSE;
    }
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    typedef BOOL (dMenuPaper_c::*Proc)();
    static Proc closeProcs[SUB_NUM] = {
        &dMenuPaper_c::onCloseMain,
        NULL,
    };
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL closed = (this->*closeProcs[mSubState])();
    if (closed) {
        dMenuMng_c::clearCloseRequest();
    }
    return closed;
}

// .text:0x59C
void dMenuPaper_c::executeSubState() {
    typedef void (dMenuPaper_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuPaper_c::subStateMain,
        &dMenuPaper_c::subStatePush,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x5D4
void dMenuPaper_c::calcPaper() {
    if (isFlag(FLAG_CALC_PAPER)) {
        offFlag(FLAG_CALC_PAPER);
        mPaper.execute();
    }
}

// .text:0x620
void dMenuPaper_c::startCloseAnm() {
    startAnm(mPaper.mpLayout, "N_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
}

// .text:0x63C
void dMenuPaper_c::initButtons() {
    mBtn.setVisible(BTN_UNUSED_0, false);
    mBtn.setVisible(BTN_UNUSED_1, false);
    mBtn.setText(BTN_BACK, MSG_BACK, TRUE);
}

// .text:0x698
void dMenuPaper_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, TRUE);
    }
}

// .text:0x6C8
int dMenuPaper_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn;
    }
    return CURSOR_NONE;
}

// .text:0x728
void dMenuPaper_c::pushCursor() {
    if (isBtn(mCursor)) {
        mBtn.push(mCursor, FALSE);
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x7A0
BOOL dMenuPaper_c::isPushEnd() {
    if (isBtn(mCursor)) {
        return !mBtn.isPushing();
    }
    return TRUE;
}

// .text:0x7F8
void dMenuPaper_c::finishPush() {
    if (isBtn(mCursor)) {
        startCloseAnm();
    } else {
        setSubState(SUB_MAIN);
    }
}

// Opens the menu that follows this one (the tab's menu or the item menu).
// Returns TRUE when it could not be opened yet.
// .text:0x834
BOOL dMenuPaper_c::openNextMenu() {
    int next;
    switch (mMenuNo) {
    case dMenuMng_c::MENU_READ_LETTER_8:
        next = dAcUitag::MENU_B_AFTER_READ_8;
        break;
    case dMenuMng_c::MENU_READ_LETTER_9:
        next = dAcUitag::MENU_B_AFTER_READ_9;
        break;
    case dMenuMng_c::MENU_READ_LETTER:
        dAcUitag::openMenuA(dAcUitag::MENU_A_ITEM_1);
        return FALSE;
    default:
        return FALSE;
    }
    if (!dAcUitag::openMenuB(next)) {
        return TRUE;
    }
    return FALSE;
}
