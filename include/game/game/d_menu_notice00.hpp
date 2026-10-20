#pragma once

// The bulletin board reader (REL d_menu_notice00NP, profile MENU_NOTICE00,
// RTTI "dMenuNotice00_c"; global scope). It is the sub menu of dMenuNotice_c
// (REL d_menu_noticeNP) that shows the notices of the town board: dMenuMng_c
// MENU_BBS_NEWEST opens it on the newest notice (the board's paper is shown at
// once), MENU_BBS_LIST on the list of the dots only (a notice is shown when a
// dot is picked). Layout/board.arc is the base menu's; this menu adds the
// window (board_win_base_00: one dot per notice, 15 at most) and the cursor
// layout (board_win_cursor_00: previous / next notice arrows, jump to the
// first / newest notice) and three buttons: close, erase (with a yes / no
// dialog) and write (changes to the keyboard menu, REL d_menu_notice01NP).
// A change of notice slides the old paper out of the base menu and the new one
// in on this menu's own dNotice_c (mNotice); when the slide ends the base paper
// shows the new notice. Names other than the RTTI name are inferred.

#include <types.h>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_notice.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_sv_notice.hpp>

class dMenuNotice00_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_DOT = 3, // 3..17: the dot of each notice
        CURSOR_DOT_LAST = CURSOR_DOT + dNoticeBoard_c::NOTICE_NUM - 1,
        CURSOR_PREV = 18,   // the previous notice
        CURSOR_NEXT = 19,   // the next notice
        CURSOR_FIRST = 20,  // the first notice
        CURSOR_NEWEST = 21, // the newest notice
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_CLOSE,
        BTN_ERASE,
        BTN_WRITE,
    };

    enum {
        DOT_NUM = dNoticeBoard_c::NOTICE_NUM,
        SCRL_NUM = 2,
        JUMP_NUM = 2,
        NO_NOTICE = -1,
        LYT_PRIORITY = 0x81,
    };

    // The dots' animations (mDotAnm; the order of the file names): the hover
    // dot, an unread notice, a read notice.
    enum DOT_ANM_e {
        DOT_ANM_IN,
        DOT_ANM_UNREAD,
        DOT_ANM_READ,
        DOT_ANM_NUM,
    };

    // The arrows' animations (mScrlBtnAnm).
    enum SCRL_ANM_e {
        SCRL_ANM_IN,
        SCRL_ANM_OUT,
        SCRL_ANM_NO_PUSH,
        SCRL_ANM_CAN_PUSH,
        SCRL_ANM_PUSH,
        SCRL_ANM_NUM,
    };

    // The jump buttons' hover animations (mJumpAnm).
    enum JUMP_ANM_e {
        JUMP_ANM_IN,
        JUMP_ANM_OUT,
        JUMP_ANM_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,   // the window is drawn
        FLAG_CALC,   // the layouts changed and need a calc
        FLAG_NOTICE, // the own paper (mNotice) is drawn: the slide is running
        FLAG_PAPER,  // the base menu's paper follows the window
        FLAG_DIALOG, // the erase dialog is drawn
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,          // waits for the pushed button to finish
        SUB_SLIDE,         // waits for the paper to slide in
        SUB_DIALOG_OPEN,   // the erase dialog opens
        SUB_DIALOG_ANSWER, // waits for the answer
        SUB_DIALOG_CLOSE,  // the dialog closes: erase on "Yes"
        SUB_NUM,
    };

    // The buttons of the erase dialog (dMenuSlctB_c).
    enum DIALOG_BTN_e {
        DIALOG_BTN_NO,
        DIALOG_BTN_YES,
        DIALOG_BTN_UNUSED, // hidden
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_WRITE = 0xB5,
        MSG_ERASE = 0xB9,
        MSG_ERASE_CONFIRM = 0xBA, // "Definitely erase?"
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_CLOSE = 0xEB,
    };

    // The balloons of the jump buttons (SYS_2D_UIballoon.bmg): "First" and
    // "Newest" (the next code).
    enum {
        BALLOON_MSG_FIRST = 0x2E,
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isDot(int cursor) {
        return cursor >= CURSOR_DOT && cursor <= CURSOR_DOT_LAST;
    }
    static bool isScrl(int cursor) {
        return cursor >= CURSOR_PREV && cursor <= CURSOR_NEXT;
    }
    static bool isJump(int cursor) {
        return cursor >= CURSOR_FIRST && cursor <= CURSOR_NEWEST;
    }

    virtual int create();     // .text:0x170
    virtual int doDelete();   // .text:0x2B0
    virtual int stateMain();  // .text:0x304
    virtual int stateAnm();   // .text:0x328
    virtual int stateClose(); // .text:0x3CC
    virtual int execute();    // .text:0x3F0
    virtual int draw();       // .text:0x4B0

    void subStateMain();         // .text:0x53C
    void subStatePush();         // .text:0x6DC
    void subStateSlide();        // .text:0x718
    void subStateDialogOpen();   // .text:0x780
    void subStateDialogAnswer(); // .text:0x7E0
    void subStateDialogClose();  // .text:0x844
    void executeSubState();      // .text:0x8FC

    static BOOL isTravelling();              // .text:0x934
    void initButtons();                      // .text:0x95C
    void setSelect(int cursor, BOOL select); // .text:0x9F8
    int hitCheck();                          // .text:0xB48
    void pushCursor();                       // .text:0xC64
    BOOL isPushEnd();                        // .text:0xDB0
    void finishPush();                       // .text:0xEA4
    void closeMenu();                        // .text:0xFB8
    void openWrite();                        // .text:0x1014
    void initList();                         // .text:0x1074
    void updateDots();                       // .text:0x10DC
    void showNotice();                       // .text:0x117C
    BOOL isSelectable(int cursor);           // .text:0x11F0
    void changeNotice(int idx);              // .text:0x128C
    void initAnms();                         // .text:0x13B4
    void calcAnms();                         // .text:0x1534
    void startDotPush(int dot);              // .text:0x1628
    void updateScrlBtnAnm();                 // .text:0x16B4
    void setDotFocus(int dot);               // .text:0x1750
    void showTutorial(int cursor);           // .text:0x17F4
    void playSeClose();                      // .text:0x18C0
    void playSeOpen();                       // .text:0x18C8
    void playSeJump();                       // .text:0x18D0
    void playSePageTurn();                   // .text:0x18D8
    void openEraseDialog();                  // .text:0x18E0

    // Implicit destructor: .text:0x1990.

    // Inline virtuals of the base class: .text:0x1AA8 .. 0x1AC0.
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

    /* 0x0D4 */ m2d::SimpleEx_c mLyt;       // board_win_base_00
    /* 0x16C */ m2d::SimpleEx_c mCursorLyt; // board_win_cursor_00
    /* 0x204 */ m2d::AnmEx2_c mDotAnm[DOT_NUM];
    /* 0x600 */ m2d::Anm_c mDotPushAnm;
    /* 0x624 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM];
    /* 0x69C */ m2d::AnmEx2_c mJumpAnm[JUMP_NUM];
    /* 0x724 */ m2d::Anm_c mJumpPushAnm[JUMP_NUM];
    /* 0x76C */ dMenuBtn_c mBtn;
    /* 0xCB0 */ dNotice_c mNotice; // the sliding-in paper
    /* 0x1484 */ dMenuSlctB_c mSlct;
    /* 0x1B74 */ int mCursor;
    /* 0x1B78 */ int mCount;   // the notices of the board
    /* 0x1B7C */ int mCurrent; // the shown notice, NO_NOTICE none
}; // size 0x1B80

namespace dMenuNotice00 {
void *classInit();
}
