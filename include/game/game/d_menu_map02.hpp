#ifndef GAME_D_MENU_MAP02_HPP
#define GAME_D_MENU_MAP02_HPP

#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_map.hpp>

// The bridge map (REL d_menu_map02NP, profile MENU_MAP02): the town map with
// up to three cursors on the blocks where a bridge can be built (the river
// blocks without a bridge). The player picks one (or backs out) and the block
// is handed to dMenuMng_c::setPickedBlock, which Pelly reads back for the
// public works vote. RTTI confirms the global class name; everything else is
// inferred.
class dMenuMap02_c : public dMenuMap_c {
  public:
    // Slots (mCursor), after the base's slots: the button bar and the spots.
    enum {
        SLOT_BTN = SLOT_TAB_LAST + 1,
        SLOT_BTN_LAST = SLOT_BTN + dMenuBtn_c::BTN_NUM - 1,
        SPOT_NUM = 3,
        SLOT_SPOT = SLOT_BTN_LAST + 1,
        SLOT_SPOT_LAST = SLOT_SPOT + SPOT_NUM - 1,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag, after the base's.
    enum {
        FLAG_SPOT_POS = 8, // place the spots on the map
        FLAG_SPOT_CALC,    // the spots changed
    };

    enum SUB_STATE_e { SUB_MAIN, SUB_PUSH };

    // The button bar's buttons (dMenuBtn_c).
    enum BTN_e { BTN_UNUSED_0, BTN_UNUSED_1, BTN_BACK };

    // The animations of a spot's cursor (m2d::AnmEx_c indices).
    enum SPOT_ANM_e { SPOT_ANM_IN, SPOT_ANM_OUT, SPOT_ANM_PUSH, SPOT_ANM_NUM };

    enum {
        MSG_BACK = 0x20,
        NO_BLOCK = -1,
        // The map picture covers this many pane units.
        MAP_SIZE = 240,
    };

    static bool isBtn(int slot) {
        return slot >= SLOT_BTN && slot <= SLOT_BTN_LAST;
    }
    static bool isSpot(int slot) {
        return slot >= SLOT_SPOT && slot <= SLOT_SPOT_LAST;
    }

    virtual int create();    // .text:0x290
    virtual int doDelete();  // .text:0x338
    virtual int execute();   // .text:0x41C
    virtual int draw();      // .text:0x460
    virtual int stateAnm();  // .text:0x3C0
    virtual int stateMain(); // .text:0x380

    void subStateMain();               // .text:0x4C0
    void subStatePush();               // .text:0x584
    void executeSubState();            // .text:0x5C0
    void preSubState();                // .text:0x5F8
    void postSubState();               // .text:0x5FC
    void setSelect(int slot, BOOL on); // .text:0x600
    int hitCheckCursor();              // .text:0x6B0
    void push();                       // .text:0x778
    BOOL isPushEnd();                  // .text:0x838
    void onPushEnd();                  // .text:0x8C8
    void startClose();                 // .text:0x98C
    void initButtons();                // .text:0x9CC
    void initSpots();                  // .text:0xA28
    void calcSpots();                  // .text:0xCC4
    void playSePush();                 // .text:0xDF4

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

    // Implicit destructor: .text:0xDFC.

    /* 0x1C04 */ dMenuBtn_c mBtn;
    /* 0x2148 */ m2d::SimpleEx_c mSpotLyt[SPOT_NUM]; // map_cursor_00
    /* 0x2310 */ m2d::AnmEx_c mSpotAnm[SPOT_NUM];    // SPOT_ANM_e
    /* 0x23C4 */ int mSpotBlockX[SPOT_NUM];          // the block of each spot
    /* 0x23D0 */ int mSpotBlockZ[SPOT_NUM];
}; // size 0x23DC

#endif
