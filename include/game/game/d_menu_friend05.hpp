#ifndef GAME_D_MENU_FRIEND05_HPP
#define GAME_D_MENU_FRIEND05_HPP

#include <game/game/d_friend.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_net.hpp>
#include <lib/dwc/dwc.h>

class dPrivateData_c;

// The "Visiting Friends" tab of the friend menu (REL d_menu_friend05NP, profile
// MENU_FRIEND05, MENU_FRIEND05 = dMenuMng_c 0x58): the third tab of
// dMenuFriend_c. The window of Layout/friend_visitor.arc ("Friends Now
// Visiting") lists the other players that are in the online session, at most
// three rows: the player's name, his town, an icon that tells whether he is
// already on the friend roster (situation) and an icon for his voice chat
// state. Picking a row asks (dMenuSlctB_c) whether to add him to the friend
// roster; Yes requests his friend data, registers it (dWifi::registerFriendReq)
// and sends the preset message 5 (dObjChat::postPreset). The tabs of the friend
// menu (dMenuFriend_c) and the tabs of the bottom panel (dAcUitag) stay usable.
// RTTI confirms the global class name dMenuFriend05_c; everything else is
// inferred.
class dMenuFriend05_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_ACTION_TAB = 0, // 0..7: the tabs of the bottom panel (dAcUitag)
        CURSOR_ACTION_TAB_LAST = CURSOR_ACTION_TAB + 7,
        CURSOR_TAB = 8, // 8..11: the tabs of dMenuFriend_c
        CURSOR_TAB_LAST = CURSOR_TAB + 3,
        CURSOR_ROW = 12, // 12..14: the visitors
        CURSOR_ROW_LAST = CURSOR_ROW + 2,
    };

    enum {
        ROW_NUM = 3,
        // mMember[]: no player in the row (dNet member numbers are 0..3).
        MEMBER_NONE = NET_MEMBER_MAX,
        // How often the voice chat states are asked, in frames.
        VOICE_QUERY_INTERVAL = 100,
    };

    // Per-row animations (mRowAnm): the file names visitor_win_00_T_msg_xx.
    enum ROW_ANM_e {
        ROW_ANM_IN,
        ROW_ANM_OUT,
        ROW_ANM_PUSH,
        ROW_ANM_NUM,
    };

    // The friend roster state of a row's player (dFriendList_c::getState):
    // 0 is not on the roster (no icon), the others pick a frame of the
    // situation icon.
    enum SITUATION_e {
        SITUATION_NONE,
        SITUATION_KNOWN,    // frame 0
        SITUATION_NO_KEY,   // frame 1
        SITUATION_KEY_ONLY, // frame 2
        SITUATION_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_UNUSED, // bit 0 is never used
        FLAG_DRAW,
        FLAG_VOICE_QUERY, // the voice chat states were asked, waiting for the
                          // replies
        FLAG_DIALOG,      // mSlct is shown
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,         // waits for the pushed tab / row to finish
        SUB_DIALOG_IN,    // the dialog opens
        SUB_DIALOG,       // waits for a dialog button
        SUB_DIALOG_OUT,   // the dialog closes: act on the pushed button
        SUB_REQUEST_DATA, // request the friend data of the player
        SUB_WAIT_DATA,    // wait for it
        SUB_REGISTER,     // register the friend
        SUB_NUM,
    };

    // dMenuSlctB_c buttons.
    enum DIALOG_BTN_e {
        DIALOG_BTN_NO,
        DIALOG_BTN_YES,
        DIALOG_BTN_OK,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_WORD0 = 0x83,       // just word slot 0 (the town name)
        MSG_TITLE = 0x93,       // "Friends Now Visiting"
        MSG_ROSTER_FULL = 0x9A, // "You can't register a new friend if your
                                // friend roster is full!"
        MSG_ALREADY_REGISTERED = 0xA1, // "That person is already registered!"
        MSG_OK = 0xEC,
        MSG_ADD_FRIEND =
            0x121, // "Add <player> from <town> to your friend roster?"
    };

    // The cursor classes.
    static bool isActionTab(int cursor) {
        return cursor >= CURSOR_ACTION_TAB && cursor <= CURSOR_ACTION_TAB_LAST;
    }
    static bool isTab(int cursor) {
        return cursor >= CURSOR_TAB && cursor <= CURSOR_TAB_LAST;
    }
    static bool isRow(int cursor) {
        return cursor >= CURSOR_ROW && cursor <= CURSOR_ROW_LAST;
    }

    virtual int create();     // .text:0x180
    virtual int doDelete();   // .text:0x278
    virtual int stateMain();  // .text:0x2CC
    virtual int stateAnm();   // .text:0x314
    virtual int execute();    // .text:0x318
    virtual int stateClose(); // .text:0x3EC
    virtual int draw();       // .text:0x410

    void subStateMain();                      // .text:0x470
    void subStatePush();                      // .text:0x53C
    void subStateDialogIn();                  // .text:0x578
    void subStateDialog();                    // .text:0x5D8
    void subStateDialogOut();                 // .text:0x63C
    void subStateRequestData();               // .text:0x708
    void subStateWaitData();                  // .text:0x760
    void subStateRegister();                  // .text:0x7C0
    BOOL closeMenu();                         // .text:0x838
    BOOL closeDialog();                       // .text:0x870
    BOOL checkCloseRequest();                 // .text:0x8CC
    void executeSubState();                   // .text:0x9E8
    int hitActionTab(const mVec2_c *pos);     // .text:0xA20
    BOOL selectActionTab(int tab, BOOL wait); // .text:0xA84
    void startClose();                        // .text:0xAD4
    void startReopen();                       // .text:0xB10
    int hitCheck();                           // .text:0xB50
    void setCursor(int cursor, BOOL select);  // .text:0xC2C
    void push();                              // .text:0xCD4
    BOOL isPushEnd();                         // .text:0xDC0
    void finishPush();                        // .text:0xE28
    void setTitle();                          // .text:0xED0
    void refreshAll();                        // .text:0xEE8
    void initRowWords();                      // .text:0xF1C
    void updateRowWords();                    // .text:0xFCC
    void updateSituations();                  // .text:0x1110
    void updateVoices();                      // .text:0x1214
    int getMember(int idx);                   // .text:0x1314
    void initMembers();                       // .text:0x1398
    dPrivateData_c *getPlayer(int row);       // .text:0x13F8
    int getFriendState(int row);              // .text:0x141C
    void updateVoice();                       // .text:0x148C
    void startVoiceQuery();                   // .text:0x1524
    void initAnms();                          // .text:0x1564
    void calcAnms();                          // .text:0x167C
    void openAddDialog(int row);              // .text:0x16DC
    void openNotice(u16 msg);                 // .text:0x1810
    void startRegister();                     // .text:0x18C4

    // Implicit destructor: .text:0x19FC.

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

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::Anm_c mSituationAnm[ROW_NUM]; // situation_xx
    /* 0x2AC */ m2d::Anm_c mVoiceAnm[ROW_NUM];     // voice_xx
    /* 0x318 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];    // T_msg_xx
    /* 0x3E4 */ m2d::Anm_c mScrlIconAnm;
    /* 0x408 */ int mCursor;
    /* 0x40C */ m2d::stringWord_c mNameWord[ROW_NUM]; // T_name_xx
    /* 0x6E8 */ m2d::stringWord_c mTownWord[ROW_NUM]; // T_msg_xx
    /* 0x9C4 */ int mVoiceTimer; // frames until the voice chat states are asked
    /* 0x9C8 */ int mMember[ROW_NUM]; // the dNet member of each row
    /* 0x9D4 */ dMenuSlctB_c mSlct;   // the "add to roster?" dialog
    /* 0x10C4 */ u8 _10C4[0x1C];
    /* 0x10E0 */ DWCFriendData mFriendData; // the picked player's friend data
    /* 0x10F0 */ dFriend_c mFriend;         // the roster entry to register
    /* 0x1168 */ int mFriendIdx;            // the free roster slot
    /* 0x116C */ int mSel;                  // the picked row
    /* 0x1170 */ u8 _1170[0x10];
}; // size 0x1180

#endif
