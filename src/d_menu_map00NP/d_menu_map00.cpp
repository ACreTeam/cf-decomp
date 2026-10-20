#include <game/game/d_menu_map00.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_pad.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuMap00 {
// .text:0x5C
BASE_PROFILE(MENU_MAP00, dMenuMap00_c)
} // namespace dMenuMap00

// .text:0x260
int dMenuMap00_c::create() {
    if (!load()) {
        return NOT_READY;
    }
    createLayouts();
    startAnm(&mWinLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    mWinLyt.calc();
    return SUCCEEDED;
}

// .text:0x2DC
int dMenuMap00_c::doDelete() {
    return remove();
}

// .text:0x2E0
int dMenuMap00_c::stateMain() {
    if (checkCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    return SUCCEEDED;
}

// .text:0x328
void dMenuMap00_c::subStateMain() {
    if (mCursor != SLOT_NONE) {
        setSlotAnm(mCursor, FALSE);
        mCursor = SLOT_NONE;
    }
    int tab = dAcUitag::getTabInput(dAcUitag_c::TAB_MAP, FALSE);
    if (tab != -1 && selectTab(tab + SLOT_TAB, TRUE)) {
        return;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    mCursor = getTabAt(&pos);
    if (mCursor != SLOT_NONE &&
        mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        selectTab(mCursor, FALSE);
        return;
    }
    if (mCursor == SLOT_NONE) {
        mCursor = hitCheck(&pos);
        if (mCursor != SLOT_NONE) {
            setSlotAnm(mCursor, TRUE);
            if (isButton(mCursor)) {
                showSlotTutorial(mCursor);
            }
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                if (pushSlot(mCursor)) {
                    if (isIcon(mCursor)) {
                        int dir = getIconScrollDir();
                        if (dir != SCROLL_NONE) {
                            startScroll(dir);
                            setSubState(SUB_ICON_SCROLL);
                        }
                    } else {
                        setSubState(SUB_PUSH);
                    }
                }
                return;
            }
        }
    }
    checkPadMove();
}

// .text:0x50C
void dMenuMap00_c::subStatePush() {
    if (!isFlag(FLAG_BUTTON_ANM)) {
        switch (mCursor) {
        case SLOT_HOUSE_BUTTON:
            changeList(FALSE);
            break;
        case SLOT_FACILITY_BUTTON:
            changeList(TRUE);
            break;
        case SLOT_SCROLL_UP:
            startScroll(SCROLL_PREV);
            setSubState(SUB_SCROLL);
            return;
        case SLOT_SCROLL_DOWN:
            startScroll(SCROLL_NEXT);
            setSubState(SUB_SCROLL);
            return;
        }
        setSubState(SUB_MAIN);
    }
}

// .text:0x5F8
void dMenuMap00_c::subStateScroll() {
    if (!isFlag(FLAG_SCROLL)) {
        updateScrollButtons();
        if (mPad::g_currentCore->down(WPAD_BUTTON_A) ||
            (mCursor == SLOT_SCROLL_UP && dPad::isUpHeld()) ||
            (mCursor == SLOT_SCROLL_DOWN && dPad::isDownHeld())) {
            switch (mCursor) {
            case SLOT_SCROLL_UP:
                if (canScrollPrev()) {
                    startScroll(SCROLL_PREV);
                    playSePageTurn();
                    return;
                }
                break;
            case SLOT_SCROLL_DOWN:
                if (canScrollNext()) {
                    startScroll(SCROLL_NEXT);
                    playSePageTurn();
                    return;
                }
                break;
            }
        }
        setSubState(SUB_MAIN);
    }
}

// .text:0x708
void dMenuMap00_c::subStateIconScroll() {
    if (!isFlag(FLAG_SCROLL)) {
        updateScrollButtons();
        int dir = getIconScrollDir();
        if (dir != SCROLL_NONE) {
            startScroll(dir);
        } else {
            setSubState(SUB_MAIN);
        }
    }
}

// .text:0x780
BOOL dMenuMap00_c::closeMenu() {
    dAcUitag::closeTab();
    startClose();
    return TRUE;
}

// .text:0x7B8
BOOL dMenuMap00_c::checkCloseRequest() {
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    static BOOL (dMenuMap00_c::*closeProcs[SUB_NUM])() = {
        &dMenuMap00_c::closeMenu, nullptr, &dMenuMap00_c::closeMenu, nullptr};
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL result = (this->*closeProcs[mSubState])();
    if (result) {
        dMenuMng_c::clearCloseRequest();
    }
    return result;
}

// .text:0x8A4
void dMenuMap00_c::executeSubState() {
    typedef void (dMenuMap00_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuMap00_c::subStateMain, &dMenuMap00_c::subStatePush,
        &dMenuMap00_c::subStateScroll, &dMenuMap00_c::subStateIconScroll};
    (this->*subStateProcs[mSubState])();
}

// .text:0x8DC
int dMenuMap00_c::getTabAt(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = -1;
    }
    dAcUitag::setTabFocus(tab);
    if (tab == -1) {
        return SLOT_NONE;
    }
    return tab + SLOT_TAB;
}

// .text:0x940
BOOL dMenuMap00_c::selectTab(int slot, BOOL delay) {
    if (dAcUitag::selectTab(slot - SLOT_TAB, delay)) {
        startClose();
        return TRUE;
    }
    return FALSE;
}

// .text:0x994
void dMenuMap00_c::startClose() {
    startAnm(&mWinLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, FALSE);
}

// .text:0x9B0
void dMenuMap00_c::checkPadMove() {
    if (dPad::isUpTriggered() && pushSlot(SLOT_SCROLL_UP)) {
        mCursor = SLOT_SCROLL_UP;
        setSubState(SUB_PUSH);
    } else if (dPad::isDownTriggered() && pushSlot(SLOT_SCROLL_DOWN)) {
        mCursor = SLOT_SCROLL_DOWN;
        setSubState(SUB_PUSH);
    } else if (dPad::isLeftTriggered() && canScrollPrev()) {
        mScroll -= SCROLL_ROW_NUM;
        if (mScroll < 0) {
            mScroll = 0;
        }
        refreshList();
        updateScrollButtons();
        playSeDecide();
    } else if (dPad::isRightTriggered() && canScrollNext()) {
        mScroll += SCROLL_ROW_NUM;
        int max = getListNum() - SCROLL_ROW_NUM;
        if (max < 0) {
            max = 0;
        }
        if (mScroll > max) {
            mScroll = max;
        }
        refreshList();
        updateScrollButtons();
        playSeDecide();
    }
}
