#ifndef GAME_D_MENU_DEBUG00_HPP
#define GAME_D_MENU_DEBUG00_HPP

#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_paper_view.hpp>

// RTTI establishes global scope. All method/field names are inferred.
class dMenuDebug00_c : public dMenuBase_c {
public:
    // Flags passed to dMenuBase_c are bit indexes, not masks.
    enum FLAG_BIT_e { FLAG_UPDATE_PAPER, FLAG_DRAW, FLAG_MAIL_READY };
    enum SUB_STATE_e { SUB_MAIN, SUB_PUSH, SUB_LOAD, SUB_REMOVE };
    enum { NO_BUTTON = -1, BUTTON_BACK = 2, BACK_MESSAGE = 0x20 };

    virtual int create();               // .text:0xB8
    virtual int doDelete();             // 0x1F0
    virtual int stateMain();            // 0x244
    virtual int draw();                 // 0x284
    virtual int stateAnm();             // 0x2D0
    virtual int stateClose();           // 0x338
    virtual int execute();              // 0x380
    void subStateMain();               // 0x3C4
    void subStatePush();               // 0x490
    void subStateLoad();               // 0x4CC
    void subStateRemove();             // 0x53C
    void executeSubState();            // 0x58C
    void calcInput();                  // 0x5C4 (empty hook)
    void executePaper();               // 0x5C8
    void startCloseAnm();              // 0x614
    void initButtons();                // 0x630
    void setButtonSelect(int, BOOL);    // 0x68C
    int hitCheck();                    // 0x6BC
    void pushButton();                 // 0x71C
    bool isPushEnd();                  // 0x794
    void finishPush();                 // 0x7EC
    bool openNextMenu();               // 0x828

    static inline bool isButton(int button) { return button >= 0 && button <= dMenuBtn_c::BTN_NUM - 1; }

    virtual int state2() { return SUCCEEDED; }             // 0x940
    virtual int state3() { return SUCCEEDED; }             // 0x938
    virtual void postDraw(MAIN_STATE_e) {}                 // 0x934
    virtual void deleteReady() {}                          // 0x930
    virtual bool createHeap() { return true; }             // 0x928

    // Implicit destructor: .text:0x8AC.

    /* 0x00D4 */ dPaper_c mPaper;
    /* 0x0AC8 */ dMenuBtn_c mButtons;
    /* 0x100C */ int mSelectedButton;
}; // size 0x1010

#endif
