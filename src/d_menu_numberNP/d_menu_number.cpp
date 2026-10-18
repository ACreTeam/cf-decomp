#include <game/game/d_menu_number.hpp>
#include <cstdio>
#include <game/game/d_letter.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_snd_util.hpp>
// The pending parse of these template inlines selects the literal functions'
// section.
#include <game/snd/snd_obj_inline.hpp>
#include <game/snd/snd_id.hpp>

// .text:0x5C
dMenuNumber_c::dMenuNumber_c() {
    mFlags = 0;
}

// .text:0x148
dMenuNumber_c::~dMenuNumber_c() {
}

// .text:0x7EC
BOOL dMenuNumber_c::load() {
    mKbd.init(dKbd::Mng_c::INIT_MODE_PAD);
    BOOL loaded = mKbd.loadRes();
    return loaded & mRes.load("Layout/number.arc");
}

// .text:0x848
BOOL dMenuNumber_c::create(int mode) {
    if (!mKbd.create()) {
        return FALSE;
    }
    mLyt.build(&mRes, "number_win_00.brlyt");
    initAnms();
    bool backKey = mode != MODE_AMOUNT;
    mLyt.setVisible("N_numBtn_11", backKey);
    mLyt.setVisible("B_numBtn_11", backKey);
    mLyt.mPos.z = -1.0f;
    mLyt.calc();
    mLyt.mCursorKey = KEY_7;
    mLyt.mMode = mode;
    mFlags = 0;
    if (mode != MODE_AMOUNT) {
        mFlags |= FLAG_KBD_EXTRA_KEYS;
    }
    return TRUE;
}

// .text:0x1DC
BOOL dMenuNumber_c::remove() {
    if (mKbd.remove(dKbd::Mng_c::REMOVE_MODE_SAVE_SETTING) == FALSE) {
        return FALSE;
    }
    return mRes.remove();
}

// .text:0x228
int dMenuNumber_c::execute(BOOL holdPush) {
    mKbd.calcClear();
    calcAnms(holdPush);
    if (mFlags & FLAG_CALC) {
        mFlags &= ~FLAG_CALC;
        mLyt.calc();
    }
    return TRUE;
}

// .text:0x29C
int dMenuNumber_c::draw() {
    mLyt.entry();
    return TRUE;
}

// .text:0x934
void dMenuNumber_c::setPos(m2d::SimpleEx_c *base, const char *paneName) {
    base->copyPos(&mLyt, paneName, "N_all");
    mFlags |= FLAG_CALC;
}

// .text:0x2C4
int dMenuNumber_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return KEY_NONE;
    }
    const mVec2_c &cursor = dObjCursor_c::getPos();
    mVec2_c pos = cursor;
    return m2d::hitButton(&pos, &mLyt, 0, KEY_BACK, -1, -1, 1.0f);
}

// .text:0x33C
int dMenuNumber_c::getKbdKey() {
    mFlags &= ~FLAG_KBD_BACK;
    int digit = mKbd.getDigit();
    if (!(mFlags & FLAG_KBD_EXTRA_KEYS) &&
        (digit == dKbd::Listener_c::DIGIT_10 ||
         digit == dKbd::Listener_c::DIGIT_11)) {
        digit = dKbd::Listener_c::DIGIT_RESULT_NONE;
    }
    if (digit != dKbd::Listener_c::DIGIT_RESULT_NONE) {
        if (digit == dKbd::Listener_c::DIGIT_11) {
            mFlags |= FLAG_KBD_BACK;
        }
        mKbd.clearEmotion();
        return getDigitKey(digit);
    }
    return KEY_NONE;
}

// .text:0x3D4
BOOL dMenuNumber_c::isSendReq() {
    if (mKbd.isSendReq()) {
        mKbd.clearEmotion();
        return TRUE;
    }
    return FALSE;
}

// .text:0x420
BOOL dMenuNumber_c::isClearReq() {
    if (mKbd.isClearReq()) {
        mKbd.clearEmotion();
        return TRUE;
    }
    return FALSE;
}

// .text:0x46C
bool dMenuNumber_c::isKbdBack() {
    u16 back = mFlags & FLAG_KBD_BACK;
    return back;
}

// .text:0x484
u8 dMenuNumber_c::getSpecialKey() {
    u8 key = mKbd.getSpecialKey();
    if (key) {
        mKbd.clearEmotion();
    }
    return key;
}

// .text:0x97C
void dMenuNumber_c::setCursor(int key, int anm) {
    mKeyAnms[key].setNext(anm);
    char name[32];
    sprintf(name, "N_numBtn_%02d", key);
    if (anm == KEY_ANM_IN) {
        mLyt.setPosZ(name, 0.5f);
        mLyt.mCursorKey = key;
    } else if (anm == KEY_ANM_OUT) {
        mLyt.setPosZ(name, 0.0f);
    }
}

// .text:0x4D4
void dMenuNumber_c::push(int key) {
    mKeyAnms[key].start(KEY_ANM_PUSH);
    mPushedKey = key;
}

// .text:0x520
void dMenuNumber_c::playKeySe(int key) {
    if (key == KEY_BACK) {
        playSe(SE_keyboard_BackSpace);
    } else if (key == KEY_CLEAR) {
        playSe(SE_Clear);
    } else {
        dLetter::playDigitKeySe(getKeyDigit(key), TRUE, FALSE);
    }
}

// .text:0x578
BOOL dMenuNumber_c::isPushing() {
    return FALSE;
}

// .text:0x580
void dMenuNumber_c::initAnms() {
    static const char *keyAnms[KEY_ANM_NUM] = {
        "number_win_00_numBtn_In.brlan",
        "number_win_00_numBtn_Out.brlan",
        "number_win_00_numBtn_Psh.brlan",
    };
    static const char *redKeyAnms[KEY_ANM_NUM] = {
        "number_win_00_numBtn_red_In.brlan",
        "number_win_00_numBtn_red_Out.brlan",
        "number_win_00_numBtn_red_Psh.brlan",
    };
    for (int i = 0; i < KEY_NUM; i++) {
        if (i == KEY_CLEAR || i == KEY_BACK) {
            mKeyAnms[i].init(&mLyt, &mRes, redKeyAnms, KEY_ANM_NUM);
            mKeyAnms[i].setNumber(i);
        } else {
            mKeyAnms[i].init(&mLyt, &mRes, keyAnms, KEY_ANM_NUM);
            mKeyAnms[i].setNumber(i);
        }
    }
}

// .text:0x628
void dMenuNumber_c::calcAnms(BOOL holdPush) {
    int i;
    int playing = 0;
    for (i = 0; i < KEY_NUM; i++) {
        m2d::AnmEx2_c &anm = mKeyAnms[i];
        playing |= anm.play();
        if (!holdPush && anm.mFrameCtrl.isStop() &&
            mKeyAnms[i].mCurIdx == KEY_ANM_PUSH) {
            anm.start(KEY_ANM_IN);
            anm.setToEnd();
            anm.setNext(KEY_ANM_OUT);
        }
    }
    if (playing) {
        mFlags |= FLAG_CALC;
    }
}

// .text:0x6E4
int dMenuNumber_c::getKeyDigit(int key) {
    static const int digits[KEY_NUM] = {7, 8, 9, 4, 5,           6,
                                        1, 2, 3, 0, DIGIT_CLEAR, DIGIT_BACK};
    return digits[key];
}

// .text:0x6F8
int dMenuNumber_c::getDigitKey(int digit) {
    static const int keys[] = {KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,    KEY_5,
                               KEY_6, KEY_7, KEY_8, KEY_9, KEY_BACK, KEY_BACK};
    return keys[digit];
}

// .text:0x70C
void dMenuNumber_c::layout_c::draw() {
    static const char *panes[PANE_NUM] = {
        "N_numBtn_00",   "N_numBtn_01", "N_numBtn_02", "N_numBtn_03",
        "N_numBtn_04",   "N_numBtn_05", "N_numBtn_06", "N_numBtn_07",
        "N_numBtn_08",   "N_numBtn_09", "N_numBtn_10", "N_numBtn_11",
        "numWinBase_00",
    };
    setVisible(panes[mCursorKey], false);
    m2d::Simple_c::draw();
    for (int i = 0; i < PANE_NUM; i++) {
        setVisible(panes[i], mCursorKey == i);
    }
    m2d::Simple_c::draw();
    for (int i = 0; i < PANE_NUM; i++) {
        setVisible(panes[i], true);
    }
    if (mMode == MODE_AMOUNT) {
        setVisible("N_numBtn_11", false);
    }
}
