#include <game/game/d_menu_lcrsr.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_mng.hpp>

static const char *l_allPane = "N_all";

// .text:0x5C
dMenuLcrsr_c::dMenuLcrsr_c() {
}

// .text:0x124
dMenuLcrsr_c::~dMenuLcrsr_c() {
}

// .text:0x1E4
BOOL dMenuLcrsr_c::load() {
    BOOL loaded = mRes.load("Layout/paper.arc");
    return loaded & (dMenuMng_c::getWinResAcc() != nullptr);
}

// .text:0x230
BOOL dMenuLcrsr_c::create() {
    static const char *focusAnmNames[FOCUS_ANM_NUM] = {
        "ltr_cursor_00_crsrFc_Out.brlan",
        "ltr_cursor_00_crsrFc_In.brlan",
    };
    static const char *btnAnmNames0[BTN_ANM_NUM + 1] = {
        "ltr_cursor_00_crsr_00_Psh.brlan",
        "ltr_cursor_00_crsr_00_noPsh.brlan",
        "ltr_cursor_00_crsr_00_canPsh.brlan",
    };
    static const char *btnAnmNames1[BTN_ANM_NUM] = {
        "ltr_cursor_00_crsr_01_Psh.brlan",
        "ltr_cursor_00_crsr_01_noPsh.brlan",
        "ltr_cursor_00_crsr_01_canPsh.brlan",
    };
    mFlags = 0;
    mLyt.build(&mRes, "ltr_cursor_00.brlyt");
    mLyt.calc();
    for (int i = 0; i < BTN_NUM; i++) {
        mFocusAnm[i].init(&mLyt, &mRes, focusAnmNames, FOCUS_ANM_NUM);
        mFocusAnm[i].setNumber(i);
    }
    mBtnAnm[0].init(&mLyt, &mRes, btnAnmNames0, BTN_ANM_NUM);
    mBtnAnm[1].init(&mLyt, &mRes, btnAnmNames1, BTN_ANM_NUM);
    mFlags |= FLAG_DRAW;
    mPushedBtn = BTN_NONE;
    return TRUE;
}

// .text:0x328
BOOL dMenuLcrsr_c::remove() {
    return mRes.remove();
}

// .text:0x330
BOOL dMenuLcrsr_c::execute() {
    calcAnm();
    if (mFlags & FLAG_CALC) {
        mFlags &= ~FLAG_CALC;
        mLyt.calc();
    }
    return TRUE;
}

// .text:0x388
BOOL dMenuLcrsr_c::draw() {
    if (mFlags & FLAG_DRAW) {
        mLyt.entry();
    }
    return TRUE;
}

// .text:0x3BC
void dMenuLcrsr_c::setPos(m2d::SimpleEx_c *layout, const char *paneName) {
    layout->copyPos(&mLyt, paneName, l_allPane);
    mFlags |= FLAG_CALC;
}

// .text:0x404
int dMenuLcrsr_c::hitCheck(const mVec2_c *pos) {
    mVec2_c p = *pos;
    int btn = m2d::hitButton(&p, &mLyt, 0, BTN_NUM - 1, -1, -1, 1.0f);
    if (btn == BTN_NONE) {
        return BTN_NONE;
    }
    u32 hidden = FLAG_HIDDEN << btn;
    u32 disabled = FLAG_DISABLED << btn;
    if (mFlags & hidden || mFlags & disabled) {
        return BTN_NONE;
    }
    return btn;
}

// .text:0x498
void dMenuLcrsr_c::setSelect(int btn, BOOL select) {
    mFocusAnm[btn].setNext(select != FALSE);
}

// .text:0x4B4
BOOL dMenuLcrsr_c::push(int btn) {
    if (mFlags & FLAG_PUSH) {
        return FALSE;
    }
    mFlags |= FLAG_PUSH;
    mBtnAnm[btn].start(BTN_ANM_PUSH);
    mPushedBtn = btn;
    return TRUE;
}

// .text:0x520
bool dMenuLcrsr_c::isPushing() {
    return mFlags & FLAG_PUSH;
}

// .text:0x52C
BOOL dMenuLcrsr_c::startAnm(int anm) {
    if (mFlags & FLAG_ANM) {
        return FALSE;
    }
    mFlags |= FLAG_ANM;
    const char *name = dMenuBase_c::getAnmName(anm);
    mLyt.setAnm(&mAnm, name, dMenuMng_c::getWinResAcc());
    return TRUE;
}

// .text:0x5A0
bool dMenuLcrsr_c::isAnm() {
    return mFlags & FLAG_ANM;
}

// .text:0x5AC
mVec3_c dMenuLcrsr_c::getPos() {
    mVec3_c pos;
    m2d::getScreenPos(&pos, mLyt.findPane(l_allPane));
    return pos;
}

// .text:0x5F0
void dMenuLcrsr_c::calcAnm() {
    int i;
    BOOL playing = FALSE;
    for (i = 0; i < BTN_NUM; i++) {
        playing |= mBtnAnm[i].play();
    }
    if (mFlags & FLAG_PUSH && !playing) {
        mFlags &= ~FLAG_PUSH;
    }
    for (i = 0; i < BTN_NUM; i++) {
        playing |= mFocusAnm[i].play();
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
    }
}

// .text:0x6E4
void dMenuLcrsr_c::setDisable(int btn, BOOL disable) {
    u32 flag = FLAG_DISABLED << btn;
    if (disable) {
        mFlags |= flag;
        mBtnAnm[btn].startEnd(BTN_ANM_NO_PUSH);
    } else {
        mFlags &= ~flag;
        mBtnAnm[btn].startEnd(BTN_ANM_CAN_PUSH);
    }
    mLyt.calc();
}
