#include <game/game/d_menu_post01.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_box.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/SC/scapi.h>

namespace dMenuPost01 {
// .text:0xCC
BASE_PROFILE(MENU_POST01, dMenuPost01_c)
} // namespace dMenuPost01

static const char *l_allPane = "N_all";

// .text:0x23C
int dMenuPost01_c::create() {
    BOOL loaded = mRes.load("Layout/post.arc");
    loaded &= mLtrWin.load();
    loaded &= mSelect.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "post_win_00.brlyt");
    mLyt.setVisible("N_sldCrsr_all", true);
    mLyt.setVisible("N_pstLtrIcn_00", false);
    mLyt.setMessage("T_btn_00", MSG_CLOSE, NULL);
    initAnm();
    if (mMenuNo == dMenuMng_c::MENU_POST_LETTERS_AFTER_READ) {
        setPage(getPostFlags() & POST_PAGE_MASK);
        if (getPostFlags() & POST_CHANGED) {
            onFlag(FLAG_CHANGED);
        }
    } else {
        setPage(0);
    }
    mLyt.calc();
    mLtrWin.create();
    initIcons();
    mSelect.build();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_IN, STATE_MAIN, TRUE);
    mLtrWin.startWindowAnm(dMenuLtr::Window_c::WIN_ANM_POST_IN, false);
    playSeSubMenuOpen();
    setSubState(SUB_MAIN);
    mCursor = SLOT_NONE;
    return SUCCEEDED;
}

// .text:0x3D4
int dMenuPost01_c::doDelete() {
    if (mLtrWin.remove() == FALSE) {
        return NOT_READY;
    }
    if (mSelect.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes.remove() != 0;
}

// .text:0x440
int dMenuPost01_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x464
int dMenuPost01_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    calcLayouts();
    return result;
}

// .text:0x4A8
int dMenuPost01_c::stateClose() {
    if (isFlag(FLAG_PAGE_TURN) && isPageButton(mCursor)) {
        setPage(mNextPage);
        startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_IN, STATE_MAIN, TRUE);
        initMails();
        offFlag(FLAG_PAGE_TURN);
        return SUCCEEDED;
    }
    if (isFlag(FLAG_OPEN_LETTER) &&
        dAcUitag::openMenuB(dAcUitag::MENU_B_READ_9) == FALSE) {
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x584
int dMenuPost01_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != SLOT_NONE && prevCursor != mCursor) {
        playSeCursor();
    }
    calcAnm();
    calcHeld();
    mLtrWin.playWindowAnm();
    mSelect.update();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
    }
    mScrollAnm.play();
    mLyt.calc();
    return result;
}

// .text:0x640
int dMenuPost01_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLtrWin.drawWindow();
        mLtrWin.drawIcons();
        mLyt.entry();
        drawIcons();
        if (isFlag(FLAG_FOLLOW) || isFlag(FLAG_MOVE)) {
            mLtrWin.drawHeldLetter();
        }
        mSelect.draw();
    }
    return SUCCEEDED;
}

// .text:0x6D4
void dMenuPost01_c::subMain() {
    hideTutorial();
    setHighlight(mCursor, FALSE);
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        mCursor = SLOT_BACK;
        pressButton();
        return;
    }
    mCursor = hitCheck();
    if (mCursor != SLOT_NONE) {
        setHighlight(mCursor, TRUE);
        showSlotTutorial(mCursor);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            pressButton();
            return;
        }
    }
    checkPadMove();
}

// .text:0x7B4
void dMenuPost01_c::subPush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x7F0
void dMenuPost01_c::subSelect() {
    if (static_cast<dSelect::BComp_c &>(mSelect).isClosed()) {
        mAnswer = mSelect.mCode;
        onSelectEnd();
    }
}

// .text:0x844
void dMenuPost01_c::subPress() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        openSelect(mCursor);
        return;
    }
    int hit = hitCheck();
    if (hit == mCursor && mHoldCount < dMenuMove_c::getHoldFrames()) {
        mHoldCount++;
        return;
    }
    if (pickUp()) {
        setSubState(SUB_DRAG);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x914
void dMenuPost01_c::subDrag() {
    hideTutorial();
    setHighlight(mCursor, FALSE);
    mCursor = hitCheckDrop();
    if (mCursor != SLOT_NONE) {
        setHighlight(mCursor, TRUE);
        showSlotTutorial(mCursor);
    }
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A) ||
        !dObjCursor_c::isVisible()) {
        dropHeld();
    }
}

// .text:0x9B8
void dMenuPost01_c::subMove() {
    if (mLtrWin.calcMove()) {
        dMail_c mail(*mLtrWin.getHeldMail());
        swapMail(mHeldFrom, &mail, TRUE);
        mLtrWin.clearHeldMail();
        playSeItemDrop();
        offFlag(FLAG_MOVE);
        setSubState(SUB_MAIN);
    }
}

// .text:0xB4C
void dMenuPost01_c::subToss() {
    BOOL done = TRUE;
    if (isLetter(mSelectSlot)) {
        done = mLtrWin.calcVanish(mSelectSlot);
    }
    if (isBox(mSelectSlot)) {
        done = mIcons[mSelectSlot - SLOT_BOX].calcVanish();
    }
    if (done) {
        removeMail(mSelectSlot, NULL);
        setSubState(SUB_MAIN);
    }
}

// .text:0xC14
void dMenuPost01_c::executeSubState() {
    static void (dMenuPost01_c::*l_sub[SUB_NUM])() = {
        &dMenuPost01_c::subMain,   &dMenuPost01_c::subPush,
        &dMenuPost01_c::subSelect, &dMenuPost01_c::subPress,
        &dMenuPost01_c::subDrag,   &dMenuPost01_c::subMove,
        &dMenuPost01_c::subToss};
    (this->*l_sub[mSubState])();
}

// .text:0xC4C
void dMenuPost01_c::setHighlight(int slot, BOOL on) {
    if (isPageButton(slot)) {
        int anmNo;
        if (on) {
            anmNo = BTN_ANM_IN;
        } else {
            anmNo = BTN_ANM_OUT;
        }
        m2d::AnmEx_c *anm = &mPageAnm[slot - SLOT_PREV_PAGE];
        anm->start(anmNo);
        anm->setToEnd();
        onFlag(FLAG_CALC);
    } else if (slot == SLOT_BACK) {
        mBackAnm.start(on == 0);
        mBackAnm.setToEnd();
        onFlag(FLAG_CALC);
    } else if (isBox(slot)) {
        dMenuLtr::Icon_c *icon = &mIcons[slot - SLOT_BOX];
        icon->setMark(on != 0);
        if (on) {
            icon->calcLayout();
        }
    } else if (isLetter(slot)) {
        mLtrWin.setMark(slot, on != 0);
    }
}

// .text:0xD9C
int dMenuPost01_c::hitSlot(bool setOffset) {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int slot =
        m2d::hitButton(&mVec2_c(pos), &mLyt, SLOT_BOX, SLOT_BACK, -1, -1, 1.0f);
    if (slot != -1) {
        if (setOffset && isBox(slot)) {
            mVec2_c ofs = getSlotPos(slot, 0.0f, 0.0f);
            ofs -= pos;
            mLtrWin.setMoveOffset(ofs);
        }
        return slot;
    }
    slot = mLtrWin.hitLetter(&mVec2_c(pos), setOffset, 1.0f);
    if (slot != -1) {
        return slot;
    }
    return SLOT_NONE;
}

// .text:0xEE4
int dMenuPost01_c::hitCheck() {
    int slot = hitSlot(true);
    if (isLetter(slot) || isBox(slot)) {
        if (isSlotEmpty(slot)) {
            return SLOT_NONE;
        }
    }
    return slot;
}

// .text:0xF80
int dMenuPost01_c::hitCheckDrop() {
    int slot = hitSlot(false);
    if (isLetter(slot) || isBox(slot)) {
        if (isLocked(slot)) {
            return SLOT_NONE;
        }
        return slot;
    }
    return SLOT_NONE;
}

// .text:0x1024
void dMenuPost01_c::checkPadMove() {
    if (dPad::isUpTriggered()) {
        setHighlight(mCursor, FALSE);
        mCursor = SLOT_PREV_PAGE;
        pressButton();
    } else if (dPad::isDownTriggered()) {
        setHighlight(mCursor, FALSE);
        mCursor = SLOT_NEXT_PAGE;
        pressButton();
    }
}

// .text:0x10A8
void dMenuPost01_c::pressButton() {
    if (isButtonEnabled(mCursor)) {
        if (isPageButton(mCursor)) {
            mPageAnm[mCursor - SLOT_PREV_PAGE].setNext(BTN_ANM_PUSH);
        } else if (mCursor == SLOT_BACK) {
            mBackAnm.setNext(BTN_ANM_PUSH);
            playSeCancel();
        } else if (isLetter(mCursor) || isBox(mCursor)) {
            setSubState(SUB_PRESS);
            mHoldCount = 0;
            return;
        } else {
            return;
        }
        setSubState(SUB_PUSH);
    }
}

// .text:0x11BC
BOOL dMenuPost01_c::isPushEnd() {
    if (isPageButton(mCursor)) {
        return mPageAnm[mCursor - SLOT_PREV_PAGE].isEnd();
    } else if (mCursor == SLOT_BACK) {
        return mBackAnm.isEnd();
    }
    return TRUE;
}

// .text:0x120C
void dMenuPost01_c::finishPush() {
    if (isPageButton(mCursor)) {
        if (mCursor == SLOT_NEXT_PAGE) {
            mNextPage = mPage + 1;
            if (mNextPage >= PAGE_NUM) {
                mNextPage = 0;
            }
        } else {
            mNextPage = mPage - 1;
            if (mNextPage < 0) {
                mNextPage = PAGE_NUM - 1;
            }
        }
        playSePageTurnNo(mNextPage);
        startPageTurn();
    } else if (mCursor == SLOT_BACK) {
        startClose();
        if (isFlag(FLAG_CHANGED)) {
            dMenuMng_c::setResult(dMenuMng_c::RESULT_OK);
        }
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x12F4
void dMenuPost01_c::startClose() {
    offFlag(FLAG_PAGE_TURN);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_OUT, STATE_CLOSE, TRUE);
    mLtrWin.startWindowAnm(dMenuLtr::Window_c::WIN_ANM_POST_OUT, false);
    playSeSubMenuClose();
    dMenuMng_c::setResult(dMenuMng_c::RESULT_CANCEL);
    hideTutorial();
}

// .text:0x1368
void dMenuPost01_c::readLetter() {
    dMenuMng_c::setMail(getMail(mSelectSlot));
    markRead(mSelectSlot);
    startClose();
    onFlag(FLAG_OPEN_LETTER);
    setPostFlags(mPage | (isFlag(FLAG_CHANGED) ? POST_CHANGED : 0));
}

// .text:0x13E8
void dMenuPost01_c::startPageTurn() {
    onFlag(FLAG_PAGE_TURN);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_OUT, STATE_CLOSE, TRUE);
    hideTutorial();
    setSubState(SUB_MAIN);
}

// .text:0x1458
void dMenuPost01_c::setPage(int page) {
    mPage = page;
    dString::Word_c word;
    dScript::formatNumber(&word, mPage + 1, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(0, &word);
    dScript::formatNumber(&word, PAGE_NUM, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(1, &word);
    mLyt.setMessage("T_page_00", MSG_PAGE_NUMBER, NULL);
    updatePageButtons();
}

// .text:0x1504
void dMenuPost01_c::initAnm() {
    static const char *l_prevAnm[] = {"post_win_00_sldCrsr_00_In.brlan",
                                      "post_win_00_sldCrsr_00_Out.brlan",
                                      "post_win_00_sldCrsr_00_Psh.brlan",
                                      "post_win_00_sldCrsr_00_CanPsh.brlan",
                                      "post_win_00_sldCrsr_00_NoPsh.brlan",
                                      NULL};
    static const char *l_nextAnm[] = {"post_win_00_sldCrsr_01_In.brlan",
                                      "post_win_00_sldCrsr_01_Out.brlan",
                                      "post_win_00_sldCrsr_01_Psh.brlan",
                                      "post_win_00_sldCrsr_01_CanPsh.brlan",
                                      "post_win_00_sldCrsr_01_NoPsh.brlan"};
    static const char *l_backAnm[] = {"post_win_00_squareBtn_In.brlan",
                                      "post_win_00_squareBtn_Out.brlan",
                                      "post_win_00_squareBtn_Psh.brlan"};
    mPageAnm[0].init(&mLyt, &mRes, l_prevAnm, PAGE_ANM_NUM);
    mPageAnm[0].start(BTN_ANM_OUT);
    mPageAnm[0].setToEnd();
    mPageAnm[1].init(&mLyt, &mRes, l_nextAnm, PAGE_ANM_NUM);
    mPageAnm[1].start(BTN_ANM_OUT);
    mPageAnm[1].setToEnd();
    mBackAnm.init(&mLyt, &mRes, l_backAnm, BACK_ANM_NUM);
    mBackAnm.start(BTN_ANM_OUT);
    mBackAnm.setToEnd();
    mLyt.setAnm(&mScrollAnm, "post_win_00_scrlIcon_00.brlan", NULL);
}

// .text:0x15D4
void dMenuPost01_c::calcAnm() {
    int playing = 0;
    for (int i = 0; i < PAGE_BTN_NUM; i++) {
        playing |= mPageAnm[i].play();
    }
    playing |= mBackAnm.play();
    if (playing) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x1658
BOOL dMenuPost01_c::isButtonEnabled(int slot) {
    switch (slot) {
    case SLOT_PREV_PAGE:
        return isPrevPageEnabled();
    case SLOT_NEXT_PAGE:
        return isNextPageEnabled();
    }
    return TRUE;
}

// .text:0x1680
BOOL dMenuPost01_c::isPrevPageEnabled() {
    return TRUE;
}

// .text:0x1688
BOOL dMenuPost01_c::isNextPageEnabled() {
    return TRUE;
}

// .text:0x1690
void dMenuPost01_c::updatePageButtons() {
    mPageAnm[0].start(isPrevPageEnabled() ? BTN_ANM_ENABLED : BTN_ANM_DISABLED);
    mPageAnm[0].setToEnd();
    mPageAnm[1].start(isNextPageEnabled() ? BTN_ANM_ENABLED : BTN_ANM_DISABLED);
    mPageAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x1710
void dMenuPost01_c::initIcons() {
    for (int i = 0; i < BOX_NUM; i++) {
        mIcons[i].create(mLtrWin.getRes());
    }
    initMails();
    unlockAll();
}

// .text:0x1784
int dMenuPost01_c::positionIcon(nw4r::lyt::Pane *pane, void *arg) {
    int index = m2d::getNameNumber(pane->GetName(), 2);
    if (index >= 0 && index < BOX_NUM) {
        mVec3_c pos;
        m2d::getScreenPos(&pos, pane);
        ((dMenuLtr::Icon_c *)arg)[index].setRootPos(&pos);
    }
    return 0;
}

// .text:0x1800
void dMenuPost01_c::calcLayouts() {
    mLyt.patrolPane(positionIcon, m2d::isRectButton, mIcons);
    for (int i = 0; i < BOX_NUM; i++) {
        mIcons[i].calcLayout();
    }
}

// .text:0x186C
void dMenuPost01_c::drawIcons() {
    for (int i = 0; i < BOX_NUM; i++) {
        mIcons[i].drawLayout();
    }
}

// .text:0x18B8
void dMenuPost01_c::initMails() {
    for (int i = 0; i < BOX_NUM; i++) {
        mIcons[i].setMail(getMail(SLOT_BOX + i), -1);
    }
}

// .text:0x1924
const mVec2_c &dMenuPost01_c::getSlotPos(int slot, float dx, float dy) {
    static mVec2_c result(0.0f, 0.0f);
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        dx *= 0.730769f;
    }
    if (isBox(slot)) {
        mVec3_c pos;
        pos = mIcons[slot - SLOT_BOX].getRootPos();
        result.x = pos.x + dx;
        result.y = pos.y + dy;
        return result;
    } else if (isLetter(slot)) {
        return mLtrWin.getLetterPos(slot, dx, dy);
    }
    return result;
}

// .text:0x1A90
dMail_c *dMenuPost01_c::getMail(int slot) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (isBox(slot)) {
        return player->getMailBox()->get(mPage, slot - SLOT_BOX);
    } else if (isLetter(slot)) {
        return &player->mLetters[slot];
    }
    return NULL;
}

// .text:0x1B30
// Puts *mail into the slot and the letter it held into *mail.
void dMenuPost01_c::swapMail(int slot, dMail_c *mail, BOOL updateIcon) {
    dMail_c old;
    old.clear();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (isBox(slot)) {
        int index = slot - SLOT_BOX;
        dSaveMailBox_c *box = player->getMailBox();
        old.copy(box->get(mPage, index));
        box->set(mPage, index, mail);
        if (updateIcon) {
            mIcons[index].setMail(mail, -1);
        }
    } else if (isLetter(slot)) {
        old.copy(&player->mLetters[slot]);
        player->setLetter(mail, slot);
        if (updateIcon) {
            mLtrWin.setMail(slot, mail, -1);
        }
    }
    mail->copy(&old);
}

// .text:0x1C7C
void dMenuPost01_c::removeMail(int slot, dMail_c *removed) {
    dMail_c mail;
    mail.clear();
    swapMail(slot, &mail, TRUE);
    if (removed) {
        removed->copy(&mail);
    }
}

// .text:0x1D00
BOOL dMenuPost01_c::isSlotEmpty(int slot) {
    dMail_c *mail = getMail(slot);
    if (mail == NULL) {
        return TRUE;
    }
    return mail->isEmpty();
}

// .text:0x1D34
void dMenuPost01_c::markRead(int slot) {
    dMail_c *src = getMail(slot);
    if (src) {
        dMail_c mail(*src);
        mail.markRead();
        swapMail(slot, &mail, FALSE);
    }
}

// .text:0x1EA0
BOOL dMenuPost01_c::isLocked(int slot) {
    if (isLetter(slot)) {
        return (mLockMask & (u16)(1 << slot)) != 0;
    }
    return FALSE;
}

// .text:0x1EEC
void dMenuPost01_c::unlockAll() {
    mLockMask = 0;
}

// .text:0x1EF8
void dMenuPost01_c::showSlotTutorial(int slot) {
    int tail = 0;
    dMail_c *mail = getMail(slot);
    if (mail == NULL || mail->isEmpty()) {
        hideTutorial();
    } else if (dObjTutorial_c::request(dObjTutorial_c::REQUEST_MAILBOX_LETTER,
                                       slot, FALSE)) {
        if (isLetter(slot) && slot >= LETTERS_PER_ROW) {
            tail = 1;
        }
        m2d::stringWord_c word;
        mail->getListWord(&word);
        mVec2_c pos = getSlotPos(slot, 0.0f, 0.0f);
        dObjTutorial_c::setWord(&pos, &word, tail);
    }
}

// .text:0x2004
void dMenuPost01_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x2008
void dMenuPost01_c::openSelect(int slot) {
    mSelectSlot = slot;
    buildSelect(slot);
    mVec2_c pos = getSlotPos(slot, 10.0f, -50.0f);
    mSelect.placeAbove(pos);
    mSelect.openPointer();
    setSubState(SUB_SELECT);
}

// .text:0x21E4
void dMenuPost01_c::buildSelect(int slot) {
    mSelect.reset();
    mSelect.mNum = 0;
    mSelect.addItem(MSG_READ, ANSWER_READ, 0, 0);
    dMail_c *mail = getMail(slot);
    if (mail->getPresent() == dItem::ITEM_ID_NONE &&
        mail->getStatus() != MAIL_STATUS_UNREAD) {
        mSelect.addItem(MSG_TOSS, ANSWER_TOSS, 0, 3);
    }
    mSelect.addItem(MSG_QUIT, ANSWER_QUIT, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x22B8
void dMenuPost01_c::onSelectEnd() {
    switch (mAnswer) {
    case ANSWER_READ:
        readLetter();
        break;
    case ANSWER_TOSS:
        setSubState(SUB_TOSS);
        hideTutorial();
        break;
    case ANSWER_QUIT:
    default:
        setSubState(SUB_MAIN);
        break;
    }
}

// .text:0x2340
BOOL dMenuPost01_c::pickUp() {
    if (isSlotEmpty(mCursor)) {
        return FALSE;
    }
    if (isLocked(mCursor)) {
        return FALSE;
    }
    mHeldFrom = mCursor;
    dMail_c mail;
    removeMail(mHeldFrom, &mail);
    mLtrWin.setHeldMail(&mail);
    onFlag(FLAG_FOLLOW);
    mLtrWin.startFollow();
    playSeItemLift();
    dObjCursor_c::hold();
    hideTutorial();
    return TRUE;
}

// .text:0x23FC
void dMenuPost01_c::calcHeld() {
    if (isFlag(FLAG_FOLLOW)) {
        mLtrWin.calcFollow();
        mLtrWin.calcHeldLetter();
    } else if (isFlag(FLAG_MOVE)) {
        mLtrWin.calcHeldLetter();
    }
}

// .text:0x2464
void dMenuPost01_c::dropHeld() {
    offFlag(FLAG_FOLLOW);
    dObjCursor_c::show();
    hideTutorial();
    if (mCursor == SLOT_NONE) {
        int target = findDropFree();
        if (target != -1) {
            mHeldFrom = target;
        }
        mLtrWin.startMove(getSlotPos(mHeldFrom, 0.0f, 0.0f));
        setSubState(SUB_MOVE);
        onFlag(FLAG_MOVE);
    } else {
        playSeItemDrop();
        dMail_c mail(*mLtrWin.getHeldMail());
        swapMail(mCursor, &mail, TRUE);
        if (mCursor != mHeldFrom) {
            swapMail(mHeldFrom, &mail, TRUE);
            if (isBox(mHeldFrom) || isBox(mCursor)) {
                onFlag(FLAG_CHANGED);
            }
        }
        mLtrWin.clearHeldMail();
        setSubState(SUB_MAIN);
    }
}

// .text:0x26D0
int dMenuPost01_c::findFreeLetter() {
    mVec2_c cursor = dObjCursor_c::getPos();
    float best = 1000000.0f;
    int found = SLOT_NONE;
    for (int i = 0; i < dMenuLtr::Window_c::LETTER_COUNT; i++) {
        if (isSlotEmpty(SLOT_LETTER + i)) {
            mVec2_c pos;
            pos = getSlotPos(SLOT_LETTER + i, 0.0f, 0.0f);
            float dx = pos.x - cursor.x;
            float dy = pos.y - cursor.y;
            float dist = dx * dx + dy * dy;
            if (dist < best) {
                best = dist;
                found = SLOT_LETTER + i;
            }
        }
    }
    return found;
}

// .text:0x27B0
int dMenuPost01_c::findFreeBox() {
    mVec2_c cursor = dObjCursor_c::getPos();
    float best = 1000000.0f;
    int found = SLOT_NONE;
    for (int i = 0; i < BOX_NUM; i++) {
        int slot = SLOT_BOX + i;
        if (isSlotEmpty(slot)) {
            mVec2_c pos;
            pos = getSlotPos(slot, 0.0f, 0.0f);
            float dx = pos.x - cursor.x;
            float dy = pos.y - cursor.y;
            float dist = dx * dx + dy * dy;
            if (dist < best) {
                best = dist;
                found = slot;
            }
        }
    }
    return found;
}

// .text:0x2884
// The free slot nearest to the cursor on the other side of the window.
int dMenuPost01_c::findDropFree() {
    mVec2_c cursor = dObjCursor_c::getPos();
    if (isLetter(mHeldFrom)) {
        if (cursor.y < 0.0f) {
            return SLOT_NONE;
        }
        return findFreeBox();
    } else if (isBox(mHeldFrom)) {
        if (cursor.y > -30.0f) {
            return SLOT_NONE;
        }
        return findFreeLetter();
    }
    return SLOT_NONE;
}
