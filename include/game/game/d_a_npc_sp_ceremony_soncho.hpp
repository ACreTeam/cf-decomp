#ifndef D_A_NPC_SP_CEREMONY_SONCHO_HPP
#define D_A_NPC_SP_CEREMONY_SONCHO_HPP

#include <game/game/d_a_npc_sp.hpp>

// Tortimer at the opening ceremony of a new construction (REL
// d_a_npc_sp_ceremony_sonchoNP, profile A_NPC_SP_CEREMONY_SONCHO): the bridge,
// fountain, windmill or lighthouse of dSaveTown_c::mNewConstruction. He wanders
// around his block (at the fountain he stays at his home position) and talks
// about the construction ("npc_SP/NPC_soncho_0"). Class names from RTTI
// ("dAcNpcSpCeremonySoncho_c", "dAcNpcSpCeremonySoncho_c::talk_c"); the rest
// is named from behaviour.
class dAcNpcSpCeremonySoncho_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE,        // wander
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    // mStep[STATE_LOOK] (l_lookSteps) and mStep[STATE_TALK] (l_talkSteps).
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_DONE };
    enum TalkStep_e {
        TALK_STEP_TALK,     // talk, then (fountain) turn back south
        TALK_STEP_TURN_END, // wait for the turn, finish the talk
        TALK_STEP_DONE
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpCeremonySoncho_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The net daub data (getDaubData / setDaubData).
    struct daub_s {
        wanderDaub_s mWander;
        u32 _4;
    }; // size 0x8

    // The message receptor: the ceremony message of the construction.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        // npc_SP/NPC_soncho_0 codes, named from the English text: the
        // announcement of each opening ceremony; the code after it is the
        // dispatcher of the later talks (testTalked).
        enum {
            MSG_BRIDGE =
                0x15, // "Good news...Our new bridge is finally finished!"
            MSG_FOUNTAIN = 0x1A,   // "...Our new fountain is finally done!"
            MSG_LIGHTHOUSE = 0x1F, // "...Our new lighthouse is finally done!"
            MSG_WINDMILL = 0x24,   // "...Our new windmill is finally done!"
        };

        talk_c(dAcNpcSpCeremonySoncho_c *npc);    // 0xAE8
        virtual ~talk_c();                        // 0xB48
        virtual void onMessageStart(int arg);     // 0xC8C
        virtual void onMessageEnd(int kind);      // 0xC90
        virtual void onAnswer(int arg);           // 0xC94
        virtual void init();                      // 0xBA0
        virtual void preExecute();                // 0xC98
        virtual void onTalkEnd();                 // 0xC9C
        virtual void getMsgInfo(msgInfo_s *info); // 0xC74
        void start(dActor_c *target);             // 0xBA4

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpCeremonySoncho_c *mpOwner;
    }; // size 0xF0

    dAcNpcSpCeremonySoncho_c() : mCeremonyTalk(this) {
    }
    virtual ~dAcNpcSpCeremonySoncho_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x1CC
    void changeState(int state);                // 0x2E8
    virtual int preCreate();                    // 0x320
    virtual int create();                       // 0x36C
    virtual int doDelete();                     // 0x478
    virtual int vtB0();                         // 0x4A4
    void initIdle();                            // 0x544
    void executeIdle();                         // 0x548
    void initLook();                            // 0x59C
    void stepLook();                            // 0x600
    void executeLook();                         // 0x660
    void initTalk();                            // 0x6A0
    void stepTalk();                            // 0x73C
    void stepTalkTurnEnd();                     // 0x7D0
    void executeTalk();                         // 0x820
    void initRemote();                          // 0x860
    void executeRemote();                       // 0x868
    void initRemoteTalk();                      // 0x908
    void executeRemoteTalk();                   // 0x90C
    virtual BOOL searchWanderPos(mVec3_c *pos); // 0x960: in the home block
    virtual BOOL isWanderWalk();                // 0xA84: only while waiting
    virtual int vtE4();                         // 0xAE0

    mAng getTalkAngle() const {
        dActor_c *target = mpRecept->mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C */ talk_c mCeremonyTalk;
    /* 0x1D6C */ int mBlockX; // the block at create
    /* 0x1D70 */ int mBlockZ;
    /* 0x1D74 */ int mState;
    /* 0x1D78 */ u8 mStep[STATE_COUNT];
    /* 0x1D80 */ daub_s mDaub;
}; // size 0x1D88

#endif
