#ifndef GAME_D_MENU_ITEM03_HPP
#define GAME_D_MENU_ITEM03_HPP

#include <game/game/d_item.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_itemxx.hpp>
#include <game/game/d_menu_slct_b.hpp>

// The pockets-full menu (REL d_menu_item03NP, profile MENU_ITEM03): the player
// has picked up, dug up or caught an item (insect, fish) but the pockets are
// full. The pocket window (dMenuItemXX_c) asks which pocket item to get rid of;
// the answer menu offers Drop (swap: the pocket item is put down on the ground
// and the new item takes its slot), Bury (dug-up items, in town), Release (an
// insect, or a fish at water) or Quit. The new item is
// dMenuMng_c::getMenuItem(); the fish menu also gets the position of the water
// (g_menuPos). It is opened as MENU_SWAP_PICK_UP / DUG / INSECT / FISH and
// reports the pocket (setSelSlot) and the result (setResult) to its opener.
// RTTI confirms the global class name dMenuItem03_c; everything else is
// inferred.
class dMenuItem03_c : public dMenuItemXX_c {
  public:
    // The dMenuBtn_c buttons (the other two are hidden).
    enum BTN_e {
        BTN_UNUSED_0,
        BTN_UNUSED_1,
        BTN_QUIT,
    };

    // The dMenuSlctB_c dialog's buttons (only the right one is shown).
    enum SLCT_BTN_e {
        SLCT_BTN_LEFT,
        SLCT_BTN_MIDDLE,
        SLCT_BTN_OK,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag (after dMenuItemXX_c's).
    enum FLAG_BIT_e {
        FLAG_ANSWERED = 11, // the answer menu was closed
        FLAG_WATER = 14,    // a fish or insect can be released here
        FLAG_DIALOG = 15,   // the dMenuSlctB_c dialog is shown
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,       // waits for the pushed button to finish
        SUB_PUT_EFFECT, // waits for the put-down request of the swapped item
        SUB_RELEASE,    // releases the insect / fish
        SUB_USE_TOOL,   // waits for the shovel request that buries the item
        SUB_WAIT_IDLE,  // waits for the player to be idle again
        SUB_DIALOG_OPEN,
        SUB_DIALOG_PUSH,
        SUB_DIALOG_CLOSE,
        SUB_SELECT, // the answer menu is open
        SUB_NUM,
    };

    // The answers of the answer menu (MComp_c::addItem).
    enum ANSWER_e {
        ANSWER_CANCEL,
        ANSWER_DROP,
        ANSWER_BURY,
        ANSWER_RELEASE,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x02,
        MSG_DROP = 0x03,
        MSG_BURY = 0x04,
        MSG_RELEASE = 0x42,
        MSG_OK = 0xEC,
        MSG_NO_SPACE_TO_PUT_DOWN = 0x132,
    };

    virtual int create();     // .text:0x268
    virtual int doDelete();   // .text:0x3DC
    virtual int stateMain();  // .text:0x454
    virtual int stateAnm();   // .text:0x494
    virtual int stateClose(); // .text:0x4EC
    virtual int execute();    // .text:0xB64
    virtual int draw();       // .text:0xBB0

    void subStateMain();                          // .text:0x510
    void subStatePush();                          // .text:0x710
    void subStatePutEffect();                     // .text:0x758
    void subStateRelease();                       // .text:0x7E0
    void subStateUseTool();                       // .text:0x87C
    void subStateWaitIdle();                      // .text:0x928
    void subStateDialogOpen();                    // .text:0x970
    void subStateDialogPush();                    // .text:0x9D0
    void subStateDialogClose();                   // .text:0xA34
    void subStateSelect();                        // .text:0xA90
    void executeSubState();                       // .text:0xB08
    void calcReturnItem();                        // .text:0xB40
    void calcUnused();                            // .text:0xB60
    void confirm();                               // .text:0xC24
    void startSwap(int slot);                     // .text:0xC7C
    void startPutDown();                          // .text:0xD18
    void initButtons();                           // .text:0xDE4
    bool isDisabled(int slot);                    // .text:0xE40
    void setDisabled(int slot, BOOL disabled);    // .text:0xE64
    void updatePockets();                         // .text:0xEE4
    bool canReleaseFish(const dItem::Item *item); // .text:0xFD0
    bool canBury(const dItem::Item *item);        // .text:0x101C
    void backToMain();                            // .text:0x1088
    void startSwapSelected();                     // .text:0x109C
    void startRelease();                          // .text:0x10A4
    void startBury();                             // .text:0x10E8
    void openAnswers(int slot);                   // .text:0x1170
    void selectAnswer(int answer);                // .text:0x12A4
    void showError(u16 msg, BOOL leave);          // .text:0x1328

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x1878
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x1874
    virtual void deleteReady() {
    } // .text:0x1870
    virtual bool createHeap() {
        return true;
    } // .text:0x1868

    // Implicit destructor: .text:0x13FC.

    /* 0x502C */ dMenuSlctB_c mSlct;
    /* 0x571C */ dMenuBtn_c mBtn;
    /* 0x5C60 */ dItem::Item mItem; // the pocket item that is put down
    /* 0x5C64 */ mVec3_c mPos;      // where a fish / insect is released
    /* 0x5C70 */ int mTask;         // the dFgobjManager request
    /* 0x5C74 */ u16 mDisabled; // bit mask of the pockets that can't be picked
}; // size 0x5C78

#endif
