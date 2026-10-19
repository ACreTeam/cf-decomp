#ifndef D_A_NPC_SP_GODDESS_HPP
#define D_A_NPC_SP_GODDESS_HPP

#include <game/game/d_a_npc_sp.hpp>

// The fountain goddess (REL d_a_npc_sp_goddessNP, profile A_NPC_SP_GODDESS,
// SP_NPC_GODDESS; d_a_str_field spawns her when an axe is thrown into the
// fountain). She speaks to the player (dDemo::requestSpeak), rises out of the
// fountain ("af_gds_appear", HMN_ANM_GDS_APPEAR1), talks ("npc_SP/NPC_goddess":
// the gold / silver axe questions), hands an axe back and sinks again. Class
// names from RTTI ("dAcNpcSpGoddess_c", "dAcNpcSpGoddess_c::talk_c"); the rest
// is named from behaviour.
class dAcNpcSpGoddess_c : public dAcNpcSp_c {
  public:
    enum State_e {
        STATE_WAIT,   // wait for the axe in the fountain, then speak
        STATE_APPEAR, // rise out of the fountain and turn to the player
        STATE_TALK,   // talk, then sink back into the fountain
        STATE_COUNT
    };
    // STATE_APPEAR steps (l_appearSteps).
    enum AppearStep_e {
        APPEAR_STEP_WAIT_DEMO, // wait for the player to be ready
        APPEAR_STEP_RISE,      // the appear timer runs
        APPEAR_STEP_TURN,      // turn to the player, then STATE_TALK
        APPEAR_STEP_NUM
    };
    // STATE_TALK steps (l_talkSteps).
    enum TalkStep_e {
        TALK_STEP_TALK,  // wait for the talk to end
        TALK_STEP_LEAVE, // the leave timer runs
        TALK_STEP_NUM
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpGoddess_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: picks the greeting and hands the axe over at the
    // message ends.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();
        // "npc_SP/NPC_goddess" message codes (from the ENG text).
        enum {
            MSG_AXE_QUESTION = 0x0B,  // "did you throw a gold axe in here? Or
                                      // maybe a silver axe?"
            MSG_HOW_DO_I_LOOK = 0x3D, // "How do I look today?"
            MSG_SIGH = 0x83,          // "Sigh... Well? What now?!"
        };
        // onMessageEnd kinds (BMG entry +0xA).
        enum {
            END_GIVE_GOLD_AXE = 0x10,    // "I'm giving you a gold axe!"
            END_GIVE_SILVER_AXE = 0x11,  // "take a silver axe instead!"
            END_RETURN_AXE = 0x12,       // "here's your axe back"
            END_RETURN_AXE_ANGRY = 0x13, // "take your axe and go"
            END_LEAVE = 0x14,            // "b'bye, now!": nothing to hand over
        };

        talk_c(dAcNpcSpGoddess_c *npc);           // 0xAA4
        virtual ~talk_c();                        // 0xB10
        virtual void init();                      // 0xB68
        void start(dActor_c *target);             // 0xBF4
        virtual void getMsgInfo(msgInfo_s *info); // 0xCA8
        virtual void onMessageStart(int arg);     // 0xCC0
        virtual void onMessageEnd(int kind);      // 0xCC4
        virtual void onAnswer(int arg);           // 0xDF8
        virtual void preExecute();                // 0xDFC
        virtual void onTalkEnd();                 // 0xE44
        virtual void onRequestEnd(int kind);      // 0xE94

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpGoddess_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // hooks, never set in this REL;
                                        // cleared after the call except
                                        // mOnPreExecute
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ Proc mOnPreExecute;
        /* 0x114 */ dItem::Item mItem; // the axe handed to the player
    }; // size 0x118

    dAcNpcSpGoddess_c() : mGoddessTalk(this) {
    }
    virtual ~dAcNpcSpGoddess_c() {
    } // 0xFBC
    virtual void onDemoCmd(int cmd, u8 member); // 0x1CC
    void changeState(int state);                // 0x250
    virtual int preCreate();                    // 0x288
    virtual int create();                       // 0x2D4
    virtual int doDelete();                     // 0x33C
    virtual int vtB0();                         // 0x368
    virtual int draw();                         // 0x3DC
    void initWait();                            // 0x514
    void executeWait();                         // 0x528
    void initAppear();                          // 0x578
    void startAppear();                         // 0x57C
    void stepAppearWaitDemo();                  // 0x5B0
    void startRise();                           // 0x5F0
    void stepAppearRise();                      // 0x630
    void startTurn();                           // 0x748
    void stepAppearTurn();                      // 0x7AC
    void executeAppear();                       // 0x7D0
    void initTalk();                            // 0x810
    void startTalk();                           // 0x814
    void stepTalk();                            // 0x898
    void startLeave();                          // 0x8D8
    void stepLeave();                           // 0x910
    void executeTalk();                         // 0xA64
    virtual int getEarType();                   // 0xEF8

    mAng getTalkAngle() const {
        dActor_c *target = mGoddessTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mGoddessTalk;
    /* 0x1D9C */ int mAppearTimer;   // counts down while she rises
    /* 0x1DA0 */ int mLeaveTimer;    // counts down while she sinks
    /* 0x1DA4 */ int mLeaveEndTimer; // then this one until the talk ends
    /* 0x1DA8 */ u8 mTevAlpha;       // TEVPREV alpha of the "e" / "b" materials
                                     // (255 -> 0 rising, 0 -> 255 sinking)
    /* 0x1DA9 */ u8 mShow;           // mIsActive follows it (vtB0)
}; // size 0x1DAC

#endif
