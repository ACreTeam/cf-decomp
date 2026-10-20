#include <game/game/d_menu_insc.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_catalog.hpp>
#include <game/game/d_fi_mdl_mgr.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/revolution/SC/scapi.h>
#include <stdio.h>

namespace dMenuInsc {
// .text:0x5C
BASE_PROFILE(MENU_INSC, dMenuInsc_c)
} // namespace dMenuInsc

static const char *l_scaleCrsr = "N_scaleCrsr";
static const char *l_icnHole00 = "N_icnHole_00";
static const char *l_icnHole01 = "N_icnHole_01";
static const char *l_icnHole07 = "N_icnHole_07";
static const char *l_msgPane = "T_msg_00";
static const char *l_blnMsgPane = "T_msg_00";
static const char *l_topTbFrm = "topTbFrm_Fc";

// .text:0x368
int dMenuInsc_c::create() {
    static const char *blnNames[2] = {
        "insectsFish_bln_00_blnIn.brlan",
        "insectsFish_bln_00_blnOut.brlan",
    };
    static const char *circleNames[2] = {
        "insectsFish_win_00_circleIn.brlan",
        "insectsFish_win_00_circleOut.brlan",
    };
    nw4r::lyt::TextBox *box;
    mModelSel = -1;
    BOOL resLoaded = mRes.load("Layout/insectsFish.arc");
    BOOL iconLoaded = mIconRes.load("Layout/icon.arc");
    if (!(resLoaded & iconLoaded)) {
        return NOT_READY;
    }
    initIcons();
    mLyt.build(&mRes, "insectsFish_win_00.brlyt\0\0\0\0");
    mSldLyt.build(&mRes, "insectsFish_win_00.brlyt\0\0\0\0");
    for (int i = 0; i < HOLE_NUM; i++) {
        char name[40];
        sprintf(name, "N_icnHole_%02d", i);
        mpHole[i] = mLyt.findPane(name);
    }
    setLytMode(&mLyt, LYT_MAIN);
    setLytMode(&mSldLyt, LYT_SLIDER);
    mTabLyt.build(&mRes, "insectsFish_tab_00.brlyt");
    mBlnLyt.build(&mRes, "insectsFish_bln_00.brlyt");
    mBlnLyt.mPriority = LYT_PRIORITY;
    mBlnAnm.init(&mBlnLyt, &mRes, blnNames, 2);
    mBlnAnm.start(BLN_ANM_OUT);
    mBlnAnm.setToEnd();
    mNameWord.setTextBox(mLyt.findTextBox("T_name_00"));
    box = mLyt.findTextBox(l_msgPane);
    m2d::stringWordLL_c empty;
    box->AllocStringBuffer(empty.getCapacity());
    box->SetString(empty.getText(), 0);
    box = mBlnLyt.findTextBox(l_blnMsgPane);
    box->AllocStringBuffer(empty.getCapacity());
    box->SetString(empty.getText(), 0);
    mSldNoPshAnm[0].create("insectsFish_win_00_sldCrsr_00_NoPsh.brlan", &mRes);
    mSldNoPshAnm[0].mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mSldNoPshAnm[1].create("insectsFish_win_00_sldCrsr_01_NoPsh.brlan", &mRes);
    mSldNoPshAnm[1].mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mLyt.setAnm(&mScrlIconAnm, "insectsFish_win_00_scrlIcon_00.brlan", nullptr);
    mInfoAnm.init(&mLyt, &mRes, circleNames, 2);
    mInfoAnm.start(INFO_ANM_OUT);
    mInfoAnm.setToEnd();
    initKind();
    setTabOnAnm(isFlag(FLAG_FISH));
    mTabOnAnm.setToEnd();
    initTab();
    initAnms();
    resetList();
    startAnm(&mLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    setTabWinAnm(TAB_WIN_ANM_IN);
    mLyt.calc();
    mSldLyt.calc();
    mTabLyt.calc();
    mBlnLyt.calc();
    onFlag(FLAG_CALC);
    onFlag(FLAG_DRAW);
    mZClear.set(180.0f, -80.0f, -230.0f, 0.0f, -499.9f);
    return SUCCEEDED;
}

// .text:0x6FC
int dMenuInsc_c::doDelete() {
    releaseModel();
    if (!mRes.remove()) {
        return NOT_READY;
    }
    bool removed = mIconRes.remove();
    return removed;
}

// .text:0x754
int dMenuInsc_c::execute() {
    hideTutorial();
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    int cursor = mCursor;
    if (cursor != CURSOR_NONE && cursor != prev) {
        if (!isUitag(cursor)) {
            if (isTab(cursor)) {
                playSeSubMenuPopUp();
            } else {
                playSeCursor();
            }
        }
    }
    mScrlIconAnm.play();
    mInfoAnm.play();
    mLyt.calc();
    return result;
}

// .text:0x828
int dMenuInsc_c::stateMain() {
    offFlag(FLAG_SWITCH);
    if (checkCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    return SUCCEEDED;
}

// .text:0x87C
int dMenuInsc_c::stateAnm() {
    if (calcAnm()) {
        changeNextState();
        if (isState(STATE_MAIN)) {
            onFlag(FLAG_MODEL_REQ);
        }
    }
    if (!isFlag(FLAG_SWITCH)) {
        mTabWinAnm.play();
    } else {
        mTabOnAnm.play();
        followTab();
    }
    mLyt.copyPos(&mSldLyt, "N_all", "N_all");
    mSldLyt.calc();
    calcTabHoverAnm();
    mTabLyt.calc();
    calcIcons();
    return SUCCEEDED;
}

// .text:0x95C
int dMenuInsc_c::stateClose() {
    releaseModel();
    if (isFlag(FLAG_SWITCH)) {
        setSubState(SUB_MAIN);
        startAnm(&mLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
        if (isFlag(FLAG_FISH)) {
            offFlag(FLAG_FISH);
        } else {
            onFlag(FLAG_FISH);
        }
        resetList();
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0xA20
void dMenuInsc_c::postExecute(MAIN_STATE_e state) {
    if (mBlnAnm.play()) {
        onFlag(FLAG_BLN_CALC);
    } else if (mBlnAnm.mCurIdx == BLN_ANM_OUT) {
        offFlag(FLAG_BLN);
    }
    calcTabHoverAnm();
    calcAnms();
    calcScroll();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mSldLyt.calc();
    }
    if (isFlag(FLAG_CALC_TAB)) {
        offFlag(FLAG_CALC_TAB);
        mTabLyt.calc();
    }
    if (isFlag(FLAG_BLN_CALC)) {
        offFlag(FLAG_BLN_CALC);
        mBlnLyt.calc();
    }
    if (isFlag(FLAG_ICON_UPDATE)) {
        offFlag(FLAG_ICON_UPDATE);
        onFlag(FLAG_ICON_CALC);
        updateIcons();
    }
    if (isFlag(FLAG_ICON_CALC)) {
        offFlag(FLAG_ICON_CALC);
        calcIcons();
    }
    offFlag(FLAG_MODEL);
    if (isFlag(FLAG_MODEL_REQ) && requestModel()) {
        calcModel();
        onFlag(FLAG_MODEL);
    }
    dMenuBase_c::postExecute(state);
}

// .text:0xBEC
int dMenuInsc_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mTabLyt.entry();
        drawIcons();
        mSldLyt.entry();
    }
    if (isFlag(FLAG_MODEL)) {
        addOrder();
        mZClear.entry();
    }
    if (isFlag(FLAG_BLN)) {
        mBlnLyt.entry();
    }
    return SUCCEEDED;
}

// .text:0xC88
void dMenuInsc_c::subStateMain() {
    setSelect(mCursor, FALSE);
    int tab = dAcUitag::getTabInput(dAcUitag_c::TAB_INSC, FALSE);
    if (tab != -1 && selectUitag(tab, TRUE)) {
        return;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    mCursor = hitUitag(&pos);
    if (mCursor != CURSOR_NONE) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            selectUitag(mCursor, FALSE);
            return;
        }
    }
    if (mCursor == CURSOR_NONE) {
        mCursor = hitTab(&pos);
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, TRUE);
            int tab = mCursor - CURSOR_TAB;
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                if (tab != isFlag(FLAG_FISH)) {
                    mBlnAnm.setNext(BLN_ANM_OUT);
                    pushTab(tab);
                    return;
                }
            } else {
                showTutorial(mCursor);
            }
        }
    }
    if (mCursor == CURSOR_NONE) {
        mCursor = hitList(&pos);
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                pushCursor();
                return;
            }
            showTutorial(mCursor);
        }
    }
    if (mCursor == CURSOR_NONE) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A) &&
            isFlag(FLAG_BLN) && mBlnAnm.mCurIdx != BLN_ANM_OUT) {
            mBlnAnm.setNext(BLN_ANM_OUT);
            playSe(SE_text_window_close);
        }
    }
    checkPadMove();
}

// .text:0xEC0
void dMenuInsc_c::subStatePush() {
    if (mTabHoverAnm.isEnd()) {
        onFlag(FLAG_SWITCH);
        setTabOnAnm(!isFlag(FLAG_FISH));
        startAnm(&mLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, FALSE);
        releaseModel();
        offFlag(FLAG_MODEL_REQ);
    }
}

// .text:0xF54
void dMenuInsc_c::subStateScroll() {
    for (int i = 0; i < SCRL_NUM; i++) {
        if (!mSldAnm[i].isEnd()) {
            return;
        }
    }
    if (isFlag(FLAG_SCROLL)) {
        return;
    }
    if (mScrollDir != SCROLL_UP) {
        setSelect(CURSOR_SCRL_DOWN, FALSE);
    } else {
        mScroll--;
        setSelect(CURSOR_SCRL_UP, FALSE);
    }
    updateScrollBtns();
    if (mPad::g_currentCore->down(WPAD_BUTTON_A) ||
        (mScrollDir == SCROLL_UP && dPad::isLeftHeld()) ||
        (mScrollDir == SCROLL_DOWN && dPad::isRightHeld())) {
        if (startScroll(mScrollDir)) {
            return;
        }
    }
    resetScrollAnm();
    onFlag(FLAG_ICON_UPDATE);
    setSubState(SUB_MAIN);
}

// .text:0x10A0
void dMenuInsc_c::subStateSlider() {
    int prev = (int)(0.1f * (mSliderPos - -(f32)SLIDER_RANGE));
    mVec2_c cursor = dObjCursor_c::getPos();
    if (SCGetAspectRatio() == 1) {
        mSliderPos = 1.368421f * (cursor.x + mDragOffset);
    } else {
        mSliderPos = cursor.x + mDragOffset;
    }
    if (mSliderPos < -(f32)SLIDER_RANGE) {
        mSliderPos = -(f32)SLIDER_RANGE;
    } else if (mSliderPos > (f32)SLIDER_RANGE) {
        mSliderPos = (f32)SLIDER_RANGE;
    }
    onFlag(FLAG_SLIDER);
    updateScrollFromSlider();
    if ((int)(0.1f * (mSliderPos - -(f32)SLIDER_RANGE)) != prev) {
        playSeSlideClick();
    }
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        offFlag(FLAG_SLIDER);
        resetScrollAnm();
        setScroll(mScroll);
        setSubState(SUB_MAIN);
    }
}

// .text:0x1208
BOOL dMenuInsc_c::closeNow() {
    dAcUitag::closeTab();
    startCloseAnm();
    return TRUE;
}

// .text:0x1240
BOOL dMenuInsc_c::checkCloseRequest() {
    typedef BOOL (dMenuInsc_c::*Proc)();
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    static Proc closeProcs[SUB_NUM] = {
        &dMenuInsc_c::closeNow,
        nullptr,
        &dMenuInsc_c::closeNow,
        &dMenuInsc_c::closeNow,
    };
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL result = (this->*closeProcs[mSubState])();
    if (result) {
        dMenuMng_c::clearCloseRequest();
    }
    return result;
}

// .text:0x1314
void dMenuInsc_c::executeSubState() {
    typedef void (dMenuInsc_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuInsc_c::subStateMain,
        &dMenuInsc_c::subStatePush,
        &dMenuInsc_c::subStateScroll,
        &dMenuInsc_c::subStateSlider,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x134C
void dMenuInsc_c::initTab() {
    static const char *tabHoverNames[3] = {
        "insectsFish_tab_00_topTb_DpdIn.brlan",
        "insectsFish_tab_00_topTb_DpdOut.brlan",
        "insectsFish_tab_00_topTbPsh.brlan",
    };
    mTabHoverAnm.init(&mTabLyt, nullptr, tabHoverNames, 3);
    mTabHoverAnm.setNext(TAB_HOVER_IN);
    mTabLyt.setVisible(l_topTbFrm, false);
    mTabHover = -1;
}

// .text:0x13B8
void dMenuInsc_c::calcTabHoverAnm() {
    if (mTabHoverAnm.play()) {
        onFlag(FLAG_CALC_TAB);
    }
}

// .text:0x13FC
int dMenuInsc_c::hitTab(const mVec2_c *pos) {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c p = *pos;
    return m2d::hitButton(&p, &mTabLyt, CURSOR_TAB, CURSOR_TAB_LAST, -1, -1,
                          1.0f);
}

// .text:0x1478
void dMenuInsc_c::pushTab(int tab) {
    mTabHoverAnm.start(TAB_HOVER_PUSH);
    char name[48];
    sprintf(name, "N_topTb_%02d", tab);
    mTabLyt.setPosFrom(name, "topTbBase_Psh", 0.0f, 9.0f);
    setSubState(SUB_PUSH);
    playSeSubMenuPush();
}

// .text:0x1510
void dMenuInsc_c::followTab() {
    if (mTabHover != -1) {
        char name[48];
        sprintf(name, "N_topTb_%02d\0", mTabHover);
        mTabLyt.setPosFrom(name, l_topTbFrm, 0.0f, 9.0f);
        onFlag(FLAG_CALC_TAB);
    }
}

// .text:0x1588
int dMenuInsc_c::hitUitag(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = -1;
    }
    dAcUitag::setTabFocus(tab);
    if (tab == -1) {
        return CURSOR_NONE;
    }
    return tab;
}

// .text:0x15EC
BOOL dMenuInsc_c::selectUitag(int tab, BOOL delay) {
    if (dAcUitag::selectTab(tab, delay)) {
        startCloseAnm();
        return TRUE;
    }
    return FALSE;
}

// .text:0x163C
void dMenuInsc_c::startCloseAnm() {
    startAnm(&mLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, FALSE);
    setTabWinAnm(TAB_WIN_ANM_OUT);
    releaseModel();
    mBlnAnm.setNext(BLN_ANM_OUT);
    offFlag(FLAG_MODEL_REQ);
}

// .text:0x16AC
void dMenuInsc_c::pushCursor() {
    if (mCursor == CURSOR_INFO) {
        showBalloon();
        return;
    }
    if (isRow(mCursor)) {
        mBlnAnm.setNext(BLN_ANM_OUT);
    }
    switch (mCursor) {
    case CURSOR_SLIDER:
        startSliderDrag();
        break;
    case CURSOR_SCRL_UP:
    case CURSOR_SCRL_DOWN:
        if (startScroll(mCursor - CURSOR_SCRL)) {
            return;
        }
        break;
    default:
        selectItem(mCursor + mScroll - CURSOR_ROW);
        playSeDecide();
        showTutorial(mCursor);
        break;
    }
}

// .text:0x1778
void dMenuInsc_c::checkPadMove() {
    if (dPad::isLeftTriggered() && startScroll(SCROLL_UP)) {
        return;
    }
    if (dPad::isRightTriggered() && startScroll(SCROLL_DOWN)) {
        return;
    }
    if (dPad::isUpTriggered() && canScrollUp()) {
        int scroll = mScroll - ROW_NUM;
        mScroll = scroll;
        if (scroll < 0) {
            mScroll = 0;
        }
        setScroll(mScroll);
        if (isRow(mCursor)) {
            hideTutorial();
        }
        playSeDecide();
    } else if (dPad::isDownTriggered() && canScrollDown()) {
        int scroll = mScroll + ROW_NUM;
        int max = mCount - ROW_NUM;
        mScroll = scroll;
        if (scroll > max) {
            mScroll = max;
        }
        setScroll(mScroll);
        if (isRow(mCursor)) {
            hideTutorial();
        }
        playSeDecide();
    }
}

// .text:0x18CC
BOOL dMenuInsc_c::isRegistered(int idx) {
    if (idx < 0 || idx >= mCount) {
        return FALSE;
    }
    int kind = isFlag(FLAG_FISH) ? dItem::KIND_FISH : dItem::KIND_INSECT;
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mCatalog.isNthRegistered(kind,
                                                                           idx);
}

// .text:0x1948
BOOL dMenuInsc_c::isRowRegistered(int cursor) {
    if (isRow(cursor)) {
        return isRegistered(cursor + mScroll - CURSOR_ROW);
    }
    return FALSE;
}

// .text:0x1980
int dMenuInsc_c::hitList(const mVec2_c *pos) {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c p = *pos;
    int cursor = m2d::hitButton(&p, &mLyt, CURSOR_ROW, CURSOR_SLIDER,
                                CURSOR_INFO, CURSOR_INFO, 1.0f);
    if (cursor != CURSOR_NONE) {
        if (!isRow(cursor) || isRowRegistered(cursor)) {
            return cursor;
        }
    }
    mVec2_c p2 = *pos;
    cursor = m2d::hitButton(&p2, &mSldLyt, CURSOR_ROW, CURSOR_SLIDER,
                            CURSOR_INFO, CURSOR_INFO, 1.0f);
    if (cursor == CURSOR_SCRL_UP && !canScrollUp()) {
        cursor = CURSOR_NONE;
    }
    if (cursor == CURSOR_SCRL_DOWN && !canScrollDown()) {
        cursor = CURSOR_NONE;
    }
    return cursor == CURSOR_NONE ? CURSOR_NONE : cursor;
}

// .text:0x1ACC
void dMenuInsc_c::initAnms() {
    static const char *sldNames0[3] = {
        "insectsFish_win_00_sldCrsr_00_In.brlan",
        "insectsFish_win_00_sldCrsr_00_Out.brlan",
        "insectsFish_win_00_sldCrsr_00_Psh.brlan",
    };
    static const char *sldNames1[3] = {
        "insectsFish_win_00_sldCrsr_01_In.brlan",
        "insectsFish_win_00_sldCrsr_01_Out.brlan",
        "insectsFish_win_00_sldCrsr_01_Psh.brlan",
    };
    static const char *scaleNames[3] = {
        "insectsFish_win_00_scaleCrsrIn.brlan",
        "insectsFish_win_00_scaleCrsrOut.brlan",
        nullptr,
    };
    static const char *holeNames[2] = {
        "insectsFish_win_00_icnHoleIn.brlan",
        "insectsFish_win_00_icnHoleOut.brlan",
    };
    mSldAnm[0].init(&mSldLyt, nullptr, sldNames0, 3);
    mSldAnm[1].init(&mSldLyt, nullptr, sldNames1, 3);
    mScaleAnm.init(&mLyt, nullptr, scaleNames, 3);
    for (int i = 0; i < ROW_NUM; i++) {
        mHoleAnm[i].init(&mLyt, &mRes, holeNames, 2);
        mHoleAnm[i].setNumber(i + HOLE_FIRST_ROW);
    }
}

// .text:0x1B98
void dMenuInsc_c::calcAnms() {
    BOOL playing = FALSE;
    int i;
    for (i = 0; i < SCRL_NUM; i++) {
        playing |= mSldAnm[i].play();
    }
    playing |= mScaleAnm.play();
    for (i = 0; i < ROW_NUM; i++) {
        playing |= mHoleAnm[i].play();
    }
    if (playing) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x1C44
void dMenuInsc_c::calcScroll() {
    if (isFlag(FLAG_SCROLL)) {
        mScrollAnm.play();
        int scroll = mScroll;
        if (mScrollAnm.mFrameCtrl.isStop()) {
            offFlag(FLAG_SCROLL);
        }
        setScroll(scroll);
        onFlag(FLAG_CALC);
        onFlag(FLAG_ICON_CALC);
    }
    if (isFlag(FLAG_SCROLL_CALC)) {
        offFlag(FLAG_SCROLL_CALC);
        onFlag(FLAG_CALC);
        u8 shown0 = mScroll > 1;
        mLyt.setVisible(l_icnHole00, shown0 != 0);
        BOOL hide1 = mScroll <= 0;
        mLyt.setVisible(l_icnHole01, hide1 == FALSE);
        BOOL hide7 = mScroll >= mCount - ROW_NUM;
        mLyt.setVisible(l_icnHole07, hide7 == FALSE);
    }
}

// .text:0x1D90
BOOL dMenuInsc_c::startScroll(int dir) {
    if (dir != SCROLL_UP) {
        if (!canScrollDown()) {
            return FALSE;
        }
        if (mScroll >= mCount - ROW_NUM) {
            return FALSE;
        }
    } else if (!canScrollUp()) {
        return FALSE;
    }
    if (isRow(mCursor)) {
        setSelect(mCursor, FALSE);
    }
    mSldAnm[dir].setNext(SLD_ANM_PUSH);
    setScrollAnm(dir);
    if (dir != SCROLL_UP) {
        mScroll++;
    }
    setSubState(SUB_SCROLL);
    playSePageTurn();
    return TRUE;
}

// .text:0x1E94
void dMenuInsc_c::startSliderDrag() {
    setSubState(SUB_SLIDER);
    mVec3_c pos = mLyt.findPane(l_scaleCrsr)->GetTranslate();
    mVec2_c cursor = dObjCursor_c::getPos();
    if (SCGetAspectRatio() == 1) {
        mDragOffset = 0.730769f * pos.y - cursor.x;
    } else {
        mDragOffset = pos.y - cursor.x;
    }
    playSeAPush();
}

// .text:0x1F44
void dMenuInsc_c::resetScrollAnm() {
    setScrollAnm(SCROLL_UP);
    offFlag(FLAG_SCROLL);
}

// .text:0x1F80
void dMenuInsc_c::setScrollAnm(int dir) {
    static const char *names[2] = {
        "insectsFish_win_00_iconR.brlan",
        "insectsFish_win_00_iconL.brlan",
    };
    onFlag(FLAG_SCROLL);
    mLyt.setAnm(&mScrollAnm, names[dir], nullptr);
    mScrollDir = dir;
    onFlag(FLAG_SCROLL_CALC);
    onFlag(FLAG_ICON_CALC);
}

// .text:0x1FF8
void dMenuInsc_c::updateSliderPane() {
    nw4r::lyt::Pane *pane = mLyt.findPane(l_scaleCrsr);
    mVec3_c pos = pane->GetTranslate();
    pos.y = mSliderPos;
    pane->SetTranslate(pos);
    onFlag(FLAG_CALC);
}

// .text:0x2068
void dMenuInsc_c::updateScrollFromSlider() {
    f32 step = (f32)SLIDER_LENGTH / (f32)(mCount - ROW_NUM);
    mScroll = (int)((mSliderPos - -(f32)SLIDER_RANGE) / step);
    onFlag(FLAG_ICON_UPDATE);
    mScroll++;
    f32 frame =
        10.0f - (f32)(int)(10.0f * ((mSliderPos - (-(f32)SLIDER_RANGE +
                                                   (f32)(mScroll - 1) * step)) /
                                    step));
    if (frame < 0.0f) {
        frame = 0.0f;
    } else if (frame >= 10.0f) {
        mScroll--;
        frame = 0.0f;
    }
    mScrollAnm.setFrame(frame);
    onFlag(FLAG_ICON_CALC);
    updateSliderPane();
    onFlag(FLAG_SCROLL_CALC);
    updateScrollBtns();
}

// .text:0x21D0
void dMenuInsc_c::setScroll(int scroll) {
    if (scroll < 0) {
        scroll = 0;
    } else if (scroll > mCount - ROW_NUM) {
        scroll = mCount - ROW_NUM;
    }
    mScroll = scroll;
    updateScrollBtns();
    if (mScrollDir == SCROLL_DOWN) {
        scroll--;
    }
    mSliderPos =
        (f32)(scroll * SLIDER_LENGTH / (mCount - ROW_NUM) - SLIDER_RANGE);
    f32 frame = mScrollAnm.mFrameCtrl.getFrame();
    f32 delta = 0.1f * (f32)(SLIDER_LENGTH / (mCount - ROW_NUM)) * frame;
    if (mScrollDir == SCROLL_UP) {
        delta = -delta;
    }
    mSliderPos += delta;
    onFlag(FLAG_SCROLL_CALC);
    updateSliderPane();
    onFlag(FLAG_ICON_UPDATE);
}

// .text:0x2300
void dMenuInsc_c::selectItem(int idx) {
    if (idx != -1 && mSel == idx) {
        return;
    }
    mSel = idx;
    if (isFlag(FLAG_FISH)) {
        mSelFish = idx;
    } else {
        mSelInsect = idx;
    }
    m2d::stringWordLL_c text;
    if (idx == -1) {
        mNameWord.clear();
        text.clear();
    } else {
        const char *group;
        int base;
        if (isFlag(FLAG_FISH)) {
            group = "sys_2D/SYS_2D_fish_text";
            base = ITEM_INDEX_FISH;
        } else {
            base = ITEM_INDEX_INSECT;
            group = "sys_2D/SYS_2D_bug_text";
        }
        int index = base + mSel;
        dItem::Item item;
        item.setFromIndex(index);
        dItem::name_c name(item);
        mNameWord.clear();
        mNameWord.set(name.getText(), 0);
        mNameWord.capitalize();
        dString::loadString(&text, (u16)(mSel + 1), group);
    }
    nw4r::lyt::TextBox *box = mLyt.findTextBox(l_msgPane);
    box->SetString(text.getText(), 0);
    mNameWord.update();
    onFlag(FLAG_CALC);
    if (mModelSel != -1) {
        releaseModel();
    }
    mModelSel = mSel;
    requestModel();
}

// .text:0x24D4
void dMenuInsc_c::reselectItem() {
    mSel = -2;
    if (isFlag(FLAG_FISH)) {
        selectItem(mSelFish);
    } else {
        selectItem(mSelInsect);
    }
}

// .text:0x2530
void dMenuInsc_c::initKind() {
    offFlag(FLAG_FISH);
    mSelInsect = -1;
    for (int i = 0; i < ITEM_NUM; i++) {
        if (isRegistered(i)) {
            mSelInsect = i;
            i = ITEM_NUM;
        }
    }
    onFlag(FLAG_FISH);
    mSelFish = -1;
    for (int i = 0; i < ITEM_NUM; i++) {
        if (isRegistered(i)) {
            mSelFish = i;
            i = ITEM_NUM;
        }
    }
    if (mSelInsect != -1) {
        offFlag(FLAG_FISH);
    } else if (mSelFish != -1) {
        onFlag(FLAG_FISH);
    } else {
        offFlag(FLAG_FISH);
    }
}

// .text:0x2628
void dMenuInsc_c::resetList() {
    updateBar();
    setKindAnms();
    resetScrollAnm();
    setScroll(0);
    reselectItem();
}

// .text:0x2678
void dMenuInsc_c::updateBar() {
    nw4r::lyt::TexMap *map = m2d::getTexMaps(mLyt.findPane("scale_00"));
    mTex::edit4b_c tex(0, 0, nullptr);
    tex.init(map->GetWidth(), map->GetHeight(), map->GetImage());
    isFlag(FLAG_FISH);
    mCount = ITEM_NUM;
    u32 i;
    int start;
    int end = 0;
    int color;
    int x;
    int y;
    for (i = 0; i < mCount; i++) {
        start = end;
        end = (i + 1) * map->GetWidth() / mCount;
        if (isRegistered(i)) {
            color = 0;
        } else {
            color = 15;
        }
        for (y = 0; y < map->GetHeight(); y++) {
            for (x = start; x < end; x++) {
                tex.set(x, y, color, FALSE);
            }
        }
    }
    tex.flush();
}

// .text:0x27C8
void dMenuInsc_c::setKindAnms() {
    static const char *names[2] = {
        "insectsFish_win_00_toFish.brlan",
        "insectsFish_win_00_toInsects.brlan",
    };
    int kind = isFlag(FLAG_FISH);
    mLyt.setAnm(&mKindAnm, names[kind], nullptr);
    mSldLyt.setAnm(&mKindSldAnm, names[kind], nullptr);
}

// .text:0x283C
void dMenuInsc_c::setTabOnAnm(int kind) {
    static const char *names[2] = {
        "insectsFish_tab_00_topTb_00_On.brlan",
        "insectsFish_tab_00_topTb_01_On.brlan",
    };
    mTabLyt.setAnm(&mTabOnAnm, names[kind], nullptr);
}

// .text:0x285C
void dMenuInsc_c::setTabWinAnm(int anm) {
    static const char *names[2] = {
        "insectsFish_tab_00_topTbIn.brlan",
        "insectsFish_tab_00_topTbOut.brlan",
    };
    mTabLyt.setAnm(&mTabWinAnm, names[anm], nullptr);
}

// .text:0x287C
void dMenuInsc_c::setSelect(int cursor, BOOL select) {
    m2d::AnmEx_c *anm = nullptr;
    switch (cursor) {
    case CURSOR_TAB:
    case CURSOR_TAB + 1: {
        int tab = cursor - CURSOR_TAB;
        if (select) {
            if (tab != mTabHover) {
                mTabHover = tab;
                mTabLyt.setVisible(l_topTbFrm, true);
                followTab();
            }
        } else if (tab == mTabHover) {
            mTabHover = -1;
            mTabLyt.setVisible(l_topTbFrm, false);
        }
        break;
    }
    case CURSOR_SCRL_UP:
    case CURSOR_SCRL_DOWN:
        anm = &mSldAnm[cursor - CURSOR_SCRL];
        break;
    case CURSOR_SLIDER:
        anm = &mScaleAnm;
        break;
    case CURSOR_ROW:
    case CURSOR_ROW + 1:
    case CURSOR_ROW + 2:
    case CURSOR_ROW + 3:
    case CURSOR_ROW_LAST:
        mHoleAnm[cursor - CURSOR_ROW].setNext(select == FALSE);
        break;
    case CURSOR_INFO:
        if (mSel >= 0) {
            mInfoAnm.setNext(select == FALSE);
        }
        return;
    }
    if (anm) {
        if (select) {
            anm->setNext(0);
        } else {
            anm->setNext(1);
        }
    }
}

// .text:0x29E4
void dMenuInsc_c::updateScrollBtns() {
    if (mScroll == 0) {
        mSldNoPshAnm[0].setToEnd();
    } else {
        mSldNoPshAnm[0].setFrame(0.0f);
    }
    if (mScroll == mCount - ROW_NUM) {
        mSldNoPshAnm[1].setToEnd();
    } else {
        mSldNoPshAnm[1].setFrame(0.0f);
    }
    mSldLyt.bind(&mSldNoPshAnm[0]);
    mSldLyt.bind(&mSldNoPshAnm[1]);
    mSldLyt.calc();
    mSldLyt.unbind(&mSldNoPshAnm[0]);
    mSldLyt.unbind(&mSldNoPshAnm[1]);
}

// .text:0x2AA8
void dMenuInsc_c::initIcons() {
    mItemAnm.create("item_icon_00_item.brlan", &mIconRes);
    for (int i = 0; i < HOLE_NUM; i++) {
        mIcon[i].create(&mIconRes, &mItemAnm);
    }
}

// .text:0x2B1C
void dMenuInsc_c::calcIcons() {
    for (int i = 0; i < HOLE_NUM; i++) {
        mVec3_c pos;
        m2d::getScreenPos(&pos, mpHole[i]);
        mIcon[i].setRootPos(&pos);
        bool visible = false;
        if (mpHole[i]->IsVisible() && mpHole[i]->GetAlpha() != 0) {
            visible = true;
        }
        mIcon[i].setVisible(visible);
        mIcon[i].calcLayouts();
    }
}

// .text:0x2BBC
void dMenuInsc_c::drawIcons() {
    for (int i = 0; i < HOLE_NUM; i++) {
        mIcon[i].drawLayouts();
    }
}

// .text:0x2C08
void dMenuInsc_c::setLytMode(m2d::SimpleEx_c *lyt, bool main) {
    lyt->setVisible("N_m_s_win_all", main);
    lyt->setVisible("N_name_all", main);
    lyt->setVisible("N_scale_all", main);
    lyt->setVisible("sldLine_00", main);
    lyt->setVisible("N_icnHole_all", main);
    bool slider = main == FALSE;
    lyt->setVisible("sldCrsrBase_00", slider);
    lyt->setVisible("sldCrsrBase_01", slider);
    lyt->setVisible("N_sldCrsr_00", slider);
    lyt->setVisible("N_sldCrsr_01", slider);
}

// .text:0x2CE0
void dMenuInsc_c::updateIcons() {
    int idx = mScroll - HOLE_FIRST_ROW;
    dItem::Item item;
    for (int i = 0; i < HOLE_NUM; i++, idx++) {
        if (!isRegistered(idx)) {
            item.mId = dItem::ITEM_ID_NONE;
        } else {
            int index =
                isFlag(FLAG_FISH) ? ITEM_INDEX_FISH : ITEM_INDEX_INSECT;
            index += idx;
            item.setFromIndex(index);
        }
        mIcon[i].setItem(item, 0, -1);
    }
}

// .text:0x2DA4
bool dMenuInsc_c::canScrollUp() {
    return mScroll > 0;
}

// .text:0x2DB8
bool dMenuInsc_c::canScrollDown() {
    return mScroll < mCount - ROW_NUM;
}

// .text:0x2DDC
BOOL dMenuInsc_c::requestModel() {
    if (mModelSel == -1) {
        return FALSE;
    }
    int myNo = dNet::getMyNo();
    return dFIMdlMgr_c::sInstance->request(
        myNo,
        isFlag(FLAG_FISH) ? (int)dFIMdlBank_c::TYPE_FISH
                          : (int)dFIMdlBank_c::TYPE_INSECT_MENU,
        mModelSel, TRUE, dFIMdlBank_c::MODE_BOOK);
}

// .text:0x2E60
void dMenuInsc_c::releaseModel() {
    int myNo = dNet::getMyNo();
    if (mModelSel >= 0) {
        m2d::initCallbackList();
        dFIMdlMgr_c::sInstance->release(myNo);
        mModelSel = -1;
    }
}

// .text:0x2EC0
void dMenuInsc_c::calcModel() {
    dFIMdlMgr_c::sInstance->getObj(dNet::getMyNo())->calcBook();
}

// .text:0x2F20
void dMenuInsc_c::addOrder() {
    dNet::getMyNo();
    m2d::addCallback(&mOrder);
}

// .text:0x2F54
void dMenuInsc_c::showBalloon() {
    if (mSel < 0) {
        return;
    }
    if (!isFlag(FLAG_BLN)) {
        nw4r::lyt::TextBox *box = mBlnLyt.findTextBox(l_blnMsgPane);
        m2d::stringWordLL_c text;
        const char *group;
        if (isFlag(FLAG_FISH)) {
            group = "sys_2D/SYS_2D_fish_time";
        } else {
            group = "sys_2D/SYS_2D_bug_time";
        }
        text.clear();
        dString::loadString(&text, (u16)(mSel + 1), group);
        box->SetString(text.getText(), 0);
        onFlag(FLAG_BLN_CALC);
        onFlag(FLAG_BLN);
        mBlnAnm.setNext(BLN_ANM_IN);
        playSe(SE_text_window_open);
    } else {
        mBlnAnm.setNext(BLN_ANM_OUT);
        playSe(SE_text_window_close);
    }
}

// .text:0x308C
void dMenuInsc_c::showTutorial(int cursor) {
    if (isTab(cursor)) {
        showTabTutorial(cursor - CURSOR_TAB);
        return;
    }
    if (isRow(cursor)) {
        showRowTutorial(cursor - CURSOR_ROW);
    }
}

// .text:0x30E0
void dMenuInsc_c::showTabTutorial(int tab) {
    u16 msg = tab + BALLOON_MSG_BUGS;
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

// .text:0x318C
void dMenuInsc_c::showRowTutorial(int row) {
    if (isRegistered(row + mScroll) &&
        dObjTutorial_c::request(TUTORIAL_ITEM, row, FALSE)) {
        char name[40];
        sprintf(name, "N_icnHole_%02d", row + HOLE_FIRST_ROW);
        mVec3_c pos;
        m2d::getScreenPos(&pos, mLyt.findPane(name));
        mVec2_c pos2;
        pos2.x = pos.x;
        pos2.y = pos.y;
        int idx;
        if (isFlag(FLAG_FISH)) {
            idx = row + mScroll + ITEM_INDEX_FISH;
        } else {
            idx = row + mScroll + ITEM_INDEX_INSECT;
        }
        dItem::Item item;
        item.setFromIndex(idx);
        dItem::name_c name_(item);
        dObjTutorial_c::setWord(&pos2, &name_, 0);
    }
}

// .text:0x32A0
void dMenuInsc_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x32A4
void dMenuInsc_c::order_c::execute() {
    dFIObj_c *obj = dFIMdlMgr_c::sInstance->getObj(dNet::getMyNo());
    if (obj) {
        obj->mMdl.entry();
    }
}
