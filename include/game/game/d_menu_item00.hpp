#ifndef GAME_D_MENU_ITEM00_HPP
#define GAME_D_MENU_ITEM00_HPP

#include <game/game/d_fg_item.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_itemxx.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/mLib/m_vec.hpp>

// The pockets menu (REL d_menu_item00NP, profile MENU_ITEM00, opened from the
// ITEM tab of the menu bar): the item window of dMenuItemXX_c with the letter
// window next to it, the pointer picks up items and letters, and every held
// thing can be dropped on a pocket, a letter, the wallet button, the face
// button or the player. The class works as a state machine of 41 sub states
// (SUB_e, the table in executeSubState): the idle states of the item mode and
// the letter mode (SUB_MAIN / SUB_LETTER_MAIN), the press and hold states of
// pockets and letters, the answer window states (SUB_SELECT ...) and one wait
// state per action (equipment change, tool use on a hole, eating, the catch
// show, the message window, ...). Answering a pocket's question window runs
// onAnswer, which dispatches to one answer* function.
// RTTI confirms dMenuItem00_c (global) and its base dMenuItemXX_c; everything
// else is inferred from behaviour. dMenuItemXX_c's constructor is inlined into
// classInit.
class dMenuItem00_c : public dMenuItemXX_c {
  public:
    // Sub states (dMenuBase_c::setSubState, the table in executeSubState).
    enum SUB_e {
        SUB_MAIN, // item mode: the pointer picks a pocket / button / tab
        SUB_POCKET_PRESS, // a pocket is pressed (A): open its answers or pick
                          // it up
        SUB_HOLD_ITEM,    // an item is held; releasing A drops it
        SUB_LETTER_MAIN,  // letter mode: the same as SUB_MAIN
        SUB_LETTER_POCKET_PRESS,
        SUB_LETTER_HOLD_ITEM,
        SUB_LETTER_PRESS, // a letter is pressed
        SUB_HOLD_LETTER,  // a letter is held
        SUB_SELECT,       // the answers of the pocket / wallet / letter / face
                          // (mSelect)
        SUB_SUB_SELECT_OUT, // the timer length answers (mSubSelect) are closing
        SUB_SUB_SELECT,     // the timer length answers: start the timer
        SUB_LETTER_OPEN, // the letter button was pushed: open the letter window
        SUB_LETTER_ANM,  // wait for the button animation, then the next sub
                         // state
        SUB_LETTER_CLOSE, // the back button was pushed: close the letter window
        SUB_LETTER_CLOSE_ANM,
        SUB_CHANGE_EQUIP, // change the held item / shirt / accessory / hat
        SUB_CHANGE_EQUIP_END,
        SUB_PUT_HELD,   // dFgobjManager::putHeldItem: wait for the result
        SUB_ROOM_DECOR, // the carpet / wallpaper changes
        SUB_SHOW_CATCH, // show off a fish / insect (dPlayerMgr::showCatch)
        SUB_SHOW_CATCH_WAIT,
        SUB_CATCH_BUTTON, // the button of the show-off
        SUB_CATCH_PUSH,
        SUB_CATCH_END,
        SUB_USE_TOOL,  // dFgobjManager::useTool on a hole: wait for the result
        SUB_WAIT_HOLE, // wait for the player to put the item into the hole
        SUB_PLANT,     // dFgobjManager::putItem: wait for the result
        SUB_GIFT,      // the gift wrapping vanishes
        SUB_VANISH_LETTER, // the letter icon vanishes
        SUB_EAT,           // dPlayerMgr::eatItem
        SUB_WAIT_EAT,
        SUB_REMOVE_MASK, // dPlayerMgr::removeMaskForEating
        SUB_WAIT_EQUIP_END,
        SUB_BEGIN_DEMO, // dPlayerMgr::beginDemo
        SUB_RELEASE,    // release a fish / insect (showCatchAt)
        SUB_WAIT_DEMO,
        SUB_MSG_WIN, // the message window (mMsgWin)
        SUB_MSG_WIN_PUSH,
        SUB_MSG_WIN_CLOSE,
        SUB_MSG_WAIT, // the Wisp's message (mRcpt)
        SUB_MSG_END,
        SUB_NUM
    };

    // The answers of the question windows (dSelect::MComp_c::addItem's second
    // argument, passed to onAnswer). The comments are the answer texts.
    enum ANS_e {
        ANS_CANCEL,
        ANS_BACK, // the code of the cancel button of the timer length answers
        ANS_DROP, // Drop, Place in Room
        ANS_TAKE_ALL,
        ANS_TAKE_100,
        ANS_TAKE_1000,
        ANS_TAKE_10000,
        ANS_ADD_TO_WALLET,
        ANS_WEAR,        // Wear, Carry
        ANS_REMOVE_HELD, // Remove Equipment
        ANS_REMOVE_HAT,
        ANS_REMOVE_ACC,
        ANS_SET_DECOR, // Hang on Wall, Spread on Floor
        ANS_SHOW_OFF,
        ANS_BURY,
        ANS_PLANT,
        ANS_OPEN_PRESENT,
        ANS_EAT,
        ANS_TAKE_MEDICINE,
        ANS_MEASURE_TIME,
        ANS_STOP_TIMER,
        ANS_TIMER_3, // 21..24: the timer lengths (SUB_SUB_SELECT)
        ANS_TIMER_5,
        ANS_TIMER_10,
        ANS_TIMER_15,
        ANS_TOSS,
        ANS_READ,
        ANS_CHANGE_ADDRESSEE,
        ANS_BEGIN_DEMO,
        ANS_REWRITE,
        ANS_REMOVE_PRESENT,
        ANS_WRITE_LETTER,
        ANS_RELEASE,
        ANS_OPEN_EGG,
        ANS_RUB, // the magic lamp
        ANS_NUM
    };

    // The message codes (SYS_2D_menu.bmg): the answer texts, then the notices
    // of showMessage.
    enum {
        MSG_QUIT = 2,
        MSG_DROP = 3,
        MSG_BURY = 4,
        MSG_PLANT = 5,
        MSG_ALL = 6,
        MSG_10000_BELLS = 7,
        MSG_1000_BELLS = 8,
        MSG_100_BELLS = 9,
        MSG_NEVER_MIND = 10,
        MSG_WEAR = 11,
        MSG_REMOVE_EQUIPMENT = 37,
        MSG_REMOVE_HEADGEAR = 38,
        MSG_REMOVE_ACCESSORY = 39,
        MSG_RELEASE = 66,
        MSG_SHOW_IT_OFF = 67,
        MSG_OPEN = 93,
        MSG_TAKE = 104,
        MSG_READ = 105,
        MSG_TOSS = 106,
        MSG_REWRITE = 107,
        MSG_WRITE_A_LETTER = 108,
        MSG_CHANGE_ADDRESSEE = 118,
        MSG_REMOVE_PRESENT = 119,
        MSG_CARRY = 120,
        MSG_ADD_TO_WALLET = 130,
        MSG_3_MINUTES = 201,
        MSG_5_MINUTES = 202,
        MSG_10_MINUTES = 203,
        MSG_15_MINUTES = 204,
        MSG_MEASURE_TIME = 205,
        MSG_STOP = 206,
        MSG_HANG_ON_WALL = 232,
        MSG_SPREAD_ON_FLOOR = 233,
        MSG_PLACE_IN_ROOM = 234,
        MSG_RUB = 238,
        MSG_CANT_CHANGE_FLOORING = 302,
        MSG_CANT_CHANGE_WALLPAPER = 303,
        MSG_CANT_PUT_IN_ROOM = 304,
        MSG_NO_SPACE_TO_PUT = 306,
        MSG_LETTERS_FULL = 308,
        MSG_REMOVE_ACCESSORY_FIRST = 310,
        MSG_CANT_PLANT = 311,
        MSG_CANT_BURY = 312,
        MSG_CANT_REMOVE_POCKETS_FULL = 313,
        MSG_CANT_REMOVE_PRESENT = 314,
        MSG_CANT_TAKE_BELLS = 315,
        MSG_NOTHING_TO_REMOVE = 326,
        MSG_NO_BUG_IN_LETTER = 330,
        MSG_NO_FISH_IN_LETTER = 331,
        MSG_NO_TURNIP_IN_LETTER = 332,
        MSG_WALLET_FULL = 333,
        MSG_NO_TIMER_OTHERS_HIDE_SEEK = 334,
        MSG_NO_TIMER_HIDE_SEEK = 335,
    };

    // More bit indexes for dMenuBase_c::onFlag / isFlag (after
    // dMenuItemXX_c::FLAG_BIT_e).
    enum FLAG_BIT00_e {
        FLAG_ANSWER = 11, // the action comes from an answer window (the item is
                          // in mSelectSlot), not from a held item
        FLAG_DRAW_MSG_WIN = 13, // mMsgWin is drawn
        FLAG_RUB = 14, // the magic lamp was rubbed: summon the Wisp when the
                       // menu closes
    };

    // mEquipKind: what is being changed (the cases of subStateChangeEquip).
    enum EQUIP_e {
        EQUIP_HELD,
        EQUIP_SHIRT,
        EQUIP_ACCESSORY,
        EQUIP_HAT,
        EQUIP_CARPET,
        EQUIP_WALLPAPER,
    };

    // The room decoration of an item (getDecorType).
    enum DECOR_e { DECOR_NONE, DECOR_CARPET, DECOR_WALLPAPER };

    // mWinMode: the main sub state of the active window layout.
    enum WIN_MODE_e {
        WIN_MODE_ITEM = SUB_MAIN,
        WIN_MODE_LETTER = SUB_LETTER_MAIN,
    };

    virtual int create();     // .text:0x290
    virtual int doDelete();   // .text:0x3FC
    virtual int execute();    // .text:0x3E60
    virtual int draw();       // .text:0x3EAC
    virtual int stateAnm();   // .text:0x4A4
    virtual int stateMain();  // .text:0x45C
    virtual int stateClose(); // .text:0x4FC

    // Sub states (executeSubState).
    void subStateMain();              // .text:0x6B8
    void subStatePocketPress();       // .text:0x9E8
    void subStateHoldItem();          // .text:0xB34
    void subStateLetterMain();        // .text:0xEA4
    void subStateLetterPocketPress(); // .text:0x1148
    void subStateLetterHoldItem();    // .text:0x12A4
    void subStateLetterPress();       // .text:0x145C
    void subStateHoldLetter();        // .text:0x155C
    void subStateSelect();            // .text:0x1648
    void subStateSubSelectOut();      // .text:0x1790
    void subStateSubSelect();         // .text:0x181C
    void subStateLetterOpen();        // .text:0x1914
    void subStateLetterAnm();         // .text:0x19E8
    void subStateLetterClose();       // .text:0x1A34
    void subStateLetterCloseAnm();    // .text:0x1AEC
    void subStateChangeEquip();       // .text:0x1B50
    void subStateChangeEquipEnd();    // .text:0x1C00
    void subStatePutHeld();           // .text:0x1CA0
    void subStateRoomDecor();         // .text:0x1D88
    void subStateShowCatch();         // .text:0x1DD8
    void subStateShowCatchWait();     // .text:0x1E44
    void subStateCatchButton();       // .text:0x1ED8
    void subStateCatchPush();         // .text:0x1FD4
    void subStateCatchEnd();          // .text:0x2044
    void subStateUseTool();           // .text:0x2084
    void subStateWaitHole();          // .text:0x2194
    void subStatePlant();             // .text:0x21D8
    void subStateGift();              // .text:0x22BC
    void subStateVanishLetter();      // .text:0x2320
    void subStateEat();               // .text:0x239C
    void subStateWaitEat();           // .text:0x242C
    void subStateRemoveMask();        // .text:0x2470
    void subStateWaitEquipEnd();      // .text:0x24FC
    void subStateBeginDemo();         // .text:0x2538
    void subStateRelease();           // .text:0x258C
    void subStateWaitDemo();          // .text:0x25F0
    void subStateMsgWin();            // .text:0x2630
    void subStateMsgWinPush();        // .text:0x2690
    void subStateMsgWinClose();       // .text:0x26F4
    void subStateMsgWait();           // .text:0x2740
    void subStateMsgEnd();            // .text:0x27C0

    // Close requests (checkCloseRequest, one handler per sub state).
    bool closeByMode();      // .text:0x283C
    bool forceClose();       // .text:0x2850
    bool forceCloseLetter(); // .text:0x28CC
    bool closeSelects();     // .text:0x2904
    bool closeMsgWin();      // .text:0x2988
    bool closeButtons();     // .text:0x29E8

    // The answers (onAnswer).
    void answerCancel();          // .text:0x2A10
    void answerTakeBag(int item); // .text:0x2A14
    void answerTakeAll();         // .text:0x2A94
    void answerAddToWallet();     // .text:0x2B10
    void answerWear();            // .text:0x2B74
    void answerSetDecor();        // .text:0x2BCC
    void answerRemove(int kind);  // .text:0x2D08
    void answerShowOff();         // .text:0x2E24
    void answerBury();            // .text:0x2E68
    void plantItem();             // .text:0x2F48
    void answerPlant();           // .text:0x2FF0
    void answerOpenPresent();     // .text:0x3040
    void answerEat();             // .text:0x3230
    void answerTakeMedicine();    // .text:0x3274
    bool addTimerAnswers();       // .text:0x32B8
    void answerMeasureTime();     // .text:0x3358
    void answerStopTimer();       // .text:0x33A4
    void answerDrop();            // .text:0x33D8
    void answerToss();            // .text:0x3460
    void answerRemovePresent();   // .text:0x34A4
    void answerWriteLetter();     // .text:0x3510
    void answerRewrite();         // .text:0x3608
    void answerBeginDemo();       // .text:0x3790
    void answerRelease();         // .text:0x37A4
    void answerOpenEgg();         // .text:0x3808
    void answerRub();             // .text:0x38D4
    void answerChangeAddressee(); // .text:0x390C
    void answerRead();            // .text:0x3970
    void onAnswer(int answer);    // .text:0x3B10

    bool checkCloseRequest(); // .text:0x3BC0
    void executeSubState();   // .text:0x3E14

    // Building the answer windows.
    bool addPocketAnswers(int slot);   // .text:0x3F10
    bool openPocketAnswers(int slot);  // .text:0x42E8
    void openMoneyAnswers();           // .text:0x439C
    void openPlayerAnswers();          // .text:0x448C
    void openLetterAnswers(int slot);  // .text:0x45AC
    bool openPresentAnswers(int slot); // .text:0x46F8

    void backToMain();                            // .text:0x47F4
    void showMessage(u16 msg, BOOL cancelSe);     // .text:0x4848
    BOOL startEquip(const dItem::Item *item);     // .text:0x491C
    void clearSlotItem(int slot) {
        setSlotItem(slot, dItem::Item(), 0);
    }

    void placeFurniture();                        // .text:0x4A94
    void putOnGround();                           // .text:0x4BBC
    bool canWear(const dItem::Item *item);        // .text:0x4CBC
    int getDecorType(const dItem::Item *item);    // .text:0x4D78
    bool canShowOff(const dItem::Item *item);     // .text:0x4DFC
    bool canBury(const dItem::Item *item);        // .text:0x4E5C
    bool canPlant(const dItem::Item *item);       // .text:0x4EF0
    bool isStationery(const dItem::Item *item);   // .text:0x4F5C
    bool isEgg(const dItem::Item *item);          // .text:0x4F8C
    BOOL isMagicLamp(const dItem::Item *item);    // .text:0x4FCC
    bool isDesignWear(const dItem::Item *item);   // .text:0x500C
    BOOL canReleaseFish(const dItem::Item *item); // .text:0x504C
    int hitSlot();                                // .text:0x5098
    void startTimer(int minutes);                 // .text:0x5170
    void playSeEggCrack();                        // .text:0x51A8
    void playSePresentOpen();                     // .text:0x51B0

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x5634
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x5630
    virtual void deleteReady() {
    } // .text:0x562C
    virtual bool createHeap() {
        return true;
    } // .text:0x5624

    // Implicit destructor: .text:0x51B8.

    // 0x5028: a 4 byte member of dMenuItemXX_c.
    /* 0x502C */ dMsg::Rcpt_c mRcpt;     // the Wisp's message (the magic lamp)
    /* 0x5090 */ dItem::Item mEquipItem; // the item to wear
    /* 0x5092 */ dItem::Item mTakeOffItem; // the item that wearing replaces
    /* 0x5094 */ dItem::Item mItem;        // the item of the running action
    /* 0x5098 */ int mRequest;             // the dFgobjManager request
    /* 0x509C */ dMenuSlctB_c mMsgWin;     // the message window
    /* 0x578C */ dMenuBtn_c mBtn;          // the button of the show-off
    /* 0x5CD0 */ mVec3_c mCatchPos; // where the catch is shown / released
    /* 0x5CDC */ int mHoleX;        // the hole in front of the player (unit)
    /* 0x5CE0 */ int mHoleZ;
    /* 0x5CE4 */ int mHoldTimer; // frames the pressed pocket has been held
    /* 0x5CE8 */ int mPressSlot; // the pressed pocket / letter
    /* 0x5CEC */ u8 mEquipKind;  // EQUIP_e
    /* 0x5CED */ u8
        mShowOff; // the show-off button is drawn instead of the window
    /* 0x5CEE */ u8 mWinMode; // WIN_MODE_e
}; // size 0x5CF0

#endif
