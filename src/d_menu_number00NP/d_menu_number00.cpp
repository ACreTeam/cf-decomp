#include <game/game/d_menu_number00.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuNumber00 {
void *classInit();
fProf::fBaseProfile_c g_profile_MENU_NUMBER00 = {
    classInit, fProf::MENU_NUMBER00, fProf::DRAW_ORDER::MENU_NUMBER00};

// .text:0x5C
void *classInit() {
    return new dMenuNumber00_c;
}
} // namespace dMenuNumber00

// .text:0xE0
int dMenuNumber00_c::create() {
    BOOL loaded = mRes.load("Layout/item_win.arc");
    loaded = loaded & mNumber.load();
    loaded &= mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    if (!mNumber.create(dMenuNumber_c::MODE_AMOUNT)) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "itm_win_00.brlyt");
    mLyt.calc();
    mBtn.create();
    initButtons();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, "N_itmWin_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    mCursor = CURSOR_NONE;
    return SUCCEEDED;
}

// .text:0x1F0
int dMenuNumber00_c::doDelete() {
    if (!mNumber.remove()) {
        return NOT_READY;
    }
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    bool removed = mRes.remove();
    return removed;
}

// .text:0x25C
int dMenuNumber00_c::stateMain() {
    calcWinAnm();
    executeSubState();
    calcLayout();
    return SUCCEEDED;
}

// .text:0x29C
int dMenuNumber00_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mNumber.setPos(&mLyt, "N_itmWin_all");
    mBtn.setPos(&mLyt, "N_itmWin_all");
    return result;
}

// .text:0x304
int dMenuNumber00_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x328
int dMenuNumber00_c::execute() {
    int result = dMenuBase_c::execute();
    mNumber.execute(FALSE);
    mBtn.execute();
    return result;
}

// .text:0x378
int dMenuNumber00_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mNumber.draw();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x3CC
void dMenuNumber00_c::subStateMain() {
    if (mCursor != CURSOR_NONE) {
        setSelect(mCursor, FALSE);
    }
    int cursor = hitCheck();
    mCursor = cursor;
    if (cursor != CURSOR_NONE) {
        setSelect(cursor, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            pushCursor();
        }
    }
}

// .text:0x458
void dMenuNumber00_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x494
void dMenuNumber00_c::executeSubState() {
    typedef void (dMenuNumber00_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {&dMenuNumber00_c::subStateMain,
                                          &dMenuNumber00_c::subStatePush};
    (this->*subStateProcs[mSubState])();
}

// .text:0x4CC
void dMenuNumber00_c::calcWinAnm() {
}

// .text:0x4D0
void dMenuNumber00_c::calcLayout() {
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
}

// .text:0x528
int dMenuNumber00_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    int key = mNumber.hitCheck();
    mVec2_c pos = dObjCursor_c::getPos();
    if (key != dMenuNumber_c::KEY_NONE) {
        return key;
    }
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn + CURSOR_BTN;
    }
    return CURSOR_NONE;
}

// .text:0x5B0
void dMenuNumber00_c::setSelect(int cursor, BOOL select) {
    if (isKey(cursor)) {
        int anm = dMenuNumber_c::KEY_ANM_OUT;
        if (select != FALSE) {
            anm = dMenuNumber_c::KEY_ANM_IN;
        }
        mNumber.setCursor(cursor, anm);
    }
    if (isButton(cursor)) {
        mBtn.setSelect(cursor - CURSOR_BTN, select, FALSE);
    }
}

// .text:0x660
void dMenuNumber00_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_NEXT, MSG_NEXT, FALSE);
}

// .text:0x6C0
void dMenuNumber00_c::pushCursor() {
    if (isKey(mCursor)) {
        mNumber.push(mCursor);
    }
    if (isButton(mCursor)) {
        mBtn.push(mCursor - CURSOR_BTN, FALSE);
    }
    setSubState(SUB_PUSH);
}

// .text:0x760
BOOL dMenuNumber00_c::isPushEnd() {
    int cursor = mCursor;
    if (isKey(cursor)) {
        return !mNumber.isPushing();
    }
    if (isButton(cursor)) {
        return !mBtn.isPushing();
    }
    return TRUE;
}

// .text:0x7EC
void dMenuNumber00_c::finishPush() {
    int cursor = mCursor;
    if (isButton(cursor)) {
        switch (cursor - CURSOR_BTN) {
        case BTN_QUIT:
            startAnm(&mLyt, "N_itmWin_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
            return;
        case BTN_NEXT:
        default:
            break;
        }
    }
    setSubState(SUB_MAIN);
}
