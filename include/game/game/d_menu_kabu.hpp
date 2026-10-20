#ifndef GAME_D_MENU_KABU_HPP
#define GAME_D_MENU_KABU_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_number.hpp>

// The amount prompt of the turnip dealings and a few other number entries (REL
// d_menu_kabuNP, profile MENU_KABU): the number pad (dMenuNumber_c) below a
// small window (Layout/kabu.arc, common_unit_win_00) that shows the amount
// typed so far, with a cancel and a confirm button. The menu number selects the
// question: how many turnips (0x45), the turnip price (0x46), the donation to
// the public works (0x47), and the auction's minimum price (0x48) and bid
// (0x49). RTTI confirms the global class name; everything else is inferred.
class dMenuKabu_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at: the twelve number keys, then the
    // three buttons.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_KEY = 0, // 0..11: dMenuNumber_c::KEY_e
        CURSOR_KEY_LAST = dMenuNumber_c::KEY_NUM - 1,
        CURSOR_BTN = dMenuNumber_c::KEY_NUM, // 12..14: BTN_e
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_CANCEL,
        BTN_UNUSED, // hidden
        BTN_CONFIRM,
    };

    // The window's animations (startWinAnm).
    enum WIN_ANM_e {
        WIN_ANM_IN,
        WIN_ANM_OUT,
        WIN_ANM_NUM,
    };

    // The frame of the icon animation: the picture on the window.
    enum ICON_e {
        ICON_TURNIP,
        ICON_PRICE,
        ICON_DONATION,
        ICON_AUCTION,
    };

    // Which "T_input_xx" pane shows the amount (mFormat).
    enum FORMAT_e {
        FORMAT_COUNT,
        FORMAT_BELLS,
        FORMAT_AUCTION,
        FORMAT_NUM,
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
        RESULT_CONFIRM = 1,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,  // the window layout needs a calc
        FLAG_NO_SE, // the cursor was moved by a key press: no cursor sound
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH, // waits for the pushed key / button to finish
        SUB_NUM,
    };

    // The message codes of the 2D menu messages (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_HOW_MANY = 0x59,
        MSG_TURNIPS = 0x5A,
        MSG_BELLS = 0x62,
        MSG_PRICE = 0x101,
        MSG_THANKS = 0x102,
        MSG_BID = 0x106,
        MSG_MIN_PRICE = 0x107,
    };

    enum {
        MAX_COUNT = 99,
        MAX_PRICE = 9999999,
        MAX_AUCTION = 99999,
        PUBLIC_WORKS_MAX = 1000000,
    };

    static bool isKey(int cursor) {
        return cursor >= CURSOR_KEY && cursor <= CURSOR_KEY_LAST;
    }
    static bool isButton(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }

    virtual int create();                    // .text:0xF0
    virtual int doDelete();                  // .text:0x328
    virtual int stateMain();                 // .text:0x394
    virtual int stateAnm();                  // .text:0x3B8
    virtual int stateClose();                // .text:0x3BC
    virtual int execute();                   // .text:0x3E0
    virtual int draw();                      // .text:0x4E4
    void subStateMain();                     // .text:0x538
    void subStatePush();                     // .text:0x6CC
    void executeSubState();                  // .text:0x708
    void setSelect(int cursor, BOOL select); // .text:0x740
    int hitCheck();                          // .text:0x7E8
    void pushCursor();                       // .text:0x860
    BOOL isPushEnd();                        // .text:0x94C
    void finishPush();                       // .text:0x9D8
    void startCloseAnm();                    // .text:0xB24
    void initButtons();                      // .text:0xB7C
    void setConfirmDisable(BOOL disable);    // .text:0xBDC
    void startWinAnm(int anm);               // .text:0xBF8
    void calcWinAnm();                       // .text:0xC18
    void updateDisplay();                    // .text:0xC7C
    BOOL inputDigit(int digit);              // .text:0xD78
    void initLayout();                       // .text:0xDC4

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x1004
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0xFFC
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0xFF8
    virtual void deleteReady() {
    } // .text:0xFF4
    virtual bool createHeap() {
        return true;
    } // .text:0xFEC

    // Implicit destructor: .text:0xF3C.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::Anm_c mWinAnm;
    /* 0x264 */ m2d::Anm_c mIconAnm;
    /* 0x288 */ dMenuBtn_c mBtn;
    /* 0x7CC */ dMenuNumber_c mNumber;
    /* 0xFF4 */ int mCursor;
    /* 0xFF8 */ int mValue;   // the amount typed so far
    /* 0xFFC */ int mMax;     // the most the amount may be
    /* 0x1000 */ int mMin;    // the least the amount may be to confirm
    /* 0x1004 */ int mDigits; // the amount's digits
    /* 0x1008 */ int mFormat; // FORMAT_e
}; // size 0x100C

#endif
