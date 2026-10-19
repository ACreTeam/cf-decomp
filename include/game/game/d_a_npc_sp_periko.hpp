#ifndef D_A_NPC_SP_PERIKO_HPP
#define D_A_NPC_SP_PERIKO_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_state.hpp>

class dAnimal_c;

// Pelly and Phyllis, the town hall clerks (REL d_a_npc_sp_perikoNP, profiles
// NPC_SP_PERIKO / NPC_SP_PERIMI, item keys SP_NPC_POST_GIRL / _GIRL2). The town
// hall has two windows: the civic center (WINDOW_TOWN_HALL) and the post
// office (WINDOW_POST_OFFICE). Alone, Pelly works by day (7:00 to 22:00) and
// Phyllis by night, and she runs to the window the player stands at; in
// multiplayer Pelly creates Phyllis (preCreate) and takes the post office
// window, Phyllis the town hall window.
// The talk class runs both windows: newcomers (npc_SP/NPC_periko_0), the town
// tune, the town fund and public works votes, the town assessment, villager
// complaints, house clearing, the mail and the Wii Message Board.
//
// Class names from RTTI ("dAcNpcSpPerikoBase_c", its talk_c,
// "dAcNpcSpPeriko_c", "dAcNpcSpPerimi_c"); mode names from the original CP932
// state labels; everything else is named from behaviour and the messages.
//
// Declaration order matters: .data holds the vtables in reverse
// class-completion order (dAcNpcSpPerimi_c, dAcNpcSpPeriko_c, the base, then
// talk_c and its templates).
class dAcNpcSpPerikoBase_c : public dAcNpcSp_c,
                             public dState::mode_c<dAcNpcSpPerikoBase_c> {
  public:
    // dAcNpc_c::action_c request priorities.
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    // Who she is (mType).
    enum Type_e {
        TYPE_PELLY,   // dAcNpcSpPeriko_c, SP_NPC_POST_GIRL
        TYPE_PHYLLIS, // dAcNpcSpPerimi_c, SP_NPC_POST_GIRL2
        TYPE_NUM
    };

    // Alone, Pelly works from PELLY_START_HOUR to PHYLLIS_START_HOUR and
    // Phyllis the rest of the day (preCreate, getMenuMsg).
    enum { PELLY_START_HOUR = 7, PHYLLIS_START_HOUR = 22 };

    // The windows of the town hall (mWindow, l_windowPos).
    enum Window_e {
        WINDOW_TOWN_HALL,   // the civic center (x 240)
        WINDOW_POST_OFFICE, // the post office (x 304)
        WINDOW_NUM,
        WINDOW_NONE = WINDOW_NUM, // getWindowAt: in front of neither
    };

    typedef dState::base_c<dAcNpcSpPerikoBase_c>::Method Method;

    // The modes (the table in create), in table order.
    enum {
        MODE_WAIT,        // "待機"
        MODE_MOVE,        // "移動": run to the other window
        MODE_DEMO_WAIT,   // "デモ待機"
        MODE_TALK,        // "会話"
        MODE_TURN_BACK,   // "初期角度に復帰" (back to the initial angle)
        MODE_PARTNER,     // "三人会話パートナーの振る舞い" (the partner of
                          // a three-way talk)
        MODE_REMOTE,      // "通信：リモート"
        MODE_REMOTE_TALK, // "通信：リモート会話"
        MODE_WAIT_CLAP,   // "拍手待ち"
        MODE_CLAP,        // "拍手"
    };

    // The message receptor (RTTI "dAcNpcSpPerikoBase_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c, public dMsgRcptExp_c<talk_c> {
      public:
        typedef dState::base_c<talk_c>::Method Method;

        // The talk modes (the table in the constructor), in table order.
        enum {
            MODE_WAIT,                // "待機"
            MODE_WII_BOARD_POST,      // "Wii伝言板書き込み"
            MODE_WII_BOARD_POST_DONE, // "Wii伝言板書き込み完了チェック"
            MODE_POST_OFFICE_CHECK,   // "郵便局の手紙枠の空きチェック"
        };

        // The message labels (l_msgLabels rows).
        enum {
            LABEL_NEWCOMER,  // "npc_SP/NPC_periko_0" (both)
            LABEL_OWN,       // "npc_SP/NPC_periko" / "npc_SP/NPC_perimi"
            LABEL_REPORT,    // "npc_SP/NPC_tousho": the townsfolk comments
            LABEL_SAVE,      // dSvMgr_c::mMessageLabel
            LABEL_NET_ERROR, // sNetErrorMessageLabel
            LABEL_DRAMA,     // "npc_SP/NPC_drama4": Pelly's sigh (Gracie
                             // story step 2)
            LABEL_NUM
        };

        // Which mail menu openMailMenu opens (mMailKind).
        enum {
            MAIL_FUTURE,     // a letter to the future self
            MAIL_TOWN,       // in town
            MAIL_OTHER_TOWN, // to another town
        };

        // What a net error message was about (mNetErrorFrom).
        enum {
            NET_ERROR_FROM_MAIL,       // the other-town mail
            NET_ERROR_FROM_WII_BOARD,  // the Wii Message Board
            NET_ERROR_FROM_BOARD_POST, // posting to the Wii Message Board
        };

        // The rooms recycleRoom clears (mRoom): dHome_c room indices.
        enum { ROOM_FIRST_FLOOR, ROOM_SECOND_FLOOR, ROOM_BASEMENT };

        // selectVillagers: villagers shown per page of the choice.
        enum { VILLAGERS_PER_PAGE = 3 };

        // onMessageEnd requests (BMG entry +0xA).
        enum {
            END_REQ_HAND_OVER = 4,      // the letters / the golden can
            END_REQ_TABLE_FIRST = 0x10, // endRequests: 0x10..0x14
            END_REQ_TABLE_NUM = 5,
        };

        // sys_SELECT/SYS_Select choices.
        enum {
            SEL_MSG_SOMEONE_ELSE = 0x0F,
            SEL_MSG_NEVER_MIND = 0x10,
            SEL_MSG_THIS_TOWN = 0x18, // "<town>."
            SEL_MSG_OTHER_TOWN = 0x19,
            SEL_MSG_FUTURE_SELF = 0x1A,
            SEL_MSG_NEVER_MIND_MAIL = 0x1B,
            SEL_MSG_FIRST_FLOOR = 0x1C,
            SEL_MSG_SECOND_FLOOR = 0x1D,
            SEL_MSG_BASEMENT = 0x1E,
            SEL_MSG_NEVER_MIND_ROOM = 0x1F,
        };

        // sys_STRING/STR_Unit catchphrases resetHabitWord switches between
        // (two per personality).
        enum {
            UNIT_HABIT_NORMAL = 0x3D, // "you know"
            UNIT_HABIT_NORMAL2,       // "uh huh"
            UNIT_HABIT_PEPPY,         // "yessiree"
            UNIT_HABIT_PEPPY2,        // "yup yup"
            UNIT_HABIT_SNOOTY,        // "yes yes"
            UNIT_HABIT_SNOOTY2,       // "totally"
            UNIT_HABIT_LAZY,          // "whoo hoo"
            UNIT_HABIT_LAZY2,         // "all right"
            UNIT_HABIT_JOCK,          // "right"
            UNIT_HABIT_JOCK2,         // "yo yo yo"
            UNIT_HABIT_CRANKY,        // "got it"
            UNIT_HABIT_CRANKY2,       // "my bad"
        };

        // npc_SP/NPC_periko_0 messages (the newcomer).
        enum {
            MSG_NEWCOMER_WELCOME = 0x01,
            MSG_NEWCOMER_VACANT_HOUSES = 0x02,
            MSG_NEWCOMER_VACANT_HOUSE = 0x04, // only one house left
            MSG_NEWCOMER_AGAIN = 0x09,        // no house yet
            MSG_NEWCOMER_HOUSE_DONE = 0x0D,   // the paperwork is done
        };

        // npc_SP/NPC_periko (/ NPC_perimi) messages, named from the English
        // text.
        enum {
            MSG_TOWN_HALL_MENU = 0x01,       // "What can I help you with?"
            MSG_TOWN_HALL_MENU_AGAIN = 0x03, // "anything else...?"
            MSG_TOWN_TUNE_SAVED = 0x05,
            MSG_TOWN_TUNE_GIVE_UP = 0x06,
            MSG_READ_COMMENTS = 0x07,
            MSG_PROBLEM_FIRST = 0x08, // 0x08 + the worst block's fault
            MSG_TOWN_WORK_TOGETHER = 0x0E,
            MSG_GREAT_PLACE = 0x0F,
            MSG_LUCKIEST_PELICAN = 0x10,
            MSG_GOLDEN_CAN = 0x11, // the town became a top contributor
            MSG_FUND_FIRST = 0x14,
            MSG_FUND = 0x15,
            MSG_FUND_DECLINED = 0x16,
            MSG_FUND_THANKS = 0x17,
            MSG_BRIDGE_SURVEY_FIRST = 0x18,
            MSG_BRIDGE_SURVEY = 0x19,
            MSG_BRIDGE_VOTED = 0x1A,
            MSG_BRIDGE_PASS = 0x1B,
            MSG_BRIDGE_BUILDING = 0x1C,
            MSG_TOWER_SURVEY_FIRST = 0x1D,
            MSG_TOWER_SURVEY = 0x1E,
            MSG_TOWER_BUILDING = 0x22,
            MSG_CONVERT_LIGHTHOUSE = 0x23, // the extra Bells: lighthouse ->
                                           // windmill?
            MSG_CONVERT_WINDMILL = 0x24,
            MSG_CONVERTING_LIGHTHOUSE = 0x27,
            MSG_CONVERTING_WINDMILL = 0x28,
            MSG_CONVERT_LIGHTHOUSE_VOTE = 0x29,
            MSG_CONVERT_WINDMILL_VOTE = 0x2A,
            MSG_FOUNTAIN_BUILDING = 0x2E,
            MSG_FUND_COOPERATION = 0x2F,  // "Thank you so much for your
                                          // cooperation!"
            MSG_FUND_PAID = 0x30,         // "We thank you for your generosity"
            MSG_TOWN_HALL_WELCOME = 0x33, // visitors: only the post office
            MSG_TOWN_HALL_CLOSED = 0x34,  // the town hall is closed at night
            MSG_WHO_PROBLEM = 0x3D,
            MSG_NOTHING_THE_MATTER = 0x3E, // "Oh, nothing's the matter then?"
            MSG_WHO_PROBLEM_MORE = 0x3F,   // the next page of villagers
            MSG_WHAT_PROBLEM = 0x40,       // "What seems to be the problem
                                           // with <villager>?"
            MSG_RUDE = 0x43,
            MSG_CLOTHES = 0x45,
            MSG_LETTERS = 0x47,
            MSG_ROOM_NEVER_MIND = 0x4A, // "that's not really my preferred
                                        // way of doing things either"
            MSG_WHICH_ROOM = 0x4B,
            MSG_ROOM_FIRST_FLOOR = 0x4C,
            MSG_ROOM_SECOND_FLOOR = 0x4D,
            MSG_ROOM_BASEMENT = 0x4E,
            MSG_ROOM_CLEARED = 0x50,
            MSG_BRIDGE_THANKS = 0x51,
            MSG_FOUNTAIN_THANKS = 0x52,
            MSG_LIGHTHOUSE_THANKS = 0x53,
            MSG_WINDMILL_THANKS = 0x54,
            MSG_LIGHTHOUSE_THANKS2 = 0x55, // after a conversion
            MSG_WINDMILL_THANKS2 = 0x56,
            MSG_WII_BOARD_OWN = 0x5B,
            MSG_WII_BOARD_OWN2 = 0x5C,
            MSG_COME_AGAIN = 0x5D,
            MSG_MAIL_SEND = 0x5F,     // (empty)
            MSG_WII_BOARD_END = 0x61, // (empty)
            MSG_POST_OFFICE = 0x64,   // (a tag only): picks the menu
            MSG_POST_OFFICE_MENU = 0x65,
            MSG_POST_OFFICE_MENU2 = 0x66, // without "Save mail."
            MSG_POST_OFFICE_MENU_AGAIN = 0x67,
            MSG_POST_OFFICE_MENU_AGAIN2 = 0x68,
            MSG_MAIL_SAVED = 0x69,
            MSG_MAIL_SAVE_CANCELLED = 0x6C,
            MSG_WII_BOARD_DAY_OVER = 0x6D,
            MSG_ADD_PHOTO = 0x6E,
            MSG_BOARD_POSTED = 0x6F,
            MSG_WII_BOARD_FRIENDS = 0x70,
            MSG_NO_MESSAGE = 0x71,
            MSG_WHERE_SEND = 0x72,
            MSG_ANY_OTHER_LETTERS = 0x73,
            MSG_NO_MAIL = 0x75,
            MSG_NOT_RESIDENT_RETURNED = 0x76,
            MSG_ALSO = 0x77,
            MSG_MAILBOX_FULL1 = 0x78,
            MSG_MAILBOX_FULL2 = 0x79,
            MSG_MAILBOX_FULL3 = 0x7A,
            MSG_MAIL_TAKEN = 0x7B,
            MSG_MAIL_TAKEN_REST = 0x7C,
            MSG_OTHER_TOWN_ROUTES_FULL = 0x7D,
            MSG_OTHER_TOWN_MAIL_TAKEN = 0x7F,
            MSG_FUTURE_WHEN = 0x81,
            MSG_FUTURE_LIMIT = 0x85,
            MSG_POST_OFFICE_FULL = 0x88,
            MSG_VISITOR_TOWN_ONLY = 0x89,
            MSG_WII_BOARD_OTHERS_ONLY = 0x8B,
            MSG_WII_BOARD_OWN_ONLY = 0x8C,
            MSG_WII_BOARD_RULES = 0x8D,
            MSG_WC24_OFF_BOARD = 0x8E,
            MSG_WC24_OFF_MAIL = 0x8F,
            MSG_PREPARE_BOARD_POST = 0x92,
            MSG_BOARD_RETRY = 0x93,
            MSG_BOARD_DISPOSED = 0x95,
            MSG_FRIENDS_ONLY = 0x96,
            MSG_OTHER_TOWN_MAIL = 0x97,
            MSG_POST_OFFICE_FULL2 = 0x98,
            MSG_VISITOR_LETTER = 0x99,
            MSG_VISITOR_NO_BOARD = 0x9A,
            MSG_MAILBOX_FULL4 = 0x9B,
        };

        // npc_SP/NPC_tousho messages: the townsfolk comments, best to worst
        // (readTownComments: MSG_REPORT_BEST + the assessment rank).
        enum {
            MSG_REPORT_BEST = 0x01, // "Fantastic! ... is in great shape"
            MSG_REPORT_GOOD = 0x02, // "still seems to be missing something"
            MSG_REPORT_D_PLUS = 0x03,
            MSG_REPORT_POOR = 0x04,
            MSG_REPORT_PITS = 0x05, // "This place is the pits!"
        };

        // npc_SP/NPC_drama4 messages.
        enum {
            MSG_DRAMA_SIGH = 0x01,
            MSG_DRAMA_SPACED_OUT = 0x02,
            MSG_DRAMA_LETTER = 0x04,
        };

        talk_c(dAcNpcSpPerikoBase_c *owner); // 0x29F0
        virtual ~talk_c() {
        } // weak
        virtual void onMessageStart(int arg);     // 0x34C0
        virtual void onMessageEnd(int kind);      // 0x34C4
        virtual void onAnswer(int answer);        // 0x4AF4
        virtual void init();                      // 0x2BEC
        virtual void onCustomTag(int no);         // 0x5634
        virtual void preExecute();                // 0x3404
        virtual void onTalkEnd();                 // 0x3454
        virtual void getMsgInfo(msgInfo_s *info); // 0x3368
        virtual void onRequestEnd(int kind);      // 0x5668

        BOOL isPelly() const;         // 0x2AC0
        BOOL isBridgeBuilt();         // 0x2AC8
        BOOL hasFountain();           // 0x2B14
        BOOL hasWindmill();           // 0x2B5C
        BOOL hasLighthouse();         // 0x2BA4
        void start(dActor_c *target); // 0x2C3C: picks the first message

        // The thanks for a finished public work (once per player).
        BOOL isBridgeThanksDue();      // 0x2E68
        BOOL isFountainThanksDue();    // 0x2F18
        BOOL isWindmillThanksDue();    // 0x2FC8
        BOOL isLighthouseThanksDue();  // 0x3078
        BOOL isWindmillThanks2Due();   // 0x3128
        BOOL isLighthouseThanks2Due(); // 0x31D8
        int getMenuMsg();              // 0x3288: the window's menu

        void setNextMessage(u16 code, int label); // 0x3458

        // ---- message handlers (onMessageEnd tables) ----
        void selectMailDestination(); // 0x3794
        void endDrama();              // 0x3880
        void resetHabitWord();        // 0x38CC: the villager's catchphrase
        void resetCloth();            // 0x3AB8: the villager's shirt
        void showMenu();              // 0x3B88
        void clearLetter();           // 0x3BC8: the villager's letter
        void selectFirstVillagers();  // 0x3BD4
        void selectVillagers();       // 0x3BE0: three a page
        void showVacantHouses();      // 0x3CF4
        void openBridgeMap();         // 0x3D7C
        void readTownComments();      // 0x3DC8
        void endBestReport();         // 0x3E0C: maybe the golden can
        void endGoodReport();         // 0x3EFC
        void endBadReport();          // 0x3F08: the worst block's fault
        void endNetError();           // 0x3F4C
        void endBoardError();         // 0x3FD4
        void lockMessage();           // 0x405C
        void endWc24Error();          // 0x406C
        void endMailReturned();       // 0x40F4
        void showFullMailboxes();     // 0x414C
        void endFullMailboxes();      // 0x42C8
        void startBoardPost();        // 0x4320
        void startPostOfficeCheck();  // 0x4490
        void handOverLetters();       // 0x4600
        void handOverGoldenCan();     // 0x4648
        void openHouseMap();          // 0x4690
        void openTownTuneMenu();      // 0x46DC
        void openFundMenu();          // 0x4728
        void openMailSaveMenu();      // 0x4774
        void openMailMenu();          // 0x47C0
        void openFutureDateMenu();    // 0x4894
        void setRoomFirstFloor();     // 0x48E0
        void setRoomSecondFloor();    // 0x48EC
        void setRoomBasement();       // 0x48F8
        void recycleRoom();           // 0x4904
        void selectRoom();            // 0x49A4

        // ---- answer handlers (onAnswer table) ----
        void setDramaHeard();         // 0x4BB4
        void selectVillager();        // 0x4BDC
        void answerSendMail();        // 0x4CC4
        void answerWiiBoard();        // 0x4D1C
        void answerOwnBoard();        // 0x4DC8
        void answerFriendBoard();     // 0x4E64
        void answerSendMessage();     // 0x4F5C
        void answerNoMessage();       // 0x4E38
        void answerAddPhoto();        // 0x4FE4
        void answerNoPhoto();         // 0x5030
        void answerMailDestination(); // 0x5038
        void startTownMail();         // 0x50F8
        void startOtherTownMail();    // 0x5158
        void startFutureMail();       // 0x5260
        void answerTownFund();        // 0x529C
        void fundConversion();        // 0x5530
        void voteWindmill();          // 0x5558
        void voteLighthouse();        // 0x5598
        int getFundMsg();             // 0x55D8

        // answerTownFund: the vote runs for 7 days from the first ask.
        static void startVote() {
            if (dSaveData_c::getRaw()->mPublicWorksDays == 0xFF) {
                dSaveData_c::getTown()->setPublicWorksDays(7);
            }
        }

        // ---- request-end methods ----
        void endHouseMap();          // 0x5700
        void endFund();              // 0x5704
        void endTownTuneMenu();      // 0x5878
        void endMailSaveMenu();      // 0x58A0
        void setFutureDateWords();   // 0x58C8
        void closeMessage();         // 0x5980
        void endTownMailMenu();      // 0x5984
        void endOtherTownMailMenu(); // 0x5A44
        void endFutureMailMenu();    // 0x5B0C
        void endFriendBoardMenu();   // 0x5C64
        void endOwnBoardMenu();      // 0x5D20
        void endPhotoMenu();         // 0x5DA0
        void endBridgeMap();         // 0x5DFC

        // ---- modes ----
        void initWait();                // 0x5E90
        void executeWait();             // 0x5E94
        void initWiiBoardPost();        // 0x5E98
        void executeWiiBoardPost();     // 0x5E9C
        void initWiiBoardPostDone();    // 0x6004
        void executeWiiBoardPostDone(); // 0x6008
        void initPostOfficeCheck();     // 0x6328
        void executePostOfficeCheck();  // 0x6354

        void setWc24ErrorMsg();       // 0x65A0
        void setNetErrorMsg();        // 0x6620
        void switchToSystemSpeaker(); // 0x66BC (request-end hideSpeaker)
        void switchToOwnSpeaker();    // 0x6798 (request-end showSpeaker)
        void hideSpeaker();           // 0x6874
        void showSpeaker();           // 0x68B4

        /* 0x0EC dMsgRcptExp_c<talk_c> */
        /* 0x128 */ u16 mMsgNo; // the first message
        /* 0x12C */ dAcNpcSpPerikoBase_c *mpOwner;
        /* 0x130 */ int mMailKind;         // MAIL_*
        /* 0x134 */ int mNetErrorFrom;     // NET_ERROR_FROM_*
        /* 0x138 */ int mVillagerPage;     // selectVillagers: the first
                                           // villager shown
        /* 0x13C */ dAnimal_c *mpVillager; // the complaint's villager
        /* 0x140 */ int mRoom;             // ROOM_*
    }; // size 0x144

    dAcNpcSpPerikoBase_c() : mClapTimer(0), mWindowTalk(this) {
    }
    virtual ~dAcNpcSpPerikoBase_c() {
    } // weak
    virtual int create();                          // 0x5B8
    virtual int preCreate();                       // 0x37C
    virtual int doDelete();                        // 0xA0C
    virtual bool canInteract(dDemoActor_c *actor); // 0xAB0
    virtual void onDemoCmd(int cmd, u8 member);    // 0xBB0
    virtual void vt80();                           // 0x25A4: the partner of
                                                   // a three-way talk
    virtual void vt84();                           // 0x2700: its end
    virtual int getEarType();                      // 0x6570
    virtual int vtB0();                            // 0xA5C

    BOOL isPelly() const; // 0x135C

    // ---- modes ----
    void initWait();          // 0x136C
    void executeWait();       // 0x1380
    void initMove();          // 0x1644
    BOOL isMoveDone();        // 0x1738
    void executeMove();       // 0x17CC
    void initTurnBack();      // 0x1BAC
    void executeTurnBack();   // 0x1BF4
    void initDemoWait();      // 0x1D68
    void executeDemoWait();   // 0x1D6C
    void initTalk();          // 0x1D70
    void executeTalk();       // 0x1D80
    void initRemote();        // 0x1DC0
    void executeRemote();     // 0x1DC4
    void initRemoteTalk();    // 0x2098
    void executeRemoteTalk(); // 0x209C
    void initWaitClap();      // 0x2218
    void executeWaitClap();   // 0x2224
    void initClap();          // 0x23A0
    void executeClap();       // 0x2408
    void initPartner();       // 0x2880
    void executePartner();    // 0x2884

    BOOL checkWindowChange();            // 0x2888: the player went to
                                         // the other window
    int getWindowAt(mVec3_c *pos) const; // 0x2914: Window_e

    BOOL isDramaDue(); // 0x655C
    void clearDrama(); // 0x6564

    /* 0x1C7C dState::mode_c<dAcNpcSpPerikoBase_c> */
    /* 0x1CA0 */ u32 mClapTimer;
    /* 0x1CA4 */ int mWindow; // Window_e
    /* 0x1CA8 */ int mType;   // Type_e
    /* 0x1CAC */ talk_c mWindowTalk;
    /* 0x1DF0 */ u8 mTypeChosen; // preCreate: mType and mWindow are set
    /* 0x1DF4 */ f32 mLookRange;
    /* 0x1DF8 */ u8 mDramaDue; // Pelly sighs (Gracie story step 2)
}; // size 0x1DFC

// Pelly (RTTI "dAcNpcSpPeriko_c", profile NPC_SP_PERIKO). The destructors of
// both derived classes are implicit: the compiler generates them ahead of the
// base and talk_c ones in the weak tail.
class dAcNpcSpPeriko_c : public dAcNpcSpPerikoBase_c {
  public:
    dAcNpcSpPeriko_c() {
    }
};

// Phyllis (RTTI "dAcNpcSpPerimi_c", profile NPC_SP_PERIMI).
class dAcNpcSpPerimi_c : public dAcNpcSpPerikoBase_c {
  public:
    dAcNpcSpPerimi_c() {
    }
};

#endif
