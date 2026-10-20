#include <game/game/d_menu_catalog.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_a_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <stdio.h>

namespace dMenuCatalog {
void *classInit();
fProf::fBaseProfile_c g_profile_MENU_CATALOG = {
    classInit, fProf::MENU_CATALOG, fProf::DRAW_ORDER::MENU_CATALOG};

// .text:0x5C
void *classInit() {
    return new dMenuCatalog_c;
}
} // namespace dMenuCatalog

static const char *l_allPane = "N_all";
static const char *l_topTbInPsh = "N_topTb_InPsh";
static const char *l_letterPane = "N_letter_00";

// .text:0x208
int dMenuCatalog_c::create() {
    BOOL resLoaded = mRes.load("Layout/catalog.arc");
    BOOL loaded = resLoaded & mBtn.load();
    loaded &= mSlct.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "catalog_win_00.brlyt");
    mLyt.calc();
    mLyt.setVisible("B_msg_00", false);
    mLyt.setVisible("B_msg_04", false);
    mLyt.setVisible("B_msg_05", false);
    mTabLyt.build(&mRes, "catalog_tab_00.brlyt");
    mTabLyt.calc();
    mPriceLyt.build(&mRes, "catalog_price_00.brlyt");
    mPriceLyt.calc();
    mPriceLyt.mPriority = LYT_PRIORITY;
    initTabAnms();
    initRows();
    initAnms();
    mBtn.create();
    mBtn.setPriority(LYT_PRIORITY);
    initButtons();
    mSlct.create();
    mSlct.setBtn(0, MSG_NO, nullptr, SE_cancel_little);
    mSlct.setBtn(1, MSG_YES, nullptr, SE_decide);
    mSlct.setBtnVisible(2, false);
    mName.setTextBox(mPriceLyt.findTextBox("T_price_00"));
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setTabAnm(TAB_WIN_ANM_IN);
    initSlider();
    for (int i = 0; i < TAB_NUM; i++) {
        mTabSel[i] = SEL_NONE;
    }
    setTabHover(-1, TAB_HOVER_IN);
    mTab = -1;
    selectTab(TAB_FURNITURE);
    setScrlAnm(SCROLL_DOWN);
    setKind();
    setSubState(SUB_MAIN);
    mZClear.set(150.0f, -140.0f, -240.0f, 0.0f, -499.9f);
    mPaperId = -1;
    mPaperState[0] = 0;
    mPaperState[1] = 0;
    mPaperSlot = 0;
    onFlag(FLAG_QUIT);
    return SUCCEEDED;
}

// .text:0x4B8
int dMenuCatalog_c::doDelete() {
    clearPreview();
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mSlct.remove()) {
        return NOT_READY;
    }
    if (!mRes.remove()) {
        return NOT_READY;
    }
    if (!mPaper[0].remove()) {
        return NOT_READY;
    }
    bool removed = mPaper[1].remove();
    return removed;
}

// .text:0x558
int dMenuCatalog_c::stateMain() {
    preMain();
    executeSubState();
    postMain();
    return SUCCEEDED;
}

// .text:0x598
int dMenuCatalog_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    if (isFlag(FLAG_QUIT)) {
        mBtn.setPos(&mLyt, l_allPane);
    }
    mLyt.copyPos(&mPriceLyt, l_allPane, "N_all");
    mPriceLyt.calc();
    if (isState(STATE_MAIN)) {
        onFlag(FLAG_PREVIEW);
    }
    return result;
}

// .text:0x64C
int dMenuCatalog_c::stateClose() {
    if (!isFlag(FLAG_QUIT)) {
        setKind();
        startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x6C0
int dMenuCatalog_c::execute() {
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
    updatePaper(0);
    updatePaper(1);
    mBtn.execute();
    mSlct.execute();
    calcTabAnm();
    calcRowAnm();
    calcScrlBtnAnm();
    if (isFlag(FLAG_CALC_TAB)) {
        offFlag(FLAG_CALC_TAB);
        mTabLyt.calc();
    }
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mPriceLyt.calc();
    }
    mSliderIconAnm.play();
    mLyt.calc();
    mAngle += mAng(PREVIEW_TURN_SPEED);
    return result;
}

// .text:0x818
int dMenuCatalog_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mTabLyt.entry();
        mPriceLyt.entry();
        mBtn.draw();
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    if (isFlag(FLAG_PREVIEW)) {
        calcPreview();
        mZClear.entry();
    }
    return SUCCEEDED;
}

// .text:0x8B4
void dMenuCatalog_c::subStateMain() {
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

// .text:0x990
void dMenuCatalog_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x9CC
void dMenuCatalog_c::subStateScroll() {
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

// .text:0xA90
void dMenuCatalog_c::subStateDialog() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_ANSWER);
    }
}

// .text:0xAF0
void dMenuCatalog_c::subStateAnswer() {
    if (mSlct.isPushEnd()) {
        switch (mSlct.mPushedBtn) {
        case 0:
            setSubState(SUB_DIALOG_CLOSE);
            mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
            break;
        case 1:
            mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
            startCloseAnm();
            dMenuMng_c::setMenuItem(mItem);
            dMenuMng_c::setResult(RESULT_ORDER);
            break;
        }
    }
}

// .text:0xBA0
void dMenuCatalog_c::subStateDialogClose() {
    if (mSlct.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_DIALOG);
    }
}

// .text:0xBFC
void dMenuCatalog_c::subStateSlider() {
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
    if (mScroll != idx - 1) {
        setScroll(idx - 1);
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

// .text:0xD70
void dMenuCatalog_c::executeSubState() {
    typedef void (dMenuCatalog_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuCatalog_c::subStateMain,   &dMenuCatalog_c::subStatePush,
        &dMenuCatalog_c::subStateScroll, &dMenuCatalog_c::subStateDialog,
        &dMenuCatalog_c::subStateAnswer, &dMenuCatalog_c::subStateDialogClose,
        &dMenuCatalog_c::subStateSlider,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xDA8
void dMenuCatalog_c::preMain() {
}

// .text:0xDAC
void dMenuCatalog_c::postMain() {
}

// .text:0xDB0
void dMenuCatalog_c::setSelect(int cursor, BOOL select) {
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
    if (isRow(cursor)) {
        int anmNo;
        if (select) {
            anmNo = ROW_ANM_IN;
        } else {
            anmNo = ROW_ANM_OUT;
        }
        int row = cursor - CURSOR_ROW;
        m2d::AnmEx2_c *anm = &mRowAnm[row];
        anm->start(anmNo);
        anm->setToEnd();
        onFlag(FLAG_CALC);
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

// .text:0xF3C
int dMenuCatalog_c::hitCheck() {
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

// .text:0x1078
void dMenuCatalog_c::checkPadMove() {
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
            int max = mCount - VISIBLE_ROWS;
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

// .text:0x11EC
void dMenuCatalog_c::pushCursor() {
    int row;
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
    } else if (isRow(cursor)) {
        row = cursor - CURSOR_ROW;
        int sel = row + mScroll;
        if (sel < 0 || sel >= mCount) {
            return;
        }
        if (mSel != sel) {
            playSeListChoice();
        }
        selectItem(sel);
        mRowAnm[row].setNext(ROW_ANM_PUSH);
    } else if (isScrl(cursor)) {
        row = cursor - CURSOR_SCRL;
        if (canScroll(row)) {
            mScrlBtnAnm[row].setNext(SCRL_ANM_PUSH);
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

// .text:0x13C4
BOOL dMenuCatalog_c::isPushEnd() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        return !mBtn.isPushing();
    }
    if (isTab(cursor)) {
        return mTabHoverAnm.mFrameCtrl.isStop();
    }
    if (isRow(cursor)) {
        return !isFlag(FLAG_ROW_ANM);
    }
    if (isScrl(cursor)) {
        return !isFlag(FLAG_SCRL_BTN_ANM);
    }
    return TRUE;
}

// .text:0x14B0
void dMenuCatalog_c::finishPush() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        switch (cursor) {
        case BTN_QUIT:
            startCloseAnm();
            break;
        case BTN_UNUSED:
            break;
        case BTN_CONFIRM:
            mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
            setSubState(SUB_DIALOG);
            onFlag(FLAG_DIALOG);
            {
                dItem::Item item(mItem);
                dItem::name_c name(item);
                dString::setWord(0, &name);
                mSlct.setMsg(MSG_ORDER, nullptr);
            }
            break;
        }
    } else if (isTab(cursor)) {
        selectTab(cursor - CURSOR_TAB);
        setSubState(SUB_MAIN);
        startTabAnm();
    } else if (isRow(cursor)) {
        setSubState(SUB_MAIN);
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

// .text:0x16B4
void dMenuCatalog_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    setTabAnm(TAB_WIN_ANM_OUT);
    dMenuMng_c::setResult(RESULT_CANCEL);
    onFlag(FLAG_QUIT);
    offFlag(FLAG_PREVIEW);
}

// .text:0x1724
void dMenuCatalog_c::startTabAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    offFlag(FLAG_QUIT);
    offFlag(FLAG_PREVIEW);
}

// .text:0x1780
void dMenuCatalog_c::initRows() {
    static const char *rowAnmNames[ROW_NUM] = {
        "catalog_win_00_T_msg_In.brlan",  "catalog_win_00_T_msg_Out.brlan",
        "catalog_win_00_T_msg_Psh.brlan", "catalog_win_00_T_msg_Before.brlan",
        "catalog_win_00_T_msg_Gry.brlan", "catalog_win_00_T_msg_White.brlan",
    };
    for (int i = 0; i < ROW_NUM; i++) {
        mRowAnm[i].init(&mLyt, &mRes, rowAnmNames, ROW_NUM);
        mRowAnm[i].setNumber(i);
    }
    for (int i = 0; i < ROW_NUM; i++) {
        char name[32];
        sprintf(name, "T_msg_%02d", i);
        mRowWord[i].setTextBox(mLyt.findTextBox(name));
    }
}

// .text:0x1848
void dMenuCatalog_c::calcRowAnm() {
    BOOL playing = FALSE;
    for (int i = 0; i < ROW_NUM; i++) {
        playing |= mRowAnm[i].play();
    }
    if (playing) {
        onFlag(FLAG_CALC);
        onFlag(FLAG_ROW_ANM);
    } else {
        offFlag(FLAG_ROW_ANM);
    }
    mSliderAnm.play();
}

// .text:0x18E8
void dMenuCatalog_c::updateRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        mRowWord[i].clear();
        u16 id = getNthItem(mScroll + i);
        if (id != dItem::ITEM_ID_NONE) {
            dItem::name_c name(id);
            mRowWord[i].copy(&name, 0);
        }
        mRowWord[i].update();
    }
    onFlag(FLAG_CALC);
}

// .text:0x199C
void dMenuCatalog_c::setRowAnms(int hold, int shift) {
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + shift;
        int anm;
        if (hold != -1 && idx == hold) {
            anm = ROW_ANM_IN;
        } else if (idx <= 0 || idx >= VISIBLE_ROWS) {
            anm = ROW_ANM_GRAY;
        } else {
            anm = ROW_ANM_OUT;
        }
        mRowAnm[i].start(anm);
        mRowAnm[i].setToEnd();
    }
    mLyt.calc();
}

// .text:0x1A48
void dMenuCatalog_c::updateSelBase() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[32];
        sprintf(name, "slctBase_%02d", i);
        bool visible = false;
        if (mSel >= 0 && mSel == i + mScroll) {
            visible = true;
        }
        mLyt.setVisible(name, visible);
    }
}

// .text:0x1AD8
void dMenuCatalog_c::refreshList() {
    updateRowWords();
    setRowAnms(-1, 0);
    updateSelBase();
}

// .text:0x1B1C
void dMenuCatalog_c::selectItem(int idx) {
    mSel = idx;
    mTabSel[mTab] = idx;
    setRowAnms(-1, 0);
    updateSelBase();
    updateSelItem();
}

// .text:0x1B74
void dMenuCatalog_c::setTabAnm(int anm) {
    static const char *names[2] = {
        "catalog_tab_00_topTbIn.brlan",
        "catalog_tab_00_topTbOut.brlan",
    };
    mTabLyt.setAnm(&mTabAnmInOut, names[anm], nullptr);
}

// .text:0x1B94
void dMenuCatalog_c::initTabAnms() {
    static const char *names[2] = {
        "catalog_tab_00_topTbOn.brlan",
        "catalog_tab_00_topTbOff.brlan",
    };
    for (int i = 0; i < TAB_NUM; i++) {
        mTabAnm[i].init(&mTabLyt, &mRes, names, 2);
        mTabAnm[i].setNumber(i);
    }
}

// .text:0x1C14
void dMenuCatalog_c::calcTabAnm() {
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

// .text:0x1D10
void dMenuCatalog_c::setTabHover(int tab, int anm) {
    static const char *names[3] = {
        "catalog_tab_00_topTbDpdIn.brlan",
        "catalog_tab_00_topTbDpdOut.brlan",
        "catalog_tab_00_topTbPsh.brlan",
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

// .text:0x1DEC
void dMenuCatalog_c::selectTab(int tab) {
    if (mTab != -1) {
        mTabAnm[mTab].setNext(TAB_ANM_OFF);
    }
    mTab = tab;
    mTabAnm[tab].setNext(TAB_ANM_ON);
    mLyt.setVisible(l_letterPane, false);
}

// .text:0x1E6C
void dMenuCatalog_c::setKind() {
    static const int kinds[TAB_NUM] = {
        dItem::KIND_FTR,   dItem::KIND_WALL,     dItem::KIND_CARPET,
        dItem::KIND_CLOTH, dItem::KIND_UMBRELLA, dItem::KIND_CAP,
        dItem::KIND_PAPER, dItem::KIND_HANIWA,   dItem::KIND_FOSSIL,
    };
    mKind = kinds[mTab];
    mLyt.setVisible("goodsBase_00", mKind != dItem::KIND_PAPER);
    mSliderPtrnAnm.setFrame(mTab);
    mCount = getCatalog()->countRegistered(mKind);
    if (mCount <= 0) {
        mSel = SEL_NONE;
    } else {
        mSel = mTabSel[mTab];
    }
    updateSelItem();
    centerScroll();
    refreshList();
    setScrlBtnAnms();
}

// .text:0x1F54
void dMenuCatalog_c::updateSelItem() {
    u16 item = getNthItem(mSel);
    if (item != mItem) {
        mItem = item;
        bool canOrder = item != dItem::ITEM_ID_NONE;
        mName.clear();
        if (canOrder) {
            dItem::Item it(mItem);
            if (dItem::infoBank_c::get()->getBITM(it)->m_noPurchase) {
                dString::loadString(&mName, MSG_NOT_FOR_SALE,
                                    "sys_2D/SYS_2D_menu");
                canOrder = false;
            } else {
                dItem::Item priceItem(mItem);
                int price = priceItem.getShopPrice();
                dString::Word_c word;
                dScript::setNumber(&word, price, 12,
                                   dScript::NUM_FORMAT_REGION);
                dString::setWord(1, &word);
                dString::loadString(&mName, MSG_BELLS, "sys_2D/SYS_2D_menu");
            }
            updatePreview();
        } else {
            clearPreview();
        }
        mName.update();
        setConfirmDisable(!canOrder);
    }
}

// .text:0x2098
void dMenuCatalog_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
}

// .text:0x20F8
void dMenuCatalog_c::setConfirmDisable(BOOL disable) {
    mBtn.setDisable(BTN_CONFIRM, disable != FALSE);
}

// .text:0x2114
void dMenuCatalog_c::initAnms() {
    static const char *scrlUpNames[5] = {
        "catalog_win_00_scrlBtn_00_In.brlan",
        "catalog_win_00_scrlBtn_00_Out.brlan",
        "catalog_win_00_scrlBtn_00_Psh.brlan",
        "catalog_win_00_scrlBtn_00_CanPsh.brlan",
        "catalog_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrlDownNames[5] = {
        "catalog_win_00_scrlBtn_01_In.brlan",
        "catalog_win_00_scrlBtn_01_Out.brlan",
        "catalog_win_00_scrlBtn_01_Psh.brlan",
        "catalog_win_00_scrlBtn_01_CanPsh.brlan",
        "catalog_win_00_scrlBtn_01_NoPsh.brlan",
    };
    static const char *sliderNames[2] = {
        "catalog_win_00_sldr_In.brlan",
        "catalog_win_00_sldr_Out.brlan",
    };
    mScrlBtnAnm[0].init(&mLyt, &mRes, scrlUpNames, 5);
    mScrlBtnAnm[1].init(&mLyt, &mRes, scrlDownNames, 5);
    mLyt.setAnm(&mSliderPtrnAnm, "catalog_win_00_scrlIconPtrn_00.brlan",
                nullptr);
    mLyt.setAnm(&mSliderIconAnm, "catalog_win_00_scrlIcon_00.brlan", nullptr);
    mSliderAnm.init(&mLyt, &mRes, sliderNames, 2);
    mSliderAnm.setNext(SLIDER_ANM_OUT);
}

// .text:0x21C8
void dMenuCatalog_c::calcScrlBtnAnm() {
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
            setScrlAnm(SCROLL_DOWN);
            refreshList();
            offFlag(FLAG_SCROLL);
        }
        onFlag(FLAG_CALC);
    }
}

// .text:0x22B8
void dMenuCatalog_c::setScrlAnm(int dir) {
    static const char *names[2] = {
        "catalog_win_00_scrlDown.brlan",
        "catalog_win_00_scrlUp.brlan",
    };
    mLyt.setAnm(&mScrlAnm, names[dir], nullptr);
}

// .text:0x22D8
BOOL dMenuCatalog_c::startScroll(int dir) {
    if (!canScroll(dir)) {
        return FALSE;
    }
    setScrlAnm(dir);
    onFlag(FLAG_SCROLL);
    if (dir == SCROLL_UP) {
        addScroll(-1);
        updateRowWords();
        setRowAnms(-1, -1);
        updateSelBase();
    } else {
        addScroll(1);
    }
    int first, last;
    int scroll = mScroll;
    if (dir == SCROLL_UP) {
        first = ROW_ANM_WHITE;
        last = ROW_ANM_GRAY;
    } else {
        first = ROW_ANM_GRAY;
        last = ROW_ANM_WHITE;
        scroll -= 1;
    }
    if (scroll + 1 != mSel) {
        mRowAnm[1].setNext(first);
    }
    if (scroll + 4 != mSel) {
        mRowAnm[4].setNext(last);
    }
    playSePageTurn();
    return TRUE;
}

// .text:0x23E4
void dMenuCatalog_c::setScrlBtnAnms() {
    mScrlBtnAnm[0].start(canScrollUp() ? SCRL_ANM_CAN_PUSH : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].start(canScrollDown() ? SCRL_ANM_CAN_PUSH
                                         : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x2464
bool dMenuCatalog_c::canScroll(int dir) {
    if (dir == SCROLL_UP) {
        return canScrollUp();
    }
    return canScrollDown();
}

// .text:0x2474
bool dMenuCatalog_c::canScrollUp() {
    return mScroll > SCROLL_MIN;
}

// .text:0x2494
bool dMenuCatalog_c::canScrollDown() {
    return mScroll + VISIBLE_ROWS < mCount;
}

// .text:0x24B8
void dMenuCatalog_c::setScroll(int scroll) {
    mScroll = scroll;
}

// .text:0x24C0
void dMenuCatalog_c::addScroll(int delta) {
    mScroll += delta;
    updateSlider();
}

// .text:0x24D0
void dMenuCatalog_c::centerScroll() {
    int scroll = mSel - CENTER_OFFSET;
    mScroll = scroll;
    if (scroll + VISIBLE_ROWS > mCount) {
        mScroll = mCount - VISIBLE_ROWS;
    }
    if (mScroll < SCROLL_MIN) {
        mScroll = SCROLL_MIN;
    }
    updateSlider();
}

// .text:0x250C
void dMenuCatalog_c::initSlider() {
    mSliderTop = mLyt.findPane("N_sldrTop")->GetTranslate().y;
    mSliderBottom = mLyt.findPane("N_sldrBottom")->GetTranslate().y;
    mpSlider = mLyt.findPane("N_sldr_all");
}

// .text:0x257C
void dMenuCatalog_c::updateSlider() {
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

// .text:0x262C
dCatalog_c *dMenuCatalog_c::getCatalog() {
    return &dPlayerMgr::Mgr_c::getCurrentPlayer()->mCatalog;
}

// .text:0x2654
u16 dMenuCatalog_c::getNthItem(int n) {
    if (n < 0 || n >= mCount) {
        return dItem::ITEM_ID_NONE;
    }
    return getCatalog()->getNthRegistered(mKind, n);
}

// .text:0x26B8
void dMenuCatalog_c::updatePreview() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        switch (mKind) {
        case dItem::KIND_PAPER: {
            dFtrm_c::getCurrent()->removeAll();
            dItem::Item item(getNthItem(mSel));
            mPaperId = dItem::seeker_c::get()->findLike(item);
            int next = mPaperSlot == 0;
            if (mPaperState[next] == PAPER_NONE) {
                mPaperState[next] = PAPER_LOAD;
            } else {
                mPaperState[next] = PAPER_REMOVE;
            }
            break;
        }
        default:
            dFtrm_c::getCurrent()->create(&dItem::Item(mItem));
            break;
        }
    }
}

// .text:0x2778
void dMenuCatalog_c::clearPreview() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        dFtrm_c::getCurrent()->removeAll();
    }
}

// .text:0x27B4
void dMenuCatalog_c::setupFtr() {
    daFTR_c *ftr = dFtrm_c::getCurrent()->get();
    if (ftr) {
        f32 scale = ftr->getCatalogScale();
        f32 height = ftr->getHeight();
        mVec3_c pos(-30.5f, -13.0f, -168.0f);
        mMtx_c mtx;
        scale = 0.625f * scale;
        f32 y = pos.y + height;
        PSMTXTrans(mtx, pos.x, y, pos.z);
        mtx.XrotM(mAng(PREVIEW_TILT));
        mtx.YrotM(mAngle);
        mMtx_c scaleMtx;
        PSMTXScale(scaleMtx, scale, scale, scale);
        PSMTXConcat(mtx, scaleMtx, mtx);
        ftr->calcMdl(&mtx);
        mOrder.mpFtr = ftr;
        mOrder.mKind = mKind;
        m2d::addCallback(&mOrder);
    }
}

// .text:0x28CC
void dMenuCatalog_c::calcPreview() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        switch (mKind) {
        case dItem::KIND_PAPER:
            drawPaper();
            break;
        default:
            setupFtr();
            break;
        }
    }
}

// .text:0x2930
void dMenuCatalog_c::order_c::execute() {
    if (mKind == dItem::KIND_PAPER) {
        return;
    }
    mpFtr->entryMdl();
}

// .text:0x2954
void dMenuCatalog_c::updatePaper(int slot) {
    if (mPaperState[slot] == PAPER_LOAD) {
        dPaper_c *paper = &mPaper[slot];
        paper->setPaper(mPaperId, TRUE);
        if (paper->load()) {
            paper->create();
            paper->copyPos(&mLyt, "N_letter_00");
            paper->execute();
            mPaperState[slot] = PAPER_SHOWN;
            mPaperId = -1;
            mPaperSlot = slot;
        }
    }
    if (mPaperState[slot] == PAPER_REMOVE) {
        if (mPaper[slot].remove()) {
            if (mPaperId == -1) {
                mPaperState[slot] = PAPER_NONE;
            } else {
                mPaperState[slot] = PAPER_LOAD;
            }
        }
    }
}

// .text:0x2A54
void dMenuCatalog_c::drawPaper() {
    if (mPaperState[mPaperSlot] == PAPER_SHOWN) {
        mLyt.setVisible(l_letterPane, true);
        mPaper[mPaperSlot].draw();
    }
}

// .text:0x2ABC
void dMenuCatalog_c::showTutorial(int cursor) {
    u16 msg;
    int tab = -1;
    if (isTab(cursor)) {
        tab = cursor - CURSOR_TAB;
        msg = tab + BALLOON_MSG_FURNITURE;
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

// .text:0x2B94
void dMenuCatalog_c::hideTutorial() {
    dObjTutorial_c::hide();
}
