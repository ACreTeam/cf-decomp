#ifndef D_A_NPC_SP_RESETSAN_HPP
#define D_A_NPC_SP_RESETSAN_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (m3d,
// fBase_c, dDvd, dActor_c, then the npc classes and daRmobj_c::model_c).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/mLib/m_mtx.hpp>

// Mr. Resetti in the town after a reset (REL d_a_npc_sp_resetsanNP, profile
// A_NPC_SP_RESETSAN; item key SP_NPC_RESETSAN, or SP_NPC_RACKETSAN for Don on
// the fifth reset). He digs up out of his mole hole (/Rmobj obj_mol) in front
// of the player, gives the lecture for the player's reset count
// (dPrivateData_c::getLastResetCount, messages npc_SP/NPC_reset, Don's
// npc_SP/NPC_racket) and digs back down. Some lectures have him dig away and
// come back, a fake reset (dShutdownFader + SE_NiseReset), a phrase to type in
// (the word input menu) or the "How sorry are ya?" quiz.
//
// Class names from RTTI ("dAcNpcSpResetsan_c", "dAcNpcSpResetsan_c::talk_c");
// everything else is named from behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor, then
// talk_c).
class dAcNpcSpResetsan_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,      // wait (hidden) until the player stands in front
        STATE_WAIT,      // a demo started (DEMO_CMD_START)
        STATE_TALK,      // talk to the player
        STATE_APPEAR,    // dig up (DEMO_CMD_REPORT), then speak
        STATE_DISAPPEAR, // dig back down after the talk
        STATE_DELETE,    // the demo ended: delete the actor
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // The reset counts with their own behaviour
    // (dPrivateData_c::getLastResetCount, 0..10).
    enum {
        RESET_COUNT_FIRST = 0, // the first lecture: he faces south
        RESET_COUNT_DON =
            5, // Don comes instead (SP_NPC_RACKETSAN); from here on Don is met
        RESET_COUNT_BACK_TURNED = 6, // he digs up with his back to the player
        RESET_COUNT_PHRASE = 8, // the first phrase to type in (else the second)
        RESET_COUNT_MAX = 10,   // the "How sorry are ya?" quiz (half the time)
    };
    // frames from the hide animation to SE_reset_san_moguru
    enum { HIDE_SND_DELAY = 30 };

    typedef void (dAcNpcSpResetsan_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: the lecture of the reset count.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_reset (from the ENG text). The first message of the
        // lecture is picked by the reset count (start).
        enum {
            MSG_LECTURE_0 = 0x01,  // the first reset
            MSG_LECTURE_1 = 0x0B,  // "Ah, so it's you..."
            MSG_LECTURE_2 = 0x15,  // "Aw, YOU again..."
            MSG_LECTURE_3 = 0x1F,  // "...Ya just don't give up, do ya, punk?"
            MSG_LECTURE_4 = 0x29,  // "...Cripes, do ya ever learn, kid?"
            MSG_LECTURE_6 = 0x33,  // "GRRARGH!"
            MSG_LECTURE_7 = 0x3D,  // "YAAARGH! YOU! AGAIN?!"
            MSG_LECTURE_8 = 0x47,  // "BRAAARGH! YOU! AGAIN?!"
            MSG_LECTURE_9 = 0x5B,  // "HEYYYY! Again with this?"
            MSG_LECTURE_10 = 0x79, // "Hey, over there. You, kid."

            MSG_INTRO = 0x05,  // "Lemme introduce myself."
            MSG_THANKS = 0x06, // "...The manual says I gotta thank ya"
            MSG_THANKS_VISITOR =
                0x07, // "...this is Animal Crossing: City Folk."
            MSG_SORRY_ABOUT_THAT = 0x08, // "Oh, hey, sorry about that."
            MSG_RIGHT_WAY = 0x09, // "There's a right way and a wrong way"
            MSG_RIGHT_WAY_VISITOR = 0x0A,
            MSG_SCRAM = 0x21,     // "SCRAM!" (he digs away and back)
            MSG_HELD_BACK = 0x22, // "The doc's been tellin' me..."
            MSG_WHERE_AT = 0x34,  // "...Huh? Where's <player> at?"
            MSG_GOT_IT_1 = 0x38,  // "...Quit resettin'. Got it this time?"
            MSG_GOT_IT_1_YES = 0x39,
            MSG_GOT_IT_1_NO = 0x3A, // "WHAAAT? QUIT JOKIN'!"
            MSG_GOT_IT_1_NO_AGAIN = 0x3B,
            MSG_GOT_IT_1_NOT_HEARD = 0x3C, // "Huh? What was that?"
            MSG_SEE_YA = 0x3F,             // "SEE YA, KID!" (the fake reset)
            MSG_GOTCHA = 0x40,             // "GOTCHA!"
            MSG_GOT_IT_2 = 0x42,           // "Ya gonna stop resettin'?"
            MSG_GOT_IT_2_YES = 0x43,
            MSG_GOT_IT_2_NO = 0x44,
            MSG_GOT_IT_2_NO_AGAIN = 0x45,
            MSG_GOT_IT_2_NOT_HEARD = 0x46,
            MSG_CLICKING = 0x49,     // the button presses are counted
            MSG_I_HEAR_THAT = 0x4A,  // "I HEAR THAT! ALWAYS CLICKIN'!"
            MSG_ANTSYPANTS = 0x4B,   // "Can't ya sit still for one minute"
            MSG_PRESS_COUNT = 0x4C,  // "Ya pressed that thing <n> times!"
            MSG_SAY_THIS_1 = 0x4F,   // "Lemme hear ya say this: ..."
            MSG_SAID_IT_1 = 0x50,    // "Good stuff!"
            MSG_SAID_WRONG_1 = 0x52, // "WRONG!"
            MSG_SAID_WRONG_1_AGAIN = 0x53, // "Wha-WHAT?! ALL WRONG!"
            MSG_SOMETIME = 0x62,           // "...take me there sometime"?
            MSG_GOT_IT_3 = 0x64,           // "Ya clear?"
            MSG_GOT_IT_3_YES = 0x65,
            MSG_GOT_IT_3_NO = 0x66,
            MSG_GOT_IT_3_NO_AGAIN = 0x67,
            MSG_GOT_IT_3_NOT_HEARD = 0x68,
            MSG_LUNCH = 0x69,      // "If ya plan a lunch..."
            MSG_MEAN_IT = 0x6A,    // "Say what ya mean..."
            MSG_SAY_THIS_2 = 0x6D, // "I wanna hear ya say this: ..."
            MSG_SAID_IT_2 = 0x6E,  // "Solid work, kid."
            MSG_SAID_WRONG_2 = 0x70,
            MSG_SAID_WRONG_2_AGAIN = 0x71,
            MSG_NO_REGRET = 0x7A,        // "Ya don't regret your behavior"
            MSG_HERE_WE_GO = 0x7B,       // "Yeah, yeah, here we go..."
            MSG_HERE_WE_GO_AGAIN = 0x7C, // (the count went down: 0x58)
            MSG_FRIEND = 0x7D,           // "Say there, friend of mine"
            MSG_FORGET_IT = 0x7E,        // "AAAAAAAAH, FORGET IT!"
            MSG_ENOUGH = 0x7F,           // "ENOUGH! NO MORE!"
            MSG_NICE_TALK = 0x80,        // "I can't take talkin' all nice"
            MSG_NICE_TALK_AGAIN = 0x81,
            MSG_CREEPED_OUT = 0x82, // "Creeped ya out, too, right?"
            MSG_SORRY_ASK = 0x83,   // (dispatcher)
            MSG_LAST_TIME = 0x84,   // "Seriously now, don't let me down!"
            MSG_SHOW_SORRY = 0x85,  // "...show me just how sorry ya are"
            MSG_HOW_SORRY = 0x86,   // "How sorry are ya?" + mSorryType
            MSG_HOW_SORRY_AGAIN =
                0x8A,                  // "Ya gotta do this again." + mSorryType
            MSG_SORRY_FAKE = 0x8E,     // "...your answer sounds fake."
            MSG_SORRY_MIDDLE = 0x8F,   // "Ya put it practically in the middle!"
            MSG_SORRY_NOT = 0x90,      // "...And here I thought ya were sorry!"
            MSG_SORRY_BELIEVED = 0x91, // "Huh, guess I can believe it."
        };
        // npc_SP/NPC_racket: Don's lecture (the fifth reset).
        enum {
            MSG_DON_FIRST = 0x01, // Don not met yet
            MSG_DON = 0x07,       // isDonAtCafe
        };
        // The answers of MSG_GOT_IT_*.
        enum { ANSWER_YES, ANSWER_NO };
        // Message tags (onMessageStart / onMessageEnd) and custom tags.
        enum { TAG_SLAP = 0x10, TAG_THUNDER = 0x11 };
        enum { CUSTOM_SLAP, CUSTOM_THUNDER };
        // Random phrases to type in (sys_STRING/STR_Unit + mPhrase).
        enum { PHRASE_FIRST = 0x15, PHRASE_NUM = 10 };
        // dNpcInfo::resetsan_c::mSorryType: which of the four quizzes is asked
        // (MSG_HOW_SORRY + type), or none.
        enum { SORRY_NONE = -1, SORRY_1, SORRY_2, SORRY_3, SORRY_4 };

        talk_c(dAcNpcSpResetsan_c *npc);          // 0xC88
        virtual ~talk_c();                        // 0xCE8
        virtual void init();                      // 0xD40
        virtual void getMsgInfo(msgInfo_s *info); // 0xF04
        virtual void onMessageStart(int arg);     // 0xF78
        virtual void onMessageEnd(int kind);      // 0x1070
        virtual void onAnswer(int arg);           // 0x15EC
        virtual void preExecute();                // 0x1808
        virtual void onTalkEnd();                 // 0x188C
        virtual void onCustomTag(int no);         // 0x18E8
        virtual void onRequestEnd(int kind);      // 0x1954

        void start(dActor_c *target); // 0xDF4
        void startWordInput();        // 0x19FC
        void holdMsg();               // 0x1A54
        void checkWord();             // 0x1A5C
        void startDigAway();          // 0x1B98: "SCRAM!"
        void startAway();             // 0x1C38
        void procAway();              // 0x1C74: dig back up after a while
        void closeMsg();              // 0x1D44
        void startTurn();             // 0x1D74
        void procTurn();              // 0x1DC0 (+ a dead blr at 0x1DFC)
        void startFakeReset();        // 0x1E00: "SEE YA, KID!"
        void procFakeReset();         // 0x1E8C

        /* 0x0EA */ u16 mMsgNo; // the first message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpResetsan_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ Proc mNextOnRequestEnd; // mOnRequestEnd after the next
        /* 0x114 */ Proc mProc;             // every frame (preExecute)
        /* 0x120 */ int mTimer;             // procAway
        /* 0x124 */ int mRetry;             // the questions asked again
        /* 0x128 */ int mPressCount;        // MSG_CLICKING: the buttons pressed
        /* 0x12C */ int mPhrase;            // the phrase to type in
        /* 0x130 */ u8 mSometime;           // MSG_SOMETIME was told
    }; // size 0x134

    dAcNpcSpResetsan_c() : mResetTalk(this) {
    }
    virtual ~dAcNpcSpResetsan_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x298
    void changeState(int state);                // 0x358
    virtual int preCreate();                    // 0x390
    virtual int create();                       // 0x424
    virtual int doDelete();                     // 0x618
    virtual int vtB0();                         // 0x66C
    virtual int draw();                         // 0x728
    void initIdle();                            // 0x778
    void executeIdle();                         // 0x7C0
    void initWait();                            // 0x894
    void executeWait();                         // 0x8A8
    void initTalk();                            // 0x8AC
    void executeTalk();                         // 0x900
    void initAppear();                          // 0x904
    void executeAppear();                       // 0xA24 (+ a dead blr at 0xA4C)
    void initDisappear();                       // 0xA50
    void executeDisappear();                    // 0xA5C
    void initDelete();                          // 0xB3C
    void executeDelete();                       // 0xB40
    void turnToPlayer();                        // 0xB7C
    void startHideAnm();                        // 0xC18: "obj_mol_1"
    void startAppearAnm();                      // 0xC34: "obj_mol_0"
    // new virtuals (+0xE8, +0xEC)
    virtual BOOL isResetti(); // 0xC50
    virtual BOOL isDon();     // 0xC6C

    /* 0x1C7C */ daRmobj_c::model_c mMoleMdl; // "obj_mol": the mole hole
    /* 0x1E28 */ mMtx_c mMoleMtx;
    /* 0x1E58 */ int mState;
    /* 0x1E5C */ u8 mStep; // STATE_DISAPPEAR: the hide animation started
    /* 0x1E60 */ talk_c mResetTalk;
    /* 0x1F94 */ int mHideSndTimer; // frames until SE_reset_san_moguru
    /* 0x1F98 */ u8 mMoleVisible;   // the mole hole is drawn
}; // size 0x1F9C

#endif
