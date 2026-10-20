#include <game/game/d_menu_item06.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuItem06 {
// .text:0x5C
BASE_PROFILE(MENU_ITEM06, dMenuItem06_c)
} // namespace dMenuItem06

static const char *l_allPane = "N_all";

// .text:0xD0
int dMenuItem06_c::create() {
    if (!mRes.load("Layout/chest.arc")) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "chest_win_00.brlyt");
    mLyt.setMessage("T_btn_00", MSG_CLOSE, NULL);
    mLyt.calc();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_IN, STATE_MAIN, TRUE);
    playSeSubMenuOpen();
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x1A4
int dMenuItem06_c::doDelete() {
    bool removed = mRes.remove();
    return removed;
}

// .text:0x1D4
int dMenuItem06_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x1F8
int dMenuItem06_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x1FC
int dMenuItem06_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x220
int dMenuItem06_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && prev != mCursor) {
        playSeCursor();
    }
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    return result;
}

// .text:0x2B4
int dMenuItem06_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
    }
    return SUCCEEDED;
}

// .text:0x2F8
void dMenuItem06_c::subStateMain() {
    setSelect(mCursor, FALSE);
    int cursor = hitCheck();
    mCursor = cursor;
    if (cursor != CURSOR_NONE) {
        setSelect(cursor, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            pushCursor();
        }
    }
}

// .text:0x37C
void dMenuItem06_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x3B8
void dMenuItem06_c::executeSubState() {
    typedef void (dMenuItem06_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuItem06_c::subStateMain,
        &dMenuItem06_c::subStatePush,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x3F0
void dMenuItem06_c::setSelect(int cursor, BOOL select) {
}

// .text:0x3F4
int dMenuItem06_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    mVec2_c hitPos = pos;
    int cursor = m2d::hitButton(&hitPos, &mLyt, CURSOR_SLIDER, CURSOR_CLOSE,
                                CURSOR_HOLE, CURSOR_HOLE_LAST, 1.0f);
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0x47C
void dMenuItem06_c::pushCursor() {
    int cursor = mCursor;
    if (isSlider(cursor) || cursor == CURSOR_CLOSE) {
        setSubState(SUB_PUSH);
    }
}

// .text:0x4C0
BOOL dMenuItem06_c::isPushEnd() {
    return TRUE;
}

// .text:0x4C8
void dMenuItem06_c::finishPush() {
    int cursor = mCursor;
    if (isSlider(cursor)) {
        startCloseAnm();
    } else if (cursor == CURSOR_CLOSE) {
        startCloseAnm();
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x510
void dMenuItem06_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_OUT, STATE_CLOSE, TRUE);
    playSeSubMenuClose();
    dMenuMng_c::setResult(RESULT_CANCEL);
}
