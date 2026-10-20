#include <game/game/d_menu_friend02.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_pad.hpp>
#include <stdio.h>
#include <string.h>

namespace dMenuFriend02 {
// .text:0x5C
BASE_PROFILE(MENU_FRIEND02, dMenuFriend02_c)
} // namespace dMenuFriend02

static const char *l_allPane = "N_all";

// .text:0x198
int dMenuFriend02_c::create() {
    BOOL resLoaded = mRes.load("Layout/friend_letter.arc");
    BOOL loaded = resLoaded & mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    buildList();
    mLyt.build(&mRes, "friend_letter_win_00.brlyt");
    mLyt.setVisible("B_msg_04", false);
    mTabLyt.build(&mRes, "friend_letter_tab_00.brlyt");
    mLyt.calc();
    mTabLyt.calc();
    mBtn.create();
    initButtons();
    initAnms();
    initTabAnm();
    initList();
    initRowWords();
    setMode(MODE_TOWN);
    updateScrollBtns();
    refreshList();
    setTabHover(-1, TAB_HOVER_IN);
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setTabAnm(TAB_WIN_ANM_IN);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x30C
int dMenuFriend02_c::doDelete() {
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes.remove() != 0;
}

// .text:0x360
int dMenuFriend02_c::stateMain() {
    if (checkCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    return SUCCEEDED;
}

// .text:0x3A8
int dMenuFriend02_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    if (!isFlag(FLAG_SWITCH)) {
        mBtn.setPos(&mLyt, l_allPane);
    }
    return result;
}

// .text:0x40C
int dMenuFriend02_c::stateClose() {
    if (isFlag(FLAG_SWITCH)) {
        startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
        mSelected = -1;
        refreshList();
        updateScrollBtns();
        return SUCCEEDED;
    }
    if (isFlag(FLAG_FORCE_CLOSE)) {
        deleteRequest();
        return SUCCEEDED;
    }
    switch (mMenuNo) {
    case dMenuMng_c::MENU_ADDRESS:
        if (lbl_8074EA7D == RESULT_CONFIRM) {
            dAcUitag::openMenuA(dAcUitag::MENU_A_WRITE);
        } else {
            dAcUitag::openMenuA(dAcUitag::MENU_A_ITEM);
            dAcUitag::showTab();
        }
        break;
    case dMenuMng_c::MENU_ADDRESS_LETTER:
        dAcUitag::openMenuA(dAcUitag::MENU_A_ITEM_1);
        break;
    default:
        dAcUitag::openMenuA(dAcUitag::MENU_A_ITEM);
        dAcUitag::showTab();
        break;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x514
int dMenuFriend02_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prev) {
        playSeCursor();
    }
    mBtn.execute();
    calcAnms();
    return result;
}

// .text:0x584
int dMenuFriend02_c::draw() {
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
    }
    mScrollIconAnm.play();
    mLyt.calc();
    if (isFlag(FLAG_TAB_CALC)) {
        offFlag(FLAG_TAB_CALC);
        mTabLyt.calc();
    }
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mTabLyt.entry();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x648
void dMenuFriend02_c::subStateMain() {
    dObjTutorial_c::hide();
    setCursor(mCursor, FALSE);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != CURSOR_NONE) {
        mCursor = btn;
        setCursor(btn, TRUE);
        push();
        return;
    }
    int hit = hitCheck();
    mCursor = hit;
    if (hit != CURSOR_NONE) {
        setCursor(hit, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push();
            return;
        }
    }
    padCheck();
    showTabTutorial();
}

// .text:0x720
void dMenuFriend02_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x75C
void dMenuFriend02_c::subStateScroll() {
    mScrollAnm.play();
    onFlag(FLAG_CALC);
    if (mScrollAnm.isEnd()) {
        if (mCursor == CURSOR_SCROLL_DOWN) {
            mScroll++;
            refreshList();
        }
        updateScrollBtns();
        if (mPad::g_currentCore->down(WPAD_BUTTON_A) ||
            (mCursor == CURSOR_SCROLL_UP && dPad::isUpHeld()) ||
            (mCursor == CURSOR_SCROLL_DOWN && dPad::isDownHeld())) {
            if (canScroll(mCursor)) {
                startScroll(mCursor);
                return;
            }
        }
        mScrollAnm.start(SCROLL_ANM_UP);
        setSubState(SUB_MAIN);
    }
}

// .text:0x86C
BOOL dMenuFriend02_c::closeMenu() {
    startClose();
    onFlag(FLAG_FORCE_CLOSE);
    dAcUitag::closeTab();
    dAcUitag::setHideAfterMenu();
    return TRUE;
}

// .text:0x8B0
BOOL dMenuFriend02_c::checkCloseRequest() {
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    static CloseProc closeProcs[SUB_NUM] = {&dMenuFriend02_c::closeMenu, NULL,
                                            NULL};
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL result = (this->*closeProcs[mSubState])();
    if (result) {
        dMenuMng_c::clearCloseRequest();
    }
    return result;
}

// .text:0x99C
void dMenuFriend02_c::executeSubState() {
    static SubStateProc subStateProcs[SUB_NUM] = {
        &dMenuFriend02_c::subStateMain,
        &dMenuFriend02_c::subStatePush,
        &dMenuFriend02_c::subStateScroll,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x9D4
void dMenuFriend02_c::setCursor(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isRow(cursor)) {
        setRowSelect(cursor, select);
    }
    if (isScrollBtn(cursor)) {
        int idx = select ? SCROLL_BTN_ANM_IN : SCROLL_BTN_ANM_OUT;
        m2d::AnmEx_c *anm = &mScrollBtnAnm[cursor - CURSOR_SCROLL_UP];
        anm->start(idx);
        anm->setToEnd();
        onFlag(FLAG_CALC);
    }
    if (isTab(cursor)) {
        int tab = cursor - CURSOR_TAB;
        if (select) {
            setTabHover(tab, TAB_HOVER_IN);
        } else {
            setTabHover(CURSOR_NONE, TAB_HOVER_OUT);
        }
    }
}

// .text:0xB20
int dMenuFriend02_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != CURSOR_NONE) {
        return btn;
    }
    mVec2_c rowPos = pos;
    int cursor = m2d::hitButton(&rowPos, &mLyt, CURSOR_ROW, CURSOR_SCROLL_DOWN,
                                -1, -1, 1.0f);
    if (isRow(cursor) && cursor - CURSOR_ROW >= mNum) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCROLL_UP && !canScrollUp()) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCROLL_DOWN && !canScrollDown()) {
        cursor = CURSOR_NONE;
    }
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    mVec2_c tabPos = pos;
    cursor = m2d::hitButton(&tabPos, &mTabLyt, CURSOR_TAB, CURSOR_TAB_LAST, -1,
                            -1, 1.0f);
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0xC7C
void dMenuFriend02_c::padCheck() {
    if (dPad::isUpTriggered() && canScrollUp()) {
        if (mCursor != CURSOR_NONE) {
            setCursor(mCursor, FALSE);
        }
        mCursor = CURSOR_SCROLL_UP;
        push();
    } else if (dPad::isDownTriggered() && canScrollDown()) {
        if (mCursor != CURSOR_NONE) {
            setCursor(mCursor, FALSE);
        }
        mCursor = CURSOR_SCROLL_DOWN;
        push();
    } else if (dPad::isLeftTriggered() && canScrollUp()) {
        mScroll -= VISIBLE_ROWS;
        if (mScroll < 0) {
            mScroll = 0;
        }
        refreshList();
        updateScrollBtns();
        playSeDecide();
    } else if (dPad::isRightTriggered() && canScrollDown()) {
        mScroll += VISIBLE_ROWS;
        int max = mNum - VISIBLE_ROWS;
        if (max < 0) {
            max = 0;
        }
        if (mScroll > max) {
            mScroll = max;
        }
        refreshList();
        updateScrollBtns();
        playSeDecide();
    }
}

// .text:0xDDC
void dMenuFriend02_c::push() {
    if (isBtn(mCursor)) {
        mBtn.push(mCursor, FALSE);
    } else if (isRow(mCursor)) {
        int row = mCursor - CURSOR_ROW;
        mRowAnm[row].setNext(ROW_ANM_PUSH);
        if (mSelected != row + mScroll) {
            playSeListChoice();
        }
    } else if (isScrollBtn(mCursor)) {
        if (canScroll(mCursor)) {
            mScrollBtnAnm[mCursor - CURSOR_SCROLL_UP].setNext(
                SCROLL_BTN_ANM_PUSH);
        } else {
            return;
        }
    } else if (isTab(mCursor)) {
        if (mCursor - CURSOR_TAB == mMode) {
            return;
        }
        setTabHover(mCursor - CURSOR_TAB, TAB_HOVER_PUSH);
        playSeSubMenuPush();
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xF48
BOOL dMenuFriend02_c::isPushEnd() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        return !mBtn.isPushing();
    }
    if (isRow(cursor)) {
        return !isFlag(FLAG_ROW_ANM);
    }
    if (isScrollBtn(cursor)) {
        return mScrollBtnAnm[cursor - CURSOR_SCROLL_UP].isEnd();
    }
    if (isTab(cursor)) {
        return mTabHoverAnm.mFrameCtrl.isStop();
    }
    return TRUE;
}

// .text:0x1038
void dMenuFriend02_c::finishPush() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        switch (cursor) {
        case BTN_QUIT:
            startClose();
            break;
        case BTN_CONFIRM:
            startClose();
            dMenuMng_c::setResult(RESULT_CONFIRM);
            applySelection();
            break;
        }
    } else if (isRow(cursor)) {
        chooseRow(cursor);
        setSubState(SUB_MAIN);
    } else if (isScrollBtn(cursor)) {
        startScroll(cursor);
        setSubState(SUB_SCROLL);
    } else if (isTab(cursor)) {
        setMode(cursor - CURSOR_TAB);
        setSubState(SUB_MAIN);
        startSwitchOut();
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x11C0
void dMenuFriend02_c::startClose() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    setTabAnm(TAB_WIN_ANM_OUT);
    dMenuMng_c::setResult(RESULT_CANCEL);
    offFlag(FLAG_SWITCH);
}

// .text:0x1224
void dMenuFriend02_c::startSwitchOut() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    onFlag(FLAG_SWITCH);
}

// .text:0x1274
void dMenuFriend02_c::initList() {
    mNum = FRIEND_NUM;
    mSelected = -1;
    mScroll = 0;
}

// .text:0x1290
void dMenuFriend02_c::startScroll(int cursor) {
    if (cursor == CURSOR_SCROLL_UP) {
        mScroll--;
        refreshList();
        mScrollAnm.start(SCROLL_ANM_DOWN);
    } else {
        mScrollAnm.start(SCROLL_ANM_UP);
    }
    playSePageTurn();
}

// .text:0x12F0
void dMenuFriend02_c::updateScrollBtns() {
    int anm = canScrollUp() ? SCROLL_BTN_ANM_CAN_PUSH : SCROLL_BTN_ANM_NO_PUSH;
    mScrollBtnAnm[0].start(anm);
    mScrollBtnAnm[0].setToEnd();
    anm = canScrollDown() ? SCROLL_BTN_ANM_CAN_PUSH : SCROLL_BTN_ANM_NO_PUSH;
    mScrollBtnAnm[1].start(anm);
    mScrollBtnAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x1370
bool dMenuFriend02_c::canScroll(int cursor) {
    if (cursor == CURSOR_SCROLL_UP) {
        return canScrollUp();
    }
    return canScrollDown();
}

// .text:0x1380
bool dMenuFriend02_c::canScrollUp() {
    return mScroll > 0;
}

// .text:0x1394
bool dMenuFriend02_c::canScrollDown() {
    return mScroll < mNum - VISIBLE_ROWS;
}

// .text:0x13B8
void dMenuFriend02_c::chooseRow(int cursor) {
    int row = cursor - CURSOR_ROW;
    int old = mSelected;
    mSelected = row + mScroll;
    if (old != mSelected) {
        int oldCursor = old - mScroll + CURSOR_ROW;
        if (isRow(oldCursor)) {
            setRowSelect(oldCursor, FALSE);
        }
    }
    mSelAddr = getAddr(row + mScroll);
    mBtn.setDisable(BTN_CONFIRM, FALSE);
    updateRowSelect();
}

// .text:0x1468
void dMenuFriend02_c::setRowSelect(int cursor, BOOL select) {
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

// .text:0x14EC
void dMenuFriend02_c::updateRowSelect() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[0x28];
        sprintf(name, "slctBase_%02d", i);
        bool selected = false;
        if (mSelected >= 0 && mSelected == mScroll + i) {
            selected = true;
        }
        mLyt.setVisible(name, selected);
    }
}

// .text:0x157C
void dMenuFriend02_c::resetRowAnms() {
    for (int cursor = CURSOR_ROW; cursor <= CURSOR_ROW_LAST; cursor++) {
        setRowSelect(cursor, FALSE);
    }
}

// .text:0x15CC
void dMenuFriend02_c::refreshList() {
    resetRowAnms();
    updateRows();
    updateRowSelect();
}

// .text:0x1608
void dMenuFriend02_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
    mBtn.setDisable(BTN_CONFIRM, TRUE);
}

// .text:0x1678
void dMenuFriend02_c::initAnms() {
    static const char *rowAnmNames[ROW_ANM_NUM] = {
        "friend_letter_win_00_T_msg_In.brlan",
        "friend_letter_win_00_T_msg_Out.brlan",
        "friend_letter_win_00_T_msg_Psh.brlan",
        "friend_letter_win_00_T_msg_Before.brlan",
    };
    static const char *scrollBtnAnmNames0[SCROLL_BTN_ANM_NUM] = {
        "friend_letter_win_00_scrlBtn_00_In.brlan",
        "friend_letter_win_00_scrlBtn_00_Out.brlan",
        "friend_letter_win_00_scrlBtn_00_Psh.brlan",
        "friend_letter_win_00_scrlBtn_00_CanPsh.brlan",
        "friend_letter_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrollBtnAnmNames1[SCROLL_BTN_ANM_NUM] = {
        "friend_letter_win_00_scrlBtn_01_In.brlan",
        "friend_letter_win_00_scrlBtn_01_Out.brlan",
        "friend_letter_win_00_scrlBtn_01_Psh.brlan",
        "friend_letter_win_00_scrlBtn_01_CanPsh.brlan",
        "friend_letter_win_00_scrlBtn_01_NoPsh.brlan",
    };
    static const char *scrollAnmNames[] = {
        "friend_letter_win_00_scrlDown.brlan",
        "friend_letter_win_00_scrlUp.brlan",
    };
    for (int i = 0; i < ROW_NUM; i++) {
        mRowAnm[i].init(&mLyt, &mRes, rowAnmNames, ROW_ANM_NUM);
        mRowAnm[i].setNumber(i);
    }
    mScrollBtnAnm[0].init(&mLyt, &mRes, scrollBtnAnmNames0, SCROLL_BTN_ANM_NUM);
    mScrollBtnAnm[1].init(&mLyt, &mRes, scrollBtnAnmNames1, SCROLL_BTN_ANM_NUM);
    mScrollAnm.init(&mLyt, &mRes, scrollAnmNames, 2);
    mScrollAnm.start(SCROLL_ANM_UP);
    mLyt.setAnm(&mScrollIconAnm, "friend_letter_win_00_scrlIcon_00.brlan",
                nullptr);
}

// .text:0x1764
void dMenuFriend02_c::calcAnms() {
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
    for (i = 0; i < SCROLL_BTN_NUM; i++) {
        playing |= mScrollBtnAnm[i].play();
    }
    if (playing) {
        onFlag(FLAG_CALC);
    }
    calcTabAnms();
}

// .text:0x1830
void dMenuFriend02_c::initRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[0x28];
        char townName[0x28];
        static char townFormat[] = "T_msg_%02d";
        sprintf(townName, townFormat, i);
        mTownWord[i].setTextBox(mLyt.findTextBox(townName));
        static char nameFormat[13] = "T_name_%02d";
        sprintf(name, nameFormat, i);
        mNameWord[i].setTextBox(mLyt.findTextBox(name));
        sprintf(name, "T_future_%02d", i);
        mLyt.setMessage(name, MSG_FUTURE_ME, nullptr);
        mpFutureBox[i] = mLyt.findTextBox(name);
        mpFutureBox[i]->SetVisible(false);
    }
}

// .text:0x192C
void dMenuFriend02_c::updateRows() {
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + mScroll;
        mTownWord[i].clear();
        mNameWord[i].clear();
        if (idx >= 0 && idx < mNum) {
            if (getAddr(idx) == ADDR_SELF) {
                mpFutureBox[i]->SetVisible(true);
            } else {
                mpFutureBox[i]->SetVisible(false);
                dLandNameWord_c town;
                getTownName(&town, idx);
                dString::setWord(0, &town);
                dString::loadString(&mTownWord[i], MSG_TOWN_NAME,
                                    "sys_2D/SYS_2D_menu");
                getPlayerName(&mNameWord[i], idx);
            }
        } else {
            mpFutureBox[i]->SetVisible(false);
        }
        mTownWord[i].update();
        mNameWord[i].update();
    }
}

// .text:0x1A68
void dMenuFriend02_c::getTownName(dScript::Word_c *word, int idx) {
    word->clear();
    u8 addr = getAddr(idx);
    if (isPlayerAddr(addr) || isAnimalAddr(addr)) {
        dSaveTown_c *town = dSaveData_c::getTown();
        word->set(town->mLandID.mName, 0);
    } else {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (addr == ADDR_SELF) {
            word->set(player->mPID.land.mName, 0);
        } else if (isFriendAddr(addr)) {
            word->set(
                player->mFriends.getFriend(addr - ADDR_FRIEND)->mPID.land.mName,
                0);
        }
    }
}

// .text:0x1B90
void dMenuFriend02_c::getPlayerName(dScript::Word_c *word, int idx) {
    word->clear();
    u8 addr = getAddr(idx);
    if (isPlayerAddr(addr)) {
        dSaveTown_c *town = dSaveData_c::getTown();
        dPrivateData_c *home = town->mHomes.getHomePlayer(addr - ADDR_PLAYER);
        word->set(home->mPID.player.mName, 0);
    } else if (isAnimalAddr(addr)) {
        dSaveTown_c *town = dSaveData_c::getTown();
        dAnimal_c *animal = town->mAnimals.mTown.getAnimal(addr - ADDR_ANIMAL);
        dHmnName::Word_c name;
        animal->mID.setWord(&name, LANGUAGE_NUM);
        word->copy(&name, 0);
    } else {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (isFriendAddr(addr)) {
            word->set(player->mFriends.getFriend(addr - ADDR_FRIEND)
                          ->mPID.player.mName,
                      0);
        } else if (addr == ADDR_SELF) {
            m2d::stringWord_c future;
            future.clear();
            dString::loadString(&future, MSG_FUTURE_ME, "sys_2D/SYS_2D_menu");
            word->copy(&future, 0);
        }
    }
}

// .text:0x1D50
void dMenuFriend02_c::initTabAnm() {
    static const char *tabAnmNames[] = {
        "friend_letter_tab_00_topTb_00_On.brlan",
        "friend_letter_tab_00_topTb_01_On.brlan",
    };
    mTabAnm.init(&mTabLyt, &mRes, tabAnmNames, 2);
}

// .text:0x1D6C
void dMenuFriend02_c::setTabAnm(int anm) {
    static const char *tabWinAnmNames[] = {
        "friend_letter_tab_00_topTbIn.brlan",
        "friend_letter_tab_00_topTbOut.brlan",
    };
    mTabLyt.setAnm(&mTabInOutAnm, tabWinAnmNames[anm], nullptr);
}

// .text:0x1D8C
void dMenuFriend02_c::setTabHover(int tab, int anm) {
    static const char *tabHoverAnmNames[] = {
        "friend_letter_tab_00_topTb_DpdIn.brlan",
        "friend_letter_tab_00_topTb_DpdOut.brlan",
        "friend_letter_tab_00_topTbPsh.brlan",
    };
    mTabHover = tab;
    if (tab == CURSOR_NONE) {
        mTabLyt.setVisible("topTbFrm_Fc", false);
        return;
    }
    mTabLyt.setVisible("topTbFrm_Fc", true);
    char name[0x28];
    sprintf(name, "N_topTb_%02d", mTabHover);
    if (anm == TAB_HOVER_PUSH) {
        mTabLyt.setPosFrom(name, "topTbBase_Psh", 0.0f, 8.0f);
    } else {
        mTabLyt.setPosFrom(name, "topTbFrm_Fc", 0.0f, 8.0f);
    }
    mTabLyt.setAnm(&mTabHoverAnm, tabHoverAnmNames[anm], nullptr);
    onFlag(FLAG_TAB_CALC);
}

// .text:0x1E88
void dMenuFriend02_c::calcTabAnms() {
    BOOL playing = FALSE;
    if (!mTabInOutAnm.mFrameCtrl.isStop()) {
        mTabInOutAnm.play();
        playing = TRUE;
    }
    playing |= mTabAnm.play();
    if (mTabHover != CURSOR_NONE) {
        if (playing) {
            char name[0x28];
            sprintf(name, "N_topTb_%02d", mTabHover);
            mTabLyt.setPosFrom(name, "topTbFrm_Fc", 0.0f, 8.0f);
        }
        if (!mTabHoverAnm.mFrameCtrl.isStop()) {
            mTabHoverAnm.play();
            playing = TRUE;
        }
    }
    if (playing) {
        onFlag(FLAG_TAB_CALC);
    }
}

// .text:0x1F60
void dMenuFriend02_c::setMode(int mode) {
    mMode = mode;
    mTabAnm.setNext(mode);
    if (mMode != MODE_TOWN) {
        mNum = mFriendNum;
    } else {
        mNum = mHomeNum;
    }
    mScroll = 0;
    if (mode == MODE_TOWN) {
        dSaveData_c *raw = dSaveData_c::getRaw();
        dString::setTownWord(0, &raw->mLandID);
    }
    mLyt.setMessage("T_ttl_00",
                    mode == MODE_TOWN ? dObjTutorial_c::MSG_TOWN
                                      : dObjTutorial_c::MSG_ANOTHER_TOWN,
                    "sys_2D/SYS_2D_UIballoon");
}

// .text:0x2010
void dMenuFriend02_c::buildList() {
    for (int i = 0; i < HOME_ADDR_NUM; i++) {
        mHomeAddr[i] = 0;
    }
    for (int i = 0; i < FRIEND_ADDR_NUM; i++) {
        mFriendAddr[i] = 0;
    }
    mHomeNum = 0;
    mFriendNum = 0;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int i;
    for (i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *home = dSaveData_c::getTown()->mHomes.getHomePlayer(i);
        if (home && home->mPID.isValid() &&
            *home->getPersonalID() != *player->getPersonalID()) {
            mHomeAddr[mHomeNum] = i + ADDR_PLAYER;
            mHomeNum++;
        }
    }
    for (i = 0; i < ANIMAL_NUM; i++) {
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(i);
        BOOL knows = FALSE;
        if (animal && animal->mID.isValid() &&
            animal->findMemory(&player->mPID)) {
            knows = TRUE;
        }
        if (knows) {
            mHomeAddr[mHomeNum] = i + ADDR_ANIMAL;
            mHomeNum++;
        }
    }
    mHomeAddr[mHomeNum] = ADDR_SELF;
    mHomeNum++;
    for (i = 0; i < FRIEND_NUM; i++) {
        BOOL registered = FALSE;
        if (dWifi::isFriendValid(i) &&
            player->mFriends.getFriend(i)->mPID.isValid()) {
            registered = TRUE;
        }
        if (registered) {
            mFriendAddr[mFriendNum] = i + ADDR_FRIEND;
            mFriendNum++;
        }
    }
}

// .text:0x22D4
u8 dMenuFriend02_c::getAddr(int idx) {
    if (mMode != MODE_TOWN) {
        return mFriendAddr[idx];
    }
    return mHomeAddr[idx];
}

// .text:0x22F8
void dMenuFriend02_c::applySelection() {
    dMail_c mail;
    switch (mMenuNo) {
    case dMenuMng_c::MENU_ADDRESS:
        mail.copy(dMenuMng_c::getMail());
        setMailTo(&mail);
        dMenuMng_c::setMail(&mail);
        break;
    case dMenuMng_c::MENU_ADDRESS_LETTER: {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        u32 slot = dMenuMng_c::getPickedLetter();
        mail.copy(&player->mLetters[slot]);
        setMailTo(&mail);
        player->setLetter(&mail, slot);
        break;
    }
    }
}

// .text:0x23C8
void dMenuFriend02_c::setMailTo(dMail_c *mail) {
    if (mSelAddr == ADDR_SELF) {
        mail->setToSelf();
    } else if (isPlayerAddr(mSelAddr)) {
        dSaveTown_c *town = dSaveData_c::getTown();
        dPrivateData_c *home =
            town->mHomes.getHomePlayer(mSelAddr - ADDR_PLAYER);
        int idx =
            dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, &home->mPID);
        mail->setToPlayer(idx, FALSE);
    } else if (isAnimalAddr(mSelAddr)) {
        mail->setToVillager(mSelAddr - ADDR_ANIMAL);
    } else if (isFriendAddr(mSelAddr)) {
        mail->setToFriend(mSelAddr - ADDR_FRIEND);
    }
}

// .text:0x24DC
void dMenuFriend02_c::showTabTutorial() {
    if (isTab(mCursor)) {
        int msg;
        const char *pane;
        if (mCursor == CURSOR_TAB + MODE_TOWN) {
            pane = "topTbIcn_00";
            msg = dObjTutorial_c::MSG_TOWN;
        } else {
            pane = "topTbIcn_A_01";
            msg = dObjTutorial_c::MSG_ANOTHER_TOWN;
        }
        if (dObjTutorial_c::request(TUTORIAL_ADDRESS, msg, TRUE)) {
            mVec3_c pos;
            m2d::getScreenPos(&pos, mTabLyt.findPane(pane));
            mVec2_c pos2(static_cast<const float *>(pos));
            dObjTutorial_c::setMsg(&pos2, msg, 0);
        }
    }
}
