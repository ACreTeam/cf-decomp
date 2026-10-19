#ifndef D_A_NPC_SP_JINGLE_HPP
#define D_A_NPC_SP_JINGLE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes). The page has
// its own inline overrides (d_a_npc_sp_jingle_inline.hpp): no dDebugPage_c
// vtable in this unit.
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page.hpp>
#include <game/game/d_a_npc_sp.hpp>

// Jingle the black-nosed reindeer on Toy Day (REL d_a_npc_sp_jingleNP, profile
// A_NPC_SP_JINGLE, messages "npc_SP/NPC_jingle"). On Toy Day (EVENT_TOY_DAY)
// he stands at his spot (dSaveTown_c::mNpcSpotA: outdoors or in a villager's
// house, dNpcInfo::jingle_c) and wanders around it. A little quiz about the
// player's taste ends with a present (a carpet, wallpaper or furniture,
// FROM_JINGLE), once per look: he remembers the hat and accessory of every
// player he gave a present (jingle_c::push / contains) and moves on to a new
// spot afterwards.
//
// Class names from RTTI ("dAcNpcSpJingle_c", "dAcNpcSpJingle_c::talk_c",
// "dAcNpcSpJingle_c::dJingleDebugPage_c"); everything else is named from
// behaviour.
class dAcNpcSpJingle_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,        // wander around the spot
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    // The steps of STATE_LOOK / STATE_TALK (l_lookSteps / l_talkSteps).
    enum Step_e { STEP_START, STEP_DONE };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    enum {
        SPOT_RETRY_FRAMES = 300, // before the next setSpot
    };

    typedef void (dAcNpcSpJingle_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dJingleDebugPage_c : public dDebugPage_c {
      public:
        // Weak copies come out in reverse declaration order.
        virtual void open();
        virtual void close();
        virtual void execute();
        virtual BOOL isActive();
    };

    // The message receptor: the quiz and the present.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_jingle (from the ENG text).
        enum {
            MSG_FIRST = 0x01,   // "Happy holidays! I'm Jingle, the black-nosed
                                // reindeer!" (the first meeting)
            MSG_AGAIN = 0x02,   // "Oh, ho ho ho...hey. It's you again."
            MSG_NO_ROOM = 0x03, // "Gee, I'd really like to give you a
                                // present... But nope, I just can't."
                                // (pockets full)
            MSG_QUIZ = 0x05, // "Say, which do you like better: things you step
                             // on or things you lean on?"
            MSG_GIFT_CARPET = 0x0B, // "...I'll give you a gift right now!"
                                    // (+ 1: "Seeing you smile...")
            MSG_GIFT_FTR = 0x0E,
            MSG_GIFT_WALL = 0x11,
            MSG_GIFT_FTR_2 = 0x14,
            MSG_BUSY = 0x16,     // "What is it, kid? I'm in a rush here."
            MSG_FOOL_ME = 0x19,  // "Oh, ho ho ho...hey." (then "You can't fool
                                 // me!": the same look)
            MSG_SPOTTED = 0x1C,  // "Ah! I've been spotted..." (the same look,
                                 // at his new spot)
            MSG_LIMIT = 0x1F,    // + 0..2: "...you've already hit your present
                                 // limit, kid."
            MSG_NO_MORE = 0x22,  // + 0..2: "Hmm, what do you want? Sorry, I
                                 // don't have any more presents for you."
            MSG_NEW_LOOK = 0x25, // "Oh, ho ho ho...hey." (then "You look just
                                 // like someone I know")
            MSG_NEVER_MIND = 0x29,    // "Hmm, never mind. Now, if you'll excuse
                                      // me, I'm sorta busy."
            MSG_OH_HEY = 0x2A,        // "Oh, hey."
            MSG_NO_ROOM_2 = 0x2B,     // "Gee, I'd really like to give you a
                                      // present... Nope, I just can't."
            MSG_QUIZ_2 = 0x2D,        // the quiz again (a new look)
            MSG_GIFT_CARPET_2 = 0x33, // "All right then, I'm going to give you
                                      // a present!" (+ 1: "Boy, seeing you
                                      // smile...")
            MSG_GIFT_FTR_3 = 0x36,
            MSG_GIFT_WALL_2 = 0x39,
            MSG_GIFT_FTR_4 = 0x3C,
            MSG_LIMIT_2 = 0x3E,     // + 0..2: "Need something? Sorry, but
                                    // you've already hit your present limit."
            MSG_NO_MORE_2 = 0x41,   // + 0..2: "Hmm, what do you want? Sorry, I
                                    // don't have any more presents for you."
            MSG_LOOKALIKE = 0x45,   // "...a kid that looked just like you came
                                    // by earlier, so I gave <name> a gift."
            MSG_SCHEDULE = 0x46,    // "Look, like I said earlier, I've got a
                                    // schedule to keep here."
            MSG_SPOTTED_2 = 0x47,   // "Ah! I've been spotted..."
            MSG_LOOKALIKE_2 = 0x48, // MSG_LOOKALIKE, at his new spot
            MSG_LATE = 0x49,        // "...But I'm already late, and I have a
                                    // schedule to keep!"
            MSG_RUN_ALONG = 0x4A,   // "...maybe you should run along and
                                    // play?"
        };
        // mMeet: how the talk started (start).
        enum {
            MEET_MOVED,     // at his new spot after a present
            MEET_TOWN,      // in the town (SCENE_ATTR_OUTDOOR | MY_TOWN)
            MEET_ELSEWHERE, // in a house
            MEET_NONE,
        };

        talk_c(dAcNpcSpJingle_c *npc);            // 0xD88
        virtual ~talk_c();                        // 0xDE8
        virtual void init();                      // 0xE40
        virtual void getMsgInfo(msgInfo_s *info); // 0x12C4
        virtual void onMessageStart(int arg);     // 0x12DC
        virtual void onMessageEnd(int kind);      // 0x12E0
        virtual void onAnswer(int arg);           // 0x1494
        virtual void preExecute();                // 0x1498
        virtual void onTalkEnd();                 // 0x149C
        virtual void onRequestEnd(int kind);      // 0x14A0

        void startMoved();            // 0xEB0
        void startTown();             // 0x102C
        void startElsewhere();        // 0x1114
        void start(dActor_c *target); // 0x11FC
        void giveGift();              // 0x1504: the present of the quiz
        void onGiftEnd();             // 0x15E8

        /* 0x0EA */ u16 mMsgNo; // the first message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpJingle_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ int mMeet; // MEET_*
    }; // size 0x10C

    // The net (daub) state of the npc.
    struct daub_s {
        u32 mAtSpot : 1; // standing at his spot (mIsActive)
        u32 _0 : 31;
        u32 _4;
    }; // size 0x8

    dAcNpcSpJingle_c() : mJingleTalk(this) {
    }
    virtual ~dAcNpcSpJingle_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x1E0
    void changeState(int state);                // 0x2FC
    virtual int preCreate();                    // 0x334
    virtual int create();                       // 0x380
    virtual int doDelete();                     // 0x560
    virtual int vtB0();                         // 0x5F8
    void initIdle();                            // 0x830
    void executeIdle();                         // 0x834
    void initLook();                            // 0x8D0
    void stepLook();                            // 0x934
    void executeLook();                         // 0x994
    void initTalk();                            // 0x9D4
    void stepTalk();                            // 0xA6C
    void executeTalk();                         // 0xAB8
    void initRemote();                          // 0xAF8
    void executeRemote();                       // 0xB00
    void initRemoteTalk();                      // 0xBA0
    void executeRemoteTalk();                   // 0xBA4
    virtual BOOL searchWanderPos(mVec3_c *pos); // 0xBF8
    bool isToyDay();                            // 0xD00
    void relocate();                            // 0xD34
    virtual int getEarType();                   // 0x15EC

    mAng getTalkAngle() const {
        dActor_c *target = mJingleTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dJingleDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep;
    /* 0x1C90 */ talk_c mJingleTalk;
    /* 0x1D9C */ u8 _1D9C[0x10];
    /* 0x1DAC */ daub_s mDaub;
    /* 0x1DB4 */ int mSpotTimer; // frames before the next setSpot
}; // size 0x1DB8

#endif
