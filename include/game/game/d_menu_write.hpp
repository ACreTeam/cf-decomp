#ifndef GAME_D_MENU_WRITE_HPP
#define GAME_D_MENU_WRITE_HPP

#include <game/game/d_kbd.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_lcrsr.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_menu_slct_c.hpp>
#include <game/game/d_paper_view.hpp>

// The letter writing menu (REL d_menu_writeNP, profile MENU_WRITE): the
// stationery (dPaper_c) with the software keyboard (dKbd::Mng_c) and a text
// cursor (dKbd::Cur_c), a button bar (Quit, Preview, Done) and two arrow
// buttons that move the text cursor between the lines (dMenuLcrsr_c). One menu
// for four requests, selected by mMenuNo: MENU_WRITE_LETTER and
// MENU_REWRITE_LETTER write a letter into the player's pocket / letter slot,
// MENU_WRITE_BOARD and MENU_WRITE_SELF write a Wii message (see
// dMenuMng_c::setupPost). RTTI confirms the class name dMenuWrite_c; everything
// else is inferred.
class dMenuWrite_c : public dMenuBase_c {
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
        FLAG_CALC_PAPER, // the paper needs an execute
        FLAG_TEXT,       // the paper text needs an update
        FLAG_CURSOR,     // the text cursor is shown
        FLAG_MOVING, // the paper is scrolling, the cursor is shown at the end
        FLAG_SLCT_B, // the dialog box is shown
        FLAG_SLCT_C, // the small dialog box is shown
        FLAG_INIT,   // the mail is set up
        FLAG_QUIT,   // the menu is closing
        FLAG_CALC_MARKS,    // the line end marks need a calc
        FLAG_MARKS,         // the line end marks are shown (not in the preview)
        FLAG_MARKS_PENDING, // show the marks after the paper stopped scrolling
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,         // waits for the pushed button to finish
        SUB_LINE_HOLD,    // an arrow is held
        SUB_PREVIEW,      // shows the written letter
        SUB_PREVIEW_PUSH, // Back was pushed
        SUB_RETURN,       // waits for the paper to scroll back
        SUB_QUIT,         // "Close without saving?"
        SUB_QUIT_PUSH,
        SUB_QUIT_END,
        SUB_DONE, // Edit / Confirm
        SUB_DONE_PUSH,
        SUB_DONE_END,
        SUB_NUM,
    };

    enum {
        MARK_NUM = dPaper_c::BODY_LINES,
        TEXT_SIZE = 0xC1,   // characters in the text buffers (a body)
        HEADER_SIZE = 0x40, // bytes in the mail's header / footer
        BODY_SIZE = 0x180,  // bytes in the mail's body
        LINE_NUM = 8,       // the paper's lines: header, six body lines, footer
        LINE_FOOTER = LINE_NUM - 1,
        KBD_MAX_LENGTH = 0xC2,
        PAPER_ITEM = 0x17F, // item index of the stationery
        LINE_BODY_FIRST = 1,
        MOVE_MAX_STEPS = 20, // of the paper scroll
        MOVE_MIN_STEPS = 10,
        MOVE_DECEL = 8,
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

    // dKbd::Mng_c::getSpecialKey: the keys that move the cursor.
    enum SPECIAL_KEY_e {
        SPECIAL_KEY_LEFT = 1,
        SPECIAL_KEY_RIGHT = 2,
        SPECIAL_KEY_UP = 3,
        SPECIAL_KEY_DOWN = 4,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x1F,
        MSG_BACK = 0x20,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_PREVIEW = 0x44,
        MSG_TO = 0x6E,               // "To <friend>"
        MSG_FROM_PLAYER = 0x8E,      // "From <player>"
        MSG_FROM_PLAYER_TOWN = 0x8F, // "From <player> of <town>"
        MSG_FROM_PLAYER_B = 0x90,    // "From <player>"
        MSG_DATE = 0x91,             // "<month> <day>, <year>"
        MSG_DONE = 0xBB,
        MSG_EDIT = 0xBC,
        MSG_CONFIRM = 0xBD,
        MSG_CLOSE_WITHOUT_SAVING = 0x123,
        MSG_FROM_FRIEND_TOWN = 0x162, // "From <friend> of <town>"
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

    virtual int create();                    // .text:0x128
    virtual int doDelete();                  // .text:0x3D4
    virtual int stateMain();                 // .text:0x4BC
    virtual int stateAnm();                  // .text:0x51C
    virtual int stateClose();                // .text:0x5BC
    virtual int execute();                   // .text:0x63C
    virtual int draw();                      // .text:0x7FC
    void subStateMain();                     // .text:0x8D4
    void subStatePush();                     // .text:0xA18
    void subStateLineHold();                 // .text:0xA54
    void subStatePreview();                  // .text:0xAF0
    void subStatePreviewPush();              // .text:0xBFC
    void subStateReturn();                   // .text:0xC4C
    void subStateQuit();                     // .text:0xCA0
    void subStateQuitPush();                 // .text:0xCFC
    void subStateQuitEnd();                  // .text:0xDBC
    void subStateDone();                     // .text:0xE18
    void subStateDonePush();                 // .text:0xE74
    void subStateDoneEnd();                  // .text:0xF14
    void startQuit();                        // .text:0xF70
    BOOL onCloseMain();                      // .text:0xFC4
    BOOL onClosePreview();                   // .text:0x1004
    BOOL onCloseQuit();                      // .text:0x103C
    BOOL onCloseDone();                      // .text:0x1098
    BOOL handleCloseRequest();               // .text:0x10EC
    void executeSubState();                  // .text:0x123C
    void initBtnText();                      // .text:0x1274
    void initPreviewBtnText();               // .text:0x12D8
    void setSelect(int cursor, BOOL select); // .text:0x1334
    int hitCheck();                          // .text:0x13D8
    void pushCursor();                       // .text:0x1464
    BOOL isPushEnd();                        // .text:0x1540
    void finishPush();                       // .text:0x15CC
    void startClose();                       // .text:0x16E4
    void backToEdit();                       // .text:0x1748
    void openQuitDialog();                   // .text:0x17C8
    void openDoneDialog();                   // .text:0x1878
    void startPreview();                     // .text:0x193C
    void finish();                           // .text:0x19C4
    void saveText();                         // .text:0x1A4C
    void updateNamePos();                    // .text:0x1AEC
    void updateLineArrows();                 // .text:0x1B6C
    bool checkText();                        // .text:0x1BD0
    void revertText();                       // .text:0x1F10
    bool hitPaper();                         // .text:0x1F98
    void updateCursorPos();                  // .text:0x20DC
    int getTextLine();                       // .text:0x219C
    void updateLine(BOOL force);             // .text:0x2220
    void setPart(int part);                  // .text:0x2284
    void calcMove();                         // .text:0x238C
    void resetMove();                        // .text:0x2488
    void startLineMove(int line);            // .text:0x24A8
    void startHomeMove();                    // .text:0x251C
    bool canMoveLine(int dir);               // .text:0x2578
    BOOL moveLine();                         // .text:0x25C8
    void startMove();                        // .text:0x26FC
    void initBoardMail();                    // .text:0x2720
    void finishBoard();                      // .text:0x2830
    void initSelfMail();                     // .text:0x28F8
    void finishSelf();                       // .text:0x2B20
    void buildPostMsg(BOOL friendKey);       // .text:0x2BF4
    void setTownWords();                     // .text:0x2E88
    void finishWriteLetter();                // .text:0x2F0C
    void finishRewriteLetter();              // .text:0x30E0
    void initMarks();                        // .text:0x3254
    void drawMarks();                        // .text:0x32C0
    void calcMarks();                        // .text:0x330C

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x35BC
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x35B4
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x35B0
    virtual void deleteReady() {
    } // .text:0x35AC
    virtual bool createHeap() {
        return true;
    } // .text:0x35A4

    // Implicit destructor: .text:0x340C.

    /* 0x0D4 */ dPaper_c mPaper;
    /* 0xAC8 */ dMenuBtn_c mBtn;
    /* 0x100C */ dMenuLcrsr_c mLcrsr;
    /* 0x1340 */ int mCursor;
    /* 0x1344 */ dKbd::Mng_c mKbd;
    /* 0x16BC */ dKbd::Cur_c mKbdCur;
    /* 0x1874 */ dMenuSlctB_c mSlctB;
    /* 0x1F64 */ dMenuSlctC_c mSlctC;
    /* 0x2478 */ int mPart; // dPaper_c::TextPart_e: the part being edited
    /* 0x247C */ int mLine; // the line of the cursor (dPaper_c line numbers)
    /* 0x2480 */ dMail_c *mpMail;
    /* 0x2484 */ wchar_t mText[TEXT_SIZE];    // the part being edited
    /* 0x2606 */ wchar_t mOldText[TEXT_SIZE]; // ... as the keyboard knew it
    /* 0x2788 */ int mMaxLength;              // of the part (characters)
    /* 0x278C */ int mLength;                 // of mText
    /* 0x2790 */ int mCursorPos;              // of the keyboard
    /* 0x2794 */ int mTextLength;             // of the keyboard
    /* 0x2798 */ m2d::moveCalc_c mMove;
    /* 0x27B0 */ mVec2_c mPos;    // of the paper
    /* 0x27B8 */ mVec2_c mTarget; // ... where it moves
    /* 0x27C0 */ m2d::ResAccLoader_c mRes;
    /* 0x2894 */ m2d::SimpleEx_c mMark[MARK_NUM]; // the line end marks
    /* 0x2C24 */ m2d::zBuffClear_c mZClear;
}; // size 0x2C4C

#endif
