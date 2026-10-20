#ifndef GAME_D_MENU_TIME00_HPP
#define GAME_D_MENU_TIME00_HPP

#include <game/game/d_date.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_number.hpp>
#include <game/game/d_menu_slct_b.hpp>

// The date and time entry menu (REL d_menu_time00NP, profile MENU_TIME00): a
// window (Layout/dayTime.arc, dayTime_win_00.brlyt) with up to five two digit
// columns (year, month, day, hour, minute), an analogue clock, the shared
// number pad and button bar, and the message box (dMenuSlctB_c) for the
// confirmations and the range errors. One menu for four requests, selected by
// mMenuNo (MENU_SET_CLOCK, MENU_BIRTHDAY, MENU_FUTURE_MAIL_DATE,
// MENU_APPOINTMENT_TIME). RTTI confirms the global class name; everything else
// is inferred.
class dMenuTime00_c : public dMenuBase_c {
  public:
    // The cursor (mCursor): 0..11 the number pad's keys
    // (dMenuNumber_c::KEY_e), 12..14 the button bar's buttons, 15..19 the
    // columns, 20..21 the a.m. / p.m. labels.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_KEY_FIRST = 0,
        CURSOR_KEY_LAST = dMenuNumber_c::KEY_NUM - 1,
        CURSOR_BTN_FIRST = dMenuNumber_c::KEY_NUM,
        CURSOR_BTN_LAST = CURSOR_BTN_FIRST + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_COLUMN_FIRST = CURSOR_BTN_LAST + 1,
        CURSOR_COLUMN_LAST = CURSOR_COLUMN_FIRST + 4,
        CURSOR_AMPM_FIRST = CURSOR_COLUMN_LAST + 1,
        CURSOR_AMPM_LAST = CURSOR_AMPM_FIRST + 1,
    };

    // The button bar's buttons.
    enum BTN_e {
        BTN_CANCEL,
        BTN_UNUSED, // hidden
        BTN_OK,
    };

    // The columns (mInput). MODE_MONTH_DAY keeps the month and the day in the
    // hour and minute columns.
    enum COLUMN_e {
        COLUMN_NONE = -1,
        COLUMN_YEAR, // years since 2000
        COLUMN_MONTH,
        COLUMN_DAY,
        COLUMN_HOUR,
        COLUMN_MINUTE,
        COLUMN_NUM,
    };

    // What the selected column edits (mField).
    enum FIELD_e {
        FIELD_NONE = -1,
        FIELD_YEAR,
        FIELD_MONTH,
        FIELD_DAY,
        FIELD_HOUR,
        FIELD_MINUTE,
    };

    // The layout (mMode).
    enum MODE_e {
        MODE_DATE,        // year, month and day
        MODE_TIME_FIRST,  // hour and minute, a.m. / p.m. in front (Japan)
        MODE_TIME_12H,    // hour and minute, a.m. / p.m. behind (America)
        MODE_TIME_24H,    // hour and minute
        MODE_TIME_KOREAN, // like MODE_TIME_FIRST
        MODE_MONTH_DAY,   // month and day, without a year
        MODE_NUM,
    };

    // What the message box asked (mDialog).
    enum DIALOG_e {
        DIALOG_DATE,      // MENU_SET_CLOCK: the date, then the time follows
        DIALOG_FINAL,     // everything entered: set it and leave
        DIALOG_CANCELLED, // No, or the error message was dismissed
    };

    // The message box's buttons.
    enum DIALOG_BTN_e {
        DIALOG_BTN_NO,
        DIALOG_BTN_YES,
        DIALOG_BTN_OK, // the error messages
    };

    // The a.m. / p.m. labels (mAmPm; AMPM_NONE until the clock is read).
    enum AMPM_e {
        AMPM_NONE = -1,
        AMPM_AM,
        AMPM_PM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,             // the layout needs a calc
        FLAG_DIALOG,           // the message box is open
        FLAG_DIALOG_ERROR,     // ... it is an error message
        FLAG_WEEKDAY,          // the weekday name needs an update
        FLAG_COLUMN_FIRST,     // 5..9: the column's tens digit was typed
        FLAG_NO_CURSOR_SE = 10 // no cursor se when the selection changes
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,         // waits for the pushed key / button / column
        SUB_NEXT_PAGE,    // the date is accepted: build the time page
        SUB_PAGE_IN,      // the next page comes in
        SUB_DIALOG_INPUT, // waits for the message box to open, then its button
        SUB_DIALOG_PUSH,  // waits for the pushed button
        SUB_DIALOG_CLOSE, // waits for the box to close
        SUB_NUM,
    };

    // Animations of the digit columns (mDigitAnm.start, the table order of
    // dayTime_win_00_T_input_*).
    enum DIGIT_ANM_e {
        DIGIT_ANM_SELECTED, // _Gry
        DIGIT_ANM_NORMAL,   // _Black
        DIGIT_ANM_FLASH,    // _Black again: plays from the start
    };

    // Animations of the a.m. / p.m. labels (mAmPmAnm.start, the table order of
    // dayTime_win_00_questFrm_S_*).
    enum AMPM_ANM_e {
        AMPM_ANM_PUSH, // _Psh
        AMPM_ANM_OFF,  // _NoPsh
        AMPM_ANM_ON,   // _CanPsh
    };

    // Animations of the question window (mQuestWinAnm).
    enum QUEST_WIN_ANM_e {
        QUEST_WIN_ANM_IN,
        QUEST_WIN_ANM_OUT,
    };

    // The frame of the icon animation (mIconAnm): the picture on the window.
    enum ICON_e {
        ICON_BIRTHDAY,
        ICON_FUTURE_MAIL,
        ICON_CLOCK,
    };

    // The weekday names' animation (mWeekdayAnm) by region.
    enum WEEKDAY_ANM_e {
        WEEKDAY_ANM_JP,
        WEEKDAY_ANM_US,
        WEEKDAY_ANM_EU,
    };

    // Word slots (dString::setWord) the confirmation messages read. The
    // appointment message has its own set.
    enum WORD_e {
        WORD_APPT_DAY = 0,
        WORD_APPT_AMPM = 1,
        WORD_APPT_HOUR = 2,
        WORD_APPT_MINUTE = 3,
        WORD_YEAR = 2,
        WORD_MONTH = 3,
        WORD_DAY = 4,
        WORD_HOUR = 5,
        WORD_MINUTE = 6,
        WORD_AMPM = 7,
        WORD_WEEKDAY = 8,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_ENTER_DATE = 0x3A,
        MSG_ENTER_TIME = 0x3B,
        MSG_YEAR = 0x3C,
        MSG_MONTH = 0x3D,
        MSG_DAY = 0x3E,
        MSG_HOUR = 0x3F,
        MSG_MINUTE = 0x40,
        MSG_ERR_YEAR = 0x41, // "You can only set the years from 2000 to 2035."
        // "There are only 31 days in January." ...
        MSG_ERR_DAYS_JAN = 0x48,
        MSG_ERR_DAYS_FEB = 0x49,
        MSG_ERR_DAYS_FEB_LEAP = 0x4A,
        MSG_ERR_DAYS_MAR = 0x4B,
        MSG_ERR_DAYS_APR = 0x4C,
        MSG_ERR_DAYS_MAY = 0x4D,
        MSG_ERR_DAYS_JUN = 0x4E,
        MSG_ERR_DAYS_JUL = 0x4F,
        MSG_ERR_DAYS_AUG = 0x50,
        MSG_ERR_DAYS_SEP = 0x51,
        MSG_ERR_DAYS_OCT = 0x52,
        MSG_ERR_DAYS_NOV = 0x53,
        MSG_ERR_DAYS_DEC = 0x54,
        MSG_ERR_MINUTE = 0x55,        // "up to 59 minutes"
        MSG_CONFIRM_DATE_FULL = 0x56, // "Is Wednesday, March 5, 2008, correct?"
        MSG_CONFIRM_MONTH_DAY = 0x57, // "Is March 5 right?"
        MSG_CONFIRM_TIME = 0x58,      // "Is 12:00 p.m. the correct time?"
        MSG_ERR_MONTH = 0x65,         // "up through December"
        MSG_ERR_HOUR = 0x66,          // "up to 12 o'clock"
        MSG_AM = 0x87,                // "a.m."
        MSG_PM = 0x88,                // "p.m."
        MSG_CONFIRM_APPOINTMENT = 0xA9, // "Is 12:00 p.m. on the 5th OK?"
        MSG_OK = 0xEC,
        MSG_SET_TIME = 0x105,       // "Please set a time!"
        MSG_ERR_DATE_PAST = 0x142,  // "tomorrow or after"
        MSG_ERR_TIME_RANGE = 0x143, // "less than 12 hours from now"
        MSG_BIRTHDAY = 0x158,       // "When is your birthday?"
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
        RESULT_OK = 1,
    };

    // The labels' panes are N_input_05 and N_input_06.
    enum {
        AMPM_PANE_FIRST = COLUMN_NUM,
    };

    enum {
        YEAR_BASE = 2000,
        YEAR_MAX = 35, // 2035
        HOUR_MAX = 23,
        HOURS_PER_HALF_DAY = 12,
        MINUTES_PER_HOUR = 60,
        MINUTE_MAX = 59,
        DAYS_MAX = 31,
        FUTURE_MAIL_HOUR = 9,   // a letter to the future arrives at 9:00
        APPOINTMENT_HOURS = 12, // MENU_APPOINTMENT_TIME: at most this far ahead
        BIRTHDAY_YEAR = 2004,   // a leap year: February has 29 days
        MINUTES_PER_HALF_DAY = 720,
        CLOCK_FRAC_BITS =
            8, // the clock positions are minutes in 24.8 fixed point
        CLOCK_MINUTE = 1 << CLOCK_FRAC_BITS,
        DIGITS_PER_COLUMN = 2,
        CLOCK_STEPS =
            30, // the hands turn to their target in about this many frames
        CLOCK_SPEED_MAX = 3000,
        CLOCK_SPEED_MIN = 200,
    };

    static bool isKey(int slot) {
        return slot >= CURSOR_KEY_FIRST && slot <= CURSOR_KEY_LAST;
    }
    static bool isBtn(int slot) {
        return slot >= CURSOR_BTN_FIRST && slot <= CURSOR_BTN_LAST;
    }
    static bool isColumn(int slot) {
        return slot >= CURSOR_COLUMN_FIRST && slot <= CURSOR_COLUMN_LAST;
    }
    static bool isAmPm(int slot) {
        return slot >= CURSOR_AMPM_FIRST && slot <= CURSOR_AMPM_LAST;
    }

    virtual int create();     // .text:0x188
    virtual int doDelete();   // .text:0x48C
    virtual int stateMain();  // .text:0x510
    virtual int stateAnm();   // .text:0x534
    virtual int stateClose(); // .text:0x5E8
    virtual int execute();    // .text:0x60C
    virtual int draw();       // .text:0x770

    void subStateMain();                                      // .text:0x7E0
    void subStatePush();                                      // .text:0x92C
    void subStateNextPage();                                  // .text:0x968
    void subStatePageIn();                                    // .text:0x9E0
    void subStateDialogInput();                               // .text:0xA30
    void subStateDialogPush();                                // .text:0xA90
    void subStateDialogClose();                               // .text:0xB28
    void executeSubState();                                   // .text:0xE24
    void initButtons();                                       // .text:0xE5C
    void setSelect(int slot, BOOL select);                    // .text:0xEE8
    int hitCheck();                                           // .text:0x1058
    void push();                                              // .text:0x1144
    BOOL isPushEnd();                                         // .text:0x1280
    void finishPush();                                        // .text:0x1370
    void startCloseAnm();                                     // .text:0x1544
    void openErrorDialog(u16 msg);                            // .text:0x15A4
    void openRangeError(u16 msg, int value);                  // .text:0x1668
    static void setYearWord(int year);                        // .text:0x1704
    static void setMonthDayWords(int month, int day);         // .text:0x1764
    static void setWeekdayWord(int year, int month, int day); // .text:0x17E4
    static void setTimeWords(int hour, int minute, int hourSlot,
                             int minuteSlot);    // .text:0x184C
    static void setAmPmWord(int ampm, int slot); // .text:0x1930
    void openConfirmDialog(u16 msg);             // .text:0x19AC
    BOOL isFutureDate();                         // .text:0x1BEC
    void onOkPushed();                           // .text:0x1CD8
    BOOL isAppointmentValid();                   // .text:0x1E88
    void getInputTime(dTime_c *time);            // .text:0x20F8
    void setMode(int mode);                      // .text:0x21A4
    void setModeByRegion();                      // .text:0x21AC
    void refreshLayout();                        // .text:0x2240
    void updateModeLayout();                     // .text:0x227C
    void updateModeTexts();                      // .text:0x24E4
    void resetClockIfShown();                    // .text:0x2674 (+ dead blr)
    void initWords();                            // .text:0x2698
    void initClockInput();                       // .text:0x273C
    void initInputs();                           // .text:0x2820
    void clearColumn(int column);                // .text:0x2A0C
    void setLabel(int idx, u16 msg);             // .text:0x2A54
    void updateColumn(int column);               // .text:0x2AB4
    void selectFirstColumn();                    // .text:0x2C4C
    BOOL selectNextColumn();                     // .text:0x2D48
    void selectColumn(int column);               // .text:0x2F44
    void finishColumn();                         // .text:0x3068
    void initAnms();                             // .text:0x318C
    void calcAnms();                             // .text:0x33A0
    void pushColumn(int column);                 // .text:0x3474
    BOOL pushAmPm(int idx);                      // .text:0x34FC
    void setQuestWinAnm(int anm);                // .text:0x3548
    void setHoverFrame(int column, BOOL show);   // .text:0x3568
    void setSelectFrame(int column, BOOL show);  // .text:0x363C
    void setAmPmFrame(int idx, BOOL show);       // .text:0x3710
    BOOL inputDigit(int digit);                  // .text:0x37E4
    BOOL checkDigitRange(int digit);             // .text:0x38C8
    BOOL isColumnFull(int digit);                // .text:0x3AF0
    void applyDigit(int digit);                  // .text:0x3C70
    int getMaxDay();                             // .text:0x3DB8
    void clampDay();                             // .text:0x3E30
    static int getConsoleRegion();               // .text:0x3EC0
    static int getConsoleLanguage();             // .text:0x3EC4
    BOOL isDayFirst();                           // .text:0x3EC8
    BOOL uses12HourClock();                      // .text:0x3F08
    void setAmPm(int ampm, BOOL immediate);      // .text:0x3F68
    void setWinAnm(int anm);                     // .text:0x3FF4
    void setClockHands(int hour, int minute);    // .text:0x4054
    void setClockHandsAt(int ticks);             // .text:0x4118
    void resetClock();                           // .text:0x4154
    void updateClockTarget();                    // .text:0x41A4
    int getClockTicks();                         // .text:0x4268
    void calcClock();                            // .text:0x42C8

    virtual int state2() {
        return SUCCEEDED;
    }
    virtual int state3() {
        return SUCCEEDED;
    }
    virtual void postDraw(MAIN_STATE_e) {
    }
    virtual void deleteReady() {
    }
    virtual bool createHeap() {
        return true;
    }

    // Implicit destructor: .text:0x433C.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx2_c mDigitAnm[COLUMN_NUM * 2]; // two per column
    /* 0x4E8 */ m2d::Anm_c mPushAnm;                     // a column was clicked
    /* 0x50C */ m2d::Anm_c mWinAnm;
    /* 0x530 */ m2d::Anm_c mQuestWinAnm; // the question window in / out
    /* 0x554 */ m2d::Anm_c mArrowAnm;
    /* 0x578 */ m2d::AnmEx2_c mAmPmAnm[2];
    /* 0x600 */ m2d::Anm_c mIconAnm;
    /* 0x624 */ m2d::Anm_c mWeekdayAnm;
    /* 0x648 */ dMenuBtn_c mBtn;
    /* 0xB8C */ dMenuNumber_c mNumber;
    /* 0x13B4 */ m2d::stringWord_c mTitle;
    /* 0x14A8 */ m2d::stringWord_c mLabel[COLUMN_NUM];
    /* 0x196C */ dMenuSlctB_c mSlct;
    /* 0x205C */ nw4r::lyt::Pane *mpHourHand;   // N_short_00
    /* 0x2060 */ nw4r::lyt::Pane *mpMinuteHand; // N_long_00
    /* 0x2064 */ int mCursor;
    /* 0x2068 */ int mMode;
    /* 0x206C */ int mColumn; // the column being edited
    /* 0x2070 */ int mInput[COLUMN_NUM];
    /* 0x2084 */ int mDigits; // digits typed into the column
    /* 0x2088 */ int mField;
    /* 0x208C */ int mDialog;
    /* 0x2090 */ int mHoverColumn;
    /* 0x2094 */ int mSelectColumn;
    /* 0x2098 */ int mAmPmHover;
    /* 0x209C */ int mAmPm;
    /* 0x20A0 */ int mClockPos;
    /* 0x20A4 */ int mClockTarget;
    /* 0x20A8 */ int mClockSpeed;
    // No mVec3_c: the target's constructor does not construct them.
    /* 0x20AC */ nw4r::math::_VEC3 mColumnPos[COLUMN_NUM];
}; // size 0x20E8

#endif
