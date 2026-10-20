#include <game/game/d_menu_point.hpp>
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

namespace dMenuPoint {
// .text:0x5C
BASE_PROFILE(MENU_POINT, dMenuPoint_c)
} // namespace dMenuPoint

static const char *l_allPane = "N_all";

// .text:0x1C8
int dMenuPoint_c::create() {
    BOOL resLoaded = mRes.load("Layout/point_win.arc");
    BOOL loaded = resLoaded & mBtn.load();
    loaded &= mSlct.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "point_win_00.brlyt");
    mLyt.calc();
    mLyt.setVisible("B_msg_00", false);
    mLyt.setVisible("B_msg_04", false);
    mLyt.setVisible("B_msg_05", false);
    mPriceLyt.build(&mRes, "point_price_00.brlyt");
    mPriceLyt.calc();
    mPriceLyt.mPriority = LYT_PRIORITY;
    mRemainLyt.build(&mRes, "point_remain_00.brlyt");
    mRemainLyt.mPriority = LYT_PRIORITY;
    initRemain();
    mRemainLyt.calc();
    mLyt.setAnm(&mScrlIconAnm, "point_win_00_scrlIcon_00.brlan", nullptr);
    initRows();
    initAnms();
    mBtn.create();
    initButtons();
    mSlct.create();
    mPriceWord.setTextBox(mPriceLyt.findTextBox("T_price_00"));
    onFlag(FLAG_DRAW);
    mSel = SEL_NONE;
    updateSelItem();
    mScroll = SCROLL_MIN;
    refreshList();
    setScrlBtnAnms();
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    mRemainAnm.setNext(REMAIN_ANM_IN);
    setScrlAnm(SCROLL_DOWN);
    setSubState(SUB_MAIN);
    mZClear.set(150.0f, -140.0f, -240.0f, 0.0f, -499.9f);
    onFlag(FLAG_QUIT);
    return SUCCEEDED;
}

// .text:0x3FC
int dMenuPoint_c::doDelete() {
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

// .text:0x46C
int dMenuPoint_c::stateMain() {
    preMain();
    executeSubState();
    postMain();
    return SUCCEEDED;
}

// .text:0x4AC
int dMenuPoint_c::stateAnm() {
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

// .text:0x560
int dMenuPoint_c::stateClose() {
    if (!isFlag(FLAG_QUIT)) {
        startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
        mRemainAnm.setNext(REMAIN_ANM_IN);
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x5D8
int dMenuPoint_c::execute() {
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
    calcRemainAnm();
    if (isFlag(FLAG_REMAIN_CALC)) {
        offFlag(FLAG_REMAIN_CALC);
        mRemainLyt.calc();
    }
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mPriceLyt.calc();
    }
    mScrlIconAnm.play();
    mLyt.calc();
    mAngle += mAng(PREVIEW_TURN_SPEED);
    return result;
}

// .text:0x6F0
int dMenuPoint_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mPriceLyt.entry();
        mRemainLyt.entry();
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

// .text:0x78C
void dMenuPoint_c::subStateMain() {
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

// .text:0x854
void dMenuPoint_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x890
void dMenuPoint_c::subStateScroll() {
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

// .text:0x954
void dMenuPoint_c::subStateDialog() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_ANSWER);
    }
}

// .text:0x9B4
void dMenuPoint_c::subStateAnswer() {
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
            dMenuMng_c::setResult(RESULT_SWAP);
            subPoints(getNthPrice(mSel));
            break;
        }
    }
}

// .text:0xA84
void dMenuPoint_c::subStateDialogClose() {
    if (mSlct.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_DIALOG);
    }
}

// .text:0xAE0
void dMenuPoint_c::executeSubState() {
    typedef void (dMenuPoint_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuPoint_c::subStateMain,   &dMenuPoint_c::subStatePush,
        &dMenuPoint_c::subStateScroll, &dMenuPoint_c::subStateDialog,
        &dMenuPoint_c::subStateAnswer, &dMenuPoint_c::subStateDialogClose,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xB18
void dMenuPoint_c::preMain() {
}

// .text:0xB1C
void dMenuPoint_c::postMain() {
}

// .text:0xB20
void dMenuPoint_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
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
}

// .text:0xC48
int dMenuPoint_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn;
    }
    mVec2_c rowPos = pos;
    int cursor = m2d::hitButton(&rowPos, &mLyt, CURSOR_ROW, CURSOR_SCRL_DOWN,
                                -1, -1, 1.0f);
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

// .text:0xD30
void dMenuPoint_c::checkPadMove() {
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
            setScrlBtnAnms();
            playSeDecide();
        } else if (dPad::isRightTriggered() && canScrollDown()) {
            int scroll = mScroll + SCROLL_PAGE;
            mScroll = scroll;
            int max = ITEM_NUM - VISIBLE_ROWS;
            if (mScroll > max) {
                mScroll = max;
            }
            refreshList();
            setScrlBtnAnms();
            playSeDecide();
        }
    }
}

// .text:0xE84
void dMenuPoint_c::pushCursor() {
    int row;
    int cursor = mCursor;
    if (isBtn(cursor)) {
        mBtn.push(cursor, FALSE);
    } else if (isRow(cursor)) {
        row = cursor - CURSOR_ROW;
        int sel = row + mScroll;
        if (sel < 0 || sel >= ITEM_NUM) {
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
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xFD0
BOOL dMenuPoint_c::isPushEnd() {
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

// .text:0x1090
void dMenuPoint_c::finishPush() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        switch (cursor) {
        case BTN_QUIT:
            startCloseAnm();
            break;
        case BTN_UNUSED:
            break;
        case BTN_CONFIRM:
            u32 price = getNthPrice(mSel);
            if (getPoints() < price) {
                openNeedPointsDialog();
            } else {
                openDialog();
            }
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

// .text:0x1210
void dMenuPoint_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    mRemainAnm.setNext(REMAIN_ANM_OUT);
    dMenuMng_c::setResult(RESULT_CANCEL);
    onFlag(FLAG_QUIT);
    offFlag(FLAG_PREVIEW);
}

// .text:0x1280
void dMenuPoint_c::initRows() {
    static const char *rowAnmNames[ROW_NUM] = {
        "point_win_00_T_msg_In.brlan",  "point_win_00_T_msg_Out.brlan",
        "point_win_00_T_msg_Psh.brlan", "point_win_00_T_msg_Before.brlan",
        "point_win_00_T_msg_Gry.brlan", "point_win_00_T_msg_White.brlan",
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

// .text:0x1348
void dMenuPoint_c::calcRowAnm() {
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
}

// .text:0x13E0
void dMenuPoint_c::updateRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        mRowWord[i].clear();
        int n = mScroll + i;
        u16 id = getNthItem(n);
        if (id != dItem::ITEM_ID_NONE) {
            if (getNthPrice(n) <= getPointsMax()) {
                dItem::name_c name(id);
                mRowWord[i].copy(&name, 0);
            } else {
                dString::loadString(&mRowWord[i], MSG_UNKNOWN_ITEM,
                                    "sys_2D/SYS_2D_menu");
            }
        }
        mRowWord[i].update();
    }
    onFlag(FLAG_CALC);
}

// .text:0x14D0
void dMenuPoint_c::setRowAnms(int hold, int shift) {
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

// .text:0x157C
void dMenuPoint_c::updateSelBase() {
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

// .text:0x160C
void dMenuPoint_c::refreshList() {
    updateRowWords();
    setRowAnms(-1, 0);
    updateSelBase();
}

// .text:0x1650
void dMenuPoint_c::selectItem(int idx) {
    mSel = idx;
    setRowAnms(-1, 0);
    updateSelBase();
    updateSelItem();
}

// .text:0x1698
void dMenuPoint_c::updateSelItem() {
    u16 item = getNthItem(mSel);
    if (item != mItem) {
        mItem = item;
        BOOL canSwap = item != dItem::ITEM_ID_NONE ? TRUE : FALSE;
        mPriceWord.clear();
        if (canSwap != FALSE) {
            u16 price = getNthPrice(mSel);
            dString::Word_c word;
            dScript::formatNumber(&word, price, 12, dScript::NUM_FORMAT_REGION,
                                  0);
            dString::setWord(0, &word);
            dString::loadString(&mPriceWord, MSG_POINTS, "sys_2D/SYS_2D_menu");
        }
        mPriceWord.update();
        BOOL disable;
        int price = getNthPrice(mSel);
        canSwap = FALSE;
        if (price != 0 && price <= getPointsMax()) {
            canSwap = TRUE;
        }
        disable = !canSwap;
        setConfirmDisable(disable);
        if (canSwap) {
            updatePreview();
        } else {
            clearPreview();
        }
        BOOL visible = disable && price != 0;
        mLyt.setVisible("N_question_00", visible);
    }
}

// .text:0x1818
void dMenuPoint_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
}

// .text:0x1878
void dMenuPoint_c::setConfirmDisable(BOOL disable) {
    mBtn.setDisable(BTN_CONFIRM, disable != FALSE);
}

// .text:0x1894
void dMenuPoint_c::initAnms() {
    static const char *scrlUpNames[5] = {
        "point_win_00_scrlBtn_00_In.brlan",
        "point_win_00_scrlBtn_00_Out.brlan",
        "point_win_00_scrlBtn_00_Psh.brlan",
        "point_win_00_scrlBtn_00_CanPsh.brlan",
        "point_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrlDownNames[5] = {
        "point_win_00_scrlBtn_01_In.brlan",
        "point_win_00_scrlBtn_01_Out.brlan",
        "point_win_00_scrlBtn_01_Psh.brlan",
        "point_win_00_scrlBtn_01_CanPsh.brlan",
        "point_win_00_scrlBtn_01_NoPsh.brlan",
    };
    mScrlBtnAnm[0].init(&mLyt, &mRes, scrlUpNames, 5);
    mScrlBtnAnm[1].init(&mLyt, &mRes, scrlDownNames, 5);
}

// .text:0x18F4
void dMenuPoint_c::calcScrlBtnAnm() {
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

// .text:0x19E4
void dMenuPoint_c::setScrlAnm(int dir) {
    static const char *names[2] = {
        "point_win_00_scrlDown.brlan",
        "point_win_00_scrlUp.brlan",
    };
    mLyt.setAnm(&mScrlAnm, names[dir], nullptr);
}

// .text:0x1A04
BOOL dMenuPoint_c::startScroll(int dir) {
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

// .text:0x1B10
void dMenuPoint_c::setScrlBtnAnms() {
    mScrlBtnAnm[0].start(canScrollUp() ? SCRL_ANM_CAN_PUSH : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].start(canScrollDown() ? SCRL_ANM_CAN_PUSH
                                         : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x1B90
bool dMenuPoint_c::canScroll(int dir) {
    if (dir == SCROLL_UP) {
        return canScrollUp();
    }
    return canScrollDown();
}

// .text:0x1BA0
bool dMenuPoint_c::canScrollUp() {
    return mScroll > SCROLL_MIN;
}

// .text:0x1BC0
bool dMenuPoint_c::canScrollDown() {
    return mScroll + VISIBLE_ROWS < ITEM_NUM;
}

// .text:0x1BE0
u16 dMenuPoint_c::getNthItem(int n) {
    if (n < 0 || n >= ITEM_NUM) {
        return dItem::ITEM_ID_NONE;
    }
    static const int items[ITEM_NUM] = {
        dItem::ITEM_IDX_BANANA,        dItem::ITEM_IDX_TRIPLE_SHELLS,
        dItem::ITEM_IDX_YOSHIS_EGG,    dItem::ITEM_IDX_BAD_BRO_STACHE,
        dItem::ITEM_IDX_TOAD_HAT,      dItem::ITEM_IDX_PEACHS_PARASOL,
        dItem::ITEM_IDX_WARIO_HAT,     dItem::ITEM_IDX_VARIA_SUIT,
        dItem::ITEM_IDX_HEROS_CLOTHES, dItem::ITEM_IDX_SAMUS_MASK,
        dItem::ITEM_IDX_HEROS_CAP,     dItem::ITEM_IDX_MAJORAS_MASK,
        dItem::ITEM_IDX_MIDNAS_MASK,   dItem::ITEM_IDX_KART,
    };
    dItem::Item item;
    item.setFromIndex(items[n]);
    return item.getId();
}

// .text:0x1C40
u16 dMenuPoint_c::getNthPrice(int n) {
    if (n < 0 || n >= ITEM_NUM) {
        return 0;
    }
    static const u16 prices[ITEM_NUM] = {500,  800,  1000, 2500, 3000,
                                         4000, 5000, 5000, 5000, 6000,
                                         6000, 6000, 6500, 10000};
    return prices[n];
}

// .text:0x1C6C
void dMenuPoint_c::updatePreview() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        dItem::Item item(mItem);
        dFtrm_c::getCurrent()->create(&item);
    }
}

// .text:0x1CC0
void dMenuPoint_c::clearPreview() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        dFtrm_c::getCurrent()->removeAll();
    }
}

// .text:0x1CFC
void dMenuPoint_c::setupFtr() {
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

// .text:0x1E04
void dMenuPoint_c::calcPreview() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        setupFtr();
    }
}

// .text:0x1E4C
void dMenuPoint_c::order_c::execute() {
    mpFtr->entryMdl();
}

// .text:0x1E60
u16 dMenuPoint_c::getPoints() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mNookPoints;
}

// .text:0x1E84
u16 dMenuPoint_c::getPointsMax() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mNookPointsMax;
}

// .text:0x1EA8
void dMenuPoint_c::subPoints(int points) {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->subNookPoints(points);
}

// .text:0x1EDC
void dMenuPoint_c::openDialog() {
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    dItem::name_c name(mItem);
    dString::setWord(0, &name);
    mSlct.setMsg(MSG_SWAP, nullptr);
    mSlct.setBtn(0, MSG_NO, nullptr, SE_cancel_little);
    mSlct.setBtn(1, MSG_YES, nullptr, 0);
    mSlct.setBtnVisible(2, false);
}

// .text:0x1FB8
void dMenuPoint_c::openNeedPointsDialog() {
    playSeCanNot();
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mSlct.setMsg(MSG_NEED_POINTS, nullptr);
    mSlct.setBtnVisible(0, false);
    mSlct.setBtnVisible(1, false);
    mSlct.setBtn(2, MSG_OK, nullptr, 0);
}

// .text:0x2064
void dMenuPoint_c::initRemain() {
    static const char *remainAnmNames[2] = {
        "point_remain_00_moneyIn.brlan",
        "point_remain_00_moneyOut.brlan",
    };
    mRemainAnm.init(&mRemainLyt, &mRes, remainAnmNames, 2);
    mRemainAnm.start(REMAIN_ANM_OUT);
    mRemainAnm.setToEnd();
    u16 points = getPoints();
    dString::Word_c word;
    dScript::formatNumber(&word, points, 12, dScript::NUM_FORMAT_REGION, 0);
    dString::setWord(0, &word);
    mRemainLyt.setMessage("T_remain_01", MSG_POINTS, nullptr);
    mRemainLyt.setMessage("T_remain_00", MSG_LEFT, nullptr);
}

// .text:0x213C
void dMenuPoint_c::calcRemainAnm() {
    if (mRemainAnm.play()) {
        onFlag(FLAG_REMAIN_CALC);
    }
}
