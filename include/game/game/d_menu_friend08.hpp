#ifndef GAME_D_MENU_FRIEND08_HPP
#define GAME_D_MENU_FRIEND08_HPP

#include <game/game/d_kbd.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_number.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <lib/dwc/dwc.h>

// The friend code entry (REL d_menu_friend08NP, profile MENU_FRIEND08):
// "What's the friend code?". The window of Layout/keyboard_add.arc
// (cmn_question_00.brlyt) shows a twelve digit number in three groups of four
// with a text cursor; the digits are typed on the shared number pad
// (dMenuNumber_c, in its MODE_FRIEND_CODE mode with the back key) or the USB
// keyboard, the pointer / d-pad moves the cursor. Confirm turns the digits into
// the friend key (u64), checks it (offline with dNetMgr_c::checkFriendKey, or
// through the network manager's requests) and fills the friend entry of
// dMenuFriend_c with it: the friend data comes from the network manager, and
// when the friend is in the current session the entry is filled from that
// player too. It is opened by the friend menu's tab bar (dMenuFriend_c) as
// MENU_FRIEND_CODE_ADD for a new friend (a name is asked next) or as
// MENU_FRIEND_CODE_EDIT to change the key of the friend that is being edited.
// RTTI confirms the global class name dMenuFriend08_c; everything else is
// inferred.
class dMenuFriend08_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_KEY = 0, // 0..11: the number pad's keys (dMenuNumber_c::KEY_e)
        CURSOR_KEY_LAST = dMenuNumber_c::KEY_NUM - 1,
        CURSOR_BTN = dMenuNumber_c::KEY_NUM, // 12..14: dMenuBtn_c buttons
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED, // hidden
        BTN_CONFIRM,
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,               // waits for the pushed key / button
        SUB_MESSAGE,            // a message box is open
        SUB_MESSAGE_PUSH,       // its OK button was pushed
        SUB_MESSAGE_CLOSE,      // waits for the message box to close
        SUB_CHECK_KEY,          // asks the network manager to check the key
        SUB_CHECK_KEY_RESULT,   // ... and waits for the answer
        SUB_KEY_DATA,           // asks for the friend data of the key
        SUB_KEY_DATA_RESULT,    // ... and waits for it
        SUB_FIND_FRIEND,        // looks the friend up in the friend list
        SUB_SEARCH_MEMBER,      // asks whether a session member has the key
        SUB_MEMBER_RESULT,      // ... and waits for the answer
        SUB_MEMBER_DATA,        // asks for the friend data of that member
        SUB_MEMBER_DATA_RESULT, // ... and waits for it
        SUB_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,         // the layout needs a calc
        FLAG_SLCT,         // the message box is drawn
        FLAG_REGISTER,     // register the friend entry (execute), then close
        FLAG_WORD,         // rewrite the digit words and the cursor
        FLAG_CUR,          // the text cursor is drawn
        FLAG_NO_CURSOR_SE, // no cursor se when the selection changes
        FLAG_HOLD,         // a key is held (the back key repeats)
    };

    enum {
        DIGIT_NUM = 12,
        DIGIT_EMPTY = -1,
        WORD_NUM = 3,
        WORD_DIGIT_NUM = DIGIT_NUM / WORD_NUM,
        KEY_REPEAT_FIRST = 20, // frames before a d-pad direction repeats
        KEY_REPEAT = 5,        // ... and between the repeats
        HOLD_FIRST = 30,       // frames before the held back key repeats
        HOLD_REPEAT = 10,
        INPUT_PANE_NUM = 5,
    };

    // The message box's buttons (dMenuSlctB_c): only the right one is used.
    enum SLCT_BTN_e {
        SLCT_BTN_LEFT,
        SLCT_BTN_MIDDLE,
        SLCT_BTN_RIGHT,
    };

    // Special keys of the USB keyboard (dMenuNumber_c::getSpecialKey).
    enum SPECIAL_KEY_e {
        SPECIAL_KEY_NONE,
        SPECIAL_KEY_LEFT,
        SPECIAL_KEY_RIGHT,
        SPECIAL_KEY_HOME,
        SPECIAL_KEY_END,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_WRONG_CODE = 0x9B, // "This friend code is incorrect and can't be
                               // registered!"
        MSG_WHATS_THE_CODE = 0x9C,     // "What's the friend code?"
        MSG_ALREADY_REGISTERED = 0xA1, // "That person is already registered!"
        MSG_OWN_CODE = 0xA2, // "You can't register your own friend code!"
        MSG_OK = 0xEC,       // "OK!"
    };

    typedef BOOL (dMenuFriend08_c::*CloseProc)();

    static bool isKey(int slot) {
        return slot >= CURSOR_KEY && slot <= CURSOR_KEY_LAST;
    }
    static bool isBtn(int slot) {
        return slot >= CURSOR_BTN && slot <= CURSOR_BTN_LAST;
    }

    virtual int create();     // .text:0x120
    virtual int doDelete();   // .text:0x2D4
    virtual int stateMain();  // .text:0x370
    virtual int stateAnm();   // .text:0x3C8
    virtual int stateClose(); // .text:0x464
    virtual int execute();    // .text:0x4B0
    virtual int draw();       // .text:0x5E4

    void subStateMain();                   // .text:0x6A0
    void subStatePush();                   // .text:0x9E0
    void subStateMessage();                // .text:0xA1C
    void subStateMessagePush();            // .text:0xA7C
    void subStateMessageClose();           // .text:0xAE0
    void subStateCheckKey();               // .text:0xB3C
    void subStateCheckKeyResult();         // .text:0xB90
    void subStateKeyData();                // .text:0xC14
    void subStateKeyDataResult();          // .text:0xC68
    void subStateFindFriend();             // .text:0xCC8
    void subStateSearchMember();           // .text:0xDAC
    void subStateMemberResult();           // .text:0xE68
    void subStateMemberData();             // .text:0xF04
    void subStateMemberDataResult();       // .text:0xF5C
    BOOL closeMenu();                      // .text:0xFB8
    BOOL closeMessage();                   // .text:0xFE8
    BOOL handleCloseRequest();             // .text:0x1044
    void executeSubState();                // .text:0x11A8
    void showCursor();                     // .text:0x11E0
    void calcLayout();                     // .text:0x11E8
    int hitCheck();                        // .text:0x1240
    void setSelect(int slot, BOOL select); // .text:0x12C8
    void initButtons();                    // .text:0x1378
    void push();                           // .text:0x13D8
    BOOL isPushEnd();                      // .text:0x14B4
    void finishPush();                     // .text:0x1540
    BOOL inputKey();                       // .text:0x16A0
    void clearInput();                     // .text:0x1A58
    void setInputFromFriendKey();          // .text:0x1AD8
    void hideInputPanes();                 // .text:0x1CC4
    void updateWords();                    // .text:0x1D34
    void initWords();                      // .text:0x1E24
    void updateCursorPos();                // .text:0x1ED0
    BOOL hitCheckWords();                  // .text:0x1F70
    void moveLeft();                       // .text:0x2080
    void moveRight();                      // .text:0x20C8
    void moveHome();                       // .text:0x2124
    void moveEnd();                        // .text:0x216C
    BOOL confirmKey();                     // .text:0x2244
    void openMessage(u16 msg);             // .text:0x243C
    void startCloseAnm();                  // .text:0x24F0
    void setWinAnm(int anm);               // .text:0x2540
    void finish(BOOL fromSession);         // .text:0x25A0
    void playSeError();                    // .text:0x261C

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

    // Implicit destructor: .text:0x2624.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::Anm_c mWinAnm;
    /* 0x264 */ m2d::Anm_c mIconAnm;
    /* 0x288 */ dMenuNumber_c mNumber;
    /* 0xAB0 */ dMenuBtn_c mBtn;
    /* 0xFF4 */ dMenuSlctB_c mSlct;
    /* 0x16E4 */ m2d::stringWord_c mWord[WORD_NUM]; // T_input_E_00..02
    /* 0x19C0 */ dKbd::Cur_c mCur;
    /* 0x1B78 */ int mCursor;
    /* 0x1B7C */ int mNum; // the text cursor: the digit it is in front of
    /* 0x1B80 */ int mDigit[DIGIT_NUM]; // DIGIT_EMPTY for the empty ones
    /* 0x1BB0 */ int mRepeat;           // d-pad repeat timer
    /* 0x1BB4 */ int mHold;             // held key timer
    /* 0x1BB8 */ u64 mKey;              // the friend key of the digits
    /* 0x1BC0 */ DWCFriendData mFriendData;
    /* 0x1BCC */ int mMember; // the session member that is asked about
}; // size 0x1BD0

#endif
