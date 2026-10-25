#include <game/game/d_menu_write.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <string.h>

namespace dMenuWrite {
// .text:0x5C
BASE_PROFILE(MENU_WRITE, dMenuWrite_c)
} // namespace dMenuWrite

static const char *l_allPane = "N_all";

// .text:0x128
int dMenuWrite_c::create() {
    if (!isFlag(FLAG_INIT)) {
        onFlag(FLAG_INIT);
        mpMail = dMenuMng_c::getMail();
        switch (mMenuNo) {
        case dMenuMng_c::MENU_WRITE_SELF:
            initSelfMail();
            break;
        case dMenuMng_c::MENU_WRITE_BOARD:
            initBoardMail();
            break;
        }
        mPaper.setMail(mpMail, TRUE);
        mKbd.init(dKbd::Mng_c::INIT_MODE_PAD);
    }
    BOOL loaded = mPaper.load();
    loaded &= mBtn.load();
    loaded &= mLcrsr.load();
    loaded &= mKbd.loadRes();
    loaded &= mKbdCur.load();
    loaded &= mSlctB.load();
    loaded &= mSlctC.load();
    loaded &= mRes.load("Layout/ltr_mark.arc");
    if (!loaded) {
        return NOT_READY;
    }
    if (!mKbd.create()) {
        return NOT_READY;
    }
    mKbd.setup(dKbd::Mng_c::SETUP_MODE_1);
    mKbd.setMaxLength(KBD_MAX_LENGTH);
    mKbd.hideToolBar();
    mKbd.setMemoMode1();
    mKbd.buildLayout();
    mPaper.create();
    mLcrsr.create();
    mBtn.create();
    initBtnText();
    mKbdCur.build();
    mSlctB.create();
    mSlctC.create();
    initMarks();
    onFlag(FLAG_DRAW);
    startAnm(mPaper.mpLayout, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    mLcrsr.startAnm(dMenuLcrsr_c::FOCUS_ANM_OUT);
    mBtn.startAnm(dMenuBtn_c::BTN_ANM_NO_PUSH);
    setSubState(SUB_MAIN);
    mPaper.setPosX(-25.0f);
    mPaper.execute();
    memset(mText, 0, sizeof(mText));
    memset(mOldText, 0, sizeof(mOldText));
    mPart = dPaper_c::PART_NONE;
    setPart(dPaper_c::PART_BODY);
    mLine = LINE_BODY_FIRST;
    updateLineArrows();
    resetMove();
    switch (mMenuNo) {
    case dMenuMng_c::MENU_WRITE_BOARD:
    case dMenuMng_c::MENU_WRITE_SELF:
        mPaper.setFooterAlign(0);
        break;
    }
    mZClear.set(240.0f, -240.0f, -320.0f, 320.0f, -499.9f);
    return SUCCEEDED;
}

// .text:0x3D4
int dMenuWrite_c::doDelete() {
    if (!mKbdCur.remove()) {
        return NOT_READY;
    }
    if (!mPaper.remove()) {
        return NOT_READY;
    }
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mLcrsr.remove()) {
        return NOT_READY;
    }
    if (!mKbd.remove(dKbd::Mng_c::REMOVE_MODE_SAVE_SETTING)) {
        return NOT_READY;
    }
    if (!mSlctB.remove()) {
        return NOT_READY;
    }
    if (!mSlctC.remove()) {
        return NOT_READY;
    }
    bool removed = mRes.remove();
    return removed;
}

// .text:0x4BC
int dMenuWrite_c::stateMain() {
    if (handleCloseRequest()) {
        return SUCCEEDED;
    }
    mCursorPos = mKbd.getCursorPos();
    mTextLength = mKbd.getTextLength();
    executeSubState();
    return SUCCEEDED;
}

// .text:0x51C
int dMenuWrite_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mPaper.calcAnm();
    if (isState(STATE_ANM)) {
        offFlag(FLAG_CURSOR);
    } else {
        onFlag(FLAG_CURSOR);
        updateCursorPos();
        onFlag(FLAG_CALC_MARKS);
        onFlag(FLAG_MARKS);
        onFlag(FLAG_TEXT);
    }
    return result;
}

// .text:0x5BC
int dMenuWrite_c::stateClose() {
    if (isFlag(FLAG_QUIT)) {
        deleteRequest();
        return SUCCEEDED;
    }
    if (mMenuNo == dMenuMng_c::MENU_WRITE_LETTER) {
        dAcUitag::openMenuA(dAcUitag::MENU_A_ITEM);
        dAcUitag::showTab();
    } else if (mMenuNo == dMenuMng_c::MENU_REWRITE_LETTER) {
        dAcUitag::openMenuA(dAcUitag::MENU_A_ITEM_1);
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x63C
int dMenuWrite_c::execute() {
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
    mKbd.calc();
    mSlctB.execute();
    mSlctC.execute();
    if (!inAnm) {
        calcMove();
    }
    if (isFlag(FLAG_TEXT)) {
        int length = mKbd.getInputLength();
        int pos = mKbd.getInputPos();
        mPaper.mSelectPart = mPart;
        mPaper.mSelectLength = length;
        mPaper.mSelectStart = pos;
        BOOL marks = isFlag(FLAG_MARKS);
        mPaper.updateText(marks, marks);
        offFlag(FLAG_TEXT);
        onFlag(FLAG_CALC_PAPER);
        updateCursorPos();
        onFlag(FLAG_CALC_MARKS);
    }
    if (isFlag(FLAG_CALC_PAPER)) {
        offFlag(FLAG_CALC_PAPER);
        mPaper.execute();
    }
    if (isFlag(FLAG_CALC_MARKS)) {
        calcMarks();
        offFlag(FLAG_CALC_MARKS);
    }
    return result;
}

// .text:0x7FC
int dMenuWrite_c::draw() {
    mZClear.entry();
    if (isFlag(FLAG_DRAW)) {
        mPaper.draw();
        if (isFlag(FLAG_MARKS)) {
            drawMarks();
        }
        mKbd.entry();
        mBtn.draw();
        mLcrsr.draw();
    }
    if (isFlag(FLAG_SLCT_B)) {
        mSlctB.draw();
    }
    if (isFlag(FLAG_SLCT_C)) {
        mSlctC.draw();
    }
    if (isFlag(FLAG_CURSOR)) {
        mKbdCur.draw();
    }
    return SUCCEEDED;
}

// .text:0x8D4
void dMenuWrite_c::subStateMain() {
    BOOL hover = FALSE;
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
                    saveText();
                    updateLine(FALSE);
                } else {
                    dKbd::playSeCanNotInput();
                    revertText();
                }
            }
        }
    }
}

// .text:0xA18
void dMenuWrite_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0xA54
void dMenuWrite_c::subStateLineHold() {
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

// .text:0xAF0
void dMenuWrite_c::subStatePreview() {
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
                onFlag(FLAG_MARKS_PENDING);
            }
        }
    }
}

// .text:0xBFC
void dMenuWrite_c::subStatePreviewPush() {
    if (mBtn.isPushing() || mBtn.mPushedBtn != BTN_DONE) {
        return;
    }
    backToEdit();
}

// .text:0xC4C
void dMenuWrite_c::subStateReturn() {
    if (isMoveEnd()) {
        offFlag(FLAG_SLCT_B);
        setSubState(SUB_MAIN);
    }
}

// .text:0xCA0
void dMenuWrite_c::subStateQuit() {
    if (isMoveEnd()) {
        if (mSlctB.checkInput()) {
            setSubState(SUB_QUIT_PUSH);
        }
    }
}

// .text:0xCFC
void dMenuWrite_c::subStateQuitPush() {
    if (mSlctB.isPushEnd()) {
        mSlctB.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
        switch (mSlctB.mPushedBtn) {
        case SLCT_BTN_YES:
            mLcrsr.startAnm(dMenuLcrsr_c::FOCUS_ANM_IN);
            startClose();
            mBtn.startAnm(dMenuBtn_c::BTN_ANM_CAN_PUSH);
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

// .text:0xDBC
void dMenuWrite_c::subStateQuitEnd() {
    if (mSlctB.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_SLCT_B);
    }
}

// .text:0xE18
void dMenuWrite_c::subStateDone() {
    if (isMoveEnd()) {
        if (mSlctC.checkInput()) {
            setSubState(SUB_DONE_PUSH);
        }
    }
}

// .text:0xE74
void dMenuWrite_c::subStateDonePush() {
    if (mSlctC.isPushEnd()) {
        mSlctC.open(dMenuSlctC_c::WIN_ANM_OUT);
        switch (mSlctC.mPushedBtn) {
        case SLCT_BTN_CONFIRM:
            startClose();
            finish();
            break;
        case SLCT_BTN_EDIT:
            backToEdit();
            mBtn.startAnm(dMenuBtn_c::BTN_ANM_NO_PUSH);
            onFlag(FLAG_MARKS_PENDING);
            break;
        default:
            backToEdit();
            break;
        }
    }
}

// .text:0xF14
void dMenuWrite_c::subStateDoneEnd() {
    if (mSlctC.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_SLCT_C);
    }
}

// .text:0xF70
void dMenuWrite_c::startQuit() {
    startClose();
    mBtn.startAnm(dMenuBtn_c::BTN_ANM_CAN_PUSH);
    finish();
    onFlag(FLAG_QUIT);
    dAcUitag::closeTab();
    dAcUitag::setHideAfterMenu();
}

// .text:0xFC4
BOOL dMenuWrite_c::onCloseMain() {
    mLcrsr.startAnm(dMenuLcrsr_c::FOCUS_ANM_IN);
    startQuit();
    return TRUE;
}

// .text:0x1004
BOOL dMenuWrite_c::onClosePreview() {
    if (!isMoveEnd()) {
        return FALSE;
    }
    startQuit();
    return TRUE;
}

// .text:0x103C
BOOL dMenuWrite_c::onCloseQuit() {
    if (!mSlctB.isWinAnmEnd()) {
        return FALSE;
    }
    mSlctB.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    return onCloseMain();
}

// .text:0x1098
BOOL dMenuWrite_c::onCloseDone() {
    if (!mSlctC.isWinAnmEnd()) {
        return FALSE;
    }
    mSlctC.open(dMenuSlctC_c::WIN_ANM_OUT);
    return onClosePreview();
}

// .text:0x10EC
BOOL dMenuWrite_c::handleCloseRequest() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_WRITE_LETTER:
    case dMenuMng_c::MENU_REWRITE_LETTER:
        break;
    default:
        return FALSE;
    }
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    typedef BOOL (dMenuWrite_c::*Proc)();
    static Proc closeProcs[SUB_NUM] = {
        &dMenuWrite_c::onCloseMain,    NULL, NULL,
        &dMenuWrite_c::onClosePreview, NULL, NULL,
        &dMenuWrite_c::onCloseQuit,    NULL, NULL,
        &dMenuWrite_c::onCloseDone,    NULL, NULL,
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

// .text:0x123C
void dMenuWrite_c::executeSubState() {
    typedef void (dMenuWrite_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuWrite_c::subStateMain,        &dMenuWrite_c::subStatePush,
        &dMenuWrite_c::subStateLineHold,    &dMenuWrite_c::subStatePreview,
        &dMenuWrite_c::subStatePreviewPush, &dMenuWrite_c::subStateReturn,
        &dMenuWrite_c::subStateQuit,        &dMenuWrite_c::subStateQuitPush,
        &dMenuWrite_c::subStateQuitEnd,     &dMenuWrite_c::subStateDone,
        &dMenuWrite_c::subStateDonePush,    &dMenuWrite_c::subStateDoneEnd,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x1274
void dMenuWrite_c::initBtnText() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setText(BTN_PREVIEW, MSG_PREVIEW, FALSE);
    mBtn.setText(BTN_DONE, MSG_DONE, FALSE);
}

// .text:0x12D8
void dMenuWrite_c::initPreviewBtnText() {
    mBtn.setVisible(BTN_QUIT, false);
    mBtn.setVisible(BTN_PREVIEW, false);
    mBtn.setText(BTN_DONE, MSG_BACK, TRUE);
}

// .text:0x1334
void dMenuWrite_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isLine(cursor)) {
        mLcrsr.setSelect(cursor - CURSOR_LINE, select);
    }
}

// .text:0x13D8
int dMenuWrite_c::hitCheck() {
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

// .text:0x1464
void dMenuWrite_c::pushCursor() {
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

// .text:0x1540
BOOL dMenuWrite_c::isPushEnd() {
    if (isBtn(mCursor)) {
        return !mBtn.isPushing();
    }
    if (isLine(mCursor)) {
        return !mLcrsr.isPushing();
    }
    return TRUE;
}

// .text:0x15CC
void dMenuWrite_c::finishPush() {
    if (isBtn(mCursor)) {
        switch (mBtn.mPushedBtn) {
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

// .text:0x16E4
void dMenuWrite_c::startClose() {
    offFlag(FLAG_MARKS);
    onFlag(FLAG_TEXT);
    startAnm(mPaper.mpLayout, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(0);
}

// .text:0x1748
void dMenuWrite_c::backToEdit() {
    mLcrsr.startAnm(dMenuLcrsr_c::FOCUS_ANM_OUT);
    setPart(dPaper_c::PART_BODY);
    mKbd.setCursorPos(0);
    updateLine(TRUE);
    initBtnText();
    mBtn.clearSelect();
    setSubState(SUB_RETURN);
}

// .text:0x17C8
void dMenuWrite_c::openQuitDialog() {
    mSlctB.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    mSlctB.setMsg(MSG_CLOSE_WITHOUT_SAVING, nullptr);
    mSlctB.setBtn(SLCT_BTN_NO, MSG_NO, nullptr, SE_cancel_little);
    mSlctB.setBtn(SLCT_BTN_YES, MSG_YES, nullptr, 0);
    mSlctB.setBtnVisible(SLCT_BTN_UNUSED, false);
    setSubState(SUB_QUIT);
    onFlag(FLAG_SLCT_B);
}

// .text:0x1878
void dMenuWrite_c::openDoneDialog() {
    mSlctC.open(dMenuSlctC_c::WIN_ANM_IN);
    mSlctC.setVisible(FALSE);
    startHomeMove();
    mLcrsr.startAnm(dMenuLcrsr_c::FOCUS_ANM_IN);
    mBtn.startAnm(dMenuBtn_c::BTN_ANM_CAN_PUSH);
    mSlctC.setBtn(SLCT_BTN_EDIT, MSG_EDIT, SE_cancel_little);
    mSlctC.setBtn(SLCT_BTN_CONFIRM, MSG_CONFIRM, SE_decide);
    setSubState(SUB_DONE);
    onFlag(FLAG_SLCT_C);
    offFlag(FLAG_MARKS);
    onFlag(FLAG_TEXT);
}

// .text:0x193C
void dMenuWrite_c::startPreview() {
    initPreviewBtnText();
    startHomeMove();
    mLcrsr.startAnm(dMenuLcrsr_c::FOCUS_ANM_IN);
    setSelect(mCursor, FALSE);
    mCursor = CURSOR_NONE;
    setSubState(SUB_PREVIEW);
    offFlag(FLAG_MARKS);
    onFlag(FLAG_TEXT);
}

// .text:0x19C4
void dMenuWrite_c::finish() {
    dMenuMng_c::setResult(1);
    switch (mMenuNo) {
    case dMenuMng_c::MENU_WRITE_BOARD:
        finishBoard();
        break;
    case dMenuMng_c::MENU_WRITE_SELF:
        finishSelf();
        break;
    case dMenuMng_c::MENU_WRITE_LETTER:
        finishWriteLetter();
        break;
    case dMenuMng_c::MENU_REWRITE_LETTER:
        finishRewriteLetter();
        break;
    }
}

// .text:0x1A4C
void dMenuWrite_c::saveText() {
    switch (mPart) {
    case dPaper_c::PART_HEADER:
    case dPaper_c::PART_HEADER_AFTER_NAME:
        memcpy(mpMail->mHeader, mText, HEADER_SIZE);
        break;
    case dPaper_c::PART_BODY:
        memcpy(mpMail->mBody, mText, BODY_SIZE);
        break;
    case dPaper_c::PART_FOOTER:
        memcpy(mpMail->mFooter, mText, HEADER_SIZE);
        break;
    }
    onFlag(FLAG_TEXT);
}

// .text:0x1AEC
// Keeps the place of the addressee's name in the header (mNamePos) behind the
// text that was typed in front of it.
void dMenuWrite_c::updateNamePos() {
    int length = m2d::Word_c::getLength(mText);
    int pos = mpMail->mNamePos + (length - mLength);
    if (pos < 0) {
        pos = 0;
    }
    int cursor = mKbd.getCursorPos();
    if (pos < cursor) {
        pos = cursor;
    }
    mpMail->mNamePos = pos;
    mLength = length;
}

// .text:0x1B6C
void dMenuWrite_c::updateLineArrows() {
    mLcrsr.setDisable(DIR_UP, !canMoveLine(DIR_UP));
    mLcrsr.setDisable(DIR_DOWN, !canMoveLine(DIR_DOWN));
}

// .text:0x1BD0
// The keyboard's text was changed: takes it over if it fits on the paper.
bool dMenuWrite_c::checkText() {
    static wchar_t text[KBD_MAX_LENGTH];
    if (!mKbd.getComposedText(text, mMaxLength + 1)) {
        return false;
    }
    switch (mPart) {
    case dPaper_c::PART_HEADER:
    case dPaper_c::PART_HEADER_AFTER_NAME:
        if (!mPaper.fitsHeader(text)) {
            return false;
        }
        break;
    case dPaper_c::PART_BODY:
        if (!mPaper.wrapBody(text, FALSE)) {
            return false;
        }
        break;
    case dPaper_c::PART_FOOTER:
        if (!mPaper.fitsFooter(text)) {
            return false;
        }
        break;
    }
    int cursor = mKbd.getCursorPos();
    int length = m2d::Word_c::getLength(text);
    int pos = mpMail->mNamePos + (length - mLength);
    if (pos < 0) {
        pos = 0;
    }
    if (mKbd.getInputLength() == 0) {
        if (mPart == dPaper_c::PART_HEADER_AFTER_NAME) {
            // The cursor leaves the text after the name to the front.
            bool change = false;
            if (cursor == 0 && (dPad::isUpTriggered() ||
                                mKbd.getSpecialKey() == SPECIAL_KEY_UP ||
                                ((dPad::isLeftTriggered() ||
                                  mKbd.getSpecialKey() == SPECIAL_KEY_LEFT) &&
                                 mCursorPos == 0))) {
                change = true;
            } else if (mpMail->mNamePos > cursor) {
                cursor = (u8)pos;
                change = true;
            }
            // The original compares the menu number twice.
            if (mMenuNo == dMenuMng_c::MENU_WRITE_BOARD ||
                mMenuNo == dMenuMng_c::MENU_WRITE_BOARD) {
                change = false;
            }
            if (change) {
                mKbd.clearEmotion();
                mPart = dPaper_c::PART_HEADER;
                mKbd.setCursorPos(cursor);
                updateCursorPos();
            }
        } else if (mPart == dPaper_c::PART_HEADER) {
            // ... or from the text in front of the name to the back.
            bool change = false;
            if (length == cursor &&
                (dPad::isDownTriggered() ||
                 mKbd.getSpecialKey() == SPECIAL_KEY_DOWN ||
                 ((dPad::isRightTriggered() ||
                   mKbd.getSpecialKey() == SPECIAL_KEY_RIGHT) &&
                  mCursorPos == length))) {
                change = true;
            } else if (pos < cursor) {
                cursor = mpMail->mNamePos;
                change = true;
            } else if (mpMail->mNamePos == 0 && pos == 0 && cursor == 0 &&
                       mCursorPos == 0 && mTextLength > length) {
                change = true;
            }
            if (change) {
                mKbd.clearEmotion();
                mPart = dPaper_c::PART_HEADER_AFTER_NAME;
                mKbd.setCursorPos(cursor);
                updateCursorPos();
            }
        }
    }
    memcpy(mText, text, BODY_SIZE);
    memcpy(mOldText, mKbd.getText(), BODY_SIZE);
    if (mPart == dPaper_c::PART_HEADER) {
        updateNamePos();
    } else if (mPart == dPaper_c::PART_HEADER_AFTER_NAME) {
        mLength = m2d::Word_c::getLength(mText);
    }
    if (cursor != mCursorPos) {
        mKbd.clearEmotion();
    }
    return true;
}

// .text:0x1F10
void dMenuWrite_c::revertText() {
    mKbd.fixInput();
    memcpy(mText, mOldText, BODY_SIZE);
    mKbd.setText(mOldText);
    mKbd.resetCursor();
    if (mPart == dPaper_c::PART_HEADER) {
        updateNamePos();
    } else if (mPart == dPaper_c::PART_HEADER_AFTER_NAME) {
        mLength = m2d::Word_c::getLength(mText);
    }
    saveText();
}

// .text:0x1F98
bool dMenuWrite_c::hitPaper() {
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
    int line = mPaper.hitCheck(&pos);
    if (line == -1) {
        return false;
    }
    mKbd.fixInput();
    u32 textPos = mPaper.getPosAtScreenX(line, pos.x);
    setPart(mPaper.getTextPart(&textPos, line));
    mKbd.setCursorPos(textPos);
    onFlag(FLAG_TEXT);
    updateLine(FALSE);
    playSeCursor();
    return true;
}

// .text:0x20DC
void dMenuWrite_c::updateCursorPos() {
    int pos = mKbd.getCursorPos();
    mVec3_c cursor;
    switch (mPart) {
    case dPaper_c::PART_HEADER:
        mPaper.getHeaderPos(&cursor, pos, FALSE);
        break;
    case dPaper_c::PART_HEADER_AFTER_NAME:
        mPaper.getHeaderPos(&cursor, pos, TRUE);
        break;
    case dPaper_c::PART_BODY:
        mPaper.getBodyPos(&cursor, pos, FALSE);
        break;
    case dPaper_c::PART_FOOTER:
        mPaper.getFooterPos(&cursor, pos);
        break;
    }
    mKbdCur.setPos(&cursor);
}

// .text:0x219C
int dMenuWrite_c::getTextLine() {
    switch (mPart) {
    case dPaper_c::PART_HEADER:
    case dPaper_c::PART_HEADER_AFTER_NAME:
        return dPaper_c::HEADER;
    case dPaper_c::PART_BODY:
        return mPaper.getBodyLine(mKbd.getCursorPos(), NULL, FALSE);
    case dPaper_c::PART_FOOTER:
        return dPaper_c::FOOTER;
    default:
        return dPaper_c::HEADER;
    }
}

// .text:0x2220
void dMenuWrite_c::updateLine(BOOL force) {
    int line = getTextLine();
    if (force || line != mLine) {
        mLine = line;
        startLineMove(line);
        updateLineArrows();
    }
}

// .text:0x2284
void dMenuWrite_c::setPart(int part) {
    mKbd.setKeyboardMode(part == dPaper_c::PART_BODY);
    mPart = part;
    switch (part) {
    case dPaper_c::PART_HEADER:
    case dPaper_c::PART_HEADER_AFTER_NAME:
        memcpy(mText, mpMail->mHeader, HEADER_SIZE);
        mMaxLength = MAIL_HEADER_LEN;
        mLength = m2d::Word_c::getLength(mText);
        break;
    case dPaper_c::PART_BODY:
        memcpy(mText, mpMail->mBody, BODY_SIZE);
        mMaxLength = MAIL_BODY_LEN;
        break;
    case dPaper_c::PART_FOOTER:
        memcpy(mText, mpMail->mFooter, HEADER_SIZE);
        mMaxLength = MAIL_FOOTER_LEN;
        break;
    }
    memcpy(mOldText, mText, BODY_SIZE);
    mKbd.fixInput();
    mKbd.setText(mOldText);
    mKbd.setCursorPos(0);
}

// .text:0x238C
void dMenuWrite_c::calcMove() {
    if (mMove.mCount >= 0) {
        if (mMove.calc()) {
            if (isFlag(FLAG_MOVING)) {
                offFlag(FLAG_MOVING);
                onFlag(FLAG_CURSOR);
                updateCursorPos();
            }
            if (isFlag(FLAG_MARKS_PENDING)) {
                offFlag(FLAG_MARKS_PENDING);
                onFlag(FLAG_MARKS);
                onFlag(FLAG_TEXT);
            }
            mPos.x = mTarget.x;
            mPos.y = mTarget.y;
        } else {
            mPos.x = mMove.mPos.x;
            mPos.y = mMove.mPos.y;
        }
        mPaper.setPos(mPos.x, mPos.y);
        onFlag(FLAG_CALC_PAPER);
        onFlag(FLAG_CALC_MARKS);
    }
}

// .text:0x2488
void dMenuWrite_c::resetMove() {
    mPos.x = -25.0f;
    mPos.y = 7.0f;
    mMove.reset();
}

// .text:0x24A8
void dMenuWrite_c::startLineMove(int line) {
    f32 y = mPaper.getLineY(line);
    mTarget.x = -25.0f;
    mTarget.y = mPos.y + (124.0f - y);
    offFlag(FLAG_CURSOR);
    onFlag(FLAG_MOVING);
    startMove();
}

// .text:0x251C
void dMenuWrite_c::startHomeMove() {
    mTarget.x = 0.0f;
    mTarget.y = 7.0f;
    offFlag(FLAG_CURSOR);
    offFlag(FLAG_MOVING);
    startMove();
}

// .text:0x2578
bool dMenuWrite_c::canMoveLine(int dir) {
    switch (dir) {
    case DIR_UP:
        return mLine > 0;
    case DIR_DOWN:
        return mLine < LINE_FOOTER;
    }
    return false;
}

// .text:0x25C8
BOOL dMenuWrite_c::moveLine() {
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
        if (line < LINE_FOOTER) {
            line++;
            if (line > LINE_BODY_FIRST && line <= dPaper_c::BODY_LINES) {
                if (!mPaper.hasBodyLine(line)) {
                    line = LINE_FOOTER;
                }
            }
        } else {
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    u32 pos = mPaper.getPosAtScreenX(line, mKbdCur.getPos().x);
    setPart(mPaper.getTextPart(&pos, line));
    mKbd.setCursorPos(pos);
    onFlag(FLAG_TEXT);
    updateLine(FALSE);
    playSePageTurn();
    return TRUE;
}

// .text:0x26FC
void dMenuWrite_c::startMove() {
    mMove.set(&mPos, &mTarget, MOVE_MAX_STEPS, MOVE_MIN_STEPS, MOVE_DECEL,
              8.0f);
}

// .text:0x2720
void dMenuWrite_c::initBoardMail() {
    mpMail->clear();
    dItem::Item paper(PAPER_ITEM);
    mpMail->setPaper(&paper);
    dString::Word_c word;
    word.set(dMenuMng_c::getFriendInfo(), 0);
    dString::setWord(0, &word);
    word.clear();
    dString::loadString(&word, MSG_TO, "sys_2D/SYS_2D_menu");
    memcpy(mpMail->mHeader, word.getText(), HEADER_SIZE);
    setTownWords();
    word.clear();
    dString::loadString(&word, MSG_FROM_PLAYER, "sys_2D/SYS_2D_menu");
    memcpy(mpMail->mFooter, word.getText(), HEADER_SIZE);
}

// .text:0x2830
void dMenuWrite_c::finishBoard() {
    dMenuMng_c::clearPost();
    setTownWords();
    dString::Word_c word;
    if (dMenuMng_c::isFriendWithKey()) {
        dString::loadString(&word, MSG_FROM_FRIEND_TOWN, "sys_2D/SYS_2D_menu");
    } else {
        dString::loadString(&word, MSG_FROM_PLAYER_TOWN, "sys_2D/SYS_2D_menu");
    }
    memcpy(dMenuMng_c::getPostTitle(), word.getText(), HEADER_SIZE);
    buildPostMsg(dMenuMng_c::isFriendWithKey());
    dMenuMng_c::setPostType(1);
}

// .text:0x28F8
void dMenuWrite_c::initSelfMail() {
    mpMail->clear();
    dItem::Item paper(PAPER_ITEM);
    mpMail->setPaper(&paper);
    dTime_c time = *dTime_c::getCurrent();
    dString::Word_c word;
    dScript::formatNumber(&word, time.year, 4, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(2, &word);
    word.clear();
    dScript::setMonthName(&word, time.mon);
    dString::setWord(3, &word);
    word.clear();
    dScript::setDayName(&word, time.mday);
    dString::setWord(4, &word);
    word.clear();
    dScript::formatNumber(&word, time.hour, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(5, &word);
    word.clear();
    dScript::formatNumber(&word, time.min, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(6, &word);
    word.clear();
    dString::loadString(&word, MSG_DATE, "sys_2D/SYS_2D_menu");
    memcpy(mpMail->mHeader, word.getText(), HEADER_SIZE);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dHmnName::Word_c name;
    player->mPID.setWord(&name);
    word.clear();
    word.copy(&name, 0);
#if VERSION != VERSION_RUUJ01_01 && VERSION != VERSION_RUUJ01_02 && VERSION != VERSION_RUUK01_01
    word.capitalize();
#endif
    memcpy(mpMail->mFooter, word.getText(),
           (PLAYER_NAME_LEN + 1) * sizeof(wchar_t));
}

// .text:0x2B20
void dMenuWrite_c::finishSelf() {
    dMenuMng_c::clearPost();
    dString::Word_c word;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dHmnName::Word_c name;
    player->mPID.setWord(&name);
    dString::setWord(1, &name);
    word.clear();
    dString::loadString(&word, MSG_FROM_PLAYER_B, "sys_2D/SYS_2D_menu");
    memcpy(dMenuMng_c::getPostTitle(), word.getText(),
           (MAIL_HEADER_LEN + 1) * sizeof(wchar_t));
    buildPostMsg(TRUE);
    dMenuMng_c::setPostType(0);
}

// .text:0x2BF4
// The Wii message: the header, a blank line, the body lines, padding to seven
// lines and the footer.
void dMenuWrite_c::buildPostMsg(BOOL friendKey) {
    wchar_t *msg = dMenuMng_c::getPostMsg();
    int pos = 0;
    int headerLength = m2d::Word_c::getLength(mpMail->mHeader);
    if (headerLength != 0) {
        memcpy(msg, mpMail->mHeader, headerLength * sizeof(wchar_t));
        pos = headerLength;
    }
    msg[pos] = L'\n';
    msg[pos + 1] = L'\n';
    pos += 2;
    int line;
    int src = 0;
    for (line = 0; line < dPaper_c::BODY_LINES; line++) {
        int length = mPaper.getBodyLineLength(line + 1);
        if (length == 0) {
            break;
        }
        memcpy(msg + pos, mpMail->mBody + src, length * sizeof(wchar_t));
        pos += length;
        src += length;
        if (!mPaper.isNewLine(line + 1)) {
            msg[pos++] = L'\n';
        }
    }
    int lines = mPaper.getBodyLineCount();
    if (msg[pos - 1] != L'\n') {
        lines--;
    }
    for (int i = lines; i < LINE_FOOTER; i++, pos++) {
        msg[pos] = L'\n';
    }
    int footerLength = m2d::Word_c::getLength(mpMail->mFooter);
    if (footerLength != 0) {
        memcpy(msg + pos, mpMail->mFooter, footerLength * sizeof(wchar_t));
        pos += footerLength;
    }
    msg[pos] = L'\n';
}

// .text:0x2E88
void dMenuWrite_c::setTownWords() {
    dHmnName::Word_c name;
    dLandNameWord_c town;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    player->mPID.setWord(&name);
    dString::setWord(1, &name);
    player->mPID.land.setWord(&town);
    dString::setWord(0, &town);
}

// .text:0x2F0C
// The letter is written: the stationery loses one use, the letter is stored and
// its greeting and signature are remembered.
void dMenuWrite_c::finishWriteLetter() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int slot = dMenuMng_c::getPickedLetter();
    dItem::Item item = player->mPockets[slot];
    int uses = item.getId() & 3;
    if (uses == 0) {
        item = dItem::ITEM_ID_NONE;
    } else {
        item = item.makeVariant(uses - 1);
    }
    player->setPocket(&item, slot, FALSE);
    int letter = player->findLetter();
    player->setLetter(mpMail, letter);
    dMail_c mail(*mpMail);
    player->mLetterStyle.store(&mail);
}

// .text:0x30E0
void dMenuWrite_c::finishRewriteLetter() {
    int slot = dMenuMng_c::getPickedLetter();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    player->setLetter(mpMail, slot);
    dMail_c mail(*mpMail);
    player->mLetterStyle.store(&mail);
}

// .text:0x3254
void dMenuWrite_c::initMarks() {
    for (int i = 0; i < MARK_NUM; i++) {
        mMark[i].build(&mRes, "ltr_mark_00.brlyt");
    }
}

// .text:0x32C0
void dMenuWrite_c::drawMarks() {
    for (int i = 0; i < MARK_NUM; i++) {
        mMark[i].entry();
    }
}

// .text:0x330C
// Shows the mark at the end of every body line: a return mark where the line
// ends in a line break, an end mark at the last line.
void dMenuWrite_c::calcMarks() {
    for (int i = 0; i < MARK_NUM; i++) {
        mVec3_c pos;
        int kind = mPaper.getBodyLineEnd(&pos, i);
        if (kind == 0) {
            mMark[i].getRootPane()->SetVisible(false);
        } else {
            mMark[i].getRootPane()->SetVisible(true);
            mMark[i].getRootPane()->SetTranslate(pos);
            mMark[i].setVisible("return_00", kind == 1);
            mMark[i].setVisible("end_00", kind == 2);
            mMark[i].calc();
        }
    }
}
