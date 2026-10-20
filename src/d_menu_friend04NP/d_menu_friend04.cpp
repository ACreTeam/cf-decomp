#include <game/game/d_menu_friend04.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_menu_friend.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <stdio.h>

namespace dMenuFriend04 {
// .text:0x5C
BASE_PROFILE(MENU_FRIEND04, dMenuFriend04_c)
} // namespace dMenuFriend04

static const char *l_allPane = "N_all";
static const char *l_balloonPane = "bln_00";
static const char *l_balloonShadowPane = "blnSdw_00";

// .text:0x200
int dMenuFriend04_c::create() {
    BOOL loaded = mRes.load("Layout/friend_WiFi.arc");
    loaded &= mBalloonRes.load("Layout/friend_WiFi_balloon.arc");
    loaded &= mSelect.load();
    loaded &= mDialog.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "friend_WiFi_win_00.brlyt");
    mSelect.build();
    mLyt.setVisible("B_msg_04", false);
    mLyt.setVisible("B_cmnt_09", false);
    mDialog.create();
    mBalloonLyt.build(&mBalloonRes, "friend_WiFi_balloon_00.brlyt");
    mBalloonLyt.mPriority = BALLOON_PRIORITY;
    initSlider();
    initAnms();
    initBalloon();
    initList();
    initRowWords();
    refreshList();
    setTexts();
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    mLyt.calc();
    onFlag(FLAG_DRAW);
    dMenuFriend_c::setTabVisible(TRUE);
    return SUCCEEDED;
}

// .text:0x35C
int dMenuFriend04_c::doDelete() {
    if (mRes.remove() == FALSE) {
        return NOT_READY;
    }
    if (mBalloonRes.remove() == FALSE) {
        return NOT_READY;
    }
    if (mSelect.remove() == FALSE) {
        return NOT_READY;
    }
    return mDialog.remove() != 0;
}

// .text:0x3E0
int dMenuFriend04_c::stateMain() {
    if (executeCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    return SUCCEEDED;
}

// .text:0x428
int dMenuFriend04_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x42C
int dMenuFriend04_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && prevCursor != mCursor) {
        if (isUitagTab(mCursor)) {
        } else if (isTab(mCursor)) {
            playSeSubMenuPopUp();
        } else {
            playSeCursor();
        }
    }
    calcAnms();
    mSelect.update();
    mDialog.execute();
    calcBalloonAnm();
    return result;
}

// .text:0x4F4
int dMenuFriend04_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x518
int dMenuFriend04_c::draw() {
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
    }
    drawBalloon();
    mSelect.draw();
    if (isFlag(FLAG_DIALOG)) {
        mDialog.draw();
    }
    return SUCCEEDED;
}

inline int dMenuFriend04_c::hitUitagTab() {
    mVec2_c pos = dObjCursor_c::getPos();
    return hitUitagTab(&pos);
}

// .text:0x5BC
void dMenuFriend04_c::subStateMain() {
    dMenuFriend_c::clearTabFocus();
    setSelect(mCursor, FALSE);
    dMenuFriend_c::hideTutorial();
    int tab = dAcUitag::getTabInput(dAcUitag_c::TAB_FRIEND, FALSE);
    if (tab != CURSOR_NONE && selectUitagTab(tab, TRUE)) {
        return;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        closeBalloon();
        dMenuFriend_c::hideTutorial();
    }
    mCursor = hitUitagTab();
    if (mCursor != CURSOR_NONE &&
        mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        push();
        return;
    }
    if (mCursor == CURSOR_NONE) {
        dObjCursor_c::getPos();
        mCursor = hitCheck();
        setSelect(mCursor, TRUE);
        if (mCursor != CURSOR_NONE &&
            mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push();
            return;
        }
    }
    checkPadMove();
}

// .text:0x71C
void dMenuFriend04_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x758
void dMenuFriend04_c::subStateScroll() {
    mScrlAnm.play();
    onFlag(FLAG_CALC);
    if (mScrlAnm.isEnd()) {
        if (mCursor == CURSOR_SCRL_NEXT) {
            mScroll++;
            updateSlider();
            refreshList();
        }
        setScrlBtnAnms();
        if (mPad::g_currentCore->down(WPAD_BUTTON_A) ||
            (dPad::isUpHeld() && mCursor == CURSOR_SCRL_PREV) ||
            (dPad::isDownHeld() && mCursor == CURSOR_SCRL_NEXT)) {
            if (canScroll(mCursor)) {
                startScroll(mCursor);
                return;
            }
        }
        mScrlAnm.start(SCRL_PAGE_ANM_UP);
        setSubState(SUB_MAIN);
    }
}

// .text:0x870
void dMenuFriend04_c::subStateMenu() {
    if (static_cast<dSelect::BComp_c &>(mSelect).isClosed()) {
        mAnswer = mSelect.mCode;
        doAnswer();
    }
}

// .text:0x8C4
void dMenuFriend04_c::subStateDialog() {
    if (mDialog.isWinAnmEnd() && mDialog.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0x924
void dMenuFriend04_c::subStateDialogPush() {
    if (mDialog.isPushEnd()) {
        setSubState(SUB_DIALOG_CLOSE);
        mDialog.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0x988
void dMenuFriend04_c::subStateDialogClose() {
    if (mDialog.isWinAnmEnd()) {
        offFlag(FLAG_DIALOG);
        switch (mDialog.mPushedBtn) {
        case 1:
            setSubState(SUB_DELETE);
            break;
        case 0:
        case 2:
        default:
            setSubState(SUB_MAIN);
            break;
        }
    }
}

// .text:0xA18
void dMenuFriend04_c::subStateDelete() {
    if (dWifi::removeFriendReq(getFriendIdx(mPicked))) {
        mNum = dWifi::getFriendNum();
        if (mScroll > 0 && mNum < mScroll + SCROLL_MARGIN) {
            mScroll = mNum - SCROLL_MARGIN;
            if (mScroll < 0) {
                mScroll = 0;
            }
        }
        updateSlider();
        setScrlBtnAnms();
        refreshList();
        setSubState(SUB_MAIN);
        dMenuFriend_c::setSave();
    }
}

// .text:0xABC
void dMenuFriend04_c::subStateSlider() {
    mVec3_c pos = mpSldr->GetTranslate();
    mVec2_c cursor = dObjCursor_c::getPos();
    f32 y = mSldrDragOfs + cursor.y;
    pos.y = y;
    if (y >= mSldrTop) {
        pos.y = mSldrTop;
    } else if (y <= mSldrBottom) {
        pos.y = mSldrBottom;
    }
    f32 ratio = (pos.y - mSldrTop) / (mSldrBottom - mSldrTop);
    int scroll = (int)(0.5f + (f32)(mNum - SCROLL_MARGIN) * ratio);
    if (mScroll != scroll) {
        mScroll = scroll;
        refreshList();
        playSeSlideClick();
    }
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        updateSlider();
        setSubState(SUB_MAIN);
        setScrlBtnAnms();
    } else {
        mpSldr->SetTranslate(pos);
    }
}

// .text:0xC28
BOOL dMenuFriend04_c::subStateCloseRequest() {
    dAcUitag::closeTab();
    startCloseAnm();
    return FALSE;
}

// .text:0xC60
BOOL dMenuFriend04_c::subStateCloseRequestMenu() {
    if (!mSelect.isReady()) {
        return FALSE;
    }
    mSelect.close();
    return subStateCloseRequest();
}

// .text:0xCB0
BOOL dMenuFriend04_c::subStateCloseRequestDialog() {
    if (!mDialog.isWinAnmEnd()) {
        return FALSE;
    }
    mDialog.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    return subStateCloseRequest();
}

// .text:0xD0C
BOOL dMenuFriend04_c::executeCloseRequest() {
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    static CloseProc closeProcs[SUB_NUM] = {
        &dMenuFriend04_c::subStateCloseRequest,
        nullptr,
        &dMenuFriend04_c::subStateCloseRequest,
        &dMenuFriend04_c::subStateCloseRequestMenu,
        &dMenuFriend04_c::subStateCloseRequestDialog,
        nullptr,
        nullptr,
        nullptr};
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL result = (this->*closeProcs[mSubState])();
    if (result) {
        dMenuMng_c::clearCloseRequest();
    }
    return result;
}

// .text:0xE10
void dMenuFriend04_c::executeSubState() {
    static SubStateProc subStateProcs[SUB_NUM] = {
        &dMenuFriend04_c::subStateMain,
        &dMenuFriend04_c::subStatePush,
        &dMenuFriend04_c::subStateScroll,
        &dMenuFriend04_c::subStateMenu,
        &dMenuFriend04_c::subStateDialog,
        &dMenuFriend04_c::subStateDialogPush,
        &dMenuFriend04_c::subStateDialogClose,
        &dMenuFriend04_c::subStateDelete,
        &dMenuFriend04_c::subStateSlider,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xE48
int dMenuFriend04_c::hitUitagTab(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = CURSOR_NONE;
    }
    dAcUitag::setTabFocus(tab);
    if (tab == CURSOR_NONE) {
        return CURSOR_NONE;
    }
    return tab;
}

// .text:0xEAC
BOOL dMenuFriend04_c::selectUitagTab(int tab, BOOL delay) {
    if (dAcUitag::selectTab(tab, delay)) {
        startCloseAnm();
        return TRUE;
    }
    return FALSE;
}

// .text:0xEFC
void dMenuFriend04_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dMenuFriend_c::requestClose();
}

// .text:0xF38
void dMenuFriend04_c::startReopenAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dMenuFriend_c::requestReopen();
    playSeSubMenuPush();
}

// .text:0xF78
void dMenuFriend04_c::startOpenMenuAnm(int idx) {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuFriend_c::requestOpenMenu(idx);
}

// .text:0xFC4
int dMenuFriend04_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int tab = dMenuFriend_c::getTabAt(&pos);
    if (tab != CURSOR_NONE) {
        return CURSOR_TAB + tab;
    }
    mVec2_c lytPos = pos;
    int cursor =
        m2d::hitButton(&lytPos, &mLyt, CURSOR_ROW, CURSOR_SLIDER, -1, -1, 1.0f);
    if (cursor == CURSOR_SLIDER && mNum <= SCROLL_MARGIN) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCRL_PREV && !canScrollPrev()) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCRL_NEXT && !canScrollNext()) {
        cursor = CURSOR_NONE;
    }
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0x10C4
void dMenuFriend04_c::setSelect(int cursor, BOOL select) {
    if (isTab(cursor)) {
        dMenuFriend_c::setTabFocus(cursor - CURSOR_TAB, select);
    } else if (isScrl(cursor)) {
        mScrlBtnAnm[cursor - CURSOR_SCRL].startEnd(select == FALSE);
        onFlag(FLAG_CALC);
    } else if (isRow(cursor)) {
        setRowAnm(cursor, select);
    } else if (isCmnt(cursor)) {
        int state = getCursorRowState(cursor);
        if (state == ROW_STATE_EMPTY) {
            return;
        }
        if (getCursorComment(cursor)[0] == 0) {
            return;
        }
        int idx = cursor - CURSOR_CMNT;
        int anmNo;
        if (select) {
            anmNo = CMNT_ANM_IN;
        } else {
            anmNo = getCmntAnm(cursor);
        }
        m2d::AnmEx2_c *anm = &mCmntAnm[idx];
        anm->start(anmNo);
        anm->setToEnd();
        onFlag(FLAG_CALC);
    } else if (cursor == CURSOR_REGISTER) {
        mRegisterAnm.start(select == FALSE);
        mRegisterAnm.setToEnd();
        onFlag(FLAG_CALC);
    } else if (cursor == CURSOR_SLIDER) {
        mSldrAnm.setNext(select == FALSE);
    }
}

// .text:0x1298
void dMenuFriend04_c::checkPadMove() {
    if (dPad::isUpTriggered() && canScrollPrev()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        mCursor = CURSOR_SCRL_PREV;
        dMenuFriend_c::hideTutorial();
        closeBalloon();
        push();
    } else if (dPad::isDownTriggered() && canScrollNext()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        mCursor = CURSOR_SCRL_NEXT;
        dMenuFriend_c::hideTutorial();
        closeBalloon();
        push();
    } else if (dPad::isLeftTriggered() && canScrollPrev()) {
        mScroll -= SCROLL_MARGIN;
        if (mScroll < 0) {
            mScroll = 0;
        }
        updateSlider();
        refreshList();
        setScrlBtnAnms();
        playSeDecide();
        closeBalloon();
    } else if (dPad::isRightTriggered() && canScrollNext()) {
        mScroll += SCROLL_MARGIN;
        int max = mNum - SCROLL_MARGIN;
        if (max < 0) {
            max = 0;
        }
        if (mScroll > max) {
            mScroll = max;
        }
        updateSlider();
        refreshList();
        setScrlBtnAnms();
        playSeDecide();
        closeBalloon();
    }
}

static char l_commentPaneFormat[13] = "N_cmnt_%02d";

// .text:0x1430
void dMenuFriend04_c::push() {
    int cursor = mCursor;
    if (isUitagTab(cursor)) {
        selectUitagTab(cursor, FALSE);
        return;
    }
    if (isTab(cursor)) {
        if (!dMenuFriend_c::startTabPush(cursor - CURSOR_TAB)) {
            return;
        }
        dMenuFriend_c::hideTutorial();
    } else if (isScrl(cursor)) {
        if (canScroll(cursor)) {
            mScrlBtnAnm[mCursor - CURSOR_SCRL].setNext(SCRL_ANM_PUSH);
        } else {
            return;
        }
    } else if (cursor == CURSOR_REGISTER) {
        mRegisterAnm.setNext(REGISTER_ANM_PUSH);
    } else if (isRow(cursor)) {
        int row = cursor - CURSOR_ROW;
        if (getCursorRowState(cursor) == ROW_STATE_EMPTY) {
            return;
        }
        mRowAnm[row].setNext(ROW_ANM_PUSH);
    } else if (isCmnt(cursor)) {
        if (!isBalloonHidden()) {
            return;
        }
        if (getCursorRowState(mCursor) == ROW_STATE_EMPTY) {
            return;
        }
        if (getCursorComment(mCursor)[0] == 0) {
            return;
        }
        mCmntPushAnm.setFrame(0.0f);
        char name[32];
        sprintf(name, l_commentPaneFormat, mCursor - CURSOR_CMNT);
        mLyt.setPosFrom(name, "cmnt_Psh", 0.0f, 0.0f);
        showBalloon(mCursor);
        playSeAPush();
    } else if (cursor == CURSOR_SLIDER) {
        mVec2_c pos = dObjCursor_c::getPos();
        mSldrDragOfs = mpSldr->GetTranslate().y - pos.y;
        setSubState(SUB_SLIDER);
        playSeAPush();
        return;
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x169C
BOOL dMenuFriend04_c::isPushEnd() {
    int cursor = mCursor;
    if (isTab(cursor)) {
        return dMenuFriend_c::isTabPushEnd(cursor - CURSOR_TAB);
    }
    if (isScrl(cursor)) {
        return mScrlBtnAnm[cursor - CURSOR_SCRL].isEnd();
    }
    if (cursor == CURSOR_REGISTER) {
        return mRegisterAnm.isEnd();
    }
    if (isRow(cursor)) {
        return mRowAnm[cursor - CURSOR_ROW].isEnd();
    }
    if (isCmnt(cursor)) {
        BOOL end = FALSE;
        end |= mCmntPushAnm.mFrameCtrl.isStop() && mBalloonAnm.isEnd();
        return end;
    } else {
        return TRUE;
    }
}

// .text:0x17D4
void dMenuFriend04_c::finishPush() {
    int cursor = mCursor;
    if (isTab(cursor)) {
        dMenuFriend_c::setTab(cursor - CURSOR_TAB);
        startReopenAnm();
    } else if (isScrl(cursor)) {
        startScroll(cursor);
        setSubState(SUB_SCROLL);
    } else if (cursor == CURSOR_REGISTER) {
        int free = findFreeFriend();
        if (free == -1) {
            openFullDialog(MSG_ROSTER_FULL);
        } else {
            dMenuFriend_c::setFriendIdx(free);
            dMenuFriend_c::clearEntry();
            startOpenMenuAnm(dMenuFriend_c::OPEN_MENU_CODE_ADD);
        }
    } else if (isRow(cursor)) {
        openMenu(cursor);
    } else if (isCmnt(cursor)) {
        setSubState(SUB_MAIN);
        return;
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x1938
void dMenuFriend04_c::setTexts() {
    mLyt.setMessage("T_cmnBtnA_00", MSG_REGISTER, nullptr);
}

// .text:0x1950
void dMenuFriend04_c::initList() {
    dPlayerMgr::Mgr_c::getCurrentPlayer();
    mNum = dWifi::getFriendNum();
    mScroll = 0;
    updateSlider();
    setScrlBtnAnms();
}

// .text:0x199C
void dMenuFriend04_c::initAnms() {
    static const char *rowAnmNames[ROW_ANM_NUM] = {
        "friend_WiFi_win_00_T_msg_In.brlan",
        "friend_WiFi_win_00_T_msg_Out.brlan",
        "friend_WiFi_win_00_T_msg_Psh.brlan",
    };
    static const char *cmntAnmNames[CMNT_ANM_NUM] = {
        "friend_WiFi_win_00_cmnt_In.brlan",
        "friend_WiFi_win_00_cmnt_Out.brlan",
        "friend_WiFi_win_00_cmnt_Gry.brlan",
        "friend_WiFi_win_00_cmnt_Pnk.brlan",
    };
    static const char *scrlBtnAnmNames0[SCRL_ANM_NUM] = {
        "friend_WiFi_win_00_scrlBtn_00_In.brlan",
        "friend_WiFi_win_00_scrlBtn_00_Out.brlan",
        "friend_WiFi_win_00_scrlBtn_00_Psh.brlan",
        "friend_WiFi_win_00_scrlBtn_00_CanPsh.brlan",
        "friend_WiFi_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrlBtnAnmNames1[SCRL_ANM_NUM] = {
        "friend_WiFi_win_00_scrlBtn_01_In.brlan",
        "friend_WiFi_win_00_scrlBtn_01_Out.brlan",
        "friend_WiFi_win_00_scrlBtn_01_Psh.brlan",
        "friend_WiFi_win_00_scrlBtn_01_CanPsh.brlan",
        "friend_WiFi_win_00_scrlBtn_01_NoPsh.brlan",
    };
    static const char *scrlAnmNames[SCRL_PAGE_ANM_NUM] = {
        "friend_WiFi_win_00_scrlDown.brlan",
        "friend_WiFi_win_00_scrlUp.brlan",
    };
    static const char *situationAnmNames[1] = {
        "friend_WiFi_win_00_situationIcon_Ptrn.brlan",
    };
    static const char *registerAnmNames[REGISTER_ANM_NUM] = {
        "friend_WiFi_win_00_cmnBtnA_in.brlan",
        "friend_WiFi_win_00_cmnBtnA_out.brlan",
        "friend_WiFi_win_00_cmnBtnA_psh.brlan",
    };
    for (int i = 0; i < ROW_NUM; i++) {
        mRowAnm[i].init(&mLyt, &mRes, rowAnmNames, ROW_ANM_NUM);
        mRowAnm[i].setNumber(i);
    }
    for (int i = 0; i < ROW_NUM; i++) {
        mCmntAnm[i].init(&mLyt, &mRes, cmntAnmNames, CMNT_ANM_NUM);
        mCmntAnm[i].setNumber(i);
        mCmntAnm[i].setNext(CMNT_ANM_OUT);
    }
    mLyt.setAnm(&mCmntPushAnm, "friend_WiFi_win_00_cmnt_Psh.brlan", nullptr);
    mCmntPushAnm.setToEnd();
    mScrlBtnAnm[0].init(&mLyt, &mRes, scrlBtnAnmNames0, SCRL_ANM_NUM);
    mScrlBtnAnm[1].init(&mLyt, &mRes, scrlBtnAnmNames1, SCRL_ANM_NUM);
    mScrlAnm.init(&mLyt, &mRes, scrlAnmNames, SCRL_PAGE_ANM_NUM);
    for (int i = 0; i < ROW_NUM; i++) {
        mSituationAnm[i].init(&mLyt, &mRes, situationAnmNames, 1);
        mSituationAnm[i].setNumber(i);
        mSituationAnm[i].start(0);
    }
    mRegisterAnm.init(&mLyt, &mRes, registerAnmNames, REGISTER_ANM_NUM);
    mLyt.setAnm(&mIconAnm, "friend_WiFi_win_00_scrlIcon_00.brlan", nullptr);
}

// .text:0x1B40
void dMenuFriend04_c::calcAnms() {
    int i;
    BOOL playing = FALSE;
    for (i = 0; i < ROW_NUM; i++) {
        playing |= mRowAnm[i].play();
    }
    if (playing) {
        onFlag(FLAG_ROW_ANM);
    } else {
        offFlag(FLAG_ROW_ANM);
    }
    for (i = 0; i < ROW_NUM; i++) {
        playing |= mCmntAnm[i].play();
    }
    for (i = 0; i < SCRL_NUM; i++) {
        playing |= mScrlBtnAnm[i].play();
    }
    mRegisterAnm.play();
    if (!mCmntPushAnm.mFrameCtrl.isStop()) {
        mCmntPushAnm.play();
    }
    mIconAnm.play();
    onFlag(FLAG_CALC);
    calcSldrAnm();
}

// .text:0x1C50
void dMenuFriend04_c::setRowAnm(int cursor, BOOL select) {
    int row = cursor - CURSOR_ROW;
    m2d::AnmEx2_c *anm;
    if (select) {
        anm = &mRowAnm[row];
        anm->start(ROW_ANM_IN);
    } else {
        anm = &mRowAnm[row];
        anm->start(ROW_ANM_OUT);
    }
    anm->setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x1CD4
void dMenuFriend04_c::refreshList() {
    updateRowIcons();
    updateRowWords();
}

// .text:0x1D08
void dMenuFriend04_c::initRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        char playerName[40];
        char townName[40];
        static char townFormat[] = "T_msg_%02d";
        sprintf(townName, townFormat, i);
        mTownWord[i].setTextBox(mLyt.findTextBox(townName));
        static char nameFormat[13] = "T_name_%02d";
        sprintf(playerName, nameFormat, i);
        mNameWord[i].setTextBox(mLyt.findTextBox(playerName));
    }
}

// .text:0x1DB8
void dMenuFriend04_c::updateRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + mScroll;
        int state = getRowState(idx);
        char name[40];
        sprintf(name, "B_msg_%02d", i);
        if (idx >= 0 && idx < mNum && state != ROW_STATE_EMPTY) {
            mTownWord[i].mpTextBox->SetVisible(true);
            if (i == ROW_NUM - 1) {
                mLyt.setVisible(name, false);
            } else {
                mLyt.setVisible(name, true);
            }
            mTownWord[i].clear();
            dLandNameWord_c land;
            land.set(getTownName(idx), 0);
            dString::setWord(0, &land);
            dString::loadString(&mTownWord[i], MSG_WORD0, "sys_2D/SYS_2D_menu");
            mTownWord[i].update();
            mNameWord[i].clear();
            mNameWord[i].set(getPlayerName(idx), 0);
            mNameWord[i].update();
        } else {
            mTownWord[i].mpTextBox->SetVisible(false);
            mLyt.setVisible(name, false);
        }
    }
}

// .text:0x1F4C
void dMenuFriend04_c::updateRowIcons() {
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + mScroll;
        int state = getRowState(idx);
        char situationName[40];
        static char situationFormat[] = "situation_%02d";
        sprintf(situationName, situationFormat, i);
        char cmntName[40];
        sprintf(cmntName, l_commentPaneFormat, i);
        char baseName[40];
        static char baseFormat[13] = "B_cmnt_%02d";
        sprintf(baseName, baseFormat, i + ROW_NUM);
        if (idx >= 0 && idx < mNum && state != ROW_STATE_EMPTY) {
            mLyt.setVisible(situationName, state != ROW_STATE_KEY_ONLY);
            mLyt.setVisible(cmntName, true);
            mLyt.setVisible(baseName, i != ROW_NUM - 1);
            f32 frame = state == ROW_STATE_KNOWN ? 0.0f : 2.0f;
            mSituationAnm[i].setFrame(frame);
            mCmntAnm[i].setNext(getCmntAnm(i + CURSOR_CMNT));
        } else {
            mLyt.setVisible(situationName, false);
            mLyt.setVisible(cmntName, false);
            mLyt.setVisible(baseName, false);
        }
    }
    onFlag(FLAG_CALC);
}

// .text:0x20D8
const wchar_t *dMenuFriend04_c::getTownName(int idx) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()
        ->mFriends.getFriend(idx)
        ->mPID.land.mName;
}

// .text:0x2110
const wchar_t *dMenuFriend04_c::getPlayerName(int idx) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()
        ->mFriends.getFriend(idx)
        ->mPID.player.mName;
}

// .text:0x2148
const wchar_t *dMenuFriend04_c::getComment(int idx) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()
        ->mFriends.getFriend(idx)
        ->mMessage;
}

// .text:0x2180
const wchar_t *dMenuFriend04_c::getCursorComment(int cursor) {
    return getComment(getFriendIdx(cursor));
}

// .text:0x21B8
int dMenuFriend04_c::getCmntAnm(int cursor) {
    dFriend_c *fr = dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.getFriend(
        getFriendIdx(cursor));
    if (fr->mMessage[0] == 0) {
        return CMNT_ANM_GRAY;
    }
    if (fr->mWc24Flag.mHasMessage) {
        return CMNT_ANM_OUT;
    }
    return CMNT_ANM_PINK;
}

// .text:0x2218
int dMenuFriend04_c::getRowState(int idx) {
    if ((u32)idx >= FRIEND_NUM) {
        return ROW_STATE_EMPTY;
    }
    switch (dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.getState(idx)) {
    case 0:
        return ROW_STATE_EMPTY;
    case 1:
        return ROW_STATE_KNOWN;
    case 2:
        return ROW_STATE_NO_KEY;
    case 3:
        return ROW_STATE_KEY_ONLY;
    default:
        return ROW_STATE_EMPTY;
    }
}

// .text:0x22A8
int dMenuFriend04_c::getCursorRowState(int cursor) {
    return getRowState(getFriendIdx(cursor));
}

// .text:0x22E0
int dMenuFriend04_c::findFreeFriend() {
    return dWifi::findFreeFriend();
}

// .text:0x22E4
int dMenuFriend04_c::getFriendIdx(int cursor) {
    if (isRow(cursor)) {
        return cursor + mScroll - CURSOR_ROW;
    }
    if (isCmnt(cursor)) {
        return cursor + mScroll - CURSOR_CMNT;
    }
    return 0;
}

// .text:0x234C
void dMenuFriend04_c::startScroll(int cursor) {
    if (cursor == CURSOR_SCRL_PREV) {
        mScroll--;
        updateSlider();
        refreshList();
        mScrlAnm.start(SCRL_PAGE_ANM_DOWN);
    } else {
        mScrlAnm.start(SCRL_PAGE_ANM_UP);
    }
    playSePageTurn();
}

// .text:0x23B4
void dMenuFriend04_c::setScrlBtnAnms() {
    mScrlBtnAnm[0].start(canScrollPrev() ? SCRL_ANM_CAN_PUSH
                                         : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].start(canScrollNext() ? SCRL_ANM_CAN_PUSH
                                         : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x2434
BOOL dMenuFriend04_c::canScroll(int cursor) {
    if (cursor == CURSOR_SCRL_PREV) {
        return canScrollPrev();
    }
    return canScrollNext();
}

// .text:0x2444
BOOL dMenuFriend04_c::canScrollPrev() {
    return mScroll > 0;
}

// .text:0x2458
BOOL dMenuFriend04_c::canScrollNext() {
    return mScroll < mNum - SCROLL_MARGIN;
}

// .text:0x247C
void dMenuFriend04_c::initSlider() {
    static const char *sldrAnmNames[SLDR_ANM_NUM] = {
        "friend_WiFi_win_00_sldr_In.brlan",
        "friend_WiFi_win_00_sldr_Out.brlan",
    };
    mSldrAnm.init(&mLyt, &mRes, sldrAnmNames, SLDR_ANM_NUM);
    mSldrAnm.setNext(SLDR_ANM_OUT);
    mSldrTop = mLyt.findPane("N_sldrTop")->GetTranslate().y;
    mSldrBottom = mLyt.findPane("N_sldrBottom")->GetTranslate().y;
    mpSldr = mLyt.findPane("N_sldr_all");
}

// .text:0x2510
void dMenuFriend04_c::calcSldrAnm() {
    if (mSldrAnm.play()) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x2554
void dMenuFriend04_c::updateSlider() {
    mVec3_c pos = mpSldr->GetTranslate();
    if (mNum - SCROLL_MARGIN <= 0) {
        pos.y = mSldrTop;
    } else {
        pos.y = mSldrTop + (f32)mScroll * ((mSldrBottom - mSldrTop) /
                                           (f32)(mNum - SCROLL_MARGIN));
    }
    mpSldr->SetTranslate(pos);
}

// .text:0x2600
void dMenuFriend04_c::openMenu(int cursor) {
    mPicked = cursor;
    setMenuItems(cursor);
    mVec2_c below(0.0f, 0.0f);
    mVec2_c above(0.0f, 0.0f);
    nw4r::lyt::Pane *pane = mTownWord[cursor - CURSOR_ROW].mpTextBox;
    mVec3_c rightBottom;
    m2d::getScreenRightBottom(&rightBottom, pane);
    mVec3_c leftTop;
    m2d::getScreenLeftTop(&leftTop, pane);
    below.y = rightBottom.y;
    above.y = leftTop.y;
    mSelect.place(&below, &above);
    mSelect.openPointer();
    setSubState(SUB_MENU);
}

// .text:0x27FC
void dMenuFriend04_c::setMenuItems(int cursor) {
    mSelect.reset();
    mSelect.mNum = 0;
    int state = getCursorRowState(cursor);
    if (state != ROW_STATE_KNOWN) {
        mSelect.addItem(MSG_REWRITE_NAME, ANSWER_NAME, 0, 0);
    }
    if (state == ROW_STATE_KEY_ONLY) {
        mSelect.addItem(MSG_REENTER_CODE, ANSWER_CODE, 0, 0);
    }
    mSelect.addItem(MSG_REWRITE_COMMENT, ANSWER_COMMENT, 0, 0);
    mSelect.addItem(MSG_DEFRIEND, ANSWER_DEFRIEND, 0, 0);
    mSelect.addItem(MSG_QUIT, ANSWER_QUIT, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x28EC
void dMenuFriend04_c::doAnswer() {
    switch (mAnswer) {
    case ANSWER_NAME:
        dMenuFriend_c::loadFriend(getFriendIdx(mPicked));
        startOpenMenuAnm(dMenuFriend_c::OPEN_MENU_FRIEND_NAME);
        break;
    case ANSWER_CODE:
        dMenuFriend_c::loadFriend(getFriendIdx(mPicked));
        startOpenMenuAnm(dMenuFriend_c::OPEN_MENU_CODE_EDIT);
        break;
    case ANSWER_COMMENT:
        dMenuFriend_c::loadFriend(getFriendIdx(mPicked));
        startOpenMenuAnm(dMenuFriend_c::OPEN_MENU_FRIEND_COMMENT);
        break;
    case ANSWER_DEFRIEND:
        openDeleteDialog();
        break;
    case ANSWER_QUIT:
    default:
        setSubState(SUB_MAIN);
        break;
    }
}

// .text:0x29B8
void dMenuFriend04_c::openFullDialog(u16 msg) {
    mDialog.setBtnVisible(0, false);
    mDialog.setBtnVisible(1, false);
    mDialog.setBtn(2, MSG_OK, nullptr, 0);
    playSeCanNot();
    mDialog.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mDialog.setMsg(msg, nullptr);
}

// .text:0x2A70
void dMenuFriend04_c::openDeleteDialog() {
    mDialog.setBtn(0, MSG_NO, nullptr, SE_cancel_little);
    mDialog.setBtn(1, MSG_YES, nullptr, 0);
    mDialog.setBtnVisible(2, false);
    mDialog.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    int idx = getFriendIdx(mPicked);
    m2d::stringWord_c word;
    word.set(getTownName(idx), 0);
    dString::setWord(0, &word);
    word.clear();
    word.set(getPlayerName(idx), 0);
    dString::setWord(1, &word);
    mDialog.setMsg(MSG_DELETE_FRIEND, nullptr);
}

// .text:0x2BA4
void dMenuFriend04_c::initBalloon() {
    static const char *balloonAnmNames[BALLOON_ANM_NUM] = {
        "friend_WiFi_balloon_00_blnIn.brlan",
        "friend_WiFi_balloon_00_blnOut.brlan",
    };
    mBalloonAnm.init(&mBalloonLyt, &mBalloonRes, balloonAnmNames,
                     BALLOON_ANM_NUM);
    mBalloonWord.setTextBox(mBalloonLyt.findTextBox("T_msg_00"));
    mBalloonX = mBalloonLyt.findPane(l_balloonPane)->GetTranslate().x;
}

// .text:0x2C20
void dMenuFriend04_c::calcBalloonAnm() {
    if (mBalloonAnm.play()) {
        onFlag(FLAG_BALLOON_CALC);
    } else if (mBalloonAnm.mCurIdx == BALLOON_ANM_OUT) {
        offFlag(FLAG_BALLOON_DRAW);
    }
}

// .text:0x2C80
void dMenuFriend04_c::drawBalloon() {
    if (isFlag(FLAG_BALLOON_DRAW)) {
        if (isFlag(FLAG_BALLOON_CALC)) {
            offFlag(FLAG_BALLOON_CALC);
            mBalloonLyt.calc();
        }
        mBalloonLyt.entry();
    }
}

// .text:0x2CF4
void dMenuFriend04_c::showBalloon(int cursor) {
    mBalloonWord.clear();
    mBalloonWord.set(getCursorComment(cursor), 0);
    mBalloonWord.update();
    fitBalloon();
    char name[40];
    sprintf(name, l_commentPaneFormat, cursor - CURSOR_CMNT);
    mLyt.copyPos(&mBalloonLyt, name, "N_blnBdy_all");
    mBalloonLyt.addPos("N_blnBdy_all", -7.0f, 14.0f);
    mBalloonAnm.setNext(BALLOON_ANM_IN);
    onFlag(FLAG_BALLOON_DRAW);
    onFlag(FLAG_BALLOON_CALC);
}

// .text:0x2DCC
BOOL dMenuFriend04_c::isBalloonHidden() {
    if (mBalloonAnm.isEnd() == FALSE) {
        return FALSE;
    }
    return mBalloonAnm.mCurIdx != BALLOON_ANM_IN;
}

// .text:0x2E1C
void dMenuFriend04_c::closeBalloon() {
    if (mBalloonAnm.mCurIdx == BALLOON_ANM_IN) {
        mBalloonAnm.setNext(BALLOON_ANM_OUT);
    }
}

// .text:0x2E38
void dMenuFriend04_c::fitBalloon() {
    static const f32 maxWidth = 624.0f;
    static const f32 margin = 46.0f;
    static const f32 maxTextWidth = 410.0f;
    f32 width = mBalloonWord.calcWidth(nullptr, 0);
    width += margin;
    f32 scale = width / maxWidth;
    nw4r::lyt::Pane *body = mBalloonLyt.findPane(l_balloonPane);
    mVec2_c scaleVec = body->GetScale();
    scaleVec.x = scale;
    body->SetScale(scaleVec);
    f32 x = 1.368421f * (mBalloonX * scale);
    mVec3_c pos = body->GetTranslate();
    pos.x = x;
    body->SetTranslate(pos);
    nw4r::lyt::Pane *shadow = mBalloonLyt.findPane(l_balloonShadowPane);
    shadow->SetScale(scaleVec);
    pos.x += 4.0f;
    pos.y -= 4.0f;
    shadow->SetTranslate(pos);
    nw4r::lyt::Pane *textBox = mBalloonWord.mpTextBox;
    f32 textX = x + 312.0f * scale - 228.0f;
    pos = textBox->GetTranslate();
    pos.x = textX;
    textBox->SetTranslate(pos);
}
