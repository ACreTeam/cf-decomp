#ifndef D_A_NPC_SP_HONMA_HPP
#define D_A_NPC_SP_HONMA_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (fBase_c, dActor_c, the script words and dItem::nameSeries_c, then the npc
// classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_item_name.hpp>
#include <game/game/d_a_npc_sp.hpp>

// Lyle of the Happy Room Academy (REL d_a_npc_sp_honmaNP, profile
// A_NPC_SP_HONMA; messages npc_SP/NPC_honma). He stands in the Academy's
// office (open HRA_OPEN_HOUR..HRA_CLOSE_HOUR), follows the player around,
// asks him to join the Academy and then answers questions about the model
// room of the month, the player's last evaluation and the rating letters.
//
// Class names from RTTI ("dAcNpcSpHonma_c", "dAcNpcSpHonma_c::talk_c");
// everything else is named from behaviour.
class dAcNpcSpHonma_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_IDLE,   // wait; follow when the player is far
        STATE_FOLLOW, // walk to the player until near
        STATE_LOOK,   // a demo started (DEMO_CMD_START): turn to the player
        STATE_TALK,   // the talk (l_talkSteps)
        STATE_COUNT
    };
    // The steps of STATE_LOOK / STATE_TALK (l_lookSteps / l_talkSteps).
    enum Step_e { STEP_START, STEP_DONE };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    // The Academy's office hours (create: mIsActive).
    enum { HRA_OPEN_HOUR = 9, HRA_CLOSE_HOUR = 21 };

    typedef void (dAcNpcSpHonma_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: the Academy's talk.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // npc_SP/NPC_honma (from the ENG text).
        enum {
            MSG_FIRST = 0x01,     // "Hey. You. Wait. ... Ol' Lyle needs to talk
                                  // at ya" (asks the player to join)
            MSG_JOINED = 0x07,    // "Good choice! Excellent!" (joined)
            MSG_ASK_AGAIN = 0x0C, // "Hey! You! Wait! It's you! You came
                                  // back!" (not a member yet)
            MSG_ASK_AGAIN_SULK = 0x0D, // "You? Oh. Great. You." (not a member,
                                       // talked to already)
            MSG_WELCOME = 0x0E,        // "Huh? Customers? ... Whatcha need?"
            MSG_QUESTION = 0x0F,       // "Hey? You? Got a question?" (talked to
                                       // already)
            MSG_ANYTHING_ELSE = 0x10,  // "Anything else Lyle can help ya
                                       // with?"
            MSG_LETTERS_STOP_ASK = 0x12,   // "Letters? ... You want us to
                                           // stop?"
            MSG_LETTERS_RESUME_ASK = 0x13, // "We sent letters. You said no.
                                           // ... Want old Lyle to send them
                                           // again?"
            MSG_LETTERS_UNCHANGED = 0x14,  // "That right? OK. Sure. Fine."
            MSG_LETTERS_STOPPED = 0x15,    // "The letters? Gone. Poof."
            MSG_LETTERS_RESUMED = 0x16,    // "The letters? All yours. We'll
                                           // send 'em again."
            MSG_NO_EVALUATION = 0x17,      // "Your room evaluation? Starts next
                                           // week."
            MSG_EVALUATION = 0x18,         // "Your latest evaluation got you N
                                           // points."
            MSG_EVALUATION_END = 0x19,     // "Uh-huh. Yep. There it is. ... Try
                                           // following that advice."
            MSG_ADVICE = 0x1F, // + mHRAInfo.mLastRemark: the evaluators'
                               // advice
            MSG_THEME = 0x3D,  // + getModelRoomTheme() (0..5): "This month's
                               // theme? ..."
            MSG_WINNER_YOU = 0x43,       // "This month? You. Your room. Big
                                         // winner."
            MSG_WINNER_PLAYER = 0x44,    // "This month's winner? Kid from
                                         // <town>. Name's <player>."
            MSG_WINNER_VILLAGER = 0x45,  // "This month's winner? Kid from
                                         // <town>. Name's <villager>."
            MSG_ON_DISPLAY_YOURS = 0x46, // "So, now? <you>'s room. On
                                         // display. In the back."
            MSG_MAKE_ROOMS = 0x47, // "Ideas? You got some? ... Now go make
                                   // some happy rooms!"
            MSG_NO_WINNER = 0x48,  // "No rooms matched. None. Zero."
        };
        // Messages below this one get the last rating's series name as word 0
        // (onMessageStart; the remarks HR_REMARK_* start at 0x51).
        enum { MSG_SERIES_WORD_END = 0x50 };
        // The message words (NPC_honma.bmg entry 0 lists 2..4).
        enum {
            WORD_SERIES = 0,              // the last rating's series
            WORD_SCORE = 0,               // MSG_EVALUATION: the points
            WORD_MODEL_ROOM_TOWN = 2,     // the model room owner's town
            WORD_MODEL_ROOM_PLAYER = 3,   // the model room owner (a player)
            WORD_MODEL_ROOM_VILLAGER = 4, // the model room owner (a villager)
        };
        enum { SCORE_DIGITS = 6 };
        // The answers of the question menus (MSG_WELCOME, MSG_QUESTION,
        // MSG_ANYTHING_ELSE).
        enum {
            ANSWER_MODEL_ROOM, // "The model room."
            ANSWER_EVALUATION, // "My latest evaluation."
            ANSWER_LETTERS,    // "Letters."
            ANSWER_NOTHING,    // "Nothing."
        };
        // The message end requests of the remarks (onMessageEnd kind): the
        // next remark of dHR::remarks_c to tell.
        enum {
            END_REMARK_SERIES = 0x11,  // after the theme remark
            END_REMARK_PARTS = 0x12,   // after the series remark
            END_REMARK_NOTABLE = 0x13, // after the parts remark
            END_REMARK_EXTRA = 0x14,   // after the notable remark
        };
        // mRemark: the remark whose words onMessageStart sets.
        enum {
            REMARK_NONE,
            REMARK_THEME,
            REMARK_SERIES,
            REMARK_PARTS,
            REMARK_NOTABLE,
            REMARK_EXTRA,
        };

        talk_c(dAcNpcSpHonma_c *npc);             // 0x760
        virtual ~talk_c();                        // 0x7C0
        virtual void init();                      // 0x818
        virtual void getMsgInfo(msgInfo_s *info); // 0x94C
        virtual void onMessageStart(int arg);     // 0x964
        virtual void onMessageEnd(int kind);      // 0xCC4
        virtual void onAnswer(int arg);           // 0x1280
        virtual void preExecute();                // 0x13E8
        virtual void onTalkEnd();                 // 0x13EC
        virtual void onRequestEnd(int kind);      // 0x1420

        void start(dActor_c *target); // 0x884

        /* 0x0EA */ u16 mMsgNo; // the first message (getMsgInfo)
        /* 0x0EC */ dAcNpcSpHonma_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks (never set here)
        /* 0x0FC */ Proc mOnRequestEnd;
        /* 0x108 */ int mRemark; // REMARK_*
    }; // size 0x10C

    dAcNpcSpHonma_c() : mHonmaTalk(this) {
    }
    virtual ~dAcNpcSpHonma_c() {
    }
    virtual void onDemoCmd(int cmd, u8 member); // 0x1CC
    void changeState(int state);                // 0x274
    virtual int preCreate();                    // 0x2AC
    virtual int create();                       // 0x2F8
    virtual int doDelete();                     // 0x3C0
    virtual int vtB0();                         // 0x3EC
    void initIdle();                            // 0x434
    void executeIdle();                         // 0x448
    void initFollow();                          // 0x498
    void executeFollow();                       // 0x4FC
    void initLook();                            // 0x564
    void stepLook();                            // 0x5C8
    void executeLook();                         // 0x628
    void initTalk();                            // 0x668
    void stepTalk();                            // 0x6D4
    void executeTalk();                         // 0x720
    virtual int getEarType();                   // 0x1484

    mAng getTalkAngle() const {
        dActor_c *target = mHonmaTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mHonmaTalk;
}; // size 0x1D90

#endif
