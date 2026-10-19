#ifndef D_A_NPC_SP_FUTA_HPP
#define D_A_NPC_SP_FUTA_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, dDebugPage_c, then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_debug_page_inline.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_effect.hpp>

// Blathers, the museum curator (REL d_a_npc_sp_futaNP, profile A_NPC_SP_FUTA).
class dAcNpcSpFuta_c : public dAcNpcSp_c {
  public:
    enum State_e {
        STATE_IDLE,
        STATE_LOOK,
        STATE_TALK,
        STATE_REMOTE,
        STATE_REMOTE_TALK,
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    enum IdleStep_e { IDLE_STEP_AWAKE, IDLE_STEP_ASLEEP, IDLE_STEP_DONE };
    enum LookStep_e { LOOK_STEP_WAKE, LOOK_STEP_TURN, LOOK_STEP_DONE };
    enum TalkStep_e { TALK_STEP_TURN, TALK_STEP_TURN_END, TALK_STEP_DONE };
    enum RemoteStep_e { REMOTE_STEP_AWAKE, REMOTE_STEP_ASLEEP, REMOTE_STEP_DONE };
    enum {
        SLEEP_START_HOUR = 5, // he dozes from 5 am to 5 pm
        SLEEP_END_HOUR = 17,
        SLEEP_WAIT_FRAMES = 180,
    };

    typedef void (dAcNpcSpFuta_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    class dFutaDebugPage_c : public dDebugPage_c {};

    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_hu-ta
        enum {
            MSG_ASLEEP = 0x01,                // "Hooozzzz... ZWAAWK!"
            MSG_WOKEN_COMPLETE = 0x02,        // "Ahem! Yes. Welcome to the museum! Drifted off, did I?"
            MSG_WOKEN = 0x03,                 // "Oh, I beg your pardon! I'm a bit of a night owl..."
            MSG_WOKEN_AGAIN = 0x04,           // "My profound apologies... I'm so very, very sleepy..."
            MSG_WELCOME_COMPLETE = 0x05,      // "Hoo hoo! Well met, I say!"
            MSG_WELCOME = 0x06,               // "Hoo there! Welcome to the museum."
            MSG_WELCOME_AGAIN = 0x07,         // "Ah, welcome, welcome!"
            MSG_ASSIST = 0x08,                // Donation! / Check a fossil! / Where am I? / Never mind.
            MSG_ASSIST_EXHIBITS = 0x09,       // ... / Exhibits! / ...
            MSG_WANDER = 0x0A,                // "I heartily encourage you to wander at your leisure."
            MSG_PROGRESS_NONE = 0x0B,         // "...we have nothing on display."
            MSG_PROGRESS_LOW = 0x0C,          // "...we have almost nothing on display..."
            MSG_PROGRESS_MID = 0x0D,          // "...our collection is coming along quite nicely."
            MSG_PROGRESS_HIGH = 0x0E,         // "Our collection has grown by leaps and bounds..."
            MSG_PROGRESS_FULL = 0x0F,         // "The collection here at the museum is second to none..."
            MSG_WHERE = 0x10,                 // "Capital question! Our primary mission is..."
            MSG_COLLECTIONS = 0x11,           // "We house four collections..."
            MSG_SOLE_STAFF = 0x12,            // "...I, your humble servant Blathers, am the sole staff member."
            MSG_EXHIBITS = 0x14,              // "Behold our current collection!"
            MSG_NO_FOSSIL = 0x15,             // "...you don't appear to have a fossil needing examination."
            MSG_SHOW_FOSSIL = 0x16,           // "Show me the fossil with all speed!"
            MSG_APPRAISE = 0x17,              // "Hmmmm... Fascinating... AHA!"
            MSG_APPRAISED = 0x18,             // 0x18..0x1A: "What you have here is <item>!"
            MSG_APPRAISED_2 = 0x19,
            MSG_APPRAISED_3 = 0x1A,
            MSG_RETURN_FOSSIL = 0x1B,         // "And thusly, I return it to your safekeeping."
            MSG_ASK_FOSSIL = 0x1C,            // "...one that has yet to find its way into our collection..."
            MSG_KEEP_FOSSIL = 0x1D,           // "Findsy-keepsy and all that."
            MSG_GIVE_FOSSIL = 0x1E,           // "Hootie HOO! Are you quite sure? Simply smashing!"
            MSG_MORE_FOSSILS = 0x1F,          // "Any additional fossils for me to appraise?" Yep! / Nope.
            MSG_NO_MORE_FOSSILS = 0x20,       // "Hoo now? Are you quite sure?"
            MSG_ASSIST_AGAIN = 0x21,          // "Well then, might I assist with anything else?"
            MSG_ASSIST_AGAIN_EXHIBITS = 0x22,
            MSG_VISITORS = 0x23,              // "...there appear to be visitors in town."
            MSG_ASK_DONATION = 0x24,          // "What shall you donate? Oh, sweet suspense!"
            MSG_DONATED_BY_ME = 0x25,         // "<item> has already been donated..."
            MSG_DONATED_BY_OTHER = 0x26,      // "...has already been donated by <donor>."
            MSG_DONATED_BY_DELETED = 0x27,    // "...a resident who has since moved away..."
            MSG_HAVE_FOSSIL = 0x28,           // "...I would purchase this from you myself..."
            MSG_HAVE_FISH = 0x29,             // "Our humble collection will only hold one of these..."
            MSG_HAVE_INSECT = 0x2A,           // "...Do NOT open that until you are outside!"
            MSG_HAVE_PICTURE = 0x2B,          // "...two identical paintings..."
            MSG_DONATE_MORE = 0x2D,           // "...do you wish to donate anything else?" Yep! / Nope.
            MSG_BYE = 0x2E,                   // "Is that so? Until next time, then, wot wot?"
            MSG_THANKS = 0x2F,                // "I salute your patronage!"
            MSG_NO_DONATION = 0x30,           // "...change our mind, did we?"
            MSG_UNIDENTIFIED = 0x31,          // "...An unidentified fossil, I should think."
            MSG_TAKE_FOSSIL = 0x32,           // "And with that, away it goes to our fine collection!"
            MSG_HAVE_FOSSIL_ALREADY = 0x33,   // "<item> has already been donated, it seems."
            MSG_FOSSIL = 0x34,                // "Hootie hoo! This... This is <item>!"
            MSG_FOSSIL_PARTS = 0x35,          // "...it would be splendid if the other parts were found."
            MSG_FOSSIL_LECTURE_END = 0x36,    // "HOO! I...beg your pardon! I was so enthralled..."
            MSG_FISH = 0x37,                  // "Ah... <item>? Yes, quite!"
            MSG_FISH_LECTURE_END = 0x38,      // "...your aquatic friend will be most happy!"
            MSG_INSECT = 0x39,                // "H-h-hoo... I daresay that is <item>."
            MSG_INSECT_LECTURE_END = 0x3A,    // "Bleh, enough, I say... Our thanks for the donation."
            MSG_PICTURE = 0x3B,               // 0x3B..0x3D: "It can only be <item>!"
            MSG_PICTURE_2 = 0x3C,
            MSG_PICTURE_3 = 0x3D,
            MSG_PICTURE_THANKS = 0x3E,        // "Now I must find a spot with proper lighting..."
            MSG_FAKE_PICTURE = 0x3F,          // "WOO HOO! What a treat... This is <item>... HOORK!"
            MSG_FORGERY_EXPOSED = 0x40,       // "...It is a vile, shameless forgery!"
            MSG_KNOWN_FORGERY = 0x41,         // "A forgery! A pure fake! Oh, the SHAME!"
            MSG_FOSSILS_COMPLETE = 0x42,
            MSG_FISH_COMPLETE = 0x43,
            MSG_INSECTS_COMPLETE = 0x44,
            MSG_PICTURES_COMPLETE = 0x45,
            MSG_RELY_ON_YOU = 0x46,           // "I trust we can rely on your unfailing patronage!"
            MSG_ALL_COMPLETE = 0x47,          // "...each and every collection is complete?!"
            MSG_MUSEUM_COMPLETE = 0x48,       // "...the museum collections are complete!"
            MSG_NEWCOMER_WOKEN = 0x49,        // "Yours is a visage that I...don't recall ever..."
            MSG_NEWCOMER = 0x4A,              // "I am the museum's curator, Blathers."
            MSG_NEWCOMER_WOKEN_AGAIN = 0x4B,  // "Ah, yes, it's you, my dear little owlet!"
            MSG_NEWCOMER_AGAIN = 0x4C,        // "I am Blathers, curator of the museum, eh wot?"
            MSG_GENEROSITY = 0x4D,            // "Thanks to the generosity of the people of <town>..."
            MSG_ASSIST_NEXT = 0x64,           // empty: on to MSG_ASSIST_AGAIN(_EXHIBITS)
        };
        enum { PICTURE_MSG_NUM = MSG_PICTURE_3 - MSG_PICTURE + 1 };
        // Answers of MSG_ASSIST(_AGAIN), MSG_ASSIST(_AGAIN)_EXHIBITS and the yes/no questions.
        enum { ASSIST_DONATE, ASSIST_CHECK_FOSSIL, ASSIST_WHERE, ASSIST_NEVER_MIND };
        enum {
            ASSIST_EX_DONATE,
            ASSIST_EX_CHECK_FOSSIL,
            ASSIST_EX_EXHIBITS,
            ASSIST_EX_WHERE,
            ASSIST_EX_NEVER_MIND
        };
        enum { ANSWER_YES, ANSWER_NO };
        // dMuseum_c::getProgress percentages (setProgressMsg).
        enum { PROGRESS_LOW_MAX = 30, PROGRESS_MID_MAX = 70, PROGRESS_FULL = 100 };
        // reqSelectItem: the menu, and its button label (a SYS_2D_menu message).
        enum { MENU_DONATE = 0x2F, MENU_FOSSIL_CHECK = 0x30 };
        enum { ITEM_SEL_MSG_DONATE = 0x38, ITEM_SEL_MSG_GIVE = 0xE7 };
        // requestItemActEx: the effect index (0 / 1 are the eat-fish effects).
        enum { HAND_EFFECT_NONE = 2 };
        // The message file of the next message (mLabel).
        enum Label_e { LABEL_MAIN, LABEL_FOSSIL, LABEL_INSECT, LABEL_FISH };
        enum { MSG_NONE = 0xFFFF };

        talk_c(dAcNpcSpFuta_c *npc);
        virtual ~talk_c();
        virtual void init();
        virtual void getMsgInfo(msgInfo_s *info);
        virtual void onMessageStart(int arg);
        virtual void onMessageEnd(int kind);
        virtual void onAnswer(int arg);
        virtual void preExecute();
        virtual void onTalkEnd();
        virtual void onRequestEnd(int kind);

        void start(dActor_c *target);
        void setProgressMsg();
        void donateItem();
        void openDonationMenu();
        void suspend();
        void openFossilMenu();
        void openExhibits();
        void takeItem();
        void giveBack();
        void receive();
        void onDonationSelected();
        void onFossilSelected();
        void endExhibits();
        void closeMsg();
        void endGiveBack();
        void endReceive();
        void startTurn();
        void execTurn();

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpFuta_c *mpOwner;
        /* 0x0F0 */ dItem::Item mItem; // the item shown or donated
        /* 0x0F4 */ Proc mOnMessageEnd;
        /* 0x100 */ Proc mOnRequestEnd;
        /* 0x10C */ Proc mNextOnRequestEnd;
        /* 0x118 */ Proc mProc;
        /* 0x124 */ int mLabel;   // Label_e
        /* 0x128 */ u8 mDonated;  // donated something this talk
        /* 0x129 */ u8 mAppraise; // "Check a fossil!": appraising, not donating
    }; // size 0x12C

    dAcNpcSpFuta_c() : mTalkFuta(this) {}
    virtual ~dAcNpcSpFuta_c() {}
    virtual void onDemoCmd(int cmd, u8 member);
    void changeState(int state);
    virtual int preCreate();
    virtual int create();
    virtual int doDelete();
    virtual int vtB0();
    void initIdle();
    void wakeUp();
    void stepIdleAwake();
    void fallAsleep();
    void stepIdleAsleep();
    void executeIdle();
    void initLook();
    void initLookAsleep();
    void stepLookWake();
    void initLookTurn();
    void stepLookTurn();
    void executeLook();
    void initTalk();
    void stepTalkTurn();
    void stepTalkTurnEnd();
    void executeTalk();
    void initRemote();
    void initRemoteAwake();
    void stepRemoteAwake();
    void initRemoteAsleep();
    void stepRemoteAsleep();
    void executeRemote();
    void initRemoteTalk();
    void executeRemoteTalk();
    BOOL isSleepTime();
    void turnToPlayer();

    mAng getTalkAngle() const {
        dActor_c *target = mTalkFuta.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const { return l_turnSpeed; }

    // The net (daub) state: fills the 8-byte net slot (dNpc::daub_c::mNpcData).
    struct daub_s {
        u32 mAsleep : 1;
        u32 mUnused : 31;
        u32 mUnused2;
    }; // size 0x8

    /* 0x1C7C */ u32 mUnused1C7C; // never accessed
    /* 0x1C80 */ u32 mUnused1C80;
    /* 0x1C84 */ dFutaDebugPage_c mDebugPage;
    /* 0x1C88 */ dLevelEffect_c mSleepEffect; // "afm_owl_sleep"
    /* 0x1D1C */ int mSleepTimer;
    /* 0x1D20 */ int mState;
    /* 0x1D24 */ u8 mStep;
    /* 0x1D28 */ talk_c mTalkFuta;
    /* 0x1E54 */ daub_s mDaub;
}; // size 0x1E5C

#endif
