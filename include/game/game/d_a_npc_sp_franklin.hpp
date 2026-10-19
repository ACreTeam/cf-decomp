#ifndef D_A_NPC_SP_FRANKLIN_HPP
#define D_A_NPC_SP_FRANKLIN_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>

// Franklin at the Harvest Festival (REL d_a_npc_sp_franklinNP, profile A_NPC_SP_FRANKLIN).
class dAcNpcSpFranklin_c : public dAcNpcSp_c {
  public:
    enum State_e {
        STATE_IDLE,
        STATE_LOOK,
        STATE_TALK,
        STATE_REMOTE,
        STATE_REMOTE_TALK,
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_DONE };
    enum TalkStep_e { TALK_STEP_TURN, TALK_STEP_TURN_END, TALK_STEP_DONE };
    enum {
        EMOTION_FRET = 6, // played while his spot is not blocked to the south
        IDLE_WAIT_MIN = 120,
        IDLE_WAIT_RAND = 300,
        SPOT_RETRY_FRAMES = 300,
    };

    typedef void (dAcNpcSpFranklin_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    class dFranklinDebugPage_c : public dDebugPage_c {};

    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes
        enum {
            MSG_FIRST = 0x01,           // "Yaaaaah!" the first meeting
            MSG_ANIMALS = 0x07,         // "...a bunch of animals!"
            MSG_KNIVES = 0x08,          // "The knife and fork the mayor's holding? Those have got to go!"
            MSG_HAND_OVER = 0x0B,
            MSG_GIFT = 0x0C,
            MSG_SECRET = 0x0D,          // "...It's our secret!"
            MSG_KNIVES_NONE = 0x0E,     // MSG_KNIVES, the player has none
            MSG_STILL_HERE = 0x11,      // "You still want something from me?"
            MSG_HAND_OVER_2 = 0x14,
            MSG_GIFT_2 = 0x15,
            MSG_SECRET_2 = 0x16,
            MSG_HAND_OVER_3 = 0x1A,     // "What are you still doing with those?!"
            MSG_GIFT_3 = 0x1B,
            MSG_HAND_OVER_4 = 0x20,     // "How many do you have?!"
            MSG_GIFT_4 = 0x21,
            MSG_HANGING_AROUND = 0x23,  // "...if all you're doing is just hanging around here..."
            MSG_JUST_YOU = 0x2A,        // + 0..18 even: "Oh! It's just you." after the scream before it
            MSG_GO_AWAY = 0x3D,         // "Wh-what do you want?! Go!"
            MSG_HAND_OVER_HIDING = 0x41, // at the new spot
            MSG_GIFT_HIDING = 0x42,
            MSG_SECRET_HIDING = 0x43,
            MSG_ANOTHER = 0x44,         // "There's no way you're carrying another knife and fork, right?"
            MSG_HAND_OVER_AGAIN = 0x46, // "Not AGAIN!"
            MSG_GIFT_AGAIN = 0x47,
            MSG_SWIPED = 0x48,          // "...I'll just assume you swiped that knife and fork"
            MSG_FOR_ME = 0x49,          // "...that knife and fork are for me, right?"
            MSG_HAND_OVER_MANY = 0x4D,  // "How many of those do you have?!"
            MSG_GIFT_MANY = 0x4E,
            // Random picks (control codes only)
            MSG_RAND_GO_AWAY = 0x5A,   // 0x24..0x26: "Just go, will you?!"
            MSG_RAND_SCREAM = 0x5B,    // 0x29..0x3B odd: a scream, then MSG_JUST_YOU
            MSG_RAND_COMPLAIN = 0x5E,  // 0x50..0x59: "This bites."
            MSG_RAND_SHAKEN = 0x61,    // 0x65..0x6C: "I'm so worked up..."
        };
        enum { ITEM_IDX_KNIFE_AND_FORK = 0x44 };

        talk_c(dAcNpcSpFranklin_c *npc);
        virtual ~talk_c();
        virtual void init();
        virtual void getMsgInfo(msgInfo_s *info);
        virtual void onMessageStart(int arg);
        virtual void onMessageEnd(int kind);
        virtual void onAnswer(int arg);
        virtual void preExecute();
        virtual void onTalkEnd();
        virtual void onRequestEnd(int kind);

        void start(dActor_c *target);
        void reqReceiveKnifeFork();
        void reqGiveGift();
        void takeKnifeFork();
        void pickUpGift();

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpFranklin_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd;
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ dItem::Item mItem; // the knife and fork, then his gift
        /* 0x10A */ u8 mTookKnifeFork;
    }; // size 0x10C

    dAcNpcSpFranklin_c() : mFranklinTalk(this) {}
    virtual ~dAcNpcSpFranklin_c() {}
    virtual void onDemoCmd(int cmd, u8 member);
    void changeState(int state);
    virtual int preCreate();
    virtual int create();
    virtual int doDelete();
    virtual int vtB0();
    virtual int canSpeak(dDemoActor_c *actor);
    virtual int getEarType();
    void initIdle();
    void executeIdle();
    void executeAtSpot(BOOL remote);
    void initLook();
    void stepLookTurn();
    void executeLook();
    void initTalk();
    void stepTalkTurn();
    void stepTalkTurnEnd();
    void executeTalk();
    void initRemote();
    void executeRemote();
    void initRemoteTalk();
    void executeRemoteTalk();
    bool isHarvestFestival();
    bool isHarvestFestivalNotStarted();
    bool isHarvestFestivalOver();
    void relocate();

    mAng getTalkAngle() const {
        dActor_c *target = mFranklinTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const { return l_turnSpeed; }

    struct daub_s {
        u32 mAtSpot : 1; // standing at the festival spot (mIsActive)
        u32 _0 : 31;
        u32 _4;
    }; // size 0x8

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dFranklinDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep[STATE_COUNT];
    /* 0x1C94 */ talk_c mFranklinTalk;
    /* 0x1DA0 */ int mIdleTimer;
    /* 0x1DA4 */ int mSpotTimer; // frames before the next setSpot
    /* 0x1DA8 */ daub_s mDaub;
}; // size 0x1DB0

#endif
