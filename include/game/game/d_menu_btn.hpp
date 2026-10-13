#pragma once

// The menus' button bar dMenuBtn_c (its RTTI name; TU d_menu_btn.cpp, .text 800CEC80..800CF77C): the layout
// cmn_btn_A_00 with up to three buttons (panes N_cmnBtnA_00..02), each with a label, a push animation and an
// in/out animation; button 0 is also pushed with B or -, button 2 with +. Used by most d_menu_*NP menus.
// Every name but dMenuBtn_c is inferred.

#include <types.h>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>

class dMenuBtn_c {
public:
    enum { BTN_NUM = 3 };

    // mBtnAnm animations.
    enum BTN_ANM_e {
        BTN_ANM_NO_PUSH,
        BTN_ANM_CAN_PUSH,
        BTN_ANM_PUSH,
    };

    enum FLAG_e {
        FLAG_DRAW = 0x1,
        FLAG_CALC = 0x2,       // the layout changed: calc it in execute
        FLAG_HIDDEN = 0x4,     // << button
        FLAG_DISABLED = 0x20,  // << button
        FLAG_PUSH = 0x1000,    // a push animation is playing
        FLAG_ANM = 0x2000,     // the window animation (mAnm) is playing
    };

    dMenuBtn_c();                                          // 800CEC80
    virtual ~dMenuBtn_c();                                 // 800CED48

    bool load();                                           // 800CEE00: the button resources are there
    bool create();                                         // 800CEE2C
    bool remove();                                         // 800CEFE8
    bool execute();                                        // 800CEFF0
    bool draw();                                           // 800CF048

    // Moves the bar onto the pane paneName of layout.
    void setPos(m2d::SimpleEx_c *layout, const char *paneName); // 800CF07C
    int hitCheck(const mVec2_c *pos);                      // 800CF0C0: the button under pos, or -1
    int checkPad(BOOL useB, BOOL usePlus);                 // 800CF150: the button pushed with B/-/+, or -1
    void setSelect(int btn, BOOL select, BOOL playSe);     // 800CF268: the cursor is on the button
    void clearSelect();                                    // 800CF2D4
    BOOL push(int btn, BOOL noSe);                         // 800CF328
    bool isPushing();                                      // 800CF3D0
    BOOL startAnm(int anm);                                // 800CF3DC: a dMenuBase_c window animation
    bool isAnm();                                          // 800CF468
    void setText(int btn, u16 msgNo, u8 isCancel);         // 800CF474: a SYS_2D_menu string; shows the button
    void setVisible(int btn, bool visible);                // 800CF504
    void setDisable(int btn, BOOL disable);                // 800CF5CC
    void setPriority(u8 priority);                         // 800CF674
    bool isEnable(int btn);                                // 800CF67C: shown and not disabled
    void calcAnm();                                        // 800CF6B4

    /* 0x004 */ m2d::SimpleEx_c mLyt;
    /* 0x09C */ m2d::AnmEx2_c mBtnAnm[BTN_NUM];   // BTN_ANM_e
    /* 0x168 */ m2d::AnmEx2_c mInOutAnm[BTN_NUM]; // 0: out, 1: in (selected)
    /* 0x234 */ m2d::stringWord_c mText[BTN_NUM];
    /* 0x510 */ m2d::Anm_c mAnm;
    /* 0x534 */ int mAnmNo;
    /* 0x538 */ u32 mFlags;
    /* 0x53C */ int mPushedBtn;
    /* 0x540 */ u8 mIsCancel[BTN_NUM];             // pushing it plays the cancel sound
};
