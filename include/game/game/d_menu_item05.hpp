#ifndef GAME_D_MENU_ITEM05_HPP
#define GAME_D_MENU_ITEM05_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_item.hpp>

// The lost and found menu (REL d_menu_item05NP, profile MENU_ITEM05): the
// player's pockets (the item window of d_menu_itemNP) with a second window
// (Layout/gather.arc) holding the lost items, twelve slots, and a close button.
// Items are carried from a slot of the lost items to an empty pocket with the
// pointer. It serves two boxes, picked by the menu number: the police box
// (MENU_LOST_AND_FOUND, the player may only fill empty pockets and what was
// taken is reported through dMenuMng_c::setLostTakenNum / setLostTakenItem) and
// the town hall's recycle box (MENU_RECYCLE_BOX, pockets with an item flag are
// locked and the box is synced to the other players on close). RTTI confirms
// the class name dMenuItem05_c; everything else is inferred.
class dMenuItem05_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_POCKET = 0, // 0..14: the pockets of the player (item window)
        CURSOR_POCKET_LAST =
            CURSOR_POCKET + dMenuItem::Window_c::POCKET_COUNT - 1,
        CURSOR_LOST = 15, // 15..26: the lost items
        CURSOR_LOST_LAST = CURSOR_LOST + 11,
        CURSOR_CLOSE = 27,
    };

    enum {
        LOST_NUM = 12,
        POCKET_NUM = dMenuItem::Window_c::POCKET_COUNT,
        TUTORIAL_TAIL_POCKET =
            9, // the balloon of the last pockets points to the other side
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH, // waits for the close button to finish its push animation
        SUB_HOLD, // an item is carried by the pointer
        SUB_DROP, // the released item moves back to the slot it came from
        SUB_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC, // the layout needs a calc
        FLAG_HOLD, // an item is carried
        FLAG_DROP, // the dropped item is still moving
    };

    // The close button animations (mAnm).
    enum ANM_BTN_e {
        ANM_BTN_IN,
        ANM_BTN_OUT,
        ANM_BTN_PUSH,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_CLOSE = 0xEB,
    };

    // The tutorial balloon request type of dObjTutorial_c::request.
    enum {
        TUTORIAL_REQUEST_ITEM = 3,
    };

    static bool isPocket(int cursor) {
        return cursor >= CURSOR_POCKET && cursor <= CURSOR_POCKET_LAST;
    }
    static bool isLost(int cursor) {
        return cursor >= CURSOR_LOST && cursor <= CURSOR_LOST_LAST;
    }

    virtual int create();                                      // .text:0x220
    virtual int doDelete();                                    // .text:0x358
    virtual int stateMain();                                   // .text:0x3AC
    virtual int stateAnm();                                    // .text:0x3D0
    virtual int stateClose();                                  // .text:0x414
    virtual int execute();                                     // .text:0x45C
    virtual int draw();                                        // .text:0x508
    void subStateMain();                                       // .text:0x58C
    void subStatePush();                                       // .text:0x68C
    void subStateHold();                                       // .text:0x6C8
    void subStateDrop();                                       // .text:0x76C
    void executeSubState();                                    // .text:0x7F0
    void setSelect(int cursor, BOOL select);                   // .text:0x828
    int hitCheck(bool pick);                                   // .text:0x918
    int hitCheckPick();                                        // .text:0xA60
    int hitCheckDrop();                                        // .text:0xB00
    void pushClose();                                          // .text:0xBA4
    BOOL isPushEnd();                                          // .text:0xC08
    void finishPush();                                         // .text:0xC24
    void startClose();                                         // .text:0xC48
    void initAnm();                                            // .text:0xD50
    void calcAnm();                                            // .text:0xDA8
    void initIcons();                                          // .text:0xDF4
    static int positionIcon(nw4r::lyt::Pane *pane, void *arg); // .text:0xE80
    void positionIcons();                                      // .text:0xEFC
    void drawIcons();                                          // .text:0xF68
    void setIcons();                                           // .text:0xFB4
    const mVec2_c &getSlotPos(int slot, f32 ofsX, f32 ofsY);   // .text:0x1028
    bool isLocked(int slot);                                   // .text:0x11B4
    void setLocked(int slot, BOOL locked);                     // .text:0x1200
    void initLocks();                                          // .text:0x1280
    u16 setSlotItem(int slot, dItem::Item item);               // .text:0x1340
    u16 putItem(int slot, dItem::Item item);                   // .text:0x146C
    u16 removeItem(int slot);                                  // .text:0x1508
    u16 getItem(int slot);                                     // .text:0x1538
    BOOL pickItem();                                           // .text:0x1604
    void calcHold();                                           // .text:0x16B0
    void dropItem();                                           // .text:0x1718
    void getItemName(dScript::Word_c *word, int slot);         // .text:0x1820
    void showTutorial(int slot);                               // .text:0x18F4
    void hideTutorial();                                       // .text:0x19F0
    int findNearestEmptyPocket();                              // .text:0x19F4
    int findNearestEmptyLost();                                // .text:0x1AD8
    int findFreeSlot();                                        // .text:0x1BBC

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x2064
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x205C
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x2058
    virtual void deleteReady() {
    } // .text:0x2054
    virtual bool createHeap() {
        return true;
    } // .text:0x204C

    // Implicit destructor: .text:0x1C84.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx_c mAnm;
    /* 0x27C */ dMenuItem::Window_c mWin;
    /* 0x2B7C */ dMenuItem::Icon_c mIcons[LOST_NUM];
    /* 0x47FC */ int mCursor;
    /* 0x4800 */ int mHeldSlot;         // the slot the carried item came from
    /* 0x4804 */ dItem::Item mHeldItem; // the carried item
    /* 0x4806 */ u16 mLockMask;         // bit n: pocket n can't be used
    /* 0x4808 */ int mLostNum; // police box: the lost items when opened, then
                               // the number taken
}; // size 0x480C

#endif
