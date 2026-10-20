#include <game/game/d_menu_bank.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_kbd.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuBank {
// .text:0x5C
BASE_PROFILE(MENU_BANK, dMenuBank_c)
} // namespace dMenuBank

static const char *l_allPane = "N_all";

// .text:0xE8
int dMenuBank_c::create() {
    int loaded = mRes.load("Layout/bank.arc");
    loaded &= mBtn.load();
    loaded &= mNumber.load();
    if (!loaded) {
        return NOT_READY;
    }
    if (!mNumber.create(dMenuNumber_c::MODE_AMOUNT)) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "money_win_00.brlyt");
    initMessages();
    mLyt.calc();
    mBtn.create();
    initButtons();
    onFlag(FLAG_DRAW);
    setWinAnm(WIN_ANM_IN);
    startAnm(&mNumber.mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    initAmounts();
    updateNumbers();
    return SUCCEEDED;
}

// .text:0x214
int dMenuBank_c::doDelete() {
    if (mNumber.remove() == FALSE) {
        return NOT_READY;
    }
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes.remove() != 0;
}

// .text:0x280
int dMenuBank_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x2A4
int dMenuBank_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x2A8
int dMenuBank_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x2CC
int dMenuBank_c::execute() {
    bool inAnm = isState(STATE_ANM);
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prevCursor &&
        !isFlag(FLAG_NO_CURSOR_SE)) {
        playSeCursor();
    }
    offFlag(FLAG_NO_CURSOR_SE);
    mNumber.execute(FALSE);
    if (inAnm) {
        mBtn.setPos(&mNumber.mLyt, l_allPane);
    }
    mBtn.execute();
    calcWinAnm();
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    return result;
}

// .text:0x3D0
int dMenuBank_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mNumber.draw();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x424
void dMenuBank_c::subStateMain() {
    onFlag(FLAG_NO_CURSOR_SE);
    setSelect(mCursor, FALSE);
    int key = mNumber.getKbdKey();
    if (key != CURSOR_NONE) {
        mCursor = key;
        setSelect(key, TRUE);
        push();
        return;
    }
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != CURSOR_NONE) {
        int slot = btn + CURSOR_BTN_FIRST;
        mCursor = slot;
        setSelect(slot, TRUE);
        push();
        return;
    }
    if (mBtn.isEnable(BTN_OK) && mNumber.isSendReq()) {
        mCursor = CURSOR_BTN_FIRST + BTN_OK;
        setSelect(mCursor, TRUE);
        push();
        return;
    }
    if (mBtn.isEnable(BTN_CANCEL) && mNumber.isClearReq()) {
        mCursor = CURSOR_BTN_FIRST + BTN_CANCEL;
        setSelect(mCursor, TRUE);
        push();
        return;
    }
    offFlag(FLAG_NO_CURSOR_SE);
    int slot = hitCheck();
    mCursor = slot;
    if (slot != CURSOR_NONE) {
        setSelect(slot, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push();
        }
    }
}

// .text:0x5B8
void dMenuBank_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x5F4
void dMenuBank_c::executeSubState() {
    typedef void (dMenuBank_c::*Proc)();
    static Proc subStateProcs[] = {&dMenuBank_c::subStateMain,
                                   &dMenuBank_c::subStatePush};
    (this->*subStateProcs[mSubState])();
}

// .text:0x62C
void dMenuBank_c::setSelect(int slot, BOOL select) {
    if (isBtn(slot)) {
        mBtn.setSelect(slot - CURSOR_BTN_FIRST, select, FALSE);
    }
    if (isKey(slot)) {
        mNumber.setCursor(slot, !select);
    }
}

// .text:0x6D4
int dMenuBank_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != CURSOR_NONE) {
        return btn + CURSOR_BTN_FIRST;
    }
    int key = mNumber.hitCheck();
    if (key != CURSOR_NONE) {
        return key;
    }
    return CURSOR_NONE;
}

// .text:0x74C
void dMenuBank_c::push() {
    int slot = mCursor;
    if (isKey(slot)) {
        mNumber.push(slot);
        if (addDigit(dMenuNumber_c::getKeyDigit(slot))) {
            mNumber.playKeySe(slot);
            updateNumbers();
        } else {
            dKbd::playSeCanNotInput();
        }
    } else if (isBtn(slot)) {
        mBtn.push(slot - CURSOR_BTN_FIRST, FALSE);
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x838
bool dMenuBank_c::isPushEnd() {
    int slot = mCursor;
    if (isKey(slot)) {
        return !mNumber.isPushing();
    }
    if (isBtn(slot)) {
        return !mBtn.isPushing();
    }
    return true;
}

// .text:0x8C4
void dMenuBank_c::finishPush() {
    int slot = mCursor;
    if (isBtn(slot)) {
        switch (slot - CURSOR_BTN_FIRST) {
        case BTN_CANCEL:
            startCloseAnm();
            break;
        case BTN_OK:
            startCloseAnm();
            dMenuMng_c::setResult(1);
            applyAmount();
            break;
        }
    } else {
        if (isKey(slot)) {
            setSubState(SUB_MAIN);
            return;
        }
        setSubState(SUB_MAIN);
    }
}

// .text:0x9B4
void dMenuBank_c::startCloseAnm() {
    setWinAnm(WIN_ANM_OUT);
    startAnm(&mNumber.mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(0);
}

// .text:0xA0C
void dMenuBank_c::initButtons() {
    mBtn.setText(BTN_CANCEL, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_OK, MSG_CONFIRM, FALSE);
}

// .text:0xA6C
void dMenuBank_c::setOkDisable(int disable) {
    mBtn.setDisable(BTN_OK, disable != 0);
}

// .text:0xA88
void dMenuBank_c::setWinAnm(int anm) {
    static const char *winAnmNames[WIN_ANM_NUM] = {
        "money_win_00_kabuWin_In.brlan",
        "money_win_00_kabuWin_Out.brlan",
    };
    mLyt.setAnm(&mWinAnm, winAnmNames[anm], nullptr);
}

// .text:0xAA8
void dMenuBank_c::calcWinAnm() {
    BOOL calc = FALSE;
    if (!mWinAnm.mFrameCtrl.isStop()) {
        mWinAnm.play();
        calc = TRUE;
    }
    if (calc) {
        onFlag(FLAG_CALC);
    }
}

// .text:0xB0C
void dMenuBank_c::initAmounts() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    mAmount = 0;
    int wallet = player->getPocketMoney();
    mWallet = wallet;
    switch (mMenuNo) {
    case dMenuMng_c::MENU_BANK_DEPOSIT: {
        int savings = player->mSavings;
        mBalance = savings;
        mMaxAmount = wallet;
        if (wallet + savings > MAX_MONEY) {
            mMaxAmount = MAX_MONEY - savings;
        }
        break;
    }
    case dMenuMng_c::MENU_BANK_WITHDRAW: {
        int savings = player->mSavings;
        mBalance = savings;
        mMaxAmount = savings;
        int room = player->getMoneyRoom(0);
        if (mMaxAmount > room) {
            mMaxAmount = room;
        }
        break;
    }
    case dMenuMng_c::MENU_BANK_PAY: {
        int debt = player->mDebt;
        mBalance = debt;
        mMaxAmount = wallet;
        if (wallet > debt) {
            mMaxAmount = debt;
        }
        break;
    }
    }
    mCursor = CURSOR_NONE;
}

// .text:0xBF4
void dMenuBank_c::applyAmount() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    switch (mMenuNo) {
    case dMenuMng_c::MENU_BANK_DEPOSIT:
        player->payMoney(mAmount, FALSE);
        player->addSavings(mAmount);
        break;
    case dMenuMng_c::MENU_BANK_WITHDRAW:
        player->addMoney(mAmount);
        player->subSavings(mAmount);
        break;
    case dMenuMng_c::MENU_BANK_PAY:
        player->payMoney(mAmount, FALSE);
        player->payDebt(mAmount);
        break;
    }
}

// .text:0xCA0
void dMenuBank_c::updateNumbers() {
    int before;
    int after;
    int amount = mAmount;
    switch (mMenuNo) {
    case dMenuMng_c::MENU_BANK_DEPOSIT:
        before = mWallet;
        after = mBalance + amount;
        break;
    case dMenuMng_c::MENU_BANK_WITHDRAW:
        before = mBalance;
        after = mWallet + amount;
        break;
    case dMenuMng_c::MENU_BANK_PAY:
        before = mWallet;
        after = mBalance;
        break;
    }
    setOkDisable(amount == 0);
    mLyt.setNumberSep("T_input_00", before, NUMBER_DIGITS,
                      dScript::NUM_FORMAT_REGION);
    mLyt.setNumberSep("T_input_01", amount, NUMBER_DIGITS,
                      dScript::NUM_FORMAT_REGION);
    mLyt.setNumberSep("T_input_02", after, NUMBER_DIGITS,
                      dScript::NUM_FORMAT_REGION);
    onFlag(FLAG_CALC);
}

// .text:0xD90
BOOL dMenuBank_c::addDigit(int digit) {
    if (digit == dMenuNumber_c::DIGIT_CLEAR) {
        mAmount = 0;
        return TRUE;
    }
    int amount = mAmount;
    int newAmount;
    if (amount > MAX_DIGITS_VALUE) {
        newAmount = mMaxAmount;
    } else {
        newAmount = digit + amount * 10;
        if (newAmount > mMaxAmount) {
            newAmount = mMaxAmount;
        }
    }
    mAmount = newAmount;
    return newAmount != amount;
}

// .text:0xDF4
void dMenuBank_c::initMessages() {
    mLyt.setMessage("T_beru_00", MSG_BELLS, nullptr);
    mLyt.setMessage("T_beru_01", MSG_BELLS, nullptr);
    mLyt.setMessage("T_beru_02", MSG_BELLS, nullptr);
    int type = TYPE_DEPOSIT;
    switch (mMenuNo) {
    case dMenuMng_c::MENU_BANK_DEPOSIT:
        type = TYPE_DEPOSIT;
        break;
    case dMenuMng_c::MENU_BANK_WITHDRAW:
        type = TYPE_WITHDRAW;
        break;
    case dMenuMng_c::MENU_BANK_PAY:
        type = TYPE_PAY;
        break;
    }
    u16 msg0[TYPE_NUM] = {MSG_YOUR_BELLS, MSG_SAVINGS, MSG_YOUR_BELLS};
    u16 msg1[TYPE_NUM] = {MSG_DEPOSIT, MSG_WITHDRAWAL, MSG_PAYMENT};
    u16 msg2[TYPE_NUM] = {MSG_SAVINGS, MSG_YOUR_BELLS, MSG_REMAINING};
    mLyt.setMessage("T_msg_00", msg0[type], nullptr);
    mLyt.setMessage("T_msg_01", msg1[type], nullptr);
    mLyt.setMessage("T_msg_02", msg2[type], nullptr);
}
