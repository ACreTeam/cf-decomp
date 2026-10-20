#ifndef GAME_D_MENU_MSLCT_HPP
#define GAME_D_MENU_MSLCT_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_ltr.hpp>

// The letter picker of the post office (REL d_menu_mslctNP, profile
// MENU_MSLCT): the player's ten letters in the shared letter window
// (dMenuLtr::Window_c) plus the button bar. The letters to hand in are marked,
// then confirmed. One menu for the three post office counters, selected by
// mMenuNo (MENU_POST_TOWN, MENU_POST_OTHER_TOWN, MENU_POST_FUTURE). RTTI
// confirms the global class name; everything else is inferred.
class dMenuMslct_c : public dMenuBase_c {
  public:
    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,   // only cleared
        FLAG_SINGLE, // only one letter can be picked (the letter to the future)
        FLAG_SENDING, // a letter is on its way to the host's post office
    };

    enum SUB_STATE_e { SUB_MAIN, SUB_PUSH };

    // The cursor (mCursor): 0..9 the letters, 10..12 the button bar's buttons.
    enum {
        CURSOR_NONE = -1,
        CURSOR_LETTER_FIRST = 0,
        CURSOR_LETTER_LAST = dMenuLtr::Window_c::LETTER_COUNT - 1,
        CURSOR_BTN_FIRST = dMenuLtr::Window_c::LETTER_COUNT,
    };

    // The button bar's buttons.
    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED, // hidden
        BTN_CONFIRM,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
    };

    static bool isLetter(int slot) {
        return slot >= CURSOR_LETTER_FIRST && slot <= CURSOR_LETTER_LAST;
    }
    static bool isBtn(int slot) {
        return slot >= CURSOR_BTN_FIRST &&
               slot <= CURSOR_BTN_FIRST + dMenuBtn_c::BTN_NUM - 1;
    }

    virtual int create();     // .text:0x25C
    virtual int doDelete();   // .text:0x340
    virtual int stateMain();  // .text:0x394
    virtual int stateAnm();   // .text:0x3B8
    virtual int stateClose(); // .text:0x3BC
    virtual int execute();    // .text:0x41C
    virtual int draw();       // .text:0x4BC

    void subStateMain();                       // .text:0x510
    void subStatePush();                       // .text:0x630
    void executeSubState();                    // .text:0x66C
    void initButtons();                        // .text:0x6A4
    void setOkDisable(int disable);            // .text:0x710
    void setSelect(int slot, BOOL select);     // .text:0x72C
    int hitCheck();                            // .text:0x7D8
    void push();                               // .text:0x870
    bool isPushEnd();                          // .text:0x9D8
    void finishPush();                         // .text:0xA30
    void startCloseAnm();                      // .text:0xADC
    bool isNoChoice(int slot);                 // .text:0xB38
    void setNoChoice(int slot, BOOL noChoice); // .text:0xB5C
    void initLetters();                        // .text:0xBDC
    bool isPicked(int slot);                   // .text:0xD0C
    void setPicked(int slot, BOOL picked);     // .text:0xD30
    void showTutorial(int slot);               // .text:0xDC4
    void hideTutorial();                       // .text:0xEB4
    dMail_c *getMail(int slot);                // .text:0xEB8
    BOOL isEmptyLetter(int slot);              // .text:0xEF0
    void clearLetter(int slot);                // .text:0xF54
    int countPicked();                         // .text:0xF88
    void sendLetters();                        // .text:0x1018
    static void setResultOk();                 // .text:0x10E4
    void sendTown();                           // .text:0x10EC
    void sendTownHost();                       // .text:0x1190
    void sendTownGuest();                      // .text:0x11E4
    void sendOtherTownSolo();                  // .text:0x1240
    void sendOtherTownNet();                   // .text:0x12B0
    void sendFuture();                         // .text:0x1304
    void addToPostOffice();                    // .text:0x135C
    void deliverToPlayers();                   // .text:0x1414
    void deliverToAnimals();                   // .text:0x14DC
    void checkRecipients(BOOL discardUnknown); // .text:0x1594
    void sendFirst();                          // .text:0x16D0
    void updateSending();                      // .text:0x176C

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

    // Implicit destructor: .text:0x1858.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes; // never loaded
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;     // never built
    /* 0x240 */ dMenuBtn_c mBtn;
    /* 0x784 */ dMenuLtr::Window_c mWin;
    /* 0x1A64 */ int mCursor;
    /* 0x1A68 */ u16 mNoChoiceMask; // letters that can't be picked
    /* 0x1A6A */ u16 mPickedMask;   // letters picked to be handed in
    /* 0x1A6C */ int mSingle;       // the letter picked in the single mode
    /* 0x1A70 */ int mSendLetter;   // the letter being sent to the host
}; // size 0x1A74

#endif
