#ifndef D_A_NPC_SP_PYONTARO_HPP
#define D_A_NPC_SP_PYONTARO_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_item.hpp>

// Zipper T. Bunny on Bunny Day (REL d_a_npc_sp_pyontaroNP, profile
// A_NPC_SP_PYONTARO, messages "npc_SP/NPC_pyontarou"). He dances in place
// while a player is near (mDaub.mDancing) and stops to catch his breath when
// they leave (mDaub.mResting). He sells a shovel to players without one,
// trades each Bunny foil for an egg furniture item and otherwise comments on
// the eggs still in the pockets or buried in town (dNpcInfo::pyontaro_c
// counts the talks). Talking to his back gets its own complaint.
// Class names from RTTI ("dAcNpcSpPyontaro_c", "dAcNpcSpPyontaro_c::talk_c");
// the rest is named from behaviour.
class dAcNpcSpPyontaro_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,        // dance (or rest) in place, facing the front
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // mStep[STATE_REMOTE] (l_remoteSteps): follows the owner's daub.
    enum RemoteStep_e {
        REMOTE_STEP_IDLE,  // neither dancing nor resting
        REMOTE_STEP_REST,  // mDaub.mResting
        REMOTE_STEP_DANCE, // mDaub.mDancing
        REMOTE_STEP_DONE
    };

    typedef void (dAcNpcSpPyontaro_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The net daub data (getDaubData / setDaubData).
    struct daub_s {
        u32 mDancing : 1; // a player is near: HMN_ANM_PYN_DANCE +
                          // SE_PyontarouDance
        u32 mResting : 1; // the players left: catching his breath
        u32 _0 : 30;
        u32 _4;
    }; // size 0x8

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_pyontarou codes, named from the English text.
        enum {
            MSG_GREETING =
                0x01, // "Boingity boing! Hello there!" (introduction)
            MSG_FOIL =
                0x08, // "Flippity skippity!..." -> "You found a Bunny foil!"
            MSG_FOIL_FOUND = 0x0A, // "...Hand over that prize ticket, please!"
            MSG_FOIL_AGAIN = 0x0D, // ... -> "You found another one!"
            MSG_EGG =
                0x12, // eggs in the pockets: "There are eggs, and then..."
            MSG_EGG_2 = 0x15,     // "..." -> "There are eggs..."
            MSG_EGG_3 = 0x17,     // "And inside the eggs, oh, what's inside!"
            MSG_EGG_4 = 0x19,     // "Open the stupid egg!"
            MSG_EGG_5 = 0x1A,     // "..." -> "Open the egg! Open the egg!"
            MSG_NO_EGGS = 0x1D,   // "..." -> "Can't even remember if there are
                                  // any eggs left"
            MSG_EGGS_LEFT = 0x20, // eggs still buried: "...Find anything?"
            MSG_EGGS_LEFT_2 = 0x22, // "...Did you find any Bunny eggs?"
            MSG_EGGS_LEFT_3 =
                0x24, // "...Would you mind standing over there maybe?"
            MSG_EGGS_LEFT_4 =
                0x27, // "...Should I tell <> that I'm running out of prizes?"
            MSG_BACK = 0x2A, // "Hey, kid... How about talkin' to my face"
            MSG_BACK_AGAIN =
                0x2B, // "Hey... Would you mind not looking at my back?"
            MSG_NO_SHOVEL =
                0x2C, // "Boingity hoy! Hey there!" (the player has no shovel)
            MSG_OFFER_SHOVEL =
                0x2F, // "Tell you what! I can sell you a shovel..."
            MSG_NO_SHOVEL_AGAIN = 0x30, // "Boingity hey! Well, hello!"
            MSG_SHOVEL_DECLINED =
                0x32, // "Boingity weird? You don't mean me, do you?!"
            MSG_SHOVEL_NO_MONEY = 0x33, // "...Exact change only, you got it?"
            MSG_SHOVEL_POCKETS_FULL =
                0x34,               // "...when your pockets are full..."
            MSG_SHOVEL_SOLD = 0x35, // "Boingity yay! Thank you..."
            MSG_SHOVEL_RESTART =
                0x36, // "All right, let's take it from the top..."
        };
        // onMessageStart: the message's animation (BMG entry +0x9).
        enum { START_DANCE = 0x10, START_STAND = 0x11 };
        // onMessageEnd requests (BMG entry +0xA).
        enum { END_REQ_TAKE_FOIL = 0x12, END_REQ_GIVE_PRIZE = 0x13 };
        // The answers of MSG_OFFER_SHOVEL.
        enum { ANSWER_YES, ANSWER_NO };

        talk_c(dAcNpcSpPyontaro_c *npc);          // 0x10B4
        virtual ~talk_c();                        // 0x1120
        virtual void onMessageStart(int arg);     // 0x1568
        virtual void onMessageEnd(int kind);      // 0x164C
        virtual void onAnswer(int arg);           // 0x1840
        virtual void init();                      // 0x1178
        virtual void preExecute();                // 0x1914
        virtual void onTalkEnd();                 // 0x1918
        virtual void getMsgInfo(msgInfo_s *info); // 0x1550
        virtual void onRequestEnd(int kind);      // 0x191C
        void start(dActor_c *target);             // 0x12E8
        void takeFoil();    // 0x1980: the player hands it over
        void givePrize();   // 0x199C
        void giveShovel();  // 0x19AC
        void removeFoil();  // 0x19F4: out of the pockets
        void pickUpPrize(); // 0x1A78
        void buyShovel();   // 0x1AB0: pick it up and pay

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpPyontaro_c *mpOwner;
        /* 0x0F0 */ Proc
            mOnMessageEnd; // one-shot hooks (cleared after the call)
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ dItem::Item mItem; // the foil taken, then the prize
    }; // size 0x10C

    dAcNpcSpPyontaro_c() : mPyontaroTalk(this) {
    }
    virtual ~dAcNpcSpPyontaro_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x368
    void changeState(int state);                // 0x484
    virtual int preCreate();                    // 0x4BC
    virtual int create();                       // 0x508
    virtual int doDelete();                     // 0x648
    virtual int vtB0();                         // 0x68C
    void initIdle();                            // 0x744
    void executeIdle();                         // 0x7A4
    void initLook();                            // 0x9C0
    void stepLook();                            // 0xA24
    void executeLook();                         // 0xA84
    void initTalk();                            // 0xAC4
    void stepTalk();                            // 0xB50
    void executeTalk();                         // 0xB9C
    void initRemote();                          // 0xBDC
    void initRemoteIdle();                      // 0xC40
    void stepRemoteIdle();                      // 0xC90
    void initRemoteRest();                      // 0xCE8
    void stepRemoteRest();                      // 0xDA0
    void initRemoteDance();                     // 0xDF8
    void stepRemoteDance();                     // 0xE9C
    void executeRemote();                       // 0xEF4
    void initRemoteTalk();                      // 0xFF0
    void executeRemoteTalk();                   // 0xFFC
    BOOL isPlayerBehind() const;                // 0x1050
    BOOL isDancing() const;                     // 0x109C
    BOOL isResting() const;                     // 0x10A8

    mAng getTalkAngle() const {
        dActor_c *target = mPyontaroTalk.mpActActor;
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
    /* 0x1C80 */ daub_s mDaub;
    /* 0x1C88 */ u8 mStep[STATE_COUNT];
    /* 0x1C90 */ talk_c mPyontaroTalk;
    /* 0x1D9C */ u8 mCreating; // set while create runs (initRemoteRest)
}; // size 0x1DA0

#endif
