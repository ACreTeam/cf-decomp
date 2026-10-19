#ifndef D_A_NPC_SP_ASAMI_HPP
#define D_A_NPC_SP_ASAMI_HPP

#include <game/game/d_a_npc_sp.hpp>

// Sable at the Able Sisters' sewing machine (REL d_a_npc_sp_asamiNP, profile
// A_NPC_SP_ASAMI). She sews in time with the machine
// (daRmobjSewingMachine_c), barely looks up for strangers (Mabel is her talk
// partner), and opens up over the days she is talked to
// (dPrivateData_c::mSableTalkDays). In Gracie's story (getGracieStep 1 or 8)
// she has the "npc_SP/NPC_drama2" talks. Class names from RTTI
// ("dAcNpcSpAsami_c", "dAcNpcSpAsami_c::talk_c"); the rest is named from
// behaviour.
class dAcNpcSpAsami_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE,        // sew
        STATE_LOOK,        // look ahead (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    // mStep of STATE_TALK (l_talkSteps).
    enum TalkStep_e {
        TALK_STEP_TALK,     // talk, then turn back to the machine
        TALK_STEP_TURN_END, // back at the machine: sew again
        TALK_STEP_DONE
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpAsami_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The net daub data (getDaubData / setDaubData).
    struct daub_s {
        u32 mTiredFace : 1; // her surprised face (setTiredFace)
        u32 _0 : 31;
        u32 _4;
    }; // size 0x8

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_asami codes, named from the English text.
        enum {
            MSG_BUSY = 0x01,        // "......" (talked to on fewer than 4 days)
            MSG_BUSY_AGAIN = 0x02,  // 0x02..0x06: "I'm a bit busy right now..."
            MSG_DAY_4 = 0x07,       // days 4..9: 3 codes each, the first talk
            MSG_DAY_4_AGAIN = 0x08, // ... then one of the other two
            MSG_PARENTS = 0x19,     // day 10: "...The couple in that photo?"
            MSG_PARENTS_AGAIN = 0x1C, // 0x1C, 0x1D
            MSG_VISITOR = 0x1F,       // a visitor (dispatcher)
            MSG_NO_HOUSE = 0x27,      // "...I'm a bit busy right now..."
            MSG_FRIEND = 0x29,        // day 11 on (dispatcher)
            MSG_FRIEND_AGAIN =
                0x2A, // 0x2A..0x2E: "It's always so good to see you"
        };
        // npc_SP/NPC_drama2 codes (Gracie's story): the talk, the answer after
        // it (sets PRIVATE_FLAG0_DRAMA_SABLE_HEARD), the repeat.
        enum {
            MSG_DRAMA_1 = 0x01,       // "Aw..." (story step 1)
            MSG_DRAMA_1_AGAIN = 0x03, // "But it's times like these..."
            MSG_DRAMA_2 = 0x05,       // "Aw..." (story step 8)
            MSG_DRAMA_2_AGAIN = 0x07, // "I can't abandon this shop..."
        };
        enum { FEEL_TIRED = 0x16 }; // setSpeakerFeel: her tired face

        talk_c(dAcNpcSpAsami_c *npc);             // 0xD30
        virtual ~talk_c();                        // 0xD98
        virtual void onMessageStart(int arg);     // 0x1114
        virtual void onMessageEnd(int kind);      // 0x1118
        virtual void onAnswer(int arg);           // 0x1210
        virtual void init();                      // 0xDF0
        virtual void setSpeakerFeel(u32 feel);    // 0x1364
        virtual void preExecute();                // 0x1214
        virtual void onTalkEnd();                 // 0x125C
        virtual void getMsgInfo(msgInfo_s *info); // 0x10D4
        virtual void onRequestEnd(int kind);      // 0x1300
        virtual int getTurnFrame();               // 0x1474
        void start(dActor_c *target);             // 0xE70
        void closeMessage();                      // 0x13E0
        void turnToPlayer();                      // 0x13E8
        void waitTurn();                          // 0x1434

        /* 0x0EC */ Proc
            mOnMessageEnd; // one-shot hooks (cleared after the call)
        /* 0x0F8 */ Proc mOnRequestEnd;
        /* 0x104 */ Proc mOnPreExecute; // until it clears itself
        /* 0x110 */ u16 mMsgNo;
        /* 0x114 */ dAcNpcSpAsami_c *mpOwner;
        /* 0x118 */ u8 mDramaHeard;
    }; // size 0x11C

    dAcNpcSpAsami_c() : mAsamiTalk(this) {
    }
    virtual ~dAcNpcSpAsami_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x1CC
    void changeState(int state);                // 0x2E8
    virtual int preCreate();                    // 0x320
    virtual int create();                       // 0x3A8
    virtual int doDelete();                     // 0x534
    virtual int vtB0();                         // 0x560
    void setTiredFace();                        // 0x734
    void resetFace();                           // 0x7D4
    void initIdle();                            // 0x874
    void executeIdle();                         // 0x878
    void initLook();                            // 0x87C
    void executeLook();                         // 0x8F8
    void initTalk();                            // 0x8FC
    void stepTalk();                            // 0xA18
    void stepTalkTurnEnd();                     // 0xAD0
    void executeTalk();                         // 0xB94
    void initRemote();                          // 0xBD4
    void executeRemote();                       // 0xBD8
    void initRemoteTalk();                      // 0xC6C
    void executeRemoteTalk();                   // 0xC70
    void turnToPlayer();                        // 0xCC4
    virtual int getEarType();                   // 0x1488

    mAng getTalkAngle() const {
        dActor_c *target = mAsamiTalk.mpActActor;
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
    /* 0x1C80 */ daub_s mDaub;
    /* 0x1C88 */ u8 mStep;
    /* 0x1C8C */ talk_c mAsamiTalk;
    /* 0x1DA8 */ u8 mDrama; // Gracie's story talk (getGracieStep 1 or 8)
}; // size 0x1DAC

#endif
