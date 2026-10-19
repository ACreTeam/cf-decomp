#ifndef GAME_D_A_SIGN_TAG_HPP
#define GAME_D_A_SIGN_TAG_HPP

// A sign of the room / field collision data (REL d_a_sign_tagNP, profile
// A_SIGN_TAG), created by dBgUtil with its message, size and flag
// (getSignMsg / getSignSize / getSignFlag). Reading it shows its
// sys_MESS/SYS_Board message (the fishermen's creed, the bee warning, ...).
// The pointer can hit its box (dObjc::dpdBox_c), and a flagged sign also
// blocks the way (a dBGCF::mvbg_c). Class name from the RTTI ("daSignTag_c");
// everything else is inferred.

// Include order: the weak RTTI in .data follows class completion (fBase_c,
// dActor_c, dMsg::Rcpt_c, then dDemoActor_c).
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_objc.hpp>

class daSignTag_c : public dDemoActor_c, public dMsg::Rcpt_c {
  public:
    enum MODE_e {
        MODE_WAIT,     // nobody reads it
        MODE_TALK,     // waits for the message window to open
        MODE_TALK_END, // waits for the message window to close
        MODE_NUM,
    };

    typedef BOOL (daSignTag_c::*InitFunc)();
    typedef void (daSignTag_c::*ExecFunc)();

    virtual int create();                          // .text:0x27C
    virtual int execute();                         // .text:0x3BC
    virtual int draw();                            // .text:0x410
    virtual int doDelete();                        // .text:0x418
    virtual bool canInteract(dDemoActor_c *actor); // .text:0x45C
    virtual void onDemoCmd(int cmd, u8 member);    // .text:0x514

    static const InitFunc sInitFuncs[MODE_NUM];    // .rodata:0x14
    static const ExecFunc sExecuteFuncs[MODE_NUM]; // .rodata:0x38

    BOOL setMode(int mode); // .text:0x53C
    void executeMode();     // .text:0x5A8

    BOOL initWait();       // .text:0x5E8
    void executeWait();    // .text:0x5F0
    BOOL initTalk();       // .text:0x5F4
    void executeTalk();    // .text:0x660
    BOOL initTalkEnd();    // .text:0x6D4
    void executeTalkEnd(); // .text:0x6DC

    static int sNum; // .bss:0x8: the signs created (the demo ID's low byte)

    /* 0x0BC dMsg::Rcpt_c */
    /* 0x120 */ int mMode;
    /* 0x124 */ u8 mNo;        // sNum when created
    /* 0x126 */ u16 mMsgCode;  // sys_MESS/SYS_Board
    /* 0x128 */ mVec3_c mSize; // x, height, z
    /* 0x134 */ u8 mSolid;     // has mMvbg
    /* 0x138 */ f32 mRadius;   // half the diagonal
    /* 0x13C */ dBGCF::mvbg_c mMvbg;
    /* 0x1D8 */ dObjc::dpdBox_c mDpdBox;
    /* 0x548 */ f32 mTalkRange;
}; // size 0x54C

#endif
