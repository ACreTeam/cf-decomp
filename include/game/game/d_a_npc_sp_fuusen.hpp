#ifndef D_A_NPC_SP_FUUSEN_HPP
#define D_A_NPC_SP_FUUSEN_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_balloon_string.hpp>
#include <game/game/d_item.hpp>
#include <game/mLib/m_3d.hpp>

// Phineas, the prize man (REL d_a_npc_sp_fuusenNP, profile A_NPC_SP_FUUSEN):
// on his visiting days he stands holding a balloon (the color of the day's
// dOrganBalloon_c) and gives the player one prize a day: a balloon, a bunny
// balloon, a pinwheel or a bubble wand ("npc_SP/NPC_balloon"). Class names
// from RTTI ("dAcNpcSpFuusen_c", "dAcNpcSpFuusen_c::talk_c"); the rest is
// named from behaviour.
class dAcNpcSpFuusen_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE, // wait
        STATE_LOOK, // turn to the player (demo start)
        STATE_TALK, // talk to the player
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // The balloons he holds (the code handles up to 3, one per hand node).
    enum { BALLOON_NUM = 1 };

    typedef void (dAcNpcSpFuusen_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_balloon codes, named from the English text.
        enum {
            MSG_HELLO =
                0x01,       // "...What can I do for you..." (Gimme something!)
            MSG_WHO = 0x02, // "Who am I? ...I'm Phineas, the prize man!"
            MSG_NEVER_MIND = 0x03,   // "Nothing to say?..."
            MSG_GREEDY = 0x04,       // "You want another one?..."
            MSG_POCKETS_FULL = 0x05, // "...your pockets are so stuffed..."
            MSG_CHOOSE = 0x06,       // "...Just tell me which prize you want"
            MSG_BALLOON = 0x07,      // "A basic balloon?..."
            MSG_BALLOON_GIVEN = 0x08,
            MSG_BUNNY_BALLOON = 0x09, // "...looking for a bunny balloon?..."
            MSG_BUNNY_BALLOON_GIVEN = 0x0A,
            MSG_PINWHEEL = 0x0B, // "Oh, a pinwheel, huh?..."
            MSG_PINWHEEL_GIVEN = 0x0C,
            MSG_BUBBLE_WAND = 0x0D, // "Ah, a bubble wand..."
            MSG_BUBBLE_WAND_GIVEN = 0x0E,
            MSG_NO_PRIZE = 0x0F, // "Really? You don't want a prize?..."
        };
        enum {
            PRIZE_COLOR_NUM = 8
        }; // the colors of the balloons and pinwheels

        talk_c(dAcNpcSpFuusen_c *npc);            // 0xB64
        virtual ~talk_c();                        // 0xBD0
        virtual void onMessageStart(int arg);     // 0xD34
        virtual void onMessageEnd(int kind);      // 0xD98
        virtual void onAnswer(int arg);           // 0xEE0
        virtual void init();                      // 0xC28
        virtual void preExecute();                // 0xFB8
        virtual void onTalkEnd();                 // 0xFBC
        virtual void getMsgInfo(msgInfo_s *info); // 0xD1C
        virtual void onRequestEnd(int kind);      // 0xFC0
        void start(dActor_c *target);             // 0xCC0
        void givePrize();                         // 0x1024

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpFuusen_c *mpOwner;
        /* 0x0F0 */ Proc
            mOnMessageStart; // one-shot hooks (cleared after the call)
        /* 0x0FC */ Proc mOnMessageEnd;
        /* 0x108 */ Proc mOnAnswer;
        /* 0x114 */ Proc mOnRequestEnd;
        /* 0x120 */ dItem::Item mPrize; // the prize chosen
    }; // size 0x124

    dAcNpcSpFuusen_c(); // 0x8C
    virtual ~dAcNpcSpFuusen_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x274
    void changeState(int state);                // 0x31C
    virtual int preCreate();                    // 0x354
    virtual int create();                       // 0x3A0
    virtual int doDelete();                     // 0x5D4
    virtual int draw();                         // 0x698
    virtual int vtB0();                         // 0x720
    void initIdle();                            // 0x950
    void executeIdle();                         // 0x964
    void initLook();                            // 0x968
    void stepLook();                            // 0x9CC
    void executeLook();                         // 0xA2C
    void initTalk();                            // 0xA6C
    void stepTalk();                            // 0xAD8
    void executeTalk();                         // 0xB24

    mAng getTalkAngle() const {
        dActor_c *target = mPrizeTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mPrizeTalk;
    /* 0x1DA8 */ EGG::FrmHeap *mpHeap; // the balloon's heap
    /* 0x1DAC */ dBalloonString_c *mpString;
    /* 0x1DB0 */ mAllocator_c mAllocator[BALLOON_NUM];
    /* 0x1DCC */ m3d::mdl_c mBalloonMdl[BALLOON_NUM];
    /* 0x1E0C */ m3d::anmMatClr_c
        mBalloonClr[BALLOON_NUM]; // the color of the day
    /* 0x1E38 */ dItem::resLoader_c mBalloonRes[BALLOON_NUM];
    /* 0x1EA8 */ int mDrawCount; // the balloon is drawn from the 2nd frame
}; // size 0x1EAC

#endif
