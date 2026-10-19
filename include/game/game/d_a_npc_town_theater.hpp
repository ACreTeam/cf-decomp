#ifndef D_A_NPC_TOWN_THEATER_HPP
#define D_A_NPC_TOWN_THEATER_HPP

// The theater's guest villager (REL d_a_npc_town_theaterNP). Class name from
// RTTI; the controller and its state names are inferred (same shape as
// d_a_npc_cafe's).

#include <game/game/d_a_npc_town.hpp>

class dAcNpcTownTheater_c;
namespace dANpcTownTheater {
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

    typedef void (State_c::*Proc)(dAcNpcTownTheater_c *);
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };
    State_c();                                        // .text 0x43C
    ~State_c();                                       // 0x454
    void change(dAcNpcTownTheater_c *npc, u32 state); // 0x494
    void execute(dAcNpcTownTheater_c *npc);           // 0x4E4
    void initIdle(dAcNpcTownTheater_c *npc);          // 0x518
    void executeIdle(dAcNpcTownTheater_c *npc);       // 0x570
    void initTalk(dAcNpcTownTheater_c *npc);          // 0x574
    void stepTalk(dAcNpcTownTheater_c *npc);          // 0x5DC
    void executeTalk(dAcNpcTownTheater_c *npc);       // 0x634
    void initLook(dAcNpcTownTheater_c *npc);          // 0x674
    void executeLook(dAcNpcTownTheater_c *npc);       // 0x69C

    /* 0x0 */ const State_s *mpState;
    /* 0x4 */ u32 mState;
    /* 0x8 */ u8 mStep;
}; // size 0xC
} // namespace dANpcTownTheater

class dAcNpcTownTheater_c : public dAcNpcTown_c {
  public:
    virtual ~dAcNpcTownTheater_c() {
    }                                              // 0x6F4
    virtual bool canInteract(dDemoActor_c *actor); // 0x1D8
    virtual void onDemoCmd(int cmd, u8 member);    // 0x240
    virtual int preCreate();                       // 0x318
    virtual int create();                          // 0x380
    virtual int doDelete();                        // 0x3E4
    virtual int vtB0();                            // 0x410

    /* 0x219C */ dANpcTownTheater::State_c mState;
}; // size 0x21A8

#endif
