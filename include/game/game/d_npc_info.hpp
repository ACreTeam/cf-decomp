#pragma once

// d_npc_info.cpp: the special NPCs' event state, one object per NPC REL. Players from this town keep most of it
// in their dPrivateData_c flags; visitors use the object's copy. Names are inferred.

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_private_data.hpp>

class mVec3_c;

void fn_800F59C8(); // 800F59C8
int fn_800F59CC();  // 800F59CC
int fn_800F59D4();  // 800F59D4

namespace dNpcInfo {

int getJingleHomeChance(); // 800F59DC
int getWalkChance();       // 800F59EC
int getWaitBase();         // 800F59FC
int getWaitRand();         // 800F5A0C

void init();  // 800F5A1C
void reset(); // 800F5AE8

struct param_c {
    param_c()
        : _04(2), _08(-1), _0C(-1), _10(0), mJingleHomeChance(50), mWalkChance(60), mWaitBase(120), mWaitRand(600) {}

    /* 0x00 */ u8 _00[4];
    /* 0x04 */ u8 _04;
    /* 0x08 */ int _08;
    /* 0x0C */ int _0C;
    /* 0x10 */ int _10;
    /* 0x14 */ int mJingleHomeChance;
    /* 0x18 */ int mWalkChance;
    /* 0x1C */ int mWaitBase;
    /* 0x20 */ int mWaitRand;
}; // size 0x24

// Tom Nook in the opening (d_a_npc_sp_op_tanukichi).
class opTanukichi_c {
public:
    ~opTanukichi_c() {}
    static opTanukichi_c *get(); // 800F5B74
    void clear();                // 800F5B7C

    /* 0x0 */ u8 mViewed[4]; // by dHomeList_c house index
}; // size 0x4

// Player select / creation (d_a_npc_sp_player_select, d_a_npc_sp_player_create).
class playerCreate_c {
public:
    ~playerCreate_c() {}
    static playerCreate_c *get(); // 800F5B94
    void clear();                 // 800F5B9C

    /* 0x0 */ u8 mFromDs;
}; // size 0x1

// Tom Nook in his store (d_a_npc_sp_tanukichi).
class tanukichi_c {
public:
    ~tanukichi_c() {}
    static tanukichi_c *get();       // 800F5BA8
    void clear();                    // 800F5BB0
    static BOOL isAllVillagersMet(); // 800F5BC4

    /* 0x0 */ u8 mJobDone;
    /* 0x1 */ u8 mTwinsGreeted;
    /* 0x2 */ u8 mTimeSpecialTold;
}; // size 0x3

// Jingle on Toy Day (d_a_npc_sp_jingle). See dItemPairRing_c.
class jingle_c {
public:
    ~jingle_c() {}
    static jingle_c *get(); // 800F5C88
    void clear();           // 800F5C94

    bool isMet();                                    // 800F5C98
    bool isGiven();                                  // 800F5CFC
    bool isTalked();                                 // 800F5D60
    bool isGivenHere();                              // 800F5DC4
    int getCount();                                  // 800F5E28
    bool isMoved();                                  // 800F5E74
    void meet();                                     // 800F5ED8
    void push(dItem::Item hat, dItem::Item acc);     // 800F5F24
    void incCount();                                 // 800F5FB8
    BOOL contains(dItem::Item hat, dItem::Item acc); // 800F6004
    void move();                                     // 800F6098
    void setSpot();                                  // 800F60E4
    bool isFirstGift();                              // 800F62F4
    bool isRefusedNewLook();                         // 800F6358
    void setGreetedNewLook();                        // 800F63BC
    bool isGreetedNewLook();                         // 800F6410
    void setRefusedNewLook();                        // 800F6474
    void resetEncounter();                           // 800F64C8

    /* 0x00 */ dItemPairRing_c mRing;
}; // size 0x24

// Franklin on Harvest Festival (d_a_npc_sp_franklin).
class franklin_c {
public:
    ~franklin_c() {}
    static franklin_c *get(); // 800F6514
    void clear();             // 800F6520

    void setTalked();                                // 800F6550
    void setGaveKnifeFork();                         // 800F65A4
    void relocate();                                 // 800F6634
    void setSpot(const mVec3_c *center, f32 radius); // 800F66AC
    BOOL isTalked();                                 // 800F67B0

    /* 0x0 */ u8 mHandedOver;
    /* 0x1 */ u8 _1;
    /* 0x2 */ u8 mAskedForHelp;
    /* 0x3 */ u8 mThanked;
    /* 0x4 */ u8 mTalkedHere;
    /* 0x5 */ u8 mTalked;
    /* 0x6 */ u8 mRelocated;
    /* 0x7 */ u8 mGaveHere;
    /* 0x8 */ u8 mGaveKnifeFork;
    /* 0x9 */ u8 mGaveKnifeFork2;
}; // size 0xA

// Zipper on Bunny Day (d_a_npc_sp_pyontaro).
class pyontaro_c {
public:
    ~pyontaro_c() {}
    static pyontaro_c *get(); // 800F67F8
    void clear();             // 800F6804
    void resetTalks();        // 800F6818

    void setIntroduced();    // 800F6830
    void setShovelOffered(); // 800F6890
    void incFoilTalks();     // 800F68F0
    void incEggTalks();      // 800F6918
    void incEggsLeftTalks(); // 800F6940
    void incNoEggsTalks();   // 800F6968
    void incBackTalks();     // 800F6990
    int getBackTalks();      // 800F6A20
    BOOL isIntroduced();     // 800F6A84
    BOOL isShovelOffered();  // 800F6ACC

    /* 0x00 */ u8 mIntroduced;
    /* 0x01 */ u8 mShovelOffered;
    /* 0x04 */ int mFoilTalks;
    /* 0x08 */ int mEggTalks;
    /* 0x0C */ int mEggsLeftTalks;
    /* 0x10 */ int mNoEggsTalks;
    /* 0x14 */ int mBackTalks;
}; // size 0x18

// Chip at the fishing tourney (d_a_npc_sp_uomasa).
class uomasa_c {
public:
    ~uomasa_c() {}
    static uomasa_c *get(); // 800F6B14
    void clear();           // 800F6B1C

    void setJoined();            // 800F6B30
    void setTalkedBeforeStart(); // 800F6B7C
    void setResultHeard();       // 800F6BC8
    BOOL isJoined();             // 800F6C14
    BOOL isTalkedBeforeStart();  // 800F6C5C
    BOOL isResultHeard();        // 800F6CA4

    /* 0x0 */ u8 mJoined;
    /* 0x1 */ u8 mTalkedBeforeStart;
    /* 0x2 */ u8 mResultHeard;
}; // size 0x3

// Nat at the Bug-Off (d_a_npc_sp_bug).
class bug_c {
public:
    ~bug_c() {}
    static bug_c *get(); // 800F6CEC
    void clear();        // 800F6CF4

    void setJoined();            // 800F6D0C
    void setTalkedBeforeStart(); // 800F6D58
    void setResultHeard();       // 800F6DA4
    void setJudged();            // 800F6DF0
    BOOL isJoined();             // 800F6E3C
    BOOL isTalkedBeforeStart();  // 800F6E84
    BOOL isResultHeard();        // 800F6ECC
    BOOL isJudged();             // 800F6F14

    /* 0x0 */ u8 mJoined;
    /* 0x1 */ u8 mTalkedBeforeStart;
    /* 0x2 */ u8 mResultHeard;
    /* 0x3 */ u8 mJudged;
}; // size 0x4

// Jack on Halloween (d_a_npc_sp_pumpking).
class pumpking_c {
public:
    ~pumpking_c() {}
    static pumpking_c *get(); // 800F6F5C
    void clear();             // 800F6F64

    void setNoCandyTold();                           // 800F6F7C
    void setCandyGiven();                            // 800F6FC8
    void setOtherGiven();                            // 800F7014
    void setRefused();                               // 800F7060
    BOOL isNoCandyTold();                            // 800F70AC
    BOOL isCandyGiven();                             // 800F70F4
    BOOL isOtherGiven();                             // 800F713C
    BOOL isRefused();                                // 800F7184
    void setSpot(const mVec3_c *center, f32 radius); // 800F71CC

    /* 0x0 */ u8 mNoCandyTold;
    /* 0x1 */ u8 mCandyGiven;
    /* 0x2 */ u8 mOtherGiven;
    /* 0x3 */ u8 mRefused;
}; // size 0x4

// Brewster (d_a_npc_sp_cafe), for visitors.
class cafe_c {
public:
    ~cafe_c() {}
    static cafe_c *get(); // 800F72E8
    void clear();         // 800F72F0
    void setTalked() { mTalked = 1; } // d_a_npc_sp_cafe
    void setServed() { mServed = 1; } // d_a_npc_sp_cafe

    /* 0x0 */ u8 mTalked;
    /* 0x1 */ u8 mServed;
}; // size 0x2

// Tortimer at the town events (d_a_npc_sp_event_soncho). Hf: Harvest Festival.
class soncho_c {
public:
    ~soncho_c() {}
    static soncho_c *get(); // 800F7300
    void clear();           // 800F730C

    dItem::Item *getPresent();         // 800F734C
    void setPresent(const dItem::Item &item); // 800F7394
    void setTalked();                  // 800F73F0
    void setFullTold();                // 800F743C
    void setTalkedAfter();             // 800F7488
    void setPopperGiven();             // 800F74D4
    void setHfRegiveTold();            // 800F7520
    void setHfLostAsked();             // 800F756C
    void setHfGreeted();               // 800F75B8
    void setHfTalkedAfter();           // 800F7604
    void setHfFullTold();              // 800F7650
    void setHfTalkedBeforeStart();     // 800F769C
    void resetPopperGiven();           // 800F76E8
    void resetHfLostAsked();           // 800F7734
    void setGiven();                   // 800F7780
    BOOL isTalked();                   // 800F77CC
    BOOL isFullTold();                 // 800F7814
    BOOL isTalkedAfter();              // 800F785C
    BOOL isPopperGiven();              // 800F78A4
    BOOL isHfRegiveTold();             // 800F78EC
    BOOL isHfLostAsked();              // 800F7934
    BOOL isHfGreeted();                // 800F797C
    BOOL isHfTalkedAfter();            // 800F79C4
    BOOL isHfFullTold();               // 800F7A0C
    BOOL isHfTalkedBeforeStart();      // 800F7A54
    BOOL isGiven();                    // 800F7A9C
    BOOL testTalked();                 // 800F7AE4
    BOOL testFullTold();               // 800F7B28
    BOOL testTalkedAfter();            // 800F7B6C
    BOOL testPopperGiven();            // 800F7BB0
    BOOL testHfRegiveTold();           // 800F7BF4
    BOOL testHfTalkedAfter();          // 800F7C38
    BOOL testHfFullTold();             // 800F7C7C
    BOOL testHfTalkedBeforeStart();    // 800F7CC0

    /* 0x0 */ u8 mTalked;
    /* 0x1 */ u8 mFullTold;
    /* 0x2 */ u8 mTalkedAfter;
    /* 0x3 */ u8 mPopperGiven;
    /* 0x4 */ u8 mHfRegiveTold;
    /* 0x5 */ u8 mHfLostAsked;
    /* 0x6 */ u8 mHfGreeted;
    /* 0x7 */ u8 mHfTalkedAfter;
    /* 0x8 */ u8 mHfFullTold;
    /* 0x9 */ u8 mHfTalkedBeforeStart;
    /* 0xA */ u8 mGiven;
    /* 0xC */ dItem::Item mPresent;
}; // size 0xE

// Booker (d_a_npc_sp_gatekeeper_b), for visitors.
class gatekeeperB_c {
public:
    ~gatekeeperB_c() {}
    static gatekeeperB_c *get(); // 800F7D04
    void clear();                // 800F7D0C

    /* 0x0 */ u8 mGreeted;
}; // size 0x1

// Lyle at the HRA (d_a_npc_sp_honma), reset by Kapp'n's bus (d_a_npc_sp_kappei).
class kappei_c {
public:
    ~kappei_c() {}
    static kappei_c *get(); // 800F7D18
    void clear();           // 800F7D20
    void setLyleTalked(u8 talked) { mLyleTalked = talked; } // d_a_npc_sp_kappei

    /* 0x0 */ u8 mLyleTalked;
}; // size 0x1

// Mr. Resetti (d_a_npc_sp_resetsan, d_npc_mng, d_sv_proc).
class resetsan_c {
public:
    ~resetsan_c() {}
    static resetsan_c *get(); // 800F7D2C
    void init();              // 800F7D38
    void set(int visit);      // 800F7D4C
    int getVisit();           // 800F7D54
    void setSorryType(int type) { mSorryType = type; } // d_a_npc_sp_resetsan

    /* 0x0 */ int mVisit;
    /* 0x4 */ u8 _4[4];
    /* 0x8 */ int mSorryType;
}; // size 0xC

// The fountain goddess (d_a_npc_sp_goddess).
class goddess_c {
public:
    ~goddess_c() {}
    static goddess_c *get();        // 800F7D70
    void clear();                   // 800F7D78
    void setItem(const dItem::Item &item); // 800F7D88
    dItem::Item getItem();          // 800F7D94

    /* 0x0 */ dItem::Item mItem;
}; // size 0x2

// Pave at the Festivale (d_a_npc_sp_dancer).
class dancer_c {
public:
    ~dancer_c() {}
    static dancer_c *get(); // 800F7DA0
    void clear();           // 800F7DAC

    BOOL isTalked();        // 800F7DC8
    void setTalked();       // 800F7E10
    BOOL isCandyAsked();    // 800F7E5C
    void setCandyAsked();   // 800F7F20
    void resetEncounter();  // 800F7FE0
    void resetCandyHints(); // 800F7FF8
    // d_a_npc_sp_dancer talk_c::start / onMessageEnd (a separate get() each time).
    void setNoCandyTold() { mNoCandyTold = 1; }
    void setWrongCandyTold() { mWrongCandyTold = 1; }
    void setTooFewTold() { mTooFewTold = 1; }
    void setGiftGiven() { mGiftGiven = 1; }

    /* 0x0 */ u8 mTalked;
    /* 0x1 */ u8 mCandyAsked[4]; // by dSaveTown_c::mDailyRand (candy colour)
    /* 0x5 */ u8 mNoCandyTold;
    /* 0x6 */ u8 mWrongCandyTold;
    /* 0x7 */ u8 mTooFewTold;
    /* 0x8 */ u8 mGiftGiven;
}; // size 0x9

// NET_PACKET_NPC_EVENT_DATA
struct totakekePacket_c {
    /* 0x00 */ wchar_t mRequestTitle[0x11];
    /* 0x22 */ u16 mSong;
    /* 0x24 */ u8 mRequesterNo;
    /* 0x25 */ u8 mRequestKind;
    /* 0x26 */ u8 mUnknownTitle;
    /* 0x27 */ u8 mPerforming;
}; // size 0x28

// K.K. Slider at the Roost (d_a_npc_sp_totakeke), synchronised over the net.
class totakeke_c {
public:
    ~totakeke_c() {}
    // mRequestKind: what the song was picked by.
    enum RequestKind_e {
        REQUEST_GOOD,        // the mood ("How ya feelin'?"): FROM_MUSIC_GOKIGEN
        REQUEST_GRUMPY,      // FROM_MUSIC_FUKIGEN
        REQUEST_LAZY,        // FROM_MUSIC_MATTARI
        REQUEST_SAD,         // FROM_MUSIC_BLUE
        REQUEST_HARD_TO_SAY, // FROM_MUSIC_UNKNOWN
        REQUEST_PICK,        // K.K.'s pick
        REQUEST_FIRST,       // the first-meeting song
        REQUEST_TITLE,       // a title typed in
        REQUEST_NONE
    };
    static totakeke_c *get(); // 800F800C
    void clear();             // 800F8018
    void resetSong();         // 800F8074

    void setTalked();                                    // 800F80E4
    BOOL isMet();                                        // 800F8148
    BOOL isTalked();                                     // 800F8170
    void start(int kind, u16 song, const void *request); // 800F81B8
    void sendSong();                                     // 800F8554
    void stop();                                         // 800F85DC
    BOOL isRequested();                                  // 800F8690
    BOOL isPlaying();                                    // 800F86D8
    void setPlaying(BOOL on);                            // 800F8700
    BOOL isOnStage();                                    // 800F8740
    void fromPacket(const totakekePacket_c *packet);     // 800F87B8
    void toPacket(totakekePacket_c *packet);             // 800F8840
    // The requested song (weak copy in d_a_npc_sp_totakeke).
    dItem::Item getSong() const { return mSong; }
    void clearReceived() { mReceived = 0; } // d_a_npc_sp_totakeke
    void clearPlayReq() { mPlayReq = 0; }
    void setWorkWarned() { mWorkWarned = 1; }

    /* 0x00 */ u8 mTalked;
    /* 0x01 */ u8 mRequested;
    /* 0x02 */ u8 mHadAudience;
    /* 0x03 */ u8 mIsRequester;
    /* 0x04 */ u8 mWorkWarned;
    /* 0x05 */ u8 _05;
    /* 0x06 */ u8 mInAudience;
    /* 0x07 */ u8 _07;
    /* 0x08 */ u8 mAudienceNum;
    /* 0x09 */ u8 mAudienceDoneNum;
    /* 0x0A */ wchar_t mRequestTitle[0x11];
    /* 0x2C */ dItem::Item mSong;
    /* 0x30 */ int mRequesterNo;
    /* 0x34 */ int mRequestKind; // 0..4 mood, 5 K.K.'s pick, 6 first meeting, 7 title, 8 none
    /* 0x38 */ u8 mUnknownTitle;
    /* 0x39 */ u8 mReceived;
    /* 0x3A */ u8 mPlayReq;
    /* 0x3B */ u8 mPerforming;
}; // size 0x3C

// The names of sys_STRING/STR_SPNpc_name (getSpNameStr; ENG text).
enum SpName_e {
    SP_NAME_JOAN = 0x01,
    SP_NAME_TOM_NOOK = 0x02,
    SP_NAME_REDD = 0x03,
    SP_NAME_HARRIET = 0x04,
    SP_NAME_BLATHERS = 0x05,
    SP_NAME_JACK = 0x06,
    SP_NAME_KK = 0x07,
    SP_NAME_SABLE = 0x08,
    SP_NAME_MABEL = 0x09,
    SP_NAME_PELLY = 0x0A,
    SP_NAME_PHYLLIS = 0x0B,
    SP_NAME_ROVER = 0x0C,
    SP_NAME_TORTIMER = 0x0D,
    SP_NAME_CELESTE = 0x0E,
    SP_NAME_TOMMY = 0x0F,
    SP_NAME_TIMMY = 0x10,
    SP_NAME_RESETTI = 0x11,
    SP_NAME_DON = 0x12,
    SP_NAME_BLANCA = 0x13,
    SP_NAME_KAPPN = 0x14,
    SP_NAME_GULLIVER = 0x15,
    SP_NAME_WENDELL = 0x16,
    SP_NAME_BREWSTER = 0x17,
    SP_NAME_LYLE = 0x18,
    SP_NAME_KATIE = 0x19,
    SP_NAME_KAITLIN = 0x1A,
    SP_NAME_ZIPPER = 0x1B,
    SP_NAME_PASCAL = 0x1C,
    SP_NAME_SAHARAH = 0x1D,
    SP_NAME_FRANKLIN = 0x1E,
    SP_NAME_GRACIE = 0x1F,
    SP_NAME_SHRUNK = 0x20,
    SP_NAME_FRILLARD = 0x21,
    SP_NAME_KATRINA = 0x22,
    SP_NAME_PETE = 0x23,
    SP_NAME_JINGLE = 0x24,
    SP_NAME_CHIP = 0x25,
    SP_NAME_COPPER = 0x26,
    SP_NAME_BOOKER = 0x27,
    SP_NAME_LABELLE = 0x28,
    SP_NAME_KICKS = 0x29,
    SP_NAME_NAT = 0x2A,
    SP_NAME_PAVE = 0x2B,
    SP_NAME_LLOID = 0x2C,
    SP_NAME_SERENA = 0x2D,
    SP_NAME_WISP = 0x2E,
    SP_NAME_UNKNOWN = 0x2F, // "?????"
    SP_NAME_CLERK = 0x30,
    SP_NAME_EMCEE = 0x31,
    SP_NAME_SNOWMAN = 0x32,
    SP_NAME_PHINEAS = 0x33,
    SP_NAME_UNKNOWN_REDD = 0x34, // "?????" (Redd before the first visit)
};

u16 getSpNameStr(const dItem::Item *npc); // 800F88AC
int getSpSound(const dItem::Item *npc);   // 800F88E0
int getSpVoice(const dItem::Item *npc);   // 800F8914
BOOL isSameFashion(const dItem::Item *a, const dItem::Item *b); // 800F8948
BOOL isPlayerFashionMatching();                                 // 800F8A20

// Katie, the lost child (d_a_npc_sp_missing_child, d_a_npc_sp_gatekeeper_a, d_travel_data).
class gatekeeperA_c {
public:
    ~gatekeeperA_c() {}
    static gatekeeperA_c *get(); // 800F8A84
    void clear();                // 800F8A8C
    void setEscorting(u8 escorting) { mEscorting = escorting; }
    void setAtGate(u8 atGate) { mAtGate = atGate; }

    /* 0x0 */ u8 mEscorting;
    /* 0x1 */ u8 mAtGate;
}; // size 0x2

// Joan (d_a_npc_sp_kaburiba).
class kaburiba_c {
public:
    ~kaburiba_c() {}
    static kaburiba_c *get(); // 800F8A9C
    void clear();             // 800F8AA4
    void setTalked();         // 800F8AB0
    BOOL isTalked();          // 800F8B08

    /* 0x0 */ u8 mTalked;
}; // size 0x1

// Celeste (d_a_npc_sp_fuko).
class fuko_c {
public:
    ~fuko_c() {}
    static fuko_c *get(); // 800F8B50
    void clear();         // 800F8B58
    void setTalked();     // 800F8B64
    BOOL isTalked();      // 800F8BB0

    /* 0x0 */ u8 mTalked;
}; // size 0x1

// Blathers (d_a_npc_sp_futa).
class futa_c {
public:
    ~futa_c() {}
    static futa_c *get();        // 800F8BF8
    void clear();                // 800F8C00
    void setGreeted(u8 greeted); // 800F8C0C
    BOOL isGreeted();            // 800F8C14

    /* 0x0 */ u8 mGreeted;
}; // size 0x1

} // namespace dNpcInfo
