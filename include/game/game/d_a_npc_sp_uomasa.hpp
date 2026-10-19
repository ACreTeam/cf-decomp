#ifndef D_A_NPC_SP_UOMASA_HPP
#define D_A_NPC_SP_UOMASA_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>

// Chip, the host of the fishing tourney (REL d_a_npc_sp_uomasaNP, profile
// A_NPC_SP_UOMASA, messages "npc_SP/NPC_uomasa"). He is out on the event day
// (EVENT_FISHING_TOURNEY) from before the start until six hours after its end:
// he explains the tourney, sells a fishing rod to players without one,
// measures the fish the players bring (dEvFishing_c::entryPlayer), eats it or
// keeps it and announces the results. The counterpart of Nat at the Bug-Off
// (d_a_npc_sp_bug).
//
// Class names from RTTI ("dAcNpcSpUomasa_c", "dAcNpcSpUomasa_c::talk_c",
// "dAcNpcSpUomasa_c::dUomasaDebugPage_c"); everything else is named from
// behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dUomasaDebugPage_c), so dUomasaDebugPage_c is declared first.
class dAcNpcSpUomasa_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,        // face the front (south)
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // mStep[STATE_IDLE] (l_idleSteps), mStep[STATE_LOOK] (l_lookSteps) and
    // mStep[STATE_TALK] (l_talkSteps).
    enum IdleStep_e { IDLE_STEP_TURN, IDLE_STEP_WAIT, IDLE_STEP_DONE };
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_DONE };
    enum TalkStep_e {
        TALK_STEP_TURN,     // wait for the talk, then turn back to the front
        TALK_STEP_TURN_END, // wait for the turn, finish the talk
        TALK_STEP_DONE
    };

    typedef void (dAcNpcSpUomasa_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dUomasaDebugPage_c : public dDebugPage_c {};

    // The message receptor: the explanation, the rod sale, the measuring of
    // the player's fish and the results.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_uomasa (from the ENG text). Two sets:
        // the tourney of one kind of fish (dEvFishing_c::getFish valid) and,
        // with _ANY, the tourney of any fish.
        enum {
            MSG_GREETING = 0x01,     // "Ah, step right up! You fish?" + rules?
            MSG_RULES_DONE = 0x04,   // "...If you got the big one of the day"
            MSG_NO_RULES = 0x05,     // "Oh HO! No rules for you?"
            MSG_ENOUGH_RULES = 0x06, // "Oh, REALLY? Enough?"
            MSG_OFFER_ROD = 0x07,    // "...How does a fishing rod at <n> Bells"
            MSG_ROD_SOLD = 0x08, // "Hey, thank ya, thank ya! Here's your rod."
            MSG_ROD_POCKETS_FULL = 0x0A, // "...Room for a fishing rod in your
                                         // pockets."
            MSG_ROD_NO_MONEY = 0x0B,     // "...short on cash!"
            MSG_ROD_DECLINED = 0x0C,     // "What?! You don't want one?!"
            MSG_ASK_CATCH = 0x0D,        // "...You catch one? Well, did you?"
            MSG_NO_CATCH_LEADING = 0x0E, // "Eh? You caught nothing?" (the
                                         // player leads; then 0x0F)
            MSG_NO_CATCH = 0x10,         // "Eh? You caught nothing?"
            MSG_LEADER_VILLAGER = 0x11,  // "...today's biggest entry is
                                         // <villager>'s"
            MSG_SHOW_FISH = 0x12,        // "...Give it here!"
            MSG_MEASURE = 0x13,          // "Now, let's just measure this tasty
                                         // fellow"
            MSG_RECORD_OWN = 0x14, // "YES! Gravy! A new record! ...you just
                                   // beat yourself"
            MSG_NEW_HOME = 0x15,   // "...put you in your new home!"
            MSG_EAT_PRIZE = 0x16,  // "DEEE-li-cious! ...the prize..."
            MSG_RECORD_VILLAGER = 0x18, // "...A new record!" (from a villager)
            MSG_STILL_LEADING = 0x19,   // "...your <n>-incher is still sitting
                                        // strong at the top!"
            MSG_TAKE_FISH = 0x1A,       // "...I'll take it off your hands!"
            MSG_EAT = 0x1B,             // "DEEE-li-cious! BUURRP!"
            MSG_NOT_BEATEN_VILLAGER = 0x1D, // "Ouch, too bad, kiddo." (a
                                            // villager leads)
            MSG_WRONG_FISH = 0x1E,   // "Hold up, now! This is no <fish>!"
            MSG_ASK_KEEP = 0x1F,     // "...do you mind if I hold on to this?"
            MSG_KEEP_OK = 0x20,      // "I can? I can really keep it?"
            MSG_KEEP_REFUSED = 0x22, // "Huh? You're de-fishing me?"
            MSG_BEFORE_START = 0x24, // "...The Fishing Tourney begins..."
            // The results (each + 1: the results were heard already).
            MSG_RESULT_WON = 0x26,         // "The lucky winner ... is you!"
            MSG_RESULT_WON_VISITOR = 0x28, // the winner, from another town
            MSG_RESULT_OTHER_TOWN = 0x2A,  // the winner is from another town
            MSG_RESULT_VILLAGER = 0x2C,    // "...<villager> from here in..."
            MSG_KEEP_RARE = 0x2E,          // "...I can KEEP this rare beauty?"
            MSG_RARE_PRIZE = 0x30, // "Even I know better than to eat a fish of
                                   // this stature!" + the prize
            MSG_RARE_TAKEN = 0x32, // "Even I know better than to eat..."

            MSG_GREETING_ANY = 0x33,
            MSG_RULES_DONE_ANY = 0x35,
            MSG_NO_RULES_ANY = 0x36,
            MSG_OFFER_ROD_ANY = 0x37,
            MSG_ROD_SOLD_ANY = 0x38,
            MSG_ROD_POCKETS_FULL_ANY = 0x3A,
            MSG_ROD_NO_MONEY_ANY = 0x3B,
            MSG_ROD_DECLINED_ANY = 0x3C,
            MSG_ASK_CATCH_ANY = 0x3D,
            MSG_NO_CATCH_LEADING_ANY = 0x3E,
            MSG_NO_CATCH_ANY = 0x40,
            MSG_LEADER_VILLAGER_ANY = 0x41,
            MSG_SHOW_FISH_ANY = 0x42,
            MSG_MEASURE_ANY = 0x43, // "Ah, <fish>! Now, let's just measure"
            MSG_RECORD_OWN_ANY = 0x44,
            MSG_NEW_HOME_ANY = 0x45,
            MSG_EAT_PRIZE_ANY = 0x46,
            MSG_RECORD_VILLAGER_ANY = 0x48,
            MSG_STILL_LEADING_ANY = 0x49,
            MSG_TAKE_FISH_ANY = 0x4A, // "Lemme just take this fine fish off
                                      // your hands!"
            MSG_EAT_ANY = 0x4B,
            MSG_NOT_BEATEN_VILLAGER_ANY = 0x4D,
            MSG_BEFORE_START_ANY = 0x4E,
            MSG_RESULT_WON_ANY = 0x50,
            MSG_RESULT_WON_VISITOR_ANY = 0x52,
            MSG_RESULT_OTHER_TOWN_ANY = 0x54,
            MSG_RESULT_VILLAGER_ANY = 0x56,
            MSG_RARE_PRIZE_ANY = 0x58,
            MSG_RARE_TAKEN_ANY = 0x5A,
            MSG_KEEP_TRYING_RARE = 0x5C, // "Don't give up now! Do it!"
                                         // (after MSG_RARE_TAKEN)

            MSG_SIZE = 0x5D,              // "Ah, it's...a <n>-incher!"
            MSG_SIZE_ANY = 0x5E,          // "Ah, it's...a <n>-incher!"
            MSG_LEADER_PLAYER = 0x5F,     // "...today's biggest entry is
                                          // <player>'s"
            MSG_RECORD_PLAYER = 0x60,     // "...A new record!" (from a
                                          // player)
            MSG_NOT_BEATEN_PLAYER = 0x61, // "A pure shame, kiddo!" (a player
                                          // leads)
            MSG_RESULT_PLAYER = 0x62,     // "...<player> from here in..."
            MSG_DISH = 0x65, // 0x65..0xA4: his recipe of the fish (+ the
                             // fish's index in its kind)
            MSG_DISH_LAST = 0xA4,
            MSG_LEADER_PLAYER_ANY = 0xA5,
            MSG_RECORD_PLAYER_ANY = 0xA6,
            MSG_NOT_BEATEN_PLAYER_ANY = 0xA7,
            MSG_RESULT_PLAYER_ANY = 0xA8,
            MSG_RESULT_WON_SENT = 0xAA, // "We stopped accepting entries"
                                        // (the prize was mailed)
            MSG_RESULT_WON_SENT_ANY = 0xAC,
        };
        // The word slots of the messages (onMessageStart).
        enum {
            WORD_TOURNEY_FISH = 0, // the tourney's fish; the rod price
            WORD_PRIZE = 1,
            WORD_FISH = 2, // the player's fish
            WORD_LEADER_NAME = 3,
            WORD_LEADER_TOWN = 4,
            WORD_LEADER_FISH = 5, // any-fish tourney
            WORD_RUNNER_UP_NAME = 6,
            WORD_RUNNER_UP_FISH = 7, // any-fish tourney
            // The sizes (setDecimal).
            WORD_LEADER_SIZE = 1,
            WORD_SIZE = 2, // the player's fish
            WORD_RUNNER_UP_SIZE = 3,
        };
        // The fish select menu (openFishSelect).
        enum { MENU_SELECT_FISH = 0x31 };
        enum { ITEM_SEL_MSG_GIVE = 0xE7 };
        // The answers of MSG_OFFER_ROD, MSG_ASK_CATCH and MSG_ASK_KEEP.
        enum { ANSWER_BUY = 0, ANSWER_DECLINE = 1 };
        enum { ANSWER_CAUGHT = 0, ANSWER_NOTHING = 1 };
        enum { ANSWER_MIND = 0, ANSWER_GO_AHEAD = 1 };

        talk_c(dAcNpcSpUomasa_c *npc);            // 0xC60
        virtual ~talk_c();                        // 0xCD0
        virtual void init();                      // 0xD28
        virtual void getMsgInfo(msgInfo_s *info); // 0x12E4
        virtual void onMessageStart(int arg);     // 0x12FC
        virtual void onMessageEnd(int kind);      // 0x1760
        virtual void onAnswer(int arg);           // 0x21EC
        virtual void preExecute();                // 0x2424
        virtual void onTalkEnd();                 // 0x2428
        virtual void onRequestEnd(int kind);      // 0x242C
        virtual void onCustomTag(int no);         // 0x2A68
        virtual int getVoiceMode();               // 0x2AD4

        void start(dActor_c *target); // 0xDC8

        // ---- request hooks (mOnMessageEnd / mOnRequestEnd) ----
        void takeFish();         // 0x24D4: the player hands the fish over
        void returnFish();       // 0x24F0
        void eatFish();          // 0x24F4
        void keepFish();         // 0x253C
        void givePrize();        // 0x2540
        void sellRod();          // 0x2550: pay and receive the rod
        void startMeasure();     // 0x25BC
        void openFishSelect();   // 0x2648: pick the fish to measure
        void onFishSelected();   // 0x26B0
        void removeFish();       // 0x2928: clear its pocket slot
        void pickUpFish();       // 0x2988: the fish he gives back
        void setForceNextEat();  // 0x29C0
        void setForceNextKeep(); // 0x29D0
        void pickUpPrize();      // 0x29E0
        void pickUpRod();        // 0x2A18
        void onMeasured();       // 0x2A5C (empty)

        /* 0x0EA */ u16 mMsgCode;   // the next message (getMsgInfo)
        /* 0x0EC */ u16 mAfterDish; // the message after MSG_DISH + n
        /* 0x0F0 */ dAcNpcSpUomasa_c *mpOwner;
        /* 0x0F4 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x100 */ Proc mOnRequestEnd;
        /* 0x10C */ Proc mNextOnRequestEnd; // the mOnRequestEnd after it
        /* 0x118 */ dItem::Item mFish;      // the fish being measured
        /* 0x11A */ dItem::Item mPrize;
        /* 0x11C */ int mSize;      // dEvFishing_c::entryPlayer
        /* 0x120 */ u8 mNewRecord;  // the fish took the lead
        /* 0x124 */ int mVoiceMode; // VOICE_MODE_*: silent while he eats
    }; // size 0x128

    dAcNpcSpUomasa_c() : mUomasaTalk(this) {
    }
    virtual ~dAcNpcSpUomasa_c() {
    } // weak 0x2B80
    virtual void onDemoCmd(int cmd, u8 member); // 0x248
    void changeState(int state);                // 0x364
    virtual int preCreate();                    // 0x39C
    virtual int create();                       // 0x3E8
    virtual int doDelete();                     // 0x514
    virtual int vtB0();                         // 0x560
    virtual int getEarType();                   // 0x2A60
    void initIdle();                            // 0x690
    void stepIdleTurn();                        // 0x6F0
    void stepIdleWait();                        // 0x74C
    void executeIdle();                         // 0x750
    void initLook();                            // 0x790
    void stepLook();                            // 0x7F4
    void executeLook();                         // 0x854
    void initTalk();                            // 0x894
    void stepTalkTurn();                        // 0x900
    void stepTalkTurnEnd();                     // 0x970
    void executeTalk();                         // 0x9C0
    void initRemote();                          // 0xA00
    void executeRemote();                       // 0xA04
    void initRemoteTalk();                      // 0xA98
    void executeRemoteTalk();                   // 0xA9C
    bool isTourneyOngoing();                    // 0xAF0
    bool isTourneyNotStarted();                 // 0xB24
    bool isTourneyOver();                       // 0xB58
    BOOL isTourneyTime();                       // 0xB8C: he is out
    BOOL isForceAdvance();                      // 0xC0C

    mAng getTalkAngle() const {
        dActor_c *target = mUomasaTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    // The net daub data (getDaubData / setDaubData).
    struct daub_s {
        u32 mActive : 1;       // tourney time: he is out (mIsActive)
        u32 mForceAdvance : 1; // the players were away:
                               // dEvFishing_c::advance(TRUE)
        u32 _0 : 30;
        u32 _4;
    }; // size 0x8

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dUomasaDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep[STATE_COUNT];
    /* 0x1C94 */ talk_c mUomasaTalk;
    /* 0x1DBC */ daub_s mDaub;
}; // size 0x1DC4

#endif
