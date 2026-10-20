#ifndef GAME_D_MENU_FRIEND03_HPP
#define GAME_D_MENU_FRIEND03_HPP

#include <game/game/d_friend.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>

// The town picker of the Wi-Fi trip (REL d_menu_friend03NP, profile
// MENU_FRIEND03): "Go to which town?". The window of Layout/friend_odekake.arc
// lists the friends that are online with their town open, five at a time,
// scrolled by two buttons or the pad. Each row shows the friend's name, the
// name of his town and four icons that tell how many players are in that town
// (blue) and how many more fit (white); a full town (red) can't be picked. The
// list is refreshed live from the network manager: rows fade out and in when a
// friend comes or goes, and a sound plays when a new one shows up. The confirm
// button hands the friend's slot to the menu manager (setSelSlot). It is opened
// as MENU_FRIEND_TOWN or MENU_FRIEND_TOWN_B; both open the same menu. RTTI
// confirms the global class name dMenuFriend03_c; everything else is inferred.
class dMenuFriend03_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_ROW = 3, // 3..7: the friend rows
        CURSOR_ROW_LAST = CURSOR_ROW + 4,
        CURSOR_SCRL = 8, // 8, 9: previous page, next page
        CURSOR_SCRL_PREV = CURSOR_SCRL,
        CURSOR_SCRL_NEXT = CURSOR_SCRL + 1,
        // The last hit button the layout is asked for (m2d::hitButton).
        CURSOR_HIT_LAST = 10,
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED, // hidden
        BTN_CONFIRM,
    };

    enum {
        ROW_NUM = 5,
        SCRL_NUM = 2,
        MAN_NUM = 4, // icons per row
        // The list scrolls while mScroll < mNum - SCROLL_MARGIN.
        SCROLL_MARGIN = ROW_NUM - 1,
        // Players in a friend's town (mMembers): none (not listed), or full.
        MEMBERS_NONE = 0,
        MEMBERS_FULL = 4,
        MEMBERS_NUM = FRIEND_NUM,
    };

    // Per-row animations (mRowAnm): the file names
    // friend_odekake_win_00_T_msg_xx.
    enum ROW_ANM_e {
        ROW_ANM_WHITE,
        ROW_ANM_GRAY, // the town is full
        ROW_ANM_IN,   // highlighted
        ROW_ANM_OUT,
        ROW_ANM_PUSH,
        ROW_ANM_BEFORE,
        ROW_ANM_NUM,
    };

    // Per-row fade animations (mFadeAnm): friend_odekake_win_00_msgMan_xx.
    enum FADE_ANM_e {
        FADE_ANM_IN,
        FADE_ANM_OUT,
        FADE_ANM_NUM,
    };

    // The player icons (mManAnm): friend_odekake_win_00_man_xx.
    enum MAN_ANM_e {
        MAN_ANM_WHITE, // free place
        MAN_ANM_BLUE,  // a player is there
        MAN_ANM_RED,   // the town is full
        MAN_ANM_NUM,
    };

    // Scroll button animations (mScrlBtnAnm).
    enum SCRL_BTN_ANM_e {
        SCRL_BTN_ANM_IN,
        SCRL_BTN_ANM_OUT,
        SCRL_BTN_ANM_PUSH,
        SCRL_BTN_ANM_CAN_PUSH,
        SCRL_BTN_ANM_NO_PUSH,
        SCRL_BTN_ANM_NUM,
    };

    // The page scroll animations (mScrlAnm).
    enum SCRL_ANM_e {
        SCRL_ANM_DOWN,
        SCRL_ANM_UP,
        SCRL_ANM_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,     // the layout needs a calc
        FLAG_ROW_ANM,  // a row animation is playing
        FLAG_REFRESH,  // rewrite the rows (stateMain)
        FLAG_FADE_ANM, // a fade animation is playing
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,        // waits for the pushed button to finish
        SUB_SCROLL,      // keeps scrolling while the button is held
        SUB_REFRESH_OUT, // the rows fade out, then show the new list
        SUB_REFRESH_IN,  // the rows fade in
        SUB_NUM,
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
        RESULT_CONFIRM = 1, // dMenuMng_c::setSelSlot(the friend's slot)
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_WORD0 = 0x83,            // just word slot 0 (the town name)
        MSG_GO_TO_WHICH_TOWN = 0xF9, // "Go to which town?"
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

    virtual int create();     // .text:0x190
    virtual int doDelete();   // .text:0x2B4
    virtual int stateMain();  // .text:0x308
    virtual int stateAnm();   // .text:0x360
    virtual int stateClose(); // .text:0x3B0
    virtual int execute();    // .text:0x3D4
    virtual int draw();       // .text:0x478

    void subStateMain();                        // .text:0x4C4
    void subStatePush();                        // .text:0x634
    void subStateScroll();                      // .text:0x670
    void subStateRefreshOut();                  // .text:0x754
    void subStateRefreshIn();                   // .text:0x7CC
    void executeSubState();                     // .text:0x81C
    void setSelect(int cursor, BOOL select);    // .text:0x854
    int hitCheck();                             // .text:0x94C
    void push();                                // .text:0xA6C
    BOOL isPushEnd();                           // .text:0xB8C
    void finishPush();                          // .text:0xC50
    void startCloseAnm();                       // .text:0xD90
    void initList();                            // .text:0xDD0
    void startScroll(int cursor);               // .text:0xE28
    void setScrlBtnAnms();                      // .text:0xE8C
    BOOL isSelectable(int cursor);              // .text:0xF28
    BOOL canScrollPrev();                       // .text:0x106C
    BOOL canScrollNext();                       // .text:0x1080
    void selectRow(int cursor);                 // .text:0x10A4
    BOOL isRowEmpty(int cursor);                // .text:0x113C
    void setRowSelect(int cursor, BOOL select); // .text:0x1190
    void resetRowAnms();                        // .text:0x125C
    void updateSelBase();                       // .text:0x12AC
    void refreshAll();                          // .text:0x133C
    void initButtons();                         // .text:0x1380
    void setConfirmDisable(int disable);        // .text:0x13F0
    void initAnms();                            // .text:0x140C
    void calcAnms();                            // .text:0x1570
    void initRowWords();                        // .text:0x1674
    void updateRowWords();                      // .text:0x1724
    const wchar_t *getTownName(int idx);        // .text:0x1884
    const wchar_t *getPlayerName(int idx);      // .text:0x18D0
    void setManAnms(int row);                   // .text:0x191C
    void updateMans();                          // .text:0x1A18
    int getMembers(int idx);                    // .text:0x1AD8
    BOOL updateMembers(BOOL init);              // .text:0x1AEC
    BOOL isFriendValid(int friendIdx);          // .text:0x1BC4
    void buildNewOrder();                       // .text:0x1BCC
    void applyNewOrder();                       // .text:0x1D6C
    BOOL isOrderChanged();                      // .text:0x1E18
    int getRowIdx(int cursor);                  // .text:0x1E5C
    void setFadeAnms(int anm);                  // .text:0x1EB0

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

    // Implicit destructor: .text:0x1F34.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];
    /* 0x394 */ m2d::AnmEx2_c mFadeAnm[ROW_NUM];
    /* 0x4E8 */ m2d::stringWord_c mNameWord[ROW_NUM]; // T_name_xx
    /* 0x9AC */ m2d::stringWord_c mTownWord[ROW_NUM]; // T_msg_xx
    /* 0xE70 */ m2d::AnmEx2_c mManAnm[ROW_NUM * MAN_NUM];
    /* 0x13C0 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM];
    /* 0x1438 */ m2d::AnmEx_c mScrlAnm;
    /* 0x1474 */ m2d::Anm_c mIconAnm;
    /* 0x1498 */ dMenuBtn_c mBtn;
    /* 0x19DC */ int mCursor;
    /* 0x19E0 */ int mScroll; // the friend of the first row (in mOrder)
    /* 0x19E4 */ int mNum;    // the listed friends
    /* 0x19E8 */ int mNumNew; // ... after the next refresh
    /* 0x19EC */ int mSel;    // the picked row's list index, -1 for none
    // Per friend slot: the players in his town (dNetMgr_c::getFriendStatus).
    /* 0x19F0 */ u8 mMembers[MEMBERS_NUM];
    // The friend slots of the list (the friends with a town open) and the
    // list that a refresh will show.
    /* 0x1A10 */ u8 mOrder[MEMBERS_NUM];
    /* 0x1A30 */ u8 mNewOrder[MEMBERS_NUM];
}; // size 0x1A50

#endif
