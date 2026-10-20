#include <game/game/d_menu_desi03.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pattern_copy.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuDesi03 {
// .text:0x5C
BASE_PROFILE(MENU_DESI03, dMenuDesi03_c)
} // namespace dMenuDesi03

static const char *l_allPane = "N_slctD_Win_all";

// .text:0x300
int dMenuDesi03_c::create() {
    BOOL loaded = mDesi.load();
    loaded &= mBtn.load();
    loaded &= mTitle.load();
    loaded &= mSelect.load();
    if (!loaded) {
        return NOT_READY;
    }
    mDesi.create(FALSE);
    mBtn.create();
    initButtons();
    hideTutorial();
    mSelect.build();
    mTitle.create();
    switch (mMenuNo) {
    case dMenuMng_c::MENU_TOWN_FLAG_DESIGN:
        mTitle.setIcon(TITLE_ICON_DESIGN);
        mTitle.setMessage(MSG_WHICH_DESIGN);
        break;
    case dMenuMng_c::MENU_FLAG_DESIGN:
        mTitle.setIcon(TITLE_ICON_DESIGN);
        mTitle.setMessage(MSG_WHICH_DESIGN);
        break;
    }
    onFlag(FLAG_DRAW);
    startAnm(mDesi.getLayout(), l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    mTitle.startAnm(ANM_UPPER_WIN_IN);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x44C
int dMenuDesi03_c::doDelete() {
    if (mSelect.remove() == FALSE) {
        return NOT_READY;
    }
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    if (mDesi.remove() == FALSE) {
        return NOT_READY;
    }
    return mTitle.remove() != 0;
}

// .text:0x4D0
int dMenuDesi03_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x4F4
int dMenuDesi03_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(mDesi.getLayout(), l_allPane);
    return result;
}

// .text:0x54C
int dMenuDesi03_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x570
int dMenuDesi03_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != SLOT_NONE && mCursor != prevCursor) {
        playSeCursor();
    }
    mBtn.execute();
    mTitle.calcAnm();
    mSelect.update();
    mDesi.calc();
    return result;
}

// .text:0x5F0
int dMenuDesi03_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mDesi.draw();
        mTitle.draw();
        mBtn.draw();
    }
    mSelect.draw();
    return SUCCEEDED;
}

// .text:0x64C
void dMenuDesi03_c::subMain() {
    setHighlight(mCursor, FALSE);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != SLOT_NONE) {
        mCursor = btn;
        setHighlight(btn, TRUE);
        push();
    } else {
        mCursor = hitCheck();
        showSlotTutorial(mCursor);
        if (mCursor != SLOT_NONE) {
            setHighlight(mCursor, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                push();
            }
        }
    }
}

// .text:0x714
void dMenuDesi03_c::subWaitPush() {
    if (isPushDone()) {
        endPush();
    }
}

// .text:0x750
void dMenuDesi03_c::subSelect() {
    if (static_cast<dSelect::BaseComp_c &>(mSelect).isClosed()) {
        u16 code = mSelect.mCode;
        mAnswer = code;
        if (code != ANSWER_CANCEL) {
            hideTutorial();
        }
        switch (mAnswer) {
        default:
            endSub();
            break;
        case ANSWER_SWAP:
            swapDesign();
            break;
        case ANSWER_COPY:
            copyDesign();
            break;
        }
    }
}

// .text:0x7E4
void dMenuDesi03_c::executeSubState() {
    static void (dMenuDesi03_c::*l_sub[SUB_NUM])() = {
        &dMenuDesi03_c::subMain, &dMenuDesi03_c::subWaitPush,
        &dMenuDesi03_c::subSelect};
    (this->*l_sub[mSubState])();
}

// .text:0x81C
void dMenuDesi03_c::setHighlight(int slot, BOOL on) {
    if (isBtn(slot)) {
        mBtn.setSelect(slot, on, FALSE);
    }
    if (isDesign(slot)) {
        if (mDesi.setSelection(slot - SLOT_DESIGN, on != 0)) {
            onFlag(FLAG_SELECTED);
        }
    }
}

// .text:0x8DC
int dMenuDesi03_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int slot = mBtn.hitCheck(&pos);
    if (slot != SLOT_NONE) {
        return slot;
    }
    slot = mDesi.hitDesign(mVec2_c(pos), FALSE, 1.0f);
    if (slot != SLOT_NONE) {
        return slot + SLOT_DESIGN;
    }
    return SLOT_NONE;
}

// .text:0x974
void dMenuDesi03_c::push() {
    if (isBtn(mCursor)) {
        mBtn.push(mCursor, FALSE);
    } else if (isDesign(mCursor)) {
        buildSelect();
        openSelect(mCursor);
        setSubState(SUB_SELECT);
        return;
    } else {
        return;
    }
    setSubState(SUB_WAIT_PUSH);
}

// .text:0xA3C
BOOL dMenuDesi03_c::isPushDone() {
    if (isBtn(mCursor)) {
        return !mBtn.isPushing();
    }
    return TRUE;
}

// .text:0xA94
void dMenuDesi03_c::endPush() {
    if (isBtn(mCursor)) {
        switch (mCursor) {
        case BTN_QUIT:
            closeMenu();
            return;
        }
        return;
    } else if (isDesign(mCursor)) {
        setSubState(SUB_MAIN);
        return;
    } else {
        setSubState(SUB_MAIN);
        return;
    }
}

// .text:0xB18
void dMenuDesi03_c::closeMenu() {
    startAnm(mDesi.getLayout(), l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(0);
    hideTutorial();
    mTitle.startAnm(ANM_UPPER_WIN_OUT);
}

// .text:0xB84
void dMenuDesi03_c::initButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED_1, false);
    mBtn.setVisible(BTN_UNUSED_2, false);
}

// .text:0xBE0
dDesign_c *dMenuDesi03_c::getDesign(int index) {
    return mDesi.getDesign(index);
}

// .text:0xBE8
int dMenuDesi03_c::getOrder(int index) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mOrgDesigns.getOrder(
        (u8)index);
}

// .text:0xC24
void dMenuDesi03_c::getSlotName(dScript::Word_c *word, int slot) {
    word->clear();
    if (isDesign(slot)) {
        word->set(getDesign(slot - SLOT_DESIGN)->mName, 0);
    }
}

// .text:0xCAC
void dMenuDesi03_c::showSlotTutorial(int slot) {
    if (isDesign(slot)) {
        if (dObjTutorial_c::request(TUTORIAL_DESIGN_NAME, slot, 0)) {
            mVec2_c balloon = getSlotPos(slot, 0.0f, 0.0f);
            m2d::stringWord_c word;
            getSlotName(&word, slot);
            dObjTutorial_c::setWord(&balloon, &word, 0);
        }
    } else {
        hideTutorial();
    }
}

// .text:0xD74
void dMenuDesi03_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0xD78
mVec2_c dMenuDesi03_c::getSlotPos(int slot, f32 x, f32 y) {
    mVec2_c pos(0.0f, 0.0f);
    if (!isDesign(slot)) {
        return pos;
    }
    return mDesi.getDesignPos(slot - SLOT_DESIGN, x, y);
}

// .text:0xDE4
void dMenuDesi03_c::openSelect(int slot) {
    mVec2_c pos = getSlotPos(slot, 10.0f, -50.0f);
    mSelect.placeAbove(pos);
    mSelect.openPointer();
    mDesign = slot;
}

// .text:0xFA8
void dMenuDesi03_c::endSub() {
    setSubState(SUB_MAIN);
}

// .text:0xFBC
void dMenuDesi03_c::swapDesign() {
    closeMenu();
    dMenuMng_c::setResult(1);
    int order = getOrder(mDesign - SLOT_DESIGN);
    int kind = mMenuNo == dMenuMng_c::MENU_TOWN_FLAG_DESIGN
                   ? dPatternCopy::PATTERN_TOWN_FLAG
                   : dPatternCopy::PATTERN_HOME;
    dPatternCopy::swap(dPatternCopy::PATTERN_MY_PLAYER, order, kind, 0, TRUE);
    dMenuMng_c::setPickedDesign(order);
}

// .text:0x1034
void dMenuDesi03_c::copyDesign() {
    closeMenu();
    dMenuMng_c::setResult(1);
    int order = getOrder(mDesign - SLOT_DESIGN);
    int kind = mMenuNo == dMenuMng_c::MENU_TOWN_FLAG_DESIGN
                   ? dPatternCopy::PATTERN_TOWN_FLAG
                   : dPatternCopy::PATTERN_HOME;
    dPatternCopy::copy(dPatternCopy::PATTERN_MY_PLAYER, order, kind, 0, TRUE);
    dMenuMng_c::setPickedDesign(-1);
}

// .text:0x10A8
void dMenuDesi03_c::buildSelect() {
    mSelect.reset();
    mSelect.mNum = 0;
    mSelect.addItem(MSG_DISPLAY_THIS, ANSWER_COPY, 0, 0);
    mSelect.addItem(MSG_SWAP_WITH_THIS, ANSWER_SWAP, 0, 0);
    mSelect.addItem(MSG_NEVER_MIND, ANSWER_CANCEL, 0, 0);
    mSelect.setCancelLast();
}
