#include <game/framework/f_base.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_menu_han.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_friend.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_prc_mng.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_sv_star_sign.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_pad.hpp>
#include <string.h>

#ifdef MUST_MATCH
// Matching-only emission order, following d_a_npc_sp_fuko's existing helper.
// There is no evidence for this helper in the original source. MWCC drops
// the unused body; its qualified calls order the retained weak functions.
// See notes/TUs/d_menu_han.txt.
static void orderHanWeakInlines(dItem::name_c *name, dMenuHan_c *menu,
                                mColor *color) {
    name->dItem::name_c::~name_c();
    menu->dMenuHan_c::createHeap();
    color->mColor::~mColor();
    menu->dMenuHan_c::~dMenuHan_c();
}
#endif

namespace dMenuHan {
// .text:0xCC
BASE_PROFILE(MENU_HAN, dMenuHan_c)
} // namespace dMenuHan

// The color of the text being composed.
static mColor l_inputColor(0xFF, 0x00, 0x00, 0xFF);

// .text:0x170
int dMenuHan_c::create() {
    mKbd.init(dKbd::Mng_c::INIT_MODE_PAD);
    BOOL loaded = mKbd.loadRes();
    loaded &= mBtn.load();
    loaded &= mCur.load();
    loaded &= mSlct.load();
    if (!loaded) {
        return NOT_READY;
    }
    if (!mKbd.create()) {
        return NOT_READY;
    }
    mKbd.setup(dKbd::Mng_c::SETUP_MODE_0);
    mKbd.setMaxLength(TEXT_SIZE);
    mKbd.hideToolBar();
    mKbd.setMemoMode1();
    mLyt.build(mKbd.getResAcc(), "cmn_question_00.brlyt");
    mLyt.setAnm(&mIconAnm, "cmn_question_00_iconPattern.brlan", nullptr);
    mSlct.create();
    setIcon();
#if VERSION == VERSION_RUUJ01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (mMenuNo == dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME || mMenuNo == dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME_2) {
        mLyt.setVisible("T_starName", true);
        mLyt.setMessage("T_starName", MSG_STAR_SIGN_LABEL, nullptr);
    } else {
        mLyt.setVisible("T_starName", false);
    }
#else
    mLyt.setVisible("T_starName", false);
#endif
    if (mMenuNo == dMenuMng_c::MENU_INPUT_TOWN_NAME ||
        mMenuNo == dMenuMng_c::MENU_INPUT_NEW_FRIEND_TOWN ||
        mMenuNo == dMenuMng_c::MENU_INPUT_FRIEND_TOWN) {
        mLyt.setVisible("T_townName", true);
        mLyt.setMessage("T_townName", MSG_TOWN_LABEL, nullptr);
    } else {
        mLyt.setVisible("T_townName", false);
    }
    mLyt.calc();
    mKbd.buildLayout();
    mCur.build();
    mBtn.create();
    initButtons();
    setQuestion();
    setMaxLength();
    onFlag(FLAG_DRAW);
    clearText();
    startAnm(&mBtn.mLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    setWinAnm(ANM_UPPER_WIN_IN);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x374
int dMenuHan_c::doDelete() {
    if (!mCur.remove()) {
        return NOT_READY;
    }
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mSlct.remove()) {
        return NOT_READY;
    }
    BOOL removed = mKbd.remove(dKbd::Mng_c::REMOVE_MODE_SAVE_SETTING);
    return removed != 0;
}

// .text:0x3FC
int dMenuHan_c::stateMain() {
    if (handleCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    calcLayout();
    return SUCCEEDED;
}

// .text:0x44C
int dMenuHan_c::stateAnm() {
    if (calcAnm()) {
        changeNextState();
        if (isState(STATE_MAIN)) {
            updateCursor();
            onFlag(FLAG_CUR);
        }
        mWinAnm.setToEnd();
    } else {
        offFlag(FLAG_CUR);
    }
    mWinAnm.play();
    mLyt.calc();
    mVec3_c pos;
    m2d::getScreenPos(&pos, mBtn.mLyt.findPane("N_all"));
    mKbd.setPos(pos);
    mKbd.calc();
    return SUCCEEDED;
}

// .text:0x530
int dMenuHan_c::stateClose() {
    if (isFlag(FLAG_REGISTER)) {
        return SUCCEEDED;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x57C
int dMenuHan_c::execute() {
    int result = dMenuBase_c::execute();
    mBtn.execute();
    mSlct.execute();
    if (isFlag(FLAG_REGISTER) && dMenuFriend_c::registerEntry()) {
        offFlag(FLAG_REGISTER);
        if (mMenuNo == dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT) {
            dMenuFriend_c::setSave();
        }
    }
    return result;
}

// .text:0x604
int dMenuHan_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mKbd.entry();
        if (!isFlag(FLAG_FRAME)) {
            mLyt.entry();
        }
    }
    if (isFlag(FLAG_CUR)) {
        mCur.draw();
    }
    mBtn.draw();
    if (isFlag(FLAG_SLCT)) {
        mSlct.draw();
    }
    return SUCCEEDED;
}

// .text:0x6A4
void dMenuHan_c::subStateMain() {
    BOOL noPointer = FALSE;
    mBtn.clearSelect();
    BOOL pressed = mPad::g_currentCore->downTrigger(WPAD_BUTTON_A);
    int btn = mBtn.checkPad(!dSaveOption::getKeySe(), FALSE);
    BOOL fromPointer = btn == BTN_NONE;
    if (fromPointer) {
        btn = hitCheckBtn();
    }
    if (!fromPointer) {
        pressed = TRUE;
    }
    if (btn != BTN_NONE) {
        mBtn.setSelect(btn, TRUE, TRUE);
        if (pressed) {
            pushButton(btn);
            return;
        }
        noPointer = TRUE;
    }
    if (hitCheckText()) {
        updateText();
        return;
    }
    mKbd.open(noPointer, FALSE);
    if (mKbd.isTextChanged()) {
        if (checkLength()) {
            mKbd.updateInput();
            updateText();
        } else {
            dKbd::playSeCanNotInput();
            restoreText();
        }
        if (mKbd.isSendReq()) {
            pushButton(BTN_CONFIRM);
        } else if (mKbd.isClearReq()) {
            pushButton(BTN_QUIT);
        }
    }
}

// .text:0x824
void dMenuHan_c::subStatePush() {
    if (mBtn.isPushing()) {
        return;
    }
    dMenuMng_c::setResult(RESULT_CANCEL);
    switch (mBtn.mPushedBtn) {
    case BTN_QUIT:
        cancel();
        startCloseAnm();
        break;
    case BTN_UNUSED:
        break;
    case BTN_CONFIRM:
        switch (mMenuNo) {
        case dMenuMng_c::MENU_INPUT_SAY:
        case dMenuMng_c::MENU_INPUT_SONG:
            apply();
            startCloseAnm();
            break;
        default:
            memcpy(mText, mKbd.getText(), mMaxLen * sizeof(wchar_t));
            mCheckHits = 0;
            mCheckText = (const u16 *)mText;
            setSubState(SUB_CHECK);
            break;
        }
        break;
    }
}

// .text:0x914
void dMenuHan_c::subStateCheck() {
    if (dPrcMng_c::startCheck(&mCheckText, 1, &mCheckResult, &mCheckHits)) {
        setSubState(SUB_CHECK_RESULT);
    }
}

// .text:0x970
void dMenuHan_c::subStateCheckResult() {
    if (dPrcMng_c::isDone()) {
        if (mCheckHits > 0) {
            showCantEnter();
        } else {
            apply();
            startCloseAnm();
        }
    }
}

// .text:0x9CC
void dMenuHan_c::subStateMessage() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_MESSAGE_PUSH);
    }
}

// .text:0xA2C
void dMenuHan_c::subStateMessagePush() {
    if (mSlct.isPushEnd()) {
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
        setSubState(SUB_MESSAGE_CLOSE);
    }
}

// .text:0xA90
void dMenuHan_c::subStateMessageClose() {
    if (mSlct.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
    }
}

// .text:0xAE0
BOOL dMenuHan_c::closeMenu() {
    startAnm(&mBtn.mLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
    setWinAnm(ANM_UPPER_WIN_OUT);
    switch (mMenuNo) {
    case dMenuMng_c::MENU_INPUT_MY_FRIEND_NAME:
        dMenuFriend_c::startUpperWinOut();
        break;
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_NAME:
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_TOWN:
    case dMenuMng_c::MENU_INPUT_FRIEND_NAME:
    case dMenuMng_c::MENU_INPUT_FRIEND_TOWN:
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT:
    case dMenuMng_c::MENU_INPUT_FRIEND_COMMENT:
        dMenuFriend_c::requestClose();
        break;
    default:
        return FALSE;
    }
    dAcUitag::closeTab();
    dAcUitag::setHideAfterMenu();
    return TRUE;
}

// .text:0xB7C
BOOL dMenuHan_c::closeMessage() {
    if (!mSlct.isWinAnmEnd()) {
        return FALSE;
    }
    mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    return closeMenu();
}

// .text:0xBD8
BOOL dMenuHan_c::handleCloseRequest() {
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    int menu = mMenuNo;
    if (menu >= dMenuMng_c::MENU_INPUT_SAY ||
        menu < dMenuMng_c::MENU_INPUT_NEW_FRIEND_NAME) {
        return FALSE;
    }
    static CloseProc closeProcs[SUB_NUM] = {
        &dMenuHan_c::closeMenu,    nullptr, nullptr, nullptr,
        &dMenuHan_c::closeMessage, nullptr, nullptr};
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL result = (this->*closeProcs[mSubState])();
    if (result) {
        dMenuMng_c::clearCloseRequest();
    }
    return result;
}

// .text:0xD04
void dMenuHan_c::executeSubState() {
    static SubStateProc subStateProcs[SUB_NUM] = {
        &dMenuHan_c::subStateMain,        &dMenuHan_c::subStatePush,
        &dMenuHan_c::subStateCheck,       &dMenuHan_c::subStateCheckResult,
        &dMenuHan_c::subStateMessage,     &dMenuHan_c::subStateMessagePush,
        &dMenuHan_c::subStateMessageClose};
    (this->*subStateProcs[mSubState])();
}

// .text:0xD3C
void dMenuHan_c::calcLayout() {
    mKbd.calc();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
}

// .text:0xDA0
void dMenuHan_c::startCloseAnm() {
    startAnm(&mBtn.mLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
    setWinAnm(ANM_UPPER_WIN_OUT);
}

// .text:0xDF0
void dMenuHan_c::showCantEnter() {
    mSlct.setBtnVisible(0, false);
    mSlct.setBtnVisible(1, false);
    mSlct.setBtn(2, MSG_OK, nullptr, 0);
    playSeCanNot();
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
    setSubState(SUB_MESSAGE);
    onFlag(FLAG_SLCT);
    mSlct.setMsg(MSG_CANT_ENTER, nullptr);
}

// .text:0xE9C
void dMenuHan_c::setInput(int input) {
    static const char *textBoxNames[INPUT_NUM] = {
        "T_input_A_00", "T_input_B_00", "T_input_C_00", "T_input_D_00",
        "T_input_F_00"};
    static const char *paneNames[INPUT_NUM] = {
        "N_input_A", "N_input_B", "N_input_C", "N_input_D", "N_input_F"};
    static const char *frameNames[INPUT_NUM] = {
        "inputFrm_A_00", "inputFrm_B_00", "inputFrm_C_00", "inputFrm_D_00",
        "inputFrm_F_00"};
    mInput = input;
    nw4r::lyt::TextBox *textBox = mLyt.findTextBox(textBoxNames[input]);
    mWord.setTextBox(textBox);
    mMaxWidth = textBox->GetSize().width;
    for (int i = 0; i < INPUT_NUM; i++) {
        if (mInput != i) {
            mLyt.setVisible(paneNames[i], false);
        }
    }
    mLyt.setVisible("N_input_E", false);
    mpFrame = mLyt.findPane(frameNames[input]);
    switch (input) {
    case INPUT_A:
        mMaxLen = 0x10;
        break;
    case INPUT_B:
        mMaxLen = 0xA;
        break;
    case INPUT_C:
        mMaxLen = 6;
        break;
    case INPUT_F:
        mMaxLen = 4;
        break;
    default:
        mMaxLen = 0x20;
        break;
    }
}

// .text:0xFC0
void dMenuHan_c::setFrameInput(nw4r::lyt::TextBox *textBox,
                               nw4r::lyt::Pane *frame) {
    mWord.setTextBox(textBox);
    mMaxWidth = textBox->GetSize().width;
    mpFrame = frame;
}

// .text:0x1014
void dMenuHan_c::setIcon() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_INPUT_PLAYER_NAME:
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_NAME:
    case dMenuMng_c::MENU_INPUT_FRIEND_NAME:
        mIconAnm.setFrame(ICON_PLAYER);
        break;
    case dMenuMng_c::MENU_INPUT_TOWN_NAME:
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_TOWN:
    case dMenuMng_c::MENU_INPUT_FRIEND_TOWN:
        mIconAnm.setFrame(ICON_TOWN);
        break;
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT:
    case dMenuMng_c::MENU_INPUT_FRIEND_COMMENT:
        mIconAnm.setFrame(ICON_COMMENT);
        break;
    case dMenuMng_c::MENU_INPUT_SAY:
        mIconAnm.setFrame(ICON_SAY);
        break;
    case dMenuMng_c::MENU_INPUT_DESIGN_NAME:
    case dMenuMng_c::MENU_INPUT_PRO_DESIGN_NAME:
        mIconAnm.setFrame(ICON_DESIGN);
        break;
    case dMenuMng_c::MENU_INPUT_MESSAGE:
        mIconAnm.setFrame(ICON_MESSAGE);
        break;
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME:
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME_2:
        mIconAnm.setFrame(ICON_STAR_SIGN);
        break;
    case dMenuMng_c::MENU_INPUT_SONG:
        mIconAnm.setFrame(ICON_SONG);
        break;
    case dMenuMng_c::MENU_INPUT_CATCHPHRASE:
        mIconAnm.setFrame(ICON_NONE);
        break;
    case dMenuMng_c::MENU_INPUT_NICKNAME:
        mIconAnm.setFrame(ICON_NICKNAME);
        break;
    case dMenuMng_c::MENU_INPUT_GREETING:
        mIconAnm.setFrame(ICON_COMMENT);
        break;
    }
}

// .text:0x10CC
void dMenuHan_c::setQuestion() {
    int region = getRegion();
    switch (mMenuNo) {
    case dMenuMng_c::MENU_INPUT_PLAYER_NAME:
        setInput(INPUT_C);
        setMsg(MSG_PLAYER_NAME);
        break;
    case dMenuMng_c::MENU_INPUT_TOWN_NAME:
        setInput(INPUT_C);
        setMsg(MSG_TOWN_NAME);
        break;
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_NAME:
    case dMenuMng_c::MENU_INPUT_FRIEND_NAME:
        setInput(INPUT_C);
        setMsg(MSG_FRIEND_NAME);
        break;
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_TOWN:
    case dMenuMng_c::MENU_INPUT_FRIEND_TOWN:
        setInput(INPUT_C);
        setMsg(MSG_FRIEND_TOWN);
        break;
    case dMenuMng_c::MENU_INPUT_MY_FRIEND_NAME: {
        mInput = INPUT_A;
        mMaxLen = 0x10;
        nw4r::lyt::Pane *frame = dMenuFriend_c::getTextBoxWindow();
        setFrameInput(dMenuFriend_c::getTextBox(), frame);
        onFlag(FLAG_FRAME);
        break;
    }
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT:
    case dMenuMng_c::MENU_INPUT_FRIEND_COMMENT:
        setInput(INPUT_A);
        setMsg(MSG_COMMENT);
        break;
    case dMenuMng_c::MENU_INPUT_SAY:
        setInput(INPUT_B);
        setMsg(MSG_SAY);
        break;
    case dMenuMng_c::MENU_INPUT_DESIGN_NAME:
    case dMenuMng_c::MENU_INPUT_PRO_DESIGN_NAME:
        setInput(INPUT_B);
        setMsg(MSG_DESIGN_NAME);
        break;
    case dMenuMng_c::MENU_INPUT_MESSAGE:
        setInput(INPUT_B);
        setMsg(MSG_MESSAGE);
        break;
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME:
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME_2:
        setInput(INPUT_C);
        setMsg(MSG_STAR_SIGN_NAME);
        break;
    case dMenuMng_c::MENU_INPUT_SONG:
        setInput(INPUT_B);
        setMsg(MSG_SONG);
        break;
    case dMenuMng_c::MENU_INPUT_CATCHPHRASE:
        if (region == 0 || region == 3) {
            setInput(INPUT_F);
        } else {
            setInput(INPUT_D);
        }
        setMsg(MSG_CATCHPHRASE);
        break;
    case dMenuMng_c::MENU_INPUT_NICKNAME:
        setInput(INPUT_C);
        setMsg(MSG_NICKNAME);
        break;
    case dMenuMng_c::MENU_INPUT_GREETING:
        setInput(INPUT_B);
        setMsg(MSG_GREETING);
        break;
    default:
        setInput(INPUT_A);
        break;
    }
}

// .text:0x12FC
void dMenuHan_c::setMaxLength() {
    int region = getRegion();
#if VERSION == VERSION_RUUK01_01
    if (region == 0 || region == 3) {
        return;
    }
#else
    if (region == 0) {
        return;
    }
#endif
    switch (mMenuNo) {
    case dMenuMng_c::MENU_INPUT_PLAYER_NAME:
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_NAME:
    case dMenuMng_c::MENU_INPUT_FRIEND_NAME:
        mMaxLen = 8;
        break;
    case dMenuMng_c::MENU_INPUT_TOWN_NAME:
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_TOWN:
    case dMenuMng_c::MENU_INPUT_FRIEND_TOWN:
        mMaxLen = 8;
        break;
    case dMenuMng_c::MENU_INPUT_MY_FRIEND_NAME:
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT:
    case dMenuMng_c::MENU_INPUT_FRIEND_COMMENT:
        mMaxLen = 0x20;
        break;
    case dMenuMng_c::MENU_INPUT_DESIGN_NAME:
    case dMenuMng_c::MENU_INPUT_PRO_DESIGN_NAME:
        mMaxLen = 0x10;
        break;
    case dMenuMng_c::MENU_INPUT_MESSAGE:
        mMaxLen = 0x10;
        break;
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME:
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME_2:
        mMaxLen = 0x10;
        break;
    case dMenuMng_c::MENU_INPUT_SONG:
        mMaxLen = 0x10;
        break;
    case dMenuMng_c::MENU_INPUT_CATCHPHRASE:
        if (region == 0 || region == 3) {
            mMaxLen = 4;
        } else {
            mMaxLen = 0xA;
        }
        break;
    case dMenuMng_c::MENU_INPUT_NICKNAME:
        mMaxLen = 8;
        break;
    case dMenuMng_c::MENU_INPUT_GREETING:
        mMaxLen = 0x10;
        break;
    default:
        mMaxLen = 0x20;
        break;
    }
}

// .text:0x13F0
void dMenuHan_c::clearText() {
    memset(mText, 0, sizeof(mText));
}

// .text:0x1400
void dMenuHan_c::cancel() {
    dMenuMng_c::setResult(RESULT_CANCEL);
    switch (mMenuNo) {
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_NAME:
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_TOWN:
    case dMenuMng_c::MENU_INPUT_FRIEND_NAME:
    case dMenuMng_c::MENU_INPUT_FRIEND_TOWN:
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT:
    case dMenuMng_c::MENU_INPUT_FRIEND_COMMENT:
        dMenuFriend_c::requestShowTabs();
        break;
    case dMenuMng_c::MENU_INPUT_MY_FRIEND_NAME:
        dMenuFriend_c::requestShowTabs();
        dMenuFriend_c::startTextBoxDown();
        dMenuFriend_c::initFrameText();
        break;
    case dMenuMng_c::MENU_INPUT_DESIGN_NAME:
        dAcUitag::openMenuA(dAcUitag::MENU_A_EDIT_DESIGN);
        break;
    case dMenuMng_c::MENU_INPUT_PRO_DESIGN_NAME:
        dAcUitag::openMenuB(dAcUitag::MENU_B_EDIT_PRO);
        break;
    case dMenuMng_c::MENU_INPUT_SAY:
    case dMenuMng_c::MENU_INPUT_MESSAGE:
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME:
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME_2:
    case dMenuMng_c::MENU_INPUT_SONG:
        break;
    }
}

// .text:0x1484
void dMenuHan_c::apply() {
    dMenuMng_c::setResult(RESULT_CONFIRM);
    dPrivateData_c *player;
    const wchar_t *text = mKbd.getText();
    dMenuMng_c::setInputBuf((const u16 *)text, mMaxLen);
    player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dDesign_c *design;
    dSvStarSign::Sign_c sign;
    switch (mMenuNo) {
    case dMenuMng_c::MENU_INPUT_PLAYER_NAME:
        player->mPID.setPlayerName(text);
        break;
    case dMenuMng_c::MENU_INPUT_TOWN_NAME: {
        dSaveTown_c *town = dSaveData_c::getTown();
        town->mLandID.setRandomId(text, getLanguage());
        break;
    }
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_NAME:
        dMenuFriend_c::requestOpenMenu(
            dMenuFriend_c::OPEN_MENU_NEW_FRIEND_TOWN);
        dMenuFriend_c::setEntryPlayerName(text);
        break;
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_TOWN:
        dMenuFriend_c::requestOpenMenu(
            dMenuFriend_c::OPEN_MENU_NEW_FRIEND_COMMENT);
        dMenuFriend_c::setEntryTown(text);
        break;
    case dMenuMng_c::MENU_INPUT_FRIEND_NAME:
        dMenuFriend_c::requestOpenMenu(dMenuFriend_c::OPEN_MENU_FRIEND_TOWN);
        dMenuFriend_c::setEntryPlayerName(text);
        break;
    case dMenuMng_c::MENU_INPUT_FRIEND_TOWN:
        dMenuFriend_c::requestShowTabs();
        dMenuFriend_c::setEntryTown(text);
        onFlag(FLAG_REGISTER);
        break;
    case dMenuMng_c::MENU_INPUT_MY_FRIEND_NAME:
        dMenuFriend_c::requestShowTabs();
        dMenuFriend_c::startTextBoxDown();
        player->mFriends.setName(text);
        break;
    case dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT:
    case dMenuMng_c::MENU_INPUT_FRIEND_COMMENT:
        dMenuFriend_c::requestShowTabs();
        dMenuFriend_c::setEntryMessage(text);
        onFlag(FLAG_REGISTER);
        break;
    case dMenuMng_c::MENU_INPUT_DESIGN_NAME: {
        design = player->mOrgDesigns.getDesign(dMenuMng_c::getDesignIndex());
        *design = *dMenuMng_c::getDesign();
        design->setName(text);
        design->setStyle(DESIGN_STYLE);
        dAcUitag::openMenuA(dAcUitag::MENU_A_DESIGN_SELECT);
        dAcUitag::showTab();
        break;
    }
    case dMenuMng_c::MENU_INPUT_PRO_DESIGN_NAME: {
        design = player->mOrgDesigns.getDesign(dMenuMng_c::getDesignIndex());
        *design = *dMenuMng_c::getDesign();
        design->setName(text);
        break;
    }
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME:
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME_2: {
        int idx = dMenuMng_c::getPickedStarSign();
        sign.copy(dSaveData_c::getTown()->getStarSignList()->get(idx));
        sign.setName(text);
        dSaveData_c::getTown()->getStarSignList()->set(idx, &sign);
        break;
    }
    case dMenuMng_c::MENU_INPUT_SONG:
        applySong();
        break;
    }
}

// .text:0x1A94
void dMenuHan_c::applySong() {
    static const int NAME_LEN = NAME_ITEM_LEN;
    wchar_t name[NAME_LEN];
    memset(name, 0, sizeof(name));
    memcpy(name, mKbd.getText(), sizeof(name));
    for (int i = 0; i < NAME_LEN; i++) {
        if (name[i] == 0xFF01) {
            name[i] = 0x21;
        }
    }
    dItem::name_c typed;
    typed.clear();
    typed.set(name, 0);
    dItem::name_c song;
    dItem::Item item;
    for (u32 i = 0; i < SONG_NUM; i++) {
        item.setFromIndex(SONG_FIRST, i, FALSE);
        setItemName(&song, item);
        if (typed.isSame(&song)) {
            dMenuMng_c::setPickedItem(item.getId());
            return;
        }
    }
    dMenuMng_c::setPickedItem(dItem::ITEM_ID_NONE);
}

// .text:0x1BE8
BOOL dMenuHan_c::checkLength() {
    const wchar_t *text = mKbd.getText();
    const wchar_t *input = mKbd.getInputText();
    int textLen = m2d::Word_c::getLength(text);
    int inputLen = m2d::Word_c::getLength(input);
    if (textLen + inputLen > mMaxLen) {
        return FALSE;
    }
    static int size = mMaxLen + 1;
    static wchar_t buf[TEXT_SIZE];
    memset(buf, 0, sizeof(buf));
    if (textLen != 0) {
        memcpy(buf, text, textLen * sizeof(wchar_t));
    }
    if (inputLen != 0) {
        memcpy(buf + textLen, input, inputLen * sizeof(wchar_t));
    }
    if (mWord.calcWidth(buf, size) > mMaxWidth) {
        return FALSE;
    }
    memcpy(mText, text, mMaxLen * sizeof(wchar_t));
    return TRUE;
}

// .text:0x1D08
void dMenuHan_c::updateText() {
    const wchar_t *text = mKbd.getText();
    const wchar_t *input = mKbd.getInputText();
    int inputPos = mKbd.getInputPos();
    mWord.clear();
    mWord.set(text, 0);
    mWord.insertColored(input, inputPos, &l_inputColor.r, nullptr);
    mWord.toFixedSpaces();
    mWord.update();
    mWord.fromFixedSpaces();
    onFlag(FLAG_CALC);
    updateCursor();
    if (mMenuNo != dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT) {
        mBtn.setDisable(BTN_CONFIRM, !hasText());
    }
}

// .text:0x1DF4
void dMenuHan_c::restoreText() {
    mKbd.fixInput();
    mKbd.setText(mText);
    mKbd.resetCursor();
    updateText();
}

// .text:0x1E40
BOOL dMenuHan_c::hasText() {
    const wchar_t *text = mKbd.getText();
    if (m2d::Word_c::getLength(mKbd.getInputText()) > 0) {
        return TRUE;
    }
    for (int i = 0; i < mMaxLen; i++) {
        if (text[i] == 0) {
            return FALSE;
        }
        if (text[i] != L' ' && text[i] != 0x3000) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x1EE0
void dMenuHan_c::pushButton(int btn) {
    if (mBtn.isEnable(btn)) {
        mKbd.fixInput();
        updateText();
        mBtn.push(btn, FALSE);
        setSubState(SUB_PUSH);
    }
}

// .text:0x1F5C
void dMenuHan_c::updateCursor() {
    mVec3_c pos;
    m2d::getScreenPos(&pos, mWord.mpTextBox);
    pos.x = mWord.getScreenX(mKbd.getCursorPos());
    mCur.setPos(&pos);
}

// .text:0x1FB4
BOOL dMenuHan_c::hitCheckText() {
    if (!mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        return FALSE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    if (!m2d::isInPaneRect(mpFrame, 0.0f, &pos)) {
        return FALSE;
    }
    mKbd.fixInput();
    memcpy(mText, mKbd.getText(), mMaxLen * sizeof(wchar_t));
    mKbd.setCursorPos(mWord.getPosAtScreenX(pos.x));
    playSeCursor();
    return TRUE;
}

// .text:0x2078
dMenuHan_c::Word_c::Word_c() {
    clear();
}

// .text:0x20BC
dMenuHan_c::Word_c::~Word_c() {
}

// .text:0x2114
u32 dMenuHan_c::Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

// .text:0x211C
wchar_t *dMenuHan_c::Word_c::getBuffer() {
    return mBuffer;
}

// .text:0x2124
void dMenuHan_c::initButtons() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_INPUT_PLAYER_NAME:
    case dMenuMng_c::MENU_INPUT_TOWN_NAME:
    case dMenuMng_c::MENU_INPUT_SAY:
    case dMenuMng_c::MENU_INPUT_MESSAGE:
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME:
    case dMenuMng_c::MENU_INPUT_STAR_SIGN_NAME_2:
    case dMenuMng_c::MENU_INPUT_CATCHPHRASE:
    case dMenuMng_c::MENU_INPUT_NICKNAME:
    case dMenuMng_c::MENU_INPUT_GREETING:
        mBtn.setVisible(BTN_QUIT, false);
        break;
    case dMenuMng_c::MENU_INPUT_DESIGN_NAME:
    case dMenuMng_c::MENU_INPUT_PRO_DESIGN_NAME:
        mBtn.setText(BTN_QUIT, MSG_BACK, TRUE);
        break;
    default:
        mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
        break;
    }
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
    if (mMenuNo != dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT) {
        mBtn.setDisable(BTN_CONFIRM, TRUE);
    }
}

// .text:0x21F4
int dMenuHan_c::hitCheckBtn() {
    mVec2_c pos = dObjCursor_c::getPos();
    return mBtn.hitCheck(&pos);
}

// .text:0x2230
void dMenuHan_c::setMsg(u16 msg) {
    mLyt.setMessage("T_msg_00", msg, nullptr);
    onFlag(FLAG_CALC);
}

// .text:0x227C
void dMenuHan_c::setWinAnm(int anm) {
    m2d::ResAccIf_c *resAcc = dMenuMng_c::getWinResAcc();
    mLyt.setAnm(&mWinAnm, getAnmName(anm), resAcc);
}
