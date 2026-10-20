#ifndef D_A_NPC_SP_PLAYER_CREATE_HPP
#define D_A_NPC_SP_PLAYER_CREATE_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_state.hpp>

// The player create npc (REL d_a_npc_sp_player_createNP, profile
// A_NPC_SP_PLAYER_CREATE): Rover on the bus. In the opening ride
// (SCENE_DM_BUS_PL_CRT) he asks the new player for the date and time, the name,
// the gender and the town, picks the player's looks from the answers and
// creates the save; in the ride to the town after a WiiConnect24 event
// (SCENE_DM_BUS_TO_TOWN) he explains the WiiConnect24 setting. Kapp'n (the bus
// driver) speaks the stop announcements. Class names from RTTI
// ("dAcNpcSpPlayerCreate_c", "...::stepDemoBus_c", "...::talk_c"); mode and
// step names from the original CP932 state labels; everything else is named
// from behaviour.
class dAcNpcSpPlayerCreate_c : public dAcNpcSp_c,
                               public dState::mode_c<dAcNpcSpPlayerCreate_c> {
  public:
    typedef dState::base_c<dAcNpcSpPlayerCreate_c>::Method Method;

    // dAcNpc_c::action_c request priorities.
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    // The message receptor (RTTI "dAcNpcSpPlayerCreate_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c, public dMsgRcptExp_c<talk_c> {
      public:
        typedef dState::base_c<talk_c>::Method Method;

        // sys_MESS/SYS_Bus message codes, named from the English text.
        enum {
            MSG_INTRO = 0x01,    // "The first step in starting your new life is
                                 // getting on the bus!"
            MSG_INTRO_JP = 0x02, // Japanese only (empty in English)
            MSG_JP_03 = 0x03,    // Japanese only (empty in English)
            MSG_JP_04 = 0x04,    // Japanese only (empty in English)
            MSG_TIME_CHECK = 0x05,       // "Is it ... today?"
            MSG_TIME_WRONG = 0x06,       // "It's not?!" (the time menu)
            MSG_TIME_CHECK_AGAIN = 0x07, // "Okeydokey, so it's ..."
            MSG_TIME_WRONG_AGAIN = 0x08, // "Did I hear you wrong?"
            MSG_TIME_OK = 0x09,          // "That's right, then? Sweet."
            MSG_TIME_THANKS = 0x0A,      // "Thanks! I don't know WHAT is up
                                         // with this watch."
            MSG_ASK_NAME = 0x0B,         // "I was about to ask what your name
                                         // was!" (the name menu)
            MSG_NAME_CHECK = 0x0C,       // "Oh, ..., is it?"
            MSG_NAME_SAME = 0x0D,  // another player of the town has the name
            MSG_NAME_WRONG = 0x0E, // "Did I get it wrong?" (the name menu)
            MSG_NAME_CHECK_AGAIN = 0x0F, // "Ah, ..., gotcha."
            MSG_NAME_BOY = 0x10,         // "It's fun, yet masculine"
            MSG_NAME_GIRL = 0x11,        // "It's fun and feminine!"
            MSG_SORRY_GIRL = 0x12,       // "you're a girl, aren't you?"
            MSG_SORRY_BOY = 0x13,        // "You're a boy?"
            MSG_ASK_TOWN = 0x14,         // "What's it called?" (the town menu)
            MSG_TOWN_CHECK = 0x15,
            MSG_TOWN_WRONG = 0x16,   // "I need to clean these ears"
            MSG_TOWN_EXCITED = 0x17, // "You must be so excited about your
                                     // new life there!"
            MSG_LOOK_FIRST = 0x1F,   // the answers that pick the player's looks
            MSG_LOOK_LAST = 0x26,    // (eight, l_lookSets)
            MSG_MAP = 0x27,          // "Here, check this out!" (gives the map)
            MSG_MAP_GIVEN = 0x28,
            MSG_ENVY = 0x29, // "the thought of you moving to ... and living
                             // on your own"
            MSG_NEXT_STOP = 0x33,     // Kapp'n: "Yar, next stop..."
            MSG_ABOUT_TO_STOP = 0x34, // "We're about to stop at ...!"
            MSG_JUST_MADE_IT = 0x3B,  // "Whew! Just made it!"
            MSG_ASK_TOWN_DS = 0x3D,   // "this town you're moving to... What's
                                      // it called?" (the town menu)
            MSG_HOUSING_BOOM = 0x40,
            MSG_MAP_DS = 0x43, // "Oh, yeah! Check this out!" (gives the map)
            MSG_MAP_DS_GIVEN = 0x44,
            MSG_ENVY_DS = 0x45,
            MSG_RECOGNIZE = 0x4A, // "Oh...?" (the WiiConnect24 ride)
            MSG_WC24_ON = 0x4C,   // "Got it! I'll get things ready"
            MSG_WC24_OFF = 0x4D,  // "OK, I won't activate this feature"
        };

        // onMessageEnd kinds (the BMG entries' end request).
        enum {
            END_REQ_TIME_MENU = 0x0B, // reqMenu06
            END_REQ_TOWN_MENU = 0x10, // reqMenu0C
            END_REQ_NAME_MENU = 0x11, // reqMenu0B
            END_REQ_SHOW_MAP = 0x12,  // reqMenu05: the town map preview (MENU_PREVIEW_MAP)
        };

        talk_c(dAcNpcSpPlayerCreate_c *npc); // 0x1324
        virtual ~talk_c() {
        } // weak
        virtual void onMessageStart(int arg);     // 0x14C0
        virtual void onMessageEnd(int kind);      // 0x1640
        virtual void onAnswer(int answer);        // 0x1FB8
        virtual void init();                      // 0x1408
        virtual void preExecute();                // 0x2140
        virtual void onTalkEnd();                 // 0x2964
        virtual void getMsgInfo(msgInfo_s *info); // 0x14A8
        virtual void onRequestEnd(int kind);      // 0x2AC8

        void start(dActor_c *target); // 0x1458

        // ---- message handlers (onMessageStart / onMessageEnd / onAnswer
        // tables; code = SYS_Bus message) ----
        void setTimeWord();          // 0x1580 (start 0x05 0x07)
        void unmuteBusBgm();         // 0x162C (start 0x33)
        void enableNet();            // 0x183C (end 0x4C)
        void disableNet();           // 0x1840 (end 0x4D)
        void startIntro();           // 0x1844 (end 0x3B)
        void askFirstQuestion();     // 0x18A0 (end 0x01)
        void startRecognize();       // 0x1938 (end 0x4A)
        void askNameOrTime();        // 0x1984 (end 0x03 0x04)
        void askNameAfterTime();     // 0x19D8 (end 0x09 0x0A)
        void askTown();              // 0x1A2C (end 0x12 0x13)
        void setPlayerLook();        // 0x1A80 (end 0x1F..0x26)
        void startCreateSave();      // 0x1B44 (end 0x17 0x29 0x40 0x45)
        void requestDriverSpeaker(); // 0x1CA0 (end 0x28 0x44)
        void startAnnounceTurn();    // 0x1CEC (end 0x33)
        void startHeadTurn();        // 0x1E5C (end 0x34)
        void setKanaOff();           // 0x2098 (answer 0x02/1)
        void setKanaOn();            // 0x20A0 (answer 0x02/0)
        void setMale();              // 0x20A8 (answer 0x10/0 0x11/1)
        void setFemale();            // 0x20F4 (answer 0x10/1 0x11/0)

        // ---- modes (the table in the constructor) ----
        void initWait();            // 0x2190
        void executeWait();         // 0x2194
        void initCreateSave();      // 0x2198
        void executeCreateSave();   // 0x2228
        void initCamera();          // 0x23A8
        void executeCamera();       // 0x23AC
        void initAnnounceTurn();    // 0x2524
        void executeAnnounceTurn(); // 0x255C
        void initHeadTurn();        // 0x2754
        void executeHeadTurn();     // 0x278C

        // ---- request-end methods (mRequestEndMethod) ----
        void startFrontView();    // 0x2B60: the bus front camera
        void showDriverSpeaker(); // 0x2CDC: Kapp'n speaks
        void showRoverSpeaker();  // 0x2D6C
        void reloadSeason();      // 0x2DFC: after the time menu
        void checkName();         // 0x2E00: after the name menu

        /* 0x0EC dMsgRcptExp_c<talk_c> */
        /* 0x128 */ u32 _128;
        /* 0x12C */ int mSaveMode; // dSvProc_c::MODE_NEW_*: executeCreateSave
        /* 0x130 */ u16 mMsgCode;  // getMsgInfo
        /* 0x134 */ dAcNpcSpPlayerCreate_c *mpOwner;
    }; // size 0x138

    // The bus demo: wait for the fade-in, then a timer, then talk.
    class stepDemoBus_c
        : public dState::step_c<dAcNpcSpPlayerCreate_c, stepDemoBus_c> {
      public:
        stepDemoBus_c() : mTimer(0) {
        }
        virtual ~stepDemoBus_c() {
        }
        void init();       // 0xEF0
        void waitFadeIn(); // 0xFA8
        void startTimer(); // 0x1068
        void waitTimer();  // 0x1108
        void finish();     // 0x11C0

        /* 0x30 */ int mTimer;
    }; // size 0x34

    dAcNpcSpPlayerCreate_c() : mTalkPc(this) {
    }
    virtual ~dAcNpcSpPlayerCreate_c() {
    } // weak
    virtual int create();                          // 0x248
    virtual int preCreate();                       // 0x1E4
    virtual int doDelete();                        // 0x340
    virtual bool canInteract(dDemoActor_c *actor); // 0x3C0
    virtual void onDemoCmd(int cmd, u8 member);    // 0x3FC
    virtual int getEarType();                      // 0x2F08
    virtual int vtB0();                            // 0x36C

    // ---- modes (the table in create) ----
    void initWait();               // 0x930
    void executeWait();            // 0x934
    void initDemoWait();           // 0x93C
    void executeDemoWait();        // 0x940
    void initTalk();               // 0x944
    void executeTalk();            // 0x954
    void initTunnelExitCheck();    // 0x994
    void executeTunnelExitCheck(); // 0x9C4
    void initNextSceneWait();      // 0xB3C
    void executeNextSceneWait();   // 0xBB4
    void initNone();               // 0xD88
    void executeNone();            // 0xD8C
    void initBusDemo();            // 0xD90
    void executeBusDemo();         // 0xEA8

    /* 0x1C7C dState::mode_c<dAcNpcSpPlayerCreate_c> */
    /* 0x1CA0 */ stepDemoBus_c mStepDemoBus;
    /* 0x1CD4 */ int mTimer; // executeNextSceneWait
    /* 0x1CD8 */ talk_c mTalkPc;
}; // size 0x1E10

#endif
