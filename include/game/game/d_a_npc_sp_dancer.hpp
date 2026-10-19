#ifndef D_A_NPC_SP_DANCER_HPP
#define D_A_NPC_SP_DANCER_HPP

#include <game/game/d_a_npc_sp.hpp>

// Pave, the dancer of the Festivale (REL d_a_npc_sp_dancerNP, profile
// A_NPC_SP_DANCER, messages "npc_SP/NPC_berliner"). He dances in the plaza and
// asks for candy of the day's colour (dSaveTown_c::mDailyRand); for three of
// them he gives a present. His per-visit state is dNpcInfo::dancer_c.
//
// Class names from RTTI ("dAcNpcSpDancer_c", "dAcNpcSpDancer_c::talk_c"); the
// rest is named from behaviour.
class dAcNpcSpDancer_c : public dAcNpcSp_c {
  public:
    enum State_e {
        STATE_IDLE,        // turn back to the front and dance
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player, then turn back
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_NUM
    };
    // mStep of STATE_IDLE / STATE_LOOK / STATE_TALK.
    enum IdleStep_e { IDLE_STEP_TURN, IDLE_STEP_DANCE, IDLE_STEP_NUM };
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_NUM };
    enum TalkStep_e { TALK_STEP_TALK, TALK_STEP_TURN, TALK_STEP_NUM };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpDancer_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: candy talk and the present.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_berliner (ENG text). Several are
        // followed by one variant per candy colour (dSaveTown_c::mDailyRand,
        // 0 blue, 1 red, 2 yellow, 3 green).
        enum Msg_e {
            MSG_FIRST_TALK = 0x01, // "One and TWO!..."
            MSG_TRY_AGAIN = 0x05,  // "Festivale comes once each year!"
            MSG_COSTUME = 0x06,    // "...in your woo-hoo plumage"
            MSG_NO_DANCER = 0x07,  // "Pave see that your feet not on fire."
            MSG_NEED_KIND = 0x0A,  // "But not just ANY kind!" (+1 + 2 * colour)
            MSG_NO_CANDY = 0x14,   // "Do you have what Pave needs?!"
            MSG_NO_CANDY_ANY = 0x16, // "You don't have any!" (+1 + 2 * colour)
            MSG_NO_CANDY_AGAIN = 0x1F, // "never have candy again" (+1 + colour)
            MSG_WRONG_CANDY =
                0x25, // "Can it be?! You are giving Pave his candy?!"
            MSG_WRONG_CANDY_SORRY =
                0x26, // "Is it finally time?!" (+1 + colour: "Sorry...")
            MSG_WRONG_CANDY_AGAIN =
                0x2C,           // "only a specific kind" (+1 + colour)
            MSG_TOO_FEW = 0x32, // "Candy?! You are giving Pave his candy?!"
            MSG_TOO_FEW_AGAIN = 0x37,   // "You DO have ... but..."
            MSG_GIVE = 0x3A,            // "You are giving Pave his candy?!"
            MSG_GIVE_TAKE = 0x3D,       // "GIVE IT TO ME!"
            MSG_GIVE_AGAIN = 0x3E,      // "Is there something else ...?"
            MSG_GIVE_AGAIN_TAKE = 0x41, // "WOO-HOO!"
            MSG_EAT = 0x42,             // "CHOMP!"
            MSG_TASTY = 0x46,           // "Woo-hoo! Tasty!"
            MSG_GIFT = 0x47,            // "Take this ...!"
            MSG_GIFT_AGAIN = 0x48,      // "This time, Pave has different gift"
            MSG_FULL = 0x49,            // "full tummy" (+ 4 * new colour)
            MSG_NEXT_CANDY = 0x4A,      // the next colour (+ 4 * colour)
            MSG_FULL_AGAIN = 0x4B,      // "full tummy again" (+ 4 * new colour)
        };
        // onMessageStart requests (MsgInfo_s::mStartReq).
        enum StartReq_e {
            START_REQ_DANCE = 0x10,      // HMN_ANM_PCK_DANCE2
            START_REQ_STOP_DANCE = 0x11, // HMN_ANM_NML_WAIT1
        };
        enum { CANDY_NUM = 3 }; // the candies of the day's colour he wants

        talk_c(dAcNpcSpDancer_c *npc);            // 0xA18
        virtual ~talk_c();                        // 0xA88
        virtual void onMessageStart(int arg);     // 0xD08
        virtual void onMessageEnd(int kind);      // 0xDD4
        virtual void onAnswer(int arg);           // 0x100C
        virtual void init();                      // 0xAE0
        virtual void preExecute();                // 0x1010
        virtual void onTalkEnd();                 // 0x1014
        virtual void getMsgInfo(msgInfo_s *info); // 0xCF0
        virtual void onRequestEnd(int kind);      // 0x1038
        void start(dActor_c *target);             // 0xB58

        // ---- hooks (mOnMessageEnd / mOnRequestEnd) ----
        void takeCandy();   // 0x109C
        void giveGift();    // 0x10B8
        void removeCandy(); // 0x10C8
        void receiveGift(); // 0x116C

        /* 0x0EA */ u16 mMsgNo; // the first message (start)
        /* 0x0EC */ dAcNpcSpDancer_c *mpOwner;
        /* 0x0F0 */ Proc
            mOnMessageEnd; // one-shot hooks (cleared after the call)
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ dItem::Item mCandy; // the candy of the day
        /* 0x10A */ dItem::Item mGift;  // his present
    }; // size 0x10C

    dAcNpcSpDancer_c() : mDancerTalk(this) {
    }
    virtual ~dAcNpcSpDancer_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x248
    void changeState(int state);                // 0x364
    virtual int preCreate();                    // 0x39C
    virtual int create();                       // 0x3F4
    virtual int doDelete();                     // 0x488
    virtual int vtB0();                         // 0x4B4
    void initIdle();                            // 0x56C
    void stepIdleTurn();                        // 0x5D8
    void stepIdleDance();                       // 0x634
    void executeIdle();                         // 0x638
    void initLook();                            // 0x678
    void stepLook();                            // 0x70C
    void executeLook();                         // 0x76C
    void initTalk();                            // 0x7AC
    void stepTalk();                            // 0x818
    void stepTalkTurn();                        // 0x888
    void executeTalk();                         // 0x8D8
    void initRemote();                          // 0x918
    void executeRemote();                       // 0x924
    void initRemoteTalk();                      // 0x9B8
    void executeRemoteTalk();                   // 0x9C4
    virtual int getEarType();                   // 0x1208

    mAng getTalkAngle() const {
        dActor_c *target = mDancerTalk.mpActActor;
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
    /* 0x1C80 */ u8 mStep[STATE_NUM];
    /* 0x1C88 */ talk_c mDancerTalk;
    /* 0x1D94 */ u8 mFacedAway; // the player was behind him (initLook)
}; // size 0x1D98

#endif
