#ifndef D_A_NPC_SP_GATEKEEPER_B_HPP
#define D_A_NPC_SP_GATEKEEPER_B_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_state.hpp>

// Booker, the second gatekeeper at the town gate (REL
// d_a_npc_sp_gatekeeper_bNP, profile A_NPC_SP_GATEKEEPER_B). He tells who came
// through the gate last (the visitor npc of the day, or the last net
// visitor), runs the lost and found (dPoliceBox_c) and changes the town flag.
//
// Class names from RTTI ("dAcNpcSpGkB_c", "dAcNpcSpGkB_c::talk_c"); mode names
// from the original CP932 mode labels (the same as in dAcNpcSpGkA_c);
// everything else is named from behaviour and the npc_SP/NPC_gatekeeperB
// messages.
class dAcNpcSpGkB_c : public dAcNpcSp_c, public dState::mode_c<dAcNpcSpGkB_c> {
  public:
    // dAcNpc_c::action_c request priorities.
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef dState::base_c<dAcNpcSpGkB_c>::Method Method;

    // The modes (the table in create), in table order.
    enum {
        MODE_WAIT,        // "待機"
        MODE_DEMO_WAIT,   // "デモ待機"
        MODE_TALK,        // "会話"
        MODE_TURN_BACK,   // "初期角度に復帰" (back to the initial angle)
        MODE_REMOTE,      // "通信：リモート"
        MODE_REMOTE_TALK, // "通信：リモート会話"
    };

    // The message receptor (RTTI "dAcNpcSpGkB_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c, public dMsgRcptExp_c<talk_c> {
      public:
        typedef dState::base_c<talk_c>::Method Method;

        // npc_SP/NPC_gatekeeperB message codes, named from the English text.
        enum {
            MSG_FIRST_MEETING = 0x01, // "Um... Welcome, I guess..." (a
                                      // visitor's first talk)
            MSG_HELLO = 0x02,         // "H-hello..." (a visitor)
            MSG_MENU = 0x04, // "What can I do for you?" What's new? / Something
                             // lost? / Change flag. / Where am I? / Nothing.
            MSG_CHANGE_FLAG = 0x0A,    // "Pl-please take your time..."
            MSG_FLAG_CANCELLED = 0x0B, // "You changed your mind then?"
            MSG_FLAG_CHANGED = 0x0C,   // "I'll just swap the current design
                                       // with this one."
            MSG_LOST_AND_FOUND_EMPTY = 0x0D, // "the lost and found is, uh,
                                             // empty."
            MSG_LOST_AND_FOUND = 0x0E,       // "we currently have some items
                                             // in the lost and found"
            MSG_LOST_ITEM_CANCELLED = 0x0F,  // "so...you changed your mind?"
            MSG_LOST_ITEM_TAKEN = 0x10,      // "So, you're going to take ...?"
            MSG_LOST_ITEMS_TAKEN = 0x11,     // "You've, uh, claimed quite a few
                                             // items there..."
            MSG_NO_VISITOR = 0x12,      // "I didn't notice anyone go through
                                        // here..."
            MSG_LAST_VISITOR = 0x13,    // "The last one to come in through here
                                        // was... ..." (net play)
            MSG_VISITOR_JOAN = 0x14,    // "the turnip peddler, Joan"
            MSG_VISITOR_SAHARAH = 0x15, // "Saharah went by"
            MSG_VISITOR_LOST_KITTEN = 0x16,    // "a little kitten on the verge
                                               // of tears"
            MSG_VISITOR_MOTHER_CAT = 0x17,     // "a mother cat asking about a
                                               // lost kitten"
            MSG_VISITOR_WENDELL = 0x18,        // "the famous artist, Wendell"
            MSG_VISITOR_KK_SLIDER = 0x19,      // "Mr. K.K. Slider came through"
            MSG_VISITOR_MYSTERIOUS_CAT = 0x1A, // "a mysterious cat"
            MSG_WELCOME = 0x1C,      // "Um... Welcome, I guess..." (a resident
                                     // without a house)
            MSG_VISITOR_CHIP = 0x1F, // (Chip: the tourney's state picks the
                                     // text)
            MSG_VISITOR_NAT = 0x20,  // (the bug master: the Bug-Off's state
                                     // picks the text)
            MSG_VISITOR_ZIPPER = 0x21, // "Zipper T. Bunny"
            MSG_VISITOR_PAVE = 0x22,   // "Pave came dancing through here"
        };

        // MSG_MENU's choices handled in onAnswer.
        enum {
            ANSWER_WHATS_NEW,      // "What's new?"
            ANSWER_SOMETHING_LOST, // "Something lost?"
        };

        // How many items the player took from the lost and found
        // (dMenuMng_c::getLostTakenNum after reqMenu1B).
        enum {
            LOST_ITEMS_NONE,
            LOST_ITEMS_ONE,
        };

        talk_c(dAcNpcSpGkB_c *npc); // 0x14B8
        virtual ~talk_c() {
        } // weak
        virtual void onMessageEnd(int kind);      // 0x1658
        virtual void onAnswer(int answer);        // 0x17AC
        virtual void init();                      // 0x1528
        virtual void preExecute();                // 0x1650
        virtual void onTalkEnd();                 // 0x1654
        virtual void getMsgInfo(msgInfo_s *info); // 0x1638
        virtual void onRequestEnd(int kind);      // 0x1ACC

        void start(dActor_c *target); // 0x1578

        // ---- message handlers (onMessageEnd / onAnswer tables) ----
        void openLostAndFoundMenu();  // 0x1714: reqMenu1B, end
                                      // onLostAndFoundMenuEnd
        void openFlagMenu();          // 0x1760: reqMenu16, end onFlagMenuEnd
        void answerWhatsNew();        // 0x187C: who came through the gate
        void answerSomethingLost();   // 0x1A58: the lost and found
        void onLostAndFoundMenuEnd(); // 0x1B64: request-end method of
                                      // openLostAndFoundMenu
        void onFlagMenuEnd();         // 0x1BE4: request-end method of
                                      // openFlagMenu

        /* 0x0EC dMsgRcptExp_c<talk_c> */
        /* 0x128 */ u16 mMsgCode; // the message start() picked
        /* 0x12C */ dAcNpcSpGkB_c *mpOwner;
        /* 0x130 */ dEquip_c mEquip; // onFlagMenuEnd: the player's new look
    }; // size 0x138

    dAcNpcSpGkB_c() : mTalkGk(this) {
    }
    virtual ~dAcNpcSpGkB_c() {
    } // weak
    virtual int create();                       // 0xA2C
    virtual int preCreate();                    // 0x980
    virtual int doDelete();                     // 0xD4C
    virtual void onDemoCmd(int cmd, u8 member); // 0x1D4
    virtual int getEarType(); // weak 0x1C78 (inline in the .cpp):
                              // NPC_EAR_TYPE_BOOKER
    virtual int vtB0();       // 0xD78

    // ---- modes ----
    void initWait();          // 0xDCC
    void executeWait();       // 0xDE0
    void initDemoWait();      // 0xDE4
    void executeDemoWait();   // 0xDE8
    void initTalk();          // 0xDEC
    void executeTalk();       // 0xE64
    void initTurnBack();      // 0xEA4
    void executeTurnBack();   // 0xEEC
    void initRemote();        // 0x1060
    void executeRemote();     // 0x1064
    void initRemoteTalk();    // 0x1338
    void executeRemoteTalk(); // 0x133C

    // initTalk: the angle to the talk partner and the turn speed.
    mAng getTalkAngle() const {
        dActor_c *target = mTalkGk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C dState::mode_c<dAcNpcSpGkB_c> */
    /* 0x1CA0 */ talk_c mTalkGk;
}; // size 0x1DD8

#endif
