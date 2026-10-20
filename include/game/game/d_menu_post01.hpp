#ifndef GAME_D_MENU_POST01_HPP
#define GAME_D_MENU_POST01_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_ltr.hpp>
#include <game/game/d_select.hpp>

// The post office's letter storage (REL d_menu_post01NP, profile MENU_POST01):
// the 16 pages of ten stored letters (dSaveMailBox_c, Layout/post.arc) next to
// the ten letters of the player (dMenuLtr::Window_c, slots 0..9). Opened by
// Pelly (dAcNpc_c::recept_c::reqMenu1D); after reading a stored letter it opens
// again on the same page (MENU_POST_LETTERS_AFTER_READ). A letter is dragged
// between the slots, or a press on it opens the read / toss / quit menu. The
// page buttons turn the page. RTTI confirms the global class name; everything
// else is inferred (the design box of d_menu_desi02 is the model).
class dMenuPost01_c : public dMenuBase_c {
  public:
    // Slots: what the cursor points at (mCursor, mHeldFrom, mSelectSlot).
    enum SLOT_e {
        SLOT_NONE = -1,
        SLOT_LETTER = 0, // 0..9: the letters of the player
        SLOT_LETTER_LAST = 9,
        SLOT_BOX = 10, // 10..19: the stored letters of the page
        SLOT_BOX_LAST = 19,
        SLOT_PREV_PAGE = 20,
        SLOT_NEXT_PAGE = 21,
        SLOT_BACK = 22,
    };

    enum {
        BOX_NUM = 10,        // stored letters per page
        PAGE_NUM = 16,       // pages of the storage
        LETTERS_PER_ROW = 5, // the letters of the player: two rows of five
        PAGE_BTN_NUM = 2,    // the previous / next page buttons
        PAGE_ANM_NUM = 5,    // BTN_ANM_IN .. BTN_ANM_DISABLED
        BACK_ANM_NUM = 3,    // BTN_ANM_IN .. BTN_ANM_PUSH
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,        // draw the layouts
        FLAG_CALC,        // the layout needs a calc
        FLAG_FOLLOW,      // the held letter follows the cursor
        FLAG_MOVE,        // the held letter flies back to its slot
        FLAG_OPEN_LETTER, // close into the letter reader, not the world
        FLAG_PAGE_TURN,   // the window closes for a page turn, not to leave
        FLAG_CHANGED,     // a letter changed its slot: the result is 1
    };

    // mSubState.
    enum SUB_STATE_e {
        SUB_MAIN,   // point at something
        SUB_PUSH,   // a button was pushed: wait for its animation
        SUB_SELECT, // the read / toss / quit menu
        SUB_PRESS,  // a letter is pressed: released it opens the menu, held it
                    // is picked up
        SUB_DRAG,   // a letter is dragged
        SUB_MOVE,   // the held letter flies back to its slot
        SUB_TOSS,   // the tossed letter vanishes
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

    // The low bits of the post flags (dMenuBase_c::setPostFlags) keep the page
    // across the letter reader; FLAG_CHANGED goes in the bit above them.
    enum {
        POST_PAGE_MASK = 0x3F,
        POST_CHANGED = 0x80,
    };

    static bool isLetter(int slot) {
        return slot >= SLOT_LETTER && slot <= SLOT_LETTER_LAST;
    }
    static bool isBox(int slot) {
        return slot >= SLOT_BOX && slot <= SLOT_BOX_LAST;
    }
    static bool isPageButton(int slot) {
        return slot >= SLOT_PREV_PAGE && slot <= SLOT_NEXT_PAGE;
    }

    virtual int create();     // .text:0x23C
    virtual int doDelete();   // .text:0x3D4
    virtual int stateMain();  // .text:0x440
    virtual int stateAnm();   // .text:0x464
    virtual int stateClose(); // .text:0x4A8
    virtual int execute();    // .text:0x584
    virtual int draw();       // .text:0x640

    void subMain();                                            // .text:0x6D4
    void subPush();                                            // .text:0x7B4
    void subSelect();                                          // .text:0x7F0
    void subPress();                                           // .text:0x844
    void subDrag();                                            // .text:0x914
    void subMove();                                            // .text:0x9B8
    void subToss();                                            // .text:0xB4C
    void executeSubState();                                    // .text:0xC14
    void setHighlight(int slot, BOOL on);                      // .text:0xC4C
    int hitSlot(bool setOffset);                               // .text:0xD9C
    int hitCheck();                                            // .text:0xEE4
    int hitCheckDrop();                                        // .text:0xF80
    void checkPadMove();                                       // .text:0x1024
    void pressButton();                                        // .text:0x10A8
    BOOL isPushEnd();                                          // .text:0x11BC
    void finishPush();                                         // .text:0x120C
    void startClose();                                         // .text:0x12F4
    void readLetter();                                         // .text:0x1368
    void startPageTurn();                                      // .text:0x13E8
    void setPage(int page);                                    // .text:0x1458
    void initAnm();                                            // .text:0x1504
    void calcAnm();                                            // .text:0x15D4
    BOOL isButtonEnabled(int slot);                            // .text:0x1658
    BOOL isPrevPageEnabled();                                  // .text:0x1680
    BOOL isNextPageEnabled();                                  // .text:0x1688
    void updatePageButtons();                                  // .text:0x1690
    void initIcons();                                          // .text:0x1710
    static int positionIcon(nw4r::lyt::Pane *pane, void *arg); // .text:0x1784
    void calcLayouts();                                        // .text:0x1800
    void drawIcons();                                          // .text:0x186C
    void initMails();                                          // .text:0x18B8
    const mVec2_c &getSlotPos(int slot, float dx, float dy);   // .text:0x1924
    dMail_c *getMail(int slot);                                // .text:0x1A90
    void swapMail(int slot, dMail_c *mail, BOOL updateIcon);   // .text:0x1B30
    void removeMail(int slot, dMail_c *removed);               // .text:0x1C7C
    BOOL isSlotEmpty(int slot);                                // .text:0x1D00
    void markRead(int slot);                                   // .text:0x1D34
    BOOL isLocked(int slot);                                   // .text:0x1EA0
    void unlockAll();                                          // .text:0x1EEC
    void showSlotTutorial(int slot);                           // .text:0x1EF8
    void hideTutorial();                                       // .text:0x2004
    void openSelect(int slot);                                 // .text:0x2008
    void buildSelect(int slot);                                // .text:0x21E4
    void onSelectEnd();                                        // .text:0x22B8
    BOOL pickUp();                                             // .text:0x2340
    void calcHeld();                                           // .text:0x23FC
    void dropHeld();                                           // .text:0x2464
    int findFreeLetter();                                      // .text:0x26D0
    int findFreeBox();                                         // .text:0x27B0
    int findDropFree();                                        // .text:0x2884

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

    // Implicit destructor: .text:0x294C.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx_c
        mPageAnm[PAGE_BTN_NUM];        // the previous / next page buttons
    /* 0x2B8 */ m2d::AnmEx_c mBackAnm; // the close button
    /* 0x2F4 */ m2d::Anm_c mScrollAnm; // the scroll icon
    /* 0x318 */ dMenuLtr::Icon_c
        mIcons[BOX_NUM]; // the stored letters of the page
    /* 0xED0 */ dMenuLtr::Window_c mLtrWin;
    /* 0x21B0 */ dSelect::MComp_c mSelect;
    /* 0x2B5C */ int mAnswer;     // ANSWER_e
    /* 0x2B60 */ int mSelectSlot; // the slot the answer menu is for
    /* 0x2B64 */ int mHeldFrom;   // the slot the held letter came from
    /* 0x2B68 */ int mCursor;     // SLOT_e under the cursor
    /* 0x2B6C */ int mPage;
    /* 0x2B70 */ int mNextPage;
    /* 0x2B74 */ int mHoldCount; // frames the letter was pressed
    /* 0x2B78 */ u16 mLockMask;  // letters that can't be picked up
}; // size 0x2B7C

#endif
