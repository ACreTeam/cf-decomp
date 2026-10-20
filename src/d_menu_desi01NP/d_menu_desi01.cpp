#include <game/game/d_menu_desi01.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuDesi01 {
// .text:0x5C
BASE_PROFILE(MENU_DESI01, dMenuDesi01_c)
} // namespace dMenuDesi01

static const char *l_allPanes[] = {"N_slctD_Win_all", nullptr};

// .text:0x2E0
int dMenuDesi01_c::create() {
    BOOL loaded = mDesi.load();
    loaded &= mBtn.load();
    loaded &= mTitle.load();
    if (!loaded) {
        return NOT_READY;
    }
    mDesi.create(FALSE);
    mBtn.create();
    initButtons();
    hideTutorial();
    mTitle.create();
    dMenuMng_c::clearPickedShirt();
    dMenuMng_c::clearPickedHat();
    switch (mMenuNo) {
    case dMenuMng_c::MENU_DESIGN_TAKE:
        mTitle.setIcon(ICON_DESIGN);
        mTitle.setMessage(MSG_PUT_WHERE);
        break;
    case dMenuMng_c::MENU_DESIGN_DISPLAY:
        mTitle.setIcon(ICON_DESIGN);
        mTitle.setMessage(MSG_DISPLAY_WHICH);
        break;
    case dMenuMng_c::MENU_DESIGN_SWAP:
        mTitle.setIcon(ICON_DESIGN);
        mTitle.setMessage(MSG_SWAP_WITH);
        break;
    case dMenuMng_c::MENU_DESIGN_PRO:
        mTitle.setIcon(ICON_DESIGN);
        mTitle.setMessage(MSG_PUT_WHERE);
        break;
    case dMenuMng_c::MENU_DESIGN_GIFT:
        mTitle.setIcon(ICON_GIFT);
        mTitle.setMessage(MSG_PUT_WHERE_GIFT);
        break;
    }
    onFlag(FLAG_DRAW);
    startAnm(mDesi.getLayout(), l_allPanes[0], ANM_WIN_IN, STATE_MAIN, TRUE);
    mTitle.startAnm(ANM_UPPER_WIN_IN);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x490
int dMenuDesi01_c::doDelete() {
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (mDesi.remove() == FALSE) {
        return NOT_READY;
    }
    return mTitle.remove() != 0;
}

// .text:0x4FC
int dMenuDesi01_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x520
int dMenuDesi01_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(mDesi.getLayout(), l_allPanes[0]);
    return result;
}

// .text:0x578
int dMenuDesi01_c::stateClose() {
    if (mMenuNo == dMenuMng_c::MENU_DESIGN_PRO &&
        lbl_8074EA7D == RESULT_CONFIRM && !dAcUitag::openMenuB(7)) {
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x5E0
int dMenuDesi01_c::execute() {
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prev) {
        playSeCursor();
    }
    mBtn.execute();
    mTitle.calcAnm();
    mDesi.calc();
    return result;
}

// .text:0x658
int dMenuDesi01_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mDesi.draw();
        mTitle.draw();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x6AC
void dMenuDesi01_c::subMain() {
    setSelect(mCursor, FALSE);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        mCursor = btn;
        setSelect(btn, TRUE);
        pushCursor();
        return;
    }
    mCursor = hitCheck();
    showTutorial(mCursor);
    if (mCursor != CURSOR_NONE) {
        setSelect(mCursor, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            pushCursor();
        }
    }
}

// .text:0x774
void dMenuDesi01_c::subPush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x7B0
void dMenuDesi01_c::executeSubState() {
    typedef void (dMenuDesi01_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {&dMenuDesi01_c::subMain,
                                          &dMenuDesi01_c::subPush};
    (this->*subStateProcs[mSubState])();
}

// .text:0x7E8
void dMenuDesi01_c::setSelect(int cursor, BOOL select) {
    if (isButton(cursor)) {
        mBtn.setSelect(cursor - CURSOR_BTN, select, FALSE);
    }
    if (isDesign(cursor)) {
        if (mDesi.setSelection(cursor - CURSOR_DESIGN, select != 0)) {
            onFlag(FLAG_SELECTED);
        }
    }
}

// .text:0x8A8
int dMenuDesi01_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn + CURSOR_BTN;
    }
    int slot = mDesi.hitDesign(mVec2_c(pos), FALSE, 1.0f);
    if (slot != -1) {
        return slot + CURSOR_DESIGN;
    }
    return CURSOR_NONE;
}

// .text:0x940
void dMenuDesi01_c::pushCursor() {
    int cursor = mCursor;
    if (isButton(cursor)) {
        mBtn.push(cursor - CURSOR_BTN, FALSE);
    } else if (isDesign(cursor)) {
        if (cursor - CURSOR_DESIGN == mDesi.getPreviousSelection()) {
            return;
        }
        playSeDecide2();
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xA00
BOOL dMenuDesi01_c::isPushEnd() {
    if (isButton(mCursor)) {
        return !mBtn.isPushing();
    }
    return TRUE;
}

// .text:0xA58
void dMenuDesi01_c::finishPush() {
    int cursor = mCursor;
    if (isButton(cursor)) {
        switch (cursor - CURSOR_BTN) {
        case BTN_CANCEL:
            startCloseAnm();
            break;
        case BTN_UNUSED:
            break;
        case BTN_CONFIRM:
            startCloseAnm();
            dMenuMng_c::setResult(RESULT_CONFIRM);
            dMenuMng_c::setDesignIndex(mDesi.getPreviousSelection());
            int slot = mDesi.getPreviousSelection();
            dMenuMng_c::setPickedDesign(getOrder(slot));
            dMenuMng_c::setDesign(getDesign(mDesi.getPreviousSelection()));
            if (mMenuNo == dMenuMng_c::MENU_DESIGN_PRO) {
                dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
                dItem::Item hat = player->mOrgDesigns.getDesignItem(
                    (u8)slot, dItem::KIND_ORG_CAP);
                if (player->mEquipment.mHat.isSame(hat)) {
                    dMenuMng_c::setPickedHat();
                }
                dItem::Item shirt = player->mOrgDesigns.getDesignItem(
                    (u8)slot, dItem::KIND_ORG_CLOTH);
                if (player->mEquipment.mShirt.isSame(shirt)) {
                    dMenuMng_c::setPickedShirt();
                }
            }
            break;
        }
    } else if (isDesign(cursor)) {
        selectDesign(cursor - CURSOR_DESIGN);
        setSubState(SUB_MAIN);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xC00
void dMenuDesi01_c::startCloseAnm() {
    startAnm(mDesi.getLayout(), l_allPanes[0], ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(RESULT_CANCEL);
    hideTutorial();
    mTitle.startAnm(ANM_UPPER_WIN_OUT);
}

// .text:0xC6C
void dMenuDesi01_c::initButtons() {
    mBtn.setText(BTN_CANCEL, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
    mBtn.setDisable(BTN_CONFIRM, TRUE);
}

// .text:0xCDC
dDesign_c *dMenuDesi01_c::getDesign(int slot) {
    return mDesi.getDesign(slot);
}

// .text:0xCE4
int dMenuDesi01_c::getOrder(int slot) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mOrgDesigns.getOrder(
        (u8)slot);
}

// .text:0xD20
void dMenuDesi01_c::selectDesign(int slot) {
    mDesi.setPreviousSelection(slot);
    onFlag(FLAG_SELECTED);
    mBtn.setDisable(BTN_CONFIRM, FALSE);
}

// .text:0xD6C
void dMenuDesi01_c::getSlotName(dScript::Word_c *word, int slot) {
    word->clear();
    if (isDesign(slot)) {
        word->set(getDesign(slot - CURSOR_DESIGN)->mName, 0);
    }
}

// .text:0xDF4
void dMenuDesi01_c::showTutorial(int cursor) {
    if (isDesign(cursor)) {
        if (dObjTutorial_c::request(TUTORIAL_DESIGN, cursor, 0)) {
            mVec2_c balloon = getSlotPos(cursor, 0.0f, 0.0f);
            m2d::stringWord_c word;
            getSlotName(&word, cursor);
            dObjTutorial_c::setWord(&balloon, &word, 0);
        }
    } else {
        hideTutorial();
    }
}

// .text:0xEBC
void dMenuDesi01_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0xEC0
mVec2_c dMenuDesi01_c::getSlotPos(int cursor, f32 x, f32 y) {
    mVec2_c pos(0.0f, 0.0f);
    if (!isDesign(cursor)) {
        return pos;
    }
    return mDesi.getDesignPos(cursor - CURSOR_DESIGN, x, y);
}
