#include <game/game/d_menu_chat.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_chat.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_pad.hpp>
#include <stdio.h>
#include <string.h>

namespace dMenuChat {
void *classInit();
fProf::fBaseProfile_c g_profile_MENU_CHAT = {classInit, fProf::MENU_CHAT,
                                             fProf::DRAW_ORDER::MENU_CHAT};

// .text:0xCC
void *classInit() {
    return new dMenuChat_c;
}
} // namespace dMenuChat

inline dMenuChat_c::KeyboardMng_c::~KeyboardMng_c() {
}

static mColor l_inputColor(0x18, 0xF7, 0xFF, 0xFF);
static mColor l_presetColor(0xFF, 0xFF, 0x00, 0xFF);


// .text:0x348
int dMenuChat_c::create() {
    static const char *actionAnmNames[] = {
        "actionBtn_00_actnBtn_Out.brlan",
        "actionBtn_00_actnBtn_In.brlan",
    };
    static const char *actionIconAnmNames[] = {
        "actionBtn_00_actnIcnPattern.brlan",
    };
    int i;
    mKbd.init(dKbd::Mng_c::INIT_MODE_PAD);
    BOOL kbdLoaded = mKbd.loadRes();
    BOOL loaded = kbdLoaded & mCur.load();
    loaded &= mKbdRes.load("Layout/keyboard_free.arc");
    loaded &= mTeikeiRes.load("Layout/teikei_win.arc");
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
    mKbd.buildLayout();
    mLytText.build(&mKbdRes, "textBox_free_00.brlyt");
    mLytSay.build(&mKbdRes, "speakBtn_00.brlyt");
    mLytAll.build(&mKbdRes, "speakBtn_00.brlyt");
    initTextAnm();
    mLytText.calc();
    mLytSay.calc();
    mLytAction.build(dAcUitag::getActionResAcc(), "actionBtn_00.brlyt");
    mLytAction.calc();
    mLytAction.setAnm(&mActionPushAnm, "actionBtn_00_actnBtn_Psh.brlan",
                      nullptr);
    initChangeBtn();
    initKbdLyt();
    mKbd.setLayers(&mLytKbd, &mLytAction, &mLytChange);
    mCur.build();
    initTeikei();
    onFlag(FLAG_DRAW);
    // The original pane-name literal retains one extra word of terminators.
    mWord.setTextBox(mLytText.findTextBox("T_textBox_00\0\0\0\0"));
    memset(mText, 0, sizeof(mText));
    for (i = 0; i < ACTION_NUM; i++) {
        mActionAnm[i].init(&mLytAction, dAcUitag::getActionResAcc(),
                           actionAnmNames, 2);
        mActionAnm[i].setNumber(i);
    }
    for (i = 0; i < ACTION_NUM; i++) {
        mActionIconAnm[i].init(&mLytAction, dAcUitag::getActionResAcc(),
                               actionIconAnmNames, 1);
        mActionIconAnm[i].setNumber(i);
        mActionIconAnm[i].start(0);
        u8 action = getAction(i);
        if (action == NO_ACTION) {
            mActionIconAnm[i].setFrame(0.0f);
            char name[0x28];
            sprintf(name, "N_actnBtn_%02d", i);
            mLytAction.setVisible(name, false);
        } else {
            mActionIconAnm[i].setFrame(action - 1);
        }
    }
    mActionSlot = -1;
    startAnm(&mLytAll, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    if (dKbd::getSavedFlag()) {
        onFlag(FLAG_PRESET);
        setMode(MODE_PRESET);
        setChangeIcon(MODE_PRESET);
    } else {
        setMode(MODE_KEYBOARD);
        updateText();
        setChangeIcon(MODE_KEYBOARD);
    }
    dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_21;
    dObjChat::fixPositions();
    mZBuff.set(240.0f, -240.0f, -320.0f, 320.0f, -499.9f);
    return SUCCEEDED;
}

// .text:0x6E0
int dMenuChat_c::doDelete() {
    if (!mCur.remove()) {
        return NOT_READY;
    }
    if (!mKbd.remove(dKbd::Mng_c::REMOVE_MODE_SAVE_SETTING)) {
        return NOT_READY;
    }
    if (!mKbdRes.remove()) {
        return NOT_READY;
    }
    bool removed = mTeikeiRes.remove();
    return removed;
}

// .text:0x768
int dMenuChat_c::stateMain() {
    int prev = mCursor;
    if (checkCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    if (mCursor != CURSOR_NONE && prev != mCursor) {
        playSeCursor();
    }
    return SUCCEEDED;
}

// .text:0x7D4
int dMenuChat_c::stateAnm() {
    if (calcAnm()) {
        changeNextState();
        if (mNextMode == MODE_KEYBOARD && isState(STATE_MAIN)) {
            updateCursor();
            onFlag(FLAG_CURSOR);
        }
    } else {
        offFlag(FLAG_CURSOR);
    }
    mVec3_c pos;
    m2d::getScreenPos(&pos, mLytAll.findPane("N_all"));
    pos.x = 0.0f;
    if (!isFlag(FLAG_SWITCH)) {
        mLytAction.findPane("N_actnBtn_all")->SetTranslate(pos);
        mLytAction.calc();
        mLytChange.findPane("N_all")->SetTranslate(pos);
        mLytChange.calc();
    }
    if (isFlag(FLAG_PRESET)) {
        mLytTeikei.findPane("N_ctlgWin_all")->SetTranslate(pos);
        mLytTeikei.calc();
    } else {
        setKbdPos(&pos);
    }
    return SUCCEEDED;
}

// .text:0x96C
void dMenuChat_c::setKbdPos(const mVec3_c *pos) {
    mVec3_c keyPos = *pos;
    mKbd.setPos(*pos);
    nw4r::lyt::Pane *keytop;
    nw4r::lyt::Pane *bound;
    if (!dSaveOption::getKeySe()) {
        keyPos.y -= 23.0f;
        keytop = mLytKbd.findPane("N_keytop_all");
        bound = mLytKbd.findPane("N_Bound_ALL");
    } else {
        keyPos.y -= 20.0f;
        keytop = mLytKbd.findPane("N_CPkeytop_all");
        bound = mLytKbd.findPane("N_CP_Bound_ALL");
    }
    keytop->SetTranslate(keyPos);
    bound->SetTranslate(keyPos);
    mLytKbd.calc();
}

// .text:0xA8C
int dMenuChat_c::stateClose() {
    if (isFlag(FLAG_SWITCH)) {
        if (mNextMode == MODE_KEYBOARD) {
            offFlag(FLAG_PRESET);
            setMode(MODE_KEYBOARD);
        } else {
            onFlag(FLAG_PRESET);
            setMode(MODE_PRESET);
        }
        startAnm(&mLytAll, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
        stateAnm();
        return SUCCEEDED;
    }
    dKbd::setSavedByte(mGroup);
    dKbd::setSavedFlag(isFlag(FLAG_PRESET));
    deleteRequest();
    return SUCCEEDED;
}

// .text:0xB68
int dMenuChat_c::execute() {
    int result = dMenuBase_c::execute();
    updateTimer();
    playAnm();
    mKbd.calc();
    calcLayouts();
    return result;
}

// .text:0xBC4
int dMenuChat_c::draw() {
    mZBuff.entry();
    if (isFlag(FLAG_DRAW)) {
        updateKbdVisible();
        if (isFlag(FLAG_PRESET)) {
            mLytTeikei.entry();
            mLytAction.entry();
            mLytChange.entry();
        } else {
            mKbd.entry();
        }
        mLytText.entry();
        mLytSay.entry();
    }
    if (isFlag(FLAG_CURSOR) && mTimer > 0) {
        mCur.draw();
    }
    return SUCCEEDED;
}

// .text:0xC84
BOOL dMenuChat_c::checkTabInput() {
    setSelect(mCursor, FALSE);
    mCursor = CURSOR_NONE;
    mActionSlot = -1;
    int tab = dAcUitag::getTabInput(dAcUitag_c::TAB_CHAT,
                                    dSaveOption::getKeySe() != 0);
    if (tab != -1 && selectTab(tab, TRUE)) {
        if (isFlag(FLAG_TAB_HIDDEN)) {
            dAcUitag::showTab();
            offFlag(FLAG_TAB_HIDDEN);
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0xD2C
void dMenuChat_c::subStateKeyboard() {
    if (checkTabInput()) {
        return;
    }
    if (!mKbd.isSignWindowOpen()) {
        if (isFlag(FLAG_TAB_HIDDEN)) {
            dAcUitag::showTab();
            offFlag(FLAG_TAB_HIDDEN);
        }
        mCursor = hitCheckKbd();
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, TRUE);
            if (mCursor == CURSOR_ENTER) {
                mKbd.mOrder = KeyboardMng_c::ORDER_AFTER;
            }
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                if (pushCursorKbd()) {
                    return;
                }
            }
            if (isAction(mCursor)) {
                showActionTutorial(mActionSlot);
            }
        }
    } else {
        if (!isFlag(FLAG_TAB_HIDDEN)) {
            dAcUitag::hideTab();
            onFlag(FLAG_TAB_HIDDEN);
        }
        if (dObjCursor_c::isVisible()) {
            if (hitSay()) {
                mCursor = CURSOR_SAY;
                setSelect(CURSOR_SAY, TRUE);
                if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                    if (pushCursorKbd()) {
                        return;
                    }
                }
            }
        }
    }
    mKbd.open(mCursor != CURSOR_NONE, 0);
    if (mKbd.isSendReq()) {
        if (sendText(TRUE)) {
            mCursor = CURSOR_ENTER;
            mKbd.mOrder = KeyboardMng_c::ORDER_AFTER;
            mKbd.closeSignWindow();
            mKbd.setABC();
            return;
        }
    } else if (mKbd.isClearReq()) {
        clearText();
        resetTimer();
        mKbd.closeSignWindow();
    } else if (checkActionKey()) {
        mKbd.closeSignWindow();
        reopenKbd();
        return;
    }
    if (mKbd.isTextChanged()) {
        resetTimer();
        if (checkPointerCursor()) {
            updateText();
        } else if (checkLength()) {
            mKbd.updateInput();
            updateText();
        } else {
            dKbd::playSeCanNotInput();
            restoreText();
        }
    }
}

// .text:0xFDC
void dMenuChat_c::subStatePreset() {
    if (checkTabInput()) {
        return;
    }
    mCursor = hitCheckPreset();
    if (mCursor != CURSOR_NONE) {
        setSelect(mCursor, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            if (pushCursorPreset()) {
                return;
            }
        }
        if (isAction(mCursor)) {
            showActionTutorial(mActionSlot);
        }
        if (isGenre(mCursor)) {
            showGenreTutorial(mCursor - CURSOR_GENRE);
        }
    }
    if (checkActionKey()) {
        return;
    }
    if (mKbd.isSendReq() && mPresetGroup != -1 && mPreset != -1) {
        sendPreset();
        mPreset = -1;
        updateRowSelect();
    } else {
        padPreset();
    }
}

// .text:0x1124
void dMenuChat_c::subStateActionPush() {
    onFlag(FLAG_ACTION);
    if (mActionPushAnm.mFrameCtrl.isStop()) {
        setSubState(SUB_EMOTION);
    } else {
        mActionPushAnm.play();
    }
}

// .text:0x1188
void dMenuChat_c::subStateEmotion() {
    if (dPlayerMgr::playEmotion(getAction(mActionSlot))) {
        setSubState(SUB_EMOTION_END);
    }
}

// .text:0x11E0
void dMenuChat_c::subStateEmotionEnd() {
    if (dPlayerMgr::isStartingDemo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        restoreSubState();
        if (isFlag(FLAG_KEEP_TIMER)) {
            resetTimer();
        }
    }
}

// .text:0x123C
void dMenuChat_c::subStateStartChat() {
    if (dPlayerMgr::startChat()) {
        setSubState(SUB_CHAT_END);
    }
}

// .text:0x1288
void dMenuChat_c::subStateChatEnd() {
    if (dPlayerMgr::isStartingDemo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        restoreSubState();
    }
}

// .text:0x12C8
void dMenuChat_c::subStateChangeToPreset() {
    if (isChangePushEnd()) {
        finishChangePush();
    }
}

// .text:0x1304
void dMenuChat_c::subStatePresetPush() {
    if (isPresetPushEnd()) {
        finishPresetPush();
    }
}

// .text:0x1340
void dMenuChat_c::subStateScrollHold() {
    if (!isFlag(FLAG_SCROLL)) {
        updateScrollBtns();
        if (mPad::g_currentCore->down(WPAD_BUTTON_A) ||
            (mCursor == CURSOR_SCROLL_UP && dPad::isUpHeld()) ||
            (mCursor == CURSOR_SCROLL_DOWN && dPad::isDownHeld())) {
            if (startScroll(mCursor - CURSOR_SCROLL_UP)) {
                return;
            }
        }
        restoreSubState();
    }
}

// .text:0x13F4
void dMenuChat_c::subStateSlider() {
    mVec3_c pos = mpSlider->GetTranslate();
    mVec2_c pointer = dObjCursor_c::getPos();
    pos.y = mSliderGrab + pointer.y;
    if (pos.y >= mSliderTop) {
        pos.y = mSliderTop;
    } else if (pos.y <= mSliderBottom) {
        pos.y = mSliderBottom;
    }
    f32 ratio = (pos.y - mSliderTop) / (mSliderBottom - mSliderTop);
    int scroll = (int)(0.5f + (f32)(mPresetNum - 3) * ratio) - 1;
    if (mScroll != scroll) {
        mScroll = scroll;
        updateRows();
        playSeSlideClick();
    }
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        updateSliderPos();
        restoreSubState();
        updateScrollBtns();
    } else {
        mpSlider->SetTranslate(pos);
    }
}

// .text:0x1554
BOOL dMenuChat_c::closeMenu() {
    if (isFlag(FLAG_TAB_HIDDEN)) {
        dAcUitag::showTab();
    }
    dAcUitag::closeTab();
    startClose();
    return TRUE;
}

// .text:0x15A0
BOOL dMenuChat_c::checkCloseRequest() {
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    static BOOL (dMenuChat_c::*closeProcs[SUB_NUM])() = {
        &dMenuChat_c::closeMenu,
        &dMenuChat_c::closeMenu,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr};
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL result = (this->*closeProcs[mSubState])();
    if (result) {
        dMenuMng_c::clearCloseRequest();
    }
    return result;
}

// .text:0x16E0
void dMenuChat_c::executeSubState() {
    static SubStateProc subStateProcs[SUB_NUM] = {
        &dMenuChat_c::subStateKeyboard,   &dMenuChat_c::subStatePreset,
        &dMenuChat_c::subStateActionPush, &dMenuChat_c::subStateEmotion,
        &dMenuChat_c::subStateEmotionEnd, &dMenuChat_c::subStateStartChat,
        &dMenuChat_c::subStateChatEnd,    &dMenuChat_c::subStateChangeToPreset,
        &dMenuChat_c::subStatePresetPush, &dMenuChat_c::subStateScrollHold,
        &dMenuChat_c::subStateSlider,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x1718
BOOL dMenuChat_c::checkLength() {
    const wchar_t *text = mKbd.getText();
    const wchar_t *input = mKbd.getInputText();
    int textLen = m2d::Word_c::getLength(text);
    int inputLen = m2d::Word_c::getLength(input);
    if (textLen + inputLen > TEXT_MAX) {
        return FALSE;
    }
    wchar_t buf[TEXT_SIZE];
    memset(buf, 0, sizeof(buf));
    if (textLen != 0) {
        memcpy(buf, text, textLen * sizeof(wchar_t));
    }
    if (inputLen != 0) {
        memcpy(buf + textLen, input, inputLen * sizeof(wchar_t));
    }
    if (mWord.calcWidth(buf, TEXT_SIZE) >= 402.0f) {
        return FALSE;
    }
    memcpy(mText, text, TEXT_MAX * sizeof(wchar_t));
    return TRUE;
}

// .text:0x1814
void dMenuChat_c::updateText() {
    const wchar_t *text = mKbd.getText();
    const wchar_t *input = mKbd.getInputText();
    int inputLen = m2d::Word_c::getLength(input);
    int inputPos = mKbd.getInputPos();
    mWord.clear();
    mWord.set(text, 0);
    if (inputLen > 0) {
        mWord.insertColored(input, inputPos, &l_inputColor.r, nullptr);
    }
    mWord.toFixedSpaces();
    mWord.update();
    mWord.fromFixedSpaces();
    onFlag(FLAG_TEXT);
    updateCursor();
}

// .text:0x18D8
void dMenuChat_c::showPreset() {
    m2d::stringWord_c word;
    getRowWord(&word, mPresetGroup, mPreset);
    mWord.clear();
    mWord.insertColored(word.getText(), 0, &l_presetColor.r, nullptr);
    mWord.update();
    onFlag(FLAG_TEXT);
    resetTimer();
}

// .text:0x197C
void dMenuChat_c::restoreText() {
    mKbd.fixInput();
    mKbd.setText(mText);
    mKbd.resetCursor();
    updateText();
}

// .text:0x19C8
void dMenuChat_c::reopenKbd() {
    if (mKbd.reopen()) {
        updateText();
    }
}

// .text:0x1A08
int dMenuChat_c::getTabAt(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = -1;
    }
    dAcUitag::setTabFocus(tab);
    if (tab == -1) {
        return -1;
    }
    return tab;
}

// .text:0x1A6C
BOOL dMenuChat_c::selectTab(int tab, BOOL delay) {
    if (dAcUitag::selectTab(tab, delay)) {
        startClose();
        return TRUE;
    }
    return FALSE;
}

// .text:0x1ABC
void dMenuChat_c::startClose() {
    offFlag(FLAG_SWITCH);
    startAnm(&mLytAll, "N_all", ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dObjChat::unfixPositions();
    mTimer = 0;
}

// .text:0x1B18
void dMenuChat_c::startSwitch() {
    clearText();
    onFlag(FLAG_SWITCH);
    mPreset = -1;
    startAnm(&mLytAll, "N_all", ANM_WIN_OUT, STATE_CLOSE, FALSE);
}

// .text:0x1B78
int dMenuChat_c::hitAction(const mVec2_c *pos) {
    mVec2_c hitPos = *pos;
    return m2d::hitButton(&hitPos, &mLytAction, 0, ACTION_NUM - 1, -1, -1,
                          1.0f);
}

// .text:0x1BCC
void dMenuChat_c::selectAction(int slot) {
    char name[0x30];
    sprintf(name, "N_actnBtn_%02d", slot);
    mLytAction.setPosFrom(name, "actnBtn_Psh", 0.0f, 0.0f);
    mActionPushAnm.setFrame(0.0f);
    setSubState(SUB_ACTION_PUSH);
    onFlag(FLAG_ACTION);
    saveTimer();
}

// .text:0x1C6C
u8 dMenuChat_c::getAction(int slot) {
    dPrivateData_c *data = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (data->mEmotions.isValid(slot) == FALSE) {
        return NO_ACTION;
    }
    return data->mEmotions.get(slot);
}

// .text:0x1CD4
BOOL dMenuChat_c::checkActionKey() {
    int slot = mKbd.getEmotion();
    if (slot != -1 && getAction(slot) != NO_ACTION) {
        mActionSlot = slot;
        selectAction(slot);
        return TRUE;
    }
    return FALSE;
}

// .text:0x1D4C
void dMenuChat_c::updateCursor() {
    mVec3_c pos;
    m2d::getScreenPos(&pos, mWord.mpTextBox);
    pos.x = mWord.getScreenX(mKbd.getCursorPos());
    mCur.setPos(&pos);
}

// .text:0x1DA4
BOOL dMenuChat_c::checkPointerCursor() {
    if (!mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        return FALSE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    if (!m2d::isInPaneRect(mLytText.findPane("T_textBox_00\0\0\0\0"), 0.0f, &pos)) {
        return FALSE;
    }
    mKbd.fixInput();
    memcpy(mText, mKbd.getText(), TEXT_MAX * sizeof(wchar_t));
    mKbd.setCursorPos(mWord.getPosAtScreenX(pos.x));
    playSeCursor();
    return TRUE;
}

// .text:0x1E70
void dMenuChat_c::showActionTutorial(int slot) {
    u8 action = getAction(slot);
    if (action != NO_ACTION) {
        u16 msg = action + dObjTutorial_c::MSG_EMOTION_BASE;
        if (dObjTutorial_c::request(0, msg, TRUE)) {
            char name[0x30];
            sprintf(name, "actnIcn_%02d", slot);
            mVec3_c pos;
            m2d::getScreenPos(&pos, mLytAction.findPane(name));
            mVec2_c pos2(static_cast<const float *>(pos));
            dObjTutorial_c::setMsg(&pos2, msg, 0);
        }
    }
}

// .text:0x1F2C
void dMenuChat_c::showGenreTutorial(int genre) {
    u16 msg = genre + dObjTutorial_c::MSG_GENRE_GREETINGS;
    if (dObjTutorial_c::request(0, msg, TRUE)) {
        char name[0x30];
        sprintf(name, "N_genreBtn_%02d", genre);
        mVec3_c pos;
        m2d::getScreenPos(&pos, mLytTeikei.findPane(name));
        mVec2_c pos2(static_cast<const float *>(pos));
        dObjTutorial_c::setMsg(&pos2, msg, 0);
    }
}

// .text:0x1FD8
void dMenuChat_c::initTextAnm() {
    // Unreferenced word retained between the original string pools.
#pragma push
#pragma explicit_zero_data on
#pragma force_active on
    static u32 l_unused = 0;
#pragma pop
    static const char *textAnmNames[] = {
        "textBox_free_00_boxAlpIn.brlan",
        "textBox_free_00_boxAlpOut.brlan",
    };
    static const char *sayAlphaAnmNames[] = {
        "speakBtn_00_spkBtn_AlpIn.brlan",
        "speakBtn_00_spkBtn_AlpOut.brlan",
    };
    static const char *sayAnmNames[] = {
        "speakBtn_00_spkBtn_In.brlan",
        "speakBtn_00_spkBtn_Out.brlan",
        "speakBtn_00_spkBtn_Psh.brlan",
    };
    mTextAnm.init(&mLytText, &mKbdRes, textAnmNames, 2);
    mTextAnm.startEnd(SHOW_ANM_OUT);
    mTimer = 0;
    mLytText.setAnm(&mTextUpAnm, "textBox_free_00_boxUp.brlan", nullptr);
    mTextUpAnm.setToEnd();
    mSayAlphaAnm.init(&mLytSay, &mKbdRes, sayAlphaAnmNames, 2);
    mSayAlphaAnm.startEnd(SHOW_ANM_OUT);
    mSayAnm.init(&mLytSay, &mKbdRes, sayAnmNames, 3);
    mSayAnm.startEnd(BTN_ANM_UNFOCUS);
}

// .text:0x20A0
void dMenuChat_c::resetTimer() {
    mTimer = TEXT_SHOW_FRAMES;
}

// .text:0x20AC
void dMenuChat_c::updateTimer() {
    if (mTimer > 0) {
        mTimer--;
        mTextAnm.setNext(SHOW_ANM_IN);
        mSayAlphaAnm.setNext(SHOW_ANM_IN);
    } else {
        mTextAnm.setNext(SHOW_ANM_OUT);
        mSayAlphaAnm.setNext(SHOW_ANM_OUT);
    }
}

// .text:0x211C
void dMenuChat_c::saveTimer() {
    if (mTimer > 0) {
        onFlag(FLAG_KEEP_TIMER);
    } else {
        offFlag(FLAG_KEEP_TIMER);
    }
    mTimer = 0;
}

// .text:0x216C
void dMenuChat_c::initKbdLyt() {
    static const char *enterAnmNames[] = {
        "EnterBtn_00_Focus-IN.brlan",
        "EnterBtn_00_Focus-OUT.brlan",
        "EnterBtn_00_Pushed.brlan",
    };
    static const char *cellPhoneAnmNames[] = {
        "EnterBtn_cellPhone_00_Focus-IN.brlan",
        "EnterBtn_cellPhone_00_Focus-OUT.brlan",
        "EnterBtn_cellPhone_00_Pushed.brlan",
    };
    if (!dSaveOption::getKeySe()) {
        mLytKbd.build(mKbd.getResAcc(), "EnterBtn_00.brlyt");
        mEnterAnm.init(&mLytKbd, mKbd.getResAcc(), enterAnmNames, 3);
    } else {
        mLytKbd.build(mKbd.getResAcc(), "EnterBtn_cellPhone_00.brlyt");
        mEnterAnm.init(&mLytKbd, mKbd.getResAcc(), cellPhoneAnmNames, 3);
    }
    mEnterAnm.start(BTN_ANM_UNFOCUS);
    mEnterAnm.setToEnd();
    mLytKbd.mPos.z = -1.0f;
    mLytKbd.calc();
}

static inline void setKbdPanes(m2d::SimpleEx_c *lyt, bool ascii) {
    lyt->setVisible("N_VK_grid", !ascii);
    lyt->setVisible("N_VK_grd_Bnd_ALL", !ascii);
    lyt->setVisible("N_VK_ascii", ascii);
    lyt->setVisible("N_VK_asc_Bnd_ALL", ascii);
}

// .text:0x2258
void dMenuChat_c::updateKbdVisible() {
    if (!dSaveOption::getKeySe()) {
        if (!mKbd.isPCKeyboardVisible()) {
            mLytKbd.getRootPane()->SetVisible(false);
        } else {
            mLytKbd.getRootPane()->SetVisible(true);
            BOOL kana = FALSE;
            if (mKbd.isKanaInput()) {
                kana = TRUE;
            }
            setKbdPanes(&mLytKbd, !kana);
        }
    }
}

// .text:0x2354
BOOL dMenuChat_c::hitKbdEnter() {
    m2d::hitArg_c hit;
    hit.mpHit = nullptr;
    hit.mScale = 1.0f;
    mVec2_c pos = dObjCursor_c::getPos();
    hit.mPos = pos;
    mLytKbd.patrolPane(m2d::isInPane, m2d::isRectButton, &hit);
    return hit.mpHit != nullptr;
}

// .text:0x23D4
BOOL dMenuChat_c::sendText(BOOL kbd) {
    mKbd.fixInput();
    memcpy(mText, mKbd.getText(), TEXT_MAX * sizeof(wchar_t));
    if (mText[0] == 0) {
        return FALSE;
    }
    if (kbd) {
        mEnterAnm.setNext(BTN_ANM_PUSH);
    } else {
        mSayAnm.setNext(BTN_ANM_PUSH);
    }
    setSubState(SUB_START_CHAT);
    dObjChat::postText(mText, dObjChat::TEXT_KIND_PLAYER);
    clearText();
    dKbd::playSeSubmit();
    return TRUE;
}

// .text:0x2494
void dMenuChat_c::sendPreset() {
    clearText();
    dObjChat::postPreset(mPresetGroup, mPreset);
    dKbd::playSeSubmit();
    setSubState(SUB_START_CHAT);
}

// .text:0x24E8
void dMenuChat_c::clearText() {
    mTimer = 0;
    memset(mText, 0, sizeof(mText));
    mKbd.setCursorPos(0);
    restoreText();
    mKbd.clearEmotion();
}

// .text:0x2544
BOOL dMenuChat_c::hitSay() {
    if (mTimer <= 0) {
        return FALSE;
    }
    m2d::hitArg_c hit;
    hit.mpHit = nullptr;
    hit.mScale = 1.0f;
    mVec2_c pos = dObjCursor_c::getPos();
    hit.mPos = pos;
    mLytSay.patrolPane(m2d::isInPane, m2d::isRectButton, &hit);
    return hit.mpHit != nullptr;
}

// .text:0x25D8
void dMenuChat_c::initChangeBtn() {
    static const char *changeAnmNames[] = {
        "chatChangeBtn_00_chngBtn_in.brlan",
        "chatChangeBtn_00_chngBtn_out.brlan",
        "chatChangeBtn_00_chngBtn_psh.brlan",
    };
    mLytChange.build(dAcUitag::getActionResAcc(), "chatChangeBtn_00.brlyt");
    mChangeAnm.init(&mLytChange, dAcUitag::getActionResAcc(), changeAnmNames,
                    3);
    mChangeAnm.startEnd(BTN_ANM_UNFOCUS);
    setChangeIcon(MODE_KEYBOARD);
    mLytChange.calc();
}

// .text:0x2664
void dMenuChat_c::setChangeIcon(int mode) {
    static const char *changeIconAnmNames[] = {
        "chatChangeBtn_00_chngBtn_01_On.brlan",
        "chatChangeBtn_00_chngBtn_00_On.brlan",
    };
    mLytChange.setAnm(&mChangeIconAnm, changeIconAnmNames[mode], nullptr);
    mChangeIconAnm.setToEnd();
    onFlag(FLAG_CHANGE);
    mNextMode = mode;
}

// .text:0x26D0
BOOL dMenuChat_c::hitChange() {
    m2d::hitArg_c hit;
    hit.mpHit = nullptr;
    hit.mScale = 1.0f;
    mVec2_c pos = dObjCursor_c::getPos();
    hit.mPos = pos;
    mLytChange.patrolPane(m2d::isInPane, m2d::isRectButton, &hit);
    return hit.mpHit != nullptr;
}

// .text:0x2750
void dMenuChat_c::initTeikei() {
    mLytTeikei.build(&mTeikeiRes, "teikei_win_00.brlyt");
    mLytTeikei.setVisible("B_msg_00", false);
    mLytTeikei.setVisible("B_msg_04", false);
    mLytTeikei.setVisible("B_msg_05", false);
    mLytTeikei.setVisible("W_gnrBtn_Fc", false);
    mLytTeikei.setAlpha("W_gnrBtn_Fc", 0xFF);
    mLytTeikei.setAnm(&mScrollIconAnm, "teikei_win_00_scrlIcon_00.brlan",
                      nullptr);
    initSlider();
    initRowAnm();
    initGenreAnm();
    mGroup = -1;
    selectGroup(dKbd::getSavedByte());
    mPresetGroup = -1;
    mPreset = -1;
    updateRows();
    initScrollAnm();
    updateScrollBtns();
    updateSliderPos();
    mLytTeikei.calc();
}

// .text:0x2868
void dMenuChat_c::initRowAnm() {
    static const char *rowAnmNames[] = {
        "teikei_win_00_T_msg_In.brlan",    "teikei_win_00_T_msg_Out.brlan",
        "teikei_win_00_T_msg_Psh.brlan",   "teikei_win_00_T_msg_Gry.brlan",
        "teikei_win_00_T_msg_White.brlan",
    };
    for (int i = 0; i < ROW_NUM; i++) {
        mRowAnm[i].init(&mLytTeikei, &mTeikeiRes, rowAnmNames, 5);
        mRowAnm[i].setNumber(i);
    }
    for (int i = 0; i < ROW_NUM; i++) {
        char name[0x20];
        sprintf(name, "T_msg_%02d", i);
        mRowWord[i].setTextBox(mLytTeikei.findTextBox(name));
    }
}

// .text:0x2930
void dMenuChat_c::playRowAnm() {
    int changed = 0;
    for (int i = 0; i < ROW_NUM; i++) {
        changed |= mRowAnm[i].play();
    }
    if (changed) {
        onFlag(FLAG_PRESET_LYT);
    }
}

// .text:0x29AC
void dMenuChat_c::updateRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        getRowWord(&mRowWord[i], mGroup, mScroll + i);
        mRowWord[i].update();
    }
}

// .text:0x2A1C
void dMenuChat_c::getRowWord(dScript::Word_c *word, int group, int idx) {
    word->clear();
    if (idx < 0 || idx >= mPresetNum) {
        return;
    }
    if (group < 0 || group >= GENRE_NUM) {
        return;
    }
    dObjChat::getPreset(word, group, idx);
}

// .text:0x2AAC
void dMenuChat_c::updateRowSelect() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[0x20];
        sprintf(name, "slctBase_%02d", i);
        BOOL visible = FALSE;
        if (mPresetGroup == mGroup) {
            int preset = mPreset;
            BOOL selected = FALSE;
            if (preset >= 0 && preset == mScroll + i) {
                selected = TRUE;
            }
            if (selected) {
                visible = TRUE;
            }
        }
        mLytTeikei.setVisible(name, visible);
    }
}

// .text:0x2B68
void dMenuChat_c::setRowAnms(int select, int shift) {
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + shift;
        int anm;
        if (select != -1 && idx == select) {
            anm = ROW_ANM_IN;
        } else if (idx <= 0 || idx >= 4) {
            anm = ROW_ANM_GRAY;
        } else {
            anm = ROW_ANM_OUT;
        }
        mRowAnm[i].startEnd(anm);
    }
    mLytTeikei.calc();
    offFlag(FLAG_PRESET_LYT);
}

// .text:0x2C18
void dMenuChat_c::updateRows() {
    updateRowWords();
    setRowAnms(-1, 0);
    updateRowSelect();
}

// .text:0x2C5C
void dMenuChat_c::initGenreAnm() {
    static const char *genreAnmNames[] = {
        "teikei_win_00_genreBtn_On.brlan",
        "teikei_win_00_genreBtn_Off.brlan",
    };
    for (int i = 0; i < GENRE_NUM; i++) {
        mGenreAnm[i].init(&mLytTeikei, &mTeikeiRes, genreAnmNames, 2);
        mGenreAnm[i].setNumber(i);
        mGenreAnm[i].startEnd(GENRE_ANM_OFF);
    }
    mLytTeikei.setAnm(&mGenrePushAnm, "teikei_win_00_gnrBtn_Psh.brlan",
                      nullptr);
    mGenrePushAnm.setToEnd();
}

// .text:0x2D08
void dMenuChat_c::playGenreAnm() {
    for (int i = 0; i < GENRE_NUM; i++) {
        mGenreAnm[i].play();
    }
    mGenrePushAnm.play();
}

// .text:0x2D68
void dMenuChat_c::selectGroup(int group) {
    if (mGroup != -1) {
        mGenreAnm[mGroup].setNext(GENRE_ANM_OFF);
    }
    mGroup = group;
    mGenreAnm[group].setNext(GENRE_ANM_ON);
    mPresetNum = dObjChat::getPresetNum(group);
    mScroll = -1;
}

// .text:0x2DE8
void dMenuChat_c::initScrollAnm() {
    static const char *scrollUpAnmNames[] = {
        "teikei_win_00_scrlBtn_00_In.brlan",
        "teikei_win_00_scrlBtn_00_Out.brlan",
        "teikei_win_00_scrlBtn_00_Psh.brlan",
        "teikei_win_00_scrlBtn_00_CanPsh.brlan",
        "teikei_win_00_scrlBtn_00_NoPsh.brlan",
    };
    static const char *scrollDownAnmNames[] = {
        "teikei_win_00_scrlBtn_01_In.brlan",
        "teikei_win_00_scrlBtn_01_Out.brlan",
        "teikei_win_00_scrlBtn_01_Psh.brlan",
        "teikei_win_00_scrlBtn_01_CanPsh.brlan",
        "teikei_win_00_scrlBtn_01_NoPsh.brlan",
    };
    static const char *sliderAnmNames[] = {
        "teikei_win_00_sldr_In.brlan",
        "teikei_win_00_sldr_Out.brlan",
    };
    mScrollBtnAnm[0].init(&mLytTeikei, &mTeikeiRes, scrollUpAnmNames, 5);
    mScrollBtnAnm[1].init(&mLytTeikei, &mTeikeiRes, scrollDownAnmNames, 5);
    mSliderAnm.init(&mLytTeikei, &mTeikeiRes, sliderAnmNames, 2);
    mSliderAnm.startEnd(SLIDER_ANM_OUT);
}

// .text:0x2E74
void dMenuChat_c::playScrollAnm() {
    int changed = 0;
    for (int i = 0; i < 2; i++) {
        changed |= mScrollBtnAnm[i].play();
    }
    if (changed) {
        onFlag(FLAG_PRESET_LYT);
    }
    if (isFlag(FLAG_SCROLL)) {
        mScrollAnm.play();
        if (mScrollAnm.mFrameCtrl.isStop()) {
            setScrollAnm(1);
            updateRows();
            offFlag(FLAG_SCROLL);
        }
        onFlag(FLAG_PRESET_LYT);
    }
    if (mSliderAnm.play()) {
        onFlag(FLAG_PRESET_LYT);
    }
    if (isFlag(FLAG_PRESET)) {
        mScrollIconAnm.play();
        onFlag(FLAG_PRESET_LYT);
    }
}

// .text:0x2F8C
void dMenuChat_c::setScrollAnm(int dir) {
    static const char *scrollAnmNames[] = {
        "teikei_win_00_scrlDown.brlan",
        "teikei_win_00_scrlUp.brlan",
    };
    mLytTeikei.setAnm(&mScrollAnm, scrollAnmNames[dir], nullptr);
}

// .text:0x2FAC
BOOL dMenuChat_c::startScroll(int dir) {
    if (!canScroll(dir)) {
        return FALSE;
    }
    setScrollAnm(dir);
    onFlag(FLAG_SCROLL);
    if (dir == 0) {
        mScroll--;
        updateRowWords();
        setRowAnms(-1, -1);
        updateRowSelect();
    } else {
        mScroll++;
    }
    updateSliderPos();
    int first;
    int second;
    if (dir == 0) {
        first = ROW_ANM_WHITE;
        second = ROW_ANM_GRAY;
    } else {
        first = ROW_ANM_GRAY;
        second = ROW_ANM_WHITE;
    }
    mRowAnm[1].setNext(first);
    mRowAnm[4].setNext(second);
    playSePageTurn();
    return TRUE;
}

// .text:0x3090
void dMenuChat_c::updateScrollBtns() {
    bool canUp = canScrollUp();
    int anm = SCROLL_BTN_ANM_NO_PUSH;
    if (canUp) {
        anm = SCROLL_BTN_ANM_CAN_PUSH;
    }
    mScrollBtnAnm[0].start(anm);
    mScrollBtnAnm[0].setToEnd();
    bool canDown = canScrollDown();
    anm = SCROLL_BTN_ANM_NO_PUSH;
    if (canDown) {
        anm = SCROLL_BTN_ANM_CAN_PUSH;
    }
    mScrollBtnAnm[1].start(anm);
    mScrollBtnAnm[1].setToEnd();
    onFlag(FLAG_PRESET_LYT);
}

// .text:0x3110
bool dMenuChat_c::canScroll(int dir) {
    if (dir == 0) {
        return canScrollUp();
    }
    return canScrollDown();
}

// .text:0x3120
bool dMenuChat_c::canScrollUp() {
    return mScroll > -1;
}

// .text:0x3140
bool dMenuChat_c::canScrollDown() {
    return mScroll + 4 < mPresetNum;
}

// .text:0x3164
void dMenuChat_c::initSlider() {
    mSliderTop = mLytTeikei.findPane("N_sldrTop")->GetTranslate().y;
    mSliderBottom = mLytTeikei.findPane("N_sldrBottom")->GetTranslate().y;
    mpSlider = mLytTeikei.findPane("N_sldr_all");
}

// .text:0x31D4
void dMenuChat_c::updateSliderPos() {
    mVec3_c pos = mpSlider->GetTranslate();
    int steps = mPresetNum - 3;
    if (steps <= 0) {
        pos.y = mSliderTop;
    } else {
        pos.y = mSliderTop + (f32)(mScroll + 1) *
                                 ((mSliderBottom - mSliderTop) / (f32)steps);
    }
    mpSlider->SetTranslate(pos);
    onFlag(FLAG_PRESET_LYT);
}

// .text:0x32A0
// This later pane-name array is distinct from the tutorial format literal.
static char l_genreButtonName[] = "N_genreBtn_%02d";

void dMenuChat_c::setSelect(int cursor, BOOL select) {
    if (isAction(cursor)) {
        mActionAnm[cursor - CURSOR_ACTION].setNext(select != 0);
    }
    if (cursor == CURSOR_ENTER) {
        mEnterAnm.setNext(select == 0);
    }
    if (cursor == CURSOR_SAY) {
        mSayAnm.setNext(select == 0);
    }
    if (cursor == CURSOR_CHANGE) {
        mChangeAnm.setNext(select == 0);
    }
    if (isRow(cursor)) {
        mRowAnm[cursor - CURSOR_ROW].startEnd(select == 0);
        onFlag(FLAG_PRESET_LYT);
    }
    if (isScrollBtn(cursor)) {
        int btn = cursor - CURSOR_SCROLL_UP;
        if (select) {
            mScrollBtnAnm[btn].setNext(SCROLL_BTN_ANM_IN);
        } else {
            mScrollBtnAnm[btn].setNext(SCROLL_BTN_ANM_OUT);
        }
    }
    if (cursor == CURSOR_SLIDER) {
        mSliderAnm.setNext(select == 0);
    }
    if (isGenre(cursor)) {
        mLytTeikei.setVisible("W_gnrBtn_Fc", select != 0);
        if (select) {
            char name[0x20];
            sprintf(name, l_genreButtonName, cursor - CURSOR_GENRE);
            mLytTeikei.setPosFrom(name, "W_gnrBtn_Fc", 0.0f, 0.0f);
        }
    }
}

// .text:0x34A0
int dMenuChat_c::hitCheck() {
    mVec2_c pos = dObjCursor_c::getPos();
    int tab = getTabAt(&pos);
    if (tab != -1) {
        return tab;
    }
    if (!dObjCursor_c::isVisible()) {
        return -1;
    }
    mActionSlot = hitAction(&pos);
    if (mActionSlot != -1) {
        return mActionSlot + CURSOR_ACTION;
    }
    if (hitSay()) {
        return CURSOR_SAY;
    }
    if (hitChange()) {
        return CURSOR_CHANGE;
    }
    return -1;
}

// .text:0x3550
int dMenuChat_c::hitCheckKbd() {
    int cursor = hitCheck();
    if (cursor != -1) {
        return cursor;
    }
    if (!dObjCursor_c::isVisible()) {
        return -1;
    }
    if (hitKbdEnter()) {
        return CURSOR_ENTER;
    }
    return -1;
}

// .text:0x35B4
int dMenuChat_c::hitCheckPreset() {
    int cursor = hitCheck();
    if (cursor != -1) {
        return cursor;
    }
    if (!dObjCursor_c::isVisible()) {
        return -1;
    }
    mVec2_c cursorPos = dObjCursor_c::getPos();
    mVec2_c pos(cursorPos);
    int hit = m2d::hitButton(&pos, &mLytTeikei, CURSOR_ROW, CURSOR_GENRE_LAST,
                             -1, -1, 1.0f);
    if (hit == CURSOR_SCROLL_UP && !canScrollUp()) {
        hit = -1;
    }
    if (hit == CURSOR_SCROLL_DOWN && !canScrollDown()) {
        hit = -1;
    }
    if (hit == CURSOR_SLIDER && mPresetNum <= 3) {
        hit = -1;
    }
    if (hit != -1) {
        return hit;
    }
    return -1;
}

// .text:0x36AC
BOOL dMenuChat_c::pushCursorKbd() {
    if (isTab(mCursor)) {
        selectTab(mCursor, FALSE);
        reopenKbd();
        return TRUE;
    }
    if (isAction(mCursor)) {
        selectAction(mActionSlot);
        reopenKbd();
        return TRUE;
    }
    if (mCursor == CURSOR_ENTER) {
        reopenKbd();
        sendText(TRUE);
        return TRUE;
    }
    if (mCursor == CURSOR_SAY) {
        reopenKbd();
        sendText(FALSE);
        return TRUE;
    }
    if (mCursor == CURSOR_CHANGE) {
        setSubState(SUB_CHANGE_TO_PRESET);
        mChangeAnm.start(BTN_ANM_PUSH);
        playSeSubMenuPush();
        reopenKbd();
        return TRUE;
    }
    return FALSE;
}

// .text:0x37DC
BOOL dMenuChat_c::isChangePushEnd() {
    if (mCursor == CURSOR_CHANGE) {
        return mChangeAnm.isEnd();
    }
    return TRUE;
}

// .text:0x37F8
void dMenuChat_c::finishChangePush() {
    if (mCursor == CURSOR_CHANGE) {
        setChangeIcon(MODE_PRESET);
        startSwitch();
        updateRows();
    }
}

// .text:0x3844
void dMenuChat_c::padPreset() {
    if (dPad::isUpTriggered() && canScrollUp()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        mCursor = CURSOR_SCROLL_UP;
        pushCursorPreset();
    } else if (dPad::isDownTriggered() && canScrollDown()) {
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, FALSE);
        }
        mCursor = CURSOR_SCROLL_DOWN;
        pushCursorPreset();
    } else if (dPad::isLeftTriggered()) {
        int group = mGroup - 1;
        if (group < 0) {
            group = GENRE_NUM - 1;
        }
        selectGroup(group);
        updateRows();
        updateSliderPos();
        updateScrollBtns();
        playSeDecide();
    } else if (dPad::isRightTriggered()) {
        int group = mGroup + 1;
        if (group >= GENRE_NUM) {
            group = 0;
        }
        selectGroup(group);
        updateRows();
        updateSliderPos();
        updateScrollBtns();
        playSeDecide();
    }
}

// .text:0x3984
BOOL dMenuChat_c::pushCursorPreset() {
    if (isTab(mCursor)) {
        selectTab(mCursor, FALSE);
        return TRUE;
    }
    if (isAction(mCursor)) {
        selectAction(mActionSlot);
        return TRUE;
    }
    if (mCursor == CURSOR_CHANGE) {
        setSubState(SUB_PRESET_PUSH);
        mChangeAnm.start(BTN_ANM_PUSH);
        playSeSubMenuPush();
        return TRUE;
    }
    if (isScrollBtn(mCursor)) {
        int dir = mCursor - CURSOR_SCROLL_UP;
        if (canScroll(dir)) {
            mScrollBtnAnm[dir].setNext(SCROLL_BTN_ANM_PUSH);
            setSubState(SUB_PRESET_PUSH);
            return TRUE;
        }
    }
    if (mCursor == CURSOR_SLIDER) {
        mVec2_c pos = dObjCursor_c::getPos();
        mSliderGrab = mpSlider->GetTranslate().y - pos.y;
        setSubState(SUB_SLIDER);
        playSeAPush();
        return TRUE;
    }
    if (isGenre(mCursor)) {
        setSubState(SUB_PRESET_PUSH);
        int genre = mCursor - CURSOR_GENRE;
        if (genre == mGroup) {
            return FALSE;
        }
        char name[0x20];
        sprintf(name, l_genreButtonName, genre);
        mLytTeikei.setPosFrom(name, "W_gnrBtn_Psh", 0.0f, 0.0f);
        mGenrePushAnm.setFrame(0.0f);
        playSeDecide();
        return TRUE;
    }
    if (isRow(mCursor)) {
        int row = mCursor - CURSOR_ROW;
        if (mGroup == mPresetGroup && mPreset == row + mScroll) {
            return FALSE;
        }
        mRowAnm[row].start(ROW_ANM_PUSH);
        playSeListChoice();
        onFlag(FLAG_PRESET_LYT);
        setSubState(SUB_PRESET_PUSH);
        return TRUE;
    }
    if (mCursor == CURSOR_SAY) {
        if (mPresetGroup == -1 || mPreset == -1) {
            return FALSE;
        }
        mSayAnm.setNext(BTN_ANM_PUSH);
        sendPreset();
        mPreset = -1;
        updateRowSelect();
        return TRUE;
    }
    return FALSE;
}

// .text:0x3C9C
BOOL dMenuChat_c::isPresetPushEnd() {
    if (mCursor == CURSOR_CHANGE) {
        return mChangeAnm.isEnd();
    }
    if (isScrollBtn(mCursor)) {
        return mScrollBtnAnm[mCursor - CURSOR_SCROLL_UP].isEnd();
    }
    if (isGenre(mCursor)) {
        return mGenrePushAnm.mFrameCtrl.isStop();
    }
    if (isRow(mCursor)) {
        return mRowAnm[mCursor - CURSOR_ROW].isEnd();
    }
    return TRUE;
}

// .text:0x3D48
void dMenuChat_c::finishPresetPush() {
    if (mCursor == CURSOR_CHANGE) {
        setChangeIcon(MODE_KEYBOARD);
        startSwitch();
    } else if (isScrollBtn(mCursor)) {
        if (startScroll(mCursor - CURSOR_SCROLL_UP)) {
            setSubState(SUB_SCROLL_HOLD);
        } else {
            restoreSubState();
        }
    } else if (isGenre(mCursor)) {
        selectGroup(mCursor - CURSOR_GENRE);
        updateRows();
        updateSliderPos();
        updateScrollBtns();
        restoreSubState();
    } else if (isRow(mCursor)) {
        int group = mGroup;
        mPreset = mCursor + mScroll - CURSOR_ROW;
        mPresetGroup = group;
        updateRowSelect();
        showPreset();
        restoreSubState();
    } else {
        restoreSubState();
    }
}

// .text:0x3E98
void dMenuChat_c::restoreSubState() {
    setSubState(mMode);
}

// .text:0x3EAC
void dMenuChat_c::setMode(u8 mode) {
    mMode = mode;
    restoreSubState();
}

// .text:0x3EB4
void dMenuChat_c::playAnm() {
    int actionChanged = 0;
    for (int i = 0; i < ACTION_NUM; i++) {
        actionChanged |= mActionAnm[i].play();
    }
    if (actionChanged) {
        onFlag(FLAG_ACTION);
    }
    if (mEnterAnm.play()) {
        onFlag(FLAG_KBD);
    } else {
        if (mEnterAnm.mCurIdx == BTN_ANM_UNFOCUS) {
            mKbd.mOrder = KeyboardMng_c::ORDER_BEFORE;
        }
        if (mEnterAnm.mCurIdx == BTN_ANM_PUSH) {
            mEnterAnm.start(BTN_ANM_FOCUS);
            mEnterAnm.setToEnd();
            onFlag(FLAG_KBD);
        }
    }
    if (mTextAnm.play()) {
        onFlag(FLAG_TEXT);
    }
    if (mSayAlphaAnm.play()) {
        onFlag(FLAG_SAY);
    }
    if (mSayAnm.play()) {
        onFlag(FLAG_SAY);
    }
    if (mChangeAnm.play()) {
        onFlag(FLAG_CHANGE);
    }
    playRowAnm();
    playScrollAnm();
    playGenreAnm();
}

// .text:0x4018
void dMenuChat_c::calcLayouts() {
    if (isFlag(FLAG_ACTION)) {
        offFlag(FLAG_ACTION);
        mLytAction.calc();
    }
    if (isFlag(FLAG_KBD)) {
        offFlag(FLAG_KBD);
        mLytKbd.calc();
    }
    if (isFlag(FLAG_TEXT)) {
        offFlag(FLAG_TEXT);
        mLytText.calc();
    }
    if (isFlag(FLAG_SAY)) {
        offFlag(FLAG_SAY);
        mLytSay.calc();
    }
    if (isFlag(FLAG_CHANGE)) {
        offFlag(FLAG_CHANGE);
        mLytChange.calc();
    }
    if (isFlag(FLAG_PRESET_LYT)) {
        offFlag(FLAG_PRESET_LYT);
        mLytTeikei.calc();
    }
}

// .text:0x4174
void dMenuChat_c::KeyboardMng_c::draw() {
    if (mFlags & FLAG_LAYOUT) {
        mLyt.draw();
    }
    if (mpActionLyt != nullptr) {
        mpActionLyt->draw();
    }
    if (mpChangeLyt != nullptr) {
        mpChangeLyt->draw();
    }
    if (mOrder == ORDER_BEFORE) {
        mpKbdLyt->draw();
    }
    mpManager->draw();
    if (mOrder == ORDER_AFTER) {
        mpKbdLyt->draw();
    }
}

// .text:0x4248
void dMenuChat_c::KeyboardMng_c::setLayers(m2d::Base_c *kbd,
                                           m2d::Base_c *action,
                                           m2d::Base_c *change) {
    mpKbdLyt = kbd;
    mpActionLyt = action;
    mpChangeLyt = change;
}

// Retained by the original link; no surviving code references it.
#pragma force_active on
extern const int l_textSize = dMenuChat_c::TEXT_SIZE;
#pragma force_active reset
