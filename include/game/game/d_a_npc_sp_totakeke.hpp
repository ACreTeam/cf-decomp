#ifndef D_A_NPC_SP_TOTAKEKE_HPP
#define D_A_NPC_SP_TOTAKEKE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>

// K.K. Slider at the Roost (REL d_a_npc_sp_totakekeNP, profile
// A_NPC_SP_TOTAKEKE). On Saturday nights (dEvent visitor 3) he sits on the
// stage of the Roost: from 20:00 he takes song requests (a mood, his pick or a
// title typed in), plays them (dBgm::StgCafe_c) for every player in the room,
// moves the stage lights of the mirror ball and hands out a copy of the song.
// The song itself is kept in dNpcInfo::totakeke_c, which is synchronised over
// the net; the playing state is in the daub data.
//
// Class names from RTTI ("dAcNpcSpTotakeke_c", "dAcNpcSpTotakeke_c::talk_c",
// "dAcNpcSpTotakeke_c::dTotakekeDebugPage_c"); everything else is named from
// behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dTotakekeDebugPage_c), so dTotakekeDebugPage_c is declared first.
class dAcNpcSpTotakeke_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,        // on the stage; speaks to the requester
        STATE_LOOK,        // (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // mStep of STATE_TALK (l_talkSteps).
    enum TalkStep_e {
        TALK_STEP_WAIT, // wait for the talk, finish it
        TALK_STEP_DONE
    };

    typedef void (dAcNpcSpTotakeke_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dTotakekeDebugPage_c : public dDebugPage_c {};

    // The message receptor: the greetings, the song request and the talk
    // around the song.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_totakeke (from the ENG text).
        enum {
            MSG_TOO_EARLY = 0x01,     // "...a live gig at 8:00 tonight..."
            MSG_FIRST_MEETING = 0x02, // "...the perfect song, just for you."
            MSG_FIRST_PLAY = 0x04,    // "Curl up in a chair..." (yes)
            MSG_FIRST_SONG = 0x06,    // "...sending this little ditty out..."
            MSG_FIRST_GIFT =
                0x07, // "I tucked a boot of my gig in your pockets"
            MSG_FIRST_NO_ROOM = 0x08,  // "Cool, thanks. Yeah. All right."
            MSG_COME_AGAIN = 0x09,     // "...every Saturday..."
            MSG_ASK_SONG = 0x0A,       // "You here for a song?"
            MSG_ASK_SONG_AGAIN = 0x0B, // "You again, huh? ...after a song?"
            MSG_NAME_SONG = 0x0D,      // "Name your lick..." (the title input)
            MSG_ASK_MOOD = 0x0E,       // "How ya feelin'?" (five moods)
            MSG_PICK = 0x11,           // "I got you covered, kitty cat."
            MSG_TITLE_OK = 0x12,       // "Mellow. ...Pull up a chair."
            MSG_TITLE_CANCEL = 0x13,   // "You're the chief, chief..."
            MSG_PICK_SONG = 0x15,    // "...dig the riffs on this ditty: <song>"
            MSG_TITLE_SONG = 0x16,   // "Here's a little ditty for <player>..."
            MSG_MOOD_SONG = 0x17,    // 0x17..0x1B: by the mood (REQUEST_*)
            MSG_GIFT = 0x1C,         // "...I laid a boot on you for <song>."
            MSG_GIFT_NO_ROOM = 0x1D, // "Why don't I slip you a boot..."
            MSG_UNKNOWN_TITLE = 0x1E,          // "...I don't know <title>."
            MSG_UNKNOWN_TITLE_AUDIENCE = 0x1F, // "...isn't in my set list..."
            MSG_VISITOR_THANKS = 0x20, // "I'll be rolling by your town, too"
            MSG_VISITOR_NO_GIFT =
                0x21, // "...I didn't burn enough to go around."
            MSG_AUDIENCE_THANKS = 0x22, // "You're a real gone crowd."
            MSG_GIFT_GIVEN = 0x23,      // "Cool. You're the most."
            MSG_DONE_TODAY = 0x24,      // "I'm all jammed out for today..."
            MSG_ENCORE = 0x25,          // "...enough in me for one more song."
            MSG_VISITOR_MEET =
                0x26, // "You didn't follow me all the way out here"
            MSG_VISITOR_ASK = 0x27, // "You ready to flip to my axe?"
            MSG_WORKING = 0x28,     // "You look all business and no fun." (job)
            MSG_WORKING_AGAIN = 0x29,         // "...until 12 chimes tonight."
            MSG_FIRST_MEETING_AGAIN = 0x2A,   // "Don't be a square, man..."
            MSG_VISITOR_FIRST_MEETING = 0x2B, // "...a little secret of mine..."
            MSG_FIRST_MEETING_REQUESTED =
                0x2C,                // "...I'd dig it if you sat back"
            MSG_SECRET_SONG = 0x2D,  // "...belting this number out to you"
            MSG_SECRET_HEARD = 0x2E, // "<song> is one of my secret numbers!"
        };
        // The kinds of talk (mKind).
        enum Kind_e {
            KIND_TALK, // a plain talk
            KIND_SONG  // he speaks to the player before playing the song
        };

        talk_c(dAcNpcSpTotakeke_c *npc);          // 0x1874
        virtual ~talk_c();                        // 0x18DC
        virtual void onMessageStart(int arg);     // 0x1D10
        virtual void onMessageEnd(int kind);      // 0x1EA8
        virtual void onAnswer(int arg);           // 0x2058
        virtual void init();                      // 0x1934
        virtual void preExecute();                // 0x2154
        virtual void onTalkEnd();                 // 0x21EC
        virtual void getMsgInfo(msgInfo_s *info); // 0x1CF8
        virtual void onRequestEnd(int kind);      // 0x2260

        void start(dActor_c *target); // 0x19B8

        // ---- message / request hooks ----
        void openTitleInput(); // 0x22C4
        void onTitleInput();   // 0x231C
        void startSong();      // 0x2444
        void waitSong();       // 0x246C
        void sitDown();        // 0x24E0
        void startSong2();     // 0x2538
        void afterSong();      // 0x253C

        /* 0x0EA */ u16 mMsgCode; // the first message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpTotakeke_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ Proc mOnPreExecute; // every frame (preExecute)
        /* 0x114 */ int mKind;          // Kind_e
        /* 0x118 */ u8 mNoTalked;       // onTalkEnd does not count the talk
    }; // size 0x11C

    dAcNpcSpTotakeke_c() : mTotakekeTalk(this) {
    }
    virtual ~dAcNpcSpTotakeke_c() {
    } // weak 0x28BC
    virtual void onDemoCmd(int cmd, u8 member);    // 0x1E0
    void changeState(int state);                   // 0x460
    virtual int preCreate();                       // 0x498
    virtual int create();                          // 0x518
    virtual int doDelete();                        // 0x710
    virtual int vtB0();                            // 0x7A0
    virtual bool canInteract(dDemoActor_c *actor); // 0x1024
    virtual int canSpeak(dDemoActor_c *actor);     // 0x109C
    virtual int getEarType();                      // 0x27C0
    void startShow(u8 listen);                     // 0x10A4
    void playSong(u8 song, int pos);               // 0x111C
    void stopShow();                               // 0x121C
    void resetShow();                              // 0x12D0
    void initIdle();                               // 0x1300
    void executeIdle();                            // 0x1304
    void initLook();                               // 0x1438
    void executeLook();                            // 0x143C
    void initTalk();                               // 0x1440
    void stepTalk();                               // 0x1480
    void executeTalk();                            // 0x14F0
    void initRemote();                             // 0x1530
    void executeRemote();                          // 0x1534
    void initRemoteTalk();                         // 0x16E8
    void executeRemoteTalk();                      // 0x16EC
    BOOL isOnStage();                              // 0x1850
    void checkSongRequest(); // inline (executeIdle / executeRemote / ...)

    // The net daub data (getDaubData / setDaubData).
    struct daub_s {
        u32 mActive : 1;  // his evening (dEvent visitor 3): he is out
        u32 mPlaying : 1; // a song is playing
        u32 mSong : 8;    // its item index + 1
        u32 _0 : 22;
        u32 mPos; // its play position (dBgm::StgCafe_c::fn_80079F30)
    }; // size 0x8

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dTotakekeDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep;
    /* 0x1C90 */ talk_c mTotakekeTalk;
    /* 0x1DAC */ daub_s mDaub;
    /* 0x1DB4 */ int mStartTimer; // frames until the song starts (startShow)
    /* 0x1DB8 */ int mEndTimer;   // frames from the song's end to stopShow
    /* 0x1DBC */ u8 mShowOn;      // startShow .. resetShow
    /* 0x1DBD */ u8 mSongOn;      // playSong .. stopShow
    /* 0x1DBE */ u8 mListen;      // the player sits in the audience
    /* 0x1DBF */ u8 mStaffroll;   // the staff roll runs with the song
    /* 0x1DC0 */ u8 mEndingReq;   // start the staff roll's ending
    /* 0x1DC1 */ u8 mTelopLocked; // dObjTelop_c::lock while listening
}; // size 0x1DC4

#endif
