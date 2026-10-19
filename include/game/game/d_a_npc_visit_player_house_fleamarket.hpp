#ifndef D_A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET_HPP
#define D_A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET_HPP

// A villager visiting the player's flea market stall (REL
// d_a_npc_visit_player_house_fleamarketNP): it comes to the player's house
// during the flea market, looks at the furniture and offers to buy pieces of it
// (d_npc_talk_fmarket's TALK_PROC_STALL_* procedure sets). The class name is
// from RTTI; the controller and its state names are inferred. The controller
// follows d_a_npc_visit_player_house's (same states plus OFFER).

#include <game/game/d_a_npc_nml.hpp>

class dAcNpcVisitPlayerHouseFleamarket_c;
namespace dANpcVisitPlayerHouseFleamarket {
enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
enum {
    BLOCKED_FRAME_LIMIT = 120,
    WANDER_ROLL_COUNT = 5,
    WANDER_MOVE_ROLL_COUNT = 3,
    // look at the player / ahead (around): each roll is a coin flip
    LOOK_ROLL_COUNT = 4,
    // STATE_WAIT: 1 in this many to come
    CALL_ROLL_COUNT = 4,
    // from the knock until the talk at the door
    KNOCK_FRAMES = 150,
    // mTimer value at which the room sound restarts (the door opened)
    KNOCK_ROOM_SE_FRAME = 98,
    // dNpc::entry_c mStallBuyCount / mStallRefuseCount: no more offers
    STALL_COUNT_MAX = 3,
    // endStaySoon: the stay ends this many seconds later
    STAY_END_SECS = 30,
    // scene exit of the player's house
    EXIT_FRONT_DOOR = 0,
    // scene exit back to the setReturnExitHere position
    EXIT_RETURN = 4,
    EXIT_WIPE = 2,
};
// The player is still at the door within this distance of l_walkTargetPos.
#define VISIT_DOOR_NEAR_DIST 48.0f
// checkOffer: each candidate replaces the pick with chance OFFER_CHANCE_TOTAL /
// candidates.
#define OFFER_CHANCE_TOTAL 100.0f

class State_c {
  public:
    enum State_e {
        STATE_IDLE, // walk around the room
        STATE_WAIT, // hidden: every cWaitCallInterval frames, 1 in 4 chance to
                    // come
        STATE_GONE, // hidden: the visit is over
        STATE_CALL, // hidden at the door: call the player
                    // (TALK_PROC_STALL_CALL)
        STATE_ARRIVE,  // knock and come in, then the first talk
        STATE_TALK,    // talk to the player
        STATE_LOOK,    // turn to the player
        STATE_SEE_OFF, // talk, then the player leaves the house
        STATE_LEAVE,   // talk, then the villager takes the player outside
        STATE_PLAYER_LEAVING, // the player walks to the exit: ask to stay
                              // (TALK_PROC_STALL_WAIT)
        STATE_GO_HOME, // the visit is over: say goodbye (TALK_PROC_STALL_BACK)
        STATE_OFFER,   // look at a furniture for sale and offer to buy it
                       // (TALK_PROC_STALL_ITEM)
        STATE_COUNT,
        STATE_NONE = STATE_COUNT
    };
    enum Step_e { STEP_START, STEP_DONE };
    enum ArriveStep_e { ARRIVE_STEP_KNOCK, ARRIVE_STEP_TALK, ARRIVE_STEP_DONE };
    // getSidePos results consumed by the obstacle handler.
    enum SideResult_e { SIDE_CLEAR, SIDE_BLOCKED, SIDE_DETOUR };

    typedef void (State_c::*Proc)(dAcNpcVisitPlayerHouseFleamarket_c *npc);
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };
    State_c();  // .text 0x994
    ~State_c(); // 0x9CC
    void change(dAcNpcVisitPlayerHouseFleamarket_c *npc, u32 state); // 0xA0C
    void execute(dAcNpcVisitPlayerHouseFleamarket_c *npc);           // 0xA5C
    void setTalk(u32 resume, u32 talk,
                 const dAcNpcNml_c::talk_c::procSet_s *set);          // 0xB04
    void initIdle(dAcNpcVisitPlayerHouseFleamarket_c *npc);           // 0xB14
    BOOL avoidObstacles(dAcNpcVisitPlayerHouseFleamarket_c *npc);     // 0xC20
    BOOL checkPlayerLeaving(dAcNpcVisitPlayerHouseFleamarket_c *npc); // 0xCE0
    BOOL checkGoHome(dAcNpcVisitPlayerHouseFleamarket_c *npc);        // 0xD70
    BOOL stopIfBlocked(dAcNpcVisitPlayerHouseFleamarket_c *npc);      // 0xE08
    BOOL checkOffer(dAcNpcVisitPlayerHouseFleamarket_c *npc);         // 0xEA8
    BOOL checkIdle(dAcNpcVisitPlayerHouseFleamarket_c *npc);          // 0x1184
    void executeIdle(dAcNpcVisitPlayerHouseFleamarket_c *npc);        // 0x1254
    void initWait(dAcNpcVisitPlayerHouseFleamarket_c *npc);           // 0x1588
    void executeWait(dAcNpcVisitPlayerHouseFleamarket_c *npc);        // 0x15E0
    void initPlayerLeaving(dAcNpcVisitPlayerHouseFleamarket_c *npc);  // 0x164C
    void
    executePlayerLeaving(dAcNpcVisitPlayerHouseFleamarket_c *npc); // 0x1700
    void initGoHome(dAcNpcVisitPlayerHouseFleamarket_c *npc);      // 0x1770
    void executeGoHome(dAcNpcVisitPlayerHouseFleamarket_c *npc);   // 0x1824
    void initGone(dAcNpcVisitPlayerHouseFleamarket_c *npc);        // 0x1830
    void initCall(dAcNpcVisitPlayerHouseFleamarket_c *npc);        // 0x1874
    void executeCall(dAcNpcVisitPlayerHouseFleamarket_c *npc);     // 0x18D4
    BOOL isPlayerAwayFromDoor();                                   // 0x1950
    void initArrive(dAcNpcVisitPlayerHouseFleamarket_c *npc);      // 0x19E8
    void stepArriveKnock(dAcNpcVisitPlayerHouseFleamarket_c *npc); // 0x1A70
    void stepArriveTalk(dAcNpcVisitPlayerHouseFleamarket_c *npc);  // 0x1ACC
    void executeArrive(dAcNpcVisitPlayerHouseFleamarket_c *npc);   // 0x1BB8
    void initTalk(dAcNpcVisitPlayerHouseFleamarket_c *npc);        // 0x1BF8
    void stepTalk(dAcNpcVisitPlayerHouseFleamarket_c *npc);        // 0x1C8C
    void executeTalk(dAcNpcVisitPlayerHouseFleamarket_c *npc);     // 0x1D08
    void initLook(dAcNpcVisitPlayerHouseFleamarket_c *npc);        // 0x1D48
    void stepLook(dAcNpcVisitPlayerHouseFleamarket_c *npc);        // 0x1DD0
    void executeLook(dAcNpcVisitPlayerHouseFleamarket_c *npc);     // 0x1E34
    void initSeeOff(dAcNpcVisitPlayerHouseFleamarket_c *npc);      // 0x1E74
    void stepSeeOff(dAcNpcVisitPlayerHouseFleamarket_c *npc);      // 0x1F08
    void executeSeeOff(dAcNpcVisitPlayerHouseFleamarket_c *npc);   // 0x1F64
    void initLeave(dAcNpcVisitPlayerHouseFleamarket_c *npc);       // 0x1FA4
    void stepLeave(dAcNpcVisitPlayerHouseFleamarket_c *npc);       // 0x2038
    void executeLeave(dAcNpcVisitPlayerHouseFleamarket_c *npc);    // 0x20A4
    void initOffer(dAcNpcVisitPlayerHouseFleamarket_c *npc);       // 0x20E4
    void executeOffer(dAcNpcVisitPlayerHouseFleamarket_c *npc);    // 0x2210

    /* 0x00 */ const State_s *mpState;
    /* 0x04 */ const dAcNpcNml_c::talk_c::procSet_s *mpTalk;
    /* 0x08 */ u32 mState;
    /* 0x0C */ u32 mResumeState; // entered on DEMO_CMD_START
    /* 0x10 */ u32
        mTalkState; // entered on DEMO_CMD_TALK / DEMO_CMD_SPEAK, with mpTalk
    /* 0x14 */ u32 mTimer;
    /* 0x18 */ u32 mWaitTimer;
    /* 0x1C */ u32 mBlockedTimer;
    /* 0x20 */ u32 mLookTimer;
    /* 0x24 */ u32 mIdleFrames; // frames since the last talk (counts up)
    /* 0x28 */ u32
        mIdleFrameLimit; // mIdleFrames beyond this: the villager goes home
    /* 0x2C */ u8 mStep;
}; // size 0x30
} // namespace dANpcVisitPlayerHouseFleamarket

// Global class name confirmed by RTTI.
class dAcNpcVisitPlayerHouseFleamarket_c : public dAcNpcNml_c {
  public:
    friend class dANpcVisitPlayerHouseFleamarket::State_c;
    enum { INVALID_FTR_HANDLE = -1, INVALID_UNIT_COORD = -1 };
    virtual ~dAcNpcVisitPlayerHouseFleamarket_c() {
    } // 0x2398
    virtual bool canInteract(dDemoActor_c *actor);       // 0x1EC
    virtual void onDemoCmd(int cmd, u8 member);          // 0x268
    BOOL isStayOver(int secs) const;                     // 0x3A0
    void endStaySoon();                                  // 0x578
    BOOL checkStayOver();                                // 0x664
    BOOL checkRefusedTooOften();                         // 0x6B0
    virtual int removeRoomFtr(int handle, int x, int z); // 0x71C
    virtual int preCreate();                             // 0x770
    virtual int create();                                // 0x79C
    virtual int doDelete();                              // 0x930
    virtual int vtB0();                                  // 0x968

    /* 0x219C */ dANpcVisitPlayerHouseFleamarket::State_c mState;
    /* 0x21CC */ int mFtrX; // the offered furniture's unit (talk_c::setRoomFtr)
    /* 0x21D0 */ int mFtrZ;
    /* 0x21D4 */ int
        mOfferUnitX; // the villager's unit when it chose the furniture
    /* 0x21D8 */ int mOfferUnitZ;
    /* 0x21DC */ dItem::Item mFtr; // the offered furniture
}; // size 0x21E0

#endif
