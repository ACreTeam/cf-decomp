#ifndef GAME_D_MENU_NUMBER00_HPP
#define GAME_D_MENU_NUMBER00_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_number.hpp>

// The bare number pad menu (REL d_menu_number00NP, profile MENU_NUMBER00, menu
// number dMenuMng_c::MENU_NUMBER00): the shared number pad (dMenuNumber_c)
// animated with the item window layout (Layout/item_win.arc, itm_win_00), plus
// a quit and a next button. It keeps no amount; pressing a key only plays the
// key animation. RTTI confirms the global class name; everything else is
// inferred.
class dMenuNumber00_c : public dMenuBase_c {
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
        BTN_QUIT,
        BTN_UNUSED, // hidden
        BTN_NEXT,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC, // the layout needs a calc
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
        MSG_NEXT = 0x21,
    };

    static bool isKey(int cursor) {
        return cursor >= CURSOR_KEY && cursor <= CURSOR_KEY_LAST;
    }
    static bool isButton(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }

    virtual int create();                    // .text:0xE0
    virtual int doDelete();                  // .text:0x1F0
    virtual int stateAnm();                  // .text:0x29C
    virtual int stateMain();                 // .text:0x25C
    virtual int stateClose();                // .text:0x304
    virtual int execute();                   // .text:0x328
    virtual int draw();                      // .text:0x378
    void subStateMain();                     // .text:0x3CC
    void subStatePush();                     // .text:0x458
    void executeSubState();                  // .text:0x494
    void calcWinAnm();                       // .text:0x4CC
    void calcLayout();                       // .text:0x4D0
    int hitCheck();                          // .text:0x528
    void setSelect(int cursor, BOOL select); // .text:0x5B0
    void initButtons();                      // .text:0x660
    void pushCursor();                       // .text:0x6C0
    BOOL isPushEnd();                        // .text:0x760
    void finishPush();                       // .text:0x7EC

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x910
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x908
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x904
    virtual void deleteReady() {
    } // .text:0x900
    virtual bool createHeap() {
        return true;
    } // .text:0x8F8

    // Implicit destructor: .text:0x860.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ dMenuNumber_c mNumber;
    /* 0xA68 */ dMenuBtn_c mBtn;
    /* 0xFAC */ int mCursor;
}; // size 0xFB0

#endif
