#ifndef GAME_D_MENU_ITEM04_HPP
#define GAME_D_MENU_ITEM04_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_item.hpp>

namespace dScript {
class Word_c;
}

// The item storage chest (REL d_menu_item04NP, profile MENU_ITEM04, menu
// dMenuMng_c::MENU_STORAGE): the pocket window (dMenuItem::Window_c) with the
// chest window (Layout/chest.arc) on top of it. The chest keeps sixteen pages
// of ten items (the player's dSaveItemBox_c); two arrow buttons turn the page
// and a square button closes the menu. Items are dragged between the pockets
// and the chest slots. RTTI confirms the class name dMenuItem04_c; everything
// else is inferred.
class dMenuItem04_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor, mHeldFrom, ...) points at.
    enum SLOT_e {
        SLOT_NONE = -1,
        SLOT_POCKET = 0, // 0..14: dMenuItem::Window_c pockets
        SLOT_POCKET_LAST = 14,
        SLOT_BOX = 15, // 15..24: the chest slots of the current page
        SLOT_BOX_LAST = 24,
        SLOT_PAGE_PREV = 25, // the arrow buttons
        SLOT_PAGE_NEXT = 26,
        SLOT_BACK = 27, // the square button
    };

    enum {
        BOX_SLOT_NUM = 10,
        PAGE_NUM = 16,
        PAGE_BTN_NUM = 2,
        POCKET_NUM = dMenuItem::Window_c::POCKET_COUNT,
        TUTORIAL_ITEM_BOX = 2, // the dObjTutorial_c::request kind
        POCKET_TAIL_UP_FIRST =
            9, // from this pocket on the balloon tail points up
    };

    // Arrow button animations (mPageBtnAnm).
    enum PAGE_BTN_ANM_e {
        PAGE_BTN_ANM_IN,
        PAGE_BTN_ANM_OUT,
        PAGE_BTN_ANM_PUSH,
        PAGE_BTN_ANM_CAN_PUSH,
        PAGE_BTN_ANM_NO_PUSH,
    };

    // Square button animations (mBackBtnAnm).
    enum BACK_BTN_ANM_e {
        BACK_BTN_ANM_IN,
        BACK_BTN_ANM_OUT,
        BACK_BTN_ANM_PUSH,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,        // the chest layout needs a calc
        FLAG_HOLD_ITEM,   // an item is held (follows the cursor)
        FLAG_RETURN_ITEM, // a released item flies back to its slot
        FLAG_PAGE_TURN,   // the close animation turns the page (else it quits)
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,   // waits for the pushed button to finish
        SUB_HOLD,   // an item is held
        SUB_RETURN, // the released item flies back
        SUB_NUM,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_PAGE = 0x85,  // "<page>/<pages>"
        MSG_CLOSE = 0xEB, // "Close", the label of the square button
    };

    static bool isPocket(int slot) {
        return slot >= SLOT_POCKET && slot <= SLOT_POCKET_LAST;
    }
    static bool isBox(int slot) {
        return slot >= SLOT_BOX && slot <= SLOT_BOX_LAST;
    }
    static bool isPageBtn(int slot) {
        return slot >= SLOT_PAGE_PREV && slot <= SLOT_PAGE_NEXT;
    }

    virtual int create();                                      // .text:0x268
    virtual int doDelete();                                    // .text:0x388
    virtual int stateAnm();                                    // .text:0x400
    virtual int stateMain();                                   // .text:0x3DC
    virtual int stateClose();                                  // .text:0x444
    virtual int execute();                                     // .text:0x4F4
    virtual int draw();                                        // .text:0x5A0
    void subStateMain();                                       // .text:0x624
    void subStatePush();                                       // .text:0x730
    void subStateHold();                                       // .text:0x76C
    void subStateReturn();                                     // .text:0x810
    void executeSubState();                                    // .text:0x894
    void setSelect(int slot, BOOL select);                     // .text:0x8CC
    int hitSlot(bool setOffset);                               // .text:0xA1C
    int hitItem();                                             // .text:0xB64
    int hitDropSlot();                                         // .text:0xC04
    void checkPadMove();                                       // .text:0xCA8
    void pushCursor();                                         // .text:0xD2C
    BOOL isPushEnd();                                          // .text:0xDDC
    void finishPush();                                         // .text:0xE2C
    void startClose();                                         // .text:0xEF8
    void startPageTurn();                                      // .text:0xF68
    void setPage(int page);                                    // .text:0xFD8
    void initAnms();                                           // .text:0x1100
    void calcAnms();                                           // .text:0x1220
    void createLayouts();                                      // .text:0x12AC
    static int positionIcon(nw4r::lyt::Pane *pane, void *arg); // .text:0x1338
    void calcLayouts();                                        // .text:0x13B4
    void drawIcons();                                          // .text:0x1420
    void refreshBox();                                         // .text:0x146C
    const mVec2_c &getSlotPos(int slot, float dx, float dy);   // .text:0x14E0
    bool isLocked(int slot);                                   // .text:0x166C
    void setLocked(int slot, BOOL locked);                     // .text:0x16B8
    void updateLocks();                                        // .text:0x1738
    u16 setItem(int slot, dItem::Item item);                   // .text:0x17D4
    u16 dropOnSlot(int slot, dItem::Item item);                // .text:0x18C8
    u16 takeItem(int slot);                                    // .text:0x1964
    u16 getItem(int slot);                                     // .text:0x1994
    bool grabItem();                                           // .text:0x1A38
    void calcHeld();                                           // .text:0x1AE4
    void dropItem();                                           // .text:0x1B4C
    void getSlotName(dScript::Word_c *word, int slot);         // .text:0x1C54
    void showSlotTutorial(int slot);                           // .text:0x1D28
    void hideTutorial();                                       // .text:0x1E24
    bool isEnabled(int slot);                                  // .text:0x1E28
    bool canPrev();                                            // .text:0x1E50
    bool canNext();                                            // .text:0x1E58
    void setPageBtnAnms();                                     // .text:0x1E60
    int findNearestPocket();                                   // .text:0x1EE0
    int findNearestBox();                                      // .text:0x1FC4
    int findReturnSlot();                                      // .text:0x20A8

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x258C
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x2584
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x2580
    virtual void deleteReady() {
    } // .text:0x257C
    virtual bool createHeap() {
        return true;
    } // .text:0x2574

    // Implicit destructor: .text:0x2170.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx_c mPageBtnAnm[PAGE_BTN_NUM];
    /* 0x2B8 */ m2d::AnmEx_c mBackBtnAnm;
    /* 0x2F4 */ m2d::Anm_c
        mColorAnm; // frmColorPtrn: the frame colour of the page
    /* 0x318 */ dMenuItem::Window_c mItemWin;
    /* 0x2C18 */ dMenuItem::Icon_c mBoxIcons[BOX_SLOT_NUM];
    /* 0x43D8 */ m2d::Anm_c mHoleAnm[BOX_SLOT_NUM]; // holeColorPtrn
    /* 0x4540 */ int mCursor;
    /* 0x4544 */ int mHeldFrom; // the slot the held item came from
    /* 0x4548 */ dItem::Item mHeldItem;
    /* 0x454C */ int mPage;
    /* 0x4550 */ int mNextPage;
    /* 0x4554 */ u16 mLocked; // bit per pocket: can't go into the chest
}; // size 0x4558

#endif
