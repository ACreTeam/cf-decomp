#ifndef D_A_RMOBJ_ATM_HPP
#define D_A_RMOBJ_ATM_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (as d_a_rmobj_sample.hpp).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_objc.hpp>

// The ATM in the city's auction house / the town hall (REL d_a_rmobj_atmNP,
// profile A_RMOBJ_ATM): the player talks to it (npc_SP/NPC_obj) to pay the
// debt, deposit or withdraw bells, or ask for a new card. The pointer hits its
// dpdBox_c. The same talk as the ATM furniture daFtrAtm_c
// (d_a_ftr_act), whose names it follows. Class name from RTTI
// ("daRmobjAtm_c").
class daRmobjAtm_c : public daRmobj_c {
  public:
    // The talk steps (l_initSteps / l_executeSteps).
    enum Step_e {
        STEP_WAIT,
        STEP_TALK,         // the greeting opens
        STEP_TALK_END,     // the talk runs; when it closes, the end (or the
                           // paid-off reaction)
        STEP_SUSPEND,      // the message window suspends for a menu
        STEP_MENU,         // the bank menu (mMenu)
        STEP_MENU_END,     // the message after the menu
        STEP_PAID_OFF,     // the player's "no more debt" reaction
        STEP_PAID_OFF_END, // its message
        STEP_NUM
    };

    // mAnswers: what the choices of the menu do.
    enum Answer_e {
        ANSWER_PAY,
        ANSWER_DEPOSIT,
        ANSWER_WITHDRAW,
        ANSWER_REISSUE_CARD,
        ANSWER_QUIT,
        ANSWER_NUM
    };

    // mMenu: the bank menu to open.
    enum Menu_e { MENU_PAY, MENU_DEPOSIT, MENU_WITHDRAW };

    // npc_SP/NPC_obj.
    enum {
        MSG_WELCOME_MAIN_BRANCH = 0x01,
        MSG_WELCOME_BRANCH = 0x02,
        MSG_THANKS = 0x03,
        MSG_NO_DEBT = 0x04,
        MSG_DEBT = 0x05, // shows the remaining debt
        MSG_SAVINGS_FULL = 0x06,
        MSG_SAVINGS = 0x07, // shows the balance
        MSG_NO_SAVINGS = 0x08,
        MSG_NOT_A_CLIENT = 0x09,
        MSG_CARD_REISSUED = 0x0A,
        MSG_WELCOME = 0x0B,
    };

    // sys_MESS/SYS_Player.
    enum {
        MSG_PLAYER_DEBT_PAID = 0x03, // "No more debt for me!"
    };

    // dSelect::Comp_c::setSysMessage: the choices of the menu.
    enum {
        SEL_MSG_PAY = 8,
        SEL_MSG_DEPOSIT,
        SEL_MSG_WITHDRAW,
        SEL_MSG_REISSUE_CARD,
        SEL_MSG_QUIT,
    };

    virtual int create();                          // 0x254
    virtual int doDelete();                        // 0x300
    virtual int execute();                         // 0x2C0
    virtual int draw();                            // 0x2F8
    virtual bool canInteract(dDemoActor_c *other); // 0x308
    virtual void onDemoCmd(int cmd, u8 member);    // 0x3A8
    virtual void onMessageStart(int arg);          // 0xA28
    virtual void onMessageEnd(int kind);           // 0xABC
    virtual void onAnswer(int arg);                // 0xD6C

    BOOL setStep(int step); // 0x404: runs the step's init; FALSE if it fails
    void executeStep();     // 0x470

    BOOL initWait();          // 0x4B0
    void executeWait();       // 0x4B8
    BOOL initTalk();          // 0x4BC
    void executeTalk();       // 0x584
    BOOL initTalkEnd();       // 0x5F8
    void executeTalkEnd();    // 0x600
    BOOL initSuspend();       // 0x704
    void executeSuspend();    // 0x718
    BOOL initMenu();          // 0x78C
    void executeMenu();       // 0x82C
    BOOL initMenuEnd();       // 0x888
    void executeMenuEnd();    // 0x890
    BOOL initPaidOff();       // 0x904
    void executePaidOff();    // 0x960
    BOOL initPaidOffEnd();    // 0x9AC
    void executePaidOffEnd(); // 0x9B4

    /* 0x15C */ int mMenuMsgCode; // the message after the menu
    /* 0x160 */ u32 _160;         // not used
    /* 0x164 */ model_c mModel;   // not used (the room has the model)
    /* 0x310 */ dObjc::dpdBox_c mDpdBox;
    /* 0x680 */ int mMenu; // Menu_e
    /* 0x684 */ int mDebt; // the player's debt when the talk started
    /* 0x688 */ int mAnswers[ANSWER_NUM]; // Answer_e of each choice
    /* 0x69C */ int mStep;                // Step_e
}; // size 0x6A0

#endif
