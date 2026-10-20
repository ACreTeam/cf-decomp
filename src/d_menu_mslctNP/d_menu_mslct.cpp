#include <game/game/d_menu_mslct.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_activity_log.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuMslct {
// .text:0x5C
BASE_PROFILE(MENU_MSLCT, dMenuMslct_c)
} // namespace dMenuMslct

static const char *l_allPane = "N_all";

// .text:0x25C
int dMenuMslct_c::create() {
    BOOL loaded = TRUE;
    loaded &= mBtn.load();
    loaded &= mWin.load();
    if (!loaded) {
        return NOT_READY;
    }
    mBtn.create();
    initButtons();
    mWin.create();
    onFlag(FLAG_DRAW);
    startAnm(&mBtn.mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    mWin.startWindowAnm(dMenuLtr::Window_c::WIN_ANM_IN, false);
    setSubState(SUB_MAIN);
    mNoChoiceMask = 0;
    mPickedMask = 0;
    mSingle = CURSOR_NONE;
    initLetters();
    dMenuMng_c::clearSelMask();
    return SUCCEEDED;
}

// .text:0x340
int dMenuMslct_c::doDelete() {
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    return mWin.remove() != 0;
}

// .text:0x394
int dMenuMslct_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x3B8
int dMenuMslct_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x3BC
int dMenuMslct_c::stateClose() {
    if (isFlag(FLAG_SENDING)) {
        return SUCCEEDED;
    }
    if (dMenuMng_c::isSelMailTaken()) {
        dActivityLog::onHeartLetter(dNet::getMyNo());
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x41C
int dMenuMslct_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prevCursor) {
        playSeCursor();
    }
    mBtn.execute();
    mWin.playWindowAnm();
    mWin.calcRotate();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
    }
    updateSending();
    return result;
}

// .text:0x4BC
int dMenuMslct_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mBtn.draw();
        mWin.drawWindow();
        mWin.drawIcons();
    }
    return SUCCEEDED;
}

// .text:0x510
void dMenuMslct_c::subStateMain() {
    setSelect(mCursor, FALSE);
    hideTutorial();
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != CURSOR_NONE) {
        mCursor = btn + CURSOR_BTN_FIRST;
        setSelect(mCursor, TRUE);
        push();
        return;
    }
    mCursor = hitCheck();
    if (isLetter(mCursor)) {
        if (isEmptyLetter(mCursor) || isNoChoice(mCursor)) {
            mCursor = CURSOR_NONE;
        }
    }
    if (mCursor != CURSOR_NONE) {
        setSelect(mCursor, TRUE);
        showTutorial(mCursor);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push();
        }
    }
}

// .text:0x630
void dMenuMslct_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x66C
void dMenuMslct_c::executeSubState() {
    typedef void (dMenuMslct_c::*Proc)();
    static Proc subStateProcs[] = {&dMenuMslct_c::subStateMain,
                                   &dMenuMslct_c::subStatePush};
    (this->*subStateProcs[mSubState])();
}

// .text:0x6A4
void dMenuMslct_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
    setOkDisable(TRUE);
}

// .text:0x710
void dMenuMslct_c::setOkDisable(int disable) {
    mBtn.setDisable(BTN_CONFIRM, disable != 0);
}

// .text:0x72C
void dMenuMslct_c::setSelect(int slot, BOOL select) {
    if (isBtn(slot)) {
        mBtn.setSelect(slot - CURSOR_BTN_FIRST, select, FALSE);
    }
    if (isLetter(slot)) {
        mWin.setMark(slot, select != 0);
    }
}

// .text:0x7D8
int dMenuMslct_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != CURSOR_NONE) {
        return btn + CURSOR_BTN_FIRST;
    }
    mVec2_c p = pos;
    int letter = mWin.hitLetter(&p, false, 1.0f);
    if (letter != CURSOR_NONE) {
        return letter;
    }
    return CURSOR_NONE;
}

// .text:0x870
void dMenuMslct_c::push() {
    int slot = mCursor;
    if (isBtn(slot)) {
        mBtn.push(slot - CURSOR_BTN_FIRST, FALSE);
    } else if (isLetter(slot)) {
        if (!isNoChoice(slot)) {
            if (isFlag(FLAG_SINGLE)) {
                if (mSingle == slot) {
                    setPicked(mSingle, FALSE);
                    mSingle = CURSOR_NONE;
                    playSeCancel2();
                } else {
                    if (mSingle != CURSOR_NONE) {
                        setPicked(mSingle, FALSE);
                    }
                    mSingle = slot;
                    setPicked(slot, TRUE);
                    playSeDecide2();
                }
            } else {
                setPicked(slot, !isPicked(slot));
                if (isPicked(slot)) {
                    playSeDecide2();
                } else {
                    playSeCancel2();
                }
            }
        }
        return;
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x9D8
bool dMenuMslct_c::isPushEnd() {
    int slot = mCursor;
    if (isBtn(slot)) {
        return !mBtn.isPushing();
    }
    return true;
}

// .text:0xA30
void dMenuMslct_c::finishPush() {
    int slot = mCursor;
    if (isBtn(slot)) {
        switch (slot - CURSOR_BTN_FIRST) {
        case BTN_QUIT:
            startCloseAnm();
            break;
        case BTN_CONFIRM:
            startCloseAnm();
            sendLetters();
            break;
        }
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xADC
void dMenuMslct_c::startCloseAnm() {
    startAnm(&mBtn.mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    mWin.startWindowAnm(dMenuLtr::Window_c::WIN_ANM_OUT, false);
    dMenuMng_c::setResult(0);
}

// .text:0xB38
bool dMenuMslct_c::isNoChoice(int slot) {
    u16 bit = 1 << slot;
    return (mNoChoiceMask & bit) != 0;
}

// .text:0xB5C
void dMenuMslct_c::setNoChoice(int slot, BOOL noChoice) {
    u16 bit = 1 << slot;
    mWin.setNoChoice(slot, noChoice);
    if (noChoice) {
        mNoChoiceMask |= bit;
    } else {
        mNoChoiceMask &= ~bit;
    }
}

// .text:0xBDC
void dMenuMslct_c::initLetters() {
    for (int i = 0; i < dMenuLtr::Window_c::LETTER_COUNT; i++) {
        bool noChoice = true;
        dMail_c *mail = getMail(i);
        if (mail->isEmpty()) {
            noChoice = false;
        } else if (!mail->isWritten()) {
            noChoice = true;
        } else {
            switch (mMenuNo) {
            case dMenuMng_c::MENU_POST_TOWN:
                noChoice = false;
                if (mail->isToOtherTown() || mail->isToFutureSelf()) {
                    noChoice = true;
                }
                break;
            case dMenuMng_c::MENU_POST_OTHER_TOWN:
                noChoice = !mail->isToOtherTown();
                break;
            case dMenuMng_c::MENU_POST_FUTURE:
                noChoice = !mail->isToFutureSelf();
                break;
            }
        }
        setNoChoice(i, noChoice);
    }
    if (mMenuNo == dMenuMng_c::MENU_POST_FUTURE) {
        onFlag(FLAG_SINGLE);
    }
}

// .text:0xD0C
bool dMenuMslct_c::isPicked(int slot) {
    u16 bit = 1 << slot;
    return (mPickedMask & bit) != 0;
}

// .text:0xD30
void dMenuMslct_c::setPicked(int slot, BOOL picked) {
    u16 bit = 1 << slot;
    mWin.setSelected(slot, picked);
    if (picked) {
        mPickedMask |= bit;
    } else {
        mPickedMask &= ~bit;
    }
    setOkDisable(mPickedMask == 0);
}

// .text:0xDC4
void dMenuMslct_c::showTutorial(int slot) {
    if (isLetter(slot) && !isEmptyLetter(slot)) {
        if (dObjTutorial_c::request(dObjTutorial_c::REQUEST_MAIL_LETTER, slot,
                                    FALSE)) {
            dMail_c *mail = getMail(slot);
            m2d::stringWord_c word;
            mail->getListWord(&word);
            mVec2_c pos = mWin.getLetterPos(slot, 0.0f, 0.0f);
            dObjTutorial_c::setWord(&pos, &word, 0);
        }
    } else {
        hideTutorial();
    }
}

// .text:0xEB4
void dMenuMslct_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0xEB8
dMail_c *dMenuMslct_c::getMail(int slot) {
    return &dPlayerMgr::Mgr_c::getCurrentPlayer()->mLetters[slot];
}

// .text:0xEF0
BOOL dMenuMslct_c::isEmptyLetter(int slot) {
    if (isLetter(slot)) {
        return dPlayerMgr::Mgr_c::getCurrentPlayer()->mLetters[slot].isEmpty();
    }
    return TRUE;
}

// .text:0xF54
void dMenuMslct_c::clearLetter(int slot) {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->clearLetter(slot);
}

// .text:0xF88
int dMenuMslct_c::countPicked() {
    int num = 0;
    for (int i = 0; i < dMenuLtr::Window_c::LETTER_COUNT; i++) {
        if (!isNoChoice(i) && isPicked(i) && !isEmptyLetter(i)) {
            num++;
        }
    }
    return num;
}

// .text:0x1018
void dMenuMslct_c::sendLetters() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_POST_TOWN:
        if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
            if (dNet::isHost()) {
                sendTownHost();
            } else {
                sendTownGuest();
            }
        } else {
            sendTown();
        }
        break;
    case dMenuMng_c::MENU_POST_OTHER_TOWN:
        if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
            sendOtherTownNet();
        } else {
            sendOtherTownSolo();
        }
        break;
    case dMenuMng_c::MENU_POST_FUTURE:
        sendFuture();
        break;
    }
}

// .text:0x10E4
void dMenuMslct_c::setResultOk() {
    dMenuMng_c::setResult(1);
}

// .text:0x10EC
void dMenuMslct_c::sendTown() {
    setResultOk();
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        deliverToAnimals();
    }
    checkRecipients(FALSE);
    addToPostOffice();
    if (countPicked()) {
        dPostOffice::deliverAll();
        addToPostOffice();
        if (countPicked()) {
            deliverToPlayers();
            if (countPicked() > 0) {
                dMenuMng_c::onSelPostFull();
            }
        }
    }
}

// .text:0x1190
void dMenuMslct_c::sendTownHost() {
    setResultOk();
    checkRecipients(FALSE);
    addToPostOffice();
    if (countPicked()) {
        dMenuMng_c::onSelPostFull();
    }
}

// .text:0x11E4
void dMenuMslct_c::sendTownGuest() {
    setResultOk();
    checkRecipients(FALSE);
    if (countPicked()) {
        sendFirst();
        onFlag(FLAG_SENDING);
    }
}

// .text:0x1240
void dMenuMslct_c::sendOtherTownSolo() {
    setResultOk();
    checkRecipients(TRUE);
    addToPostOffice();
    if (countPicked()) {
        dPostOffice::deliverAll();
        addToPostOffice();
        if (countPicked()) {
            dMenuMng_c::onSelPostFull();
        }
    }
}

// .text:0x12B0
void dMenuMslct_c::sendOtherTownNet() {
    setResultOk();
    checkRecipients(TRUE);
    addToPostOffice();
    if (countPicked()) {
        dMenuMng_c::onSelPostFull();
    }
}

// .text:0x1304
void dMenuMslct_c::sendFuture() {
    if (mSingle != CURSOR_NONE) {
        dMenuMng_c::setResult(1);
        dMenuMng_c::setMail(getMail(mSingle));
        dMenuMng_c::setSelSlot(mSingle);
    }
}

// .text:0x135C
void dMenuMslct_c::addToPostOffice() {
    for (int i = 0; i < dMenuLtr::Window_c::LETTER_COUNT; i++) {
        if (!isNoChoice(i) && isPicked(i) && !isEmptyLetter(i)) {
            dMail_c *mail = getMail(i);
            if (!dPostOffice::isFull()) {
                dPostOffice::add(mail);
                clearLetter(i);
                dMenuMng_c::onSelMailTaken();
            }
        }
    }
}

// .text:0x1414
void dMenuMslct_c::deliverToPlayers() {
    for (int i = 0; i < dMenuLtr::Window_c::LETTER_COUNT; i++) {
        if (!isNoChoice(i) && isPicked(i) && !isEmptyLetter(i)) {
            dMail_c *mail = getMail(i);
            if (!dPostOffice::deliver(mail)) {
                int player = dPostOffice::getToPlayerIdx(mail);
                if (player >= 0) {
                    dMenuMng_c::onSelSlot(player);
                }
            } else {
                clearLetter(i);
                dMenuMng_c::onSelMailTaken();
            }
        }
    }
}

// .text:0x14DC
void dMenuMslct_c::deliverToAnimals() {
    for (int i = 0; i < dMenuLtr::Window_c::LETTER_COUNT; i++) {
        if (!isNoChoice(i) && isPicked(i) && !isEmptyLetter(i)) {
            dMail_c *mail = getMail(i);
            if (dPostOffice::getToAnimalIdx(mail) >= 0) {
                dPostOffice::deliverToAnimal(mail);
                clearLetter(i);
                dMenuMng_c::onSelMailTaken();
            }
        }
    }
}

// .text:0x1594
void dMenuMslct_c::checkRecipients(BOOL discardUnknown) {
    for (int i = 0; i < dMenuLtr::Window_c::LETTER_COUNT; i++) {
        if (!isNoChoice(i) && isPicked(i) && !isEmptyLetter(i)) {
            dMail_c *mail = getMail(i);
            int player = dPostOffice::getToPlayerIdx(mail);
            if (player == -3) {
                dMenuMng_c::onSelNotResident();
                setNoChoice(i, TRUE);
            } else if (player == -2) {
                if (dPostOffice::getFriendIdx(mail) < 0) {
                    dMenuMng_c::onSelNotFriend();
                    setNoChoice(i, TRUE);
                }
            } else if (player == -1) {
                if (dPostOffice::getToAnimalIdx(mail) < 0) {
                    dMenuMng_c::onSelNotResident();
                    if (discardUnknown) {
                        clearLetter(i);
                        dMenuMng_c::onSelMailTaken();
                    } else {
                        setNoChoice(i, TRUE);
                    }
                }
            }
        }
    }
}

// .text:0x16D0
void dMenuMslct_c::sendFirst() {
    mSendLetter = CURSOR_NONE;
    for (int i = 0; i < dMenuLtr::Window_c::LETTER_COUNT; i++) {
        if (!isNoChoice(i) && isPicked(i) && !isEmptyLetter(i)) {
            dMail_c *mail = getMail(i);
            mSendLetter = i;
            dPostOffice::sendMail(mail);
            break;
        }
    }
}

// .text:0x176C
void dMenuMslct_c::updateSending() {
    if (isFlag(FLAG_SENDING)) {
        switch (dPostOffice::getSyncState()) {
        case dPostOffice::MAIL_SYNC_SENT:
            return;
        case dPostOffice::MAIL_SYNC_ACCEPTED:
            clearLetter(mSendLetter);
            dMenuMng_c::onSelMailTaken();
            if (countPicked() > 0) {
                sendFirst();
            } else {
                offFlag(FLAG_SENDING);
            }
            break;
        case dPostOffice::MAIL_SYNC_ACCEPTED_FULL:
            clearLetter(mSendLetter);
            dMenuMng_c::onSelMailTaken();
            offFlag(FLAG_SENDING);
            if (countPicked() > 0) {
                dMenuMng_c::onSelPostFull();
            }
            break;
        case dPostOffice::MAIL_SYNC_REJECTED:
            offFlag(FLAG_SENDING);
            dMenuMng_c::onSelPostFull();
            break;
        default:
            break;
        }
    }
}
