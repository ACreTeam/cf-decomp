#include <game/game/d_menu_star01.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_save_data.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuStar01 {
// .text:0x5C
BASE_PROFILE(MENU_STAR01, dMenuStar01_c)
} // namespace dMenuStar01

static const char *l_allPane = "N_all";

// .text:0x1F0
int dMenuStar01_c::create() {
    BOOL loaded = mStar.load();
    loaded &= mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    mBtn.create();
    mBtn.setPriority(dMenuStar_c::LYT_PRIORITY);
    initBtn();
    mStar.create();
    onFlag(FLAG_DRAW);
    startAnm(&mBtn.mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    mStar.startWinAnm(dMenuStar_c::WIN_ANM_IN);
    setSubState(SUB_MAIN);
    dObjTutorial_c::setPriority(dMenuStar_c::LYT_PRIORITY);
    return SUCCEEDED;
}

// .text:0x2C4
int dMenuStar01_c::doDelete() {
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    if (mStar.remove() == FALSE) {
        return NOT_READY;
    }
    dObjTutorial_c::setPriority(BALLOON_PRIORITY_DEFAULT);
    return SUCCEEDED;
}

// .text:0x328
int dMenuStar01_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x34C
int dMenuStar01_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x350
int dMenuStar01_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x374
int dMenuStar01_c::execute() {
    int cursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != cursor) {
        playSeCursor();
    }
    mBtn.execute();
    mStar.execute();
    return result;
}

// .text:0x3E4
int dMenuStar01_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mStar.draw();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x430
void dMenuStar01_c::subMain() {
    setSelect(mCursor, FALSE);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        mCursor = btn;
        setSelect(mCursor, TRUE);
        push();
    } else {
        mCursor = hitCheck();
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                push();
                return;
            }
        }
        checkPad();
    }
}

// .text:0x4F8
void dMenuStar01_c::subPush() {
    if (isPushEnd()) {
        onPushEnd();
    }
}

// .text:0x534
void dMenuStar01_c::subHold() {
    BOOL held = isFlag(FLAG_CLICK) ? mPad::g_currentCore->down(WPAD_BUTTON_A)
                                   : isPadHeld();
    if (held) {
        mStar.move(mCursor - CURSOR_STAR);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x634
void dMenuStar01_c::executeSubState() {
    typedef void (dMenuStar01_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuStar01_c::subMain,
        &dMenuStar01_c::subPush,
        &dMenuStar01_c::subHold,
    };
    updateSign();
    (this->*subStateProcs[mSubState])();
}

// .text:0x680
void dMenuStar01_c::initBtn() {
    mBtn.setVisible(BTN_UNUSED_0, false);
    mBtn.setVisible(BTN_UNUSED_1, false);
    mBtn.setText(BTN_BACK, MSG_BACK, 0);
}

// .text:0x6DC
void dMenuStar01_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isStarBtn(cursor)) {
        mStar.setSelect(cursor - CURSOR_STAR, select);
    }
}

// .text:0x780
int dMenuStar01_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn;
    }
    btn = mStar.hitCheck(&pos);
    if (btn != -1) {
        return btn + CURSOR_STAR;
    }
    return CURSOR_NONE;
}

// .text:0x7FC
void dMenuStar01_c::updateSign() {
    hideSignName();
    mVec2_c pos(0.0f, 0.0f);
    int sign = mStar.mStar.findSign(&pos, 50.0f);
    if (sign != -1) {
        showSignName(sign);
    }
}

// .text:0x860
void dMenuStar01_c::checkPad() {
    if (dPad::isUpTriggered() && mStar.canZoomIn()) {
        setSelect(mCursor, FALSE);
        mCursor = CURSOR_ZOOM_IN;
        push();
        offFlag(FLAG_CLICK);
    } else if (dPad::isDownTriggered() && mStar.canZoomOut()) {
        setSelect(mCursor, FALSE);
        mCursor = CURSOR_ZOOM_OUT;
        push();
        offFlag(FLAG_CLICK);
    } else if (dPad::isLeftTriggered()) {
        setSelect(mCursor, FALSE);
        mCursor = CURSOR_TURN_B;
        push();
        offFlag(FLAG_CLICK);
    } else if (dPad::isRightTriggered()) {
        setSelect(mCursor, FALSE);
        mCursor = CURSOR_TURN_A;
        push();
        offFlag(FLAG_CLICK);
    }
}

// .text:0x994
void dMenuStar01_c::push() {
    if (isBtn(mCursor)) {
        mBtn.push(mCursor, FALSE);
    } else if (isStarBtn(mCursor)) {
        mStar.push(mCursor - CURSOR_STAR);
        onFlag(FLAG_CLICK);
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xA48
BOOL dMenuStar01_c::isPushEnd() {
    if (isBtn(mCursor)) {
        return !mBtn.isPushing();
    }
    if (isStarBtn(mCursor)) {
        return mStar.isBtnAnmEnd(mCursor - CURSOR_STAR);
    }
    return TRUE;
}

// .text:0xAD0
void dMenuStar01_c::onPushEnd() {
    if (isBtn(mCursor)) {
        close();
    } else if (isStarBtn(mCursor)) {
        setSubState(SUB_HOLD);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xB40
void dMenuStar01_c::close() {
    startAnm(&mBtn.mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    mStar.startWinAnm(dMenuStar_c::WIN_ANM_OUT);
    dMenuMng_c::setResult(dMenuMng_c::RESULT_CANCEL);
    hideSignName();
}

// .text:0xBA0
void dMenuStar01_c::showSignName(int sign) {
    if (sign != -1 && dSaveData_c::getTown()->getStarSigns()->isValid(sign)) {
        if (dObjTutorial_c::request(dObjTutorial_c::REQUEST_STAR_SIGN, sign,
                                    FALSE)) {
            dSvStarSign::Sign_c signCopy =
                *dSaveData_c::getTown()->getStarSignList()->get(sign);
            m2d::stringWord_c word;
            word.clear();
            word.set(signCopy.getName(), 0);
            mVec2_c pos(0.0f, 15.0f);
            dObjTutorial_c::setWord(&pos, &word, 0);
        }
    } else {
        hideSignName();
    }
}

// .text:0xE50
void dMenuStar01_c::hideSignName() {
    dObjTutorial_c::hide();
}
