#ifndef GAME_D_MENU_INSC_HPP
#define GAME_D_MENU_INSC_HPP

#include <game/cLib/c_line.hpp>
#include <game/mLib/m_tex.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_line.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_item.hpp>

// The insect and fish encyclopedia (REL d_menu_inscNP, profile MENU_INSC; menus
// 3 and 4 of the menu table): the window insectsFish_win_00
// (Layout/insectsFish.arc) with two tabs on top (Bugs, Fish), a vertical list
// of 5 icons (dMenuItem::Icon_c, Layout/icon.arc) of the creatures the player
// has caught (dCatalog_c kinds 7 and 8, 64 each), a scroll bar with a bar
// texture marking the registered ones, the name and description of the selected
// creature, its 3D model (dFIMdlMgr_c, drawn through the m2d callback list by
// order_c) and a balloon with the months it appears (SYS_2D_bug_time /
// SYS_2D_fish_time). There is no button bar: the bottom tabs of dAcUitag are
// used to leave. RTTI confirms the class names dMenuInsc_c and
// dMenuInsc_c::order_c; everything else is inferred.
class dMenuInsc_c : public dMenuBase_c {
  public:
    enum {
        TAB_NUM = 2,
        ROW_NUM = 5,
        SCRL_NUM = 2,
        HOLE_NUM = 8, // the icon slots: the five rows and one above / two below
        HOLE_FIRST_ROW = 2,
        ITEM_NUM = 64, // the creatures of a kind
    };

    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_UITAG = 0, // 0..7: the dAcUitag tabs at the bottom of the screen
        CURSOR_UITAG_LAST = 7,
        CURSOR_TAB = 8, // 8, 9: the Bugs / Fish tabs
        CURSOR_TAB_LAST = CURSOR_TAB + TAB_NUM - 1,
        CURSOR_ROW = 10, // 10..14: the five list icons
        CURSOR_ROW_LAST = CURSOR_ROW + ROW_NUM - 1,
        CURSOR_SCRL = 15, // 15, 16: scroll up, scroll down
        CURSOR_SCRL_UP = CURSOR_SCRL,
        CURSOR_SCRL_DOWN = CURSOR_SCRL + 1,
        CURSOR_SLIDER = 17, // the slider (N_scaleCrsr)
        CURSOR_INFO = 18,   // the selected creature: shows the months balloon
    };

    // Window layout: which half of insectsFish_win_00 a layout shows.
    enum LYT_e {
        LYT_SLIDER,
        LYT_MAIN,
    };

    // The scroll direction (startScroll, canScroll).
    enum SCROLL_e {
        SCROLL_UP,
        SCROLL_DOWN,
    };

    // The scroll button animations (mSldAnm).
    enum SLD_ANM_e {
        SLD_ANM_IN,
        SLD_ANM_OUT,
        SLD_ANM_PUSH,
        SLD_ANM_NUM,
    };

    // Slider handle animations (mScaleAnm).
    enum SCALE_ANM_e {
        SCALE_ANM_IN,
        SCALE_ANM_OUT,
    };

    // Icon hole animations (mHoleAnm).
    enum HOLE_ANM_e {
        HOLE_ANM_IN,
        HOLE_ANM_OUT,
    };

    // The months balloon animations (mBlnAnm).
    enum BLN_ANM_e {
        BLN_ANM_IN,
        BLN_ANM_OUT,
    };

    // The selected creature's circle animations (mInfoAnm).
    enum INFO_ANM_e {
        INFO_ANM_IN,
        INFO_ANM_OUT,
    };

    // Top tab animations (mTabHoverAnm).
    enum TAB_HOVER_ANM_e {
        TAB_HOVER_IN,
        TAB_HOVER_OUT,
        TAB_HOVER_PUSH,
    };

    // The tab row's animations (mTabWinAnm).
    enum TAB_WIN_ANM_e {
        TAB_WIN_ANM_IN,
        TAB_WIN_ANM_OUT,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_CALC,        // the slider layout needs a calc
        FLAG_DRAW,        // draw the layouts
        FLAG_CALC_TAB,    // the tab layout needs a calc
        FLAG_SWITCH,      // the close animation switches the kind instead of
                          // quitting
        FLAG_FISH,        // the Fish tab is shown (else Bugs)
        FLAG_SCROLL,      // a one step scroll animation plays
        FLAG_SCROLL_CALC, // the scroll position changed: update the edge icons
        FLAG_SLIDER,      // the slider is dragged
        FLAG_MODEL,       // the model is loaded: draw it
        FLAG_MODEL_REQ,   // request the model of the selected creature
        FLAG_BLN_CALC,    // the balloon layout needs a calc
        FLAG_BLN,         // draw the balloon
        FLAG_ICON_CALC,   // the icons need a calc
        FLAG_ICON_UPDATE, // the icons' items changed
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,   // waits for the pushed tab
        SUB_SCROLL, // keeps scrolling while the button is held
        SUB_SLIDER, // the slider is dragged
        SUB_NUM,
    };

    // The first item index (dItem::Item::setFromIndex) of the kinds: the 64
    // creatures are consecutive.
    enum {
        ITEM_INDEX_INSECT = 0x192,
        ITEM_INDEX_FISH = 0x1D2,
    };

    // The tab tutorial balloons (SYS_2D_UIballoon.bmg): "Bugs", then "Fish".
    enum {
        BALLOON_MSG_BUGS = 0x15,
    };

    // dObjTutorial_c::request: the creature icon of a row.
    enum {
        TUTORIAL_ITEM = 0xC,
    };

    enum {
        LYT_PRIORITY = 0x86,
        SLIDER_RANGE = 206, // the slider handle moves between -206 and 206
        SLIDER_LENGTH = SLIDER_RANGE * 2,
    };

    static bool isUitag(int cursor) {
        return cursor >= CURSOR_UITAG && cursor <= CURSOR_UITAG_LAST;
    }
    static bool isTab(int cursor) {
        return cursor >= CURSOR_TAB && cursor <= CURSOR_TAB_LAST;
    }
    static bool isRow(int cursor) {
        return cursor >= CURSOR_ROW && cursor <= CURSOR_ROW_LAST;
    }

    // The model's node of the 2D callback list: draws the creature between the
    // layouts.
    class order_c : public m2d::order3DMdl_c {
      public:
        virtual void execute(); // .text:0x32A4
    };

    virtual int create();                             // .text:0x368
    virtual int doDelete();                           // .text:0x6FC
    virtual int execute();                            // .text:0x754
    virtual int stateMain();                          // .text:0x828
    virtual int stateAnm();                           // .text:0x87C
    virtual int stateClose();                         // .text:0x95C
    virtual void postExecute(MAIN_STATE_e state);     // .text:0xA20
    virtual int draw();                               // .text:0xBEC
    void subStateMain();                              // .text:0xC88
    void subStatePush();                              // .text:0xEC0
    void subStateScroll();                            // .text:0xF54
    void subStateSlider();                            // .text:0x10A0
    BOOL closeNow();                                  // .text:0x1208
    BOOL checkCloseRequest();                         // .text:0x1240
    void executeSubState();                           // .text:0x1314
    void initTab();                                   // .text:0x134C
    void calcTabHoverAnm();                           // .text:0x13B8
    int hitTab(const mVec2_c *pos);                   // .text:0x13FC
    void pushTab(int tab);                            // .text:0x1478
    void followTab();                                 // .text:0x1510
    int hitUitag(const mVec2_c *pos);                 // .text:0x1588
    BOOL selectUitag(int tab, BOOL delay);            // .text:0x15EC
    void startCloseAnm();                             // .text:0x163C
    void pushCursor();                                // .text:0x16AC
    void checkPadMove();                              // .text:0x1778
    BOOL isRegistered(int idx);                       // .text:0x18CC
    BOOL isRowRegistered(int cursor);                 // .text:0x1948
    int hitList(const mVec2_c *pos);                  // .text:0x1980
    void initAnms();                                  // .text:0x1ACC
    void calcAnms();                                  // .text:0x1B98
    void calcScroll();                                // .text:0x1C44
    BOOL startScroll(int dir);                        // .text:0x1D90
    void startSliderDrag();                           // .text:0x1E94
    void resetScrollAnm();                            // .text:0x1F44
    void setScrollAnm(int dir);                       // .text:0x1F80
    void updateSliderPane();                          // .text:0x1FF8
    void updateScrollFromSlider();                    // .text:0x2068
    void setScroll(int scroll);                       // .text:0x21D0
    void selectItem(int idx);                         // .text:0x2300
    void reselectItem();                              // .text:0x24D4
    void initKind();                                  // .text:0x2530
    void resetList();                                 // .text:0x2628
    void updateBar();                                 // .text:0x2678
    void setKindAnms();                               // .text:0x27C8
    void setTabOnAnm(int kind);                       // .text:0x283C
    void setTabWinAnm(int anm);                       // .text:0x285C
    void setSelect(int cursor, BOOL select);          // .text:0x287C
    void updateScrollBtns();                          // .text:0x29E4
    void initIcons();                                 // .text:0x2AA8
    void calcIcons();                                 // .text:0x2B1C
    void drawIcons();                                 // .text:0x2BBC
    void setLytMode(m2d::SimpleEx_c *lyt, bool main); // .text:0x2C08
    void updateIcons();                               // .text:0x2CE0
    bool canScrollUp();                               // .text:0x2DA4
    bool canScrollDown();                             // .text:0x2DB8
    BOOL requestModel();                              // .text:0x2DDC
    void releaseModel();                              // .text:0x2E60
    void calcModel();                                 // .text:0x2EC0
    void addOrder();                                  // .text:0x2F20
    void showBalloon();                               // .text:0x2F54
    void showTutorial(int cursor);                    // .text:0x308C
    void showTabTutorial(int tab);                    // .text:0x30E0
    void showRowTutorial(int row);                    // .text:0x318C
    void hideTutorial();                              // .text:0x32A0

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x35F4
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x35EC
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x35E8
    virtual void deleteReady() {
    } // .text:0x35E4
    virtual bool createHeap() {
        return true;
    } // .text:0x35DC

    // Implicit destructor: .text:0x3318.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;     // Layout/insectsFish.arc
    /* 0x1A8 */ m2d::ResAccLoader_c mIconRes; // Layout/icon.arc
    /* 0x27C */ m2d::SimpleEx_c mLyt;         // insectsFish_win_00: the window
    /* 0x314 */ m2d::SimpleEx_c
        mSldLyt; // ... again: only the scroll buttons and the slider
    /* 0x3AC */ m2d::Anm_c mKindAnm;    // toFish / toInsects on mLyt
    /* 0x3D0 */ m2d::Anm_c mKindSldAnm; // ... on mSldLyt
    /* 0x3F4 */ m2d::Anm_c mScrollAnm;  // iconR / iconL: the one step scroll
    /* 0x418 */ m2d::AnmEx_c mSldAnm[SCRL_NUM]; // sldCrsr_0x_In / Out / Psh
    /* 0x490 */ m2d::AnmEx_c
        mSldNoPshAnm[SCRL_NUM]; // sldCrsr_0x_NoPsh: the button can't be pushed
    /* 0x508 */ m2d::AnmEx_c mScaleAnm;
    /* 0x544 */ m2d::AnmEx2_c mHoleAnm[ROW_NUM];
    /* 0x698 */ m2d::Anm_c mScrlIconAnm;
    /* 0x6BC */ m2d::AnmEx_c mInfoAnm; // circleIn / circleOut
    /* 0x6F8 */ m2d::Anm_c mItemAnm;   // item_icon_00_item
    /* 0x71C */ m2d::SimpleEx_c mTabLyt;
    /* 0x7B4 */ m2d::AnmEx_c mTabHoverAnm;
    /* 0x7F0 */ m2d::Anm_c mTabOnAnm;
    /* 0x814 */ m2d::Anm_c mTabWinAnm;
    /* 0x838 */ int mTabHover; // the hovered tab, -1 none
    /* 0x83C */ m2d::itemWord_c mNameWord;
    /* 0x888 */ m2d::zBuffClear_c mZClear;
    /* 0x8B0 */ m2d::SimpleEx_c mBlnLyt;
    /* 0x948 */ m2d::AnmEx_c mBlnAnm;
    /* 0x984 */ dMenuItem::Icon_c mIcon[HOLE_NUM];
    /* 0x1C84 */ nw4r::lyt::Pane *mpHole[HOLE_NUM];
    /* 0x1CA4 */ order_c mOrder;
    /* 0x1CB4 */ int mCursor;
    /* 0x1CB8 */ f32 mSliderPos;
    /* 0x1CBC */ f32 mDragOffset;
    /* 0x1CC0 */ int mScroll; // list index of the first row
    /* 0x1CC4 */ int mCount;
    /* 0x1CC8 */ int mScrollDir;
    /* 0x1CCC */ int mSel;      // the selected creature, -1 none
    /* 0x1CD0 */ int mModelSel; // the creature of the requested model, -1 none
    /* 0x1CD4 */ int mSelFish;
    /* 0x1CD8 */ int mSelInsect;
}; // size 0x1CDC

#endif
