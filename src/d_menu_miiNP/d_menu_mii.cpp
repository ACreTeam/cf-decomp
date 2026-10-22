#include <game/game/d_menu_mii.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/RFL.h>
#include <stdio.h>

namespace dMenuMii {
// .text:0x5C
BASE_PROFILE(MENU_MII, dMenuMii_c)
} // namespace dMenuMii

static const char *l_allPane = "N_Mii_win_all";
static const char *l_pushPane = "icnHole_Psh";
static const char *l_focusPane = "icnHole_Fc";
static const char *l_beforePane = "icnHole_Before";

// .text:0x2A0
int dMenuMii_c::create() {
    int loaded = mRes.load("Layout/slct_mii.arc");
    loaded &= mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "slct_Mii_win_00.brlyt");
    mLyt.setVisible("B_icnHole_05", false);
    mLyt.setVisible("B_icnHole_11", false);
    initAnm();
    mLyt.calc();
    mBtn.create();
    initButtons();
    initMessage();
    initCaptures();
    initMiiList();
    initScrlBtnAnm();
    setState(STATE_MAIN);
    setSubState(SUB_OPEN);
    return SUCCEEDED;
}

// .text:0x3AC
int dMenuMii_c::doDelete() {
    freeCaptures();
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes.remove() != 0;
}

// .text:0x404
int dMenuMii_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x428
int dMenuMii_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(&mLyt, l_allPane);
    return result;
}

// .text:0x478
int dMenuMii_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x49C
int dMenuMii_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prevCursor) {
        playSeCursor();
    }
    mBtn.execute();
    calcAnm();
    mScrlIconAnm.play();
    mLyt.calc();
    if (isFlag(FLAG_UPDATE)) {
        offFlag(FLAG_UPDATE);
        if (isFlag(FLAG_BEFORE_ALPHA)) {
            offFlag(FLAG_BEFORE_ALPHA);
            updateBeforeAlpha();
        }
    }
    return result;
}

// .text:0x570
int dMenuMii_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x5BC
void dMenuMii_c::subStateOpen() {
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    stateAnm();
    onFlag(FLAG_DRAW);
}

// .text:0x638
void dMenuMii_c::subStateMain() {
    setSelect(mCursor, FALSE);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != CURSOR_NONE) {
        mCursor = btn + CURSOR_BTN_FIRST;
        setSelect(mCursor, TRUE);
        push();
        return;
    }
    mCursor = hitCheck();
    if (mCursor != CURSOR_NONE) {
        setSelect(mCursor, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push();
            return;
        }
    }
    checkPadScroll();
}

// .text:0x700
void dMenuMii_c::subStatePush() {
    if (isPushEnd()) {
        onPushEnd();
    }
}

// .text:0x73C
void dMenuMii_c::subStateScroll() {
    mFlipAnm.play();
    onFlag(FLAG_UPDATE);
    onFlag(FLAG_BEFORE_ALPHA);
    if (mFlipAnm.isEnd()) {
        if (--mScrollCount > 0) {
            scroll(mCursor, FALSE);
            mFlipAnm.play();
        } else {
            initScrlBtnAnm();
            if (mFlipAnm.mCurIdx == FLIP_ANM_LEFT_END) {
                refreshIcons();
            }
            mFlipAnm.start(FLIP_ANM_LEFT_MIDDLE);
            setSubState(SUB_MAIN);
        }
    }
}

// .text:0x800
void dMenuMii_c::executeSubState() {
    typedef void (dMenuMii_c::*Proc)();
    static Proc subStateProcs[] = {
        &dMenuMii_c::subStateOpen, &dMenuMii_c::subStateMain,
        &dMenuMii_c::subStatePush, &dMenuMii_c::subStateScroll};
    (this->*subStateProcs[mSubState])();
}

// .text:0x838
void dMenuMii_c::initButtons() {
    mBtn.setVisible(BTN_CANCEL, false);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_OK, MSG_CONFIRM, FALSE);
}

// .text:0x894
void dMenuMii_c::setOkDisable(int disable) {
    mBtn.setDisable(BTN_OK, disable != 0);
}

// .text:0x8B0
void dMenuMii_c::initAnm() {
    static const char *scrlBtn0AnmNames[] = {
        "slct_Mii_win_00_scrlBtn_00_In.brlan",
        "slct_Mii_win_00_scrlBtn_00_Out.brlan",
        "slct_Mii_win_00_scrlBtn_00_Psh.brlan",
        "slct_Mii_win_00_scrlBtn_00_NoPsh.brlan",
        "slct_Mii_win_00_scrlBtn_00_CanPsh.brlan",
    };
    static const char *scrlBtn1AnmNames[] = {
        "slct_Mii_win_00_scrlBtn_01_In.brlan",
        "slct_Mii_win_00_scrlBtn_01_Out.brlan",
        "slct_Mii_win_00_scrlBtn_01_Psh.brlan",
        "slct_Mii_win_00_scrlBtn_01_NoPsh.brlan",
        "slct_Mii_win_00_scrlBtn_01_CanPsh.brlan",
    };
    static const char *flipAnmNames[] = {
        "slct_Mii_win_00_icnR_start_00.brlan",
        "slct_Mii_win_00_icnR_mdl_00.brlan",
        "slct_Mii_win_00_icnR_end_00.brlan",
        "slct_Mii_win_00_icnL_start_00.brlan",
        "slct_Mii_win_00_icnL_mdl_00.brlan",
        "slct_Mii_win_00_icnL_end_00.brlan",
    };
    mScrlBtnAnm[SCRL_LEFT].init(&mLyt, &mRes, scrlBtn0AnmNames, 5);
    mScrlBtnAnm[SCRL_RIGHT].init(&mLyt, &mRes, scrlBtn1AnmNames, 5);
    mLyt.setAlpha(l_focusPane, 255);
    mLyt.setVisible(l_focusPane, false);
    mFocusIcon = -1;
    mLyt.setAlpha(l_beforePane, 255);
    mLyt.setVisible(l_beforePane, false);
    mBeforeIcon = -1;
    mLyt.setAnm(&mPushAnm, "slct_Mii_win_00_icnHole_Psh.brlan", nullptr);
    mPushAnm.setToEnd();
    mLyt.setAnm(&mScrlIconAnm, "slct_Mii_win_00_scrlIcon_00.brlan", nullptr);
    mFlipAnm.init(&mLyt, &mRes, flipAnmNames, 6);
    mFlipAnm.start(FLIP_ANM_LEFT_MIDDLE);
}

// .text:0x9C0
void dMenuMii_c::calcAnm() {
    int update = FALSE;
    for (int i = 0; i < SCRL_NUM; i++) {
        update |= mScrlBtnAnm[i].play();
    }
    if (!mPushAnm.mFrameCtrl.isStop()) {
        mPushAnm.play();
        update = TRUE;
    }
    if (update) {
        onFlag(FLAG_UPDATE);
    }
}

// .text:0xA58
void dMenuMii_c::setPushPos(int icon) {
    mPushAnm.setFrame(0.0f);
    char name[40];
    sprintf(name, "N_icnHole_%02d", icon);
    mLyt.setPosFrom(name, l_pushPane, 0.0f, 0.0f);
}

// .text:0xAD8
BOOL dMenuMii_c::selectIcon(int icon) {
    setBeforeIcon(icon);
    if (icon >= ICON_COLUMNS) {
        icon--;
    }
    int listIndex = icon + mPage * PAGE_SIZE;
    if (listIndex == mSelected) {
        return FALSE;
    }
    if (mSelected == -1) {
        setOkDisable(FALSE);
    }
    mSelected = icon + mPage * PAGE_SIZE;
    return TRUE;
}

// .text:0xB64
void dMenuMii_c::setBeforeIcon(int icon) {
    if (mBeforeIcon != icon) {
        if (mBeforeIcon == -1) {
            mLyt.setVisible(l_beforePane, true);
        }
        char name[40];
        sprintf(name, "N_icnHole_%02d", icon);
        mLyt.setPosFrom(name, l_beforePane, 0.0f, 0.0f);
        mBeforeIcon = icon;
        onFlag(FLAG_UPDATE);
        onFlag(FLAG_BEFORE_ALPHA);
    }
}

// .text:0xC14
void dMenuMii_c::hideBeforeIcon() {
    mLyt.setVisible(l_beforePane, false);
    mBeforeIcon = -1;
}

// .text:0xC58
void dMenuMii_c::updateBeforeIcon() {
    int icon = getIcon(mSelected);
    if (icon == -1) {
        hideBeforeIcon();
    } else {
        setBeforeIcon(icon);
    }
}

// .text:0xCA8
void dMenuMii_c::updateBeforeAlpha() {
    if (mBeforeIcon != -1) {
        char name[40];
        sprintf(name, "N_icnHole_%02d", mBeforeIcon);
        nw4r::lyt::Pane *pane = mLyt.findPane(name);
        mLyt.setAlpha(l_beforePane, pane->GetAlpha());
    }
}

// .text:0xD14
void dMenuMii_c::initScrlBtnAnm() {
    mScrlBtnAnm[SCRL_LEFT].start(canScrollLeft() ? SCRL_ANM_CAN_PUSH
                                                 : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[SCRL_LEFT].setToEnd();
    mScrlBtnAnm[SCRL_RIGHT].start(canScrollRight() ? SCRL_ANM_CAN_PUSH
                                                   : SCRL_ANM_NO_PUSH);
    mScrlBtnAnm[SCRL_RIGHT].setToEnd();
    onFlag(FLAG_UPDATE);
}

// .text:0xD94
BOOL dMenuMii_c::canScroll(int btn) {
    if (btn == SCRL_LEFT) {
        return canScrollLeft();
    }
    return canScrollRight();
}

// .text:0xDA4
BOOL dMenuMii_c::canScrollLeft() {
    return mPage > 0;
}

// .text:0xDB8
BOOL dMenuMii_c::canScrollRight() {
    return mPageNum - 1 > mPage;
}

// .text:0xDDC
void dMenuMii_c::scroll(int slot, BOOL first) {
    if (slot == CURSOR_SCRL_FIRST + SCRL_LEFT) {
        if (first) {
            mFlipAnm.start(FLIP_ANM_RIGHT_START);
        } else if (mScrollCount <= 1) {
            mFlipAnm.start(FLIP_ANM_RIGHT_END);
        } else {
            mFlipAnm.start(FLIP_ANM_RIGHT_MIDDLE);
        }
        mScroll = mScroll - 1;
        refreshIcons();
        if (first) {
            setPage(mPage - 1);
        }
    } else {
        if (first) {
            mFlipAnm.start(FLIP_ANM_LEFT_START);
        } else if (mScrollCount <= 1) {
            mFlipAnm.start(FLIP_ANM_LEFT_END);
        } else {
            mFlipAnm.start(FLIP_ANM_LEFT_MIDDLE);
        }
        refreshIcons();
        mScroll = mScroll + 1;
        if (first) {
            setPage(mPage + 1);
        }
    }
    if (first) {
        mScrollCount = SCROLL_REPEAT;
    }
}

// .text:0xF04
void dMenuMii_c::setSelect(int slot, BOOL select) {
    if (isBtn(slot)) {
        mBtn.setSelect(slot - CURSOR_BTN_FIRST, select, FALSE);
    }
    if (isScrl(slot)) {
        mScrlBtnAnm[slot - CURSOR_SCRL_FIRST].setNext(!select);
    }
    if (isIcon(slot)) {
        if (select) {
            if (slot != mFocusIcon) {
                char name[40];
                sprintf(name, "N_icnHole_%02d", slot);
                mLyt.setVisible(l_focusPane, true);
                mLyt.setPosFrom(name, l_focusPane, 0.0f, 0.0f);
                mFocusIcon = slot;
                onFlag(FLAG_UPDATE);
            }
        } else if (slot == mFocusIcon) {
            mLyt.setVisible(l_focusPane, false);
            mFocusIcon = -1;
        }
    }
}

// .text:0x106C
int dMenuMii_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c cursor = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&cursor);
    if (btn != CURSOR_NONE) {
        return btn + CURSOR_BTN_FIRST;
    }
    mVec2_c pos = cursor;
    int hit = m2d::hitButton(&pos, &mLyt, 0, CURSOR_SCRL_FIRST + SCRL_RIGHT, -1,
                             -1, 1.0f);
    if (hit == CURSOR_SCRL_FIRST + SCRL_LEFT && !canScrollLeft()) {
        hit = CURSOR_NONE;
    }
    if (hit == CURSOR_SCRL_FIRST + SCRL_RIGHT && !canScrollRight()) {
        hit = CURSOR_NONE;
    }
    if (hit != CURSOR_NONE) {
        if (isIcon(hit) && getListIndex(hit) >= mMiiNum) {
            return CURSOR_NONE;
        }
        return hit;
    }
    return CURSOR_NONE;
}

// .text:0x1198
void dMenuMii_c::checkPadScroll() {
    if (dPad::isLeftTriggered() && canScrollLeft()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        mCursor = CURSOR_SCRL_FIRST + SCRL_LEFT;
        push();
    } else if (dPad::isRightTriggered() && canScrollRight()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        mCursor = CURSOR_SCRL_FIRST + SCRL_RIGHT;
        push();
    }
}

// .text:0x124C
void dMenuMii_c::push() {
    int slot = mCursor;
    if (isBtn(slot)) {
        mBtn.push(slot - CURSOR_BTN_FIRST, FALSE);
    } else if (isIcon(slot)) {
        if (selectIcon(slot)) {
            setPushPos(slot);
            playSeListChoice();
        }
    } else if (isScrl(slot)) {
        int btn = slot - CURSOR_SCRL_FIRST;
        if (canScroll(btn)) {
            mScrlBtnAnm[btn].setNext(SCRL_ANM_PUSH);
        } else {
            return;
        }
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x136C
BOOL dMenuMii_c::isPushEnd() {
    int slot = mCursor;
    if (isBtn(slot)) {
        return !mBtn.isPushing();
    }
    if (isIcon(slot)) {
        return mPushAnm.mFrameCtrl.isStop();
    }
    if (isScrl(slot)) {
        return mScrlBtnAnm[slot - CURSOR_SCRL_FIRST].isEnd();
    }
    return TRUE;
}

// .text:0x1428
void dMenuMii_c::onPushEnd() {
    int slot = mCursor;
    if (isBtn(slot)) {
        switch (slot - CURSOR_BTN_FIRST) {
        case BTN_CANCEL:
            startCloseAnm();
            break;
        case BTN_OK:
            startCloseAnm();
            if (applyMii()) {
                dMenuMng_c::setResult(1);
            }
            break;
        }
    } else if (isIcon(slot)) {
        setSubState(SUB_MAIN);
    } else if (isScrl(slot)) {
        scroll(slot, TRUE);
        setSubState(SUB_SCROLL);
        playSePageTurn();
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x156C
void dMenuMii_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(0);
}

// .text:0x15AC
void dMenuMii_c::initMessage() {
    mLyt.setMessage("T_msg_00", MSG_CHOOSE_MII, nullptr);
}

// .text:0x15C4
void dMenuMii_c::initCaptures() {
    int i;
    mAllocator_c *allocator = m2d::ResAccLoader_c::getAllocator();
    for (i = 0; i < ICON_NUM; i++) {
        char name[40];
        sprintf(name, "MiiIcon_%02d", i);
        mpIconPane[i] = mLyt.findPane(name);
        mpIconBuf[i] = allocator->alloc(CAPTURE_BUF);
        mCapture[i].clear();
    }
    mpOrigImage = m2d::getTexMaps(mpIconPane[0])->GetImage();
}

// .text:0x1678
void dMenuMii_c::freeCaptures() {
    if (mpIconBuf[0] != nullptr) {
        int i;
        mAllocator_c *allocator = m2d::ResAccLoader_c::getAllocator();
        for (i = 0; i < ICON_NUM; i++) {
            allocator->free(mpIconBuf[i]);
            mpIconBuf[i] = nullptr;
        }
    }
    m2d::initCaptureList();
}

// .text:0x1704
void dMenuMii_c::setPage(int page) {
    mPage = page;
    dString::Word_c word;
    dScript::formatNumber(&word, mPage + 1, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(0, &word);
    dScript::formatNumber(&word, mPageNum, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(1, &word);
    mLyt.setMessage("T_nmbr_00", MSG_PAGE, nullptr);
}

// .text:0x17A8
void dMenuMii_c::initMiiList() {
    mMiiNum = RFLGetAvailableOfficialDataNum();
    mPageNum = (mMiiNum + PAGE_SIZE - 1) / PAGE_SIZE;
    setPage(0);
    mSelected = -1;
    u16 current = getCurrentMiiIndex();
    setOkDisable(TRUE);
    int i = 0;
    int count = 0;
    int otherCount = 0;
    int others[MII_MAX];
    for (; i < MII_MAX; i++) {
        if (RFLIsAvailableOfficialData(i)) {
            dPlayerMgr::MiiData_c info;
            if (RFLGetAdditionalInfo((RFLCharInfo *)&info, RFLDataSource_Official,
                               nullptr, i) == RFLErrcode_Success) {
                if ((info.mPersonalInfo >> 17) & 1) {
                    if (current == i) {
                        mSelected = count;
                    }
                    mMiiIndex[count] = i;
                    count++;
                } else {
                    others[otherCount] = i;
                    otherCount++;
                }
            }
        }
    }
    for (i = 0; i < otherCount; i++) {
        if (current == others[i]) {
            mSelected = count;
        }
        mMiiIndex[count] = others[i];
        count++;
    }
    for (; count < MII_MAX; count++) {
        mMiiIndex[count] = MII_NONE;
    }
    updateBeforeIcon();
    refreshIcons();
    if (mSelected != -1) {
        setOkDisable(FALSE);
    }
}

// .text:0x1988
void dMenuMii_c::bindIconBuffers() {
    for (int i = 0; i < ICON_COLUMNS; i++) {
        int column = (i + mScroll) % ICON_COLUMNS;
        nw4r::lyt::TexMap *tex = m2d::getTexMaps(mpIconPane[i]);
        tex->SetImage(mpIconBuf[column]);
        column += ICON_COLUMNS;
        tex = m2d::getTexMaps(mpIconPane[i + ICON_COLUMNS]);
        tex->SetImage(mpIconBuf[column]);
    }
}

// .text:0x1A28
void dMenuMii_c::updateMiiIcon(int icon) {
    int slot =
        (icon + mScroll) % ICON_COLUMNS + (icon >= ICON_COLUMNS) * ICON_COLUMNS;
    u16 id = getMiiId(icon);
    char name[40];
    sprintf(name, "MiiIcon_%02d", icon);
    if (id == MII_NONE) {
        mLyt.setVisible(name, false);
    } else {
        mLyt.setVisible(name, true);
        m2d::captureNode_c *node = &mCapture[slot];
        if (id != node->mID) {
            node->set(id, CAPTURE_SIZE, CAPTURE_SIZE, mpIconBuf[slot]);
            node->entry();
        }
    }
}

// .text:0x1B3C
void dMenuMii_c::updateMiiIcons() {
    for (int i = 0; i < ICON_NUM; i++) {
        updateMiiIcon(i);
    }
}

// .text:0x1B88
int dMenuMii_c::getListIndex(int icon) {
    int column = icon % ICON_COLUMNS + mScroll;
    int page = column / PAGE_COLUMNS;
    int row = icon >= ICON_COLUMNS;
    int pos = row * PAGE_COLUMNS - page * PAGE_COLUMNS + column;
    return page * PAGE_SIZE + pos;
}

// .text:0x1BF4
int dMenuMii_c::getIcon(int listIndex) {
    int page = listIndex / PAGE_SIZE;
    int base = page * PAGE_COLUMNS;
    int inPage = listIndex - page * PAGE_SIZE;
    int row = inPage >= PAGE_COLUMNS;
    int column = inPage % PAGE_COLUMNS + base - mScroll;
    if (column < 0 || column >= ICON_COLUMNS) {
        return -1;
    }
    return column + row * ICON_COLUMNS;
}

// .text:0x1C74
u16 dMenuMii_c::getMiiId(int icon) {
    int listIndex = getListIndex(icon);
    if (listIndex >= MII_MAX) {
        return MII_NONE;
    }
    return mMiiIndex[listIndex];
}

// .text:0x1CC0
void dMenuMii_c::refreshIcons() {
    updateBeforeIcon();
    bindIconBuffers();
    updateMiiIcons();
    onFlag(FLAG_UPDATE);
}

// .text:0x1D08
BOOL dMenuMii_c::applyMii() {
    if (mSelected == -1) {
        return FALSE;
    }
    u16 id = mMiiIndex[mSelected];
    if (id == MII_NONE) {
        return FALSE;
    }
    dPlayerMgr::MiiData_c info;
    if (RFLGetAdditionalInfo((RFLCharInfo *)&info, RFLDataSource_Official, nullptr,
                       id) != RFLErrcode_Success) {
        return FALSE;
    }
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiId =
        *(const dMiiCreateID_c *)info.mCreateID;
    return TRUE;
}

// .text:0x1DC0
u16 dMenuMii_c::getCurrentMiiIndex() {
    u16 index;
    if (RFLSearchOfficialData(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiId,
                              &index) == FALSE) {
        return MII_NONE;
    }
    return index;
}
