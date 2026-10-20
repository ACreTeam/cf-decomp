#ifndef GAME_D_MENU_STAR00_HPP
#define GAME_D_MENU_STAR00_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_star_draw.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_menu_slct_c.hpp>
#include <game/game/d_menu_star.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_snd_util.hpp>

// The constellation editor (REL d_menu_star00NP, profile MENU_STAR00): the
// star chart (dMenuStar_c) with a dMenuBtn_c (Quit, a hidden button and
// Confirm). The player links stars of the sky with lines (up to
// SIGN_LINE_NUM) to draw a constellation (dSvStarSign::Sign_c): pointing at
// a star and pressing A starts a line, pointing at a star linked to it adds
// the line, pointing at a drawn line and pressing A removes it. Confirm asks
// "Is this OK?" (dMenuSlctC_c) and registers the sign in the town's list;
// Quit asks "Quit without creating a constellation?" (dMenuSlctB_c).
// MENU_STAR_SIGN_NEW draws a new constellation in the first free slot,
// MENU_STAR_SIGN_EDIT edits the slot dMenuMng_c::getPickedStarSign. RTTI
// confirms the class name dMenuStar00_c; everything else is inferred.
class dMenuStar00_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_CHART = 3, // the chart's buttons (dMenuStar_c::BTN_e)
        CURSOR_CHART_ZOOM_IN = CURSOR_CHART + dMenuStar_c::BTN_ZOOM_IN,
        CURSOR_CHART_ZOOM_OUT = CURSOR_CHART + dMenuStar_c::BTN_ZOOM_OUT,
        CURSOR_CHART_TURN_LEFT = CURSOR_CHART + dMenuStar_c::BTN_TURN_LEFT,
        CURSOR_CHART_TURN_RIGHT = CURSOR_CHART + dMenuStar_c::BTN_TURN_RIGHT,
        CURSOR_CHART_LAST = CURSOR_CHART + dMenuStar_c::BTN_NUM,
    };

    // The dMenuBtn_c buttons (the middle one is hidden).
    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED,
        BTN_CONFIRM,
    };

    // What a click on the chart does (mMode).
    enum MODE_e {
        MODE_START,  // no star picked: a click picks the first star of a line
        MODE_LINE,   // a star is picked: link it to a star next to it
        MODE_CHANGE, // an edited constellation: pick a star or remove a line
        MODE_FULL,   // all lines are used: lines can only be removed
        MODE_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW = 0,
        FLAG_SLCT_B = 2,  // draw the quit dialog
        FLAG_SLCT_C = 3,  // draw the confirm dialog
        FLAG_DONE = 4,    // the constellation is registered
        FLAG_POINTER = 5, // a chart button is held with the pointer (else with
                          // the pad)
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH, // waits for the pushed button to finish
        SUB_HOLD, // a chart button is held
        SUB_OK_OPEN,
        SUB_OK_WAIT,
        SUB_OK_PUSH,
        SUB_OK_CLOSE,
        SUB_QUIT_WAIT,
        SUB_QUIT_PUSH,
        SUB_QUIT_CLOSE,
        SUB_NUM,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_IS_OK = 0xA3,      // "Is this OK?"
        MSG_QUIT_NEW = 0x159,  // "Quit without creating a constellation?"
        MSG_QUIT_EDIT = 0x15A, // "Quit without changing the constellation?"
    };

    enum {
        STAR_RADIUS = 30, // how near the cursor must be to pick a star
        LINK_RADIUS = 20, // ... to pick the star linked to the first
        LINE_RADIUS = 15, // ... to pick a line
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isChart(int cursor) {
        return cursor >= CURSOR_CHART && cursor <= CURSOR_CHART_LAST;
    }

    virtual int create();     // .text:0x208
    virtual int doDelete();   // .text:0x320
    virtual int stateMain();  // .text:0x3A4
    virtual int stateAnm();   // .text:0x3C8
    virtual int stateClose(); // .text:0x43C
    virtual int execute();    // .text:0x460
    virtual int draw();       // .text:0x4E0
    void subStateMain();      // .text:0x564
    void subStatePush();      // .text:0x64C
    void subStateHold();      // .text:0x688
    void subStateOkOpen();    // .text:0x788
    void subStateOkWait();    // .text:0x814
    void subStateOkPush();    // .text:0x864
    void subStateOkClose();   // .text:0x938
    void subStateQuitWait();  // .text:0x9B8
    void subStateQuitPush();  // .text:0xA18
    void subStateQuitClose(); // .text:0xA7C
    void executeSubState();   // .text:0xB04
    void initButtons();       // .text:0xBB8
    void updateConfirmBtn();  // .text:0xC20
    bool isChartBtnHeld();
    void setSelect(int cursor, BOOL select); // .text:0xC64
    int hitCheck();                          // .text:0xD08
    void checkPadMove();                     // .text:0xD84
    BOOL pushCursor();                       // .text:0xEB8
    BOOL isPushEnd();                        // .text:0xF9C
    void finishPush();                       // .text:0xFF4
    void startClose(BOOL btnOut);            // .text:0x1094
    void openQuitDialog();                   // .text:0x1104
    void startConfirm();                     // .text:0x11C4
    void initSign();                         // .text:0x1258
    BOOL isCursorOnChart();                  // .text:0x13DC
    void executeMode();                      // .text:0x1468
    void modeStart();                        // .text:0x14A0
    void modeLine();                         // .text:0x14C0
    void modeChange();                       // .text:0x1648
    void modeFull();                         // .text:0x1768
    BOOL pickStar();                         // .text:0x15AC
    BOOL eraseLine();                        // .text:0x16A4
    int getLineNum();                        // .text:0x176C
    u16 findLinkedLine();                    // .text:0x1810
    u16 findSignLine();                      // .text:0x18D0
    void addLine(u16 line);                  // .text:0x193C
    void removeLine(u16 line);               // .text:0x19AC
    int findStar();                          // .text:0x1A1C
    void setHoverStar(int star);             // .text:0x1A68

    void setLineStar(int star); // .text:0x1920
    void clearLineStar();       // .text:0x192C
    void setMode(int mode);     // .text:0x1AB8
    void playSeStarChoice();    // .text:0x1AC0
    void playSeLineChoice();    // .text:0x1AC8
    void playSeLineCancel();    // .text:0x1AD0
    void playSeConfirm();       // .text:0x1AD8
    void playSePointing();      // .text:0x1AE0

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x1D7C
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x1D74
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x1D70
    virtual void deleteReady() {
    } // .text:0x1D6C
    virtual bool createHeap() {
        return true;
    } // .text:0x1D64

    // Implicit destructor: .text:0x1AE8.

    /* 0x0D4 */ dMenuBtn_c mBtn;
    /* 0x618 */ dMenuStar_c mChart;
    /* 0xB48 */ int mCursor;
    /* 0xB4C */ dMenuSlctB_c mSlctB;
    /* 0x123C */ dMenuSlctC_c mSlctC;
    /* 0x1750 */ dStarSign_c mSign; // the constellation being drawn
    /* 0x17C0 */ int mMode;         // MODE_e
    /* 0x17C4 */ int mSlot;         // its slot in the town's list
    /* 0x17C8 */ int mLineStar;     // the first star of the line being drawn
    /* 0x17CC */ u16 mHoverLine;    // the line the cursor points at
    /* 0x17D0 */ int mHoverStar;    // the star the cursor points at
}; // size 0x17D4

#endif
