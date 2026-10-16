#ifndef D_A_NPC_IN_HPP
#define D_A_NPC_IN_HPP

#include <game/game/d_a_npc_nml.hpp>

class dAcNpcIn_c;
namespace dANpcIn {
// Inferred IDs and tuning choices used by the indoor villager controller.
enum Animation_e {
    ANM_WAIT = 0,
    ANM_WALK = 1,
    ANM_SAD_WAIT = 0xC8,
    ANM_SAD_WALK = 0xC9
};
enum Emotion_e { EMOTION_DEFAULT = 0, EMOTION_SICK = 7 };
enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };
enum {
    COLLISION_WEIGHT = 0x80,
    BLOCKED_FRAME_LIMIT = 120,
    WANDER_ROLL_COUNT = 5,
    WANDER_MOVE_ROLL_COUNT = 3,
    EMOTION_ROLL_COUNT = 5,
    SICK_EMOTION_ROLL_COUNT = 3
};
// Inferred names, from the state handlers and conversation procedure sets.
class State_c {
  public:
    enum State_e {
        IDLE,
        TALK,
        LOOK,
        REMOTE,
        REMOTE_LOOK,
        FMARKET,
        FMARKET_REMOTE,
        SICK,
        CHANGE_CLOTH,
        COUNT
    };
    enum Step_e { STEP_START, STEP_DONE };
    // getSidePos results consumed by the obstacle handler.
    enum SideResult_e { SIDE_CLEAR, SIDE_BLOCKED, SIDE_DETOUR };
    typedef void (State_c::*Proc)(dAcNpcIn_c *npc);
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };
    State_c();  // 0x7A0
    ~State_c(); // 0x848
    void setTalk(u32 resume, u32 talk,
                 const dAcNpcNml_c::talk_c::procSet_s *set); // 0x8C8
    void change(dAcNpcIn_c *npc, u32 state);                 // 0x8D8
    u32 getEventState(dAcNpcIn_c *npc, u32 state);           // 0x928
    u32 getLocalEventState(dAcNpcIn_c *npc);                 // 0xA18
    void selectLocal(dAcNpcIn_c *npc);                       // 0xA20
    u32 getRemoteEventState(dAcNpcIn_c *npc);                // 0xAE0
    void selectRemote(dAcNpcIn_c *npc);                      // 0xAE8
    void enterSelected(dAcNpcIn_c *npc);                     // 0xB24
    void startTalk(dAcNpcIn_c *npc);                         // 0xB2C
    void execute(dAcNpcIn_c *npc);                           // 0xBDC
    void initIdle(dAcNpcIn_c *npc);                          // 0xC38
    BOOL avoidObstacles(dAcNpcIn_c *npc);                    // 0xD38
    BOOL stopIfBlocked(dAcNpcIn_c *npc);                     // 0xDF8
    BOOL checkMove(dAcNpcIn_c *npc);                         // 0xE98
    void executeIdle(dAcNpcIn_c *npc);                       // 0xF14
    void initTalk(dAcNpcIn_c *npc);                          // 0x1134
    void stepTalk(dAcNpcIn_c *npc);                          // 0x11A8
    void executeTalk(dAcNpcIn_c *npc);                       // 0x1208
    void initLook(dAcNpcIn_c *npc);                          // 0x1248
    void stepLook(dAcNpcIn_c *npc);                          // 0x12A8
    void executeLook(dAcNpcIn_c *npc);                       // 0x130C
    void initRemote(dAcNpcIn_c *npc);                        // 0x134C
    void executeRemote(dAcNpcIn_c *npc);                     // 0x140C
    void initRemoteLook(dAcNpcIn_c *npc);                    // 0x14F8
    void executeRemoteLook(dAcNpcIn_c *npc);                 // 0x1530
    void initFmarket(dAcNpcIn_c *npc);                       // 0x1678
    BOOL checkFmarketMove(dAcNpcIn_c *npc);                  // 0x17B0
    void executeFmarket(dAcNpcIn_c *npc);                    // 0x182C
    void initFmarketRemote(dAcNpcIn_c *npc);                 // 0x1C28
    void executeFmarketRemote(dAcNpcIn_c *npc);              // 0x1D24
    void initSick(dAcNpcIn_c *npc);                          // 0x1F44
    void executeSick(dAcNpcIn_c *npc);                       // 0x20C4
    void initChangeCloth(dAcNpcIn_c *npc);                   // 0x2420
    void stepChangeCloth(dAcNpcIn_c *npc);                   // 0x24D4
    void executeChangeCloth(dAcNpcIn_c *npc);                // 0x2530
    /* 0x00 */ const State_s *mpState;
    /* 0x04 */ const dAcNpcNml_c::talk_c::procSet_s *mpTalk;
    /* 0x08 */ u32 mState;
    /* 0x0C */ u32 mResumeState;
    /* 0x10 */ u32 mTalkState;
    /* 0x14 */ u32 mSelectedState;
    /* 0x18 */ u32 mTimer;
    /* 0x1C */ u32 mWaitTimer;
    /* 0x20 */ u8 mSalePending;
    /* 0x21 */ u8 mStep;
    /* 0x24 */ u32 mBlockedTimer;
    /* 0x28 */ dLevelEffect_c mEffect;
}; // size 0xBC
} // namespace dANpcIn

// Global class name confirmed by RTTI.
class dAcNpcIn_c : public dAcNpcNml_c {
  public:
    friend class dANpcIn::State_c;
    enum {
        INVALID_FTR_HANDLE = -1,
        INVALID_UNIT_COORD = -1
    };
    virtual ~dAcNpcIn_c() {
    } // 0x26D0
    virtual bool canInteract(dDemoActor_c *actor);       // 0x260
    virtual void onDemoCmd(int cmd, u8 member);          // 0x28C
    virtual int removeRoomFtr(int handle, int x, int z); // 0x41C
    virtual int preCreate();                             // 0x4F4
    virtual int create();                                // 0x520
    virtual int doDelete();                              // 0x600
    virtual int vtB0();                                  // 0x638
    BOOL isSick() const;                                 // 0x67C
    enum Sickness_e { SICK_START, SICK_WORSE, SICK_WORST, SICK_NONE };
    int getSickness() const; // 0x71C
    BOOL isSickness(int value, int expected) const {
        return value == expected;
    }
    void stopEmotion(); // 0x780
    dAnimal_c *getAnimal() const {
        return mpAnimal;
    }
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
    void activate() {
        if (!mIsActive) {
            mIsActive = TRUE;
            mLastPos = mPos;
        }
    }
    /* 0x219C */ dANpcIn::State_c mState;
    /* 0x2258 */ int mFtrX;
    /* 0x225C */ int mFtrZ;
    /* 0x2260 */ dItem::Item mFtr;
    /* 0x2264 */ moveParam_c mMoveParam;
}; // size 0x2270

#endif
