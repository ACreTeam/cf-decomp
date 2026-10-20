#include <game/game/d_menu_museum.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <stdio.h>

namespace dMenuMuseum {
// .text:0x5C
BASE_PROFILE(MENU_MUSEUM, dMenuMuseum_c)
} // namespace dMenuMuseum

static const char *l_allPane = "N_all";
static const char *l_topTbInPsh = "N_topTb_InPsh";

// .text:0x1A0
int dMenuMuseum_c::create() {
    BOOL resLoaded = mRes.load("Layout/donate.arc");
    BOOL loaded = resLoaded & mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "donate_win_00.brlyt");
    mTabLyt.build(&mRes, "donate_tab_00.brlyt");
    initSlider();
    initTabAnms();
    initAnms();
    initRows();
    mBtn.create();
    initButtons();
    mLyt.calc();
    mTabLyt.calc();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setTabAnm(TAB_WIN_ANM_IN);
    setScrlAnm(SCROLL_UP);
    offFlag(FLAG_SCROLL);
    setTabHover(-1, TAB_HOVER_IN);
    mTab = -1;
    selectTab(TAB_FOSSIL);
    setKind();
    setSubState(SUB_MAIN);
    onFlag(FLAG_QUIT);
    return SUCCEEDED;
}

// .text:0x320
int dMenuMuseum_c::doDelete() {
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    bool removed = mRes.remove();
    return removed;
}

// .text:0x374
int dMenuMuseum_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x398
int dMenuMuseum_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    if (isFlag(FLAG_QUIT)) {
        mBtn.setPos(&mLyt, l_allPane);
    }
    return result;
}

// .text:0x3FC
int dMenuMuseum_c::stateClose() {
    if (!isFlag(FLAG_QUIT)) {
        startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
        setKind();
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x470
int dMenuMuseum_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    int cursor = mCursor;
    if (cursor != CURSOR_NONE && cursor != prev) {
        if (isTab(cursor)) {
            playSeSubMenuPopUp();
        } else {
            playSeCursor();
        }
    }
    mBtn.execute();
    calcTabAnm();
    calcScrlBtnAnm();
    calcSliderAnm();
    if (isFlag(FLAG_CALC_TAB)) {
        offFlag(FLAG_CALC_TAB);
        mTabLyt.calc();
    }
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
    }
    mSliderIconAnm.play();
    mLyt.calc();
    return result;
}

// .text:0x588
int dMenuMuseum_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mTabLyt.entry();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x5DC
void dMenuMuseum_c::subStateMain() {
    setSelect(mCursor, FALSE);
    hideTutorial();
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
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                pushCursor();
                return;
            }
            showTutorial(mCursor);
        }
        checkPadMove();
    }
}

// .text:0x6B8
void dMenuMuseum_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x6F4
void dMenuMuseum_c::subStateScroll() {
    if (!isFlag(FLAG_SCROLL)) {
        setScrlBtnAnms();
        if (mPad::g_currentCore->down(WPAD_BUTTON_A) ||
            (dPad::isUpHeld() && mCursor == CURSOR_SCRL_UP) ||
            (dPad::isDownHeld() && mCursor == CURSOR_SCRL_DOWN)) {
            if (startScroll(mCursor - CURSOR_SCRL)) {
                return;
            }
        }
        setSubState(SUB_MAIN);
    }
}

// .text:0x7B8
void dMenuMuseum_c::subStateSlider() {
    mVec3_c pos = mpSlider->GetTranslate();
    mVec2_c cursor = dObjCursor_c::getPos();
    f32 y = mDragOffset + cursor.y;
    pos.y = y;
    if (y >= mSliderTop) {
        pos.y = mSliderTop;
    } else if (y <= mSliderBottom) {
        pos.y = mSliderBottom;
    }
    f32 ratio = (pos.y - mSliderTop) / (mSliderBottom - mSliderTop);
    int idx = (int)(0.5f + (f32)(mCount - SLIDER_OFFSET) * ratio);
    int scroll = idx - 1;
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
        mpSlider->SetTranslate(pos);
    }
}

// .text:0x928
void dMenuMuseum_c::executeSubState() {
    typedef void (dMenuMuseum_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuMuseum_c::subStateMain,
        &dMenuMuseum_c::subStatePush,
        &dMenuMuseum_c::subStateScroll,
        &dMenuMuseum_c::subStateSlider,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x960
void dMenuMuseum_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isTab(cursor)) {
        int tab = cursor - CURSOR_TAB;
        if (select) {
            setTabHover(tab, TAB_HOVER_IN);
        } else {
            setTabHover(-1, TAB_HOVER_OUT);
        }
    }
    if (isScrl(cursor)) {
        int dir = cursor - CURSOR_SCRL;
        if (select) {
            mScrlBtnAnm[dir].setNext(SCRL_ANM_IN);
        } else {
            mScrlBtnAnm[dir].setNext(SCRL_ANM_OUT);
        }
    }
    if (cursor == CURSOR_SLIDER) {
        mSliderAnm.setNext(select == FALSE);
    }
}

// .text:0xA90
int dMenuMuseum_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn;
    }
    mVec2_c tabPos = pos;
    int tab = m2d::hitButton(&tabPos, &mTabLyt, CURSOR_TAB, CURSOR_TAB_LAST, -1,
                             -1, 1.0f);
    if (tab != -1) {
        return tab;
    }
    mVec2_c rowPos = pos;
    int cursor =
        m2d::hitButton(&rowPos, &mLyt, CURSOR_ROW, CURSOR_SLIDER, -1, -1, 1.0f);
    if (cursor == CURSOR_SLIDER && mCount <= SLIDER_OFFSET) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCRL_UP && !canScrollUp()) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCRL_DOWN && !canScrollDown()) {
        cursor = CURSOR_NONE;
    }
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0xBCC
void dMenuMuseum_c::checkPadMove() {
    if (dPad::isUpTriggered() && canScrollUp()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        hideTutorial();
        mCursor = CURSOR_SCRL_UP;
        pushCursor();
    } else if (dPad::isDownTriggered() && canScrollDown()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        hideTutorial();
        mCursor = CURSOR_SCRL_DOWN;
        pushCursor();
    } else {
        if (dPad::isLeftTriggered() && canScrollUp()) {
            int scroll = mScroll - SCROLL_PAGE;
            mScroll = scroll;
            if (scroll < SCROLL_MIN) {
                mScroll = SCROLL_MIN;
            }
            refreshList();
            updateSlider();
            setScrlBtnAnms();
            playSeDecide();
        }
        if (dPad::isRightTriggered() && canScrollDown()) {
            int scroll = mScroll + SCROLL_PAGE;
            int max = mCount - ROW_NUM;
            mScroll = scroll;
            if (scroll > max) {
                mScroll = max;
            }
            refreshList();
            updateSlider();
            setScrlBtnAnms();
            playSeDecide();
        }
    }
}

// .text:0xD40
void dMenuMuseum_c::pushCursor() {
    int dir;
    int cursor = mCursor;
    if (isBtn(cursor)) {
        mBtn.push(cursor, FALSE);
    } else if (isTab(cursor)) {
        int tab = cursor - CURSOR_TAB;
        if (tab == mTab) {
            return;
        }
        setTabHover(tab, TAB_HOVER_PUSH);
        playSeSubMenuPush();
    } else if (isScrl(cursor)) {
        dir = cursor - CURSOR_SCRL;
        if (canScroll(dir)) {
            mScrlBtnAnm[dir].setNext(SCRL_ANM_PUSH);
        } else {
            return;
        }
    } else if (cursor == CURSOR_SLIDER) {
        mVec2_c pos = dObjCursor_c::getPos();
        mDragOffset = mpSlider->GetTranslate().y - pos.y;
        setSubState(SUB_SLIDER);
        playSeAPush();
        return;
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xE9C
BOOL dMenuMuseum_c::isPushEnd() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        return !mBtn.isPushing();
    }
    if (isTab(cursor)) {
        return mTabHoverAnm.mFrameCtrl.isStop();
    }
    if (isScrl(cursor)) {
        return !isFlag(FLAG_SCRL_BTN_ANM);
    }
    return TRUE;
}

// .text:0xF54
void dMenuMuseum_c::finishPush() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        startCloseAnm();
    } else if (isTab(cursor)) {
        selectTab(cursor - CURSOR_TAB);
        setSubState(SUB_MAIN);
        startTabAnm();
    } else if (isScrl(cursor)) {
        if (startScroll(cursor - CURSOR_SCRL)) {
            setSubState(SUB_SCROLL);
        } else {
            setSubState(SUB_MAIN);
        }
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x1080
void dMenuMuseum_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    setTabAnm(TAB_WIN_ANM_OUT);
    onFlag(FLAG_QUIT);
}

// .text:0x10DC
void dMenuMuseum_c::startTabAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    offFlag(FLAG_QUIT);
}

// .text:0x112C
void dMenuMuseum_c::setTabAnm(int anm) {
    static const char *names[2] = {
        "donate_tab_00_topTbIn.brlan",
        "donate_tab_00_topTbOut.brlan",
    };
    mTabLyt.setAnm(&mTabAnmInOut, names[anm], nullptr);
}

// .text:0x114C
void dMenuMuseum_c::initTabAnms() {
    static const char *names[2] = {
        "donate_tab_00_topTbOn.brlan",
        "donate_tab_00_topTbOff.brlan",
    };
    for (int i = 0; i < TAB_NUM; i++) {
        mTabAnm[i].init(&mTabLyt, &mRes, names, 2);
        mTabAnm[i].setNumber(i);
    }
}

// .text:0x11CC
void dMenuMuseum_c::calcTabAnm() {
    BOOL playing = FALSE;
    if (!mTabAnmInOut.mFrameCtrl.isStop()) {
        mTabAnmInOut.play();
        playing = TRUE;
    }
    for (int i = 0; i < TAB_NUM; i++) {
        playing |= mTabAnm[i].play();
    }
    if (mTabHover != -1) {
        if (playing) {
            char name[32];
            sprintf(name, "N_topTb_%02d", mTabHover);
            mTabLyt.setPosFrom(name, l_topTbInPsh, 0.0f, 0.0f);
        }
        if (!mTabHoverAnm.mFrameCtrl.isStop()) {
            mTabHoverAnm.play();
            playing = TRUE;
        }
    }
    if (playing) {
        onFlag(FLAG_CALC_TAB);
    }
}

// .text:0x12C8
void dMenuMuseum_c::setTabHover(int tab, int anm) {
    static const char *names[3] = {
        "donate_tab_00_topTbDpdIn.brlan",
        "donate_tab_00_topTbDpdOut.brlan",
        "donate_tab_00_topTbPsh.brlan",
    };
    mTabHover = tab;
    if (tab == -1) {
        mTabLyt.setVisible(l_topTbInPsh, false);
    } else {
        mTabLyt.setVisible(l_topTbInPsh, true);
        char name[32];
        sprintf(name, "N_topTb_%02d", mTabHover);
        mTabLyt.setPosFrom(name, l_topTbInPsh, 0.0f, 0.0f);
        mTabLyt.setAnm(&mTabHoverAnm, names[anm], nullptr);
        if (anm != TAB_HOVER_PUSH) {
            mTabHoverAnm.setToEnd();
        }
        onFlag(FLAG_CALC_TAB);
    }
}

// .text:0x13A4
void dMenuMuseum_c::selectTab(int tab) {
    if (mTab != -1) {
        mTabAnm[mTab].setNext(TAB_ANM_OFF);
    }
    mTab = tab;
    mTabAnm[tab].setNext(TAB_ANM_ON);
}

// .text:0x1410
void dMenuMuseum_c::setKind() {
    static const int kinds[TAB_NUM] = {
        dItem::KIND_FOSSIL,
        dItem::KIND_INSECT,
        dItem::KIND_FISH,
        dItem::KIND_PICTURE,
    };
    mKind = kinds[mTab];
    mSliderPtrnAnm.setFrame(mTab);
    mCount = dSaveData_c::getTown()->mMuseum.countDonated(mKind);
    updateItems();
    mScroll = -1;
    updateSlider();
    refreshList();
    setScrlBtnAnms();
}

// .text:0x14B8
void dMenuMuseum_c::initButtons() {
    mBtn.setText(BTN_BACK, MSG_BACK, TRUE);
    mBtn.setVisible(BTN_UNUSED_1, false);
    mBtn.setVisible(BTN_UNUSED_2, false);
}

// .text:0x1514
void dMenuMuseum_c::initAnms() {
    static const char *scrlUpNames[5] = {
        "donate_win_00_scrlBtn_00_In.brlan",
        "donate_win_00_scrlBtn_00_Out.brlan",
        "donate_win_00_scrlBtn_00_Psh.brlan",
        "donate_win_00_scrlBtn_00_CanPsh.brlan",
        "donate_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrlDownNames[5] = {
        "donate_win_00_scrlBtn_01_In.brlan",
        "donate_win_00_scrlBtn_01_Out.brlan",
        "donate_win_00_scrlBtn_01_Psh.brlan",
        "donate_win_00_scrlBtn_01_CanPsh.brlan",
        "donate_win_00_scrlBtn_01_NoPsh.brlan",
    };
    mScrlBtnAnm[0].init(&mLyt, &mRes, scrlUpNames, 5);
    mScrlBtnAnm[1].init(&mLyt, &mRes, scrlDownNames, 5);
}

// .text:0x1574
void dMenuMuseum_c::calcScrlBtnAnm() {
    BOOL playing = FALSE;
    for (int i = 0; i < SCRL_NUM; i++) {
        playing |= mScrlBtnAnm[i].play();
    }
    if (playing) {
        onFlag(FLAG_CALC);
        onFlag(FLAG_SCRL_BTN_ANM);
    } else {
        offFlag(FLAG_SCRL_BTN_ANM);
    }
    if (isFlag(FLAG_SCROLL)) {
        mScrlAnm.play();
        if (mScrlAnm.mFrameCtrl.isStop()) {
            setScrlAnm(SCROLL_UP);
            refreshList();
            offFlag(FLAG_SCROLL);
        }
        onFlag(FLAG_CALC);
    }
}

// .text:0x1664
void dMenuMuseum_c::setScrlAnm(int dir) {
    static const char *names[2] = {
        "donate_win_00_scrlDown.brlan",
        "donate_win_00_scrlUp.brlan",
    };
    mLyt.setAnm(&mScrlAnm, names[dir], nullptr);
}

// .text:0x1684
BOOL dMenuMuseum_c::startScroll(int dir) {
    if (!canScroll(dir)) {
        return FALSE;
    }
    setScrlAnm(dir);
    onFlag(FLAG_SCROLL);
    if (dir == SCROLL_UP) {
        mScroll -= 1;
        updateSlider();
    } else {
        mScroll += 1;
        updateSlider();
        refreshList();
    }
    playSePageTurn();
    return TRUE;
}

// .text:0x1728
void dMenuMuseum_c::setScrlBtnAnms() {
    mScrlBtnAnm[0].start(canScrollUp() ? SCRL_ANM_CAN_PUSH : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].start(canScrollDown() ? SCRL_ANM_CAN_PUSH
                                         : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x17A8
bool dMenuMuseum_c::canScroll(int dir) {
    if (dir == SCROLL_UP) {
        return canScrollUp();
    }
    return canScrollDown();
}

// .text:0x17B8
bool dMenuMuseum_c::canScrollUp() {
    return mScroll > SCROLL_MIN;
}

// .text:0x17D8
bool dMenuMuseum_c::canScrollDown() {
    return mScroll + ROW_NUM < mCount;
}

// .text:0x17FC
void dMenuMuseum_c::initSlider() {
    static const char *sliderNames[2] = {
        "donate_win_00_sldr_In.brlan",
        "donate_win_00_sldr_Out.brlan",
    };
    mSliderAnm.init(&mLyt, &mRes, sliderNames, 2);
    mSliderAnm.setNext(SLIDER_ANM_OUT);
    mSliderTop = mLyt.findPane("N_sldrTop")->GetTranslate().y;
    mSliderBottom = mLyt.findPane("N_sldrBottom")->GetTranslate().y;
    mpSlider = mLyt.findPane("N_sldr_all");
}

// .text:0x1890
void dMenuMuseum_c::calcSliderAnm() {
    if (mSliderAnm.play()) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x18D4
void dMenuMuseum_c::updateSlider() {
    mVec3_c pos = mpSlider->GetTranslate();
    if (mCount - SLIDER_OFFSET <= 0) {
        pos.y = mSliderTop;
    } else {
        pos.y =
            mSliderTop + (f32)(mScroll + 1) * ((mSliderBottom - mSliderTop) /
                                               (f32)(mCount - SLIDER_OFFSET));
    }
    mpSlider->SetTranslate(pos);
}

// .text:0x1984
void dMenuMuseum_c::initRows() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[48];
        sprintf(name, "T_gift_%02d", i);
        mRowWord[i].setTextBox(mLyt.findTextBox(name));
        sprintf(name, "T_name_%02d", i);
        mDonorWord[i].setTextBox(mLyt.findTextBox(name));
    }
    mLyt.setAnm(&mSliderPtrnAnm, "donate_win_00_scrlIconPtrn_00.brlan",
                nullptr);
    mLyt.setAnm(&mSliderIconAnm, "donate_win_00_scrlIcon_00.brlan", nullptr);
}

// .text:0x1A5C
void dMenuMuseum_c::refreshList() {
    updateRowWords();
}

// .text:0x1A60
void dMenuMuseum_c::updateRowWords() {
    dHmnName::Word_c donor;
    for (int i = 0; i < ROW_NUM; i++) {
        mRowWord[i].clear();
        mDonorWord[i].clear();
        u16 id = getNthItem(i + mScroll);
        if (id != dItem::ITEM_ID_NONE) {
            dItem::name_c name(id);
            dItem::Item item(id);
            mRowWord[i].copy(&name, 0);
            if (dSaveData_c::getTown()->mMuseum.setDonorWord(&donor, item)) {
                mDonorWord[i].copy(&donor, 0);
            } else {
                dString::loadString(&mDonorWord[i], MSG_SOMEONE,
                                    "sys_2D/SYS_2D_menu");
            }
        }
        mRowWord[i].update();
        mDonorWord[i].update();
    }
    onFlag(FLAG_CALC);
}

// .text:0x1B88
u16 dMenuMuseum_c::getNthItem(int n) {
    if (n < 0 || n >= mCount) {
        return dItem::ITEM_ID_NONE;
    }
    return mItems[n];
}

// .text:0x1BB8
void dMenuMuseum_c::updateItems() {
    for (int i = 0; i < mCount; i++) {
        mItems[i] = dSaveData_c::getTown()->mMuseum.getNthDonated(mKind, i);
    }
}

// .text:0x1C2C
void dMenuMuseum_c::showTutorial(int cursor) {
    u16 msg;
    int tab = -1;
    if (isTab(cursor)) {
        tab = cursor - CURSOR_TAB;
        msg = tab + BALLOON_MSG_FOSSIL;
    }
    if (tab != -1) {
        if (dObjTutorial_c::request(0, msg, TRUE)) {
            char name[48];
            sprintf(name, "topTbIcn_%02d", tab);
            mVec3_c pos;
            m2d::getScreenPos(&pos, mTabLyt.findPane(name));
            mVec2_c pos2;
            pos2.x = pos.x;
            pos2.y = pos.y;
            dObjTutorial_c::setMsg(&pos2, msg, 0);
        }
    }
}

// .text:0x1D04
void dMenuMuseum_c::hideTutorial() {
    dObjTutorial_c::hide();
}
