#ifndef GAME_D_MENU_HAN_HPP
#define GAME_D_MENU_HAN_HPP

#include <game/game/d_kbd.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_slct_b.hpp>

// The text entry menu (REL d_menu_hanNP, profile MENU_HAN): one question
// window (Layout/ cmn_question_00.brlyt, "What's your name?") with a text
// field and the software keyboard (dKbd::Mng_c) under it. The menu number
// (dMenuBase_c::mMenuNo, a dMenuMng_c::MENU_INPUT_* value) selects the
// question, the input field, the maximum length and what is done with the
// text:
//   - the player's name and the town's name (asked by the NPCs),
//   - the data of a friend entry (name, town, comment) opened by the friend
//     menu (dMenuFriend_c), and the player's own friend name,
//   - the name of a design, a constellation, the exact message, the "say it"
//     text, the catchphrase / nickname / greeting asked by the NPCs and the
//     K.K. song request (the song named is looked up in the item names).
// The text goes to dMenuMng_c's input buffer (getInputBuf) and some of the
// menus store it themselves. A text that matches the NG word list
// (dPrcMng_c) is refused with a message. RTTI confirms the global class names
// dMenuHan_c and dMenuHan_c::Word_c; everything else is inferred.
class dMenuHan_c : public dMenuBase_c {
  public:
    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,          // typing
        SUB_PUSH,          // waits for the pushed button
        SUB_CHECK,         // starts the NG word check
        SUB_CHECK_RESULT,  // waits for the check
        SUB_MESSAGE,       // the "You can't enter that!" box is open
        SUB_MESSAGE_PUSH,  // its OK button was pushed
        SUB_MESSAGE_CLOSE, // waits for it to close
        SUB_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,     // the layout needs a calc
        FLAG_CUR,      // the text cursor is drawn
        FLAG_REGISTER, // register the friend entry (execute), then close
        FLAG_FRAME,    // the text is in the friend menu's frame: no layout
        FLAG_SLCT,     // the message box is drawn
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_QUIT, // "Quit" / "Back", not always there
        BTN_UNUSED,
        BTN_CONFIRM, // disabled while the text is empty
        BTN_NONE = -1,
    };

    // dMenuMng_c::setResult values.
    enum RESULT_e {
        RESULT_CANCEL,
        RESULT_CONFIRM,
    };

    // The input fields of the question layout: a text box
    // ("T_input_<x>_00"), its pane ("N_input_<x>") and a frame
    // ("inputFrm_<x>_00"). The pane "N_input_E" is always hidden.
    enum INPUT_e {
        INPUT_A, // 16 characters
        INPUT_B, // 10
        INPUT_C, // 6 (names in Japanese)
        INPUT_D, // 32
        INPUT_F, // 4
        INPUT_NUM,
    };

    // The icon of the question (mIconAnm frames).
    enum ICON_e {
        ICON_NONE,
        ICON_COMMENT = 1,
        ICON_TOWN = 3,
        ICON_DESIGN = 4,
        ICON_PLAYER = 5,
        ICON_NICKNAME = 6,
        ICON_SAY = 7,
        ICON_SONG = 8,
        ICON_STAR_SIGN = 9,
        ICON_MESSAGE = 11,
    };

    enum {
        TEXT_SIZE = 0x21,     // the longest text with its terminator
        NAME_ITEM_LEN = 0x11, // the item name for the song request
        SONG_NUM = 0x4E,      // songs (item indexes ITEM_IDX_SONG_FIRST..)
        SONG_FIRST = 0xEE,    // item index of the first song
        DESIGN_STYLE = 10,    // dDesign_c::setStyle value of a named design
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_BACK = 0x20,
        MSG_CONFIRM = 0x22,
        MSG_PLAYER_NAME = 0x29,    // "What's your name?"
        MSG_TOWN_NAME = 0x2A,      // "What's your town's name?"
        MSG_FRIEND_NAME = 0x9D,    // "What's your friend's name?"
        MSG_FRIEND_TOWN = 0x9E,    // "What's your friend's town?"
        MSG_COMMENT = 0x9F,        // "Any comments?"
        MSG_STAR_SIGN_LABEL = 0xA5, // the constellation name suffix label (empty in English)
        MSG_SAY = 0xBF,            // "Say it!"
        MSG_DESIGN_NAME = 0xC0,    // "What's this design called?"
        MSG_MESSAGE = 0xC1,        // "What's the exact message?"
        MSG_STAR_SIGN_NAME = 0xC2, // "What's it called?"
        MSG_CATCHPHRASE = 0xC3,    // "What's the catchphrase?"
        MSG_NICKNAME = 0xC4,       // "Choose a nickname!"
        MSG_GREETING = 0xC5,       // "What's the greeting?"
        MSG_OK = 0xEC,             // "OK!"
        MSG_TOWN_LABEL = 0x11D,    // the (empty) town name label
        MSG_SONG = 0x122,          // "Request which song?"
        MSG_CANT_ENTER = 0x149,    // "You can't enter that!"
    };

    // The text of the question layout and the entered text.
    class Word_c : public m2d::Word_c {
      public:
        Word_c();                     // .text:0x2078
        virtual ~Word_c();            // .text:0x20BC
        virtual u32 getBufferSize();  // .text:0x2114
        virtual wchar_t *getBuffer(); // .text:0x211C

        /* 0x28 */ wchar_t mBuffer[0x2F];
    }; // size 0x88

    typedef void (dMenuHan_c::*SubStateProc)();
    typedef BOOL (dMenuHan_c::*CloseProc)();

    virtual int create();     // .text:0x170
    virtual int doDelete();   // .text:0x374
    virtual int stateMain();  // .text:0x3FC
    virtual int stateAnm();   // .text:0x44C
    virtual int stateClose(); // .text:0x530
    virtual int execute();    // .text:0x57C
    virtual int draw();       // .text:0x604

    void subStateMain();         // .text:0x6A4
    void subStatePush();         // .text:0x824
    void subStateCheck();        // .text:0x914
    void subStateCheckResult();  // .text:0x970
    void subStateMessage();      // .text:0x9CC
    void subStateMessagePush();  // .text:0xA2C
    void subStateMessageClose(); // .text:0xA90
    BOOL closeMenu();            // .text:0xAE0
    BOOL closeMessage();         // .text:0xB7C
    BOOL handleCloseRequest();   // .text:0xBD8
    void executeSubState();      // .text:0xD04
    void calcLayout();           // .text:0xD3C
    void startCloseAnm();        // .text:0xDA0
    void showCantEnter();        // .text:0xDF0
    void setInput(int input);    // .text:0xE9C
    void setFrameInput(nw4r::lyt::TextBox *textBox,
                       nw4r::lyt::Pane *frame); // .text:0xFC0
    void setIcon();                             // .text:0x1014
    void setQuestion();                         // .text:0x10CC
    void setMaxLength();                        // .text:0x12FC
    void clearText();                           // .text:0x13F0
    void cancel();                              // .text:0x1400
    void apply();                               // .text:0x1484
    void applySong();                           // .text:0x1A94
    BOOL checkLength();                         // .text:0x1BE8
    void updateText();                          // .text:0x1D08
    void restoreText();                         // .text:0x1DF4
    BOOL hasText();                             // .text:0x1E40
    void pushButton(int btn);                   // .text:0x1EE0
    void updateCursor();                        // .text:0x1F5C
    BOOL hitCheckText();                        // .text:0x1FB4
    void initButtons();                         // .text:0x2124
    int hitCheckBtn();                          // .text:0x21F4
    void setMsg(u16 msg);                       // .text:0x2230
    void setWinAnm(int anm);                    // .text:0x227C

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

    // Implicit destructor: .text:0x23E0.

    /* 0x0D4 */ m2d::SimpleEx_c mLyt;
    /* 0x16C */ nw4r::lyt::Pane *mpFrame; // the pane the pointer hits to move
                                          // the text cursor
    /* 0x170 */ m2d::Anm_c mWinAnm;
    /* 0x194 */ m2d::Anm_c mIconAnm;
    /* 0x1B8 */ dKbd::Mng_c mKbd;
    /* 0x530 */ dKbd::Cur_c mCur;
    /* 0x6E8 */ dMenuBtn_c mBtn;
    /* 0xC2C */ dMenuSlctB_c mSlct;
    /* 0x131C */ Word_c mWord;
    /* 0x13A4 */ wchar_t mText[TEXT_SIZE]; // the last accepted text
    /* 0x13E8 */ u32 mMaxLen;
    /* 0x13EC */ f32 mMaxWidth;  // of the text box
    /* 0x13F0 */ int mInput;     // INPUT_e
    /* 0x13F4 */ int mCheckHits; // NG words found
    /* 0x13F8 */ const u16 *mCheckText;
    /* 0x13FC */ u8 mCheckResult;
}; // size 0x1400

#endif
