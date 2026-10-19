#ifndef D_A_NPC_SP_RACCO_HPP
#define D_A_NPC_SP_RACCO_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_effect.hpp>

namespace nw4r {
namespace ef {
class Particle;
}
} // namespace nw4r

// Pascal the sea otter (REL d_a_npc_sp_raccoNP, profile A_NPC_SP_RACCO). He
// sits on one of the "racco" beach units (BG_ATTR_BR_RACCO_*), talks to the
// player, trades a pirate item for a scallop, then dives back into the sea and
// swims away (over the waterfall when his course leads there). His state is
// shared with the other consoles through net record NET_REC_RACCO, and the
// town remembers he came (SAVE_FLAG_RACCO_APPEARED). Class names from RTTI
// ("dAcNpcSpRakosuke_c" and its talk_c); the rest is named from behaviour.
class dAcNpcSpRakosuke_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE,        // sit on the beach
        STATE_LOOK,        // turn to the player (demo start)
        STATE_REMOTE,      // another console owns the npc
        STATE_DIVE,        // dive into the sea
        STATE_SWIM,        // swim away
        STATE_TALK,        // talk to the player
        STATE_REMOTE_TALK, // another console's player talks to him
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpRakosuke_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The net record (NET_REC_RACCO).
    enum Status_e {
        STATUS_BEACH, // on the beach
        STATUS_DIVE,  // diving
        STATUS_SWIM,  // swimming at (mUnitX, mUnitZ)
        STATUS_GONE,  // gone for the day
    };
    struct record_s {
        u16 mUnitX : 7;
        u16 mUnitZ : 7;
        u16 mStatus : 2; // Status_e
    }; // size 0x2

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_rakosuke codes, named from the English text.
        enum {
            MSG_START = 0x01,       // the dispatcher
            MSG_SHARE = 0x05,       // "......" -> a random saying
            MSG_RIGHT_ON = 0x07,    // "...Right on."
            MSG_BLEW_MIND = 0x08,   // "...Dude, you just blew my mind."
            MSG_SCALLOP_ASK = 0x09, // "Hey, hook an otter up with a scallop"
            MSG_GIVE = 0x0A,        // "Look, maaan. This here? I'm givin' it"
            MSG_SAYING = 0x0B,      // 0x0B..0xFF: his sayings
            MSG_SAYING_NUM = 0xF5,
        };

        talk_c(dAcNpcSpRakosuke_c *npc);          // 0x1D3C
        virtual ~talk_c();                        // 0x1D9C
        virtual void onMessageStart(int arg);     // 0x1EC4
        virtual void onMessageEnd(int kind);      // 0x1F08
        virtual void onAnswer(int arg);           // 0x20D0
        virtual void init();                      // 0x1DF4
        virtual void preExecute();                // 0x215C
        virtual void onTalkEnd();                 // 0x2160
        virtual void getMsgInfo(msgInfo_s *info); // 0x1EAC
        virtual void onRequestEnd(int kind);      // 0x2164
        void start(dActor_c *target);             // 0x1E5C
        void requestScallop();                    // 0x21C8: take the scallop
        void requestGive();                       // 0x221C: MSG_GIVE next
        void giveTreasure();                      // 0x2228: a random item
        void afterGive();                         // 0x22B8

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpRakosuke_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ int mScallopSlot; // the pocket of the scallop
    }; // size 0x10C

    dAcNpcSpRakosuke_c() : mRaccoTalk(this) {
    }
    virtual ~dAcNpcSpRakosuke_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member);    // 0x388
    virtual bool canInteract(dDemoActor_c *actor); // 0x4A8
    void changeState(int state);                   // 0x524
    void sendRecord(BOOL gone);                    // 0x55C
    void recvRecord();                             // 0x620
    void breakSnowmen();                           // 0x660
    void addWallPush(mVec3_c *dir);                // 0x670
    void calcDiveAngle();                          // 0x808
    void fadeShadow(BOOL in);                      // 0x970
    BOOL isNearFall();                             // 0xA58
    virtual int preCreate();                       // 0xB04
    virtual int create();                          // 0xB50
    virtual int doDelete();                        // 0xDAC
    virtual int vtB0();                            // 0xE6C
    void initIdle();                               // 0xEE0
    void executeIdle();                            // 0xF24
    void initLook();                               // 0xF3C
    void stepLook();                               // 0xFAC
    void executeLook();                            // 0x100C
    void initRemote();                             // 0x1070
    void executeRemote();                          // 0x1080
    void initRemoteTalk();                         // 0x1130
    void executeRemoteTalk();                      // 0x1140
    void initDive();                               // 0x1158
    void stepDiveTurn();                           // 0x123C
    void stepDive();                               // 0x1364
    void executeDive();                            // 0x1560
    void initSwim();                               // 0x1600
    void stepSwim();                               // 0x1710
    void stepWaterfall();                          // 0x19A4
    void executeSwim();                            // 0x1B48
    void initTalk();                               // 0x1C2C
    void stepTalk();                               // 0x1CA4
    void executeTalk();                            // 0x1CFC
    virtual int getEarType();                      // 0x22BC

    mAng getTalkAngle() const {
        dActor_c *target = mRaccoTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mRaccoTalk;
    /* 0x1D90 */ mAng mDiveAngle; // toward the sea (calcDiveAngle)
    /* 0x1D94 */ mVec3_c mVelocity;
    /* 0x1DA0 */ record_s mRecord;
    /* 0x1DA2 */ u8 mTalked; // dive after the talk: end the talk demo
    /* 0x1DA4 */ dBGCF::acch_c mSwimAcch;
    /* 0x1DEC */ dLevelEffect_c mShadow; // "af_npc_rakosuke_moya"
    /* 0x1E80 */ u8 mShadowAlpha;
}; // size 0x1E84

#endif
