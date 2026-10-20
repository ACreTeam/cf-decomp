#ifndef GAME_D_MENU_DESI03_HPP
#define GAME_D_MENU_DESI03_HPP

#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_desi.hpp>
#include <game/game/d_menu_title.hpp>
#include <game/game/d_select.hpp>

class dDesign_c;

// The design picker of the flag menus (REL d_menu_desi03NP, profile
// MENU_DESI03; menus 0x66 and 0x6B): shows the player's eight designs
// (dMenuDesi::Desi_c) under the "Which design?" title; tapping one opens an
// answer menu to copy it to, or swap it with, the flag's design. RTTI confirms
// the global class name; everything else is inferred.
class dMenuDesi03_c : public dMenuBase_c {
  public:
    // Slots: what the cursor points at (mCursor).
    enum SLOT_e {
        SLOT_NONE = -1,
        SLOT_BTN = 0, // 0..2: dMenuBtn_c buttons (0 is Quit)
        SLOT_BTN_LAST = 2,
        SLOT_DESIGN = 3, // 3..10: the eight designs
        SLOT_DESIGN_LAST = 10,
    };

    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED_1, // hidden
        BTN_UNUSED_2, // hidden
    };

    // The title bar's icon (dMenuTitle::Title_c::setIcon).
    enum TITLE_ICON_e {
        TITLE_ICON_DESIGN = 2,
    };

    // The dObjTutorial_c::request kind that shows a design's name.
    enum { TUTORIAL_DESIGN_NAME = 7 };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,     // draw the design window
        FLAG_SELECTED, // a design is highlighted
    };

    // mSubState.
    enum SUB_STATE_e {
        SUB_MAIN,      // pick something with the cursor
        SUB_WAIT_PUSH, // wait for the pushed button / design to finish
        SUB_SELECT,    // the answer menu
        SUB_NUM
    };

    // The answers of the answer menu (dSelect::BaseComp_c::mCode).
    enum ANSWER_e {
        ANSWER_SWAP,
        ANSWER_COPY,
        ANSWER_CANCEL,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_DISPLAY_THIS = 0xD6,
        MSG_SWAP_WITH_THIS = 0xD7,
        MSG_NEVER_MIND = 0xD8,
        MSG_WHICH_DESIGN = 0xFD,
    };

    static bool isBtn(int slot) {
        return slot >= SLOT_BTN && slot <= SLOT_BTN_LAST;
    }
    static bool isDesign(int slot) {
        return slot >= SLOT_DESIGN && slot <= SLOT_DESIGN_LAST;
    }

    virtual int create();     // .text:0x300
    virtual int doDelete();   // .text:0x44C
    virtual int stateMain();  // .text:0x4D0
    virtual int stateAnm();   // .text:0x4F4
    virtual int stateClose(); // .text:0x54C
    virtual int execute();    // .text:0x570
    virtual int draw();       // .text:0x5F0

    void subMain();                                    // .text:0x64C
    void subWaitPush();                                // .text:0x714
    void subSelect();                                  // .text:0x750
    void executeSubState();                            // .text:0x7E4
    void setHighlight(int slot, BOOL on);              // .text:0x81C
    int hitCheck();                                    // .text:0x8DC
    void push();                                       // .text:0x974
    BOOL isPushDone();                                 // .text:0xA3C
    void endPush();                                    // .text:0xA94
    void closeMenu();                                  // .text:0xB18
    void initButtons();                                // .text:0xB84
    dDesign_c *getDesign(int index);                   // .text:0xBE0
    int getOrder(int index);                           // .text:0xBE8
    void getSlotName(dScript::Word_c *word, int slot); // .text:0xC24
    void showSlotTutorial(int slot);                   // .text:0xCAC
    void hideTutorial();                               // .text:0xD74
    mVec2_c getSlotPos(int slot, f32 x, f32 y);        // .text:0xD78
    void openSelect(int slot);                         // .text:0xDE4
    void endSub();                                     // .text:0xFA8
    void swapDesign();                                 // .text:0xFBC
    void copyDesign();                                 // .text:0x1034
    void buildSelect();                                // .text:0x10A8

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

    /* 0x0D4 */ dMenuDesi::Desi_c mDesi;
    /* 0x484 */ dMenuBtn_c mBtn;
    /* 0x9C8 */ dMenuTitle::Title_c mTitle;
    /* 0xB80 */ int mCursor; // SLOT_e under the cursor
    /* 0xB84 */ dSelect::MComp_c mSelect;
    /* 0x1530 */ int mDesign; // the design slot the answer menu is for
    /* 0x1534 */ int mAnswer;
}; // size 0x1538

#endif
