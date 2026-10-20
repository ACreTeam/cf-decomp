#ifndef GAME_D_MENU_ITEM02_HPP
#define GAME_D_MENU_ITEM02_HPP

#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_itemxx.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_menu_title.hpp>

// The pocket picker menus (REL d_menu_item02NP; the menus 0x29..0x34, 0x77 and
// 0x78 of dMenuMng_c): the pockets of dMenuItemXX_c with a title and a
// confirm / quit button bar, from which an item (or, for the sell menu, several
// items) is chosen for a villager's request (give, sell, donate, ...). Pockets
// in the mask given to dMenuMng_c::openItemSel / setPocketMask are disabled.
// The song menu (0x2B) learns the chosen K.K. song instead. RTTI confirms the
// class name; everything else is inferred.
class dMenuItem02_c : public dMenuItemXX_c {
  public:
    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,         // waits for the pushed button to finish
        SUB_DIALOG,       // the "already there" dialog: waits for its button
        SUB_DIALOG_PUSH,  // waits for the dialog button to finish
        SUB_DIALOG_CLOSE, // waits for the dialog to close
        SUB_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag (after dMenuItemXX_c's).
    enum {
        FLAG_MULTI = 13,  // several pockets can be picked (the sell menu)
        FLAG_DIALOG = 14, // the dialog is shown
    };

    // The dMenuBtn_c buttons.
    enum BTN_e {
        BTN_QUIT,
        BTN_UNUSED,
        BTN_CONFIRM,
    };

    // The dMenuSlctB_c buttons of the dialog (only the third one is shown).
    enum DIALOG_BTN_e {
        DIALOG_BTN_YES,
        DIALOG_BTN_NO,
        DIALOG_BTN_OK,
    };

    enum { SELECTED_NONE = 0xFF }; // mSelectedSlot: nothing picked

    // The title icons (dMenuTitle::Title_c::setIcon).
    enum TITLE_ICON_e {
        ICON_SELL = 1,
        ICON_GIVE_A = 3,
        ICON_GIVE = 4,
        ICON_SONG = 5,
        ICON_AUCTION = 6,
        ICON_GIVE_B = 7,
        ICON_DONATE = 8,
        ICON_GIVE_C = 9,
        ICON_SWAP = 10,
        ICON_QUEST = 11,
        ICON_GIVE_D = 12,
        ICON_GIVE_E = 13,
        ICON_GYROID = 14,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
        MSG_SONG_KNOWN = 0xAB, // "This music is already there."
        MSG_OK = 0xEC,
        MSG_SELL = 0xFB,      // "What do you want to sell?"
        MSG_GIVE_WHAT = 0xFF, // "What do you want to give?"
        MSG_CHOOSE_SONG = 0x100,
        MSG_AUCTION = 0x108,
        MSG_GIVE_WHICH = 0x109,
        MSG_DONATE = 0x10A,
        MSG_IDENTIFY = 0x10B,
        MSG_SWAP = 0x10C,
        MSG_GIVE_WHICH_B = 0x10D,
        MSG_GIVE_WHICH_C = 0x10E,
        MSG_GIVE_WHAT_B = 0x116,
        MSG_GIVE_GYROID = 0x128,
    };

    virtual int create();                     // .text:0x28C
    virtual int doDelete();                   // .text:0x5F4
    virtual int stateMain();                  // .text:0x66C
    virtual int stateAnm();                   // .text:0x6AC
    virtual int stateClose();                 // .text:0x6B0
    void subStateMain();                      // .text:0x708
    void subStatePush();                      // .text:0x9D8
    void subStateDialog();                    // .text:0xA9C
    void subStateDialogPush();                // .text:0xAFC
    void subStateDialogClose();               // .text:0xB60
    void executeSubState();                   // .text:0xBBC
    void updateReturn();                      // .text:0xBF4
    void stubUpdate();                        // .text:0xC14 (empty)
    virtual int execute();                    // .text:0xC18
    virtual int draw();                       // .text:0xC88
    void closeMenu();                         // .text:0xD04
    void setupButtons();                      // .text:0xD68
    void setConfirmDisabled(int disabled);    // .text:0xDF4
    BOOL isDisabled(int slot);                // .text:0xE10
    void setDisabled(int slot, int disabled); // .text:0xE34
    void updateDisabled();                    // .text:0xEB4
    BOOL isSelected(int slot);                // .text:0xF10
    void setSelected(int slot, int selected); // .text:0xF34
    void showDialog(u16 msgNo);               // .text:0xFB4
    BOOL learnSong();                         // .text:0x106C
    void playSeSongSelect();                  // .text:0x1114
    void playSeSongDeselect();                // .text:0x111C

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x15DC
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x15D8
    virtual void deleteReady() {
    } // .text:0x15D4
    virtual bool createHeap() {
        return true;
    } // .text:0x15CC

    // Implicit destructor: .text:0x1124.

    /* 0x502C */ dMenuSlctB_c mSlct;
    /* 0x571C */ dMenuBtn_c mBtn;
    /* 0x5C60 */ dMenuTitle::Title_c mTitle;
    /* 0x5E18 */ u16 mDisabledMask; // the pockets that can't be picked
    /* 0x5E1A */ u16 mSelectedMask; // the picked pockets
    /* 0x5E1C */ u8 mSelectedSlot;  // the picked pocket (0xFF none)
}; // size 0x5E20

#endif
