#ifndef D_A_NPC_SP_JONNY_HPP
#define D_A_NPC_SP_JONNY_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_hmn_anm.hpp>

// Gulliver washed up on the beach (REL d_a_npc_sp_jonnyNP, profile
// A_NPC_SP_JONNY; dNpcMng_c::isFlag13Visit / setPosBeach spawn him after the
// UFO was shot down). He lies unconscious and talks in his sleep
// ("npc_SP/NPC_jonny" 1..10) until a talk wakes him up
// (SAVE_FLAG_GULLIVER_AWAKE); then he wanders around his block and asks for
// his spaceship part (ITEM_IDX_SPACESHIP_PART, dug up in town). The player who
// brings it gets a FROM_JONNY item (dSaveTown_c::mGulliverHelper).
//
// Class names from RTTI ("dAcNpcSpJonny_c", "dAcNpcSpJonny_c::talk_c");
// everything else is named from behaviour.
class dAcNpcSpJonny_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_SLEEP,       // unconscious on the beach
        STATE_ROAM,        // awake: wander around his block
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_NUM
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpJonny_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: the sleep talk, the wake-up and the spaceship
    // part.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_jonny message codes, named from the English text.
        enum {
            MSG_SLEEP_TALK = 0x01, // 0x01..0x0A: "Looka...me. I'm...so
                                   // unconscious..." (asleep)
            MSG_SLEEP_TALK_NUM = 10,
            MSG_WAKE_UP = 0x0B, // 0x0B..0x0F: "Oh! What is wrong with this
                                // planet?" ... (he wakes up)
            MSG_WAKE_UP_NUM = 5,
            MSG_PART_LOST = 0x10, // "No way! It's not here! It's gone!"
            MSG_INTRO = 0x11,     // "Why, hello, little space thing!..."
            MSG_FIND_PART = 0x12, // "I've gotta find that spaceship part and
                                  // get outta here..."
            MSG_FIND_PART_ASK = 0x13, // ... / Here you go. / Must be tough.
            MSG_PART_TOUGH = 0x14,    // "You bet it's tough!..."
            MSG_PART_WRONG = 0x15,    // "Oh, you've got to be kidding me!"
            MSG_PART_FOUND = 0x16,    // "Wow! You've found it!..."
            MSG_REWARD = 0x17,        // "I brought that with me from my
                                      // homeworld..."
            MSG_THANKS_HELPER = 0x1D, // (a tag only) the player who helped him
            MSG_THANKS = 0x1E,        // (a tag only) any other player
        };

        // The answers of MSG_FIND_PART_ASK.
        enum { ANSWER_HERE_YOU_GO = 0, ANSWER_MUST_BE_TOUGH = 1 };

        talk_c(dAcNpcSpJonny_c *npc);             // 0xD44
        virtual ~talk_c();                        // 0xDA4
        virtual void onMessageStart(int arg);     // 0x1130
        virtual void onMessageEnd(int kind);      // 0x1134
        virtual void onAnswer(int arg);           // 0x12B4
        virtual void init();                      // 0xDFC
        virtual void preExecute();                // 0x1364
        virtual void onTalkEnd();                 // 0x1368
        virtual void getMsgInfo(msgInfo_s *info); // 0x1118
        virtual void onRequestEnd(int kind);      // 0x136C
        void start(dActor_c *target);             // 0xEBC

        // ---- message / request hooks ----
        void playWakeUp();     // 0x13E8
        void openPartSelect(); // 0x1488
        void takePart();       // 0x150C
        void handOver();       // 0x1560
        void wakeUp();         // 0x15C4
        void closeWakeUp();    // 0x16AC
        void onPartSelected(); // 0x16B0
        void closePartTaken(); // 0x1764
        void giveReward();     // 0x1768
        void closeReward();    // 0x1818

        /* 0x0EA */ u16 mMsgNo; // the message start() picked
        /* 0x0EC */ dAcNpcSpJonny_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ Proc mNextOnRequestEnd; // mOnRequestEnd after the next one
        /* 0x114 */ Proc mOnAnswer;         // (not cleared by init)
    }; // size 0x120

    dAcNpcSpJonny_c() : mJonnyTalk(this) {
    }
    virtual ~dAcNpcSpJonny_c() {
    } // weak
    virtual void onDemoCmd(int cmd, u8 member); // 0x1CC
    void changeState(int state);                // 0x320
    virtual int preCreate();                    // 0x358
    virtual int create();                       // 0x3A4
    virtual int doDelete();                     // 0x4D8
    virtual int vtB0();                         // 0x504

    // ---- states ----
    void initSleep();         // 0x5D0
    void executeSleep();      // 0x5E4
    void initRoam();          // 0x5E8: dAcNpcSp_c::initWander
    void executeRoam();       // 0x5EC
    mVec3_c getWanderPos();   // 0x8B8: a free unit of his block
    void initLook();          // 0x9D8
    void stepLook();          // 0xA54
    void executeLook();       // 0xAB4
    void initTalk();          // 0xAF4
    void stepTalk();          // 0xB80
    void executeTalk();       // 0xBCC
    void initRemote();        // 0xC0C
    void executeRemote();     // 0xC14
    void initRemoteTalk();    // 0xCD0
    void executeRemoteTalk(); // 0xCD4

    // Back to his own state: wander once awake, else sleep.
    void changeStateIdle() {
        if (mAwake) {
            changeState(STATE_ROAM);
        } else {
            changeState(STATE_SLEEP);
        }
    }

    mAng getTalkAngle() const {
        dActor_c *target = mJonnyTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C */ int mBlockX; // his block (mPos at create, in blocks)
    /* 0x1C80 */ int mBlockZ;
    /* 0x1C84 */ int mState; // State_e
    /* 0x1C88 */ u8 mStep;
    /* 0x1C8C */ talk_c mJonnyTalk;
    /* 0x1DAC */ u8 mAwake; // SAVE_FLAG_GULLIVER_AWAKE seen
}; // size 0x1DB0

#endif
