#ifndef GAME_D_MENU_ITEM01_HPP
#define GAME_D_MENU_ITEM01_HPP

#include <game/game/d_menu_itemxx.hpp>

// The pocket picker (REL d_menu_item01NP, profile MENU_ITEM01): the pocket
// window of dMenuItemXX_c where the cursor points at a pocket slot. A on a
// filled pocket opens an answer window (dMenuItemXX_c::mSelect) that only
// offers "Quit" in the retail build; the code handlers for more answers are
// still there: an answer can open a second window below (mSubSelect) or hand
// the slot back through dMenuMng_c::setSelSlot. RTTI confirms the global class
// name dMenuItem01_c; everything else is inferred.
class dMenuItem01_c : public dMenuItemXX_c {
  public:
    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,       // the cursor moves over the pockets
        SUB_SELECT,     // the first answer window is open
        SUB_SUB_OPEN,   // the second answer window opens
        SUB_SUB_SELECT, // the second answer window is open
        SUB_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag, after dMenuItemXX_c's.
    enum {
        FLAG_ANSWERED = 11, // an answer of the first window was taken
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 2,
    };

    // Answer codes (BaseComp_c::mCode) of the answer windows.
    enum {
        ANSWER_QUIT = 0,     // the only answer retail adds
        ANSWER_CONFIRM = 1,  // in the second window: keep the first one open
        ANSWER_OPEN_SUB = 2, // in the first window: opens the second one
        ANSWER_PICK = 3,     // in the first window: returns the slot
    };

    virtual int create();    // .text:0x250
    virtual int doDelete();  // .text:0x318
    virtual int stateMain(); // .text:0x31C
    virtual int draw();      // .text:0x798

    void subStateMain();        // .text:0x35C
    void subStateSelect();      // .text:0x478
    void subStateSubOpen();     // .text:0x5CC
    void subStateSubSelect();   // .text:0x658
    void pick();                // .text:0x6F0
    void cancel();              // .text:0x734
    void onAnswerSub();         // .text:0x738
    void executeSubState();     // .text:0x73C
    void updateReturn();        // .text:0x774
    void updateExtra();         // .text:0x794
    bool setupSelect(int slot); // .text:0x79C
    void backToMain();          // .text:0x848

    virtual int state2() {
        return SUCCEEDED;
    }
    virtual void postDraw(MAIN_STATE_e) {
    }
    virtual void deleteReady() {
    }
    virtual bool createHeap() {
        return true;
    }

    // Implicit destructor: .text:0x89C.

}; // size 0x502C (the 4 bytes at 0x5028 belong to dMenuItemXX_c)

#endif
