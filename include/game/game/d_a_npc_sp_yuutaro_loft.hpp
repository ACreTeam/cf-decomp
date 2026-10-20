#ifndef D_A_NPC_SP_YUUTARO_LOFT_HPP
#define D_A_NPC_SP_YUUTARO_LOFT_HPP

#include <lib/egg/math/eggMath.h>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_objc.hpp>

// Wisp in the player's attic (REL d_a_npc_sp_yuutaro_loftNP, profile
// A_NPC_SP_YUUTARO_LOFT, messages "npc_SP/NPC_yutarou"). Waits hidden in the
// player's own house; using the magic lamp from the pockets (d_menu_item00:
// summon) places him next to the player in the attic, he appears
// ("af_gst_apear") and grants one wish: weeding the town, clearing the house
// of roaches, or an item (furniture, hat, accessory, clothes). Granting it
// takes the magic lamp back (takeLamp); then he vanishes
// ("af_gst_disappear"). Online he is too shy and only talks.
//
// Class names from RTTI ("dAcNpcSpYuutaroLoft_c",
// "dAcNpcSpYuutaroLoft_c::talk_c"); everything else is named from behaviour
// and the messages. The town Wisp is d_a_npc_sp_yuutaroNP.
class dAcNpcSpYuutaroLoft_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_WAIT, // hidden, waits for the lamp
        STATE_LOOK, // turn to the player (demo start)
        STATE_TALK, // appear and grant the wish (TalkStep_e)
        STATE_NUM
    };
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_NUM };
    // mStep of STATE_TALK (l_talkSteps order).
    enum TalkStep_e {
        TALK_STEP_APPEAR,    // appear for the talk
        TALK_STEP_TALK,      // wait for the talk
        TALK_STEP_DISAPPEAR, // vanish
        TALK_STEP_NET_TALK,  // online: talk without appearing
        TALK_STEP_NUM
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpYuutaroLoft_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // Message codes of npc_SP/NPC_yutarou (ENG text).
        enum Msg_e {
            MSG_NET_ALONE = 0x1A,    // "I'm kind of shy ... Are you sure you
                                     // aren't expecting guests?"
            MSG_ATTIC_MY_HOUSE = 0x1B, // "... do you think maybe we can...go
                                       // upstairs to the attic?"
            MSG_ATTIC_OTHER_HOUSE = 0x1C, // "Not this house. ... the attic in
                                          // your house?"
            MSG_ATTIC_ELSEWHERE = 0x1D,   // "I don't really like scary places
                                          // like this."
            MSG_NET_GUESTS = 0x1E,   // "I'm no good with crowds ... Is someone
                                     // coming?"
            MSG_FIRST_APPEAR = 0x47, // "Tah-Dahh!!" (the first time)
            MSG_FIRST_GREET = 0x48,  // "Nice, huh? ... It's time to grant that
                                     // wish!"
            MSG_ASK_WISH = 0x49,     // "What do you want me to do?" (Weed! /
                                     // Smash roaches! / Give me stuff! / Meh.)
            MSG_WEED_DONE = 0x4A,    // "Weeding? ... Just leave it to me!"
            MSG_LEAVE = 0x4B,        // "Tah-Dahh!!" (he goes back to the lamp)
            MSG_WEED_NONE = 0x4C,    // "... it doesn't look like your town has
                                     // very many weeds."
            MSG_ROACH_DONE = 0x4D,   // "MEEEEEEEP! ... I'll just use MAGIC!"
            MSG_ROACH_NONE = 0x4E, // "... I don't think you have any roaches."
            MSG_NO_WISH = 0x4F,    // "Really? You don't want anything?"
            MSG_STUFF = 0x50,      // "I gotcha! ... AH HA!"
            MSG_ASK_ITEM = 0x51,   // "Which do you want?" (Furniture! / A
                                   // hat! / An accessory! / Clothes! / None)
            MSG_GIVE_FTR = 0x52,   // "Furniture? You got it."
            MSG_GIVE_CAP = 0x53,   // "A hat? You got it."
            MSG_GIVE_ACC = 0x54,   // "An accessory? You got it."
            MSG_GIVE_CLOTH = 0x55, // "Clothes? You got it."
            MSG_TAKE_LAMP = 0x56,  // "I'll put it in your pocket and take the
                                   // lamp back."
            MSG_NO_ITEM = 0x57,    // "Oh! Well, that's fine. ... choose
                                   // something else?"
            MSG_APPEAR = 0x58,     // "Tah-Dahh! You rang?"
            MSG_SKIP = 0x59,       // "Yes, yes. Enough of that. ..."
        };
        // Choices of MSG_ASK_WISH.
        enum Wish_e { WISH_WEED, WISH_ROACH, WISH_STUFF, WISH_NONE };
        // Choices of MSG_ASK_ITEM.
        enum ItemAnswer_e {
            ITEM_ANSWER_FTR,
            ITEM_ANSWER_CAP,
            ITEM_ANSWER_ACC,
            ITEM_ANSWER_CLOTH,
            ITEM_ANSWER_NONE
        };

        talk_c(dAcNpcSpYuutaroLoft_c *npc);       // 0xC8C
        virtual ~talk_c();                        // 0xCEC
        virtual void onMessageStart(int arg);     // 0xEA0
        virtual void onMessageEnd(int kind);      // 0xF04
        virtual void onAnswer(int arg);           // 0xFD0
        virtual void init();                      // 0xD44
        virtual void preExecute();                // 0x128C
        virtual void onTalkEnd();                 // 0x1290
        virtual void getMsgInfo(msgInfo_s *info); // 0xE88
        virtual void onRequestEnd(int kind);      // 0x1294
        void takeLamp();                          // 0xB88
        void start(dActor_c *target);             // 0xDDC

        /* 0x0EA */ u16 mMsgNo; // the first message (start)
        /* 0x0EC */ dAcNpcSpYuutaroLoft_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageStart; // one-shot hooks (cleared after the
                                          // call), except onMessageEnd; none
                                          // is ever set
        /* 0x0FC */ Proc mOnMessageEnd;
        /* 0x108 */ Proc mOnAnswer;
        /* 0x114 */ Proc mOnRequestEnd;
    }; // size 0x120

    dAcNpcSpYuutaroLoft_c() : mWispTalk(this), mSummoned(0) {
    }
    virtual ~dAcNpcSpYuutaroLoft_c() {
    } // weak
    virtual void onDemoCmd(int cmd, u8 member); // 0x1D0
    virtual int canSpeak(dDemoActor_c *actor);  // 0x278
    void summon(int lampSlot);                  // 0x280
    void changeState(int state);                // 0x374
    virtual int preCreate();                    // 0x3AC
    virtual int create();                       // 0x3F8
    virtual int doDelete();                     // 0x47C
    virtual int vtB0();                         // 0x4A8
    void initWait();                            // 0x534
    void executeWait();                         // 0x590
    void initLook();                            // 0x594
    void stepLook();                            // 0x5F8
    void executeLook();                         // 0x658
    void initTalk();                            // 0x698
    void stepTalkAppear();                      // 0x738
    void stepTalk();                            // 0x8D8
    void stepTalkDisappear();                   // 0x940
    void stepNetTalk();                         // 0xAC0
    void executeTalk();                         // 0xB08

    // The fade of the (appearing / disappearing) body: the length of node
    // 0xB's x axis.
    f32 getBodyScale() const {
        mMtx_c mtx = mNodeMtx0B;
        return EGG::Mathf::sqrt(mtx.m[0][0] * mtx.m[0][0] +
                                mtx.m[0][1] * mtx.m[0][1] +
                                mtx.m[0][2] * mtx.m[0][2]);
    }

    mAng getTalkAngle() const {
        dActor_c *target = mWispTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }
    // Turn to the talk partner (stepTalkAppear).
    void turnTalk(mAng angle, BOOL noCamera) {
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, noCamera);
    }

    /* 0x1C7C */ int mState;
    /* 0x1C80 */ u8 mStep;
    /* 0x1C84 */ talk_c mWispTalk;
    /* 0x1DA4 */ u8 mSummoned;  // summon: speak to the player (vtB0)
    /* 0x1DA8 */ int mLampSlot; // the pocket of the rubbed lamp (-1: none)
}; // size 0x1DAC

#endif
