#ifndef D_A_NPC_VISIT_NPC_HOUSE_HPP
#define D_A_NPC_VISIT_NPC_HOUSE_HPP

// The villager at home when the player visits on their invitation (REL
// d_a_npc_visit_npc_houseNP, profile A_NPC_VISIT_NPC_HOUSE): the appointment
// quest QUEST_KIND_APPOINTMENT_0 (d_npc_talk_quest_q09). The villager welcomes
// the player at the door, walks around the room, offers to sell the furniture
// the player points at once the trade was offered, and talks to the player
// when they walk to the exit. The class name is from RTTI; the controller and
// its state names are inferred from the code and the q09 talk procedure sets
// it uses.

#include <game/game/d_a_npc_nml.hpp>

class dAcNpcVisitNpcHouse_c;
namespace dANpcVisitNpcHouse {
enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
enum {
    BLOCKED_FRAME_LIMIT = 120,
    WANDER_ROLL_COUNT = 5,
    WANDER_MOVE_ROLL_COUNT = 3,
};

class State_c {
  public:
    enum State_e {
        STATE_IDLE,        // walk around the room
        STATE_OFF_SCREEN,  // wait until the villager is in view (not entered
                           // in this REL)
        STATE_TALK,        // talk to the player
        STATE_LOOK,        // turn to the player
        STATE_WELCOME,     // speak to the player at the door
                           // (TALK_PROC_INVITE_WELCOME)
        STATE_HOLD_PLAYER, // the player walked to the exit before the first
                           // talk: talk, then the player steps back
        STATE_SEE_OFF,     // the player walked to the exit after it: talk, then
                           // the player leaves
        STATE_COUNT,
        STATE_NONE = STATE_COUNT
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum HoldStep_e { HOLD_STEP_TALK, HOLD_STEP_MOVE, HOLD_STEP_DONE };
    // getSidePos results consumed by the obstacle handler.
    enum SideResult_e { SIDE_CLEAR, SIDE_BLOCKED, SIDE_DETOUR };

    typedef void (State_c::*Proc)(dAcNpcVisitNpcHouse_c *npc);
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };
    State_c();                                          // 0x530
    ~State_c();                                         // 0x564
    void change(dAcNpcVisitNpcHouse_c *npc, u32 state); // 0x5A4
    void execute(dAcNpcVisitNpcHouse_c *npc);           // 0x5F4
    void setTalk(u32 resume, u32 talk,
                 const dAcNpcNml_c::talk_c::procSet_s *set); // 0x650
    void initIdle(dAcNpcVisitNpcHouse_c *npc);               // 0x660
    BOOL avoidObstacles(dAcNpcVisitNpcHouse_c *npc);         // 0x6F0
    BOOL stopIfBlocked(dAcNpcVisitNpcHouse_c *npc);          // 0x7B0
    BOOL checkMove(dAcNpcVisitNpcHouse_c *npc);              // 0x850
    void executeIdle(dAcNpcVisitNpcHouse_c *npc);            // 0x8CC
    void initOffScreen(dAcNpcVisitNpcHouse_c *npc);          // 0xD90
    void executeOffScreen(dAcNpcVisitNpcHouse_c *npc);       // 0xDA4
    void initTalk(dAcNpcVisitNpcHouse_c *npc);               // 0xDF8
    void stepTalk(dAcNpcVisitNpcHouse_c *npc);               // 0xE6C
    void executeTalk(dAcNpcVisitNpcHouse_c *npc);            // 0xEC4
    void initLook(dAcNpcVisitNpcHouse_c *npc);               // 0xF04
    void stepLook(dAcNpcVisitNpcHouse_c *npc);               // 0xF64
    void executeLook(dAcNpcVisitNpcHouse_c *npc);            // 0xFC8
    void initWelcome(dAcNpcVisitNpcHouse_c *npc);            // 0x1008
    void stepWelcome(dAcNpcVisitNpcHouse_c *npc);            // 0x101C
    void executeWelcome(dAcNpcVisitNpcHouse_c *npc);         // 0x1074
    void initHoldPlayer(dAcNpcVisitNpcHouse_c *npc);         // 0x10B4
    void stepHoldTalk(dAcNpcVisitNpcHouse_c *npc);           // 0x1120
    void stepHoldMove(dAcNpcVisitNpcHouse_c *npc);           // 0x11AC
    void executeHoldPlayer(dAcNpcVisitNpcHouse_c *npc);      // 0x1204
    void initSeeOff(dAcNpcVisitNpcHouse_c *npc);             // 0x1244
    void stepSeeOff(dAcNpcVisitNpcHouse_c *npc);             // 0x12B0
    void executeSeeOff(dAcNpcVisitNpcHouse_c *npc);          // 0x1308

    /* 0x00 */ const State_s *mpState;
    /* 0x04 */ const dAcNpcNml_c::talk_c::procSet_s *mpTalk;
    /* 0x08 */ u32 mState;
    /* 0x0C */ u32 mResumeState; // entered on DEMO_CMD_START
    /* 0x10 */ u32 mTalkState; // entered on DEMO_CMD_TALK / SPEAK, with mpTalk
    /* 0x14 */ u32 mTimer;
    /* 0x18 */ u32 mWaitTimer;
    /* 0x1C */ u32 mBlockedTimer;
    /* 0x20 */ u8 _20; // only cleared
    /* 0x21 */ u8 mStep;
    /* 0x22 */ u8 _22;
    /* 0x23 */ u8 mSpeakRequested; // dDemo::requestSpeak pending / done
}; // size 0x24
} // namespace dANpcVisitNpcHouse

// Global class name confirmed by RTTI.
class dAcNpcVisitNpcHouse_c : public dAcNpcNml_c {
  public:
    friend class dANpcVisitNpcHouse::State_c;
    enum { INVALID_FTR_HANDLE = -1, INVALID_UNIT_COORD = -1 };

    virtual ~dAcNpcVisitNpcHouse_c() {
    } // 0x13D0
    virtual bool canInteract(dDemoActor_c *actor);       // 0x1EC
    virtual void onDemoCmd(int cmd, u8 member);          // 0x254
    virtual int preCreate();                             // 0x38C
    virtual int create();                                // 0x3B8
    virtual int doDelete();                              // 0x434
    virtual int vtB0();                                  // 0x47C
    virtual int removeRoomFtr(int handle, int x, int z); // 0x4A8

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

    /* 0x219C */ dANpcVisitNpcHouse::State_c mState;
    /* 0x21C0 */ int mFtrX; // the furniture to sell (TALK_PROC_INVITE_TRADE)
    /* 0x21C4 */ int mFtrZ;
    /* 0x21C8 */ dItem::Item mFtr;
}; // size 0x21D0

#endif
