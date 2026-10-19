#ifndef D_A_NPC_SP_CAFE_HPP
#define D_A_NPC_SP_CAFE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (m3d,
// fBase_c, dDvd, dActor_c, dDebugPage_c, then the npc classes and
// daRmobj_c::model_c).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_effect.hpp>
#include <game/mLib/m_mtx.hpp>

// Brewster, the master of the Roost (REL d_a_npc_sp_cafeNP, profile
// A_NPC_SP_CAFE). He sells coffee (and serves it with the cup, pot, beans and
// milk props of the counter), keeps the gyroids players leave with him, and
// keeps quiet while K.K. Slider plays. The counter props are the models
// "obj_cafe1".."obj_cafe6" of /Rmobj (see the model members).
//
// Class names from RTTI ("dAcNpcSpCafe_c", "dAcNpcSpCafe_c::talk_c",
// "dAcNpcSpCafe_c::dCafeDebugPage_c", "dNpcCafe_Calibrate_HIO_c"); everything
// else is named from behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dCafeDebugPage_c), so dCafeDebugPage_c is declared first.
class dAcNpcSpCafe_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,        // face the counter; speak to a player who sits down
        STATE_LOOK,        // wait (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // mStep of STATE_LOOK (l_lookSteps; initLook starts done) and STATE_TALK
    // (l_talkSteps).
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_DONE };
    enum TalkStep_e {
        TALK_STEP_TURN,     // wait for the talk, then turn back to the counter
        TALK_STEP_TURN_END, // wait for the turn, finish the talk
        TALK_STEP_DONE
    };

    typedef void (dAcNpcSpCafe_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dCafeDebugPage_c : public dDebugPage_c {};

    // The message receptor: picks the greeting, sells and serves the coffee
    // and stores / returns the gyroids.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // The kinds of talk (mMsgKind; onDemoCmd).
        enum MsgKind_e {
            MSG_KIND_TALK, // the player talks to him
            MSG_KIND_SPEAK // he speaks to the player who sat down
        };

        // The steps of the coffee service (mMode). startTurn turns him toward
        // the step's place (l_HIO.mTurnFrames[mode] frames) and starts the
        // next step.
        enum Mode_e {
            MODE_START,      // take a cup from the shelf
            MODE_TAKE_CUP,   // then pour
            MODE_POUR,       // then serve
            MODE_POUR_MILK,  // then add the pigeon milk
            MODE_ADD_MILK,   // then serve the second cup
            MODE_SERVE,      // the cup slides to the player
            MODE_SERVE_MILK, // the second cup slides to the player
            MODE_DRINK,      // the player drinks (getVoiceMode)
            MODE_CLEAR,      // take the cup back
            MODE_PUT_AWAY,   // put it away, back to polishing
            MODE_NUM
        };

        enum { COFFEE_PRICE = 200 }; // Bells

        // Message codes of npc_SP/NPC_master (ENG text in the comments).
        enum {
            MSG_GREETING =
                0x01, // 0x01..0x09: "...Welcome to the Roost." (by visits)
            MSG_ASK_COFFEE =
                0x11, // "Need a little eye-opener? ..." (0x11..0x16 by visits)
            MSG_DECLINED = 0x17, // 0x17..0x1A: "...Suit yourself."
            MSG_THANKS = 0x1C,   // 0x1C..0x21: "Thanks..." (by visits)
            MSG_OFFER_MILK =
                0x22, // "Would you care for a splash of pigeon milk?"
            MSG_MILK_YES = 0x23,   // "Coming right up..."
            MSG_MILK_NO = 0x24,    // "Black it is."
            MSG_SERVE = 0x25,      // 0x25..0x2A: "There you go..." (by visits)
            MSG_TOO_HOT = 0x2B,    // 0x2B..0x30: "'It's gotta cool'? Hmm..."
            MSG_REACT_MILK = 0x31, // " Sweet and smooth! ... Pigeon Blend!"
            MSG_REACT = 0x32,      // 0x32..0x3C: the player's reaction
            MSG_THANK_YOU = 0x3D,  // "...Thank you."
            MSG_ASK_VALENTINE = 0x3E,   // "It's 200 Bells a cup..."
            MSG_ALL_RIGHT = 0x3F,       // "All right..."
            MSG_SERVE_VALENTINE = 0x40, // "There you go... So... Drink up."
            MSG_TOO_HOT_VALENTINE = 0x41,
            MSG_REACT_CHOCOLATE = 0x42, // "This is hot chocolate!!"
            MSG_HAPPY_VALENTINE = 0x43, // "...Happy Valentine's Day."
            MSG_SPECIAL_DAY = 0x44,     // "It's a special day..."
            MSG_REGULAR = 0x45,    // 0x45..0x4A: "...I roast my own blends ..."
            MSG_NO_HOME = 0x4D,    // "...Hi."
            MSG_SHOW_SOON = 0x4E,  // "The show's about to begin, so..."
            MSG_SHOW_QUIET = 0x4F, // "Not a peep out of you during the show..."
            MSG_VISITOR_WELCOME = 0x51, // "...Welcome to the Roost."
            MSG_VISITOR_SIT = 0x52,     // "Take a seat at the counter ..."
            MSG_VISITOR_ASK = 0x53,     // "Cup of java's 200 Bells..."
            MSG_VISITOR_SERVE =
                0x56, // "Coming right up... You got a favorite blend ..."
            MSG_VISITOR_HERE =
                0x57, // "Here it is... This is the pride of <town>."
            MSG_VISITOR_TOO_HOT =
                0x58, // "...I'm sorry? We...don't really do that here..."
            MSG_VISITOR_REACT = 0x59,  // 0x59..0x5D
            MSG_VISITOR_THANKS = 0x5E, // "...Much obliged."
            MSG_VISITOR_AGAIN = 0x5F,  // 0x5F..0x63: "Spread the word..."
            MSG_VISITOR_TOWN =
                0x60, // "I hear coffee in <town> is pretty good, too..."
            MSG_PERFORMANCE =
                0x10,        // "No pictures during the performance, please..."
            MSG_SEAT = 0x0A, // 0x0A..0x0F: "Counter seat's open..." (by visits)
            MSG_BUY = 0x66,  // (empty) the player buys a cup: pay COFFEE_PRICE
            MSG_GYROID_THANKS =
                0x67, // 0x67..0x69: "Ah, thank you for your support..."
            MSG_GYROID_NICE = 0x6A, // 0x6A..0x6C: "This is a nice one..."
            MSG_GYROID_ASK =
                0x75, // "...Say. ... I see you've got a gyroid there..."
            MSG_GYROID_NEXT_TIME = 0x76, // "Right... Next time, maybe."
            MSG_GYROID_CHANGE = 0x77,    // "Change of heart, coo? ..."
            MSG_GYROID_STORE = 0x78, // "Coo... Lovely gyroid you've got there."
            MSG_GYROID_STORED = 0x79, // "<item>... In fine condition, coo..."
            MSG_GYROID_HAVE = 0x7A,   // "... I think you already have one of
                                      // those stored here."
            MSG_GYROID_MORE = 0x7B,   // "Coo? If you've got more, ..."
            MSG_GYROID_LATER = 0x7C,  // "Oh. Next time, maybe..."
            MSG_GYROID_SAFE = 0x7D,   // 0x7D..0x7F: "Don't worry. ..."
            MSG_POCKETS_FULL = 0x80,  // "That'll be a little tough, ... Your
                                      // pockets are stuffed..."
            MSG_RETURN_LATER = 0x81,  // "Maybe later, coo? ..."
            MSG_RETURNED =
                0x82, // 0x82..0x84: "I took very good care of your <item>..."
        };

        talk_c(dAcNpcSpCafe_c *npc);              // 0x1704
        virtual ~talk_c();                        // 0x17C4
        virtual void onMessageStart(int arg);     // 0x1EF0
        virtual void onMessageEnd(int kind);      // 0x1FB0
        virtual void onAnswer(int arg);           // 0x249C
        virtual void init();                      // 0x1850
        virtual int getVoiceMode();               // 0x1EC4
        virtual void preExecute();                // 0x2984
        virtual void onTalkEnd();                 // 0x29CC
        virtual void getMsgInfo(msgInfo_s *info); // 0x1ED8
        virtual void onRequestEnd(int kind);      // 0x2BD4

        void start(dActor_c *target); // 0x1904
        BOOL isKKOnStage();           // 0x5D18: K.K. Slider is on the stage

        // ---- message / request hooks ----
        void closeCoffeeMsg(); // 0x2C38
        void closeMsg();       // 0x2C9C
        void selectGyroid();   // 0x2CF4
        void openGyroidMenu(); // 0x2D80
        void startServe();     // 0x2DDC
        void startDrink();     // 0x2E18
        void startReaction();  // 0x2E4C
        void storeGyroid();    // 0x2EEC
        void returnGyroid();   // 0x2FD8
        void stopProc();       // 0x3060
        void startSteam();     // 0x3080
        void nextServeMsg();   // 0x30A0

        // ---- the coffee service (mProc) ----
        void startTurn();      // 0x30FC
        void execTurn();       // 0x3398
        void initTakeCup();    // 0x38E0
        void execTakeCup();    // 0x395C
        void execTakeCupEnd(); // 0x3BB0 (+ a dead blr at 0x3C90)
        void initPour();       // 0x3C94
        void execPour();       // 0x3D90
        void initAddMilk();    // 0x4128
        void execAddMilk();    // 0x42BC
        void initServe();      // 0x473C
        void execServe();      // 0x48E4
        void initServeMilk();  // 0x4B5C
        void execServeMilk();  // 0x4D04
        void execSteam();      // 0x4F7C
        void startDrinkAnm();  // 0x5018
        void execDrink();      // 0x5124
        void startReturnCup(); // 0x5274
        void execWaitReturn(); // 0x5330
        void initClear();      // 0x5370
        void execClear();      // 0x5458
        void initPutAway();    // 0x57E8
        void execPutAway();    // 0x58F8
        void initPolish();     // 0x5C00
        void finishServe();    // 0x5CB4

        /* 0x0EC */ int mMode;    // Mode_e
        /* 0x0F0 */ u16 mMsgCode; // the next message (getMsgInfo)
        /* 0x0F4 */ dAcNpcSpCafe_c *mpOwner;
        /* 0x0F8 */ dLevelEffect_c mEffect; // "af_obj_coffee" (the steam)
        /* 0x18C */ Proc mOnMessageEnd;     // one-shot hooks
        /* 0x198 */ Proc mOnAnswer;
        /* 0x1A4 */ Proc mOnRequestEnd;
        /* 0x1B0 */ Proc
            mProc; // the serving sequence, every frame (preExecute)
        /* 0x1BC */ int mMsgKind;      // MsgKind_e
        /* 0x1C0 */ dItem::Item mItem; // the gyroid (stored / returned)
        /* 0x1C4 */ int mServeMsg;     // the message after the serving
        /* 0x1C8 */ int mReactMsg;     // the player's reaction (after drinking)
        /* 0x1CC */ int mThanksMsg;    // after the reaction
        /* 0x1D0 */ u8 mServed;        // the player bought a coffee (onTalkEnd)
        /* 0x1D1 */ u8
            mValentine; // February 14th (6 am on), for a regular of the town
        /* 0x1D2 */ u8 mMilk;         // pigeon milk added
        /* 0x1D3 */ u8 mGyroidStored; // a gyroid was stored in this talk
        /* 0x1D4 */ s16 mTurnAngle;   // the angle he turns to (startTurn)
    }; // size 0x1D8

    dAcNpcSpCafe_c() : mTalkCafe(this), mBasePos(376.0f, 24.0f, 284.0f) {
        mPolishing = 1;
        mDrawPolish = 1;
        mDrawCup = 0;
        mUseCup2 = 0;
    }
    virtual ~dAcNpcSpCafe_c() {
    } // 0x6208
    virtual void onDemoCmd(int cmd, u8 member);              // 0x628
    void changeState(int state);                             // 0x7A8
    virtual int preCreate();                                 // 0x7E0
    virtual int create();                                    // 0x844
    virtual int doDelete();                                  // 0xB54
    virtual int vtB0();                                      // 0xC30
    virtual int draw();                                      // 0x1034
    virtual int canSpeak(dDemoActor_c *actor);               // 0x10BC
    void initIdle();                                         // 0x110C
    void executeIdle();                                      // 0x1154
    void initLook();                                         // 0x11D0
    void stepLook();                                         // 0x1214
    void executeLook();                                      // 0x1274
    void initTalk();                                         // 0x12B4
    void stepTalkTurn();                                     // 0x135C
    void stepTalkEnd();                                      // 0x13CC
    void executeTalk();                                      // 0x1440
    void initRemote();                                       // 0x1480
    void executeRemote();                                    // 0x1484
    void initRemoteTalk();                                   // 0x1540
    void executeRemoteTalk();                                // 0x1544
    BOOL isPlayerSeated();                                   // 0x1598
    void setBusy();                                          // 0x15F0
    void clearBusy();                                        // 0x1640
    static BOOL isGyroid(const dItem::Item *item, int flag); // 0x1690

    mAng getTalkAngle() const {
        dActor_c *target = mTalkCafe.mpActActor;
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
        u32 mBusy : 1; // serving coffee (no K.K. idle animation, no talk)
        u32 _0 : 31;
        u32 _4;
    }; // size 0x8

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dCafeDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep;
    /* 0x1C90 */ talk_c mTalkCafe;
    /* 0x1E68 */ daRmobj_c::model_c mPolishCupMdl;
    /* 0x2014 */ daRmobj_c::model_c mClothMdl;
    /* 0x21C0 */ daRmobj_c::model_c mCupMdl;
    /* 0x236C */ daRmobj_c::model_c mSaucerMdl;
    /* 0x2518 */ daRmobj_c::model_c mMakerMdl;
    /* 0x26C4 */ daRmobj_c::model_c mCup2Mdl;
    /* 0x2870 */ mMtx_c mPolishCupMtx; // the hand matrices while he holds them
    /* 0x28A0 */ mMtx_c mClothMtx;
    /* 0x28D0 */ mMtx_c mCupMtx;
    /* 0x2900 */ mMtx_c mSaucerMtx;
    /* 0x2930 */ mMtx_c mMakerMtx; // at mBasePos, rotated
    /* 0x2960 */ mVec3_c mBasePos; // (376, 24, 284)
    /* 0x296C */ mMtx_c mCup2Mtx;
    /* 0x299C */ u8 mPolishing;  // the cup follows his hands (vtB0)
    /* 0x299D */ u8 mDrawPolish; // draw the cup and saucer
    /* 0x299E */ u8 mDrawCup;
    /* 0x299F */ u8 mUseCup2;  // the second pot instead of the first
    /* 0x29A0 */ u8 mKKAnm;    // playing along with K.K. Slider
    /* 0x29A1 */ u8 mLookedAt; // looked at the player once (vtB0)
    /* 0x29A4 */ daub_s mDaub;
}; // size 0x29AC

#endif
