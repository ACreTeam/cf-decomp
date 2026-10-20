#include <game/game/d_menu_notice01.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <string.h>

namespace dMenuNotice01 {
// .text:0x5C
BASE_PROFILE(MENU_NOTICE01, dMenuNotice01_c)
} // namespace dMenuNotice01

static const char *l_allPane = "N_all";

// .text:0x100
int dMenuNotice01_c::create() {
    if (!isFlag(FLAG_INIT)) {
        onFlag(FLAG_INIT);
        mKbd.init(dKbd::Mng_c::INIT_MODE_PAD);
    }
    BOOL loaded = mBtn.load();
    loaded &= mKbd.loadRes();
    loaded &= mKbdCur.load();
    loaded &= mLcrsr.load();
    loaded &= mSlctB.load();
    loaded &= mSlctC.load();
    loaded &= mRes.load("Layout/ltr_mark.arc");
    if (!loaded) {
        return NOT_READY;
    }
    if (!mKbd.create()) {
        return NOT_READY;
    }
    mBtn.create();
    initBtnText();
    mKbd.setup(dKbd::Mng_c::SETUP_MODE_1);
    mKbd.setMaxLength(KBD_MAX_LENGTH);
    mKbd.hideToolBar();
    mKbd.setMemoMode1();
    mKbd.buildLayout();
    mKbdCur.build();
    mLcrsr.create();
    mSlctB.create();
    mSlctC.create();
    initMarks();
    onFlag(FLAG_DRAW);
    mLine = 0;
    dMenuNotice_c::setPriority(LYT_PRIORITY);
    dMenuNotice_c::initNew();
    updateLineArrows();
    startAnm(&mBtn.mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    onFlag(FLAG_PAPER);
    setSubState(SUB_MAIN);
    memset(mText, 0, sizeof(mText));
    memset(mOldText, 0, sizeof(mOldText));
    resetMove();
    return SUCCEEDED;
}

// .text:0x2D8
int dMenuNotice01_c::doDelete() {
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mKbdCur.remove()) {
        return NOT_READY;
    }
    if (!mKbd.remove(dKbd::Mng_c::REMOVE_MODE_SAVE_SETTING)) {
        return NOT_READY;
    }
    if (!mLcrsr.remove()) {
        return NOT_READY;
    }
    if (!mSlctB.remove()) {
        return NOT_READY;
    }
    if (!mSlctC.remove()) {
        return NOT_READY;
    }
    if (!mRes.remove()) {
        return NOT_READY;
    }
    dMenuNotice_c::setPriority(dNotice_c::LYT_PRIORITY);
    return SUCCEEDED;
}

// .text:0x3B8
int dMenuNotice01_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x3DC
int dMenuNotice01_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mLcrsr.setPos(&mBtn.mLyt, l_allPane);
    if (isFlag(FLAG_PAPER)) {
        dMenuNotice_c::copyPos(&mBtn.mLyt, l_allPane);
        dMenuNotice_c::setPosX(mPos.x);
        dMenuNotice_c::setHidden(FALSE);
    }
    mVec3_c pos;
    m2d::getScreenPos(&pos, mBtn.mLyt.findPane(l_allPane));
    mKbd.setPos(pos);
    if (isState(STATE_MAIN)) {
        onFlag(FLAG_CURSOR);
        updateCursorPos();
    } else {
        offFlag(FLAG_CURSOR);
    }
    return result;
}

// .text:0x4DC
int dMenuNotice01_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x500
int dMenuNotice01_c::execute() {
    bool inAnm = isState(STATE_ANM);
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prev) {
        playSeCursor();
    }
    mBtn.execute();
    mLcrsr.execute();
    if (mLcrsr.isAnm()) {
        mVec3_c pos = mLcrsr.getPos();
        mKbd.setPos(pos);
    }
    if (!inAnm || !isFlag(FLAG_PAPER)) {
        calcMove();
    }
    if (isFlag(FLAG_TEXT)) {
        offFlag(FLAG_TEXT);
        dMenuNotice_c::setSelect(mKbd.getInputPos(), mKbd.getInputLength());
        dMenuNotice_c::updateText(isFlag(FLAG_MARKS));
        updateCursorPos();
    }
    mKbd.calc();
    mSlctB.execute();
    mSlctC.execute();
    return result;
}

// .text:0x658
int dMenuNotice01_c::draw() {
    if (dMenuNotice_c::isUpdated()) {
        onFlag(FLAG_CALC_MARKS);
    }
    if (isFlag(FLAG_CALC_MARKS)) {
        calcMarks();
        offFlag(FLAG_CALC_MARKS);
    }
    if (isFlag(FLAG_MARKS)) {
        drawMarks();
    }
    if (isFlag(FLAG_DRAW)) {
        mKbd.entry();
        mBtn.draw();
        mLcrsr.draw();
    }
    if (isFlag(FLAG_CURSOR)) {
        mKbdCur.draw();
    }
    if (isFlag(FLAG_SLCT_B)) {
        mSlctB.draw();
    }
    if (isFlag(FLAG_SLCT_C)) {
        mSlctC.draw();
    }
    return SUCCEEDED;
}

// .text:0x760
void dMenuNotice01_c::subStateMain() {
    BOOL hover = FALSE;
    if (!isFlag(FLAG_MARKS)) {
        onFlag(FLAG_MARKS);
        onFlag(FLAG_TEXT);
    }
    setSelect(mCursor, FALSE);
    int btn = mBtn.checkPad(!dSaveOption::getKeySe(), TRUE);
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
                return;
            }
            hover = TRUE;
        }
        if (!hitPaper()) {
            mKbd.open(hover, 0);
            if (mKbd.isTextChanged()) {
                if (checkText()) {
                    mKbd.updateInput();
                    markTextChanged();
                    updateLine(FALSE);
                } else {
                    dKbd::playSeCanNotInput();
                    revertText();
                }
            }
        }
    }
}

// .text:0x8D0
void dMenuNotice01_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x90C
void dMenuNotice01_c::subStateLineHold() {
    if (isMoveEnd()) {
        if (mPad::g_currentCore->down(WPAD_BUTTON_A)) {
            if (!moveLine()) {
                setSubState(SUB_MAIN);
            }
        } else {
            setSubState(SUB_MAIN);
        }
    }
}

// .text:0x9A8
void dMenuNotice01_c::subStatePreview() {
    if (isMoveEnd()) {
        mBtn.clearSelect();
        BOOL trigger = mPad::g_currentCore->downTrigger(WPAD_BUTTON_A);
        int btn = mBtn.checkPad(TRUE, TRUE);
        if (btn == -1) {
            if (dObjCursor_c::isVisible()) {
                mVec2_c pos = dObjCursor_c::getPos();
                btn = mBtn.hitCheck(&pos);
            }
        } else {
            trigger = TRUE;
        }
        if (btn != -1) {
            mBtn.setSelect(btn, TRUE, TRUE);
            if (trigger) {
                mBtn.push(btn, FALSE);
                setSubState(SUB_PREVIEW_PUSH);
            }
        }
    }
}

// .text:0xAA8
void dMenuNotice01_c::subStatePreviewPush() {
    if (!mBtn.isPushing()) {
        switch (mBtn.mPushedBtn) {
        case BTN_DONE:
            backToEdit();
            break;
        default:
            backToEdit();
            break;
        }
    }
}

// .text:0xB04
void dMenuNotice01_c::subStateReturn() {
    if (isMoveEnd()) {
        offFlag(FLAG_SLCT_B);
        setSubState(SUB_MAIN);
    }
}

// .text:0xB58
void dMenuNotice01_c::subStateQuit() {
    if (isMoveEnd()) {
        if (mSlctB.checkInput()) {
            setSubState(SUB_QUIT_PUSH);
        }
    }
}

// .text:0xBB4
void dMenuNotice01_c::subStateQuitPush() {
    if (mSlctB.isPushEnd()) {
        mSlctB.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
        switch (mSlctB.mPushedBtn) {
        case SLCT_BTN_YES:
            startClose();
            break;
        case SLCT_BTN_NO:
            setSubState(SUB_QUIT_END);
            break;
        default:
            setSubState(SUB_QUIT_END);
            break;
        }
    }
}

// .text:0xC5C
void dMenuNotice01_c::subStateQuitEnd() {
    if (mSlctB.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_SLCT_B);
    }
}

// .text:0xCB8
void dMenuNotice01_c::subStateDone() {
    if (isMoveEnd()) {
        if (mSlctC.checkInput()) {
            setSubState(SUB_DONE_PUSH);
        }
    }
}

// .text:0xD14
void dMenuNotice01_c::subStateDonePush() {
    if (mSlctC.isPushEnd()) {
        mSlctC.open(dMenuSlctC_c::WIN_ANM_OUT);
        switch (mSlctC.mPushedBtn) {
        case SLCT_BTN_CONFIRM:
            finish();
            break;
        case SLCT_BTN_EDIT:
            backToEdit();
            mBtn.startAnm(dMenuBtn_c::BTN_ANM_NO_PUSH);
            break;
        default:
            backToEdit();
            break;
        }
    }
}

// .text:0xDA0
void dMenuNotice01_c::subStateDoneEnd() {
    if (mSlctC.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_SLCT_C);
    }
}

// .text:0xDFC
void dMenuNotice01_c::subStateCloseWait() {
    if (isMoveEnd()) {
        if (mSlctC.isWinAnmEnd()) {
            setState(STATE_CLOSE);
        }
    }
}

// .text:0xE4C
void dMenuNotice01_c::executeSubState() {
    typedef void (dMenuNotice01_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuNotice01_c::subStateMain,
        &dMenuNotice01_c::subStatePush,
        &dMenuNotice01_c::subStateLineHold,
        &dMenuNotice01_c::subStatePreview,
        &dMenuNotice01_c::subStatePreviewPush,
        &dMenuNotice01_c::subStateReturn,
        &dMenuNotice01_c::subStateQuit,
        &dMenuNotice01_c::subStateQuitPush,
        &dMenuNotice01_c::subStateQuitEnd,
        &dMenuNotice01_c::subStateDone,
        &dMenuNotice01_c::subStateDonePush,
        &dMenuNotice01_c::subStateDoneEnd,
        &dMenuNotice01_c::subStateCloseWait,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xE84
void dMenuNotice01_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isLine(cursor)) {
        mLcrsr.setSelect(cursor - CURSOR_LINE, select);
    }
}

// .text:0xF28
int dMenuNotice01_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn;
    }
    mVec2_c linePos = pos;
    int line = mLcrsr.hitCheck(&linePos);
    if (line != -1) {
        return line + CURSOR_LINE;
    }
    return CURSOR_NONE;
}

// .text:0xFB4
void dMenuNotice01_c::pushCursor() {
    if (isBtn(mCursor)) {
        mBtn.push(mCursor, FALSE);
    } else if (isLine(mCursor)) {
        int dir = mCursor - CURSOR_LINE;
        if (canMoveLine(dir)) {
            mLcrsr.push(dir);
        }
    } else {
        return;
    }
    mKbd.fixInput();
    onFlag(FLAG_TEXT);
    setSubState(SUB_PUSH);
}

// .text:0x1090
BOOL dMenuNotice01_c::isPushEnd() {
    if (isBtn(mCursor)) {
        return !mBtn.isPushing();
    }
    if (isLine(mCursor)) {
        return !mLcrsr.isPushing();
    }
    return TRUE;
}

// .text:0x111C
void dMenuNotice01_c::finishPush() {
    if (isBtn(mCursor)) {
        switch (mCursor) {
        case BTN_QUIT:
            openQuitDialog();
            break;
        case BTN_PREVIEW:
            startPreview();
            break;
        case BTN_DONE:
            openDoneDialog();
            break;
        }
    } else if (isLine(mCursor)) {
        if (moveLine()) {
            setSubState(SUB_LINE_HOLD);
        } else {
            setSubState(SUB_MAIN);
        }
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x1230
void dMenuNotice01_c::startClose() {
    startAnm(&mBtn.mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuNotice_c::requestOpenMenu(dMenuNotice_c::OPEN_NEWEST);
    onFlag(FLAG_PAPER);
    offFlag(FLAG_MARKS);
    onFlag(FLAG_TEXT);
}

// .text:0x12A0
void dMenuNotice01_c::finish() {
    setSubState(SUB_CLOSE_WAIT);
    dMenuNotice_c::requestOpenMenu(dMenuNotice_c::OPEN_LIST);
    offFlag(FLAG_PAPER);
    dMenuMng_c::setResult(1);
    dMenuNotice_c::post();
    dMenuNotice_c::updateText(FALSE);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mErrand.notifyNoticePosted();
    startHomeMove(0.0f);
}

// .text:0x1320
void dMenuNotice01_c::startPreview() {
    offFlag(FLAG_MARKS);
    onFlag(FLAG_TEXT);
    initPreviewBtnText();
    startHomeMove(0.0f);
    mLcrsr.startAnm(dMenuLcrsr_c::FOCUS_ANM_IN);
    setSelect(mCursor, FALSE);
    mCursor = CURSOR_NONE;
    setSubState(SUB_PREVIEW);
}

// .text:0x13B0
void dMenuNotice01_c::backToEdit() {
    mLcrsr.startAnm(dMenuLcrsr_c::FOCUS_ANM_OUT);
    mKbd.setCursorPos(0);
    updateLine(TRUE);
    initBtnText();
    mBtn.clearSelect();
    setSubState(SUB_RETURN);
}

// .text:0x1424
void dMenuNotice01_c::openQuitDialog() {
    mSlctB.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    mSlctB.setMsg(MSG_QUIT_WITHOUT_WRITING, nullptr);
    mSlctB.setBtn(SLCT_BTN_NO, MSG_NO, nullptr, SE_cancel_little);
    mSlctB.setBtn(SLCT_BTN_YES, MSG_YES, nullptr, 0);
    mSlctB.setBtnVisible(SLCT_BTN_UNUSED, false);
    setSubState(SUB_QUIT);
    onFlag(FLAG_SLCT_B);
}

// .text:0x14D4
void dMenuNotice01_c::openDoneDialog() {
    offFlag(FLAG_MARKS);
    onFlag(FLAG_TEXT);
    mSlctC.open(dMenuSlctC_c::WIN_ANM_IN);
    mSlctC.setVisible(FALSE);
    startHomeMove(0.0f);
    mLcrsr.startAnm(dMenuLcrsr_c::FOCUS_ANM_IN);
    mBtn.startAnm(dMenuBtn_c::BTN_ANM_CAN_PUSH);
    mSlctC.setBtn(SLCT_BTN_EDIT, MSG_EDIT, SE_cancel_little);
    mSlctC.setBtn(SLCT_BTN_CONFIRM, MSG_CONFIRM, 0);
    setSubState(SUB_DONE);
    onFlag(FLAG_SLCT_C);
}

// .text:0x159C
void dMenuNotice01_c::initBtnText() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setText(BTN_PREVIEW, MSG_PREVIEW, FALSE);
    mBtn.setText(BTN_DONE, MSG_DONE, FALSE);
}

// .text:0x1600
void dMenuNotice01_c::initPreviewBtnText() {
    mBtn.setVisible(BTN_QUIT, false);
    mBtn.setVisible(BTN_PREVIEW, false);
    mBtn.setText(BTN_DONE, MSG_BACK, TRUE);
}

// .text:0x165C
// The keyboard's text was changed: takes it over if it fits on the paper.
bool dMenuNotice01_c::checkText() {
    static wchar_t text[KBD_MAX_LENGTH];
    if (!mKbd.getComposedText(text, TEXT_SIZE)) {
        return false;
    }
    if (!dMenuNotice_c::setText(text)) {
        return false;
    }
    memcpy(mText, text, TEXT_BYTES);
    memcpy(mOldText, mKbd.getText(), TEXT_BYTES);
    return true;
}

// .text:0x16F4
void dMenuNotice01_c::markTextChanged() {
    onFlag(FLAG_TEXT);
    onFlag(FLAG_CALC_MARKS);
}

// .text:0x1730
void dMenuNotice01_c::updateLineArrows() {
    mLcrsr.setDisable(DIR_UP, !canMoveLine(DIR_UP));
    mLcrsr.setDisable(DIR_DOWN, !canMoveLine(DIR_DOWN));
}

// .text:0x1794
void dMenuNotice01_c::revertText() {
    mKbd.fixInput();
    memcpy(mText, mOldText, TEXT_BYTES);
    mKbd.setText(mOldText);
    mKbd.resetCursor();
    dMenuNotice_c::setText(mText);
    markTextChanged();
}

// .text:0x17F8
bool dMenuNotice01_c::hitPaper() {
    if (!mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        return false;
    }
    if (!dObjCursor_c::isVisible()) {
        return false;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    if (mKbd.isSignWindowOpen()) {
        if (pos.y < 80.0f) {
            return false;
        }
    } else {
        if (pos.y < 50.0f) {
            return false;
        }
    }
    int line = dMenuNotice_c::hitCheck(&pos);
    if (line == -1) {
        return false;
    }
    mKbd.fixInput();
    mKbd.setCursorPos(dMenuNotice_c::getPosAtScreenX(line, pos.x));
    memcpy(mText, mKbd.getText(), TEXT_BYTES);
    memcpy(mOldText, mKbd.getText(), TEXT_BYTES);
    onFlag(FLAG_TEXT);
    updateLine(FALSE);
    playSeCursor();
    return true;
}

// .text:0x1944
void dMenuNotice01_c::updateCursorPos() {
    mVec3_c pos = dMenuNotice_c::getPos(mKbd.getCursorPos());
    mKbdCur.setPos(&pos);
}

// .text:0x198C
int dMenuNotice01_c::getTextLine() {
    return dMenuNotice_c::getLine(mKbd.getCursorPos());
}

// .text:0x19B4
void dMenuNotice01_c::updateLine(BOOL force) {
    int line = getTextLine();
    if (force || line != mLine) {
        mLine = line;
        startLineMove(line);
        updateLineArrows();
    }
}

// .text:0x1A18
void dMenuNotice01_c::calcMove() {
    if (mMove.mCount >= 0) {
        if (mMove.calc()) {
            if (isFlag(FLAG_MOVING)) {
                onFlag(FLAG_CURSOR);
                updateCursorPos();
            }
            mPos.x = mTarget.x;
            mPos.y = mTarget.y;
        } else {
            mPos.x = mMove.mPos.x;
            mPos.y = mMove.mPos.y;
        }
        dMenuNotice_c::setPos(mPos.x, mPos.y);
    }
}

// .text:0x1AB4
void dMenuNotice01_c::resetMove() {
    mPos.x = -25.0f;
    mPos.y = 7.0f;
    mMove.reset();
    dMenuNotice_c::setPos(mPos.x, mPos.y);
}

// .text:0x1B08
void dMenuNotice01_c::startLineMove(int line) {
    f32 y = dMenuNotice_c::getLineY(line);
    mTarget.x = -25.0f;
    mTarget.y = mPos.y + (124.0f - y);
    offFlag(FLAG_CURSOR);
    onFlag(FLAG_MOVING);
    startMove();
}

// .text:0x1B7C
void dMenuNotice01_c::startHomeMove(f32 dy) {
    mTarget.x = 0.0f;
    mTarget.y = 7.0f + dy;
    offFlag(FLAG_CURSOR);
    offFlag(FLAG_MOVING);
    startMove();
}

// .text:0x1BDC
bool dMenuNotice01_c::canMoveLine(int dir) {
    switch (dir) {
    case DIR_UP:
        return mLine > 0;
    case DIR_DOWN:
        return dMenuNotice_c::getLineNum() > mLine + 1;
    }
    return false;
}

// .text:0x1C58
BOOL dMenuNotice01_c::moveLine() {
    int dir = mLcrsr.mPushedBtn;
    int line = mLine;
    if (!canMoveLine(dir)) {
        return FALSE;
    }
    switch (dir) {
    case DIR_UP:
        if (line > 0) {
            line--;
        } else {
            return FALSE;
        }
        break;
    case DIR_DOWN:
        if (line < LINE_LAST) {
            line++;
        } else {
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    mKbd.fixInput();
    mKbd.setCursorPos(dMenuNotice_c::getPosAtScreenX(line, mKbdCur.getPos().x));
    onFlag(FLAG_TEXT);
    updateLine(FALSE);
    playSePageTurn();
    return TRUE;
}

// .text:0x1D48
void dMenuNotice01_c::startMove() {
    mMove.set(&mPos, &mTarget, MOVE_MAX_STEPS, MOVE_MIN_STEPS, MOVE_DECEL,
              8.0f);
}

// .text:0x1D6C
void dMenuNotice01_c::initMarks() {
    for (int i = 0; i < MARK_NUM; i++) {
        mMark[i].build(&mRes, "ltr_mark_00.brlyt");
    }
}

// .text:0x1DD8
void dMenuNotice01_c::drawMarks() {
    for (int i = 0; i < MARK_NUM; i++) {
        mMark[i].entry();
    }
}

// .text:0x1E24
// Shows the mark at the end of every line: a return mark where the line ends
// in a line break, an end mark at the last line.
void dMenuNotice01_c::calcMarks() {
    for (int i = 0; i < MARK_NUM; i++) {
        mVec3_c pos;
        int kind = dMenuNotice_c::getLineEnd(&pos, i);
        if (kind == dNotice_c::LINE_END_NONE) {
            mMark[i].getRootPane()->SetVisible(false);
        } else {
            mMark[i].getRootPane()->SetVisible(true);
            mMark[i].getRootPane()->SetTranslate(pos);
            mMark[i].setVisible("return_00",
                                kind == dNotice_c::LINE_END_NEWLINE);
            mMark[i].setVisible("end_00", kind == dNotice_c::LINE_END_TEXT);
            mMark[i].calc();
        }
    }
}
