#ifndef D_A_NPC_CAFE_HPP
#define D_A_NPC_CAFE_HPP

#include <game/game/d_a_npc_nml.hpp>

class dAcNpcCafe_c;
namespace dANpcCafe {
// Inferred names: the three states are idle, conversation and player-facing.
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

    typedef void (State_c::*Proc)(dAcNpcCafe_c *);
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };
    State_c();                                 // .text 0x40C
    ~State_c();                                // .text 0x424
    void change(dAcNpcCafe_c *npc, u32 state); // 0x464
    void execute(dAcNpcCafe_c *npc);           // 0x4B4
    void initIdle(dAcNpcCafe_c *npc);          // 0x4E8
    void executeIdle(dAcNpcCafe_c *npc);       // 0x54C
    void initTalk(dAcNpcCafe_c *npc);          // 0x550
    void stepTalk(dAcNpcCafe_c *npc);          // 0x5B8
    void executeTalk(dAcNpcCafe_c *npc);       // 0x610
    void initLook(dAcNpcCafe_c *npc);          // 0x650
    void executeLook(dAcNpcCafe_c *npc);       // 0x678
    /* 0x0 */ const State_s *mpState;
    /* 0x4 */ u32 mState;
    /* 0x8 */ u8 mStep;
}; // size 0xC
} // namespace dANpcCafe

// Global class name is confirmed by RTTI.
class dAcNpcCafe_c : public dAcNpcNml_c {
  public:
    virtual ~dAcNpcCafe_c() {
    } // 0x6D0
    virtual bool canInteract(dDemoActor_c *actor); // 0x1D8
    virtual void onDemoCmd(int cmd, u8 member);    // 0x204
    virtual int preCreate();                       // 0x2DC
    virtual int create();                          // 0x34C
    virtual int doDelete();                        // 0x3B4
    virtual int vtB0();                            // 0x3E0
    /* 0x219C */ dANpcCafe::State_c mState;
}; // size 0x21A8

#endif
