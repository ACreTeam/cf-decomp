#ifndef D_A_NPC_VISIT_PLAYER_HOUSE_HPP
#define D_A_NPC_VISIT_PLAYER_HOUSE_HPP

// A villager visiting the player's house (REL d_a_npc_visit_player_houseNP):
// the appointment quest QUEST_KIND_APPOINTMENT_1 (d_npc_talk_quest_q08). The
// class name is from RTTI; the controller and its state names are inferred from
// the code and the q08 talk procedure sets it uses.

#include <game/game/d_a_npc_nml.hpp>

class dAcNpcVisitPlayerHouse_c;
namespace dANpcVisitPlayerHouse {
enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
enum {
    BLOCKED_FRAME_LIMIT = 120,
    WANDER_ROLL_COUNT = 5,
    WANDER_MOVE_ROLL_COUNT = 3,
    // look around / at the player / ahead: each roll is a coin flip
    LOOK_ROLL_COUNT = 4,
    // frames until the next look change: at least this, plus up to cWaitRange
    LOOK_WAIT_BASE = 120,
    // from the knock until the talk at the door
    KNOCK_FRAMES = 150,
    // mTimer value at which the room sound restarts (the door opened)
    KNOCK_ROOM_SE_FRAME = 98,
    // the villager no longer comes this long after the meeting time
    MEET_LATE_MINUTES = 15,
    // the appointment is over this long after the meeting time
    MEET_EXPIRE_HOURS = 1,
    // scene exit of the player's house
    EXIT_FRONT_DOOR = 0,
    // scene exit back to the setReturnExitHere position
    EXIT_RETURN = 4,
    EXIT_WIPE = 2,
};
// setStayTime: talks and frames until the villager goes home.
enum {
    STAY_TALK_RANGE = 3,
    // after a reload
    STAY_TALK_BASE = 3,
    // after arriving
    FIRST_STAY_TALK_BASE = 7,
    STAY_TIME_RANGE = 7200,
    // after a reload
    STAY_TIME_BASE = 7200,
    // after arriving
    FIRST_STAY_TIME_BASE = 10800,
    // before the first talk
    FIRST_STAY_FRAMES = 1800,
};
// The player is still at the door within this distance of l_walkTargetPos.
#define VISIT_DOOR_NEAR_DIST 48.0f

class State_c {
  public:
    enum State_e {
        STATE_IDLE,    // walk around the room
        STATE_WAIT,    // hidden until the meeting time
        STATE_GONE,    // hidden: the meeting time passed
        STATE_CALL,    // hidden at the door: call the player
                       // (TALK_PROC_VISIT_CALL)
        STATE_ARRIVE,  // knock and come in, then the first talk
        STATE_TALK,    // talk to the player
        STATE_LOOK,    // turn to the player
        STATE_SEE_OFF, // talk, then the player leaves the house
        STATE_LEAVE,   // talk, then the villager takes the player outside
        STATE_PLAYER_LEAVING, // the player walks to the exit: ask to stay
                              // (TALK_PROC_VISIT_WAIT)
        STATE_GO_HOME, // the visit is over: say goodbye (TALK_PROC_VISIT_BACK)
        STATE_COUNT,
        STATE_NONE = STATE_COUNT
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum ArriveStep_e { ARRIVE_STEP_KNOCK, ARRIVE_STEP_TALK, ARRIVE_STEP_DONE };
    // getSidePos results consumed by the obstacle handler.
    enum SideResult_e { SIDE_CLEAR, SIDE_BLOCKED, SIDE_DETOUR };

    typedef void (State_c::*Proc)(dAcNpcVisitPlayerHouse_c *npc);
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };
    State_c();                                             // .text 0x694
    ~State_c();                                            // 0x6D4
    void change(dAcNpcVisitPlayerHouse_c *npc, u32 state); // 0x714
    void execute(dAcNpcVisitPlayerHouse_c *npc);           // 0x764
    void setTalk(u32 resume, u32 talk,
                 const dAcNpcNml_c::talk_c::procSet_s *set);  // 0x81C
    BOOL isStayOver();                                        // 0x82C
    void setStayTime(BOOL resumed);                           // 0x8BC
    void initIdle(dAcNpcVisitPlayerHouse_c *npc);             // 0x938
    BOOL avoidObstacles(dAcNpcVisitPlayerHouse_c *npc);       // 0x9D4
    BOOL stopIfBlocked(dAcNpcVisitPlayerHouse_c *npc);        // 0xA94
    BOOL checkPlayerLeaving(dAcNpcVisitPlayerHouse_c *npc);   // 0xB34
    BOOL checkGoHome(dAcNpcVisitPlayerHouse_c *npc);          // 0xBC4
    BOOL checkIdle(dAcNpcVisitPlayerHouse_c *npc);            // 0xC44
    void executeIdle(dAcNpcVisitPlayerHouse_c *npc);          // 0xCF8
    void initWait(dAcNpcVisitPlayerHouse_c *npc);             // 0x102C
    void executeWait(dAcNpcVisitPlayerHouse_c *npc);          // 0x1070
    void initGone(dAcNpcVisitPlayerHouse_c *npc);             // 0x12E0
    void initCall(dAcNpcVisitPlayerHouse_c *npc);             // 0x1324
    void executeCall(dAcNpcVisitPlayerHouse_c *npc);          // 0x1384
    BOOL isPlayerAwayFromDoor();                              // 0x157C
    void initArrive(dAcNpcVisitPlayerHouse_c *npc);           // 0x1614
    void stepArriveKnock(dAcNpcVisitPlayerHouse_c *npc);      // 0x169C
    void stepArriveTalk(dAcNpcVisitPlayerHouse_c *npc);       // 0x16F8
    void executeArrive(dAcNpcVisitPlayerHouse_c *npc);        // 0x177C
    void initTalk(dAcNpcVisitPlayerHouse_c *npc);             // 0x17BC
    void stepTalk(dAcNpcVisitPlayerHouse_c *npc);             // 0x1828
    void executeTalk(dAcNpcVisitPlayerHouse_c *npc);          // 0x18A8
    void initLook(dAcNpcVisitPlayerHouse_c *npc);             // 0x18E8
    void stepLook(dAcNpcVisitPlayerHouse_c *npc);             // 0x1948
    void executeLook(dAcNpcVisitPlayerHouse_c *npc);          // 0x19AC
    void initSeeOff(dAcNpcVisitPlayerHouse_c *npc);           // 0x19EC
    void stepSeeOff(dAcNpcVisitPlayerHouse_c *npc);           // 0x1A58
    void executeSeeOff(dAcNpcVisitPlayerHouse_c *npc);        // 0x1AB4
    void initLeave(dAcNpcVisitPlayerHouse_c *npc);            // 0x1AF4
    void stepLeave(dAcNpcVisitPlayerHouse_c *npc);            // 0x1B60
    void executeLeave(dAcNpcVisitPlayerHouse_c *npc);         // 0x1BCC
    void initPlayerLeaving(dAcNpcVisitPlayerHouse_c *npc);    // 0x1C0C
    void executePlayerLeaving(dAcNpcVisitPlayerHouse_c *npc); // 0x1CC0
    void initGoHome(dAcNpcVisitPlayerHouse_c *npc);           // 0x1D30
    void executeGoHome(dAcNpcVisitPlayerHouse_c *npc);        // 0x1DE4

    /* 0x00 */ const State_s *mpState;
    /* 0x04 */ const dAcNpcNml_c::talk_c::procSet_s *mpTalk;
    /* 0x08 */ u32 mState;
    /* 0x0C */ u32 mResumeState; // entered on DEMO_CMD_START
    /* 0x10 */ u32 mTalkState;   // entered on DEMO_CMD_SPEAK, with mpTalk
    /* 0x14 */ u32 mTimer;
    /* 0x18 */ u32 mWaitTimer;
    /* 0x1C */ u32 mFirstStayTimer; // before the first talk: frames until the
                                    // villager goes home
    /* 0x20 */ u32 mBlockedTimer;
    /* 0x24 */ u32 mLookTimer;
    /* 0x28 */ u32
        mStayTimer; // after the first talk: frames until the villager goes home
    /* 0x2C */ u8 mTalkCount; // talks left before mStayTimer restarts
    /* 0x2D */ u8 mStep;
}; // size 0x30
} // namespace dANpcVisitPlayerHouse

// Global class name confirmed by RTTI.
class dAcNpcVisitPlayerHouse_c : public dAcNpcNml_c {
  public:
    friend class dANpcVisitPlayerHouse::State_c;
    virtual ~dAcNpcVisitPlayerHouse_c() {
    } // 0x1EA0
    virtual bool canInteract(dDemoActor_c *actor); // 0x1E4
    virtual void onDemoCmd(int cmd, u8 member);    // 0x260
    virtual int preCreate();                       // 0x3B4
    virtual int create();                          // 0x3E0
    virtual int doDelete();                        // 0x630
    virtual int vtB0();                            // 0x668

    /* 0x219C */ dANpcVisitPlayerHouse::State_c mState;
}; // size 0x21D0

#endif
