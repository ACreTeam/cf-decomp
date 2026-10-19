#ifndef D_A_NPC_SP_PERIO_HPP
#define D_A_NPC_SP_PERIO_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes). There is no
// dDebugPage_c vtable: dPerioDebugPage_c overrides all of its virtuals (as in
// d_a_npc_sp_grace).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page.hpp>
#include <game/game/d_a_npc_sp.hpp>

// Pete the mail carrier (REL d_a_npc_sp_perioNP, profile A_NPC_SP_PERIO,
// messages "npc_SP/NPC_perio"). He wanders around town at 9:00 and 17:00
// (until half past) and chats. Right after the game was loaded with a
// distribution letter waiting (PRIVATE_FLAG0_DIST_MAIL_PENDING; no villager
// throws a birthday party then) he waits at the player's house, calls the
// player over and hands the letter over (dPostOffice::getDistMail).
//
// Class names from RTTI ("dAcNpcSpPerio_c", "dAcNpcSpPerio_c::talk_c",
// "dAcNpcSpPerio_c::dPerioDebugPage_c"); everything else is named from
// behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dPerioDebugPage_c), so dPerioDebugPage_c is declared first.
class dAcNpcSpPerio_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_DELIVER,      // the letter: call the player over
        STATE_IDLE,         // wander
        STATE_LOOK,         // turn to the player (demo start)
        STATE_DELIVER_WALK, // the player walks up to him
        STATE_TALK,         // talk to the player
        STATE_REMOTE,       // another console owns the npc
        STATE_REMOTE_TALK,  // another console's player talks to the npc
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // mStep in STATE_LOOK (l_lookSteps) and STATE_TALK (l_talkSteps).
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_DONE };
    enum TalkStep_e { TALK_STEP_TALK, TALK_STEP_DONE };
    // Mail is delivered from 9:00 and from 17:00, for half an hour.
    enum {
        ROUND_HOUR_MORNING = 9,
        ROUND_HOUR_EVENING = 17,
        ROUND_MINUTES = 30,
    };

    typedef void (dAcNpcSpPerio_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dPerioDebugPage_c : public dDebugPage_c {
      public:
        // Weak copies come out in reverse declaration order.
        virtual void open();
        virtual void close();
        virtual void execute();
        virtual BOOL isActive();
    };

    // The message receptor: the chats and the letter.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // The talks (mKind).
        enum Kind_e {
            KIND_TALK,    // the player talks to him
            KIND_DELIVER, // he calls the player over with the letter
        };

        // Message codes of npc_SP/NPC_perio (from the ENG text).
        enum {
            MSG_FIRST_MEETING = 0x01, // "Oh? Hey there! It's not every day..."
            MSG_CHAT = 0x02,          // 0x02..0x0B: the chats
            MSG_CHAT_NUM = 10,
            MSG_BUSY = 0x0C, // 0x0C..0x10: "I gotta get these deliveries..."
            MSG_BUSY_NUM = 5,
            MSG_LETTER_FULL = 0x1F, // "...your hands are full, so you can't
                                    // take it right now."
            MSG_LETTER = 0x20,      // "You have a letter from <sender>!"
            MSG_LETTER_DONE = 0x21, // "OK, looks like you're all set!"
        };

        talk_c(dAcNpcSpPerio_c *npc);             // 0xA40
        virtual ~talk_c();                        // 0xAA8
        virtual void init();                      // 0xB00
        virtual void getMsgInfo(msgInfo_s *info); // 0xC5C
        virtual void onMessageStart(int arg);     // 0xC74
        virtual void onMessageEnd(int kind);      // 0xCE0
        virtual void onAnswer(int arg);           // 0xD98
        virtual void preExecute();                // 0xD9C
        virtual void onTalkEnd();                 // 0xDA0
        virtual void onRequestEnd(int kind);      // 0xE00

        void start(dActor_c *target); // 0xB68
        void giveLetter();            // 0xE64 (mOnMessageEnd)
        void onLetterGiven();         // 0xEAC (mOnRequestEnd, empty)

        /* 0x0EC */ int mKind; // Kind_e
        /* 0x0F0 */ u16 mMsgNo;
        /* 0x0F4 */ dAcNpcSpPerio_c *mpOwner;
        /* 0x0F8 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x104 */ Proc mOnRequestEnd;
    }; // size 0x110

    dAcNpcSpPerio_c() : mPerioTalk(this) {
    }
    virtual ~dAcNpcSpPerio_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x1D8
    void changeState(int state);                // 0x328
    virtual int preCreate();                    // 0x360
    virtual int create();                       // 0x3AC
    virtual int doDelete();                     // 0x578
    virtual int vtB0();                         // 0x5A4
    void initDeliver();                         // 0x5EC
    void executeDeliver();                      // 0x5F0
    void initIdle();                            // 0x5F8
    void executeIdle();                         // 0x5FC
    void initLook();                            // 0x610
    void stepLook();                            // 0x6A0
    void executeLook();                         // 0x700
    void initDeliverWalk();                     // 0x740
    void executeDeliverWalk();                  // 0x7A4
    void initTalk();                            // 0x7F0
    void stepTalk();                            // 0x888
    void executeTalk();                         // 0x8F8
    void initRemote();                          // 0x938
    void executeRemote();                       // 0x940
    void initRemoteTalk();                      // 0x9E8
    void executeRemoteTalk();                   // 0x9EC
    virtual int getEarType();                   // 0xEB0

    mAng getTalkAngle() const {
        dActor_c *target = mPerioTalk.mpActActor;
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
    /* 0x1C84 */ dPerioDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep;
    /* 0x1C90 */ talk_c mPerioTalk;
    /* 0x1DA0 */ u8 mDelivering; // here with the distribution letter
    /* 0x1DA1 */ u8 mFieldMuted; // the field BGM is muted for the letter
}; // size 0x1DA4

#include <game/game/d_a_npc_sp_perio_inline.hpp>

#endif
