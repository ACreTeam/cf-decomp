#ifndef GAME_D_MENU_CATALOG_HPP
#define GAME_D_MENU_CATALOG_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_line.hpp>
#include <game/game/d_catalog.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_paper_view.hpp>

class daFTR_c;

// The furniture catalog (REL d_menu_catalogNP, profile MENU_CATALOG): the
// catalog window (Layout/catalog.arc) with a row of category tabs on top, a
// list of the registered items of the tab (six lines, scrolled by two buttons
// or a slider), the item's price and a 3D preview of the furniture (or a paper
// for stationery). The confirm button asks (dMenuSlctB_c) whether to order the
// item and returns it through the menu manager. RTTI confirms the class names
// dMenuCatalog_c and dMenuCatalog_c::order_c; everything else is inferred.
class dMenuCatalog_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_TAB = 3, // 3..11: the category tabs
        CURSOR_TAB_LAST = CURSOR_TAB + 8,
        CURSOR_ROW = 12, // 12..17: the list lines
        CURSOR_ROW_LAST = CURSOR_ROW + 5,
        CURSOR_SCRL = 18, // 18, 19: scroll up, scroll down
        CURSOR_SCRL_UP = CURSOR_SCRL,
        CURSOR_SCRL_DOWN = CURSOR_SCRL + 1,
        CURSOR_SLIDER = 20, // the slider
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED, // hidden
        BTN_CONFIRM,
    };

    enum {
        TAB_NUM = 9,
        ROW_NUM = 6,
        SCRL_NUM = 2,
        PAPER_NUM = 2,
        VISIBLE_ROWS = 4,
        SEL_NONE = -100, // no selection
        SCROLL_MIN = -1,
        SCROLL_PAGE = 3, // the lines a left / right press scrolls
        SLIDER_OFFSET =
            3, // the slider spans mCount - SLIDER_OFFSET steps (none if fewer)
        CENTER_OFFSET = 2, // the selection is centered on the third line
    };

    // The tabs: Furniture, Wallpaper, Flooring, Clothes, Umbrella, Headgear,
    // Stationery, Gyroids, Fossils.
    enum TAB_e {
        TAB_FURNITURE,
        TAB_WALLPAPER,
        TAB_FLOORING,
        TAB_CLOTHES,
        TAB_UMBRELLA,
        TAB_HEADGEAR,
        TAB_STATIONERY,
        TAB_GYROIDS,
        TAB_FOSSILS,
    };

    // Per-line animations (mRowAnm): the file names catalog_win_00_T_msg_xx.
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

    // The tab row's animations (mTabAnmInOut).
    enum TAB_WIN_ANM_e {
        TAB_WIN_ANM_IN,
        TAB_WIN_ANM_OUT,
    };

    // Tab on / off animations (mTabAnm).
    enum TAB_ANM_e {
        TAB_ANM_ON,
        TAB_ANM_OFF,
    };

    // The tab hover animation (mTabHoverAnm).
    enum TAB_HOVER_ANM_e {
        TAB_HOVER_IN,
        TAB_HOVER_OUT,
        TAB_HOVER_PUSH,
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

    // The paper preview slots' states (mPaperState).
    enum PAPER_STATE_e {
        PAPER_NONE,
        PAPER_LOAD,   // load mPaperId into the slot
        PAPER_SHOWN,  // loaded, drawn
        PAPER_REMOVE, // remove the slot; then load mPaperId if any
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,         // the window layout needs a calc
        FLAG_CALC_TAB,     // the tab layout needs a calc
        FLAG_ROW_ANM,      // a line animation is playing
        FLAG_SCRL_BTN_ANM, // a scroll button animation is playing
        FLAG_SCROLL,       // the list is scrolling
        FLAG_DIALOG,       // draw the order dialog
        FLAG_QUIT,    // the close animation quits the menu (else it changes the
                      // tab)
        FLAG_PREVIEW, // draw the preview
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,          // waits for the pushed button to finish
        SUB_SCROLL,        // keeps scrolling while the button is held
        SUB_DIALOG,        // the order dialog opens
        SUB_DIALOG_ANSWER, // waits for the answer
        SUB_DIALOG_CLOSE,  // the dialog closes after "No"
        SUB_SLIDER,        // the slider is dragged
        SUB_NUM,
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
        RESULT_ORDER = 1,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_BELLS = 0x33,
        MSG_NOT_FOR_SALE = 0x34,
        MSG_ORDER = 0x35,
    };

    // The tab tutorial balloons (SYS_2D_UIballoon.bmg): one per tab, in order.
    enum {
        BALLOON_MSG_FURNITURE =
            0x1D, // also Wallpaper, Flooring, Clothes, Umbrella,
                  // Headgear, Stationery, Gyroids, Fossils
    };

    enum {
        LYT_PRIORITY = 0x86,
        PREVIEW_TURN_SPEED = 0x80,
        PREVIEW_TILT = 0x1000,
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isTab(int cursor) {
        return cursor >= CURSOR_TAB && cursor <= CURSOR_TAB_LAST;
    }
    static bool isRow(int cursor) {
        return cursor >= CURSOR_ROW && cursor <= CURSOR_ROW_LAST;
    }
    static bool isScrl(int cursor) {
        return cursor >= CURSOR_SCRL && cursor <= CURSOR_SCRL_DOWN;
    }

    // The preview's node of the 2D callback list: draws the furniture between
    // the layouts.
    class order_c : public m2d::order3DMdl_c {
      public:
        virtual void execute(); // .text:0x28D4

        /* 0x10 */ int
            mKind; // dCatalog_c kind; kind 9 (stationery) has no model
        /* 0x14 */ daFTR_c *mpFtr;
    }; // size 0x18

    virtual int create();                    // .text:0x1AC
    virtual int doDelete();                  // .text:0x45C
    virtual int stateMain();                 // .text:0x4FC
    virtual int stateAnm();                  // .text:0x53C
    virtual int stateClose();                // .text:0x5F0
    virtual int execute();                   // .text:0x664
    virtual int draw();                      // .text:0x7BC
    void subStateMain();                     // .text:0x858
    void subStatePush();                     // .text:0x934
    void subStateScroll();                   // .text:0x970
    void subStateDialog();                   // .text:0xA34
    void subStateAnswer();                   // .text:0xA94
    void subStateDialogClose();              // .text:0xB44
    void subStateSlider();                   // .text:0xBA0
    void executeSubState();                  // .text:0xD14
    void preMain();                          // .text:0xD4C
    void postMain();                         // .text:0xD50
    void setSelect(int cursor, BOOL select); // .text:0xD54
    int hitCheck();                          // .text:0xEE0
    void checkPadMove();                     // .text:0x101C
    void pushCursor();                       // .text:0x1190
    BOOL isPushEnd();                        // .text:0x1368
    void finishPush();                       // .text:0x1454
    void startCloseAnm();                    // .text:0x1658
    void startTabAnm();                      // .text:0x16C8
    void initRows();                         // .text:0x1724
    void calcRowAnm();                       // .text:0x17EC
    void updateRowWords();                   // .text:0x188C
    void setRowAnms(int hold, int shift);    // .text:0x1940
    void updateSelBase();                    // .text:0x19EC
    void refreshList();                      // .text:0x1A7C
    void selectItem(int idx);                // .text:0x1AC0
    void setTabAnm(int anm);                 // .text:0x1B18
    void initTabAnms();                      // .text:0x1B38
    void calcTabAnm();                       // .text:0x1BB8
    void setTabHover(int tab, int anm);      // .text:0x1CB4
    void selectTab(int tab);                 // .text:0x1D90
    void setKind();                          // .text:0x1E10
    void updateSelItem();                    // .text:0x1EF8
    void initButtons();                      // .text:0x203C
    void setConfirmDisable(BOOL disable);    // .text:0x209C
    void initAnms();                         // .text:0x20B8
    void calcScrlBtnAnm();                   // .text:0x216C
    void setScrlAnm(int dir);                // .text:0x225C
    BOOL startScroll(int dir);               // .text:0x227C
    void setScrlBtnAnms();                   // .text:0x2388
    bool canScroll(int dir);                 // .text:0x2408
    bool canScrollUp();                      // .text:0x2418
    bool canScrollDown();                    // .text:0x2438
    void setScroll(int scroll);              // .text:0x245C
    void addScroll(int delta);               // .text:0x2464
    void centerScroll();                     // .text:0x2474
    void initSlider();                       // .text:0x24B0
    void updateSlider();                     // .text:0x2520
    dCatalog_c *getCatalog();                // .text:0x25D0
    u16 getNthItem(int n);                   // .text:0x25F8
    void updatePreview();                    // .text:0x265C
    void clearPreview();                     // .text:0x271C
    void setupFtr();                         // .text:0x2758
    void calcPreview();                      // .text:0x2870
    void updatePaper(int slot);              // .text:0x28F8
    void drawPaper();                        // .text:0x29F8
    void showTutorial(int cursor);           // .text:0x2A60
    void hideTutorial();                     // .text:0x2B38

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x2DAC
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x2DA4
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x2DA0
    virtual void deleteReady() {
    } // .text:0x2D9C
    virtual bool createHeap() {
        return true;
    } // .text:0x2D94

    // Implicit destructor: .text:0x2B3C.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];
    /* 0x3D8 */ m2d::itemWord_c mRowWord[ROW_NUM];
    /* 0x5A0 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM];
    /* 0x618 */ m2d::Anm_c mScrlAnm;
    /* 0x63C */ m2d::Anm_c mSliderPtrnAnm;
    /* 0x660 */ m2d::Anm_c mSliderIconAnm;
    /* 0x684 */ m2d::SimpleEx_c mTabLyt;
    /* 0x71C */ m2d::Anm_c mTabAnmInOut;
    /* 0x740 */ m2d::Anm_c mTabHoverAnm;
    /* 0x764 */ m2d::AnmEx2_c mTabAnm[TAB_NUM];
    /* 0x9C8 */ m2d::SimpleEx_c mPriceLyt;
    /* 0xA60 */ dMenuBtn_c mBtn;
    /* 0xFA4 */ order_c mOrder;
    /* 0xFBC */ m2d::zBuffClear_c mZClear;
    /* 0xFE4 */ int mCursor;
    /* 0xFE8 */ int mTabHover; // the tab with the hover animation, -1 none
    /* 0xFEC */ int mTab;      // TAB_e
    /* 0xFF0 */ int mSel;      // the selected line (list index), SEL_NONE none
    /* 0xFF4 */ int mCount;    // registered items of the tab
    /* 0xFF8 */ int mScroll;   // list index of the second line
    /* 0xFFC */ int mKind;     // dCatalog_c kind of the tab
    /* 0x1000 */ u16 mItem;    // the selected item
    /* 0x1004 */ m2d::stringWord_c mName; // price or "Not for Sale"
    /* 0x10F8 */ dMenuSlctB_c mSlct;
    /* 0x17E8 */ int mTabSel[TAB_NUM]; // each tab's selection
    /* 0x180C */ mAng mAngle;          // the preview's turn
    /* 0x1810 */ dPaper_c mPaper[PAPER_NUM];
    /* 0x2BF8 */ int mPaperState[PAPER_NUM];
    /* 0x2C00 */ int mPaperId;   // paper to load next, -1 none
    /* 0x2C04 */ int mPaperSlot; // the slot shown
    /* 0x2C08 */ m2d::AnmEx_c mSliderAnm;
    /* 0x2C44 */ f32 mSliderTop;
    /* 0x2C48 */ f32 mSliderBottom;
    /* 0x2C4C */ nw4r::lyt::Pane *mpSlider;
    /* 0x2C50 */ f32 mDragOffset;
}; // size 0x2C54

#endif
