#ifndef D_A_NPC_SP_CATHERINE_HPP
#define D_A_NPC_SP_CATHERINE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_private_data.hpp>

// Harriet, the hairdresser of Shampoodle in the city (REL
// d_a_npc_sp_catherineNP, profile A_NPC_SP_CATHERINE). She styles the
// player's hair (a questionnaire picks the hairstyle and its colour, or the
// town's trend, or the "masterpiece" she made for another player of a friend
// town: dSaveCatherine_c) and gives Mii makeovers (the player picks a Mii of
// the album and wears it as a mask). The first makeover is free. She walks to
// the perm machine (daRmobjPermMachine_c::m_instance), switches it on and
// walks back to the counter. Her messages: "npc_SP/NPC_catherine".
//
// Class names from RTTI ("dAcNpcSpCatherine_c", "dAcNpcSpCatherine_c::talk_c",
// "dAcNpcSpCatherine_c::dCatherineDebugPage_c",
// "dNpcCatherine_Calibrate_HIO_c"); everything else is named from behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dCatherineDebugPage_c), so dCatherineDebugPage_c is declared first.
class dAcNpcSpCatherine_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,       // stand at the counter; speak to a seated player
        STATE_LOOK,       // turn to the player (demo start)
        STATE_TALK,       // talk to the player
        STATE_GREET,      // speak: welcome the player who came in
        STATE_SEE_OFF,    // speak: the player is at the (locked) exit
        STATE_TO_CHAIR,   // walk over to the seated player, then speak
        STATE_SERVED,     // turn back after the service
        STATE_TO_COUNTER, // walk back behind the counter
        STATE_SEAT,       // the player walks to the chair (MSG_BYE_TRIAL_YES)
        STATE_NUM
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // mStep of STATE_LOOK (l_lookSteps).
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_DONE };
    // mStep of STATE_TALK (l_talkSteps).
    enum TalkStep_e {
        TALK_STEP_TALK, // wait for the talk to end
        TALK_STEP_WALK, // wait for the walk, then turn to the player
        TALK_STEP_NUM
    };
    // mStep of STATE_TO_CHAIR (l_toChairSteps).
    enum ToChairStep_e {
        TO_CHAIR_STEP_START, // walk to the front of the machines
        TO_CHAIR_STEP_WALK,  // then on to the chair
        TO_CHAIR_STEP_NUM
    };
    // mStep of STATE_SERVED (l_servedSteps).
    enum ServedStep_e { SERVED_STEP_TURN, SERVED_STEP_NUM };
    // mStep of STATE_TO_COUNTER (l_toCounterSteps).
    enum ToCounterStep_e {
        TO_COUNTER_STEP_FRONT, // walk to the front of the counter
        TO_COUNTER_STEP_TURN,  // turn around
        TO_COUNTER_STEP_DONE,  // back to STATE_IDLE
        TO_COUNTER_STEP_NUM
    };

    typedef void (dAcNpcSpCatherine_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dCatherineDebugPage_c : public dDebugPage_c {};

    // The message receptor: the greetings, the hair styling and the
    // makeovers, and the walk to the perm machine and back (mProc).
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // The kinds of talk (mKind).
        enum Kind_e {
            KIND_GREET,     // STATE_GREET: the player came in
            KIND_SEE_OFF,   // STATE_SEE_OFF: the player is at the exit
            KIND_TALK,      // the player talks to her
            KIND_SERVE,     // she came over to the seated player
            KIND_SEATED,    // the player sat down without asking her
            KIND_CANCELLED, // the service was called off (no money / items)
        };

        // The gender of the hairstyles (mHairGender): the player's
        // (dPlayerID_c::mGender), or the other one once the player agreed
        // (MSG_SWAP_GIRL / MSG_SWAP_BOY). dSaveCatherine_c keeps one record
        // per group.
        enum { HAIR_GENDER_MALE, HAIR_GENDER_FEMALE };
        // dPrivateData_c::mHarrietVisits: from this many styled visits
        // she suggests the other gender's hairstyles.
        enum { VISITS_GENDER_SWAP = 15 };
        // The service fee (l_param.mPrice) in Bells.
        enum { PRICE = 3000 };
        // mMiiIndex / getMiiIndex: no Mii in the album.
        enum { MII_INDEX_NONE = 0xFFFF };

        // Message codes of npc_SP/NPC_catherine (from the ENG text).
        enum {
            MSG_NONE = 0x00,
            MSG_FIRST_VISIT = 0x01,   // "...This is your first time here..."
            MSG_ASK_SERVICE = 0x02,   // "...change your hairstyle today?..."
            MSG_ASK_SERVICE_F = 0x03, // (the same, for girls)
            MSG_NEVER_MIND = 0x04,    // "Wha--? I mean, sure thing!..."
            MSG_PRICE = 0x05,         // "...that'll be <price>"
            MSG_NO_MONEY = 0x07,      // "...You seem to be short on funds!"
            MSG_TAKE_SEAT = 0x08,    // "...take a seat in front of the mirror!"
            MSG_TO_SEAT = 0x09,      // (empty) the hair service was accepted
            MSG_REMOVE_ITEMS = 0x0A, // "...remove anything that might get in
                                     // the way..."
            MSG_HOW_HAIR = 0x0C,     // "How shall we transform your hair..."
            MSG_HOW_HAIR_2 = 0x0D,   // "Well, <name>, what look are you..."
            MSG_WARM_COLOR = 0x15,   // "...Safe.../Lighthearted!/Fiery!..."
            MSG_WARM_COLOR_2 = 0x16, // "Something a little warmer?..."
            MSG_COOL_COLOR = 0x17,   // "...Mysterious!/Fresh!/Attractive!..."
            MSG_COOL_COLOR_2 = 0x18, // "...something cooler, hmm?..."
            MSG_TO_DONE = 0x19,      // (empty) -> mFinishMsg
            MSG_DONE = 0x1A,         // "We're all done!... 3,000 Bells..."
            MSG_DONE_SAME = 0x1B,    // "...That IS the best hairstyle for you"
            MSG_HAVE_IDEA = 0x1C,    // "...do you have an idea of the
                                     // hairstyle...?"
            MSG_NOT_SURE = 0x1E,     // "I know what you mean!...NOT an easy..."
            MSG_QUESTIONS = 0x1F,    // "...All you have to do is answer my
                                     // questions!"
            MSG_TRENDY = 0x20,       // "...those trendy new hairstyles?..."
            MSG_TREND_COLOR = 0x21,  // "...follow the most recent color fad?"
            MSG_TREND_DONE = 0x22,   // (empty) the trend's colour
            MSG_DONE_TREND = 0x23,   // "Ta-daaaa! You're all set!..."
            MSG_OWN_COLOR = 0x24,    // "Of course not...A warm color..."
            MSG_RETURN_ITEMS = 0x25, // "And here are your things back!"
            MSG_DONE_OWN_COLOR = 0x26,    // "Ta-daaaa! You're all set!..."
            MSG_MASTERPIECE_INTRO = 0x27, // "...The other day, <player> of
                                          // <town>..."
            MSG_MASTERPIECE = 0x28,       // "I recently created a modern
                                          // masterpiece..."
            MSG_MASTERPIECE_AGAIN = 0x29, // "...I kind of messed up..."
            MSG_SAME_COLOR = 0x2A, // "...Would you like the same color, too?"
            MSG_SAME_COLOR_2 = 0x2B,
            MSG_SAME_COLOR_YES = 0x2C,   // (empty) her colour (masterpiece)
            MSG_SAME_COLOR_YES_2 = 0x2D, // (empty) ... (messed up)
            MSG_PICK_COLOR = 0x2E,       // "Sure thing! So, what kind of color"
            MSG_DONE_MASTERPIECE = 0x2F, // "Ta-daaaa! You're all set!..."
            MSG_DONE_MASTERPIECE_AGAIN = 0x30,
            MSG_SWAP_TO_GIRL = 0x31,   // "...good with a girl's hairstyle!..."
            MSG_SWAP_TO_BOY = 0x32,    // "...good with a boy's hairstyle!..."
            MSG_SWAP_SORRY = 0x34,     // "Oh! I'm so sorry!..."
            MSG_SWAP_GIRL = 0x35,      // "...pretend you're a girl!..."
            MSG_SWAP_BOY = 0x36,       // "...pretend you're a boy!..."
            MSG_START_STYLE = 0x37,    // "OK, we're ready to style!..."
            MSG_START_STYLE_2 = 0x38,  // "OK, we're all set!..."
            MSG_NO_WAIT = 0x3D,        // "...There's no wait right now..."
            MSG_SIT_AT_MACHINE = 0x3E, // "...sit somewhere we have a machine"
            MSG_MAKEOVER_FIRST = 0x47, // (empty) -> the makeover offer
            MSG_MAKEOVER_OFFER = 0x48, // "...bitten by the makeover bug..."
            MSG_MAKEOVER_OFFER_2 = 0x49, // "...interested in our makeover..."
            MSG_MAKEOVER_TRIAL = 0x4A,   // "Yes! I think you should!..."
            MSG_MAKEOVER_FEE = 0x4B,     // "...the makeover fee will be 3,000"
            MSG_NO_MII = 0x4E,           // "...there are no Miis..."
            MSG_MAKEOVER_SEAT = 0x4F,    // "Now, be a dear and take a seat..."
            MSG_MAKEOVER_REMOVE_ITEMS = 0x50, // "...taking off your
                                              // accessories..."
            MSG_MAKEOVER_FORGOT = 0x51, // "...you've gotta take everything..."
            MSG_PICK_MII = 0x52,       // "...pick out a makeover model from the
                                       // album?"
            MSG_CONFIRM_MII = 0x53,    // "So, you'd like to try the <Mii>
                                       // style?"
            MSG_PICK_MII_AGAIN = 0x54, // "...take another peek at the album?"
            MSG_START_MAKEOVER = 0x55, // "...I am gonna do you UP!"
            MSG_REMOVE_MII_MASK = 0x56, // "...take off your old Mii makeup"
            MSG_POCKETS_FULL = 0x57,    // "...But you can't do that if you have
                                        // no space"
            MSG_MAKEOVER_DONE_TRIAL = 0x58, // "Ta-daaaa! All finished!..."
            MSG_MAKEOVER_DONE = 0x59, // "...made OVER!...3,000 Bells, please!"
            MSG_SAME_MII = 0x5A,      // "...this is the same thing we did the
                                      // other day!..."
            MSG_MIX_IT_UP = 0x5B,     // "Good idea!...the album again!"
            MSG_PAY_LATER = 0x5C,     // "...you haven't got enough cash..."
            MSG_MII_BROKEN = 0x63,    // "...Your Mii save data...broken!"
            MSG_WELCOME = 0x65,       // "Hi there, sugar! Welcome to
                                      // Shampoodle..."
            MSG_WELCOME_BACK = 0x66,  // "Hi there, sweetie! Oh, it's <name>!"
            MSG_BYE = 0x67,           // "Stop by again whenever..."
            MSG_BYE_HURRY = 0x68, // "...something important must have come up"
            MSG_BYE_TRIAL = 0x69, // "...your trial makeover?..."
            MSG_BYE_TRIAL_YES = 0x6A, // "Great! Then go on and have a seat!"
            MSG_BYE_TRIAL_NO = 0x6B,  // "What?! Well...all right..."
            MSG_ASK_SERVICE_AGAIN = 0x6C, // "...I know exactly what you need."
            MSG_GO_SIT = 0x6E,            // "Go on and take a seat, shug!"
            MSG_AFTER_STYLE = 0x6F, // 0x6F..0x73: "You look like a star!..."
            MSG_AFTER_STYLE_NUM = 5,
            MSG_QUESTIONNAIRE = 0x78, // (empty) -> the girls' / boys' one
            MSG_QUESTION_GIRL = 0x79, // "What style are you going for?..."
            MSG_QUESTION_BOY = 0x83,  // "...Cute./Grown-up./CRAZY!"
            // The questionnaire results (each picks a hairstyle).
            MSG_RESULT_GIRL_STAYING_POWER = 0x8D, // "...staying power..."
            MSG_RESULT_GIRL_NATURAL = 0x8E,       // "...nice and natural!"
            MSG_RESULT_GIRL_YOUNG = 0x8F,      // "...Kids grow up so fast..."
            MSG_RESULT_GIRL_INNOCENT = 0x90,   // "...young and innocent..."
            MSG_RESULT_GIRL_ADORED = 0x91,     // "...Being adored..."
            MSG_RESULT_GIRL_HEART = 0x92,      // "...the heart is like any..."
            MSG_RESULT_GIRL_ADORATION = 0x93,  // "...mass adoration!"
            MSG_RESULT_GIRL_STRONG = 0x94,     // "...Being strong..."
            MSG_RESULT_GIRL_POP_STAR = 0x95,   // "A pop star!..."
            MSG_RESULT_GIRL_MODEL = 0x96,      // "A model!..."
            MSG_RESULT_GIRL_DESIGNER = 0x97,   // "Like a fashion designer!..."
            MSG_RESULT_GIRL_CHARISMA = 0x98,   // "...natural charisma..."
            MSG_RESULT_BOY_DYNAMIC = 0xA1,     // "...dynamic, flexible style"
            MSG_RESULT_BOY_EASY = 0xA2,        // "...very little effort..."
            MSG_RESULT_BOY_YOUNG = 0xA3,       // "...Kids grow up so fast..."
            MSG_RESULT_BOY_BOY = 0xA4,         // "...still just a boy..."
            MSG_RESULT_BOY_BIG_BROTHER = 0xA5, // "A big brother who yearns..."
            MSG_RESULT_BOY_BIGHEARTED = 0xA6,  // "...a bighearted fellow!"
            MSG_RESULT_BOY_BRAVE = 0xA7,       // "...that's really brave..."
            MSG_RESULT_BOY_QUICK = 0xA8,       // "...quick on your feet!"
            MSG_RESULT_BOY_MOVIE_STAR = 0xA9,  // "A movie star!..."
            MSG_RESULT_BOY_ROCK_STAR = 0xAA,   // "A rock star!..."
            MSG_RESULT_BOY_WORK = 0xAB,        // "...it takes work..."
            MSG_RESULT_BOY_CHARISMA = 0xAC,    // "...natural charisma..."
        };
        // The answers of the choices.
        enum {
            ANSWER_HAIR = 0,       // MSG_ASK_SERVICE: "Hairstyle!"
            ANSWER_MAKEOVER = 1,   // ... "Makeover!"
            ANSWER_NEVER_MIND = 2, // ... "Never mind."
            ANSWER_PAY = 0,        // MSG_PRICE
            ANSWER_YES = 0,        // the yes / no choices
            ANSWER_NO = 1,
            ANSWER_MIX_IT_UP = 0,   // MSG_SAME_MII: "Let's mix it up!"
            ANSWER_FINE = 1,        // ... "It's fine!"
            ANSWER_COLOR_OTHER = 4, // the colour choices: "Somethin' else!"
            ANSWER_COLOR_NUM = 5,   // the colour choices
        };

        talk_c(dAcNpcSpCatherine_c *npc);         // 0x11F4
        virtual ~talk_c();                        // 0x12AC
        virtual void init();                      // 0x1320
        virtual void getMsgInfo(msgInfo_s *info); // 0x1770
        virtual void onMessageStart(int arg);     // 0x1788
        virtual void onMessageEnd(int kind);      // 0x18C8
        virtual void onAnswer(int arg);           // 0x2028
        virtual void preExecute();                // 0x23E0
        virtual void onTalkEnd();                 // 0x2428
        virtual void onRequestEnd(int kind);      // 0x25BC

        void start(dActor_c *target); // 0x13A0

        // ---- message / request hooks ----
        void openMiiAlbum();       // 0x2620
        void confirmMii();         // 0x2628
        void resumeTalk();         // 0x2684
        void startStyling();       // 0x26DC
        void startMakeover();      // 0x272C
        void stopProc();           // 0x2784
        void takeOffAccessories(); // 0x27A4: into the pockets
        void askMii();             // 0x2884
        void hideAccessories();    // 0x2898
        void removeMiiMask();      // 0x2920
        void askHair();            // 0x29B0
        void restoreEquip();       // 0x2A88
        void restoreMiiMask();     // 0x2B08
        void endReturnItems();     // 0x2B88
        void startReturn();        // 0x2BA4

        // ---- the walk to the machine and back (mProc) ----
        void startToMachine();    // 0x2BE0
        void execToMachine();     // 0x2CB4
        void startSwitchOn();     // 0x2DC4
        void execTurnToMachine(); // 0x2E4C
        void execSwitchOn();      // 0x2ED4
        void startToChair();      // 0x2FB0
        void execToChair();       // 0x3084
        void execTurnToPlayer();  // 0x320C
        void startMiiMask();      // 0x32C4
        void execMakeover();      // 0x3314
        void startHairChange();   // 0x33F4
        void execPerm();          // 0x344C
        void finishPerm();        // 0x34AC

        u16 getMiiIndex(); // 0x3578: the player's Mii in the album (0xFFFF)

        /* 0x0EA */ u16 mMsgNo; // the message start() picked
        /* 0x0EC */ dAcNpcSpCatherine_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ Proc mProc;     // every frame (preExecute)
        /* 0x114 */ int mKind;      // Kind_e
        /* 0x118 */ int mFinishMsg; // the result message of the hair styling
        /* 0x11C */ int mReturnMsg; // after the walk back (MSG_RETURN_ITEMS)
        /* 0x120 */ u8 mHair;       // the new hairstyle (requestHairChange)
        /* 0x121 */ u8 mHairColor;
        /* 0x122 */ u8 mHairGender;      // HAIR_GENDER_*
        /* 0x124 */ dEquip_c mEquipOrig; // what the player wore
        /* 0x12C */ dEquip_c mEquip;     // ... without the hat and accessory
        /* 0x134 */ u16 mMiiIndex;       // the Mii worn before the makeover
        /* 0x136 */ u8 mExitRequested;   // the player leaves (bgm kept)
        /* 0x137 */ u8 mAccepted;        // the service was accepted: sit down
        /* 0x138 */ u8 mMakeover;        // ... the makeover (not the hair)
        /* 0x139 */ u8 mSpecialMake;     // the machine makes over (not a perm)
        /* 0x13A */ u8 mMessedUp;        // MSG_MASTERPIECE_AGAIN was told
        /* 0x13B */ u8 mItemsRemoved;    // the hat / accessory came off
        /* 0x13C */ u8 mMiiMaskRemoved;
        /* 0x13D */ u8 mAsked;          // MSG_ASK_SERVICE was told
        /* 0x13E */ u8 mAtMachineFront; // execToMachine: the second leg
        /* 0x13F */ u8 mAtChairFront;   // execToChair: the second leg
    }; // size 0x140

    dAcNpcSpCatherine_c() : mCatherineTalk(this) {
        mTurnWalk = 0;
        mWalking = 0;
        mItemsRemoved = 0;
        mServing = 0;
    }
    virtual ~dAcNpcSpCatherine_c() {
    } // weak 0x394C
    virtual void onDemoCmd(int cmd, u8 member); // 0x26C
    void changeState(int state);                // 0x398
    virtual int preCreate();                    // 0x3D0
    virtual int create();                       // 0x41C
    virtual int doDelete();                     // 0x48C
    virtual int vtB0();                         // 0x4D0
    virtual int getEarType();                   // 0x35C0

    // ---- states ----
    void initIdle();           // 0x518 (+ a dead blr at 0x538)
    void executeIdle();        // 0x53C
    void initLook();           // 0x5E0
    void stepLook();           // 0x65C
    void executeLook();        // 0x6BC
    void initTalk();           // 0x6FC
    void stepTalkWalk();       // 0x788
    void stepTalk();           // 0x87C
    void executeTalk();        // 0x8D4
    void initGreet();          // 0x914
    void executeGreet();       // 0x918
    void initSeeOff();         // 0x964
    void executeSeeOff();      // 0x968
    void initToChair();        // 0x9B4
    void stepToChairStart();   // 0x9F8
    void stepToChairWalk();    // 0xBB8
    void executeToChair();     // 0xD04
    void initServed();         // 0xD44
    void stepServedTurn();     // 0xDB0 (+ a dead blr at 0xDD8)
    void executeServed();      // 0xDDC
    void initToCounter();      // 0xE1C
    void stepToCounterFront(); // 0xEE0
    void stepToCounterTurn();  // 0xFC0
    void stepToCounterDone();  // 0x1034 (+ a dead blr at 0x1054)
    void executeToCounter();   // 0x1058
    void initSeat();           // 0x1098
    void executeSeat();        // 0x10F4

    BOOL isPlayerAtExit(); // 0x1144: in front of the locked exit
    BOOL isPlayerSeated(); // 0x119C: on one of the chairs

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dCatherineDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep;
    /* 0x1C90 */ talk_c mCatherineTalk;
    /* 0x1DD0 */ u8 mAfterStyleMsg;  // MSG_AFTER_STYLE + rnd
    /* 0x1DD1 */ u8 mTurnWalk;       // the next walk turns first
    /* 0x1DD2 */ u8 mWalking;        // the talk started while walking
    /* 0x1DD3 */ u8 mItemsRemoved;   // back via the counter (STATE_TO_COUNTER)
    /* 0x1DD4 */ u8 mServing;        // the service runs (STATE_SERVED)
    /* 0x1DD8 */ mVec3_c mTargetPos; // where she walks to
}; // size 0x1DE4

#endif
