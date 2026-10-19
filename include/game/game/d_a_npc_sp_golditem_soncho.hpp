#ifndef D_A_NPC_SP_GOLDITEM_SONCHO_HPP
#define D_A_NPC_SP_GOLDITEM_SONCHO_HPP

#include <game/game/d_a_npc_sp.hpp>

// Tortimer with a golden tool (REL d_a_npc_sp_golditem_sonchoNP, profile
// A_NPC_SP_GOLDITEM_SONCHO): when the player has completed the insect catalog
// he calls them over and gives the golden net, otherwise (all fish) the golden
// fishing rod ("npc_SP/NPC_soncho_0"). If the pockets are full he wanders
// around his block until the player talks to him again. Class names from RTTI
// ("dAcNpcSpGoldItemSoncho_c", "dAcNpcSpGoldItemSoncho_c::talk_c"); the rest is
// named from behaviour.
class dAcNpcSpGoldItemSoncho_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_CALL, // call the player over (speak demo)
        STATE_IDLE, // wander
        STATE_LOOK, // turn to the player (demo start)
        STATE_TALK, // talk to the player
        STATE_WALK, // the called player walks up to him
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpGoldItemSoncho_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_soncho_0 codes, named from the English text.
        enum {
            MSG_POCKETS_FULL = 0x33,  // "...your pockets are stuffed..."
            MSG_NET_HERO = 0x34,      // "...caught every single kind of bug"
            MSG_NET_THANKS = 0x35,    // after the net (first in town)
            MSG_NET_NOT_FIRST = 0x36, // "...You're not in trouble!..."
            MSG_NET_THANKS_NOT_FIRST =
                0x37,                      // "...someone...already snagged one"
            MSG_NET_GIVEN = 0x38,          // the net was given before
            MSG_POCKETS_FULL_AGAIN = 0x3E, // "...clear some stuff out..."
            MSG_NET_HERO_AGAIN = 0x3C,     // "Ahem, ahem!..." (ready now)
            MSG_NET_NOT_FIRST_AGAIN = 0x3D,
            MSG_ROD = 0x3F,           // "...it's a golden fishing rod."
            MSG_ROD_THANKS = 0x40,    // after the rod (first in town)
            MSG_ROD_NOT_FIRST = 0x41, // "...you're not the first..."
            MSG_ROD_THANKS_NOT_FIRST = 0x42,
            MSG_ROD_GIVEN = 0x43, // the rod was given before
            MSG_ROD_AGAIN = 0x47, // "Ahem! Oh...Looks like you're ready"
            MSG_ROD_NOT_FIRST_AGAIN = 0x48,
        };
        // l_msgs index: the pockets are full, the net (+ not the first in
        // town), the rod (+ not the first); MSG_AGAIN later in the table for
        // the talks after the call.
        enum {
            GIVE_POCKETS_FULL,
            GIVE_NET,
            GIVE_NET_NOT_FIRST,
            GIVE_ROD,
            GIVE_ROD_NOT_FIRST,
            GIVE_NUM
        };

        talk_c(dAcNpcSpGoldItemSoncho_c *npc);    // 0xA34
        virtual ~talk_c();                        // 0xA94
        virtual void onMessageStart(int arg);     // 0xCB4
        virtual void onMessageEnd(int kind);      // 0xD18
        virtual void onAnswer(int arg);           // 0xEE0
        virtual void init();                      // 0xAEC
        virtual void preExecute();                // 0xF44
        virtual void onTalkEnd();                 // 0xF48
        virtual void getMsgInfo(msgInfo_s *info); // 0xC9C
        virtual void onRequestEnd(int kind);      // 0xF4C
        void start(dActor_c *target);             // 0xB84
        void giveItem();                          // 0xDDC
        void thank();                             // 0xE88

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpGoldItemSoncho_c *mpOwner;
        /* 0x0F0 */ Proc
            mOnMessageStart; // one-shot hooks (cleared after the call)
        /* 0x0FC */ Proc mOnMessageEnd;
        /* 0x108 */ Proc mOnAnswer;
        /* 0x114 */ Proc mOnRequestEnd;
    }; // size 0x120

    dAcNpcSpGoldItemSoncho_c() : mGoldTalk(this) {
    }
    virtual ~dAcNpcSpGoldItemSoncho_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x1CC
    void changeState(int state);                // 0x29C
    virtual int preCreate();                    // 0x2D4
    virtual int create();                       // 0x320
    virtual int doDelete();                     // 0x480
    virtual int vtB0();                         // 0x4AC
    void initCall();                            // 0x4F4
    void executeCall();                         // 0x540
    void initIdle();                            // 0x548
    void executeIdle();                         // 0x554
    virtual BOOL searchWanderPos(mVec3_c *pos); // 0x568: in his block
    virtual BOOL isWanderWalk();                // 0x6A8: only while waiting
    void initLook();                            // 0x704
    void stepLook();                            // 0x794
    void executeLook();                         // 0x7F4
    void initTalk();                            // 0x834
    void stepTalk();                            // 0x8CC
    void executeTalk();                         // 0x934
    void initWalk();                            // 0x974
    void executeWalk();                         // 0x9E0

    mAng getTalkAngle() const {
        dActor_c *target = mGoldTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mGoldTalk;
    /* 0x1DA4 */ u8 mCalling;     // calling the player over (STATE_CALL..WALK)
    /* 0x1DA5 */ u8 mNet;         // the golden net, else the golden rod
    /* 0x1DA6 */ u8 mFirstCall;   // until he starts wandering
    /* 0x1DA7 */ u8 mFirstInTown; // no player of the town has it yet
    /* 0x1DA8 */ int mBlockX;     // the block at create
    /* 0x1DAC */ int mBlockZ;
}; // size 0x1DB0

#endif
