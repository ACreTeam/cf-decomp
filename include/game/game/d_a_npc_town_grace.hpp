#ifndef D_A_NPC_TOWN_GRACE_HPP
#define D_A_NPC_TOWN_GRACE_HPP

// The guest villager in Gracie's shop (REL d_a_npc_town_graceNP). Class name
// from RTTI; the controller and its state names are inferred (the wander code
// is d_a_npc_in's idle state without the room bounds).

#include <game/game/d_a_npc_town.hpp>

class dAcNpcTownGrace_c;
namespace dANpcTownGrace {
enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
enum {
    COLLISION_WEIGHT = 0x80,
    BLOCKED_FRAME_LIMIT = 120,
    WANDER_ROLL_COUNT = 5,
    WANDER_MOVE_ROLL_COUNT = 3,
};

class State_c {
  public:
    enum State_e {
        STATE_IDLE,
        STATE_TALK,
        STATE_LOOK,
        STATE_COUNT,
        STATE_NONE = STATE_COUNT
    };
    enum Step_e { STEP_START, STEP_DONE };
    // getSidePos results consumed by avoidObstacles.
    enum SideResult_e { SIDE_CLEAR, SIDE_BLOCKED, SIDE_DETOUR };

    typedef void (State_c::*Proc)(dAcNpcTownGrace_c *);
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };
    State_c();                                      // .text 0x414
    ~State_c();                                     // 0x438
    void change(dAcNpcTownGrace_c *npc, u32 state); // 0x478
    void execute(dAcNpcTownGrace_c *npc);           // 0x4C8
    void initIdle(dAcNpcTownGrace_c *npc);          // 0x510
    BOOL avoidObstacles(dAcNpcTownGrace_c *npc);    // 0x59C
    BOOL stopIfBlocked(dAcNpcTownGrace_c *npc);     // 0x65C
    BOOL checkMove(dAcNpcTownGrace_c *npc);         // 0x6FC
    void executeIdle(dAcNpcTownGrace_c *npc);       // 0x778
    void initTalk(dAcNpcTownGrace_c *npc);          // 0xA84
    void stepTalk(dAcNpcTownGrace_c *npc);          // 0xB18
    void executeTalk(dAcNpcTownGrace_c *npc);       // 0xB70
    void initLook(dAcNpcTownGrace_c *npc);          // 0xBB0
    void stepLook(dAcNpcTownGrace_c *npc);          // 0xC10
    void executeLook(dAcNpcTownGrace_c *npc);       // 0xC74

    /* 0x00 */ const State_s *mpState;
    /* 0x04 */ u32 mState;
    /* 0x08 */ u32 mWaitTimer; // idle: frames until the next wander roll
    /* 0x0C */ u32 mLookTimer; // idle: frames until the next look-around /
                               // look-at-player switch
    /* 0x10 */ u8 mStep;
    /* 0x14 */ u32 mBlockedTimer; // frames walking into a wall
}; // size 0x18
} // namespace dANpcTownGrace

class dAcNpcTownGrace_c : public dAcNpcTown_c {
  public:
    friend class dANpcTownGrace::State_c;

    virtual ~dAcNpcTownGrace_c() {
    }                                              // 0xD08
    virtual bool canInteract(dDemoActor_c *actor); // 0x1D8
    virtual void onDemoCmd(int cmd, u8 member);    // 0x240
    virtual int preCreate();                       // 0x318
    virtual int create();                          // 0x344
    virtual int doDelete();                        // 0x3BC
    virtual int vtB0();                            // 0x3E8

    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }
    mAng getTalkAngle() const {
        dActor_c *target = mNmlTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }

    /* 0x219C */ dANpcTownGrace::State_c mState;
}; // size 0x21B4

#endif
