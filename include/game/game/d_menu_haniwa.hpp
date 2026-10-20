#ifndef GAME_D_MENU_HANIWA_HPP
#define GAME_D_MENU_HANIWA_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_line.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_private_data.hpp>

class daFTR_c;

// The gyroid storage list (REL d_menu_haniwaNP, profile MENU_HANIWA): the
// haniwa window (Layout/haniwa_win.arc) with a list of the gyroids the player
// has stored (dPrivateGyroids_c; six lines, scrolled by two buttons or a
// slider), a 3D preview of the selected gyroid and a quit and a confirm
// button. The confirm button asks (dMenuSlctB_c) whether to take the gyroid
// with you; on yes it leaves the storage and goes into the player's pockets.
// A cut down dMenuCatalog_c (no tabs, no price, no papers). RTTI confirms the
// class names dMenuHaniwa_c and dMenuHaniwa_c::order_c; everything else is
// inferred.
class dMenuHaniwa_c : public dMenuBase_c {
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
        CURSOR_SLIDER = 11, // the slider
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
        GYROID_NUM = 127, // the kinds of gyroid (dItem index 0x5C1 ..)
        VISIBLE_ROWS = 4,
        SEL_NONE = -100, // no selection
        SCROLL_MIN = -1,
        SCROLL_PAGE = 3, // the lines a left / right press scrolls
        SLIDER_OFFSET =
            3, // the slider spans mCount - SLIDER_OFFSET steps (none if fewer)
    };

    // Per-line animations (mRowAnm): the file names haniwa_win_00_T_msg_xx.
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

    // The slider animations (mSliderAnm).
    enum SLIDER_ANM_e {
        SLIDER_ANM_IN,
        SLIDER_ANM_OUT,
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
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,          // waits for the pushed button to finish
        SUB_SCROLL,        // keeps scrolling while the button is held
        SUB_DIALOG,        // the dialog opens
        SUB_DIALOG_ANSWER, // waits for the answer
        SUB_DIALOG_CLOSE,  // the dialog closes after "No"
        SUB_SLIDER,        // the slider is dragged
        SUB_NUM,
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
        RESULT_TAKE = 1,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_TAKE = 0x127, // "Take the <item> with you?"
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

    // The preview's node of the 2D callback list: draws the gyroid between the
    // layouts.
    class order_c : public m2d::order3DMdl_c {
      public:
        virtual void execute(); // .text:0x1F7C

        /* 0x10 */ daFTR_c *mpFtr;
    }; // size 0x14

    virtual int create();                    // .text:0x1B0
    virtual int doDelete();                  // .text:0x37C
    virtual int stateMain();                 // .text:0x3EC
    virtual int stateAnm();                  // .text:0x42C
    virtual int stateClose();                // .text:0x4B0
    virtual int execute();                   // .text:0x51C
    virtual int draw();                      // .text:0x5E4
    void subStateMain();                     // .text:0x670
    void subStatePush();                     // .text:0x738
    void subStateScroll();                   // .text:0x774
    void subStateDialog();                   // .text:0x838
    void subStateAnswer();                   // .text:0x898
    void subStateDialogClose();              // .text:0x980
    void subStateSlider();                   // .text:0x9DC
    void executeSubState();                  // .text:0xB4C
    void preMain();                          // .text:0xB84
    void postMain();                         // .text:0xB88
    void setSelect(int cursor, BOOL select); // .text:0xB8C
    int hitCheck();                          // .text:0xCB8
    void checkPadMove();                     // .text:0xDB8
    void pushCursor();                       // .text:0xF2C
    BOOL isPushEnd();                        // .text:0x10C0
    void finishPush();                       // .text:0x1180
    void startCloseAnm();                    // .text:0x12C8
    void initRows();                         // .text:0x132C
    void calcRowAnm();                       // .text:0x13F4
    void updateRowWords();                   // .text:0x1494
    void setRowAnms(int hold, int shift);    // .text:0x1548
    void updateSelBase();                    // .text:0x15F4
    void refreshList();                      // .text:0x1684
    void selectItem(int idx);                // .text:0x16C8
    void updateSelItem();                    // .text:0x1710
    void initButtons();                      // .text:0x17B0
    void setConfirmDisable(BOOL disable);    // .text:0x1810
    void initAnms();                         // .text:0x182C
    void calcScrlBtnAnm();                   // .text:0x18B8
    void setScrlAnm(int dir);                // .text:0x19A8
    BOOL startScroll(int dir);               // .text:0x19C8
    void setScrlBtnAnms();                   // .text:0x1ADC
    bool canScroll(int dir);                 // .text:0x1B5C
    bool canScrollUp();                      // .text:0x1B6C
    bool canScrollDown();                    // .text:0x1B8C
    dPrivateGyroids_c *getGyroids();         // .text:0x1BB0
    void updateItems();                      // .text:0x1BD8
    u16 getNthItem(int n);                   // .text:0x1D6C
    void updatePreview();                    // .text:0x1D9C
    void clearPreview();                     // .text:0x1DF0
    void setupFtr();                         // .text:0x1E2C
    void calcPreview();                      // .text:0x1F34
    void openDialog();                       // .text:0x1F90
    void initSlider();                       // .text:0x206C
    void updateSlider();                     // .text:0x20DC

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x23E4
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x23DC
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x23D8
    virtual void deleteReady() {
    } // .text:0x23D4
    virtual bool createHeap() {
        return true;
    } // .text:0x23CC

    // Implicit destructor: .text:0x218C.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];
    /* 0x3D8 */ m2d::stringWord_c mRowWord[ROW_NUM];
    /* 0x990 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM];
    /* 0xA08 */ m2d::Anm_c mScrlAnm;
    /* 0xA2C */ m2d::Anm_c mSliderIconAnm;
    /* 0xA50 */ dMenuBtn_c mBtn;
    /* 0xF94 */ order_c mOrder;
    /* 0xFA8 */ m2d::zBuffClear_c mZClear;
    /* 0xFD0 */ int mCursor;
    /* 0xFD4 */ int mSel;    // the selected line (list index), SEL_NONE none
    /* 0xFD8 */ int mScroll; // list index of the second line
    /* 0xFDC */ u16 mItem;   // the selected gyroid
    /* 0xFE0 */ dMenuSlctB_c mSlct;
    /* 0x16D0 */ mAng mAngle; // the preview's turn
    /* 0x16D4 */ int mCount;  // stored gyroids
    /* 0x16D8 */ dItem::Item mItems[GYROID_NUM];
    /* 0x17D8 */ m2d::AnmEx_c mSliderAnm;
    /* 0x1814 */ f32 mSliderTop;
    /* 0x1818 */ f32 mSliderBottom;
    /* 0x181C */ nw4r::lyt::Pane *mpSlider;
    /* 0x1820 */ f32 mDragOffset;
}; // size 0x1824

#endif
