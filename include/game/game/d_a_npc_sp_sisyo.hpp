#ifndef D_A_NPC_SP_SISYO_HPP
#define D_A_NPC_SP_SISYO_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>

// Dr. Shrunk on the stage of the Marquee, the city theater (REL
// d_a_npc_sp_sisyoNP, profile A_NPC_SP_SISYO; sisyo = shishou, master). On a
// day of one of Master Frillard's programs (dTheater::common_c::
// isKyosyoProgram) a player who has seen a show already gets Frillard
// (SP_NPC_KYOSYO) instead. Once the player sits down in the audience (the
// chair of dPlayerMgr::getChairPosition) he runs the show: as the emcee he
// announces it (npc_SP/NPC_theater), the lights go down, the curtain
// (daRmobjTheater_c) rises, he tells the program's joke and shows its emotion
// (npc_SP/NPC_shisho; the program is a STR_Reaction emotion), the curtain
// falls (an encore follows on the first visit), the lights come back on and
// the player learns the emotion (dPrivateEmotions_c; with four emotions
// already, the player first picks one to forget: sys_MESS/SYS_Player).
//
// Class names from RTTI ("dAcNpcSpSisyo_c", "dAcNpcSpSisyo_c::talk_c",
// "dAcNpcSpSisyo_c::dSisyoDebugPage_c"); everything else is named from
// behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dSisyoDebugPage_c), so dSisyoDebugPage_c is declared first.
class dAcNpcSpSisyo_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE, // wait for a player to sit down
        STATE_LOOK, // the demo started
        STATE_SHOW, // run the show (mStep: l_showSteps)
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // The show (mStep, l_showSteps), in table order.
    enum ShowStep_e {
        SHOW_STEP_OPENING,     // the emcee's announcement
        SHOW_STEP_DIM,         // the lights go down
        SHOW_STEP_RAISE_WAIT,  // the player looks at the stage
        SHOW_STEP_RAISE,       // the curtain rises
        SHOW_STEP_PERFORM,     // the joke
        SHOW_STEP_PERFORMED,   // first show: the encore
        SHOW_STEP_ENCORE_WAIT, // the curtain rises again
        SHOW_STEP_ENCORE,      // the encore talk
        SHOW_STEP_ENCORE_END,  // the curtain falls again
        SHOW_STEP_SWAP,        // forget an emotion for the new one
        SHOW_STEP_LIGHTS_WAIT, // the player looks back
        SHOW_STEP_LIGHTS_UP,   // the lights come back on
        SHOW_STEP_ENDING,      // the emcee's goodbye, the emotion is learnt
        SHOW_STEP_DONE
    };
    // The show's timers (frames).
    enum {
        DIM_TIME = 180,         // the lights go down
        RAISE_WAIT_TIME = 120,  // before the curtain rises
        LOOK_TIME = 60,         // .. the player turns to / from the stage
        ENCORE_WAIT_TIME = 30,  // before the encore's curtain
        LIGHTS_WAIT_TIME = 120, // after the show
        LIGHTS_UP_TIME = 60,    // the lights come back on
    };

    typedef void (dAcNpcSpSisyo_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dSisyoDebugPage_c : public dDebugPage_c {};

    // The message receptor: the emcee's announcements, the show and the
    // emotion swap.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // The talks (mType): which part of the show and message file.
        enum Type_e {
            TYPE_OPENING, // npc_SP/NPC_theater: the emcee's announcement
            TYPE_SHOW,    // npc_SP/NPC_shisho: the joke
            TYPE_ENDING,  // npc_SP/NPC_theater: the emcee's goodbye
            TYPE_ENCORE,  // npc_SP/NPC_shisho: the encore of the first show
            TYPE_SWAP,    // sys_MESS/SYS_Player: forget an emotion
            TYPE_NONE
        };

        // Message codes of npc_SP/NPC_theater (from the ENG text).
        enum {
            MSG_THEATER_SHRUNK = 0x0C,   // "...pleased to welcome Dr. Shrunk"
            MSG_THEATER_FRILLARD = 0x0D, // "...present Master Frillard"
            MSG_THEATER_ENDING = 0x0E,   // "We hope you enjoyed the show"
            MSG_THEATER_FIRST = 0x10,    // "...very pleased to present Dr.
                                         // Shrunk" (the first show)
        };
        // Message codes of npc_SP/NPC_shisho (from the ENG text).
        enum {
            MSG_SHOW_FIRST = 0x01,  // "Thank you! Dr. Shrunk is in the house!"
            MSG_ENCORE = 0x04,      // "So? See what I did there? ... emotion"
            MSG_SHOW_SHRUNK = 0x0A, // the curtain rises (Shrunk)
            MSG_SHOW_FRILLARD = 0x0E,   // "Ready for a thrill?"
            MSG_FRILLARD_SHTICK = 0x0F, // "Then Frillard's classic shtick..."
            MSG_AHEM = 0x14,            // "Um, ahem."
            // The jokes: MSG_JOKE + the program (its STR_Reaction emotion).
            MSG_JOKE = 0x14,
            MSG_JOKE_FIRST = 0x15,
            MSG_JOKE_LAST = 0x31,
        };
        // Message codes of sys_MESS/SYS_Player (from the ENG text).
        enum {
            MSG_SWAP_ASK = 0x1F,   // "I can learn <emotion>, but I'll have to
                                   // forget an old emotion first."
            MSG_SWAP_WHICH = 0x20, // "Which emotion should I swap it out with?"
        };
        // The answers of MSG_SWAP_ASK.
        enum { ANSWER_YES, ANSWER_NO };
        // The choices of MSG_SWAP_WHICH: the four emotions, then
        // STR_REACTION_NEVER_MIND.
        enum {
            SWAP_CHOICE_NUM = 5,
            SWAP_CHOICE_CANCEL = 4,
            STR_REACTION_NEVER_MIND = 0x1E, // sys_STRING/STR_Reaction
        };

        talk_c(dAcNpcSpSisyo_c *npc);             // 0xEA4
        virtual ~talk_c();                        // 0xF20
        virtual void init();                      // 0xF78
        virtual int getVoiceMode();               // 0x12DC
        virtual void getMsgInfo(msgInfo_s *info); // 0x12FC
        virtual void onMessageStart(int arg);     // 0x136C
        virtual void onMessageEnd(int kind);      // 0x13F0
        virtual void onAnswer(int arg);           // 0x1624
        virtual void preExecute();                // 0x1794
        virtual void onTalkEnd();                 // 0x1838
        virtual void onRequestEnd(int kind);      // 0x1934

        void start(dActor_c *target); // 0xFF4
        int getEmotion() const {
            return mEmotion;
        }

        // ---- request hooks (mOnMessageEnd / mOnAnswer / mOnRequestEnd) ----
        void initCurtainClose();    // 0x1998: the curtain falls after the joke
        void executeCurtainClose(); // 0x19C8 (mProc)
        void closeMsg();            // 0x1A58
        void endJoke();             // 0x1AB0: closeMsg, the show BGM stops
        void startCurtainClose();   // 0x1B20
        void setDone();             // 0x1B24
        void onRequestEndNone();    // 0x1B30

        /* 0x0EA */ u16 mMsgCode; // the first message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpSisyo_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnAnswer;
        /* 0x108 */ Proc mOnRequestEnd;
        /* 0x114 */ Proc mProc;    // run every frame (preExecute)
        /* 0x120 */ int mType;     // Type_e
        /* 0x124 */ int mEmotion;  // the program: its STR_Reaction emotion
        /* 0x128 */ int mTimer;    // executeCurtainClose
        /* 0x12C */ u8 mDone;      // the talk's part of the show is over
        /* 0x12D */ u8 mShowEnded; // the show BGM stopped (endJoke)
    }; // size 0x130

    dAcNpcSpSisyo_c() : mSisyoTalk(this) {
    }
    virtual ~dAcNpcSpSisyo_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member);            // 0x1E0
    void changeState(int state);                           // 0x270
    virtual int preCreate();                               // 0x2A8
    virtual int create();                                  // 0x33C
    virtual int doDelete();                                // 0x3C4
    virtual int vtB0();                                    // 0x408
    virtual int getEarType();                              // 0x4A8
    void initIdle();                                       // 0x4EC
    void executeIdle();                                    // 0x500
    void initLook();                                       // 0x548
    void executeLook();                                    // 0x58C
    void initShow();                                       // 0x590
    void initOpening();                                    // 0x594
    void executeOpening();                                 // 0x618
    void initDim();                                        // 0x664
    void executeDim();                                     // 0x6B0
    void initRaiseWait();                                  // 0x6D0
    void executeRaiseWait();                               // 0x6E4
    void initRaise();                                      // 0x71C
    void executeRaise();                                   // 0x79C
    void initPerform();                                    // 0x7D8
    void executePerform();                                 // 0x82C
    void initPerformed();                                  // 0x878
    void executePerformed();                               // 0x884
    void initEncoreWait();                                 // 0x8D4
    void executeEncoreWait();                              // 0x8E8
    void initEncore();                                     // 0x954
    void executeEncore();                                  // 0x9A8
    void initEncoreEnd();                                  // 0x9F4
    void executeEncoreEnd();                               // 0xA2C
    void initLightsWait();                                 // 0xA68
    void executeLightsWait();                              // 0xA7C
    void initLightsUp();                                   // 0xAB4
    void executeLightsUp();                                // 0xAC8
    void initSwap();                                       // 0xC1C
    void executeSwap();                                    // 0xC70
    void initEnding();                                     // 0xCBC
    void executeEnding();                                  // 0xD10
    void executeShow();                                    // 0xDB0
    BOOL isAudienceSeated();                               // 0xDF0
    virtual void getName(dHmnName::Word_c *name, int len) const; // 0xE1C
    virtual int getVoiceType() const;                      // 0xE88

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dSisyoDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep; // ShowStep_e
    /* 0x1C90 */ talk_c mSisyoTalk;
    /* 0x1DC0 */ int mTimer;
    /* 0x1DC4 */ u8 mEmcee;        // he speaks as the "Emcee" (getName)
    /* 0x1DC8 */ int mBuzzerTimer; // frames until SE_buzzer
    /* 0x1DCC */ u8 mEnvNoise;     // hold SE_Theater_EnvNoise
}; // size 0x1DD0

#endif
