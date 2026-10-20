#include <game/game/d_menu_kabu.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_kbd.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuKabu {
void *classInit();
fProf::fBaseProfile_c g_profile_MENU_KABU = {classInit, fProf::MENU_KABU,
                                             fProf::DRAW_ORDER::MENU_KABU};

// .text:0x5C
void *classInit() {
    return new dMenuKabu_c;
}
} // namespace dMenuKabu

static const char *l_allPanes[] = {"N_all", nullptr};

// .text:0xF0
int dMenuKabu_c::create() {
    u8 menu;
    BOOL resLoaded = mRes.load("Layout/kabu.arc");
    BOOL loaded = resLoaded & mBtn.load();
    loaded &= mNumber.load();
    if (!loaded) {
        return NOT_READY;
    }
    if (!mNumber.create(dMenuNumber_c::MODE_AMOUNT)) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "common_unit_win_00.brlyt");
    initLayout();
    mLyt.calc();
    mBtn.create();
    initButtons();
    setConfirmDisable(FALSE);
    onFlag(FLAG_DRAW);
    startWinAnm(WIN_ANM_IN);
    startAnm(&mNumber.mLyt, l_allPanes[0], ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    menu = mMenuNo;
    mValue = 0;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    mMin = 1;
    switch (menu) {
    case dMenuMng_c::MENU_TURNIP_COUNT:
        mMax = MAX_COUNT;
        mDigits = 2;
        mFormat = FORMAT_COUNT;
        break;
    case dMenuMng_c::MENU_TURNIP_PRICE:
        mMax = MAX_PRICE;
        mDigits = 7;
        mFormat = FORMAT_BELLS;
        break;
    case dMenuMng_c::MENU_DONATION:
        mMax = player->getPocketMoney();
        mDigits = 7;
        mFormat = FORMAT_BELLS;
        break;
    case dMenuMng_c::MENU_AUCTION_MIN:
    case dMenuMng_c::MENU_AUCTION_BID:
        mMax = MAX_AUCTION;
        mDigits = 5;
        mFormat = FORMAT_AUCTION;
        if (menu == dMenuMng_c::MENU_AUCTION_BID) {
            mMin = dMenuMng_c::getNumberMin();
        }
        break;
    }
    mLyt.setVisible("N_input_00", mFormat == FORMAT_COUNT);
    mLyt.setVisible("N_input_01", mFormat == FORMAT_BELLS);
    mLyt.setVisible("N_input_02", mFormat == FORMAT_AUCTION);
    updateDisplay();
    mCursor = CURSOR_NONE;
    return SUCCEEDED;
}

// .text:0x328
int dMenuKabu_c::doDelete() {
    if (!mNumber.remove()) {
        return NOT_READY;
    }
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    bool removed = mRes.remove();
    return removed;
}

// .text:0x394
int dMenuKabu_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x3B8
int dMenuKabu_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x3BC
int dMenuKabu_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x3E0
int dMenuKabu_c::execute() {
    bool anm = isState(STATE_ANM);
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prev && !isFlag(FLAG_NO_SE)) {
        playSeCursor();
    }
    offFlag(FLAG_NO_SE);
    mNumber.execute(FALSE);
    if (anm) {
        mBtn.setPos(&mNumber.mLyt, l_allPanes[0]);
    }
    mBtn.execute();
    calcWinAnm();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    return result;
}

// .text:0x4E4
int dMenuKabu_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mNumber.draw();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x538
void dMenuKabu_c::subStateMain() {
    onFlag(FLAG_NO_SE);
    setSelect(mCursor, FALSE);
    int key = mNumber.getKbdKey();
    if (key != dMenuNumber_c::KEY_NONE) {
        mCursor = key;
        setSelect(key, TRUE);
        pushCursor();
        return;
    }
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        int cursor = btn + CURSOR_BTN;
        mCursor = cursor;
        setSelect(cursor, TRUE);
        pushCursor();
        return;
    }
    if (mBtn.isEnable(BTN_CONFIRM) && mNumber.isSendReq()) {
        mCursor = CURSOR_BTN + BTN_CONFIRM;
        setSelect(CURSOR_BTN + BTN_CONFIRM, TRUE);
        pushCursor();
        return;
    }
    if (mBtn.isEnable(BTN_CANCEL) && mNumber.isClearReq()) {
        mCursor = CURSOR_BTN + BTN_CANCEL;
        setSelect(CURSOR_BTN + BTN_CANCEL, TRUE);
        pushCursor();
        return;
    }
    offFlag(FLAG_NO_SE);
    int cursor = hitCheck();
    mCursor = cursor;
    if (cursor != CURSOR_NONE) {
        setSelect(cursor, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            pushCursor();
        }
    }
}

// .text:0x6CC
void dMenuKabu_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x708
void dMenuKabu_c::executeSubState() {
    typedef void (dMenuKabu_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {&dMenuKabu_c::subStateMain,
                                          &dMenuKabu_c::subStatePush};
    (this->*subStateProcs[mSubState])();
}

// .text:0x740
void dMenuKabu_c::setSelect(int cursor, BOOL select) {
    if (isButton(cursor)) {
        mBtn.setSelect(cursor - CURSOR_BTN, select, FALSE);
    }
    if (isKey(cursor)) {
        mNumber.setCursor(cursor, !select);
    }
}

// .text:0x7E8
int dMenuKabu_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn + CURSOR_BTN;
    }
    int key = mNumber.hitCheck();
    if (key != dMenuNumber_c::KEY_NONE) {
        return key;
    }
    return CURSOR_NONE;
}

// .text:0x860
void dMenuKabu_c::pushCursor() {
    int cursor = mCursor;
    if (isKey(cursor)) {
        mNumber.push(cursor);
        if (inputDigit(dMenuNumber_c::getKeyDigit(cursor))) {
            mNumber.playKeySe(cursor);
            updateDisplay();
        } else {
            dKbd::playSeCanNotInput();
        }
    } else if (isButton(cursor)) {
        mBtn.push(cursor - CURSOR_BTN, FALSE);
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x94C
BOOL dMenuKabu_c::isPushEnd() {
    int cursor = mCursor;
    if (isKey(cursor)) {
        return !mNumber.isPushing();
    }
    if (isButton(cursor)) {
        return !mBtn.isPushing();
    }
    return TRUE;
}

// .text:0x9D8
void dMenuKabu_c::finishPush() {
    int cursor = mCursor;
    if (isButton(cursor)) {
        switch (cursor - CURSOR_BTN) {
        case BTN_CANCEL:
            startCloseAnm();
            break;
        case BTN_UNUSED:
            break;
        case BTN_CONFIRM:
            startCloseAnm();
            dMenuMng_c::setResult(RESULT_CONFIRM);
            switch (mMenuNo) {
            case dMenuMng_c::MENU_DONATION:
                dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(mValue, FALSE);
                dSaveTown_c *town = dSaveData_c::getTown();
                int total = mValue + town->mPublicWorksBells;
                if (total >= PUBLIC_WORKS_MAX) {
                    total = PUBLIC_WORKS_MAX;
                }
                dSaveData_c::getTown()->mPublicWorksBells = total;
                break;
            }
            dMenuMng_c::setNumber(mValue);
            break;
        }
        return;
    } else if (isKey(cursor)) {
        setSubState(SUB_MAIN);
        return;
    }
    setSubState(SUB_MAIN);
}

// .text:0xB24
void dMenuKabu_c::startCloseAnm() {
    startWinAnm(WIN_ANM_OUT);
    startAnm(&mNumber.mLyt, l_allPanes[0], ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(RESULT_CANCEL);
}

// .text:0xB7C
void dMenuKabu_c::initButtons() {
    mBtn.setText(BTN_CANCEL, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
}

// .text:0xBDC
void dMenuKabu_c::setConfirmDisable(BOOL disable) {
    mBtn.setDisable(BTN_CONFIRM, disable != FALSE);
}

// .text:0xBF8
void dMenuKabu_c::startWinAnm(int anm) {
    static const char *winAnmNames[WIN_ANM_NUM] = {
        "common_unit_win_00_unitWin_In.brlan",
        "common_unit_win_00_unitWin_Out.brlan",
    };
    mLyt.setAnm(&mWinAnm, winAnmNames[anm], nullptr);
}

// .text:0xC18
void dMenuKabu_c::calcWinAnm() {
    bool playing = false;
    if (!mWinAnm.mFrameCtrl.isStop()) {
        mWinAnm.play();
        playing = true;
    }
    if (playing) {
        onFlag(FLAG_CALC);
    }
}

// .text:0xC7C
void dMenuKabu_c::updateDisplay() {
    const char *panes[FORMAT_NUM] = {"T_input_00", "T_input_01", "T_input_02"};
    BOOL hasValue = mValue > 0;
    int format = mFormat;
    setConfirmDisable(mMin > mValue);
    const char *pane = panes[format];
    mLyt.setVisible(pane, hasValue);
    if (hasValue) {
        if (mMenuNo == dMenuMng_c::MENU_TURNIP_COUNT) {
            mLyt.setNumber(pane, mValue, mDigits, dScript::NUM_FORMAT_REGION);
        } else {
            mLyt.setNumberSep(pane, mValue, mDigits,
                              dScript::NUM_FORMAT_REGION);
        }
    }
    onFlag(FLAG_CALC);
}

// .text:0xD78
BOOL dMenuKabu_c::inputDigit(int digit) {
    if (digit == dMenuNumber_c::DIGIT_CLEAR) {
        mValue = 0;
        return TRUE;
    }
    int old = mValue;
    int max = mMax;
    int value = digit + old * 10;
    if (value > max) {
        value = max;
    }
    mValue = value;
    return value != old;
}

// .text:0xDC4
void dMenuKabu_c::initLayout() {
    mLyt.setAnm(&mIconAnm, "common_unit_win_00_icon_Ptrn.brlan", nullptr);
    mLyt.setMessage("T_kabu_00", MSG_TURNIPS, nullptr);
    mLyt.setMessage("T_beru_00", MSG_BELLS, nullptr);
    mLyt.setMessage("T_beru_01", MSG_BELLS, nullptr);
    switch (mMenuNo) {
    case dMenuMng_c::MENU_TURNIP_COUNT:
        mLyt.setMessage("T_msg_00", MSG_HOW_MANY, nullptr);
        mIconAnm.setFrame(ICON_TURNIP);
        break;
    case dMenuMng_c::MENU_TURNIP_PRICE:
        mLyt.setMessage("T_msg_00", MSG_PRICE, nullptr);
        mIconAnm.setFrame(ICON_PRICE);
        break;
    case dMenuMng_c::MENU_DONATION:
        mLyt.setMessage("T_msg_00", MSG_THANKS, nullptr);
        mIconAnm.setFrame(ICON_DONATION);
        break;
    case dMenuMng_c::MENU_AUCTION_MIN:
        mLyt.setMessage("T_msg_00", MSG_MIN_PRICE, nullptr);
        mIconAnm.setFrame(ICON_AUCTION);
        break;
    case dMenuMng_c::MENU_AUCTION_BID:
        mLyt.setMessage("T_msg_00", MSG_BID, nullptr);
        mIconAnm.setFrame(ICON_AUCTION);
        break;
    }
}
