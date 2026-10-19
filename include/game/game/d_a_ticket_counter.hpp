#ifndef D_A_TICKET_COUNTER_HPP
#define D_A_TICKET_COUNTER_HPP

// Include order: the RTTI in .data follows class completion (fBase_c,
// dActor_c, dMsg::Rcpt_c, dDemoActor_c).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_theater.hpp>

// The ticket counter of the theater lobby (REL d_a_ticket_counterNP, profile
// A_TICKET_COUNTER, placed in SCENE_RM_THEATER_ENT): the clerk sells the
// tickets (TICKET_PRICE Bells) and picks the show from this week's programs.
// The lobby's exits (dBGCF next areas) are locked by ticket: with a ticket
// the street exit (NEXT_STREET) is locked and the two theater doors are open,
// without one the other way round. A player walking into a locked exit is
// stopped by the clerk and walked back. No model, just a pointer box. Class
// name from RTTI ("daTicketCounter_c").
class daTicketCounter_c : public dDemoActor_c, public dMsg::Rcpt_c {
  public:
    enum State_e {
        STATE_WAIT,        // stops a player at a locked exit (DEMO_CMD_SPEAK)
        STATE_TALK,        // the player talks to the clerk
        STATE_TALK_END,    // waits for the talk to end
        STATE_STOP,        // no ticket: the theater doors are closed
        STATE_STOP_END,    // walks the player back out of the doorway
        STATE_LEAVE,       // with a ticket: asks before the player goes out
        STATE_LEAVE_END,   // walks the player back in, or leaves the scene
        STATE_WAIT_PLAYER, // ends the demo once the player stopped walking
        STATE_NUM
    };

    // The lobby's exits (dBGCF next area index).
    enum {
        NEXT_STREET,  // out to the street
        NEXT_THEATER, // the two doors into the theater
        NEXT_THEATER2,
    };

    // npc_SP/NPC_theater message codes, named from the English text.
    enum {
        MSG_NO_TICKET = 0x01, // "...please buy a ticket at the ticket counter."
        MSG_NO_REENTRY = 0x02,   // "...you can't go back into the theater once
                                 // you leave..."
        MSG_WELCOME = 0x03,      // (tags only) the clerk's greeting
        MSG_COME_AGAIN = 0x04,   // "All right. Well, please come back again
                                 // soon!"
        MSG_NO_MONEY = 0x05,     // "...you don't have enough money..."
        MSG_WHICH_SHOW = 0x06,   // "What show would you like to see today?"
        MSG_SHOW_PRICE = 0x07,   // "So, the show you'd like to see is <show>?
                                 // ...800 Bells."
        MSG_THANK_YOU = 0x08,    // "Thank you. The show will begin in a few
                                 // moments..."
        MSG_FIRST_TIME = 0x09,   // "Is this your first time here to see a
                                 // show...a special performance."
        MSG_SPECIAL_SHOW = 0x0A, // "The performance will begin in just a few
                                 // moments..."
        MSG_LEAVE = 0x0B, // "...You DO know you won't be able to enter the
                          // theater again if you leave...?" / "I'll go back
                          // in." / "I'm going out."
        MSG_GO_IN = 0x0F, // "The show will begin in a few moments. Please make
                          // your way into the theater and take your seat."
        MSG_TICKETS = 0x15, // "Welcome to the Marquee! ...tickets are 800 Bells
                            // each." / "Sounds good!" / "No, thanks."
        MSG_CLOSED = 0x16,  // (tags only) the closed hours message
    };

    // sys_MESS/SYS_Door: "Closed / Hours: 10 a.m.-11 p.m." (the barricade is
    // up).
    enum { DOOR_MSG_CLOSED = 0x0B };

    // sys_STRING/STR_Reaction: "Never mind" (the show list's cancel answer).
    // The shows themselves are the reactions Dr. Shrunk teaches: program
    // index + 1 = the reaction's string.
    enum { STR_REACTION_NEVER_MIND = 0x1E };

    // The answers of MSG_TICKETS and MSG_LEAVE.
    enum { ANSWER_BUY, ANSWER_NO };
    enum { ANSWER_GO_IN, ANSWER_GO_OUT };
    // The answers of MSG_WHICH_SHOW: this week's four programs, then "Never
    // mind" (also the cancel answer).
    enum { SHOW_ANSWER_NEVER_MIND = THEATER_WEEK_PROGRAM_NUM, SHOW_ANSWER_NUM };

    // The ticket price in Bells.
    enum { TICKET_PRICE = 800 };
    // The theater is open from OPEN_HOUR to CLOSE_HOUR.
    enum { OPEN_HOUR = 10, CLOSE_HOUR = 23 };

    typedef BOOL (daTicketCounter_c::*InitProc)();
    typedef void (daTicketCounter_c::*ExecuteProc)();

    virtual int create();                          // 0x16C
    virtual int doDelete();                        // 0x2B8
    virtual int execute();                         // 0x274
    virtual int draw();                            // 0x2B0
    virtual bool canInteract(dDemoActor_c *actor); // 0xA38
    virtual void onDemoCmd(int cmd, u8 member);    // 0xAD8
    virtual void onMessageStart(int arg);          // 0xBE4
    virtual void onMessageEnd(int kind);           // 0xC68
    virtual void onAnswer(int arg);                // 0xDD4
    virtual int getVoiceMode();                    // 0xBCC

    void initParam();            // 0x2E8
    BOOL changeState(int state); // 0x2FC
    void executeState();         // 0x368
    BOOL initWait();             // 0x3A8
    void executeWait();          // 0x3B0
    BOOL initTalk();             // 0x41C
    void executeTalk();          // 0x4F8
    BOOL initTalkEnd();          // 0x56C
    void executeTalkEnd();       // 0x574
    BOOL initStop();             // 0x630
    void executeStop();          // 0x75C
    BOOL initStopEnd();          // 0x7D0
    void executeStopEnd();       // 0x7D8
    BOOL initLeave();            // 0x870
    void executeLeave();         // 0x8DC
    BOOL initLeaveEnd();         // 0x950
    void executeLeaveEnd();      // 0x958
    BOOL initWaitPlayer();       // 0x9F0
    void executeWaitPlayer();    // 0x9F8
    void movePlayer(int state);  // 0xB34

    /* 0x0BC dMsg::Rcpt_c */
    /* 0x120 */ int mState;    // State_e
    /* 0x124 */ BOOL mLeaving; // MSG_LEAVE: the player chose to go out
    /* 0x128 */ int mPrice;    // TICKET_PRICE
    /* 0x12C */ dObjc::dpdBox_c mBox;
}; // size 0x49C

#endif
