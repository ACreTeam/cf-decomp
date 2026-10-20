#ifndef GAME_D_MENU_KEYTEST_HPP
#define GAME_D_MENU_KEYTEST_HPP

#include <game/game/d_kbd.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>

// The keyboard test menu (REL d_menu_keytestNP, profile MENU_KEYTEST): a debug
// menu that opens the software keyboard (Layout/keyboard_add.arc) over an
// empty text box (textBox_free_00) and closes again when the typed memo is
// empty. RTTI confirms the class name dMenuKeytest_c; everything else is
// inferred.
class dMenuKeytest_c : public dMenuBase_c {
  public:
    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_PAUSE, // the sub state is not run (never set)
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_OPEN,  // the keyboard is open until the input is sent or cancelled
        SUB_CHECK, // closes the menu when the input is empty
        SUB_IDLE,
        SUB_UNUSED, // no function
        SUB_NUM,
    };

    // The room of the text box for the typed text.
    enum {
        TEXT_SIZE = 0x68,
    };

    virtual int create();    // .text:0xD8
    virtual int doDelete();  // .text:0x1C0
    virtual int stateMain(); // .text:0x218
    virtual int draw();      // .text:0x26C
    void subStateOpen();     // .text:0x2A8
    void subStateCheck();    // .text:0x344
    void subStateIdle();     // .text:0x38C
    void executeSubState();  // .text:0x390
    void calcKeyTest();      // .text:0x3C8 (empty, debug code was stripped)
    void calcKbd();          // .text:0x3CC

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x478
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x470
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x46C
    virtual void deleteReady() {
    } // .text:0x468
    virtual bool createHeap() {
        return true;
    } // .text:0x460

    // Implicit destructor: .text:0x3D4.

    /* 0x0D4 */ dKbd::Mng_c mKbd;
    /* 0x44C */ m2d::ResAccLoader_c mRes;
    /* 0x520 */ m2d::SimpleEx_c mLyt;
}; // size 0x5B8

#endif
