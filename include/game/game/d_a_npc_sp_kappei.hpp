#ifndef D_A_NPC_SP_KAPPEI_HPP
#define D_A_NPC_SP_KAPPEI_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (m3d,
// fBase_c, dDvd, dActor_c, then the npc classes and daRmobj_c::model_c).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_a_rmobj.hpp>

// Kapp'n driving the bus (REL d_a_npc_sp_kappeiNP, profile A_NPC_SP_KAPPEI;
// item key SP_NPC_KAPPN). He sits at the wheel (/Rmobj obj_bus_handl, the
// steering wheel) in the bus scenes: SCENE_DM_BUS_TO_TOWN (to the city) and
// SCENE_DM_BUS_TO_LAND (back to the town). On the ride he talks to the player
// (npc_SP/NPC_kappeiM / NPC_kappeiF by the player's gender): the camera turns
// to the side view, he tells one of his stories (the message feel tags turn
// the wheel), then the camera turns back and the bus arrives
// (dSTR::requestBus). In the player-create bus (SCENE_DM_BUS_PL_CRT) he only
// sits there.
//
// Class names from RTTI ("dAcNpcSpKappei_c", "dAcNpcSpKappei_c::talk_c");
// everything else is named from behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor, then
// talk_c).
class dAcNpcSpKappei_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,  // ask for the talk (dDemo::requestSpeak) on a ride
        STATE_WAIT,  // a demo started (DEMO_CMD_START)
        STATE_TALK,  // the ride's talk (l_talkSteps)
        STATE_DONE,  // the ride is over
        STATE_SPEAK, // DEMO_CMD_SPEAK: wait until he stands still, then talk
        STATE_COUNT
    };
    // The steps of STATE_TALK (l_talkSteps).
    enum TalkStep_e {
        TALK_STEP_SIDE_VIEW, // the camera turns to the bus's side view
        TALK_STEP_START,     // after the camera move: start the talk
        TALK_STEP_TALK,      // the talk
        TALK_STEP_ARRIVE,    // the camera turns back, the bus arrives
        TALK_STEP_COUNT
    };
    // The steps of STATE_SPEAK (l_speakSteps).
    enum SpeakStep_e { SPEAK_STEP_WAIT, SPEAK_STEP_COUNT };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpKappei_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: the story of the ride.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_kappeiM / NPC_kappeiF (from the ENG text).
        enum {
            MSG_FIRST_RIDE = 0x01, // "Well ahoy there, matey." (the first ride)
            MSG_RODE_BEFORE =
                0x05, // "Ye rode me bus the other day, didn't ye?"
            MSG_FIRST_RETURN = 0x06, // "Ye rode me bus out to the big city
                                     // earlier, did ye not?"
            MSG_RETURN_FORTUNES =
                0x07,           // "So, how'd yer fortunes fare in thar?"
            MSG_TO_CITY = 0x0B, // + 0..9: the stories on the way to the city
            MSG_TO_TOWN = 0x15, // + 0..9: the stories on the way back
            MSG_STORY_NUM = 10,
        };
        // The message feel tags (setSpeakerFeel) that turn the wheel.
        enum {
            FEEL_NONE = 0x00,    // back to the straight wheel
            FEEL_WHEEL_1 = 0x30, // obj_bus_handl_1 (then _2)
            FEEL_WHEEL_2 = 0x31, // obj_bus_handl_3 (then _4)
            FEEL_WHEEL_3 = 0x32, // obj_bus_handl_5 (then _6)
        };

        talk_c(dAcNpcSpKappei_c *npc);            // 0x9F4
        virtual ~talk_c();                        // 0xA54
        virtual void init();                      // 0xAAC
        virtual void getMsgInfo(msgInfo_s *info); // 0xCA0
        virtual void onMessageStart(int arg);     // 0xD08
        virtual void onMessageEnd(int kind);      // 0xD1C
        virtual void onAnswer(int arg);           // 0xD80
        virtual void preExecute();                // 0xD84
        virtual void onTalkEnd();                 // 0xD88
        virtual void setSpeakerFeel(u32 feel);    // 0xDB0
        virtual void onKeyWait();                 // 0xEA4
        virtual void onPageWait();                // 0xEC8
        virtual void onTextEnd();                 // 0xEEC
        virtual void onSelectStart();             // 0xF10
        virtual void onTypeStart();               // 0xF34
        virtual void onRequestEnd(int kind);      // 0xF48

        void start(dActor_c *target); // 0xB18

        /* 0x0EA */ u16 mMsgNo; // the first message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpKappei_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks (never set here)
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ u32 mFeel; // the last feel tag (FEEL_*)
    }; // size 0x10C

    dAcNpcSpKappei_c() : mBusTalk(this) {
    }
    virtual ~dAcNpcSpKappei_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x298
    void changeState(int state);                // 0x364
    virtual int preCreate();                    // 0x39C
    virtual int create();                       // 0x40C
    virtual int doDelete();                     // 0x4F0
    virtual int vtB0();                         // 0x544
    virtual int draw();                         // 0x638
    void initIdle();                            // 0x670
    void executeIdle();                         // 0x674
    void initWait();                            // 0x6DC
    void executeWait();                         // 0x6E0
    void initTalk();                            // 0x6E4
    void stepSideView();                        // 0x6FC
    void stepStartTalk();                       // 0x764
    void stepTalk();                            // 0x7CC
    void stepArrive();                          // 0x824
    void executeTalk();                         // 0x910
    void initDone();                            // 0x950
    void executeDone();                         // 0x954
    void initSpeak();                           // 0x958
    void executeSpeak();                        // 0x99C
    void stepSpeakWait();                       // 0x9DC

    /* 0x1C7C */ int mState;
    /* 0x1C80 */ u8 mStep;
    /* 0x1C84 */ talk_c mBusTalk;
    /* 0x1D90 */ daRmobj_c::model_c mWheelMdl; // "obj_bus_handl"
    /* 0x1F3C */ int mSideViewTimer;           // frames until the side view
    /* 0x1F40 */ int mReturnViewTimer; // after the talk: until the camera
                                       // turns back
    /* 0x1F44 */ int mArriveTimer;     // after the talk: until the arrival
}; // size 0x1F48

#endif
