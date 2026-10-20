#include <game/game/d_menu_haniwa.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <stdio.h>

namespace dMenuHaniwa {
// .text:0x5C
BASE_PROFILE(MENU_HANIWA, dMenuHaniwa_c)
} // namespace dMenuHaniwa

static const char *l_allPane = "N_all";

// .text:0x1B0
int dMenuHaniwa_c::create() {
    BOOL resLoaded = mRes.load("Layout/haniwa_win.arc");
    BOOL loaded = resLoaded & mBtn.load();
    loaded &= mSlct.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "haniwa_win_00.brlyt");
    mLyt.calc();
    mLyt.setVisible("B_msg_00", false);
    mLyt.setVisible("B_msg_04", false);
    mLyt.setVisible("B_msg_05", false);
    mLyt.setAnm(&mSliderIconAnm, "haniwa_win_00_scrlIcon_00.brlan", nullptr);
    initRows();
    initAnms();
    mBtn.create();
    initButtons();
    mSlct.create();
    initSlider();
    onFlag(FLAG_DRAW);
    mSel = SEL_NONE;
    updateSelItem();
    mScroll = SCROLL_MIN;
    updateSlider();
    updateItems();
    refreshList();
    setScrlBtnAnms();
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setScrlAnm(SCROLL_DOWN);
    setSubState(SUB_MAIN);
    mZClear.set(150.0f, -140.0f, -240.0f, 0.0f, -499.9f);
    onFlag(FLAG_QUIT);
    return SUCCEEDED;
}

// .text:0x37C
int dMenuHaniwa_c::doDelete() {
    clearPreview();
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mSlct.remove()) {
        return NOT_READY;
    }
    bool removed = mRes.remove();
    return removed;
}

// .text:0x3EC
int dMenuHaniwa_c::stateMain() {
    preMain();
    executeSubState();
    postMain();
    return SUCCEEDED;
}

// .text:0x42C
int dMenuHaniwa_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    if (isFlag(FLAG_QUIT)) {
        mBtn.setPos(&mLyt, l_allPane);
    }
    if (isState(STATE_MAIN)) {
        onFlag(FLAG_PREVIEW);
    }
    return result;
}

// .text:0x4B0
int dMenuHaniwa_c::stateClose() {
    if (!isFlag(FLAG_QUIT)) {
        startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x51C
int dMenuHaniwa_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    int cursor = mCursor;
    if (cursor != CURSOR_NONE && cursor != prev) {
        playSeCursor();
    }
    mBtn.execute();
    mSlct.execute();
    calcRowAnm();
    calcScrlBtnAnm();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
    }
    mSliderIconAnm.play();
    mLyt.calc();
    mAngle += mAng(PREVIEW_TURN_SPEED);
    return result;
}

// .text:0x5E4
int dMenuHaniwa_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
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

// .text:0x670
void dMenuHaniwa_c::subStateMain() {
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
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                pushCursor();
                return;
            }
        }
        checkPadMove();
    }
}

// .text:0x738
void dMenuHaniwa_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x774
void dMenuHaniwa_c::subStateScroll() {
    if (!isFlag(FLAG_SCROLL)) {
        setScrlBtnAnms();
        if (mPad::g_currentCore->down(WPAD_BUTTON_A) ||
            (mCursor == CURSOR_SCRL_UP && dPad::isUpHeld()) ||
            (mCursor == CURSOR_SCRL_DOWN && dPad::isDownHeld())) {
            if (startScroll(mCursor - CURSOR_SCRL)) {
                return;
            }
        }
        setSubState(SUB_MAIN);
    }
}

// .text:0x838
void dMenuHaniwa_c::subStateDialog() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_ANSWER);
    }
}

// .text:0x898
void dMenuHaniwa_c::subStateAnswer() {
    if (mSlct.isPushEnd()) {
        switch (mSlct.mPushedBtn) {
        case 0:
        case 2:
            setSubState(SUB_DIALOG_CLOSE);
            mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
            break;
        case 1:
            mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
            startCloseAnm();
            dMenuMng_c::setMenuItem(mItem);
            dMenuMng_c::setResult(RESULT_TAKE);
            dPrivateGyroids_c *gyroids = getGyroids();
            gyroids->reset(dItem::Item(mItem));
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            player->pickUp(dItem::Item(mItem), FALSE);
            break;
        }
    }
}

// .text:0x980
void dMenuHaniwa_c::subStateDialogClose() {
    if (mSlct.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_DIALOG);
    }
}

// .text:0x9DC
void dMenuHaniwa_c::subStateSlider() {
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

// .text:0xB4C
void dMenuHaniwa_c::executeSubState() {
    typedef void (dMenuHaniwa_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuHaniwa_c::subStateMain,   &dMenuHaniwa_c::subStatePush,
        &dMenuHaniwa_c::subStateScroll, &dMenuHaniwa_c::subStateDialog,
        &dMenuHaniwa_c::subStateAnswer, &dMenuHaniwa_c::subStateDialogClose,
        &dMenuHaniwa_c::subStateSlider,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xB84
void dMenuHaniwa_c::preMain() {
}

// .text:0xB88
void dMenuHaniwa_c::postMain() {
}

// .text:0xB8C
void dMenuHaniwa_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isRow(cursor)) {
        mRowAnm[cursor - CURSOR_ROW].startEnd(select == FALSE);
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

// .text:0xCB8
int dMenuHaniwa_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn;
    }
    mVec2_c rowPos = pos;
    int cursor =
        m2d::hitButton(&rowPos, &mLyt, CURSOR_ROW, CURSOR_SLIDER, -1, -1, 1.0f);
    if (cursor == CURSOR_SCRL_UP && !canScrollUp()) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCRL_DOWN && !canScrollDown()) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SLIDER && mCount <= SLIDER_OFFSET) {
        cursor = CURSOR_NONE;
    }
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0xDB8
void dMenuHaniwa_c::checkPadMove() {
    if (dPad::isUpTriggered() && canScrollUp()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        mCursor = CURSOR_SCRL_UP;
        pushCursor();
    } else if (dPad::isDownTriggered() && canScrollDown()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
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
        } else if (dPad::isRightTriggered() && canScrollDown()) {
            int scroll = mScroll + SCROLL_PAGE;
            mScroll = scroll;
            int max = mCount - VISIBLE_ROWS;
            if (max < 0) {
                max = 0;
            }
            if (mScroll > max) {
                mScroll = max;
            }
            refreshList();
            updateSlider();
            setScrlBtnAnms();
            playSeDecide();
        }
    }
}

// .text:0xF2C
void dMenuHaniwa_c::pushCursor() {
    int row;
    int cursor = mCursor;
    if (isBtn(cursor)) {
        mBtn.push(cursor, FALSE);
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

// .text:0x10C0
BOOL dMenuHaniwa_c::isPushEnd() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        return !mBtn.isPushing();
    }
    if (isRow(cursor)) {
        return !isFlag(FLAG_ROW_ANM);
    }
    if (isScrl(cursor)) {
        return !isFlag(FLAG_SCRL_BTN_ANM);
    }
    return TRUE;
}

// .text:0x1180
void dMenuHaniwa_c::finishPush() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        switch (cursor) {
        case BTN_QUIT:
            startCloseAnm();
            break;
        case BTN_UNUSED:
            break;
        case BTN_CONFIRM:
            openDialog();
            break;
        }
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

// .text:0x12C8
void dMenuHaniwa_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(RESULT_CANCEL);
    onFlag(FLAG_QUIT);
    offFlag(FLAG_PREVIEW);
}

// .text:0x132C
void dMenuHaniwa_c::initRows() {
    static const char *rowAnmNames[ROW_NUM] = {
        "haniwa_win_00_T_msg_In.brlan",  "haniwa_win_00_T_msg_Out.brlan",
        "haniwa_win_00_T_msg_Psh.brlan", "haniwa_win_00_T_msg_Before.brlan",
        "haniwa_win_00_T_msg_Gry.brlan", "haniwa_win_00_T_msg_White.brlan",
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

// .text:0x13F4
void dMenuHaniwa_c::calcRowAnm() {
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

// .text:0x1494
void dMenuHaniwa_c::updateRowWords() {
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

// .text:0x1548
void dMenuHaniwa_c::setRowAnms(int hold, int shift) {
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

// .text:0x15F4
void dMenuHaniwa_c::updateSelBase() {
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

// .text:0x1684
void dMenuHaniwa_c::refreshList() {
    updateRowWords();
    setRowAnms(-1, 0);
    updateSelBase();
}

// .text:0x16C8
void dMenuHaniwa_c::selectItem(int idx) {
    mSel = idx;
    setRowAnms(-1, 0);
    updateSelBase();
    updateSelItem();
}

// .text:0x1710
void dMenuHaniwa_c::updateSelItem() {
    u16 item = getNthItem(mSel);
    if (item != mItem) {
        mItem = item;
        bool canTake = item != dItem::ITEM_ID_NONE;
        setConfirmDisable(TRUE);
        if (canTake) {
            updatePreview();
        } else {
            clearPreview();
        }
        setConfirmDisable(!canTake);
    }
}

// .text:0x17B0
void dMenuHaniwa_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
}

// .text:0x1810
void dMenuHaniwa_c::setConfirmDisable(BOOL disable) {
    mBtn.setDisable(BTN_CONFIRM, disable != FALSE);
}

// .text:0x182C
void dMenuHaniwa_c::initAnms() {
    static const char *scrlUpNames[5] = {
        "haniwa_win_00_scrlBtn_00_In.brlan",
        "haniwa_win_00_scrlBtn_00_Out.brlan",
        "haniwa_win_00_scrlBtn_00_Psh.brlan",
        "haniwa_win_00_scrlBtn_00_CanPsh.brlan",
        "haniwa_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrlDownNames[5] = {
        "haniwa_win_00_scrlBtn_01_In.brlan",
        "haniwa_win_00_scrlBtn_01_Out.brlan",
        "haniwa_win_00_scrlBtn_01_Psh.brlan",
        "haniwa_win_00_scrlBtn_01_CanPsh.brlan",
        "haniwa_win_00_scrlBtn_01_NoPsh.brlan",
    };
    static const char *sliderNames[2] = {
        "haniwa_win_00_sldr_In.brlan",
        "haniwa_win_00_sldr_Out.brlan",
    };
    mScrlBtnAnm[0].init(&mLyt, &mRes, scrlUpNames, 5);
    mScrlBtnAnm[1].init(&mLyt, &mRes, scrlDownNames, 5);
    mSliderAnm.init(&mLyt, &mRes, sliderNames, 2);
    mSliderAnm.setNext(SLIDER_ANM_OUT);
}

// .text:0x18B8
void dMenuHaniwa_c::calcScrlBtnAnm() {
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

// .text:0x19A8
void dMenuHaniwa_c::setScrlAnm(int dir) {
    static const char *names[2] = {
        "haniwa_win_00_scrlDown.brlan",
        "haniwa_win_00_scrlUp.brlan",
    };
    mLyt.setAnm(&mScrlAnm, names[dir], nullptr);
}

// .text:0x19C8
BOOL dMenuHaniwa_c::startScroll(int dir) {
    if (!canScroll(dir)) {
        return FALSE;
    }
    setScrlAnm(dir);
    onFlag(FLAG_SCROLL);
    if (dir == SCROLL_UP) {
        mScroll -= 1;
        updateRowWords();
        setRowAnms(-1, -1);
        updateSelBase();
    } else {
        mScroll += 1;
    }
    updateSlider();
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

// .text:0x1ADC
void dMenuHaniwa_c::setScrlBtnAnms() {
    mScrlBtnAnm[0].start(canScrollUp() ? SCRL_ANM_CAN_PUSH : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].start(canScrollDown() ? SCRL_ANM_CAN_PUSH
                                         : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x1B5C
bool dMenuHaniwa_c::canScroll(int dir) {
    if (dir == SCROLL_UP) {
        return canScrollUp();
    }
    return canScrollDown();
}

// .text:0x1B6C
bool dMenuHaniwa_c::canScrollUp() {
    return mScroll > SCROLL_MIN;
}

// .text:0x1B8C
bool dMenuHaniwa_c::canScrollDown() {
    return mScroll + VISIBLE_ROWS < mCount;
}

// .text:0x1BB0
dPrivateGyroids_c *dMenuHaniwa_c::getGyroids() {
    return &dPlayerMgr::Mgr_c::getCurrentPlayer()->mGyroids;
}

// .text:0x1BD8
void dMenuHaniwa_c::updateItems() {
    dPrivateGyroids_c *gyroids = getGyroids();
    u32 i;
    mCount = 0;
    for (i = 0; i < GYROID_NUM; i++) {
        mItems[i] = dItem::ITEM_ID_NONE;
    }
    dItem::Item item;
    for (i = 0; i < GYROID_NUM; i++) {
        item.setFromIndex(dItem::ITEM_IDX_MEGA_CLANKOID, i, 0);
        if (gyroids->isSet(item)) {
            mItems[mCount] = item;
            mCount++;
        }
    }
}

// .text:0x1D6C
u16 dMenuHaniwa_c::getNthItem(int n) {
    if (n < 0 || n >= mCount) {
        return dItem::ITEM_ID_NONE;
    }
    return mItems[n].getId();
}

// .text:0x1D9C
void dMenuHaniwa_c::updatePreview() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        dFtrm_c::getCurrent()->create(&dItem::Item(mItem));
    }
}

// .text:0x1DF0
void dMenuHaniwa_c::clearPreview() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        dFtrm_c::getCurrent()->removeAll();
    }
}

// .text:0x1E2C
void dMenuHaniwa_c::setupFtr() {
    daFTR_c *ftr = dFtrm_c::getCurrent()->get();
    if (ftr) {
        f32 scale = ftr->getCatalogScale();
        f32 height = ftr->getHeight();
        mVec3_c pos(-30.5f, -13.0f, -168.0f);
        mMtx_c mtx;
        f32 y = pos.y + height;
        PSMTXTrans(mtx, pos.x, y, pos.z);
        mtx.XrotM(mAng(PREVIEW_TILT));
        mtx.YrotM(mAngle);
        mMtx_c scaleMtx;
        PSMTXScale(scaleMtx, scale, scale, scale);
        PSMTXConcat(mtx, scaleMtx, mtx);
        ftr->calcMdl(&mtx);
        mOrder.mpFtr = ftr;
        m2d::addCallback(&mOrder);
    }
}

// .text:0x1F34
void dMenuHaniwa_c::calcPreview() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        setupFtr();
    }
}

// .text:0x1F7C
void dMenuHaniwa_c::order_c::execute() {
    mpFtr->entryMdl();
}

// .text:0x1F90
void dMenuHaniwa_c::openDialog() {
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    dItem::name_c name(mItem);
    dString::setWord(0, &name);
    mSlct.setMsg(MSG_TAKE, nullptr);
    mSlct.setBtn(0, MSG_NO, nullptr, SE_cancel_little);
    mSlct.setBtn(1, MSG_YES, nullptr, 0);
    mSlct.setBtnVisible(2, false);
}

// .text:0x206C
void dMenuHaniwa_c::initSlider() {
    mSliderTop = mLyt.findPane("N_sldrTop")->GetTranslate().y;
    mSliderBottom = mLyt.findPane("N_sldrBottom")->GetTranslate().y;
    mpSlider = mLyt.findPane("N_sldr_all");
}

// .text:0x20DC
void dMenuHaniwa_c::updateSlider() {
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
