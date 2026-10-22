#include <game/game/d_menu_item05.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_save_data.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/sc.h>

namespace dMenuItem05 {
// .text:0xCC
BASE_PROFILE(MENU_ITEM05, dMenuItem05_c)
} // namespace dMenuItem05

static const char *l_allPane = "N_all";

// .text:0x220
int dMenuItem05_c::create() {
    BOOL resLoaded = mRes.load("Layout/gather.arc");
    BOOL winLoaded = mWin.load();
    if (!(resLoaded & winLoaded)) {
        return NOT_READY;
    }
    mWin.create();
    initIcons();
    mLyt.build(&mRes, "gather_win_00.brlyt");
    mLyt.setMessage("T_btn_00", MSG_CLOSE, nullptr);
    initAnm();
    mLyt.calc();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_IN, STATE_MAIN, TRUE);
    mWin.startAnm(dMenuItem::Window_c::ANM_CHEST_IN);
    playSeSubMenuOpen();
    mCursor = CURSOR_NONE;
    if (mMenuNo == dMenuMng_c::MENU_LOST_AND_FOUND) {
        mLostNum = dSaveData_c::getTown()->mPoliceBox.count();
    }
    dPostOffice::clearRecycleSync();
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x358
int dMenuItem05_c::doDelete() {
    if (!mWin.remove()) {
        return NOT_READY;
    }
    bool removed = mRes.remove();
    return removed;
}

// .text:0x3AC
int dMenuItem05_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x3D0
int dMenuItem05_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    positionIcons();
    return result;
}

// .text:0x414
int dMenuItem05_c::stateClose() {
    if (!dPostOffice::isRecycleSyncDone()) {
        return NOT_READY;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x45C
int dMenuItem05_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    int cursor = mCursor;
    if (cursor != CURSOR_NONE && prev != cursor) {
        playSeCursor();
    }
    mWin.calcIcons();
    calcAnm();
    calcHold();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    return result;
}

// .text:0x508
int dMenuItem05_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mWin.draw();
        mLyt.entry();
        drawIcons();
        if (isFlag(FLAG_HOLD) || isFlag(FLAG_DROP)) {
            mWin.drawHeldItem();
        }
    }
    return SUCCEEDED;
}

// .text:0x58C
void dMenuItem05_c::subStateMain() {
    hideTutorial();
    setSelect(mCursor, FALSE);
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        mCursor = CURSOR_CLOSE;
        pushClose();
    } else {
        int cursor = hitCheckPick();
        mCursor = cursor;
        if (cursor != CURSOR_NONE) {
            setSelect(cursor, TRUE);
            showTutorial(mCursor);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                if (pickItem()) {
                    setSubState(SUB_HOLD);
                } else {
                    pushClose();
                }
            }
        }
    }
}

// .text:0x68C
void dMenuItem05_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x6C8
void dMenuItem05_c::subStateHold() {
    hideTutorial();
    setSelect(mCursor, FALSE);
    int cursor = hitCheckDrop();
    mCursor = cursor;
    if (cursor != CURSOR_NONE) {
        setSelect(cursor, TRUE);
        showTutorial(mCursor);
    }
    if (!(mPad::g_currentCore->down(WPAD_BUTTON_A) &&
          dObjCursor_c::isVisible())) {
        dropItem();
    }
}

// .text:0x76C
void dMenuItem05_c::subStateDrop() {
    if (mWin.calcMove()) {
        setSlotItem(mHeldSlot, mHeldItem);
        mHeldItem = dItem::ITEM_ID_NONE;
        playSeItemDrop();
        offFlag(FLAG_DROP);
        setSubState(SUB_MAIN);
    }
}

// .text:0x7F0
void dMenuItem05_c::executeSubState() {
    typedef void (dMenuItem05_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuItem05_c::subStateMain,
        &dMenuItem05_c::subStatePush,
        &dMenuItem05_c::subStateHold,
        &dMenuItem05_c::subStateDrop,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x828
void dMenuItem05_c::setSelect(int cursor, BOOL select) {
    if (cursor == CURSOR_CLOSE) {
        mAnm.start(select == 0);
        mAnm.setToEnd();
        onFlag(FLAG_CALC);
    } else if (isPocket(cursor)) {
        mWin.setMark(cursor, select);
    } else if (isLost(cursor)) {
        dMenuItem::Icon_c *icon = &mIcons[cursor - CURSOR_LOST];
        icon->setMark(select);
        if (select) {
            icon->calcLayouts();
        }
    }
}

// .text:0x918
int dMenuItem05_c::hitCheck(bool pick) {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    mVec2_c ofs;
    mVec2_c btnPos = pos;
    int btn = m2d::hitButton(&btnPos, &mLyt, CURSOR_CLOSE, CURSOR_CLOSE,
                             CURSOR_LOST, CURSOR_LOST_LAST, 1.0f);
    if (btn != -1) {
        if (pick && isLost(btn)) {
            const mVec2_c &slotPos = getSlotPos(btn, 0.0f, 0.0f);
            f32 sy = slotPos.y;
            f32 sx = slotPos.x;
            ofs.x = sx - pos.x;
            ofs.y = sy - pos.y;
            mWin.setMoveOffset(ofs);
        }
        return btn;
    }
    mVec2_c pocketPos = pos;
    int pocket = mWin.hitPocket(&pocketPos, pick, 1.0f);
    if (pocket != -1) {
        return pocket;
    } else {
        return CURSOR_NONE;
    }
}

// .text:0xA60
int dMenuItem05_c::hitCheckPick() {
    int cursor = hitCheck(TRUE);
    if (isPocket(cursor) || isLost(cursor)) {
        if (getItem(cursor) == dItem::ITEM_ID_NONE) {
            return CURSOR_NONE;
        }
    }
    return cursor;
}

// .text:0xB00
int dMenuItem05_c::hitCheckDrop() {
    int cursor = hitCheck(FALSE);
    if (isPocket(cursor) || isLost(cursor)) {
        if (isLocked(cursor)) {
            return CURSOR_NONE;
        }
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0xBA4
void dMenuItem05_c::pushClose() {
    if (mCursor == CURSOR_CLOSE) {
        mAnm.setNext(ANM_BTN_PUSH);
        playSeCancel();
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xC08
BOOL dMenuItem05_c::isPushEnd() {
    if (mCursor == CURSOR_CLOSE) {
        return mAnm.isEnd();
    }
    return TRUE;
}

// .text:0xC24
void dMenuItem05_c::finishPush() {
    if (mCursor == CURSOR_CLOSE) {
        startClose();
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xC48
void dMenuItem05_c::startClose() {
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_OUT, STATE_CLOSE, TRUE);
    mWin.startAnm(dMenuItem::Window_c::ANM_CHEST_OUT);
    playSeSubMenuClose();
    dMenuMng_c::setResult(dMenuMng_c::RESULT_OK);
    if (mMenuNo == dMenuMng_c::MENU_LOST_AND_FOUND) {
        mLostNum -= dSaveData_c::getTown()->mPoliceBox.count();
        dMenuMng_c::setLostTakenNum(mLostNum);
        if (mLostNum > 0) {
            for (int i = 0; i <= CURSOR_POCKET_LAST; i++) {
                if (!isLocked(i) && getItem(i) != dItem::ITEM_ID_NONE) {
                    dMenuMng_c::setLostTakenItem(getItem(i));
                    break;
                }
            }
        }
    }
    if (mMenuNo == dMenuMng_c::MENU_RECYCLE_BOX) {
        dPostOffice::broadcastRecycleBin();
    }
    hideTutorial();
}

// .text:0xD50
void dMenuItem05_c::initAnm() {
    static const char *names[3] = {
        "gather_win_00_squareBtn_In.brlan",
        "gather_win_00_squareBtn_Out.brlan",
        "gather_win_00_squareBtn_Psh.brlan",
    };
    mAnm.init(&mLyt, &mRes, names, 3);
    mAnm.start(ANM_BTN_OUT);
    mAnm.setToEnd();
}

// .text:0xDA8
void dMenuItem05_c::calcAnm() {
    if (mAnm.play()) {
        onFlag(FLAG_CALC);
    }
    mWin.calcAnm();
}

// .text:0xDF4
void dMenuItem05_c::initIcons() {
    for (int i = 0; i < LOST_NUM; i++) {
        mIcons[i].create(mWin.getIconRes(), mWin.getItemAnm());
    }
    setIcons();
    initLocks();
}

// .text:0xE80
int dMenuItem05_c::positionIcon(nw4r::lyt::Pane *pane, void *arg) {
    dMenuItem::Icon_c *icons = static_cast<dMenuItem::Icon_c *>(arg);
    int idx = m2d::getNameNumber(pane->GetName(), 2);
    if (idx >= 0 && idx < LOST_NUM) {
        mVec3_c pos;
        m2d::getScreenPos(&pos, pane);
        icons[idx].setRootPos(&pos);
    }
    return 0;
}

// .text:0xEFC
void dMenuItem05_c::positionIcons() {
    mLyt.patrolPane(positionIcon, m2d::isCircleButton, mIcons);
    for (int i = 0; i < LOST_NUM; i++) {
        mIcons[i].calcLayouts();
    }
}

// .text:0xF68
void dMenuItem05_c::drawIcons() {
    for (int i = 0; i < LOST_NUM; i++) {
        mIcons[i].drawLayouts();
    }
}

// .text:0xFB4
void dMenuItem05_c::setIcons() {
    for (int i = 0; i < LOST_NUM; i++) {
        mIcons[i].setItem(getItem(CURSOR_LOST + i), 0, -1);
    }
}

// .text:0x1028
const mVec2_c &dMenuItem05_c::getSlotPos(int slot, f32 ofsX, f32 ofsY) {
    static mVec2_c pos(0.0f, 0.0f);
    if (SCGetAspectRatio() == SC_ASPECT_RATIO_16x9) {
        ofsX *= 0.730769f;
    }
    mVec3_c base;
    if (isPocket(slot)) {
        base = mWin.getPocketPos(slot);
        pos.x = base.x + ofsX;
        pos.y = base.y + ofsY;
        return pos;
    } else if (isLost(slot)) {
        base = mIcons[slot - CURSOR_LOST].getRootPos();
        pos.x = base.x + ofsX;
        pos.y = base.y + ofsY;
        return pos;
    }
    return pos;
}

// .text:0x11B4
bool dMenuItem05_c::isLocked(int slot) {
    if (isPocket(slot)) {
        u16 bit = 1 << slot;
        return (mLockMask & bit) != 0;
    }
    return false;
}

// .text:0x1200
void dMenuItem05_c::setLocked(int slot, BOOL locked) {
    u16 bit = 1 << slot;
    mWin.setNoChoice(slot, locked);
    if (locked) {
        mLockMask |= bit;
    } else {
        mLockMask &= ~bit;
    }
}

// .text:0x1280
void dMenuItem05_c::initLocks() {
    mLockMask = 0;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    for (int i = 0; i < POCKET_NUM; i++) {
        dItem::Item *pocket = &player->mPockets[i];
        BOOL locked = FALSE;
        if (mMenuNo == dMenuMng_c::MENU_LOST_AND_FOUND) {
            if (pocket->mId != dItem::ITEM_ID_NONE) {
                locked = TRUE;
            }
        } else if (player->getPocketFlag(i)) {
            locked = TRUE;
        } else {
            int kind = pocket->getKind();
            if (kind == dItem::KIND_INSECT || kind == dItem::KIND_FISH) {
                locked = TRUE;
            }
        }
        setLocked(i, locked);
    }
}

// .text:0x1340
u16 dMenuItem05_c::setSlotItem(int slot, dItem::Item item) {
    if (isPocket(slot)) {
        return mWin.setPocket(slot, item, 0);
    }
    if (isLost(slot)) {
        dPoliceBox_c *box;
        dRecycleBin_c *bin;
        int lost = slot - CURSOR_LOST;
        u16 prev;
        dSaveTown_c *town = dSaveData_c::getTown();
        if (mMenuNo == dMenuMng_c::MENU_LOST_AND_FOUND) {
            box = &town->mPoliceBox;
            prev = box->get(lost);
            box->set(lost, item.mId);
        } else {
            bin = &town->mRecycleBin;
            prev = bin->get(lost);
            bin->set(lost, item.mId);
        }
        mIcons[lost].setItem(item, 0, -1);
        return prev;
    }
    return dItem::ITEM_ID_NONE;
}

// .text:0x146C
u16 dMenuItem05_c::putItem(int slot, dItem::Item item) {
    dItem::Item cur = getItem(slot);
    dItem::Item src = item;
    if (dMenuItem::combineItems(cur, src)) {
        setSlotItem(slot, cur);
        return src.mId;
    }
    return setSlotItem(slot, item);
}

// .text:0x1508
u16 dMenuItem05_c::removeItem(int slot) {
    return setSlotItem(slot, dItem::ITEM_ID_NONE);
}

// .text:0x1538
u16 dMenuItem05_c::getItem(int slot) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (isPocket(slot)) {
        return player->mPockets[slot].mId;
    }
    if (isLost(slot)) {
        int lost = slot - CURSOR_LOST;
        dSaveTown_c *town = dSaveData_c::getTown();
        if (mMenuNo == dMenuMng_c::MENU_LOST_AND_FOUND) {
            return town->mPoliceBox.get(lost);
        }
        return town->mRecycleBin.get(lost);
    }
    return dItem::ITEM_ID_NONE;
}

// .text:0x1604
BOOL dMenuItem05_c::pickItem() {
    if (getItem(mCursor) == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    if (isLocked(mCursor)) {
        return FALSE;
    }
    mHeldSlot = mCursor;
    mHeldItem = removeItem(mCursor);
    mWin.setHeldItem(mHeldItem, 0);
    onFlag(FLAG_HOLD);
    mWin.startFollow();
    playSeItemLift();
    dObjCursor_c::hold();
    hideTutorial();
    return TRUE;
}

// .text:0x16B0
void dMenuItem05_c::calcHold() {
    if (isFlag(FLAG_HOLD)) {
        mWin.calcFollow();
        mWin.calcHeldItem();
    } else if (isFlag(FLAG_DROP)) {
        mWin.calcHeldItem();
    }
}

// .text:0x1718
void dMenuItem05_c::dropItem() {
    offFlag(FLAG_HOLD);
    dObjCursor_c::show();
    hideTutorial();
    if (mCursor == CURSOR_NONE) {
        int slot = findFreeSlot();
        if (slot != -1) {
            mHeldSlot = slot;
        }
        mWin.startMove(getSlotPos(mHeldSlot, 0.0f, 0.0f));
        setSubState(SUB_DROP);
        onFlag(FLAG_DROP);
    } else {
        playSeItemDrop();
        dItem::Item left = putItem(mCursor, mHeldItem);
        if (!(left == dItem::ITEM_ID_NONE)) {
            setSlotItem(mHeldSlot, left);
            mHeldItem = dItem::ITEM_ID_NONE;
        }
        setSubState(SUB_MAIN);
    }
}

// .text:0x1820
void dMenuItem05_c::getItemName(dScript::Word_c *word, int slot) {
    word->clear();
    if (isPocket(slot)) {
        mWin.getPocketName(word, slot);
    } else if (isLost(slot)) {
        dItem::name_c name(getItem(slot));
        word->copy(&name, 0);
    }
}

// .text:0x18F4
void dMenuItem05_c::showTutorial(int slot) {
    bool tail = false;
    if (getItem(slot) != dItem::ITEM_ID_NONE) {
        if (isPocket(slot) && slot >= TUTORIAL_TAIL_POCKET) {
            tail = true;
        }
        if (dObjTutorial_c::request(TUTORIAL_REQUEST_ITEM, slot, FALSE)) {
            mVec2_c pos = getSlotPos(slot, 0.0f, 0.0f);
            m2d::stringWord_c word;
            getItemName(&word, slot);
            dObjTutorial_c::setWord(&pos, &word, tail);
        }
    } else {
        hideTutorial();
    }
}

// .text:0x19F0
void dMenuItem05_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x19F4
int dMenuItem05_c::findNearestEmptyPocket() {
    mVec2_c cursor = dObjCursor_c::getPos();
    int nearest = -1;
    f32 nearestDist = 1000000.0f;
    for (int i = 0; i < POCKET_NUM; i++) {
        if (getItem(i) == dItem::ITEM_ID_NONE) {
            mVec2_c pos;
            pos = getSlotPos(i, 0.0f, 0.0f);
            f32 dx = pos.x - cursor.x;
            f32 dy = pos.y - cursor.y;
            f32 dist = dx * dx + dy * dy;
            if (dist < nearestDist) {
                nearestDist = dist;
                nearest = i;
            }
        }
    }
    return nearest;
}

// .text:0x1AD8
int dMenuItem05_c::findNearestEmptyLost() {
    mVec2_c cursor = dObjCursor_c::getPos();
    int nearest = -1;
    f32 nearestDist = 1000000.0f;
    for (int i = 0; i < LOST_NUM; i++) {
        if (getItem(CURSOR_LOST + i) == dItem::ITEM_ID_NONE) {
            mVec2_c pos;
            pos = getSlotPos(CURSOR_LOST + i, 0.0f, 0.0f);
            f32 dx = pos.x - cursor.x;
            f32 dy = pos.y - cursor.y;
            f32 dist = dx * dx + dy * dy;
            if (dist < nearestDist) {
                nearestDist = dist;
                nearest = CURSOR_LOST + i;
            }
        }
    }
    return nearest;
}

// .text:0x1BBC
int dMenuItem05_c::findFreeSlot() {
    mVec2_c cursor = dObjCursor_c::getPos();
    if (isPocket(mHeldSlot)) {
        if (cursor.y < 50.0f) {
            return -1;
        }
        return findNearestEmptyLost();
    }
    if (isLost(mHeldSlot)) {
        if (cursor.y > 30.0f) {
            return -1;
        }
        return findNearestEmptyPocket();
    }
    return -1;
}
