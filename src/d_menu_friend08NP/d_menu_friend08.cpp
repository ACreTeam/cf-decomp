#include <game/game/d_menu_friend08.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_friend.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_friend.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <stdio.h>

namespace dMenuFriend08 {
// .text:0x5C
BASE_PROFILE(MENU_FRIEND08, dMenuFriend08_c)
} // namespace dMenuFriend08

// .text:0x120
int dMenuFriend08_c::create() {
    BOOL loaded = mRes.load("Layout/keyboard_add.arc");
    loaded &= mNumber.load();
    loaded &= mBtn.load();
    loaded &= mSlct.load();
    loaded &= mCur.load();
    if (!loaded) {
        return NOT_READY;
    }
    if (!mNumber.create(dMenuNumber_c::MODE_FRIEND_CODE)) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "cmn_question_00.brlyt");
    mLyt.setAnm(&mIconAnm, "cmn_question_00_iconPattern.brlan", nullptr);
    mIconAnm.setFrame(10.0f);
    hideInputPanes();
    mLyt.setVisible("T_starName", false);
    mLyt.setVisible("T_townName", false);
    mLyt.calc();
    mBtn.create();
    initButtons();
    mSlct.create();
    mCur.build();
    clearInput();
    initWords();
    if (mMenuNo == dMenuMng_c::MENU_FRIEND_CODE_EDIT) {
        setInputFromFriendKey();
    }
    onFlag(FLAG_DRAW);
    startAnm(&mBtn.mLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    setWinAnm(ANM_UPPER_WIN_IN);
    setSubState(SUB_MAIN);
    mCursor = CURSOR_NONE;
    return SUCCEEDED;
}

// .text:0x2D4
int dMenuFriend08_c::doDelete() {
    if (mCur.remove() == FALSE) {
        return NOT_READY;
    }
    if (mNumber.remove() == FALSE) {
        return NOT_READY;
    }
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    if (mSlct.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes.remove() != 0;
}

// .text:0x370
int dMenuFriend08_c::stateMain() {
    if (handleCloseRequest()) {
        return SUCCEEDED;
    }
    showCursor();
    executeSubState();
    calcLayout();
    return SUCCEEDED;
}

// .text:0x3C8
int dMenuFriend08_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    offFlag(FLAG_CUR);
    mNumber.setPos(&mBtn.mLyt, "N_all");
    if (isState(STATE_MAIN)) {
        updateCursorPos();
        mWinAnm.setToEnd();
    }
    mWinAnm.play();
    mLyt.calc();
    return result;
}

// .text:0x464
int dMenuFriend08_c::stateClose() {
    if (isFlag(FLAG_REGISTER)) {
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x4B0
int dMenuFriend08_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prevCursor &&
        !isFlag(FLAG_NO_CURSOR_SE)) {
        playSeCursor();
    }
    offFlag(FLAG_NO_CURSOR_SE);
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        offFlag(FLAG_HOLD);
    }
    if (isFlag(FLAG_HOLD) && hitCheck() != dMenuNumber_c::KEY_BACK) {
        offFlag(FLAG_HOLD);
    }
    mNumber.execute(isFlag(FLAG_HOLD));
    mBtn.execute();
    mSlct.execute();
    if (isFlag(FLAG_REGISTER) && dMenuFriend_c::registerEntry()) {
        offFlag(FLAG_REGISTER);
    }
    return result;
}

// .text:0x5E4
int dMenuFriend08_c::draw() {
    if (isFlag(FLAG_WORD)) {
        offFlag(FLAG_WORD);
        updateWords();
        updateCursorPos();
    }
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mNumber.draw();
        mBtn.draw();
    }
    if (isFlag(FLAG_SLCT)) {
        mSlct.draw();
    }
    if (isFlag(FLAG_CUR)) {
        mCur.draw();
    }
    return SUCCEEDED;
}

// .text:0x6A0
void dMenuFriend08_c::subStateMain() {
    onFlag(FLAG_NO_CURSOR_SE);
    setSelect(mCursor, FALSE);
    int key = mNumber.getKbdKey();
    if (key != CURSOR_NONE) {
        mCursor = key;
        setSelect(key, TRUE);
        push();
        return;
    }
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != CURSOR_NONE) {
        int slot = btn + CURSOR_BTN;
        mCursor = slot;
        setSelect(slot, TRUE);
        push();
        return;
    }
    if (mBtn.isEnable(BTN_CONFIRM) && mNumber.isSendReq()) {
        mCursor = CURSOR_BTN + BTN_CONFIRM;
        push();
        return;
    }
    if (mBtn.isEnable(BTN_QUIT) && mNumber.isClearReq()) {
        mCursor = CURSOR_BTN + BTN_QUIT;
        push();
        return;
    }
    u8 special = mNumber.getSpecialKey();
    BOOL moved = FALSE;
    switch (special) {
    case SPECIAL_KEY_LEFT:
        moveLeft();
        return;
    case SPECIAL_KEY_RIGHT:
        moveRight();
        return;
    case SPECIAL_KEY_HOME:
        moveHome();
        return;
    case SPECIAL_KEY_END:
        moveEnd();
        return;
    default:
        if (dPad::isLeftHeld()) {
            if (dPad::isLeftTriggered()) {
                moveLeft();
                moved = TRUE;
                mRepeat = KEY_REPEAT_FIRST;
            } else if (mRepeat > 0) {
                mRepeat--;
            } else {
                moveLeft();
                moved = TRUE;
                mRepeat = KEY_REPEAT;
            }
        } else if (dPad::isRightHeld()) {
            if (dPad::isRightTriggered()) {
                moveRight();
                moved = TRUE;
                mRepeat = KEY_REPEAT_FIRST;
            } else if (mRepeat > 0) {
                mRepeat--;
            } else {
                moveRight();
                moved = TRUE;
                mRepeat = KEY_REPEAT;
            }
        } else if (dPad::isUpTriggered()) {
            moveHome();
            moved = TRUE;
        } else if (dPad::isDownTriggered()) {
            moveEnd();
            moved = TRUE;
        }
        break;
    }
    if (!moved) {
        offFlag(FLAG_NO_CURSOR_SE);
    }
    int slot = hitCheck();
    mCursor = slot;
    if (slot != CURSOR_NONE) {
        setSelect(slot, TRUE);
        if (!moved && mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push();
            onFlag(FLAG_HOLD);
            mHold = HOLD_FIRST;
        }
    } else if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        if (hitCheckWords()) {
            onFlag(FLAG_CALC);
        }
    }
}

// .text:0x9E0
void dMenuFriend08_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0xA1C
void dMenuFriend08_c::subStateMessage() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_MESSAGE_PUSH);
    }
}

// .text:0xA7C
void dMenuFriend08_c::subStateMessagePush() {
    if (mSlct.isPushEnd()) {
        setSubState(SUB_MESSAGE_CLOSE);
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0xAE0
void dMenuFriend08_c::subStateMessageClose() {
    if (mSlct.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_SLCT);
    }
}

// .text:0xB3C
void dMenuFriend08_c::subStateCheckKey() {
    if (dNetMgr_c::reqCheckKey(mKey)) {
        setSubState(SUB_CHECK_KEY_RESULT);
    }
}

// .text:0xB90
void dMenuFriend08_c::subStateCheckKeyResult() {
    int result;
    if (dNetMgr_c::sInstance->mReq.takeResult(
            dNetFriend::NET_FRIEND_REQ_CHECK_KEY, &result)) {
        if (result != 0) {
            setSubState(SUB_KEY_DATA);
        } else {
            openMessage(MSG_WRONG_CODE);
            playSeError();
        }
    }
}

// .text:0xC14
void dMenuFriend08_c::subStateKeyData() {
    if (dNetMgr_c::reqKeyData(mKey)) {
        setSubState(SUB_KEY_DATA_RESULT);
    }
}

// .text:0xC68
void dMenuFriend08_c::subStateKeyDataResult() {
    if (dNetMgr_c::sInstance->mReq.takeResultData(
            dNetFriend::NET_FRIEND_REQ_KEY_DATA, &mFriendData)) {
        setSubState(SUB_FIND_FRIEND);
    }
}

// .text:0xCC8
void dMenuFriend08_c::subStateFindFriend() {
    int idx = dWifi::findFriendDataReq(&mFriendData);
    if (idx != WIFI_PENDING) {
        BOOL canRegister = idx == -1;
        if (mMenuNo == dMenuMng_c::MENU_FRIEND_CODE_EDIT) {
            canRegister |= idx == dMenuFriend_c::getFriendIdx();
        }
        if (canRegister) {
            if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
                mMember = 0;
                setSubState(SUB_SEARCH_MEMBER);
            } else {
                dMenuFriend_c::setEntryFriendData(&mFriendData);
                finish(FALSE);
            }
        } else {
            openMessage(MSG_ALREADY_REGISTERED);
            playSeCanNot();
        }
    }
}

// .text:0xDAC
void dMenuFriend08_c::subStateSearchMember() {
    int myNo = dNet::getMyNo();
    if (mMember >= NET_MEMBER_MAX) {
        dMenuFriend_c::setEntryFriendData(&mFriendData);
        finish(FALSE);
    } else if (myNo == mMember) {
        mMember++;
    } else if (!dNet::isMember(mMember)) {
        mMember++;
    } else {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(mMember);
        if (dNetMgr_c::reqIsFriend(&player->mFriends.mUserData, mKey)) {
            setSubState(SUB_MEMBER_RESULT);
        }
    }
}

// .text:0xE68
void dMenuFriend08_c::subStateMemberResult() {
    int result;
    if (dNetMgr_c::sInstance->mReq.takeResult(
            dNetFriend::NET_FRIEND_REQ_IS_FRIEND, &result)) {
        if (result != 0) {
            dMenuFriend_c::setEntryFromNetPlayer(mMember);
            setSubState(SUB_MEMBER_DATA);
        } else {
            mMember++;
            setSubState(SUB_SEARCH_MEMBER);
        }
    }
}

// .text:0xF04
void dMenuFriend08_c::subStateMemberData() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(mMember);
    if (dNetMgr_c::reqMyData(&player->mFriends.mUserData)) {
        setSubState(SUB_MEMBER_DATA_RESULT);
    }
}

// .text:0xF5C
void dMenuFriend08_c::subStateMemberDataResult() {
    if (dNetMgr_c::sInstance->mReq.takeResultData(
            dNetFriend::NET_FRIEND_REQ_MY_DATA, &mFriendData)) {
        dMenuFriend_c::setEntryFriendData(&mFriendData);
        finish(TRUE);
    }
}

// .text:0xFB8
BOOL dMenuFriend08_c::closeMenu() {
    startCloseAnm();
    dMenuFriend_c::requestClose();
    dAcUitag::closeTab();
    dAcUitag::setHideAfterMenu();
    return TRUE;
}

// .text:0xFE8
BOOL dMenuFriend08_c::closeMessage() {
    if (!mSlct.isWinAnmEnd()) {
        return FALSE;
    }
    mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    return closeMenu();
}

// .text:0x1044
BOOL dMenuFriend08_c::handleCloseRequest() {
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    static CloseProc procs[SUB_NUM] = {&dMenuFriend08_c::closeMenu,
                                       nullptr,
                                       &dMenuFriend08_c::closeMessage,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr};
    if (!procs[mSubState]) {
        return FALSE;
    }
    BOOL handled = (this->*procs[mSubState])();
    if (handled) {
        dMenuMng_c::clearCloseRequest();
    }
    return handled;
}

// .text:0x11A8
void dMenuFriend08_c::executeSubState() {
    typedef void (dMenuFriend08_c::*Proc)();
    static Proc procs[SUB_NUM] = {&dMenuFriend08_c::subStateMain,
                                  &dMenuFriend08_c::subStatePush,
                                  &dMenuFriend08_c::subStateMessage,
                                  &dMenuFriend08_c::subStateMessagePush,
                                  &dMenuFriend08_c::subStateMessageClose,
                                  &dMenuFriend08_c::subStateCheckKey,
                                  &dMenuFriend08_c::subStateCheckKeyResult,
                                  &dMenuFriend08_c::subStateKeyData,
                                  &dMenuFriend08_c::subStateKeyDataResult,
                                  &dMenuFriend08_c::subStateFindFriend,
                                  &dMenuFriend08_c::subStateSearchMember,
                                  &dMenuFriend08_c::subStateMemberResult,
                                  &dMenuFriend08_c::subStateMemberData,
                                  &dMenuFriend08_c::subStateMemberDataResult};
    (this->*procs[mSubState])();
}

// .text:0x11E0
void dMenuFriend08_c::showCursor() {
    onFlag(FLAG_CUR);
}

// .text:0x11E8
void dMenuFriend08_c::calcLayout() {
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
}

// .text:0x1240
int dMenuFriend08_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    int key = mNumber.hitCheck();
    mVec2_c pos = dObjCursor_c::getPos();
    if (key != CURSOR_NONE) {
        return key;
    }
    int btn = mBtn.hitCheck(&pos);
    if (btn != CURSOR_NONE) {
        return btn + CURSOR_BTN;
    }
    return CURSOR_NONE;
}

// .text:0x12C8
void dMenuFriend08_c::setSelect(int slot, BOOL select) {
    if (isKey(slot)) {
        int anm = dMenuNumber_c::KEY_ANM_OUT;
        if (select) {
            anm = dMenuNumber_c::KEY_ANM_IN;
        }
        mNumber.setCursor(slot, anm);
    }
    if (isBtn(slot)) {
        mBtn.setSelect(slot - CURSOR_BTN, select, FALSE);
    }
}

// .text:0x1378
void dMenuFriend08_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
}

// .text:0x13D8
void dMenuFriend08_c::push() {
    int slot = mCursor;
    if (isKey(slot)) {
        mNumber.push(slot);
        if (inputKey()) {
            mNumber.playKeySe(slot);
        } else {
            dKbd::playSeCanNotInput();
        }
    }
    slot = mCursor;
    if (isBtn(slot)) {
        mBtn.push(slot - CURSOR_BTN, FALSE);
    }
    offFlag(FLAG_HOLD);
    setSubState(SUB_PUSH);
}

// .text:0x14B4
BOOL dMenuFriend08_c::isPushEnd() {
    int slot = mCursor;
    if (isKey(slot)) {
        return !mNumber.isPushing();
    }
    if (isBtn(slot)) {
        return !mBtn.isPushing();
    }
    return TRUE;
}

// .text:0x1540
void dMenuFriend08_c::finishPush() {
    int slot = mCursor;
    if (isBtn(slot)) {
        dMenuMng_c::setResult(0);
        switch (mCursor - CURSOR_BTN) {
        case BTN_QUIT:
            startCloseAnm();
            dMenuFriend_c::requestShowTabs();
            break;
        case BTN_CONFIRM:
            if (confirmKey()) {
                finish(FALSE);
            }
            break;
        case BTN_UNUSED:
            return;
        default:
            return;
        }
    } else if (isKey(slot)) {
        if (slot == dMenuNumber_c::KEY_BACK && isFlag(FLAG_HOLD)) {
            if (mHold > 0) {
                mHold--;
            } else {
                push();
                onFlag(FLAG_HOLD);
                mHold = HOLD_REPEAT;
            }
        } else {
            setSubState(SUB_MAIN);
        }
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x16A0
BOOL dMenuFriend08_c::inputKey() {
    int digit = dMenuNumber_c::getKeyDigit(mCursor);
    if (digit == dMenuNumber_c::DIGIT_BACK) {
        if (mNumber.isKbdBack()) {
            // delete the digit after the text cursor
            if (mNum < DIGIT_NUM && mDigit[mNum] != DIGIT_EMPTY) {
                for (int i = mNum; i < DIGIT_NUM - 1; i++) {
                    mDigit[i] = mDigit[i + 1];
                }
                mDigit[DIGIT_NUM - 1] = DIGIT_EMPTY;
                mBtn.setDisable(BTN_CONFIRM, TRUE);
                onFlag(FLAG_WORD);
            } else {
                return FALSE;
            }
        } else {
            // delete the digit before the text cursor
            if (mNum > 0) {
                mNum--;
                for (int i = mNum; i < DIGIT_NUM - 1; i++) {
                    mDigit[i] = mDigit[i + 1];
                }
                mDigit[DIGIT_NUM - 1] = DIGIT_EMPTY;
                mBtn.setDisable(BTN_CONFIRM, TRUE);
                onFlag(FLAG_WORD);
            } else {
                return FALSE;
            }
        }
    } else if (digit == dMenuNumber_c::DIGIT_CLEAR) {
        clearInput();
    } else {
        if (mDigit[DIGIT_NUM - 1] == DIGIT_EMPTY) {
            for (int i = DIGIT_NUM - 2; i >= mNum; i--) {
                mDigit[i + 1] = mDigit[i];
            }
            mDigit[mNum] = digit;
            mNum++;
            if (mDigit[DIGIT_NUM - 1] != DIGIT_EMPTY) {
                mBtn.setDisable(BTN_CONFIRM, FALSE);
            }
            onFlag(FLAG_WORD);
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

// .text:0x1A58
void dMenuFriend08_c::clearInput() {
    mNum = 0;
    for (int i = 0; i < DIGIT_NUM; i++) {
        mDigit[i] = DIGIT_EMPTY;
    }
    mBtn.setDisable(BTN_CONFIRM, TRUE);
    onFlag(FLAG_WORD);
}

// .text:0x1AD8
void dMenuFriend08_c::setInputFromFriendKey() {
    u64 key = dMenuFriend_c::getFriendKey();
    for (int i = 0; i < DIGIT_NUM; i++) {
        mDigit[DIGIT_NUM - 1 - i] = key % 10;
        key /= 10;
    }
    mNum = DIGIT_NUM;
    mBtn.setDisable(BTN_CONFIRM, FALSE);
    onFlag(FLAG_WORD);
}

// .text:0x1CC4
void dMenuFriend08_c::hideInputPanes() {
    static const char *paneNames[INPUT_PANE_NUM] = {
        "N_input_A", "N_input_B", "N_input_C", "N_input_D", "N_input_F"};
    for (int i = 0; i < INPUT_PANE_NUM; i++) {
        mLyt.setVisible(paneNames[i], false);
    }
    onFlag(FLAG_WORD);
}

// .text:0x1D34
void dMenuFriend08_c::updateWords() {
    int idx = 0;
    for (int i = 0; i < WORD_NUM; i++) {
        int value, num;
        num = 0;
        value = 0;
        for (int j = 0; j < WORD_DIGIT_NUM; j++) {
            int digit = mDigit[idx];
            if (digit == DIGIT_EMPTY) {
                break;
            }
            value *= 10;
            value += digit;
            num = j + 1;
            idx++;
        }
        mWord[i].clear();
        if (num != 0) {
            dScript::formatNumber(&mWord[i], value, num,
                                  dScript::NUM_FORMAT_ZERO_PAD, 0);
        }
        mWord[i].update();
    }
}

// .text:0x1E24
void dMenuFriend08_c::initWords() {
    for (int i = 0; i < WORD_NUM; i++) {
        char name[0x20];
        sprintf(name, "T_input_E_%02d", i);
        mWord[i].setTextBox(mLyt.findTextBox(name));
    }
    onFlag(FLAG_WORD);
    mLyt.setMessage("T_msg_00", MSG_WHATS_THE_CODE, nullptr);
}

// .text:0x1ED0
void dMenuFriend08_c::updateCursorPos() {
    int word = mNum / WORD_DIGIT_NUM;
    int col = mNum % WORD_DIGIT_NUM;
    if (col == 0 && word > 0) {
        col = WORD_DIGIT_NUM;
        word--;
    }
    mVec3_c pos;
    m2d::getScreenPos(&pos, mWord[word].mpTextBox);
    pos.x = mWord[word].getScreenX(col);
    mCur.setPos(&pos);
}

// .text:0x1F70
BOOL dMenuFriend08_c::hitCheckWords() {
    if (!mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        return FALSE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    for (int i = 0; i < WORD_NUM; i++) {
        if (m2d::isInPaneRect(mWord[i].mpTextBox, 0.0f, &pos)) {
            int col = mWord[i].getPosAtScreenX(pos.x);
            mNum = col + i * WORD_DIGIT_NUM;
            while (mNum > 0 && mDigit[mNum - 1] == DIGIT_EMPTY) {
                mNum--;
            }
            updateCursorPos();
            playSeCursor();
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x2080
void dMenuFriend08_c::moveLeft() {
    if (mNum > 0) {
        mNum--;
        playSeCursor();
    }
    updateCursorPos();
}

// .text:0x20C8
void dMenuFriend08_c::moveRight() {
    if (mNum < DIGIT_NUM && mDigit[mNum] != DIGIT_EMPTY) {
        mNum++;
        playSeCursor();
    }
    updateCursorPos();
}

// .text:0x2124
void dMenuFriend08_c::moveHome() {
    if (mNum != 0) {
        mNum = 0;
        playSeCursor();
    }
    updateCursorPos();
}

// .text:0x216C
void dMenuFriend08_c::moveEnd() {
    int i = 0;
    int prev = mNum;
    for (; i < DIGIT_NUM; i++) {
        if (mDigit[i] == DIGIT_EMPTY) {
            break;
        }
        mNum = i + 1;
    }
    if (mNum != prev) {
        playSeCursor();
    }
    updateCursorPos();
}

// .text:0x2244
BOOL dMenuFriend08_c::confirmKey() {
    mKey = 0;
    for (int i = 0; i < DIGIT_NUM; i++) {
        if (mDigit[i] == DIGIT_EMPTY) {
            return FALSE;
        }
        mKey *= 10;
        mKey += mDigit[i];
    }
    u64 myKey = dWifi::getMyFriendKey();
    if (myKey != 0 && myKey == mKey) {
        openMessage(MSG_OWN_CODE);
        playSeCanNot();
        return FALSE;
    }
    if (dNet::isNetMgrRunning()) {
        setSubState(SUB_CHECK_KEY);
        return FALSE;
    }
    if (!dNetMgr_c::checkFriendKey(
            &dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.mUserData, mKey)) {
        openMessage(MSG_WRONG_CODE);
        playSeError();
        return FALSE;
    }
    if (!dMenuFriend_c::setEntryKey(mKey)) {
        openMessage(MSG_ALREADY_REGISTERED);
        playSeCanNot();
        return FALSE;
    }
    return TRUE;
}

// .text:0x243C
void dMenuFriend08_c::openMessage(u16 msg) {
    mSlct.setBtnVisible(SLCT_BTN_LEFT, false);
    mSlct.setBtnVisible(SLCT_BTN_MIDDLE, false);
    mSlct.setBtn(SLCT_BTN_RIGHT, MSG_OK, nullptr, 0);
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
    setSubState(SUB_MESSAGE);
    onFlag(FLAG_SLCT);
    mSlct.setMsg(msg, nullptr);
}

// .text:0x24F0
void dMenuFriend08_c::startCloseAnm() {
    startAnm(&mBtn.mLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
    setWinAnm(ANM_UPPER_WIN_OUT);
}

// .text:0x2540
void dMenuFriend08_c::setWinAnm(int anm) {
    m2d::ResAccIf_c *resAcc = dMenuMng_c::getWinResAcc();
    mLyt.setAnm(&mWinAnm, getAnmName(anm), resAcc);
}

// .text:0x25A0
void dMenuFriend08_c::finish(BOOL fromSession) {
    dMenuMng_c::setResult(1);
    if (fromSession || mMenuNo == dMenuMng_c::MENU_FRIEND_CODE_EDIT) {
        onFlag(FLAG_REGISTER);
        startCloseAnm();
        dMenuFriend_c::requestShowTabs();
    } else {
        startCloseAnm();
        dMenuFriend_c::requestOpenMenu(
            dMenuFriend_c::OPEN_MENU_NEW_FRIEND_NAME);
    }
}

// .text:0x261C
void dMenuFriend08_c::playSeError() {
    playSe(SE_CanNotSet);
}
