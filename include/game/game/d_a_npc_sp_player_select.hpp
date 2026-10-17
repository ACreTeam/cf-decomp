#ifndef D_A_NPC_SP_PLAYER_SELECT_HPP
#define D_A_NPC_SP_PLAYER_SELECT_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_state.hpp>
#include <game/game/d_sv_step.hpp>

// The player select npc (REL d_a_npc_sp_player_selectNP, profile
// A_NPC_SP_PLAYER_SELECT): Rover at the start of the game. Runs the first demo
// (walks in and turns to the camera), then talks: chooses or creates the
// player, deletes players or the save, moves a Wild World character from the DS
// and handles players returning from / leaving for a DS trip. Class names from
// RTTI ("dAcNpcSpPlayerSelect_c",
// "...::stepFirstDemo_c", "...::talk_c" and its five steps); mode and step
// names from the original CP932 state labels; everything else is named from
// behaviour.
class dAcNpcSpPlayerSelect_c : public dAcNpcSp_c,
                               public dState::mode_c<dAcNpcSpPlayerSelect_c> {
  public:
    typedef dState::base_c<dAcNpcSpPlayerSelect_c>::Method Method;

    class talk_c : public dAcNpcSp_c::receptSp_c, public dMsgRcptExp_c<talk_c> {
      public:
        typedef dState::base_c<talk_c>::Method Method;

        // sys_MESS/SYS_Start message codes (l_msgLabel), named from the English
        // text.
        enum {
            MSG_HELLO = 0x01, // "Came to play, huh?" (a save exists)
            MSG_START_WITH_TIME =
                0x02,                 // the start menu, with the date and time
            MSG_START = 0x03,         // "Wanna get started?"
            MSG_PLAYER_SELECT = 0x04, // "You are...?"
            MSG_START_AGAIN = 0x05,   // "So, what do you want to do?"
            MSG_PREPARATIONS = 0x06,  // "Before playing..." (openTitleMenu)
            MSG_NEW_PLAYER = 0x0A,    // start fresh or move from the DS
            MSG_VOICE = 0x0E,         // Animalese / Bebebese / Shhhh!
            MSG_VOICE_CONFIRM = 0x0F,
            MSG_START_FRESH = 0x11,        // then the new player's save
            MSG_NEW_PLAYER_SAVED = 0x12,   // after the save: Rover leaves
            MSG_KANA = 0x13,               // hiragana only or kanji (Japanese)
            MSG_KEY_INPUT = 0x16,          // keyboard or cell-phone style
            MSG_TUTORIAL = 0x19,           // explanations over menu icons
            MSG_DELETE_AWAY_PLAYER = 0x1D, // the player is out on the DS
            MSG_DELETE_LAST_PLAYER = 0x1F, // the only house: erases the town
            MSG_DELETE_PLAYER_SELECT = 0x20, // "Whose house ...?"
            MSG_DELETE_PLAYER_CONFIRM = 0x22,
            MSG_DELETE_CANCELLED = 0x23,
            MSG_PLAYER_DELETED = 0x25,
            MSG_TOWN_DELETED = 0x2A,
            MSG_DS_PLAYER_LIVES_HERE = 0x2D,        // the DS town's only player
            MSG_DS_PLAYER_LIVES_HERE_SELECT = 0x2E, // "Pick other player."
            MSG_GREET_FIRST_IN_TOWN = 0x33, // SAVE_FLAG_ROVER_GREETED not set
            MSG_GREET_FIRST = 0x34, // PRIVATE_FLAG0_ROVER_GREETED not set
            MSG_GREET_BIRTHDAY = 0x35,
            MSG_GREET_MONTH = 0x39, // 30 days or more since the last play
            MSG_GREET_WEEK = 0x3A,  // 7 days or more
            MSG_GREET_DAYS = 0x3B,  // 2 days or more
            MSG_GREET_DAY = 0x3C,   // 1 day
            MSG_GREET_TODAY = 0x3D,
            MSG_GREET_CLOCK_BACK = 0x47,    // the clock went back
            MSG_GREET_LEAP_BIRTHDAY = 0x4B, // born on Feb 29, not a leap year
            MSG_AWAY_PLAYER = 0x4F, // the player is out on the DS: play anyway?
            MSG_SAVE_DAYS_PASSED = 0x51,
            MSG_SAVE_TODAY = 0x52,
            MSG_WC24_EXPLAIN = 0x5B,
            MSG_WC24_ON = 0x5E,
            MSG_WC24_OFF = 0x5F,
            MSG_WC24_ON_AGAIN = 0x60, // after MSG_WC24_EXPLAIN
            MSG_WC24_OFF_AGAIN = 0x61,
            MSG_SAVE_AWAY_PLAYER = 0x63, // "With that out of the way..."
            MSG_SAVING = 0x65,
            MSG_DELETING_PLAYER = 0x66,
            MSG_DELETING_TOWN = 0x67,
            MSG_DS_COMMUNICATING = 0x6A,
            MSG_DELETING_TOWN_AND_HOUSES = 0x6D,
            MSG_DS_SEARCHING = 0x6F,
            MSG_DS_RECEIVING = 0x70,
            MSG_DS_REFUSING = 0x71,
            MSG_DS_CONNECT = 0x79,        // "make sure the Nintendo DS ..."
            MSG_DS_RETURN_CONFIRM = 0x7A, // a player of this town: "I'm back!"
            MSG_DS_VISIT_CONFIRM = 0x7B,  // a visitor from another town
            MSG_DS_WRONG_PLAYER = 0x7C,
            MSG_DS_VISIT_WELCOME = 0x7D,
            MSG_DS_RETURN_WELCOME = 0x7E,
            MSG_DS_RETURN_ALREADY_BACK = 0x7F, // an older copy of the player
            MSG_DS_RETURN_NOT_AWAY = 0x80,
            MSG_DS_REFUSE_ERROR = 0x81,
            MSG_DS_CONNECT_AGAIN = 0x82,
            MSG_DS_TURN_OFF = 0x83,
            MSG_DS_NOT_FOUND = 0x84,
            MSG_DS_ERROR_RETRY = 0x85,
            MSG_DS_ERROR_CONTINUE = 0x86,
            MSG_DS_RETURN_LEFT_TOWN = 0x87, // the player moved out
            MSG_DS_QUIT = 0x88,
            MSG_FIRST_MEETING = 0x97, // no save yet
            MSG_START_FRESH_GO = 0x98,
            MSG_START_OVER = 0x9A,
            MSG_DS_EMIGRATE_START = 0x9B,  // "Download the Moving Van"
            MSG_DS_EMIGRATE_SELECT = 0x9C, // "So, who's going to visit?"
            MSG_DS_EMIGRATE_CONFIRM = 0x9D,
            MSG_DS_EMIGRATE_QUIT = 0x9E,
            MSG_DS_EMIGRATE_QUIT_NEW = 0x9F, // no save yet
            MSG_DS_EMIGRATE_OTHER_TOWN = 0xA0,
            MSG_DS_EMIGRATE_SAVED = 0xA1, // after the save: Rover leaves
            MSG_DS_EMIGRATE_NOT_FOUND = 0xA2,
            MSG_DS_NO_TOWN = 0xA4,
            MSG_DS_TOWN_CORRUPT = 0xA5,
            MSG_DS_NO_PLAYERS = 0xA6,
            MSG_DS_NO_CARD = 0xA7,
            MSG_DS_WRONG_CARD = 0xA8,
            MSG_DS_CARD_ERROR = 0xA9,
            MSG_DS_EMIGRATE_DONE = 0xAA,
            MSG_DS_CANCEL_HINT = 0xAB,
            MSG_DS_EMIGRATE_QUIT_SAVED = 0xB6,
        };

        // dSelect::Comp_c::setSysMessage: the choices (sys_SELECT/SYS_Select).
        enum {
            SEL_MSG_NEW_PLAYER = 0x01, // "I'm new!"
            SEL_MSG_NEVER_MIND = 0x02,
            SEL_MSG_NEVER_MIND_START = 0x05,
            SEL_MSG_SOUND = 0x13,
            SEL_MSG_TEXT = 0x14,
            SEL_MSG_MENU = 0x15,
            SEL_MSG_WC24 = 0x16,
            SEL_MSG_NEVER_MIND_PREPARATIONS = 0x17,
        };

        // onMessageEnd kinds (the BMG entries' end request). 16..20 are the
        // talk's own requests (dAcNpcNml_c's reqProc0..4).
        enum {
            END_REQ_TIME_MENU = 11,      // reqMenu06
            END_REQ_SYSTEM_SPEAKER = 16, // before a system message
            END_REQ_GO_HOME = 17,        // the welcome back greetings
            END_REQ_NEW_PLAYER = 18,
            END_REQ_TITLE = 19, // the town was erased
            END_REQ_NONE = 20,
        };

        // mEndAction: where onTalkEnd goes.
        enum {
            END_ACTION_DS_VISIT,   // the DS visitor's checkpoint scene
            END_ACTION_DS_RETURN,  // exit 4 (the player is back from the DS)
            END_ACTION_GO_HOME,    // the current player's house
            END_ACTION_NEW_PLAYER, // the bus (player creation)
            END_ACTION_TITLE,
        };

        // mSaveKind: what startSave does before saving.
        enum {
            SAVE_KIND_NEW_PLAYER,  // save only
            SAVE_KIND_DAYS_PASSED, // WC24 start, growth, save
            SAVE_KIND_TODAY,       // as SAVE_KIND_DAYS_PASSED, played today
            SAVE_KIND_AWAY_PLAYER, // as SAVE_KIND_DAYS_PASSED, out on the DS
            SAVE_KIND_DS_RETURN,   // as SAVE_KIND_DAYS_PASSED
            SAVE_KIND_DS_VISIT,    // growth, save
            SAVE_KIND_DS_EMIGRATE, // save only
        };

        // Saves the game (DL box, then the whole save).
        class stepSave_c : public dSvStep_c<talk_c, stepSave_c> {
          public:
            stepSave_c() {
            }
            virtual ~stepSave_c() {
            }
            void finish(); // 0x4EC8
        }; // size 0x30

        // Deletes the DL box and the save file.
        class stepDelete_c : public dSvStep_c<talk_c, stepDelete_c> {
          public:
            stepDelete_c() : mTimer(0) {
            }
            virtual ~stepDelete_c() {
            }
            void initDelete(); // 0x52B4
            void finish();     // 0x5394

            /* 0x30 */ u32 mTimer; // 300 frames: finish waits for it (the
                                   // message stays up at least this long)
        }; // size 0x34

        // Deletes one player, then saves.
        class stepDeletePlayer_c
            : public dSvStep_c<talk_c, stepDeletePlayer_c> {
          public:
            stepDeletePlayer_c() : mTimer(0) {
            }
            virtual ~stepDeletePlayer_c() {
            }
            void initDelete(); // 0x575C
            void finish();     // 0x5840

            /* 0x30 */ u32 mTimer; // 300 frames: finish waits for it (the
                                   // message stays up at least this long)
        }; // size 0x34

        // Moves a Wild World character from the DS (DS download play).
        class stepDSEmigrate_c
            : public dState::step_c<talk_c, stepDSEmigrate_c> {
          public:
            stepDSEmigrate_c() : mTimer(0) {
            }
            virtual ~stepDSEmigrate_c() {
            }
            void sendApp();     // 0x5B18
            void connect();     // 0x5BE0
            void download();    // 0x5DA8
            void endTransfer(); // 0x5EDC: cancel the transfer and go to finish
            void finish();      // 0x5F2C

            /* 0x30 */ u32 mTimer; // connect: the time left to wait for the DS
        }; // size 0x34

        // Waits for a DS carrying a travelling player (outing to / from the
        // DS).
        class stepSearchDSCarrier_c
            : public dState::step_c<talk_c, stepSearchDSCarrier_c> {
          public:
            stepSearchDSCarrier_c() : mTimer(0) {
            }
            virtual ~stepSearchDSCarrier_c() {
            }
            void startListen(); // 0x61F4
            void connect();     // 0x62C8
            void download();    // 0x6768

            /* 0x30 */ u32 mTimer; // connect: the time left to wait for the DS
        }; // size 0x34

        talk_c(dAcNpcSpPlayerSelect_c *npc); // 0x114C
        virtual ~talk_c() {
        } // 0x896C
        virtual void onMessageStart(int arg);     // 0x1428
        virtual void onMessageEnd(int kind);      // 0x17B0
        virtual void onAnswer(int answer);        // 0x37A8
        virtual void init();                      // 0x12CC
        virtual int getForcedVoiceMode();         // 0x13D0
        virtual void preExecute();                // 0x127C
        virtual void onTalkEnd();                 // 0x4138
        virtual void getMsgInfo(msgInfo_s *info); // 0x1414
        virtual void onRequestEnd(int kind);      // 0x7830

        void start(dActor_c *target); // 0x1328

        // ---- message handlers (onMessageStart / onMessageEnd / onAnswer
        // tables; code = SYS_Start message) ----
        void disableHomeAndReset(); // 0x14F4 (start 0x65): saving
        void startDSEmigrate();     // 0x1528 (start 0x9B)
        void setTimeWord();         // 0x1684 (start 0x02): word 0 = am/pm now
        void setFirstPlayerName();  // 0x1730 (start 0x1F)
        void finishTravelReturn();  // 0x1798 (start 0x7E)
        void finishTravelVisit();   // 0x17A4 (start 0x7D)
        void returnFromNetError();  // 0x1980 (end NetworkError 0x15 0x16 0x1A
                                    // 0x1B 0x1D 0x1F)
        void returnFromNetError2(); // 0x19C0 (end NetworkError 0x18 0x1C)
        void holdNetError();        // 0x1A00 (end NetworkError 0x17 0x1E)
        void returnFromNetError3(); // 0x1A10 (end NetworkError 0x13 0x19)
        void askNetSetting();       // 0x1A50 (end 0x5B)
        void openTitleMenu();       // 0x1AB8 (end 0x06)
        void openPlayerSelect();    // 0x1BF4 (end 0x04)
        void openDeletePlayerSelect(); // 0x1D40 (end 0x20)
        void startSave();              // 0x1E18 (end 0x65): by mSaveKind
        void startFileDelete();        // 0x22AC (end 0x67 0x6D)
        void startPlayerDelete();      // 0x2420 (end 0x66)
        void finishNewPlayerKana();    // 0x2594 (end 0x11)
        void setSaveKindTravel();      // 0x25CC (end 0x4F)
        void
        createDsPlayer(); // 0x25D8 (end 0xA1): the Wild World player mDsPlayer
        void createDefaultPlayer();   // 0x2754 (end 0x12 0x98): "プレイヤN"
        void checkDsSave();           // 0x27DC (end 0xAA)
        void checkDsConnected();      // 0x2820 (end 0xAB)
        void checkDsEmigrateEnd();    // 0x28DC (end 0x6A)
        void openDsPlayerSelect();    // 0x2980 (end 0x9C)
        void returnToStart();         // 0x2A60 (end 0xA0 0xA4..0xA9)
        void startSearchDS();         // 0x2AC4 (end 0x6F)
        void holdSearchDS();          // 0x2C34 (end 0x70)
        void startRefuseCheckDS();    // 0x2CA4 (end 0x71)
        void startAcceptReturn();     // 0x2E14 (end 0x7E)
        void startAcceptVisit();      // 0x30DC (end 0x7D)
        void startRefuseDS();         // 0x33A4 (end 0x7F 0x80 0x87)
        void switchToSystemSpeaker(); // 0x3650: Rover speaking ->
                                      // hideRoverSpeaker after request 1
        void switchToRoverSpeaker();  // 0x36FC: not Rover -> showRoverSpeaker
                                      // after request 1
        void selectNetSetting();      // 0x3924 (answer 0x06/3)
        void saveDsPlayer();          // 0x39A0 (answer 0x9D/0)
        void selectPlayer();          // 0x39F4 (answer 0x04/0..3)
        void selectDeletePlayer();    // 0x3A98 (answer 0x20/0..3)
        void selectDsPlayer();        // 0x3B04 (answer 0x9C/0..3)
        void cancelDsPlayer();        // 0x3C28 (answer 0x9E/2)
        void retryDsPlayer();         // 0x3C2C (answer 0x9E/0)
        void
        answerSystemSpeaker();    // 0x3C98 (answer 0x4F 0x79 0x88 0x7C 0x82 /0)
        void selectVoiceMode0();  // 0x3C9C (answer 0x0E/0)
        void selectVoiceMode1();  // 0x3CE4 (answer 0x0E/1)
        void selectVoiceMode2();  // 0x3D2C (answer 0x0E/2)
        void cancelVoiceMode();   // 0x3D74 (answer 0x0F/1)
        void decideVoiceMode();   // 0x3D80 (answer 0x0F/0)
        void setKanaOff();        // 0x3DB8 (answer 0x13/1)
        void setKanaOn();         // 0x3DC0 (answer 0x13/0)
        void setKeySeOff();       // 0x3DC8 (answer 0x16/0)
        void setKeySeOn();        // 0x3DD0 (answer 0x16/1)
        void setTutorialOn();     // 0x3DD8 (answer 0x19/0)
        void setTutorialOff();    // 0x3DE0 (answer 0x19/1)
        void enableNet();         // 0x3DE8 (answer 0x5E..0x61/0)
        void disableNet();        // 0x3EC0 (answer 0x5E..0x61/1)
        void checkTravelPlayer(); // 0x3EC4 (answer 0x7A/0)
        void acceptTravel();      // 0x3F08 (answer 0x7B/0)
        void refuseTravel();      // 0x3F0C (answer 0x7A 0x7B /1)
        void checkTravelReturn(); // 0x40BC (answer 0x81/1)
        int getTravelPlayerMsg(); // 0x43C8: 0x7E/0x7F/0x80
        int
        getDsSaveMsg(); // 0x4444: sets mDsPlayer when the save has one player
        int getGreetingMsg(dPrivateData_c *player); // 0x45C0

        // ---- modes (l_modes) ----
        void initWait();             // 0x470C
        void executeWait();          // 0x4710
        void failWithNandError();    // 0x4714
        void initWC24Start();        // 0x48A0
        void executeWC24Start();     // 0x4A08
        void initGrowth();           // 0x4B94
        void executeGrowth();        // 0x4BA0
        void initSave();             // 0x4D14
        void executeSave();          // 0x4E80
        void initFileDelete();       // 0x5080
        void executeFileDelete();    // 0x5258
        void initPlayerDelete();     // 0x5528
        void executePlayerDelete();  // 0x5700
        void initDSEmigrate();       // 0x59D4
        void executeDSEmigrate();    // 0x5AD0
        void initSearchDS();         // 0x60D4
        void executeSearchDS();      // 0x61AC
        void initAcceptDS();         // 0x6DF0
        void executeAcceptDS();      // 0x6DFC
        void initRefuseDS();         // 0x70C0
        void executeRefuseDS();      // 0x70CC
        void initRefuseCheckDS();    // 0x7370
        void executeRefuseCheckDS(); // 0x737C
        void initEndDS();            // 0x7650
        void executeEndDS();         // 0x7670

        void hideRoverSpeaker(); // 0x78C8: request-end method of
                                 // switchToSystemSpeaker
        void showRoverSpeaker(); // 0x791C: request-end method of
                                 // switchToRoverSpeaker
        void requestMessage(u16 code, const char *label); // 0x79BC

        /* 0x0EC dMsgRcptExp_c<talk_c> */
        /* 0x128 */ stepSave_c mStepSave;
        /* 0x158 */ stepDelete_c mStepDelete;
        /* 0x18C */ stepDeletePlayer_c mStepDeletePlayer;
        /* 0x1C0 */ stepDSEmigrate_c mStepDSEmigrate;
        /* 0x1F4 */ stepSearchDSCarrier_c mStepSearchDSCarrier;
        /* 0x228 */ u32 _228;
        /* 0x22C */ int
            mPlayerIndex; // the player to delete (selectDeletePlayer)
        /* 0x230 */ int
            mDsPlayer; // the Wild World player (dDsPlayerData index)
        /* 0x234 */ const char *mMsgLabel; // getMsgInfo
        /* 0x238 */ u16 mMsgCode;
        /* 0x23C */ dAcNpcSpPlayerSelect_c *mpOwner;
        /* 0x240 */ int mEndAction; // END_ACTION_*: onTalkEnd
        /* 0x244 */ int mSaveKind;  // SAVE_KIND_*: startSave
        /* 0x248 */ int mVoiceMode; // VOICE_MODE_*: getForcedVoiceMode
        /* 0x24C */ int _24C;
    }; // size 0x250

    // The first demo: wait for the fade, walk in to the home position and turn
    // to the front.
    class stepFirstDemo_c
        : public dState::step_c<dAcNpcSpPlayerSelect_c, stepFirstDemo_c> {
      public:
        stepFirstDemo_c() : _30(0) {
        }
        virtual ~stepFirstDemo_c() {
        }
        void waitFade();      // 0xBCC
        void initWalk();      // 0xCB0
        void waitWalk();      // 0xD80
        void initTurnFront(); // 0xE34
        void waitTurnFront(); // 0xEFC
        void finish();        // 0xFB0

        /* 0x30 */ u32 _30;
    }; // size 0x34

    dAcNpcSpPlayerSelect_c() : mTalkPs(this), mTimer(0) {
    }
    virtual ~dAcNpcSpPlayerSelect_c() {
    } // 0x8B04
    virtual int create();                          // 0x2B0
    virtual int preCreate();                       // 0x264
    virtual int doDelete();                        // 0x660
    virtual bool canInteract(dDemoActor_c *actor); // 0x84C
    virtual int getEarType();                      // 0x7B1C
    virtual int getSoundId();                      // 0x888
    virtual int vtB0();                            // 0x754
    f32 calcFade(f32 dist, f32 start,
                 f32 end); // 0x718: 1 up to start, 0 from end

    void initTalkWait();     // 0x894
    void executeTalkWait();  // 0x8A0
    void initFirstDemo();    // 0xA50
    void executeFirstDemo(); // 0xB84
    void initDemoWait();     // 0x1128
    void executeDemoWait();  // 0x112C
    void initTalk();         // 0x1130
    void executeTalk();      // 0x1140
    void initNone();         // 0x1144
    void executeNone();      // 0x1148

    void setModelColor(u8 color);                       // 0x7A04
    void setMatColor(u8 color, nw4r::g3d::ResMdl *mdl); // 0x7A60

    /* 0x1C7C dState::mode_c<dAcNpcSpPlayerSelect_c> */
    /* 0x1CA0 */ stepFirstDemo_c mStepFirstDemo;
    /* 0x1CD4 */ u8 mSaveOnDelete; // doDelete: run the save process first
    /* 0x1CD5 */ u8 mColor;
    /* 0x1CD8 */ talk_c mTalkPs;
    /* 0x1F28 */ mVec3_c mHomePos;
    /* 0x1F34 */ u8 mDsDlActive;
    /* 0x1F38 */ int mTimer;
}; // size 0x1F3C

#endif
