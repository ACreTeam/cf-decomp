#ifndef GAME_D_MENU_FRIEND02_HPP
#define GAME_D_MENU_FRIEND02_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>

class dMail_c;

// The addressee menu of a letter (REL d_menu_friend02NP, profile
// MENU_FRIEND02): a window (Layout/friend_letter.arc) listing who the letter
// can be sent to, five lines at a time, scrolled by two buttons, with two tabs
// on top: the people of this town (the other players, the villagers who know
// the player and the player themself as "The Future Me") and the friends of
// other towns. Confirm stores the chosen addressee in the letter (the menu
// manager's mail for MENU_ADDRESS, the letter dMenuMng_c::getPickedLetter of
// the player for MENU_ADDRESS_LETTER). RTTI confirms the global class name
// dMenuFriend02_c; everything else is inferred.
class dMenuFriend02_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_ROW = 3, // 3..7: the lines
        CURSOR_ROW_LAST = CURSOR_ROW + 4,
        CURSOR_SCROLL_UP = 8,
        CURSOR_SCROLL_DOWN = 9,
        CURSOR_TAB = 10, // 10, 11: the tabs (MODE_e)
        CURSOR_TAB_LAST = CURSOR_TAB + 1,
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED, // hidden
        BTN_CONFIRM,
    };

    // The two lists (mMode): the tabs.
    enum MODE_e {
        MODE_TOWN,
        MODE_FRIEND,
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,   // waits for the pushed thing to finish
        SUB_SCROLL, // a scroll button is held
        SUB_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,        // the window layout needs a calc
        FLAG_ROW_ANM,     // a line animation is playing
        FLAG_TAB_CALC,    // the tab layout needs a calc
        FLAG_SWITCH,      // the window closes to reopen on the other tab
        FLAG_FORCE_CLOSE, // the menu is closed from outside: no tab menu
                          // follows
    };

    // The animations of a line (mRowAnm).
    enum ROW_ANM_e {
        ROW_ANM_IN,
        ROW_ANM_OUT,
        ROW_ANM_PUSH,
        ROW_ANM_BEFORE,
        ROW_ANM_NUM,
    };

    // The animations of a scroll button (mScrollBtnAnm).
    enum SCROLL_BTN_ANM_e {
        SCROLL_BTN_ANM_IN,
        SCROLL_BTN_ANM_OUT,
        SCROLL_BTN_ANM_PUSH,
        SCROLL_BTN_ANM_CAN_PUSH,
        SCROLL_BTN_ANM_NO_PUSH,
        SCROLL_BTN_ANM_NUM,
    };

    // The scroll animation (mScrollAnm).
    enum SCROLL_ANM_e {
        SCROLL_ANM_DOWN,
        SCROLL_ANM_UP,
    };

    // The tab row's animations (mTabInOutAnm).
    enum TAB_WIN_ANM_e {
        TAB_WIN_ANM_IN,
        TAB_WIN_ANM_OUT,
    };

    // The tab hover animation (mTabHoverAnm).
    enum TAB_HOVER_ANM_e {
        TAB_HOVER_IN,
        TAB_HOVER_OUT,
        TAB_HOVER_PUSH,
    };

    // The addressee codes of the lists (mHomeAddr, mFriendAddr).
    enum ADDR_e {
        ADDR_PLAYER =
            1, // 1..4: the player living in home 0..3 (not the current one)
        ADDR_PLAYER_LAST = 4,
        ADDR_ANIMAL = 5, // 5..14: the villager 0..9
        ADDR_ANIMAL_LAST = 14,
        ADDR_SELF = 15,   // the player themself: "The Future Me"
        ADDR_FRIEND = 16, // 16..47: the friend 0..31
        ADDR_FRIEND_LAST = 47,
    };

    enum {
        ROW_NUM = 5,
        SCROLL_BTN_NUM = 2,
        VISIBLE_ROWS = 4,   // the page a left / right press scrolls
        HOME_ADDR_NUM = 15, // 4 players, 10 villagers, the player themself
        FRIEND_ADDR_NUM = 32,
        TUTORIAL_ADDRESS = 13, // dObjTutorial_c::request
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
        RESULT_CONFIRM = 1,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_FUTURE_ME = 0x79, // "The Future Me"
        MSG_TOWN_NAME = 0x83, // the town name of a line
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isRow(int cursor) {
        return cursor >= CURSOR_ROW && cursor <= CURSOR_ROW_LAST;
    }
    static bool isScrollBtn(int cursor) {
        return cursor >= CURSOR_SCROLL_UP && cursor <= CURSOR_SCROLL_DOWN;
    }
    static bool isTab(int cursor) {
        return cursor >= CURSOR_TAB && cursor <= CURSOR_TAB_LAST;
    }
    static bool isPlayerAddr(u8 addr) {
        return addr >= ADDR_PLAYER && addr <= ADDR_PLAYER_LAST;
    }
    static bool isAnimalAddr(u8 addr) {
        return addr >= ADDR_ANIMAL && addr <= ADDR_ANIMAL_LAST;
    }
    static bool isFriendAddr(u8 addr) {
        return addr >= ADDR_FRIEND && addr <= ADDR_FRIEND_LAST;
    }

    typedef BOOL (dMenuFriend02_c::*CloseProc)();
    typedef void (dMenuFriend02_c::*SubStateProc)();

    virtual int create();     // .text:0x198
    virtual int doDelete();   // .text:0x30C
    virtual int stateMain();  // .text:0x360
    virtual int stateAnm();   // .text:0x3A8
    virtual int stateClose(); // .text:0x40C
    virtual int execute();    // .text:0x514
    virtual int draw();       // .text:0x584

    void subStateMain();                                // .text:0x648
    void subStatePush();                                // .text:0x720
    void subStateScroll();                              // .text:0x75C
    BOOL closeMenu();                                   // .text:0x86C
    BOOL checkCloseRequest();                           // .text:0x8B0
    void executeSubState();                             // .text:0x99C
    void setCursor(int cursor, BOOL select);            // .text:0x9D4
    int hitCheck();                                     // .text:0xB20
    void padCheck();                                    // .text:0xC7C
    void push();                                        // .text:0xDDC
    BOOL isPushEnd();                                   // .text:0xF48
    void finishPush();                                  // .text:0x1038
    void startClose();                                  // .text:0x11C0
    void startSwitchOut();                              // .text:0x1224
    void initList();                                    // .text:0x1274
    void startScroll(int cursor);                       // .text:0x1290
    void updateScrollBtns();                            // .text:0x12F0
    bool canScroll(int cursor);                         // .text:0x1370
    bool canScrollUp();                                 // .text:0x1380
    bool canScrollDown();                               // .text:0x1394
    void chooseRow(int cursor);                         // .text:0x13B8
    void setRowSelect(int cursor, BOOL select);         // .text:0x1468
    void updateRowSelect();                             // .text:0x14EC
    void resetRowAnms();                                // .text:0x157C
    void refreshList();                                 // .text:0x15CC
    void initButtons();                                 // .text:0x1608
    void initAnms();                                    // .text:0x1678
    void calcAnms();                                    // .text:0x1764
    void initRowWords();                                // .text:0x1830
    void updateRows();                                  // .text:0x192C
    void getTownName(dScript::Word_c *word, int idx);   // .text:0x1A68
    void getPlayerName(dScript::Word_c *word, int idx); // .text:0x1B90
    void initTabAnm();                                  // .text:0x1D50
    void setTabAnm(int anm);                            // .text:0x1D6C
    void setTabHover(int tab, int anm);                 // .text:0x1D8C
    void calcTabAnms();                                 // .text:0x1E88
    void setMode(int mode);                             // .text:0x1F60
    void buildList();                                   // .text:0x2010
    u8 getAddr(int idx);                                // .text:0x22D4
    void applySelection();                              // .text:0x22F8
    void setMailTo(dMail_c *mail);                      // .text:0x23C8
    void showTabTutorial();                             // .text:0x24DC

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

    // Implicit destructor: .text:0x25AC.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt; // the list window
    /* 0x240 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];
    /* 0x394 */ m2d::Anm_c mScrollIconAnm;
    /* 0x3B8 */ m2d::stringWord_c mNameWord[ROW_NUM];     // T_name_xx
    /* 0x87C */ m2d::stringWord_c mTownWord[ROW_NUM];     // T_msg_xx
    /* 0xD40 */ nw4r::lyt::TextBox *mpFutureBox[ROW_NUM]; // T_future_xx
    /* 0xD54 */ m2d::AnmEx_c mScrollBtnAnm[SCROLL_BTN_NUM];
    /* 0xDCC */ m2d::AnmEx_c mScrollAnm;
    /* 0xE08 */ m2d::SimpleEx_c mTabLyt;
    /* 0xEA0 */ m2d::Anm_c mTabInOutAnm;
    /* 0xEC4 */ m2d::Anm_c mTabHoverAnm;
    /* 0xEE8 */ m2d::AnmEx2_c mTabAnm; // the tab that is on
    /* 0xF2C */ int mTabHover;         // the hovered tab, -1 none
    /* 0xF30 */ dMenuBtn_c mBtn;
    /* 0x1474 */ int mCursor;
    /* 0x1478 */ int mScroll;   // the entry shown on the first line
    /* 0x147C */ int mNum;      // the entries of the current list
    /* 0x1480 */ int mSelected; // the chosen entry, -1 none
    /* 0x1484 */ int mSelAddr;  // its addressee code
    /* 0x1488 */ int mMode;     // MODE_e
    /* 0x148C */ u8 mHomeAddr[HOME_ADDR_NUM];
    /* 0x149B */ u8 mFriendAddr[FRIEND_ADDR_NUM];
    /* 0x14BC */ int mHomeNum;
    /* 0x14C0 */ int mFriendNum;
}; // size 0x14C4

#endif
