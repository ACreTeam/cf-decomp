#ifndef GAME_D_MENU_ITEM06_HPP
#define GAME_D_MENU_ITEM06_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>

// A small chest window menu (REL d_menu_item06NP, profile MENU_ITEM06): the
// chest window (Layout/chest.arc, chest_win_00.brlyt) with a close button. No
// entry of the menu manager's table opens it. RTTI confirms the class name
// dMenuItem06_c; everything else is inferred.
class dMenuItem06_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_HOLE = 0, // 0..9: the item holes (circle hit boxes, inert)
        CURSOR_HOLE_LAST = 9,
        CURSOR_SLIDER = 10, // 10, 11: the slider cursors (close the menu)
        CURSOR_SLIDER_LAST = 11,
        CURSOR_CLOSE = 12, // the close button
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC, // the layout needs a calc
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH, // waits for the pushed button to finish
        SUB_NUM,
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_CLOSE = 0xEB,
    };

    static bool isSlider(int cursor) {
        return cursor >= CURSOR_SLIDER && cursor <= CURSOR_SLIDER_LAST;
    }

    virtual int create();                    // .text:0xD0
    virtual int doDelete();                  // .text:0x1A4
    virtual int stateMain();                 // .text:0x1D4
    virtual int stateAnm();                  // .text:0x1F8
    virtual int stateClose();                // .text:0x1FC
    virtual int execute();                   // .text:0x220
    virtual int draw();                      // .text:0x2B4
    void subStateMain();                     // .text:0x2F8
    void subStatePush();                     // .text:0x37C
    void executeSubState();                  // .text:0x3B8
    void setSelect(int cursor, BOOL select); // .text:0x3F0
    int hitCheck();                          // .text:0x3F4
    void pushCursor();                       // .text:0x47C
    BOOL isPushEnd();                        // .text:0x4C0
    void finishPush();                       // .text:0x4C8
    void startCloseAnm();                    // .text:0x510

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x5EC
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x5E4
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x5E0
    virtual void deleteReady() {
    } // .text:0x5DC
    virtual bool createHeap() {
        return true;
    } // .text:0x5D4

    // Implicit destructor: .text:0x554.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ int mCursor;
}; // size 0x244

#endif
