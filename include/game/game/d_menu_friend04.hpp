#ifndef GAME_D_MENU_FRIEND04_HPP
#define GAME_D_MENU_FRIEND04_HPP

#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_select.hpp>

// The Friend Roster tab of the friend menu (REL d_menu_friend04NP, profile
// MENU_FRIEND04): the window of Layout/friend_WiFi.arc lists the player's
// registered Wi-Fi friends (the dFriendList_c of the current player), five at a
// time, scrolled by two buttons, the pad or a slider. It is the child menu of
// dMenuFriend_c's first tab (dMenuFriend_c::TAB_ROSTER). Each row shows the
// friend's town (T_msg_xx) and name (T_name_xx), a status icon
// (situation_xx) and a comment icon (N_cmnt_xx) that opens a balloon
// (Layout/friend_WiFi_balloon.arc) with the friend's message. Picking a row
// opens a small menu (dSelect::MComp_c): rewrite the name, rewrite the comment,
// reenter the code, defriend (after a Yes / No dialog) or quit. The button at
// the bottom registers a new friend (it fails with a dialog when the roster is
// full). The edited friend is handed to dMenuFriend_c (loadFriend) and the
// matching input menu is opened through dMenuFriend_c::requestOpenMenu.
// The cursor (mCursor) also points at the tabs of the bottom panel (dAcUitag)
// and at the four tabs of dMenuFriend_c. RTTI confirms the global class name
// dMenuFriend04_c; everything else is inferred.
class dMenuFriend04_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_UITAG = 0, // 0..7: the tabs of the bottom panel (dAcUitag)
        CURSOR_UITAG_LAST = CURSOR_UITAG + dAcUitag_c::TAB_NUM - 1,
        CURSOR_TAB = 8, // 8..11: the tabs of dMenuFriend_c (TAB_e)
        CURSOR_TAB_LAST = CURSOR_TAB + 3,
        CURSOR_ROW = 12, // 12..16: the friend rows
        CURSOR_ROW_LAST = CURSOR_ROW + 4,
        CURSOR_CMNT = 17, // 17..21: the comment icons, one per row
        CURSOR_CMNT_LAST = CURSOR_CMNT + 4,
        CURSOR_SCRL = 22, // 22, 23: previous page, next page
        CURSOR_SCRL_PREV = CURSOR_SCRL,
        CURSOR_SCRL_NEXT = CURSOR_SCRL + 1,
        CURSOR_REGISTER = 24, // "Register a Friend"
        CURSOR_SLIDER = 25,
    };

    enum {
        ROW_NUM = 5,
        SCRL_NUM = 2,
        // The list scrolls while mScroll < mNum - SCROLL_MARGIN; a left / right
        // press scrolls SCROLL_MARGIN friends.
        SCROLL_MARGIN = ROW_NUM - 1,
        BALLOON_PRIORITY = 0x84, // m2d::Base_c::mPriority of the balloon layout
    };

    // The state of a friend slot (getRowState).
    enum ROW_STATE_e {
        ROW_STATE_KNOWN,    // the friend's player and town are known
        ROW_STATE_NO_KEY,   // registered without a friend key
        ROW_STATE_KEY_ONLY, // only the friend key is known
        ROW_STATE_EMPTY,    // not a friend
    };

    // Per-row animations (mRowAnm): friend_WiFi_win_00_T_msg_xx.
    enum ROW_ANM_e {
        ROW_ANM_IN,
        ROW_ANM_OUT,
        ROW_ANM_PUSH,
        ROW_ANM_NUM,
    };

    // Comment icon animations (mCmntAnm): friend_WiFi_win_00_cmnt_xx.
    enum CMNT_ANM_e {
        CMNT_ANM_IN,
        CMNT_ANM_OUT,  // the friend has a message to send
        CMNT_ANM_GRAY, // no message
        CMNT_ANM_PINK, // a message that is not marked
        CMNT_ANM_NUM,
    };

    // Register button animations (mRegisterAnm): friend_WiFi_win_00_cmnBtnA_xx.
    enum REGISTER_ANM_e {
        REGISTER_ANM_IN,
        REGISTER_ANM_OUT,
        REGISTER_ANM_PUSH,
        REGISTER_ANM_NUM,
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
        FLAG_CALC,         // the window layout needs a calc
        FLAG_DRAW,         // the window layout is drawn
        FLAG_ROW_ANM,      // a row animation is playing
        FLAG_DIALOG,       // draw the dialog
        FLAG_BALLOON_CALC, // the balloon layout needs a calc
        FLAG_BALLOON_DRAW, // draw the balloon
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,         // waits for the pushed button to finish
        SUB_SCROLL,       // keeps scrolling while the button is held
        SUB_MENU,         // the friend's menu (mSelect) is open
        SUB_DIALOG,       // a dialog opens, waits for the input
        SUB_DIALOG_PUSH,  // waits for the pushed dialog button
        SUB_DIALOG_CLOSE, // the dialog closes, then acts on the answer
        SUB_DELETE,       // defriend
        SUB_SLIDER,       // the slider is dragged
        SUB_NUM,
    };

    // The answers of the friend's menu (dSelect::BaseComp_c::mCode).
    enum ANSWER_e {
        ANSWER_NAME,
        ANSWER_COMMENT,
        ANSWER_CODE,
        ANSWER_DEFRIEND,
        ANSWER_QUIT,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x2,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_WORD0 = 0x83,    // just word slot 0 (the town name)
        MSG_REGISTER = 0x92, // "Register a Friend"
        MSG_REWRITE_NAME = 0x95,
        MSG_REWRITE_COMMENT = 0x96,
        MSG_REENTER_CODE = 0x97,
        MSG_DEFRIEND = 0x98,
        MSG_ROSTER_FULL = 0x9A,   // "You can't register a new friend if your
                                  // friend roster is full!"
        MSG_DELETE_FRIEND = 0xA4, // "Delete <player> from <town> off of your
                                  // friend roster?"
        MSG_OK = 0xEC,
    };

    static bool isUitagTab(int cursor) {
        return cursor >= CURSOR_UITAG && cursor <= CURSOR_UITAG_LAST;
    }
    static bool isTab(int cursor) {
        return cursor >= CURSOR_TAB && cursor <= CURSOR_TAB_LAST;
    }
    static bool isRow(int cursor) {
        return cursor >= CURSOR_ROW && cursor <= CURSOR_ROW_LAST;
    }
    static bool isCmnt(int cursor) {
        return cursor >= CURSOR_CMNT && cursor <= CURSOR_CMNT_LAST;
    }
    static bool isScrl(int cursor) {
        return cursor >= CURSOR_SCRL_PREV && cursor <= CURSOR_SCRL_NEXT;
    }

    typedef BOOL (dMenuFriend04_c::*CloseProc)();
    typedef void (dMenuFriend04_c::*SubStateProc)();

    virtual int create();     // .text:0x200
    virtual int doDelete();   // .text:0x35C
    virtual int stateMain();  // .text:0x3E0
    virtual int stateAnm();   // .text:0x428
    virtual int execute();    // .text:0x42C
    virtual int stateClose(); // .text:0x4F4
    virtual int draw();       // .text:0x518

    void subStateMain();                         // .text:0x5BC
    void subStatePush();                         // .text:0x71C
    void subStateScroll();                       // .text:0x758
    void subStateMenu();                         // .text:0x870
    void subStateDialog();                       // .text:0x8C4
    void subStateDialogPush();                   // .text:0x924
    void subStateDialogClose();                  // .text:0x988
    void subStateDelete();                       // .text:0xA18
    void subStateSlider();                       // .text:0xABC
    BOOL subStateCloseRequest();                 // .text:0xC28
    BOOL subStateCloseRequestMenu();             // .text:0xC60
    BOOL subStateCloseRequestDialog();           // .text:0xCB0
    BOOL executeCloseRequest();                  // .text:0xD0C
    void executeSubState();                      // .text:0xE10
    int hitUitagTab();
    int hitUitagTab(const mVec2_c *pos);         // .text:0xE48
    BOOL selectUitagTab(int tab, BOOL delay);    // .text:0xEAC
    void startCloseAnm();                        // .text:0xEFC
    void startReopenAnm();                       // .text:0xF38
    void startOpenMenuAnm(int idx);              // .text:0xF78
    int hitCheck();                              // .text:0xFC4
    void setSelect(int cursor, BOOL select);     // .text:0x10C4
    void checkPadMove();                         // .text:0x1298
    void push();                                 // .text:0x1430
    BOOL isPushEnd();                            // .text:0x169C
    void finishPush();                           // .text:0x17D4
    void setTexts();                             // .text:0x1938
    void initList();                             // .text:0x1950
    void initAnms();                             // .text:0x199C
    void calcAnms();                             // .text:0x1B40
    void setRowAnm(int cursor, BOOL select);     // .text:0x1C50
    void refreshList();                          // .text:0x1CD4
    void initRowWords();                         // .text:0x1D08
    void updateRowWords();                       // .text:0x1DB8
    void updateRowIcons();                       // .text:0x1F4C
    const wchar_t *getTownName(int idx);         // .text:0x20D8
    const wchar_t *getPlayerName(int idx);       // .text:0x2110
    const wchar_t *getComment(int idx);          // .text:0x2148
    const wchar_t *getCursorComment(int cursor); // .text:0x2180
    int getCmntAnm(int cursor);                  // .text:0x21B8
    int getRowState(int idx);                    // .text:0x2218
    int getCursorRowState(int cursor);           // .text:0x22A8
    int findFreeFriend();                        // .text:0x22E0
    int getFriendIdx(int cursor);                // .text:0x22E4
    void startScroll(int cursor);                // .text:0x234C
    void setScrlBtnAnms();                       // .text:0x23B4
    BOOL canScroll(int cursor);                  // .text:0x2434
    BOOL canScrollPrev();                        // .text:0x2444
    BOOL canScrollNext();                        // .text:0x2458
    void initSlider();                           // .text:0x247C
    void calcSldrAnm();                          // .text:0x2510
    void updateSlider();                         // .text:0x2554
    void openMenu(int cursor);                   // .text:0x2600
    void setMenuItems(int cursor);               // .text:0x27FC
    void doAnswer();                             // .text:0x28EC
    void openFullDialog(u16 msg);                // .text:0x29B8
    void openDeleteDialog();                     // .text:0x2A70
    void initBalloon();                          // .text:0x2BA4
    void calcBalloonAnm();                       // .text:0x2C20
    void drawBalloon();                          // .text:0x2C80
    void showBalloon(int cursor);                // .text:0x2CF4
    BOOL isBalloonHidden();                      // .text:0x2DCC
    void closeBalloon();                         // .text:0x2E1C
    void fitBalloon();                           // .text:0x2E38

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

    // Implicit destructor: .text:0x2F8C.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM];
    /* 0x2B8 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];
    /* 0x40C */ m2d::AnmEx2_c mSituationAnm[ROW_NUM];
    /* 0x560 */ m2d::AnmEx2_c mCmntAnm[ROW_NUM];
    /* 0x6B4 */ m2d::Anm_c mCmntPushAnm; // played on the pane "cmnt_Psh"
    /* 0x6D8 */ m2d::AnmEx_c mRegisterAnm;
    /* 0x714 */ m2d::AnmEx_c mScrlAnm;
    /* 0x750 */ m2d::Anm_c mIconAnm;
    /* 0x774 */ m2d::stringWord_c mNameWord[ROW_NUM];
    /* 0xC38 */ m2d::stringWord_c mTownWord[ROW_NUM];
    /* 0x10FC */ dMenuSlctB_c mDialog;
    /* 0x17EC */ dSelect::MComp_c mSelect;
    /* 0x2198 */ m2d::SimpleEx_c mBalloonLyt;
    /* 0x2230 */ m2d::ResAccLoader_c mBalloonRes;
    /* 0x2304 */ m2d::AnmEx_c mBalloonAnm;
    /* 0x2340 */ m2d::stringWord_c mBalloonWord;
    /* 0x2434 */ f32 mBalloonX;
    /* 0x2438 */ int mPicked; // the cursor of the row the menu was opened on
    /* 0x243C */ int mAnswer;
    /* 0x2440 */ int mCursor;
    /* 0x2444 */ int mScroll; // the friend of the first row
    /* 0x2448 */ int mNum;    // the registered friends
    /* 0x244C */ m2d::AnmEx_c mSldrAnm;
    /* 0x2488 */ f32 mSldrTop;
    /* 0x248C */ f32 mSldrBottom;
    /* 0x2490 */ nw4r::lyt::Pane *mpSldr;
    /* 0x2494 */ f32 mSldrDragOfs;
}; // size 0x2498

#endif
