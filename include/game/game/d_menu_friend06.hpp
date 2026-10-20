#ifndef GAME_D_MENU_FRIEND06_HPP
#define GAME_D_MENU_FRIEND06_HPP

#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/mLib/m_vec.hpp>

// The Friend Code tab of the friend menu (REL d_menu_friend06NP, profile
// MENU_FRIEND06): the window of Layout/friend_code.arc with the player's own
// friend code ("Your Friend Code", "town player" and the 12 digit code in
// groups of four). It is the child menu of dMenuFriend_c's second tab
// (dMenuFriend_c::TAB_CODE). It has no buttons of its own: the cursor
// (mCursor) points at one of the bottom panel tabs (dAcUitag), at one of the
// four tabs of dMenuFriend_c, or at the frame with the comment text box.
// RTTI confirms the global class name dMenuFriend06_c; everything else is
// inferred.
class dMenuFriend06_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_UITAG = 0, // 0..7: the tabs of the bottom panel (dAcUitag)
        CURSOR_UITAG_LAST = CURSOR_UITAG + dAcUitag_c::TAB_NUM - 1,
        CURSOR_TAB = 8, // 8..11: the tabs of dMenuFriend_c (TAB_e)
        CURSOR_TAB_LAST = CURSOR_TAB + 3,
        CURSOR_FRAME = 12, // the frame's comment text box
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_UNUSED,
        FLAG_DRAW, // the layout is drawn
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH, // waits for the pushed tab or frame to finish
        SUB_NUM,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_YOUR_FRIEND_CODE = 0x94, // "Your Friend Code"
        MSG_NO_FRIEND_CODE = 0x99,   // "You don't have a friend code yet."
        MSG_TOWN_PLAYER = 0x154,     // word 0 (town) and word 1 (player)
    };

    static bool isUitagTab(int cursor) {
        return cursor >= CURSOR_UITAG && cursor <= CURSOR_UITAG_LAST;
    }
    static bool isTab(int cursor) {
        return cursor >= CURSOR_TAB && cursor <= CURSOR_TAB_LAST;
    }

    virtual int create();     // .text:0xD8
    virtual int doDelete();   // .text:0x1A0
    virtual int execute();    // .text:0x284
    virtual int draw();       // .text:0x384
    virtual int stateAnm();   // .text:0x218
    virtual int stateMain();  // .text:0x1D0
    virtual int stateClose(); // .text:0x348

    BOOL executeCloseRequest();               // .text:0x508
    void executeSubState();                   // .text:0x5DC
    BOOL subStateCloseRequest();              // .text:0x4D0
    void subStateMain();                      // .text:0x3C8
    void subStatePush();                      // .text:0x494
    int hitUitagTab(const mVec2_c *pos);      // .text:0x614
    BOOL selectUitagTab(int tab, BOOL delay); // .text:0x678
    void startCloseAnm();                     // .text:0x6C8
    void startReopenAnm();                    // .text:0x704
    void startCommentAnm();                   // .text:0x744
    int hitCheck();                           // .text:0x78C
    void setSelect(int cursor, BOOL select);  // .text:0x81C
    void push();                              // .text:0x88C
    BOOL isPushEnd();                         // .text:0x948
    void finishPush();                        // .text:0x988
    void setTexts();                          // .text:0xA14
    void setFriendCode(u64 key);              // .text:0xAD0

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

    // Implicit destructor: .text:0xC50.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::Anm_c mAnm; // scrlIcon_00
    /* 0x264 */ int mCursor;
}; // size 0x268

#endif
