#include <game/game/d_menu_time00.hpp>
#include <cstdio>
#include <game/framework/f_profile.hpp>
#include <game/game/d_kbd.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_box.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>

namespace dMenuTime00 {
// .text:0x5C
BASE_PROFILE(MENU_TIME00, dMenuTime00_c)
} // namespace dMenuTime00

static const char *l_allPane = "N_all";
static const char *l_hoverFrame = "inputFc_00";
static const char *l_selectFrame = "inputFc_01";
static const char *l_amPmFrame = "inputFc_02";

// .text:0x188
int dMenuTime00_c::create() {
    int loaded = mRes.load("Layout/dayTime.arc");
    loaded &= mBtn.load();
    loaded &= mNumber.load();
    loaded &= mSlct.load();
    if (!loaded) {
        return NOT_READY;
    }
    if (!mNumber.create(dMenuNumber_c::MODE_AMOUNT)) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "dayTime_win_00.brlyt");
    mLyt.setAlpha(l_hoverFrame, 255);
    mLyt.setAlpha(l_selectFrame, 255);
    mLyt.setAlpha(l_amPmFrame, 255);
    mLyt.setVisible(l_hoverFrame, false);
    mLyt.setVisible(l_selectFrame, false);
    mLyt.setVisible(l_amPmFrame, false);
    mLyt.setMessage("T_quest_05", MSG_AM, nullptr);
    mLyt.setMessage("T_quest_06", MSG_PM, nullptr);
    mpMinuteHand = mLyt.findPane("N_long_00");
    mpHourHand = mLyt.findPane("N_short_00");
    BOOL showClock = TRUE;
    if (mMenuNo != dMenuMng_c::MENU_SET_CLOCK &&
        mMenuNo != dMenuMng_c::MENU_APPOINTMENT_TIME) {
        showClock = FALSE;
    }
    mpMinuteHand->SetVisible(showClock);
    mpHourHand->SetVisible(showClock);
    mHoverColumn = COLUMN_NONE;
    mSelectColumn = COLUMN_NONE;
    mAmPmHover = AMPM_NONE;
    mAmPm = AMPM_NONE;
    mCursor = CURSOR_NONE;
    mColumnPos[0] = mLyt.getPos("N_input_00");
    mColumnPos[1] = mLyt.getPos("N_input_01");
    mColumnPos[2] = mLyt.getPos("N_input_02");
    mColumnPos[3] = mLyt.getPos("N_input_03");
    mColumnPos[4] = mLyt.getPos("N_input_04");
    initWords();
    initAnms();
    mLyt.calc();
    mBtn.create();
    initButtons();
    mSlct.create();
    initInputs();
    refreshLayout();
    onFlag(FLAG_DRAW);
    setQuestWinAnm(QUEST_WIN_ANM_IN);
    mQuestWinAnm.setToEnd();
    startAnm(&mBtn.mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setWinAnm(ANM_UPPER_WIN_IN);
    return SUCCEEDED;
}

// .text:0x48C
int dMenuTime00_c::doDelete() {
    if (mNumber.remove() == FALSE) {
        return NOT_READY;
    }
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    if (mSlct.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes.remove() != 0;
}

// .text:0x510
int dMenuTime00_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x534
int dMenuTime00_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mLyt.setPosFrom("N_input_05", "S_Psh_05", 0.0f, 0.0f);
    mLyt.setPosFrom("N_input_06", "S_Psh_06", 0.0f, 0.0f);
    mWinAnm.play();
    mLyt.calc();
    mNumber.setPos(&mBtn.mLyt, l_allPane);
    return result;
}

// .text:0x5E8
int dMenuTime00_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x60C
int dMenuTime00_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prevCursor &&
        !isFlag(FLAG_NO_CURSOR_SE)) {
        playSeCursor();
    }
    offFlag(FLAG_NO_CURSOR_SE);
    mBtn.execute();
    mNumber.execute(FALSE);
    mSlct.execute();
    calcAnms();
    if (isFlag(FLAG_WEEKDAY)) {
        offFlag(FLAG_WEEKDAY);
        if (mInput[COLUMN_MONTH] - 1 <= MONTH_DECEMBER) {
            int weekday = dTime_c::getWeekday(mInput[COLUMN_YEAR],
                                              mInput[COLUMN_MONTH] - 1,
                                              mInput[COLUMN_DAY]);
            dString::Word_c word;
            word.clear();
            dScript::setWeekdayName(&word, weekday, TRUE);
            mLyt.setWord("T_youbi_00", &word);
        }
        onFlag(FLAG_CALC);
    }
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    return result;
}

// .text:0x770
int dMenuTime00_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        mNumber.draw();
        mBtn.draw();
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    return SUCCEEDED;
}

// .text:0x7E0
void dMenuTime00_c::subStateMain() {
    onFlag(FLAG_NO_CURSOR_SE);
    setSelect(mCursor, FALSE);
    int key = mNumber.getKbdKey();
    if (key != CURSOR_NONE) {
        mCursor = key;
        setSelect(key, TRUE);
        push();
        return;
    }
    int btn = mBtn.checkPad(TRUE, FALSE);
    if (btn != CURSOR_NONE) {
        int slot = btn + CURSOR_BTN_FIRST;
        mCursor = slot;
        setSelect(slot, TRUE);
        push();
        return;
    }
    if (mBtn.isEnable(BTN_OK) && mNumber.isSendReq()) {
        mCursor = CURSOR_BTN_FIRST + BTN_OK;
        setSelect(CURSOR_BTN_FIRST + BTN_OK, TRUE);
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

// .text:0x92C
void dMenuTime00_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x968
void dMenuTime00_c::subStateNextPage() {
    if (mQuestWinAnm.mFrameCtrl.isStop()) {
        refreshLayout();
        selectFirstColumn();
        updateColumn(COLUMN_HOUR);
        setQuestWinAnm(QUEST_WIN_ANM_IN);
        setSubState(SUB_PAGE_IN);
    }
}

// .text:0x9E0
void dMenuTime00_c::subStatePageIn() {
    if (mQuestWinAnm.mFrameCtrl.isStop()) {
        setSubState(SUB_MAIN);
    }
}

// .text:0xA30
void dMenuTime00_c::subStateDialogInput() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0xA90
void dMenuTime00_c::subStateDialogPush() {
    if (mSlct.isPushEnd()) {
        if (isFlag(FLAG_DIALOG_ERROR)) {
            mDialog = DIALOG_CANCELLED;
        } else if (mSlct.mPushedBtn == DIALOG_BTN_NO) {
            mDialog = DIALOG_CANCELLED;
        }
        setSubState(SUB_DIALOG_CLOSE);
        mSlct.open(dMenuSlctB_c::WIN_ANM_DOWN_OUT, FALSE, FALSE);
    }
}

// .text:0xB28
void dMenuTime00_c::subStateDialogClose() {
    if (!mSlct.isWinAnmEnd()) {
        return;
    }
    switch (mDialog) {
    case DIALOG_CANCELLED:
        if (isFlag(FLAG_DIALOG_ERROR) && mColumn >= 0) {
            int limit;
            BOOL changed = FALSE;
            switch (mField) {
            case FIELD_YEAR:
                limit = YEAR_MAX;
                break;
            case FIELD_MONTH:
                limit = MONTH_NUM;
                break;
            case FIELD_DAY:
                limit = getMaxDay();
                break;
            case FIELD_HOUR:
                if (uses12HourClock()) {
                    limit = HOURS_PER_HALF_DAY;
                } else {
                    limit = HOUR_MAX;
                }
                break;
            case FIELD_MINUTE:
                limit = MINUTE_MAX;
                break;
            }
            if (mInput[mColumn] > limit) {
                mInput[mColumn] = limit;
                changed = TRUE;
            }
            if (changed) {
                updateColumn(mColumn);
            }
            mDigitAnm[mColumn * 2].start(DIGIT_ANM_SELECTED);
            mDigitAnm[mColumn * 2 + 1].start(DIGIT_ANM_SELECTED);
            onFlag(FLAG_CALC);
        }
        setSubState(SUB_MAIN);
        offFlag(FLAG_DIALOG);
        break;
    case DIALOG_DATE:
        setModeByRegion();
        setQuestWinAnm(QUEST_WIN_ANM_OUT);
        setSubState(SUB_NEXT_PAGE);
        break;
    case DIALOG_FINAL: {
        dTime_c time;
        startCloseAnm();
        dMenuMng_c::setResult(RESULT_OK);
        switch (mMenuNo) {
        case dMenuMng_c::MENU_SET_CLOCK:
            getInputTime(&time);
            dTime_c::setOffset(time);
            dTime_c::saveOffset();
            dSaveData_c::getTown()->mTimeOffset.setChanged();
            break;
        case dMenuMng_c::MENU_FUTURE_MAIL_DATE: {
            getInputTime(&time);
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            dPlayerPostBox_c *postBox = player->getPostBox();
            postBox->setFutureMail(dMenuMng_c::getMail(), &time);
            player->clearLetter(dMenuMng_c::getSelSlot());
            break;
        }
        case dMenuMng_c::MENU_APPOINTMENT_TIME:
            getInputTime(&time);
            dMenuMng_c::setTime(&time);
            break;
        case dMenuMng_c::MENU_BIRTHDAY: {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            int month = mInput[COLUMN_HOUR] - 1;
            int day = mInput[COLUMN_MINUTE];
            player->mBirthday.mMonth = month;
            player->mBirthday.mDay = day;
            break;
        }
        }
        break;
    }
    }
}

// .text:0xE24
void dMenuTime00_c::executeSubState() {
    typedef void (dMenuTime00_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuTime00_c::subStateMain,        &dMenuTime00_c::subStatePush,
        &dMenuTime00_c::subStateNextPage,    &dMenuTime00_c::subStatePageIn,
        &dMenuTime00_c::subStateDialogInput, &dMenuTime00_c::subStateDialogPush,
        &dMenuTime00_c::subStateDialogClose};
    (this->*subStateProcs[mSubState])();
}

// .text:0xE5C
void dMenuTime00_c::initButtons() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_SET_CLOCK:
    case dMenuMng_c::MENU_BIRTHDAY:
    case dMenuMng_c::MENU_FUTURE_MAIL_DATE:
    case dMenuMng_c::MENU_APPOINTMENT_TIME:
        mBtn.setVisible(BTN_CANCEL, false);
        break;
    default:
        mBtn.setText(BTN_CANCEL, MSG_QUIT, TRUE);
        break;
    }
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_OK, MSG_CONFIRM, FALSE);
}

// .text:0xEE8
void dMenuTime00_c::setSelect(int slot, BOOL select) {
    if (isBtn(slot)) {
        mBtn.setSelect(slot - CURSOR_BTN_FIRST, select, FALSE);
    }
    if (isKey(slot)) {
        mNumber.setCursor(slot, !select);
    }
    if (isColumn(slot)) {
        int column = slot - CURSOR_COLUMN_FIRST;
        if (select) {
            setHoverFrame(column, TRUE);
            setSelectFrame(column, FALSE);
        } else if (mColumn == column) {
            setHoverFrame(column, FALSE);
            setSelectFrame(column, TRUE);
        } else {
            setHoverFrame(column, FALSE);
        }
    }
    if (isAmPm(slot)) {
        setAmPmFrame(slot - CURSOR_AMPM_FIRST, select);
    }
}

// .text:0x1058
int dMenuTime00_c::hitCheck() {
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
    mVec2_c hitPos = pos;
    int hit = m2d::hitButton(&hitPos, &mLyt, CURSOR_COLUMN_FIRST,
                             CURSOR_AMPM_LAST, -1, -1, 1.0f);
    if (isAmPm(hit) && hit - CURSOR_AMPM_FIRST == mAmPm) {
        hit = CURSOR_NONE;
    }
    if (hit != CURSOR_NONE) {
        return hit;
    }
    return CURSOR_NONE;
}

// .text:0x1144
void dMenuTime00_c::push() {
    int slot = mCursor;
    if (isKey(slot)) {
        mNumber.push(slot);
        if (mColumn == COLUMN_NONE) {
            dKbd::playSeCanNotInput();
        } else {
            mNumber.playKeySe(slot);
        }
    } else if (isBtn(slot)) {
        mBtn.push(slot - CURSOR_BTN_FIRST, FALSE);
    } else if (isColumn(slot)) {
        pushColumn(slot - CURSOR_COLUMN_FIRST);
    } else if (isAmPm(slot)) {
        if (!pushAmPm(slot - CURSOR_AMPM_FIRST)) {
            return;
        }
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x1280
BOOL dMenuTime00_c::isPushEnd() {
    int slot = mCursor;
    if (isBtn(slot)) {
        return !mBtn.isPushing();
    }
    if (isKey(slot)) {
        return !mNumber.isPushing();
    }
    if (isColumn(slot)) {
        return mPushAnm.mFrameCtrl.isStop();
    }
    if (isAmPm(slot)) {
        return mAmPmAnm[slot - CURSOR_AMPM_FIRST].mFrameCtrl.isStop();
    }
    return TRUE;
}

// .text:0x1370
void dMenuTime00_c::finishPush() {
    if (isBtn(mCursor)) {
        switch (mCursor - CURSOR_BTN_FIRST) {
        case BTN_CANCEL:
            startCloseAnm();
            break;
        case BTN_OK:
            onOkPushed();
            break;
        }
    } else if (isKey(mCursor)) {
        if (mColumn != COLUMN_NONE) {
            inputDigit(dMenuNumber_c::getKeyDigit(mCursor));
        }
        if (mSubState != SUB_DIALOG_INPUT) {
            setSubState(SUB_MAIN);
        }
    } else if (isColumn(mCursor)) {
        selectColumn(mCursor - CURSOR_COLUMN_FIRST);
        setSelect(mCursor, TRUE);
        setSubState(SUB_MAIN);
        updateClockTarget();
    } else if (isAmPm(mCursor)) {
        setSubState(SUB_MAIN);
        setAmPm(mCursor - CURSOR_AMPM_FIRST, FALSE);
        updateClockTarget();
        setSelect(mCursor, FALSE);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x1544
void dMenuTime00_c::startCloseAnm() {
    finishColumn();
    startAnm(&mBtn.mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    setWinAnm(ANM_UPPER_WIN_OUT);
    dMenuMng_c::setResult(RESULT_CANCEL);
}

// .text:0x15A4
void dMenuTime00_c::openErrorDialog(u16 msg) {
    mSlct.setBtnVisible(DIALOG_BTN_NO, false);
    mSlct.setBtnVisible(DIALOG_BTN_YES, false);
    mSlct.setBtn(DIALOG_BTN_OK, MSG_OK, nullptr, 0);
    playSeCanNot();
    mSlct.open(dMenuSlctB_c::WIN_ANM_DOWN_IN, TRUE, FALSE);
    setSubState(SUB_DIALOG_INPUT);
    onFlag(FLAG_DIALOG);
    mSlct.setMsg(msg, nullptr);
    onFlag(FLAG_DIALOG_ERROR);
}

// .text:0x1668
void dMenuTime00_c::openRangeError(u16 msg, int value) {
    openErrorDialog(msg);
    selectColumn(mColumn);
    mInput[mColumn] = value;
    mDigitAnm[mColumn * 2].start(DIGIT_ANM_FLASH);
    mDigitAnm[mColumn * 2 + 1].start(DIGIT_ANM_FLASH);
    updateColumn(mColumn);
}

// .text:0x1704
void dMenuTime00_c::setYearWord(int year) {
    dString::Word_c word;
    dScript::formatNumber(&word, year + YEAR_BASE, 4, dScript::NUM_FORMAT_PLAIN,
                          0);
    dString::setWord(WORD_YEAR, &word);
}

// .text:0x1764
void dMenuTime00_c::setMonthDayWords(int month, int day) {
    dString::Word_c word;
    dScript::setMonthName(&word, month - 1);
    dString::setWord(WORD_MONTH, &word);
    word.clear();
    dScript::setDayName(&word, day);
    dString::setWord(WORD_DAY, &word);
}

// .text:0x17E4
void dMenuTime00_c::setWeekdayWord(int year, int month, int day) {
    int weekday = dTime_c::getWeekday(year, month - 1, day);
    dString::Word_c word;
    word.clear();
    dScript::setWeekdayName(&word, weekday, FALSE);
    dString::setWord(WORD_WEEKDAY, &word);
}

// .text:0x184C
void dMenuTime00_c::setTimeWords(int hour, int minute, int hourSlot,
                                 int minuteSlot) {
    dString::Word_c word;
    dScript::formatNumber(&word, hour, 2, dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(hourSlot, &word);
    word.clear();
    switch (getConsoleRegion()) {
    case CONSOLE_REGION_NA:
    case CONSOLE_REGION_EU:
        dScript::formatNumber(&word, minute, 2, dScript::NUM_FORMAT_ZERO_PAD,
                              0);
        break;
    default:
        dScript::formatNumber(&word, minute, 2, dScript::NUM_FORMAT_PLAIN, 0);
        break;
    }
    dString::setWord(minuteSlot, &word);
}

// .text:0x1930
void dMenuTime00_c::setAmPmWord(int ampm, int slot) {
    static u16 ampmMsgs[2] = {MSG_AM, MSG_PM};
    dString::Word_c word;
    if (ampm != AMPM_NONE) {
        dString::loadString(&word, ampmMsgs[ampm], "sys_2D/SYS_2D_menu");
    }
    dString::setWord(slot, &word);
}

// .text:0x19AC
void dMenuTime00_c::openConfirmDialog(u16 msg) {
    dString::Word_c word;
    int ampm;
    BOOL last = TRUE;
    switch (mMenuNo) {
    case dMenuMng_c::MENU_BIRTHDAY:
        setMonthDayWords(mInput[COLUMN_HOUR], mInput[COLUMN_MINUTE]);
        break;
    case dMenuMng_c::MENU_FUTURE_MAIL_DATE:
        setYearWord(mInput[COLUMN_YEAR]);
        setMonthDayWords(mInput[COLUMN_MONTH], mInput[COLUMN_DAY]);
        setWeekdayWord(mInput[COLUMN_YEAR], mInput[COLUMN_MONTH],
                       mInput[COLUMN_DAY]);
        break;
    case dMenuMng_c::MENU_SET_CLOCK: {
        if (mMode == MODE_DATE) {
            last = FALSE;
        }
        setYearWord(mInput[COLUMN_YEAR]);
        setMonthDayWords(mInput[COLUMN_MONTH], mInput[COLUMN_DAY]);
        setWeekdayWord(mInput[COLUMN_YEAR], mInput[COLUMN_MONTH],
                       mInput[COLUMN_DAY]);
        setTimeWords(mInput[COLUMN_HOUR], mInput[COLUMN_MINUTE], WORD_HOUR,
                     WORD_MINUTE);
        ampm = mAmPm;
        if (!uses12HourClock()) {
            ampm = AMPM_NONE;
        }
        setAmPmWord(ampm, WORD_AMPM);
        break;
    }
    case dMenuMng_c::MENU_APPOINTMENT_TIME: {
        word.clear();
        dScript::setDayName(&word, mInput[COLUMN_DAY]);
        dString::setWord(WORD_APPT_DAY, &word);
        setTimeWords(mInput[COLUMN_HOUR], mInput[COLUMN_MINUTE], WORD_APPT_HOUR,
                     WORD_APPT_MINUTE);
        ampm = mAmPm;
        if (!uses12HourClock()) {
            ampm = AMPM_NONE;
        }
        setAmPmWord(ampm, WORD_APPT_AMPM);
        break;
    }
    }
    mSlct.setBtn(DIALOG_BTN_NO, MSG_NO, nullptr, SE_cancel_little);
    if (last) {
        mSlct.setBtn(DIALOG_BTN_YES, MSG_YES, nullptr, SE_decide);
    } else {
        mSlct.setBtn(DIALOG_BTN_YES, MSG_YES, nullptr, 0);
    }
    mSlct.setBtnVisible(DIALOG_BTN_OK, false);
    mSlct.open(dMenuSlctB_c::WIN_ANM_DOWN_IN, FALSE, FALSE);
    setSubState(SUB_DIALOG_INPUT);
    onFlag(FLAG_DIALOG);
    mSlct.setMsg(msg, nullptr);
    offFlag(FLAG_DIALOG_ERROR);
}

// .text:0x1BEC
BOOL dMenuTime00_c::isFutureDate() {
    dTime_c now = *dTime_c::getCurrent();
    int year = mInput[COLUMN_YEAR] + YEAR_BASE;
    int month = mInput[COLUMN_MONTH] - 1;
    int day = mInput[COLUMN_DAY];
    if (year == now.year) {
        if (month == now.mon) {
            return day > now.mday;
        }
        return month > now.mon;
    }
    return year > now.year;
}

// .text:0x1CD8
void dMenuTime00_c::onOkPushed() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_BIRTHDAY:
        finishColumn();
        mDialog = DIALOG_FINAL;
        openConfirmDialog(MSG_CONFIRM_MONTH_DAY);
        break;
    case dMenuMng_c::MENU_FUTURE_MAIL_DATE:
        if (mInput[COLUMN_YEAR] > YEAR_MAX) {
            openErrorDialog(MSG_ERR_YEAR);
            break;
        }
        if (!isFutureDate()) {
            openErrorDialog(MSG_ERR_DATE_PAST);
            break;
        }
        finishColumn();
        mDialog = DIALOG_FINAL;
        openConfirmDialog(MSG_CONFIRM_DATE_FULL);
        break;
    case dMenuMng_c::MENU_SET_CLOCK:
        if (mMode == MODE_DATE) {
            if (mInput[COLUMN_YEAR] > YEAR_MAX) {
                openErrorDialog(MSG_ERR_YEAR);
                break;
            }
            finishColumn();
            mDialog = DIALOG_DATE;
            openConfirmDialog(MSG_CONFIRM_DATE_FULL);
            break;
        }
        if ((mMode == MODE_TIME_FIRST || mMode == MODE_TIME_KOREAN ||
             mMode == MODE_TIME_12H) &&
            mInput[COLUMN_HOUR] == 0) {
            mInput[COLUMN_HOUR] = HOURS_PER_HALF_DAY;
            updateColumn(COLUMN_HOUR);
        }
        finishColumn();
        mDialog = DIALOG_FINAL;
        openConfirmDialog(MSG_CONFIRM_TIME);
        updateClockTarget();
        break;
    case dMenuMng_c::MENU_APPOINTMENT_TIME:
        if (isAppointmentValid()) {
            finishColumn();
            mDialog = DIALOG_FINAL;
            openConfirmDialog(MSG_CONFIRM_APPOINTMENT);
            updateClockTarget();
        } else {
            openErrorDialog(MSG_ERR_TIME_RANGE);
        }
        break;
    }
    selectColumn(COLUMN_NONE);
}

// .text:0x1E88
BOOL dMenuTime00_c::isAppointmentValid() {
    dTime_c now = *dTime_c::getCurrent();
    mInput[COLUMN_YEAR] = now.year - YEAR_BASE;
    mInput[COLUMN_MONTH] = now.mon + 1;
    mInput[COLUMN_DAY] = now.mday;
    dTime_c time;
    getInputTime(&time);
    if (!dTime_c::isSameOrAfter(time, now)) {
        time.add(1, 0, 0, 0);
        mInput[COLUMN_YEAR] = time.year - YEAR_BASE;
        mInput[COLUMN_MONTH] = time.mon + 1;
        mInput[COLUMN_DAY] = time.mday;
    }
    now.add(0, APPOINTMENT_HOURS, 0, 0);
    if (!dTime_c::isSameOrAfter(time, now)) {
        return TRUE;
    }
    return FALSE;
}

// .text:0x20F8
void dMenuTime00_c::getInputTime(dTime_c *time) {
    int year = mInput[COLUMN_YEAR] + YEAR_BASE;
    int month = mInput[COLUMN_MONTH] - 1;
    int day = mInput[COLUMN_DAY];
    int hour = mInput[COLUMN_HOUR];
    int minute = mInput[COLUMN_MINUTE];
    if (uses12HourClock()) {
        if (mAmPm != AMPM_AM) {
            if (hour < HOURS_PER_HALF_DAY) {
                hour += HOURS_PER_HALF_DAY;
            }
        } else if (hour >= HOURS_PER_HALF_DAY) {
            hour = 0;
        }
    }
    time->set(year, month, day, hour, minute, 0);
    time->normalize();
}

// .text:0x21A4
void dMenuTime00_c::setMode(int mode) {
    mMode = mode;
}

// .text:0x21AC
void dMenuTime00_c::setModeByRegion() {
    switch (getConsoleRegion()) {
    case CONSOLE_REGION_JP:
        setMode(MODE_TIME_FIRST);
        break;
    case CONSOLE_REGION_NA:
    case CONSOLE_REGION_EU:
        if (uses12HourClock()) {
            setMode(MODE_TIME_12H);
        } else {
            setMode(MODE_TIME_24H);
        }
        break;
    case CONSOLE_REGION_KR:
        setMode(MODE_TIME_KOREAN);
        break;
    }
}

// .text:0x2240
void dMenuTime00_c::refreshLayout() {
    updateModeLayout();
    updateModeTexts();
    resetClockIfShown();
}

// .text:0x227C
void dMenuTime00_c::updateModeLayout() {
    bool timeMode;
    bool dateShown;
    bool amPmShown;
    BOOL dateMode = FALSE;
    BOOL amPm = FALSE;
    switch (mMode) {
    case MODE_DATE:
        dateMode = TRUE;
        amPm = FALSE;
        break;
    case MODE_MONTH_DAY:
        dateMode = FALSE;
        amPm = FALSE;
        if (isDayFirst()) {
            mLyt.setPos("N_input_04", (const mVec3_c *)&mColumnPos[3]);
            mLyt.setPos("N_input_03", (const mVec3_c *)&mColumnPos[4]);
        } else {
            mLyt.setPos("N_input_03", (const mVec3_c *)&mColumnPos[3]);
            mLyt.setPos("N_input_04", (const mVec3_c *)&mColumnPos[4]);
        }
        break;
    case MODE_TIME_FIRST:
    case MODE_TIME_12H:
    case MODE_TIME_KOREAN:
        dateMode = FALSE;
        amPm = TRUE;
        if (mMode == MODE_TIME_12H) {
            mLyt.setPos("N_input_03", (const mVec3_c *)&mColumnPos[0]);
            mLyt.setPos("N_input_04", (const mVec3_c *)&mColumnPos[1]);
        } else {
            mLyt.setPos("N_input_03", (const mVec3_c *)&mColumnPos[1]);
            mLyt.setPos("N_input_04", (const mVec3_c *)&mColumnPos[2]);
        }
        break;
    case MODE_TIME_24H:
        dateMode = FALSE;
        amPm = FALSE;
        mLyt.setPos("N_input_03", (const mVec3_c *)&mColumnPos[3]);
        mLyt.setPos("N_input_04", (const mVec3_c *)&mColumnPos[4]);
        break;
    }
    timeMode = !dateMode;
    dateShown = !timeMode;
    mLyt.setVisible("N_input_00", dateShown);
    mLyt.setVisible("N_input_01", dateShown);
    mLyt.setVisible("N_input_02", dateShown);
    mLyt.setVisible("N_input_03", timeMode);
    mLyt.setVisible("N_input_04", timeMode);
    amPmShown = amPm != 0;
    mLyt.setVisible("N_input_05", amPmShown);
    mLyt.setVisible("N_input_06", amPmShown);
    mLyt.setVisible("S_Psh_05", amPmShown);
    mLyt.setVisible("S_Psh_06", amPmShown);
    mLyt.setVisible("B_input_05", amPmShown);
    mLyt.setVisible("B_input_06", amPmShown);
    mLyt.setVisible("N_youbi", dateShown);
    mLyt.setVisible("N_cmnQ_icn_00", timeMode);
    if (mColumn != COLUMN_NONE) {
        setSelectFrame(mColumn, TRUE);
    }
    onFlag(FLAG_CALC);
}

// .text:0x24E4
void dMenuTime00_c::updateModeTexts() {
    static const u16 titleMsgs[MODE_NUM] = {MSG_ENTER_DATE, MSG_ENTER_TIME,
                                            MSG_ENTER_TIME, MSG_ENTER_TIME,
                                            MSG_ENTER_TIME, MSG_ENTER_DATE};
    u16 msg = titleMsgs[mMode];
    switch (mMenuNo) {
    case dMenuMng_c::MENU_APPOINTMENT_TIME:
        msg = MSG_SET_TIME;
        break;
    case dMenuMng_c::MENU_BIRTHDAY:
        msg = MSG_BIRTHDAY;
        break;
    }
    mTitle.clear();
    dString::loadString(&mTitle, msg, "sys_2D/SYS_2D_menu");
    mTitle.update();
    switch (mMode) {
    case MODE_DATE:
        setLabel(COLUMN_YEAR, MSG_YEAR);
        setLabel(COLUMN_MONTH, MSG_MONTH);
        setLabel(COLUMN_DAY, MSG_DAY);
        updateColumn(COLUMN_YEAR);
        updateColumn(COLUMN_MONTH);
        updateColumn(COLUMN_DAY);
        break;
    case MODE_TIME_FIRST:
    case MODE_TIME_12H:
    case MODE_TIME_24H:
    case MODE_TIME_KOREAN:
        setLabel(COLUMN_HOUR, MSG_HOUR);
        setLabel(COLUMN_MINUTE, MSG_MINUTE);
        updateColumn(COLUMN_HOUR);
        updateColumn(COLUMN_MINUTE);
        break;
    case MODE_MONTH_DAY:
        setLabel(COLUMN_HOUR, MSG_MONTH);
        setLabel(COLUMN_MINUTE, MSG_DAY);
        updateColumn(COLUMN_HOUR);
        updateColumn(COLUMN_MINUTE);
        break;
    }
    onFlag(FLAG_CALC);
}

// .text:0x2674
void dMenuTime00_c::resetClockIfShown() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_SET_CLOCK:
    case dMenuMng_c::MENU_APPOINTMENT_TIME:
        resetClock();
        break;
    }
}

// .text:0x2698
void dMenuTime00_c::initWords() {
    mTitle.setTextBox(mLyt.findTextBox("T_msg_00"));
    for (int i = 0; i < COLUMN_NUM; i++) {
        char name[32];
        sprintf(name, "T_quest_%02d", i);
        mLabel[i].setTextBox(mLyt.findTextBox(name));
    }
}

// .text:0x273C
void dMenuTime00_c::initClockInput() {
    dTime_c now = *dTime_c::getCurrent();
    mInput[COLUMN_HOUR] = now.hour;
    if (uses12HourClock()) {
        if (mInput[COLUMN_HOUR] >= HOURS_PER_HALF_DAY) {
            mInput[COLUMN_HOUR] -= HOURS_PER_HALF_DAY;
            setAmPm(AMPM_PM, TRUE);
        } else {
            setAmPm(AMPM_AM, TRUE);
        }
        if (mInput[COLUMN_HOUR] == 0) {
            mInput[COLUMN_HOUR] = HOURS_PER_HALF_DAY;
        }
    }
    mInput[COLUMN_MINUTE] = now.min;
}

// .text:0x2820
void dMenuTime00_c::initInputs() {
    mInput[COLUMN_YEAR] = 0;
    u8 menu = mMenuNo;
    mInput[COLUMN_MONTH] = 0;
    mInput[COLUMN_DAY] = 0;
    mInput[COLUMN_HOUR] = 0;
    mInput[COLUMN_MINUTE] = 0;
    mColumn = COLUMN_NONE;
    setSubState(SUB_MAIN);
    dTime_c now = *dTime_c::getCurrent();
    switch (menu) {
    case dMenuMng_c::MENU_SET_CLOCK:
        mInput[COLUMN_YEAR] = now.year - YEAR_BASE;
        mInput[COLUMN_MONTH] = now.mon + 1;
        mInput[COLUMN_DAY] = now.mday;
        initClockInput();
        setMode(MODE_DATE);
        resetClock();
        selectFirstColumn();
        break;
    case dMenuMng_c::MENU_APPOINTMENT_TIME:
        mInput[COLUMN_YEAR] = now.year - YEAR_BASE;
        mInput[COLUMN_MONTH] = now.mon + 1;
        mInput[COLUMN_DAY] = now.mday;
        initClockInput();
        setModeByRegion();
        refreshLayout();
        selectFirstColumn();
        break;
    case dMenuMng_c::MENU_FUTURE_MAIL_DATE:
        now.add(1, 0, 0, 0);
        mInput[COLUMN_YEAR] = now.year - YEAR_BASE;
        mInput[COLUMN_MONTH] = now.mon + 1;
        mInput[COLUMN_DAY] = now.mday;
        mInput[COLUMN_HOUR] = FUTURE_MAIL_HOUR;
        mInput[COLUMN_MINUTE] = 0;
        setMode(MODE_DATE);
        selectFirstColumn();
        break;
    case dMenuMng_c::MENU_BIRTHDAY:
        mInput[COLUMN_HOUR] = 1;
        mInput[COLUMN_MINUTE] = 1;
        setMode(MODE_MONTH_DAY);
        selectFirstColumn();
        break;
    }
}

// .text:0x2A0C
void dMenuTime00_c::clearColumn(int column) {
    if (mMode == MODE_MONTH_DAY ||
        (mMode == MODE_DATE &&
         (column == COLUMN_MONTH || column == COLUMN_DAY))) {
        mInput[column] = 1;
    } else {
        mInput[column] = 0;
    }
}

// .text:0x2A54
void dMenuTime00_c::setLabel(int idx, u16 msg) {
    m2d::stringWord_c *label = &mLabel[idx];
    label->clear();
    dString::loadString(label, msg, "sys_2D/SYS_2D_menu");
    label->update();
}

// .text:0x2AB4
void dMenuTime00_c::updateColumn(int column) {
    int i;
    int value = mInput[column];
    BOOL zeroPad = FALSE;
    int region = getConsoleRegion();
    if ((region == CONSOLE_REGION_NA || region == CONSOLE_REGION_EU) &&
        column == COLUMN_MINUTE &&
        (mMode == MODE_TIME_12H || mMode == MODE_TIME_24H)) {
        zeroPad = TRUE;
    }
    int digits[2];
    if (column != COLUMN_YEAR && !isFlag(FLAG_COLUMN_FIRST + column) &&
        value < 10) {
        digits[0] = zeroPad ? 0 : -1;
        digits[1] = value;
    } else {
        digits[0] = value / 10;
        digits[1] = value % 10;
    }
    for (i = 0; i < 2; i++) {
        char name[32];
        sprintf(name, "T_input_%02d", column * 2 + i);
        if (digits[i] == -1) {
            mLyt.setVisible(name, false);
        } else {
            mLyt.setNumber(name, digits[i], 1, dScript::NUM_FORMAT_PLAIN);
            mLyt.setVisible(name, true);
        }
        onFlag(FLAG_CALC);
    }
    if (mMode == MODE_DATE && mInput[COLUMN_MONTH] > 0 &&
        mInput[COLUMN_DAY] > 0) {
        onFlag(FLAG_WEEKDAY);
    }
}

// .text:0x2C4C
void dMenuTime00_c::selectFirstColumn() {
    int region = getConsoleRegion();
    switch (mMode) {
    case MODE_DATE:
        switch (region) {
        case CONSOLE_REGION_JP:
        case CONSOLE_REGION_KR:
            selectColumn(COLUMN_YEAR);
            break;
        case CONSOLE_REGION_NA:
            if (isDayFirst()) {
                selectColumn(COLUMN_DAY);
            } else {
                selectColumn(COLUMN_MONTH);
            }
            break;
        case CONSOLE_REGION_EU:
            selectColumn(COLUMN_DAY);
            break;
        }
        break;
    case MODE_MONTH_DAY:
        if (isDayFirst()) {
            selectColumn(COLUMN_MINUTE);
        } else {
            selectColumn(COLUMN_HOUR);
        }
        break;
    case MODE_TIME_FIRST:
    case MODE_TIME_12H:
    case MODE_TIME_24H:
    case MODE_TIME_KOREAN:
        selectColumn(COLUMN_HOUR);
        break;
    }
}

// .text:0x2D48
BOOL dMenuTime00_c::selectNextColumn() {
    int region = getConsoleRegion();
    if (mMode == MODE_MONTH_DAY) {
        if (isDayFirst()) {
            if (mColumn == COLUMN_MINUTE) {
                selectColumn(COLUMN_HOUR);
                return TRUE;
            }
            if (mColumn == COLUMN_HOUR) {
                return FALSE;
            }
        }
        if (mColumn == COLUMN_HOUR) {
            selectColumn(COLUMN_MINUTE);
            return TRUE;
        }
        if (mColumn == COLUMN_MINUTE) {
            return FALSE;
        }
    }
    switch (mColumn) {
    case COLUMN_YEAR:
        switch (region) {
        case CONSOLE_REGION_JP:
            selectColumn(COLUMN_MONTH);
            break;
        case CONSOLE_REGION_NA:
            return FALSE;
        case CONSOLE_REGION_EU:
            return FALSE;
        case CONSOLE_REGION_KR:
            selectColumn(COLUMN_MONTH);
            break;
        }
        return TRUE;
    case COLUMN_MONTH:
        if (isDayFirst()) {
            selectColumn(COLUMN_YEAR);
        } else {
            selectColumn(COLUMN_DAY);
        }
        return TRUE;
    case COLUMN_DAY:
        switch (region) {
        case CONSOLE_REGION_NA:
            if (isDayFirst()) {
                selectColumn(COLUMN_MONTH);
            } else {
                selectColumn(COLUMN_YEAR);
            }
            return TRUE;
        case CONSOLE_REGION_EU:
            selectColumn(COLUMN_MONTH);
            return TRUE;
        }
        return FALSE;
    case COLUMN_HOUR:
        selectColumn(COLUMN_MINUTE);
        return TRUE;
    case COLUMN_MINUTE:
        return FALSE;
    }
    return FALSE;
}

// .text:0x2F44
void dMenuTime00_c::selectColumn(int column) {
    if (mColumn != COLUMN_NONE && mColumn != column) {
        finishColumn();
    }
    if (mColumn != COLUMN_NONE) {
        mDigitAnm[mColumn * 2].start(DIGIT_ANM_NORMAL);
        mDigitAnm[mColumn * 2 + 1].start(DIGIT_ANM_NORMAL);
    }
    mColumn = column;
    if (column != COLUMN_NONE) {
        setSelectFrame(column, TRUE);
        mDigitAnm[mColumn * 2].start(DIGIT_ANM_SELECTED);
        mDigitAnm[mColumn * 2 + 1].start(DIGIT_ANM_SELECTED);
        if (mMode == MODE_MONTH_DAY) {
            mField = column - COLUMN_DAY;
        } else {
            mField = column;
        }
    } else {
        mField = FIELD_NONE;
    }
    mDigits = 0;
    onFlag(FLAG_CALC);
}

// .text:0x3068
void dMenuTime00_c::finishColumn() {
    if (mMode == MODE_DATE) {
        updateColumn(COLUMN_YEAR);
        if (mInput[COLUMN_MONTH] == 0) {
            mInput[COLUMN_MONTH] = 1;
            updateColumn(COLUMN_MONTH);
        }
        if (mInput[COLUMN_DAY] == 0) {
            mInput[COLUMN_DAY] = 1;
            updateColumn(COLUMN_DAY);
        }
    }
    if (mMode == MODE_MONTH_DAY) {
        if (mInput[COLUMN_HOUR] == 0) {
            mInput[COLUMN_HOUR] = 1;
            updateColumn(COLUMN_HOUR);
        }
        if (mInput[COLUMN_MINUTE] == 0) {
            mInput[COLUMN_MINUTE] = 1;
            updateColumn(COLUMN_MINUTE);
        }
    }
    clampDay();
    if (mColumn != COLUMN_NONE) {
        mDigitAnm[mColumn * 2].start(DIGIT_ANM_NORMAL);
        mDigitAnm[mColumn * 2 + 1].start(DIGIT_ANM_NORMAL);
    }
    setSelectFrame(mColumn, FALSE);
}

// .text:0x318C
void dMenuTime00_c::initAnms() {
    static const char *questAnmNames[3] = {
        "dayTime_win_00_questFrm_S_Psh.brlan",
        "dayTime_win_00_questFrm_S_NoPsh.brlan",
        "dayTime_win_00_questFrm_S_CanPsh.brlan"};
    static const char *inputAnmNames[3] = {
        "dayTime_win_00_T_input_Gry.brlan",
        "dayTime_win_00_T_input_Black.brlan",
        "dayTime_win_00_T_input_Black.brlan"};
    static const char *weekdayAnmNames[3] = {"dayTime_win_00_jp_youbi.brlan",
                                             "dayTime_win_00_us_youbi.brlan",
                                             "dayTime_win_00_eu_youbi.brlan"};
    mLyt.setAnm(&mPushAnm, "dayTime_win_00_questFrm_Psh.brlan", nullptr);
    mPushAnm.setToEnd();
    mAmPmAnm[0].init(&mLyt, &mRes, questAnmNames, 3);
    mAmPmAnm[0].setNumber(AMPM_PANE_FIRST);
    mAmPmAnm[1].init(&mLyt, &mRes, questAnmNames, 3);
    mAmPmAnm[1].setNumber(AMPM_PANE_FIRST + 1);
    if (getConsoleLanguage() == LANGUAGE_US ||
        getConsoleLanguage() == LANGUAGE_EN) {
        mLyt.setAnm(&mArrowAnm, "dayTime_win_00_input_S_R.brlan", nullptr);
    } else {
        mLyt.setAnm(&mArrowAnm, "dayTime_win_00_input_S_L.brlan", nullptr);
    }
    mArrowAnm.setToEnd();
    for (int i = 0; i < COLUMN_NUM * 2; i++) {
        mDigitAnm[i].init(&mLyt, &mRes, inputAnmNames, 3);
        mDigitAnm[i].setNumber(i);
        mDigitAnm[i].start(DIGIT_ANM_NORMAL);
        mDigitAnm[i].setToEnd();
    }
    mLyt.setAnm(&mIconAnm, "dayTime_win_00_icnPattern.brlan", nullptr);
    switch (mMenuNo) {
    case dMenuMng_c::MENU_BIRTHDAY:
        mIconAnm.setFrame(ICON_BIRTHDAY);
        break;
    case dMenuMng_c::MENU_FUTURE_MAIL_DATE:
        mIconAnm.setFrame(ICON_FUTURE_MAIL);
        break;
    case dMenuMng_c::MENU_SET_CLOCK:
    case dMenuMng_c::MENU_APPOINTMENT_TIME:
        mIconAnm.setFrame(ICON_CLOCK);
        break;
    }
    int weekdayAnm = WEEKDAY_ANM_JP;
    switch (getConsoleRegion()) {
    case CONSOLE_REGION_NA:
        if (isDayFirst()) {
            weekdayAnm = WEEKDAY_ANM_EU;
        } else {
            weekdayAnm = WEEKDAY_ANM_US;
        }
        break;
    case CONSOLE_REGION_EU:
        weekdayAnm = WEEKDAY_ANM_EU;
        break;
    }
    mLyt.setAnm(&mWeekdayAnm, weekdayAnmNames[weekdayAnm], nullptr);
    mWeekdayAnm.setToEnd();
}

// .text:0x33A0
void dMenuTime00_c::calcAnms() {
    BOOL playing = FALSE;
    for (int i = 0; i < COLUMN_NUM * 2; i++) {
        playing |= mDigitAnm[i].play();
    }
    if (!mPushAnm.mFrameCtrl.isStop()) {
        mPushAnm.play();
        playing = TRUE;
    }
    playing |= mAmPmAnm[0].play();
    playing |= mAmPmAnm[1].play();
    if (!mQuestWinAnm.mFrameCtrl.isStop()) {
        mQuestWinAnm.play();
        playing = TRUE;
    }
    if (playing) {
        onFlag(FLAG_CALC);
    }
    calcClock();
}

// .text:0x3474
void dMenuTime00_c::pushColumn(int column) {
    mPushAnm.setFrame(0.0f);
    char name[32];
    sprintf(name, "N_input_%02d", column);
    mLyt.setPosFrom(name, "questFrm_Psh", 0.0f, -25.0f);
    playSeDecide2();
}

// .text:0x34FC
BOOL dMenuTime00_c::pushAmPm(int idx) {
    if (idx == mAmPm) {
        return FALSE;
    }
    mAmPmAnm[idx].setNext(AMPM_ANM_PUSH);
    playSeDecide2();
    return TRUE;
}

// .text:0x3548
void dMenuTime00_c::setQuestWinAnm(int anm) {
    static const char *questWinAnmNames[2] = {"dayTime_win_00_cmnQ_In.brlan",
                                              "dayTime_win_00_cmnQ_Out.brlan"};
    mLyt.setAnm(&mQuestWinAnm, questWinAnmNames[anm], nullptr);
}

// .text:0x3568
void dMenuTime00_c::setHoverFrame(int column, BOOL show) {
    if (show) {
        if (mHoverColumn == COLUMN_NONE) {
            mLyt.setVisible(l_hoverFrame, true);
        }
        char name[40];
        sprintf(name, "N_input_%02d", column);
        mLyt.setPosFrom(name, l_hoverFrame, 0.0f, -23.0f);
        mHoverColumn = column;
    } else if (column == mHoverColumn) {
        mLyt.setVisible(l_hoverFrame, false);
        mHoverColumn = COLUMN_NONE;
    }
    onFlag(FLAG_CALC);
}

// .text:0x363C
void dMenuTime00_c::setSelectFrame(int column, BOOL show) {
    if (show) {
        if (mSelectColumn == COLUMN_NONE) {
            mLyt.setVisible(l_selectFrame, true);
        }
        char name[40];
        sprintf(name, "N_input_%02d", column);
        mLyt.setPosFrom(name, l_selectFrame, 0.0f, -23.0f);
        mSelectColumn = column;
    } else if (column == mSelectColumn) {
        mLyt.setVisible(l_selectFrame, false);
        mSelectColumn = COLUMN_NONE;
    }
    onFlag(FLAG_CALC);
}

// .text:0x3710
void dMenuTime00_c::setAmPmFrame(int idx, BOOL show) {
    if (show) {
        if (mAmPmHover == AMPM_NONE) {
            mLyt.setVisible(l_amPmFrame, true);
        }
        char name[40];
        sprintf(name, "N_input_%02d", idx + AMPM_PANE_FIRST);
        mLyt.setPosFrom(name, l_amPmFrame, 0.0f, -1.0f);
        mAmPmHover = idx;
    } else if (idx == mAmPmHover) {
        mLyt.setVisible(l_amPmFrame, false);
        mAmPmHover = AMPM_NONE;
    }
    onFlag(FLAG_CALC);
}

// .text:0x37E4
BOOL dMenuTime00_c::inputDigit(int digit) {
    if (mDigits < 0 || mDigits > DIGITS_PER_COLUMN) {
        return FALSE;
    }
    if (digit == dMenuNumber_c::DIGIT_CLEAR) {
        clearColumn(mColumn);
        selectColumn(mColumn);
        updateColumn(mColumn);
        return TRUE;
    }
    if (checkDigitRange(digit)) {
        return FALSE;
    }
    if (isColumnFull(digit)) {
        if (!selectNextColumn()) {
            selectColumn(COLUMN_NONE);
            updateClockTarget();
            return TRUE;
        }
    }
    applyDigit(digit);
    return TRUE;
}

// .text:0x38C8
BOOL dMenuTime00_c::checkDigitRange(int digit) {
    static u16 dayLimitMsgs[MONTH_NUM] = {
        MSG_ERR_DAYS_JAN, MSG_ERR_DAYS_FEB, MSG_ERR_DAYS_MAR, MSG_ERR_DAYS_APR,
        MSG_ERR_DAYS_MAY, MSG_ERR_DAYS_JUN, MSG_ERR_DAYS_JUL, MSG_ERR_DAYS_AUG,
        MSG_ERR_DAYS_SEP, MSG_ERR_DAYS_OCT, MSG_ERR_DAYS_NOV, MSG_ERR_DAYS_DEC};
    if (mDigits == 0) {
        return FALSE;
    }
    if (mDigits == 1) {
        int first = mInput[mColumn];
        int value = digit + first * 10;
        if ((mField == FIELD_MONTH || mField == FIELD_DAY) && digit == 0 &&
            first == 0) {
            return TRUE;
        }
        if (mField == FIELD_DAY) {
            int maxDay = getMaxDay();
            if (value > maxDay) {
                int month;
                switch (mMenuNo) {
                case dMenuMng_c::MENU_BIRTHDAY:
                    month = mInput[COLUMN_HOUR];
                    break;
                case dMenuMng_c::MENU_SET_CLOCK:
                case dMenuMng_c::MENU_FUTURE_MAIL_DATE:
                    month = mInput[COLUMN_MONTH];
                    break;
                }
                if (maxDay == 29) {
                    openRangeError(MSG_ERR_DAYS_FEB_LEAP, value);
                } else {
                    openRangeError(dayLimitMsgs[month - 1], value);
                }
                return TRUE;
            }
        }
        if (mField == FIELD_MINUTE && value >= MINUTES_PER_HOUR) {
            openRangeError(MSG_ERR_MINUTE, value);
            return TRUE;
        }
        if (mField == FIELD_YEAR && value > YEAR_MAX) {
            openRangeError(MSG_ERR_YEAR, value);
            return TRUE;
        }
        if (mField == FIELD_MONTH && value > MONTH_NUM) {
            openRangeError(MSG_ERR_MONTH, value);
            return TRUE;
        }
        if (mField == FIELD_HOUR) {
            if (uses12HourClock()) {
                if (value > HOURS_PER_HALF_DAY) {
                    openRangeError(MSG_ERR_HOUR, value);
                    return TRUE;
                }
            } else if (value > HOUR_MAX) {
                openRangeError(MSG_ERR_HOUR, value);
                return TRUE;
            }
        }
        return FALSE;
    }
    if (mDigits == 2) {
        if (mField == FIELD_MINUTE || mField == FIELD_DAY) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x3AF0
BOOL dMenuTime00_c::isColumnFull(int digit) {
    if (mDigits == 2) {
        return TRUE;
    }
    if (mDigits == 1) {
        if (mField == FIELD_MONTH) {
            int first = mInput[mColumn];
            if (first > 1) {
                return TRUE;
            }
            if (first == 1 && digit > 2) {
                return TRUE;
            }
        }
        if (mField == FIELD_HOUR) {
            if (uses12HourClock()) {
                int first = mInput[mColumn];
                if (first > 1 || (first == 1 && digit > 2)) {
                    return TRUE;
                }
            } else {
                int first = mInput[mColumn];
                if (first > 2 || (first == 2 && digit > 3)) {
                    return TRUE;
                }
            }
        }
        if (mField == FIELD_DAY && mDigits == 1) {
            if (getMaxDay() < mInput[mColumn] * 10) {
                return TRUE;
            }
        }
        if (mField == FIELD_MINUTE && mDigits == 1) {
            if (mInput[mColumn] >= MINUTES_PER_HOUR / 10) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// .text:0x3C70
void dMenuTime00_c::applyDigit(int digit) {
    if (mDigits == 0) {
        mInput[mColumn] = digit;
        mDigitAnm[mColumn * 2 + 1].start(DIGIT_ANM_FLASH);
        offFlag(FLAG_COLUMN_FIRST + mColumn);
    } else if (mDigits == 1) {
        mInput[mColumn] = digit + mInput[mColumn] * 10;
        onFlag(FLAG_COLUMN_FIRST + mColumn);
    } else {
        return;
    }
    mDigits++;
    updateColumn(mColumn);
    if ((mField == FIELD_YEAR && mDigits == 2) ||
        (mField == FIELD_MONTH && (mDigits != 1 || digit != 0))) {
        clampDay();
    }
    if (isColumnFull(0)) {
        if (!selectNextColumn()) {
            selectColumn(COLUMN_NONE);
        }
        updateClockTarget();
    }
}

// .text:0x3DB8
int dMenuTime00_c::getMaxDay() {
    switch (mMenuNo) {
    case dMenuMng_c::MENU_BIRTHDAY: {
        int month = mInput[COLUMN_HOUR];
        if (month == 0) {
            return DAYS_MAX;
        }
        return dTime_c::getDaysInMonth(BIRTHDAY_YEAR, month - 1);
    }
    case dMenuMng_c::MENU_SET_CLOCK:
    case dMenuMng_c::MENU_FUTURE_MAIL_DATE: {
        int month = mInput[COLUMN_MONTH];
        if (month == 0) {
            return DAYS_MAX;
        }
        return dTime_c::getDaysInMonth(mInput[COLUMN_YEAR] + YEAR_BASE,
                                       month - 1);
    }
    case dMenuMng_c::MENU_APPOINTMENT_TIME:
    default:
        return DAYS_MAX;
    }
}

// .text:0x3E30
void dMenuTime00_c::clampDay() {
    int maxDay = getMaxDay();
    switch (mMenuNo) {
    case dMenuMng_c::MENU_BIRTHDAY:
        if (mInput[COLUMN_MINUTE] > maxDay) {
            mInput[COLUMN_MINUTE] = maxDay;
            updateColumn(COLUMN_MINUTE);
        }
        break;
    case dMenuMng_c::MENU_SET_CLOCK:
    case dMenuMng_c::MENU_FUTURE_MAIL_DATE:
        if (mInput[COLUMN_DAY] > maxDay) {
            mInput[COLUMN_DAY] = maxDay;
            updateColumn(COLUMN_DAY);
        }
        break;
    }
}

// .text:0x3EC0
int dMenuTime00_c::getConsoleRegion() {
    return getRegion();
}

// .text:0x3EC4
int dMenuTime00_c::getConsoleLanguage() {
    return getLanguage();
}

// .text:0x3EC8
BOOL dMenuTime00_c::isDayFirst() {
    switch (getConsoleLanguage()) {
    case LANGUAGE_MX:
    case LANGUAGE_QC:
    case LANGUAGE_EN:
    case LANGUAGE_ES:
    case LANGUAGE_FR:
    case LANGUAGE_IT:
    case LANGUAGE_DE:
        return TRUE;
    }
    return FALSE;
}

// .text:0x3F08
BOOL dMenuTime00_c::uses12HourClock() {
    if (getConsoleRegion() == CONSOLE_REGION_EU) {
        return getConsoleLanguage() == LANGUAGE_EN;
    }
    if (getConsoleRegion() == CONSOLE_REGION_NA) {
        return getConsoleLanguage() == LANGUAGE_US;
    }
    return TRUE;
}

// .text:0x3F68
void dMenuTime00_c::setAmPm(int ampm, BOOL immediate) {
    mAmPm = ampm;
    int anm = ampm == AMPM_AM ? AMPM_ANM_ON : AMPM_ANM_OFF;
    mAmPmAnm[0].start(anm);
    m2d::AnmEx2_c *anm1 = &mAmPmAnm[1];
    anm = AMPM_ANM_OFF;
    if (ampm == AMPM_PM) {
        anm = AMPM_ANM_ON;
    }
    anm1->start(anm);
    if (immediate) {
        mAmPmAnm[0].setToEnd();
        mAmPmAnm[1].setToEnd();
    }
}

// .text:0x3FF4
void dMenuTime00_c::setWinAnm(int anm) {
    m2d::ResAccIf_c *resAcc = dMenuMng_c::getWinResAcc();
    mLyt.setAnm(&mWinAnm, dMenuBase_c::getAnmName(anm), resAcc);
}

// .text:0x4054
void dMenuTime00_c::setClockHands(int hour, int minute) {
    mVec3_c rot;
    rot.x = 0.0f;
    rot.y = 0.0f;
    rot.z = -(0.1f * (minute * 60));
    mpMinuteHand->SetRotate(rot);
    if (hour > HOURS_PER_HALF_DAY) {
        hour -= HOURS_PER_HALF_DAY;
    }
    rot.z = -((hour * 60 + minute) * 60 / 120.0f);
    mpHourHand->SetRotate(rot);
}

// .text:0x4118
void dMenuTime00_c::setClockHandsAt(int ticks) {
    int minutes = ticks / CLOCK_MINUTE;
    setClockHands(minutes / MINUTES_PER_HOUR, minutes % MINUTES_PER_HOUR);
}

// .text:0x4154
void dMenuTime00_c::resetClock() {
    mClockPos = getClockTicks();
    mClockTarget = getClockTicks();
    mClockSpeed = 0;
    setClockHandsAt(mClockPos);
}

// .text:0x41A4
void dMenuTime00_c::updateClockTarget() {
    if (mMode == MODE_DATE || mMode == MODE_MONTH_DAY) {
        return;
    }
    mClockTarget = getClockTicks();
    if (mClockTarget == mClockPos) {
        return;
    }
    while (mClockTarget <= mClockPos) {
        mClockTarget += MINUTES_PER_HALF_DAY * CLOCK_MINUTE;
    }
    mClockSpeed = (mClockTarget - mClockPos) / CLOCK_STEPS;
    if (mClockSpeed > CLOCK_SPEED_MAX) {
        mClockSpeed = CLOCK_SPEED_MAX;
    } else if (mClockSpeed < CLOCK_SPEED_MIN) {
        mClockSpeed = CLOCK_SPEED_MIN;
    }
}

// .text:0x4268
int dMenuTime00_c::getClockTicks() {
    int minutes =
        mInput[COLUMN_HOUR] * MINUTES_PER_HOUR + mInput[COLUMN_MINUTE];
    if (uses12HourClock() && mAmPm == AMPM_PM) {
        minutes += MINUTES_PER_HALF_DAY;
    }
    return minutes << CLOCK_FRAC_BITS;
}

// .text:0x42C8
void dMenuTime00_c::calcClock() {
    if (mClockSpeed > 0) {
        mClockPos += mClockSpeed;
        if (mClockTarget <= mClockPos) {
            resetClock();
            playSe(SE_Register);
        } else {
            setClockHandsAt(mClockPos);
            holdSe(SE_Lev_TokeiKaiten);
        }
        onFlag(FLAG_CALC);
    }
}
