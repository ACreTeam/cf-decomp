#include <game/game/d_menu_edit.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_org_obj.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/os/OSCache.h>
#include <revolution/sc.h>
#include <stdio.h>
#include <game/cLib/c_math.hpp>
#include <string.h>

namespace dMenuEdit {
// .text:0xCC
BASE_PROFILE(MENU_EDIT, dMenuEdit_c)

// The patrolTargetName callback of changePaletteAnm: replaces the last two
// characters of the animation target's name.
void setNameSuffix(char *name, void *arg);
} // namespace dMenuEdit

// The two work buffers (the design being edited and the undo copy) of the
// four textures of a design.
static dDesignTex_c l_tex[dMenuEdit_c::TEX_BUF_NUM][ORG_DESIGN_TEX_NUM];
// The palette being used (dDesign_c::getPaletteData).
static u8 l_palette;

// .text:0x320
int dMenuEdit_c::create() {
    static const char *l_mnqnAnm[2] = {
        "edit_design_win_00_mnqnIn.brlan",
        "edit_design_win_00_mnqnOut.brlan",
    };
    bool loaded = mRes.load("Layout/edit_win.arc") != 0;
    loaded &= mBtn.load();
    loaded &= mSlct.load();
    loaded &= mRes2.load("Layout/desi_win.arc");
    loaded &= mSelect.load();
    if (!loaded) {
        return NOT_READY;
    }
    mSelect.build();
    mBtn.create();
    mSlct.create();
    mSlct.setBtn(0, MSG_NO, NULL, SE_cancel_little);
    mSlct.setBtn(1, MSG_YES, NULL, 0);
    mSlct.setBtnVisible(2, false);
    mLyt.build(&mRes, "edit_design_win_00.brlyt");
    mLyt.setAnm(&mBtnAnm, "edit_design_win_00_btnR_psh.brlan", NULL);
    mPreviewAnm.init(&mLyt, &mRes, l_mnqnAnm, 2);
    mPreviewAnm.start(1);
    mPreviewAnm.setToEnd();
    mLyt.setAnm(&mScrlIconAnm, "edit_design_win_00_scrlIcon_00.brlan", NULL);
    mLyt.setAnm(&mIconPtrnAnm, "edit_design_win_00_icnPtrn_00.brlan", NULL);
    switch (mMenuNo) {
    case dMenuMng_c::MENU_EDIT_PRO:
        onFlag(FLAG_PRO);
        mIconPtrnAnm.setFrame(1.0f);
        mLyt.setAnm(&mFrmPtrnAnm, "edit_design_win_00_frmPtrn_grn.brlan", NULL);
        mFrmPtrnAnm.setToEnd();
        break;
    case dMenuMng_c::MENU_EDIT_FACE:
        mIconPtrnAnm.setFrame(2.0f);
        mLyt.setAnm(&mFrmPtrnAnm, "edit_design_win_00_frmPtrn_pnk.brlan", NULL);
        mFrmPtrnAnm.setToEnd();
        break;
    default:
        mIconPtrnAnm.setFrame(0.0f);
        mLyt.setAnm(&mFrmPtrnAnm, "edit_design_win_00_frmPtrn_blue.brlan",
                    NULL);
        mFrmPtrnAnm.setToEnd();
        break;
    }
    mToolLyt.build(&mRes, "slct_tool_00.brlyt");
    mToolLyt.setAnm(&mToolWinBtnAnm, "slct_tool_00_toolWinBtnPsh.brlan", NULL);
    mToolLyt.setAnm(&mToolIcnAnm, "slct_tool_00_toolIcn.brlan", NULL);
    mToolLyt.setAnm(&mToolBtnAnm, "slct_tool_00_toolBtnPsh.brlan", NULL);
    mToolLyt.mPriority = LYT_PRIORITY;
    mColorLyt.build(&mRes, "slct_clr_00.brlyt");
    mColorLyt.setAnm(&mPltBtnAnm, "slct_clr_00_pltBtnPsh.brlan", NULL);
    mColorLyt.setAnm(&mClrBtnAnm, "slct_clr_00_clrBtnPsh.brlan", NULL);
    mColorLyt.setAnm(&mClrCrsrAnm, "slct_clr_00_clrCrsrPsh.brlan", NULL);
    mColorLyt.mPriority = LYT_PRIORITY;
    if (isFlag(FLAG_PRO)) {
        mPartsLyt.build(&mRes, "pro_design_win_00.brlyt");
        initPartsWin();
        mPartsLyt.mPriority = LYT_PRIORITY;
        initPartsTexture();
        mPartsLyt.calc();
        mPickupLyt.build(&mRes2, "pickUp_design_00.brlyt");
        mPickupLyt.setVisible("N_pro_00", false);
        mPickupLyt.mPriority = LYT_PRIORITY;
    }
    mToolCrsr.build(&mRes, "tool_crsr_00.brlyt");
    mPenFrame.build(&mRes, "penFrm_crsr_00.brlyt");
    mArrowStart.build(&mRes, "arrow_crsr_00.brlyt");
    mArrowEnd.build(&mRes, "arrow_crsr_00.brlyt");
    mRulerStart.build(&mRes, "ruler_crsr_00.brlyt");
    mRulerStart.setVisible("ruler_RB", false);
    mRulerEnd.build(&mRes, "ruler_crsr_00.brlyt");
    mRulerEnd.setVisible("ruler_LT", false);
    mLyt.calc();
    onFlag(FLAG_DRAW);
    clearSelect();
    initDesign();
    initExtra();
    setGrid(false);
    setupBtns();
    mZClear.set(180.0f, 45.0f, -300.0f, -120.0f, -499.9f);
    startAnm(&mLyt, "N_editD_Win_all", 0, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x7A4
int dMenuEdit_c::doDelete() {
    hidePreview();
    if (mSelect.remove() == FALSE) {
        return NOT_READY;
    }
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    if (mSlct.remove() == FALSE) {
        return NOT_READY;
    }
    if (mRes.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes2.remove() != 0;
}

// .text:0x844
int dMenuEdit_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x868
int dMenuEdit_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    if (isState(STATE_MAIN)) {
        onFlag(FLAG_PREVIEW);
        initPreview();
    }
    return result;
}

// .text:0x8CC
int dMenuEdit_c::stateClose() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_EDIT_FACE:
        break;
    case dMenuMng_c::MENU_EDIT_DESIGN:
        if (isFlag(FLAG_SAVED)) {
            dAcUitag::openMenuA(dAcUitag::MENU_A_DESIGN_NAME);
        } else {
            dAcUitag::openMenuA(dAcUitag::MENU_A_DESIGN);
            dAcUitag::showTab();
        }
        break;
    case dMenuMng_c::MENU_EDIT_PRO:
        if (isFlag(FLAG_SAVED)) {
            if (dAcUitag::openMenuB(8) == FALSE) {
                return SUCCEEDED;
            }
        }
        break;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x974
int dMenuEdit_c::execute() {
    hideTutorial();
    BOOL anming = isState(STATE_ANM);
    int result = dMenuBase_c::execute();
    calcAnm(anming);
    mBtn.execute();
    mSlct.execute();
    mSelect.update();
    if (isFlag(FLAG_FOLLOW)) {
        mMove.calcFollow();
    }
    if (isFlag(FLAG_MOVE) || isFlag(FLAG_FOLLOW)) {
        mVec2_c *pos = mMove.getPos();
        mPickupLyt.mPos.x = pos->x;
        mPickupLyt.mPos.y = pos->y;
        mPickupLyt.calc();
    }
    if (isFlag(FLAG_PREVIEW)) {
        mTurn += mAng(PICKUP_SPEED);
    }
    return result;
}

// .text:0xA84
int dMenuEdit_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mBtn.draw();
        switch (mWinMode) {
        case WIN_TOOL:
            if (isFlag(FLAG_PRO)) {
                mPartsLyt.entry();
            }
            mColorLyt.entry();
            mToolLyt.entry();
            break;
        case WIN_COLOR:
            if (isFlag(FLAG_PRO)) {
                mPartsLyt.entry();
            }
            mToolLyt.entry();
            mColorLyt.entry();
            break;
        default:
            mColorLyt.entry();
            mToolLyt.entry();
            if (isFlag(FLAG_PRO)) {
                mPartsLyt.entry();
                if (isFlag(FLAG_FOLLOW) || isFlag(FLAG_MOVE) ||
                    isFlag(FLAG_PLACED)) {
                    mPickupLyt.entry();
                }
            }
            break;
        }
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    if (isFlag(FLAG_ON_CANVAS)) {
        if (isFlag(FLAG_PEN_FRAME)) {
            mPenFrame.entry();
        }
        if (isFlag(FLAG_RULER)) {
            if (isFlag(FLAG_RULER_END)) {
                mRulerEnd.entry();
            }
            mRulerStart.entry();
        } else if (isFlag(FLAG_ARROW)) {
            if (isFlag(FLAG_ARROW_END)) {
                mArrowEnd.entry();
            }
            mArrowStart.entry();
        } else {
            mToolCrsr.entry();
        }
    }
    if (isFlag(FLAG_PREVIEW)) {
        calcPreview();
    }
    mSelect.draw();
    return SUCCEEDED;
}

// .text:0xCAC
void dMenuEdit_c::subStateMain() {
    int hit;
    int btn = mBtn.checkPad(0, 1);
    if (btn != BTN_NONE) {
        dObjCursor_c::show();
        offFlag(FLAG_ON_CANVAS);
        hit = btn + BTN_QUIT;
        setHover(hit);
        pushBtn(hit);
    } else {
        mVec2_c pos = dObjCursor_c::getPos();
        hit = hitCheck(1.0f, pos);
        setHover(hit);
        if (hit == BTN_CANVAS) {
            dObjCursor_c::hide();
            onFlag(FLAG_ON_CANVAS);
            if (isFlag(FLAG_PEN_FRAME)) {
                startFilter();
                placePenFrame();
            } else if (isFlag(FLAG_RULER)) {
                startFilter();
                placeRulerStart();
            }
        } else {
            dObjCursor_c::show();
            offFlag(FLAG_ON_CANVAS);
        }
        if (hit != BTN_NONE) {
            showTutorial(hit);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                pushBtn(hit);
                return;
            }
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
                switch (hit) {
                case BTN_CANVAS:
                    startFilter();
                    pickColor();
                    break;
                }
                return;
            }
        }
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_Z)) {
            if (mZoom > ZOOM_1X) {
                setZoom(mZoom - 1);
                playSeDecide();
            }
        } else if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_C)) {
            if (mZoom < ZOOM_4X) {
                setZoom(mZoom + 1);
                playSeDecide();
            }
        } else if (!checkShortcuts()) {
            checkPadMove();
        }
    }
}

// .text:0xF08
void dMenuEdit_c::subStatePaint() {
    updateFilter();
    clampPixelToView();
    if (isFlag(FLAG_PEN_FRAME)) {
        placePenFrame();
    }
    if (isFlag(FLAG_RULER_END)) {
        clampPixelToView();
        placeRuler();
    }
    switch (mTool) {
    case BTN_TOOL_FIRST + TOOL_PEN_S:
        drawLine(mPrevX, mPrevY, mPixelX, mPixelY, mColor, 0);
        break;
    case BTN_TOOL_FIRST + TOOL_PEN_M:
        drawLine(mPrevX, mPrevY, mPixelX, mPixelY, mColor, 1);
        break;
    case BTN_TOOL_FIRST + TOOL_PEN_L:
        drawLine(mPrevX, mPrevY, mPixelX, mPixelY, mColor, 2);
        break;
    case BTN_TOOL_FIRST + TOOL_LINE:
        placeArrowStart();
        restoreUndo();
        drawLine(mStartX, mStartY, mPixelX, mPixelY, mColor, 0);
        break;
    case BTN_TOOL_FIRST + TOOL_RECT:
        restoreUndo();
        drawRect(mStartX, mStartY, mPixelX, mPixelY, mColor);
        break;
    case BTN_TOOL_FIRST + TOOL_OVAL:
        restoreUndo();
        drawOval(mStartX, mStartY, mPixelX, mPixelY, mColor);
        break;
    case BTN_TOOL_FIRST + TOOL_HEART:
        restoreUndo();
        drawStamp(mPixelX, mPixelY, mColor, STAMP_HEART);
        break;
    case BTN_TOOL_FIRST + TOOL_STAR:
        restoreUndo();
        drawStamp(mPixelX, mPixelY, mColor, STAMP_STAR);
        break;
    }
    if (mPrevX != mPixelX || mPrevY != mPixelY) {
        switch (mTool) {
        case BTN_TOOL_FIRST + TOOL_PEN_S:
            playSePen3();
            break;
        case BTN_TOOL_FIRST + TOOL_PEN_M:
            playSePen2();
            break;
        case BTN_TOOL_FIRST + TOOL_PEN_L:
            playSePen();
            break;
        case BTN_TOOL_FIRST + TOOL_RECT:
        case BTN_TOOL_FIRST + TOOL_OVAL:
        case BTN_TOOL_FIRST + TOOL_LINE:
            if (mPad::g_currentCore->down(WPAD_BUTTON_A)) {
                playSeSetMove();
            }
            break;
        }
    }
    mTex.flush();
    updatePreviewTexture();
    mPrevX = mPixelX;
    mPrevY = mPixelY;
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        offFlag(FLAG_ARROW_END);
        offFlag(FLAG_RULER_END);
        setSubState(SUB_MAIN);
        switch (mTool) {
        case BTN_TOOL_FIRST + TOOL_RECT:
        case BTN_TOOL_FIRST + TOOL_OVAL:
        case BTN_TOOL_FIRST + TOOL_LINE:
            playSeSetEnd();
            break;
        }
    } else if (mTool == BTN_TOOL_FIRST + TOOL_PEN_S ||
               mTool == BTN_TOOL_FIRST + TOOL_PEN_M ||
               mTool == BTN_TOOL_FIRST + TOOL_PEN_L) {
        mVec2_c pos = getFilteredPointer();
        int hit = hitCheck(1.0f, pos);
        if (hit != BTN_CANVAS) {
            dObjCursor_c::show();
            offFlag(FLAG_ON_CANVAS);
            setSubState(SUB_DRAG_IN);
        }
    }
}

// .text:0x1294
void dMenuEdit_c::subStateScroll() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A) ||
        !dObjCursor_c::isVisible()) {
        dObjCursor_c::show();
        playSeFramePlace();
        setSubState(SUB_MAIN);
    } else {
        int x = mScrollX;
        int y = mScrollY;
        scrollToPointer();
        if (x != mScrollX || y != mScrollY) {
            playSeFrameSlide();
        }
    }
}

// .text:0x1354
void dMenuEdit_c::subStateDragIn() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        setSubState(SUB_MAIN);
    } else {
        mVec2_c pos = getFilteredPointer();
        int hit = hitCheck(1.0f, pos);
        if (hit == BTN_CANVAS) {
            dObjCursor_c::hide();
            onFlag(FLAG_ON_CANVAS);
            setSubState(SUB_PAINT);
            updateFilter();
            clampPixelToView();
            mPrevX = mPixelX;
            mPrevY = mPixelY;
            subStatePaint();
        } else {
            updateFilter();
        }
    }
}

// .text:0x1450
void dMenuEdit_c::subStateTool() {
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        closeToolWin();
        playSeCancel();
    } else {
        mVec2_c pos = dObjCursor_c::getPos();
        int hit = hitCheckTool(1.0f, pos);
        setHover(hit);
        if (hit != BTN_NONE) {
            showTutorial(hit);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                if (isTool(hit)) {
                    selectTool(hit);
                    onFlag(FLAG_TOOL_BTN_ANM);
                    mToolBtnAnm.setFrame(0.0f);
                    closeToolWin();
                    playSeDecide();
                } else if (hit == BTN_TOOL_BACK) {
                    closeToolWin();
                    playSeCancel();
                }
            }
        }
    }
}

// .text:0x1598
void dMenuEdit_c::subStateColor() {
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        closeColorWin();
        playSeCancel();
    } else {
        mVec2_c pos = dObjCursor_c::getPos();
        int hit = hitCheckColor(1.0f, pos);
        setHover(hit);
        if (hit != BTN_NONE) {
            showTutorial(hit);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                if (hit == BTN_PALETTE_PREV) {
                    changePaletteAnm(-1);
                    return;
                } else if (hit == BTN_PALETTE_NEXT) {
                    changePaletteAnm(1);
                    return;
                } else if (hit == BTN_COLOR_BACK) {
                    playSeCancel();
                    closeColorWin();
                    return;
                } else if (isColor(hit)) {
                    setColor(hit - BTN_COLOR_BACK);
                    onFlag(FLAG_COLOR_BTN_ANM);
                    mClrBtnAnm.setFrame(0.0f);
                    closeColorWin();
                    playSeDecide();
                    return;
                }
            }
        }
        if (checkShortcuts()) {
            return;
        }
    }
}

// .text:0x1720
void dMenuEdit_c::subStateParts() {
    mVec2_c pos = dObjCursor_c::getPos();
    int hit = hitCheckParts(pos, TRUE);
    setHover(hit);
    if (hit != BTN_NONE) {
        showTutorial(hit);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            if (hit == BTN_PARTS_BACK) {
                playSeCancel();
                setPartsBtnAnm(1);
                setSubState(SUB_PARTS_ANM);
                setNextSubState(SUB_MAIN);
                setHover(BTN_NONE);
                mBtn.setDisable(0, FALSE);
                mBtn.setDisable(2, FALSE);
                if (dOrgObj_c::sInstance != NULL) {
                    dOrgObj_c::sInstance->setClothAnm(-1);
                }
                backupUndo();
            } else {
                setSubState(SUB_PARTS_PRESS);
                mHeldPart = hit;
                mHoldTimer = 0;
            }
        }
    }
}

// .text:0x1870
void dMenuEdit_c::subStatePartsPress() {
    showTutorial(mHeldPart);
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        if (mPart != mHeldPart - BTN_PART_FIRST) {
            selectPart(mHeldPart - BTN_PART_FIRST);
            int texNo = mTexNo;
            if (dOrgObj_c::sInstance != NULL) {
                dOrgObj_c::sInstance->setClothAnm(texNo);
            }
            attachTexture(FALSE);
            playSeDecide();
        }
        setSubState(SUB_PARTS);
    } else {
        mVec2_c pos = dObjCursor_c::getPos();
        int hit = hitCheckParts(pos, TRUE);
        if (hit == mHeldPart && mHoldTimer < dMenuMove_c::getHoldFrames()) {
            mHoldTimer++;
        } else {
            pickUpPart();
        }
    }
}

// .text:0x197C
void dMenuEdit_c::subStatePartsDrag() {
    mVec2_c pos = dObjCursor_c::getPos();
    int hit = hitCheckParts(pos, FALSE);
    if (hit == BTN_PARTS_BACK) {
        hit = BTN_NONE;
    }
    setHover(hit);
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A) ||
        !dObjCursor_c::isVisible()) {
        offFlag(FLAG_FOLLOW);
        dObjCursor_c::show();
        if (hit != BTN_NONE && hit != mHeldPart) {
            setupSwapMenu();
            openSwapMenu(hit);
            setSubState(SUB_PARTS_ANSWER);
            mVec2_c partPos = getPartPos(hit, 15.0f, 15.0f);
            mMove.setPos(partPos);
            mPickupLyt.mPos.x = partPos.x;
            mPickupLyt.mPos.y = partPos.y;
            mPickupLyt.calc();
            onFlag(FLAG_PLACED);
        } else {
            mMove.startMove(getPartPos(mHeldPart, 0.0f, 0.0f));
            onFlag(FLAG_MOVE);
            setSubState(SUB_PARTS_DROP);
        }
    }
}

// .text:0x1B18
void dMenuEdit_c::subStatePartsDrop() {
    if (mMove.calcMove()) {
        playSeItemDrop();
        offFlag(FLAG_MOVE);
        setSubState(SUB_PARTS);
        showPart();
    }
}

// .text:0x1B80
void dMenuEdit_c::subStatePartsAnswer() {
    if (mSelect.isClosed()) {
        mAnswer = mSelect.mCode;
        offFlag(FLAG_PLACED);
        showPart();
        switch (mAnswer) {
        case ANSWER_CANCEL:
            break;
        case ANSWER_COPY: {
            int to = getTexIndex(mSwapPart - BTN_PART_FIRST);
            int from = getTexIndex(mHeldPart - BTN_PART_FIRST);
            copyTexture(from, to);
            backupUndo();
            break;
        }
        case ANSWER_SWAP: {
            int to = getTexIndex(mSwapPart - BTN_PART_FIRST);
            int from = getTexIndex(mHeldPart - BTN_PART_FIRST);
            swapTexture(from, to);
            backupUndo();
            break;
        }
        }
        setSubState(SUB_PARTS);
    }
}

// .text:0x1C9C
void dMenuEdit_c::subStateDialog() {
    if (mSlct.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0x1CEC
void dMenuEdit_c::subStateColorAnm() {
    if (isFlag(FLAG_COLOR_BTN_ANM)) {
        mClrBtnAnm.play();
        if (mClrBtnAnm.mFrameCtrl.isStop()) {
            offFlag(FLAG_COLOR_BTN_ANM);
        } else {
            onFlag(FLAG_CALC_COLOR);
            return;
        }
    }
    if (playColorBtnAnm()) {
        changeNextSubState();
    }
}

// .text:0x1D74
void dMenuEdit_c::subStateToolAnm() {
    if (isFlag(FLAG_TOOL_BTN_ANM)) {
        mToolBtnAnm.play();
        if (mToolBtnAnm.mFrameCtrl.isStop()) {
            offFlag(FLAG_TOOL_BTN_ANM);
        } else {
            onFlag(FLAG_CALC_TOOL);
            return;
        }
    }
    if (playToolBtnAnm()) {
        changeNextSubState();
    }
}

// .text:0x1DFC
void dMenuEdit_c::subStateButton() {
    if (!mBtn.isPushing()) {
        switch (mPushedBtn) {
        case BTN_QUIT:
            mSlct.open(0, 0, 0);
            if (mMenuNo == dMenuMng_c::MENU_EDIT_FACE) {
                mSlct.setMsg(MSG_QUIT_FACE, NULL);
            } else {
                mSlct.setMsg(MSG_CLOSE_NO_SAVE, NULL);
            }
            setSubState(SUB_DIALOG_OPEN);
            setNextSubState(SUB_DIALOG);
            onFlag(FLAG_DIALOG);
            break;
        case BTN_DONE:
            saveDesign();
            startCloseAnm();
            onFlag(FLAG_SAVED);
            dMenuMng_c::setResult(1);
            break;
        default:
            setSubState(SUB_MAIN);
            break;
        }
    }
}

// .text:0x1F14
void dMenuEdit_c::subStateDialogOpen() {
    if (mSlct.isWinAnmEnd()) {
        changeNextSubState();
        if (mSubState == SUB_MAIN) {
            offFlag(FLAG_DIALOG);
        }
    }
}

// .text:0x1F6C
void dMenuEdit_c::subStateDialogPush() {
    if (mSlct.isPushEnd()) {
        switch (mSlct.mPushedBtn) {
        case DIALOG_NO:
            mSlct.open(1, 0, 0);
            setSubState(SUB_DIALOG_OPEN);
            setNextSubState(SUB_MAIN);
            break;
        case DIALOG_YES:
            mSlct.open(1, 0, 0);
            startCloseAnm();
            break;
        }
    }
}

// .text:0x2018
void dMenuEdit_c::subStatePartsAnm() {
    if (mPartsBtnAnm.mFrameCtrl.isStop()) {
        changeNextSubState();
    }
}

// .text:0x2058
void dMenuEdit_c::executeSubState() {
    static void (dMenuEdit_c::*l_sub[SUB_NUM])() = {
        &dMenuEdit_c::subStateMain,        &dMenuEdit_c::subStatePaint,
        &dMenuEdit_c::subStateScroll,      &dMenuEdit_c::subStateDragIn,
        &dMenuEdit_c::subStateTool,        &dMenuEdit_c::subStateColor,
        &dMenuEdit_c::subStateParts,       &dMenuEdit_c::subStatePartsPress,
        &dMenuEdit_c::subStatePartsDrag,   &dMenuEdit_c::subStatePartsDrop,
        &dMenuEdit_c::subStatePartsAnswer, &dMenuEdit_c::subStateDialog,
        &dMenuEdit_c::subStateColorAnm,    &dMenuEdit_c::subStateToolAnm,
        &dMenuEdit_c::subStateButton,      &dMenuEdit_c::subStateDialogOpen,
        &dMenuEdit_c::subStateDialogPush,  &dMenuEdit_c::subStatePartsAnm};
    (this->*l_sub[mSubState])();
}

// .text:0x2090
void dMenuEdit_c::calcAnm(BOOL anming) {
    if (mPreviewAnm.play()) {
        onFlag(FLAG_CALC);
    }
    if (isFlag(FLAG_CRSR_ANM)) {
        mClrCrsrAnm.play();
        if (mClrCrsrAnm.mFrameCtrl.isStop()) {
            offFlag(FLAG_CRSR_ANM);
        }
        onFlag(FLAG_CALC_COLOR);
    }
    if (isFlag(FLAG_BTN_ANM)) {
        mBtnAnm.play();
        if (mBtnAnm.mFrameCtrl.isStop()) {
            offFlag(FLAG_BTN_ANM);
        }
        onFlag(FLAG_CALC);
    }
    if (anming) {
        mLyt.copyPos(&mToolLyt, "N_btnL_00_all");
        mLyt.copyPos(&mColorLyt, "N_btnL_01_all");
        if (isFlag(FLAG_PRO)) {
            mLyt.copyPos(&mPartsLyt, "N_editD_Win_all", "N_all");
            onFlag(FLAG_CALC_PARTS);
        }
        onFlag(FLAG_CALC_TOOL);
        onFlag(FLAG_CALC_COLOR);
        mBtn.setPos(&mLyt, "N_editD_Win_all");
    }
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        onFlag(FLAG_CALC_TOOL);
        onFlag(FLAG_CALC_COLOR);
    }
    mScrlIconAnm.play();
    mLyt.calc();
    if (isFlag(FLAG_CALC_TOOL)) {
        offFlag(FLAG_CALC_TOOL);
        mToolLyt.calc();
    }
    if (isFlag(FLAG_CALC_COLOR)) {
        offFlag(FLAG_CALC_COLOR);
        mColorLyt.calc();
    }
    if (isFlag(FLAG_ON_CANVAS)) {
        mVec2_c pos;
        if (isFlag(FLAG_FILTER)) {
            pos = getFilteredPointer();
        } else {
            pos = dObjCursor_c::getPos();
        }
        if (isFlag(FLAG_ARROW) && !isFlag(FLAG_ARROW_END)) {
            mArrowStart.mPos.x = pos.x;
            mArrowStart.mPos.y = pos.y;
            mArrowStart.calc();
        } else {
            mToolCrsr.mPos.x = pos.x;
            mToolCrsr.mPos.y = pos.y;
            mToolCrsr.calc();
        }
    }
    if (isFlag(FLAG_PRO)) {
        if (!mPartsBtnAnm.mFrameCtrl.isStop()) {
            mPartsBtnAnm.play();
            onFlag(FLAG_CALC_PARTS);
        }
        if (mChngBtnAnm.play()) {
            onFlag(FLAG_CALC_PARTS);
        }
        if (!mPartBtnAnm.mFrameCtrl.isStop()) {
            mPartBtnAnm.play();
            onFlag(FLAG_CALC_PARTS);
        }
        if (isFlag(FLAG_CALC_PARTS)) {
            offFlag(FLAG_CALC_PARTS);
            mPartsLyt.calc();
        }
    }
}

// .text:0x2438
void dMenuEdit_c::startCloseAnm() {
    startAnm(&mLyt, "N_editD_Win_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dObjCursor_c::show();
    offFlag(FLAG_SAVED);
    offFlag(FLAG_PREVIEW);
}

// .text:0x2498
void dMenuEdit_c::checkPadMove() {
    int x = mScrollX;
    int y = mScrollY;
    int newX = x;
    int newY = y;
    if ((dPad::getHeld() >> 3) & 1) {
        newY = y - 1;
    } else if ((dPad::getHeld() >> 2) & 1) {
        newY = y + 1;
    }
    if (dPad::getHeld() & 1) {
        newX = x - 1;
    } else if ((dPad::getHeld() >> 1) & 1) {
        newX = x + 1;
    }
    if (newX != mScrollX || newY != mScrollY) {
        if (mPadRepeat == 0 || mPadRepeat == 5) {
            setScroll(newX, newY);
            if (x != mScrollX || y != mScrollY) {
                playSeFrameSlide();
            }
        }
        if (mPadRepeat < 5) {
            mPadRepeat++;
        }
    } else {
        mPadRepeat = 0;
    }
}

// .text:0x259C
BOOL dMenuEdit_c::checkShortcuts() {
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_UP)) {
        changePalette(-1);
        return TRUE;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_DOWN)) {
        changePalette(1);
        return TRUE;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_LEFT)) {
        setColor(mColor - 1);
        playSeDecide();
        return TRUE;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_RIGHT)) {
        setColor(mColor + 1);
        playSeDecide();
        return TRUE;
    }
    return FALSE;
}

// .text:0x26B4
void dMenuEdit_c::initDesign() {
    selectPart(PART_FRONT);
    mBuf = 0;
    mUndoBuf = 0;
    mColor = 1;
    mScrollX = 0;
    mScrollY = 0;
    dDesign_c *design = getDesign(dMenuMng_c::getDesignIndex());
    u8 *tex = (u8 *)getTexture();
    memcpy(tex, design->getTexture(), TEX_BUF_SIZE);
    DCFlushRange(tex, TEX_BUF_SIZE);
    memcpy(tex + TEX_BUF_SIZE, design->getTexture(), TEX_BUF_SIZE);
    DCFlushRange(tex + TEX_BUF_SIZE, TEX_BUF_SIZE);
    setPalette(design->mPaletteIdx);
    setZoom(ZOOM_1X);
    attachTexture(TRUE);
    selectTool(BTN_TOOL_FIRST + TOOL_PEN_M);
}

// .text:0x27A8
void dMenuEdit_c::saveDesign() {
    mTexNo = 0;
    u8 *tex = (u8 *)getTexture();
    dDesign_c *design = getDesign(dMenuMng_c::getDesignIndex());
    if (!isFlag(FLAG_PRO)) {
        memcpy(tex + TEX_SIZE, tex, TEX_SIZE);
        memcpy(tex + TEX_SIZE * 2, tex, TEX_SIZE);
        memcpy(tex + TEX_SIZE * 3, tex, TEX_SIZE);
    }
    design->setTexture(tex, TRUE);
    design->setPalette(l_palette, TRUE);
    design->mCreator = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID;
    if (mMenuNo == dMenuMng_c::MENU_EDIT_PRO) {
        design->mIsPro = TRUE;
    } else {
        design->mIsPro = FALSE;
    }
}

// .text:0x2940
void dMenuEdit_c::setPalette(u8 palette) {
    l_palette = palette;
    const u16 *colors = dDesign_c::getPaletteData(palette);
    m2d::getTexMaps(mLyt.findPane("myDsgn_00"))->SetPalette((void *)colors);
    m2d::getTexMaps(mLyt.findPane("myDsgnThum_00"))->SetPalette((void *)colors);
    for (int i = 0; i <= PALETTE_COLOR_NUM - 2; i++) {
        char name[40];
        sprintf(name, "clrBtn_%02d", i);
        nw4r::lyt::Window *window = mColorLyt.findWindow(name);
        nw4r::lyt::Material *material = m2d::getMaterial(window);
        nw4r::lyt::Material *frame = window->GetFrameMaterial(0);
        GXColorS10 col;
        m2d::rgb565ToColor(colors[i + 1], &col);
        material->SetTevColor(1, col);
        frame->SetTevColor(1, col);
        col.a = 0;
        material->SetTevColor(0, col);
        frame->SetTevColor(0, col);
    }
    setColor(mColor);
    mColorLyt.setNumber("T_page_00", palette + 1, 12,
                        dScript::NUM_FORMAT_REGION);
    if (isFlag(FLAG_PRO)) {
        setPartsPalette();
    }
}

// The thumbnail guides of the zoom levels (setZoom, setScroll).
static const char *l_thumb[dMenuEdit_c::ZOOM_NUM] = {NULL, "N_thmGd_M",
                                                     "N_thmGd_S"};

// .text:0x2AE8
void dMenuEdit_c::setZoom(int zoom) {
    static const f32 l_scale[ZOOM_NUM] = {1.0f, 0.5f, 0.25f};
    static const char *l_grid[ZOOM_NUM] = {"gridS_00", "gridM_00", "gridL_00"};
    static const int l_gridNum = ZOOM_NUM;
    static const int l_thumbNum = ZOOM_NUM;
    mZoom = zoom;
    f32 scale = l_scale[zoom];
    nw4r::lyt::TexSRT *srt = m2d::getTexSRTs(mLyt.findPane("myDsgn_00"));
    srt->scale.y = scale;
    srt->scale.x = scale;
    int i;
    for (i = 0; i < l_gridNum; i++) {
        mLyt.setVisible(l_grid[i], i == zoom);
    }
    for (i = 1; i < l_thumbNum; i++) {
        mLyt.setVisible(l_thumb[i], i == zoom);
    }
    onFlag(FLAG_CALC);
    setScroll(mScrollX, mScrollY);
    updatePenFrame();
}

// .text:0x2BE8
void dMenuEdit_c::setGrid(bool on) {
    static const char *l_icon[2] = {"N_btnR_icnR_00", "N_btnR_icnL_00"};
    if (on) {
        onFlag(FLAG_GRID);
    } else {
        offFlag(FLAG_GRID);
    }
    mLyt.setVisible("N_grid_00", on);
    mLyt.setAlpha(l_icon[on], 0xFF);
    mLyt.setAlpha(l_icon[!on], 0x80);
    onFlag(FLAG_CALC);
}

// .text:0x2C94
void dMenuEdit_c::setScroll(int x, int y) {
    static const char *l_gridLine0 = "gridLine_00";
    static const char *l_gridLine1 = "gridLine_01";
    static const int l_limit[ZOOM_NUM] = {0, 8, 12};
    static const f32 l_gridOfs[ZOOM_NUM] = {0.0f, 20.0f, 40.0f};
    int limit = l_limit[mZoom];
    int minimum = -limit;
    if (x < minimum) {
        x = minimum;
    } else if (x > limit) {
        x = limit;
    }
    if (y < minimum) {
        y = minimum;
    } else if (y > limit) {
        y = limit;
    }
    nw4r::lyt::TexSRT *srt = m2d::getTexSRTs(mLyt.findPane("myDsgn_00"));
    srt->translate.x = (1.0f / 32.0f) * x;
    srt->translate.y = (1.0f / 32.0f) * y;
    if (mZoom != ZOOM_1X) {
        nw4r::lyt::Pane *pane = mLyt.findPane(l_thumb[mZoom]);
        mVec2_c thumbPos(3.0f * x, 3.0f * -y);
        pane->SetTranslate(mVec3_c(thumbPos, 0.0f));
        onFlag(FLAG_CALC);
    }
    mScrollX = x;
    mScrollY = y;
    f32 gridX = x * -l_gridOfs[mZoom];
    f32 gridY = y * l_gridOfs[mZoom];
    mVec3_c pos;
    if (gridX >= -160.0f && gridX <= 160.0f) {
        nw4r::lyt::Pane *pane = mLyt.findPane(l_gridLine0);
        pos = pane->GetTranslate();
        pos.x = gridX;
        pane->SetTranslate(pos);
        mLyt.setVisible(l_gridLine0, true);
    } else {
        mLyt.setVisible(l_gridLine0, false);
    }
    if (gridY >= -160.0f && gridY <= 160.0f) {
        nw4r::lyt::Pane *pane = mLyt.findPane(l_gridLine1);
        pos = pane->GetTranslate();
        pos.y = gridY;
        pane->SetTranslate(pos);
        mLyt.setVisible(l_gridLine1, true);
    } else {
        mLyt.setVisible(l_gridLine1, false);
    }
}

// .text:0x2F58
void dMenuEdit_c::attachTexture(BOOL keepOrig) {
    void *tex = getTexture();
    nw4r::lyt::TexMap *map = m2d::getTexMaps(mLyt.findPane("myDsgn_00"));
    if (keepOrig) {
        mOrigTex0 = map->GetImage();
    }
    map->SetImage(tex);
    map = m2d::getTexMaps(mLyt.findPane("myDsgnThum_00"));
    if (keepOrig) {
        mOrigTex1 = map->GetImage();
    }
    map->SetImage(tex);
    mTex.init(PIXEL_NUM, PIXEL_NUM * 4, tex);
}

// .text:0x3000
void *dMenuEdit_c::getTexture() {
    return getTexture(mTexNo);
}

// .text:0x3008
void *dMenuEdit_c::getTexture(int tex) {
    return &l_tex[mBuf][tex];
}

// .text:0x3028
void dMenuEdit_c::copyTexture(int from, int to) {
    void *src = getTexture(from);
    memcpy(getTexture(to), src, TEX_SIZE);
    DCFlushRange(getTexture(0), TEX_BUF_SIZE);
}

// .text:0x3098
void dMenuEdit_c::swapTexture(int a, int b) {
    static u8 tmp[TEX_SIZE];
    void *texA = getTexture(a);
    void *texB = getTexture(b);
    memcpy(tmp, texA, TEX_SIZE);
    memcpy(texA, texB, TEX_SIZE);
    memcpy(texB, tmp, TEX_SIZE);
    DCFlushRange(getTexture(0), TEX_BUF_SIZE);
}

// .text:0x313C
void dMenuEdit_c::selectTool(int btn) {
    char name[40];
    int tool = btn - BTN_TOOL_FIRST;
    mTool = btn;
    sprintf(name, "N_toolBtn_%02d", tool);
    mToolLyt.setPosFrom(name, "toolBtnFc_01", 0.0f, 0.0f);
    mToolLyt.setPosFrom(name, "toolBtn_Psh_00", 0.0f, 0.0f);
    onFlag(FLAG_CALC_TOOL);
    mToolIcnAnm.setFrame(tool);
    mToolCrsr.setVisible("N_pen_all", false);
    mToolCrsr.setVisible("N_bkt_all", false);
    mToolCrsr.setVisible("N_heart_all", false);
    mToolCrsr.setVisible("N_star_all", false);
    offFlag(FLAG_PEN_FRAME);
    offFlag(FLAG_ARROW);
    offFlag(FLAG_RULER);
    switch (btn - BTN_TOOL_FIRST) {
    case TOOL_PEN_S:
    case TOOL_PEN_M:
    case TOOL_PEN_L:
        mToolCrsr.setVisible("N_pen_all", true);
        onFlag(FLAG_PEN_FRAME);
        updatePenFrame();
        break;
    case TOOL_FILL:
        mToolCrsr.setVisible("N_bkt_all", true);
        onFlag(FLAG_PEN_FRAME);
        updatePenFrame();
        break;
    case TOOL_COAT:
    case TOOL_DOTS:
    case TOOL_STRIPES:
        mToolCrsr.setVisible("N_bkt_all", true);
        break;
    case TOOL_HEART:
        mToolCrsr.setVisible("N_heart_all", true);
        break;
    case TOOL_STAR:
        mToolCrsr.setVisible("N_star_all", true);
        break;
    case TOOL_LINE:
        onFlag(FLAG_ARROW);
        onFlag(FLAG_PEN_FRAME);
        updatePenFrame();
        break;
    case TOOL_RECT:
    case TOOL_OVAL:
        onFlag(FLAG_RULER);
        break;
    }
}

// .text:0x333C
dDesign_c *dMenuEdit_c::getDesign(int index) {
    if (mMenuNo == dMenuMng_c::MENU_EDIT_DESIGN) {
        return dMenuMng_c::getDesign();
    } else if (mMenuNo == dMenuMng_c::MENU_EDIT_PRO) {
        return dMenuMng_c::getDesign();
    } else if (mMenuNo == dMenuMng_c::MENU_EDIT_FACE) {
        return &dSaveData_c::getDesignBoard()->mDesign;
    } else {
        return dPlayerMgr::Mgr_c::getCurrentPlayer()->mOrgDesigns.getDesign(
            (u8)index);
    }
}

// .text:0x33AC
void dMenuEdit_c::setHover(int btn) {
    if (mHover != btn) {
        deselectBtn(mHover);
        selectBtn(btn);
        if (btn != BTN_NONE && btn != BTN_CANVAS && btn != BTN_THUMB) {
            playSeCursor();
        }
        mHover = btn;
    }
}

static const char *l_btnLFc0 = "btnL_fc_00";
static const char *l_btnRFc = "btnR_Fc";
static const char *l_toolBtnFc = "toolBtnFc_00";
static const char *l_btnLFc1 = "btnL_fc_01";
static const char *l_clrBtnFc = "clrBtnFc_00";
static const char *l_clrCrsRFc = "clrCrs_00_Fc";
static const char *l_clrCrsLFc = "clrCrsr_10_Fc";

// .text:0x3420
void dMenuEdit_c::setBtnSelect(int btn, bool on) {
    char name[40];
    if (btn == BTN_TOOL || btn == BTN_TOOL_BACK) {
        mToolLyt.setVisible(l_btnLFc0, on);
    }
    if (isTool(btn)) {
        if (on) {
            sprintf(name, "N_toolBtn_%02d", btn - BTN_TOOL_FIRST);
            mToolLyt.setPosFrom(name, l_toolBtnFc, 0.0f, 0.0f);
            onFlag(FLAG_CALC_TOOL);
        }
        mToolLyt.setVisible(l_toolBtnFc, on);
    }
    if (btn == BTN_COLOR || btn == BTN_COLOR_BACK) {
        mColorLyt.setVisible(l_btnLFc1, on);
    }
    if (isColor(btn)) {
        if (on) {
            sprintf(name, "N_clrBtn_%02d", btn - BTN_COLOR_FIRST);
            mColorLyt.setPosFrom(name, l_clrBtnFc, 0.0f, 0.0f);
            onFlag(FLAG_CALC_COLOR);
        }
        mColorLyt.setVisible(l_clrBtnFc, on);
    }
    if (btn == BTN_PALETTE_NEXT) {
        mColorLyt.setVisible(l_clrCrsLFc, on);
    }
    if (btn == BTN_PALETTE_PREV) {
        mColorLyt.setVisible(l_clrCrsRFc, on);
    }
    if (btn >= BTN_GRID && btn <= BTN_UNDO) {
        mLyt.setVisible(l_btnRFc, on);
        sprintf(name, "N_btnR_%02d_all", btn);
        if (on) {
            mLyt.setPosFrom(name, l_btnRFc, 0.0f, 0.0f);
            mLyt.setPosFrom(name, "btnR_psh", 0.0f, 0.0f);
        }
        onFlag(FLAG_CALC);
    }
    if (isMenuBtn(btn)) {
        mBtn.setSelect(btn - BTN_QUIT, on, FALSE);
    }
    if (btn == BTN_PREVIEW) {
        mPreviewAnm.setNext(!on);
    }
    if (isFlag(FLAG_PRO)) {
        if (btn == BTN_PARTS || btn == BTN_PARTS_BACK) {
            mChngBtnAnm.setNext(!on);
        } else if (isPart(btn)) {
            mPartsLyt.setVisible("partsBtnFc_00", on);
            if (on) {
                sprintf(name, "N_parts_%02d", btn - BTN_PART_FIRST);
                mPartsLyt.setPosFrom(name, "partsBtnFc_00", 0.0f, 0.0f);
            }
            onFlag(FLAG_CALC_PARTS);
        }
    }
}

// .text:0x3728
void dMenuEdit_c::selectBtn(int btn) {
    setBtnSelect(btn, TRUE);
}

// .text:0x3730
void dMenuEdit_c::deselectBtn(int btn) {
    setBtnSelect(btn, FALSE);
}

// .text:0x3738
void dMenuEdit_c::clearSelect() {
    mHover = BTN_NONE;
    mLyt.setVisible(l_btnRFc, false);
    mToolLyt.setVisible(l_btnLFc0, false);
    mToolLyt.setVisible(l_toolBtnFc, false);
    mColorLyt.setVisible(l_btnLFc1, false);
    mColorLyt.setVisible(l_clrBtnFc, false);
    mColorLyt.setVisible(l_clrCrsRFc, false);
    mColorLyt.setVisible(l_clrCrsLFc, false);
}

// .text:0x37E8
void dMenuEdit_c::setupBtns() {
    mBtn.setText(0, MSG_QUIT, TRUE);
    mBtn.setVisible(1, false);
    mBtn.setText(2, MSG_DONE, FALSE);
}

// .text:0x3848
void dMenuEdit_c::showTutorial(int btn) {
    u16 msg = 0;
    nw4r::lyt::Pane *pane = NULL;
    switch (btn) {
    case BTN_GRID:
        msg = BALLOON_GRID;
        pane = mLyt.findPane("N_btnR_00_all");
        break;
    case BTN_ZOOM_IN:
        msg = BALLOON_ZOOM_IN;
        pane = mLyt.findPane("N_btnR_01_all");
        break;
    case BTN_ZOOM_OUT:
        msg = BALLOON_ZOOM_OUT;
        pane = mLyt.findPane("N_btnR_02_all");
        break;
    case BTN_UNDO:
        msg = BALLOON_UNDO;
        pane = mLyt.findPane("N_btnR_03_all");
        break;
    case BTN_TOOL:
        msg = BALLOON_TOOLS;
        pane = mLyt.findPane("N_btnL_00_all");
        break;
    case BTN_COLOR:
        msg = BALLOON_COLORS;
        pane = mLyt.findPane("N_btnL_01_all");
        break;
    case BTN_TOOL_BACK:
        msg = BALLOON_CLOSE;
        pane = mLyt.findPane("N_btnL_00_all");
        break;
    case BTN_COLOR_BACK:
        msg = BALLOON_CLOSE_2;
        pane = mLyt.findPane("N_btnL_01_all");
        break;
    case BTN_PARTS_BACK:
        msg = BALLOON_CLOSE_2;
        pane = mPartsLyt.findPane("N_chngBtn");
        break;
    case BTN_PARTS:
        msg = BALLOON_OTHER_PARTS;
        pane = mPartsLyt.findPane("N_chngBtn");
        break;
    case BTN_PART_FIRST:
        msg = BALLOON_FRONT;
        pane = mPartsLyt.findPane("N_clothes_00");
        break;
    case BTN_PART_FIRST + 1:
        msg = BALLOON_SLEEVE_R;
        pane = mPartsLyt.findPane("N_clothes_01");
        break;
    case BTN_PART_FIRST + 2:
        msg = BALLOON_SLEEVE_L;
        pane = mPartsLyt.findPane("N_clothes_02");
        break;
    case BTN_PART_LAST:
        msg = BALLOON_BACK;
        pane = mPartsLyt.findPane("N_clothes_03");
        break;
    }
    if (btn >= BTN_TOOL_FIRST && btn <= BTN_TOOL_LAST) {
        char name[40];
        int tool = btn - BTN_TOOL_FIRST;
        sprintf(name, "N_toolBtn_%02d", tool);
        pane = mToolLyt.findPane(name);
        msg = tool + BALLOON_TOOL_FIRST;
    }
    if (msg != 0 && pane != NULL) {
        if (dObjTutorial_c::request(0, msg, TRUE)) {
            mVec3_c pos;
            m2d::getScreenPos(&pos, pane);
            mVec2_c balloon;
            balloon.x = pos.x;
            balloon.y = pos.y;
            dObjTutorial_c::setMsg(&balloon, msg, 1);
        }
    }
}

// .text:0x3A8C
void dMenuEdit_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x3A90
void dMenuEdit_c::initPreview() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_EDIT_DESIGN:
        setPreviewKind(PREVIEW_CLOTH);
        if (dOrgObj_c::sInstance != NULL) {
            dOrgObj_c::sInstance->setClothAnm(-1);
        }
        break;
    case dMenuMng_c::MENU_EDIT_PRO:
        setPreviewKind(PREVIEW_CLOTH);
        if (dOrgObj_c::sInstance != NULL) {
            dOrgObj_c::sInstance->setClothAnm(-1);
        }
        break;
    case dMenuMng_c::MENU_EDIT_FACE:
        setPreviewKind(PREVIEW_CAT);
        break;
    }
    updatePreviewTexture();
}

// .text:0x3B38
void dMenuEdit_c::setPreviewKind(int kind) {
    switch (kind) {
    case PREVIEW_CLOTH:
        if (dOrgObj_c::sInstance != NULL) {
            dOrgObj_c *org = dOrgObj_c::sInstance;
            org->mDrawFlags = 0;
            org->mDrawFlags |= 1 << dOrgObj_c::DRAW_CLOTH;
        }
        break;
    case PREVIEW_UMB:
        if (dOrgObj_c::sInstance != NULL) {
            dOrgObj_c *org = dOrgObj_c::sInstance;
            org->mDrawFlags = 0;
            org->mDrawFlags |= 1 << dOrgObj_c::DRAW_UMB;
        }
        break;
    case PREVIEW_CAP:
        if (dOrgObj_c::sInstance != NULL) {
            dOrgObj_c *org = dOrgObj_c::sInstance;
            org->mDrawFlags = 0;
            org->mDrawFlags |= 1 << dOrgObj_c::DRAW_CAP;
        }
        break;
    case PREVIEW_CAT:
        if (dOrgObj_c::sInstance != NULL) {
            dOrgObj_c *org = dOrgObj_c::sInstance;
            org->mDrawFlags = 0;
            org->mDrawFlags |= 1 << dOrgObj_c::DRAW_CAT;
        }
        break;
    }
    mPreviewKind = kind;
}

// .text:0x3BE4
void dMenuEdit_c::hidePreview() {
    if (dOrgObj_c::sInstance != NULL) {
        dOrgObj_c::sInstance->mDrawFlags = 0;
    }
}

// .text:0x3C00
void dMenuEdit_c::calcPreview() {
    static const f32 l_scale[PREVIEW_NUM] = {0.77f, 0.69f, 0.84f, 0.6f};
    static mVec3_c l_pos[PREVIEW_NUM] = {
        mVec3_c(-45.0f, 14.0f, -174.7f), mVec3_c(-45.5f, 16.8f, -174.7f),
        mVec3_c(-45.5f, 19.0f, -174.7f), mVec3_c(-46.0f, 14.0f, -174.7f)};
    f32 scale = l_scale[mPreviewKind];
    mMtx_c mtx;
    mVec3_c pos = l_pos[mPreviewKind];
    mtx.trans(pos);
    mtx.XrotM(mAng(0x1400));
    mtx.YrotM(mTurn);
    mtx.scaleM(scale, scale, scale);
    if (dOrgObj_c::sInstance != NULL) {
        dOrgObj_c::sInstance->calc(&mtx);
    }
    m2d::addCallback(&mOrder);
    mZClear.entry();
}

// .text:0x3DBC
void dMenuEdit_c::updatePreviewTexture() {
    u8 *tex = (u8 *)getTexture(0);
    if (!isFlag(FLAG_PRO) && mPreviewKind == PREVIEW_CLOTH) {
        memcpy(tex + TEX_SIZE, tex, TEX_SIZE);
        memcpy(tex + TEX_SIZE * 2, tex, TEX_SIZE);
        memcpy(tex + TEX_SIZE * 3, tex, TEX_SIZE);
    }
    const u16 *palette = dDesign_c::getPaletteData(l_palette);
    if (dOrgObj_c::sInstance != NULL) {
        dOrgObj_c::sInstance->replaceTexture(tex, (void *)palette);
        dOrgObj_c::sInstance->bindCat();
        dOrgObj_c::sInstance->bindCloth();
        dOrgObj_c::sInstance->bindUmb();
        dOrgObj_c::sInstance->bindCap();
    }
}

// .text:0x3E90
void dMenuEdit_c::nextPreviewKind() {
    if (mMenuNo == dMenuMng_c::MENU_EDIT_DESIGN) {
        if (mPreviewKind >= PREVIEW_CAP) {
            setPreviewKind(PREVIEW_CLOTH);
        } else {
            setPreviewKind(mPreviewKind + 1);
        }
        updatePreviewTexture();
        playSeDecide();
    }
}

// .text:0x3EF0
void dMenuEdit_c::order_c::execute() {
    if (dOrgObj_c::sInstance != NULL) {
        dOrgObj_c::sInstance->drawCloth();
        dOrgObj_c::sInstance->drawUmb();
        dOrgObj_c::sInstance->drawCap();
        dOrgObj_c::sInstance->drawCat();
    }
}

// .text:0x3F40
int dMenuEdit_c::hitLayout(f32 scale, mVec2_c pos, m2d::Simple_c *lyt,
                           int rectBase, int rectMax, int circleBase,
                           int circleMax) {
    int hit;
    if (!dObjCursor_c::isVisible()) {
        hit = -1;
    } else {
        mVec2_c p = pos;
        hit = m2d::hitButton(&p, lyt, rectBase, rectMax, circleBase, circleMax,
                             scale);
    }
    return hit;
}

// .text:0x3FDC
int dMenuEdit_c::hitCheck(f32 scale, mVec2_c pos) {
    int hit = hitLayout(scale, pos, &mLyt, 0, 6, 7, 8);
    if (hit != BTN_NONE) {
        return hit;
    }
    hit = mBtn.hitCheck(&pos);
    if (hit != BTN_NONE) {
        return hit + BTN_QUIT;
    }
    if (isFlag(FLAG_PRO)) {
        hit = hitLayout(scale, pos, &mPartsLyt, BTN_PART_FIRST, BTN_PARTS_BACK,
                        -1, -1);
        if (hit != BTN_NONE) {
            return BTN_PARTS;
        }
    }
    return BTN_NONE;
}

// .text:0x40D4
int dMenuEdit_c::hitCheckTool(f32 scale, mVec2_c pos) {
    return hitLayout(scale, pos, &mToolLyt, BTN_TOOL_FIRST, BTN_TOOL_LAST,
                     BTN_TOOL_BACK, BTN_TOOL_BACK);
}

// .text:0x411C
int dMenuEdit_c::hitCheckColor(f32 scale, mVec2_c pos) {
    int hit = hitLayout(scale, pos, &mColorLyt, BTN_COLOR_FIRST,
                        BTN_PALETTE_NEXT, -1, -1);
    if (hit != BTN_NONE) {
        return hit;
    }
    m2d::hitArg_c arg;
    arg.mpHit = NULL;
    arg.mScale = scale;
    arg.mPos = pos;
    mColorLyt.patrolPane(m2d::isInPane, m2d::isCircleButton, &arg);
    if (arg.mpHit != NULL) {
        return BTN_COLOR_BACK;
    }
    return BTN_NONE;
}

// .text:0x41EC
int dMenuEdit_c::hitCheckParts(mVec2_c pos, BOOL grab) {
    int hit = hitLayout(1.0f, pos, &mPartsLyt, BTN_PART_FIRST, BTN_PARTS_BACK,
                        -1, -1);
    if (grab == TRUE && hit >= BTN_PART_FIRST && hit <= BTN_PART_LAST) {
        mVec2_c target = getPartPos(hit, 0.0f, 0.0f);
        mVec2_c ofs;
        ofs.x = target.x - pos.x;
        ofs.y = target.y - pos.y;
        mMove.setOfs(ofs);
    }
    return hit;
}

// .text:0x42C8
void dMenuEdit_c::scrollToPointer() {
    mVec2_c pos = dObjCursor_c::getPos();
    nw4r::lyt::Pane *pane = mLyt.findPane("myDsgnThum_00");
    mVec3_c panePos;
    m2d::getScreenPos(&panePos, pane);
    pos.x = pos.x - panePos.x;
    pos.y = pos.y - panePos.y;
    nw4r::lyt::Size size = pane->GetSize();
    pos.x /= size.width;
    pos.y /= -size.height;
    pos.x *= 32.0f;
    pos.y *= 32.0f;
    setScroll((int)pos.x, (int)pos.y);
}

// .text:0x4394
mVec2_c dMenuEdit_c::getFilteredPointer() {
    mVec2_c pointer = dObjCursor_c::getPos();
    mVec2_c result;
    result.x = mFilterX + 0.85f * (pointer.x - mFilterX);
    result.y = mFilterY + 0.85f * (pointer.y - mFilterY);
    return result;
}

// .text:0x4408
void dMenuEdit_c::startFilter() {
    offFlag(FLAG_FILTER);
    mVec2_c pointer = dObjCursor_c::getPos();
    setPixelFromPos(&pointer);
    mFilterX = pointer.x;
    mFilterY = pointer.y;
}

// .text:0x445C
void dMenuEdit_c::updateFilter() {
    mVec2_c pointer = getFilteredPointer();
    setPixelFromPos(&pointer);
    onFlag(FLAG_FILTER);
}

// .text:0x44A8
void dMenuEdit_c::setPixelFromPos(const mVec2_c *pos) {
    mVec2_c p = *pos;
    nw4r::lyt::Pane *pane = mLyt.findPane("myDsgn_00");
    mVec3_c screen;
    m2d::getScreenPos(&screen, pane);
    p.x = p.x - screen.x;
    p.y = p.y - screen.y;
    nw4r::lyt::Size size = pane->GetSize();
    if (SCGetAspectRatio() == 1) {
        f32 width = size.width;
        width = width * 0.730769f;
        p.x /= width;
    } else {
        f32 width = size.width;
        p.x /= width;
    }
    p.y /= -size.height;
    nw4r::lyt::TexSRT *srt = m2d::getTexSRTs(pane);
    p.x *= 32.0f * srt->scale.x;
    p.y *= 32.0f * srt->scale.y;
    p.x += 16.0f;
    p.y += 16.0f;
    int x = (int)p.x + mScrollX;
    int y = (int)p.y + mScrollY;
    if (x < 0) {
        x = 0;
    } else if (x >= PIXEL_NUM) {
        x = PIXEL_NUM - 1;
    }
    if (y < 0) {
        y = 0;
    } else if (y >= PIXEL_NUM) {
        y = PIXEL_NUM - 1;
    }
    mPixelX = x;
    mPixelY = y;
}

// .text:0x463C
mVec2_c dMenuEdit_c::getPixelPos(int x, int y, int ofsX, int ofsY) {
    int dx = (int)((f32)(x - mScrollX) - 16.0f);
    int dy = (int)((f32)(y - mScrollY) - 16.0f);
    nw4r::lyt::Pane *pane = mLyt.findPane("myDsgn_00");
    mVec3_c screen;
    m2d::getScreenPos(&screen, pane);
    nw4r::lyt::Size size = pane->GetSize();
    nw4r::lyt::TexSRT *srt = m2d::getTexSRTs(pane);
    f32 pixelW = size.width;
    pixelW = pixelW / (32.0f * srt->scale.x);
    if (SCGetAspectRatio() == 1) {
        pixelW *= 0.730769f;
    }
    f32 pixelH = size.height;
    pixelH = pixelH / (32.0f * srt->scale.y);
    mVec2_c result;
    result.x = screen.x + pixelW * dx + 0.5f * (ofsX * pixelW);
    result.y = screen.y + pixelH * -dy + 0.5f * (-ofsY * pixelH);
    return result;
}

// .text:0x47F4
void dMenuEdit_c::drawLine(int x0, int y0, int x1, int y1, int color,
                           int size) {
    int dx2X;
    int dx2Y;
    int dy2X;
    int dy2Y;
    int errY;
    int iY;
    int x;
    int errX;
    int y;
    int dx;
    int dy;
    int iX;
    int sx;
    int sy;
    if (x1 > x0) {
        dx = x1 - x0;
        sx = 1;
    } else {
        sx = -1;
        dx = x0 - x1;
    }
    if (y1 > y0) {
        dy = y1 - y0;
        sy = 1;
    } else {
        sy = -1;
        dy = y0 - y1;
    }
    x = x0;
    y = y0;
    if (dx >= dy) {
        dy2X = dy * 2;
        errX = -dx;
        dx2X = dx * 2;
        for (iX = 0; iX <= dx; iX++) {
            drawDot(x, y, color, size);
            errX += dy2X;
            x += sx;
            if (errX >= 0) {
                y += sy;
                errX -= dx2X;
            }
        }
    } else {
        errY = -dy;
        dx2Y = dx * 2;
        dy2Y = dy * 2;
        for (iY = 0; iY <= dy; iY++) {
            drawDot(x, y, color, size);
            errY += dx2Y;
            y += sy;
            if (errY >= 0) {
                x += sx;
                errY -= dy2Y;
            }
        }
    }
}

// The target's trailing dead blr (fn_4954) belongs to setPixel.
// .text:0x4910
void dMenuEdit_c::setPixel(int x, int y, int color) {
    if (x < 0 || x >= PIXEL_NUM) {
        return;
    }
    if (y < 0 || y >= PIXEL_NUM) {
        return;
    }
    mTex.set(x, y, color, FALSE);
}

// .text:0x4958
void dMenuEdit_c::drawDot(int x, int y, int color, int size) {
    setPixel(x, y, color);
    if (size >= 1) {
        setPixel(x - 1, y, color);
        setPixel(x - 1, y + 1, color);
        setPixel(x, y + 1, color);
        if (size >= 2) {
            setPixel(x - 1, y - 1, color);
            setPixel(x, y - 1, color);
            setPixel(x + 1, y - 1, color);
            setPixel(x + 1, y, color);
            setPixel(x + 1, y + 1, color);
        }
    }
}

// .text:0x4A4C
BOOL dMenuEdit_c::beginTool() {
    BOOL result = TRUE;
    backupUndo();
    mTex::edit4b_c tex(0, 0, NULL);
    int x = mPixelX;
    int y = mPixelY;
    mStartX = x;
    mStartY = y;
    mPrevX = x;
    mPrevY = y;
    switch (mTool) {
    case BTN_TOOL_FIRST + TOOL_PEN_S:
        drawDot(x, y, mColor, 0);
        playSePen3();
        break;
    case BTN_TOOL_FIRST + TOOL_PEN_M:
        drawDot(x, y, mColor, 1);
        playSePen2();
        break;
    case BTN_TOOL_FIRST + TOOL_PEN_L:
        drawDot(x, y, mColor, 2);
        playSePen();
        break;
    case BTN_TOOL_FIRST + TOOL_LINE:
        placeArrowEnd();
        onFlag(FLAG_ARROW_END);
        playSeSetStart();
        break;
    case BTN_TOOL_FIRST + TOOL_RECT:
    case BTN_TOOL_FIRST + TOOL_OVAL:
        placeRuler();
        onFlag(FLAG_RULER_END);
        playSeSetStart();
        break;
    case BTN_TOOL_FIRST + TOOL_HEART:
        drawStamp(x, y, mColor, STAMP_HEART);
        result = FALSE;
        playSePaintDecide();
        break;
    case BTN_TOOL_FIRST + TOOL_STAR:
        drawStamp(x, y, mColor, STAMP_STAR);
        result = FALSE;
        playSePaintDecide();
        break;
    case BTN_TOOL_FIRST + TOOL_FILL:
        floodFill(x, y, mColor);
        result = FALSE;
        playSePaintDecide();
        break;
    case BTN_TOOL_FIRST + TOOL_COAT:
        fillAll(mColor);
        result = FALSE;
        playSePaintDecide();
        break;
    case BTN_TOOL_FIRST + TOOL_DOTS:
        drawDots(mColor);
        result = FALSE;
        playSePaintDecide();
        break;
    case BTN_TOOL_FIRST + TOOL_STRIPES:
        drawStripes(mColor);
        result = FALSE;
        playSePaintDecide();
        break;
    default:
        result = FALSE;
        break;
    }
    mTex.flush();
    updatePreviewTexture();
    return result;
}

// .text:0x4C64
void dMenuEdit_c::backupUndo() {
    void *src = l_tex[mUndoBuf = mBuf];
    void *dst = l_tex[(mBuf + 1) & 1];
    memcpy(dst, src, TEX_BUF_SIZE);
    DCFlushRange(dst, TEX_BUF_SIZE);
}

// .text:0x4CC4
void dMenuEdit_c::restoreUndo() {
    dDesignTex_c *dst = l_tex[mBuf];
    memcpy(dst, l_tex[(mBuf + 1) & 1], TEX_BUF_SIZE);
    DCFlushRange(dst, TEX_BUF_SIZE);
}

// .text:0x4D20
void dMenuEdit_c::undo() {
    mBuf = (mBuf + 1) & 1;
    attachTexture(FALSE);
    if (mUndoBuf == mBuf) {
        playSeRedo();
    } else {
        playSeUndo();
    }
    updatePreviewTexture();
}

// .text:0x4D8C
void dMenuEdit_c::pickColor() {
    int color = mTex.get(mPixelX, mPixelY);
    if (color == 0) {
        color = PALETTE_COLOR_NUM - 1;
    }
    setColor(color);
    playSeSpoit();
}

// .text:0x4DE0
int dMenuEdit_c::findPixel(int x0, int x1, int y, int color) {
    for (int x = x0; x <= x1; x++) {
        int pixel = mTex.get(x, y);
        if (pixel == color) {
            return x;
        }
    }
    return -1;
}

// .text:0x4E58
int dMenuEdit_c::findSpanStart(int x, int y, int color) {
    if (x <= 0) {
        return 0;
    }
    for (int i = x - 1; i >= 0; i--) {
        int pixel = mTex.get(i, y);
        if (pixel != color) {
            return i + 1;
        }
    }
    return 0;
}

// .text:0x4EEC
int dMenuEdit_c::findSpanEnd(int x, int y, int color) {
    if (x >= PIXEL_NUM - 1) {
        return PIXEL_NUM - 1;
    }
    for (int i = x + 1; i <= PIXEL_NUM - 1; i++) {
        int pixel = mTex.get(i, y);
        if (pixel != color) {
            return i - 1;
        }
    }
    return PIXEL_NUM - 1;
}

// .text:0x4F80
void dMenuEdit_c::floodFill(int x, int y, int color) {
    static const int l_queueMask = FILL_QUEUE_NUM - 1;
    static u8 queue[FILL_QUEUE_NUM][4];
    int end;
    int start;
    int rowBelow;
    int right;
    int foundBelow;
    int target;
    int lastBelow;
    int rowAbove;
    int head;
    int tail;
    int row;
    int pixel;
    u8 *entry;
    int i;
    int foundAbove;
    int left;
    int lastAbove;
    int scanAbove;
    target = mTex.get(x, y);
    if (target == color) {
        return;
    }
    head = 0;
    tail = 1;
    queue[0][0] = x;
    queue[0][1] = x;
    queue[0][2] = y;
    do {
        entry = queue[head];
        head++;
        left = entry[0];
        row = entry[2];
        right = entry[1];
        head &= l_queueMask;
        pixel = mTex.get(left, row);
        if (color == pixel) {
            continue;
        }
        start = findSpanStart(left, row, target);
        end = findSpanEnd(right, row, target);
        for (i = start; i <= end; i++) {
            mTex.set(i, row, color, FALSE);
        }
        if (row > 0) {
            rowAbove = row - 1;
            scanAbove = start;
            do {
                foundAbove = findPixel(scanAbove, end, rowAbove, target);
                if (foundAbove < 0) {
                    scanAbove = 0xFF;
                } else {
                    lastAbove = findSpanEnd(foundAbove, rowAbove, target);
                    queue[tail][0] = foundAbove;
                    queue[tail][1] = lastAbove;
                    queue[tail][2] = rowAbove;
                    tail++;
                    tail &= l_queueMask;
                    scanAbove = lastAbove + 2;
                }
            } while (scanAbove <= end);
        }
        if (row < PIXEL_NUM - 1) {
            rowBelow = row + 1;
            do {
                foundBelow = findPixel(start, end, rowBelow, target);
                if (foundBelow < 0) {
                    start = 0xFF;
                } else {
                    lastBelow = findSpanEnd(foundBelow, rowBelow, target);
                    queue[tail][0] = foundBelow;
                    queue[tail][1] = lastBelow;
                    queue[tail][2] = rowBelow;
                    tail++;
                    tail &= l_queueMask;
                    start = lastBelow + 2;
                }
            } while (start <= end);
        }
    } while (head != tail);
}

// .text:0x517C
void dMenuEdit_c::fillAll(int color) {
    memset(getTexture(), 0x11111111 * color, TEX_SIZE);
}

// .text:0x51BC
void dMenuEdit_c::drawRect(int x0, int y0, int x1, int y1, int color) {
    if (x0 > x1) {
        u8 tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    if (y0 > y1) {
        u8 tmp = y0;
        y0 = y1;
        y1 = tmp;
    }
    for (u8 x = x0; x <= x1; x++) {
        mTex.set(x, y0, color, FALSE);
        mTex.set(x, y1, color, FALSE);
    }
    for (u8 y = y0; y <= y1; y++) {
        mTex.set(x0, y, color, FALSE);
        mTex.set(x1, y, color, FALSE);
    }
}

// .text:0x52E8
void dMenuEdit_c::drawOval(int x0, int y0, int x1, int y1, int color) {
    static const int l_round = 45;
    static const int l_steps = 64;
    static const u16 l_ovalTable[320] = {
        0x0000, 0x0006, 0x000C, 0x0012, 0x0019, 0x001F, 0x0025, 0x002B, 0x0031,
        0x0038, 0x003E, 0x0044, 0x004A, 0x0050, 0x0056, 0x005C, 0x0061, 0x0067,
        0x006D, 0x0073, 0x0078, 0x007E, 0x0083, 0x0088, 0x008E, 0x0093, 0x0098,
        0x009D, 0x00A2, 0x00A7, 0x00AB, 0x00B0, 0x00B5, 0x00B9, 0x00BD, 0x00C1,
        0x00C5, 0x00C9, 0x00CD, 0x00D1, 0x00D4, 0x00D8, 0x00DB, 0x00DE, 0x00E1,
        0x00E4, 0x00E7, 0x00EA, 0x00EC, 0x00EE, 0x00F1, 0x00F3, 0x00F4, 0x00F6,
        0x00F8, 0x00F9, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FE, 0x00FF, 0x00FF,
        0x00FF, 0x0100, 0x00FF, 0x00FF, 0x00FF, 0x00FE, 0x00FE, 0x00FD, 0x00FC,
        0x00FB, 0x00F9, 0x00F8, 0x00F6, 0x00F4, 0x00F3, 0x00F1, 0x00EE, 0x00EC,
        0x00EA, 0x00E7, 0x00E4, 0x00E1, 0x00DE, 0x00DB, 0x00D8, 0x00D4, 0x00D1,
        0x00CD, 0x00C9, 0x00C5, 0x00C1, 0x00BD, 0x00B9, 0x00B5, 0x00B0, 0x00AB,
        0x00A7, 0x00A2, 0x009D, 0x0098, 0x0093, 0x008E, 0x0088, 0x0083, 0x007E,
        0x0078, 0x0073, 0x006D, 0x0067, 0x0061, 0x005C, 0x0056, 0x0050, 0x004A,
        0x0044, 0x003E, 0x0038, 0x0031, 0x002B, 0x0025, 0x001F, 0x0019, 0x0012,
        0x000C, 0x0006, 0x0000, 0xFFFA, 0xFFF4, 0xFFEE, 0xFFE7, 0xFFE1, 0xFFDB,
        0xFFD5, 0xFFCF, 0xFFC8, 0xFFC2, 0xFFBC, 0xFFB6, 0xFFB0, 0xFFAA, 0xFFA4,
        0xFF9F, 0xFF99, 0xFF93, 0xFF8D, 0xFF88, 0xFF82, 0xFF7D, 0xFF78, 0xFF72,
        0xFF6D, 0xFF68, 0xFF63, 0xFF5E, 0xFF59, 0xFF55, 0xFF50, 0xFF4B, 0xFF47,
        0xFF43, 0xFF3F, 0xFF3B, 0xFF37, 0xFF33, 0xFF2F, 0xFF2C, 0xFF28, 0xFF25,
        0xFF22, 0xFF1F, 0xFF1C, 0xFF19, 0xFF16, 0xFF14, 0xFF12, 0xFF0F, 0xFF0D,
        0xFF0C, 0xFF0A, 0xFF08, 0xFF07, 0xFF05, 0xFF04, 0xFF03, 0xFF02, 0xFF02,
        0xFF01, 0xFF01, 0xFF01, 0xFF00, 0xFF01, 0xFF01, 0xFF01, 0xFF02, 0xFF02,
        0xFF03, 0xFF04, 0xFF05, 0xFF07, 0xFF08, 0xFF0A, 0xFF0C, 0xFF0D, 0xFF0F,
        0xFF12, 0xFF14, 0xFF16, 0xFF19, 0xFF1C, 0xFF1F, 0xFF22, 0xFF25, 0xFF28,
        0xFF2C, 0xFF2F, 0xFF33, 0xFF37, 0xFF3B, 0xFF3F, 0xFF43, 0xFF47, 0xFF4B,
        0xFF50, 0xFF55, 0xFF59, 0xFF5E, 0xFF63, 0xFF68, 0xFF6D, 0xFF72, 0xFF78,
        0xFF7D, 0xFF82, 0xFF88, 0xFF8D, 0xFF93, 0xFF99, 0xFF9F, 0xFFA4, 0xFFAA,
        0xFFB0, 0xFFB6, 0xFFBC, 0xFFC2, 0xFFC8, 0xFFCF, 0xFFD5, 0xFFDB, 0xFFE1,
        0xFFE7, 0xFFEE, 0xFFF4, 0xFFFA, 0x0000, 0x0006, 0x000C, 0x0012, 0x0019,
        0x001F, 0x0025, 0x002B, 0x0031, 0x0038, 0x003E, 0x0044, 0x004A, 0x0050,
        0x0056, 0x005C, 0x0061, 0x0067, 0x006D, 0x0073, 0x0078, 0x007E, 0x0083,
        0x0088, 0x008E, 0x0093, 0x0098, 0x009D, 0x00A2, 0x00A7, 0x00AB, 0x00B0,
        0x00B5, 0x00B9, 0x00BD, 0x00C1, 0x00C5, 0x00C9, 0x00CD, 0x00D1, 0x00D4,
        0x00D8, 0x00DB, 0x00DE, 0x00E1, 0x00E4, 0x00E7, 0x00EA, 0x00EC, 0x00EE,
        0x00F1, 0x00F3, 0x00F4, 0x00F6, 0x00F8, 0x00F9, 0x00FB, 0x00FC, 0x00FD,
        0x00FE, 0x00FE, 0x00FF, 0x00FF, 0x00FF,
    };
    u8 px;
    int cx;
    int cy;

    if (x0 > x1) {
        u8 tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    if (y0 > y1) {
        u8 tmp = y0;
        y0 = y1;
        y1 = tmp;
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int rx = dx >> 1;
    int ry = dy >> 1;
    cx = x0 + rx;
    cy = y0 + ry;
    // @HACK: The y-first aggregate initialization matters - idk man
    struct Center {
        int y;
        int x;
    };
    Center center = {cy, cx};
    center.x = (dx & 1) + cx;
    center.y = (dy & 1) + cy;
    s16 i;
    int a;
    int b;
    int step;
    step = l_steps / (((rx + ry) - (((rx + ry) >> 1) - ((rx + ry) >> 3) -
                                    (cM::absI(rx - ry) >> 1) - 5) |
                       1) +
                      1);
    for (i = 0; i < l_steps; i += step) {
        a = (rx * l_ovalTable[i + l_steps] + l_round) >> 8;
        b = (ry * l_ovalTable[i] + l_round) >> 8;
        px = a + center.x;
        mTex.set(px, b + center.y, color, FALSE);
        mTex.set(px, cy - b, color, FALSE);
        u8 qx = cx - a;
        mTex.set(qx, b + center.y, color, FALSE);
        mTex.set(qx, cy - b, color, FALSE);
    }
}

// .text:0x54A4
void dMenuEdit_c::drawDots(int color) {
    static const u16 l_dotsMask[16] = {
        0xF00F, 0xF00F, 0xE007, 0xC003, 0x03C0, 0x07E0, 0x0FF0, 0x0FF0,
        0x0FF0, 0x0FF0, 0x07E0, 0x03C0, 0xC003, 0xE007, 0xF00F, 0xF00F};
    int x, y;
    for (y = 0; y < 16; y++) {
        u16 row = l_dotsMask[y];
        for (x = 0; x < 16; x++) {
            if (row & (1 << x)) {
                mTex.set(x, y, color, FALSE);
                mTex.set(x + 16, y, color, FALSE);
                mTex.set(x, y + 16, color, FALSE);
                mTex.set(x + 16, y + 16, color, FALSE);
            }
        }
    }
}

// .text:0x55A8
void dMenuEdit_c::drawStripes(int color) {
    static const int l_stripeStart = 2;
    int x, y;
    for (y = l_stripeStart; y < PIXEL_NUM; y += 4) {
        for (x = 0; x < PIXEL_NUM; x++) {
            mTex.set(x, y, color, FALSE);
            mTex.set(x, y + 1, color, FALSE);
        }
    }
}

// .text:0x5654
void dMenuEdit_c::drawStamp(int x, int y, int color, int stamp) {
    static const u16 l_heartMask[16] = {
        0x0000, 0x0000, 0x0000, 0x0000, 0x1C70, 0x3EF8, 0x3FF8, 0x3FF8,
        0x3FF8, 0x1FF0, 0x0FE0, 0x07C0, 0x0100, 0x0000, 0x0000, 0x0000};
    static const u16 l_starMask[16] = {
        0x0000, 0x0000, 0x0000, 0x0100, 0x0100, 0x0380, 0x3FF8, 0x0FE0,
        0x07C0, 0x07C0, 0x0EE0, 0x0C60, 0x1010, 0x0000, 0x0000, 0x0000};
    static const u16 *l_stamp[2] = {l_heartMask, l_starMask};
    const u16 *mask = l_stamp[stamp];
    int j, i;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            if (mask[i] & (1 << j)) {
                setPixel(x + j - 8, y + i - 8, color);
            }
        }
    }
}

// .text:0x56F8
BOOL dMenuEdit_c::playColorBtnAnm() {
    BOOL stopped = mPltBtnAnm.mFrameCtrl.isStop();
    mPltBtnAnm.play();
    onFlag(FLAG_CALC_COLOR);
    return stopped;
}

// .text:0x574C
void dMenuEdit_c::closeColorWin() {
    setColorBtnAnm(1);
    setSubState(SUB_COLOR_ANM);
    setNextSubState(SUB_MAIN);
    setHover(BTN_NONE);
    mBtn.setDisable(0, FALSE);
    mBtn.setDisable(2, FALSE);
}

// .text:0x57CC
void dMenuEdit_c::setColorBtnAnm(int back) {
    static const char *l_anm[2] = {"slct_clr_00_pltBtnPsh.brlan",
                                   "slct_clr_00_bckBtnPsh.brlan"};
    mColorLyt.setAnm(&mPltBtnAnm, l_anm[back], NULL);
}

// .text:0x57EC
void dMenuEdit::setNameSuffix(char *name, void *arg) {
    const char *suffix = (const char *)arg;
    int len = strlen(name);
    name[len - 2] = suffix[0];
    name[len - 1] = suffix[1];
}

// .text:0x5838
void dMenuEdit_c::changePalette(int delta) {
    int palette = l_palette + delta;
    if (palette < 0) {
        palette = PALETTE_NUM - 1;
    }
    if (palette > PALETTE_NUM - 1) {
        palette = 0;
    }
    setPalette(palette);
    updatePreviewTexture();
    playSePageTurnNo(l_palette);
}

// .text:0x58A0
void dMenuEdit_c::changePaletteAnm(int dir) {
    changePalette(dir);
    mColorLyt.unbind(&mClrCrsrAnm);
    mClrCrsrAnm.patrolTargetName(dMenuEdit::setNameSuffix,
                                 (void *)(dir < 0 ? "00" : "10"));
    mColorLyt.bind(&mClrCrsrAnm);
    onFlag(FLAG_CRSR_ANM);
    mClrCrsrAnm.setFrame(0.0f);
}

// .text:0x5934
BOOL dMenuEdit_c::playToolBtnAnm() {
    BOOL stopped = mToolWinBtnAnm.mFrameCtrl.isStop();
    mToolWinBtnAnm.play();
    onFlag(FLAG_CALC_TOOL);
    return stopped;
}

// .text:0x5988
void dMenuEdit_c::closeToolWin() {
    setToolBtnAnm(1);
    setSubState(SUB_TOOL_ANM);
    setNextSubState(SUB_MAIN);
    setHover(BTN_NONE);
    mBtn.setDisable(0, FALSE);
    mBtn.setDisable(2, FALSE);
}

// .text:0x5A08
void dMenuEdit_c::setToolBtnAnm(int back) {
    static const char *l_anm[2] = {"slct_tool_00_toolWinBtnPsh.brlan",
                                   "slct_tool_00_bckBtnPsh.brlan"};
    mToolLyt.setAnm(&mToolWinBtnAnm, l_anm[back], NULL);
}

// .text:0x5A28
void dMenuEdit_c::setColor(int color) {
    if (color < 1) {
        color = PALETTE_COLOR_NUM - 1;
    } else if (color > PALETTE_COLOR_NUM - 1) {
        color = 1;
    }
    mColor = color;
    char name[40];
    sprintf(name, "N_clrBtn_%02d", color - 1);
    mColorLyt.setPosFrom(name, "clrBtnFc_01", 0.0f, 0.0f);
    mColorLyt.setPosFrom(name, "clrBtn_Psh_00", 0.0f, 0.0f);
    nw4r::lyt::Material *material =
        m2d::getMaterial(mColorLyt.findPane("clr_00"));
    const u16 *palette = dDesign_c::getPaletteData(l_palette);
    GXColorS10 col;
    m2d::rgb565ToColor(palette[color], &col);
    material->SetTevColor(1, col);
    col.a = 0;
    material->SetTevColor(0, col);
    onFlag(FLAG_CALC_COLOR);
}

// .text:0x5B60
void dMenuEdit_c::setPartsBtnAnm(int back) {
    static const char *l_anm[2] = {"pro_design_win_00_chngBtnPsh.brlan",
                                   "pro_design_win_00_bckBtnPsh.brlan"};
    mPartsLyt.setAnm(&mPartsBtnAnm, l_anm[back], NULL);
}

// .text:0x5B80
void dMenuEdit_c::initPartsWin() {
    static const char *l_chngBtnAnm[2] = {
        "pro_design_win_00_chngBtn_in.brlan",
        "pro_design_win_00_chngBtn_out.brlan"};
    setPartsBtnAnm(1);
    mPartsBtnAnm.setToEnd();
    mChngBtnAnm.init(&mPartsLyt, &mRes, l_chngBtnAnm, 2);
    mChngBtnAnm.start(1);
    mChngBtnAnm.setToEnd();
    mPartsLyt.setAnm(&mPartBtnAnm, "pro_design_win_00_partsBtnPsh.brlan", NULL);
    mPartBtnAnm.setToEnd();
    mPartsLyt.setAnm(&mClothPtrnAnm, "pro_design_win_00_clothPtrn.brlan", NULL);
}

// .text:0x5C24
void dMenuEdit_c::initPartsTexture() {
    for (int i = PART_FRONT; i < PART_NUM; i++) {
        mTexNo = getTexIndex(i);
        void *tex = getTexture();
        m2d::getTexMaps(getPartPane(BTN_PART_FIRST + i))->SetImage(tex);
    }
}

// .text:0x5C9C
void dMenuEdit_c::setPartsPalette() {
    const u16 *palette = dDesign_c::getPaletteData(l_palette);
    for (int i = 0; i < PART_NUM; i++) {
        m2d::getTexMaps(getPartPane(BTN_PART_FIRST + i))
            ->SetPalette((void *)palette);
    }
}

// .text:0x5D08
void dMenuEdit_c::selectPart(int part) {
    static const f32 l_clothFrame[PART_NUM] = {0.0f, 2.0f, 3.0f, 1.0f};
    mPart = part;
    mTexNo = getTexIndex(part);
    if (isFlag(FLAG_PRO)) {
        char name[40];
        sprintf(name, "N_parts_%02d", mPart);
        mPartsLyt.setPosFrom(name, "partsBtnFc_01", 0.0f, 0.0f);
        mPartsLyt.setPosFrom(name, "partsBtn_Psh_00", 0.0f, 0.0f);
        mPartBtnAnm.setFrame(0.0f);
        onFlag(FLAG_CALC_PARTS);
        mClothPtrnAnm.setFrame(l_clothFrame[mPart]);
    }
}

// .text:0x5DE4
int dMenuEdit_c::getTexIndex(int part) {
    static const int l_texIndex[PART_NUM] = {
        ORG_DESIGN_TEX_FRONT, ORG_DESIGN_TEX_SLEEVE_0, ORG_DESIGN_TEX_SLEEVE_1,
        ORG_DESIGN_TEX_BACK};
    return l_texIndex[part];
}

// .text:0x5DF8
mVec2_c dMenuEdit_c::getPartPos(int btn, f32 x, f32 y) {
    char name[40];
    sprintf(name, "N_myDsgn_%02d", btn - BTN_PART_FIRST);
    mVec3_c pos;
    m2d::getScreenPos(&pos, mPartsLyt.findPane(name));
    if (SCGetAspectRatio() == 1) {
        x *= 0.730769f;
    }
    mVec2_c result;
    result.x = pos.x + x;
    result.y = pos.y + y;
    return result;
}

// .text:0x5EB8
void dMenuEdit_c::showPart() {
    getPartPane(mHeldPart)->SetVisible(true);
}

// .text:0x5EEC
void dMenuEdit_c::pickUpPart() {
    onFlag(FLAG_FOLLOW);
    dObjCursor_c::hold();
    nw4r::lyt::TexMap *dst = m2d::getTexMaps(mPickupLyt.findPane("myDsgn"));
    nw4r::lyt::Pane *part = getPartPane(mHeldPart);
    part->SetVisible(false);
    nw4r::lyt::TexMap *src = m2d::getTexMaps(part);
    dst->SetImage(src->GetImage());
    dst->SetPalette(src->GetPalette());
    setSubState(SUB_PARTS_DRAG);
    mMove.startFollow();
    playSeItemLift();
}

// .text:0x5F90
nw4r::lyt::Pane *dMenuEdit_c::getPartPane(int btn) {
    static char partPaneFormat[13] = "myDsgn_%02d";
    char name[40];
    sprintf(name, partPaneFormat, btn - BTN_PART_FIRST);
    return mPartsLyt.findPane(name);
}

// .text:0x5FE0
void dMenuEdit_c::setupSwapMenu() {
    mSelect.reset();
    mSelect.mNum = 0;
    mSelect.addItem(MSG_SWAP, ANSWER_SWAP, 0, 0);
    mSelect.addItem(MSG_COPY, ANSWER_COPY, 0, 0);
    mSelect.addItem(MSG_NEVER_MIND, ANSWER_CANCEL, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x6074
void dMenuEdit_c::openSwapMenu(int btn) {
    mVec2_c pos = getPartPos(btn, 10.0f, -50.0f);
    mSwapPart = btn;
    mSelect.placeAbove(pos);
    mSelect.openPointer();
}

// .text:0x6230
void dMenuEdit_c::pushBtn(int btn) {
    switch (btn) {
    case BTN_GRID:
        setGrid(!isFlag(FLAG_GRID));
        mBtnAnm.setFrame(0.0f);
        onFlag(FLAG_BTN_ANM);
        break;
    case BTN_ZOOM_IN:
        if (mZoom < ZOOM_4X) {
            setZoom(mZoom + 1);
            mBtnAnm.setFrame(0.0f);
            onFlag(FLAG_BTN_ANM);
            playSeDecide();
        }
        break;
    case BTN_ZOOM_OUT:
        if (mZoom > ZOOM_1X) {
            setZoom(mZoom - 1);
            mBtnAnm.setFrame(0.0f);
            onFlag(FLAG_BTN_ANM);
            playSeDecide();
        }
        break;
    case BTN_UNDO:
        undo();
        mBtnAnm.setFrame(0.0f);
        onFlag(FLAG_BTN_ANM);
        break;
    case BTN_TOOL:
        setToolBtnAnm(0);
        setSubState(SUB_TOOL_ANM);
        setNextSubState(SUB_TOOL);
        setHover(BTN_NONE);
        mWinMode = WIN_TOOL;
        mBtn.setDisable(0, TRUE);
        mBtn.setDisable(2, TRUE);
        break;
    case BTN_COLOR:
        setColorBtnAnm(0);
        setSubState(SUB_COLOR_ANM);
        setNextSubState(SUB_COLOR);
        setHover(BTN_NONE);
        mWinMode = WIN_COLOR;
        mBtn.setDisable(0, TRUE);
        mBtn.setDisable(2, TRUE);
        break;
    case BTN_CANVAS:
        startFilter();
        clampPixelToView();
        if (beginTool()) {
            setSubState(SUB_PAINT);
        }
        break;
    case BTN_THUMB:
        if (mZoom != ZOOM_1X) {
            playSeFrameGet();
            dObjCursor_c::hold();
            scrollToPointer();
            setSubState(SUB_SCROLL);
        }
        break;
    case BTN_QUIT:
    case BTN_BTN_HIDDEN:
    case BTN_DONE:
        if (mBtn.push(btn - BTN_QUIT, FALSE)) {
            mPushedBtn = btn;
            setSubState(SUB_BUTTON);
        }
        break;
    case BTN_PREVIEW:
        nextPreviewKind();
        break;
    case BTN_PARTS:
        setPartsBtnAnm(0);
        setSubState(SUB_PARTS_ANM);
        setNextSubState(SUB_PARTS);
        setHover(BTN_NONE);
        mWinMode = WIN_PARTS;
        mBtn.setDisable(0, TRUE);
        mBtn.setDisable(2, TRUE);
        {
            int texNo = mTexNo;
            if (dOrgObj_c::sInstance != NULL) {
                dOrgObj_c::sInstance->setClothAnm(texNo);
            }
        }
        backupUndo();
        break;
    }
    switch (btn) {
    case BTN_GRID:
        playSeModeChange();
        break;
    case BTN_ZOOM_IN:
    case BTN_ZOOM_OUT:
    case BTN_UNDO:
    case BTN_CANVAS:
    case BTN_THUMB:
    case BTN_PREVIEW:
    case BTN_QUIT:
    case BTN_BTN_HIDDEN:
    case BTN_DONE:
        break;
    default:
        playSeDecide();
        break;
    }
}

// .text:0x6584
void dMenuEdit_c::initExtra() {
}

// .text:0x6588
void dMenuEdit_c::updatePenFrame() {
    static const char *l_penFrame[7] = {"penFrm_1x", "penFrm_2x", "penFrm_3x",
                                        "penFrm_4x", "penFrm_6x", "penFrm_8x",
                                        "penFrm_12x"};
    static const int l_penFrameIdx[9] = {0, 1, 3, 1, 3, 5, 2, 4, 6};
    int base;
    switch (mTool) {
    case BTN_TOOL_FIRST + TOOL_PEN_S:
    case BTN_TOOL_FIRST + TOOL_LINE:
    case BTN_TOOL_FIRST + TOOL_FILL:
        base = 0;
        break;
    case BTN_TOOL_FIRST + TOOL_PEN_M:
        base = 3;
        break;
    case BTN_TOOL_FIRST + TOOL_PEN_L:
        base = 6;
        break;
    default:
        return;
    }
    for (int i = 0; i < 7; i++) {
        mPenFrame.setVisible(l_penFrame[i], false);
    }
    base += mZoom;
    mPenFrame.setVisible(l_penFrame[l_penFrameIdx[base]], true);
}

// .text:0x6674
void dMenuEdit_c::clampPixelToView() {
    static const int l_margin[ZOOM_NUM] = {16, 8, 4};
    int margin = l_margin[mZoom];
    int scrollX = mScrollX;
    int scrollY = mScrollY;
    int minX = scrollX + 16 - margin;
    int maxX = scrollX + margin + 15;
    int minY = scrollY + 16 - margin;
    int maxY = scrollY + margin + 15;
    switch (mTool) {
    case BTN_TOOL_FIRST + TOOL_PEN_S:
        break;
    case BTN_TOOL_FIRST + TOOL_PEN_M:
        minX += 1;
        maxY -= 1;
        break;
    case BTN_TOOL_FIRST + TOOL_PEN_L:
        minX += 1;
        minY += 1;
        maxX -= 1;
        maxY -= 1;
        break;
    case BTN_TOOL_FIRST + TOOL_LINE:
    case BTN_TOOL_FIRST + TOOL_FILL:
        break;
    }
    if (minX > mPixelX) {
        mPixelX = minX;
    } else if (maxX < mPixelX) {
        mPixelX = maxX;
    }
    if (minY > mPixelY) {
        mPixelY = minY;
    } else if (maxY < mPixelY) {
        mPixelY = maxY;
    }
}

// .text:0x673C
void dMenuEdit_c::placePenFrame() {
    clampPixelToView();
    int x = mPixelX;
    int y = mPixelY;
    switch (mTool) {
    case BTN_TOOL_FIRST + TOOL_PEN_L:
        x += 1;
        y -= 1;
        break;
    case BTN_TOOL_FIRST + TOOL_PEN_S:
    case BTN_TOOL_FIRST + TOOL_PEN_M:
    case BTN_TOOL_FIRST + TOOL_LINE:
    case BTN_TOOL_FIRST + TOOL_FILL:
        break;
    default:
        return;
    }
    mVec2_c pos = getPixelPos(x, y, 2, 0);
    mPenFrame.mPos.x = pos.x;
    mPenFrame.mPos.y = pos.y;
    mPenFrame.calc();
}

// .text:0x67EC
void dMenuEdit_c::placeArrowStart() {
    mVec2_c pos = getPixelPos(mPixelX, mPixelY, 1, 1);
    mArrowStart.mPos.x = pos.x;
    mArrowStart.mPos.y = pos.y;
    mArrowStart.calc();
}

// .text:0x6854
void dMenuEdit_c::placeArrowEnd() {
    mVec2_c pos = getPixelPos(mPixelX, mPixelY, 1, 1);
    mArrowEnd.mPos.x = pos.x;
    mArrowEnd.mPos.y = pos.y;
    mArrowEnd.calc();
}

// .text:0x68BC
void dMenuEdit_c::placeRulerStart() {
    mVec2_c pos = getPixelPos(mPixelX, mPixelY, 0, 0);
    mVec2_c scale(1.0f, 1.0f);
    mRulerStart.getRootPane()->SetScale(scale);
    mRulerStart.mPos.x = pos.x;
    mRulerStart.mPos.y = pos.y;
    mRulerStart.calc();
}

// .text:0x694C
void dMenuEdit_c::placeRuler() {
    mVec2_c scale;
    scale.y = 1.0f;
    int topX, topY, bottomX, bottomY;
    if (mPixelY >= mStartY) {
        topX = mStartX;
        topY = mStartY;
        bottomX = mPixelX;
        bottomY = mPixelY;
    } else {
        bottomX = mStartX;
        bottomY = mStartY;
        topX = mPixelX;
        topY = mPixelY;
    }
    mVec2_c start;
    mVec2_c end;
    if (bottomX < topX) {
        scale.x = -1.0f;
        start = getPixelPos(topX, topY, 2, 0);
        end = getPixelPos(bottomX, bottomY, 0, 2);
    } else {
        scale.x = 1.0f;
        start = getPixelPos(topX, topY, 0, 0);
        end = getPixelPos(bottomX, bottomY, 2, 2);
    }
    mRulerStart.getRootPane()->SetScale(scale);
    mRulerStart.mPos.x = start.x;
    mRulerStart.mPos.y = start.y;
    mRulerStart.calc();
    mRulerEnd.getRootPane()->SetScale(scale);
    mRulerEnd.mPos.x = end.x;
    mRulerEnd.mPos.y = end.y;
    mRulerEnd.calc();
}

// .text:0x6AFC
void dMenuEdit_c::playSeUndo() {
    playSe(SE_EDIT_UNDO);
}

// .text:0x6B04
void dMenuEdit_c::playSeRedo() {
    playSe(SE_EDIT_REDO);
}

// .text:0x6B0C
void dMenuEdit_c::playSePaintDecide() {
    playSe(SE_EDIT_PAINT_DECIDE);
}

// .text:0x6B14
void dMenuEdit_c::playSePen() {
    playSe(SE_EDIT_PEN);
}

// .text:0x6B1C
void dMenuEdit_c::playSePen2() {
    playSe(SE_EDIT_PEN2);
}

// .text:0x6B24
void dMenuEdit_c::playSePen3() {
    playSe(SE_EDIT_PEN3);
}

// .text:0x6B2C
void dMenuEdit_c::playSeSetStart() {
    playSe(SE_EDIT_SET_START_POINT);
}

// .text:0x6B34
void dMenuEdit_c::playSeSetMove() {
    playSe(SE_EDIT_SET_MOVE);
}

// .text:0x6B3C
void dMenuEdit_c::playSeSetEnd() {
    playSe(SE_EDIT_SET_END_POINT);
}

// .text:0x6B44
void dMenuEdit_c::playSeModeChange() {
    playSe(SE_EDIT_MODE_CHANGE);
}

// .text:0x6B4C
void dMenuEdit_c::playSeSpoit() {
    playSe(SE_EDIT_SPOIT);
}

// .text:0x6B54
void dMenuEdit_c::playSeFrameSlide() {
    playSe(SE_EDIT_frame_slide);
}

// .text:0x6B5C
void dMenuEdit_c::playSeFrameGet() {
    playSe(SE_EDIT_frame_get);
}

// .text:0x6B64
void dMenuEdit_c::playSeFramePlace() {
    playSe(SE_EDIT_frame_place);
}
