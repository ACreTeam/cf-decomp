#ifndef D_A_NPC_SP_KUTSUMIGAKI_HPP
#define D_A_NPC_SP_KUTSUMIGAKI_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (m3d,
// fBase_c, dDvd, dActor_c, dDebugPage_c, then the npc classes and
// daRmobj_c::model_c), as in d_a_npc_sp_fortune.
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_a_rmobj.hpp>

// Kicks the shoe-shine skunk in the city (REL d_a_npc_sp_kutsumigakiNP,
// profile A_NPC_SP_KUTSUMIGAKI, messages npc_SP/NPC_shoeshine). For 500 Bells
// he shines the player's shoes to match the outfit: by its colours (the
// shirt, hat, accessory and hair colours) or by its looks (the shirt, hat and
// accessory looks and the hairstyle). After a few shines he offers shoes of
// the other kind (masculine / feminine). The shine is a little show: he gets
// up from his stoop, jumps in front of the player, brushes the shoes with his
// cloth (/Rmobj skk_cloth) and jumps back.
//
// Class names from RTTI ("dAcNpcSpKutsumigaki_c",
// "dAcNpcSpKutsumigaki_c::talk_c",
// "dAcNpcSpKutsumigaki_c::dKutsumigakiDebugPage_c"); everything else is named
// from behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dKutsumigakiDebugPage_c), so dKutsumigakiDebugPage_c is declared
// first.
class dAcNpcSpKutsumigaki_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,  // sit on the stoop
        STATE_TALK,  // talk to the player
        STATE_SHINE, // shine the player's shoes (l_shineSteps)
        STATE_COUNT
    };
    // The steps of STATE_SHINE (l_shineSteps), in table order.
    enum ShineStep_e {
        SHINE_STEP_STAND,  // get up and hop to the player's feet
        SHINE_STEP_WAIT,   // wait a moment
        SHINE_STEP_JUMP,   // jump around the player
        SHINE_STEP_BRUSH,  // brush the shoes
        SHINE_STEP_FINISH, // finish and hop back to the stoop
        SHINE_STEP_COUNT
    };
    // The places he hops to during the shine (setShinePos).
    enum ShinePos_e {
        SHINE_POS_FEET,  // in front of the player's feet
        SHINE_POS_JUMP,  // beside the player (the big jump)
        SHINE_POS_SHOES, // at the shoes
        SHINE_POS_BRUSH, // brushing
        SHINE_POS_BACK   // back
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpKutsumigaki_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dKutsumigakiDebugPage_c : public dDebugPage_c {};

    // The message receptor: the offer, the choice of colour or look and the
    // payment.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_shoeshine (from the ENG text).
        enum {
            MSG_FIRST_OFFER = 0x01,   // "Awright there... Fancy a shoe shine?"
            MSG_DECLINED = 0x02,      // "Awright, suit yourself!"
            MSG_NO_MONEY = 0x03,      // "...you're a few Bells short, eh?"
            MSG_HOW = 0x04,           // "...match 'em by color, or ... style"
            MSG_NEVER_MIND = 0x05,    // "Hey, now? ...You're sure then?"
            MSG_BY_COLOR = 0x06,      // "One set of shoes in your color..."
            MSG_BY_LOOK = 0x07,       // "...the finish to match your style."
            MSG_DONE = 0x08,          // "Annnnnnd here we have it!"
            MSG_PAY = 0x09,           // "That'll be 500 Bells..."
            MSG_DONE_FREE = 0x0A,     // "...this time the shine's on the house"
            MSG_SHINED = 0x0B,        // "Boy, do you ever look sharp..."
            MSG_REGULAR_OFFER = 0x0C, // "Aha, hello there. Here for another"
            MSG_OFFER = 0x0D,         // "Ah, are you looking to have ..."
            MSG_OFFER_RANDOM = 0x0E,  // (jumps to one of 0x0F..0x11)
            MSG_ASK_SWAP = 0x14,      // (jumps to MSG_SWAP_* after a payment)
            MSG_SWAP_TO_FEMALE = 0x15, // "...something cuter for a change?"
            MSG_SWAP_TO_MALE = 0x16,   // "...a little boyish charm..."
            MSG_SWAP_HOW = 0x17,       // "I knew it! ...the color or the style"
            MSG_SWAP_DECLINED = 0x18,  // "Really?! ...I'll give you the usual."
            MSG_ASK_KIND = 0x19,       // "...masculine? Or ... feminine"
            MSG_ASK_KIND_REPEAT = 0x1C, // (jumps to MSG_ASK_KIND)
            MSG_ASK_KIND_2 = 0x1D,      // as MSG_ASK_KIND, the answers swapped
        };
        // The answers of MSG_SWAP_TO_FEMALE / MSG_SWAP_TO_MALE, MSG_ASK_KIND
        // and MSG_ASK_KIND_2.
        enum { ANSWER_SWAP_YES = 0 };
        enum { ANSWER_MASCULINE = 0 };
        enum { ANSWER_2_FEMININE = 0, ANSWER_2_MASCULINE = 1 };
        // After this many shines he asks about the other kind of shoes.
        enum { SHINE_COUNT_MAX = 15 };

        talk_c(dAcNpcSpKutsumigaki_c *npc);       // 0x1F84
        virtual ~talk_c();                        // 0x1FE4
        virtual void init();                      // 0x203C
        virtual void getMsgInfo(msgInfo_s *info); // 0x21B0
        virtual void onMessageStart(int arg);     // 0x21C8
        virtual void onMessageEnd(int kind);      // 0x21CC
        virtual void onAnswer(int arg);           // 0x23C0
        virtual void preExecute();                // 0x2498
        virtual void onTalkEnd();                 // 0x24E0
        virtual void onRequestEnd(int kind);      // 0x24E4

        void start(dActor_c *target); // 0x20E4
        void closeMsg();              // 0x2548 (mOnMessageEnd)
        void onShineRequested();      // 0x25A0 (mOnRequestEnd)
        void startShine();            // 0x25A4
        void execShineWait();         // 0x25CC (mProc)
        void clearProc();             // 0x26B8 (mOnRequestEnd)

        /* 0x0EA */ u16 mMsgNo; // the first message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpKutsumigaki_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ Proc mProc;       // every frame (preExecute)
        /* 0x114 */ u8 mOrgShoeColor; // the player's shoes before the shine
        /* 0x115 */ u8 mShoeColor;    // ... and after it
        /* 0x116 */ u8 mOrgShoeKind;  // dPlayerMgr::ShoeKind_e
        /* 0x117 */ u8 mShoeKind;
    }; // size 0x118

    dAcNpcSpKutsumigaki_c() : mShineTalk(this) {
    }
    virtual ~dAcNpcSpKutsumigaki_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member);    // 0xE04
    virtual bool canInteract(dDemoActor_c *actor); // 0xEC8
    void changeState(int state);                   // 0x109C
    void setShinePos(int pos);                     // 0x10D4
    virtual int preCreate();                       // 0x124C
    virtual int create();                          // 0x12A0
    virtual int doDelete();                        // 0x13F4
    virtual int draw();                            // 0x145C
    virtual int vtB0();                            // 0x14D0
    virtual int getEarType();                      // 0x26D8
    void initIdle();                               // 0x1518
    void executeIdle();                            // 0x15C8
    void initTalk();                               // 0x15CC
    void stepTalk();                               // 0x166C
    void executeTalk();                            // 0x16B8
    void initShine();                              // 0x16F8
    void stepStand();                              // 0x1788
    void stepWait();                               // 0x1910
    void stepJump();                               // 0x19AC
    void stepBrush();                              // 0x1CDC
    void stepFinish();                             // 0x1DA4
    void executeShine();                           // 0x1F44

    mAng getTalkAngle() const {
        dActor_c *target = mShineTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dKutsumigakiDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep;
    /* 0x1C90 */ talk_c mShineTalk;
    /* 0x1DA8 */ int mTimer;        // frames into / left in the shine step
    /* 0x1DAC */ u8 mShined;        // the shine is over (the talk goes on)
    /* 0x1DAD */ u8 mTurnBack;      // turn back to the front (initIdle)
    /* 0x1DB0 */ mVec3_c mShinePos; // where he hops to (setShinePos)
    /* 0x1DBC */ daRmobj_c::model_c mClothMdl; // "skk_cloth"
}; // size 0x1F68

#endif
