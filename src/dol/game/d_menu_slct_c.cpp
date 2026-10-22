#include <game/game/d_menu_slct_c.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/wpad.h>
#include <cstdio>

static char l_lytName[] = "cmn_dialog_C_00.brlyt";
static char l_winAnmName0[] = "cmn_dialog_C_00_dialgIn.brlan";
static char l_pushAnmName[] = "cmn_dialog_C_00_dialgBtnPsh.brlan";
static char l_btnTextName[20] = "T_dialgBtn_%02d";
static char l_msgTextName[] = "T_dialg_00";

dMenuSlctC_c::dMenuSlctC_c() {}

dMenuSlctC_c::~dMenuSlctC_c() {}

bool dMenuSlctC_c::load() {
    return dMenuMng_c::getBtnResAcc() != nullptr;
}

bool dMenuSlctC_c::remove() {
    return true;
}

bool dMenuSlctC_c::execute() {
    calcAnm();
    if (mFlags & FLAG_CALC) {
        mFlags &= ~FLAG_CALC;
        mLyt.calc();
    }
    return true;
}

bool dMenuSlctC_c::draw() {
    if (mFlags & FLAG_DRAW) {
        mLyt.entry();
    }
    return true;
}

void dMenuSlctC_c::open(int anm) {
    static const char *l_winAnmName[] = {
        "cmn_dialog_C_00_dialgIn.brlan",
        "cmn_dialog_C_00_dialgOut.brlan",
    };
    mLyt.changeAnm(&mWinAnm, l_winAnmName[anm], nullptr);
    mWinAnm.setFrame(0.0f);
    mWinAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mFlags |= FLAG_WIN_ANM;
    clearSelect();
    switch (anm) {
    case WIN_ANM_IN:
        dMenuBase_c::playSeSelectWindowOpen();
        break;
    }
}

bool dMenuSlctC_c::isWinAnmEnd() {
    return !(mFlags & FLAG_WIN_ANM);
}

int dMenuSlctC_c::hitCheck(const mVec2_c *pos) {
    mVec2_c p = *pos;
    return m2d::hitButton(&p, &mLyt, 0, 1, -1, -1, 1.0f);
}

void dMenuSlctC_c::setSelect(int btn, BOOL select) {
    if (mBtnAnm[btn].setNext(select == FALSE) && select) {
        dMenuBase_c::playSeCursor();
    }
}

void dMenuSlctC_c::clearSelect() {
    for (int i = 0; i < BTN_NUM; i++) {
        setSelect(i, FALSE);
    }
}

static char l_btnPaneName[20] = "N_dialgBtn_%02d";

void dMenuSlctC_c::push(int btn) {
    mFlags |= FLAG_PUSH;
    char name[40];
    sprintf(name, l_btnPaneName, btn);
    mLyt.setPosFrom(name, "dialgBtn_psh", 0.0f, 0.0f);
    mPushAnm.setFrame(0.0f);
    mPushedBtn = btn;
    mFlags |= FLAG_CALC;
    if (mBtnSe[btn] != 0) {
        playSe(mBtnSe[btn]);
    } else {
        dMenuBase_c::playSeDecide();
    }
}

bool dMenuSlctC_c::isPushEnd() {
    return !(mFlags & FLAG_PUSH);
}

BOOL dMenuSlctC_c::checkInput() {
    clearSelect();
    if (mBtnEnable[0] && (mPad::g_currentCore->downTrigger(WPAD_BUTTON_MINUS) || mPad::g_currentCore->downTrigger(WPAD_BUTTON_B))) {
        push(0);
        return TRUE;
    }
    if (mBtnEnable[1] && mPad::g_currentCore->downTrigger(WPAD_BUTTON_PLUS)) {
        push(1);
        return TRUE;
    }
    if (!dObjCursor_c::isVisible()) {
        return FALSE;
    }
    mVec2_c cursor = dObjCursor_c::getPos();
    mVec2_c pos = cursor;
    int btn = hitCheck(&pos);
    if (btn != -1) {
        setSelect(btn, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push(btn);
            return TRUE;
        }
    }
    return FALSE;
}

void dMenuSlctC_c::initBtnAnm() {
    static const char *l_btn0AnmName[] = {
        "cmn_dialog_C_00_dialgBtn_00_in.brlan",
        "cmn_dialog_C_00_dialgBtn_00_out.brlan",
    };
    static const char *l_btn1AnmName[] = {
        "cmn_dialog_C_00_dialgBtn_01_in.brlan",
        "cmn_dialog_C_00_dialgBtn_01_out.brlan",
    };
    mBtnAnm[0].init(&mLyt, dMenuMng_c::getBtnResAcc(), l_btn0AnmName, 2);
    mBtnAnm[1].init(&mLyt, dMenuMng_c::getBtnResAcc(), l_btn1AnmName, 2);
}

void dMenuSlctC_c::calcAnm() {
    int i;
    BOOL playing = FALSE;
    for (i = 0; i < BTN_NUM; i++) {
        playing |= mBtnAnm[i].play();
    }
    if (mFlags & FLAG_PUSH) {
        if (mPushAnm.mFrameCtrl.isStop()) {
            mFlags &= ~FLAG_PUSH;
        } else {
            mPushAnm.play();
            playing = TRUE;
        }
    }
    if (mFlags & FLAG_WIN_ANM) {
        if (mWinAnm.mFrameCtrl.isStop()) {
            mFlags &= ~FLAG_WIN_ANM;
        } else {
            mWinAnm.play();
        }
        playing = TRUE;
    }
    if (playing) {
        mFlags |= FLAG_CALC;
    }
}

void dMenuSlctC_c::setVisible(BOOL visible) {
    mLyt.setVisible("N_dialg_all", visible != FALSE);
}

void dMenuSlctC_c::setMsg(u16 msgNo) {
    setVisible(TRUE);
    dString::loadString(&mMsg, msgNo, "sys_2D/SYS_2D_menu");
    mMsg.update();
    mFlags |= FLAG_CALC;
}

void dMenuSlctC_c::setBtnVisible(int btn, bool visible) {
    char name[40];
    sprintf(name, l_btnPaneName, btn);
    mLyt.setVisible(name, visible);
    mBtnEnable[btn] = visible;
}

void dMenuSlctC_c::setBtn(int btn, u16 msgNo, int se) {
    setBtnVisible(btn, true);
    m2d::stringWord_c *text = &mBtnText[btn];
    text->clear();
    dString::loadString(text, msgNo, "sys_2D/SYS_2D_menu");
    text->update();
    mFlags |= FLAG_CALC;
    mBtnSe[btn] = se;
}

bool dMenuSlctC_c::create() {
    int i;
    mFlags = 0;
    mLyt.build(dMenuMng_c::getBtnResAcc(), l_lytName);
    mLyt.setAnm(&mWinAnm, l_winAnmName0, nullptr);
    mLyt.setAnm(&mPushAnm, l_pushAnmName, nullptr);
    for (i = 0; i < BTN_NUM; i++) {
        char name[40];
        sprintf(name, l_btnTextName, i);
        mBtnText[i].setTextBox(mLyt.findTextBox(name));
        mBtnSe[i] = 0;
        mBtnEnable[i] = 1;
    }
    mLyt.mPriority = 0x87;
    mLyt.calc();
    mFlags |= FLAG_DRAW;
    mPushedBtn = -1;
    mMsg.setTextBox(mLyt.findTextBox(l_msgTextName));
    initBtnAnm();
    return true;
}
