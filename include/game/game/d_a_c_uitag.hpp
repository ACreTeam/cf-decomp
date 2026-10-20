#pragma once

// The UI tag process (RTTI "dAcUitag_c"): the menu tabs at the bottom of the screen (tab_00), the save button
// (saveBtn_00), the camera message (cameraMsg_00) and the emotion buttons (outdoor_actionBtn_00). It also takes
// the photos (the 1 button). Source: src/dol/game/d_a_c_uitag.cpp (.text 8017E7B0..801811D0).
// Only the class name is from the RTTI; every other name is inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>

class dAcUitag_c : public fBase_c {
public:
    // The menu bar tabs (tab_00); l_tabMenus holds the menu each one opens.
    enum Tab_e {
        TAB_ITEM,   // the pockets (REL d_menu_item00NP)
        TAB_DESIGN, // the design menu (REL d_menu_desi00)
        TAB_INSC,   // the insect and fish encyclopedia (REL d_menu_inscNP)
        TAB_MAP,    // the town map (REL d_menu_map00NP)
        TAB_CHAT,   // the chat menu (REL d_menu_chat)
        TAB_PHOTO,  // the photo menu (REL d_menu_photoNP)
        TAB_FRIEND, // the friend menu (REL d_menu_friendNP and its sub menus)
        TAB_CLOSE,  // only shown while a menu is open; selecting it closes the menu
        TAB_NUM,
    };

    enum { ACTION_NUM = 4 };

    // mTabBtnAnm kinds (l_tabBtnAnms).
    enum TabBtnAnm_e {
        TAB_BTN_IN,  // "tab_00_dpdIn": the pointer is on the tab
        TAB_BTN_OUT, // "tab_00_dpdOut"
        TAB_BTN_PSH, // "tab_00_dpdPsh": the tab was selected
    };

    // mTabAnm animations (l_tabAnms).
    enum TabAnm_e {
        TAB_ANM_IN,  // the tabs slide in
        TAB_ANM_OUT, // the tabs slide out
    };

    // mFlags. Bits 0..7: mTabBtnAnm[i] plays; bits 8..15: mTabClrAnm[i] plays.
    enum Flag_e {
        FLAG_TAB_BTN_ANM = 0x1,
        FLAG_TAB_CLR_ANM = 0x100,
        FLAG_NAND_WAIT = 0x80000,        // doDelete waits for dNand to be idle
        FLAG_CAMERA_CALC = 0x100000,     // mCameraLyt needs a calc
        FLAG_ACTION_LOCK = 0x200000,     // the emotion buttons are shown by startEmotion
        FLAG_MENU_WAIT = 0x400000,       // a menu demo was requested
        FLAG_NO_SAVE = 0x800000,         // no save button (online guest, or the player flag 0x28)
        FLAG_HIDE_AFTER_MENU = 0x1000000, // hide the tabs when the menu closes
        FLAG_ACTION_CALC = 0x2000000,    // mActionLyt needs a calc
        FLAG_NO_A_BUTTON = 0x4000000,    // the A button is used by this frame
        FLAG_SAVE_CALC = 0x8000000,      // mSaveBtnLyt needs a calc
        FLAG_TAB_CALC = 0x10000000,      // mTabLyt needs a calc
        FLAG_CLOSE_ICON = 0x20000000,    // the TAB_CLOSE icon is shown
        FLAG_SHOW = 0x40000000,          // draw the tabs and the save button
        FLAG_TAB_ANM = 0x80000000,       // mTabAnm plays
    };

    // mState: l_states.
    enum State_e {
        STATE_HIDDEN,
        STATE_IDLE,
        STATE_TAB_WAIT,
        STATE_MENU_REQUEST,
        STATE_MENU,
        STATE_MENU_CLOSE,
        STATE_OPEN_MENU_A,
        STATE_OPEN_MENU_B,
        STATE_SAVE,
        STATE_ACTION_REQUEST,
        STATE_EMOTION,
        STATE_EMOTION_WAIT,
    };

    // mPhotoState: l_photoStates.
    enum PhotoState_e {
        PHOTO_STATE_IDLE,
        PHOTO_STATE_SHUTTER,
        PHOTO_STATE_WAIT,
    };

    // mCameraMsgState
    enum CameraMsgState_e {
        CAMERA_MSG_INIT,
        CAMERA_MSG_SHOWN,
        CAMERA_MSG_DONE,
    };

    virtual int create();   // 8017E944
    virtual int doDelete(); // 8017EB80
    virtual int execute();  // 8017EC14
    virtual int draw();     // 8017ED08
    virtual ~dAcUitag_c();  // 80181064

    BOOL isFlag(u32 flag) { return (mFlags & flag) != 0; }

    void calcAnm(); // 8017EE1C

    void stateHidden();        // 8017F028
    void stateIdle();          // 8017F0F4
    void stateTabWait();       // 8017F378
    void stateMenuRequest();   // 8017F42C
    void stateMenu();          // 8017F4D8
    void stateMenuClose();     // 8017F4DC
    void stateOpenMenuA();     // 8017F52C
    void stateOpenMenuB();     // 8017F57C
    void stateSave();          // 8017F5CC
    void stateActionRequest(); // 8017F628
    void stateEmotion();       // 8017F68C
    void stateEmotionWait();   // 8017F6D4

    void setTabAnm(u8 anm, BOOL silent);               // 8017F728
    void unbindTabAnm();                               // 8017F7E8
    void setTabBtnAnm(u8 kind, int tab);               // 8017F800
    void changeTabBtnAnm(u8 kind, int tab);            // 8017F8D8 (works on l_this)
    void stopTabBtnAnm(m2d::Anm_c *anm, int tab);      // 8017F974
    void setTabClrAnm(int tab, int on);                // 8017F9C0
    void stopTabClrAnm(m2d::Anm_c *anm, int tab);      // 8017FA78
    BOOL playTabAnm();                                 // 8017FA80
    BOOL checkTabButton();                             // 8017FBB4
    int getButtonTab();                                // 8017FC4C
    int getCursorTab();                                // 80180300: the tab under the cursor, or -1
    void setCloseIcon(bool on);                        // 8018032C
    void finishTab();                                  // 801803C4
    void setSaveBtnAnm(int anm, BOOL force);           // 8018043C
    int hitSaveBtn();                                  // 80180534
    void setPhotoState(int state);                     // 801805A8
    void photoStateIdle();                             // 801805B0
    void photoStateShutter();                          // 80180620
    void photoStateWait();                             // 80180674
    void showActionTutorial();                         // 80180920
    void updateCameraMsg();                            // 801809E0
    void hideCameraMsg();                              // 80180AA4
    void initAction();                                 // 80180AE4
    void updateActionIcons(int sel, BOOL anim);        // 80180C1C
    void setActionHighlight(int idx, BOOL on);         // 80180DA8
    BOOL hitAction();                                  // 80180E50
    u8 getEmotion(int idx);                            // 80180ED8: 0xFF when the slot is empty
    void setActionPsh(int idx);                        // 80180F50

    /* 0x064 */ m2d::ResAccLoader_c mResAcc;       // "Layout/uitag.arc"
    /* 0x138 */ m2d::ResAccLoader_c mActionResAcc; // "Layout/outdoor_actionBtn.arc"
    /* 0x20C */ m2d::SimpleEx_c mTabLyt;
    /* 0x2A4 */ m2d::Anm_c mTabBtnAnm[TAB_NUM];
    /* 0x3C4 */ m2d::Anm_c mTabClrAnm[TAB_NUM];
    /* 0x4E4 */ m2d::Anm_c mUnusedAnm;
    /* 0x508 */ m2d::Anm_c mTabAnm;
    /* 0x52C */ int mTabAnmIdx; // TabAnm_e
    /* 0x530 */ m2d::SimpleEx_c mSaveBtnLyt;
    /* 0x5C8 */ m2d::AnmEx_c mSaveBtnAnm;
    /* 0x604 */ m2d::SimpleEx_c mCameraLyt;
    /* 0x69C */ m2d::AnmEx_c mCameraAnm;
    /* 0x6D8 */ m2d::SimpleEx_c mActionLyt;
    /* 0x770 */ m2d::AnmEx_c mActionAnm;
    /* 0x7AC */ m2d::Anm_c mActionPshAnm;
    /* 0x7D0 */ m2d::AnmEx2_c mActionIconAnm[ACTION_NUM];
    /* 0x8E0 */ m2d::AnmEx2_c mActionColorAnm[ACTION_NUM];
    /* 0x9F0 */ int mActionSel;
    /* 0x9F4 */ u32 mFlags;
    /* 0x9F8 */ int mState;
    /* 0x9FC */ int mPhotoState;
    /* 0xA00 */ int mPointerWait;
    /* 0xA04 */ int mTabWait;
    /* 0xA08 */ int mCameraMsgState;
    /* 0xA0C */ int mCameraMsgTimer;
    /* 0xA10 */ int mMenuRetry;
    /* 0xA14 */ u8 mTabBtnKind[TAB_NUM]; // TabBtnAnm_e playing
    /* 0xA1C */ u8 mTabBtnReq[TAB_NUM];  // TabBtnAnm_e wanted
}; // size 0xA24

namespace dAcUitag {

// The indices of openMenuA (the menu table l_menusA); only the ones named so far.
enum MenuA_e {
    MENU_A_DESIGN_SELECT = 1, // menu 0x0E: the design menu for the design in getDesignIndex
    MENU_A_EDIT_DESIGN = 2,   // menu 0x0F: the design editor
    MENU_A_ITEM = 8,    // menu 0x00: the item menu
    MENU_A_ITEM_1 = 9,  // menu 0x01: the item menu, second kind
    MENU_A_WRITE = 10,  // menu 0x41: the letter writing menu
};

// The indices of openMenuB (the menu table l_menusB); only the ones named so far.
enum MenuB_e {
    MENU_B_ITEM_SELECT = 1, // menu 0x2B: the pocket picker (after dMenuMng_c::setPocketMask)
    MENU_B_EDIT_PRO = 7, // menu 0x10: the pro design editor
};

int getTabMenu(int tab); // 8017E8FC: the menu of a tab
BOOL canOpenTab();       // 8017FAF4
int getNextTab(int tab); // 8017FCE0: the tab the - / + buttons move to, or -1
// 8017FD74: the tab picked by the buttons (also 2: tab 5, B / Z: TAB_CLOSE unless noClose), or -1. Used by menus.
int getTabInput(int tab, BOOL noClose);
BOOL isPointerInScreen(); // 8017FE28: the pointer is shown (dObjCursor_c::isVisible) and inside the screen
BOOL isPointerActive();   // 8017FEC4
// Inferred: the bottom UI panel exists and its animation is in the shown state.
bool isBottomPanelShown();        // 8017FF74
int getTabAt(const mVec2_c *pos); // 8017FF98: the tab under a screen position, or -1
void setTabFocus(int tab);        // 80180024: -1: none
BOOL selectTab(int tab, BOOL delay); // 80180088
void openMenuA(int idx);          // 801801E4
BOOL openMenuB(int idx);          // 801801F8
BOOL closeTab();                  // 80180240
void setHideAfterMenu();          // 801802AC
void hideTab();                   // 801802C0
void showTab();                   // 801802D0
m2d::ResAccLoader_c *getActionResAcc(); // 801802E0
void setNandWait();               // 801802EC
BOOL isPhotoBlocked();            // 801806B4
BOOL showTabTutorial(int tab);    // 801807D8
void showBtnTutorial(m2d::SimpleEx_c *lyt, u16 msg); // 8018089C
void hideCameraMsg();             // 80180AD0: d_obj_keyboard, when the typed text changes
void startEmotion(int slot);      // 80180FC4: d_obj_keyboard, starts the emotion of slot
void playSePush();                // 80181020
void playSePopUp();               // 80181024

} // namespace dAcUitag
