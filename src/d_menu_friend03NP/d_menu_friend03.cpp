#include <game/game/d_menu_friend03.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <stdio.h>

namespace dMenuFriend03 {
// .text:0x5C
BASE_PROFILE(MENU_FRIEND03, dMenuFriend03_c)
} // namespace dMenuFriend03

static const char *l_allPane = "N_all";

// The status data of a friend (dNetMgr_c::getFriendStatus).
static u8 l_friendStatus[0x68];

// .text:0x190
int dMenuFriend03_c::create() {
    BOOL loaded = mRes.load("Layout/friend_odekake.arc");
    loaded &= mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "friend_odekake_win_00.brlyt");
    mLyt.setVisible("B_msg_04", false);
    mLyt.setMessage("T_ttl_00", MSG_GO_TO_WHICH_TOWN, nullptr);
    mLyt.calc();
    mBtn.create();
    initButtons();
    initAnms();
    initList();
    initRowWords();
    refreshAll();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x2B4
int dMenuFriend03_c::doDelete() {
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes.remove() != 0;
}

// .text:0x308
int dMenuFriend03_c::stateMain() {
    executeSubState();
    if (isFlag(FLAG_REFRESH)) {
        offFlag(FLAG_REFRESH);
        refreshAll();
    }
    return SUCCEEDED;
}

// .text:0x360
int dMenuFriend03_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(&mLyt, l_allPane);
    return result;
}

// .text:0x3B0
int dMenuFriend03_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x3D4
int dMenuFriend03_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prevCursor) {
        playSeCursor();
    }
    mBtn.execute();
    calcAnms();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    return result;
}

// .text:0x478
int dMenuFriend03_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x4C4
void dMenuFriend03_c::subStateMain() {
    if (dNetMgr_c::sInstance->mInfo.mError.mState) {
        startCloseAnm();
        return;
    }
    if (updateMembers(FALSE)) {
        buildNewOrder();
        BOOL changed = isOrderChanged();
        setScrlBtnAnms();
        if (changed) {
            mSel = -1;
            setFadeAnms(FADE_ANM_OUT);
            setSubState(SUB_REFRESH_OUT);
            return;
        }
        onFlag(FLAG_REFRESH);
    }
    setSelect(mCursor, FALSE);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != CURSOR_NONE) {
        mCursor = btn;
        setSelect(btn, TRUE);
        push();
    } else {
        mCursor = hitCheck();
        if (isRowEmpty(mCursor)) {
            mCursor = CURSOR_NONE;
        }
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                push();
            }
        }
    }
}

// .text:0x634
void dMenuFriend03_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x670
void dMenuFriend03_c::subStateScroll() {
    mScrlAnm.play();
    onFlag(FLAG_CALC);
    if (mScrlAnm.isEnd()) {
        if (mCursor == CURSOR_SCRL_NEXT) {
            mScroll++;
            onFlag(FLAG_REFRESH);
        }
        setScrlBtnAnms();
        if (mPad::g_currentCore->down(WPAD_BUTTON_A) && isSelectable(mCursor)) {
            startScroll(mCursor);
        } else {
            mScrlAnm.start(SCRL_ANM_UP);
            setSubState(SUB_MAIN);
        }
    }
}

// .text:0x754
void dMenuFriend03_c::subStateRefreshOut() {
    if (!isFlag(FLAG_FADE_ANM)) {
        setFadeAnms(FADE_ANM_IN);
        applyNewOrder();
        setScrlBtnAnms();
        onFlag(FLAG_REFRESH);
        setSubState(SUB_REFRESH_IN);
    }
}

// .text:0x7CC
void dMenuFriend03_c::subStateRefreshIn() {
    if (!isFlag(FLAG_FADE_ANM)) {
        setSubState(SUB_MAIN);
    }
}

// .text:0x81C
void dMenuFriend03_c::executeSubState() {
    typedef void (dMenuFriend03_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuFriend03_c::subStateMain,
        &dMenuFriend03_c::subStatePush,
        &dMenuFriend03_c::subStateScroll,
        &dMenuFriend03_c::subStateRefreshOut,
        &dMenuFriend03_c::subStateRefreshIn,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x854
void dMenuFriend03_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtn.setSelect(cursor, select, FALSE);
    }
    if (isRow(cursor)) {
        setRowSelect(cursor, select);
    }
    if (isScrl(cursor)) {
        int anmNo;
        if (select) {
            anmNo = SCRL_BTN_ANM_IN;
        } else {
            anmNo = SCRL_BTN_ANM_OUT;
        }
        m2d::AnmEx_c *anm = &mScrlBtnAnm[cursor - CURSOR_SCRL];
        anm->start(anmNo);
        anm->setToEnd();
        onFlag(FLAG_CALC);
    }
}

// .text:0x94C
int dMenuFriend03_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int cursor = mBtn.hitCheck(&pos);
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    mVec2_c lytPos = pos;
    cursor = m2d::hitButton(&lytPos, &mLyt, CURSOR_ROW, CURSOR_HIT_LAST, -1, -1,
                            1.0f);
    if (isRow(cursor) && !isSelectable(cursor)) {
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

// .text:0xA6C
void dMenuFriend03_c::push() {
    if (!isSelectable(mCursor)) {
        return;
    }
    if (isBtn(mCursor)) {
        mBtn.push(mCursor, FALSE);
    } else if (isRow(mCursor)) {
        int row = mCursor - CURSOR_ROW;
        mRowAnm[row].setNext(ROW_ANM_PUSH);
        if (mSel != row + mScroll) {
            playSeListChoice();
        }
    } else if (isScrl(mCursor)) {
        mScrlBtnAnm[mCursor - CURSOR_SCRL].setNext(SCRL_BTN_ANM_PUSH);
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xB8C
BOOL dMenuFriend03_c::isPushEnd() {
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
    return TRUE;
}

// .text:0xC50
void dMenuFriend03_c::finishPush() {
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
            dMenuMng_c::setSelSlot(mOrder[mSel]);
            dMenuMng_c::setResult(RESULT_CONFIRM);
            break;
        }
    } else if (isRow(cursor)) {
        selectRow(cursor);
        setSubState(SUB_MAIN);
    } else if (isScrl(cursor)) {
        startScroll(cursor);
        setSubState(SUB_SCROLL);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xD90
void dMenuFriend03_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(RESULT_CANCEL);
}

// .text:0xDD0
void dMenuFriend03_c::initList() {
    updateMembers(TRUE);
    buildNewOrder();
    applyNewOrder();
    mSel = -1;
    mScroll = 0;
    setScrlBtnAnms();
}

// .text:0xE28
void dMenuFriend03_c::startScroll(int cursor) {
    if (cursor == CURSOR_SCRL_PREV) {
        mScroll--;
        onFlag(FLAG_REFRESH);
        mScrlAnm.start(SCRL_ANM_DOWN);
    } else {
        mScrlAnm.start(SCRL_ANM_UP);
    }
    playSePageTurn();
}

// .text:0xE8C
void dMenuFriend03_c::setScrlBtnAnms() {
    mScrlBtnAnm[0].start(canScrollPrev() ? SCRL_BTN_ANM_CAN_PUSH
                                         : SCRL_BTN_ANM_NO_PUSH);
    mScrlBtnAnm[0].setToEnd();
    mScrlBtnAnm[1].start(canScrollNext() ? SCRL_BTN_ANM_CAN_PUSH
                                         : SCRL_BTN_ANM_NO_PUSH);
    mScrlBtnAnm[1].setToEnd();
    setConfirmDisable(isSelectable(BTN_CONFIRM) == FALSE);
    onFlag(FLAG_CALC);
}

// .text:0xF28
BOOL dMenuFriend03_c::isSelectable(int cursor) {
    if (isScrl(cursor)) {
        if (cursor == CURSOR_SCRL_PREV) {
            return canScrollPrev();
        }
        return canScrollNext();
    }
    if (isBtn(cursor)) {
        if (cursor == BTN_CONFIRM) {
            if (mSel == -1) {
                return FALSE;
            }
            if (getMembers(mSel) == MEMBERS_FULL) {
                return FALSE;
            }
        }
        return TRUE;
    }
    if (isRow(cursor)) {
        int idx = getRowIdx(cursor);
        if (idx == -1) {
            return FALSE;
        }
        if (getMembers(idx) == MEMBERS_NONE ||
            getMembers(idx) == MEMBERS_FULL) {
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x106C
BOOL dMenuFriend03_c::canScrollPrev() {
    return mScroll > 0;
}

// .text:0x1080
BOOL dMenuFriend03_c::canScrollNext() {
    return mScroll < mNum - SCROLL_MARGIN;
}

// .text:0x10A4
void dMenuFriend03_c::selectRow(int cursor) {
    int prev = mSel;
    mSel = cursor + mScroll - CURSOR_ROW;
    setConfirmDisable(FALSE);
    if (prev != mSel) {
        int prevCursor = prev - mScroll + CURSOR_ROW;
        if (isRow(prevCursor)) {
            setRowSelect(prevCursor, FALSE);
        }
    }
    updateSelBase();
}

// .text:0x113C
BOOL dMenuFriend03_c::isRowEmpty(int cursor) {
    if (!isRow(cursor)) {
        return FALSE;
    }
    int idx = cursor + mScroll - CURSOR_ROW;
    if (idx < 0 || idx >= mNum) {
        return TRUE;
    }
    return FALSE;
}

// .text:0x1190
void dMenuFriend03_c::setRowSelect(int cursor, BOOL select) {
    int row = cursor - CURSOR_ROW;
    int idx = getRowIdx(cursor);
    if (idx != -1) {
        m2d::AnmEx2_c *anm;
        if (getMembers(idx) == MEMBERS_FULL) {
            anm = &mRowAnm[row];
            anm->start(ROW_ANM_GRAY);
        } else if (select) {
            anm = &mRowAnm[row];
            anm->start(ROW_ANM_IN);
        } else {
            anm = &mRowAnm[row];
            anm->start(ROW_ANM_OUT);
        }
        anm->setToEnd();
        onFlag(FLAG_CALC);
    }
}

// .text:0x125C
void dMenuFriend03_c::resetRowAnms() {
    for (int i = CURSOR_ROW; i <= CURSOR_ROW_LAST; i++) {
        setRowSelect(i, FALSE);
    }
}

// .text:0x12AC
void dMenuFriend03_c::updateSelBase() {
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

// .text:0x133C
void dMenuFriend03_c::refreshAll() {
    resetRowAnms();
    updateRowWords();
    updateMans();
    updateSelBase();
}

// .text:0x1380
void dMenuFriend03_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
    mBtn.setDisable(BTN_CONFIRM, TRUE);
}

// .text:0x13F0
void dMenuFriend03_c::setConfirmDisable(int disable) {
    mBtn.setDisable(BTN_CONFIRM, disable != 0);
}

// .text:0x140C
void dMenuFriend03_c::initAnms() {
    static const char *rowAnmNames[ROW_ANM_NUM] = {
        "friend_odekake_win_00_T_msg_White.brlan",
        "friend_odekake_win_00_T_msg_Gry.brlan",
        "friend_odekake_win_00_T_msg_In.brlan",
        "friend_odekake_win_00_T_msg_Out.brlan",
        "friend_odekake_win_00_T_msg_Psh.brlan",
        "friend_odekake_win_00_T_msg_Before.brlan",
    };
    static const char *fadeAnmNames[FADE_ANM_NUM] = {
        "friend_odekake_win_00_msgMan_alpIn.brlan",
        "friend_odekake_win_00_msgMan_alpOut.brlan",
    };
    static const char *scrlBtnAnmNames0[SCRL_BTN_ANM_NUM] = {
        "friend_odekake_win_00_scrlBtn_00_In.brlan",
        "friend_odekake_win_00_scrlBtn_00_Out.brlan",
        "friend_odekake_win_00_scrlBtn_00_Psh.brlan",
        "friend_odekake_win_00_scrlBtn_00_CanPsh.brlan",
        "friend_odekake_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrlBtnAnmNames1[SCRL_BTN_ANM_NUM] = {
        "friend_odekake_win_00_scrlBtn_01_In.brlan",
        "friend_odekake_win_00_scrlBtn_01_Out.brlan",
        "friend_odekake_win_00_scrlBtn_01_Psh.brlan",
        "friend_odekake_win_00_scrlBtn_01_CanPsh.brlan",
        "friend_odekake_win_00_scrlBtn_01_NoPsh.brlan",
    };
    static const char *scrlAnmNames[SCRL_ANM_NUM] = {
        "friend_odekake_win_00_scrlDown.brlan",
        "friend_odekake_win_00_scrlUp.brlan",
    };
    static const char *manAnmNames[MAN_ANM_NUM] = {
        "friend_odekake_win_00_man_white.brlan",
        "friend_odekake_win_00_man_blue.brlan",
        "friend_odekake_win_00_man_red.brlan",
    };
    for (int i = 0; i < ROW_NUM; i++) {
        mRowAnm[i].init(&mLyt, &mRes, rowAnmNames, ROW_ANM_NUM);
        mRowAnm[i].setNumber(i);
    }
    for (int i = 0; i < ROW_NUM; i++) {
        mFadeAnm[i].init(&mLyt, &mRes, fadeAnmNames, FADE_ANM_NUM);
        mFadeAnm[i].setNumber(i);
    }
    mScrlBtnAnm[0].init(&mLyt, &mRes, scrlBtnAnmNames0, SCRL_BTN_ANM_NUM);
    mScrlBtnAnm[1].init(&mLyt, &mRes, scrlBtnAnmNames1, SCRL_BTN_ANM_NUM);
    mScrlAnm.init(&mLyt, &mRes, scrlAnmNames, SCRL_ANM_NUM);
    mScrlAnm.start(SCRL_ANM_UP);
    for (int i = 0; i < ROW_NUM * MAN_NUM; i++) {
        mManAnm[i].init(&mLyt, &mRes, manAnmNames, MAN_ANM_NUM);
        mManAnm[i].setNumber(i);
    }
    mLyt.setAnm(&mIconAnm, "friend_odekake_win_00_scrlIcon_00.brlan", nullptr);
}

// .text:0x1570
void dMenuFriend03_c::calcAnms() {
    int i;
    BOOL playing = FALSE;
    offFlag(FLAG_FADE_ANM);
    for (i = 0; i < ROW_NUM; i++) {
        playing |= mFadeAnm[i].play();
    }
    if (playing) {
        onFlag(FLAG_FADE_ANM);
    }
    for (i = 0; i < ROW_NUM; i++) {
        playing |= mRowAnm[i].play();
    }
    if (playing) {
        onFlag(FLAG_ROW_ANM);
    } else {
        offFlag(FLAG_ROW_ANM);
    }
    for (i = 0; i < SCRL_NUM; i++) {
        playing |= mScrlBtnAnm[i].play();
    }
    mIconAnm.play();
    onFlag(FLAG_CALC);
}

// .text:0x1674
void dMenuFriend03_c::initRowWords() {
    char nameName[40];
    char townName[40];
    for (int i = 0; i < ROW_NUM; i++) {
        sprintf(townName, "T_msg_%02d", i);
        mTownWord[i].setTextBox(mLyt.findTextBox(townName));
        sprintf(nameName, "T_name_%02d", i);
        mNameWord[i].setTextBox(mLyt.findTextBox(nameName));
    }
}

// .text:0x1724
void dMenuFriend03_c::updateRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + mScroll;
        char name[40];
        sprintf(name, "B_msg_%02d", i);
        if (idx >= 0 && idx < mNum) {
            mTownWord[i].mpTextBox->SetVisible(true);
            mLyt.setVisible(name, true);
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

// .text:0x1884
const wchar_t *dMenuFriend03_c::getTownName(int idx) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    return player->mFriends.getFriend(mOrder[idx])->mPID.land.mName;
}

// .text:0x18D0
const wchar_t *dMenuFriend03_c::getPlayerName(int idx) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    return player->mFriends.getFriend(mOrder[idx])->mPID.player.mName;
}

// .text:0x191C
void dMenuFriend03_c::setManAnms(int row) {
    int i = row * MAN_NUM;
    int j;
    int members = getMembers(row + mScroll);
    if (members == MEMBERS_NONE || members == MEMBERS_FULL) {
        for (j = 0; j < MAN_NUM; i++, j++) {
            mManAnm[i].start(MAN_ANM_RED);
            mManAnm[i].setToEnd();
        }
    } else {
        for (j = 0; j < members; i++, j++) {
            mManAnm[i].start(MAN_ANM_BLUE);
            mManAnm[i].setToEnd();
        }
        for (; j < MAN_NUM; i++, j++) {
            mManAnm[i].start(MAN_ANM_WHITE);
            mManAnm[i].setToEnd();
        }
    }
}

// .text:0x1A18
void dMenuFriend03_c::updateMans() {
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + mScroll;
        char name[32];
        sprintf(name, "N_man_%02d", i);
        if (idx >= 0 && idx < mNum) {
            mLyt.setVisible(name, true);
            setManAnms(i);
        } else {
            mLyt.setVisible(name, false);
        }
    }
    onFlag(FLAG_CALC);
}

// .text:0x1AD8
int dMenuFriend03_c::getMembers(int idx) {
    return mMembers[mOrder[idx]];
}

// .text:0x1AEC
BOOL dMenuFriend03_c::updateMembers(BOOL init) {
    u32 i;
    BOOL changed = FALSE;
    BOOL appeared = FALSE;
    for (i = 0; i < MEMBERS_NUM; i++) {
        u32 size;
        u8 members;
        if (isFriendValid(i) == FALSE) {
            members = 0;
        } else {
            members =
                dNetMgr_c::sInstance->getFriendStatus(i, l_friendStatus, &size);
        }
        u8 prev = mMembers[i];
        changed |= (members != prev);
        if (prev == MEMBERS_NONE && members != MEMBERS_NONE) {
            appeared = TRUE;
        }
        mMembers[i] = members;
    }
    if (!init && appeared) {
        playSe(SE_Network_MessageAlert);
    }
    return changed;
}

// .text:0x1BC4
BOOL dMenuFriend03_c::isFriendValid(int friendIdx) {
    return dWifi::isFriendValid(friendIdx);
}

// .text:0x1BCC
void dMenuFriend03_c::buildNewOrder() {
    int i;
    for (i = 0; i < MEMBERS_NUM; i++) {
        mNewOrder[i] = 0xFF;
    }
    int num = 0;
    for (i = 0; i < MEMBERS_NUM; i++) {
        if (mMembers[i] != MEMBERS_NONE) {
            mNewOrder[num++] = i;
        }
    }
    mNumNew = num;
}

// .text:0x1D6C
void dMenuFriend03_c::applyNewOrder() {
    for (int i = 0; i < MEMBERS_NUM; i++) {
        mOrder[i] = mNewOrder[i];
    }
    mNum = mNumNew;
}

// .text:0x1E18
BOOL dMenuFriend03_c::isOrderChanged() {
    for (int i = mScroll; i < mScroll + ROW_NUM; i++) {
        if (mOrder[i] != mNewOrder[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x1E5C
int dMenuFriend03_c::getRowIdx(int cursor) {
    if (!isRow(cursor)) {
        return -1;
    }
    int idx = cursor + mScroll - CURSOR_ROW;
    if (idx < 0 || idx >= mNum) {
        return -1;
    }
    return idx;
}

// .text:0x1EB0
void dMenuFriend03_c::setFadeAnms(int anm) {
    for (int i = 0; i < ROW_NUM - 1; i++) {
        int idx = mScroll + i;
        if (mOrder[idx] != mNewOrder[idx]) {
            mFadeAnm[i].setNext(anm);
        }
    }
}
