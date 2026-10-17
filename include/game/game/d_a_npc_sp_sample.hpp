#ifndef D_A_NPC_SP_SAMPLE_HPP
#define D_A_NPC_SP_SAMPLE_HPP

#include <game/game/d_a_npc_sp.hpp>

// The sample special npc (REL d_a_npc_sp_sampleNP, profile A_NPC_SP_SAMPLE):
// uses Tom Nook's messages
// ("npc_SP/NPC_tanukichi" 1..3 in turn), follows the player around and talks.
// Class names from RTTI
// ("dAcNpcSpSample_c", "dAcNpcSpSample_c::talk_c"); the rest is named from
// behaviour.
class dAcNpcSpSample_c : public dAcNpcSp_c {
  public:
    // Inferred state names (l_states order).
    enum State_e {
        STATE_IDLE,        // wait; follow when the player is far
        STATE_FOLLOW,      // walk to the player until near
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpSample_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: starts the next of three sample messages.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();
        enum { MSG_NUM = 3 };

        talk_c(dAcNpcSpSample_c *npc);            // 0x868
        virtual ~talk_c();                        // 0x8C8
        virtual void onMessageStart(int arg);     // 0xA38
        virtual void onMessageEnd(int kind);      // 0xA9C
        virtual void onAnswer(int arg);           // 0xAE4
        virtual void init();                      // 0x920
        virtual void preExecute();                // 0xB48
        virtual void onTalkEnd();                 // 0xB4C
        virtual void getMsgInfo(msgInfo_s *info); // 0xA20
        virtual void onRequestEnd(int kind);      // 0xB50
        void start(dActor_c *target);             // 0x9B8

        /* 0x0EA */ u16 mMsgNo; // 1..MSG_NUM, next message code
        /* 0x0EC */ dAcNpcSpSample_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageStart; // one-shot hooks (cleared after the
                                          // call), except onMessageEnd
        /* 0x0FC */ Proc mOnMessageEnd;
        /* 0x108 */ Proc mOnAnswer;
        /* 0x114 */ Proc mOnRequestEnd;
    }; // size 0x120

    dAcNpcSpSample_c() : mSampleTalk(this) {
    }
    virtual ~dAcNpcSpSample_c() {
    } // 0xC58
    virtual void onDemoCmd(int cmd, u8 member); // 0x1CC
    void changeState(int state);                // 0x2E8
    virtual int preCreate();                    // 0x320
    virtual int create();                       // 0x36C
    virtual int doDelete();                     // 0x3D8
    virtual int vtB0();                         // 0x404
    void initIdle();                            // 0x44C
    void executeIdle();                         // 0x460
    void initFollow();                          // 0x4B0
    void executeFollow();                       // 0x514
    void initLook();                            // 0x57C
    void stepLook();                            // 0x5E0
    void executeLook();                         // 0x640
    void initTalk();                            // 0x680
    void stepTalk();                            // 0x6EC
    void executeTalk();                         // 0x738
    void initRemote();                          // 0x778
    void executeRemote();                       // 0x77C
    void initRemoteTalk();                      // 0x810
    void executeRemoteTalk();                   // 0x814

    mAng getTalkAngle() const {
        dActor_c *target = mSampleTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mSampleTalk;
}; // size 0x1DA4

#endif
