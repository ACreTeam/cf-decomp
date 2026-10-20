#pragma once

// The friend menu's tab bar (REL d_menu_friendNP, profile MENU_FRIEND, RTTI
// "dMenuFriend_c"). It is the base of the friend sub menus: the layout of the
// four tabs on the top of the screen (Friend Roster, Friend Code, Visiting
// Friends, Mic Chat; Layout/friend_tab.arc) and the frame with the player's
// friend name (Layout/friend_frame.arc). Each tab opens one of the sub menus
// (REL d_menu_friend04..07) as its child menu; the sub menus (04..08) call the
// static functions below to move the tabs, to show the frame and to edit the
// friend entry that is being registered. Names other than the class name are
// inferred.

#include <types.h>
#include <game/game/d_friend.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/dwc/dwc.h>

class dMenuFriend_c : public dMenuBase_c {
  public:
    // The tabs (mTab), in the order of the layout's N_topTb_NN panes.
    enum TAB_e {
        TAB_ROSTER, // Friend Roster (REL d_menu_friend04)
        TAB_CODE,   // Friend Code (REL d_menu_friend06)
        TAB_VISIT,  // Visiting Friends (REL d_menu_friend05)
        TAB_MIC,    // Mic Chat (REL d_menu_friend07)
        TAB_NUM,
    };

    // The sub states (setSubState); the tab layout reacts to the child menu
    // through them.
    enum SUB_STATE_e {
        SUB_IDLE,        // the child menu runs
        SUB_OPEN_TAB,    // open the menu of the current tab
        SUB_WAIT_REOPEN, // wait for the child to close, then SUB_OPEN_TAB
        SUB_WAIT_CLOSE,  // wait for the child to close, then close this menu
        SUB_OPEN_MENU,   // open the menu mNextMenu selects
        SUB_WAIT_MENU,   // wait for the child to close, hide the tabs, then
                         // SUB_OPEN_MENU
        SUB_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_TAB_CALC,       // the tab layout needs a calc
        FLAG_TAB,            // the tab layout is shown
        FLAG_FRAME_CALC,     // the frame layout needs a calc
        FLAG_FRAME,          // the frame layout is shown
        FLAG_FRAME_ANM,      // mFrameMoveAnm is playing
        FLAG_FRAME_ANM_END,  // remove mFrameMoveAnm when it stops (the text box
                             // went down)
        FLAG_FRAME_ANM_DONE, // mFrameMoveAnm was removed (consumed by
                             // isFrameAnmDone)
        FLAG_SAVE,           // the friend list is saved when the menu closes
    };

    // The animations of the tab layout (setTabAnm): the tabs slide in or out.
    enum TAB_ANM_e {
        TAB_ANM_IN,
        TAB_ANM_OUT,
    };

    // The animations of a tab (mTabBtnAnm): selected, not selected.
    enum TAB_BTN_ANM_e {
        TAB_BTN_ANM_ON,
        TAB_BTN_ANM_OFF,
    };

    // The animations of the frame's text box (mFrameAnm).
    enum FRAME_ANM_e {
        FRAME_ANM_IN,
        FRAME_ANM_OUT,
    };

    enum {
        LYT_PRIORITY = 0x82, // both layouts (m2d::Base_c::mPriority)
        NO_FOCUS = -1,
        // Number of menus requestOpenMenu selects from (l_openMenu).
        OPEN_MENU_NUM = 9,
        // l_openMenu indexes of the text entry menus (dMenuHan_c). A friend
        // added by code (friend08) is asked for its name, town and comment in
        // turn (MENU_INPUT_NEW_FRIEND_*); the friend roster (friend04)
        // rewrites the name, town (asked after the name) and comment of a
        // friend (MENU_INPUT_FRIEND_*); the frame's text box (friend06) opens
        // the player's own friend name (MENU_INPUT_MY_FRIEND_NAME).
        OPEN_MENU_NEW_FRIEND_NAME = 0,
        OPEN_MENU_NEW_FRIEND_TOWN = 1,
        OPEN_MENU_FRIEND_NAME = 2,
        OPEN_MENU_FRIEND_TOWN = 3,
        OPEN_MENU_NEW_FRIEND_COMMENT = 4,
        OPEN_MENU_FRIEND_COMMENT = 5,
        OPEN_MENU_MY_FRIEND_NAME = 6,
        // l_openMenu indexes of the friend code entry (dMenuFriend08_c): add a
        // friend, reenter the code of a friend.
        OPEN_MENU_CODE_ADD = 7,
        OPEN_MENU_CODE_EDIT = 8,
    };

    typedef void (dMenuFriend_c::*SubStateProc)();

    virtual int create();     // .text:0x138
    virtual int doDelete();   // .text:0x2B0
    virtual int stateMain();  // .text:0x318
    virtual int stateAnm();   // .text:0x358
    virtual int execute();    // .text:0x35C
    virtual int stateClose(); // .text:0x360
    virtual int draw();       // .text:0x42C

    void subStateIdle();         // .text:0x504
    void subStateOpenTab();      // .text:0x508
    void subStateWaitReopen();   // .text:0x564
    void subStateOpenMenu();     // .text:0x588
    void subStateWaitMenu();     // .text:0x5E4
    void subStateWaitClose();    // .text:0x638
    void executeSubState();      // .text:0x650
    void preSubState();          // .text:0x688
    void postSubState();         // .text:0x68C
    void setTabAnm(int anm);     // .text:0x690
    void initTabAnm();           // .text:0x6B0
    void calcTab();              // .text:0x77C
    void initFrameAnm();         // .text:0x874
    void calcFrame();            // .text:0x8DC
    static void initFrameText(); // .text:0x15F8

    // Implicit destructor: .text:0x16FC.

    // The interface of the friend sub menus (all work on the open menu,
    // sInstance).

    // Each request below fails (FALSE) while the menu is still handling a
    // previous one.
    static BOOL
    requestClose(); // .text:0x9C8: the child closes, then this menu closes
    static BOOL requestReopen(); // .text:0xA2C: the child closes, then the
                                 // tab's menu opens again
    static BOOL
    requestOpenMenu(int idx); // .text:0xA7C: the child closes, the tabs hide,
                              // then l_openMenu[idx] opens
    static BOOL requestShowTabs(); // .text:0xB10: show the bottom panel's tabs,
                                   // the tab's menu opens again

    static void setTabVisible(BOOL visible); // .text:0xB7C
    static void setTabFocus(
        int tab,
        BOOL focus); // .text:0xBA4: the pointer is on a tab (tutorial balloon)
    static void clearTabFocus();                // .text:0xCDC
    static int getTabAt(const mVec2_c *pos);    // .text:0xD1C: the tab under a
                                                // screen position, or -1
    static BOOL isFrameHit(const mVec2_c *pos); // .text:0xD74: the frame's
                                                // button is under the position
    static BOOL
    startTabPush(int tab);       // .text:0xDDC: FALSE if tab is the current one
    static BOOL isTabPushEnd(int tab); // .text:0xE80 (tab is unused)
    static void setTab(int tab); // .text:0xE90: switch the tab's animations
    static void placeFrameTextBox(m2d::SimpleEx_c *lyt,
                                  const char *paneName); // .text:0xF00
    static void setFrameVisible(BOOL visible);           // .text:0xFBC

    // The friend entry that is edited (mFriendData, mFriend), registered into
    // slot mFriendIdx.
    static void clearEntry();                            // .text:0x1040
    static void setEntryPlayerName(const wchar_t *name); // .text:0x1088
    static void setEntryTown(const wchar_t *townName);   // .text:0x109C
    static BOOL setEntryKey(
        u64 key); // .text:0x10EC: FALSE if the key is another slot's friend
    static void setEntryFriendData(const DWCFriendData *data); // .text:0x1158
    static u64 getFriendKey(); // .text:0x1170: the key of slot mFriendIdx
    static void setEntryMessage(const wchar_t *message); // .text:0x1180
    static void setEntryFromNetPlayer(int member);       // .text:0x11E4
    static void setFriendIdx(int idx);                   // .text:0x1308
    static int getFriendIdx();                           // .text:0x1318
    static BOOL registerEntry(); // .text:0x1328: dWifi::registerFriendReq
    static void
    loadFriend(int idx); // .text:0x1340: slot idx of the player's list

    // The frame's text box.
    static void startTextBoxUp();   // .text:0x13C0
    static void startTextBoxDown(); // .text:0x1420
    static void startUpperWinOut(); // .text:0x147C: also requestClose
    static BOOL isFrameAnmDone();   // .text:0x14D0: TRUE once, after the frame
                                    // animation stopped
    static void
    showTextBox(BOOL show); // .text:0x1524: slide the text box in or out
    static void resetTextBoxPush();             // .text:0x15A0
    static BOOL isTextBoxPushEnd();             // .text:0x15B8
    static nw4r::lyt::TextBox *getTextBox();    // .text:0x15C8: T_textBox_00
    static nw4r::lyt::Pane *getTextBoxWindow(); // .text:0x15E0: W_textBox_00

    static void
    setSave(); // .text:0x1664: save the friend list when the menu closes
    static void hideTutorial(); // .text:0x16F8

    // A tutorial balloon at pane (request, position, message).
    static void showTutorial(u16 msg, nw4r::lyt::Pane *pane,
                             int tail); // .text:0x1674

    // Inline virtuals of the base class: .text:0x17EC .. 0x1804.
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

    static dMenuFriend_c *sInstance;

    /* 0x0D4 */ m2d::ResAccLoader_c mTabRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLytTab;
    /* 0x240 */ m2d::Anm_c mTabAnm;
    /* 0x264 */ m2d::Anm_c mTabPushAnm;
    /* 0x288 */ m2d::AnmEx2_c mTabBtnAnm[TAB_NUM];
    /* 0x398 */ m2d::ResAccLoader_c mFrameRes;
    /* 0x46C */ m2d::SimpleEx_c mLytFrame;
    /* 0x504 */ m2d::AnmEx_c mFrameAnm;
    /* 0x540 */ m2d::Anm_c mFrameMoveAnm;
    /* 0x564 */ m2d::Anm_c mFramePushAnm;
    /* 0x588 */ int mTab;
    /* 0x58C */ int mFocus;
    /* 0x590 */ int mNextMenu;
    /* 0x594 */ u8 _594[0xC];
    /* 0x5A0 */ DWCFriendData mFriendData;
    /* 0x5B0 */ dFriend_c mFriend;
    /* 0x628 */ int mFriendIdx;
    /* 0x62C */ u8 _62C[0x14];
}; // size 0x640

namespace dMenuFriend {
void *classInit();
}
