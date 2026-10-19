#ifndef D_A_NPC_SP_YUUTARO_HPP
#define D_A_NPC_SP_YUUTARO_HPP

#include <lib/egg/math/eggMath.h>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_objc.hpp>

// Wisp, the lamp ghost in town (REL d_a_npc_sp_yuutaroNP, profile
// A_NPC_SP_YUUTARO, messages "npc_SP/NPC_yutarou"). On his nights
// (dEvent::isVisitorHere(VISITOR_WISP) with the lamp buried,
// EVENT_LAMP_BURIED) he calls the player over, appears when the player comes
// close ("af_gst_apear"), asks for his lamp and vanishes again
// ("af_gst_disappear") when the player walks away. Given the empty lamp he
// turns it into a magic lamp in the player's pocket and leaves; the wish
// itself is granted by the separate d_a_npc_sp_yuutaro_loftNP (the attic).
//
// Class names from RTTI ("dAcNpcSpYuutaro_c", "dAcNpcSpYuutaro_c::talk_c");
// everything else is named from behaviour and the messages.
class dAcNpcSpYuutaro_c : public dAcNpcSp_c {
  public:
    // The states (l_states order).
    enum State_e {
        STATE_GUIDE, // call the player over, appear / disappear (GuideStep_e)
        STATE_LOOK,  // turn to the player (demo start)
        STATE_TALK,  // talk to the player (TalkStep_e)
        STATE_NUM
    };
    // mGuideStep of STATE_GUIDE (l_guideSteps order).
    enum GuideStep_e {
        GUIDE_STEP_CALL,      // hidden; call the player when near
        GUIDE_STEP_LEAD,      // lead the player to his spot
        GUIDE_STEP_WRONG_WAY, // the player went the wrong way
        GUIDE_STEP_APPEAR,    // appear (HMN_ANM_GST_APPEAR1)
        GUIDE_STEP_SHOWN,     // visible; vanish when the player goes away
        GUIDE_STEP_DISAPPEAR, // disappear (HMN_ANM_GST_DISAPPEAR1)
        GUIDE_STEP_HIDDEN,    // hidden; appear when the player comes close
        GUIDE_STEP_NUM,       // gone: the lamp was returned today
    };
    enum LookStep_e { LOOK_STEP_TURN, LOOK_STEP_NUM };
    // mStep of STATE_TALK (l_talkSteps order).
    enum TalkStep_e {
        TALK_STEP_APPEAR,    // appear for the talk
        TALK_STEP_TALK,      // wait for the talk
        TALK_STEP_DISAPPEAR, // vanish with the lamp
        TALK_STEP_NUM
    };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpYuutaro_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // What start() picks a message for (mMsgKind).
        enum MsgKind_e {
            MSG_KIND_CALL,      // the player came near (GUIDE_STEP_CALL)
            MSG_KIND_WRONG_WAY, // the player walks away (GUIDE_STEP_LEAD)
            MSG_KIND_RIGHT_WAY, // the player comes back (GUIDE_STEP_WRONG_WAY)
            MSG_KIND_ARRIVED,   // the player reached him (GUIDE_STEP_LEAD)
            MSG_KIND_LAMP,      // the player talks to him: the lamp
        };

        // Message codes of npc_SP/NPC_yutarou (ENG text). The empty entries
        // pick one of the messages after them.
        enum Msg_e {
            MSG_CALL = 0x01,        // "Hellooooooo. Excuse me? ... would you
                                    // mind coming over here" (0x04..0x08)
            MSG_WRONG_WAY = 0x02,   // "Wait, no! Not that way!" (0x09..0x0D)
            MSG_RIGHT_WAY = 0x03,   // "Yes, that's it. It's this way."
                                    // (0x0E..0x12)
            MSG_RIGHT_WAY_4 = 0x10, // "That's it. Sorry for all the trouble."
            MSG_RELIEF = 0x1F,      // "Whe-e-e-e-e-e-e-w! What a relief!"
            MSG_INTRO = 0x20,      // "... I'm actually the spirit of the lamp!"
            MSG_MEET_AGAIN = 0x21, // "Say, thanks for finding my lamp! ..."
                                   // (0x22..0x25)
            MSG_ASK_LAMP = 0x26,   // "Sooooo... Did you find my lamp?"
            MSG_LAMP_GONE = 0x27,  // "It looks like it's not there. ...
                                   // Someone must have already scooped it up."
            MSG_ASK_LAMP_AGAIN = 0x28, // "Did you find my lamp?" (0x29..0x2C)
            MSG_FIND_LAMP = 0x2D,      // "Please find my lamp!" (0x2E..0x33)
            MSG_LAMP_FOUND = 0x34,     // "You did it! You found my lamp! ... I
                                       // have to grant your wish."
            MSG_LAMP_FOUND_AGAIN = 0x35, // the same (0x36..0x39)
        };

        talk_c(dAcNpcSpYuutaro_c *npc);           // 0x1254
        virtual ~talk_c();                        // 0x12B4
        virtual void onMessageStart(int arg);     // 0x161C
        virtual void onMessageEnd(int kind);      // 0x1680
        virtual void onAnswer(int arg);           // 0x1738
        virtual void init();                      // 0x130C
        virtual void preExecute();                // 0x179C
        virtual void onTalkEnd();                 // 0x17A0
        virtual void getMsgInfo(msgInfo_s *info); // 0x1604
        virtual void onRequestEnd(int kind);      // 0x17A4
        void start(dActor_c *target);             // 0x13E8
        void setWispName();                       // 0x1808

        /* 0x0EA */ u16 mMsgNo; // the first message (start)
        /* 0x0EC */ dAcNpcSpYuutaro_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageStart; // one-shot hooks (cleared after the
                                          // call), except onMessageEnd; none
                                          // is ever set
        /* 0x0FC */ Proc mOnMessageEnd;
        /* 0x108 */ Proc mOnAnswer;
        /* 0x114 */ Proc mOnRequestEnd;
        /* 0x120 */ int mMsgKind;  // MsgKind_e
        /* 0x124 */ u8 mLampTaken; // the magic lamp was handed over
    }; // size 0x128

    dAcNpcSpYuutaro_c() : mWispTalk(this), mLampAsked(0) {
    }
    virtual ~dAcNpcSpYuutaro_c() {
    } // weak
    virtual void onDemoCmd(int cmd, u8 member);    // 0x1D0
    virtual bool canInteract(dDemoActor_c *actor); // 0x280 (+ a dead blr)
    void changeState(int state);                   // 0x2A4
    virtual int preCreate();                       // 0x2DC
    virtual int create();                          // 0x328
    virtual int doDelete();                        // 0x43C
    virtual int vtB0();                            // 0x468
    void initGuide();                              // 0x4B0
    void stepCall();                               // 0x5C8
    void stepLead();                               // 0x630
    void stepWrongWay();                           // 0x6DC
    void stepAppear();                             // 0x768
    void stepShown();                              // 0x910
    void stepDisappear();                          // 0x990
    void stepHidden();                             // 0xB1C
    void executeGuide();                           // 0xBB0
    void initLook();                               // 0xC44
    void stepLook();                               // 0xCA8
    void executeLook();                            // 0xD08
    void initTalk();                               // 0xD48
    void stepTalkAppear();                         // 0xE48
    void stepTalk();                               // 0xFF8
    void stepTalkDisappear();                      // 0x1080
    void executeTalk();                            // 0x1214
    virtual int getEarType();                      // 0x18DC

    // Visible (with a shadow, pushes and is pushed) or not.
    void show() {
        mObjc.mType |= dObjc::TYPE_NPC;
        mIsActive = 1;
        mDrawShadow = 1;
    }
    void hide() {
        mObjc.mType &= ~dObjc::TYPE_NPC;
        mIsActive = 0;
        mDrawShadow = 0;
    }
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
    // Turn to the talk partner (initTalk / stepTalkAppear).
    void turnTalk(mAng angle, BOOL noCamera) {
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, noCamera);
    }

    /* 0x1C7C */ int mState;
    /* 0x1C80 */ u8 mStep;
    /* 0x1C84 */ talk_c mWispTalk;
    /* 0x1DAC */ int mGuideStep; // GuideStep_e
    /* 0x1DB0 */ u8 _1DB0[0xC];
    /* 0x1DBC */ u8 mLampAsked; // asked for the lamp during this appearance
}; // size 0x1DC0

#endif
