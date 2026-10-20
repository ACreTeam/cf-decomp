#ifndef GAME_D_MENU_DESI01_HPP
#define GAME_D_MENU_DESI01_HPP

#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_desi.hpp>
#include <game/game/d_menu_title.hpp>

// The design slot prompt (REL d_menu_desi01NP, profile MENU_DESI01): the eight
// designs of dMenuDesi::Desi_c below a title bar (dMenuTitle::Title_c) with a
// question, a quit and a confirm button. The player points at one design and
// confirms; the design, its slot and its order are handed back through
// dMenuMng_c. The menu number selects the question and the title icon (see
// dMenuMng_c::MENU_DESIGN_TAKE and the ones after it). RTTI confirms the
// global class name; everything else is inferred.
class dMenuDesi01_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at: the three buttons, then the eight
    // designs.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: BTN_e
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_DESIGN = dMenuBtn_c::BTN_NUM, // 3..10: the designs
        CURSOR_DESIGN_LAST = CURSOR_DESIGN + 7,
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_CANCEL,
        BTN_UNUSED, // hidden
        BTN_CONFIRM,
    };

    // The frame of the title bar's icon animation.
    enum ICON_e {
        ICON_DESIGN = 0,
        ICON_GIFT = 3,
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
        RESULT_CONFIRM = 1,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_SELECTED, // the highlighted design changed
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH, // waits for the pushed button / design to finish
        SUB_NUM,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_PUT_WHERE = 0xFC, // "Put it where?"
        MSG_PUT_WHERE_GIFT = 0xFE,
        MSG_SWAP_WITH = 0x103,     // "Swap with which one?"
        MSG_DISPLAY_WHICH = 0x104, // "Display which?"
    };

    // Tutorial balloon kind (dObjTutorial_c::request).
    enum { TUTORIAL_DESIGN = 5 };

    static bool isButton(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isDesign(int cursor) {
        return cursor >= CURSOR_DESIGN && cursor <= CURSOR_DESIGN_LAST;
    }

    virtual int create();     // .text:0x2E0
    virtual int doDelete();   // .text:0x490
    virtual int stateMain();  // .text:0x4FC
    virtual int stateAnm();   // .text:0x520
    virtual int stateClose(); // .text:0x578
    virtual int execute();    // .text:0x5E0
    virtual int draw();       // .text:0x658

    void subMain();                                    // .text:0x6AC
    void subPush();                                    // .text:0x774
    void executeSubState();                            // .text:0x7B0
    void setSelect(int cursor, BOOL select);           // .text:0x7E8
    int hitCheck();                                    // .text:0x8A8
    void pushCursor();                                 // .text:0x940
    BOOL isPushEnd();                                  // .text:0xA00
    void finishPush();                                 // .text:0xA58
    void startCloseAnm();                              // .text:0xC00
    void initButtons();                                // .text:0xC6C
    dDesign_c *getDesign(int slot);                    // .text:0xCDC
    int getOrder(int slot);                            // .text:0xCE4
    void selectDesign(int slot);                       // .text:0xD20
    void getSlotName(dScript::Word_c *word, int slot); // .text:0xD6C
    void showTutorial(int cursor);                     // .text:0xDF4
    void hideTutorial();                               // .text:0xEBC
    mVec2_c getSlotPos(int cursor, f32 x, f32 y);      // .text:0xEC0

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

    // Implicit destructor: .text:0xF2C.

    /* 0x0D4 */ dMenuDesi::Desi_c mDesi;
    /* 0x484 */ dMenuBtn_c mBtn;
    /* 0x9C8 */ dMenuTitle::Title_c mTitle;
    /* 0xB80 */ int mCursor;
}; // size 0xB84

#endif
