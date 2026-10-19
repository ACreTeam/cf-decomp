#ifndef D_A_NPC_SP_GRACE_HPP
#define D_A_NPC_SP_GRACE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, the script words, dDebugPage_c, the dItem name words,
// then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_debug_page.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_a_npc_sp.hpp>

// Gracie in her GracieGrace shop in the city (REL d_a_npc_sp_graceNP,
// profile A_NPC_SP_GRACE). She is out on the days the town flag
// SAVE_FLAG_GRACIE_IN_SHOP is set and gives the fashion check: she judges the
// player's shirt, hat, accessory, hair and shoes (their colours, fashion
// themes and looks), counts penalty points and gives a 10% discount for a
// perfect outfit. On steps 2 and 6 of the GracieGrace story she instead sighs
// over her troubles (npc_SP/NPC_drama5).
//
// Class names from RTTI ("dAcNpcSpGrace_c", "dAcNpcSpGrace_c::talk_c",
// "dAcNpcSpGrace_c::dGarceDebugPage_c", sic); everything else is named from
// behaviour.
//
// The vtables in .data are in reverse class-completion order (the actor,
// talk_c, dGarceDebugPage_c), so dGarceDebugPage_c is declared first.
class dAcNpcSpGrace_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE, // stand still
        STATE_LOOK, // turn to the player (demo start)
        STATE_TALK, // talk to the player
        STATE_COUNT
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // The GracieGrace story steps (dPrivateData_c::getGracieStep) with her
    // story talk (npc_SP/NPC_drama5).
    enum { STORY_STEP_DRAMA = 2, STORY_STEP_DRAMA_2 = 6 };
    enum Step_e { STEP_START, STEP_DONE };

    typedef void (dAcNpcSpGrace_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The page of the debug menu (empty in this build).
    class dGarceDebugPage_c : public dDebugPage_c {
      public:
        // Weak copies come out in reverse declaration order.
        virtual void open();
        virtual void close();
        virtual void execute();
        virtual BOOL isActive();
    };

    // The message receptor: the greetings and the fashion check.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_grace (from the ENG text). The checks
        // run through the message flow: onMessageEnd judges one part per
        // message and picks the next remark; 0x5E..0x63 are empty hops.
        enum {
            MSG_FIRST_MEETING = 0x01, // (empty: the first meeting)
            MSG_CHECK = 0x02, // "...I just happen to have a little time..."
            MSG_FRIEND_STYLE =
                0x03,        // "...I saw the most stylish young thing..."
            MSG_SHOP = 0x04, // "Of course! ...my best work is on display."
            MSG_SAVED_OUTFIT = 0x05,  // "...you can't go wearing it every day!"
            MSG_HELLO_CHECKED = 0x06, // "...keeping up on the hottest trends"
            MSG_CHECK_2 = 0x07,       // "...the countess of couture..."
            MSG_FRIEND_STYLE_2 = 0x08,  // "...seen this fashionista..."
            MSG_SHOP_2 = 0x09,          // "Oh, what resourcefulness!..."
            MSG_HELLO = 0x0A,           // "...What can Gracie do for you?"
            MSG_CHECK_3 = 0x0B,         // "...the ferocious fashionista..."
            MSG_FAN = 0x0C,             // "Oh? Are you a fan of my work?"
            MSG_HELLO_AGAIN = 0x0D,     // "...Have a passion for high fashion"
            MSG_DISCOUNT = 0x0E,        // "...I'm gonna give you 10% off..."
            MSG_STUDY = 0x0F,           // "...you've still got much to learn"
            MSG_SAVED_OUTFIT_2 = 0x10,  // "...as hot as it is..."
            MSG_HELLO_CHECKED_2 = 0x11, // "...keeping a watch on the hottest"
            MSG_CHECK_4 = 0x12,         // "...the countess of couture..."
            MSG_FRIEND_STYLE_3 = 0x13,  // "...seen this fashionista..."
            MSG_SHOP_3 = 0x14,          // "Oh, what resourcefulness!..."
            MSG_BUSY = 0x15,            // "...I'm quite busy right now."
            MSG_FRIEND_STYLE_4 = 0x16,  // "...the most ferocious fashionista"
            MSG_SHOP_4 = 0x17,          // "Flawless fashion sense..."
            MSG_INTRO = 0x18,           // "Oh, who's this now?..."
            MSG_INTRO_2 = 0x19,         // "Oh, who's this now?..."
            MSG_NO_HAT_ACC =
                0x1F, // "...can't bother to accessorize OR wear a cute hat?"
            MSG_NO_HAT_ACC_GRACE =
                0x20, // "...wearing my ... original pieces without a hat"
            MSG_ALL_ONE_COLOR = 0x21, // "...head to toe in <color>!"
            MSG_COLOR_CLASH = 0x22,   // "...something about your use of color"
            MSG_COLOR_OK = 0x23,      // "...you're pulling off that ensemble"
            MSG_FASHION_THEME = 0x24, // "...make <fashion> a central theme"
            MSG_ONE_COLOR = 0x25,     // "...nothing but <color>."
            MSG_THEME_CLASH = 0x26,   // "...some serious clash going on..."
            MSG_THEME_ONE_COLOR = 0x27, // "...nothing but <color>."
            MSG_THEME_HAIR = 0x28,      // "...something about your hair!"
            MSG_THEME_HAIR_2 = 0x29,    // "...get you to a hairdresser, STAT!"
            MSG_THEME_SHOES = 0x2A,     // "...look at the color of your shoes"
            MSG_THEME_SHOES_2 = 0x2B,   // "...those shoes have got to go!"
            MSG_LOOK_THEME = 0x2C, // "...I like how you're working that <look>"
            MSG_LOOK_SAFE = 0x2D,  // "...Playing it safe, are we?"
            MSG_LOOK_NONE = 0x2E,  // "...I just don't see a solid theme..."
            MSG_ONE_COLOR_2 = 0x2F,   // "...or so I thought ... nothing but"
            MSG_ONE_COLOR_3 = 0x30,   // "...gonna ding you for wearing nothing"
            MSG_SHOES = 0x31,         // "...look at the color of those shoes!"
            MSG_SHOES_2 = 0x32,       // "Oh, my! Those shoes have got to go!"
            MSG_COLOR_CLASH_2 = 0x33, // "...pairing those colors together?"
            MSG_COLOR_CLASH_3 = 0x34, // "...pairing those colors together?"
            MSG_ONE_COLOR_4 = 0x35,   // "Moving on... nothing but <color>!"
            MSG_COLOR_FINE = 0x36, // 0x36..0x39: "...but look at those shoes!"
            MSG_COLOR_FINE_2 = 0x37,
            MSG_COLOR_FINE_3 = 0x38,
            MSG_COLOR_FINE_4 = 0x39,
            MSG_HAIR = 0x3A,        // "And look at that hair!..."
            MSG_HAIR_2 = 0x3B,      // "...And your HAIR, honey!..."
            MSG_SHOES_3 = 0x3C,     // "...a crime against sidewalks."
            MSG_SHOES_4 = 0x3D,     // "...those shoes are going to have to go"
            MSG_COLOR_WAR = 0x3E,   // "...all the colors are at war..."
            MSG_COLOR_WAR_2 = 0x3F, // "...brush up against some wet paint?"
            MSG_HAIR_3 = 0x40,      // "...your hair is atrocious!"
            MSG_CHECK_BED_HEAD = 0x42,   // "...tame that wild mane of yours!"
            MSG_CHECK_BED_HEAD_2 = 0x43, // "...have someone cure your..."
            MSG_CHECK_BED_HEAD_3 = 0x44, // "...that mangled mop..."
            MSG_BED_HEAD_AGAIN = 0x45,   // "...Your hair is still a disaster!"
            MSG_MANNEQUIN_OUTFIT =
                0x47, // "That's the exact same outfit as the one hanging..."
            MSG_MII_MASK = 0x48,       // "That makeup you're sporting..."
            MSG_SHOES_OK = 0x49,       // "...those shoes aren't bad, either..."
            MSG_SHOES_OK_2 = 0x4A,     // "...a small victory..."
            MSG_TO_RESULT = 0x5E,      // (empty) -> the result
            MSG_TO_HAIR = 0x5F,        // (empty) -> the hair
            MSG_TO_HAIR_THEME = 0x60,  // (empty) -> the hair after a look theme
            MSG_TO_COLORS = 0x61,      // (empty) -> the colours
            MSG_TO_SHOES = 0x62,       // (empty) -> the shoes
            MSG_TO_THEME_HAIR = 0x63,  // (empty) -> the hair (fashion theme)
            MSG_RESULT_5 = 0x65,       // 5+ penalty points: "...THOSE people!"
            MSG_RESULT_4 = 0x66,       // "...yaks with better fashion sense."
            MSG_RESULT_3 = 0x67,       // "...you're one of THOSE types."
            MSG_RESULT_2 = 0x68,       // "...an A for effort..."
            MSG_RESULT_1 = 0x69,       // "...still a step away..."
            MSG_RESULT_PERFECT = 0x6A, // "...you look faaaaaabulous!..."
            MSG_RESULT_PERFECT_AGAIN = 0x6B, // "...You've got that fire..."
        };
        // Message codes of npc_SP/NPC_drama5 (the GracieGrace story).
        enum {
            MSG_DRAMA_SIGH = 0x01,    // step 2: "Oh, sigh..."
            MSG_DRAMA_TROUBLE = 0x03, // step 2: "...Everything is going wrong!"
            MSG_DRAMA_AGAIN = 0x04,   // step 2: "You again? What now?..."
            MSG_DRAMA_SIGH_2 = 0x05,  // step 6: "Total sigh..."
            MSG_DRAMA_TROUBLE_2 = 0x07, // step 6: "...OK, fine! Everything!"
            MSG_DRAMA_AGAIN_2 = 0x08,   // step 6: "You again? What is it?..."
        };
        // The answers of the choices.
        enum { ANSWER_ADVICE, ANSWER_SHOP };

        // The colour judgements (judgeBasicColors / judgeColors /
        // judgeHiddenHairColors).
        enum ColorResult_e {
            COLOR_RESULT_OK,     // a few colours that go together
            COLOR_RESULT_CLASH,  // too many colours
            COLOR_RESULT_SINGLE, // everything in one colour
        };
        // The penalty points of the remarks (mPenalty).
        enum {
            PENALTY_MINOR = 1,
            PENALTY_MAJOR = 2,
            PENALTY_NO_HAT_ACC = 3,
        };
        enum {
            CHECKS_NUM = 3
        }; // dPrivateData_c::mGracieChecks counts up to it

        talk_c(dAcNpcSpGrace_c *npc);             // 0x7E0
        virtual ~talk_c();                        // 0x848
        virtual void init();                      // 0x8A0
        virtual void getMsgInfo(msgInfo_s *info); // 0xB6C
        virtual void onMessageStart(int arg);     // 0xBAC
        virtual void onMessageEnd(int kind);      // 0xD98
        virtual void onAnswer(int arg);           // 0x1FC8
        virtual void preExecute();                // 0x2438
        virtual void onTalkEnd();                 // 0x243C
        virtual void onRequestEnd(int kind);      // 0x24C8

        void start(dActor_c *target); // 0x980

        // ---- the fashion check ----
        // The number of different colours (not none) among the shirt and the
        // selected parts.
        int countColors(BOOL hat, BOOL hair, BOOL acc, BOOL shoes); // 0x1A38
        void startCheck();  // 0x1C80: the first remark (themes / looks)
        void finishCheck(); // 0x1E8C: the result, the discount, recordOutfit
        int judgeBasicColors(); // 0x252C: no hat and accessory: ColorResult_e
        int judgeShoeColor();   // 0x2660: 1 if the shoes match no colour
        int judgeHiddenHairColors(); // 0x2734: hair hidden: ColorResult_e
        int judgeColors();           // 0x288C: with the hair: ColorResult_e
        int judgeHair();      // 0x2CB4: 1 if the hair fits none of the looks
        int judgeShoeShirt(); // 0x2DB4: 1 if the shoes don't go with the shirt
        BOOL isHairShown();   // 0x311C: the hat leaves the hair visible
        BOOL isSavedOutfit(); // 0x3188: the player wears a saved outfit
        void recordOutfit();  // 0x3274: save the outfit, share it with friends
        void addPenalty(int points); // 0x34A0

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpGrace_c *mpOwner;
        /* 0x0F0 */ Proc
            mOnMessageEnd; // one-shot hooks (cleared after the call)
        /* 0x0FC */ Proc mOnAnswer;
        /* 0x108 */ Proc mOnRequestEnd;
        /* 0x114 */ int mPenalty;     // penalty points (0: perfect)
        /* 0x118 */ u8 mOneColorTold; // MSG_THEME_ONE_COLOR: skip the shoes
        /* 0x119 */ u8 mLookTheme;    // MSG_LOOK_THEME: the hair follows
        /* 0x11A */ u8 mNoLook;       // MSG_LOOK_NONE: the "...But" wordings
        /* 0x11B */ u8 mChecking;     // startCheck .. finishCheck
        /* 0x11C */ u8 mHairNext;     // after the shoes: the hair
        /* 0x11D */ u8
            mHairNextTheme; // after the shoes: the hair if mLookTheme
        /* 0x11E */ u8 mBasicColorsJudged; // set by the judge* functions
        /* 0x11F */ u8 mShoeColorJudged;
        /* 0x120 */ u8 mHiddenHairColorsJudged;
        /* 0x121 */ u8 mColorsJudged;
        /* 0x122 */ u8 mHairJudged;
        /* 0x123 */ u8 mShoeShirtJudged;
        /* 0x124 */ int mBasicColorsCase; // judgeBasicColors' deciding case
        /* 0x128 */ u8
            mShoeColorMatch; // judgeShoeColor: the shoes match a colour
        /* 0x12C */ int mHiddenHairColorsCase;
        /* 0x130 */ int mColorsCase;
        /* 0x134 */ int mMainColorCount; // judgeColors: the most used colour
        /* 0x138 */ int mColorCount;     // judgeColors: the parts counted
        /* 0x13C */ int mMainColor;
        /* 0x140 */ u8 mHairFits;        // judgeHair
        /* 0x141 */ u8 mShoesMatchShirt; // judgeShoeShirt
        /* 0x142 */ u8 mDramaTalked;     // the story talk was heard once
        /* 0x143 */ u8 mFriendMentioned; // onTalkEnd clears the friend player
    }; // size 0x144

    dAcNpcSpGrace_c() : mGraceTalk(this) {
    }
    virtual ~dAcNpcSpGrace_c() {
    } // weak 0x365C
    virtual void onDemoCmd(int cmd, u8 member); // 0x2C0
    void changeState(int state);                // 0x38C
    virtual int preCreate();                    // 0x3C4
    virtual int create();                       // 0x410
    virtual int doDelete();                     // 0x4DC
    virtual int vtB0();                         // 0x540
    virtual int getEarType();                   // 0x34B0
    void initIdle();                            // 0x5CC
    void executeIdle();                         // 0x5E0
    void initLook();                            // 0x5E4
    void stepLook();                            // 0x648
    void executeLook();                         // 0x6A8
    void initTalk();                            // 0x6E8
    void stepTalk();                            // 0x754
    void executeTalk();                         // 0x7A0

    mAng getTalkAngle() const {
        dActor_c *target = mGraceTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C */ u32 _1C7C;
    /* 0x1C80 */ u32 _1C80;
    /* 0x1C84 */ dGarceDebugPage_c mDebugPage;
    /* 0x1C88 */ int mState;
    /* 0x1C8C */ u8 mStep;
    /* 0x1C90 */ talk_c mGraceTalk;
    /* 0x1DD4 */ u8 mDrama; // the story talk (npc_SP/NPC_drama5)
}; // size 0x1DD8

#include <game/game/d_a_npc_sp_grace_inline.hpp>

#endif
