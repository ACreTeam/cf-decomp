#ifndef GAME_D_MENU_MAP00_HPP
#define GAME_D_MENU_MAP00_HPP

#include <game/game/d_menu_map.hpp>

// The town map menu opened from the map tab (REL d_menu_map00NP, profile
// MENU_MAP00): dMenuMap_c as is, plus the input handling of the cursor, the
// dAcUitag tabs and the pad. RTTI confirms the class name dMenuMap00_c;
// everything else is inferred.
class dMenuMap00_c : public dMenuMap_c {
  public:
    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,        // the pointer picks a slot
        SUB_PUSH,        // waits for the pushed button animation
        SUB_SCROLL,      // keeps scrolling while the scroll button is held
        SUB_ICON_SCROLL, // scrolls the list to the selected icon
        SUB_NUM,
    };

    virtual int create();                 // .text:0x260
    virtual int doDelete();               // .text:0x2DC
    virtual int stateMain();              // .text:0x2E0
    void subStateMain();                  // .text:0x328
    void subStatePush();                  // .text:0x50C
    void subStateScroll();                // .text:0x5F8
    void subStateIconScroll();            // .text:0x708
    BOOL closeMenu();                     // .text:0x780
    BOOL checkCloseRequest();             // .text:0x7B8
    void executeSubState();               // .text:0x8A4
    int getTabAt(const mVec2_c *pos);     // .text:0x8DC
    BOOL selectTab(int slot, BOOL delay); // .text:0x940
    void startClose();                    // .text:0x994
    void checkPadMove();                  // .text:0x9B0

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0xC88
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0xC80
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0xC7C
    virtual void deleteReady() {
    } // .text:0xC78
    virtual bool createHeap() {
        return true;
    } // .text:0xC70

    // Implicit destructor: .text:0xB0C.
};

#endif
