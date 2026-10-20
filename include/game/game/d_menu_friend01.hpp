#ifndef GAME_D_MENU_FRIEND01_HPP
#define GAME_D_MENU_FRIEND01_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>

// The Wii friend picker (REL d_menu_friend01NP, profile MENU_FRIEND01): the
// window of Layout/friend_dengon.arc lists the registered Wii friends (the
// WiiConnect24 roster, dWifi) by name, five at a time, scrolled by two buttons,
// the pad or a slider. Each row has a town button that opens a balloon
// (Layout/friend_WiFi_balloon.arc) with the name of the friend's town. The
// confirm button hands the picked friend to the menu manager. It is opened as
// MENU_FRIEND_SELECT (which first updates the roster when it isn't known yet)
// or MENU_FRIEND_SELECT_KNOWN. RTTI confirms the global class name
// dMenuFriend01_c; everything else is inferred.
class dMenuFriend01_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_ROW = 3, // 3..8: the friend rows
        CURSOR_ROW_LAST = CURSOR_ROW + 5,
        CURSOR_SCRL = 9, // 9, 10: previous page, next page
        CURSOR_SCRL_PREV = CURSOR_SCRL,
        CURSOR_SCRL_NEXT = CURSOR_SCRL + 1,
        CURSOR_SLIDER = 11,
        CURSOR_TOWN = 12, // 12..17: the town buttons, one per row
        CURSOR_TOWN_LAST = CURSOR_TOWN + 5,
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED, // hidden
        BTN_CONFIRM,
    };

    enum {
        ROW_NUM = 6, // the sixth row is only there while scrolling
        VISIBLE_ROWS = 5,
        SCRL_NUM = 2,
        SCROLL_PAGE = 5,         // the friends a left / right press scrolls
        BALLOON_PRIORITY = 0x84, // m2d::Base_c::mPriority of the balloon layout
    };

    // Per-row animations (mRowAnm): the file names
    // friend_dengon_win_00_T_msg_xx.
    enum ROW_ANM_e {
        ROW_ANM_IN,
        ROW_ANM_OUT,
        ROW_ANM_PUSH,
        ROW_ANM_BEFORE,
        ROW_ANM_NUM,
    };

    // Town button animations (mTownAnm): friend_dengon_win_00_mura_xx.
    enum TOWN_ANM_e {
        TOWN_ANM_IN,
        TOWN_ANM_OUT,
        TOWN_ANM_NUM,
    };

    // Scroll button animations (mScrlBtnAnm).
    enum SCRL_ANM_e {
        SCRL_ANM_IN,
        SCRL_ANM_OUT,
        SCRL_ANM_PUSH,
        SCRL_ANM_CAN_PUSH,
        SCRL_ANM_NO_PUSH,
        SCRL_ANM_NUM,
    };

    // The page scroll animations (mScrlAnm).
    enum SCRL_PAGE_ANM_e {
        SCRL_PAGE_ANM_DOWN,
        SCRL_PAGE_ANM_UP,
        SCRL_PAGE_ANM_NUM,
    };

    // The slider animations (mSldrAnm).
    enum SLDR_ANM_e {
        SLDR_ANM_IN,
        SLDR_ANM_OUT,
        SLDR_ANM_NUM,
    };

    // The balloon animations (mBalloonAnm).
    enum BALLOON_ANM_e {
        BALLOON_ANM_IN,
        BALLOON_ANM_OUT,
        BALLOON_ANM_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,         // the window layout needs a calc
        FLAG_ROW_ANM,      // a row or town animation is playing
        FLAG_SEND_TO,      // opened as MENU_FRIEND_SELECT_KNOWN
        FLAG_BALLOON_CALC, // the balloon layout needs a calc
        FLAG_BALLOON_DRAW, // draw the balloon
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_START_ROSTER, // asks the roster update to start
        SUB_WAIT_ROSTER,  // waits for the roster update
        SUB_MAIN,
        SUB_PUSH,   // waits for the pushed button to finish
        SUB_SCROLL, // keeps scrolling while the button is held
        SUB_SLIDER, // the slider is dragged
        SUB_NUM,
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
        RESULT_CONFIRM =
            1, // the picked friend is dMenuMng_c::setFriendIdx(mSel)
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_WORD0 = 0x83,   // just word slot 0 (the town name)
        MSG_SEND_TO = 0xFA, // "Send to which friend?"
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isRow(int cursor) {
        return cursor >= CURSOR_ROW && cursor <= CURSOR_ROW_LAST;
    }
    static bool isScrl(int cursor) {
        return cursor >= CURSOR_SCRL_PREV && cursor <= CURSOR_SCRL_NEXT;
    }
    static bool isTown(int cursor) {
        return cursor >= CURSOR_TOWN && cursor <= CURSOR_TOWN_LAST;
    }

    virtual int create();     // .text:0x1A0
    virtual int doDelete();   // .text:0x31C
    virtual int stateMain();  // .text:0x388
    virtual int stateAnm();   // .text:0x3AC
    virtual int stateClose(); // .text:0x3FC
    virtual int execute();    // .text:0x458
    virtual int draw();       // .text:0x504

    void subStateStartRoster();              // .text:0x558
    void subStateWaitRoster();               // .text:0x5A4
    void subStateMain();                     // .text:0x61C
    void subStatePush();                     // .text:0x710
    void subStateScroll();                   // .text:0x74C
    void subStateSlider();                   // .text:0x864
    void executeSubState();                  // .text:0x9D0
    void setSelect(int cursor, BOOL select); // .text:0xA08
    int hitCheck();                          // .text:0xB5C
    void checkPadMove();                     // .text:0xCE4
    void push();                             // .text:0xE74
    BOOL isPushEnd();                        // .text:0x1080
    void finishPush();                       // .text:0x11AC
    void showList();                         // .text:0x1320
    void startCloseAnm();                    // .text:0x1398
    void initList();                         // .text:0x13EC
    void startScroll(int cursor);            // .text:0x143C
    void setScrlBtnAnms();                   // .text:0x14A4
    BOOL canScroll(int cursor);              // .text:0x1524
    BOOL canScrollPrev();                    // .text:0x1534
    BOOL canScrollNext();                    // .text:0x1548
    void selectRow(int cursor);              // .text:0x156C
    void setRowAnm(int cursor, BOOL select); // .text:0x1604
    void resetRowAnms();                     // .text:0x1688
    void updateSelBase();                    // .text:0x16D8
    void updateTownBtns();                   // .text:0x1768
    void refreshList();                      // .text:0x17EC
    void initButtons();                      // .text:0x1830
    void setConfirmDisable(int disable);     // .text:0x1890
    void initAnms();                         // .text:0x18AC
    void calcAnms();                         // .text:0x1A00
    void initRowWords();                     // .text:0x1B0C
    void updateRowWords();                   // .text:0x1B94
    const wchar_t *getFriendName(int idx);   // .text:0x1C68
    const wchar_t *getTownName(int cursor);  // .text:0x1CCC
    void initSlider();                       // .text:0x1D94
    void calcSldrAnm();                      // .text:0x1E28
    void updateSlider();                     // .text:0x1E6C
    void initBalloon();                      // .text:0x1F18
    void calcBalloonAnm();                   // .text:0x1F94
    void drawBalloon();                      // .text:0x1FF4
    void showBalloon(int cursor);            // .text:0x2068
    BOOL isBalloonHidden();                  // .text:0x2178
    void closeBalloon();                     // .text:0x21C8
    void fitBalloon();                       // .text:0x21E4

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

    // Implicit destructor: .text:0x2338.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::ResAccLoader_c mUnusedRes;
    /* 0x27C */ m2d::SimpleEx_c mLyt;
    /* 0x314 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];
    /* 0x4AC */ m2d::AnmEx2_c mTownAnm[ROW_NUM];
    /* 0x644 */ m2d::Anm_c mTownPushAnm; // played on the pane "mura_Psh"
    /* 0x668 */ m2d::stringWord_c mRowWord[ROW_NUM];
    /* 0xC20 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM];
    /* 0xC98 */ m2d::AnmEx_c mScrlAnm;
    /* 0xCD4 */ m2d::Anm_c mIconAnm;
    /* 0xCF8 */ dMenuBtn_c mBtn;
    /* 0x123C */ m2d::SimpleEx_c mBalloonLyt;
    /* 0x12D4 */ m2d::ResAccLoader_c mBalloonRes;
    /* 0x13A8 */ m2d::AnmEx_c mBalloonAnm;
    /* 0x13E4 */ m2d::stringWord_c mBalloonWord;
    /* 0x14D8 */ f32 mBalloonX;
    /* 0x14DC */ int mCursor;
    /* 0x14E0 */ int mScroll; // the friend of the first row
    /* 0x14E4 */ int mNum;    // the roster's friends
    /* 0x14E8 */ int mSel;    // the picked friend, -1 for none
    /* 0x14EC */ m2d::AnmEx_c mSldrAnm;
    /* 0x1528 */ f32 mSldrTop;
    /* 0x152C */ f32 mSldrBottom;
    /* 0x1530 */ nw4r::lyt::Pane *mpSldr;
    /* 0x1534 */ f32 mSldrDragOfs;
}; // size 0x1538

#endif
