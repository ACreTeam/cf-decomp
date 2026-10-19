#ifndef D_A_NPC_SP_PUMPKING_HPP
#define D_A_NPC_SP_PUMPKING_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes). The page uses the
// dDebugPage_c defaults, so the base vtable is emitted here.
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_private_data.hpp>

// Jack, the czar of Halloween (REL d_a_npc_sp_pumpkingNP, profile
// A_NPC_SP_PUMPKING, messages "npc_SP/NPC_pumpking"). On Halloween night he
// stands at his spot (dSaveTown_c::mNpcSpotB, dNpcInfo::pumpking_c) and asks
// for candy: candy gets a present (FROM_HALLOWEEN), anything else, no candy or
// a refusal gets a trick (a pumpkin head, a moldy or patched shirt, or a
// jack-in-the-box in a pocket). After the talk he plays his disappear
// animation, vanishes and moves to a new spot.
//
// Class names from RTTI ("dAcNpcSpPumpking_c", "dAcNpcSpPumpking_c::talk_c",
// "dAcNpcSpPumpking_c::dPumpkingDebugPage_c"); everything else is named from
// behaviour.
class dAcNpcSpPumpking_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,        // wander around the spot
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player, then disappear
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // mStep[STATE_LOOK] (l_lookSteps) and mStep[STATE_TALK] (l_talkSteps).
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_DONE };
    enum TalkStep_e {
        TALK_STEP_TALK,      // until the talk is over
        TALK_STEP_DISAPPEAR, // the disappear animation (vtB0 starts it)
        TALK_STEP_LEAVE,     // mLeaveTimer, then the demo ends
        TALK_STEP_DONE
    };
    enum {
        RELOCATE_FRAMES = 600,   // after his spot was used, before he moves
        LEAVE_FRAMES = 60,       // after the disappear animation
        LEAVE_HIDE_FRAME = 10,   // mLeaveTimer value that clears mPresent
        SPOT_RETRY_FRAMES = 300, // before the next setSpot
    };

    typedef void (dAcNpcSpPumpking_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty: the dDebugPage_c defaults).
    class dPumpkingDebugPage_c : public dDebugPage_c {};

    // The message receptor: the candy talk.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_pumpking (from the ENG text).
        enum {
            MSG_TRICK_OR_TREAT = 0x01, // "Trick or treat?"
            MSG_ASK_CANDY =
                0x02, // "...Care to grease my palm with a little sugar?"
            MSG_ASK_CANDY_NONE = 0x03, // the same, the player has no candy:
                                       // "You do have candy, right?"
            MSG_GIVE_CANDY =
                0x04, // "Well then, let's have that candy, pumpkin..."
            MSG_CANDY_THANKS = 0x05,  // "Ahhhh...scrumptious! Thanks, pumpkin!"
            MSG_CANDY_PRESENT = 0x06, // "...Take this as my thanks!"
            MSG_CANDY_GIFT = 0x07,    // "It's my own personal <item>, pumpkin!"
            MSG_CANDY_THANKS_2 = 0x08, // "Excellent! Thanks, pumpkin!"
            MSG_CANDY_PRESENT_2 =
                0x09, // "...I hope you like getting presents as much as I do!"
            MSG_CANDY_GIFT_2 = 0x0A,     // "It's <item>, pumpkin."
            MSG_NO_CANDY = 0x0B,         // "Whaaat?! ... You haven't got any?"
            MSG_NO_CANDY_TRICK = 0x0C,   // "Do you get it now?"
            MSG_NO_CANDY_2 = 0x0D,       // "What's that?! You don't have any?"
            MSG_NO_CANDY_TRICK_2 = 0x0E, // "Do you understand now?"
            MSG_NOT_CANDY =
                0x0F, // "This isn't candy, pumpkin. ... Try THIS out"
            MSG_NOT_CANDY_TRICK =
                0x10, // "So, maybe now you've seen the error of your ways"
            MSG_NOT_CANDY_2 =
                0x11, // "Yes! Candy! ... This isn't candy, pumpkin."
            MSG_NOT_CANDY_TRICK_2 =
                0x12, // "Now do you see what happens when you disappoint me?"
            MSG_REFUSED = 0x13,       // "\"No way,\" you say?"
            MSG_REFUSED_TRICK = 0x14, // "Now you get it, right?"
            MSG_REFUSED_2 = 0x15,     // "No?! NO?! Am I hearing you correctly?"
            MSG_REFUSED_TRICK_2 = 0x16, // "Bring me a treat next time"
        };
        // Answers of MSG_ASK_CANDY / MSG_ASK_CANDY_NONE / MSG_GIVE_CANDY.
        enum { ANSWER_YES = 0, ANSWER_NO = 1 };

        talk_c(dAcNpcSpPumpking_c *npc);
        virtual ~talk_c();
        virtual void init();
        virtual void getMsgInfo(msgInfo_s *info);
        virtual void onMessageStart(int arg);
        virtual void onMessageEnd(int kind);
        virtual void onAnswer(int arg);
        virtual void preExecute();
        virtual void onTalkEnd();
        virtual void onRequestEnd(int kind);

        void start(dActor_c *target);
        void reqSelectCandy();  // mOnAnswer: the pocket menu
        void reqReceiveItem();  // the chosen item to him
        void reqWait();         // mOnMessageEnd before the present
        void reqTrickPose();    // mOnMessageEnd (not candy)
        void reqGiveGift();     // mOnMessageEnd: his present
        void trickClothes();    // the trick
        void onCandySelected(); // mOnRequestEnd of reqSelectCandy
        void removeItem();      // the given item leaves the pocket
        void reqThanksPose();   // mOnRequestEnd after the candy
        void closeThanks();
        void trickAfterPose(); // mOnRequestEnd (not candy)
        void onThanksDone();   // empty
        void pickUpGift();     // mOnRequestEnd: his present
        void onTrickDone();    // empty
        void trickPockets();   // a pocket item becomes a jack-in-the-box

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpPumpking_c *mpOwner;
        /* 0x0F0 */ dItem::Item mItem;  // the item the player gave
        /* 0x0F2 */ dItem::Item mGift;  // his present
        /* 0x0F4 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x100 */ Proc mOnAnswer;
        /* 0x10C */ Proc mOnRequestEnd;
        /* 0x118 */ Proc mNextRequestEnd; // becomes mOnRequestEnd after it ran
        /* 0x124 */ dEquip_c mEquip;      // the tricked clothes
    }; // size 0x12C

    dAcNpcSpPumpking_c() : mPumpkingTalk(this) {
    }
    virtual ~dAcNpcSpPumpking_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member);
    void changeState(int state);
    virtual int preCreate();
    virtual int create();
    virtual int doDelete();
    virtual int vtB0();
    void initIdle();
    void executeIdle();
    void initLook();
    void stepLookTurn();
    void executeLook();
    void initTalk();
    void stepTalk();
    void stepTalkDisappear();
    void stepTalkLeave();
    void executeTalk();
    void initRemote();
    void executeRemote();
    void initRemoteTalk();
    void executeRemoteTalk();
    virtual BOOL searchWanderPos(mVec3_c *pos);
    bool isHalloween();
    void relocate();

    mAng getTalkAngle() const {
        dActor_c *target = mPumpkingTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    // The net (daub) state.
    struct daub_s {
        u32 mPresent : 1; // at his spot (mIsActive)
        u32 mTalking : 1; // the talk is on (its end starts the disappear
                          // animation)
        u32 mLeaving : 1; // the talk and his exit are on (no appear effect)
        u32 _0 : 29;
        u32 _4;
    }; // size 0x8

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dPumpkingDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep[STATE_COUNT];
    /* 0x1C94 */ talk_c mPumpkingTalk;
    /* 0x1DC0 */ daub_s mDaub;
    /* 0x1DC8 */ int mRelocateTimer; // RELOCATE_FRAMES once his spot was used
    /* 0x1DCC */ int mLeaveTimer;
    /* 0x1DD0 */ u8 mWasTalking;
    /* 0x1DD1 */ u8 mDisappearing; // the disappear animation plays
    /* 0x1DD4 */ int mSpotTimer;   // frames before the next setSpot
}; // size 0x1DD8

#endif
