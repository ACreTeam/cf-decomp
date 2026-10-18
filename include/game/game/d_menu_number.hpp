#ifndef GAME_D_MENU_NUMBER_HPP
#define GAME_D_MENU_NUMBER_HPP

#include <game/game/d_kbd.hpp>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>

// The number pad of the menus that ask for an amount (REL d_menu_numberNP; used
// by d_menu_number00, d_menu_bank, d_menu_kabu, d_menu_time00 and
// d_menu_friend08): twelve layout keys and the USB keyboard's digits. RTTI
// confirms the class names; everything else is inferred.
class dMenuNumber_c {
  public:
    // The layout keys: a phone-style 3x4 grid (KEY_7 at the top left), then
    // clear and back.
    enum KEY_e {
        KEY_NONE = -1,
        KEY_7,
        KEY_8,
        KEY_9,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_0,
        KEY_CLEAR, // "N_numBtn_10"
        KEY_BACK,  // "N_numBtn_11", hidden in MODE_AMOUNT
        KEY_NUM,
    };

    // create() modes.
    enum MODE_e {
        MODE_AMOUNT, // amounts (bank, turnips, time, number00): no back key
        MODE_FRIEND_CODE, // d_menu_friend08: the back key and the keyboard's
                          // extra digits
    };

    // getKeyDigit results for the two red keys.
    enum {
        DIGIT_CLEAR = -1,
        DIGIT_BACK = -2,
    };

    // Key animations (m2d::AnmEx2_c indices).
    enum KEY_ANM_e {
        KEY_ANM_IN, // under the cursor
        KEY_ANM_OUT,
        KEY_ANM_PUSH,
        KEY_ANM_NUM,
    };

    enum FLAG_e {
        FLAG_CALC = 0x1,
        FLAG_KBD_EXTRA_KEYS =
            0x4, // MODE_FRIEND_CODE: the keyboard's digits 10 and 11 count
        FLAG_KBD_BACK = 0x8, // the last keyboard digit was 11
    };

    // The pad's layout: draws the key under the cursor on top of the others.
    class layout_c : public m2d::SimpleEx_c {
      public:
        // The keys' panes ("N_numBtn_00".."N_numBtn_11") and the window base
        // ("numWinBase_00").
        enum { PANE_NUM = KEY_NUM + 1 };

        virtual void draw(); // .text:0x70C

        /* 0x98 */ int mMode;      // MODE_e
        /* 0x9C */ int mCursorKey; // KEY_e, drawn last
    }; // size 0xA0

    dMenuNumber_c();                   // .text:0x5C
    virtual ~dMenuNumber_c();          // .text:0x148
    BOOL remove();                     // .text:0x1DC
    int execute(BOOL holdPush);        // .text:0x228
    int draw();                        // .text:0x29C
    int hitCheck();                    // .text:0x2C4
    int getKbdKey();                   // .text:0x33C
    BOOL isSendReq();                  // .text:0x3D4
    BOOL isClearReq();                 // .text:0x420
    bool isKbdBack();                  // .text:0x46C
    u8 getSpecialKey();                // .text:0x484
    void push(int key);                // .text:0x4D4
    void playKeySe(int key);           // .text:0x520
    BOOL isPushing();                  // .text:0x578
    void initAnms();                   // .text:0x580
    void calcAnms(BOOL holdPush);      // .text:0x628
    static int getKeyDigit(int key);   // .text:0x6E4: -1 clear, -2 back
    static int getDigitKey(int digit); // .text:0x6F8
    BOOL load();                       // .text:0x7EC
    BOOL create(int mode);             // .text:0x848
    void setPos(m2d::SimpleEx_c *base, const char *paneName); // .text:0x934
    void setCursor(int key, int anm);                         // .text:0x97C

    /* 0x004 */ m2d::ResAccLoader_c mRes;
    /* 0x0D8 */ layout_c mLyt;
    /* 0x178 */ m2d::AnmEx2_c mKeyAnms[KEY_NUM];
    /* 0x4A8 */ dKbd::Mng_c mKbd;
    /* 0x820 */ int mPushedKey;
    /* 0x824 */ u16 mFlags;
}; // size 0x828

#endif
