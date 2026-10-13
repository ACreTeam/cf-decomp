// The menus' button bar (dMenuBtn_c). .text 800CEC80..800CF77C, .data 804EB520..804EB6E0,
// .sdata 8074AAC0..8074AAF8, .sdata2 80750898. See include/game/game/d_menu_btn.hpp.
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <lib/revolution/WPAD/WPAD.h>
#include <cstdio>

static const char *l_textBoxName = "T_cmnBtnA_";

static const char *l_btnAnmName[] = {
    "cmn_btn_A_00_cmnBtnA_NoPsh.brlan",
    "cmn_btn_A_00_cmnBtnA_CanPsh.brlan",
    "cmn_btn_A_00_cmnBtnA_psh.brlan",
};

static const char *l_inOutAnmName0[] = {
    "cmn_btn_A_00_cmnBtnA_00_out.brlan",
    "cmn_btn_A_00_cmnBtnA_00_in.brlan",
};

static const char *l_inOutAnmName1[] = {
    "cmn_btn_A_00_cmnBtnA_01_out.brlan",
    "cmn_btn_A_00_cmnBtnA_01_in.brlan",
};

static const char *l_inOutAnmName2[] = {
    "cmn_btn_A_00_cmnBtnA_02_out.brlan",
    "cmn_btn_A_00_cmnBtnA_02_in.brlan",
};

dMenuBtn_c::dMenuBtn_c() {}

dMenuBtn_c::~dMenuBtn_c() {}

bool dMenuBtn_c::load() {
    return fn_8019A718() != nullptr;
}

bool dMenuBtn_c::create() {
    int i;
    mFlags = 0;
    mLyt.build(fn_8019A718(), "cmn_btn_A_00.brlyt");
    for (i = 0; i < BTN_NUM; i++) {
        char name[32];
        sprintf(name, "%s%02d", l_textBoxName, i);
        mText[i].setTextBox(mLyt.findTextBox(name));
        mIsCancel[i] = 0;
    }
    mLyt.calc();
    for (i = 0; i < BTN_NUM; i++) {
        mBtnAnm[i].init(&mLyt, fn_8019A718(), l_btnAnmName, 3);
        mBtnAnm[i].setNumber(i);
        mBtnAnm[i].start(BTN_ANM_CAN_PUSH);
        mBtnAnm[i].setToEnd();
    }
    mInOutAnm[0].init(&mLyt, fn_8019A718(), l_inOutAnmName0, 2);
    mInOutAnm[1].init(&mLyt, fn_8019A718(), l_inOutAnmName1, 2);
    mInOutAnm[2].init(&mLyt, fn_8019A718(), l_inOutAnmName2, 2);
    for (i = 0; i < BTN_NUM; i++) {
        mInOutAnm[i].setNext(0);
    }
    mFlags |= FLAG_DRAW;
    mPushedBtn = -1;
    mLyt.mPriority = 0x82;
    mAnmNo = dMenuBase_c::ANM_NUM;
    return true;
}

bool dMenuBtn_c::remove() {
    return true;
}

bool dMenuBtn_c::execute() {
    calcAnm();
    if (mFlags & FLAG_CALC) {
        mFlags &= ~FLAG_CALC;
        mLyt.calc();
    }
    return true;
}

bool dMenuBtn_c::draw() {
    if (mFlags & FLAG_DRAW) {
        mLyt.entry();
    }
    return true;
}

void dMenuBtn_c::setPos(m2d::SimpleEx_c *layout, const char *paneName) {
    layout->copyPos(&mLyt, paneName, "N_all");
    mFlags |= FLAG_CALC;
}

int dMenuBtn_c::hitCheck(const mVec2_c *pos) {
    mVec2_c p = *pos;
    int btn = m2d::hitButton(&p, &mLyt, 0, 2, -1, -1, 1.0f);
    if (btn == -1) {
        return -1;
    }
    return isEnable(btn) ? btn : -1;
}

int dMenuBtn_c::checkPad(BOOL useB, BOOL usePlus) {
    if (useB && mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        if (isEnable(0)) {
            return 0;
        }
        if (usePlus && isEnable(2)) {
            return 2;
        }
    }
    if (isEnable(0) && mPad::g_currentCore->downTrigger(WPAD_BUTTON_MINUS)) {
        return 0;
    }
    if (isEnable(2) && mPad::g_currentCore->downTrigger(WPAD_BUTTON_PLUS)) {
        return 2;
    }
    return -1;
}

void dMenuBtn_c::setSelect(int btn, BOOL select, BOOL playSe) {
    bool changed = mInOutAnm[btn].setNext(select != FALSE);
    if (playSe && changed && select) {
        dMenuBase_c::playSeCursor();
    }
}

void dMenuBtn_c::clearSelect() {
    for (int i = 0; i < BTN_NUM; i++) {
        setSelect(i, FALSE, FALSE);
    }
}

BOOL dMenuBtn_c::push(int btn, BOOL noSe) {
    if (mFlags & FLAG_PUSH) {
        return FALSE;
    }
    mFlags |= FLAG_PUSH;
    mBtnAnm[btn].start(BTN_ANM_PUSH);
    mPushedBtn = btn;
    mFlags |= FLAG_CALC;
    if (!noSe) {
        if (mIsCancel[btn]) {
            dMenuBase_c::playSeCancel();
        } else {
            dMenuBase_c::playSeDecide();
        }
    }
    return TRUE;
}

bool dMenuBtn_c::isPushing() {
    return mFlags & FLAG_PUSH;
}

BOOL dMenuBtn_c::startAnm(int anm) {
    if (mFlags & FLAG_ANM) {
        return FALSE;
    }
    if (anm == mAnmNo) {
        return FALSE;
    }
    mAnmNo = anm;
    mFlags |= FLAG_ANM;
    const char *name = dMenuBase_c::getAnmName(anm);
    mLyt.setAnm(&mAnm, name, fn_8019A6FC());
    return TRUE;
}

bool dMenuBtn_c::isAnm() {
    return mFlags & FLAG_ANM;
}

void dMenuBtn_c::setText(int btn, u16 msgNo, u8 isCancel) {
    m2d::stringWord_c *text = &mText[btn];
    text->clear();
    dString::loadString(text, msgNo, "sys_2D/SYS_2D_menu");
    text->update();
    mFlags |= FLAG_CALC;
    mIsCancel[btn] = isCancel;
    setVisible(btn, true);
}

void dMenuBtn_c::setVisible(int btn, bool visible) {
    char name[32];
    sprintf(name, "N_cmnBtnA_%02d", btn);
    mLyt.setVisible(name, visible);
    sprintf(name, "cmnBtnA_psh_%02d", btn);
    mLyt.setVisible(name, visible);
    u32 flag = FLAG_HIDDEN << btn;
    if (visible) {
        mFlags &= ~flag;
    } else {
        mFlags |= flag;
    }
    mFlags |= FLAG_CALC;
}

void dMenuBtn_c::setDisable(int btn, BOOL disable) {
    u32 flag = FLAG_DISABLED << btn;
    if (disable) {
        mFlags |= flag;
        mBtnAnm[btn].start(BTN_ANM_NO_PUSH);
    } else {
        mFlags &= ~flag;
        mBtnAnm[btn].start(BTN_ANM_CAN_PUSH);
    }
    mBtnAnm[btn].setToEnd();
    mLyt.calc();
}

void dMenuBtn_c::setPriority(u8 priority) {
    mLyt.mPriority = priority;
}

bool dMenuBtn_c::isEnable(int btn) {
    u32 hidden = FLAG_HIDDEN << btn;
    u32 disabled = FLAG_DISABLED << btn;
    return !(mFlags & hidden || mFlags & disabled);
}

void dMenuBtn_c::calcAnm() {
    int i;
    BOOL playing = FALSE;
    for (i = 0; i < BTN_NUM; i++) {
        playing |= mBtnAnm[i].play();
        playing |= mInOutAnm[i].play();
    }
    if (mFlags & FLAG_ANM) {
        if (mAnm.mFrameCtrl.isStop()) {
            mFlags &= ~FLAG_ANM;
        } else {
            mAnm.play();
        }
        playing = TRUE;
    }
    if (playing) {
        mFlags |= FLAG_CALC;
    } else {
        mFlags &= ~FLAG_PUSH;
    }
}
