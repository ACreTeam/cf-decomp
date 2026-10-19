#ifndef D_A_NPC_SP_KABURIBA_HPP
#define D_A_NPC_SP_KABURIBA_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_item.hpp>

// Joan, the turnip seller (REL d_a_npc_sp_kaburibaNP, profile
// A_NPC_SP_KABURIBA, messages "npc_SP/NPC_kaburiba"). On Sunday mornings
// (dEvent::isVisitorHere(VISITOR_JOAN)) she wanders around town, explains
// turnips, sells white turnips in bunches of ten at the stalk market's price
// (dSaveStalkMarket_c::mBuyPrice) and one red turnip seed a week to
// residents. Class names from RTTI ("dAcNpcSpKaburiba_c",
// "dAcNpcSpKaburiba_c::talk_c"); the rest is named from behaviour.
class dAcNpcSpKaburiba_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE,        // wander
        STATE_LOOK,        // turn to the player (demo start), keeps wandering
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpKaburiba_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The net daub data (getDaubData / setDaubData).
    struct daub_s {
        u32 mVisiting : 1; // dEvent::isVisitorHere(VISITOR_JOAN): mIsActive
        u32 _0 : 31;
        u32 _4;
    }; // size 0x8

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_kaburiba codes, named from the English text.
        enum {
            MSG_GREETING = 0x01,        // "Hooo, hello there!" (first talk)
            MSG_INTRO = 0x02,           // "I've been selling turnips here..."
            MSG_OFFER_LESSON = 0x03,    // "...a quick lesson in turnips..."
            MSG_LESSON_TOPIC = 0x04,    // "What do you want to hear about?"
            MSG_LESSON_DECLINED = 0x05, // "Well, that's fine, of course."
            MSG_WELCOME_BACK = 0x06, // "Well now, my little customer is back!"
            MSG_MENU = 0x07,         // "...what tickles your fancy today?"
            MSG_NEITHER = 0x08,      // "Oh my, isn't that just too bad..."
            MSG_RED_LESSON = 0x10, // "You need to plant red turnips yourself."
            MSG_RED_LESSON_END = 0x11, // "So, that's about all ... red turnips"
            MSG_LESSON_AGAIN = 0x12, // "...having to explain something over..."
            MSG_WHITE_PRICE = 0x13, // "So today, the asking price is <n> Bells"
            MSG_DECLINED = 0x14,    // "Well, isn't that too bad for ol' Joan"
            MSG_HOW_MANY = 0x15,    // "How many do you want? ...bunches of ten"
            MSG_CONFIRM = 0x16,     // "So, you want <n>? ...comes to <n> Bells"
            MSG_TOO_MANY = 0x17,    // "...you can't hold that many turnips!"
            MSG_THANKS = 0x18,      // "Hyeh heh heh... Come again, kiddo!"
            MSG_RED_MENU = 0x1A,    // "Red turnips, huh? What about 'em?"
            MSG_RED_SOLD_OUT = 0x1B,   // "...all sold out of red turnips today"
            MSG_RED_SEED_OFFER = 0x1C, // "...one seed today, ... 1,000 Bells"
            MSG_RED_SEED_SOLD = 0x1D,  // "...stay on top of watering it"
            MSG_NO_MONEY = 0x1E,       // "...You don't have enough Bells!"
            MSG_NO_ROOM = 0x1F, // "...You don't have enough room for this."
            MSG_WHITE_LESSON =
                0x20, // "These white turnips are mostly called..."
            MSG_WHITE_PRICE_AGAIN =
                0x22, // "Today, white turnips are at <n> Bells"
            MSG_RED_VISITOR =
                0x23, // "...aren't you just visiting from another town?"
        };
        // The answers (MSG_MENU, MSG_LESSON_TOPIC; MSG_WHITE_PRICE,
        // MSG_RED_MENU; MSG_CONFIRM, MSG_TOO_MANY, MSG_RED_SEED_OFFER).
        enum { ANSWER_WHITE, ANSWER_RED, ANSWER_NEITHER };
        enum { ANSWER_BUY, ANSWER_EXPLAIN, ANSWER_NEVER_MIND };
        enum { ANSWER_YES, ANSWER_NO };

        enum {
            TURNIP_BUNCH =
                10, // turnips per bunch (the number menu counts bunches)
            TURNIP_POCKET = 100,   // turnips per pocket (ITEM_IDX_100_TURNIPS)
            RED_SEED_PRICE = 1000, // Bells
        };

        talk_c(dAcNpcSpKaburiba_c *npc);          // 0x840
        virtual ~talk_c();                        // 0x8BC
        virtual void onMessageStart(int arg);     // 0xA28
        virtual void onMessageEnd(int kind);      // 0xAF0
        virtual void onAnswer(int arg);           // 0xC64
        virtual void init();                      // 0x914
        virtual void preExecute();                // 0x1110
        virtual void onTalkEnd();                 // 0x1114
        virtual void getMsgInfo(msgInfo_s *info); // 0xA10
        virtual void onRequestEnd(int kind);      // 0x1154
        void start(dActor_c *target);             // 0x998
        void giveItem();                          // 0x11B8
        void onItemGiven();                       // 0x11C8 (empty)
        void openNumberMenu();                    // 0x11CC: how many bunches
        void onNumberEntered();                   // 0x11D4

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpKaburiba_c *mpOwner;
        /* 0x0F0 */ Proc
            mOnMessageEnd; // one-shot hooks (cleared after the call)
        /* 0x0FC */ Proc mOnAnswer;
        /* 0x108 */ Proc mOnRequestEnd;
        /* 0x114 */ dItem::Item mItem; // the item handed over
        /* 0x118 */ int mNextMsg;      // after MSG_RED_LESSON_END
        /* 0x11C */ u8 mSeedBought; // a red turnip seed was bought in this talk
    }; // size 0x120

    dAcNpcSpKaburiba_c() : mKaburibaTalk(this) {
    }
    virtual ~dAcNpcSpKaburiba_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x1CC
    void changeState(int state);                // 0x2E8
    virtual int preCreate();                    // 0x320
    virtual int create();                       // 0x36C
    virtual int doDelete();                     // 0x450
    virtual int vtB0();                         // 0x47C
    void initIdle();                            // 0x5A0
    void executeIdle();                         // 0x5A4
    void initLook();                            // 0x5B8
    void initTalk();                            // 0x61C
    void stepTalk();                            // 0x6B4
    void executeTalk();                         // 0x700
    void initRemote();                          // 0x740
    void executeRemote();                       // 0x748
    void initRemoteTalk();                      // 0x7E8
    void executeRemoteTalk();                   // 0x7EC
    virtual int getEarType();                   // 0x1228

    mAng getTalkAngle() const {
        dActor_c *target = mKaburibaTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mKaburibaTalk;
    /* 0x1DA4 */ daub_s mDaub;
}; // size 0x1DAC

#endif
