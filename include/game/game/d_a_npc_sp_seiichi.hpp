#ifndef D_A_NPC_SP_SEIICHI_HPP
#define D_A_NPC_SP_SEIICHI_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_item.hpp>

// Wendell, the hungry walrus (REL d_a_npc_sp_seiichiNP, profile
// A_NPC_SP_SEIICHI). He wanders around town on his visitor days
// (dEvent::isVisitorHere(VISITOR_WENDELL)) and asks for food; for food he
// likes he gives one of his 52 wallpaper designs (items ITEM_IDX_GRASS +
// design, KIND_DSN_DATA_SEIICHI) into one of the player's design slots. After
// a new distributed design (dSaveDistPattern_c) arrived, he also comes to the
// player right at the start of play to hand over that design.
//
// Class names from RTTI ("dAcNpcSpSeiichi_c", "dAcNpcSpSeiichi_c::talk_c");
// everything else is named from behaviour.
class dAcNpcSpSeiichi_c : public dAcNpcSp_c {
  public:
    // The states (l_states), in table order.
    enum State_e {
        STATE_ARRIVE, // speak to the player at the start of play (dist design)
        STATE_WALK,   // wander around town
        STATE_LOOK,   // turn to the player (demo start)
        STATE_TALK,   // talk to the player
        STATE_FETCH,  // the player walks over in front of him (speak start)
        STATE_COUNT
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef void (dAcNpcSpSeiichi_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // The message receptor: the food the player gives him and the design he
    // gives back.
    class talk_c : public dAcNpcSp_c::receptSp_c {
      public:
        typedef void (talk_c::*Proc)();

        talk_c(dAcNpcSpSeiichi_c *npc);           // 0x98C
        virtual ~talk_c();                        // 0x9F8
        virtual void onMessageStart(int arg);     // 0xE50
        virtual void onMessageEnd(int kind);      // 0xE54
        virtual void onAnswer(int arg);           // 0xFA4
        virtual void init();                      // 0xA50
        virtual void preExecute();                // 0x1050
        virtual void onTalkEnd();                 // 0x10A0
        virtual void getMsgInfo(msgInfo_s *info); // 0xE38
        virtual void onRequestEnd(int kind);      // 0x10A4
        void start(dActor_c *target);             // 0xAEC

        // ---- message / request hooks ----
        void openFoodSelect(); // 0x1120: the pocket menu for the food
        void onFoodSelected(); // 0x11A0
        void takeFood();       // 0x1234
        void tasteFood();      // 0x1250: his reaction and the design
        void eatFood();        // 0x1D28
        void setForceNext();   // 0x1D30
        void returnFood();     // 0x1D40
        void pickUpFood();     // 0x1D44: back into the player's pockets
        void openSlotSelect(); // 0x1D88: the design slot menu
        void onSlotSelected(); // 0x1D90: copy the design there

        /* 0x0EA */ u16 mMsgNo; // the message start() picked
        /* 0x0EC */ dAcNpcSpSeiichi_c *mpOwner;
        /* 0x0F0 */ Proc mOnMessageEnd; // one-shot hooks
        /* 0x0FC */ Proc mOnAnswer;
        /* 0x108 */ Proc mOnRequestEnd;
        /* 0x114 */ Proc mNextOnRequestEnd; // mOnRequestEnd after the next one
        /* 0x120 */ dItem::Item mFood;      // the item the player gave
        /* 0x122 */ u8 _122;
        /* 0x123 */ u8 mWaitEquip; // the new design is put on the player first
    }; // size 0x124

    dAcNpcSpSeiichi_c() : mWendellTalk(this) {
    }
    virtual ~dAcNpcSpSeiichi_c() {
    } // weak
    virtual void onDemoCmd(int cmd, u8 member); // 0x1D4
    void changeState(int state);                // 0x2A4
    virtual int preCreate();                    // 0x2DC
    virtual int create();                       // 0x328
    virtual int doDelete();                     // 0x4D8
    virtual int vtB0();                         // 0x52C

    // ---- states ----
    void initArrive();    // 0x610
    void executeArrive(); // 0x658
    void initWalk();      // 0x660
    void executeWalk();   // 0x664
    void initLook();      // 0x678
    void stepLook();      // 0x708
    void executeLook();   // 0x768
    void initTalk();      // 0x7A8
    void stepTalk();      // 0x840
    void executeTalk();   // 0x88C
    void initFetch();     // 0x8CC
    void executeFetch();  // 0x938

    mAng getTalkAngle() const {
        dActor_c *target = mWendellTalk.mpActActor;
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
    /* 0x1C84 */ talk_c mWendellTalk;
    /* 0x1DA8 */ dItem::resLoader_c mDesignLoader; // the design to give
    /* 0x1E18 */ u16 mDesignItem; // its item index (vtB0 loads it), 0 = none
    /* 0x1E1C */ u32 _1E1C;       // only cleared in create
    /* 0x1E20 */ u8 mArriving;    // STATE_ARRIVE until the player came over
    /* 0x1E21 */ u8 mDistDesign;  // here for the distributed design
}; // size 0x1E24

#endif
