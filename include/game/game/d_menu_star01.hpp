#ifndef GAME_D_MENU_STAR01_HPP
#define GAME_D_MENU_STAR01_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_star.hpp>
#include <game/game/d_pad.hpp>

// The weak vtables/RTTI of the classes come out in reverse include order:
// fBase_c's last, then dDvd::brresBank_c's, dMenuBase_c's first.
#include <game/game/d_menu_base.hpp>

// The constellation viewer (REL d_menu_star01NP, profile MENU_STAR01): the star
// chart (dMenuStar_c, REL d_menu_starNP) with the button bar (dMenuBtn_c) of
// the menus. The sky is zoomed and turned with the star chart's buttons or the
// D-pad, and the name of the constellation in the middle of the chart is shown
// in a tutorial balloon. RTTI confirms the global class name; everything else
// is inferred.
class dMenuStar01_c : public dMenuBase_c {
  public:
    // mCursor: what the pointer or the D-pad selects.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // dMenuBtn_c buttons 0..2
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_STAR = 3, // + the dMenuStar_c::BTN_e
        CURSOR_ZOOM_IN = CURSOR_STAR + dMenuStar_c::BTN_ZOOM_IN,
        CURSOR_ZOOM_OUT = CURSOR_STAR + dMenuStar_c::BTN_ZOOM_OUT,
        CURSOR_TURN_A = CURSOR_STAR + dMenuStar_c::BTN_TURN_LEFT, // D-pad right
        CURSOR_TURN_B = CURSOR_STAR + dMenuStar_c::BTN_TURN_RIGHT, // D-pad left
        CURSOR_STAR_LAST = 7,
    };

    // The button bar's buttons (dMenuBtn_c).
    enum BTN_e {
        BTN_UNUSED_0,
        BTN_UNUSED_1,
        BTN_BACK,
    };

    // Message of SYS_2D_menu.bmg.
    enum {
        MSG_BACK = 0x20, // "Back"
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum {
        FLAG_DRAW = 0,
        FLAG_CLICK = 2, // the star button is held by the pointer, not the D-pad
    };

    // mSubState.
    enum SUB_STATE_e {
        SUB_MAIN, // select and push
        SUB_PUSH, // wait for the push animation
        SUB_HOLD, // keep moving the sky while the button is held
        SUB_NUM
    };

    enum {
        BALLOON_PRIORITY_DEFAULT = 0x84,
    };

    virtual int create();     // .text:0x1F0
    virtual int doDelete();   // .text:0x2C4
    virtual int stateMain();  // .text:0x328
    virtual int stateAnm();   // .text:0x34C
    virtual int stateClose(); // .text:0x350
    virtual int execute();    // .text:0x374
    virtual int draw();       // .text:0x3E4

    void subMain();                          // .text:0x430
    void subPush();                          // .text:0x4F8
    void subHold();                          // .text:0x534
    void executeSubState();                  // .text:0x634
    void initBtn();                          // .text:0x680
    void setSelect(int cursor, BOOL select); // .text:0x6DC
    int hitCheck();                          // .text:0x780
    void updateSign();                       // .text:0x7FC
    void checkPad();                         // .text:0x860
    void push();                             // .text:0x994
    BOOL isPushEnd();                        // .text:0xA48
    void onPushEnd();                        // .text:0xAD0
    void close();                            // .text:0xB40
    void showSignName(int sign);             // .text:0xBA0
    void hideSignName();                     // .text:0xE50

    // The D-pad direction of the selected star button is held.
    BOOL isPadHeld() {
        BOOL held = FALSE;
        if (mCursor == CURSOR_ZOOM_IN && dPad::isUpHeld() ||
            mCursor == CURSOR_ZOOM_OUT && dPad::isDownHeld() ||
            mCursor == CURSOR_TURN_B && dPad::isLeftHeld() ||
            mCursor == CURSOR_TURN_A && dPad::isRightHeld()) {
            held = TRUE;
        }
        return held;
    }

    // Implicit destructor: .text:0xE54.

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isStarBtn(int cursor) {
        return cursor >= CURSOR_STAR && cursor <= CURSOR_STAR_LAST;
    }

    /* 0x0D4 */ dMenuBtn_c mBtn;
    /* 0x618 */ dMenuStar_c mStar;
    /* 0xB48 */ int mCursor;
}; // size 0xB4C

#endif
