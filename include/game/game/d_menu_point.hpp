#ifndef GAME_D_MENU_POINT_HPP
#define GAME_D_MENU_POINT_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_line.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_private_data.hpp>

class daFTR_c;

// Nook's point machine (REL d_menu_pointNP, profile MENU_POINT, opened as
// dMenuMng_c::MENU_POINT_PRESENT by daRmobjPoint_c): the point_win window
// (Layout/point_win.arc) with a list of the presents the player can swap
// Nook points for (fourteen fixed items; six lines, scrolled by two buttons),
// a 3D preview of the selected present, a price window and the points left,
// and a quit and a confirm button. The confirm button asks (dMenuSlctB_c)
// whether to swap; on yes the points are paid and the menu returns the item
// (dMenuMng_c::setMenuItem). Presents whose price is above the highest point
// balance ever reached show as "??????????". A sibling of dMenuHaniwa_c and
// dMenuCatalog_c. RTTI confirms the class names dMenuPoint_c and
// dMenuPoint_c::order_c; everything else is inferred.
class dMenuPoint_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_ROW = 3, // 3..8: the list lines
        CURSOR_ROW_LAST = CURSOR_ROW + 5,
        CURSOR_SCRL = 9, // 9, 10: scroll up, scroll down
        CURSOR_SCRL_UP = CURSOR_SCRL,
        CURSOR_SCRL_DOWN = CURSOR_SCRL + 1,
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED, // hidden
        BTN_CONFIRM,
    };

    enum {
        ROW_NUM = 6,
        SCRL_NUM = 2,
        ITEM_NUM = 14, // the presents
        VISIBLE_ROWS = 4,
        SEL_NONE = -100, // no selection
        SCROLL_MIN = -1,
        SCROLL_PAGE = 3, // the lines a left / right press scrolls
        LYT_PRIORITY = 0x86,
    };

    // Per-line animations (mRowAnm): the file names point_win_00_T_msg_xx.
    enum ROW_ANM_e {
        ROW_ANM_IN,
        ROW_ANM_OUT,
        ROW_ANM_PUSH,
        ROW_ANM_BEFORE,
        ROW_ANM_GRAY,
        ROW_ANM_WHITE,
    };

    // Scroll button animations (mScrlBtnAnm).
    enum SCRL_ANM_e {
        SCRL_ANM_IN,
        SCRL_ANM_OUT,
        SCRL_ANM_PUSH,
        SCRL_ANM_CAN_PUSH,
        SCRL_ANM_NO_PUSH,
    };

    // The points-left window animations (mRemainAnm).
    enum REMAIN_ANM_e {
        REMAIN_ANM_IN,
        REMAIN_ANM_OUT,
    };

    // The scroll direction (startScroll, canScroll).
    enum SCROLL_e {
        SCROLL_UP,
        SCROLL_DOWN,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,         // the window layout needs a calc
        FLAG_ROW_ANM,      // a line animation is playing
        FLAG_SCRL_BTN_ANM, // a scroll button animation is playing
        FLAG_SCROLL,       // the list is scrolling
        FLAG_DIALOG,       // draw the dialog
        FLAG_QUIT,         // the close animation quits the menu
        FLAG_PREVIEW,      // draw the preview
        FLAG_REMAIN_CALC,  // the points-left layout needs a calc
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,          // waits for the pushed button to finish
        SUB_SCROLL,        // keeps scrolling while the button is held
        SUB_DIALOG,        // the dialog opens
        SUB_DIALOG_ANSWER, // waits for the answer
        SUB_DIALOG_CLOSE,  // the dialog closes after "No"
        SUB_NUM,
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
        RESULT_SWAP = 1,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_OK = 0xEC,            // "OK!"
        MSG_POINTS = 0x119,       // "<number> Points"
        MSG_UNKNOWN_ITEM = 0x11A, // "??????????"
        MSG_SWAP = 0x11B,         // "Swap with the <item>?"
        MSG_LEFT = 0x11C,         // "Left:"
        MSG_NEED_POINTS =
            0x145, // "You need more points to swap for that item!"
    };

    enum {
        PREVIEW_TURN_SPEED = 0x80,
        PREVIEW_TILT = 0x1000,
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isRow(int cursor) {
        return cursor >= CURSOR_ROW && cursor <= CURSOR_ROW_LAST;
    }
    static bool isScrl(int cursor) {
        return cursor >= CURSOR_SCRL && cursor <= CURSOR_SCRL_DOWN;
    }

    // The preview's node of the 2D callback list: draws the present between
    // the layouts.
    class order_c : public m2d::order3DMdl_c {
      public:
        virtual void execute(); // .text:0x1E4C

        /* 0x10 */ daFTR_c *mpFtr;
    }; // size 0x14

    virtual int create();                    // .text:0x1C8
    virtual int doDelete();                  // .text:0x3FC
    virtual int stateMain();                 // .text:0x46C
    virtual int stateAnm();                  // .text:0x4AC
    virtual int stateClose();                // .text:0x560
    virtual int execute();                   // .text:0x5D8
    virtual int draw();                      // .text:0x6F0
    void subStateMain();                     // .text:0x78C
    void subStatePush();                     // .text:0x854
    void subStateScroll();                   // .text:0x890
    void subStateDialog();                   // .text:0x954
    void subStateAnswer();                   // .text:0x9B4
    void subStateDialogClose();              // .text:0xA84
    void executeSubState();                  // .text:0xAE0
    void preMain();                          // .text:0xB18
    void postMain();                         // .text:0xB1C
    void setSelect(int cursor, BOOL select); // .text:0xB20
    int hitCheck();                          // .text:0xC48
    void checkPadMove();                     // .text:0xD30
    void pushCursor();                       // .text:0xE84
    BOOL isPushEnd();                        // .text:0xFD0
    void finishPush();                       // .text:0x1090
    void startCloseAnm();                    // .text:0x1210
    void initRows();                         // .text:0x1280
    void calcRowAnm();                       // .text:0x1348
    void updateRowWords();                   // .text:0x13E0
    void setRowAnms(int hold, int shift);    // .text:0x14D0
    void updateSelBase();                    // .text:0x157C
    void refreshList();                      // .text:0x160C
    void selectItem(int idx);                // .text:0x1650
    void updateSelItem();                    // .text:0x1698
    void initButtons();                      // .text:0x1818
    void setConfirmDisable(BOOL disable);    // .text:0x1878
    void initAnms();                         // .text:0x1894
    void calcScrlBtnAnm();                   // .text:0x18F4
    void setScrlAnm(int dir);                // .text:0x19E4
    BOOL startScroll(int dir);               // .text:0x1A04
    void setScrlBtnAnms();                   // .text:0x1B10
    bool canScroll(int dir);                 // .text:0x1B90
    bool canScrollUp();                      // .text:0x1BA0
    bool canScrollDown();                    // .text:0x1BC0
    u16 getNthItem(int n);                   // .text:0x1BE0
    u16 getNthPrice(int n);                  // .text:0x1C40
    void updatePreview();                    // .text:0x1C6C
    void clearPreview();                     // .text:0x1CC0
    void setupFtr();                         // .text:0x1CFC
    void calcPreview();                      // .text:0x1E04
    u16 getPoints();                         // .text:0x1E60
    u16 getPointsMax();                      // .text:0x1E84
    void subPoints(int points);              // .text:0x1EA8
    void openDialog();                       // .text:0x1EDC
    void openNeedPointsDialog();             // .text:0x1FB8
    void initRemain();                       // .text:0x2064
    void calcRemainAnm();                    // .text:0x213C

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x239C
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x2394
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x2390
    virtual void deleteReady() {
    } // .text:0x238C
    virtual bool createHeap() {
        return true;
    } // .text:0x2384

    // Implicit destructor: .text:0x2180.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];
    /* 0x3D8 */ m2d::stringWord_c mRowWord[ROW_NUM];
    /* 0x990 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM];
    /* 0xA08 */ m2d::Anm_c mScrlAnm;
    /* 0xA2C */ m2d::Anm_c mScrlIconAnm;
    /* 0xA50 */ m2d::SimpleEx_c mPriceLyt;
    /* 0xAE8 */ m2d::SimpleEx_c mRemainLyt;
    /* 0xB80 */ m2d::AnmEx_c mRemainAnm;
    /* 0xBBC */ dMenuBtn_c mBtn;
    /* 0x1100 */ order_c mOrder;
    /* 0x1114 */ m2d::zBuffClear_c mZClear;
    /* 0x113C */ int mCursor;
    /* 0x1140 */ int mSel;    // the selected line (list index), SEL_NONE none
    /* 0x1144 */ int _1144;   // never accessed
    /* 0x1148 */ int mScroll; // list index of the second line
    /* 0x114C */ u16 mItem;   // the selected present
    /* 0x1150 */ m2d::stringWord_c mPriceWord;
    /* 0x1244 */ dMenuSlctB_c mSlct;
    /* 0x1934 */ mAng mAngle; // the preview's turn
}; // size 0x1938

#endif
