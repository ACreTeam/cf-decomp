#ifndef D_A_NPC_SP_TUNEKICHI_HPP
#define D_A_NPC_SP_TUNEKICHI_HPP

#include <game/game/d_a_npc_sp.hpp>

// Crazy Redd in his tent (REL d_a_npc_sp_tunekichiNP, profile
// A_NPC_SP_TUNEKICHI). He greets the player at the entrance, follows them
// around the tent, pitches the item they point at (dSaveShopGallery_c) and
// sells it, and says goodbye at the locked exit, where he hands out an
// invitation for a referral. Membership costs 3,000 Bells; the member who
// referred a new member gets 1,000 Bells. Class names from RTTI
// ("dAcNpcSpTunekichi_c", "dAcNpcSpTunekichi_c::talk_c"); the rest is named
// from behaviour.
class dAcNpcSpTunekichi_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE,   // wait; follow or turn to the player
        STATE_TURN,   // turn to the player
        STATE_FOLLOW, // walk (or run) after the player
        STATE_LOOK,   // turn to the player (demo start)
        STATE_TALK,   // talk to the player
        STATE_GREET,  // the welcome at the entrance
        STATE_BYE,    // the goodbye at the exit
        STATE_COUNT
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpTunekichi_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // What start() talks about (mKind).
        enum Kind_e {
            KIND_GREET, // the welcome at the entrance
            KIND_BYE,   // the goodbye at the exit
            KIND_TALK,  // the player talks to him
            KIND_ITEM,  // the player points at an item for sale
        };

        // npc_SP/NPC_tunekichi codes, named from the English text.
        enum {
            MSG_ITEM_AGAIN =
                0x01, // "...taken quite a shine to that there <item>"
            MSG_ITEM_PITCH = 0x02, // 0x02..0x07: the item pitches, each once
            MSG_ITEM_PITCH_NUM = 6,
            MSG_PICTURE_PITCH = 0x0A, // 0x0A, 0x0B: the picture pitches
            MSG_PICTURE_PITCH_NUM = 2,
            MSG_BUY_DECLINED = 0x0C, // "What'n the heck?!..."
            MSG_NO_MONEY = 0x0D,     // "...a little short on cash this month!"
            MSG_POCKETS_FULL = 0x0E, // "...your pockets are plumb full up."
            MSG_BOUGHT = 0x0F,       // "...You will NOT be sorry with this one"
            MSG_BOUGHT_2 = 0x10,     // "Many thanks!"
            MSG_BOUGHT_3 = 0x11,     // "...you're a savvy shopper."
            MSG_LOOK_AROUND = 0x12,  // "Please! Take a look around!..."
            MSG_SOLD_OUT = 0x13,     // "...I sold the whole dang shipment!..."
            MSG_ITEM_NOT_MEMBER = 0x14, // "Ho, friend!...taken an interest"
            MSG_ITEM_NOT_MEMBER_AGAIN =
                0x15,           // "...these goods are for members only."
            MSG_WELCOME = 0x1F, // "Well, if it ain't our favorite cousin!..."
            MSG_WELCOME_BACK = 0x20, // "Hey, <name>, you're back!..."
            MSG_REFERRAL_REWARD =
                0x21, // "...you introduced <name> from <town>..."
            MSG_JOIN_OFFER =
                0x22, // "...you happened to know one of our members"
            MSG_JOIN_INVITER = 0x23, // "...after your friend went through..."
            MSG_JOINED = 0x24, // "...You, <name>, are now an official member"
            MSG_MEMBERS_ONLY = 0x25, // "And now you can take full advantage..."
            MSG_BYE = 0x33,          // "Thank you very much!"
            MSG_BYE_NO_PURCHASE = 0x34, // "Thank ya much!"
            MSG_REFERRAL_ASK =
                0x35, // "...You wouldn't happen to know anyone..."
            MSG_REFERRAL_AGAIN = 0x37, // "...How's that referral business..."
            MSG_REFERRAL_MORE = 0x38,  // "...Do you know anyone else..."
            MSG_JOIN_DECIDED = 0x3D,   // "Well? Ya made a decision?..."
            MSG_ALL_SOLD =
                0x3F, // "...Redd's a better salesman than he thought!"
            MSG_LOOK_REST =
                0x40, // "...why not take a peek at the rest of our wares?"
            MSG_BOUGHT_ITEM =
                0x41, // "<item>! Now that is a FANTASTIC choice..."
            MSG_LOOK_TOUCH =
                0x42, // "Cousin! Look around, touch the merchandise..."
            MSG_SUPPLIERS =
                0x43, // "Ol' Redd really has to hand it to his suppliers..."
            MSG_HAGGLE = 0x44,    // "...You wanna haggle prices with ol' Redd?"
            MSG_FORGERIES = 0x45, // "...There's a whole lot of cads..."
        };

        enum { JOIN_PRICE = 3000, REFERRAL_REWARD = 1000 };

        talk_c(dAcNpcSpTunekichi_c *npc);         // 0xDB0
        virtual ~talk_c();                        // 0xE4C
        virtual void onMessageStart(int arg);     // 0x13C0
        virtual void onMessageEnd(int kind);      // 0x1524
        virtual void onAnswer(int arg);           // 0x1780
        virtual void init();                      // 0xEC0
        virtual void preExecute();                // 0x17BC
        virtual void onTalkEnd();                 // 0x17C0
        virtual void getMsgInfo(msgInfo_s *info); // 0x13A8
        virtual void onRequestEnd(int kind);      // 0x1A40
        void start(dActor_c *target);             // 0xF68
        BOOL buy();                               // 0x1830: buy mItem

        // The item pitches already used (cleared when all were used).
        struct pitched_c {

            BOOL isAll() const {
                BOOL all = TRUE;
                for (int i = 0; i < MSG_ITEM_PITCH_NUM; i++) {
                    if (!mUsed[i]) {
                        all = FALSE;
                        break;
                    }
                }
                return all;
            }
            void clear() {
                for (int i = 0; i < MSG_ITEM_PITCH_NUM; i++) {
                    mUsed[i] = 0;
                }
            }

            u8 mUsed[MSG_ITEM_PITCH_NUM];
        };

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpTunekichi_c *mpOwner;
        /* 0x0F0 */ Proc
            mOnMessageEnd; // one-shot hooks (cleared after the call)
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ int mKind;           // Kind_e
        /* 0x10C */ dItem::Item mItem;   // the item pointed at
        /* 0x10E */ dItem::Item mBought; // the last item bought
        /* 0x110 */ int mUnitX;          // the unit of mItem
        /* 0x114 */ int mUnitZ;
        /* 0x118 */ pitched_c mPitched;
        /* 0x11E */ u8 mExiting; // the goodbye requested the scene exit
    }; // size 0x120

    dAcNpcSpTunekichi_c() : mReddTalk(this) {
    }
    virtual ~dAcNpcSpTunekichi_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x204
    void changeState(int state);                // 0x2D8
    virtual int preCreate();                    // 0x310
    virtual int create();                       // 0x35C
    virtual int doDelete();                     // 0x3C4
    virtual int vtB0();                         // 0x3F0
    void initIdle();                            // 0x438
    void executeIdle();                         // 0x44C
    void initTurn();                            // 0x590
    void executeTurn();                         // 0x5EC
    void initFollow();                          // 0x770
    void executeFollow();                       // 0x7F4
    void initLook();                            // 0xA0C
    void stepLook();                            // 0xA70
    void executeLook();                         // 0xAD0
    void initTalk();                            // 0xB10
    void stepTalk();                            // 0xB7C
    void executeTalk();                         // 0xBD4
    void initGreet();                           // 0xC14
    void executeGreet();                        // 0xC18
    void initBye();                             // 0xC64
    void executeBye();                          // 0xC68
    BOOL isAtExit();                            // 0xCB4: on the locked exit
    void setPointedItem(const dItem::Item & item, int unitX, int unitZ);
    BOOL checkPointedItem();                    // 0xCF4: -> mReddTalk.mItem
    f32 getStopDist();                          // 0xD80: stop following
    f32 getFollowDist();                        // 0xD8C: start following
    virtual int getEarType();                   // 0x1AA4

    mAng getTalkAngle() const {
        dActor_c *target = mReddTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mReddTalk;
    /* 0x1DA4 */ u8 mFollowing; // set once he followed the player
}; // size 0x1DA8

#endif
