#ifndef D_A_NPC_SP_BUG_HPP
#define D_A_NPC_SP_BUG_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>

// Nat, the host of the Bug-Off (REL d_a_npc_sp_bugNP, profile A_NPC_SP_BUG).
// He is out on the event day (EVENT_BUG_OFF) from before the start until six
// hours after its end: he explains the tournament, sells a net to players
// without one, judges the insects the players bring (dBugOff_c::entryPlayer)
// and announces the results.
//
// Class names from RTTI ("dAcNpcSpBug_c", "dAcNpcSpBug_c::talk_c",
// "dAcNpcSpBug_c::dBugDebugPage_c"); everything else is named from behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dBugDebugPage_c), so dBugDebugPage_c is declared first.
class dAcNpcSpBug_c : public dAcNpcSp_c {
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

    typedef void (dAcNpcSpBug_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dBugDebugPage_c : public dDebugPage_c {};

    // The message receptor: the explanation, the net sale, the judging of the
    // player's insect and the results.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_bug (from the ENG text).
        enum {
            MSG_GREETING = 0x01,   // "BLARGLE BARGLE! ..." the explanation
            MSG_ABOUT = 0x02,      // "Are you here to learn about the Bug-Off?"
            MSG_EXPLAINED = 0x09,  // "...I wouldn't dream of eating the bug"
            MSG_KNOW_DRILL = 0x0A, // "I see you already know the drill!"
            MSG_GREETING_NO_NET = 0x0B, // the greeting, the player has no net
            MSG_OFFER_NET = 0x0D,       // "...part with mine for <n> Bells?"
            MSG_NET_NO_MONEY = 0x0E, // "...not the amount to which we agreed!"
            MSG_NET_POCKETS_FULL =
                0x0F,                  // "...your pockets are completely full!"
            MSG_NET_SOLD = 0x10,       // "Righto, then! Here you go!"
            MSG_NET_DECLINED = 0x11,   // "You truly aren't interested...?"
            MSG_ASK_BUG = 0x12,        // "...have you located any bugs for me?"
            MSG_NO_BUG_LEADING = 0x13, // "Shame, that." (the player leads)
            MSG_NO_BUG = 0x16,         // "Oh, that IS a bother."
            MSG_NO_BUG_LEADER = 0x17,  // "...bring it here posthaste!"
            MSG_LEADER_VILLAGER = 0x18, // "...the current leader is ..."
            MSG_SHOW_BUG = 0x19,        // "...why don't you show me this bug"
            MSG_RANK = 0x1A,      // 0x1A..0x1E: by the insect's rank 1..5 (6)
            MSG_EXQUISITE = 0x1F, // "Exquisite!"
            MSG_GRADE = 0x20,     // 0x20..0x22: by mGrade (the judge's notes)
            MSG_SCORE = 0x2C,     // "...this fine insect is worth <n> points!"
            MSG_BEAT_OWN = 0x2D,  // "...you just bested your previous score"
            MSG_LEAD_FROM_VILLAGER = 0x30, // "You just took the lead from ..."
            MSG_EAT = 0x33,                // "BLAAAARGH!"
            MSG_HEART_RACING = 0x35,       // "This one gets my tummy--"
            MSG_PRIZE = 0x36,              // "Now. Your prize. It's <item>."
            MSG_HUNT_MORE = 0x37,     // "...hunt down a few more rare bugs"
            MSG_STILL_LEADING = 0x38, // "...is still in the lead!"
            MSG_TAKE_BUG = 0x39, // "...I'm going to take your delicious <bug>."
            MSG_NOT_BEATEN_VILLAGER = 0x3A, // "...is a smidge better."
            MSG_KEEP_HUNTING = 0x3B,        // "Don't give up the hunt!"
            MSG_KEEP_HUNTING_AGAIN = 0x3C,  // "Hunt bugs in the fields!"
            MSG_BEFORE_START = 0x3D,        // "...Bugs sure are wonderful"
            MSG_START_TIME = 0x3E, // "...starting today's Bug-Off at 9 a.m."
            MSG_BEFORE_START_AGAIN = 0x3F, // "Today's Bug-Off starts at 9 a.m."
            // The results (each + 1: the results were heard already).
            MSG_RESULT_WON = 0x40,          // "Today's winner was YOU"
            MSG_RESULT_WON_VISITOR = 0x42,  // the winner, from another town
            MSG_RESULT_OTHER_TOWN = 0x44,   // the winner is from another town
            MSG_RESULT_VILLAGER = 0x46,     // "...<town>'s very own <villager>"
            MSG_LEADER_PLAYER = 0x48,       // "...the current top score is ..."
            MSG_LEAD_FROM_PLAYER = 0x49,    // "Your bug just took the lead!"
            MSG_NOT_BEATEN_PLAYER = 0x4A,   // "...is a smidge better."
            MSG_RESULT_PLAYER = 0x4B,       // "...<town>'s very own <player>"
            MSG_RESULT_WON_SENT = 0x4D,     // the winner, the prize was mailed
            MSG_LOWER_RANK_VILLAGER = 0x4F, // "...was a smidge more rare."
            MSG_LOWER_RANK_PLAYER = 0x50,   // "...was more rare."
            MSG_ASK_KEEP = 0x51,        // "...give it a new home with myself?"
            MSG_KEEP_REFUSED = 0x52,    // "Eh? No? Bangers and mash."
            MSG_CHANGE_MIND = 0x53,     // "...But if you change your mind..."
            MSG_RECORD_VILLAGER = 0x54, // "...the current record holder is..."
            MSG_RECORD_PLAYER = 0x55,   // "...our current record is held by..."
            MSG_RECORD_YOURS = 0x56,    // "...our current record holder is..."
        };
        // The answers of the two-way choices.
        enum { ANSWER_YES, ANSWER_NO };

        talk_c(dAcNpcSpBug_c *npc);               // 0xC60
        virtual ~talk_c();                        // 0xCD0
        virtual void init();                      // 0xD28
        virtual void getMsgInfo(msgInfo_s *info); // 0x1104
        virtual void onMessageStart(int arg);     // 0x111C
        virtual void onMessageEnd(int kind);      // 0x14DC
        virtual void onAnswer(int arg);           // 0x1B30
        virtual void preExecute();                // 0x1DE4
        virtual void onTalkEnd();                 // 0x1DE8
        virtual void onCustomTag(int no);         // 0x1DEC
        virtual void onRequestEnd(int kind);      // 0x1E20

        void start(dActor_c *target); // 0xDC8

        // ---- request hooks (mOnMessageEnd / mOnRequestEnd) ----
        void takeBug();       // 0x1EC8: the player hands the insect over
        void handActD();      // 0x1EE4
        void givePrize();     // 0x1EE8
        void sellNet();       // 0x1EF8: pay and receive the net
        void startThinking(); // 0x1F64
        void openBugSelect(); // 0x1FF0: pick the insect to judge
        void onBugSelected(); // 0x2058
        void judgeBug();      // 0x2214
        void setForceNext();  // 0x225C
        void pickUpPrize();   // 0x226C
        void pickUpNet();     // 0x22A4
        void closeMsg();      // 0x22E8
        void handActE();      // 0x22F4
        void setForceNext2(); // 0x22F8

        /* 0x0EA */ u16 mMsgCode; // the next message (getMsgInfo)
        /* 0x0EC */ u16 _0EC;
        /* 0x0F0 */ dAcNpcSpBug_c *mpOwner;
        /* 0x0F4 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x100 */ Proc mOnRequestEnd;
        /* 0x10C */ Proc mNextOnRequestEnd; // the mOnRequestEnd after it
        /* 0x118 */ dItem::Item mBug;       // the insect being judged
        /* 0x11A */ dItem::Item mPrize;
        /* 0x11C */ int mScore;    // dBugOff_c::entryPlayer
        /* 0x120 */ int mGrade;    // 0..2 (dBugOff_c::judge's size)
        /* 0x124 */ u8 mNewRecord; // the insect took the lead
    }; // size 0x128

    dAcNpcSpBug_c() : mBugTalk(this) {
    }
    virtual ~dAcNpcSpBug_c() {
    } // weak 0x23A4
    virtual void onDemoCmd(int cmd, u8 member); // 0x248
    void changeState(int state);                // 0x364
    virtual int preCreate();                    // 0x39C
    virtual int create();                       // 0x3E8
    virtual int doDelete();                     // 0x514
    virtual int vtB0();                         // 0x560
    virtual int getEarType();                   // 0x22EC
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
    bool isBugOffOngoing();                     // 0xAF0
    bool isBugOffNotStarted();                  // 0xB24
    bool isBugOffOver();                        // 0xB58
    BOOL isBugOffTime();                        // 0xB8C: he is out
    BOOL isForceAdvance();                      // 0xC0C

    mAng getTalkAngle() const {
        dActor_c *target = mBugTalk.mpActActor;
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
        u32 mActive : 1;       // Bug-Off time: he is out (mIsActive)
        u32 mForceAdvance : 1; // the players were away:
                               // dBugOff_c::advance(TRUE)
        u32 _0 : 30;
        u32 _4;
    }; // size 0x8

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dBugDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep[STATE_COUNT];
    /* 0x1C94 */ talk_c mBugTalk;
    /* 0x1DBC */ daub_s mDaub;
}; // size 0x1DC4

#endif
