#ifndef D_A_NPC_SP_EVENT_SONCHO_HPP
#define D_A_NPC_SP_EVENT_SONCHO_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_event.hpp>

// Tortimer at the town events (REL d_a_npc_sp_event_sonchoNP, 23 profiles
// NPC_SP_EV_SONCHO_CD .. NPC_SP_EV_SONCHO19). He stands in town on an event
// day, greets the player and hands out the event's present (the event's
// dItem::FROM_* group, kept in dNpcInfo::soncho_c).
//
// Class names from RTTI: the base dAcNpcSpEvSoncho_c and its talk_c, the
// special events (countdown, new year, harvest festival, fireworks) and the
// normal (holiday) events dAcNpcSpEvSoncho01_c..19_c; everything else is named
// from behaviour.
//
// Declaration order matters: .data holds the vtables in reverse
// class-completion order (19 .. 01, Ht, Hf, Ny, Cd, NormalEvent, SpecialEvent,
// the base; each actor before its nested talk class).
class dAcNpcSpEvSoncho_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_IDLE,        // wander (the countdown: stand facing south)
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_COUNT
    };
    // mStep[STATE_LOOK] (l_lookSteps) and mStep[STATE_TALK] (l_talkSteps).
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_DONE };
    enum TalkStep_e {
        TALK_STEP_TURN,     // talk, then (countdown) turn back south
        TALK_STEP_TURN_END, // wait for the turn, finish the talk
        TALK_STEP_DONE
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpEvSoncho_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The net daub data (getDaubData / setDaubData).
    struct daub_s {
        u32 mActive : 1; // event time: he is out (mIsActive)
        u32 _0 : 31;
        wanderDaub_s mWander;
    }; // size 0x8

    // The message receptor. getMsgCode picks the message to start,
    // selectNextMsg reacts to the message that ended.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        talk_c(dAcNpcSpEvSoncho_c *npc);      // 0x3974
        virtual ~talk_c();                    // 0x3A4C
        virtual void onMessageStart(int arg); // 0x4854
        virtual void onMessageEnd(int kind);  // 0x53F0
        virtual void onAnswer(int arg);       // 0x5530
        virtual void init();                  // 0x3B54
        virtual void preExecute();            // 0x5540
        virtual void onTalkEnd();             // 0x5544
        virtual void onRequestEnd(int kind);  // 0x556C
        // new virtuals (+0xA8..+0xB4)
        virtual int getMsgCode() = 0;     // +0xA8: the message to start
        virtual void selectNextMsg() = 0; // +0xAC: on the end of a message
        virtual void onAnswerSelect() {
        } // +0xB0 (weak): onAnswer
        virtual void onStart() {
        } // +0xB4 (weak): start

        void start(dActor_c *target);      // 0x4708
        void setPresent(dItem::Item item); // 0x5464
        void giveItem();                   // 0x55D0: hand the present over
        void pickUpPresent();              // 0x5614: the player takes it

        // Give the present at the end of the message.
        void setGivePresent() {
            mOnMessageEnd = &talk_c::giveItem;
            mOnRequestEnd = &talk_c::pickUpPresent;
        }

        /* 0x0EA */ u16 mMsgNo;         // getMsgCode
        /* 0x0EC */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0F8 */ Proc mOnRequestEnd;
        /* 0x104 */ dAcNpcSpEvSoncho_c *mpOwner;
    }; // size 0x108

    dAcNpcSpEvSoncho_c() {
    }
    virtual ~dAcNpcSpEvSoncho_c() {
    } // weak 0x6FE0
    virtual int create();                       // 0x26B4
    virtual int preCreate();                    // 0x24E0
    virtual int doDelete();                     // 0x27F0
    virtual void onDemoCmd(int cmd, u8 member); // 0x2344
    virtual int vtB0();                         // 0x281C
    virtual BOOL isWanderWalk();                // 0x5760
    virtual int vtE4();                         // 0x396C
    // new virtuals (+0xE8..+0xF4)
    virtual dItem::Item getPresent(); // 0x3170: picked once (soncho_c)
    virtual BOOL isEventTime();       // 0x2F5C: he is out
    virtual int getPresentFrom() = 0; // the present's dItem::FROM_* group
    virtual dQuestEvent_e getEvent() = 0;

    void changeState(int state);        // 0x246C
    void initIdle();                    // 0x29A0
    void executeIdle();                 // 0x2A18
    void initLook();                    // 0x2AB8
    void stepLook();                    // 0x2B1C
    void executeLook();                 // 0x2B7C
    void initTalk();                    // 0x2BBC
    void stepTalkTurn();                // 0x2C58
    void stepTalkTurnEnd();             // 0x2CF4
    void executeTalk();                 // 0x2D44
    void initRemote();                  // 0x2D84
    void executeRemote();               // 0x2D8C
    void initRemoteTalk();              // 0x2E38
    void executeRemoteTalk();           // 0x2E3C
    bool isEventOngoing();              // 0x2E90
    bool isEventNotStarted();           // 0x2ED4
    bool isEventOver();                 // 0x2F18
    BOOL adjustWanderPos(mVec3_c *pos); // 0x341C

    talk_c *getTalk() {
        return static_cast<talk_c *>(mpRecept);
    }
    mAng getTalkAngle() const {
        dActor_c *target = mpRecept->mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C */ daub_s mDaub;
    /* 0x1C84 */ int mState;
    /* 0x1C88 */ u8 mStep[STATE_COUNT];
}; // size 0x1C90

// The town events (countdown, new year, harvest festival, fireworks).
class dAcNpcSpEvSonchoSpecialEvent_c : public dAcNpcSpEvSoncho_c {
  public:
    class talkSpecial_c : public dAcNpcSpEvSoncho_c::talk_c {
      public:
        talkSpecial_c(dAcNpcSpEvSoncho_c *npc); // 0x3A10
        virtual ~talkSpecial_c();               // 0x3AFC
        virtual int getMsgCode();               // 0x4600
        virtual void selectNextMsg();           // 0x4FC0
    };

    dAcNpcSpEvSonchoSpecialEvent_c() {
    }
    virtual ~dAcNpcSpEvSonchoSpecialEvent_c() {
    } // weak 0x7104
    virtual int preCreate(); // 0x250C
};

// The holidays (Tortimer's gift days, dAcNpcSpEvSoncho01_c..19_c).
class dAcNpcSpEvSonchoNormalEvent_c : public dAcNpcSpEvSoncho_c {
  public:
    class talkNormal_c : public dAcNpcSpEvSoncho_c::talk_c {
      public:
        // npc_SP/NPC_soncho_1: 30 messages per event from the event's
        // first message (getMsgBase), named from the English text.
        enum {
            MSG_GREETING = 0x01,      // "Hey there, <player>."
            MSG_TODAY_IS = 0x02,      // "It's <event>, sprout!"
            MSG_TODAY_IS_ALSO = 0x03, // another town event is on too
            MSG_ABOUT = 0x04,         // about the event
            MSG_ABOUT_AGAIN = 0x05,   // talked before, no present yet
            MSG_GIVE = 0x06,          // "I think you should have this."
            MSG_GIVEN = 0x07,         // "It's <present>."
            MSG_POCKETS_FULL = 0x08,  // too much stuff
            MSG_POCKETS_FULL_AGAIN = 0x09,
            MSG_VISITOR_GREETING = 0x0A, // "Are you from another town?"
            MSG_EXTRA_1 = 0x13,          // event specific (questions)
            MSG_EXTRA_2 = 0x14,
            MSG_TALKED = 0x15, // after the present
            MSG_OVER = 0x16,   // the event is over
            MSG_PER_EVENT = 30,
        };
        // Setsubun's MSG_EXTRA_1 choice: the ogre mask's colour.
        enum { ANSWER_RED, ANSWER_BLUE };

        talkNormal_c(dAcNpcSpEvSoncho_c *npc);    // 0x39D4
        virtual ~talkNormal_c();                  // 0x3AA4
        virtual void getMsgInfo(msgInfo_s *info); // 0x4800
        virtual int getMsgCode();                 // 0x4608
        virtual void selectNextMsg();             // 0x4FC4
        virtual void onAnswerSelect();            // 0x5498

        dAcNpcSpEvSonchoNormalEvent_c *getOwner() {
            return static_cast<dAcNpcSpEvSonchoNormalEvent_c *>(mpOwner);
        }
        // The owner's first message (the event's messages follow it).
        int getEventMsgBase() {
            return getOwner()->getMsgBase();
        }
    };

    // The first message of each event in NPC_soncho_1 (getMsgBase).
    enum {
        MSG_BASE_SETSUBUN = 0,
        MSG_BASE_GIRLS_DAY = 30,
        MSG_BASE_CHILDRENS_DAY = 60,
        MSG_BASE_JP_AUTUMN_MOON = 90,
        MSG_BASE_TANABATA = 120,
        MSG_BASE_GROUNDHOG_DAY = 150,
        MSG_BASE_NATURE_DAY = 180,
        MSG_BASE_LABOR_DAY = 210,
        MSG_BASE_EXPLORERS_DAY = 240,
        MSG_BASE_NA_AUTUMN_MOON = 270,
        MSG_BASE_MIDSUMMERS_DAY = 300,
        MSG_BASE_NAUGHTY_OR_NICE = 330,
        MSG_BASE_MIDWINTERS_DAY = 360,
        MSG_BASE_EU_AUTUMN_MOON = 390,
        MSG_BASE_LUNAR_NEW_YEAR = 420,
        MSG_BASE_ARBOR_DAY = 450,
        MSG_BASE_TEACHERS_DAY = 480,
        MSG_BASE_DAEBOREUM = 510,
        MSG_BASE_APRIL_FOOLS = 540,
    };

    dAcNpcSpEvSonchoNormalEvent_c() : mNormalTalk(this) {
    }
    virtual ~dAcNpcSpEvSonchoNormalEvent_c() {
    } // weak 0x738C
    virtual int preCreate();                    // 0x2668
    virtual BOOL searchWanderPos(mVec3_c *pos); // 0x363C
    // new virtual (+0xF8)
    virtual int getMsgBase() = 0; // the event's first message in NPC_soncho_1

    /* 0x1C90 */ talkNormal_c mNormalTalk;
}; // size 0x1D98

// The New Year's countdown.
class dAcNpcSpEvSonchoCd_c : public dAcNpcSpEvSonchoSpecialEvent_c {
  public:
    class talkCountdown_c
        : public dAcNpcSpEvSonchoSpecialEvent_c::talkSpecial_c {
      public:
        // npc_SP/NPC_soncho_2 (shared with New Year's Day), named from the
        // English text. From MSG_LATE_VISITOR on, even codes are for
        // visitors, odd ones for residents.
        enum {
            MSG_GREETING = 0x33,         // "Hello there, <player>."
            MSG_VISITOR_GREETING = 0x34, // "You! Whippersnapper!"
            MSG_POPPER_INTRO_1 = 0x35,   // "...the big countdown is tonight!"
            MSG_POPPER_INTRO_2 = 0x36,
            MSG_POPPER_INTRO_3 = 0x37,
            MSG_POPPER_INTRO_4 = 0x38,
            MSG_POPPER_INTRO_5 = 0x39,
            MSG_MORNING = 0x3F,      // "Today marks the end of a long year."
            MSG_AFTERNOON = 0x40,    // "The countdown is a two-year party!"
            MSG_EVENING = 0x41,      // "Pretty soon, this long year..."
            MSG_LATE_VISITOR = 0x42, // 23:00..23:29
            MSG_LATE = 0x43,
            MSG_LATER_VISITOR = 0x44, // 23:30..
            MSG_LATER = 0x45,
            MSG_LAST_MINUTES_VISITOR = 0x46, // the last minutes
            MSG_LAST_MINUTES = 0x47,
            MSG_LAST_MINUTE_VISITOR = 0x48, // 23:59
            MSG_LAST_MINUTE = 0x49,
            MSG_HAPPY_NEW_YEAR = 0x6D, // "HAPPY NEW YEAR!"
            MSG_POPPER = 0x79,         // a party popper
            MSG_POPPER_GIVE = 0x7A,
            MSG_TALKED_MORNING = 0x7B, // talked before
            MSG_TALKED_AFTERNOON = 0x7C,
            MSG_TALKED_EVENING = 0x7D,
            MSG_TALKED_LATE = 0x7E,
            MSG_TALKED_LATER = 0x7F,
            MSG_TALKED_LAST_MINUTES = 0x80,
            MSG_TALKED_LAST_MINUTE = 0x81,
            MSG_TALKED_AFTER = 0x82, // after the countdown
            MSG_TALKED_OVER = 0x83,
        };

        talkCountdown_c(dAcNpcSpEvSoncho_c *npc) : talkSpecial_c(npc) {
        }
        virtual ~talkCountdown_c() {
        } // weak 0x722C
        virtual void getMsgInfo(msgInfo_s *info); // 0x47A0
        virtual int getMsgCode();                 // 0x3D28
        virtual void selectNextMsg();             // 0x4A2C
    };

    dAcNpcSpEvSonchoCd_c() : mCdTalk(this) {
    }
    virtual ~dAcNpcSpEvSonchoCd_c() {
    } // weak 0x7A94
    virtual int preCreate();                    // 0x2538
    virtual BOOL searchWanderPos(mVec3_c *pos); // 0x394C
    virtual BOOL isEventTime();                 // 0x3084
    virtual int getPresentFrom() {
        return dItem::FROM_COUNTDOWN;
    } // weak 0x7A8C
    virtual dQuestEvent_e getEvent() {
        return EVENT_COUNTDOWN;
    } // weak 0x74CC

    /* 0x1C90 */ talkCountdown_c mCdTalk;
}; // size 0x1D98

// New Year's Day.
class dAcNpcSpEvSonchoNy_c : public dAcNpcSpEvSonchoSpecialEvent_c {
  public:
    class talkNewyear_c : public dAcNpcSpEvSonchoSpecialEvent_c::talkSpecial_c {
      public:
        // npc_SP/NPC_soncho_2, named from the English text.
        enum {
            MSG_GREETING = 0x01,     // "Happy New Year!"
            MSG_NEW_YEAR = 0x02,     // "Fresh like a new pair of socks!"
            MSG_GOALS = 0x03,        // "...thinking about your goals"
            MSG_GOALS_AGAIN = 0x04,  // "...goals for next year, too?"
            MSG_GIVE = 0x05,         // "Close your eyes..."
            MSG_GIVEN = 0x06,        // "It's <shirt>."
            MSG_POCKETS_FULL = 0x07, // "...you have too much stuff!"
            MSG_POCKETS_FULL_AGAIN = 0x08,
            MSG_VISITOR_GREETING = 0x09,   // "Oh! Er... Happy New Year!"
            MSG_WEARING_PRESENT = 0x24,    // wearing this year's shirt
            MSG_WEARING_OTHER_YEAR = 0x25, // another year's New Year's shirt
            MSG_TALKED = 0x26,
            MSG_OVER = 0x27,
        };

        talkNewyear_c(dAcNpcSpEvSoncho_c *npc) : talkSpecial_c(npc) {
        }
        virtual ~talkNewyear_c() {
        } // weak 0x7284
        virtual void getMsgInfo(msgInfo_s *info); // 0x47B8
        virtual int getMsgCode();                 // 0x3C78
        virtual void selectNextMsg();             // 0x491C
        BOOL isWearingNewYearShirt();             // 0x3BB4
        BOOL isWearingPresent();                  // 0x3C20
    };

    dAcNpcSpEvSonchoNy_c() : mNyTalk(this) {
    }
    virtual ~dAcNpcSpEvSonchoNy_c() {
    } // weak 0x794C
    virtual int preCreate();                    // 0x2584
    virtual BOOL searchWanderPos(mVec3_c *pos); // 0x37C0
    virtual dItem::Item getPresent();           // 0x3360: this year's shirt
    virtual int getPresentFrom() {
        return dItem::FROM_NEWYEAR;
    } // weak 0x7944
    virtual dQuestEvent_e getEvent() {
        return EVENT_NEW_YEARS_DAY;
    } // weak 0x793C

    /* 0x1C90 */ talkNewyear_c mNyTalk;
}; // size 0x1D98

// The Harvest Festival.
class dAcNpcSpEvSonchoHf_c : public dAcNpcSpEvSonchoSpecialEvent_c {
  public:
    class talkHarvestFestival_c
        : public dAcNpcSpEvSonchoSpecialEvent_c::talkSpecial_c {
      public:
        // npc_SP/NPC_soncho_3, named from the English text. He hands out
        // knives and forks (his own, and a villager's to deliver).
        enum {
            MSG_BEFORE_GREETING = 0x01, // "It's the most wonderful day..."
            MSG_BEFORE_VISITOR_GREETING = 0x03,
            MSG_BEFORE_TALKED = 0x04, // "The party starts around 3!"
            MSG_GREETING = 0x0A,      // "Gobble dee HO!"
            MSG_FEAST = 0x0B,
            MSG_TALK = 0x0C, // "...roomiest pants"
            MSG_POCKETS_FULL = 0x0D,
            MSG_POCKETS_FULL_AGAIN = 0x0E,
            MSG_GIVE = 0x0F, // "...hold out your hands"
            MSG_GIVEN = 0x10,
            MSG_HAVING_FUN = 0x11, // "Are you having fun, sprout?"
            MSG_NO_GUEST = 0x12,   // "Where's the guest of honor?!"
            MSG_DELIVER = 0x13,    // "...hold on to these for me?"
            MSG_DELIVER_POCKETS_FULL = 0x14,
            MSG_DELIVER_GIVEN = 0x15, // "This is <villager>'s <item>."
            MSG_DELIVER_GIVEN_VISITOR =
                0x16,                    // "...who lives in this very town."
            MSG_HAVING_FUN_AGAIN = 0x17, // "So, you having fun?"
            MSG_WAIT_GUEST = 0x18,
            MSG_DELIVER_MORE = 0x19, // "...deliver these as well"
            MSG_DELIVER_MORE_POCKETS_FULL = 0x1A,
            MSG_DELIVER_MORE_GIVEN = 0x1B, // "That <item> belongs to..."
            MSG_DELIVER_MORE_GIVEN_VISITOR = 0x1C,
            MSG_NO_VILLAGER = 0x1D, // "With a knife in one hand..."
            MSG_LOST = 0x1F,        // "You say I never even gave them"
            MSG_LOST_AGAIN = 0x20,
            MSG_REGIVE = 0x21, // "...I should give you these"
            MSG_REGIVE_POCKETS_FULL = 0x22,
            MSG_REGIVEN_AGAIN = 0x23,
            MSG_REGIVEN = 0x24,
            MSG_REGIVEN_VISITOR = 0x25,
            MSG_REGIVEN_VISITOR_AGAIN = 0x26,
            MSG_DROPPED = 0x27, // "Hey, did you drop that...?"
            MSG_DROPPED_AGAIN = 0x28,
            MSG_AFTER = 0x33,    // the festival is over
            MSG_CARRYING = 0x3E, // carrying knives and forks
            MSG_CARRYING_LOST = 0x3F,
            MSG_AFTER_TALKED = 0x40,
        };

        talkHarvestFestival_c(dAcNpcSpEvSoncho_c *npc)
            : talkSpecial_c(npc), mAnimalIdx(-1) {
        }
        virtual ~talkHarvestFestival_c() {
        } // weak 0x72DC
        virtual void onMessageStart(int arg);     // 0x48A8
        virtual void onTalkEnd();                 // 0x5568
        virtual void getMsgInfo(msgInfo_s *info); // 0x47D0
        virtual int getMsgCode();                 // 0x3FF4
        virtual void selectNextMsg();             // 0x4ADC
        virtual void onStart();                   // 0x4818
        BOOL isDroppedNear();                     // 0x5654

        /* 0x108 */ u32 mAnimalIdx; // the villager whose knives and forks he
                                    // hands out (pickHarvestFestivalAnimal)
    }; // size 0x10C

    dAcNpcSpEvSonchoHf_c() : mHfTalk(this) {
    }
    virtual ~dAcNpcSpEvSonchoHf_c() {
    } // weak 0x77FC
    virtual int preCreate();                    // 0x25D0
    virtual int vtB0();                         // 0x294C
    virtual BOOL searchWanderPos(mVec3_c *pos); // 0x3888
    virtual dItem::Item getPresent();           // 0x3414: a knife and fork
    virtual BOOL isEventTime();                 // 0x30C8
    virtual int getPresentFrom() {
        return dItem::FROM_HARVESTFESTIVAL;
    } // weak 0x77F4
    virtual dQuestEvent_e getEvent() {
        return EVENT_HARVEST_FESTIVAL;
    } // weak 0x74D4

    /* 0x1C90 */ talkHarvestFestival_c mHfTalk;
}; // size 0x1D9C

// The fireworks shows (hanabi).
class dAcNpcSpEvSonchoHt_c : public dAcNpcSpEvSonchoSpecialEvent_c {
  public:
    class talkHanabi_c : public dAcNpcSpEvSonchoSpecialEvent_c::talkSpecial_c {
      public:
        // npc_SP/NPC_soncho_4, named from the English text (the codes up
        // to MSG_TALKED_OVER only pick a message).
        enum {
            MSG_PRESENT = 0x01, // a firework
            MSG_PRESENT_GIVE = 0x02,
            MSG_BEFORE = 0x03, // more than 30 minutes before the show
            MSG_BEFORE_SOON = 0x04,
            MSG_STARTED = 0x05, // within the first hour
            MSG_SHOW = 0x06,
            MSG_ENDING = 0x07, // within the last hour
            MSG_TALKED_OVER = 0x08,
            MSG_GREETING = 0x0B,         // "Hey there, <player>."
            MSG_VISITOR_GREETING = 0x0C, // "Oh, hello there, visitor."
            MSG_FIRST_SHOW = 0x0D, // "This is the first Fireworks Show..."
            MSG_TODAY_SHOW = 0x0E,
            MSG_LAST_SHOW = 0x0F, // "Today is the last Fireworks Show..."
            MSG_GIVE = 0x10,
            MSG_GIVE_2 = 0x11,
            MSG_OVER_FIRST = 0x33, // "The first fireworks of summer are over."
            MSG_OVER_SECOND = 0x34,
            MSG_OVER = 0x35,
            MSG_OVER_NEXT_LAST = 0x36, // "...next Sunday will be our last"
            MSG_OVER_LAST = 0x37,
        };

        talkHanabi_c(dAcNpcSpEvSoncho_c *npc) : talkSpecial_c(npc) {
        }
        virtual ~talkHanabi_c() {
        } // weak 0x7334
        virtual void getMsgInfo(msgInfo_s *info); // 0x47E8
        virtual int getMsgCode();                 // 0x4140
        virtual void selectNextMsg();             // 0x4E48
    };

    dAcNpcSpEvSonchoHt_c() : mHtTalk(this) {
    }
    virtual ~dAcNpcSpEvSonchoHt_c() {
    } // weak 0x76B4
    virtual int preCreate();                    // 0x261C
    virtual BOOL searchWanderPos(mVec3_c *pos); // 0x36F8
    virtual dItem::Item getPresent(); // 0x322C: the firework the player lacks
    virtual BOOL isEventTime();       // 0x2FDC
    virtual int getPresentFrom() {
        return dItem::FROM_FIREWORKS;
    } // weak 0x74DC
    virtual dQuestEvent_e getEvent() {
        return EVENT_FIREWORKS;
    } // weak 0x74C4

    /* 0x1C90 */ talkHanabi_c mHtTalk;
}; // size 0x1D98

// The holidays: the event, the present's group and the first message of the
// event in NPC_soncho_1 (30 messages each). Their destructors are implicit.
#define EV_SONCHO_NORMAL_EVENT(className, event, from, msgBase)                \
    class className : public dAcNpcSpEvSonchoNormalEvent_c {                   \
      public:                                                                  \
        virtual int getMsgBase() {                                             \
            return msgBase;                                                    \
        }                                                                      \
        virtual int getPresentFrom() {                                         \
            return from;                                                       \
        }                                                                      \
        virtual dQuestEvent_e getEvent() {                                     \
            return event;                                                      \
        }                                                                      \
    };

EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho01_c, EVENT_JP_SETSUBUN,
                       dItem::FROM_SETSUBUN,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_SETSUBUN)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho02_c, EVENT_JP_GIRLS_DAY,
                       dItem::FROM_HINA,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_GIRLS_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho03_c, EVENT_JP_CHILDRENS_DAY,
                       dItem::FROM_KODOMO,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_CHILDRENS_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho04_c, EVENT_JP_AUTUMN_MOON,
                       dItem::FROM_TSUKIMI_JP,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_JP_AUTUMN_MOON)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho05_c, EVENT_JP_TANABATA,
                       dItem::FROM_TANABATA,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_TANABATA)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho06_c, EVENT_NA_GROUNDHOG_DAY,
                       dItem::FROM_GROUNDHOG,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_GROUNDHOG_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho07_c, EVENT_NA_NATURE_DAY,
                       dItem::FROM_EARTH_DAY,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_NATURE_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho08_c, EVENT_NA_LABOR_DAY,
                       dItem::FROM_LABOR_DAY,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_LABOR_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho09_c, EVENT_NA_EXPLORERS_DAY,
                       dItem::FROM_COLUMBUS_DAY,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_EXPLORERS_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho10_c, EVENT_NA_AUTUMN_MOON,
                       dItem::FROM_HARVESTMOON,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_NA_AUTUMN_MOON)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho11_c, EVENT_EU_MIDSUMMERS_DAY,
                       dItem::FROM_MIDSUMMER,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_MIDSUMMERS_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho12_c, EVENT_EU_NAUGHTY_OR_NICE_DAY,
                       dItem::FROM_ST_NICHOLAS_DAY,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_NAUGHTY_OR_NICE)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho13_c, EVENT_EU_MIDWINTERS_DAY,
                       dItem::FROM_MIDWINTER,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_MIDWINTERS_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho14_c, EVENT_EU_AUTUMN_MOON,
                       dItem::FROM_TSUKIMI_EU,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_EU_AUTUMN_MOON)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho15_c, EVENT_KR_LUNAR_NEW_YEAR,
                       dItem::FROM_OLD_NEWYEAR,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_LUNAR_NEW_YEAR)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho16_c, EVENT_KR_ARBOR_DAY,
                       dItem::FROM_PLANTING_DAY,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_ARBOR_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho17_c, EVENT_KR_TEACHERS_DAY,
                       dItem::FROM_MASTERS_DAY,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_TEACHERS_DAY)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho18_c, EVENT_KR_DAEBOREUM,
                       dItem::FROM_TSUKIMI_KR,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_DAEBOREUM)
EV_SONCHO_NORMAL_EVENT(dAcNpcSpEvSoncho19_c, EVENT_APRIL_FOOLS_DAY,
                       dItem::FROM_APRIL_FOOL,
                       dAcNpcSpEvSonchoNormalEvent_c::MSG_BASE_APRIL_FOOLS)

#undef EV_SONCHO_NORMAL_EVENT

#endif
