#include <game/framework/f_profile_name.hpp>
#include <game/game/d_obj_keyboard.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_chat.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_color.hpp>
#include <string.h>

CUSTOM_BASE_PROFILE(OBJ_KEYBOARD, dObjKeyboard_c, fProf::OBJ_KEYBOARD,
                    fProf::DRAW_ORDER::OBJ_KEYBOARD);

static u16 l_flags;

int dObjKeyboard_c::create() {
    l_flags = 0;
    bool loaded = mRes.load("Layout/keyboard_free.arc");
    if (!(loaded & mCur.load())) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "textBox_free_00.brlyt");
    mCapsLyt.build(&mRes, "caps_00.brlyt");
    mCapsLyt.setAnm(&mNotJpAnm, "caps_00_notJP_00.brlan", nullptr);
    mNotJpAnm.setToEnd();
    initAnm();
    mWord.setTextBox(mLyt.findTextBox("T_textBox_00"));
    memset(mText, 0, sizeof(mText));
    mLyt.calc();
    mCapsLyt.calc();
    mCur.build();
    mState = STATE_WAIT;
    return SUCCEEDED;
}

int dObjKeyboard_c::doDelete() {
    if (!mCur.remove()) {
        return NOT_READY;
    }
    if (!mKbd.remove(1)) {
        return NOT_READY;
    }
    if (!mRes.remove()) {
        return NOT_READY;
    }
    l_flags = 0;
    return SUCCEEDED;
}

int dObjKeyboard_c::execute() {
    static void (dObjKeyboard_c::*l_stateFuncs[])() = {
        &dObjKeyboard_c::stateWait,         &dObjKeyboard_c::stateOpen,
        &dObjKeyboard_c::stateInput,        &dObjKeyboard_c::stateClose,
        &dObjKeyboard_c::stateEmotionStart, &dObjKeyboard_c::stateEmotion,
        &dObjKeyboard_c::stateEmotionEnd,
    };
    l_flags &= ~FLAG_KBD_UPDATED;
    (this->*l_stateFuncs[mState])();
    playAnm();
    if (l_flags & FLAG_CALC) {
        l_flags &= ~FLAG_CALC;
        mLyt.calc();
        mCapsLyt.calc();
    }
    return SUCCEEDED;
}

int dObjKeyboard_c::draw() {
    mLyt.entry();
    mCapsLyt.entry();
    if (mTimer > 0) {
        mCur.draw();
    }
    return SUCCEEDED;
}

void dObjKeyboard_c::stateWait() {
    if (canOpen()) {
        mState = STATE_OPEN;
        l_flags |= FLAG_OPEN;
    }
}

void dObjKeyboard_c::stateOpen() {
    l_flags |= FLAG_OPEN;
    mKbd.init(1);
    if (mKbd.loadRes() && mKbd.create()) {
        mKbd.setMaxLength(TEXT_LEN + 1);
        mState = STATE_INPUT;
    }
}

void dObjKeyboard_c::stateInput() {
    if (!canOpen()) {
        mTimer = 0;
        mState = STATE_CLOSE;
        return;
    }
    if (dDemo::isType(dDemo::TYPE_UITAG)) {
        mKbd.reopen();
    } else {
        mKbd.open(1, 1);
    }
    if (mKbd.isClearReq()) {
        clearText();
        return;
    }
    if (send()) {
        return;
    }
    int slot = mKbd.getEmotion();
    if (slot != -1) {
        mEmotionSlot = slot;
        mEmotion = getEmotion(slot);
        mKbd.clearEmotion();
        if (mEmotion != 0xFF) {
            dDemo::requestKeyboard();
            mState = STATE_EMOTION_START;
            mKbd.calc();
            if (mKbd.isUpdated()) {
                l_flags |= FLAG_KBD_UPDATED;
            }
            return;
        }
    }
    if (mKbd.isTextChanged()) {
        dAcUitag::hideCameraMsg();
        resetTimer();
        if (checkLength()) {
            mKbd.updateInput();
            updateText();
        } else {
            dKbd::playSeCanNotInput();
            restoreText();
        }
    }
    mKbd.calc();
    if (mKbd.isUpdated()) {
        l_flags |= FLAG_KBD_UPDATED;
    }
    mKbd.postCalc();
}

void dObjKeyboard_c::stateClose() {
    if (mKbd.remove(1)) {
        mState = STATE_WAIT;
        l_flags &= ~FLAG_OPEN;
    }
}

void dObjKeyboard_c::stateEmotionStart() {
    if (dDemo::isActiveKeyboard()) {
        dAcUitag::startEmotion(mEmotionSlot);
        mState = STATE_EMOTION;
        stateEmotion();
    } else {
        mState = STATE_INPUT;
    }
    if (mKbd.isUpdated()) {
        l_flags |= FLAG_KBD_UPDATED;
    }
}

void dObjKeyboard_c::stateEmotion() {
    if (dPlayerMgr::playEmotion(mEmotion)) {
        mState = STATE_EMOTION_END;
    }
    if (mKbd.isUpdated()) {
        l_flags |= FLAG_KBD_UPDATED;
    }
}

void dObjKeyboard_c::stateEmotionEnd() {
    if (dPlayerMgr::isStartingDemo(4)) {
        dDemo::finishKeyboard();
        mState = STATE_INPUT;
    }
    if (mKbd.isUpdated()) {
        l_flags |= FLAG_KBD_UPDATED;
    }
}

BOOL dObjKeyboard_c::canOpen() {
    if (dDemo::isType(dDemo::TYPE_ANY) && !dDemo::isType(dDemo::TYPE_UITAG)) {
        return FALSE;
    }
    if (!dMenuMng_c::isFree()) {
        return FALSE;
    }
    return TRUE;
}

BOOL dObjKeyboard_c::checkLength() {
    const wchar_t *text = mKbd.getText();
    const wchar_t *input = mKbd.getInputText();
    int textLen = m2d::Word_c::getLength(text);
    int inputLen = m2d::Word_c::getLength(input);
    if (textLen + inputLen >= TEXT_LEN) {
        return FALSE;
    }
    wchar_t buf[TEXT_LEN + 1];
    memset(buf, 0, sizeof(buf));
    if (textLen != 0) {
        memcpy(buf, text, textLen * sizeof(wchar_t));
    }
    if (inputLen != 0) {
        memcpy(buf + textLen, input, inputLen * sizeof(wchar_t));
    }
    if (mWord.calcWidth(buf, TEXT_LEN + 1) >= 402.0f) {
        return FALSE;
    }
    memcpy(mText, text, TEXT_LEN * sizeof(wchar_t));
    return TRUE;
}

void dObjKeyboard_c::updateText() {
    static mColor l_inputColor(0x18, 0xF7, 0xFF, 0xFF);
    const wchar_t *text = mKbd.getText();
    const wchar_t *input = mKbd.getInputText();
    int inputLen = m2d::Word_c::getLength(input);
    int inputPos = mKbd.getInputPos();
    mWord.clear();
    mWord.set(text, 0);
    if (inputLen > 0) {
        mWord.insertColored(input, inputPos, &l_inputColor.r, nullptr);
    }
    mWord.update();
    l_flags |= FLAG_CALC;
    setCursor();
}

void dObjKeyboard_c::restoreText() {
    mKbd.fixInput();
    mKbd.setText(mText);
    mKbd.resetCursor();
    updateText();
}

void dObjKeyboard_c::clearText() {
    mTimer = 0;
    memset(mText, 0, sizeof(mText));
    mKbd.setCursorPos(0);
    restoreText();
    mKbd.clearEmotion();
}

BOOL dObjKeyboard_c::send() {
    if (!mKbd.isSendReq()) {
        return FALSE;
    }
    mKbd.fixInput();
    memcpy(mText, mKbd.getText(), TEXT_LEN * sizeof(wchar_t));
    if (mText[0] == 0) {
        return FALSE;
    }
    dObjChat::postText(mText, dObjChat::TEXT_KIND_PLAYER);
    clearText();
    dKbd::playSeSubmit();
    return TRUE;
}

void dObjKeyboard_c::initAnm() {
    static const char *l_boxAnmName[] = {
        "textBox_free_00_boxAlpIn.brlan",
        "textBox_free_00_boxAlpOut.brlan",
    };
    static const char *l_capsAnmName[] = {
        "caps_00_AlpIn.brlan",
        "caps_00_AlpOut.brlan",
    };
    mBoxAnm.init(&mLyt, &mRes, l_boxAnmName, 2);
    mBoxAnm.start(ANM_OUT);
    mBoxAnm.setToEnd();
    mTimer = 0;
    mLyt.setAnm(&mDownAnm, "textBox_free_00_boxDown.brlan", nullptr);
    mDownAnm.setToEnd();
    mCapsAnm.init(&mCapsLyt, &mRes, l_capsAnmName, 2);
    mCapsAnm.start(ANM_OUT);
    mCapsAnm.setToEnd();
}

void dObjKeyboard_c::resetTimer() {
    mTimer = 1800;
}

void dObjKeyboard_c::playAnm() {
    if (mTimer > 0) {
        mTimer--;
        mBoxAnm.setNext(ANM_IN);
    } else {
        mBoxAnm.setNext(ANM_OUT);
    }
    if ((l_flags & FLAG_KBD_UPDATED) && mTimer > 0) {
        mCapsAnm.setNext(ANM_IN);
    } else {
        mCapsAnm.setNext(ANM_OUT);
    }
    bool box = mBoxAnm.play();
    bool caps = mCapsAnm.play();
    if (box | caps) {
        l_flags |= FLAG_CALC;
    }
}

bool dObjKeyboard_c::isOpen() {
    u16 open = l_flags & FLAG_OPEN;
    return open;
}

dObjKeyboard_c::Word_c::Word_c() {
    clear();
}

dObjKeyboard_c::Word_c::~Word_c() {
}

u32 dObjKeyboard_c::Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

wchar_t *dObjKeyboard_c::Word_c::getBuffer() {
    return mBuffer;
}

void dObjKeyboard_c::setCursor() {
    mVec3_c pos;
    m2d::getScreenPos(&pos, mWord.mpTextBox);
    pos.x = mWord.getScreenX(mKbd.getCursorPos());
    mCur.setPos(&pos);
}

u8 dObjKeyboard_c::getEmotion(int slot) {
    dPrivateData_c *data = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (data->mEmotions.isValid(slot) == FALSE) {
        return 0xFF;
    }
    return data->mEmotions.get(slot);
}

dObjKeyboard_c::~dObjKeyboard_c() {
}
