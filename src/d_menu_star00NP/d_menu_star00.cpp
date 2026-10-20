#include <game/game/d_menu_star00.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/SC/scapi.h>

namespace dMenuStar00 {
// .text:0x5C
BASE_PROFILE(MENU_STAR00, dMenuStar00_c)
} // namespace dMenuStar00

// .text:0x208
int dMenuStar00_c::create() {
    BOOL loaded = mChart.load();
    loaded &= mBtn.load();
    loaded &= mSlctB.load();
    loaded &= mSlctC.load();
    if (!loaded) {
        return NOT_READY;
    }
    initSign();
    mBtn.create();
    mBtn.setPriority(dMenuStar_c::LYT_PRIORITY);
    initButtons();
    mSlctB.create();
    mSlctC.create();
    mChart.create();
    if (mMenuNo == dMenuMng_c::MENU_STAR_SIGN_EDIT) {
        mChart.setAngle(&mSign);
    }
    onFlag(FLAG_DRAW);
    setState(STATE_ANM);
    setNextState(STATE_MAIN);
    mBtn.startAnm(dMenuBtn_c::BTN_ANM_NO_PUSH);
    mChart.startWinAnm(dMenuStar_c::WIN_ANM_IN);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x320
int dMenuStar00_c::doDelete() {
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mChart.remove()) {
        return NOT_READY;
    }
    if (!mSlctB.remove()) {
        return NOT_READY;
    }
    bool removed = mSlctC.remove() != 0;
    return removed;
}

// .text:0x3A4
int dMenuStar00_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x3C8
int dMenuStar00_c::stateAnm() {
    if (!mBtn.isAnm() && mChart.isWinAnmEnd() && mChart.isCursorAnmEnd() &&
        !mChart.isTurning()) {
        changeNextState();
    }
    return SUCCEEDED;
}

// .text:0x43C
int dMenuStar00_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x460
int dMenuStar00_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    int cursor = mCursor;
    if (cursor != CURSOR_NONE && cursor != prev) {
        playSeCursor();
    }
    mBtn.execute();
    mChart.execute();
    mSlctB.execute();
    mSlctC.execute();
    return result;
}

// .text:0x4E0
int dMenuStar00_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mChart.draw();
        mBtn.draw();
    }
    if (isFlag(FLAG_SLCT_C)) {
        mSlctC.draw();
    }
    if (isFlag(FLAG_SLCT_B)) {
        mSlctB.draw();
    }
    return SUCCEEDED;
}

// .text:0x564
void dMenuStar00_c::subStateMain() {
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
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A) &&
                pushCursor()) {
                return;
            }
        } else if (isCursorOnChart()) {
            executeMode();
        }
        checkPadMove();
    }
}

// .text:0x64C
void dMenuStar00_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

inline bool dMenuStar00_c::isChartBtnHeld() {
    if (isFlag(FLAG_POINTER)) {
        return mPad::g_currentCore->down(WPAD_BUTTON_A);
    }
    bool held = false;
    if ((mCursor == CURSOR_CHART_ZOOM_IN && dPad::isUpHeld()) ||
        (mCursor == CURSOR_CHART_ZOOM_OUT && dPad::isDownHeld()) ||
        (mCursor == CURSOR_CHART_TURN_RIGHT && dPad::isLeftHeld()) ||
        (mCursor == CURSOR_CHART_TURN_LEFT && dPad::isRightHeld())) {
        held = true;
    }
    return held;
}

// .text:0x688
void dMenuStar00_c::subStateHold() {
    if (isChartBtnHeld()) {
        mChart.move(mCursor - CURSOR_CHART);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x788
void dMenuStar00_c::subStateOkOpen() {
    mSlctC.open(dMenuSlctC_c::WIN_ANM_IN);
    mSlctC.setMsg(MSG_IS_OK);
    mSlctC.setBtn(0, MSG_NO, SE_cancel_little);
    mSlctC.setBtn(1, MSG_YES, 0);
    setSubState(SUB_OK_WAIT);
    onFlag(FLAG_SLCT_C);
}

// .text:0x814
void dMenuStar00_c::subStateOkWait() {
    if (mSlctC.checkInput()) {
        setSubState(SUB_OK_PUSH);
    }
}

// .text:0x864
void dMenuStar00_c::subStateOkPush() {
    if (mSlctC.isPushEnd()) {
        switch (mSlctC.mPushedBtn) {
        case 0:
            setSubState(SUB_OK_CLOSE);
            mChart.mStar.mWhiteLines = false;
            mSlctC.open(dMenuSlctC_c::WIN_ANM_OUT);
            mChart.mTargetScale = 1.0f;
            break;
        case 1:
            mSlctC.open(dMenuSlctC_c::WIN_ANM_OUT);
            startClose(FALSE);
            dMenuMng_c::setResult(1);
            dSaveData_c::getTown()->getStarSignList()->set(mSlot, &mSign);
            onFlag(FLAG_DONE);
            break;
        }
    }
}

// .text:0x938
void dMenuStar00_c::subStateOkClose() {
    if (mSlctC.isWinAnmEnd()) {
        setState(STATE_ANM);
        setNextState(STATE_MAIN);
        mBtn.startAnm(dMenuBtn_c::BTN_ANM_NO_PUSH);
        mChart.setCursorAnm(dMenuStar_c::CURSOR_ANM_START);
        setSubState(SUB_MAIN);
    }
}

// .text:0x9B8
void dMenuStar00_c::subStateQuitWait() {
    if (mSlctB.isWinAnmEnd() && mSlctB.checkInput()) {
        setSubState(SUB_QUIT_PUSH);
    }
}

// .text:0xA18
void dMenuStar00_c::subStateQuitPush() {
    if (mSlctB.isPushEnd()) {
        setSubState(SUB_QUIT_CLOSE);
        mSlctB.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0xA7C
void dMenuStar00_c::subStateQuitClose() {
    if (mSlctB.isWinAnmEnd()) {
        offFlag(FLAG_SLCT_B);
        switch (mSlctB.mPushedBtn) {
        case 0:
            setSubState(SUB_MAIN);
            break;
        case 1:
            startClose(TRUE);
            break;
        }
    }
}

// .text:0xB04
void dMenuStar00_c::executeSubState() {
    typedef void (dMenuStar00_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuStar00_c::subStateMain,     &dMenuStar00_c::subStatePush,
        &dMenuStar00_c::subStateHold,     &dMenuStar00_c::subStateOkOpen,
        &dMenuStar00_c::subStateOkWait,   &dMenuStar00_c::subStateOkPush,
        &dMenuStar00_c::subStateOkClose,  &dMenuStar00_c::subStateQuitWait,
        &dMenuStar00_c::subStateQuitPush, &dMenuStar00_c::subStateQuitClose,
    };
    u16 prevLine = mHoverLine;
    if (prevLine != dSvStarSign::SIGN_LINE_NONE) {
        mChart.mStar.mSelLine = -1;
        mHoverLine = dSvStarSign::SIGN_LINE_NONE;
    }
    int prevStar = mHoverStar;
    mHoverStar = -1;
    (this->*subStateProcs[mSubState])();
    if ((mHoverLine != dSvStarSign::SIGN_LINE_NONE && prevLine != mHoverLine) ||
        (mHoverStar != -1 && prevStar != mHoverStar)) {
        playSePointing();
    }
}

// .text:0xBB8
void dMenuStar00_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
    updateConfirmBtn();
}

// .text:0xC20
void dMenuStar00_c::updateConfirmBtn() {
    int num = getLineNum();
    mBtn.setDisable(BTN_CONFIRM, num == 0);
}

// .text:0xC64
void dMenuStar00_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isChart(cursor)) {
        mChart.setSelect(cursor - CURSOR_CHART, select);
    }
}

// .text:0xD08
int dMenuStar00_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn;
    }
    int chartBtn = mChart.hitCheck(&pos);
    if (chartBtn != -1) {
        return chartBtn + CURSOR_CHART;
    }
    return CURSOR_NONE;
}

// .text:0xD84
void dMenuStar00_c::checkPadMove() {
    if (dPad::isUpTriggered() && mChart.canZoomIn()) {
        setSelect(mCursor, FALSE);
        mCursor = CURSOR_CHART_ZOOM_IN;
        pushCursor();
        offFlag(FLAG_POINTER);
    } else if (dPad::isDownTriggered() && mChart.canZoomOut()) {
        setSelect(mCursor, FALSE);
        mCursor = CURSOR_CHART_ZOOM_OUT;
        pushCursor();
        offFlag(FLAG_POINTER);
    } else if (dPad::isLeftTriggered()) {
        setSelect(mCursor, FALSE);
        mCursor = CURSOR_CHART_TURN_RIGHT;
        pushCursor();
        offFlag(FLAG_POINTER);
    } else if (dPad::isRightTriggered()) {
        setSelect(mCursor, FALSE);
        mCursor = CURSOR_CHART_TURN_LEFT;
        pushCursor();
        offFlag(FLAG_POINTER);
    }
}

// .text:0xEB8
BOOL dMenuStar00_c::pushCursor() {
    if (isBtn(mCursor)) {
        if (mCursor == BTN_QUIT) {
            mBtn.push(BTN_QUIT, FALSE);
        } else {
            mBtn.push(BTN_CONFIRM, TRUE);
            playSeConfirm();
        }
    } else if (isChart(mCursor)) {
        mChart.push(mCursor - CURSOR_CHART);
        onFlag(FLAG_POINTER);
    } else {
        return FALSE;
    }
    setSubState(SUB_PUSH);
    return TRUE;
}

// .text:0xF9C
BOOL dMenuStar00_c::isPushEnd() {
    if (isBtn(mCursor)) {
        return !mBtn.isPushing();
    }
    return TRUE;
}

// .text:0xFF4
void dMenuStar00_c::finishPush() {
    if (isBtn(mCursor)) {
        switch (mCursor) {
        case BTN_QUIT:
            openQuitDialog();
            break;
        case BTN_UNUSED:
            break;
        case BTN_CONFIRM:
            startConfirm();
            break;
        }
        return;
    }
    if (isChart(mCursor)) {
        setSubState(SUB_HOLD);
        return;
    }
    setSubState(SUB_MAIN);
}

// .text:0x1094
void dMenuStar00_c::startClose(BOOL btnOut) {
    setState(STATE_ANM);
    setNextState(STATE_CLOSE);
    if (btnOut) {
        mBtn.startAnm(dMenuBtn_c::BTN_ANM_CAN_PUSH);
    }
    mChart.startWinAnm(dMenuStar_c::WIN_ANM_OUT);
    dMenuMng_c::setResult(0);
}

// .text:0x1104
void dMenuStar00_c::openQuitDialog() {
    mSlctB.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    setSubState(SUB_QUIT_WAIT);
    onFlag(FLAG_SLCT_B);
    mSlctB.setMsg(mMenuNo == dMenuMng_c::MENU_STAR_SIGN_NEW ? MSG_QUIT_NEW
                                                            : MSG_QUIT_EDIT,
                  NULL);
    mSlctB.setBtn(0, MSG_NO, NULL, SE_cancel_little);
    mSlctB.setBtn(1, MSG_YES, NULL, 0);
    mSlctB.setBtnVisible(2, false);
}

// .text:0x11C4
void dMenuStar00_c::startConfirm() {
    setState(STATE_ANM);
    setNextState(STATE_MAIN);
    mBtn.startAnm(dMenuBtn_c::BTN_ANM_CAN_PUSH);
    mChart.setCursorAnm(dMenuStar_c::CURSOR_ANM_FINISH);
    setSubState(SUB_OK_OPEN);
    mChart.mStar.mWhiteLines = true;
    clearLineStar();
    mChart.mTargetScale = 0.0f;
    mChart.startTurn(&mSign);
}

// .text:0x1258
void dMenuStar00_c::initSign() {
    mSign.clear();
    switch (mMenuNo) {
    case dMenuMng_c::MENU_STAR_SIGN_NEW:
        mSlot = dSaveData_c::getTown()->mStarSigns.findFree();
        mMode = MODE_START;
        dMenuMng_c::setPickedStarSign(mSlot);
        break;
    case dMenuMng_c::MENU_STAR_SIGN_EDIT:
        mSlot = dMenuMng_c::getPickedStarSign();
        mSign.copy(dSaveData_c::getTown()->mStarSigns.get(mSlot));
        mMode = MODE_CHANGE;
        break;
    }
    mSign.mOwner = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID;
    mChart.mStar.mSignIdx = mSlot;
    mChart.mStar.mSign = &mSign;
    mChart.mStar.mSelStar = -1;
    mChart.mStar.mSelLine = -1;
    mChart.mStar.mWhiteLines = false;
    mHoverLine = dSvStarSign::SIGN_LINE_NONE;
}

// .text:0x13DC
BOOL dMenuStar00_c::isCursorOnChart() {
    if (!dObjCursor_c::isVisible()) {
        return FALSE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    // The 16:9 screen is wider than the chart's 4:3 coordinates (26 / 19).
    if (SCGetAspectRatio() == 1) {
        pos.x *= 1.368421f;
    }
    // The chart's circle has the radius 220.
    if (pos.x * pos.x + pos.y * pos.y < 48400.0f) {
        return TRUE;
    }
    return FALSE;
}

// .text:0x1468
void dMenuStar00_c::executeMode() {
    typedef void (dMenuStar00_c::*Proc)();
    static Proc modeProcs[MODE_NUM] = {
        &dMenuStar00_c::modeStart,
        &dMenuStar00_c::modeLine,
        &dMenuStar00_c::modeChange,
        &dMenuStar00_c::modeFull,
    };
    (this->*modeProcs[mMode])();
}

// .text:0x14A0
void dMenuStar00_c::modeStart() {
    if (pickStar()) {
        return;
    }
}

// .text:0x14C0
void dMenuStar00_c::modeLine() {
    int line = findLinkedLine();
    if (line != (u16)dSvStarSign::SIGN_LINE_NONE) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            addLine(line);
            if (getLineNum() >= dSvStarSign::SIGN_LINE_NUM) {
                setMode(MODE_FULL);
                clearLineStar();
            } else {
                const dStarDraw_c::line_c *(*getLine)(int) =
                    &dStarDraw_c::getLine;
                const dStarDraw_c::line_c *l = getLine(line);
                if (mLineStar == l->mStar1) {
                    setLineStar(l->mStar2);
                } else {
                    setLineStar(l->mStar1);
                }
            }
        }
    } else if (!pickStar()) {
        if (eraseLine()) {
            return;
        }
    }
}

// .text:0x15AC
BOOL dMenuStar00_c::pickStar() {
    int star = findStar();
    if (star != -1) {
        setHoverStar(star);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            setMode(MODE_LINE);
            setLineStar(star);
            playSeStarChoice();
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x1648
void dMenuStar00_c::modeChange() {
    if (getLineNum() >= dSvStarSign::SIGN_LINE_NUM) {
        setMode(MODE_FULL);
    } else if (pickStar()) {
        return;
    }
    eraseLine();
}

// .text:0x16A4
BOOL dMenuStar00_c::eraseLine() {
    u16 line = findSignLine();
    if (line != dSvStarSign::SIGN_LINE_NONE) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            removeLine(line);
            clearLineStar();
            if (getLineNum() == 0) {
                setMode(MODE_START);
            } else {
                setMode(MODE_CHANGE);
            }
        } else {
            mHoverLine = line;
            mChart.mStar.mSelLine = line;
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x1768
void dMenuStar00_c::modeFull() {
    eraseLine();
}

// .text:0x176C
int dMenuStar00_c::getLineNum() {
    int num = 0;
    for (int i = 0; i < dSvStarSign::SIGN_LINE_NUM; i++) {
        if (mSign.mLines[i] != dSvStarSign::SIGN_LINE_NONE) {
            num++;
        }
    }
    return num;
}

// .text:0x1810
u16 dMenuStar00_c::findLinkedLine() {
    u16 line = dSvStarSign::SIGN_LINE_NONE;
    mVec2_c pos = dObjCursor_c::getPos();
    int star = mChart.mStar.findLinkedStar(
        mLineStar, &pos, &line, dStarDraw_c::isLineSelectable, LINK_RADIUS);
    if (line != dSvStarSign::SIGN_LINE_NONE) {
        setHoverStar(star);
        return line;
    }
    line = mChart.mStar.findLine(mLineStar, &pos, dStarDraw_c::isLineSelectable,
                                 LINE_RADIUS);
    if (line != dSvStarSign::SIGN_LINE_NONE) {
        mChart.mStar.mSelLine = line;
        mHoverLine = line;
    }
    return line;
}

// .text:0x18D0
u16 dMenuStar00_c::findSignLine() {
    mVec2_c pos = dObjCursor_c::getPos();
    return mChart.mStar.findSignLine(&pos, dStarDraw_c::isLineRemovable, NULL,
                                     LINE_RADIUS);
}

// .text:0x1920
void dMenuStar00_c::setLineStar(int star) {
    mLineStar = star;
    mChart.mStar.mSelStar = star;
}

// .text:0x192C
void dMenuStar00_c::clearLineStar() {
    mLineStar = -1;
    mChart.mStar.mSelStar = -1;
}

// .text:0x193C
void dMenuStar00_c::addLine(u16 line) {
    const u16 *p = mSign.mLines;
    for (int i = 0; i < dSvStarSign::SIGN_LINE_NUM; p++, i++) {
        if (*p == dSvStarSign::SIGN_LINE_NONE) {
            mSign.setLine(i, line);
            updateConfirmBtn();
            playSeLineChoice();
            break;
        }
    }
}

// .text:0x19AC
void dMenuStar00_c::removeLine(u16 line) {
    const u16 *p = mSign.mLines;
    for (int i = 0; i < dSvStarSign::SIGN_LINE_NUM; p++, i++) {
        if (line == *p) {
            mSign.clearLine(i);
            updateConfirmBtn();
            playSeLineCancel();
            break;
        }
    }
}

// .text:0x1A1C
int dMenuStar00_c::findStar() {
    mVec2_c pos = dObjCursor_c::getPos();
    return mChart.mStar.findStar(&pos, dStarDraw_c::isStarSelectable,
                                 STAR_RADIUS);
}

// .text:0x1A68
void dMenuStar00_c::setHoverStar(int star) {
    mVec2_c pos;
    mChart.mStar.getStarScreenPos(star, &pos);
    mChart.setCursorPos(&pos);
    mHoverStar = star;
}

// .text:0x1AB8
void dMenuStar00_c::setMode(int mode) {
    mMode = mode;
}

// .text:0x1AC0
void dMenuStar00_c::playSeStarChoice() {
    playSe(SE_Seiza_StarChoice);
}

// .text:0x1AC8
void dMenuStar00_c::playSeLineChoice() {
    playSe(SE_Seiza_LineChoice);
}

// .text:0x1AD0
void dMenuStar00_c::playSeLineCancel() {
    playSe(SE_Seiza_LineCancel);
}

// .text:0x1AD8
void dMenuStar00_c::playSeConfirm() {
    playSe(SE_Seiza_Decide);
}

// .text:0x1AE0
void dMenuStar00_c::playSePointing() {
    playSe(SE_Seiza_PointingStarLine);
}
