#pragma once

// The bulletin board write menu (REL d_menu_notice01NP, profile
// MENU_NOTICE01, menu number dMenuMng_c::MENU_BBS_WRITE): the sub menu of
// dMenuNotice_c (REL d_menu_noticeNP) in which a new notice is written. The
// notice paper (dNotice_c, owned by the open dMenuNotice_c and reached through
// its static functions) is edited with the software keyboard (dKbd::Mng_c) and
// a text cursor (dKbd::Cur_c); the menu adds a button bar (Quit, Preview,
// Done), two arrow buttons that move the text cursor between the lines
// (dMenuLcrsr_c) and two dialogs. It is a close relative of dMenuWrite_c. RTTI
// confirms the class name dMenuNotice01_c; everything else is inferred.

#include <game/game/d_kbd.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_lcrsr.hpp>
#include <game/game/d_menu_notice.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_menu_slct_c.hpp>

class dMenuNotice01_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_LINE = 3, // 3, 4: the arrows (dMenuLcrsr_c buttons)
        CURSOR_LINE_UP = CURSOR_LINE,
        CURSOR_LINE_DOWN = CURSOR_LINE + 1,
    };

    // The dMenuBtn_c buttons. In the preview only the last one (Back) is shown.
    enum BTN_e {
        BTN_QUIT,
        BTN_PREVIEW,
        BTN_DONE,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_INIT = 2,   // the keyboard is initialised
        FLAG_PAPER = 3,  // the notice paper follows the layout and the offset
        FLAG_TEXT = 4,   // the notice text needs an update
        FLAG_CURSOR = 5, // the text cursor is shown
        FLAG_MOVING = 6, // the paper is scrolling, the cursor is shown at the
                         // end
        FLAG_SLCT_B = 7, // the dialog box is shown
        FLAG_SLCT_C = 8, // the small dialog box is shown
        FLAG_CALC_MARKS = 10, // the line end marks need a calc
        FLAG_MARKS = 11,      // the line end marks are shown
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,         // waits for the pushed button to finish
        SUB_LINE_HOLD,    // an arrow is held
        SUB_PREVIEW,      // shows the written notice
        SUB_PREVIEW_PUSH, // Back was pushed
        SUB_RETURN,       // waits for the paper to scroll back
        SUB_QUIT,         // "Quit without writing anything?"
        SUB_QUIT_PUSH,
        SUB_QUIT_END,
        SUB_DONE, // Edit / Confirm
        SUB_DONE_PUSH,
        SUB_DONE_END,
        SUB_CLOSE_WAIT, // the notice was posted: waits for the dialog to close
        SUB_NUM,
    };

    enum {
        MARK_NUM = dNotice_c::LINE_NUM,
        TEXT_SIZE = 0xC1,   // characters in the text buffers
        TEXT_BYTES = 0x180, // bytes copied of a text
        KBD_MAX_LENGTH = 0xC2,
        LYT_PRIORITY = 0x80, // of the paper while it is written
        MOVE_MAX_STEPS = 20, // of the paper scroll
        MOVE_MIN_STEPS = 10,
        MOVE_DECEL = 8,
        LINE_LAST = dNotice_c::LINE_NUM - 1,
    };

    // The arrow buttons (mLcrsr): the direction the text cursor moves.
    enum DIR_e {
        DIR_UP,
        DIR_DOWN,
    };

    // The buttons of the dialogs (mSlctB, mSlctC).
    enum SLCT_BTN_e {
        SLCT_BTN_NO = 0, // mSlctB
        SLCT_BTN_YES = 1,
        SLCT_BTN_UNUSED = 2,
        SLCT_BTN_EDIT = 0, // mSlctC
        SLCT_BTN_CONFIRM = 1,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x1F,
        MSG_BACK = 0x20,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_PREVIEW = 0x44,
        MSG_QUIT_WITHOUT_WRITING = 0xB7,
        MSG_DONE = 0xBB,
        MSG_EDIT = 0xBC,
        MSG_CONFIRM = 0xBD,
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isLine(int cursor) {
        return cursor >= CURSOR_LINE_UP && cursor <= CURSOR_LINE_DOWN;
    }

    // The paper isn't scrolling (the move counter is negative when idle).
    BOOL isMoveEnd() {
        return (u32)mMove.mCount >> 31;
    }

    virtual int create();                    // .text:0x100
    virtual int doDelete();                  // .text:0x2D8
    virtual int stateMain();                 // .text:0x3B8
    virtual int stateAnm();                  // .text:0x3DC
    virtual int stateClose();                // .text:0x4DC
    virtual int execute();                   // .text:0x500
    virtual int draw();                      // .text:0x658
    void subStateMain();                     // .text:0x760
    void subStatePush();                     // .text:0x8D0
    void subStateLineHold();                 // .text:0x90C
    void subStatePreview();                  // .text:0x9A8
    void subStatePreviewPush();              // .text:0xAA8
    void subStateReturn();                   // .text:0xB04
    void subStateQuit();                     // .text:0xB58
    void subStateQuitPush();                 // .text:0xBB4
    void subStateQuitEnd();                  // .text:0xC5C
    void subStateDone();                     // .text:0xCB8
    void subStateDonePush();                 // .text:0xD14
    void subStateDoneEnd();                  // .text:0xDA0
    void subStateCloseWait();                // .text:0xDFC
    void executeSubState();                  // .text:0xE4C
    void setSelect(int cursor, BOOL select); // .text:0xE84
    int hitCheck();                          // .text:0xF28
    void pushCursor();                       // .text:0xFB4
    BOOL isPushEnd();                        // .text:0x1090
    void finishPush();                       // .text:0x111C
    void startClose();         // .text:0x1230: close, back to the newest notice
    void finish();             // .text:0x12A0: post the notice
    void startPreview();       // .text:0x1320
    void backToEdit();         // .text:0x13B0
    void openQuitDialog();     // .text:0x1424
    void openDoneDialog();     // .text:0x14D4
    void initBtnText();        // .text:0x159C
    void initPreviewBtnText(); // .text:0x1600
    bool checkText();          // .text:0x165C
    void markTextChanged();    // .text:0x16F4
    void updateLineArrows();   // .text:0x1730
    void revertText();         // .text:0x1794
    bool hitPaper();           // .text:0x17F8
    void updateCursorPos();    // .text:0x1944
    int getTextLine();         // .text:0x198C
    void updateLine(BOOL force);  // .text:0x19B4
    void calcMove();              // .text:0x1A18
    void resetMove();             // .text:0x1AB4
    void startLineMove(int line); // .text:0x1B08
    void startHomeMove(f32 dy);   // .text:0x1B7C
    bool canMoveLine(int dir);    // .text:0x1BDC
    BOOL moveLine();              // .text:0x1C58
    void startMove();             // .text:0x1D48
    void initMarks();             // .text:0x1D6C
    void drawMarks();             // .text:0x1DD8
    void calcMarks();             // .text:0x1E24

    // Implicit destructor: .text:0x1F1C.

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x2098
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x2090
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x208C
    virtual void deleteReady() {
    } // .text:0x2088
    virtual bool createHeap() {
        return true;
    } // .text:0x2080

    /* 0x0D4 */ dMenuBtn_c mBtn;
    /* 0x618 */ dKbd::Mng_c mKbd;
    /* 0x990 */ dKbd::Cur_c mKbdCur;
    /* 0xB48 */ dMenuLcrsr_c mLcrsr;
    /* 0xE7C */ dMenuSlctB_c mSlctB;
    /* 0x156C */ dMenuSlctC_c mSlctC;
    /* 0x1A80 */ int _1A80; // never used
    /* 0x1A84 */ int mCursor;
    /* 0x1A88 */ int mLine;                   // the line of the cursor
    /* 0x1A8C */ wchar_t mText[TEXT_SIZE];    // the text being edited
    /* 0x1C0E */ wchar_t mOldText[TEXT_SIZE]; // ... as the keyboard knew it
    /* 0x1D90 */ m2d::moveCalc_c mMove;
    /* 0x1DA8 */ mVec2_c mPos;    // of the paper
    /* 0x1DB0 */ mVec2_c mTarget; // ... where it moves
    /* 0x1DB8 */ m2d::ResAccLoader_c mRes;
    /* 0x1E8C */ m2d::SimpleEx_c mMark[MARK_NUM]; // the line end marks
}; // size 0x221C
