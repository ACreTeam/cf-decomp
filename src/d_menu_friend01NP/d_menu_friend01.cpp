#include <game/game/d_menu_friend01.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_pad.hpp>
#include <stdio.h>

namespace dMenuFriend01 {
// .text:0x5C
BASE_PROFILE(MENU_FRIEND01, dMenuFriend01_c)
} // namespace dMenuFriend01

static const char *l_allPane = "N_all";
static const char *l_balloonPane = "bln_00";
static const char *l_balloonShadowPane = "blnSdw_00";

// .text:0x1A0
int dMenuFriend01_c::create() {
    BOOL loaded = mRes.load("Layout/friend_dengon.arc");
    loaded &= mBtn.load();
    loaded &= mBalloonRes.load("Layout/friend_WiFi_balloon.arc");
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "friend_dengon_win_00.brlyt");
    mLyt.setVisible("B_msg_05", false);
    mLyt.setMessage("T_ttl_00", MSG_SEND_TO, nullptr);
    mLyt.calc();
    mBtn.create();
    initButtons();
    initSlider();
    initAnms();
    initRowWords();
    setConfirmDisable(TRUE);
    mBalloonLyt.build(&mBalloonRes, "friend_WiFi_balloon_00.brlyt");
    mBalloonLyt.mPriority = BALLOON_PRIORITY;
    initBalloon();
    dMenuMng_c::offNetError();
    if (mMenuNo == dMenuMng_c::MENU_FRIEND_SELECT_KNOWN) {
        onFlag(FLAG_SEND_TO);
        showList();
    } else if (dWifi::hasRoster()) {
        showList();
    } else {
        setState(STATE_MAIN);
        setSubState(SUB_START_ROSTER);
    }
    return SUCCEEDED;
}

// .text:0x31C
int dMenuFriend01_c::doDelete() {
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    if (mRes.remove() == FALSE) {
        return NOT_READY;
    }
    return mBalloonRes.remove() != 0;
}

// .text:0x388
int dMenuFriend01_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x3AC
int dMenuFriend01_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(&mLyt, l_allPane);
    return result;
}

// .text:0x3FC
int dMenuFriend01_c::stateClose() {
    if (lbl_8074EA7D == RESULT_CONFIRM && !dAcUitag::openMenuB(0)) {
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x458
int dMenuFriend01_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prevCursor) {
        playSeCursor();
    }
    mBtn.execute();
    calcAnms();
    calcBalloonAnm();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    return result;
}

// .text:0x504
int dMenuFriend01_c::draw() {
    drawBalloon();
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x558
void dMenuFriend01_c::subStateStartRoster() {
    if (dWifi::startUpdateRoster()) {
        setSubState(SUB_WAIT_ROSTER);
    }
}

// .text:0x5A4
void dMenuFriend01_c::subStateWaitRoster() {
    switch (dWifi::getThreadResult()) {
    case dWifi::THREAD_RESULT_OK:
        showList();
        stateAnm();
        break;
    case dWifi::THREAD_RESULT_FAILED:
        dMenuMng_c::setResult(RESULT_CANCEL);
        dMenuMng_c::onNetError();
        deleteRequest();
        break;
    }
}

// .text:0x61C
void dMenuFriend01_c::subStateMain() {
    setSelect(mCursor, FALSE);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != CURSOR_NONE) {
        mCursor = btn;
        setSelect(btn, TRUE);
        push();
    } else {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            closeBalloon();
        }
        int cursor = hitCheck();
        mCursor = cursor;
        if (cursor != CURSOR_NONE) {
            setSelect(cursor, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                push();
                return;
            }
        }
        checkPadMove();
    }
}

// .text:0x710
void dMenuFriend01_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x74C
void dMenuFriend01_c::subStateScroll() {
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
            (mCursor == CURSOR_SCRL_PREV && dPad::isUpHeld()) ||
            (mCursor == CURSOR_SCRL_NEXT && dPad::isDownHeld())) {
            if (canScroll(mCursor)) {
                startScroll(mCursor);
                return;
            }
        }
        mScrlAnm.start(SCRL_PAGE_ANM_UP);
        setSubState(SUB_MAIN);
    }
}

// .text:0x864
void dMenuFriend01_c::subStateSlider() {
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
    int scroll = (int)(0.5f + (f32)(mNum - VISIBLE_ROWS) * ratio);
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

// .text:0x9D0
void dMenuFriend01_c::executeSubState() {
    typedef void (dMenuFriend01_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuFriend01_c::subStateStartRoster,
        &dMenuFriend01_c::subStateWaitRoster,
        &dMenuFriend01_c::subStateMain,
        &dMenuFriend01_c::subStatePush,
        &dMenuFriend01_c::subStateScroll,
        &dMenuFriend01_c::subStateSlider,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xA08
void dMenuFriend01_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isRow(cursor)) {
        setRowAnm(cursor, select);
    }
    if (isTown(cursor)) {
        mTownAnm[cursor - CURSOR_TOWN].setNext(select == FALSE);
    }
    if (isScrl(cursor)) {
        int anmNo;
        if (select) {
            anmNo = SCRL_ANM_IN;
        } else {
            anmNo = SCRL_ANM_OUT;
        }
        m2d::AnmEx_c *anm = &mScrlBtnAnm[cursor - CURSOR_SCRL];
        anm->start(anmNo);
        anm->setToEnd();
        onFlag(FLAG_CALC);
    }
    if (cursor == CURSOR_SLIDER) {
        mSldrAnm.setNext(select == FALSE);
    }
}

// .text:0xB5C
int dMenuFriend01_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != CURSOR_NONE) {
        return btn;
    }
    mVec2_c lytPos = pos;
    int cursor = m2d::hitButton(&lytPos, &mLyt, CURSOR_ROW, CURSOR_TOWN_LAST,
                                -1, -1, 1.0f);
    if (isRow(cursor) && cursor + mScroll - CURSOR_ROW >= mNum) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SLIDER && mNum <= VISIBLE_ROWS) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCRL_PREV && !canScrollPrev()) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCRL_NEXT && !canScrollNext()) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_ROW_LAST || cursor == CURSOR_TOWN_LAST) {
        cursor = CURSOR_NONE;
    }
    if (isTown(cursor) && getTownName(cursor) == nullptr) {
        cursor = CURSOR_NONE;
    }
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0xCE4
void dMenuFriend01_c::checkPadMove() {
    if (dPad::isUpTriggered() && canScrollPrev()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        mCursor = CURSOR_SCRL_PREV;
        push();
        closeBalloon();
    } else if (dPad::isDownTriggered() && canScrollNext()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        mCursor = CURSOR_SCRL_NEXT;
        push();
        closeBalloon();
    } else if (dPad::isLeftTriggered() && canScrollPrev()) {
        mScroll -= SCROLL_PAGE;
        if (mScroll < 0) {
            mScroll = 0;
        }
        refreshList();
        setScrlBtnAnms();
        updateSlider();
        playSeDecide();
        closeBalloon();
    } else if (dPad::isRightTriggered() && canScrollNext()) {
        mScroll += SCROLL_PAGE;
        int max = mNum - VISIBLE_ROWS;
        if (max < 0) {
            max = 0;
        }
        if (mScroll > max) {
            mScroll = max;
        }
        refreshList();
        setScrlBtnAnms();
        updateSlider();
        playSeDecide();
        closeBalloon();
    }
}

static char l_townPaneFormat[16] = "N_mura_%02d";

// .text:0xE74
void dMenuFriend01_c::push() {
    if (isBtn(mCursor)) {
        mBtn.push(mCursor, FALSE);
    } else if (isRow(mCursor)) {
        int row = mCursor - CURSOR_ROW;
        mRowAnm[row].setNext(ROW_ANM_PUSH);
        if (mSel != row + mScroll) {
            playSeListChoice();
        }
    } else if (isScrl(mCursor)) {
        if (canScroll(mCursor)) {
            mScrlBtnAnm[mCursor - CURSOR_SCRL].setNext(SCRL_ANM_PUSH);
        } else {
            return;
        }
    } else if (mCursor == CURSOR_SLIDER) {
        mVec2_c pos = dObjCursor_c::getPos();
        mSldrDragOfs = mpSldr->GetTranslate().y - pos.y;
        setSubState(SUB_SLIDER);
        playSeAPush();
        return;
    } else if (isTown(mCursor)) {
        if (!isBalloonHidden()) {
            return;
        }
        if (!getTownName(mCursor)) {
            return;
        }
        mTownPushAnm.setFrame(0.0f);
        char name[32];
        sprintf(name, l_townPaneFormat, mCursor - CURSOR_TOWN);
        mLyt.setPosFrom(name, "mura_Psh", 0.0f, 0.0f);
        showBalloon(mCursor);
        playSeAPush();
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x1080
BOOL dMenuFriend01_c::isPushEnd() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        return !mBtn.isPushing();
    }
    if (isRow(cursor)) {
        return !isFlag(FLAG_ROW_ANM);
    }
    if (isScrl(cursor)) {
        return mScrlBtnAnm[cursor - CURSOR_SCRL].isEnd();
    }
    if (isTown(cursor)) {
        BOOL end = FALSE;
        end |= mTownPushAnm.mFrameCtrl.isStop() && mBalloonAnm.isEnd();
        return end;
    }
    return TRUE;
}

// .text:0x11AC
void dMenuFriend01_c::finishPush() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        switch (cursor) {
        case BTN_QUIT:
            startCloseAnm();
            break;
        case BTN_UNUSED:
            break;
        case BTN_CONFIRM:
            startCloseAnm();
            dMenuMng_c::setResult(RESULT_CONFIRM);
            dMenuMng_c::setFriendIdx(mSel);
            break;
        }
    } else if (isRow(cursor)) {
        selectRow(cursor);
        setSubState(SUB_MAIN);
    } else if (isScrl(cursor)) {
        startScroll(cursor);
        setSubState(SUB_SCROLL);
    } else if (isTown(cursor)) {
        setSubState(SUB_MAIN);
        return;
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x1320
void dMenuFriend01_c::showList() {
    initList();
    refreshList();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
}

// .text:0x1398
void dMenuFriend01_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(RESULT_CANCEL);
    closeBalloon();
}

// .text:0x13EC
void dMenuFriend01_c::initList() {
    mNum = dWifi::getRosterNum();
    mSel = -1;
    mScroll = 0;
    updateSlider();
    setScrlBtnAnms();
}

// .text:0x143C
void dMenuFriend01_c::startScroll(int cursor) {
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

// .text:0x14A4
void dMenuFriend01_c::setScrlBtnAnms() {
    mScrlBtnAnm[0].start(canScrollPrev() ? SCRL_ANM_CAN_PUSH
                                         : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].start(canScrollNext() ? SCRL_ANM_CAN_PUSH
                                         : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x1524
BOOL dMenuFriend01_c::canScroll(int cursor) {
    if (cursor == CURSOR_SCRL_PREV) {
        return canScrollPrev();
    }
    return canScrollNext();
}

// .text:0x1534
BOOL dMenuFriend01_c::canScrollPrev() {
    return mScroll > 0;
}

// .text:0x1548
BOOL dMenuFriend01_c::canScrollNext() {
    return mScroll < mNum - VISIBLE_ROWS;
}

// .text:0x156C
void dMenuFriend01_c::selectRow(int cursor) {
    int prev = mSel;
    mSel = cursor + mScroll - CURSOR_ROW;
    setConfirmDisable(FALSE);
    if (prev != mSel) {
        int prevCursor = prev - mScroll + CURSOR_ROW;
        if (isRow(prevCursor)) {
            setRowAnm(prevCursor, FALSE);
        }
    }
    updateSelBase();
}

// .text:0x1604
void dMenuFriend01_c::setRowAnm(int cursor, BOOL select) {
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

// .text:0x1688
void dMenuFriend01_c::resetRowAnms() {
    for (int i = CURSOR_ROW; i <= CURSOR_ROW_LAST; i++) {
        setRowAnm(i, FALSE);
    }
}

// .text:0x16D8
void dMenuFriend01_c::updateSelBase() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[32];
        sprintf(name, "slctBase_%02d", i);
        bool visible = false;
        if (mSel >= 0 && mSel == mScroll + i) {
            visible = true;
        }
        mLyt.setVisible(name, visible);
    }
}

// .text:0x1768
void dMenuFriend01_c::updateTownBtns() {
    for (int i = CURSOR_TOWN; i <= CURSOR_TOWN_LAST; i++) {
        char name[32];
        sprintf(name, l_townPaneFormat, i - CURSOR_TOWN);
        mLyt.setVisible(name, getTownName(i) != nullptr);
    }
}

// .text:0x17EC
void dMenuFriend01_c::refreshList() {
    resetRowAnms();
    updateRowWords();
    updateSelBase();
    updateTownBtns();
}

// .text:0x1830
void dMenuFriend01_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
}

// .text:0x1890
void dMenuFriend01_c::setConfirmDisable(int disable) {
    mBtn.setDisable(BTN_CONFIRM, disable != 0);
}

// .text:0x18AC
void dMenuFriend01_c::initAnms() {
    static const char *rowAnmNames[ROW_ANM_NUM] = {
        "friend_dengon_win_00_T_msg_In.brlan",
        "friend_dengon_win_00_T_msg_Out.brlan",
        "friend_dengon_win_00_T_msg_Psh.brlan",
        "friend_dengon_win_00_T_msg_Before.brlan",
    };
    static const char *townAnmNames[TOWN_ANM_NUM] = {
        "friend_dengon_win_00_mura_In.brlan",
        "friend_dengon_win_00_mura_Out.brlan",
    };
    static const char *scrlBtnAnmNames0[SCRL_ANM_NUM] = {
        "friend_dengon_win_00_scrlBtn_00_In.brlan",
        "friend_dengon_win_00_scrlBtn_00_Out.brlan",
        "friend_dengon_win_00_scrlBtn_00_Psh.brlan",
        "friend_dengon_win_00_scrlBtn_00_CanPsh.brlan",
        "friend_dengon_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrlBtnAnmNames1[SCRL_ANM_NUM] = {
        "friend_dengon_win_00_scrlBtn_01_In.brlan",
        "friend_dengon_win_00_scrlBtn_01_Out.brlan",
        "friend_dengon_win_00_scrlBtn_01_Psh.brlan",
        "friend_dengon_win_00_scrlBtn_01_CanPsh.brlan",
        "friend_dengon_win_00_scrlBtn_01_NoPsh.brlan",
    };
    static const char *scrlAnmNames[SCRL_PAGE_ANM_NUM] = {
        "friend_dengon_win_00_scrlDown.brlan",
        "friend_dengon_win_00_scrlUp.brlan",
    };
    for (int i = 0; i < ROW_NUM; i++) {
        mRowAnm[i].init(&mLyt, &mRes, rowAnmNames, ROW_ANM_NUM);
        mRowAnm[i].setNumber(i);
    }
    for (int i = 0; i < ROW_NUM; i++) {
        mTownAnm[i].init(&mLyt, &mRes, townAnmNames, TOWN_ANM_NUM);
        mTownAnm[i].setNumber(i);
    }
    mLyt.setAnm(&mTownPushAnm, "friend_dengon_win_00_mura_Psh.brlan", nullptr);
    mTownPushAnm.setToEnd();
    mScrlBtnAnm[0].init(&mLyt, &mRes, scrlBtnAnmNames0, SCRL_ANM_NUM);
    mScrlBtnAnm[1].init(&mLyt, &mRes, scrlBtnAnmNames1, SCRL_ANM_NUM);
    mScrlAnm.init(&mLyt, &mRes, scrlAnmNames, SCRL_PAGE_ANM_NUM);
    mScrlAnm.start(SCRL_PAGE_ANM_UP);
    mLyt.setAlpha("line_00", 0xFF);
    mLyt.setAnm(&mIconAnm, "friend_dengon_win_00_scrlIcon_00.brlan", nullptr);
}

// .text:0x1A00
void dMenuFriend01_c::calcAnms() {
    int i;
    BOOL playing = FALSE;
    for (i = 0; i < ROW_NUM; i++) {
        playing |= mRowAnm[i].play();
    }
    for (i = 0; i < ROW_NUM; i++) {
        playing |= mTownAnm[i].play();
    }
    if (!mTownPushAnm.mFrameCtrl.isStop()) {
        mTownPushAnm.play();
        playing = TRUE;
    }
    if (playing) {
        onFlag(FLAG_ROW_ANM);
    } else {
        offFlag(FLAG_ROW_ANM);
    }
    for (i = 0; i < SCRL_NUM; i++) {
        playing |= mScrlBtnAnm[i].play();
    }
    calcSldrAnm();
    mIconAnm.play();
    onFlag(FLAG_CALC);
}

// .text:0x1B0C
void dMenuFriend01_c::initRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[32];
        sprintf(name, "T_msg_%02d", i);
        mRowWord[i].setTextBox(mLyt.findTextBox(name));
    }
}

// .text:0x1B94
void dMenuFriend01_c::updateRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + mScroll;
        char name[40];
        sprintf(name, "B_msg_%02d", i);
        if (idx >= 0 && idx < mNum) {
            mRowWord[i].clear();
            const wchar_t *friendName = getFriendName(idx);
            if (friendName) {
                mRowWord[i].set(friendName, 0);
                mLyt.setVisible(name, true);
            } else {
                mLyt.setVisible(name, false);
            }
            mRowWord[i].update();
        }
    }
}

// .text:0x1C68
const wchar_t *dMenuFriend01_c::getFriendName(int idx) {
    if (!dWifi::isWc24On()) {
        return nullptr;
    }
    if (idx >= mNum) {
        return nullptr;
    }
    return dWifi::getRosterInfo(idx);
}

// .text:0x1CCC
const wchar_t *dMenuFriend01_c::getTownName(int cursor) {
    int idx = cursor + mScroll - CURSOR_TOWN;
    if (idx >= mNum) {
        return nullptr;
    }
    static dLandID_c land;
    land = *dWifi::getRosterLand(idx);
    if (land.isValid() == FALSE) {
        return nullptr;
    }
    return land.mName;
}

// .text:0x1D94
void dMenuFriend01_c::initSlider() {
    static const char *sldrAnmNames[SLDR_ANM_NUM] = {
        "friend_dengon_win_00_sldr_In.brlan",
        "friend_dengon_win_00_sldr_Out.brlan",
    };
    mSldrAnm.init(&mLyt, &mRes, sldrAnmNames, SLDR_ANM_NUM);
    mSldrAnm.setNext(SLDR_ANM_OUT);
    mSldrTop = mLyt.findPane("N_sldrTop")->GetTranslate().y;
    mSldrBottom = mLyt.findPane("N_sldrBottom")->GetTranslate().y;
    mpSldr = mLyt.findPane("N_sldr_all");
}

// .text:0x1E28
void dMenuFriend01_c::calcSldrAnm() {
    if (mSldrAnm.play()) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x1E6C
void dMenuFriend01_c::updateSlider() {
    mVec3_c pos = mpSldr->GetTranslate();
    if (mNum - VISIBLE_ROWS <= 0) {
        pos.y = mSldrTop;
    } else {
        pos.y = mSldrTop + (f32)mScroll * ((mSldrBottom - mSldrTop) /
                                           (f32)(mNum - VISIBLE_ROWS));
    }
    mpSldr->SetTranslate(pos);
}

// .text:0x1F18
void dMenuFriend01_c::initBalloon() {
    static const char *balloonAnmNames[BALLOON_ANM_NUM] = {
        "friend_WiFi_balloon_00_blnIn.brlan",
        "friend_WiFi_balloon_00_blnOut.brlan",
    };
    mBalloonAnm.init(&mBalloonLyt, &mBalloonRes, balloonAnmNames,
                     BALLOON_ANM_NUM);
    mBalloonWord.setTextBox(mBalloonLyt.findTextBox("T_msg_00"));
    mBalloonX = mBalloonLyt.findPane(l_balloonPane)->GetTranslate().x;
}

// .text:0x1F94
void dMenuFriend01_c::calcBalloonAnm() {
    if (mBalloonAnm.play()) {
        onFlag(FLAG_BALLOON_CALC);
    } else if (mBalloonAnm.mCurIdx == BALLOON_ANM_OUT) {
        offFlag(FLAG_BALLOON_DRAW);
    }
}

// .text:0x1FF4
void dMenuFriend01_c::drawBalloon() {
    if (isFlag(FLAG_BALLOON_DRAW)) {
        if (isFlag(FLAG_BALLOON_CALC)) {
            offFlag(FLAG_BALLOON_CALC);
            mBalloonLyt.calc();
        }
        mBalloonLyt.entry();
    }
}

// .text:0x2068
void dMenuFriend01_c::showBalloon(int cursor) {
    m2d::stringWord_c word;
    word.set(getTownName(cursor), 0);
    dString::setWord(0, &word);
    mBalloonWord.clear();
    dString::loadString(&mBalloonWord, MSG_WORD0, "sys_2D/SYS_2D_menu");
    mBalloonWord.update();
    fitBalloon();
    char name[40];
    sprintf(name, l_townPaneFormat, cursor - CURSOR_TOWN);
    mLyt.copyPos(&mBalloonLyt, name, "N_blnBdy_all");
    mBalloonLyt.addPos("N_blnBdy_all", 14.0f, 35.0f);
    mBalloonAnm.setNext(BALLOON_ANM_IN);
    onFlag(FLAG_BALLOON_DRAW);
    onFlag(FLAG_BALLOON_CALC);
}

// .text:0x2178
BOOL dMenuFriend01_c::isBalloonHidden() {
    if (mBalloonAnm.isEnd() == FALSE) {
        return FALSE;
    }
    return mBalloonAnm.mCurIdx != BALLOON_ANM_IN;
}

// .text:0x21C8
void dMenuFriend01_c::closeBalloon() {
    if (mBalloonAnm.mCurIdx == BALLOON_ANM_IN) {
        mBalloonAnm.setNext(BALLOON_ANM_OUT);
    }
}

// .text:0x21E4
void dMenuFriend01_c::fitBalloon() {
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
