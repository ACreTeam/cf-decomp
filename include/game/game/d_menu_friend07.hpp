#ifndef GAME_D_MENU_FRIEND07_HPP
#define GAME_D_MENU_FRIEND07_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/mLib/m_vec.hpp>

// The Mic Chat tab of the friend menu (REL d_menu_friend07NP, profile
// MENU_FRIEND07): the window of Layout/chat_voice.arc
// (chat_voice_win_00.brlyt) with the Mic Chat settings. Mic Chat can be turned
// On or Off, its Volume has a down and an up button (1..10), and the
// Headphones setting is Use or Don't Use. It is the fourth tab of
// dMenuFriend_c (opened as MENU_FRIEND07 by its tab bar); the tab bar of the
// friend menu and the tabs of the bottom UI panel (dAcUitag) are both
// reachable from here. Turning Mic Chat On, and the headphone choices, show a
// message in a dialog (dMenuSlctB_c). RTTI confirms the global class name
// dMenuFriend07_c; everything else is inferred.
class dMenuFriend07_c : public dMenuBase_c {
  public:
    // What the pointer / cursor points at (mCursor).
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_TAB = 0, // 0..7: the tabs of the bottom UI panel
        CURSOR_TAB_LAST = 7,
        CURSOR_FRIEND_TAB = 8, // 8..11: the tabs of dMenuFriend_c
        CURSOR_FRIEND_TAB_LAST = 11,
        CURSOR_ON = 12,  // Mic Chat On
        CURSOR_OFF = 13, // Mic Chat Off
        CURSOR_VOL_DOWN = 14,
        CURSOR_VOL_UP = 15,
        CURSOR_USE = 16,     // Headphones: Use
        CURSOR_NOT_USE = 17, // Headphones: Don't Use
        CURSOR_BTN = CURSOR_ON,
        CURSOR_BTN_LAST = CURSOR_NOT_USE,
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,         // a button's push animation
        SUB_DIALOG,       // the dialog opens, waits for its button
        SUB_DIALOG_PUSH,  // the dialog button's push animation
        SUB_DIALOG_CLOSE, // the dialog closes
        SUB_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW = 1,
        FLAG_DIALOG = 2, // the dialog is drawn
    };

    // The animations of a button (mBtnAnm): the cursor on it, off it.
    enum BTN_ANM_e {
        BTN_ANM_IN,
        BTN_ANM_OUT,
    };

    // The animations of the On / Off buttons and of the volume buttons
    // (mSwitchAnm, mOffAnm, mVolAnm): the push, then the look of the button.
    enum SWITCH_ANM_e {
        SWITCH_ANM_PUSH,
        SWITCH_ANM_WHITE, // the button can be pushed
        SWITCH_ANM_NORMAL,
    };
    enum VOL_ANM_e {
        VOL_ANM_PUSH,
        VOL_ANM_GRAY, // Mic Chat is off
        VOL_ANM_YELLOW,
    };

    // The animations of the Use / Don't Use buttons (mUseAnm): the push, then
    // the look of the button.
    enum USE_ANM_e {
        USE_ANM_PUSH,
        USE_ANM_WHITE,  // can be pushed
        USE_ANM_NORMAL, // the current choice
        USE_ANM_GRAY,
        USE_ANM_YELLOW,
    };

    // The animations of a title (mVolTitleAnm, mUseTitleAnm).
    enum TITLE_ANM_e {
        TITLE_ANM_GRAY,
        TITLE_ANM_WHITE,
    };

    // The volume range of Mic Chat.
    enum {
        VOLUME_MIN = 1,
        VOLUME_MAX = 10,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_HEADPHONES = 0xA6,
        MSG_USE = 0xA7,
        MSG_DONT_USE = 0xA8,
        MSG_MIC_CHAT_SETTINGS = 0xAF,
        MSG_MIC_CHAT = 0xB0,
        MSG_ON = 0xB1,
        MSG_OFF = 0xB2,
        MSG_VOLUME = 0xB3,
        MSG_OK = 0xEC,
        MSG_HEADPHONES_USE = 0x152,      // "...settings have been set to Use."
        MSG_HEADPHONES_DONT_USE = 0x153, // "... set to Don't Use."
        MSG_MIC_CHAT_START = 0x15F,      // "Now starting Mic Chat. ..."
    };

    static bool isTab(int cursor) {
        return cursor >= CURSOR_TAB && cursor <= CURSOR_TAB_LAST;
    }
    static bool isFriendTab(int cursor) {
        return cursor >= CURSOR_FRIEND_TAB && cursor <= CURSOR_FRIEND_TAB_LAST;
    }
    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }

    typedef BOOL (dMenuFriend07_c::*CloseProc)();
    typedef void (dMenuFriend07_c::*SubStateProc)();

    virtual int create();     // .text:0x168
    virtual int doDelete();   // .text:0x248
    virtual int stateMain();  // .text:0x29C
    virtual int stateAnm();   // .text:0x2E4
    virtual int execute();    // .text:0x2E8
    virtual int stateClose(); // .text:0x3B4
    virtual int draw();       // .text:0x3D8

    void subStateMain();                     // .text:0x438
    void subStatePush();                     // .text:0x504
    void subStateDialog();                   // .text:0x540
    void subStateDialogPush();               // .text:0x5A0
    void subStateDialogClose();              // .text:0x604
    BOOL closeMain();                        // .text:0x660
    BOOL closeDialog();                      // .text:0x698
    BOOL checkCloseRequest();                // .text:0x6F4
    void executeSubState();                  // .text:0x7EC
    int getTabAt(const mVec2_c *pos);        // .text:0x824
    BOOL selectTab(int tab, BOOL delay);     // .text:0x888
    void startClose();                       // .text:0x8D8
    void startReopen();                      // .text:0x914
    int hitCheck();                          // .text:0x954
    void setSelect(int cursor, BOOL select); // .text:0xA3C
    void push();                             // .text:0xAE4
    BOOL isPushEnd();                        // .text:0xCD8
    void finishPush();                       // .text:0xD6C
    void initMessages();                     // .text:0xF40
    void initState();                        // .text:0x1018
    void initAnms();                         // .text:0x1050
    void playAnms();                         // .text:0x12DC
    BOOL isVoiceChatOff();                   // .text:0x136C
    void setVoiceChatOff(BOOL off);          // .text:0x1398
    BOOL isHeadphoneUsed();                  // .text:0x1464
    void setHeadphoneUsed(BOOL used);        // .text:0x148C
    void updateAnms(BOOL headphone);         // .text:0x1520
    u8 getVolume();                          // .text:0x16F0
    void setVolume(u8 volume);               // .text:0x16F4
    void changeVolume(int delta);            // .text:0x1740
    void updateVolumeGray();                 // .text:0x17AC
    void playSeSwitchOn();                   // .text:0x182C
    void playSeSwitchOff();                  // .text:0x1834
    void playSeVolume();                     // .text:0x183C
    void openDialog(u16 msg);                // .text:0x188C

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

    // Implicit destructor: .text:0x194C.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::Anm_c mIconAnm; // scrlIcon_00
    // The highlight of the six buttons (CURSOR_BTN..).
    /* 0x264 */ m2d::AnmEx2_c mBtnAnm[6];
    /* 0x3FC */ m2d::AnmEx2_c mSwitchAnm[2]; // On and Off buttons
    /* 0x484 */ m2d::AnmEx2_c mVolAnm[2];    // volume down and up
    /* 0x50C */ m2d::AnmEx2_c mUseAnm[2];    // Use and Don't Use
    /* 0x594 */ m2d::Anm_c mVolGrayAnm;
    /* 0x5B8 */ m2d::AnmEx2_c mVolTitleAnm;
    /* 0x5FC */ m2d::AnmEx2_c mUseTitleAnm;
    /* 0x640 */ dMenuSlctB_c mSlct;
    /* 0xD30 */ int mCursor;
}; // size 0xD34

#endif
