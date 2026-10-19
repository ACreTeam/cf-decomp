#ifndef D_A_NPC_SP_MISSING_MOTHER_HPP
#define D_A_NPC_SP_MISSING_MOTHER_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_state.hpp>

// Katie's mother (REL d_a_npc_sp_missing_motherNP, profile
// A_NPC_SP_MISSING_MOTHER, item key SP_NPC_MISSING_MOTHER). She wanders around
// a town looking for her daughter, and waits for her in the checkpoint scene
// of the reunion demo (driven by d_a_npc_sp_missing_child).
//
// Class names from RTTI ("dAcNpcSpMissingMother_c" and its talk_c); mode names
// from the original CP932 state labels; everything else is named from
// behaviour.
class dAcNpcSpMissingMother_c : public dAcNpcSp_c,
                                public dState::mode_c<dAcNpcSpMissingMother_c> {
  public:
    // dAcNpc_c::action_c request priorities.
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef dState::base_c<dAcNpcSpMissingMother_c>::Method Method;

    // The modes (the table in create), in table order.
    enum {
        MODE_WAIT,        // "待機"
        MODE_DEMO_WAIT,   // "デモ待機"
        MODE_TALK,        // "会話"
        MODE_WANDER_WALK, // "うろうろ歩き"
        MODE_HIDDEN,      // "非表示" (out of every player's view)
        MODE_REUNION,     // "再会デモ"
    };

    // The message receptor (RTTI "dAcNpcSpMissingMother_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c, public dMsgRcptExp_c<talk_c> {
      public:
        // npc_SP/NPC_maigo_mother message codes, named from the English text.
        enum {
            MSG_FIRST_MEETING = 0x01, // "Oh, dear. My daughter has wandered
                                      // off..."
            MSG_FIRST_MEETING_AGAIN = 0x02, // the same, after Katie was brought
                                            // home once
            MSG_MEET = 0x03,                // later meetings
            MSG_MEET_AGAIN = 0x04,
        };

        talk_c(dAcNpcSpMissingMother_c *npc);     // 0xEC8
        virtual ~talk_c();                        // 0xF30
        virtual void onMessageStart(int arg);     // 0x10B0
        virtual void init();                      // 0xF88
        virtual void getMsgInfo(msgInfo_s *info); // 0xFD4
        void start(dActor_c *target);             // 0xF8C

        /* 0x0EC dMsgRcptExp_c<talk_c> */
        /* 0x128 */ dAcNpcSpMissingMother_c *mpOwner;
    }; // size 0x12C

    dAcNpcSpMissingMother_c() : mMotherTalk(this) {
    }
    virtual ~dAcNpcSpMissingMother_c() {
    } // weak
    virtual int create();                       // 0x228
    virtual int preCreate();                    // 0x1D4
    virtual int doDelete();                     // 0x440
    virtual void onDemoCmd(int cmd, u8 member); // 0x4C0
    virtual int getEarType();                   // 0x10F4
    virtual int vtB0();                         // 0x46C

    // ---- modes ----
    void initWait();          // 0x8B8
    void executeWait();       // 0x8CC
    void initWanderWalk();    // 0xA28
    void executeWanderWalk(); // 0xA2C
    void initHidden();        // 0xBBC
    void executeHidden();     // 0xBD0
    void initDemoWait();      // 0xD44
    void executeDemoWait();   // 0xDA0 (+ a dead blr at 0xDCC)
    void initTalk();          // 0xDD0
    void executeTalk();       // 0xE70
    void initReunion();       // 0xEB0
    void executeReunion();    // 0xEC4

    // initTalk: the angle to the talk partner and the turn speed.
    mAng getTalkAngle() const {
        dActor_c *target = mMotherTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C dState::mode_c<dAcNpcSpMissingMother_c> */
    /* 0x1CA0 */ talk_c mMotherTalk;
}; // size 0x1DCC

#endif
