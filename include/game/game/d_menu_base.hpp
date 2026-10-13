#pragma once

// The menu base class dMenuBase_c (its RTTI name; TU d_menu_base.cpp, .text 800CE43C..800CEC80): the base of the
// menus in the d_menu_*NP modules. A menu registers with the menu manager (d_menu_mng), runs a state machine
// (execute calls the state's virtual), plays its window animations and holds the menu sound effects. Also
// dMenuMove_c, a menu window's eased position (d_menu_item, d_menu_ltr, d_menu_desi, d_menu_edit).
// Every name but dMenuBase_c and the sound labels is inferred.

#include <types.h>
#include <game/cLib/c_line.hpp>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_2d.hpp>
#include <game/mLib/m_vec.hpp>

class dMenuBase_c;

// A menu's node in the menu manager's list.
class dMenuNd_c : public cLineNd_c {
public:
    /* 0x08 */ dMenuBase_c *mpOwner;
};

class dMenuBase_c : public fBase_c {
public:
    enum STATE_e {
        STATE_ANM,   // plays the window animation, then changes to mNextState
        STATE_MAIN,
        STATE_2,
        STATE_3,
        STATE_CLOSE, // deletes the menu
        STATE_NUM
    };

    // The window animations.
    enum ANM_e {
        ANM_WIN_IN,
        ANM_WIN_OUT,
        ANM_WIN_UP,
        ANM_WIN_DOWN,
        ANM_UPPER_WIN_IN,
        ANM_UPPER_WIN_OUT,
        ANM_NUM
    };

    dMenuBase_c();                                         // 800CE43C
    virtual ~dMenuBase_c();                                // 800CE48C

    static const char *getAnmName(int anm);                // 800CE4F8
    void setRepeat(int delay, int interval, int count);    // 800CE50C

    virtual int preCreate();                               // 800CE530
    virtual void postCreate(MAIN_STATE_e state);           // 800CE574
    virtual int preDelete();                               // 800CE608
    virtual void postDelete(MAIN_STATE_e state);           // 800CE634
    virtual int preExecute();                              // 800CE684
    virtual void postExecute(MAIN_STATE_e state);          // 800CE6B0
    virtual int execute();                                 // 800CE6B4

    void setState(u8 state);                               // 800CE6F0
    bool isState(u8 state);                                // 800CE6F8
    void changeNextState();                                // 800CE70C
    void setNextState(u8 state);                           // 800CE714

    // Binds an animation (anm: ANM_e) made for the pane "N_all" to the layout's pane paneName and plays it in
    // STATE_ANM; nextState follows it.
    void startAnm(m2d::Simple_c *layout, const char *paneName, int anm, u8 nextState, BOOL playSe); // 800CE71C

    // vtable 0x4C..0x5C: the states.
    virtual int stateAnm();                                // 800CE804
    virtual int stateMain() { return SUCCEEDED; }          // 800CEC78
    virtual int state2() { return SUCCEEDED; }             // 800CEC70
    virtual int state3() { return SUCCEEDED; }             // 800CEC68
    virtual int stateClose();                              // 800CE8EC

    bool calcAnm();                                        // 800CE84C: TRUE once the animation is done

    void onFlag(int bit);                                  // 800CE910
    void offFlag(int bit);                                 // 800CE928
    bool isFlag(int bit);                                  // 800CE940

    // The derived menus' own state in STATE_MAIN.
    virtual void setSubState(u8 state);                    // 800CE960
    void setNextSubState(u8 state);                        // 800CE968
    void changeNextSubState();                             // 800CE970

    void setPostFlags(u32 flags);                          // 800CE97C (d_menu_post01)
    u32 getPostFlags();                                    // 800CE984 (d_menu_post01)

    static void playSeCursor();                            // 800CE98C
    static void playSeDecide();                            // 800CE994
    static void playSeDecide2();                           // 800CE99C
    static void playSeCancel();                            // 800CE9A4
    static void playSeCancel2();                           // 800CE9AC
    static void playSeSlideClick();                        // 800CE9B4
    static void playSeAPush();                             // 800CE9BC
    static void playSeSubMenuOpen();                       // 800CE9C4
    static void playSeSubMenuClose();                      // 800CE9CC
    static void playSeSubMenuPopUp();                      // 800CE9D4
    static void playSeSubMenuPush();                       // 800CE9DC
    static void playSePageTurn();                          // 800CE9E4
    static void playSePageTurnNo(int page);                // 800CE9EC: SE_gamen_okuri01..16
    static void playSeItemLift();                          // 800CEA00
    static void playSeItemDrop();                          // 800CEA08
    static void playSeSelectWindowOpen();                  // 800CEA10
    static void playSeListChoice();                        // 800CEA18
    static void playSeCanNotLeave();                       // 800CEA20
    static void playSeCanNot();                            // 800CEA28

protected:
    /* 0x64 */ u8 mState;     // STATE_e
    /* 0x65 */ u8 mNextState;
    /* 0x66 */ u8 mMenuNo;    // the menu id (mParam)
    /* 0x67 */ u8 mNextSubState;
    /* 0x68 */ u8 mSubState;     // the derived menu's state
    /* 0x6C */ dMenuNd_c mNode;
    /* 0x78 */ m2d::Anm_c mAnm;
    /* 0x9C */ m2d::Simple_c *mpAnmLayout;
    /* 0xA0 */ char mAnmPaneName[20];
    /* 0xB4 */ u32 mFlags;
    // Only set by preCreate; key repeat is a guess.
    /* 0xB8 */ int mRepeatCnt[4];
    /* 0xC8 */ int mRepeatDelay;
    /* 0xCC */ int mRepeatInterval;
    /* 0xD0 */ int mRepeatCount;

    static u32 s_postFlags;
};

// A menu window's position (mPos): it follows the cursor with an offset that eases out, or eases to a target.
class dMenuMove_c {
public:
    void calcFollow();                           // 800CEA30
    void startFollow();                          // 800CEAE4
    mVec2_c *getPos();                           // 800CEAF0
    void setOfs(const mVec2_c &ofs);             // 800CEAF8
    bool isMoveEnd();                            // 800CEB0C
    bool calcMove();                             // 800CEB1C: TRUE once at the target
    void startMove(const mVec2_c &target);       // 800CEBDC
    void stop();                                 // 800CEBF8
    void setPos(const mVec2_c &pos);             // 800CEC08

    /* 0x00 */ mVec2_c mOfs;
    /* 0x08 */ int mFollowCnt;
    /* 0x0C */ mVec2_c mPos;
    /* 0x14 */ mVec2_c mTarget;
    /* 0x1C */ int mMoveCnt;
};

extern "C" {
int fn_800CEC1C(); // 800CEC1C: 15 (d_menu_item00, d_menu_desi00, d_menu_edit, d_menu_post00/01)
}
