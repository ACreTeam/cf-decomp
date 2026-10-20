#ifndef GAME_D_MENU_PHOTO01_HPP
#define GAME_D_MENU_PHOTO01_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>

// The preview of the photo Pelly attaches to a Wii Message Board letter (REL
// d_menu_photo01NP, profile MENU_PHOTO01, opened as
// dMenuMng_c::MENU_POST_PHOTO): the window of Layout/photo.arc shows the photo
// that was taken in town (dPhoto's capture texture, copied into the picture
// pane photo_00), or the NoPhoto picture when there is none, and two buttons:
// "Quit" and "Use This". "Use This" (or PLUS) keeps the photo: it requests a
// small JPEG encode (dPhoto::requestEncode) and closes with result 1; "Quit"
// (or B / MINUS) closes with result 0. The menu only closes once dPhoto has no
// request running. RTTI confirms the global class name dMenuPhoto01_c;
// everything else is inferred.
class dMenuPhoto01_c : public dMenuBase_c {
  public:
    // The cursor (mCursor): the buttons.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN_FIRST = 0,
        CURSOR_UNUSED = 0, // hidden (N_cmnBtnA_00)
        CURSOR_QUIT = 1,
        CURSOR_USE = 2,
        CURSOR_BTN_LAST = CURSOR_USE,
        CURSOR_BTN_NUM,
    };

    // The button animations (mBtnAnm): photo_win_00_cmnBtnA_xx.
    enum BTN_ANM_e {
        BTN_ANM_OFF, // not selected (_out)
        BTN_ANM_ON,  // selected (_in)
        BTN_ANM_PUSH,
        BTN_ANM_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC, // set while a button animation plays
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH, // waits for the pushed button to finish
        SUB_NUM,
    };

    // dMenuMng_c::setResult values.
    enum RESULT_e {
        RESULT_QUIT,
        RESULT_USE,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x2,
        MSG_USE_THIS = 0x28,
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN_FIRST && cursor <= CURSOR_BTN_LAST;
    }

    virtual int create();     // .text:0x100
    virtual int doDelete();   // .text:0x20C
    virtual int stateMain();  // .text:0x23C
    virtual int stateAnm();   // .text:0x260
    virtual int stateClose(); // .text:0x264
    virtual int execute();    // .text:0x2A4
    virtual int draw();       // .text:0x328

    void subStateMain();                     // .text:0x36C
    void subStatePush();                     // .text:0x480
    void executeSubState();                  // .text:0x4BC
    void initAnms();                         // .text:0x4F4
    void calcAnms();                         // .text:0x5B4
    void setSelect(int cursor, BOOL select); // .text:0x630
    int hitCheck();                          // .text:0x670
    void push();                             // .text:0x6F8
    BOOL isPushEnd();                        // .text:0x794
    void finishPush();                       // .text:0x7D0
    void startCloseAnm();                    // .text:0x868
    BOOL copyPhoto();                        // .text:0x8A8

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

    // Implicit destructor: .text:0x954.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx2_c
        mBtnAnm[CURSOR_BTN_NUM]; // BTN_ANM_e, one per button
    /* 0x30C */ m2d::Anm_c mScrlIconAnm;
    /* 0x330 */ m2d::Anm_c mNoPhotoAnm;
    /* 0x354 */ int mCursor;
}; // size 0x358

#endif
