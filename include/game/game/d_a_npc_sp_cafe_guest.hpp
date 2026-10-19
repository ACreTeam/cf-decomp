#ifndef D_A_NPC_SP_CAFE_GUEST_HPP
#define D_A_NPC_SP_CAFE_GUEST_HPP

#include <game/game/d_a_npc_sp.hpp>

// The special npc guest at the Roost (REL d_a_npc_sp_cafe_guestNP, profile
// A_NPC_SP_CAFE_GUEST): sits at the counter and chats over a coffee. The
// scene data places it with the placeholder id SP_NPC_CAFE_GUEST; preCreate
// replaces that with today's guest of this time of day
// (dSaveCafeGuest_c::getCurrentGuest): Pelly, Phyllis, Kapp'n, Mr. Resetti
// or Don Resetti off duty, each with a message sheet npc_SP/NPC_cafe_*.
// Class names from RTTI ("dAcNpcSpCafeGuest_c",
// "dAcNpcSpCafeGuest_c::talk_c"); the rest is named from behaviour.
class dAcNpcSpCafeGuest_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_WAIT, // sit and look ahead
        STATE_LOOK, // a demo started (DEMO_CMD_START): look at the player
        STATE_TALK, // the talk (l_talkSteps)
        STATE_COUNT
    };
    // The steps of STATE_TALK (l_talkSteps).
    enum TalkStep_e { TALK_STEP_TALK, TALK_STEP_COUNT };
    enum Priority_e { PRIORITY_IDLE = 0 };

    // The head's look limits while seated (create).
    enum {
        LOOK_PITCH_MAX = 0x7FF,
        LOOK_YAW_MAX = 0x1800,
    };

    typedef void (dAcNpcSpCafeGuest_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: a greeting by how often the player met this guest
    // at the Roost, then chat for the rest of the day.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_cafe_periko / _perimi / _kappei / _reset / _racket (the
        // same codes in every sheet).
        enum {
            MSG_FIRST = 0x01,   // the first meeting at the Roost
            MSG_AGAIN = 0x02,   // met here one to three times before
            MSG_REGULAR = 0x03, // met here more often
            MSG_CHAT = 0x04,    // + 0..4: talked to today already
            MSG_CHAT_NUM = 5,
        };
        // The meetings that still count as MSG_AGAIN.
        enum { AGAIN_MIN = 1, AGAIN_MAX = 3 };

        talk_c(dAcNpcSpCafeGuest_c *npc);         // 0x658
        virtual ~talk_c();                        // 0x6B8
        virtual void init();                      // 0x710
        virtual void getMsgInfo(msgInfo_s *info); // 0x914
        virtual void onMessageStart(int arg);     // 0x9BC
        virtual void onMessageEnd(int kind);      // 0x9C0
        virtual void onAnswer(int arg);           // 0xA24
        virtual void preExecute();                // 0xA28
        virtual void onTalkEnd();                 // 0xA2C
        virtual void onRequestEnd(int kind);      // 0xB90

        void start(dActor_c *target); // 0x778

        /* 0x0EA */ u16 mMsgNo; // the first message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpCafeGuest_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks (never set here)
        /* 0x0FC */ Proc mOnRequestEnd;
    }; // size 0x108

    dAcNpcSpCafeGuest_c() : mGuestTalk(this) {
    }
    virtual ~dAcNpcSpCafeGuest_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x1CC
    void changeState(int state);                // 0x274
    virtual int preCreate();                    // 0x2AC
    virtual int create();                       // 0x3A8
    virtual int doDelete();                     // 0x450
    virtual int vtB0();                         // 0x47C
    virtual int draw();                         // 0x4D0
    void initWait();                            // 0x4D4
    void executeWait();                         // 0x52C
    void initLook();                            // 0x530
    void executeLook();                         // 0x558
    void initTalk();                            // 0x55C
    void stepTalk();                            // 0x5CC
    void executeTalk();                         // 0x618
    virtual int getEarType();                   // 0xBF4

    /* 0x1C7C */ int mState;
    /* 0x1C80 */ u8 mStep;
    /* 0x1C84 */ talk_c mGuestTalk;
}; // size 0x1D8C

#endif
