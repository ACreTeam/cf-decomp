#ifndef D_A_NPC_SP_OP_TANUKICHI_HPP
#define D_A_NPC_SP_OP_TANUKICHI_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_state.hpp>

// Tom Nook in the game opening (REL d_a_npc_sp_op_tanukichiNP, profile
// A_NPC_SP_OP_TANUKICHI). He waits for the new player in front of each free
// house of the house tour, calls the player over (the player walks up to him),
// talks about the house and sells it. Once the player owns a house he wanders
// around the town.
//
// Class names from RTTI ("dAcNpcSpOpTanukichi_c" and its talk_c); mode names
// from the original CP932 mode labels; everything else is named from
// behaviour and the npc_SP/NPC_tanukichi_0 messages.

// Wandering tuning once the player owns a house (dAcNpcSp_c wander overrides).
#define OP_TANUKICHI_WANDER_WAIT_BASE 120  // frames between walks: at least ...
#define OP_TANUKICHI_WANDER_WAIT_RAND 180  // ... plus up to this many
#define OP_TANUKICHI_WANDER_MAX_DIST 64.0f // two units

class dAcNpcSpOpTanukichi_c : public dAcNpcSp_c,
                              public dState::mode_c<dAcNpcSpOpTanukichi_c> {
  public:
    // dAcNpc_c::action_c request priorities.
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef dState::base_c<dAcNpcSpOpTanukichi_c>::Method Method;

    // The modes (the table in create), in table order.
    enum {
        MODE_FORCED_TALK,         // "強制会話": call the player over (first)
        MODE_DEMO_WAIT,           // "デモ待機"
        MODE_PLAYER_MOVE_REQUEST, // "プレイヤー移動要求": walk the player up
        MODE_PLAYER_MOVE_DONE,    // "プレイヤー移動完了": wait for the player
        MODE_TALK,                // "会話"
        MODE_WANDER_WALK,         // "うろうろ歩き" (the player owns a house)
        MODE_WAIT,                // "待機"
        MODE_HIDDEN,              // "非表示" (out of every player's view)
    };

    // The message receptor (RTTI "dAcNpcSpOpTanukichi_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c, public dMsgRcptExp_c<talk_c> {
      public:
        // npc_SP/NPC_tanukichi_0 message codes, named from the English text.
        enum {
            MSG_GREETING = 0x01,    // "Ah, hello! You must be ..."
            MSG_LOOK_AROUND = 0x03, // "Oh, gracious, really? ... Take a look at
                                    // the other houses"
            MSG_HOUSE_BOUGHT = 0x08,   // "Well then, that decides it!"
            MSG_FIRST_HOUSE = 0x10,    // "I bet that was the first time you've
                                       // seen a house like that"
            MSG_READY_TO_BUY = 0x11,   // "So, you really like it, yes?"
            MSG_TAKE_YOUR_TIME = 0x12, // "it really is a gigantic purchase"
            MSG_OTHER_PROPERTIES = 0x13,  // "Even if this house doesn't work
                                          // for you"
            MSG_NONE = 0x14,              // (empty: the player has a house)
            MSG_HOW_WAS_THE_HOUSE = 0x15, // "So, how was the house, hm?"
            MSG_SOMEONES_HOUSE = 0x16,    // "This is ...'s house. I handled
                                          // the sale."
            MSG_SOMEONES_HOUSE_2 = 0x17,  // "This is ...'s house. I can't sell
                                          // you this place"
        };
        // Custom tags (onCustomTag).
        enum { CUSTOM_PLAY_BGM }; // StgField_c::play1C

        talk_c(dAcNpcSpOpTanukichi_c *npc);       // 0x1240
        virtual ~talk_c();                        // 0x12A4
        virtual void onMessageStart(int arg);     // 0x14A0
        virtual void onMessageEnd(int kind);      // 0x161C
        virtual void onAnswer(int answer);        // 0x16F4
        virtual void init();                      // 0x12FC
        virtual void onCustomTag(int no);         // 0x1718
        virtual void preExecute();                // 0x16F8
        virtual void onTalkEnd();                 // 0x16FC
        virtual void getMsgInfo(msgInfo_s *info); // 0x1488
        void start(dActor_c *target);             // 0x1330

        /* 0x0EC dMsgRcptExp_c<talk_c> */
        /* 0x128 */ u16 mMsgCode; // the message start() picked
        /* 0x12C */ dAcNpcSpOpTanukichi_c *mpOwner;
        /* 0x130 */ u8 mTalked;           // the first talk (start) is done
        /* 0x131 */ u8 mDayChangeBlocked; // until the talk ends
    }; // size 0x134

    dAcNpcSpOpTanukichi_c() : mOpTalk(this) {
    }
    virtual ~dAcNpcSpOpTanukichi_c() {
    } // weak
    virtual int create();                       // 0x280
    virtual int preCreate();                    // 0x1D4
    virtual int doDelete();                     // 0x340
    virtual void onDemoCmd(int cmd, u8 member); // 0x3C0
    virtual int getEarType();                   // 0x1734: by the store's stage
    virtual int vtB0();                         // 0x36C
    virtual int getWanderWaitBase() {
        return OP_TANUKICHI_WANDER_WAIT_BASE;
    } // weak
    virtual int getWanderWaitRand() {
        return OP_TANUKICHI_WANDER_WAIT_RAND;
    } // weak
    virtual f32 getWanderMaxDist() {
        return OP_TANUKICHI_WANDER_MAX_DIST;
    } // weak

    // ---- modes ----
    void initWait();                 // 0xA50
    void executeWait();              // 0xA64
    void initDemoWait();             // 0xA68
    void executeDemoWait();          // 0xA7C
    void initPlayerMoveRequest();    // 0xA80
    void executePlayerMoveRequest(); // 0xA84
    void initPlayerMoveDone();       // 0xC2C
    void executePlayerMoveDone();    // 0xC30
    void initTalk();                 // 0xDA4
    void executeTalk();              // 0xE44
    void initForcedTalk();           // 0xE8C
    void executeForcedTalk();        // 0xECC
    void initWanderWalk();           // 0xED4
    void executeWanderWalk();        // 0xF28
    void initHidden();               // 0x10B8
    void executeHidden();            // 0x10CC

    // initTalk: the angle to the talk partner and the turn speed.
    mAng getTalkAngle() const {
        dActor_c *target = mOpTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C dState::mode_c<dAcNpcSpOpTanukichi_c> */
    /* 0x1CA0 */ talk_c mOpTalk;
    /* 0x1DD4 */ u8 mOpeningBgm; // the forced talk started the opening bgm
}; // size 0x1DD8

#endif
