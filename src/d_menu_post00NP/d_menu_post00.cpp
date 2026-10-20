#include <game/game/d_menu_post00.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_box.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/SC/scapi.h>

namespace dMenuPost00 {
// .text:0xCC
BASE_PROFILE(MENU_POST00, dMenuPost00_c)
} // namespace dMenuPost00

static const char *l_allPane = "N_all";

// .text:0x23C
int dMenuPost00_c::create() {
    BOOL loaded = mRes.load("Layout/post.arc");
    loaded &= mLtrWin.load();
    loaded &= mSelect.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "post_win_00.brlyt");
    mLyt.setVisible("N_sldCrsr_all", false);
    mLyt.setVisible("N_pstLtrIcn_00", true);
    mLyt.setMessage("T_btn_00", MSG_CLOSE, NULL);
    initAnm();
    setPage(0);
    mLyt.calc();
    mLtrWin.create();
    initIcons();
    mSelect.build();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_IN, STATE_MAIN, TRUE);
    mLtrWin.startWindowAnm(dMenuLtr::Window_c::WIN_ANM_POST_IN, false);
    playSeSubMenuOpen();
    if (mMenuNo == dMenuMng_c::MENU_MAILBOX_AFTER_READ) {
        setSubState(SUB_MAIN);
    } else {
        setSubState(SUB_TAKE);
    }
    mCursor = SLOT_NONE;
    return SUCCEEDED;
}

// .text:0x3BC
int dMenuPost00_c::doDelete() {
    if (mLtrWin.remove() == FALSE) {
        return NOT_READY;
    }
    if (mSelect.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes.remove() != 0;
}

// .text:0x428
int dMenuPost00_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x44C
int dMenuPost00_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    calcLayouts();
    return result;
}

// .text:0x490
int dMenuPost00_c::stateClose() {
    if (isFlag(FLAG_OPEN_LETTER) &&
        dAcUitag::openMenuB(dAcUitag::MENU_B_READ_8) == FALSE) {
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x4EC
int dMenuPost00_c::execute() {
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

// .text:0x5A8
int dMenuPost00_c::draw() {
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

// .text:0x63C
void dMenuPost00_c::subMain() {
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
        }
    }
}

// .text:0x710
void dMenuPost00_c::subPush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x74C
void dMenuPost00_c::subSelect() {
    if (static_cast<dSelect::BComp_c &>(mSelect).isClosed()) {
        mAnswer = mSelect.mCode;
        onSelectEnd();
    }
}

// .text:0x7A0
void dMenuPost00_c::subPress() {
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

// .text:0x870
void dMenuPost00_c::subDrag() {
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

// .text:0x914
void dMenuPost00_c::subMove() {
    if (mLtrWin.calcMove()) {
        dMail_c held(*mLtrWin.getHeldMail());
        swapMail(mHeldFrom, &held, TRUE);
        mLtrWin.clearHeldMail();
        playSeItemDrop();
        offFlag(FLAG_MOVE);
        setSubState(SUB_MAIN);
    }
}

// .text:0xAA8
void dMenuPost00_c::subToss() {
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

// .text:0xB70
void dMenuPost00_c::subTake() {
    int from = findBoxMail();
    if (from == SLOT_NONE) {
        setSubState(SUB_MAIN);
        return;
    }
    int to = findFreeLetter();
    if (to == SLOT_NONE) {
        setSubState(SUB_MAIN);
        return;
    }
    dMail_c mail;
    removeMail(from, &mail);
    mLtrWin.setHeldMail(&mail);
    mLtrWin.setMovePos(getSlotPos(from, 0.0f, 0.0f));
    mHeldFrom = to;
    mLtrWin.startMove(getSlotPos(to, 0.0f, 0.0f));
    onFlag(FLAG_MOVE);
    setSubState(SUB_TAKE_MOVE);
}

// .text:0xCA8
void dMenuPost00_c::subTakeMove() {
    subMove();
    if (!isFlag(FLAG_MOVE)) {
        setSubState(SUB_TAKE);
    }
}

// .text:0xD00
void dMenuPost00_c::executeSubState() {
    static void (dMenuPost00_c::*l_sub[SUB_NUM])() = {
        &dMenuPost00_c::subMain,    &dMenuPost00_c::subPush,
        &dMenuPost00_c::subSelect,  &dMenuPost00_c::subPress,
        &dMenuPost00_c::subDrag,    &dMenuPost00_c::subMove,
        &dMenuPost00_c::subToss,    &dMenuPost00_c::subTake,
        &dMenuPost00_c::subTakeMove};
    (this->*l_sub[mSubState])();
}

// .text:0xD38
void dMenuPost00_c::setHighlight(int slot, BOOL on) {
    if (isPageButton(slot)) {
        mPageAnm[slot - SLOT_NEXT_PAGE].start(on ? BTN_ANM_IN : BTN_ANM_OUT);
        mPageAnm[slot - SLOT_NEXT_PAGE].setToEnd();
        onFlag(FLAG_CALC);
    } else if (slot == SLOT_BACK) {
        mBackAnm.start(on == 0);
        mBackAnm.setToEnd();
        onFlag(FLAG_CALC);
    } else if (isBox(slot)) {
        mIcons[slot - SLOT_BOX].setMark(on != 0);
    } else if (isLetter(slot)) {
        mLtrWin.setMark(slot, on != 0);
    }
}

// .text:0xE68
int dMenuPost00_c::hitSlot(bool setOffset) {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    mVec2_c btnPos(pos);
    int slot =
        m2d::hitButton(&btnPos, &mLyt, SLOT_BOX, SLOT_BACK, -1, -1, 1.0f);
    if (slot != SLOT_NONE) {
        return slot;
    }
    mVec2_c letterPos(pos);
    slot = mLtrWin.hitLetter(&letterPos, setOffset, 1.0f);
    if (slot != SLOT_NONE) {
        return slot;
    }
    return SLOT_NONE;
}

// .text:0xF34
int dMenuPost00_c::hitCheck() {
    int slot = hitSlot(true);
    if (isLetter(slot) || isBox(slot)) {
        if (isSlotEmpty(slot)) {
            return SLOT_NONE;
        }
    }
    return slot;
}

// .text:0xFD0
int dMenuPost00_c::hitCheckDrop() {
    int slot = hitSlot(false);
    if (isLetter(slot)) {
        if (isLocked(slot)) {
            return SLOT_NONE;
        }
        if (!isSlotEmpty(slot) && isBox(mHeldFrom)) {
            return SLOT_NONE;
        }
        return slot;
    } else if (isBox(slot)) {
        if (isLetter(mHeldFrom)) {
            return SLOT_NONE;
        }
        return slot;
    }
    return SLOT_NONE;
}

// .text:0x10E4
void dMenuPost00_c::pressButton() {
    if (isButtonEnabled(mCursor)) {
        if (isPageButton(mCursor)) {
            mPageAnm[mCursor - SLOT_NEXT_PAGE].setNext(BTN_ANM_PUSH);
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

// .text:0x11F8
BOOL dMenuPost00_c::isPushEnd() {
    if (isPageButton(mCursor)) {
        return mPageAnm[mCursor - SLOT_NEXT_PAGE].isEnd();
    } else if (mCursor == SLOT_BACK) {
        return mBackAnm.isEnd();
    }
    return TRUE;
}

// .text:0x1248
void dMenuPost00_c::finishPush() {
    if (isPageButton(mCursor)) {
        startClose();
    } else if (mCursor == SLOT_BACK) {
        startClose();
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x1290
void dMenuPost00_c::startClose() {
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_OUT, STATE_CLOSE, TRUE);
    mLtrWin.startWindowAnm(dMenuLtr::Window_c::WIN_ANM_POST_OUT, false);
    playSeSubMenuClose();
    dMenuMng_c::setResult(0);
    hideTutorial();
}

// .text:0x12F8
void dMenuPost00_c::readLetter() {
    dMenuMng_c::setMail(getMail(mSelectSlot));
    markRead(mSelectSlot);
    startClose();
    onFlag(FLAG_OPEN_LETTER);
}

// .text:0x134C
void dMenuPost00_c::setPage(int page) {
    mPage = page;
    dString::Word_c word;
    dScript::formatNumber(&word, mPage + 1, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(0, &word);
    dScript::formatNumber(&word, PAGE_NUM, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(1, &word);
    mLyt.setMessage("T_page_00", MSG_PAGE_NUMBER, NULL);
    updatePageButtons();
}

// .text:0x13F8
void dMenuPost00_c::initAnm() {
    static const char *l_nextAnm[] = {"post_win_00_sldCrsr_00_In.brlan",
                                      "post_win_00_sldCrsr_00_Out.brlan",
                                      "post_win_00_sldCrsr_00_Psh.brlan",
                                      "post_win_00_sldCrsr_00_CanPsh.brlan",
                                      "post_win_00_sldCrsr_00_NoPsh.brlan",
                                      NULL};
    static const char *l_prevAnm[] = {"post_win_00_sldCrsr_01_In.brlan",
                                      "post_win_00_sldCrsr_01_Out.brlan",
                                      "post_win_00_sldCrsr_01_Psh.brlan",
                                      "post_win_00_sldCrsr_01_CanPsh.brlan",
                                      "post_win_00_sldCrsr_01_NoPsh.brlan"};
    static const char *l_backAnm[] = {"post_win_00_squareBtn_In.brlan",
                                      "post_win_00_squareBtn_Out.brlan",
                                      "post_win_00_squareBtn_Psh.brlan"};
    mPageAnm[0].init(&mLyt, &mRes, l_nextAnm, PAGE_ANM_NUM);
    mPageAnm[0].start(BTN_ANM_OUT);
    mPageAnm[0].setToEnd();
    mPageAnm[1].init(&mLyt, &mRes, l_prevAnm, PAGE_ANM_NUM);
    mPageAnm[1].start(BTN_ANM_OUT);
    mPageAnm[1].setToEnd();
    mBackAnm.init(&mLyt, &mRes, l_backAnm, BACK_ANM_NUM);
    mBackAnm.start(BTN_ANM_OUT);
    mBackAnm.setToEnd();
    mLyt.setAnm(&mScrollAnm, "post_win_00_scrlIcon_00.brlan", NULL);
}

// .text:0x14C8
void dMenuPost00_c::calcAnm() {
    int playing = 0;
    for (int i = 0; i < 2; i++) {
        playing |= mPageAnm[i].play();
    }
    playing |= mBackAnm.play();
    if (playing) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x154C
BOOL dMenuPost00_c::isButtonEnabled(int slot) {
    switch (slot) {
    case SLOT_NEXT_PAGE:
        return isNextPageEnabled();
    case SLOT_PREV_PAGE:
        return isPrevPageEnabled();
    }
    return TRUE;
}

// .text:0x1574
BOOL dMenuPost00_c::isNextPageEnabled() {
    return mPage < PAGE_NUM - 1;
}

// .text:0x1590
BOOL dMenuPost00_c::isPrevPageEnabled() {
    return mPage > 0;
}

// .text:0x15A4
void dMenuPost00_c::updatePageButtons() {
    mPageAnm[0].start(isNextPageEnabled() ? BTN_ANM_ENABLED : BTN_ANM_DISABLED);
    mPageAnm[0].setToEnd();
    mPageAnm[1].start(isPrevPageEnabled() ? BTN_ANM_ENABLED : BTN_ANM_DISABLED);
    mPageAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x1624
void dMenuPost00_c::initIcons() {
    for (int i = 0; i < LETTER_NUM; i++) {
        mIcons[i].create(mLtrWin.getRes());
    }
    initMails();
    unlockAll();
}

// .text:0x1698
int dMenuPost00_c::positionIcon(nw4r::lyt::Pane *pane, void *arg) {
    int index = m2d::getNameNumber(pane->GetName(), 2);
    if (index >= 0 && index < LETTER_NUM) {
        mVec3_c pos;
        m2d::getScreenPos(&pos, pane);
        ((dMenuLtr::Icon_c *)arg)[index].setRootPos(&pos);
    }
    return 0;
}

// .text:0x1714
void dMenuPost00_c::calcLayouts() {
    mLyt.patrolPane(positionIcon, m2d::isRectButton, mIcons);
    for (int i = 0; i < LETTER_NUM; i++) {
        mIcons[i].calcLayout();
    }
}

// .text:0x1780
void dMenuPost00_c::drawIcons() {
    for (int i = 0; i < LETTER_NUM; i++) {
        mIcons[i].drawLayout();
    }
}

// .text:0x17CC
void dMenuPost00_c::initMails() {
    for (int i = 0; i < LETTER_NUM; i++) {
        mIcons[i].setMail(getMail(SLOT_BOX + i), -1);
    }
}

// .text:0x1838
const mVec2_c &dMenuPost00_c::getSlotPos(int slot, float dx, float dy) {
    static mVec2_c pos(0.0f, 0.0f);
    mVec3_c root;
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        dx *= 0.730769f;
    }
    if (isBox(slot)) {
        root = mIcons[slot - SLOT_BOX].getRootPos();
        pos.x = root.x + dx;
        pos.y = root.y + dy;
        return pos;
    }
    if (isLetter(slot)) {
        return mLtrWin.getLetterPos(slot, dx, dy);
    }
    return pos;
}

// .text:0x19A4
dMail_c *dMenuPost00_c::getMail(int slot) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (isBox(slot)) {
        return &player->getPostBox()->mMails[slot - SLOT_BOX];
    } else if (isLetter(slot)) {
        return &player->mLetters[slot];
    }
    return NULL;
}

// .text:0x1A38
void dMenuPost00_c::swapMail(int slot, dMail_c *mail, BOOL updateIcon) {
    dMail_c old;
    old.clear();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (isBox(slot)) {
        int index = slot - SLOT_BOX;
        dPlayerPostBox_c *box = player->getPostBox();
        old.copy(&box->mMails[index]);
        box->set(index, mail, TRUE);
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

// .text:0x1B7C
void dMenuPost00_c::removeMail(int slot, dMail_c *removed) {
    dMail_c empty;
    empty.clear();
    swapMail(slot, &empty, TRUE);
    if (removed) {
        removed->copy(&empty);
    }
}

// .text:0x1C00
BOOL dMenuPost00_c::isSlotEmpty(int slot) {
    dMail_c *mail = getMail(slot);
    if (mail == NULL) {
        return TRUE;
    }
    return mail->isEmpty();
}

// .text:0x1C34
int dMenuPost00_c::findBoxMail() {
    for (int i = SLOT_BOX; i <= SLOT_BOX_LAST; i++) {
        if (!isSlotEmpty(i)) {
            return i;
        }
    }
    return SLOT_NONE;
}

// .text:0x1C94
int dMenuPost00_c::findFreeLetter() {
    for (int i = SLOT_LETTER; i <= SLOT_LETTER_LAST; i++) {
        if (isSlotEmpty(i) == TRUE) {
            return i;
        }
    }
    return SLOT_NONE;
}

// .text:0x1CF4
void dMenuPost00_c::markRead(int slot) {
    dMail_c *mail = getMail(slot);
    if (mail) {
        dMail_c copy(*mail);
        copy.markRead();
        swapMail(slot, &copy, FALSE);
    }
}

// .text:0x1E60
BOOL dMenuPost00_c::isLocked(int slot) {
    if (isLetter(slot)) {
        return (mLockMask & (u16)(1 << slot)) != 0;
    }
    return FALSE;
}

// .text:0x1EAC
void dMenuPost00_c::unlockAll() {
    mLockMask = 0;
}

// .text:0x1EB8
void dMenuPost00_c::showSlotTutorial(int slot) {
    int tail = 0;
    dMail_c *mail = getMail(slot);
    if (mail == NULL || mail->isEmpty()) {
        hideTutorial();
        return;
    }
    if (dObjTutorial_c::request(dObjTutorial_c::REQUEST_MAILBOX_LETTER, slot,
                                0)) {
        if (isLetter(slot) && slot >= LETTER_ROW_NUM) {
            tail = 1;
        }
        m2d::stringWord_c word;
        mail->getListWord(&word);
        mVec2_c pos = getSlotPos(slot, 0.0f, 0.0f);
        dObjTutorial_c::setWord(&pos, &word, tail);
    }
}

// .text:0x1FC4
void dMenuPost00_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x1FC8
void dMenuPost00_c::openSelect(int slot) {
    mSelectSlot = slot;
    buildSelect(slot);
    mVec2_c pos = getSlotPos(slot, 10.0f, -50.0f);
    mSelect.placeAbove(pos);
    mSelect.openPointer();
    setSubState(SUB_SELECT);
}

// .text:0x21A4
void dMenuPost00_c::buildSelect(int slot) {
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

// .text:0x2278
void dMenuPost00_c::onSelectEnd() {
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

// .text:0x2300
BOOL dMenuPost00_c::pickUp() {
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

// .text:0x23BC
void dMenuPost00_c::calcHeld() {
    if (isFlag(FLAG_FOLLOW)) {
        mLtrWin.calcFollow();
        mLtrWin.calcHeldLetter();
    } else if (isFlag(FLAG_MOVE)) {
        mLtrWin.calcHeldLetter();
    }
}

// .text:0x2424
void dMenuPost00_c::dropHeld() {
    offFlag(FLAG_FOLLOW);
    dObjCursor_c::show();
    hideTutorial();
    if (mCursor == SLOT_NONE) {
        int slot = findDropFree();
        if (slot != SLOT_NONE) {
            mHeldFrom = slot;
        }
        mLtrWin.startMove(getSlotPos(mHeldFrom, 0.0f, 0.0f));
        setSubState(SUB_MOVE);
        onFlag(FLAG_MOVE);
    } else {
        playSeItemDrop();
        dMail_c held(*mLtrWin.getHeldMail());
        swapMail(mCursor, &held, TRUE);
        if (mCursor != mHeldFrom) {
            swapMail(mHeldFrom, &held, TRUE);
        }
        mLtrWin.clearHeldMail();
        setSubState(SUB_MAIN);
    }
}

// .text:0x263C
int dMenuPost00_c::findNearestFree() {
    mVec2_c cursor = dObjCursor_c::getPos();
    int nearest = SLOT_NONE;
    float nearestDist = 1000000.0f;
    for (int i = 0; i < LETTER_NUM; i++) {
        if (isSlotEmpty(i)) {
            mVec2_c pos;
            pos = getSlotPos(i, 0.0f, 0.0f);
            float dx = pos.x - cursor.x;
            float dy = pos.y - cursor.y;
            float dist = dx * dx + dy * dy;
            if (dist < nearestDist) {
                nearestDist = dist;
                nearest = i;
            }
        }
    }
    return nearest;
}

// .text:0x271C
int dMenuPost00_c::findDropFree() {
    mVec2_c cursor = dObjCursor_c::getPos();
    if (isBox(mHeldFrom)) {
        if (cursor.y > -30.0f) {
            return SLOT_NONE;
        }
        return findNearestFree();
    }
    return SLOT_NONE;
}
