#ifndef D_A_NPC_SP_FORTUNE_HPP
#define D_A_NPC_SP_FORTUNE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (m3d,
// fBase_c, dDvd, dActor_c, dDebugPage_c, then the npc classes and
// daRmobj_c::model_c).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/cLib/c_math.hpp>

// Katrina the fortune teller in the city (REL d_a_npc_sp_fortuneNP, profile
// A_NPC_SP_FORTUNE). For 100 Bells (once a week) she reads the player's
// fortune from the stars (dPrivateData_c::mFortune: love, friendship, money,
// items or bad luck); once a month she reveals the town's charm (the jinx,
// dSvActivity_c::mJinx: its outcome and condition). Each reading ends with
// her "Keeee heh ha mo-ata" trance: a bucket, a can or a washbasin
// (/Rmobj obj_fortune_*) falls on the player's head and knocks them out.
//
// Class names from RTTI ("dAcNpcSpFortune_c", "dAcNpcSpFortune_c::talk_c",
// "dAcNpcSpFortune_c::dFortuneDebugPage_c"); everything else is named from
// behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dFortuneDebugPage_c), so dFortuneDebugPage_c is declared first.
class dAcNpcSpFortune_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE, // stand still
        STATE_LOOK, // turn to the player (demo start)
        STATE_TALK, // talk to the player
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // What falls on the player's head (mDropKind; 0: nothing).
    enum Drop_e { DROP_NONE, DROP_BUCKET, DROP_CAN, DROP_TARAI, DROP_NUM };

    typedef void (dAcNpcSpFortune_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dFortuneDebugPage_c : public dDebugPage_c {};

    // The message receptor: the greetings, the fortune and the town's charm.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_hakkemie (from the ENG text).
        enum {
            MSG_FIRST_MEETING = 0x01, // "Keeee ha mo-ata..." (then 0x02)
            MSG_INTRO = 0x02, // "Welcome, lost one of the <sign> sign..."
            MSG_WELCOME_BACK = 0x03,   // "...Welcome back, lost soul..."
            MSG_HELLO = 0x04,          // "...What is it that you wish?"
            MSG_NO_MONEY = 0x06,       // "...Your pockets hold less than 100"
            MSG_TOO_SOON = 0x07,       // "...a short time since your last"
            MSG_AFTER_FORTUNE = 0x08,  // "It may be better not to think..."
            MSG_AFTER_CHARM = 0x09,    // "May happiness bloom for you..."
            MSG_FORTUNE_TRANCE = 0x0A, // "...your star matrix into view"
            MSG_FORTUNE_SEEING = 0x0B, // "...Aha, yes... I am starting to see"
            MSG_FORTUNE_BIRTH_SIGN = 0x0C, // "...<sign> is nearing the sun."
            MSG_CHARM_TRANCE = 0x0D, // "...how to bring more happiness..."
            MSG_CHARM_SEEING = 0x0E, // "...Ahem. I see it..."
            MSG_COME_CLOSER = 0x10,  // "Come a bit closer..."
            MSG_CHARM_TOLD = 0x11,   // "...the stars can give you no further"
            // The readings: five messages each (setFortuneMsg).
            MSG_FORTUNE_NONE = 0x15,
            MSG_FORTUNE_BAD = 0x1F,
            MSG_FORTUNE_LOVE = 0x29,
            MSG_FORTUNE_FRIENDSHIP = 0x33,
            MSG_FORTUNE_MONEY = 0x3D,
            MSG_FORTUNE_ITEM = 0x47,
            MSG_FORTUNE_PAY = 0x57, // "...Now I will require 100 Bells."
            MSG_FORTUNE_BYE = 0x58, // "May happiness find you and yours..."
            // "I have revealed for you a hidden charm...": + dJinxOutcome_e.
            MSG_CHARM_OUTCOME = 0x65,
            // "Yesss... You must ...": + dJinxCondition_e.
            MSG_CHARM_CONDITION = 0x79,
            MSG_CHARM_NOT_YOU = 0x8D, // "This is <town>'s charm..."
            MSG_CHARM_PAY = 0x8E,     // "...Now I will require 100 Bells!"
            MSG_CHARM_BYE = 0x8F,     // "May happiness find you..."
            MSG_CHARM_FREE = 0x91,    // "...And that is all."
        };
        // The answers of MSG_INTRO / MSG_WELCOME_BACK / MSG_HELLO.
        enum { ANSWER_FORTUNE, ANSWER_CHARM, ANSWER_NOTHING };
        // The BGM is muted by the message tag (onCustomTag) of the trance.
        enum { TAG_TRANCE = 0 };

        talk_c(dAcNpcSpFortune_c *npc);           // 0xDD0
        virtual ~talk_c();                        // 0xE44
        virtual void init();                      // 0xE9C
        virtual void getMsgInfo(msgInfo_s *info); // 0x103C
        virtual void onMessageStart(int arg);     // 0x1054
        virtual void onMessageEnd(int kind);      // 0x13F0
        virtual void onAnswer(int arg);           // 0x19FC
        virtual void preExecute();                // 0x1B74
        virtual void onTalkEnd();                 // 0x1BE0
        virtual void onCustomTag(int no);         // 0x1C28
        virtual void onRequestEnd(int kind);      // 0x1C80

        void start(dActor_c *target); // 0xF28

        /* 0x0EA */ u16 mMsgCode; // the first message (getMsgInfo)
        /* 0x0EC */ u16 mNextMsg; // the message after the trance (preExecute)
        /* 0x0F0 */ dAcNpcSpFortune_c *mpOwner;
        /* 0x0F4 */ Proc mOnMessageEnd; // one-shot hooks (never set)
        /* 0x100 */ Proc mOnAnswer;
        /* 0x10C */ Proc mOnRequestEnd;
        /* 0x118 */ u8 mGreeted;   // talked to her once (onTalkEnd)
        /* 0x119 */ u8 mPaid;      // the fortune was paid for
        /* 0x11A */ u8 mCharmPaid; // the charm was paid for
        /* 0x11B */ u8 mFaint;     // the next message knocks the player out
        /* 0x11C */ u8 mCallOver;  // she called the player closer
        /* 0x11D */ u8 mBgmMuted;  // the trance muted the BGM
    }; // size 0x120

    dAcNpcSpFortune_c() : mFortuneTalk(this) {
    }
    virtual ~dAcNpcSpFortune_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x3DC
    void changeState(int state);                // 0x4A8
    virtual int preCreate();                    // 0x4E0
    virtual int create();                       // 0x52C
    virtual int doDelete();                     // 0x628
    virtual int vtB0();                         // 0x6C8
    virtual int draw();                         // 0x970
    virtual int getEarType();                   // 0x1CE4
    void initIdle();                            // 0x9F8
    void executeIdle();                         // 0xA0C
    void initLook();                            // 0xA10
    void stepLook();                            // 0xA74
    void executeLook();                         // 0xAD4
    void initTalk();                            // 0xB14
    void stepTalk();                            // 0xB80
    void stepTalkEnd();                         // 0xBF0
    void executeTalk();                         // 0xC40
    void startDrop(int kind);                   // 0xC80
    BOOL isDropDone();                          // 0xDBC
    void startRandomDrop() {
        startDrop(cM::rndInt(DROP_NUM - 1) + 1);
    }

    mAng getTalkAngle() const {
        dActor_c *target = mFortuneTalk.mpActActor;
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
    /* 0x1C84 */ dFortuneDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 _1C8C; // only cleared (create)
    /* 0x1C8D */ u8 mLookStep;
    /* 0x1C8E */ u8 mTalkStep;
    /* 0x1C90 */ talk_c mFortuneTalk;
    /* 0x1DB0 */ daRmobj_c::model_c mBucketMdl; // "obj_fortune_bucket"
    /* 0x1F5C */ daRmobj_c::model_c mCanMdl;    // "obj_fortune_ittocan"
    /* 0x2108 */ daRmobj_c::model_c mTaraiMdl;  // "obj_fortune_tarai"
    /* 0x22B4 */ int mDropKind;                 // Drop_e
    /* 0x22B8 */ int mDropTimer;                // frames until it falls
    /* 0x22BC */ int mFallSndTimer;             // frames until SE_FallObj
    /* 0x22C0 */ int mHitTimer;        // frames until it hits the player
    /* 0x22C4 */ int mRollTimer;       // frames until it rolls away
    /* 0x22C8 */ int mBusyTimer;       // the talk waits for it (isDropDone)
    /* 0x22CC */ dSndObjSimple_c mSnd; // the falling object's sounds
}; // size 0x22FC

#endif
