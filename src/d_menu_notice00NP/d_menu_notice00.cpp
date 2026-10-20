#include <game/game/d_menu_notice00.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_notice.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <stdio.h>

namespace dMenuNotice00 {
fProf::fBaseProfile_c g_profile_MENU_NOTICE00 = {
    classInit, fProf::MENU_NOTICE00, fProf::DRAW_ORDER::MENU_NOTICE00};

// .text:0x5C
void *classInit() {
    return new dMenuNotice00_c;
}
} // namespace dMenuNotice00

static const char *l_allPane = "N_all";

// .text:0x170
int dMenuNotice00_c::create() {
    BOOL loaded = mBtn.load();
    loaded &= mSlct.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(dMenuNotice_c::getResAcc(), "board_win_baase_00.brlyt");
    mCursorLyt.build(dMenuNotice_c::getResAcc(), "board_win_cursor_00.brlyt");
    initAnms();
    mLyt.calc();
    mCursorLyt.calc();
    mCursorLyt.mPriority = LYT_PRIORITY;
    mBtn.create();
    mSlct.create();
    initButtons();
    mNotice.build(dMenuNotice_c::getResAcc());
    setSubState(SUB_MAIN);
    initList();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, FALSE);
    playSeOpen();
    return SUCCEEDED;
}

// .text:0x2B0
int dMenuNotice00_c::doDelete() {
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    return mSlct.remove() != 0;
}

// .text:0x304
int dMenuNotice00_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x328
int dMenuNotice00_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(&mLyt, l_allPane);
    if (isFlag(FLAG_PAPER)) {
        dMenuNotice_c::copyPos(&mLyt, l_allPane);
    }
    mLyt.copyPos(&mCursorLyt, "N_all", "N_all");
    mCursorLyt.calc();
    return result;
}

// .text:0x3CC
int dMenuNotice00_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x3F0
int dMenuNotice00_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prev) {
        playSeCursor();
    }
    mBtn.execute();
    mSlct.execute();
    calcAnms();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
        mCursorLyt.calc();
    }
    return result;
}

// .text:0x4B0
int dMenuNotice00_c::draw() {
    if (isFlag(FLAG_NOTICE)) {
        mNotice.draw();
    }
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mBtn.draw();
        mCursorLyt.entry();
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    return SUCCEEDED;
}

// .text:0x53C
void dMenuNotice00_c::subStateMain() {
    setSelect(mCursor, FALSE);
    dObjTutorial_c::hide();
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        mCursor = btn;
        setSelect(btn, TRUE);
        pushCursor();
    } else if (dPad::isLeftTriggered() && isSelectable(CURSOR_PREV)) {
        mCursor = CURSOR_PREV;
        pushCursor();
    } else if (dPad::isRightTriggered() && isSelectable(CURSOR_NEXT)) {
        mCursor = CURSOR_NEXT;
        pushCursor();
    } else if (dPad::isUpTriggered() && isSelectable(CURSOR_FIRST)) {
        mCursor = CURSOR_FIRST;
        pushCursor();
    } else if (dPad::isDownTriggered() && isSelectable(CURSOR_NEWEST)) {
        mCursor = CURSOR_NEWEST;
        pushCursor();
    } else {
        int cursor = hitCheck();
        mCursor = cursor;
        if (cursor != CURSOR_NONE) {
            setSelect(cursor, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                pushCursor();
            } else {
                showTutorial(mCursor);
            }
        }
    }
}

// .text:0x6DC
void dMenuNotice00_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x718
void dMenuNotice00_c::subStateSlide() {
    if (dMenuNotice_c::isAnmEnd()) {
        offFlag(FLAG_NOTICE);
        showNotice();
        updateScrlBtnAnm();
        setSubState(SUB_MAIN);
    }
}

// .text:0x780
void dMenuNotice00_c::subStateDialogOpen() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_ANSWER);
    }
}

// .text:0x7E0
void dMenuNotice00_c::subStateDialogAnswer() {
    if (mSlct.isPushEnd()) {
        setSubState(SUB_DIALOG_CLOSE);
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0x844
void dMenuNotice00_c::subStateDialogClose() {
    if (mSlct.isWinAnmEnd()) {
        offFlag(FLAG_DIALOG);
        switch (mSlct.mPushedBtn) {
        case DIALOG_BTN_NO:
            setSubState(SUB_MAIN);
            break;
        case DIALOG_BTN_YES:
            dMenuNotice_c::removeNotice(mCurrent);
            updateDots();
            int count = mCount;
            int idx = mCurrent;
            if (count == 0) {
                idx = NO_NOTICE;
            } else if (idx >= count) {
                idx = count - 1;
            }
            changeNotice(idx);
            break;
        }
    }
}

// .text:0x8FC
void dMenuNotice00_c::executeSubState() {
    typedef void (dMenuNotice00_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuNotice00_c::subStateMain,
        &dMenuNotice00_c::subStatePush,
        &dMenuNotice00_c::subStateSlide,
        &dMenuNotice00_c::subStateDialogOpen,
        &dMenuNotice00_c::subStateDialogAnswer,
        &dMenuNotice00_c::subStateDialogClose,
    };
    (this->*subStateProcs[mSubState])();
}

// The player is away on a trip with the DS.
// .text:0x934
BOOL dMenuNotice00_c::isTravelling() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
        PRIVATE_FLAG0_TRAVELLING);
}

// .text:0x95C
void dMenuNotice00_c::initButtons() {
    mBtn.setText(BTN_CLOSE, MSG_CLOSE, TRUE);
    if ((!dNet::isSessionActive() || dNet::isHost()) && !isTravelling()) {
        mBtn.setText(BTN_ERASE, MSG_ERASE, FALSE);
    } else {
        mBtn.setVisible(BTN_ERASE, false);
    }
    mBtn.setText(BTN_WRITE, MSG_WRITE, FALSE);
}

// .text:0x9F8
void dMenuNotice00_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isDot(cursor)) {
        int dot = cursor - CURSOR_DOT;
        mDotAnm[dot].setNext(
            select
                ? DOT_ANM_IN
                : (dNotice::isNoticeRead(dot) ? DOT_ANM_READ : DOT_ANM_UNREAD));
    }
    if (isScrl(cursor)) {
        mScrlBtnAnm[cursor - CURSOR_PREV].setNext(select == FALSE);
    }
    if (isJump(cursor)) {
        mJumpAnm[cursor - CURSOR_FIRST].setNext(select == FALSE);
    }
}

// .text:0xB48
int dMenuNotice00_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn;
    }
    mVec2_c dotPos = pos;
    int cursor = m2d::hitButton(&dotPos, &mLyt, CURSOR_DOT, CURSOR_DOT_LAST, -1,
                                -1, 1.0f);
    if (cursor != -1) {
        return cursor;
    }
    mVec2_c arrowPos = pos;
    cursor = m2d::hitButton(&arrowPos, &mCursorLyt, CURSOR_PREV, CURSOR_NEWEST,
                            -1, -1, 1.0f);
    if (cursor == CURSOR_PREV && mCurrent <= 0) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_NEXT && mCurrent >= mCount - 1) {
        cursor = CURSOR_NONE;
    }
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0xC64
void dMenuNotice00_c::pushCursor() {
    if (isSelectable(mCursor)) {
        int cursor = mCursor;
        if (isBtn(cursor)) {
            mBtn.push(cursor, FALSE);
        } else if (isDot(cursor)) {
            startDotPush(cursor - CURSOR_DOT);
            playSePageTurn();
        } else if (isScrl(cursor)) {
            mScrlBtnAnm[cursor - CURSOR_PREV].setNext(SCRL_ANM_PUSH);
            playSePageTurn();
        } else if (isJump(cursor)) {
            mJumpPushAnm[cursor - CURSOR_FIRST].setFrame(0.0f);
            playSeJump();
        } else {
            return;
        }
        setSubState(SUB_PUSH);
    }
}

// .text:0xDB0
BOOL dMenuNotice00_c::isPushEnd() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        return !mBtn.isPushing();
    }
    if (isDot(cursor)) {
        return mDotPushAnm.mFrameCtrl.isStop();
    }
    if (isScrl(cursor)) {
        return mScrlBtnAnm[cursor - CURSOR_PREV].mFrameCtrl.isStop();
    }
    if (isJump(cursor)) {
        return mJumpPushAnm[cursor - CURSOR_FIRST].mFrameCtrl.isStop();
    }
    return TRUE;
}

// .text:0xEA4
void dMenuNotice00_c::finishPush() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        switch (cursor) {
        case BTN_CLOSE:
            closeMenu();
            break;
        case BTN_ERASE:
            openEraseDialog();
            break;
        case BTN_WRITE:
            openWrite();
            break;
        }
        return;
    } else if (isDot(cursor)) {
        changeNotice(cursor - CURSOR_DOT);
    } else if (isScrl(cursor)) {
        if (cursor == CURSOR_PREV) {
            changeNotice(mCurrent - 1);
        } else {
            changeNotice(mCurrent + 1);
        }
    } else if (isJump(cursor)) {
        if (cursor == CURSOR_FIRST) {
            changeNotice(0);
        } else {
            changeNotice(mCount - 1);
        }
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xFB8
void dMenuNotice00_c::closeMenu() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    playSeClose();
    dMenuNotice_c::requestClose();
    onFlag(FLAG_PAPER);
}

// .text:0x1014
void dMenuNotice00_c::openWrite() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    playSeClose();
    dMenuNotice_c::requestOpenMenu(dMenuNotice_c::OPEN_WRITE);
    onFlag(FLAG_PAPER);
}

// .text:0x1074
void dMenuNotice00_c::initList() {
    updateDots();
    mCurrent = mCount - 1;
    setDotFocus(mCurrent);
    if (mMenuNo == dMenuMng_c::MENU_BBS_NEWEST) {
        onFlag(FLAG_PAPER);
        showNotice();
    }
    updateScrlBtnAnm();
}

// .text:0x10DC
void dMenuNotice00_c::updateDots() {
    mCount = dNotice::getNoticeCount();
    mBtn.setDisable(BTN_ERASE, mCount == 0);
    for (int i = 0; i < DOT_NUM; i++) {
        char name[32];
        sprintf(name, "N_dot_%02d", i);
        mLyt.setVisible(name, i < mCount);
    }
}

// .text:0x117C
void dMenuNotice00_c::showNotice() {
    if (mCurrent == NO_NOTICE) {
        dMenuNotice_c::setHidden(TRUE);
    } else {
        dMenuNotice_c::setHidden(FALSE);
        dMenuNotice_c::setNotice(dNotice::getNotice(mCurrent));
        dNotice::setNoticeRead(mCurrent);
        mDotAnm[mCurrent].startEnd(DOT_ANM_READ);
    }
}

// .text:0x11F0
BOOL dMenuNotice00_c::isSelectable(int cursor) {
    if (isDot(cursor)) {
        return cursor - CURSOR_DOT != mCurrent;
    }
    if (cursor == CURSOR_PREV || cursor == CURSOR_FIRST) {
        return mCurrent > 0;
    }
    if (cursor == CURSOR_NEXT || cursor == CURSOR_NEWEST) {
        return mCurrent < mCount - 1;
    }
    return TRUE;
}

// .text:0x128C
void dMenuNotice00_c::changeNotice(int idx) {
    BOOL forward = mCurrent < idx;
    dMenuNotice_c::setNextAnm(forward ? dNotice_c::ANM_L_OUT
                                      : dNotice_c::ANM_R_OUT);
    int prev = mCurrent;
    mCurrent = idx;
    setDotFocus(idx);
    if (mCurrent != NO_NOTICE) {
        dNotice::setNoticeRead(mCurrent);
        if (mCurrent + CURSOR_DOT != mCursor) {
            setSelect(mCurrent + CURSOR_DOT, FALSE);
        }
    }
    setSelect(prev + CURSOR_DOT, FALSE);
    if (mCurrent != NO_NOTICE) {
        mNotice.setNotice(dNotice::getNotice(mCurrent));
        mNotice.startAnm(forward ? dNotice_c::ANM_R_IN : dNotice_c::ANM_L_IN,
                         TRUE);
        onFlag(FLAG_NOTICE);
    }
    onFlag(FLAG_CALC);
    setSubState(SUB_SLIDE);
}

// .text:0x13B4
void dMenuNotice00_c::initAnms() {
    static const char *dotNames[] = {
        "board_win_baase_00_dot_In.brlan",
        "board_win_baase_00_dot_midoku.brlan",
        "board_win_baase_00_dot_kidoku.brlan",
    };
    static const char *prevNames[] = {
        "board_win_cursor_00_scrlBtn_00_In.brlan",
        "board_win_cursor_00_scrlBtn_00_Out.brlan",
        "board_win_cursor_00_scrlBtn_00_NoPsh.brlan",
        "board_win_cursor_00_scrlBtn_00_CanPsh.brlan",
        "board_win_cursor_00_scrlBtn_00_Psh.brlan",
    };
    static const char *nextNames[] = {
        "board_win_cursor_00_scrlBtn_01_In.brlan",
        "board_win_cursor_00_scrlBtn_01_Out.brlan",
        "board_win_cursor_00_scrlBtn_01_NoPsh.brlan",
        "board_win_cursor_00_scrlBtn_01_CanPsh.brlan",
        "board_win_cursor_00_scrlBtn_01_Psh.brlan",
    };
    static const char *jumpNames[] = {
        "board_win_cursor_00_crsrFc_In.brlan",
        "board_win_cursor_00_crsrFc_Out.brlan",
    };
    for (int i = 0; i < DOT_NUM; i++) {
        mDotAnm[i].init(&mLyt, dMenuNotice_c::getResAcc(), dotNames,
                        DOT_ANM_NUM);
        mDotAnm[i].setNumber(i);
        if (dNotice::isNoticeRead(i)) {
            mDotAnm[i].startEnd(DOT_ANM_READ);
        } else {
            mDotAnm[i].startEnd(DOT_ANM_UNREAD);
        }
    }
    mLyt.setAnm(&mDotPushAnm, "board_win_baase_00_dot_Psh.brlan", nullptr);
    mDotPushAnm.setToEnd();
    mScrlBtnAnm[0].init(&mCursorLyt, dMenuNotice_c::getResAcc(), prevNames,
                        SCRL_ANM_NUM);
    mScrlBtnAnm[1].init(&mCursorLyt, dMenuNotice_c::getResAcc(), nextNames,
                        SCRL_ANM_NUM);
    for (int i = 0; i < JUMP_NUM; i++) {
        mJumpAnm[i].init(&mCursorLyt, dMenuNotice_c::getResAcc(), jumpNames,
                         JUMP_ANM_NUM);
        mJumpAnm[i].setNumber(i);
    }
    mCursorLyt.setAnm(&mJumpPushAnm[0], "board_win_cursor_00_crsr_00_Psh.brlan",
                      nullptr);
    mJumpPushAnm[0].setToEnd();
    mCursorLyt.setAnm(&mJumpPushAnm[1], "board_win_cursor_00_crsr_01_Psh.brlan",
                      nullptr);
    mJumpPushAnm[1].setToEnd();
}

// .text:0x1534
void dMenuNotice00_c::calcAnms() {
    int i;
    BOOL playing = FALSE;
    for (i = 0; i < DOT_NUM; i++) {
        playing |= mDotAnm[i].play();
    }
    if (!mDotPushAnm.mFrameCtrl.isStop()) {
        playing = TRUE;
        mDotPushAnm.play();
    }
    for (i = 0; i < SCRL_NUM; i++) {
        playing |= mScrlBtnAnm[i].play();
    }
    for (i = 0; i < JUMP_NUM; i++) {
        playing |= mJumpAnm[i].play();
        m2d::Anm_c *push = &mJumpPushAnm[i];
        if (!push->mFrameCtrl.isStop()) {
            playing = TRUE;
            push->play();
        }
    }
    if (playing) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x1628
void dMenuNotice00_c::startDotPush(int dot) {
    mDotPushAnm.setFrame(0.0f);
    char name[32];
    sprintf(name, "N_dot_%02d", dot);
    mLyt.setPosFrom(name, "dot_Psh", 0.0f, 0.0f);
    onFlag(FLAG_CALC);
}

// .text:0x16B4
void dMenuNotice00_c::updateScrlBtnAnm() {
    mScrlBtnAnm[0].start(isSelectable(CURSOR_PREV) ? SCRL_ANM_CAN_PUSH
                                                   : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].start(isSelectable(CURSOR_NEXT) ? SCRL_ANM_CAN_PUSH
                                                   : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[1].setToEnd();
    mCursorLyt.calc();
    offFlag(FLAG_CALC);
}

// .text:0x1750
void dMenuNotice00_c::setDotFocus(int dot) {
    if (dot == NO_NOTICE) {
        mLyt.setVisible("dotFc_00", false);
    } else {
        mLyt.setVisible("dotFc_00", true);
        char name[32];
        sprintf(name, "N_dot_%02d", dot);
        mLyt.setPosFrom(name, "dotFc_00", 0.0f, 0.0f);
    }
}

// .text:0x17F4
void dMenuNotice00_c::showTutorial(int cursor) {
    if (isJump(cursor)) {
        int jump = cursor - CURSOR_FIRST;
        u16 msg = jump + BALLOON_MSG_FIRST;
        if (dObjTutorial_c::request(dObjTutorial_c::REQUEST_MENU, msg, TRUE)) {
            char name[48];
            sprintf(name, "crsr_%02d", jump);
            mVec3_c pos;
            m2d::getScreenPos(&pos, mCursorLyt.findPane(name));
            mVec2_c pos2;
            pos2.x = pos.x;
            pos2.y = pos.y;
            dObjTutorial_c::setMsg(&pos2, msg, TRUE);
        }
    }
}

// .text:0x18C0
void dMenuNotice00_c::playSeClose() {
    playSe(SE_bbs_close);
}

// .text:0x18C8
void dMenuNotice00_c::playSeOpen() {
    playSe(SE_bbs_open);
}

// .text:0x18D0
void dMenuNotice00_c::playSeJump() {
    playSe(SE_BBS_JumpFirstLast);
}

// .text:0x18D8
void dMenuNotice00_c::playSePageTurn() {
    playSe(SE_bbs_PageOkuri);
}

// .text:0x18E0
void dMenuNotice00_c::openEraseDialog() {
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    setSubState(SUB_DIALOG_OPEN);
    onFlag(FLAG_DIALOG);
    mSlct.setMsg(MSG_ERASE_CONFIRM, nullptr);
    mSlct.setBtn(DIALOG_BTN_NO, MSG_NO, nullptr, SE_cancel_little);
    mSlct.setBtn(DIALOG_BTN_YES, MSG_YES, nullptr, 0);
    mSlct.setBtnVisible(DIALOG_BTN_UNUSED, false);
}
