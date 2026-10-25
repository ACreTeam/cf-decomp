#include <game/game/d_menu_photo.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_photo.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_screenshot.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <string.h>

namespace dMenuPhoto {
// .text:0x5C
BASE_PROFILE(MENU_PHOTO, dMenuPhoto_c)
} // namespace dMenuPhoto

// .text:0x110
int dMenuPhoto_c::create() {
    dMenuMng_c::onFlag(dMenuMng_c::FLAG_PHOTO);
    BOOL loaded = mRes.load("Layout/photo.arc");
    loaded &= mSlct.load();
    loaded &= mComp.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "photo_win_00.brlyt");
    mLyt.setVisible("N_cmnBtnA_01", false);
    mLyt.setVisible("N_cmnBtnA_02", false);
    mLyt.setAnm(&mScrlIconAnm, "photo_win_00_scrlIcon_00.brlan", nullptr);
    setTexts();
    mSlct.create();
    mComp.build();
    setSubState(SUB_MAIN);
    bool noPhoto = !initPhoto();
    mLyt.setVisible("photo_00", !noPhoto);
    mLyt.setVisible("no_photo", noPhoto);
    mLyt.setVisible("T_no_photo", noPhoto);
    mLyt.setAlpha("photo_00", 255);
    mLyt.setAlpha("no_photo", 255);
    mLyt.setAlpha("T_no_photo", 255);
    initAnms();
    startAnm(&mLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    mLyt.calc();
    onFlag(FLAG_DRAW);
    return SUCCEEDED;
}

// .text:0x2BC
int dMenuPhoto_c::doDelete() {
    if (!mComp.remove()) {
        return NOT_READY;
    }
    if (!mRes.remove()) {
        return NOT_READY;
    }
    if (!mSlct.remove()) {
        return NOT_READY;
    }
    if (!dScreenshot::close()) {
        return NOT_READY;
    }
    dScreenshot::clearBusy();
    dMenuMng_c::offFlag(dMenuMng_c::FLAG_PHOTO);
    return SUCCEEDED;
}

// .text:0x350
int dMenuPhoto_c::stateMain() {
    if (executeCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    updateBtnAnm();
    return SUCCEEDED;
}

// .text:0x3A0
int dMenuPhoto_c::execute() {
    int result = dMenuBase_c::execute();
    mSlct.execute();
    mComp.update();
    mScrlIconAnm.play();
    mLyt.calc();
    if (mTimer > 0) {
        mTimer--;
    }
    return result;
}

// .text:0x41C
int dMenuPhoto_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    mComp.draw();
    return SUCCEEDED;
}

// .text:0x484
void dMenuPhoto_c::subStateMain() {
    mBtnAnm.setNext(BTN_ANM_OFF);
    int tab = dAcUitag::getTabInput(dAcUitag_c::TAB_PHOTO, FALSE);
    if (tab == TAB_NONE || !selectUitagTab(tab, TRUE)) {
        mVec2_c pos = dObjCursor_c::getPos();
        int uitagTab = hitUitagTab(&pos);
        if (uitagTab != TAB_NONE) {
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                selectUitagTab(uitagTab, FALSE);
            }
        } else {
            mVec2_c lytPos = pos;
            if (m2d::hitButton(&lytPos, &mLyt, 0, 0, -1, -1, 1.0f) !=
                BTN_NONE) {
                if (mBtnAnm.setNext(BTN_ANM_ON)) {
                    playSeCursor();
                }
                if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                    mBtnAnm.setNext(BTN_ANM_PUSH);
                    setSubState(SUB_PUSH);
                }
            }
        }
    }
}

// .text:0x5E0
void dMenuPhoto_c::subStatePush() {
    if (!isFlag(FLAG_BTN_ANM)) {
        setAnswers();
        openAnswers();
    }
}

// .text:0x628
void dMenuPhoto_c::subStateMount() {
    int result = dScreenshot::getResult();
    switch (result) {
    case dScreenshot::RESULT_BUSY:
        return;
    case dScreenshot::RESULT_OK:
        dScreenshot::close();
        if (mMode == ANSWER_SAVE) {
            openSaveConfirm();
        } else {
            openEraseConfirm();
        }
        break;
    case dScreenshot::RESULT_BAD_CARD:
        showMessage(SD_MSG_NO_CARD, TRUE);
        break;
    case dScreenshot::RESULT_DEVICE_ERROR:
    case dScreenshot::RESULT_NO_CARD:
        showMessage(SD_MSG_UNKNOWN_DEVICE, TRUE);
        break;
    case dScreenshot::RESULT_ERROR:
    default:
        showMessage(SD_MSG_NO_CARD, TRUE);
        break;
    }
}

// .text:0x6E8
void dMenuPhoto_c::subStateDialog() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0x748
void dMenuPhoto_c::subStateDialogPush() {
    if (mSlct.isPushEnd()) {
        setSubState(SUB_DIALOG_END);
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0x7AC
void dMenuPhoto_c::subStateDialogEnd() {
    if (mSlct.isWinAnmEnd()) {
        if (mSlct.mPushedBtn == DIALOG_BTN_YES) {
            if (dReset::Manage_c::GetInstance()->isResetMode()) {
                setSubState(SUB_MAIN);
                offFlag(FLAG_DIALOG);
            } else {
                dScreenshot::setBusy();
                mTimer = MESSAGE_FRAMES;
                if (mMode == ANSWER_SAVE) {
                    startSave();
                } else {
                    startErase();
                }
            }
        } else {
            setSubState(SUB_MAIN);
            offFlag(FLAG_DIALOG);
        }
    }
}

// .text:0x894
void dMenuPhoto_c::subStateSaveEncode() {
    if (mSlct.isWinAnmEnd() && dPhoto::getMode() == dPhoto::MODE_NONE) {
        mStep = STEP_NONE;
        stepSave();
        setSubState(SUB_SAVE);
    }
}

// .text:0x900
void dMenuPhoto_c::subStateSave() {
    int result = dScreenshot::getResult();
    mErrMsg = SD_MSG_NONE;
    switch (result) {
    case dScreenshot::RESULT_BUSY:
        return;
    case dScreenshot::RESULT_OK:
        break;
    default:
        mErrMsg = getErrorMessage(result);
        break;
    }
    if (mErrMsg != SD_MSG_NONE) {
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
        setSubState(SUB_ERROR);
        dScreenshot::clearBusy();
    } else if (stepSave()) {
        dScreenshot::close();
        setSubState(SUB_SAVE_WAIT);
    }
}

// .text:0x9CC
void dMenuPhoto_c::subStateSaveWait() {
    if (mTimer <= 0) {
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
        setSubState(SUB_SAVE_DONE);
        dScreenshot::clearBusy();
    }
}

// .text:0xA30
void dMenuPhoto_c::subStateSaveDone() {
    if (mSlct.isWinAnmEnd()) {
        showMessage(SD_MSG_SAVED, FALSE);
    }
}

// .text:0xA78
void dMenuPhoto_c::subStateError() {
    if (mSlct.isWinAnmEnd()) {
        showMessage(mErrMsg, TRUE);
    }
}

// .text:0xAC0
void dMenuPhoto_c::subStateAnswer() {
    if (mComp.isClosed()) {
        mMode = mComp.mCode;
        switch (mMode) {
        default:
            onQuitAnswer();
            break;
        case ANSWER_SAVE:
            onSaveAnswer();
            break;
        case ANSWER_ERASE:
            onEraseAnswer();
            break;
        }
    }
}

// .text:0xB40
void dMenuPhoto_c::subStateErase() {
    if (mSlct.isWinAnmEnd()) {
        int result = dScreenshot::getResult();
        mErrMsg = SD_MSG_NONE;
        switch (result) {
        case dScreenshot::RESULT_BUSY:
            return;
        case dScreenshot::RESULT_OK:
            break;
        default:
            mErrMsg = getErrorMessage(result);
            if (result == dScreenshot::RESULT_REMOVED) {
                mErrMsg = SD_MSG_ERASE_ERROR;
            }
            break;
        }
        if (mErrMsg != SD_MSG_NONE) {
            mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
            setSubState(SUB_ERROR);
            dScreenshot::clearBusy();
        } else if (stepErase()) {
            dScreenshot::close();
            setSubState(SUB_ERASE_WAIT);
        }
    }
}

// .text:0xC38
void dMenuPhoto_c::subStateEraseWait() {
    if (mTimer <= 0) {
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
        setSubState(SUB_ERASE_DONE);
        dScreenshot::clearBusy();
    }
}

// .text:0xC9C
void dMenuPhoto_c::subStateEraseDone() {
    if (mSlct.isWinAnmEnd()) {
        showMessage(SD_MSG_ERASED, FALSE);
    }
}

// .text:0xCE4
BOOL dMenuPhoto_c::subStateCloseRequest() {
    dAcUitag::closeTab();
    startCloseAnm();
    return TRUE;
}

// .text:0xD1C
BOOL dMenuPhoto_c::subStateCloseDialog() {
    if (!mSlct.isWinAnmEnd()) {
        return FALSE;
    }
    mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    return subStateCloseRequest();
}

// .text:0xD78
BOOL dMenuPhoto_c::subStateCloseAnswer() {
    if (!mComp.isReady()) {
        return FALSE;
    }
    mComp.close();
    return subStateCloseRequest();
}

// .text:0xDC8
BOOL dMenuPhoto_c::executeCloseRequest() {
    typedef BOOL (dMenuPhoto_c::*Proc)();
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    static Proc closeProcs[SUB_NUM] = {&dMenuPhoto_c::subStateCloseRequest,
                                       nullptr,
                                       nullptr,
                                       &dMenuPhoto_c::subStateCloseDialog,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       &dMenuPhoto_c::subStateCloseAnswer};
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL handled = (this->*closeProcs[mSubState])();
    if (handled) {
        dMenuMng_c::clearCloseRequest();
    }
    return handled;
}

// .text:0xF2C
void dMenuPhoto_c::executeSubState() {
    typedef void (dMenuPhoto_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuPhoto_c::subStateMain,       &dMenuPhoto_c::subStatePush,
        &dMenuPhoto_c::subStateMount,      &dMenuPhoto_c::subStateDialog,
        &dMenuPhoto_c::subStateDialogPush, &dMenuPhoto_c::subStateDialogEnd,
        &dMenuPhoto_c::subStateSaveEncode, &dMenuPhoto_c::subStateSave,
        &dMenuPhoto_c::subStateSaveWait,   &dMenuPhoto_c::subStateSaveDone,
        &dMenuPhoto_c::subStateError,      &dMenuPhoto_c::subStateErase,
        &dMenuPhoto_c::subStateEraseWait,  &dMenuPhoto_c::subStateEraseDone,
        &dMenuPhoto_c::subStateAnswer};
    (this->*subStateProcs[mSubState])();
}

// .text:0xF64
void dMenuPhoto_c::updateBtnAnm() {
    if (mBtnAnm.play()) {
        onFlag(FLAG_BTN_ANM);
        onFlag(FLAG_BTN_ANM_PLAYED);
    } else {
        offFlag(FLAG_BTN_ANM);
    }
}

// .text:0xFC4
int dMenuPhoto_c::hitUitagTab(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = TAB_NONE;
    }
    dAcUitag::setTabFocus(tab);
    if (tab == TAB_NONE) {
        return TAB_NONE;
    }
    return tab;
}

// .text:0x1028
BOOL dMenuPhoto_c::selectUitagTab(int tab, BOOL delay) {
    if (dAcUitag::selectTab(tab, delay)) {
        startCloseAnm();
        return TRUE;
    }
    return FALSE;
}

// .text:0x1078
void dMenuPhoto_c::startCloseAnm() {
    startAnm(&mLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, FALSE);
}

// .text:0x1094
void dMenuPhoto_c::setTexts() {
    mLyt.setMessage("T_cmnBtnA_00", MSG_SD_CARD, nullptr);
    mLyt.setMessage("T_no_photo", MSG_NO_PHOTOS, nullptr);
}

// .text:0x10EC
void dMenuPhoto_c::initAnms() {
    static const char *btnAnmNames[BTN_ANM_NUM] = {
        "photo_win_00_cmnBtnA_out.brlan", "photo_win_00_cmnBtnA_in.brlan",
        "photo_win_00_cmnBtnA_psh.brlan"};
    mBtnAnm.init(&mLyt, &mRes, btnAnmNames, BTN_ANM_NUM);
    mBtnAnm.start(BTN_ANM_OFF);
    mBtnAnm.setToEnd();
}

// .text:0x1144
BOOL dMenuPhoto_c::initPhoto() {
    mLyt.setAnm(&mNoPhotoAnm, "photo_win_00_NoPhoto.brlan", nullptr);
    mLyt.calc();
    mLyt.unbind(&mNoPhotoAnm);
    mNoPhotoAnm.remove();
    if (dPhoto::getMode() != dPhoto::MODE_NONE || !dPhoto::isCaptured()) {
        onFlag(FLAG_NO_PHOTO);
        return FALSE;
    }
    nw4r::lyt::TexMap *tex = m2d::getTexMaps(mLyt.findPane("photo_00"));
    void *image = tex->GetImage();
    void *buffer = dPhoto::getTexBuffer();
    int size = dPhoto::getTexWidth() * dPhoto::getTexHeight() * 2;
    memcpy(image, buffer, size);
    DCFlushRange(image, size);
    return TRUE;
}

// .text:0x123C
void dMenuPhoto_c::showMessage(u16 msg, BOOL error) {
    mSlct.setBtnVisible(DIALOG_BTN_NO, false);
    mSlct.setBtnVisible(DIALOG_BTN_YES, false);
    mSlct.setBtn(DIALOG_BTN_OK, MSG_OK, nullptr, 0);
    mSlct.setMsg(msg, "sys_2D/SYS_SDcard");
    mSlct.fitLines(dMenuSlctB_c::POS_UPPER);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    if (error) {
        playSeCanNot();
        mSlct.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
        mSlct.setColor(dMenuSlctB_c::COLOR_RED);
    } else {
        mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
        mSlct.setColor(dMenuSlctB_c::COLOR_ORANGE);
    }
}

// .text:0x1348
void dMenuPhoto_c::openSaveConfirm() {
    mSlct.setBtn(DIALOG_BTN_NO, SEL_MSG_NEVER_MIND, "sys_SELECT/SYS_Select",
                 SE_cancel_little);
    mSlct.setBtn(DIALOG_BTN_YES, SEL_MSG_SAVE, "sys_SELECT/SYS_Select", 0);
    mSlct.setBtnVisible(DIALOG_BTN_OK, false);
    mSlct.setMsg(SD_MSG_SAVE_ASK, "sys_2D/SYS_SDcard");
    mSlct.fitLines(dMenuSlctB_c::POS_UPPER);
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mSlct.setColor(dMenuSlctB_c::COLOR_ORANGE);
}

// .text:0x1420
void dMenuPhoto_c::openEraseConfirm() {
    mSlct.setBtn(DIALOG_BTN_NO, SEL_MSG_CANCEL, "sys_SELECT/SYS_Select",
                 SE_cancel_little);
    mSlct.setBtn(DIALOG_BTN_YES, SEL_MSG_ERASE, "sys_SELECT/SYS_Select", 0);
    mSlct.setBtnVisible(DIALOG_BTN_OK, false);
    mSlct.setMsg(SD_MSG_ERASE_ASK, "sys_2D/SYS_SDcard");
    mSlct.fitLines(dMenuSlctB_c::POS_UPPER);
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mSlct.setColor(dMenuSlctB_c::COLOR_ORANGE);
}

// .text:0x14F8
void dMenuPhoto_c::startSave() {
    mSlct.setBtnVisible(DIALOG_BTN_NO, false);
    mSlct.setBtnVisible(DIALOG_BTN_YES, false);
    mSlct.setBtnVisible(DIALOG_BTN_OK, false);
    mSlct.setMsg(SD_MSG_SAVING, "sys_2D/SYS_SDcard");
    mSlct.fitLines(dMenuSlctB_c::POS_CENTER);
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, TRUE);
    onFlag(FLAG_DIALOG);
    mSlct.setColor(dMenuSlctB_c::COLOR_ORANGE);
    setSubState(SUB_SAVE_ENCODE);
    dPhoto::requestEncode(false);
}

// .text:0x15BC
void dMenuPhoto_c::startErase() {
    mSlct.setBtnVisible(DIALOG_BTN_NO, false);
    mSlct.setBtnVisible(DIALOG_BTN_YES, false);
    mSlct.setBtnVisible(DIALOG_BTN_OK, false);
    mSlct.setMsg(SD_MSG_ERASING, "sys_2D/SYS_SDcard");
    mSlct.fitLines(dMenuSlctB_c::POS_CENTER);
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, TRUE);
    onFlag(FLAG_DIALOG);
    mSlct.setColor(dMenuSlctB_c::COLOR_ORANGE);
    mStep = STEP_NONE;
    stepErase();
    setSubState(SUB_ERASE);
}

// .text:0x1688
u16 dMenuPhoto_c::getErrorMessage(int result) const {
    switch (result) {
    case dScreenshot::RESULT_BAD_CARD:
        return SD_MSG_NO_CARD;
    case dScreenshot::RESULT_REMOVED:
        return SD_MSG_UNABLE_TO_SAVE;
    case dScreenshot::RESULT_DEVICE_ERROR:
    case dScreenshot::RESULT_NO_CARD:
        return SD_MSG_UNKNOWN_DEVICE;
    case dScreenshot::RESULT_DEVICE_ERROR_2:
        return SD_MSG_UNKNOWN_DEVICE;
    case dScreenshot::RESULT_WRITE_PROTECTED:
        return SD_MSG_WRITE_PROTECTED;
    case dScreenshot::RESULT_NO_SPACE:
        return SD_MSG_NO_SPACE;
    case dScreenshot::RESULT_FULL:
        return SD_MSG_TOO_MANY_FOLDERS;
    case dScreenshot::RESULT_TOO_MANY_FILES:
        return SD_MSG_TOO_MANY_FILES;
    case dScreenshot::RESULT_NO_PHOTO:
        return SD_MSG_NO_PHOTO_DATA;
    case dScreenshot::RESULT_ERROR:
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUP01_01 || VERSION == VERSION_RUUJ01_02 || \
    VERSION == VERSION_RUUK01_01
        return SD_MSG_UNABLE_TO_SAVE;
#else
        return SD_MSG_NO_CARD;
#endif
    default:
        return SD_MSG_NO_CARD;
    }
}

// .text:0x1700
BOOL dMenuPhoto_c::stepSave() {
    if (++mStep >= SAVE_STEP_NUM) {
        return TRUE;
    }
    switch (mStep) {
    case SAVE_STEP_MOUNT:
        dScreenshot::startMountLock();
        break;
    case SAVE_STEP_MAKE_ROOT:
        dScreenshot::startMakeRoot();
        break;
    case SAVE_STEP_MAKE_PATH:
        dScreenshot::startMakePath();
        break;
    case SAVE_STEP_WRITE:
        dScreenshot::startWrite();
        break;
    }
    return FALSE;
}

// .text:0x177C
BOOL dMenuPhoto_c::stepErase() {
    if (++mStep >= ERASE_STEP_NUM) {
        return TRUE;
    }
    switch (mStep) {
    case ERASE_STEP_MOUNT:
        dScreenshot::startMountLock();
        break;
    case ERASE_STEP_CHECK:
        dScreenshot::startCheck();
        break;
    case ERASE_STEP_DELETE:
        dScreenshot::startDelete();
        break;
    }
    return FALSE;
}

// .text:0x17EC
void dMenuPhoto_c::openAnswers() {
    mVec2_c pos(0.0f, -100.0f);
    mComp.placeAbove(pos);
    mComp.openPointer();
    setSubState(SUB_ANSWER);
}

// .text:0x19A8
void dMenuPhoto_c::onSaveAnswer() {
    if (isFlag(FLAG_NO_PHOTO)) {
        showMessage(SD_MSG_NO_PHOTO_TAKEN, TRUE);
    } else {
        dScreenshot::startMount();
        setSubState(SUB_MOUNT);
    }
}

// .text:0x1A10
void dMenuPhoto_c::onEraseAnswer() {
    dScreenshot::startMount();
    setSubState(SUB_MOUNT);
}

// .text:0x1A54
void dMenuPhoto_c::onQuitAnswer() {
    setSubState(SUB_MAIN);
}

// .text:0x1A68
void dMenuPhoto_c::setAnswers() {
    mComp.reset();
    mComp.mNum = 0;
    mComp.addItem(MSG_SAVE_PHOTO, ANSWER_SAVE, 0, 0);
    mComp.addItem(MSG_ERASE_ALL, ANSWER_ERASE, 0, 0);
    mComp.addItem(MSG_QUIT, ANSWER_QUIT, 0, 0);
    mComp.setCancelLast();
}
