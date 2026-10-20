#ifndef GAME_D_MENU_MAP03_HPP
#define GAME_D_MENU_MAP03_HPP

#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_map.hpp>

// The town map with a button bar (REL d_menu_map03NP, profile MENU_MAP03,
// MENU_HOUSE_MAP in dMenuMng_c, opened by Pelly): dMenuMap_c plus an OK button
// (dMenuBtn_c) and a frame (map_cursor_00) on each of the four player houses.
// RTTI confirms the class name dMenuMap03_c; everything else is inferred.
class dMenuMap03_c : public dMenuMap_c {
  public:
    // The dMenuBtn_c buttons (the other two are hidden).
    enum BTN_e {
        BTN_UNUSED_0,
        BTN_UNUSED_1,
        BTN_OK,
    };

    // mCursor values past the base class's slots: the dMenuBtn_c buttons.
    enum {
        SLOT_BTN = SLOT_TAB_LAST + 1,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag (after the base's).
    enum {
        FLAG_COPY_POS = 8,   // the house frames follow the map window
        FLAG_CALC_FRAME = 9, // the house frames need a calc
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH, // waits for the pushed button to finish
        SUB_NUM,
    };

    // The frame animations (mFrameAnm).
    enum FRAME_ANM_e {
        FRAME_ANM_IN,
        FRAME_ANM_OUT,
        FRAME_ANM_PUSH,
        FRAME_ANM_NUM,
    };

    enum {
        HOUSE_NUM = 4,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_OK = 0xEC,
    };

    virtual int create();    // .text:0x290
    virtual int doDelete();  // .text:0x338
    virtual int stateMain(); // .text:0x380
    virtual int stateAnm();  // .text:0x3C0
    virtual int execute();   // .text:0x41C
    virtual int draw();      // .text:0x460
    void subStateMain();     // .text:0x4C0
    void subStatePush();     // .text:0x5B4
    void executeSubState();  // .text:0x60C
    void beginSubState();    // .text:0x644
    void endSubState();      // .text:0x648
    void initButtons();      // .text:0x64C
    int hitCheckBtn();       // .text:0x6A8
    void initFrames();       // .text:0x6E4
    void calcFrames();       // .text:0x838

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0xB24
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0xB1C
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0xB18
    virtual void deleteReady() {
    } // .text:0xB14
    virtual bool createHeap() {
        return true;
    } // .text:0xB0C

    // Implicit destructor: .text:0x968.

    /* 0x1C04 */ dMenuBtn_c mBtn;
    /* 0x2148 */ m2d::SimpleEx_c mFrame[HOUSE_NUM];
    /* 0x23A8 */ m2d::AnmEx_c mFrameAnm[HOUSE_NUM];
}; // size 0x2498

#endif
