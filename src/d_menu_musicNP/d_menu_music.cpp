#include <game/game/d_menu_music.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <stdio.h>

namespace dMenuMusic {
// .text:0xCC
BASE_PROFILE(MENU_MUSIC, dMenuMusic_c)
} // namespace dMenuMusic

static const char *l_allPane = "N_all";
static const char *l_squareBtnFcPane = "squareBtn_Fc";
static const char *l_squareBtnPshPane = "squareBtn_Psh";
static const char *l_circleBtnPshPane = "circleBtn_Psh";

// .text:0x188
int dMenuMusic_c::create() {
    BOOL resLoaded = mRes.load("Layout/music.arc");
    BOOL loaded = resLoaded & mBtn.load();
    loaded &= mSlct.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "music_catalog_win_00.brlyt");
    mLyt.setVisible(l_squareBtnFcPane, false);
    mLyt.setAlpha(l_squareBtnFcPane, 255);
    mBtn.create();
    initButtons();
    mSlct.create();
    initSlider();
    initAnms();
    mLyt.calc();
    initList();
    initRows();
    refreshList();
    updateButtons();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x2CC
int dMenuMusic_c::doDelete() {
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mRes.remove()) {
        return NOT_READY;
    }
    bool removed = mSlct.remove() != 0;
    return removed;
}

// .text:0x338
int dMenuMusic_c::stateMain() {
    executeSubState();
    if (isFlag(FLAG_UPDATE_LIST)) {
        offFlag(FLAG_UPDATE_LIST);
        refreshList();
    }
    stubUpdate();
    if (isFlag(FLAG_UPDATE_BTN)) {
        offFlag(FLAG_UPDATE_BTN);
        updateButtons();
    }
    return SUCCEEDED;
}

// .text:0x3C0
int dMenuMusic_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(&mLyt, l_allPane);
    return result;
}

// .text:0x410
int dMenuMusic_c::stateClose() {
    if (isFlag(FLAG_ADD) &&
        !dAcUitag::openMenuB(dAcUitag::MENU_B_ITEM_SELECT)) {
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x46C
int dMenuMusic_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    int cursor = mCursor;
    if (cursor != CURSOR_NONE && cursor != prev) {
        playSeCursor();
    }
    mBtn.execute();
    mSlct.execute();
    calcAnms();
    if (isPlaying()) {
        mPlayingAnm.play();
    }
    mScrlIconAnm.play();
    mLyt.calc();
    return result;
}

// .text:0x518
int dMenuMusic_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mBtn.draw();
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    return SUCCEEDED;
}

// .text:0x580
void dMenuMusic_c::subStateMain() {
    setSelect(mCursor, FALSE);
    dObjTutorial_c::hide();
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

// .text:0x658
void dMenuMusic_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x694
void dMenuMusic_c::subStateScroll() {
    mScrlAnm.play();
    onFlag(FLAG_CALC);
    if (mScrlAnm.isEnd()) {
        if (mCursor == CURSOR_SCRL_DOWN) {
            mScroll += 1;
            updateSlider();
            onFlag(FLAG_UPDATE_LIST);
        }
        setScrlBtnAnms();
        if (mPad::g_currentCore->down(WPAD_BUTTON_A) ||
            (dPad::isUpHeld() && mCursor == CURSOR_SCRL_UP) ||
            (dPad::isDownHeld() && mCursor == CURSOR_SCRL_DOWN)) {
            if (isEnabled(mCursor)) {
                startScroll(mCursor);
                return;
            }
        }
        mScrlAnm.start(SCRL_LIST_ANM_UP);
        setSubState(SUB_MAIN);
    }
}

// .text:0x7B0
void dMenuMusic_c::subStateDialog() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0x810
void dMenuMusic_c::subStateDialogPush() {
    if (mSlct.isPushEnd()) {
        setSubState(SUB_DIALOG_CLOSE);
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0x874
void dMenuMusic_c::subStateDialogClose() {
    if (mSlct.isWinAnmEnd()) {
        switch (mSlct.mPushedBtn) {
        case DIALOG_BTN_YES:
            ejectSong();
            break;
        }
        setSubState(SUB_MAIN);
        offFlag(FLAG_DIALOG);
    }
}

// .text:0x8E8
void dMenuMusic_c::subStateSlider() {
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

// .text:0xA58
void dMenuMusic_c::executeSubState() {
    typedef void (dMenuMusic_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuMusic_c::subStateMain,       &dMenuMusic_c::subStatePush,
        &dMenuMusic_c::subStateScroll,     &dMenuMusic_c::subStateDialog,
        &dMenuMusic_c::subStateDialogPush, &dMenuMusic_c::subStateDialogClose,
        &dMenuMusic_c::subStateSlider,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xA90
void dMenuMusic_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isRow(cursor)) {
        setRowAnm(cursor, select, 0);
    }
    if (isScrl(cursor)) {
        int dir = cursor - CURSOR_SCRL;
        mScrlBtnAnm[dir].start(select ? 0 : 1);
        mScrlBtnAnm[dir].setToEnd();
        onFlag(FLAG_CALC);
    }
    if (isSquare(cursor)) {
        int idx = cursor - CURSOR_SQUARE;
        if (select) {
            if (mHover != idx) {
                char name[48];
                sprintf(name, "N_btn_%02d", idx);
                mLyt.setPosFrom(name, l_squareBtnFcPane, 0.0f, 0.0f);
                if (mHover == -1) {
                    mLyt.setVisible(l_squareBtnFcPane, true);
                }
                mHover = idx;
            }
        } else if (mHover == idx) {
            mLyt.setVisible(l_squareBtnFcPane, false);
            mHover = -1;
        }
    }
    if (isCircle(cursor)) {
        mCircleAnm[cursor - CURSOR_CIRCLE].setNext(select == FALSE);
    }
    if (cursor == CURSOR_SLIDER) {
        mSliderAnm.setNext(select == FALSE);
    }
}

// .text:0xC94
int dMenuMusic_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn;
    }
    mVec2_c hitPos = pos;
    int cursor = m2d::hitButton(&hitPos, &mLyt, CURSOR_ROW, CURSOR_SLIDER,
                                CURSOR_CIRCLE, CURSOR_CIRCLE_LAST, 1.0f);
    if (isRow(cursor) && !isEnabled(cursor)) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SLIDER && mCount <= SLIDER_OFFSET) {
        cursor = CURSOR_NONE;
    }
    if (cursor != CURSOR_NONE && !canUse(cursor)) {
        cursor = CURSOR_NONE;
    }
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0xDB4
void dMenuMusic_c::checkPadMove() {
    if (dPad::isUpTriggered() && canScrollUp()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        dObjTutorial_c::hide();
        mCursor = CURSOR_SCRL_UP;
        pushCursor();
    } else if (dPad::isDownTriggered() && canScrollDown()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        dObjTutorial_c::hide();
        mCursor = CURSOR_SCRL_DOWN;
        pushCursor();
    } else if (dPad::isLeftTriggered() && canScrollUp()) {
        int scroll = mScroll - SCROLL_PAGE;
        mScroll = scroll;
        if (scroll < SCROLL_MIN) {
            mScroll = SCROLL_MIN;
        }
        updateSlider();
        onFlag(FLAG_UPDATE_LIST);
        setScrlBtnAnms();
        playSeDecide();
    } else if (dPad::isRightTriggered() && canScrollDown()) {
        int scroll = mScroll + SCROLL_PAGE;
        int max = mCount - SCROLL_ROWS;
        mScroll = scroll;
        if (scroll > max) {
            mScroll = max;
        }
        updateSlider();
        onFlag(FLAG_UPDATE_LIST);
        setScrlBtnAnms();
        playSeDecide();
    }
}

// .text:0xF2C
void dMenuMusic_c::pushCursor() {
    if (!isEnabled(mCursor)) {
        return;
    }
    switch (mCursor) {
    case CURSOR_ADD:
        playSeAdd();
        break;
    case CURSOR_EJECT:
        playSeEject();
        break;
    case CURSOR_PLAY:
        playSePlay();
        break;
    case CURSOR_STOP:
        playSeStop();
        break;
    }
    int cursor = mCursor;
    char name[48];
    if (isBtn(cursor)) {
        mBtn.push(cursor, FALSE);
    } else if (isRow(cursor)) {
        int row = cursor - CURSOR_ROW;
        mRowAnm[row].setNext(ROW_ANM_PUSH);
        if (mSel != row + mScroll) {
            playSeListChoice();
        }
    } else if (isScrl(cursor)) {
        mScrlBtnAnm[cursor - CURSOR_SCRL].setNext(SCRL_ANM_PUSH);
    } else if (isSquare(cursor)) {
        sprintf(name, "N_btn_%02d", cursor - CURSOR_SQUARE);
        mLyt.setPosFrom(name, l_squareBtnPshPane, 0.0f, 0.0f);
        mSquarePushAnm.setFrame(0.0f);
    } else if (isCircle(cursor)) {
        sprintf(name, "N_btn_%02d", cursor - CURSOR_CIRCLE + SQUARE_NUM);
        mLyt.setPosFrom(name, l_circleBtnPshPane, 0.0f, 0.0f);
        mCirclePushAnm.setFrame(0.0f);
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

// .text:0x11C0
BOOL dMenuMusic_c::isPushEnd() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        return !mBtn.isPushing();
    }
    if (isRow(cursor)) {
        return mRowAnm[cursor - CURSOR_ROW].isEnd();
    }
    if (isScrl(cursor)) {
        return mScrlBtnAnm[cursor - CURSOR_SCRL].isEnd();
    }
    if (isSquare(cursor)) {
        return mSquarePushAnm.mFrameCtrl.isStop();
    }
    if (isCircle(cursor)) {
        return mCirclePushAnm.mFrameCtrl.isStop();
    }
    return TRUE;
}

// .text:0x12E0
void dMenuMusic_c::finishPush() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        switch (cursor) {
        case BTN_CLOSE:
            startCloseAnm();
            break;
        }
    } else if (isRow(cursor)) {
        selectRow(cursor);
        setSubState(SUB_MAIN);
    } else if (isScrl(cursor)) {
        startScroll(cursor);
        setSubState(SUB_SCROLL);
    } else if (isSquare(cursor)) {
        switch (cursor) {
        case CURSOR_ADD:
            pushAdd();
            break;
        case CURSOR_EJECT:
            pushEject();
            break;
        default:
            setSubState(SUB_MAIN);
            break;
        }
    } else if (isCircle(cursor)) {
        switch (cursor) {
        case CURSOR_PLAY:
            playMusic();
            break;
        case CURSOR_STOP:
            stopMusic();
            break;
        default:
            setSubState(SUB_MAIN);
            break;
        }
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x14BC
void dMenuMusic_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(RESULT_CANCEL);
}

// .text:0x14FC
void dMenuMusic_c::startAddAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    onFlag(FLAG_ADD);
}

// .text:0x154C
void dMenuMusic_c::initList() {
    mPlaying = getCurrentRoomSong();
    updatePlayingName();
    mSel = -1;
    mScroll = -1;
    loadSongs();
    updateSlider();
    mHover = -1;
}

// .text:0x15B4
void dMenuMusic_c::startScroll(int cursor) {
    if (cursor == CURSOR_SCRL_UP) {
        mScroll -= 1;
        updateSlider();
        updateRowWords();
        updateSelMarks();
        resetRowAnms(-1);
        mScrlAnm.start(SCRL_LIST_ANM_DOWN);
    } else {
        mScrlAnm.start(SCRL_LIST_ANM_UP);
    }
    int first, second;
    if (cursor == CURSOR_SCRL_UP) {
        first = ROW_ANM_OUT;
        second = ROW_ANM_GRAY;
    } else {
        first = ROW_ANM_GRAY;
        second = ROW_ANM_OUT;
    }
    mRowAnm[ROW_PICK_FIRST].setNext(first);
    mRowAnm[ROW_PICK_END].setNext(second);
    playSePageTurn();
}

// .text:0x166C
void dMenuMusic_c::setScrlBtnAnms() {
    mScrlBtnAnm[0].start(canScrollUp() ? SCRL_ANM_CAN_PUSH : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].start(canScrollDown() ? SCRL_ANM_CAN_PUSH
                                         : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x16EC
bool dMenuMusic_c::canUse(int cursor) {
    switch (cursor) {
    case CURSOR_SCRL_UP:
        return canScrollUp();
    case CURSOR_SCRL_DOWN:
        return canScrollDown();
    case CURSOR_STOP:
        return isPlaying() == TRUE;
    case CURSOR_PLAY: {
        bool usable = false;
        if (!isPlaying() && mSel != -1) {
            usable = true;
        }
        return usable;
    }
    case CURSOR_ADD:
        return true;
    case CURSOR_EJECT:
        return mSel != -1;
    }
    return true;
}

// .text:0x17B0
bool dMenuMusic_c::isEnabled(int cursor) {
    switch (cursor) {
    case CURSOR_SCRL_UP:
        return canScrollUp();
    case CURSOR_SCRL_DOWN:
        return canScrollDown();
    case CURSOR_STOP:
        return isPlaying() == TRUE;
    case CURSOR_PLAY: {
        bool usable = false;
        if (!isPlaying() && mSel != -1) {
            usable = true;
        }
        return usable;
    }
    case CURSOR_ADD:
        return true;
    case CURSOR_EJECT:
        return mSel != -1;
    }
    if (isRow(cursor)) {
        int row = cursor - CURSOR_ROW;
        if (row == 0 || row >= ROW_PICK_END) {
            return false;
        }
        row += mScroll;
        return row < mCount && row >= 0;
    }
    return true;
}

// .text:0x18D8
bool dMenuMusic_c::canScrollUp() {
    return mScroll > SCROLL_MIN;
}

// .text:0x18F8
bool dMenuMusic_c::canScrollDown() {
    return mCount > mScroll + SCROLL_ROWS;
}

// .text:0x191C
void dMenuMusic_c::selectRow(int cursor) {
    int prev = mSel;
    mSel = cursor + mScroll - CURSOR_ROW;
    onFlag(FLAG_UPDATE_BTN);
    if (prev != mSel) {
        int prevCursor = prev - mScroll + CURSOR_ROW;
        if (isRow(prevCursor)) {
            setRowAnm(prevCursor, FALSE, 0);
        }
        if (isPlaying()) {
            playMusic();
        }
    }
    updateSelMarks();
}

// .text:0x19D0
void dMenuMusic_c::initSlider() {
    static const char *sliderNames[2] = {
        "music_catalog_win_00_sldr_In.brlan",
        "music_catalog_win_00_sldr_Out.brlan",
    };
    mSliderAnm.init(&mLyt, &mRes, sliderNames, 2);
    mSliderAnm.setNext(SLIDER_ANM_OUT);
    mSliderTop = mLyt.findPane("N_sldrTop")->GetTranslate().y;
    mSliderBottom = mLyt.findPane("N_sldrBottom")->GetTranslate().y;
    mpSlider = mLyt.findPane("N_sldr_all");
}

// .text:0x1A64
void dMenuMusic_c::calcSliderAnm() {
    if (mSliderAnm.play()) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x1AA8
void dMenuMusic_c::updateSlider() {
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

// .text:0x1B58
void dMenuMusic_c::setRowAnm(int cursor, BOOL highlight, int offset) {
    int row = cursor - CURSOR_ROW;
    int idx = row + offset;
    if (highlight) {
        mRowAnm[row].startEnd(ROW_ANM_IN);
    } else if (idx <= 0 || idx >= SCROLL_ROWS) {
        mRowAnm[row].startEnd(ROW_ANM_GRAY);
    } else {
        mRowAnm[row].startEnd(ROW_ANM_OUT);
    }
    onFlag(FLAG_CALC);
}

// .text:0x1BF0
void dMenuMusic_c::resetRowAnms(int offset) {
    for (int cursor = CURSOR_ROW; cursor <= CURSOR_ROW_LAST; cursor++) {
        setRowAnm(cursor, FALSE, offset);
    }
}

// .text:0x1C50
void dMenuMusic_c::refreshList() {
    resetRowAnms(0);
    updateRowWords();
    updateSelMarks();
}

// .text:0x1C90
void dMenuMusic_c::initButtons() {
    mBtn.setVisible(BTN_UNUSED_0, false);
    mBtn.setVisible(BTN_UNUSED_1, false);
    mBtn.setText(BTN_CLOSE, MSG_CLOSE, TRUE);
}

// .text:0x1CEC
void dMenuMusic_c::initAnms() {
    static const char *rowNames[5] = {
        "music_catalog_win_00_T_music_In.brlan",
        "music_catalog_win_00_T_music_Out.brlan",
        "music_catalog_win_00_T_music_Psh.brlan",
        "music_catalog_win_00_T_music_Before.brlan",
        "music_catalog_win_00_T_msg_Gry.brlan",
    };
    static const char *scrlUpNames[5] = {
        "music_catalog_win_00_scrlBtn_00_In.brlan",
        "music_catalog_win_00_scrlBtn_00_Out.brlan",
        "music_catalog_win_00_scrlBtn_00_Psh.brlan",
        "music_catalog_win_00_scrlBtn_00_CanPsh.brlan",
        "music_catalog_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrlDownNames[5] = {
        "music_catalog_win_00_scrlBtn_01_In.brlan",
        "music_catalog_win_00_scrlBtn_01_Out.brlan",
        "music_catalog_win_00_scrlBtn_01_Psh.brlan",
        "music_catalog_win_00_scrlBtn_01_CanPsh.brlan",
        "music_catalog_win_00_scrlBtn_01_NoPsh.brlan",
    };
    static const char *scrlNames[2] = {
        "music_catalog_win_00_scrlDown.brlan",
        "music_catalog_win_00_scrlUp.brlan",
    };
    static const char *circleNames[4] = {
        "music_catalog_win_00_circleBtn_In.brlan",
        "music_catalog_win_00_circleBtn_Out.brlan",
        "music_catalog_win_00_circleBtn_Ylw.brlan",
        "music_catalog_win_00_circleBtn_Gry.brlan",
    };
    static const char *squareNames[2] = {
        "music_catalog_win_00_squareBtn_Ylw.brlan",
        "music_catalog_win_00_squareBtn_Gry.brlan",
    };
    static const char *frmNames[2] = {
        "music_catalog_win_00_Frm_Off.brlan",
        "music_catalog_win_00_Frm_On.brlan",
    };
    for (int i = 0; i < ROW_NUM; i++) {
        mRowAnm[i].init(&mLyt, &mRes, rowNames, 5);
        mRowAnm[i].setNumber(i);
    }
    mScrlBtnAnm[0].init(&mLyt, &mRes, scrlUpNames, 5);
    mScrlBtnAnm[0].start(SCRL_ANM_OUT);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].init(&mLyt, &mRes, scrlDownNames, 5);
    mScrlBtnAnm[1].start(SCRL_ANM_OUT);
    mScrlBtnAnm[1].setToEnd();
    mScrlAnm.init(&mLyt, &mRes, scrlNames, 2);
    mScrlAnm.start(SCRL_LIST_ANM_UP);
    for (int i = 0; i < CIRCLE_NUM; i++) {
        mCircleAnm[i].init(&mLyt, &mRes, circleNames, 4);
        mCircleAnm[i].setNumber(i + SQUARE_NUM);
    }
    for (int i = 0; i < SQUARE_NUM; i++) {
        mSquareAnm[i].init(&mLyt, &mRes, squareNames, 2);
        mSquareAnm[i].setNumber(i);
    }
    mLyt.setAnm(&mSquarePushAnm, "music_catalog_win_00_squareBtn_Psh.brlan",
                nullptr);
    mSquarePushAnm.setToEnd();
    mLyt.setAnm(&mCirclePushAnm, "music_catalog_win_00_circleBtn_Psh.brlan",
                nullptr);
    mCirclePushAnm.setToEnd();
    mLyt.setAnm(&mScrlIconAnm, "music_catalog_win_00_scrlIcon_00.brlan",
                nullptr);
    mLyt.setAnm(&mPlayingAnm, "music_catalog_win_00_playing_00.brlan", nullptr);
    mFrmAnm.init(&mLyt, &mRes, frmNames, 2);
}

// .text:0x1EDC
void dMenuMusic_c::calcAnms() {
    int i;
    BOOL playing = FALSE;
    for (i = 0; i < ROW_NUM; i++) {
        playing |= mRowAnm[i].play();
    }
    for (i = 0; i < SCRL_NUM; i++) {
        playing |= mScrlBtnAnm[i].play();
    }
    for (i = 0; i < CIRCLE_NUM; i++) {
        playing |= mCircleAnm[i].play();
    }
    for (i = 0; i < SQUARE_NUM; i++) {
        playing |= mSquareAnm[i].play();
    }
    if (!mSquarePushAnm.mFrameCtrl.isStop()) {
        playing = TRUE;
        mSquarePushAnm.play();
    }
    if (!mCirclePushAnm.mFrameCtrl.isStop()) {
        playing = TRUE;
        mCirclePushAnm.play();
    }
    playing |= mFrmAnm.play();
    if (playing) {
        onFlag(FLAG_CALC);
    }
    calcSliderAnm();
}

// .text:0x200C
void dMenuMusic_c::setFrmAnm(bool on) {
    mFrmAnm.setNext(on);
}

// .text:0x2014
void dMenuMusic_c::initRows() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[32];
        sprintf(name, "T_music_%02d", i);
        mRowWord[i].setTextBox(mLyt.findTextBox(name));
    }
}

// .text:0x209C
void dMenuMusic_c::updateSelMarks() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[32];
        sprintf(name, "slctBase_%02d", i);
        bool visible = false;
        if (mSel != -1 && mSel == mScroll + i) {
            visible = true;
        }
        mLyt.setVisible(name, visible);
    }
}

// .text:0x212C
void dMenuMusic_c::updateRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[48];
        int song = i + mScroll;
        sprintf(name, "B_music_%02d", i);
        mRowWord[i].clear();
        if (song >= 0 && song < mCount) {
            wchar_t *str = getSongName(song);
            if (str) {
                mRowWord[i].set(str, 0);
                mLyt.setVisible(name, true);
            } else {
                mLyt.setVisible(name, false);
            }
        }
        mRowWord[i].update();
    }
}

// .text:0x2200
wchar_t *dMenuMusic_c::getSongName(int idx) {
    if (idx >= mCount) {
        return nullptr;
    }
    static dItem::name_c name;
    setItemName(&name, mSongs[idx]);
    return name.getText();
}

// .text:0x22C4
void dMenuMusic_c::updatePlayingName() {
    dItem::name_c name;
    if (!mPlaying.isValid()) {
        name.clear();
    } else {
        setItemName(&name, mPlaying);
    }
    mLyt.setWord("T_msg_00", &name);
}

// .text:0x2348
dPrivateSongs_c *dMenuMusic_c::getSongs() {
    return &dPlayerMgr::Mgr_c::getCurrentPlayer()->mSongs;
}

// .text:0x2370
void dMenuMusic_c::loadSongs() {
    dPrivateSongs_c *songs = getSongs();
    mCount = 0;
    for (int i = 0; i < SONG_NUM; i++) {
        mSongs[i] = dItem::ITEM_ID_NONE;
    }
    dItem::Item item;
    for (u32 i = 0; i < SONG_NUM; i++) {
        item.setFromIndex(dItem::ITEM_IDX_KK_CHORALE, i, FALSE);
        if (songs->isSet(item)) {
            mSongs[mCount] = item;
            if (item.isSame(mPlaying)) {
                mSel = mCount;
            }
            mCount++;
        }
    }
    setScrlBtnAnms();
}

// .text:0x24E4
void dMenuMusic_c::openMessage(u16 msg) {
    mSlct.setBtnVisible(DIALOG_BTN_NO, false);
    mSlct.setBtnVisible(DIALOG_BTN_YES, false);
    mSlct.setBtn(DIALOG_BTN_OK, MSG_OK, nullptr, 0);
    playSeCanNot();
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mSlct.setMsg(msg, nullptr);
}

// .text:0x259C
void dMenuMusic_c::openEjectDialog() {
    mSlct.setBtn(DIALOG_BTN_NO, MSG_NO, nullptr, SE_cancel_little);
    mSlct.setBtn(DIALOG_BTN_YES, MSG_YES, nullptr, 0);
    mSlct.setBtnVisible(DIALOG_BTN_OK, false);
    dItem::name_c name(mSongs[mSel]);
    dString::setWord(0, &name);
    mSlct.setMsg(MSG_EJECT, nullptr);
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
}

// .text:0x2684
void dMenuMusic_c::stopMusic() {
    dFTR::switchOffAll();
    mPlaying = dItem::ITEM_ID_NONE;
    onFlag(FLAG_UPDATE_LIST);
    onFlag(FLAG_UPDATE_BTN);
    setSubState(SUB_MAIN);
    updatePlayingName();
}

// .text:0x26F4
void dMenuMusic_c::playMusic() {
    if (isPlaying()) {
        dFTR::switchOffAll();
    }
    mPlaying = mSongs[mSel];
    dFTR::playMusic(&mPlaying);
    updatePlayingName();
    setSubState(SUB_MAIN);
    onFlag(FLAG_UPDATE_LIST);
    onFlag(FLAG_UPDATE_BTN);
}

// .text:0x2780
bool dMenuMusic_c::isPlaying() {
    return mPlaying.isValid();
}

// .text:0x27A4
void dMenuMusic_c::pushEject() {
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) == -1) {
        openMessage(MSG_POCKETS_FULL);
    } else if (isSongInOtherCurrentRoom(mSongs[mSel])) {
        openMessage(MSG_PLAYING_ELSEWHERE);
    } else {
        openEjectDialog();
    }
}

// .text:0x2824
void dMenuMusic_c::ejectSong() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int pocket = player->findEmptyPocket(0);
    player->setPocket(&mSongs[mSel], pocket, FALSE);
    getSongs()->reset(mSongs[mSel]);
    if (isPlaying()) {
        stopMusic();
    }
    mSel = -1;
    loadSongs();
    if (mCount < mScroll + SCROLL_ROWS) {
        int scroll = mCount - SCROLL_ROWS;
        mScroll = scroll;
        if (scroll < SCROLL_MIN) {
            mScroll = SCROLL_MIN;
        }
    }
    updateSlider();
    onFlag(FLAG_UPDATE_LIST);
    onFlag(FLAG_UPDATE_BTN);
}

// A pocket that is not a song is masked out of the pocket picker.
static BOOL isNotMusic(void *pocket, int flag) {
    if (flag) {
        return TRUE;
    }
    return ((dItem::Item *)pocket)->getKind() != dItem::KIND_MUSIC;
}

// .text:0x2950
void dMenuMusic_c::pushAdd() {
    u16 mask = dMenuMng_c::makePocketMask(isNotMusic);
    if (mask == POCKET_MASK_ALL) {
        openMessage(MSG_NO_MUSIC);
    } else {
        dMenuMng_c::setPocketMask(mask);
        startAddAnm();
    }
}

// .text:0x29AC
void dMenuMusic_c::updateButtons() {
    mCircleAnm[0].start(isEnabled(CURSOR_PLAY) ? CIRCLE_ANM_YELLOW
                                               : CIRCLE_ANM_GRAY);
    mCircleAnm[0].setToEnd();
    mCircleAnm[1].start(isEnabled(CURSOR_STOP) ? CIRCLE_ANM_YELLOW
                                               : CIRCLE_ANM_GRAY);
    mCircleAnm[1].setToEnd();
    mSquareAnm[1].start(!isEnabled(CURSOR_EJECT));
    mSquareAnm[1].setToEnd();
    onFlag(FLAG_CALC);
    setFrmAnm(isPlaying() != FALSE);
    mScrlIconAnm.mFrameCtrl.mFlags =
        isPlaying() ? m2d::FrameCtrl_c::LOOP : m2d::FrameCtrl_c::REVERSE;
    mScrlIconAnm.mFrameCtrl.setRate(isPlaying() ? 1.0f : 0.2f);
}

// .text:0x2ABC
void dMenuMusic_c::stubUpdate() {
}

// .text:0x2AC0
void dMenuMusic_c::showTutorial(int cursor) {
    int button = -1;
    switch (cursor) {
    case CURSOR_ADD:
        button = 0;
        break;
    case CURSOR_EJECT:
        button = 1;
        break;
    case CURSOR_PLAY:
        button = 2;
        break;
    case CURSOR_STOP:
        button = 3;
        break;
    }
    if (button != -1) {
        u16 msg = button + BALLOON_MSG_ADD;
        if (dObjTutorial_c::request(dObjTutorial_c::REQUEST_MENU, msg, TRUE)) {
            char name[48];
            sprintf(name, "btnIcon_%02d", button);
            mVec3_c pos;
            m2d::getScreenPos(&pos, mLyt.findPane(name));
            mVec2_c pos2;
            pos2.x = pos.x;
            pos2.y = pos.y;
            dObjTutorial_c::setMsg(&pos2, msg, 1);
        }
    }
}

// .text:0x2BBC
void dMenuMusic_c::playSeAdd() {
    playSe(SE_MusicAdd);
}

// .text:0x2BC4
void dMenuMusic_c::playSeEject() {
    playSe(SE_MusicDel);
}

// .text:0x2BCC
void dMenuMusic_c::playSeStop() {
    playSe(SE_MusicStop);
}

// .text:0x2BD4
void dMenuMusic_c::playSePlay() {
    playSe(SE_MusicStart);
}
