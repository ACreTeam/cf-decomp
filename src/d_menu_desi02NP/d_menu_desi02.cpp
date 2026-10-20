#include <game/game/d_menu_desi02.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_dsn.hpp>
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
#include <game/snd/snd_id.hpp>
#include <stdio.h>
#include <revolution/SC/scapi.h>

namespace dMenuDesi02 {
// .text:0x5C
BASE_PROFILE(MENU_DESI02, dMenuDesi02_c)
} // namespace dMenuDesi02

static const char *l_allPane = "N_all";
static const char *l_frame = "myDsgnFc";

// .text:0x30C
int dMenuDesi02_c::create() {
    BOOL loaded = mRes.load("Layout/leave_design.arc");
    loaded &= mDesi.load();
    loaded &= mDialog.load();
    loaded &= mSelect.load();
    if (!loaded) {
        return NOT_READY;
    }
    mDesi.create(TRUE);
    mSelect.build();
    hideTutorial();
    mLyt.build(&mRes, "leave_design_win_00.brlyt");
    mLyt.setMessage("T_btn_00", MSG_CLOSE, NULL);
    initPanes();
    mDialog.create();
    initAnm();
    setPage(0);
    mLyt.calc();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_IN, STATE_MAIN, TRUE);
    mDesi.startLeaveAnm(0);
    playSeSubMenuOpen();
    setSubState(SUB_MAIN);
    dMenuMng_c::clearPickedShirt();
    dMenuMng_c::clearPickedHat();
    return SUCCEEDED;
}

// .text:0x460
int dMenuDesi02_c::doDelete() {
    if (mSelect.remove() == FALSE) {
        return NOT_READY;
    }
    if (mRes.remove() == FALSE) {
        return NOT_READY;
    }
    if (mDesi.remove() == FALSE) {
        return NOT_READY;
    }
    return mDialog.remove() != 0;
}

// .text:0x4E4
int dMenuDesi02_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x508
int dMenuDesi02_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x50C
int dMenuDesi02_c::stateClose() {
    if (isFlag(FLAG_PAGE_TURN) && isPageButton(mCursor)) {
        setPage(mNextPage);
        startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_IN, STATE_MAIN, TRUE);
        updateBoxDesigns();
        offFlag(FLAG_PAGE_TURN);
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x5BC
int dMenuDesi02_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != SLOT_NONE && prevCursor != mCursor) {
        playSeCursor();
    }
    calcAnm();
    mDialog.execute();
    mSelect.update();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    if (calcPickup()) {
        onFlag(FLAG_DESI_CALC);
    }
    mDesi.calc();
    return result;
}

// .text:0x68C
int dMenuDesi02_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mDesi.draw();
        mLyt.entry();
        if (isFlag(FLAG_FOLLOW) || isFlag(FLAG_MOVE) || isFlag(FLAG_PLACED)) {
            mDesi.drawPickup();
        }
    }
    if (isFlag(FLAG_DIALOG)) {
        mDialog.draw();
    }
    mSelect.draw();
    return SUCCEEDED;
}

// .text:0x740
void dMenuDesi02_c::subMain() {
    setHighlight(mCursor, FALSE);
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        mCursor = SLOT_BACK;
        pressButton();
        return;
    }
    mCursor = hitCheck(TRUE);
    showSlotTutorial(mCursor);
    if (mCursor != SLOT_NONE) {
        setHighlight(mCursor, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            if (isDesign(mCursor) || isBox(mCursor)) {
                pickUpDesign(mCursor);
            } else if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                pressButton();
            }
            return;
        }
    }
    checkPadMove();
}

// .text:0x890
void dMenuDesi02_c::subPush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x8CC
void dMenuDesi02_c::subDrag() {
    hideTutorial();
    setHighlight(mCursor, FALSE);
    mCursor = hitCheck(FALSE);
    if (mCursor != SLOT_NONE) {
        if (mHeld != mCursor) {
            showSlotTutorial(mCursor);
        }
        setHighlight(mCursor, TRUE);
    }
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A) ||
        !dObjCursor_c::isVisible()) {
        offFlag(FLAG_FOLLOW);
        dObjCursor_c::show();
        if (isDesign(mCursor) || isBox(mCursor)) {
            if (dropOn(mHeld, mCursor)) {
                dropHeld(mHeld);
                playSeItemDrop();
            } else {
                offFlag(FLAG_FOLLOW);
                onFlag(FLAG_PLACED);
                mDesi.setPickupPos(getSlotPos(mCursor, 15.0f, 15.0f));
            }
        } else {
            mDesi.startMove(getSlotPos(mHeld, 0.0f, 0.0f), FALSE);
            onFlag(FLAG_MOVE);
            setSubState(SUB_MOVE);
        }
    }
}

// .text:0xA88
void dMenuDesi02_c::subMove() {
    if (mDesi.calcMove()) {
        dropHeld(mHeld);
        offFlag(FLAG_MOVE);
    }
}

// .text:0xAD8
void dMenuDesi02_c::subDialog() {
    if (mDialog.isWinAnmEnd() && mDialog.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0xB38
void dMenuDesi02_c::subDialogPush() {
    if (mDialog.isPushEnd()) {
        setSubState(SUB_DIALOG_END);
        mDialog.open(1, 0, 0);
    }
}

// .text:0xB9C
void dMenuDesi02_c::subDialogEnd() {
    if (mDialog.isWinAnmEnd()) {
        switch (mDialog.mPushedBtn) {
        case 0:
            break;
        case 1:
            copyDesign(mHeld, mCursor);
            playSeItemDrop();
            break;
        default:
            break;
        }
        dropHeld(mHeld);
        offFlag(FLAG_DIALOG);
    }
}

// .text:0xC14
void dMenuDesi02_c::subSelect() {
    if (static_cast<dSelect::BComp_c &>(mSelect).isClosed()) {
        mAnswer = mSelect.mCode;
        switch (mAnswer) {
        case ANSWER_SWAP:
            swapDesigns(mHeld, mTarget);
            break;
        case ANSWER_COPY:
            openDialog(MSG_OVERWRITE);
            return;
        case ANSWER_CANCEL:
        default:
            break;
        }
        dropHeld(mHeld);
    }
}

// .text:0xCAC
void dMenuDesi02_c::executeSubState() {
    static void (dMenuDesi02_c::*l_sub[SUB_NUM])() = {
        &dMenuDesi02_c::subMain,      &dMenuDesi02_c::subPush,
        &dMenuDesi02_c::subDrag,      &dMenuDesi02_c::subMove,
        &dMenuDesi02_c::subDialog,    &dMenuDesi02_c::subDialogPush,
        &dMenuDesi02_c::subDialogEnd, &dMenuDesi02_c::subSelect};
    (this->*l_sub[mSubState])();
}

// .text:0xCE4
void dMenuDesi02_c::setHighlight(int slot, BOOL on) {
    if (isDesign(slot)) {
        if (mDesi.setSelection(slot - SLOT_DESIGN, on != 0)) {
            onFlag(FLAG_DESI_CALC);
        }
    } else if (isBox(slot)) {
        if (setBoxHighlight(slot, on)) {
            onFlag(FLAG_CALC);
        }
    } else if (isPageButton(slot)) {
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
    }
}

// .text:0xE34
int dMenuDesi02_c::hitCheck(BOOL hover) {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int slot = m2d::hitButton(&mVec2_c(pos), &mLyt, 0, 6, -1, -1, 1.0f);
    if (hover == FALSE && !isBox(slot)) {
        slot = SLOT_NONE;
    }
    if (hover == TRUE && isBox(slot)) {
        if (getDesign(slot) == NULL) {
            slot = SLOT_NONE;
        }
    }
    if (slot != SLOT_NONE) {
        if (hover && isBox(slot)) {
            mVec2_c ofs = getSlotPos(slot, 0.0f, 0.0f);
            ofs.x = ofs.x - pos.x;
            ofs.y = ofs.y - pos.y;
            mDesi.setOffset(ofs);
        }
        return slot;
    }
    slot = mDesi.hitDesign(mVec2_c(pos), hover, 1.0f);
    if (slot != -1) {
        return slot + SLOT_DESIGN;
    }
    return SLOT_NONE;
}

// .text:0xFEC
void dMenuDesi02_c::checkPadMove() {
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

// .text:0x1070
void dMenuDesi02_c::pressButton() {
    if (isButtonEnabled(mCursor)) {
        if (isPageButton(mCursor)) {
            mPageAnm[mCursor - SLOT_PREV_PAGE].setNext(BTN_ANM_PUSH);
        } else if (mCursor == SLOT_BACK) {
            mBackAnm.setNext(BTN_ANM_PUSH);
            playSeCancel();
        } else {
            return;
        }
        setSubState(SUB_PUSH);
    }
}

// .text:0x1120
BOOL dMenuDesi02_c::isPushEnd() {
    if (isPageButton(mCursor)) {
        return mPageAnm[mCursor - SLOT_PREV_PAGE].isEnd();
    } else if (mCursor == SLOT_BACK) {
        return mBackAnm.isEnd();
    }
    return TRUE;
}

// .text:0x1170
void dMenuDesi02_c::finishPush() {
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
            dMenuMng_c::setResult(1);
        }
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x1258
void dMenuDesi02_c::startClose() {
    offFlag(FLAG_PAGE_TURN);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_OUT, STATE_CLOSE, TRUE);
    mDesi.startLeaveAnm(1);
    playSeSubMenuClose();
    dMenuMng_c::setResult(0);
    hideTutorial();
}

// .text:0x12C8
void dMenuDesi02_c::startPageTurn() {
    onFlag(FLAG_PAGE_TURN);
    startAnm(&mLyt, l_allPane, ANM_UPPER_WIN_OUT, STATE_CLOSE, TRUE);
    hideTutorial();
    setSubState(SUB_MAIN);
}

// .text:0x1338
void dMenuDesi02_c::setPage(int page) {
    mPage = page;
    dString::Word_c word;
    dScript::formatNumber(&word, mPage + 1, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(0, &word);
    dScript::formatNumber(&word, PAGE_NUM, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(1, &word);
    mLyt.setMessage("T_page_00", MSG_PAGE_NUMBER, NULL);
    updatePageButtons();
}

// .text:0x13E4
void dMenuDesi02_c::initAnm() {
    static const char *l_prevAnm[] = {
        "leave_design_win_00_sldCrsr_00_In.brlan",
        "leave_design_win_00_sldCrsr_00_Out.brlan",
        "leave_design_win_00_sldCrsr_00_Psh.brlan",
        "leave_design_win_00_sldCrsr_00_CanPsh.brlan",
        "leave_design_win_00_sldCrsr_00_NoPsh.brlan",
        NULL};
    static const char *l_nextAnm[] = {
        "leave_design_win_00_sldCrsr_01_In.brlan",
        "leave_design_win_00_sldCrsr_01_Out.brlan",
        "leave_design_win_00_sldCrsr_01_Psh.brlan",
        "leave_design_win_00_sldCrsr_01_CanPsh.brlan",
        "leave_design_win_00_sldCrsr_01_NoPsh.brlan"};
    static const char *l_backAnm[] = {
        "leave_design_win_00_squareBtn_In.brlan",
        "leave_design_win_00_squareBtn_Out.brlan",
        "leave_design_win_00_squareBtn_Psh.brlan"};
    mPageAnm[0].init(&mLyt, &mRes, l_prevAnm, 5);
    mPageAnm[0].start(1);
    mPageAnm[0].setToEnd();
    mPageAnm[1].init(&mLyt, &mRes, l_nextAnm, 5);
    mPageAnm[1].start(1);
    mPageAnm[1].setToEnd();
    mBackAnm.init(&mLyt, &mRes, l_backAnm, 3);
    mBackAnm.start(1);
    mBackAnm.setToEnd();
}

// .text:0x14A0
void dMenuDesi02_c::calcAnm() {
    int playing = 0;
    for (int i = 0; i < 2; i++) {
        playing |= mPageAnm[i].play();
    }
    playing |= mBackAnm.play();
    if (playing) {
        onFlag(FLAG_CALC);
    }
    if (mDesi.calcLeaveAnm()) {
        onFlag(FLAG_DESI_CALC);
    }
}

// .text:0x1540
BOOL dMenuDesi02_c::isButtonEnabled(int slot) {
    switch (slot) {
    case SLOT_PREV_PAGE:
        return isPrevPageEnabled();
    case SLOT_NEXT_PAGE:
        return isNextPageEnabled();
    }
    return TRUE;
}

// .text:0x1568
BOOL dMenuDesi02_c::isPrevPageEnabled() {
    return TRUE;
}

// .text:0x1570
BOOL dMenuDesi02_c::isNextPageEnabled() {
    return TRUE;
}

// .text:0x1578
void dMenuDesi02_c::updatePageButtons() {
    mPageAnm[0].start(isPrevPageEnabled() ? BTN_ANM_ENABLED : BTN_ANM_DISABLED);
    mPageAnm[0].setToEnd();
    mPageAnm[1].start(isNextPageEnabled() ? BTN_ANM_ENABLED : BTN_ANM_DISABLED);
    mPageAnm[1].setToEnd();
    onFlag(FLAG_CALC);
}

// .text:0x15F8
dDesign_c *dMenuDesi02_c::getDesign(int slot) {
    if (isDesign(slot)) {
        return mDesi.getDesign(slot - SLOT_DESIGN);
    } else if (isBox(slot)) {
        return dPlayerMgr::Mgr_c::getCurrentPlayer()->getDesignBox()->get(mPage,
                                                                          slot);
    }
    return NULL;
}

// .text:0x1698
void dMenuDesi02_c::setDesign(int slot, dDesign_c *design) {
    onFlag(FLAG_CHANGED);
    if (isDesign(slot)) {
        int index = slot - SLOT_DESIGN;
        mDesi.setDesign(index, design);
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        dItem::Item hat = player->mOrgDesigns.getDesignItem((u8)index, 0x15);
        if (player->mEquipment.mHat.isSame(hat)) {
            dMenuMng_c::setPickedHat();
        }
        dItem::Item shirt = player->mOrgDesigns.getDesignItem((u8)index, 0x13);
        if (player->mEquipment.mShirt.isSame(shirt)) {
            dMenuMng_c::setPickedShirt();
        }
    } else if (isBox(slot)) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->getDesignBox()->set(mPage, slot,
                                                                   design);
    }
}

// .text:0x17B0
void dMenuDesi02_c::removeDesign(int slot) {
    if (isBox(slot)) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->getDesignBox()->remove(mPage,
                                                                      slot);
    }
}

static char l_boxPaneFormat[14] = "myDsgn_%02d";
static char l_facePaneFormat[14] = "myDsgnFc_%02d";

// .text:0x1818
void dMenuDesi02_c::setDesignVisible(int slot, BOOL visible) {
    if (isDesign(slot)) {
        mDesi.setDesignVisible(slot - SLOT_DESIGN, visible != 0);
    } else if (isBox(slot)) {
        char name[40];
        sprintf(name, l_boxPaneFormat, slot);
        bool shown = visible != 0;
        mLyt.setVisible(name, shown);
        sprintf(name, l_facePaneFormat, slot);
        mLyt.setVisible(name, shown);
        dDesign_c *design = getDesign(slot);
        BOOL isPro = FALSE;
        if (design != NULL && design->mIsPro != 0) {
            isPro = TRUE;
        }
        visible &= isPro;
        sprintf(name, "N_pro_%02d", slot);
        mLyt.setVisible(name, visible != 0);
    }
}

// .text:0x1960
void dMenuDesi02_c::updateBoxDesigns() {
    char name[40];
    for (int i = 0; i < BOX_NUM; i++) {
        dDesign_c *design = getDesign(i);
        if (design == NULL) {
            setDesignVisible(i, FALSE);
        } else {
            setDesignVisible(i, TRUE);
            nw4r::lyt::TexMap *tex = m2d::getTexMaps(mBoxPanes[i]);
            tex->SetImage(design->getTexture());
            tex->SetPalette(design->getPalette());
        }
        sprintf(name, "N_pro_%02d", i);
        bool isPro = false;
        if (design != NULL && design->mIsPro != 0) {
            isPro = true;
        }
        mLyt.setVisible(name, isPro);
    }
}

// .text:0x1A48
void dMenuDesi02_c::initPanes() {
    char name[40];
    for (int i = 0; i < BOX_NUM; i++) {
        sprintf(name, l_boxPaneFormat, i);
        mBoxPanes[i] = mLyt.findPane(name);
    }
    updateBoxDesigns();
    mLyt.setVisible(l_frame, false);
    mHighlight = SLOT_NONE;
}

// .text:0x1AEC
BOOL dMenuDesi02_c::setBoxHighlight(int slot, bool on) {
    char name[40];
    if (on) {
        if (mHighlight == SLOT_NONE) {
            mLyt.setVisible(l_frame, true);
        }
        if (slot != mHighlight) {
            sprintf(name, "N_myDsgn_%02d", slot);
            mHighlight = slot;
            mLyt.setPosFrom(name, l_frame, 0.0f, 0.0f);
            return TRUE;
        }
    } else if (slot == mHighlight) {
        mLyt.setVisible(l_frame, false);
        mHighlight = SLOT_NONE;
    }
    return FALSE;
}

// .text:0x1BC4
mVec2_c dMenuDesi02_c::getSlotPos(int slot, f32 x, f32 y) {
    mVec2_c result(0.0f, 0.0f);
    if (isDesign(slot)) {
        return mDesi.getDesignPos(slot - SLOT_DESIGN, x, y);
    } else if (isBox(slot)) {
        char name[40];
        sprintf(name, "N_myDsgn_%02d", slot);
        mVec3_c pos;
        m2d::getScreenPos(&pos, mLyt.findPane(name));
        if (SCGetAspectRatio() == 1) {
            x *= 0.730769f;
        }
        result.x = pos.x + x;
        result.y = pos.y + y;
        return result;
    }
    return result;
}

// .text:0x1D08
BOOL dMenuDesi02_c::dropOn(int from, int to) {
    if (from == to) {
        return TRUE;
    }
    dDesign_c *src = getDesign(from);
    if (getDesign(to) == NULL) {
        setDesign(to, src);
        if (isBox(from)) {
            removeDesign(from);
        }
        updateBoxDesigns();
        return TRUE;
    }
    buildSwapSelect();
    openSelect(to);
    return FALSE;
}

// .text:0x1DE0
void dMenuDesi02_c::copyDesign(int from, int to) {
    setDesign(to, getDesign(from));
    if (isDesign(to)) {
        mDesi.updateDesigns();
    } else if (isBox(to)) {
        updateBoxDesigns();
    }
}

// .text:0x1E7C
void dMenuDesi02_c::swapOrder(int a, int b) {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mOrgDesigns.mOrder.swap(
        (u8)(a - SLOT_DESIGN), (u8)(b - SLOT_DESIGN));
    mDesi.updateDesigns();
}

// .text:0x1EE0
void dMenuDesi02_c::swapDesigns(int a, int b) {
    if (isDesign(a) && isDesign(b)) {
        swapOrder(a, b);
    } else {
        dDesign_c *designA = getDesign(a);
        dDesign_c *designB = getDesign(b);
        dDesign_c temp = *designA;
        setDesign(a, designB);
        setDesign(b, &temp);
        updateBoxDesigns();
        mDesi.updateDesigns();
    }
}

// .text:0x21D4
void dMenuDesi02_c::getSlotName(dScript::Word_c *word, int slot) {
    word->clear();
    word->set(getDesign(slot)->mName, 0);
}

// .text:0x223C
void dMenuDesi02_c::showSlotTutorial(int slot) {
    if (isDesign(slot) || isBox(slot)) {
        if (getDesign(slot) != NULL && dObjTutorial_c::request(6, slot, 0)) {
            mVec2_c pos = getSlotPos(slot, 0.0f, 0.0f);
            m2d::stringWord_c word;
            getSlotName(&word, slot);
            dObjTutorial_c::setWord(&pos, &word, 0);
        }
    } else {
        hideTutorial();
    }
}

// .text:0x2338
void dMenuDesi02_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x233C
bool dMenuDesi02_c::calcPickup() {
    if (isFlag(FLAG_FOLLOW)) {
        mDesi.calcFollow();
        mDesi.calcPickup();
        return true;
    } else if (isFlag(FLAG_MOVE)) {
        mDesi.calcPickup();
        return true;
    }
    return false;
}

// .text:0x23B4
void dMenuDesi02_c::pickUpDesign(int slot) {
    mHeld = slot;
    mDesi.setPickupDesign(getDesign(slot));
    setDesignVisible(slot, FALSE);
    setSubState(SUB_DRAG);
    mDesi.startFollow(FALSE);
    onFlag(FLAG_FOLLOW);
    dObjCursor_c::hold();
    hideTutorial();
    playSeItemLift();
}

// .text:0x244C
void dMenuDesi02_c::dropHeld(int slot) {
    if (getDesign(slot) != NULL) {
        setDesignVisible(slot, TRUE);
    }
    offFlag(FLAG_FOLLOW);
    offFlag(FLAG_PLACED);
    setSubState(SUB_MAIN);
    hideTutorial();
}

// .text:0x24D4
void dMenuDesi02_c::openDialog(u16 msg) {
    mDialog.setBtn(0, MSG_NO, NULL, SE_cancel_little);
    mDialog.setBtn(1, MSG_YES, NULL, 0);
    mDialog.setBtnVisible(2, false);
    mDialog.open(0, 0, 0);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mDialog.setMsg(msg, NULL);
}

// .text:0x2590
void dMenuDesi02_c::buildSwapSelect() {
    mSelect.reset();
    mSelect.mNum = 0;
    mSelect.addItem(MSG_SWAP, ANSWER_SWAP, 0, 0);
    mSelect.addItem(MSG_COPY, ANSWER_COPY, 0, 0);
    mSelect.addItem(MSG_NEVER_MIND, ANSWER_CANCEL, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x2624
void dMenuDesi02_c::openSelect(int target) {
    mVec2_c pos = getSlotPos(target, 10.0f, -50.0f);
    mTarget = target;
    mSelect.placeAbove(pos);
    mSelect.openPointer();
    setSubState(SUB_SELECT);
}
