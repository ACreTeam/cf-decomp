#ifndef GAME_D_A_NAMEPLATE_HPP
#define GAME_D_A_NAMEPLATE_HPP

// The nameplate at a villager's house (REL d_a_nameplateNP, profile
// A_NAMEPLATE). Talking to it shows "<name>'s House" (sys_MESS/SYS_Board
// MSG_HOUSE) with the name of the villager of town record mParam, or Tom Nook's
// while the house has no resident. Class name from the RTTI
// ("daNameplate_c"); everything else is inferred.

#include <game/game/d_demo_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_objc.hpp>

class daNameplate_c : public dDemoActor_c, private dMsg::Rcpt_c {
  public:
    // sys_MESS/SYS_Board message codes, named from the English text.
    enum {
        MSG_HOUSE = 0x01, // "<name>'s House"
    };

    enum MODE_e {
        MODE_WAIT,     // nobody reads it
        MODE_TALK,     // waits for the message window to open
        MODE_TALK_END, // waits for the message window to close
        MODE_NUM,
    };

    typedef BOOL (daNameplate_c::*InitFunc)();
    typedef void (daNameplate_c::*ExecFunc)();

    virtual int create();                          // .text:0xF0
    virtual int execute();                         // .text:0x160
    virtual int draw();                            // .text:0x198
    virtual int doDelete();                        // .text:0x1A0
    virtual bool canInteract(dDemoActor_c *actor); // .text:0x1A8
    virtual void onDemoCmd(int cmd, u8 member);    // .text:0x240

    static const InitFunc sInitFuncs[MODE_NUM];    // .rodata:0xC
    static const ExecFunc sExecuteFuncs[MODE_NUM]; // .rodata:0x30

    BOOL setMode(int mode); // .text:0x268
    void executeMode();     // .text:0x2D4

    BOOL initWait();       // .text:0x314
    void executeWait();    // .text:0x31C
    BOOL initTalk();       // .text:0x320
    void executeTalk();    // .text:0x41C
    BOOL initTalkEnd();    // .text:0x490
    void executeTalkEnd(); // .text:0x498

    /* 0x0BC dMsg::Rcpt_c */
    /* 0x120 */ int mMode;
    /* 0x124 */ dObjc::ac_c mObjc;
}; // size 0x168

#endif
