#include <game/game/d_menu_item04.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_box.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/sc.h>

namespace dMenuItem04 {
BASE_PROFILE(MENU_ITEM04, dMenuItem04_c)
} // namespace dMenuItem04

static const char *l_allPane = "N_all";

// .text:0x268
int dMenuItem04_c::create() {
    BOOL resLoaded = mRes.load("Layout/chest.arc");
    BOOL winLoaded = mItemWin.load();
    if (!(resLoaded & winLoaded)) {
        return NOT_READY;
    }
    mItemWin.create();
    createLayouts();
    mLyt.build(&mRes, "chest_win_00.brlyt");
    mLyt.setMessage("T_btn_00", MSG_CLOSE, nullptr);
    initAnms();
    setPage(0);
    mLyt.calc();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_IN, STATE_MAIN, TRUE);
    mItemWin.startAnm(dMenuItem::Window_c::ANM_CHEST_IN);
    playSeSubMenuOpen();
    mCursor = SLOT_NONE;
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x388
int dMenuItem04_c::doDelete() {
    if (!mItemWin.remove()) {
        return NOT_READY;
    }
    bool removed = mRes.remove();
    return removed;
}

// .text:0x3DC
int dMenuItem04_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x400
int dMenuItem04_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    calcLayouts();
    return result;
}

// .text:0x444
int dMenuItem04_c::stateClose() {
    if (isFlag(FLAG_PAGE_TURN) && isPageBtn(mCursor)) {
        setPage(mNextPage);
        startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_IN, STATE_MAIN, TRUE);
        refreshBox();
        offFlag(FLAG_PAGE_TURN);
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x4F4
int dMenuItem04_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != SLOT_NONE && prev != mCursor) {
        playSeCursor();
    }
    mItemWin.calcIcons();
    calcAnms();
    calcHeld();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    return result;
}

// .text:0x5A0
int dMenuItem04_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mItemWin.draw();
        mLyt.entry();
        drawIcons();
        if (isFlag(FLAG_HOLD_ITEM) || isFlag(FLAG_RETURN_ITEM)) {
            mItemWin.drawHeldItem();
        }
    }
    return SUCCEEDED;
}

// .text:0x624
void dMenuItem04_c::subStateMain() {
    hideTutorial();
    setSelect(mCursor, FALSE);
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        mCursor = SLOT_BACK;
        pushCursor();
        return;
    }
    int slot = hitItem();
    mCursor = slot;
    if (slot != SLOT_NONE) {
        setSelect(slot, TRUE);
        showSlotTutorial(mCursor);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            if (grabItem()) {
                setSubState(SUB_HOLD);
            } else {
                pushCursor();
            }
            return;
        }
    }
    checkPadMove();
}

// .text:0x730
void dMenuItem04_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x76C
void dMenuItem04_c::subStateHold() {
    hideTutorial();
    setSelect(mCursor, FALSE);
    int slot = hitDropSlot();
    mCursor = slot;
    if (slot != SLOT_NONE) {
        setSelect(slot, TRUE);
        showSlotTutorial(mCursor);
    }
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A) ||
        !dObjCursor_c::isVisible()) {
        dropItem();
    }
}

// .text:0x810
void dMenuItem04_c::subStateReturn() {
    if (mItemWin.calcMove()) {
        dItem::Item item = mHeldItem;
        setItem(mHeldFrom, item);
        mHeldItem = dItem::ITEM_ID_NONE;
        playSeItemDrop();
        offFlag(FLAG_RETURN_ITEM);
        setSubState(SUB_MAIN);
    }
}

// .text:0x894
void dMenuItem04_c::executeSubState() {
    typedef void (dMenuItem04_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuItem04_c::subStateMain,
        &dMenuItem04_c::subStatePush,
        &dMenuItem04_c::subStateHold,
        &dMenuItem04_c::subStateReturn,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x8CC
void dMenuItem04_c::setSelect(int slot, BOOL select) {
    if (isPageBtn(slot)) {
        mPageBtnAnm[slot - SLOT_PAGE_PREV].start(select ? 0 : 1);
        mPageBtnAnm[slot - SLOT_PAGE_PREV].setToEnd();
        onFlag(FLAG_CALC);
    } else if (slot == SLOT_BACK) {
        mBackBtnAnm.start(select == FALSE);
        mBackBtnAnm.setToEnd();
        onFlag(FLAG_CALC);
    } else if (isPocket(slot)) {
        mItemWin.setMark(slot, select);
    } else if (isBox(slot)) {
        dMenuItem::Icon_c *icon = &mBoxIcons[slot - SLOT_BOX];
        icon->setMark(select);
        if (select) {
            icon->calcLayouts();
        }
    }
}

// .text:0xA1C
int dMenuItem04_c::hitSlot(bool setOffset) {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    mVec2_c cursor = dObjCursor_c::getPos();
    mVec2_c ofs;
    mVec2_c pos = cursor;
    int slot = m2d::hitButton(&pos, &mLyt, SLOT_PAGE_PREV, SLOT_BACK, SLOT_BOX,
                              SLOT_BOX_LAST, 1.0f);
    if (slot != -1) {
        if (setOffset && isBox(slot)) {
            const mVec2_c &center = getSlotPos(slot, 0.0f, 0.0f);
            float cy = center.y;
            float cx = center.x;
            ofs.x = cx - cursor.x;
            ofs.y = cy - cursor.y;
            mItemWin.setMoveOffset(ofs);
        }
        return slot;
    }
    mVec2_c pocketPos = cursor;
    int pocket = mItemWin.hitPocket(&pocketPos, setOffset, 1.0f);
    if (pocket != -1) {
        return pocket;
    }
    return SLOT_NONE;
}

// .text:0xB64
int dMenuItem04_c::hitItem() {
    int slot = hitSlot(true);
    if (isPocket(slot) || isBox(slot)) {
        if (getItem(slot) == dItem::ITEM_ID_NONE) {
            return SLOT_NONE;
        }
    }
    return slot;
}

// .text:0xC04
int dMenuItem04_c::hitDropSlot() {
    int slot = hitSlot(false);
    if (isPocket(slot) || isBox(slot)) {
        if (isLocked(slot)) {
            return SLOT_NONE;
        }
        return slot;
    }
    return SLOT_NONE;
}

// .text:0xCA8
void dMenuItem04_c::checkPadMove() {
    if (dPad::isUpTriggered()) {
        setSelect(mCursor, FALSE);
        mCursor = SLOT_PAGE_PREV;
        pushCursor();
    } else if (dPad::isDownTriggered()) {
        setSelect(mCursor, FALSE);
        mCursor = SLOT_PAGE_NEXT;
        pushCursor();
    }
}

// .text:0xD2C
void dMenuItem04_c::pushCursor() {
    if (isEnabled(mCursor)) {
        int slot = mCursor;
        if (isPageBtn(slot)) {
            mPageBtnAnm[slot - SLOT_PAGE_PREV].setNext(PAGE_BTN_ANM_PUSH);
        } else if (slot == SLOT_BACK) {
            mBackBtnAnm.setNext(BACK_BTN_ANM_PUSH);
            playSeCancel();
        } else {
            return;
        }
        setSubState(SUB_PUSH);
    }
}

// .text:0xDDC
BOOL dMenuItem04_c::isPushEnd() {
    int slot = mCursor;
    if (isPageBtn(slot)) {
        return mPageBtnAnm[slot - SLOT_PAGE_PREV].isEnd();
    }
    if (slot == SLOT_BACK) {
        return mBackBtnAnm.isEnd();
    }
    return TRUE;
}

// .text:0xE2C
void dMenuItem04_c::finishPush() {
    int slot = mCursor;
    if (isPageBtn(slot)) {
        if (slot == SLOT_PAGE_NEXT) {
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
    } else if (slot == SLOT_BACK) {
        startClose();
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xEF8
void dMenuItem04_c::startClose() {
    offFlag(FLAG_PAGE_TURN);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_OUT, STATE_CLOSE, TRUE);
    mItemWin.startAnm(dMenuItem::Window_c::ANM_CHEST_OUT);
    playSeSubMenuClose();
    dMenuMng_c::setResult(dMenuMng_c::RESULT_CANCEL);
    hideTutorial();
}

// .text:0xF68
void dMenuItem04_c::startPageTurn() {
    onFlag(FLAG_PAGE_TURN);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_OUT, STATE_CLOSE, TRUE);
    hideTutorial();
    setSubState(SUB_MAIN);
}

// .text:0xFD8
void dMenuItem04_c::setPage(int page) {
    mPage = page;
    mColorAnm.setFrame((f32)page);
    for (int i = 0; i < BOX_SLOT_NUM; i++) {
        mHoleAnm[i].setFrame((f32)page);
    }
    dString::Word_c word;
    dScript::formatNumber(&word, mPage + 1, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(0, &word);
    dScript::formatNumber(&word, PAGE_NUM, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(1, &word);
    mLyt.setMessage("T_page_00", MSG_PAGE, nullptr);
    setPageBtnAnms();
}

// .text:0x1100
void dMenuItem04_c::initAnms() {
    static const char *pagePrevAnms[] = {
        "chest_win_00_sldCrsr_00_In.brlan",
        "chest_win_00_sldCrsr_00_Out.brlan",
        "chest_win_00_sldCrsr_00_Psh.brlan",
        "chest_win_00_sldCrsr_00_CanPsh.brlan",
        "chest_win_00_sldCrsr_00_NoPsh.brlan",
    };
    static const char *pageNextAnms[] = {
        "chest_win_00_sldCrsr_01_In.brlan",
        "chest_win_00_sldCrsr_01_Out.brlan",
        "chest_win_00_sldCrsr_01_Psh.brlan",
        "chest_win_00_sldCrsr_01_CanPsh.brlan",
        "chest_win_00_sldCrsr_01_NoPsh.brlan",
        nullptr,
    };
    static const char *backAnms[] = {
        "chest_win_00_squareBtn_In.brlan",
        "chest_win_00_squareBtn_Out.brlan",
        "chest_win_00_squareBtn_Psh.brlan",
        nullptr,
    };
    mPageBtnAnm[0].init(&mLyt, &mRes, pagePrevAnms, 5);
    mPageBtnAnm[0].start(1);
    mPageBtnAnm[0].setToEnd();
    mPageBtnAnm[1].init(&mLyt, &mRes, pageNextAnms, 5);
    mPageBtnAnm[1].start(1);
    mPageBtnAnm[1].setToEnd();
    mBackBtnAnm.init(&mLyt, &mRes, backAnms, 3);
    mBackBtnAnm.start(1);
    mBackBtnAnm.setToEnd();
    mLyt.setAnm(&mColorAnm, "chest_win_00_frmColorPtrn.brlan", nullptr);
    for (int i = 0; i < BOX_SLOT_NUM; i++) {
        mHoleAnm[i].create("chest_win_00_holeColorPtrn.brlan", &mRes);
        m2d::setAnmNumber(&mHoleAnm[i], i);
        mLyt.bind(&mHoleAnm[i]);
        mHoleAnm[i].setFrame(0.0f);
    }
}

// .text:0x1220
void dMenuItem04_c::calcAnms() {
    int playing = 0;
    for (int i = 0; i < PAGE_BTN_NUM; i++) {
        playing |= mPageBtnAnm[i].play();
    }
    playing |= mBackBtnAnm.play();
    if (playing) {
        onFlag(FLAG_CALC);
    }
    mItemWin.calcAnm();
}

// .text:0x12AC
void dMenuItem04_c::createLayouts() {
    for (int i = 0; i < BOX_SLOT_NUM; i++) {
        mBoxIcons[i].create(mItemWin.getIconRes(), mItemWin.getItemAnm());
    }
    refreshBox();
    updateLocks();
}

// .text:0x1338
int dMenuItem04_c::positionIcon(nw4r::lyt::Pane *pane, void *arg) {
    int index = m2d::getNameNumber(pane->GetName(), 2);
    if (index >= 0 && index < BOX_SLOT_NUM) {
        mVec3_c pos;
        m2d::getScreenPos(&pos, pane);
        ((dMenuItem::Icon_c *)arg)[index].setRootPos(&pos);
    }
    return 0;
}

// .text:0x13B4
void dMenuItem04_c::calcLayouts() {
    mLyt.patrolPane(positionIcon, m2d::isCircleButton, mBoxIcons);
    for (int i = 0; i < BOX_SLOT_NUM; i++) {
        mBoxIcons[i].calcLayouts();
    }
}

// .text:0x1420
void dMenuItem04_c::drawIcons() {
    for (int i = 0; i < BOX_SLOT_NUM; i++) {
        mBoxIcons[i].drawLayouts();
    }
}

// .text:0x146C
void dMenuItem04_c::refreshBox() {
    for (int i = 0; i < BOX_SLOT_NUM; i++) {
        dItem::Item item = getItem(SLOT_BOX + i);
        mBoxIcons[i].setItem(item, 0, -1);
    }
}

// .text:0x14E0
const mVec2_c &dMenuItem04_c::getSlotPos(int slot, float dx, float dy) {
    static mVec2_c pos(0.0f, 0.0f);
    mVec3_c p;
    if (SCGetAspectRatio() == SC_ASPECT_RATIO_16x9) {
        dx *= 0.730769f;
    }
    if (isPocket(slot)) {
        p = mItemWin.getPocketPos(slot);
        pos.x = p.x + dx;
        pos.y = p.y + dy;
        return pos;
    } else if (isBox(slot)) {
        p = mBoxIcons[slot - SLOT_BOX].getRootPos();
        pos.x = p.x + dx;
        pos.y = p.y + dy;
        return pos;
    }
    return pos;
}

// .text:0x166C
bool dMenuItem04_c::isLocked(int slot) {
    if (isPocket(slot)) {
        return (mLocked & (u16)(1 << slot)) != 0;
    }
    return false;
}

// .text:0x16B8
void dMenuItem04_c::setLocked(int slot, BOOL locked) {
    u16 bit = 1 << slot;
    mItemWin.setNoChoice(slot, locked);
    if (locked) {
        mLocked |= bit;
    } else {
        mLocked &= ~bit;
    }
}

// .text:0x1738
void dMenuItem04_c::updateLocks() {
    mLocked = 0;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item *pockets = player->mPockets;
    for (int i = 0; i < POCKET_NUM; i++) {
        BOOL locked = FALSE;
        if (player->getPocketFlag(i) != 0) {
            locked = TRUE;
        } else if (dMenuItem::needsSpecialHandling(&pockets[i])) {
            locked = TRUE;
        }
        setLocked(i, locked);
    }
}

// .text:0x17D4
u16 dMenuItem04_c::setItem(int slot, dItem::Item item) {
    if (isPocket(slot)) {
        return mItemWin.setPocket(slot, item, 0);
    }
    if (isBox(slot)) {
        int box = slot - SLOT_BOX;
        dSaveItemBox_c *itemBox =
            dPlayerMgr::Mgr_c::getCurrentPlayer()->getItemBox();
        mBoxIcons[box].setItem(item, 0, -1);
        return itemBox->set(mPage, box, &item);
    }
    return dItem::ITEM_ID_NONE;
}

// .text:0x18C8
u16 dMenuItem04_c::dropOnSlot(int slot, dItem::Item item) {
    dItem::Item dst(getItem(slot));
    dItem::Item src(item);
    if (dMenuItem::combineItems(dst, src)) {
        setItem(slot, dst);
        return src.mId;
    }
    return setItem(slot, item);
}

// .text:0x1964
u16 dMenuItem04_c::takeItem(int slot) {
    return setItem(slot, dItem::ITEM_ID_NONE);
}

// .text:0x1994
u16 dMenuItem04_c::getItem(int slot) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (isPocket(slot)) {
        return player->mPockets[slot].mId;
    }
    if (isBox(slot)) {
        return player->getItemBox()->get(mPage, slot - SLOT_BOX);
    }
    return dItem::ITEM_ID_NONE;
}

// .text:0x1A38
bool dMenuItem04_c::grabItem() {
    if (getItem(mCursor) == dItem::ITEM_ID_NONE) {
        return false;
    }
    if (isLocked(mCursor)) {
        return false;
    }
    mHeldFrom = mCursor;
    mHeldItem = takeItem(mHeldFrom);
    mItemWin.setHeldItem(mHeldItem, 0);
    onFlag(FLAG_HOLD_ITEM);
    mItemWin.startFollow();
    playSeItemLift();
    dObjCursor_c::hold();
    hideTutorial();
    return true;
}

// .text:0x1AE4
void dMenuItem04_c::calcHeld() {
    if (isFlag(FLAG_HOLD_ITEM)) {
        mItemWin.calcFollow();
        mItemWin.calcHeldItem();
    } else if (isFlag(FLAG_RETURN_ITEM)) {
        mItemWin.calcHeldItem();
    }
}

// .text:0x1B4C
void dMenuItem04_c::dropItem() {
    offFlag(FLAG_HOLD_ITEM);
    dObjCursor_c::show();
    hideTutorial();
    if (mCursor == SLOT_NONE) {
        int slot = findReturnSlot();
        if (slot != -1) {
            mHeldFrom = slot;
        }
        mItemWin.startMove(getSlotPos(mHeldFrom, 0.0f, 0.0f));
        setSubState(SUB_RETURN);
        onFlag(FLAG_RETURN_ITEM);
    } else {
        playSeItemDrop();
        dItem::Item old = dropOnSlot(mCursor, mHeldItem);
        if (old.getId() != dItem::ITEM_ID_NONE) {
            setItem(mHeldFrom, old);
            mHeldItem = dItem::ITEM_ID_NONE;
        }
        setSubState(SUB_MAIN);
    }
}

// .text:0x1C54
void dMenuItem04_c::getSlotName(dScript::Word_c *word, int slot) {
    word->clear();
    if (isPocket(slot)) {
        mItemWin.getPocketName(word, slot);
    } else if (isBox(slot)) {
        dItem::name_c name(getItem(slot));
        word->copy(&name, 0);
    }
}

// .text:0x1D28
void dMenuItem04_c::showSlotTutorial(int slot) {
    int lower = 0;
    if (getItem(slot) != dItem::ITEM_ID_NONE) {
        if (isPocket(slot) && slot >= POCKET_TAIL_UP_FIRST) {
            lower = 1;
        }
        if (dObjTutorial_c::request(TUTORIAL_ITEM_BOX, slot, 0)) {
            mVec2_c pos = getSlotPos(slot, 0.0f, 0.0f);
            m2d::stringWord_c word;
            getSlotName(&word, slot);
            dObjTutorial_c::setWord(&pos, &word, lower);
        }
    } else {
        hideTutorial();
    }
}

// .text:0x1E24
void dMenuItem04_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x1E28
bool dMenuItem04_c::isEnabled(int slot) {
    switch (slot) {
    case SLOT_PAGE_PREV:
        return canPrev();
    case SLOT_PAGE_NEXT:
        return canNext();
    }
    return true;
}

// .text:0x1E50
bool dMenuItem04_c::canPrev() {
    return true;
}

// .text:0x1E58
bool dMenuItem04_c::canNext() {
    return true;
}

// .text:0x1E60
void dMenuItem04_c::setPageBtnAnms() {
    mPageBtnAnm[0].start(canPrev() ? PAGE_BTN_ANM_CAN_PUSH
                                   : PAGE_BTN_ANM_NO_PUSH);
    mPageBtnAnm[0].setToEnd();
    mPageBtnAnm[1].start(canNext() ? PAGE_BTN_ANM_CAN_PUSH
                                   : PAGE_BTN_ANM_NO_PUSH);
    mPageBtnAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x1EE0
int dMenuItem04_c::findNearestPocket() {
    mVec2_c cursor = dObjCursor_c::getPos();
    float nearest = 1000000.0f;
    int found = -1;
    for (int i = 0; i < POCKET_NUM; i++) {
        if (getItem(i) == dItem::ITEM_ID_NONE) {
            mVec2_c pos;
            pos = getSlotPos(i, 0.0f, 0.0f);
            float dx = pos.x - cursor.x;
            float dy = pos.y - cursor.y;
            float dist = dx * dx + dy * dy;
            if (dist < nearest) {
                nearest = dist;
                found = i;
            }
        }
    }
    return found;
}

// .text:0x1FC4
int dMenuItem04_c::findNearestBox() {
    mVec2_c cursor = dObjCursor_c::getPos();
    float nearest = 1000000.0f;
    int found = -1;
    for (int i = 0; i < BOX_SLOT_NUM; i++) {
        if (getItem(SLOT_BOX + i) == dItem::ITEM_ID_NONE) {
            mVec2_c pos;
            pos = getSlotPos(SLOT_BOX + i, 0.0f, 0.0f);
            float dx = pos.x - cursor.x;
            float dy = pos.y - cursor.y;
            float dist = dx * dx + dy * dy;
            if (dist < nearest) {
                nearest = dist;
                found = SLOT_BOX + i;
            }
        }
    }
    return found;
}

// .text:0x20A8
int dMenuItem04_c::findReturnSlot() {
    mVec2_c cursor = dObjCursor_c::getPos();
    if (isPocket(mHeldFrom)) {
        if (cursor.y < 50.0f) {
            return SLOT_NONE;
        }
        return findNearestBox();
    }
    if (isBox(mHeldFrom)) {
        if (cursor.y > 30.0f) {
            return SLOT_NONE;
        }
        return findNearestPocket();
    }
    return SLOT_NONE;
}
