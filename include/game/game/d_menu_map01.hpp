#ifndef GAME_D_MENU_MAP01_HPP
#define GAME_D_MENU_MAP01_HPP

#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_map.hpp>

// The town map Rover shows the new player on the bus (REL d_menu_map01NP,
// profile MENU_MAP01; menu 0x35, dMenuMng_c::MENU_PREVIEW_MAP): the town map
// menu (dMenuMap_c) opened on the facility list with the town hall selected,
// and a single "OK!" button to close it. RTTI confirms the global class name;
// everything else is inferred.
class dMenuMap01_c : public dMenuMap_c {
  public:
    // mCursor: the dMenuBtn_c buttons follow the dMenuMap_c slots.
    enum {
        SLOT_BTN = SLOT_TAB_LAST + 1,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag, after the dMenuMap_c ones.
    enum {
        FLAG_BTN_POS = 8, // the button bar was placed by stateAnm
    };

    // mSubState.
    enum SUB_STATE_e {
        SUB_MAIN,  // wait for the button
        SUB_CLOSE, // close the window
        SUB_NUM
    };

    // Message of SYS_2D_menu.bmg.
    enum {
        MSG_OK = 0xEC, // "OK!"
    };

    virtual int create();    // .text:0x268
    virtual int doDelete();  // .text:0x320
    virtual int stateMain(); // .text:0x368
    virtual int stateAnm();  // .text:0x3A8
    virtual int execute();   // .text:0x404
    virtual int draw();      // .text:0x448

    void subMain();         // .text:0x4C0
    void subClose();        // .text:0x5B4
    void executeSubState(); // .text:0x60C
    void preMain();         // .text:0x644: empty
    void postMain();        // .text:0x648: empty
    void initBtn();         // .text:0x64C
    int hitCheckBtn();      // .text:0x6A8

    // Implicit destructor: .text:0x6E4.

    /* 0x1C04 */ dMenuBtn_c mBtn;
}; // size 0x2148

#endif
