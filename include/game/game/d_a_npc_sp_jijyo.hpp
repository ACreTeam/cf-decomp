#ifndef D_A_NPC_SP_JIJYO_HPP
#define D_A_NPC_SP_JIJYO_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_private_data.hpp>

// Labelle at the counter of GracieGrace (REL d_a_npc_sp_jijyoNP, profile
// A_NPC_SP_JIJYO): she greets the player, talks about the display (the
// mannequin's outfit) and sells the pointed clothes, accessories, umbrellas,
// furniture, wallpaper and carpets of the shop (by cash or shopping card, at
// the season sale's price); the clothes may be tried on first. During
// Gracie's story (getGracieStep 2, 6 or 9) she has the "npc_SP/NPC_drama3"
// talks. Her messages: "npc_SP/NPC_cotton".
//
// Class names from RTTI ("dAcNpcSpJijyo_c", "dAcNpcSpJijyo_c::talk_c");
// everything else is named from behaviour. The states and the sale follow
// d_a_npc_sp_kinuyo (Mabel, the other clothes shop).
class dAcNpcSpJijyo_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_WAIT,     // stand; turn to / walk over to the player
        STATE_TURN,     // turn to the player
        STATE_APPROACH, // walk over to the player
        STATE_LOOK,     // demo start
        STATE_TALK,     // talk to the player
        STATE_GREET,    // speak: welcome the player who came in
        STATE_SEE_OFF,  // speak: the player is at the (locked) exit
        STATE_NUM
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpJijyo_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: the greetings, the display, the sale (and trying
    // on) of the pointed item.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // The kinds of talk (mKind; onDemoCmd and the states).
        enum Kind_e {
            KIND_GREET,   // the player came in (STATE_GREET)
            KIND_SEE_OFF, // the player is at the exit (STATE_SEE_OFF)
            KIND_TALK,    // the player talks to the npc
            KIND_ITEM,    // the npc speaks about the item the player points at
            KIND_DISPLAY, // ... about the display the player points at
        };

        // npc_SP/NPC_cotton message codes, named from the English text.
        enum {
            MSG_NONE = 0x00,
            MSG_WELCOME = 0x01, // "Welcome to GracieGrace, where we specialize
                                // in wearable art..."
            MSG_SEE_OFF = 0x02, // "Thank you for stopping by."
            MSG_DISPLAY = 0x08, // "That display highlights some of our
                                // premiere coordinates."
            MSG_DISPLAY_TREND = 0x09, // "...the pieces that are currently the
                                      // trend in <town>." (a friend's outfit)
            MSG_EMPTY_0A = 0x0A,      // (empty in the ENG BMG)
            MSG_CLOTH = 0x0B,         // "Oh, yes, that lovely piece is <item>."
                                      // / How much is it? / Can I try it on?
            MSG_CLOTH_LOOK = 0x0C,    // MSG_CLOTH with the fashion theme
            MSG_CAP = 0x0D,           // "Oh, good eye! ... <item>."
            MSG_CAP_LOOK = 0x0E,
            MSG_ACC = 0x0F, // "Oh, yes, you're looking at <item>."
            MSG_ACC_LOOK = 0x10,
            MSG_FTR = 0x11,       // "I see you have your eye on <item>."
            MSG_FTR_BASIC = 0x12, // "...coordinate it with matching carpet and
                                  // wallpaper..."
            MSG_CARPET = 0x13,    // "That <item> is on display in the store
                                  // right now."
            MSG_CARPET_BASIC = 0x14, // "...the same as the one on display..."
            MSG_WALL = 0x15,
            MSG_WALL_BASIC = 0x16, // "...coordinated it with matching furniture
                                   // and carpet..."
            MSG_UMBRELLA = 0x17,   // "...It works in rain or sun..."
            MSG_DECLINED = 0x18,   // "Oh, yes, of course. I completely
                                   // understand."
            MSG_EMPTY_19 = 0x19,   // (empty in the ENG BMG)
            MSG_TRIED_ON_CAP = 0x1D, // "Wow, that looks amazing on you! Who
                                     // needs diamonds..."
            MSG_TRIED_ON_ACC = 0x1E, // "...Accessories are the best way to
                                     // dress up an outfit!"
            MSG_TRIED_ON_CLOTH =
                0x1F, // + the cloth's look (dItem::LOOK_*, getStyle):
                      // "...that outfit makes you look as cute
                      // as a button!" ...
            MSG_TRIED_ON_OTHER = 0x2A, // "So, what do you think? Every piece we
                                       // carry was lovingly sewn..."
            MSG_EMPTY_2B = 0x2B,       // (empty in the ENG BMG)
            MSG_PRICE = 0x2C,          // "Ah yes, that beauty costs <n> Bells."
            MSG_PRICE_SALE = 0x2D,     // "Because it's a sale item, you save
                                       // <n>%..."
            MSG_PRICE_DISCOUNT = 0x2E, // "...The owner has told me to give you
                                       // a special discount of <n>%."
            MSG_NO_MONEY = 0x2F,       // "...you don't have enough money..."
            MSG_POCKETS_FULL = 0x30,   // "...you've got your hands full
                                       // already."
            MSG_THANKS = 0x31,         // "Thank you so much. I hope you enjoy
                                       // your purchase."
            MSG_THANKS_SLIP = 0x32,    // "Y'all come back now, y'hear?..."
            MSG_BOUGHT = 0x33,         // (a tag only: one of the two above)
            MSG_PAY_HOW = 0x35,        // "How would you like to pay, by cash or
                                       // shopping card?"
            MSG_PAY_CARD = 0x36,       // "You don't seem to have enough cash on
                                       // you... by shopping card instead?"
            MSG_SOLD_OUT = 0x3D,       // "...we've sold out of all of our sale
                                       // items." (next season on <month>)
            MSG_SALE_CLEARANCE = 0x3E, // "Right now we're having a clearance
                                       // sale..."
            MSG_SALE_SPRING = 0x3F,    // "We're in the middle of our big
                                       // spring sale now."
            MSG_SALE_SUMMER = 0x40,
            MSG_SALE_FALL = 0x41,
            MSG_SALE_WINTER = 0x42,
            MSG_MENU = 0x43, // (a tag only: one of 0x44..0x46, "Please take
                             // your time looking around." ...)
        };

        // npc_SP/NPC_drama3 message codes (her part of Gracie's story).
        enum {
            DRAMA_MSG_SIGH =
                0x01, // "Sigh.... / What's wrong? / Um..." (step 2)
            DRAMA_MSG_TOLD = 0x03,   // "...I didn't realize that anyone could
                                     // hear me."
            DRAMA_MSG_SIGH_2 = 0x05, // "Sigh.... / You don't look well." (6)
            DRAMA_MSG_TOLD_2 = 0x07, // "...Lately, things just haven't been
                                     // going so well."
            DRAMA_MSG_SIGH_3 = 0x0A, // "Sigh.... / Label? / Service, please!"
            DRAMA_MSG_TOLD_3 = 0x0C, // "...how on earth did you know that
                                     // name?"
        };

        // The answers (dSelect cursor) of the item, price and payment
        // questions.
        enum {
            ANSWER_BUY = 0,     // "How much is it?" / "I'll take it!" / "Cash."
            ANSWER_TRY_ON = 1,  // "Can I try it on?" (three choices)
            ANSWER_CARD = 1,    // "Shopping card." (MSG_PAY_HOW)
            ANSWER_DECLINE = 1, // "Never mind." / "It's not me." (two choices)
            ANSWER_NEVER_MIND = 2, // "Never mind." (three choices)
        };

        // The price cuts of the season sale (dSaveShopGrace_c::getSaleStage)
        // and of Gracie's discount, in percent.
        enum {
            SALE_PERCENT_1 = 20,
            SALE_PERCENT_2 = 30,
            SALE_PERCENT_3 = 50,
            DISCOUNT_PERCENT = 10,
        };

        talk_c(dAcNpcSpJijyo_c *npc);         // 0x11A8
        virtual ~talk_c();                    // 0x1250
        virtual void onMessageStart(int arg); // 0x1750
        virtual void onMessageEnd(int kind);  // 0x1A24
        virtual void onAnswer(int arg);       // 0x1B20
        virtual void init();                  // 0x12C4
        virtual void
        setSpeakerFeel(u32 feel);  // 0x20C4 (+ a dead blr at 0x20E8)
        virtual void preExecute(); // 0x2018
        virtual void onTalkEnd();  // 0x201C
        virtual void getMsgInfo(msgInfo_s *info); // 0x1704
        virtual void onRequestEnd(int kind);      // 0x2220
        void start(dActor_c *target);             // 0x1388

        void onBought();    // 0x20EC
        void askPrice();    // 0x21A0
        void toggleTryOn(); // 0x22C8
        void endTryOn();    // 0x2488
        void tryOnAnswer(); // 0x24E8
        void startTryOn();  // 0x2540
        void finishTryOn(); // 0x2588
        void doNothing2();  // 0x25D8

        void setPointedItem(const dItem::Item &item, int unitX, int unitZ) {
            mItem = item;
            mUnitX = unitX;
            mUnitZ = unitZ;
        }
        // The feel the npc kept for the end of its walk (setSpeakerFeel).
        int takePendingFeel() {
            if (mDelayFeel) {
                int feel = mPendingFeel;
                mDelayFeel = 0;
                mPendingFeel = 0;
                return feel;
            }
            mPendingFeel = 0;
            return 0;
        }

        /* 0x0EA */ u16 mMsgNo; // the message start() picked
        /* 0x0EC */ dAcNpcSpJijyo_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageStart; // one-shot hooks
        /* 0x0FC */ Proc mOnMessageEnd;
        /* 0x108 */ Proc mOnAnswer;
        /* 0x114 */ Proc mOnRequestEnd;
        /* 0x120 */ Proc mNextOnRequestEnd; // mOnRequestEnd after the next one
        /* 0x12C */ dEquip_c mEquipOrig;    // the player's own (try on)
        /* 0x134 */ dEquip_c mEquip;        // what the player wears now
        /* 0x13C */ int mKind;              // Kind_e
        /* 0x140 */ dItem::Item mItem;      // the item talked about
        /* 0x144 */ int mPrice;             // its price (after the sale)
        /* 0x148 */ int mUnitX;             // the pointed item's unit
        /* 0x14C */ int mUnitZ;
        /* 0x150 */ u8 mExitRequested; // the scene change was requested
        /* 0x151 */ u8 mTrying;        // the player tries the item on
        /* 0x152 */ u8 mMiiMask;       // the player wore a Mii mask
        /* 0x154 */ int mBuyCount;     // purchases (up to 2: the news)
        /* 0x158 */ u8 mShopCamera;    // the shop camera was set (initTalk)
        /* 0x159 */ u8 mDelayFeel;     // keep the feel until she arrived
        /* 0x15C */ int mPendingFeel;  // ... that feel
    }; // size 0x160

    dAcNpcSpJijyo_c() : mLabelleTalk(this) {
    }
    virtual ~dAcNpcSpJijyo_c() {
    } // weak
    virtual void onDemoCmd(int cmd, u8 member); // 0x248
    void changeState(int state);                // 0x31C
    virtual int preCreate();                    // 0x354
    virtual int create();                       // 0x3A0
    virtual int doDelete();                     // 0x498
    virtual int vtB0();                         // 0x4C4
    virtual int getEarType();                   // 0x25DC

    // ---- states ----
    void initWait();        // 0x50C
    void executeWait();     // 0x550
    void initTurn();        // 0x6C8
    void executeTurn();     // 0x724
    void initApproach();    // 0x8D0
    void executeApproach(); // 0x970
    void initLook();        // 0xAE8
    void stepLook();        // 0xAF4
    void executeLook();     // 0xB04
    void initTalk();        // 0xB44
    void stepTalk();        // 0xC68
    void executeTalk();     // 0xE54
    void initGreet();       // 0xE94
    void executeGreet();    // 0xE98
    void initSeeOff();      // 0xEE4
    void executeSeeOff();   // 0xEE8

    BOOL isSeeOffTime();     // 0xF34: the player is at the exit
    BOOL isDisplayPointed(); // 0xF74: the player points at the display
    BOOL checkPointedItem(); // 0x10FC: the player points at a shop item
    f32 getStopDist();       // 0x1188: STATE_APPROACH stops within it
    f32 getMoveDist();       // 0x1198: STATE_WAIT walks over beyond it

    mAng getTalkAngle() const {
        dActor_c *target = mLabelleTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return mAng((u16)l_turnSpeed.mAngle);
    }

    /* 0x1C7C */ int mState; // State_e
    /* 0x1C80 */ u8 mStep[STATE_NUM];
    /* 0x1C88 */ talk_c mLabelleTalk;
    /* 0x1DE8 */ u8 mDrama;      // her part of the Gracie story is due
    /* 0x1DE9 */ u8 mApproached; // walking over to the player
}; // size 0x1DEC

#endif
