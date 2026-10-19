#ifndef D_A_NPC_SP_KINUYO_HPP
#define D_A_NPC_SP_KINUYO_HPP

// Include order: the weak RTTI / vtables in .data follow reverse class
// completion (fBase_c and dActor_c last, then dItem::nameFashion_c and its
// bases, then the npc classes).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_private_data.hpp>

// Mabel at the counter of the Able Sisters (REL d_a_npc_sp_kinuyoNP, profile
// A_NPC_SP_KINUYO): she sells the clothes, accessories and umbrellas of the
// shop (the player may try them on first), copies designs between the
// player's list and the shop's display, makes Pro designs and stores designs.
// During the Gracie story she tells about her sister ("npc_SP/NPC_drama1").
// Sable (SP_NPC_SABLE) is her partner in the talks.
//
// Class names from RTTI ("dAcNpcSpKinuyo_c", "dAcNpcSpKinuyo_c::talk_c");
// everything else is named from behaviour. The states follow
// d_a_npc_sp_tanukichi (the other shopkeeper).
class dAcNpcSpKinuyo_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_WAIT,        // stand; turn to / walk over to the player
        STATE_TURN,        // turn to the player
        STATE_APPROACH,    // walk / run over to the player
        STATE_LOOK,        // turn to the player (demo start)
        STATE_TALK,        // talk to the player
        STATE_GREET,       // speak: welcome the player who came in
        STATE_SEE_OFF,     // speak: the player is at the (locked) exit
        STATE_MULTI_WAIT,  // wait while others are in town
        STATE_WATCH,       // the partner of a three-way talk (vt80)
        STATE_REMOTE,      // another console owns the npc
        STATE_REMOTE_TALK, // another console's player talks to the npc
        STATE_NUM
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpKinuyo_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: the greetings, the sale (and trying on) of the
    // pointed item, the design copies and the Pro designs.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        // The kinds of talk (mKind; onDemoCmd and the states).
        enum Kind_e {
            KIND_GREET,   // the player came in (STATE_GREET)
            KIND_SEE_OFF, // the player is at the exit (STATE_SEE_OFF)
            KIND_TALK,    // the player talks to the npc
            KIND_ITEM,    // the npc speaks about the item the player points at
        };

        // What the design menu does with the shop's design mDesignIdx
        // (mDesignMenu; the answers of MSG_DESIGN_*).
        enum DesignMenu_e {
            DESIGN_MENU_DISPLAY, // "Display mine!": copy the player's over it
            DESIGN_MENU_TAKE,    // "I want it!": copy it to the player's
            DESIGN_MENU_SWAP,    // "Switch it out."
            DESIGN_MENU_NONE
        };

        talk_c(dAcNpcSpKinuyo_c *npc);            // 0x137C
        virtual ~talk_c();                        // 0x1430
        virtual void onMessageStart(int arg);     // 0x1A60
        virtual void onMessageEnd(int kind);      // 0x1D0C
        virtual void onAnswer(int arg);           // 0x1EEC
        virtual void init();                      // 0x14A4
        virtual void preExecute();                // 0x2228
        virtual void onTalkEnd();                 // 0x222C
        virtual void getMsgInfo(msgInfo_s *info); // 0x1A14
        virtual void onRequestEnd(int kind);      // 0x2414
        virtual int getTurnFrame();               // 0x2B64
        void start(dActor_c *target);             // 0x1570

        BOOL buy();                             // 0x22C4
        static BOOL isDesignEquipped(u32 slot); // 0x26C8

        // ---- message / request hooks ----
        void openDesignMenu();     // 0x24BC (+ a dead blr at 0x24FC)
        void openStoreMenu();      // 0x2500
        void toggleTryOn();        // 0x2508
        void onDesignMenuEnd();    // 0x26D0
        void onStoreMenuEnd();     // 0x2864
        void refreshEquip();       // 0x2908
        void endTryOn();           // 0x294C
        void doNothing();          // 0x29B8
        void openProDesignMenu();  // 0x29BC
        void onProDesignMenuEnd(); // 0x29C4
        void tryOnAnswer();        // 0x2A70
        void startTryOn();         // 0x2AC8
        void finishTryOn();        // 0x2B10
        void doNothing2();         // 0x2B60

        void setPointedItem(const dItem::Item &item, int unitX, int unitZ) {
            mItem = item;
            mUnitX = unitX;
            mUnitZ = unitZ;
        }

        /* 0x0EA */ u16 mMsgNo; // the message start() picked
        /* 0x0EC */ dAcNpcSpKinuyo_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageStart; // one-shot hooks
        /* 0x0FC */ Proc mOnMessageEnd;
        /* 0x108 */ Proc mOnAnswer;
        /* 0x114 */ Proc mOnRequestEnd;
        /* 0x120 */ Proc mNextOnRequestEnd; // mOnRequestEnd after the next one
        /* 0x12C */ dEquip_c mEquipOrig;    // the player's own (try on)
        /* 0x134 */ dEquip_c mEquip;        // what the player wears now
        /* 0x13C */ int mKind;              // Kind_e
        /* 0x140 */ dItem::Item mItem;      // the item talked about
        /* 0x144 */ int mDesignIdx;         // the shop's design (TA_CLOTH)
        /* 0x148 */ int mUnitX;             // the pointed item's unit
        /* 0x14C */ int mUnitZ;
        /* 0x150 */ int mDesignMenu;   // DesignMenu_e
        /* 0x154 */ u32 mProSlot;      // the player's design made Pro
        /* 0x158 */ int mWearAnswer;   // the Pro design's first answer
        /* 0x15C */ u16 mDesignMsg;    // said after the design menu
        /* 0x15E */ u8 mExitRequested; // the scene change was requested
        /* 0x15F */ u8 mTrying;        // the player tries the item on
        /* 0x160 */ u8 mMiiMask;       // the player wore a Mii mask
        /* 0x161 */ u8 mBought;
    }; // size 0x164

    dAcNpcSpKinuyo_c() : mMabelTalk(this) {
    }
    virtual ~dAcNpcSpKinuyo_c() {
    } // weak
    virtual void onDemoCmd(int cmd, u8 member); // 0x1D8
    void changeState(int state);                // 0x3A8
    virtual int preCreate();                    // 0x3E0
    virtual int create();                       // 0x42C
    virtual int doDelete();                     // 0x5A8
    virtual int vtB0();                         // 0x5D4
    virtual int canSpeak(dDemoActor_c *actor);  // 0x61C
    virtual void vt80();      // 0x1318: three-way talk partner
    virtual void vt84();      // 0x1320: its end
    virtual int getEarType(); // 0x2B78

    // ---- states ----
    void initWait();          // 0x66C
    void executeWait();       // 0x680
    void initTurn();          // 0x7F0
    void executeTurn();       // 0x84C
    void initApproach();      // 0x9F0
    void executeApproach();   // 0xA74
    void initLook();          // 0xCAC
    void stepLook();          // 0xD10
    void executeLook();       // 0xD70
    void initTalk();          // 0xDB0
    void stepTalk();          // 0xE1C
    void executeTalk();       // 0xE8C
    void initGreet();         // 0xECC
    void executeGreet();      // 0xED0
    void initSeeOff();        // 0xF1C
    void executeSeeOff();     // 0xF20
    void initMultiWait();     // 0xF6C
    void executeMultiWait();  // 0xF80
    void initWatch();         // 0xFD8
    void executeWatch();      // 0x1070
    void initRemote();        // 0x1074
    void executeRemote();     // 0x1078
    void initRemoteTalk();    // 0x1140
    void executeRemoteTalk(); // 0x1144

    BOOL isSeeOffTime();     // 0x1198: the player is at the exit
    BOOL checkPointedItem(); // 0x11F0: the player points at a shop item
    BOOL isClosingTime();    // 0x127C
    BOOL isShopClosed();     // 0x12C4: never
    f32 getStopDist();       // 0x12E8: STATE_APPROACH stops within it
    f32 getMoveDist();       // 0x12F4: STATE_APPROACH starts / runs beyond it

    mAng getTalkAngle() const {
        dActor_c *target = mMabelTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    /* 0x1C7C */ int mState; // State_e
    /* 0x1C80 */ u8 mStep;
    /* 0x1C84 */ talk_c mMabelTalk;
    /* 0x1DE8 */ u8 mApproached; // walked over to the player once
    /* 0x1DE9 */ u8 mDrama;      // her part of the Gracie story is due
}; // size 0x1DEC

#endif
