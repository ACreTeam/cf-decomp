#ifndef D_A_NPC_SP_HANIWA_HPP
#define D_A_NPC_SP_HANIWA_HPP

#include <game/game/d_a_npc_sp.hpp>

// Lloid, the gyroid of the auction house (REL d_a_npc_sp_haniwaNP, profile
// A_NPC_SP_HANIWA; dScene::SCENE_RM_AUCTION, the auction itself is dSvAuc_c).
// He welcomes the player who comes in, sees the player off at the exit, takes
// items for display (with a reserve price), takes bids on the displayed items
// the player stands in front of, hands back unsold / won items and opens the
// checkroom.
//
// Class names from RTTI ("dAcNpcSpHaniwa_c", "dAcNpcSpHaniwa_c::talk_c");
// everything else is named from behaviour and the npc_SP/NPC_auction messages.
class dAcNpcSpHaniwa_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE, // face the player; speak at the exit or in front of an item
        STATE_LOOK, // turn to the player (demo start)
        STATE_TALK, // talk to the player
        STATE_GREET, // speak: welcome the player who came in
        STATE_EXIT,  // speak: see off the player at the exit
        STATE_ITEM,  // speak: the player stands in front of a displayed item
        STATE_NUM
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpHaniwa_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // What start() picks a message for (mMsgKind).
        enum MsgKind_e {
            MSG_KIND_GREET, // the player came in (STATE_GREET)
            MSG_KIND_EXIT,  // the player is at the exit (STATE_EXIT)
            MSG_KIND_TALK,  // the player talks to him
            MSG_KIND_ITEM // the player stands in front of an item (STATE_ITEM)
        };

        // What the player can pay a bid with (checkMoney).
        enum Pay_e {
            PAY_NONE, // neither the pockets nor the card cover it
            PAY_CASH, // only the pockets
            PAY_CARD, // only the card (savings)
            PAY_BOTH
        };

        // Message codes of npc_SP/NPC_auction (ENG text in the comments).
        enum {
            MSG_WELCOME =
                0x01, // "Mmmm, yes, yes. Welcome to the auction house..."
            MSG_RETURN_ITEM =
                0x02, // "We have something that needs be returned..."
            MSG_FIRST_VISIT = 0x03, // "...would this be <you> first time...?"
            MSG_ABOUT_DISPLAY =
                0x06, // "If <you> wishes to display an item for sale..."
            MSG_ABOUT_BID =
                0x07, // "If an item on display catches <your> eye..."
            MSG_ANYTHING_ELSE =
                0x08, // "...Was there anything else...?" (Displays / Bidding)
            MSG_COME_AGAIN =
                0x09,        // "A thousand hearty thanks! Do come again soon."
            MSG_MENU = 0x0B, // "How can I be of assistance this fine day?"
            MSG_MENU_AGAIN = 0x0C, // "And what action does <you> wish to take?"
            MSG_ANYTHING_ELSE_CHECKROOM =
                0x0F, // MSG_ANYTHING_ELSE with the checkroom
            MSG_LOOK_AROUND =
                0x10, // "Has something in our humble shop caught <your> eye?"
            MSG_NOTHING_ON_DISPLAY =
                0x11, // "...we have no items on display at this time."
            MSG_NO_ROOM = 0x12, // "...we have no more room for items."
            MSG_ONE_DISPLAY =
                0x13, // "I fear we can display only one item per customer."
            MSG_WHICH_ITEM = 0x14, // "Which item does <you> wish to display?"
            MSG_DISPLAY_CANCELED =
                0x15, // "Very well. That display has been canceled."
            MSG_CONFIRM_ITEM =
                0x16, // "Does <you> truly wish to display <item>?"
            MSG_ASK_RESERVE =
                0x17, // "...now a reserve price must be determined."
            MSG_CONFIRM_RESERVE =
                0x18, // "...set <your> reserve price at ... <bells>?"
            MSG_DISPLAYED =
                0x19, // "I shall inform <you> of this auction's result..."
            MSG_TOO_BIG = 0x1A, // "...we simply do not have room to display an
                                // item of that size."
            MSG_NEXT_DISPLAY =
                0x1B, // "...Our next display period ... begin on <date>"
            MSG_NEXT_BIDDING = 0x1C, // "...we are not taking bids ... next
                                     // bidding period ... <date>"
            MSG_ONE_BID =
                0x1D, // "...a strict limit of one bid per customer..."
            MSG_TAKE_RETURN_FIRST =
                0x1E, // "...there is an item that we must return to you."
            MSG_UNSOLD =
                0x1F, // "That is <item>, an item displayed here by <you>..."
            MSG_WON =
                0x20, // "That is <item>, a glorious item won by ... <you>."
            MSG_POCKETS_FULL =
                0x21, // "...<you> has too many items in <your> pockets..."
            MSG_ASK_TAKE_HOME =
                0x22,           // "...would prefer to take it home today?"
            MSG_TAKE_IT = 0x23, // "...Please, take the item at your leisure."
            MSG_SEND_LATER =
                0x24, // "I shall try to send it again at a later time."
            MSG_OWN_ITEM = 0x25,  // "That is <item> that was displayed by you."
            MSG_ITEM_INFO = 0x28, // "That is <item>, which <seller> from <town>
                                  // has displayed."
            MSG_BIDDING_ENDED =
                0x29, // "...we have already ended the bidding for that piece."
            MSG_CALCULATING = 0x2A, // "...still calculating his numbers..."
            MSG_ASK_BID = 0x2C, // "The reserve price for that item is <bells>.
                                // ... place a bid?"
            MSG_NO_BID = 0x2D,  // "You do not wish to bid? It shall be done."
            MSG_CONFIRM_BID =
                0x2E, // "...wishes to bid <bells> on <item>?" (cash)
            MSG_BID_PLACED = 0x2F, // "Very good. The bid has been placed."
            MSG_NOT_ENOUGH = 0x30, // "...does not have enough money."
            MSG_CASH_OR_CARD =
                0x31, // "...Would <you> care to pay by cash or card?"
            MSG_CARD_INSTEAD =
                0x32, // "...wallet is a bit light. ... pay by card instead?"
            MSG_BIDDING_LATER = 0x34, // "The next bidding period ... begin on
                                      // the date of <date>"
            MSG_MY_BID = 0x36, // "We have a bid of <bells> Bells from <you>!"
            MSG_FIRST_VISIT_ITEM = 0x3A, // "...Is this your first visit to our
                                         // humble auction house?"
            MSG_BID_ELSEWHERE =
                0x3B, // "...<you> has already entered a bid on another item."
            MSG_DISPLAY_OPEN =
                0x3D, // "...currently accepting items for display."
            MSG_DISPLAY_FULL = 0x3E, // "We have used all available space..."
            MSG_BIDDING_OPEN = 0x3F, // "...currently accepting bids!"
            MSG_PREPARING_DISPLAY = 0x40, // "...preparing for the next auction!
                                          // ... display period ..."
            MSG_PREPARING_BIDDING =
                0x41, // "...preparing to accept bids. ... bidding period ..."
            MSG_CHECKROOM =
                0x42, // "<name>, here is your exclusive, private checkroom!"
            MSG_CHECKROOM_THANKS = 0x43, // "Thanks upon thanks! ..."
        };

        // The answers of MSG_MENU / MSG_MENU_AGAIN.
        enum {
            MENU_DISPLAY,
            MENU_BID,
            MENU_CHECKROOM,
            MENU_WHATS_THIS,
            MENU_NEVER_MIND
        };

        talk_c(dAcNpcSpHaniwa_c *npc);            // 0xA68
        virtual ~talk_c();                        // 0xADC
        virtual void onMessageStart(int arg);     // 0xF94
        virtual void onMessageEnd(int kind);      // 0xFF8
        virtual void onAnswer(int arg);           // 0x16E0
        virtual void init();                      // 0xB34
        virtual void preExecute();                // 0x1940
        virtual void onTalkEnd();                 // 0x1944
        virtual void getMsgInfo(msgInfo_s *info); // 0xF7C
        virtual void onRequestEnd(int kind);      // 0x1C88

        void start(dActor_c *target);      // 0xBCC
        void startDisplay();               // 0x199C: "Display an item!"
        void startBid();                   // 0x1AC0: "Bid!"
        void placeBid(int bid, BOOL card); // 0x1B8C
        int checkMoney(int bid);           // 0x1C18: Pay_e

        // ---- message / request hooks ----
        void selectItem();    // 0x1D78
        void openPriceMenu(); // 0x1E18
        void openBidMenu();   // 0x1E80
        void openCheckroom(); // 0x2184
        void endSelectItem(); // 0x2218
        void endPriceMenu();  // 0x22D0
        void setItemWords();  // 0x2348
        void endBidMenu();    // 0x2598
        void endCheckroom();  // 0x267C

        /* 0x0EA */ u16 mMsgNo; // the next message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpHaniwa_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageStart; // one-shot hooks (cleared after the
                                          // call), except onMessageEnd
        /* 0x0FC */ Proc mOnMessageEnd;
        /* 0x108 */ Proc mOnAnswer;
        /* 0x114 */ Proc mOnRequestEnd;
        /* 0x120 */ int mMsgKind;      // MsgKind_e
        /* 0x124 */ u8 mExiting;       // the exit was requested (onTalkEnd)
        /* 0x126 */ dItem::Item mItem; // the item to display
        /* 0x128 */ int mBid;          // the bid entered (endBidMenu)
        /* 0x12C */ u32 mSlot;         // the auction slot of the pointed item
        /* 0x130 */ int mFtrHandle;    // the furniture handle of that item
        /* 0x134 */ u8 mFirstVisit;    // the first talk in the auction house
    }; // size 0x138

    dAcNpcSpHaniwa_c() : mTalkHaniwa(this) {
    }
    virtual ~dAcNpcSpHaniwa_c() {
    } // 0x2744
    virtual void onDemoCmd(int cmd, u8 member); // 0x20C
    void changeState(int state);                // 0x2E0
    void calcAnmRate();                         // 0x330
    virtual int getEarType();                   // 0x3BC
    virtual int preCreate();                    // 0x3C4
    virtual int create();                       // 0x440
    virtual int doDelete();                     // 0x49C
    virtual int vtB0();                         // 0x4C8
    void initIdle();                            // 0x524
    void executeIdle();                         // 0x580
    void initLook();                            // 0x618
    void stepLook();                            // 0x67C
    void executeLook();                         // 0x6DC
    void initTalk();                            // 0x730
    void stepTalk();                            // 0x79C
    void executeTalk();                         // 0x7F4
    void initGreet();                           // 0x848
    void executeGreet();                        // 0x84C
    void initExit();                            // 0x8A0
    void executeExit();                         // 0x8A4
    void initItem();                            // 0x8F8
    void executeItem();                         // 0x8FC
    BOOL isAtExit();                            // 0x950
    BOOL checkPointedItem();                    // 0x990

    mAng getTalkAngle() const {
        dActor_c *target = mTalkHaniwa.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C */ int mState;
    /* 0x1C80 */ u8 mStep;
    /* 0x1C84 */ talk_c mTalkHaniwa;
}; // size 0x1DBC

#endif
