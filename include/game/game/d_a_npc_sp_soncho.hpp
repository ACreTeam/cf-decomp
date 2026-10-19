#ifndef D_A_NPC_SP_SONCHO_HPP
#define D_A_NPC_SP_SONCHO_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_state.hpp>

// Tortimer dozing in front of the town hall for a new player (REL
// d_a_npc_sp_sonchoNP, profile A_NPC_SP_SONCHO). Until the player has a house
// (or once the part-time job's work clothes are on) he sleeps at the town hall
// (the "afm_owl_sleep" bubble at his head, the TTI_SLP animations); otherwise
// he stands as SP_NPC_SONCHO_EVENT. The first talk (npc_SP/NPC_soncho_0) is
// with Pelly / Phyllis as the partner.
//
// Class names from RTTI ("dAcNpcSpSoncho_c", "dAcNpcSpSoncho_c::talk_c"); mode
// names from the original CP932 labels; everything else is named from
// behaviour.
class dAcNpcSpSoncho_c : public dAcNpcSp_c,
                         public dState::mode_c<dAcNpcSpSoncho_c> {
  public:
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
    // MODE_DOZE1: the bubble animation loops this often before the nod.
    enum { DOZE_LOOPS = 10 };

    typedef dState::base_c<dAcNpcSpSoncho_c>::Method Method;

    // The modes (the table in create), in table order.
    enum {
        MODE_WAIT,      // "待機"
        MODE_DEMO_WAIT, // "デモ待機"
        MODE_TALK,      // "会話"
        MODE_DOZE1,     // "居眠り１": sleep, the bubble
        MODE_DOZE2,     // "居眠り２": the nodding animation
        MODE_DOZE3,     // "居眠り３"
    };

    // The message receptor (RTTI "dAcNpcSpSoncho_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        // npc_SP/NPC_soncho_0 message codes, named from the English text.
        enum {
            MSG_FIRST = 0x01, // "You! Whippersnapper! Who are you? And ...
                              // what is your favorite animal?"
            MSG_FIRST_VISITOR = 0x06, // "Eh?! You! I've seen you somewhere..."
            MSG_AGAIN = 0x0A,         // later talks (dispatcher)
        };
        // The answers of MSG_FIRST.
        enum { ANSWER_CAT, ANSWER_PELICAN, ANSWER_RACCOON, ANSWER_TURTLE };
        // Custom tags (onCustomTag).
        enum { CUSTOM_PARTNER_WAIT };

        talk_c(dAcNpcSpSoncho_c *npc); // 0x1244
        virtual ~talk_c() {
        } // weak 0x162C
        virtual void init();                      // 0x1298
        virtual void getMsgInfo(msgInfo_s *info); // 0x139C
        virtual void onAnswer(int arg);           // 0x13B4
        virtual void onCustomTag(int no);         // 0x1420

        void start(dActor_c *target); // 0x12CC

        /* 0x0EA */ u16 mMsgNo;
        /* 0x0EC */ dAcNpcSpSoncho_c *mpOwner;
    }; // size 0xF0

    dAcNpcSpSoncho_c() : mSonchoTalk(this) {
    }
    virtual ~dAcNpcSpSoncho_c() {
    } // weak 0x1684
    virtual int preCreate();                    // 0x204
    virtual int create();                       // 0x2B4
    virtual int doDelete();                     // 0x568
    virtual int vtB0();                         // 0x594
    virtual void onDemoCmd(int cmd, u8 member); // 0x5E8
    virtual void vt84();                        // 0xB1C
    virtual int getEarType();                   // 0x1488

    // ---- modes ----
    void initWait();        // 0xB44
    void executeWait();     // 0xB6C
    void initDemoWait();    // 0xB70
    void executeDemoWait(); // 0xB74
    void initTalk();        // 0xB78
    void executeTalk();     // 0xBDC
    void initDoze1();       // 0xC1C
    void executeDoze1();    // 0xC60
    void initDoze2();       // 0xF24
    void executeDoze2();    // 0xF4C
    void initDoze3();       // 0x10B4
    void executeDoze3();    // 0x10DC

    mAng getTalkAngle() const {
        dActor_c *target = mSonchoTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C dState::mode_c<dAcNpcSpSoncho_c> */
    /* 0x1CA0 */ talk_c mSonchoTalk;
    /* 0x1D90 */ u8 mDozeCount; // MODE_DOZE1 loops until MODE_DOZE2
    /* 0x1D94 */ dLevelEffect_c mSleepEffect; // "afm_owl_sleep"
}; // size 0x1E28

#endif
