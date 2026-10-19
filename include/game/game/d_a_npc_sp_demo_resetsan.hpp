#ifndef D_A_NPC_SP_DEMO_RESETSAN_HPP
#define D_A_NPC_SP_DEMO_RESETSAN_HPP

#include <game/game/d_a_npc_sp.hpp>

// Mr. Resetti and his brother Don in the Reset Surveillance Center (REL
// d_a_npc_sp_demo_resetsanNP, scene RM_RESET; profiles NPC_SP_DEMO_RESETSAN
// (Resetti, item key SP_NPC_RESETTI) and NPC_SP_DEMO_RACKETSAN (Don, item key
// SP_NPC_DON)). Messages npc_SP/NPC_reset_kanshi_center. Which of the visits
// it is (mVisit) decides who is there and what they say; on some visits the
// talk ends with the "RESET SCRAMBLE!": Resetti hits the switch and the seat
// (d_a_rmobj_reset) launches him out.
//
// Class names from RTTI (dAcNpcSpDemoResetsanBase_c and its talk_c,
// dAcNpcSpDemoResetsan_c::talkDemoReset_c,
// dAcNpcSpDemoRacketsan_c::talkDemoRacket_c); everything else is named from
// behaviour.
//
// Declaration order matters: .data holds the vtables in reverse
// class-completion order (Racketsan, its talk class, Resetsan, its talk class,
// the base, its talk class).
class dAcNpcSpDemoResetsanBase_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE,         // wait; the player may interrupt the brothers
        STATE_DEMO_WAIT,    // a demo started (DEMO_CMD_START)
        STATE_TALK,         // talk to the player
        STATE_PARTNER_TALK, // the partner talks (vt80)
        STATE_COUNT
    };
    // mStep in STATE_TALK (l_talkSteps).
    enum TalkStep_e {
        TALK_STEP_TALK,     // wait for the talk (or start the scramble)
        TALK_STEP_SCRAMBLE, // wait for the door to close behind Resetti
        TALK_STEP_END,      // wait a moment, then finish the talk
        TALK_STEP_DONE
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    // The visits (mVisit, picked in preCreate from the player's flags; from
    // the fifth on by a daily random).
    enum Visit_e {
        VISIT_NONE,
        VISIT_FIRST,    // the first visit: Resetti off duty, the silver shovel
        VISIT_SECOND,   // Resetti alone
        VISIT_THIRD,    // Resetti alone (the third and fourth visits)
        VISIT_DON,      // Don is there too: the brothers' first chat
        VISIT_DON_ONLY, // Resetti is out on a reset call: Don alone
        VISIT_CHAT,     // the brothers chat
        VISIT_RESETTI,  // Resetti alone
        VISIT_CHAT2,    // the brothers chat
        VISIT_EMPTY,    // nobody is there
    };

    typedef void (dAcNpcSpDemoResetsanBase_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor (RTTI "dAcNpcSpDemoResetsanBase_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_reset_kanshi_center, named from the English text.
        enum {
            MSG_FIRST = 0x01,       // "Hey, whoa! You! How'd ya get in here?!"
            MSG_FIRST_GIVE = 0x03,  // "Oh, RIGHT! Figures." (a present)
            MSG_FIRST_GIVEN = 0x04, // "...it's a Silver Shovel."
            MSG_FIRST_POCKETS_FULL = 0x05, // "And look at those pockets!"
            MSG_POCKETS_FULL = 0x06,       // "I try to give ya somethin' nice"
            MSG_POCKETS_FULL_END = 0x07,   // "Too bad, kid."
            MSG_POCKETS_FULL_AGAIN = 0x08, // "Look at them pockets!"
            MSG_FIRST_TALKED = 0x09,       // "I ain't got nothin' else, kid!"
            MSG_FIRST_TALKED_AGAIN = 0x0A,
            MSG_SECOND = 0x15, // "What're ya thinkin', comin' back in here?!"
            MSG_SECOND_TALKED = 0x16,
            MSG_THIRD = 0x1F,
            MSG_THIRD_TALKED = 0x23,
            MSG_DON = 0x34,        // "WAAAAH! Whoa, ya scared me, kid!"
            MSG_DON_TALKED = 0x3F, // "There ain't nothin' here."
            MSG_DON_TALKED_AGAIN = 0x40,
            MSG_SCRAMBLE = 0x3C,   // "RESET SCRAMBLE!"
            MSG_RANGERS_GO = 0x3D, // "RANGERS, GO!"
            MSG_LAUNCHED = 0x3E,   // "AAAARGH! MY SOCK!"
            MSG_RE_ASK = 0x44,     // asks again (the code is picked anew)
            MSG_DON_FIRST = 0x45,  // "WAAAHHH! Say, who are ya?" (Don not met)
            MSG_DON_ONLY = 0x47,   // "My brother Sonny got a reset emergency"
            MSG_DON_ONLY_TALKED = 0x4B,
            MSG_CHAT = 0x51,
            MSG_SCRAMBLE_2 = 0x58, // "RESET SCRAMBLE!"
            MSG_RANGERS_GO_2 = 0x59,
            MSG_LAUNCHED_2 = 0x5A, // "AAAARGH! MY BOOT!"
            MSG_CHAT_TALKED = 0x5B,
            MSG_RESETTI = 0x65, // "Oh, it's you, huh? How ya doin'?"
            MSG_RESETTI_TALKED = 0x69,
            MSG_SCRAMBLE_3 = 0x73,
            MSG_RANGERS_GO_3 = 0x74,
            MSG_LAUNCHED_3 = 0x75,
            MSG_CHAT2_RESETTI = 0x8F, // interrupted: Resetti speaks
            MSG_CHAT2_DON = 0x93,     // interrupted: Don speaks
            MSG_CHAT2 = 0x97,
        };
        // Message tags (onMessageStart / onMessageEnd).
        enum {
            TAG_SLAP = 0x10,    // SE_resetsan_viblslap
            TAG_THUNDER = 0x11, // SE_resetsan_kaminari, the player flinches
        };
        // Custom tags (onCustomTag).
        enum { CUSTOM_SLAP, CUSTOM_THUNDER, CUSTOM_LIGHT_ON };

        talk_c(dAcNpcSpDemoResetsanBase_c *npc);  // 0xFEC
        virtual ~talk_c();                        // 0x10D4
        virtual void onMessageStart(int arg);     // 0x14A0
        virtual void onMessageEnd(int kind);      // 0x160C
        virtual void onAnswer(int arg);           // 0x195C
        virtual void init();                      // 0x11DC
        virtual void onCustomTag(int no);         // 0x18CC
        virtual void preExecute();                // 0x1960
        virtual void onTalkEnd();                 // 0x19D8
        virtual void getMsgInfo(msgInfo_s *info); // 0x1488
        virtual void onRequestEnd(int kind);      // 0x1AE8
        // new virtual (+0xA8): the partner of the talk (the other brother)
        virtual void setPartnerNpc() = 0;

        int getMsgCode() const;       // 0x1268
        void start(dActor_c *target); // 0x140C
        void setTalked();             // 0x19A8
        void giveShovel();            // 0x1B4C
        void pickUpShovel();          // 0x1B94
        void holdMsg();               // 0x1C78
        void startScramble();         // 0x1C80
        void procWaitCamera();        // 0x1CB0
        void procSwitch();            // 0x1D20
        void procLaunch();            // 0x1DF8
        void startBack();             // 0x1E40
        void procBack();              // 0x1EBC

        /* 0x0EA */ u16 mMsgNo;         // getMsgCode
        /* 0x0EC */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0F8 */ Proc mOnRequestEnd;
        /* 0x104 */ Proc mProc; // every frame (preExecute)
        /* 0x110 */ u8 mTalked;
        /* 0x111 */ u8 mPocketsFullTold;
        /* 0x112 */ u8 mTalkedAgain;
        /* 0x113 */ u8 mScramble; // the talk ends with the scramble
        /* 0x114 */ int mTimer;   // procBack
        /* 0x118 */ int mFrame;   // procSwitch
        /* 0x11C */ dAcNpcSpDemoResetsanBase_c *mpOwner;
    }; // size 0x120

    dAcNpcSpDemoResetsanBase_c() : mVisit(VISIT_NONE) {
    }
    virtual ~dAcNpcSpDemoResetsanBase_c() {
    } // weak 0x2000
    virtual int create();                          // 0x740
    virtual int preCreate();                       // 0x430
    virtual int doDelete();                        // 0x828
    virtual bool canInteract(dDemoActor_c *actor); // 0x8B4
    virtual void onDemoCmd(int cmd, u8 member);    // 0x344
    virtual int canSpeak(dDemoActor_c *actor);     // 0x950
    virtual void vt80();                           // 0xEF8
    virtual void vt84();                           // 0xF5C
    virtual int vtB0();                            // 0x854
    // new virtuals (+0xE8..+0xF0)
    virtual BOOL isResetti() = 0;
    virtual BOOL isDon() = 0;
    virtual talk_c *getOwnTalk() = 0;

    void changeState(int state); // 0x3F8
    BOOL checkInterrupt();       // 0x998
    void initIdle();             // 0xB6C
    void executeIdle();          // 0xB80
    void initDemoWait();         // 0xBC0
    void executeDemoWait();      // 0xBD4
    void initTalk();             // 0xBD8
    void startTalk();            // 0xBDC
    void stepTalk();             // 0xCC0
    void closeDoor();            // 0xD5C
    void stepScramble();         // 0xDB4 (+ a dead blr at 0xDDC)
    void startTalkEnd();         // 0xDE0
    void stepTalkEnd();          // 0xE2C
    void executeTalk();          // 0xEB0
    void initPartnerTalk();      // 0xEF0
    void executePartnerTalk();   // 0xEF4
    void lookFront();            // 0xFD0
    void startSwitchAnm();       // 0x1560
    void startGearAnm();         // 0x1588
    void startBackAnm();         // 0x15E4

    talk_c *getTalk() {
        return static_cast<talk_c *>(mpRecept);
    }
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

    /* 0x1C7C */ int mState;
    /* 0x1C80 */ u8 mStep;
    /* 0x1C84 */ int mVisit;
    /* 0x1C88 */ int mTimer; // startTalkEnd
}; // size 0x1C8C

// Mr. Resetti.
class dAcNpcSpDemoResetsan_c : public dAcNpcSpDemoResetsanBase_c {
  public:
    class talkDemoReset_c : public dAcNpcSpDemoResetsanBase_c::talk_c {
      public:
        talkDemoReset_c(dAcNpcSpDemoResetsanBase_c *npc); // 0x105C
        virtual ~talkDemoReset_c();                       // 0x112C
        virtual void setPartnerNpc();                     // 0x1BD8: Don
    };

    dAcNpcSpDemoResetsan_c() : mResetTalk(this) {
    }
    virtual ~dAcNpcSpDemoResetsan_c() {
    } // weak 0x228C
    virtual int preCreate();                   // 0x6A8
    virtual int canSpeak(dDemoActor_c *actor); // 0x990
    virtual BOOL isResetti() {
        return TRUE;
    } // weak 0x2284
    virtual BOOL isDon() {
        return FALSE;
    } // weak 0x227C
    virtual talk_c *getOwnTalk() {
        return &mResetTalk;
    } // weak 0x2274

    /* 0x1C8C */ talkDemoReset_c mResetTalk;
}; // size 0x1DAC

// Don Resetti.
class dAcNpcSpDemoRacketsan_c : public dAcNpcSpDemoResetsanBase_c {
  public:
    class talkDemoRacket_c : public dAcNpcSpDemoResetsanBase_c::talk_c {
      public:
        talkDemoRacket_c(dAcNpcSpDemoResetsanBase_c *npc); // 0x1098
        virtual ~talkDemoRacket_c();                       // 0x1184
        virtual void setPartnerNpc();                      // 0x1C28: Resetti
    };

    dAcNpcSpDemoRacketsan_c() : mRacketTalk(this) {
    }
    virtual ~dAcNpcSpDemoRacketsan_c() {
    } // weak 0x213C
    virtual int preCreate();                   // 0x6F4
    virtual int canSpeak(dDemoActor_c *actor); // 0x994
    virtual BOOL isResetti() {
        return FALSE;
    } // weak 0x2134
    virtual BOOL isDon() {
        return TRUE;
    } // weak 0x212C
    virtual talk_c *getOwnTalk() {
        return &mRacketTalk;
    } // weak 0x2124

    /* 0x1C8C */ talkDemoRacket_c mRacketTalk;
}; // size 0x1DAC

#endif
