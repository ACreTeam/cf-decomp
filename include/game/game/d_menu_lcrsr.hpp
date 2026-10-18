#ifndef GAME_D_MENU_LCRSR_HPP
#define GAME_D_MENU_LCRSR_HPP

#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>

// The letter cursor of the letter menus (REL d_menu_lcrsrNP; used by
// d_menu_write and d_menu_notice01): two push buttons from Layout/paper.arc
// ("ltr_cursor_00"), modelled on dMenuBtn_c. RTTI confirms the class name;
// everything else is inferred.
class dMenuLcrsr_c {
  public:
    enum {
        BTN_NONE = -1,
        BTN_NUM = 2,
    };

    // Button animations (mBtnAnm).
    enum BTN_ANM_e {
        BTN_ANM_PUSH,
        BTN_ANM_NO_PUSH,
        BTN_ANM_CAN_PUSH,
        BTN_ANM_NUM,
    };

    // Focus animations (mFocusAnm).
    enum FOCUS_ANM_e {
        FOCUS_ANM_OUT,
        FOCUS_ANM_IN,
        FOCUS_ANM_NUM,
    };

    enum FLAG_e {
        FLAG_DRAW = 0x1,
        FLAG_CALC = 0x2,
        FLAG_HIDDEN = 0x4,    // << button
        FLAG_DISABLED = 0x10, // << button
        FLAG_PUSH = 0x1000,
        FLAG_ANM = 0x2000,
    };

    dMenuLcrsr_c();                                             // .text:0x5C
    virtual ~dMenuLcrsr_c();                                    // .text:0x124
    BOOL load();                                                // .text:0x1E4
    BOOL create();                                              // .text:0x230
    BOOL remove();                                              // .text:0x328
    BOOL execute();                                             // .text:0x330
    BOOL draw();                                                // .text:0x388
    void setPos(m2d::SimpleEx_c *layout, const char *paneName); // .text:0x3BC
    int hitCheck(const mVec2_c *pos);                           // .text:0x404
    void setSelect(int btn, BOOL select);                       // .text:0x498
    BOOL push(int btn);                                         // .text:0x4B4
    bool isPushing();                                           // .text:0x520
    BOOL startAnm(int anm);                                     // .text:0x52C
    bool isAnm();                                               // .text:0x5A0
    mVec3_c getPos(); // .text:0x5AC: the screen position of the layout
    void calcAnm();   // .text:0x5F0
    void setDisable(int btn, BOOL disable); // .text:0x6E4

    /* 0x004 */ m2d::SimpleEx_c mLyt;
    /* 0x09C */ m2d::AnmEx2_c mFocusAnm[BTN_NUM];
    /* 0x124 */ m2d::AnmEx2_c _124[BTN_NUM]; // never initialised or played
    /* 0x1AC */ m2d::AnmEx2_c mBtnAnm[BTN_NUM];
    /* 0x234 */ m2d::Anm_c mAnm; // a dMenuBase_c window animation (startAnm)
    /* 0x258 */ m2d::ResAccLoader_c mRes;
    /* 0x32C */ u32 mFlags;
    /* 0x330 */ int mPushedBtn;
}; // size 0x334

#endif
