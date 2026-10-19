#ifndef D_A_RMOBJ_PERM_MACHINE_HPP
#define D_A_RMOBJ_PERM_MACHINE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (as d_a_rmobj_sample.hpp).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_effect.hpp>

// The perm machine at Shampoodle (REL d_a_rmobj_perm_machineNP, profile
// A_RMOBJ_PERM_MACHINE): the hair machine ("obj_tw_b_machine") Harriet
// (d_a_npc_sp_catherine) runs through m_instance. Its chr animation 0 is the
// perm, animation 1 the special makeover. A moving bg box keeps the player
// out. Class name from RTTI ("daRmobjPermMachine_c").
class daRmobjPermMachine_c : public daRmobj_c {
  public:
    // The steps (l_initSteps / l_executeSteps).
    enum Step_e {
        STEP_WAIT,         // idle
        STEP_PERM,         // the perm animation (smoke from the head)
        STEP_SPECIAL_MAKE, // the makeover animation (sparks, then smoke)
        STEP_NUM
    };

    // The chr animations of the model.
    enum Anm_e { ANM_PERM, ANM_SPECIAL_MAKE };

    daRmobjPermMachine_c();          // 0xFC
    virtual ~daRmobjPermMachine_c(); // 0x410

    virtual int create();   // 0x54C
    virtual int doDelete(); // 0x734
    virtual int execute();  // 0x6D0
    virtual int draw();     // 0x70C

    BOOL setStep(int step); // 0x790: runs the step's init; FALSE if it fails
    void executeStep();     // 0x7FC

    BOOL initWait();           // 0x83C
    void executeWait();        // 0x844
    BOOL initPerm();           // 0x848
    void executePerm();        // 0x8A8
    BOOL initSpecialMake();    // 0x970
    void executeSpecialMake(); // 0x9F4

    // Used by Harriet.
    BOOL isPermDone();        // 0xADC
    BOOL isSpecialMakeDone(); // 0xAE4
    void startSwitchSound(); // 0xAEC

    static daRmobjPermMachine_c *m_instance; // .bss 0x8: set while it exists

    /* 0x15C */ dLevelEffect_c mEffect; // the makeover sparks ("tuno")
    /* 0x1F0 */ int mStep;              // Step_e
    /* 0x1F4 */ model_c mModel;
    /* 0x3A0 */ mVec3_c mSoundPos;   // the "body" node
    /* 0x3AC */ u8 mPermDone;        // the perm animation has ended
    /* 0x3AD */ u8 mSpecialMakeDone; // the makeover animation has ended
    /* 0x3B0 */ dBGCF::mvbg_c mMvbg;
}; // size 0x44C

#endif
