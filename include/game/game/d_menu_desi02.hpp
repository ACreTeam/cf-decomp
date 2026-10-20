#ifndef GAME_D_MENU_DESI02_HPP
#define GAME_D_MENU_DESI02_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_desi.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_select.hpp>

class dDesign_c;

// The design box menu (REL d_menu_desi02NP, profile MENU_DESI02): the 16 pages
// of four stored designs (dSaveDesignBox_c, Layout/leave_design.arc) next to
// the eight designs of the player (dMenuDesi::Desi_c). A design is dragged
// between the box and the player's designs; dropping it on a used slot asks
// swap or copy. RTTI confirms the global class name; everything else is
// inferred.
class dMenuDesi02_c : public dMenuBase_c {
  public:
    // Slots: what the cursor points at (mCursor, mHeld, mTarget).
    enum SLOT_e {
        SLOT_NONE = -1,
        SLOT_BOX = 0, // 0..3: the stored designs of the page
        SLOT_BOX_LAST = 3,
        SLOT_PREV_PAGE = 4,
        SLOT_NEXT_PAGE = 5,
        SLOT_BACK = 6,
        SLOT_DESIGN = 7, // 7..14: the eight designs of the player
        SLOT_DESIGN_LAST = 14,
    };

    enum {
        BOX_NUM = 4,   // designs per page
        PAGE_NUM = 16, // pages of the design box
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,      // draw the design window
        FLAG_CALC,      // the layout needs a calc
        FLAG_DESI_CALC, // the designs of the player changed
        FLAG_PAGE_TURN, // the window closes for a page turn, not to leave
        FLAG_FOLLOW,    // the held design follows the cursor
        FLAG_MOVE,      // the held design flies back
        FLAG_PLACED,    // the held design is drawn at a fixed position
        FLAG_DIALOG,    // draw the dialog box
        FLAG_CHANGED,   // a design was changed: the result is 1
    };

    // mSubState.
    enum SUB_STATE_e {
        SUB_MAIN,        // point at something
        SUB_PUSH,        // a button was pushed: wait for its animation
        SUB_DRAG,        // a design is dragged
        SUB_MOVE,        // the held design flies back
        SUB_DIALOG,      // the dialog box opens
        SUB_DIALOG_PUSH, // a dialog button was pushed
        SUB_DIALOG_END,  // the dialog box closes
        SUB_SELECT,      // the swap / copy answer menu
        SUB_NUM
    };

    // The answers of the swap / copy menu (dSelect::BaseComp_c::mCode).
    enum ANSWER_e {
        ANSWER_SWAP,
        ANSWER_COPY,
        ANSWER_CANCEL,
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
        MSG_NEVER_MIND = 0x0F,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_PAGE_NUMBER = 0x85, // "page / pages"
        MSG_OVERWRITE = 0xD3,
        MSG_COPY = 0xD9,
        MSG_SWAP = 0xDA,
        MSG_CLOSE = 0xEB,
    };

    static bool isBox(int slot) {
        return slot >= SLOT_BOX && slot <= SLOT_BOX_LAST;
    }
    static bool isDesign(int slot) {
        return slot >= SLOT_DESIGN && slot <= SLOT_DESIGN_LAST;
    }
    static bool isPageButton(int slot) {
        return slot >= SLOT_PREV_PAGE && slot <= SLOT_NEXT_PAGE;
    }

    virtual int create();     // .text:0x30C
    virtual int doDelete();   // .text:0x460
    virtual int stateMain();  // .text:0x4E4
    virtual int stateAnm();   // .text:0x508
    virtual int stateClose(); // .text:0x50C
    virtual int execute();    // .text:0x5BC
    virtual int draw();       // .text:0x68C

    void subMain();                                    // .text:0x740
    void subPush();                                    // .text:0x890
    void subDrag();                                    // .text:0x8CC
    void subMove();                                    // .text:0xA88
    void subDialog();                                  // .text:0xAD8
    void subDialogPush();                              // .text:0xB38
    void subDialogEnd();                               // .text:0xB9C
    void subSelect();                                  // .text:0xC14
    void executeSubState();                            // .text:0xCAC
    void setHighlight(int slot, BOOL on);              // .text:0xCE4
    int hitCheck(BOOL hover);                          // .text:0xE34
    void checkPadMove();                               // .text:0xFEC
    void pressButton();                                // .text:0x1070
    BOOL isPushEnd();                                  // .text:0x1120
    void finishPush();                                 // .text:0x1170
    void startClose();                                 // .text:0x1258
    void startPageTurn();                              // .text:0x12C8
    void setPage(int page);                            // .text:0x1338
    void initAnm();                                    // .text:0x13E4
    void calcAnm();                                    // .text:0x14A0
    BOOL isButtonEnabled(int slot);                    // .text:0x1540
    BOOL isPrevPageEnabled();                          // .text:0x1568
    BOOL isNextPageEnabled();                          // .text:0x1570
    void updatePageButtons();                          // .text:0x1578
    dDesign_c *getDesign(int slot);                    // .text:0x15F8
    void setDesign(int slot, dDesign_c *design);       // .text:0x1698
    void removeDesign(int slot);                       // .text:0x17B0
    void setDesignVisible(int slot, BOOL visible);     // .text:0x1818
    void updateBoxDesigns();                           // .text:0x1960
    void initPanes();                                  // .text:0x1A48
    BOOL setBoxHighlight(int slot, bool on);           // .text:0x1AEC
    mVec2_c getSlotPos(int slot, f32 x, f32 y);        // .text:0x1BC4
    BOOL dropOn(int from, int to);                     // .text:0x1D08
    void copyDesign(int from, int to);                 // .text:0x1DE0
    void swapOrder(int a, int b);                      // .text:0x1E7C
    void swapDesigns(int a, int b);                    // .text:0x1EE0
    void getSlotName(dScript::Word_c *word, int slot); // .text:0x21D4
    void showSlotTutorial(int slot);                   // .text:0x223C
    void hideTutorial();                               // .text:0x2338
    bool calcPickup();                                 // .text:0x233C
    void pickUpDesign(int slot);                       // .text:0x23B4
    void dropHeld(int slot);                           // .text:0x244C
    void openDialog(u16 msg);                          // .text:0x24D4
    void buildSwapSelect();                            // .text:0x2590
    void openSelect(int target);                       // .text:0x2624

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

    // Implicit destructor: .text:0x27F8.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx_c mPageAnm[2]; // the previous / next page buttons
    /* 0x2B8 */ m2d::AnmEx_c mBackAnm;    // the close button
    /* 0x2F4 */ int mCursor;              // SLOT_e under the cursor
    /* 0x2F8 */ int mPage;
    /* 0x2FC */ int mNextPage;
    /* 0x300 */ dMenuDesi::Desi_c mDesi;
    /* 0x6B0 */ dMenuSlctB_c mDialog;
    /* 0xDA0 */ nw4r::lyt::Pane *mBoxPanes[BOX_NUM];
    /* 0xDB0 */ int mHighlight; // the highlighted box slot
    /* 0xDB4 */ int mHeld;      // the slot being dragged
    /* 0xDB8 */ dSelect::MComp_c mSelect;
    /* 0x1764 */ int mTarget; // the slot the answer menu is for
    /* 0x1768 */ int mAnswer;
}; // size 0x176C

#endif
