#ifndef GAME_D_MENU_BANK_HPP
#define GAME_D_MENU_BANK_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_number.hpp>

// The amount menu of the ATM (REL d_menu_bankNP, profile MENU_BANK): the money
// window (Layout/bank.arc) plus the shared number pad and button bar. One menu
// for the three transactions, selected by mMenuNo (MENU_BANK_DEPOSIT,
// MENU_BANK_WITHDRAW, MENU_BANK_PAY). RTTI confirms the global class name;
// everything else is inferred.
class dMenuBank_c : public dMenuBase_c {
  public:
    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,         // the money layout needs a calc
        FLAG_NO_CURSOR_SE, // no cursor se when the selection changes
    };

    enum SUB_STATE_e { SUB_MAIN, SUB_PUSH };

    // The cursor (mCursor): 0..11 the number pad's keys
    // (dMenuNumber_c::KEY_e), 12..14 the button bar's buttons.
    enum {
        CURSOR_NONE = -1,
        CURSOR_KEY_FIRST = 0,
        CURSOR_KEY_LAST = dMenuNumber_c::KEY_NUM - 1,
        CURSOR_BTN_FIRST = dMenuNumber_c::KEY_NUM,
    };

    // The button bar's buttons.
    enum BTN_e {
        BTN_CANCEL,
        BTN_UNUSED, // hidden
        BTN_OK,
    };

    enum WIN_ANM_e { WIN_ANM_IN, WIN_ANM_OUT, WIN_ANM_NUM };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_YOUR_BELLS = 0x5E,
        MSG_PAYMENT = 0x5F,
        MSG_REMAINING = 0x60,
        MSG_SAVINGS = 0x61,
        MSG_BELLS = 0x62, // the unit label next to each amount
        MSG_DEPOSIT = 0x63,
        MSG_WITHDRAWAL = 0x64,
    };

    // The transaction, from mMenuNo (initMessages' table index).
    enum TYPE_e { TYPE_DEPOSIT, TYPE_WITHDRAW, TYPE_PAY, TYPE_NUM };

    enum {
        MAX_MONEY = 999999999,
        MAX_DIGITS_VALUE = 99999999, // one more digit no longer fits
        NUMBER_DIGITS = 12,
    };

    static bool isKey(int slot) {
        return slot >= CURSOR_KEY_FIRST && slot <= CURSOR_KEY_LAST;
    }
    static bool isBtn(int slot) {
        return slot >= CURSOR_BTN_FIRST &&
               slot <= CURSOR_BTN_FIRST + dMenuBtn_c::BTN_NUM - 1;
    }

    virtual int create();     // .text:0xE8
    virtual int doDelete();   // .text:0x214
    virtual int stateMain();  // .text:0x280
    virtual int stateAnm();   // .text:0x2A4
    virtual int stateClose(); // .text:0x2A8
    virtual int execute();    // .text:0x2CC
    virtual int draw();       // .text:0x3D0

    void subStateMain();                   // .text:0x424
    void subStatePush();                   // .text:0x5B8
    void executeSubState();                // .text:0x5F4
    void setSelect(int slot, BOOL select); // .text:0x62C
    int hitCheck();                        // .text:0x6D4
    void push();                           // .text:0x74C
    bool isPushEnd();                      // .text:0x838
    void finishPush();                     // .text:0x8C4
    void startCloseAnm();                  // .text:0x9B4
    void initButtons();                    // .text:0xA0C
    void setOkDisable(int disable);        // .text:0xA6C
    void setWinAnm(int anm);               // .text:0xA88
    void calcWinAnm();                     // .text:0xAA8
    void initAmounts();                    // .text:0xB0C
    void applyAmount();                    // .text:0xBF4
    void updateNumbers();                  // .text:0xCA0
    BOOL addDigit(int digit);              // .text:0xD90
    void initMessages();                   // .text:0xDF4

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

    // Implicit destructor: .text:0xF50.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt; // the money window
    /* 0x240 */ m2d::Anm_c mWinAnm;
    /* 0x264 */ dMenuBtn_c mBtn;
    /* 0x7A8 */ dMenuNumber_c mNumber;
    /* 0xFD0 */ int mCursor;
    /* 0xFD4 */ int mWallet;    // the pocket money
    /* 0xFD8 */ int mAmount;    // the typed amount
    /* 0xFDC */ int mBalance;   // the savings, or the debt when paying
    /* 0xFE0 */ int mMaxAmount; // the most the amount can be
}; // size 0xFE4

#endif
