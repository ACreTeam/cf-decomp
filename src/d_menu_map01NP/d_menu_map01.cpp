#include <game/game/d_menu_map01.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuMap01 {
// .text:0x5C
BASE_PROFILE(MENU_MAP01, dMenuMap01_c)
} // namespace dMenuMap01

// .text:0x268
int dMenuMap01_c::create() {
    BOOL loaded = load();
    loaded &= mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    createLayouts();
    changeList(TRUE);
    select(ENTRY_TOWN_HALL);
    mBtn.create();
    initBtn();
    startAnm(&mWinLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    mWinLyt.calc();
    return SUCCEEDED;
}

// .text:0x320
int dMenuMap01_c::doDelete() {
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    return remove();
}

// .text:0x368
int dMenuMap01_c::stateMain() {
    preMain();
    executeSubState();
    postMain();
    return SUCCEEDED;
}

// .text:0x3A8
int dMenuMap01_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(&mWinLyt, "N_all");
    onFlag(FLAG_BTN_POS);
    return result;
}

// .text:0x404
int dMenuMap01_c::execute() {
    int result = dMenuMap_c::execute();
    mBtn.execute();
    return result;
}

// .text:0x448
int dMenuMap01_c::draw() {
    int result = dMenuMap_c::draw();
    if (isFlag(FLAG_BTN_POS)) {
        offFlag(FLAG_BTN_POS);
    }
    if (isFlag(FLAG_DRAW)) {
        mBtn.draw();
    }
    return result;
}

// .text:0x4C0
void dMenuMap01_c::subMain() {
    mBtn.clearSelect();
    BOOL pushed = mPad::g_currentCore->downTrigger(WPAD_BUTTON_A);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        pushed = TRUE;
    }
    if (btn == -1) {
        btn = hitCheckBtn();
    }
    if (btn != -1) {
        mCursor = SLOT_BTN + btn;
        mBtn.setSelect(btn, TRUE, FALSE);
        if (pushed) {
            mBtn.push(btn, FALSE);
            setSubState(SUB_CLOSE);
        }
    } else {
        mCursor = SLOT_NONE;
    }
}

// .text:0x5B4
void dMenuMap01_c::subClose() {
    if (!mBtn.isPushing()) {
        startAnm(&mWinLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
    }
}

// .text:0x60C
void dMenuMap01_c::executeSubState() {
    typedef void (dMenuMap01_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuMap01_c::subMain,
        &dMenuMap01_c::subClose,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x644
void dMenuMap01_c::preMain() {
}

// .text:0x648
void dMenuMap01_c::postMain() {
}

// .text:0x64C
void dMenuMap01_c::initBtn() {
    mBtn.setVisible(0, false);
    mBtn.setVisible(1, false);
    mBtn.setText(2, MSG_OK, 0);
}

// .text:0x6A8
int dMenuMap01_c::hitCheckBtn() {
    mVec2_c pos = dObjCursor_c::getPos();
    return mBtn.hitCheck(&pos);
}
