#ifndef GAME_D_MENU_POST00_HPP
#define GAME_D_MENU_POST00_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_ltr.hpp>
#include <game/game/d_select.hpp>

// The mailbox menu (REL d_menu_post00NP, profile MENU_POST00): the ten letters
// of the player's house mailbox (dPlayerPostBox_c, Layout/post.arc) next to the
// ten letters of the player (dMenuLtr::Window_c, slots 0..9). Opened by the
// mailbox (MENU_MAILBOX) it first moves every letter of the mailbox into the
// free letter slots; after reading a letter (MENU_MAILBOX_AFTER_READ) it starts
// with the player pointing. A letter is dragged between the slots, or a press
// on it opens the read / toss / quit menu. The page buttons (16 pages, hidden
// in the layout) only close the menu. RTTI confirms the global class name;
// everything else is inferred (the design box of d_menu_desi02 is the model).
class dMenuPost00_c : public dMenuBase_c {
  public:
    // Slots: what the cursor points at (mCursor, mHeldFrom, mSelectSlot).
    enum SLOT_e {
        SLOT_NONE = -1,
        SLOT_LETTER = 0, // 0..9: the letters of the player
        SLOT_LETTER_LAST = 9,
        SLOT_BOX = 10, // 10..19: the letters of the mailbox
        SLOT_BOX_LAST = 19,
        SLOT_NEXT_PAGE = 20,
        SLOT_PREV_PAGE = 21,
        SLOT_BACK = 22,
    };

    enum {
        LETTER_NUM = 10,    // letters in the mailbox
        LETTER_ROW_NUM = 5, // letters per row of the letter window
        PAGE_NUM = 16,
        PAGE_ANM_NUM = 5, // animations of a page button
        BACK_ANM_NUM = 3, // ... of the close button: in, out, push
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,        // draw the layouts
        FLAG_CALC,        // the layout needs a calc
        FLAG_FOLLOW,      // the held letter follows the cursor
        FLAG_MOVE,        // the held letter flies to a slot
        FLAG_OPEN_LETTER, // close into the letter reader, not the world
    };

    // mSubState.
    enum SUB_STATE_e {
        SUB_MAIN,   // point at something
        SUB_PUSH,   // a button was pushed: wait for its animation
        SUB_SELECT, // the read / toss / quit menu
        SUB_PRESS,  // a letter is pressed: released it opens the menu, held it
                    // is picked up
        SUB_DRAG,   // a letter is dragged
        SUB_MOVE,   // the held letter flies to a slot
        SUB_TOSS,   // the tossed letter vanishes
        SUB_TAKE,   // take the next letter of the mailbox
        SUB_TAKE_MOVE, // ... it flies into a free letter slot
        SUB_NUM
    };

    // The answers of the read / toss / quit menu (dSelect::BaseComp_c::mCode).
    enum ANSWER_e {
        ANSWER_READ,
        ANSWER_TOSS,
        ANSWER_QUIT,
    };

    // The animations of the buttons (mPageAnm, mBackAnm).
    enum BTN_ANM_e {
        BTN_ANM_IN,
        BTN_ANM_OUT,
        BTN_ANM_PUSH,
        BTN_ANM_ENABLED,
        BTN_ANM_DISABLED,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x02,
        MSG_READ = 0x69,
        MSG_TOSS = 0x6A,
        MSG_PAGE_NUMBER = 0x85, // "page / pages"
        MSG_CLOSE = 0xEB,
    };

    static bool isLetter(int slot) {
        return slot >= SLOT_LETTER && slot <= SLOT_LETTER_LAST;
    }
    static bool isBox(int slot) {
        return slot >= SLOT_BOX && slot <= SLOT_BOX_LAST;
    }
    static bool isPageButton(int slot) {
        return slot >= SLOT_NEXT_PAGE && slot <= SLOT_PREV_PAGE;
    }

    virtual int create();     // .text:0x23C
    virtual int doDelete();   // .text:0x3BC
    virtual int stateMain();  // .text:0x428
    virtual int stateAnm();   // .text:0x44C
    virtual int stateClose(); // .text:0x490
    virtual int execute();    // .text:0x4EC
    virtual int draw();       // .text:0x5A8

    void subMain();                                            // .text:0x63C
    void subPush();                                            // .text:0x710
    void subSelect();                                          // .text:0x74C
    void subPress();                                           // .text:0x7A0
    void subDrag();                                            // .text:0x870
    void subMove();                                            // .text:0x914
    void subToss();                                            // .text:0xAA8
    void subTake();                                            // .text:0xB70
    void subTakeMove();                                        // .text:0xCA8
    void executeSubState();                                    // .text:0xD00
    void setHighlight(int slot, BOOL on);                      // .text:0xD38
    int hitSlot(bool setOffset);                               // .text:0xE68
    int hitCheck();                                            // .text:0xF34
    int hitCheckDrop();                                        // .text:0xFD0
    void pressButton();                                        // .text:0x10E4
    BOOL isPushEnd();                                          // .text:0x11F8
    void finishPush();                                         // .text:0x1248
    void startClose();                                         // .text:0x1290
    void readLetter();                                         // .text:0x12F8
    void setPage(int page);                                    // .text:0x134C
    void initAnm();                                            // .text:0x13F8
    void calcAnm();                                            // .text:0x14C8
    BOOL isButtonEnabled(int slot);                            // .text:0x154C
    BOOL isNextPageEnabled();                                  // .text:0x1574
    BOOL isPrevPageEnabled();                                  // .text:0x1590
    void updatePageButtons();                                  // .text:0x15A4
    void initIcons();                                          // .text:0x1624
    static int positionIcon(nw4r::lyt::Pane *pane, void *arg); // .text:0x1698
    void calcLayouts();                                        // .text:0x1714
    void drawIcons();                                          // .text:0x1780
    void initMails();                                          // .text:0x17CC
    const mVec2_c &getSlotPos(int slot, float dx, float dy);   // .text:0x1838
    dMail_c *getMail(int slot);                                // .text:0x19A4
    void swapMail(int slot, dMail_c *mail, BOOL updateIcon);   // .text:0x1A38
    void removeMail(int slot, dMail_c *removed);               // .text:0x1B7C
    BOOL isSlotEmpty(int slot);                                // .text:0x1C00
    int findBoxMail();                                         // .text:0x1C34
    int findFreeLetter();                                      // .text:0x1C94
    void markRead(int slot);                                   // .text:0x1CF4
    BOOL isLocked(int slot);                                   // .text:0x1E60
    void unlockAll();                                          // .text:0x1EAC
    void showSlotTutorial(int slot);                           // .text:0x1EB8
    void hideTutorial();                                       // .text:0x1FC4
    void openSelect(int slot);                                 // .text:0x1FC8
    void buildSelect(int slot);                                // .text:0x21A4
    void onSelectEnd();                                        // .text:0x2278
    BOOL pickUp();                                             // .text:0x2300
    void calcHeld();                                           // .text:0x23BC
    void dropHeld();                                           // .text:0x2424
    int findNearestFree();                                     // .text:0x263C
    int findDropFree();                                        // .text:0x271C

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

    // Implicit destructor: .text:0x279C.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx_c mPageAnm[2]; // the next / previous page buttons
    /* 0x2B8 */ m2d::AnmEx_c mBackAnm;    // the close button
    /* 0x2F4 */ m2d::Anm_c mScrollAnm;    // the scroll icon
    /* 0x318 */ dMenuLtr::Icon_c
        mIcons[LETTER_NUM]; // the letters of the mailbox
    /* 0xED0 */ dMenuLtr::Window_c mLtrWin;
    /* 0x21B0 */ dSelect::MComp_c mSelect;
    /* 0x2B5C */ int mAnswer;     // ANSWER_e
    /* 0x2B60 */ int mSelectSlot; // the slot the answer menu is for
    /* 0x2B64 */ int
        mHeldFrom; // the slot the held letter goes to when it is released
    /* 0x2B68 */ int mCursor; // SLOT_e under the cursor
    /* 0x2B6C */ int mPage;
    /* 0x2B70 */ int mHoldCount; // frames the letter was pressed
    /* 0x2B74 */ u16 mLockMask;  // letters that can't be picked up
}; // size 0x2B78

#endif
