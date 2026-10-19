#ifndef D_A_NPC_SP_RORAN_HPP
#define D_A_NPC_SP_RORAN_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_item.hpp>

// Saharah, the carpet / wallpaper recycler (REL d_a_npc_sp_roranNP, profile
// A_NPC_SP_RORAN). On her visiting day she wanders around town and trades the
// player's old flooring (or old wallpaper: dSaveTown_c::mPlayerFlag.mState)
// for new ones from her own collection, a rare one (FROM_ROLAN) for three or
// more ("npc_SP/NPC_rollan"). Residents are sent to ask villagers for the old
// ones (PRIVATE_FLAG1_SAHARAH_ASKED, dSvPlayerFlag). Class names from RTTI
// ("dAcNpcSpRoran_c", "dAcNpcSpRoran_c::talk_c"); the rest is named from
// behaviour.
class dAcNpcSpRoran_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE,        // wander
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpRoran_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The net daub data (getDaubData / setDaubData).
    struct daub_s {
        u32 mVisiting : 1; // dEvent::isVisitorHere(VISITOR_SAHARAH): mIsActive
        u32 _0 : 31;
        u32 _4;
    }; // size 0x8

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_rollan codes, named from the English text. The
        // wallpaper talks are the carpet ones + MSG_WALLPAPER.
        enum {
            MSG_INTRO = 0x01,         // "I came from far away..." (resident)
            MSG_OFFER = 0x02,         // "I want old carpets!..." (Here you go!)
            MSG_INTRO_VISITOR = 0x03, // "...You do not live here? ..."
            MSG_DECLINED = 0x04,      // "Too bad. Yes, very too bad..."
            MSG_NONE = 0x05,          // "...Your recycling disappoints..."
            MSG_NONE_NEW = 0x06,      // ... "Your carpets are still lovely!"
            MSG_TAKE = 0x07,          // "Oh! You bring carpet!..."
            MSG_TAKE_OLD_ONLY = 0x08, // ... "Yes, only old carpets..."
            MSG_GIVE = 0x09,          // "Yes, now I give to you."
            MSG_GIVEN = 0x0A,         // "Here... And still new! You take."
            MSG_GIVEN_RARE = 0x0B,    // "Here... Very exotic..."
            MSG_MORE = 0x0C,          // "More! I will take any old carpets."
            MSG_WALLPAPER = 0x14,     // offset of the wallpaper talks
            MSG_WALLPAPER_OFFER = MSG_WALLPAPER + MSG_OFFER,
            MSG_WALLPAPER_INTRO_VISITOR = MSG_WALLPAPER + MSG_INTRO_VISITOR,
            MSG_WALLPAPER_NONE = MSG_WALLPAPER + MSG_NONE,
            MSG_WALLPAPER_NONE_NEW = MSG_WALLPAPER + MSG_NONE_NEW,
            MSG_WALLPAPER_TAKE = MSG_WALLPAPER + MSG_TAKE,
            MSG_WALLPAPER_TAKE_OLD_ONLY = MSG_WALLPAPER + MSG_TAKE_OLD_ONLY,
            MSG_WALLPAPER_GIVE = MSG_WALLPAPER + MSG_GIVE,
            MSG_WALLPAPER_GIVEN = MSG_WALLPAPER + MSG_GIVEN,
            MSG_WALLPAPER_GIVEN_RARE = MSG_WALLPAPER + MSG_GIVEN_RARE,
        };
        enum { RARE_COUNT = 3 }; // old ones for a rare one

        talk_c(dAcNpcSpRoran_c *npc);             // 0xA00
        virtual ~talk_c();                        // 0xA6C
        virtual void onMessageStart(int arg);     // 0xC60
        virtual void onMessageEnd(int kind);      // 0xCC0
        virtual void onAnswer(int arg);           // 0xFE4
        virtual void init();                      // 0xAC4
        virtual void preExecute();                // 0x115C
        virtual void onTalkEnd();                 // 0x1160
        virtual void getMsgInfo(msgInfo_s *info); // 0xC48
        virtual void onRequestEnd(int kind);      // 0x1164
        void start(dActor_c *target);             // 0xB3C
        void takeOld();                           // 0x11C8: hand them over
        void removeOld();                         // 0x11F4: out of the pockets
        void giveNew();                           // 0x11E4
        void pickUpNew();                         // 0x1288

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpRoran_c *mpOwner;
        /* 0x0F0 */ Proc
            mOnMessageEnd; // one-shot hooks (cleared after the call)
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ dItem::Item mItem; // the old one taken, then the new one
        /* 0x10C */ int mOldNum;       // the old ones in the pockets
    }; // size 0x110

    dAcNpcSpRoran_c() : mRoranTalk(this) {
    }
    virtual ~dAcNpcSpRoran_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x2EC
    void changeState(int state);                // 0x408
    virtual int preCreate();                    // 0x440
    virtual int create();                       // 0x48C
    virtual int doDelete();                     // 0x570
    virtual int vtB0();                         // 0x59C
    void initIdle();                            // 0x6C0
    void executeIdle();                         // 0x6C4
    void initLook();                            // 0x6D8
    void stepLook();                            // 0x73C
    void executeLook();                         // 0x79C
    void initTalk();                            // 0x7DC
    void stepTalk();                            // 0x874
    void executeTalk();                         // 0x8C0
    void initRemote();                          // 0x900
    void executeRemote();                       // 0x908
    void initRemoteTalk();                      // 0x9A8
    void executeRemoteTalk();                   // 0x9AC
    virtual int getEarType();                   // 0x12C0

    mAng getTalkAngle() const {
        dActor_c *target = mRoranTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mRoranTalk;
    /* 0x1D94 */ daub_s mDaub;
}; // size 0x1D9C

#endif
