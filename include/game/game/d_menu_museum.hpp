#ifndef GAME_D_MENU_MUSEUM_HPP
#define GAME_D_MENU_MUSEUM_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>

// The museum donation list (REL d_menu_museumNP, profile MENU_MUSEUM): the
// donate window (Layout/donate.arc) with a row of four category tabs on top
// (Fossils, Bugs, Fish, Paintings), a list of the donated exhibits of the tab
// with the donor's name next to each (seven lines, scrolled by two buttons or a
// slider) and a back button. The lines can't be selected. RTTI confirms the
// class name dMenuMuseum_c; everything else is inferred.
class dMenuMuseum_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_TAB = 3, // 3..6: the category tabs
        CURSOR_TAB_LAST = CURSOR_TAB + 3,
        CURSOR_ROW = 7,   // 7..12: the list lines (hit, but they do nothing)
        CURSOR_SCRL = 13, // 13, 14: scroll up, scroll down
        CURSOR_SCRL_UP = CURSOR_SCRL,
        CURSOR_SCRL_DOWN = CURSOR_SCRL + 1,
        CURSOR_SLIDER = 15, // the slider
    };

    // The dMenuBtn_c buttons (the other two are hidden).
    enum BTN_e {
        BTN_BACK,
        BTN_UNUSED_1,
        BTN_UNUSED_2,
    };

    enum {
        TAB_NUM = 4,
        ROW_NUM = 7,
        SCRL_NUM = 2,
        ITEM_NUM = 64, // the most donated exhibits of a tab (bugs, fish)
        SCROLL_MIN = -1,
        SCROLL_PAGE = 6, // the lines a left / right press scrolls
        SLIDER_OFFSET =
            6, // the slider spans mCount - SLIDER_OFFSET steps (none if fewer)
    };

    // The tabs.
    enum TAB_e {
        TAB_FOSSIL,
        TAB_INSECT,
        TAB_FISH,
        TAB_PICTURE,
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

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,         // the window layout needs a calc
        FLAG_CALC_TAB,     // the tab layout needs a calc
        FLAG_SCRL_BTN_ANM, // a scroll button animation is playing
        FLAG_SCROLL,       // the list is scrolling
        FLAG_QUIT, // the close animation quits the menu (else it changes the
                   // tab)
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,   // waits for the pushed button to finish
        SUB_SCROLL, // keeps scrolling while the button is held
        SUB_SLIDER, // the slider is dragged
        SUB_NUM,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_BACK = 0x20,
        MSG_SOMEONE = 0x39, // the donor of an exhibit whose player has left
    };

    // The tab tutorial balloons (SYS_2D_UIballoon.bmg): one per tab, in order
    // (Fossils, Bugs, Fish, Paintings).
    enum {
        BALLOON_MSG_FOSSIL = 0x26,
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isTab(int cursor) {
        return cursor >= CURSOR_TAB && cursor <= CURSOR_TAB_LAST;
    }
    static bool isScrl(int cursor) {
        return cursor >= CURSOR_SCRL && cursor <= CURSOR_SCRL_DOWN;
    }

    virtual int create();                    // .text:0x1A0
    virtual int doDelete();                  // .text:0x320
    virtual int stateMain();                 // .text:0x374
    virtual int stateAnm();                  // .text:0x398
    virtual int stateClose();                // .text:0x3FC
    virtual int execute();                   // .text:0x470
    virtual int draw();                      // .text:0x588
    void subStateMain();                     // .text:0x5DC
    void subStatePush();                     // .text:0x6B8
    void subStateScroll();                   // .text:0x6F4
    void subStateSlider();                   // .text:0x7B8
    void executeSubState();                  // .text:0x928
    void setSelect(int cursor, BOOL select); // .text:0x960
    int hitCheck();                          // .text:0xA90
    void checkPadMove();                     // .text:0xBCC
    void pushCursor();                       // .text:0xD40
    BOOL isPushEnd();                        // .text:0xE9C
    void finishPush();                       // .text:0xF54
    void startCloseAnm();                    // .text:0x1080
    void startTabAnm();                      // .text:0x10DC
    void setTabAnm(int anm);                 // .text:0x112C
    void initTabAnms();                      // .text:0x114C
    void calcTabAnm();                       // .text:0x11CC
    void setTabHover(int tab, int anm);      // .text:0x12C8
    void selectTab(int tab);                 // .text:0x13A4
    void setKind();                          // .text:0x1410
    void initButtons();                      // .text:0x14B8
    void initAnms();                         // .text:0x1514
    void calcScrlBtnAnm();                   // .text:0x1574
    void setScrlAnm(int dir);                // .text:0x1664
    BOOL startScroll(int dir);               // .text:0x1684
    void setScrlBtnAnms();                   // .text:0x1728
    bool canScroll(int dir);                 // .text:0x17A8
    bool canScrollUp();                      // .text:0x17B8
    bool canScrollDown();                    // .text:0x17D8
    void initSlider();                       // .text:0x17FC
    void calcSliderAnm();                    // .text:0x1890
    void updateSlider();                     // .text:0x18D4
    void initRows();                         // .text:0x1984
    void refreshList();                      // .text:0x1A5C
    void updateRowWords();                   // .text:0x1A60
    u16 getNthItem(int n);                   // .text:0x1B88
    void updateItems();                      // .text:0x1BB8
    void showTutorial(int cursor);           // .text:0x1C2C
    void hideTutorial();                     // .text:0x1D04

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x1F28
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x1F20
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x1F1C
    virtual void deleteReady() {
    } // .text:0x1F18
    virtual bool createHeap() {
        return true;
    } // .text:0x1F10

    // Implicit destructor: .text:0x1D08.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM];
    /* 0x2B8 */ m2d::Anm_c mScrlAnm;
    /* 0x2DC */ m2d::Anm_c mSliderPtrnAnm;
    /* 0x300 */ m2d::Anm_c mSliderIconAnm;
    /* 0x324 */ m2d::SimpleEx_c mTabLyt;
    /* 0x3BC */ m2d::Anm_c mTabAnmInOut;
    /* 0x3E0 */ m2d::Anm_c mTabHoverAnm;
    /* 0x404 */ m2d::AnmEx2_c mTabAnm[TAB_NUM];
    /* 0x514 */ m2d::itemWord_c mRowWord[ROW_NUM];     // exhibit names
    /* 0x728 */ m2d::stringWord_c mDonorWord[ROW_NUM]; // donor names
    /* 0xDD4 */ dMenuBtn_c mBtn;
    /* 0x1318 */ int mCursor;
    /* 0x131C */ int mTabHover; // the tab with the hover animation, -1 none
    /* 0x1320 */ int mTab;      // TAB_e
    /* 0x1324 */ int mCount;    // donated exhibits of the tab
    /* 0x1328 */ int mScroll;   // list index of the second line
    /* 0x132C */ int mKind;     // dItem kind of the tab
    /* 0x1330 */ m2d::AnmEx_c mSliderAnm;
    /* 0x136C */ f32 mSliderTop;
    /* 0x1370 */ f32 mSliderBottom;
    /* 0x1374 */ nw4r::lyt::Pane *mpSlider;
    /* 0x1378 */ f32 mDragOffset;
    /* 0x137C */ u16 mItems[ITEM_NUM]; // the donated exhibits of the tab
}; // size 0x13FC

#endif
