#ifndef GAME_D_MENU_PAPER_HPP
#define GAME_D_MENU_PAPER_HPP

#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_paper_view.hpp>

// The letter reading menu (REL d_menu_paperNP, profile MENU_PAPER): a letter
// on its stationery (dPaper_c) with a single Back button (dMenuBtn_c). One
// menu for dMenuMng_c's MENU_READ_LETTER_NPC, MENU_READ_LETTER_8,
// MENU_READ_LETTER_9 and MENU_READ_LETTER, which differ only in the menu that
// is opened when it closes. RTTI confirms the class name dMenuPaper_c;
// everything else is inferred.
class dMenuPaper_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
    };

    // The dMenuBtn_c buttons; only the last one (Back) is shown.
    enum BTN_e {
        BTN_UNUSED_0,
        BTN_UNUSED_1,
        BTN_BACK,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_CALC_PAPER, // the paper needs an execute
        FLAG_DRAW,
        FLAG_QUIT, // the menu is closing for good
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH, // waits for the pushed button to finish
        SUB_NUM,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_BACK = 0x20,
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }

    virtual int create();                    // .text:0xB8
    virtual int doDelete();                  // .text:0x170
    virtual int stateMain();                 // .text:0x1C4
    virtual int draw();                      // .text:0x214
    virtual int stateAnm();                  // .text:0x260
    virtual int stateClose();                // .text:0x2C8
    virtual int execute();                   // .text:0x334
    void subStateMain();                     // .text:0x378
    void subStatePush();                     // .text:0x434
    BOOL onCloseMain();                      // .text:0x470
    BOOL handleCloseRequest();               // .text:0x4B4
    void executeSubState();                  // .text:0x59C
    void calcPaper();                        // .text:0x5D4
    void startCloseAnm();                    // .text:0x620
    void initButtons();                      // .text:0x63C
    void setSelect(int cursor, BOOL select); // .text:0x698
    int hitCheck();                          // .text:0x6C8
    void pushCursor();                       // .text:0x728
    BOOL isPushEnd();                        // .text:0x7A0
    void finishPush();                       // .text:0x7F8
    BOOL openNextMenu();                     // .text:0x834

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x94C
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x944
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x940
    virtual void deleteReady() {
    } // .text:0x93C
    virtual bool createHeap() {
        return true;
    } // .text:0x934

    // Implicit destructor: .text:0x8B8.

    /* 0x0D4 */ dPaper_c mPaper;
    /* 0xAC8 */ dMenuBtn_c mBtn;
    /* 0x100C */ int mCursor;
}; // size 0x1010

#endif
