#include <game/game/d_menu_friend06.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_friend.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuFriend06 {
// .text:0x5C
BASE_PROFILE(MENU_FRIEND06, dMenuFriend06_c)
} // namespace dMenuFriend06

static const char *l_allPane = "N_all";

// The separator of the groups of the friend code.
static wchar_t l_codeSeparatorText[3] = L"ー";
static const wchar_t *l_codeSeparator = l_codeSeparatorText;

// .text:0xD8
int dMenuFriend06_c::create() {
    BOOL loaded = mRes.load("Layout/friend_code.arc");
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "my_friendCord_win_00.brlyt");
    mLyt.setAnm(&mAnm, "my_friendCord_win_00_scrlIcon_00.brlan", nullptr);
    setTexts();
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    mLyt.calc();
    onFlag(FLAG_DRAW);
    dMenuFriend_c::setTabVisible(TRUE);
    return SUCCEEDED;
}

// .text:0x1A0
int dMenuFriend06_c::doDelete() {
    return mRes.remove() != 0;
}

// .text:0x1D0
int dMenuFriend06_c::stateMain() {
    if (executeCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    return SUCCEEDED;
}

// .text:0x218
int dMenuFriend06_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    if (isState(STATE_MAIN)) {
        dMenuFriend_c::isFrameAnmDone();
    }
    dMenuFriend_c::setFrameVisible(TRUE);
    dMenuFriend_c::placeFrameTextBox(&mLyt, "N_myCordWin_all");
    return result;
}

// .text:0x284
int dMenuFriend06_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && prevCursor != mCursor) {
        if (isUitagTab(mCursor)) {
        } else if (isTab(mCursor)) {
            playSeSubMenuPopUp();
        } else {
            playSeCursor();
        }
    }
    mAnm.play();
    mLyt.calc();
    return result;
}

// .text:0x348
int dMenuFriend06_c::stateClose() {
    dMenuFriend_c::setFrameVisible(FALSE);
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x384
int dMenuFriend06_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
    }
    return SUCCEEDED;
}

// .text:0x3C8
void dMenuFriend06_c::subStateMain() {
    dMenuFriend_c::clearTabFocus();
    setSelect(mCursor, FALSE);
    dMenuFriend_c::hideTutorial();
    int tab = dAcUitag::getTabInput(dAcUitag_c::TAB_FRIEND, FALSE);
    if (tab == CURSOR_NONE || !selectUitagTab(tab, TRUE)) {
        dObjCursor_c::getPos();
        mCursor = hitCheck();
        setSelect(mCursor, TRUE);
        if (mCursor != CURSOR_NONE &&
            mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push();
            dMenuFriend_c::hideTutorial();
        }
    }
}

// .text:0x494
void dMenuFriend06_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x4D0
BOOL dMenuFriend06_c::subStateCloseRequest() {
    dAcUitag::closeTab();
    startCloseAnm();
    return FALSE;
}

// .text:0x508
BOOL dMenuFriend06_c::executeCloseRequest() {
    typedef BOOL (dMenuFriend06_c::*Proc)();
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    static Proc closeProcs[SUB_NUM] = {&dMenuFriend06_c::subStateCloseRequest,
                                       nullptr};
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL handled = (this->*closeProcs[mSubState])();
    if (handled) {
        dMenuMng_c::clearCloseRequest();
    }
    return handled;
}

// .text:0x5DC
void dMenuFriend06_c::executeSubState() {
    typedef void (dMenuFriend06_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {&dMenuFriend06_c::subStateMain,
                                          &dMenuFriend06_c::subStatePush};
    (this->*subStateProcs[mSubState])();
}

// .text:0x614
int dMenuFriend06_c::hitUitagTab(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = CURSOR_NONE;
    }
    dAcUitag::setTabFocus(tab);
    if (tab == CURSOR_NONE) {
        return CURSOR_NONE;
    }
    return tab;
}

// .text:0x678
BOOL dMenuFriend06_c::selectUitagTab(int tab, BOOL delay) {
    if (dAcUitag::selectTab(tab, delay)) {
        startCloseAnm();
        return TRUE;
    }
    return FALSE;
}

// .text:0x6C8
void dMenuFriend06_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dMenuFriend_c::requestClose();
}

// .text:0x704
void dMenuFriend06_c::startReopenAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dMenuFriend_c::requestReopen();
    playSeSubMenuPush();
}

// .text:0x744
void dMenuFriend06_c::startCommentAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dMenuFriend_c::requestOpenMenu(dMenuFriend_c::OPEN_MENU_MY_FRIEND_NAME);
    dMenuFriend_c::startTextBoxUp();
    playSeSubMenuPush();
}

// .text:0x78C
int dMenuFriend06_c::hitCheck() {
    mVec2_c pos = dObjCursor_c::getPos();
    int cursor = hitUitagTab(&pos);
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    int tab = dMenuFriend_c::getTabAt(&pos);
    if (tab != CURSOR_NONE) {
        return CURSOR_TAB + tab;
    }
    if (dMenuFriend_c::isFrameHit(&pos)) {
        return CURSOR_FRAME;
    }
    return CURSOR_NONE;
}

// .text:0x81C
void dMenuFriend06_c::setSelect(int cursor, BOOL select) {
    if (isTab(cursor)) {
        dMenuFriend_c::setTabFocus(cursor - CURSOR_TAB, select);
    }
    if (cursor == CURSOR_FRAME) {
        dMenuFriend_c::showTextBox(select);
    }
}

// .text:0x88C
void dMenuFriend06_c::push() {
    int cursor = mCursor;
    if (isUitagTab(cursor)) {
        selectUitagTab(cursor, FALSE);
        return;
    }
    if (isTab(cursor)) {
        if (!dMenuFriend_c::startTabPush(cursor - CURSOR_TAB)) {
            return;
        }
    } else if (cursor == CURSOR_FRAME) {
        dMenuFriend_c::resetTextBoxPush();
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x948
BOOL dMenuFriend06_c::isPushEnd() {
    if (isTab(mCursor)) {
        return dMenuFriend_c::isTabPushEnd(mCursor - CURSOR_TAB);
    }
    if (mCursor == CURSOR_FRAME) {
        return dMenuFriend_c::isTextBoxPushEnd();
    }
    return TRUE;
}

// .text:0x988
void dMenuFriend06_c::finishPush() {
    if (isTab(mCursor)) {
        dMenuFriend_c::setTab(mCursor - CURSOR_TAB);
        startReopenAnm();
    } else if (mCursor == CURSOR_FRAME) {
        startCommentAnm();
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xA14
void dMenuFriend06_c::setTexts() {
    mLyt.setMessage("T_title_00", MSG_YOUR_FRIEND_CODE, nullptr);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dString::setTownWord(0, &player->mPID.land);
    dString::setPlayerWord(1, &player->mPID);
    mLyt.setMessage("T_msg_00", MSG_TOWN_PLAYER, nullptr);
    u64 key = dWifi::getMyFriendKey();
    if (key == 0) {
        mLyt.setMessage("T_msg_01", MSG_NO_FRIEND_CODE, nullptr);
    } else {
        setFriendCode(key);
    }
}

// .text:0xAD0
void dMenuFriend06_c::setFriendCode(u64 key) {
    u32 low = key % 10000;
    key /= 10000;
    m2d::stringWord_c code;
    m2d::stringWord_c part;
    dScript::formatNumber(&part, (key / 10000) % 10000, 4,
                          dScript::NUM_FORMAT_ZERO_PAD, 0);
    code.appendWord(&part, 0);
    code.appendText(l_codeSeparator, 0);
    part.clear();
    dScript::formatNumber(&part, key % 10000, 4, dScript::NUM_FORMAT_ZERO_PAD,
                          0);
    code.appendWord(&part, 0);
    code.appendText(l_codeSeparator, 0);
    part.clear();
    dScript::formatNumber(&part, low, 4, dScript::NUM_FORMAT_ZERO_PAD, 0);
    code.appendWord(&part, 0);
    mLyt.setWord("T_msg_01", &code);
}
