#ifndef D_A_NPC_SP_TANUKICHI_HPP
#define D_A_NPC_SP_TANUKICHI_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/sLib/s_lib.hpp>

// Tom Nook and the twins Timmy and Tommy in Nook's store (REL
// d_a_npc_sp_tanukichiNP, profiles NPC_SP_TANUKICHI / NPC_SP_MAMEKICHI /
// NPC_SP_TUBUKICHI). They greet the player who comes in, walk over to the
// player in the store, see the player off at the door, buy and sell, take
// catalog orders, sell house upgrades / roof paint and, for a new player, run
// the part-time job (PRIVATE_FLAG0_PART_TIME_JOB).
//
// Class names from RTTI ("dAcNpcSpTanukiBase_c", its talk_c,
// "dAcNpcSpTanukichi_c", "dAcNpcSpTubuMame_c", "dAcNpcSpMamekichi_c",
// "dAcNpcSpTubukichi_c"); everything else is named from behaviour and the
// npc_SP/NPC_tanukichi (store), NPC_tanukichi_1 (part-time job), NPC_twins and
// NPC_drama6 messages.
//
// .data holds the vtables in reverse class-completion order: Tubukichi,
// Mamekichi, TubuMame, Tanukichi, TanukiBase, TanukiBase::talk_c.
class dAcNpcSpTanukiBase_c : public dAcNpcSp_c {
  public:
    // dAcNpc_c::action_c request priorities.
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    // The states (l_states order).
    enum State_e {
        STATE_WAIT,        // stand; turn to / walk over to the player
        STATE_TURN,        // turn to the player
        STATE_APPROACH,    // walk / run over to the player
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_GREET,       // speak: welcome the player who came in
        STATE_SEE_OFF,     // speak: the player is at the (locked) exit
        STATE_CLOSING,     // speak: past closing time
        STATE_MOVE_PLAYER, // walk the player back from the stairs
        STATE_WATCH,       // the other twin talks: face the player
        STATE_MULTI_WAIT,  // wait while others are in town
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_NUM
    };
    enum Step_e { STEP_START, STEP_DONE };

    typedef void (dAcNpcSpTanukiBase_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor (RTTI "dAcNpcSpTanukiBase_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // What start() picks a message for (mKind).
        enum {
            KIND_GREET,   // the player came in (STATE_GREET)
            KIND_SEE_OFF, // the player is at the exit (STATE_SEE_OFF)
            KIND_CLOSING, // past closing time (STATE_CLOSING)
            KIND_TALK,    // the player talks to the npc
            KIND_ITEM,    // the npc speaks about the item the player points at
        };

        // npc_SP/NPC_tanukichi message codes, named from the English text
        // (NPC_twins uses the same codes for the twins).
        enum {
            MSG_WELCOME = 0x01,             // "Welcome! Please, please, browse"
            MSG_THANKS_FOR_SHOPPING = 0x02, // "Thanks much for shopping!"
            MSG_CLOSED_NOTICE = 0x03,       // closed on the upgrade day
            MSG_WHAT_CAN_I_DO = 0x04,       // the store menu
            MSG_VERY_GOOD_THEN = 0x05,      // "Very good then."
            MSG_ANYTHING_ELSE = 0x06,       // the store menu again
            MSG_ANYTHING_ELSE_2 = 0x07,     // "Well, all right then..."
            MSG_SELL_WHAT = 0x08,           // "Sell what, hm?"
            MSG_NO_SELLING = 0x09,          // the item select was closed
            MSG_SELL_PRICE = 0x0A,          // "I'll take that for ..."
            MSG_SELL_PRICE_ALL = 0x0B,      // "for all of that, ..."
            MSG_SOLD = 0x0C,                // "Thanks much! Anything else?"
            MSG_SELL_TOO_MUCH_MONEY = 0x0D, // the money goes in bags
            MSG_SELL_NO_ROOM = 0x0E,        // the player can't carry it
            MSG_SELL_NOT_BUYING = 0x0F,     // "I will not be buying that!"
            MSG_SELL_IMPORTANT = 0x12,      // something important is mixed in
            MSG_SELL_KEPT = 0x13,           // the player kept it
            MSG_TURNIP_PRICE = 0x14,        // "one turnip sells for ..."
            MSG_TURNIP_SUNDAY = 0x15,       // the stalk market is closed
            MSG_CATALOG = 0x16,             // "This catalog is for you"
            MSG_SELL_IMPORTANT_ONE = 0x17,  // that looks important
            MSG_ORDERED = 0x18,             // "The item will go to your house"
            MSG_NO_ORDER = 0x19,            // "So, no ordering then?"
            MSG_ORDERS_FULL = 0x1A,         // too many orders in
            MSG_ORDER_NO_MONEY = 0x1B,      // not enough money
            MSG_ORDER_OUT_OF_TOWN = 0x1C,   // no catalog for visitors
            MSG_FORGED_PAINTING = 0x1D,     // "this is a forgery"
            MSG_SELL_PRICE_FORGERY = 0x1E,  // the forgery's price
            MSG_SELL_PRICE_FORGERY_ALL = 0x1F,
            MSG_PRICE = 0x20,         // "It costs ... Will you buy it?"
            MSG_PRICE_TOOL = 0x21,    // "How do I use it?"
            MSG_PRICE_AGAIN = 0x22,   // "A steal at the low, low cost"
            MSG_PRICE_PREVIEW = 0x23, // wallpaper / carpet: "Let's see it!"
            MSG_PRICE_AFTER_PREVIEW = 0x24, // "So? You are loving it, yes?"
            MSG_PRICE_SPECIAL = 0x25,       // today's special
            MSG_PRICE_PAPER = 0x26,         // stationery, four to a pack
            MSG_NOT_BUYING = 0x27,          // "Maybe something else, hm?"
            MSG_BOUGHT = 0x28,              // "Thanks much!"
            MSG_BOUGHT_EXACTLY = 0x29,      // "It's ... Bells exactly!"
            MSG_BUY_NO_MONEY = 0x2A,        // not enough money
            MSG_BUY_NO_ROOM = 0x2B,         // the pockets are full
            MSG_PRICE_PAPER_AGAIN = 0x2C,   // the stationery after the preview
            MSG_CLOSING = 0x2D,             // "it's past closing time!"
            MSG_WHITE_TURNIP_SUNDAY = 0x2F, // no turnips on Sunday
            MSG_PRICE_SILVER_TOOL = 0x30,   // "an eye for quality"
            MSG_PRICE_SILVER_TOOL_AGAIN = 0x31,
            MSG_POINT_SYSTEM = 0x32, // the Tom Nook Point System
            MSG_HOW_TO_ROD = 0x33,   // how to use the tools
            MSG_HOW_TO_SHOVEL = 0x34,
            MSG_HOW_TO_NET = 0x35,
            MSG_HOW_TO_WATERING_CAN = 0x36,
            MSG_HOW_TO_AXE = 0x37,
            MSG_HOW_TO_TIMER = 0x38,
            MSG_HOW_TO_SLINGSHOT = 0x39,
            MSG_HOW_TO_END = 0x3A,   // (empty)
            MSG_ASK_HOW_TO = 0x3B,   // "Do you know how to use that?"
            MSG_KNOWS_HOW_TO = 0x3C, // "Good, good!"
            MSG_RENOVATED_1 = 0x3D,  // the house upgrades are done
            MSG_RENOVATED_2 = 0x41,
            MSG_RENOVATED_3 = 0x45,
            MSG_RENOVATED_4 = 0x48,
            MSG_FLAG = 0x4C, // "did you take a look at your flag?"
            MSG_ROOF_PAINT_OUT_OF_TOWN = 0x51, // paint is not sold to visitors
            MSG_PRICE_ROOF_PAINT = 0x52,       // paint for the roof
            MSG_ROOF_PAINT_BOUGHT = 0x53,      // "I'll go paint it"
            MSG_LOAN_PAID_1 = 0x56,            // the loans are paid off
            MSG_LOAN_PAID_2 = 0x59,
            MSG_LOAN_PAID_3 = 0x5C,
            MSG_LOAN_PAID_4 = 0x62,
            MSG_LOAN_PAID_5 = 0x65,
            MSG_ROOF_ASK_1 = 0x5F, // the roof color menus
            MSG_ROOF_ASK_2 = 0x61,
            MSG_ROOF_ASK_3 = 0x6D,
            MSG_ROOF_ASK_4 = 0x6F,
            MSG_ROOF_COLORS_2 = 0x70, // "How about these, then?"
            MSG_ROOF_COLORS_3 = 0x71, // "a bit somber..."
            MSG_ROOF_ASK_5 = 0x73,
            MSG_ROOF_COLOR = 0x74,            // "Your new roof is going to be"
            MSG_TIME_SPECIAL = 0x83,          // on sale (Time Special)
            MSG_TIME_SPECIAL_END = 0x84,      // (empty)
            MSG_WELCOME_POINT_SPECIAL = 0x86, // today is the Point Special
            MSG_WELCOME_TIME_SPECIAL = 0x87,  // a Time Special is running
            MSG_WELCOME_SPECIAL = 0x88,       // one item on special today
            MSG_GRAND_OPENING_1 = 0x89,       // Nook 'n' Go
            MSG_GRAND_OPENING_2 = 0x8A,       // Nookway
            MSG_GRAND_OPENING_3 = 0x8B,       // Nookington's
            MSG_GRAND_REOPENING_1 = 0x8C,
            MSG_GRAND_REOPENING_2 = 0x8D,
            MSG_GRAND_REOPENING_3 = 0x8E,
            MSG_MENU = 0x95,       // "So... What can I do for you?"
            MSG_STORE_VOTE = 0x97, // "something I want to ask you about"
            MSG_STORE_VOTE_DECLINED = 0x98, // "Oh, well. Never mind, hm?"
            MSG_STORE_VOTE_REASON = 0x99,   // what the store should focus on
            MSG_STORE_VOTE_HOURS = 0x9A,
            MSG_STORE_VOTE_BALANCE = 0x9B,
            MSG_STORE_VOTE_VARIETY = 0x9C,
            MSG_STORE_VOTE_THANKS = 0xA3, // "Thanks for that advice"
            MSG_STORE_VOTE_RESULT = 0xA4, // about that talk we had
            MSG_STORE_VOTE_RESULT_KEPT = 0xA5,
            MSG_KEEP_SHOPPING_1 = 0xA6, // "Don't forget to keep shopping"
            MSG_KEEP_SHOPPING_2 = 0xA7,
            MSG_KEEP_SHOPPING_3 = 0xA8,
        };

        // npc_SP/NPC_tanukichi_1 message codes (the part-time job).
        enum {
            JOB_MSG_NOT_ALL_MET = 0x01, // "You already finished saying hello?"
            JOB_MSG_HELLO_FIRST = 0x02, // "better to get this out of the way"
            JOB_MSG_ALL_MET = 0x03,     // "So, you've said hello to everyone"
            JOB_MSG_START = 0x04,       // the part-time job starts
            JOB_MSG_CHANGE_CLOTHES = 0x05,  // change into the work clothes
            JOB_MSG_CLOTHES_NO_ROOM = 0x06, // "You have too much stuff"
            JOB_MSG_DROP_STUFF = 0x07,      // "drop off some stuff"
            JOB_MSG_WAIT_CHANGE = 0x08,     // "Let me know when you're done"
            JOB_MSG_DROPPED_STUFF = 0x09, // "Looks like you dropped some stuff"
            JOB_MSG_CLOTHES_ON = 0x0A,    // "that looks quite fitting"
            JOB_MSG_PLANT = 0x0B,         // plant the seeds and saplings
            JOB_MSG_PLANT_DONE_ASK = 0x0C, // "let me know when you're done"
            JOB_MSG_PLANT_NO_ROOM = 0x0D,  // the pockets are full
            JOB_MSG_GIVE_SEEDS = 0x0E,     // "There you go"
            JOB_MSG_NOT_CHANGED = 0x0F,    // "I need you to change clothes"
            JOB_MSG_LOST_CLOTHES =
                0x10, // "what did you do with your work clothes"
            JOB_MSG_LOST_CLOTHES_FULL = 0x11, // ... and the pockets are full
            JOB_MSG_LOST_CLOTHES_NO_ROOM = 0x12,
            JOB_MSG_NEW_CLOTHES = 0x13,     // another set of work clothes
            JOB_MSG_PLANTED = 0x14,         // "You made it look nice"
            JOB_MSG_HOW_TO_PLANT = 0x15,    // nothing planted yet
            JOB_MSG_PLANT_MORE = 0x16,      // some planted
            JOB_MSG_REAL_PART_TIMER = 0x17, // the next jobs
            JOB_MSG_DELIVER_NO_ROOM = 0x18, // the pockets are full
            JOB_MSG_DELIVER_FTR = 0x19,     // deliver furniture
            JOB_MSG_DELIVERED = 0x1C,       // "And you made the delivery"
            JOB_MSG_NEXT_JOB = 0x1D,
            JOB_MSG_LETTER = 0x1E,    // direct mail
            JOB_MSG_ROOM_MADE = 0x20, // "you made some room" (then the letter)
            JOB_MSG_NOT_DELIVERED = 0x21, // "You haven't finished the delivery"
            JOB_MSG_NOT_DELIVERED_AGAIN = 0x22,
            JOB_MSG_HOW_TO_LETTER = 0x23, // how to write a letter
            JOB_MSG_MORE_PAPER = 0x24,    // "Do you need more stationery?"
            JOB_MSG_PAPER_NO_ROOM = 0x25,
            JOB_MSG_GIVE_PAPER = 0x26,  // "Brand-new stationery!"
            JOB_MSG_LETTER_SENT = 0x28, // "Did you send it out?"
            JOB_MSG_MORE_PAPER_2 = 0x29,
            JOB_MSG_LETTER_WRONG = 0x2A, // "send it to ... this time!"
            JOB_MSG_LETTER_LATE = 0x2B,
            JOB_MSG_CARPET = 0x2C, // deliver a carpet
            JOB_MSG_CARPET_NO_ROOM = 0x2D,
            JOB_MSG_CARPET_GIVEN = 0x2E,
            JOB_MSG_DELIVERY_WAIT = 0x2F, // "You've got a delivery to make"
            JOB_MSG_CARPET_DELIVERED = 0x30,
            JOB_MSG_WATERING_CAN = 0x31, // deliver a watering can
            JOB_MSG_WATERING_CAN_DELIVERED = 0x33,
            JOB_MSG_NOTICE_WAIT = 0x34, // the bulletin board message
            JOB_MSG_NOTICE_POSTED = 0x35,
            JOB_MSG_LOAN = 0x36,            // "Your home loan?"
            JOB_MSG_NO_WORK_LEFT = 0x37,    // "no work left here": the job ends
            JOB_MSG_NOT_FOR_WORKERS = 0x38, // "Those are for customers!"
            JOB_MSG_SECOND_FLOOR = 0x39,    // "That's the second floor!"
            JOB_MSG_NO_WORK_CLOTHES =
                0x3A,                // "Why aren't your work clothes on?!"
            JOB_MSG_FTR_LOST = 0x3B, // the furniture wasn't delivered
            JOB_MSG_FTR_GIVE_UP_ASK = 0x3C, // "You can't find the place?"
            JOB_MSG_FTR_GIVE_UP = 0x3D,     // "Just deliver this instead"
            JOB_MSG_FTR_AGAIN = 0x3E,
            JOB_MSG_LETTER_LOST = 0x3F, // the letter wasn't sent
            JOB_MSG_LETTER_INSTEAD = 0x40,
            JOB_MSG_DO_IT_RIGHT = 0x41,
            JOB_MSG_CARPET_LOST = 0x42, // the carpet wasn't delivered
            JOB_MSG_CARPET_GIVE_UP_ASK = 0x43,
            JOB_MSG_CARPET_GIVE_UP = 0x44,
            JOB_MSG_CARPET_AGAIN = 0x45,
            JOB_MSG_WATERING_CAN_LOST = 0x46,
            JOB_MSG_WATERING_CAN_GIVE_UP = 0x47,
            JOB_MSG_FIND_ELSE = 0x48, // "I'll find something else"
            JOB_MSG_NOTICE = 0x49,    // post on the bulletin board
            JOB_MSG_MORE_PAPER_3 = 0x4A,
            JOB_MSG_ROOM_MADE_CHECK = 0x4B, // "Wait a second!"
            JOB_MSG_CLOTHES_NO_ROOM_2 = 0x4C,
            JOB_MSG_SAY_HELLO = 0x4D,     // "Some neighbors haven't met you"
            JOB_MSG_INTRODUCTIONS = 0x4E, // go and say hello first
            JOB_MSG_DELIVER_OTHER = 0x4F, // deliver to someone else
            JOB_MSG_DELIVER_OTHER_2 = 0x50,
            JOB_MSG_THINGS_CHANGED = 0x51,
            // "You said hello, yes?", then the next job
            JOB_MSG_HELLO_THEN_FTR = 0x52,
            JOB_MSG_HELLO_THEN_LETTER = 0x53,
            JOB_MSG_HELLO_THEN_OTHER_FTR = 0x54,
            JOB_MSG_HELLO_THEN_CARPET = 0x55,
            JOB_MSG_HELLO_THEN_OTHER_LETTER = 0x56,
            JOB_MSG_HELLO_THEN_WATERING_CAN = 0x57,
            JOB_MSG_HELLO_THEN_OTHER_CARPET = 0x58,
            // "you made some room", then the next job
            JOB_MSG_ROOM_THEN_OTHER_FTR = 0x59,
            JOB_MSG_ROOM_THEN_CARPET = 0x5A,
            JOB_MSG_ROOM_THEN_WATERING_CAN = 0x5B,
            JOB_MSG_ROOM_THEN_OTHER_CARPET = 0x5C,
        };

        // npc_SP/NPC_drama6 message codes (Tom Nook and the Able sisters'
        // story, dPrivateData_c::getGracieStep).
        enum {
            DRAMA_MSG_WORRIED = 0x01,      // "What's wrong? / Back to work!"
            DRAMA_MSG_BACK_TO_WORK = 0x02, // "Sorry about that."
            DRAMA_MSG_TOLD = 0x07,         // the player heard about Labelle
            DRAMA_MSG_REGRET = 0x09,       // the second time
            DRAMA_MSG_BACK_TO_WORK_2 = 0x0A,
            DRAMA_MSG_TOLD_2 = 0x0D,
        };

        // sys_MAIL1 MAIL_NPC_Tanukichi codes: the catalog order's delivery
        // letter, signed by the store of each stage (SHOP_STAGE_* + 1).
        enum {
            ORDER_MAIL_CRANNY = 1,
            ORDER_MAIL_NOOK_N_GO = 2,
            ORDER_MAIL_NOOKWAY = 3,
            ORDER_MAIL_NOOKINGTONS = 4,
        };

        // The roof color menus (MSG_ROOF_ASK_*, MSG_ROOF_COLORS_*) offer four
        // colors each, then "Another color.".
        enum { ROOF_COLORS_PER_MENU = 4 };

        // The part-time job's planting (giveSeeds): four seed bags and three
        // saplings.
        enum {
            JOB_SEED_BAG_NUM = 4,
            JOB_SAPLING_NUM = 3,
            JOB_PLANT_ITEM_NUM = JOB_SEED_BAG_NUM + JOB_SAPLING_NUM,
        };

        talk_c(dAcNpcSpTanukiBase_c *npc);        // 0x24C4
        virtual ~talk_c();                        // 0x2560
        virtual void onMessageStart(int arg);     // 0x37C0
        virtual void onMessageEnd(int kind);      // 0x3D40
        virtual void onAnswer(int answer);        // 0x51B0
        virtual void init();                      // 0x25B8
        virtual void onCustomTag(int no);         // 0x5EC4
        virtual void preExecute();                // 0x59DC
        virtual void onTalkEnd();                 // 0x59E0
        virtual void getMsgInfo(msgInfo_s *info); // 0x36E4
        virtual void onRequestEnd(int kind);      // 0x5E60
        void start(dActor_c *target);             // 0x2654

        u16 getPriceMsg();               // 0x2EA8
        int getJobTalkMsg();             // 0x2FB0
        int getJobGreetMsg();            // 0x36A0
        int getJobSeeOffMsg();           // 0x36D4
        int getJobItemMsg();             // 0x36DC
        void onJobMessageStart(int arg); // 0x3BE8
        void onJobMessageEnd(int kind);  // 0x42A4
        void onStoreAnswer(int sel);
        void onStoreAnswer(int code, int sel);
        void onJobAnswer(int answer);                    // 0x5954
        BOOL buy();                                      // 0x5B58
        u16 getHowToMsg(dItem::Item *item);              // 0x5D68
        void startGiveSeeds();                           // 0x5DEC
        static BOOL cannotSell(void *pockets, int flag); // 0x5F60
        // ---- message hooks (mOnMessageEnd / mOnAnswer / mOnRequestEnd) ----
        void selectSellItems();   // 0x5FFC
        void openCatalog();       // 0x6060
        void openStationery();    // 0x60B8
        void handOver();          // 0x6168
        void receiveItem();       // 0x6178
        void onSellSelected();    // 0x6194
        void onCatalogSelected(); // 0x64D8
        void askPaperAgain();     // 0x657C
        void giveItem();          // 0x6588
        void giveErrandItem();    // 0x65C0
        void giveSeeds();         // 0x65F8
        void takeErrandItems();   // 0x66F8

        void setPointedItem(const dItem::Item &item, int unitX, int unitZ) {
            mItem = item;
            mUnitX = unitX;
            mUnitZ = unitZ;
        }

        /* 0x0EA */ u16 mMsgCode; // the message start() picked
        /* 0x0EC */ dAcNpcSpTanukiBase_c *mpOwner;
        /* 0x0F0 */ Proc
            mOnMessageEnd; // one-shot hooks (cleared after the call)
        /* 0x0FC */ Proc mOnAnswer;
        /* 0x108 */ Proc mOnRequestEnd;
        /* 0x114 */ int mKind;         // KIND_*
        /* 0x118 */ dItem::Item mItem; // the item talked about
        /* 0x11C */ int mPrice;        // its price
        /* 0x120 */ int mSellPrice;    // what the store pays
        /* 0x124 */ int mUnitX;        // the pointed item's unit
        /* 0x128 */ int mUnitZ;
        /* 0x12C */ int mTurnipPrice;     // the stalk market's price now
        /* 0x130 */ u8 mExitRequested;    // the scene change was requested
        /* 0x131 */ u8 mRoomPreview;      // a wallpaper / carpet is shown
        /* 0x132 */ u8 mDeliveryReminded; // JOB_MSG_NOT_DELIVERED was said
        /* 0x133 */ u8 mBought;
        /* 0x134 */ u8 mHowToAsked;
        /* 0x135 */ u8 mOrdered;
        /* 0x136 */ u8 mSaleNow; // a Time Special is running
    }; // size 0x138

    dAcNpcSpTanukiBase_c() : mBaseTalk(this) {
    }
    virtual ~dAcNpcSpTanukiBase_c() {
    } // weak
    virtual void onDemoCmd(int cmd, u8 member); // 0x838
    virtual int preCreate();                    // 0xA4C
    virtual int create();                       // 0xAF4
    virtual int doDelete();                     // 0xEF0
    virtual int vtB0();                         // 0xF1C
    virtual int canSpeak(dDemoActor_c *actor);  // 0xF64
    virtual int getEarType() {
        return NPC_EAR_TYPE_NONE;
    } // weak
    // new virtuals (+0xE8..+0x104)
    virtual u16 getPartnerProfName() = 0; // the other twin
    virtual BOOL isNearerToPlayer() = 0;  // than the other twin
    virtual BOOL isTanukichi() = 0;
    virtual BOOL isTubukichi() = 0;
    virtual BOOL isMamekichi() = 0;
    virtual BOOL isTwin() {
        return isTubukichi() || isMamekichi();
    } // weak
    virtual f32 getStopDist() = 0; // STATE_APPROACH stops within it
    virtual f32 getMoveDist() = 0; // STATE_APPROACH starts / runs beyond it

    void changeState(int state); // 0xA14

    // ---- states ----
    void initWait();          // 0xFB4
    void executeWait();       // 0x103C
    void initTurn();          // 0x124C
    void executeTurn();       // 0x12B0
    void initApproach();      // 0x1480
    void executeApproach();   // 0x1504
    void initLook();          // 0x1774
    void stepLook();          // 0x17D8
    void executeLook();       // 0x1838
    void initTalk();          // 0x1878
    void stepTalk();          // 0x18E4
    void executeTalk();       // 0x193C
    void initGreet();         // 0x19D4
    void executeGreet();      // 0x19E0
    void initSeeOff();        // 0x1A70
    void executeSeeOff();     // 0x1A7C
    void initClosing();       // 0x1AC0
    void executeClosing();    // 0x1ACC
    void initMovePlayer();    // 0x1B5C
    void executeMovePlayer(); // 0x1BB8
    void initWatch();         // 0x1C08
    void executeWatch();      // 0x1CA0
    void initMultiWait();     // 0x1D14
    void executeMultiWait();  // 0x1D28
    void initRemote();        // 0x1D80
    void executeRemote();     // 0x1D84
    void initRemoteTalk();    // 0x1E4C
    void executeRemoteTalk(); // 0x1E50

    BOOL isSeeOffTime();     // 0x1EA4: the player is at the exit
    BOOL checkPointedItem(); // 0x1F78: the player points at a shop item
    BOOL isWorkerAtExit();   // 0x2078: the part-timer is at the stairs
    BOOL isClosingTime();    // 0x20F4
    BOOL isShopClosed();     // 0x2190

    // executeWait: turn to the player once he is behind / after walking over.
    BOOL isFacingAway() {
        BOOL ret = FALSE;
        if (!mApproached) {
            if (sLib::distanceAngle(mAngle.y, getAngleYToPlayer(_1B70)) >
                0x4000) {
                ret = TRUE;
            }
        }
        return ret;
    }
    BOOL isNotFacing() {
        BOOL ret = FALSE;
        if (mApproached) {
            if (sLib::distanceAngle(mAngle.y, getAngleYToPlayer(_1B70)) > 0) {
                ret = TRUE;
            }
        }
        return ret;
    }

    mAng getTalkAngle() const {
        dActor_c *target = mBaseTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C */ int mState; // STATE_*
    /* 0x1C80 */ u8 mStep[STATE_NUM];
    /* 0x1C90 */ talk_c mBaseTalk;
    /* 0x1DC8 */ u8 mApproached; // walked over to the player once
    /* 0x1DC9 */ u8 mDrama;      // the Able sisters' story is under way
}; // size 0x1DCC

// Tom Nook (RTTI "dAcNpcSpTanukichi_c").
class dAcNpcSpTanukichi_c : public dAcNpcSpTanukiBase_c {
  public:
    virtual ~dAcNpcSpTanukichi_c();   // 0x67E0
    virtual int getEarType();         // 0x6768: by the store's stage
    virtual u16 getPartnerProfName(); // 0x24AC: his own
    virtual BOOL isNearerToPlayer() {
        return TRUE;
    }
    virtual BOOL isTanukichi() {
        return TRUE;
    }
    virtual BOOL isTubukichi() {
        return FALSE;
    }
    virtual BOOL isMamekichi() {
        return FALSE;
    }
    virtual f32 getStopDist(); // 0x21C4
    virtual f32 getMoveDist(); // 0x231C
};

// The twins Timmy and Tommy (RTTI "dAcNpcSpTubuMame_c").
class dAcNpcSpTubuMame_c : public dAcNpcSpTanukiBase_c {
  public:
    virtual ~dAcNpcSpTubuMame_c();   // 0x584 (inline, in the .cpp)
    virtual int getEarType();        // 0x67D8
    virtual BOOL isNearerToPlayer(); // 0x23CC
    virtual f32 getStopDist();       // 0x21E8
    virtual f32 getMoveDist();       // 0x2340
};

// Timmy (RTTI "dAcNpcSpMamekichi_c").
class dAcNpcSpMamekichi_c : public dAcNpcSpTubuMame_c {
  public:
    virtual ~dAcNpcSpMamekichi_c();   // 0x6918
    virtual u16 getPartnerProfName(); // 0x24B4: Tommy
    virtual BOOL isTanukichi() {
        return FALSE;
    }
    virtual BOOL isTubukichi() {
        return FALSE;
    }
    virtual BOOL isMamekichi() {
        return TRUE;
    }
};

// Tommy (RTTI "dAcNpcSpTubukichi_c").
class dAcNpcSpTubukichi_c : public dAcNpcSpTubuMame_c {
  public:
    virtual ~dAcNpcSpTubukichi_c();   // 0x6A54
    virtual u16 getPartnerProfName(); // 0x24BC: Timmy
    virtual BOOL isTanukichi() {
        return FALSE;
    }
    virtual BOOL isTubukichi() {
        return TRUE;
    }
    virtual BOOL isMamekichi() {
        return FALSE;
    }
};

#endif
