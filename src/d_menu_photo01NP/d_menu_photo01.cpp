#include <game/game/d_menu_photo01.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_photo.hpp>
#include <game/mLib/m_pad.hpp>
#include <string.h>

namespace dMenuPhoto01 {
// .text:0x5C
BASE_PROFILE(MENU_PHOTO01, dMenuPhoto01_c)
} // namespace dMenuPhoto01

static const char *l_allPane = "N_all";

// .text:0x100
int dMenuPhoto01_c::create() {
    dMenuMng_c::onFlag(dMenuMng_c::FLAG_PHOTO);
    if (!mRes.load("Layout/photo.arc")) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "photo_win_00.brlyt");
    mLyt.setVisible("N_cmnBtnA_00", false);
    mLyt.setAnm(&mScrlIconAnm, "photo_win_00_scrlIcon_00.brlan", NULL);
    mLyt.setAnm(&mNoPhotoAnm, "photo_win_00_NoPhoto.brlan", NULL);
    initAnms();
    copyPhoto();
    mLyt.calc();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x20C
int dMenuPhoto01_c::doDelete() {
    return mRes.remove() != 0;
}

// .text:0x23C
int dMenuPhoto01_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x260
int dMenuPhoto01_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x264
int dMenuPhoto01_c::stateClose() {
    if (dPhoto::getMode() == dPhoto::MODE_NONE) {
        deleteRequest();
    }
    return SUCCEEDED;
}

// .text:0x2A4
int dMenuPhoto01_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && prevCursor != mCursor) {
        playSeCursor();
    }
    calcAnms();
    mScrlIconAnm.play();
    mLyt.calc();
    return result;
}

// .text:0x328
int dMenuPhoto01_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
    }
    return SUCCEEDED;
}

// .text:0x36C
void dMenuPhoto01_c::subStateMain() {
    setSelect(mCursor, FALSE);
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_PLUS)) {
        mCursor = CURSOR_USE;
        push();
    } else if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_MINUS) ||
               mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        mCursor = CURSOR_QUIT;
        push();
    } else {
        mCursor = hitCheck();
        if (mCursor != CURSOR_NONE) {
            setSelect(mCursor, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                push();
            }
        }
    }
}

// .text:0x480
void dMenuPhoto01_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x4BC
void dMenuPhoto01_c::executeSubState() {
    typedef void (dMenuPhoto01_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {&dMenuPhoto01_c::subStateMain,
                                          &dMenuPhoto01_c::subStatePush};
    (this->*subStateProcs[mSubState])();
}

// .text:0x4F4
void dMenuPhoto01_c::initAnms() {
    static const char *btnAnmNames[BTN_ANM_NUM] = {
        "photo_win_00_cmnBtnA_out.brlan",
        "photo_win_00_cmnBtnA_in.brlan",
        "photo_win_00_cmnBtnA_psh.brlan",
    };
    for (int i = 0; i < CURSOR_BTN_NUM; i++) {
        mBtnAnm[i].init(&mLyt, &mRes, btnAnmNames, BTN_ANM_NUM);
        mBtnAnm[i].setNumber(i);
        mBtnAnm[i].start(0);
        mBtnAnm[i].setToEnd();
    }
    mLyt.setMessage("T_cmnBtnA_01", MSG_QUIT, NULL);
    mLyt.setMessage("T_cmnBtnA_02", MSG_USE_THIS, NULL);
}

// .text:0x5B4
void dMenuPhoto01_c::calcAnms() {
    int i;
    BOOL playing = FALSE;
    for (i = 0; i < CURSOR_BTN_NUM; i++) {
        playing |= mBtnAnm[i].play();
    }
    if (playing) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x630
void dMenuPhoto01_c::setSelect(int cursor, BOOL select) {
    if (isBtn(cursor)) {
        mBtnAnm[cursor].setNext(select != FALSE);
    }
}

// .text:0x670
int dMenuPhoto01_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    mVec2_c hitPos = pos;
    int hit = m2d::hitButton(&hitPos, &mLyt, CURSOR_BTN_FIRST, CURSOR_BTN_LAST,
                             CURSOR_NONE, CURSOR_NONE, 1.0f);
    if (hit != CURSOR_NONE) {
        return hit;
    }
    return CURSOR_NONE;
}

// .text:0x6F8
void dMenuPhoto01_c::push() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        mBtnAnm[cursor].setNext(BTN_ANM_PUSH);
        if (cursor == CURSOR_QUIT) {
            playSeCancel();
        } else {
            playSeDecide();
        }
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x794
BOOL dMenuPhoto01_c::isPushEnd() {
    if (isBtn(mCursor)) {
        return mBtnAnm[mCursor].isEnd();
    }
    return TRUE;
}

// .text:0x7D0
void dMenuPhoto01_c::finishPush() {
    int cursor = mCursor;
    if (isBtn(cursor)) {
        switch (cursor) {
        case CURSOR_QUIT:
            startCloseAnm();
            break;
        case CURSOR_USE:
            startCloseAnm();
            dMenuMng_c::setResult(RESULT_USE);
            if (dPhoto::isCaptured()) {
                dPhoto::requestEncode(true);
            }
            break;
        }
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x868
void dMenuPhoto01_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(RESULT_QUIT);
}

// .text:0x8A8
BOOL dMenuPhoto01_c::copyPhoto() {
    if (dPhoto::getMode() != dPhoto::MODE_NONE || !dPhoto::isCaptured()) {
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
