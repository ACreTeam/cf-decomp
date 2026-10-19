#ifndef D_A_NPC_SP_MYSTERYCAT_HPP
#define D_A_NPC_SP_MYSTERYCAT_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_state.hpp>

// Blanca, the mysterious cat (REL d_a_npc_sp_mysterycatNP, profile
// A_NPC_SP_MYSTERYCAT, SP_NPC_MASK_CAT). Her face is the design relayed
// between friend towns (dSaveExtra_c::mDesignBoard): she asks the player to
// draw her a new one (the design canvas, recept_c::reqMenu32), wanders around
// the town and hides when no player sees her.
//
// Class names from RTTI ("dAcNpcSpMysterycat_c" and its talk_c); mode names
// from the original CP932 mode labels; everything else is named from behaviour
// and the npc_SP/NPC_mysterycat messages.
class dAcNpcSpMysterycat_c : public dAcNpcSp_c,
                             public dState::mode_c<dAcNpcSpMysterycat_c> {
  public:
    // dAcNpc_c::action_c request priorities.
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    // The modes (the table in create), in table order.
    enum {
        MODE_WAIT,        // "待機" (first): starts wandering
        MODE_DEMO_WAIT,   // "デモ待機": turn to the player (never entered)
        MODE_TALK,        // "会話"
        MODE_WANDER_WALK, // "うろうろ歩き"
        MODE_HIDDEN,      // "非表示" (out of every player's view)
        MODE_NUM
    };

    // The message receptor (RTTI "dAcNpcSpMysterycat_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c, public dMsgRcptExp_c<talk_c> {
      public:
        typedef dState::base_c<talk_c>::Method Method;

        // npc_SP/NPC_mysterycat message codes, named from the English text.
        // Each request (MSG_*_ASK) is answered by the next codes: refused,
        // agreed (END_REQ_DRAW_FACE: the canvas), drawn (code + 1 after the
        // canvas), canvas cancelled.
        enum {
            MSG_FIRST_ASK = 0x01,    // "Do you think my face says "<style>"?"
            MSG_FIRST_DRAW = 0x04,   // "OK then, work your magic!"
            MSG_FIRST_CANCEL = 0x07, // "I think you promised to draw me a
                                     // new face, bucko!"
            MSG_CRITICIZED_DRAW = 0x0B,   // "draw me up a face that shows how
                                          // <style> I am!" (the asked style)
            MSG_CRITICIZED_CANCEL = 0x0E, // "Wait just one second! We had a
                                          // deal!"
            MSG_NO_FACE = 0x0F,        // "I don't really have a face right now"
            MSG_NO_FACE_DRAW = 0x11,   // "my face is your canvas"
            MSG_NO_FACE_CANCEL = 0x14, // "You were supposed to draw me a new
                                       // face!"
            MSG_MY_FACE_BY_YOU = 0x15, // "My face has a real <style> vibe"
            MSG_MY_FACE_BY_OTHER = 0x16, // "How does my face look? That
                                         // <name> drew it for me."
            MSG_SICK_OF_FACE = 0x19,     // "I am SO sick of this mug of mine!"
            MSG_SICK_DRAW = 0x1B,        // "OK, go to town!"
            MSG_SICK_CANCEL = 0x1D,      // "Oh, come on! YOU PROMISED!"
        };
        // onMessageEnd kinds (BMG entry +0xA).
        enum {
            END_REQ_DRAW_FACE = 0x10,  // open the design canvas
            END_REQ_11 = 0x11,         // (no messages use it)
            END_REQ_CLEAR_FACE = 0x12, // (no messages use it) blank face
        };
        // Message words.
        enum {
            WORD_STYLE = 2,        // her design's style (STR_Unit)
            WORD_RANDOM_STYLE = 3, // another random style
        };

        talk_c(dAcNpcSpMysterycat_c *npc) : mpOwner(npc) {
            init();
        }
        virtual void onMessageStart(int arg);     // 0x120C
        virtual void onMessageEnd(int kind);      // 0x12FC
        virtual void onAnswer(int arg);           // 0x1384
        virtual void init();                      // 0x1040
        virtual void preExecute();                // 0x1388
        virtual void onTalkEnd();                 // 0x138C
        virtual void getMsgInfo(msgInfo_s *info); // 0x10D0
        virtual void onRequestEnd(int kind);      // 0x1390
        void start(dActor_c *target);             // 0x1088
        void pickRandomStyle();                   // 0x12A8
        void onDesignMenuEnd();                   // 0x1428

        /* 0x0EC dMsgRcptExp_c<talk_c> */
        /* 0x128 */ dAcNpcSpMysterycat_c *mpOwner;
        /* 0x12C */ int mRandomStyle; // another style than her design's
        /* 0x130 */ int mStyle;       // her design's style (onMessageStart)
    }; // size 0x134

    dAcNpcSpMysterycat_c() : mMysteryTalk(this), mTimer(0) {
    }
    virtual ~dAcNpcSpMysterycat_c() {
    } // weak
    virtual bool canInteract(dDemoActor_c *actor); // 0x20C
    virtual void onDemoCmd(int cmd, u8 member);    // 0x258
    virtual int preCreate();                       // 0x650
    virtual int create();                          // 0x69C
    virtual int doDelete();                        // 0x848
    virtual int vtB0();                            // 0x89C
    void updateFaceTex();                          // 0x904
    virtual int getEarType();                      // 0x15E8

    // ---- modes ----
    void initWait();          // 0xA10
    void executeWait();       // 0xA24
    void initDemoWait();      // 0xB80
    void executeDemoWait();   // 0xC14
    void initTalk();          // 0xC44
    void executeTalk();       // 0xCE4
    void initWanderWalk();    // 0xD24
    void executeWanderWalk(); // 0xD28
    void initHidden();        // 0xEB8
    void executeHidden();     // 0xECC

    // initTalk: the angle to the talk partner and the turn speed.
    mAng getTalkAngle() const {
        dActor_c *target = mMysteryTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C dState::mode_c<dAcNpcSpMysterycat_c> */
    /* 0x1CA0 */ talk_c mMysteryTalk;
    /* 0x1DD4 */ int mTimer; // counts down to 0 (vtB0); never set otherwise
    /* 0x1DD8 */ dItem::resLoader_c mFaceRes; // her face: the board design
}; // size 0x1E48

#endif
