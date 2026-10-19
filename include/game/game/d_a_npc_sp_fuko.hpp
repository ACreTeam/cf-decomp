#ifndef D_A_NPC_SP_FUKO_HPP
#define D_A_NPC_SP_FUKO_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_objc.hpp>

// Celeste, the caretaker of the observatory (REL d_a_npc_sp_fukoNP, profile
// A_NPC_SP_FUKO). She dozes at her post by day (5 am..5 pm) while nobody is
// near, shows the telescope (make, change, view and erase the town's
// constellations, dSvStarSign) and tells when a constellation is best seen
// (dStarDraw_c::getSignTime / getSignDate).
//
// Class names from RTTI ("dAcNpcSpFuko_c", "dAcNpcSpFuko_c::talk_c",
// "dAcNpcSpFuko_c::dFukoDebugPage_c"); everything else is named from
// behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dFukoDebugPage_c), so dFukoDebugPage_c is declared first.
class dAcNpcSpFuko_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,        // awake at her post; dozes off by day
        STATE_LOOK,        // wake up / turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // mStep of STATE_IDLE (l_idleSteps), STATE_LOOK (l_lookSteps), STATE_TALK
    // (l_talkSteps) and STATE_REMOTE (l_remoteSteps).
    enum IdleStep_e { IDLE_STEP_AWAKE, IDLE_STEP_ASLEEP, IDLE_STEP_DONE };
    enum LookStep_e { LOOK_STEP_WAKE, LOOK_STEP_TURN, LOOK_STEP_DONE };
    enum TalkStep_e {
        TALK_STEP_TURN,     // wait for the talk, then turn back to the front
        TALK_STEP_TURN_END, // wait for the turn, finish the talk
        TALK_STEP_DONE
    };
    enum RemoteStep_e {
        REMOTE_STEP_AWAKE,
        REMOTE_STEP_ASLEEP,
        REMOTE_STEP_DONE
    };
    enum {
        SLEEP_START_HOUR = 5,    // she dozes from 5 am (isSleepTime) ...
        SLEEP_END_HOUR = 17,     // ... to 5 pm
        SLEEP_WAIT_FRAMES = 180, // alone this long before she dozes off
    };

    typedef void (dAcNpcSpFuko_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dFukoDebugPage_c : public dDebugPage_c {};

    // The message receptor: the greeting (new constellations), the telescope
    // menus and the constellation lists.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // The kinds of talk (mMsgKind; onDemoCmd).
        enum MsgKind_e {
            MSG_KIND_TALK, // the player talks to her
            MSG_KIND_SPEAK // she speaks to the player
        };

        // Message codes of npc_SP/NPC_hu-ko (from the ENG text).
        enum {
            MSG_NEWCOMER = 0x01,       // "Hello and welcome! So, just moved
                                       // into our quaint little town...?"
            MSG_NEWCOMER_AGAIN = 0x02, // "I am the caretaker of our fair
                                       // observatory..."
            MSG_WAKE = 0x03,       // "I beg your pardon! I was merely taking a
                                   // moment to rest my eyes."
            MSG_WELCOME = 0x04,    // "Welcome to <town>'s observatory!"
            MSG_WAKE_AGAIN = 0x05, // "Oh... I've not been getting near
                                   // enough sleep these days..."
            MSG_WELCOME_AGAIN = 0x06, // "Welcome to the observatory!"
            MSG_NEW_SIGN = 0x07,      // "...I have discovered a new
                                      // constellation! Its name is <sign>."
            MSG_NEW_SIGNS = 0x08,     // "...I have discovered many new
                                      // constellations!"
            MSG_VISITOR = 0x09,       // "...the telescope is not functioning
                                      // properly..." (visitors record nothing)
            MSG_TELESCOPE = 0x0A,     // "What you see here is the telescope..."
            MSG_ASSIST = 0x0B,        // "How may I be of assistance?"
            MSG_NEWCOMER_DOZING = 0x11, // "Hmm... Goodness gracious! I didn't
                                        // realize I had a visitor..."
            MSG_NEWCOMER_DOZING_AGAIN = 0x12, // "Oh... I'm afraid I have
                                              // been burning the candle..."
            MSG_HOOT = 0x13,        // "Hrmmrmrm smack smack... HOOOOT!" (woken)
            MSG_NEVER_DO = 0x15,    // "Oh, dear me! This will never do..."
            MSG_USE = 0x16,         // "You would like to use the telescope?"
                                    // Constellations / Stargazing / Forget it
            MSG_USE_ERASE = 0x17,   // (the same) ... / Erasing! / ...
            MSG_USE_VISITOR = 0x18, // To see stars! / Oh, nothing.
            MSG_USE_VISITOR_SIGNS = 0x19, // (the same; the town has some)
            MSG_ENJOY_MAKE = 0x1B,        // "Hootie-toot! Please enjoy."
            MSG_ASK_MAKE = 0x1C, // "...create a new constellation? Or would
                                 // you rather modify an existing one?"
            MSG_TOO_MANY = 0x1D, // "...The only way to make room for a new
                                 // constellation is to erase an older one."
            MSG_MAKE_CANCELLED = 0x1E,   // "Pardon? Changed your mind...?"
            MSG_WHICH_CHANGE = 0x1F,     // "Which constellation did you intend
                                         // to change?"
            MSG_CHANGE_CANCELLED = 0x20, // "Oh? Are you quite sure? ...It is
                                         // quite stunning as is!"
            MSG_ASK_RENAME = 0x21, // "...register this constellation with a
                                   // new name?" New name! / It's fine.
            MSG_RENAME = 0x22,     // "Smashing! I trust you shall give it a
                                   // name..."
            MSG_RENAMED = 0x23,    // "Splendid! ...as I register <sign> into
                                   // my logs as a new constellation."
            MSG_ASK_LOG = 0x24,    // "...They should shine at their brilliant
                                // best on the <day> of <month>... Shall I log
                                // this constellation in my charts?"
            MSG_ASK_NAME = 0x25,   // "Have you decided on a name...?"
            MSG_NAME_TAKEN = 0x26, // "How unusual! Another constellation has
                                   // already been registered under that very
                                   // name!"
            MSG_REGISTERED = 0x27, // "Splendid! I shall now register <sign>."
            MSG_ASK_ERASE_NEW = 0x28,   // "After all your hard work! Are you
                                        // sure you want to erase your
                                        // constellation?"
            MSG_NEW_ERASED = 0x29,      // "This constellation has now been
                                        // reunited with its fellow stars..."
            MSG_ENJOY_VIEW = 0x2A,      // "Hootie-toot! Do enjoy yourself!"
            MSG_ALL_DONE = 0x2B,        // "All done? Brilliant, isn't it?"
            MSG_VIEW_CANCELLED = 0x2D,  // "Is that so? What a shame..."
            MSG_WHICH_LEARN = 0x2E,     // "Which constellation do you wish to
                                        // learn about?"
            MSG_WHICH_ERASE = 0x30,     // "...Which constellation do you intend
                                        // to obliterate?"
            MSG_ERASE_CANCELLED = 0x31, // "Quite right. I think it's best to
                                        // leave things as they are."
            MSG_ASK_ERASE = 0x32, // "...certain that you'd like to erase dear
                                  // <sign> from the night skies?"
            MSG_ERASED = 0x33,    // "...Farewell, <sign>. I reunite you with
                                  // your starry brethren."
            MSG_USE_AGAIN = 0x34, // "Is there anything else I can assist you
                                  // with?" (MSG_USE + 0x1E; the same choices)
            MSG_USE_AGAIN_ERASE = 0x35,
            MSG_USE_AGAIN_VISITOR = 0x36,
            MSG_USE_AGAIN_VISITOR_SIGNS = 0x37,
            MSG_VIEW_TIME = 0x38, // "<sign>? ...Tonight's ideal viewing time
                                  // is <hour>:<minute>."
            MSG_VIEW_NOW = 0x39,  // "<sign>, is that right? ...It's shining
                                  // at its brightest right now!"
            MSG_VIEW_TOMORROW = 0x3A, // "...the viewing conditions for that
                                      // one will be much improved tomorrow."
            MSG_BEST_EVENING = 0x3B,  // "...the best evening viewing falls on
                                      // <month> <day> at 8:00."
            MSG_PAGE_2 = 0x3D, // "How about...these?" (the next 4 of the list)
            MSG_PAGE_3 = 0x3E, // "Perhaps...these?"
            MSG_PAGE_4 = 0x3F, // "Well... These?"
            MSG_FROM_BEGINNING = 0x42, // "Very well, from the beginning..."
            MSG_ASK_LIKED = 0x47,      // "So? Did you find a constellation that
                                       // you liked?" (a visitor)
            MSG_WHICH_LIKED = 0x49,    // "...which constellation was that?"
            MSG_LIKED_CANCELLED = 0x4A, // "Oh, change of heart? Well, that IS
                                        // too bad..."
            MSG_LIKED = 0x4B, // "...So, you say the constellation of <sign>
                              // appealed to you, did it?"
        };
        // onMessageEnd kinds (INF1 end requests).
        enum { END_REQ_USE_AGAIN = 0x10 }; // then MSG_USE_AGAIN
        // The sys_SELECT/SYS_Select answers of the constellation lists.
        enum { SEL_MSG_NEXT = 0x06, SEL_MSG_QUIT = 0x07 };
        // mSelList: constellations per page, and the other entries.
        enum { SEL_LIST_NUM = 4, SEL_LIST_NONE = -1, SEL_LIST_NEXT = -2 };

        // What the player picked at the telescope (mMode): the list of
        // constellations that follows is for it.
        enum Mode_e {
            MODE_MAKE,     // make / change a constellation
            MODE_CHANGE,   // change one: pick it
            MODE_VIEW,     // stargazing: pick one to learn its time
            MODE_FAVORITE, // a visitor names the one they liked
            MODE_ERASE,    // erase one
            MODE_NONE
        };

        talk_c(dAcNpcSpFuko_c *npc);              // 0x1348
        virtual ~talk_c();                        // 0x13B4
        virtual void init();                      // 0x140C
        virtual void getMsgInfo(msgInfo_s *info); // 0x1558
        virtual void onMessageStart(int arg);     // 0x1570
        virtual void onMessageEnd(int kind);      // 0x19C8
        virtual void onAnswer(int arg);           // 0x1ED4
        virtual void preExecute();                // 0x2660
        virtual void onTalkEnd();                 // 0x26A8
        virtual void onRequestEnd(int kind);      // 0x29B8

        void start(dActor_c *target); // 0x14E4
        int getGreetingMsg() const;   // 0x27A0

        // ---- request hooks (mOnMessageEnd / mOnAnswer / mOnRequestEnd) ----
        void setRequest();         // 0x2A60
        void playerTurn();         // 0x2A68
        void openMakeMenu();       // 0x2A6C
        void openChangeMenu();     // 0x2A74
        void openViewMenu();       // 0x2A80
        void openNameMenu();       // 0x2A88
        void openRenameMenu();     // 0x2A90
        void startMake();          // 0x2A98
        void startChange();        // 0x2AE0
        void startView();          // 0x2B28
        void startMakeMenu();      // 0x2B70
        void startChangeMenu();    // 0x2BB8
        void startViewMenu();      // 0x2C00
        void endMake();            // 0x2C48
        void endChange();          // 0x2C9C
        void endView();            // 0x2CF0
        void checkName();          // 0x2D9C
        void checkRename();        // 0x2DF8
        BOOL isNameTaken(int idx); // 0x2E70
        void startTurn();          // 0x2F84
        void execTurn();           // 0x2FD0

        /* 0x0EA */ u16 mMsgNo; // the next message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpFuko_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnAnswer;
        /* 0x108 */ Proc mOnRequestEnd;
        /* 0x114 */ Proc mNextOnRequestEnd; // the mOnRequestEnd after it
        /* 0x120 */ Proc mProc;             // every frame (preExecute)
        /* 0x12C */ int mMsgKind;           // MsgKind_e
        /* 0x130 */ int mMode;              // Mode_e
        /* 0x134 */ int mSelList[SEL_LIST_NUM + 1]; // the constellation of each
                                                    // answer (SEL_LIST_*)
        /* 0x148 */ int mSelSign;                   // the constellation picked
        /* 0x14C */ u8 mListEnd; // no more constellations after the list
        /* 0x14D */ u8 mAsleep;  // she was dozing when the talk started
    }; // size 0x150

    dAcNpcSpFuko_c() : mTalkFuko(this) {
    }
    virtual ~dAcNpcSpFuko_c() {
    } // weak 0x3204
    virtual void onDemoCmd(int cmd, u8 member); // 0x2A0
    void changeState(int state);                // 0x410
    virtual int preCreate();                    // 0x448
    virtual int create();                       // 0x494
    virtual int doDelete();                     // 0x550
    virtual int vtB0();                         // 0x594
    virtual int canSpeak(dDemoActor_c *actor);  // 0x688
    virtual int getEarType();                   // 0x3010
    void initIdle();                            // 0x6D8
    void wakeUp();                              // 0x6EC
    void stepIdleAwake();                       // 0x75C
    void fallAsleep();                          // 0x7E0
    void stepIdleAsleep();                      // 0x85C
    void executeIdle();                         // 0x8CC
    void initLook();                            // 0x934
    void initLookAsleep();                      // 0x988
    void stepLookWake();                        // 0x9E0
    void initLookTurn();                        // 0xA3C
    void stepLookTurn();                        // 0xAA0
    void executeLook();                         // 0xAFC
    void initTalk();                            // 0xB3C
    void stepTalkTurn();                        // 0xBE4
    void stepTalkTurnEnd();                     // 0xC54
    void executeTalk();                         // 0xCA4
    void initRemote();                          // 0xCE4
    void initRemoteAwake();                     // 0xCF8
    void stepRemoteAwake();                     // 0xD48
    void initRemoteAsleep();                    // 0xD7C
    void stepRemoteAsleep();                    // 0xDE0
    void executeRemote();                       // 0xE30
    void initRemoteTalk();                      // 0xF1C
    void executeRemoteTalk();                   // 0xF20
    BOOL isSleepTime();                         // 0xF74: 5 am..5 pm
    int countNewSigns();                        // 0x1008
    BOOL isTelescopePressed();                  // 0x108C
    void turnToPlayer();                        // 0x12EC

    mAng getTalkAngle() const {
        dActor_c *target = mTalkFuko.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    // The net (daub) state of the npc.
    struct daub_s {
        u32 mAsleep : 1; // dozing at her post
        u32 _0 : 31;
        u32 _4;
    }; // size 0x8

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dFukoDebugPage_c mDebugPage;
    /* 0x1C88 */ dLevelEffect_c mSleepEffect; // "afm_owl_sleep"
    /* 0x1D1C */ int mSleepTimer; // frames alone before she dozes off
    /* 0x1D20 */ int mState;
    /* 0x1D24 */ u8 mStep;
    /* 0x1D28 */ talk_c mTalkFuko;
    /* 0x1E78 */ dObjc::dpdBox_c mTelescopeBox; // the pointer target
    /* 0x21E8 */ daub_s mDaub;
}; // size 0x21F0

#endif
