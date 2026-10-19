#ifndef D_A_RMOBJ_POINT_HPP
#define D_A_RMOBJ_POINT_HPP

// Include order: the RTTI in .data follows class completion (fBase_c,
// dActor_c, dMsg::Rcpt_c, dDemoActor_c).
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_objc.hpp>

// Nook's Point-Tracking System of the store (REL d_a_rmobj_pointNP, profile
// A_RMOBJ_POINT): a terminal the player talks to for visitor points, the
// total points and, once a day, a present mailed home. Not a daRmobj_c: no
// model, just a collision box and a pointer box. Class name from RTTI
// ("daRmobjPoint_c").
class daRmobjPoint_c : public dDemoActor_c, public dMsg::Rcpt_c {
  public:
    enum State_e {
        STATE_IDLE,    // nothing
        STATE_TALK,    // the welcome message, then the talk
        STATE_SUSPEND, // the present: wait for the message window to suspend
        STATE_MENU,    // the present menu
        STATE_RESUME,  // back to the talk
        STATE_NONE,    // mNextState: no change
    };

    typedef void (daRmobjPoint_c::*Proc)();
    struct State_s {
        Proc mInit;
        Proc mExecute;
    };

    // NPC_sp/NPC_obj message codes, named from the English text.
    enum {
        MSG_MENU_CANCEL = 0x28, // (empty) the present menu was closed
        MSG_NO_CARD = 0x29,     // "...Unable to read..."
        MSG_WELCOME_MEMBER =
            0x2A, // "...Member <name>, please make your selection"
        MSG_WELCOME_VISITOR = 0x2B, // "...Member <name> from <town>, ..."
        MSG_ANYTHING_ELSE = 0x2C,   // "Can I help you with anything else?"
        MSG_POINT_STATUS =
            0x30,             // "<name>, here is your current point status..."
        MSG_NEXT_RANK = 0x31, // "...you need <n> points."
        MSG_PLATINUM = 0x32,  // "Just as platinum glimmers..."
        MSG_VISITED_TODAY =
            0x33, // "You have already earned visitor points for today."
        MSG_VISIT_POINTS = 0x34, // "...+3 points for visiting our store!"
        MSG_VISIT_POINTS_FAR =
            0x35, // "...+5 points..." (a player from another town)
        MSG_PRESENT_TODAY =
            0x36,        // "You can only exchange one present per day."
        MSG_BUSY = 0x37, // "...the service is not available at this time."
        MSG_PRESENT_SENT =
            0x39, // "Transaction complete. Your present will be sent..."
    };

    // sys_STRING/STR_Unit: the rank names ("Basic", "Bronze", "Silver",
    // "Gold", "Platinum"), by dSaveShop_c::getRank.
    enum { STR_UNIT_RANK = 0x29 };
    enum { RANK_PLATINUM = 4 };

    // The answers of MSG_WELCOME_MEMBER / MSG_ANYTHING_ELSE.
    enum {
        ANSWER_VISITOR_POINTS,
        ANSWER_TOTAL_POINTS,
        ANSWER_PRESENT,
    };

    virtual int create();                          // 0x2EC
    virtual int doDelete();                        // 0x390
    virtual int execute();                         // 0x3C8
    virtual int draw();                            // 0x40C
    virtual bool canInteract(dDemoActor_c *actor); // 0xC08
    virtual void onDemoCmd(int cmd, u8 member);    // 0xCA8
    virtual void onMessageStart(int arg);          // 0x890
    virtual void onMessageEnd(int kind);           // 0x978
    virtual void onAnswer(int arg);                // 0xA20

    void initIdle();          // 0x414
    void executeIdle();       // 0x418
    void initTalk();          // 0x41C
    void executeTalk();       // 0x4E4
    void initSuspend();       // 0x56C
    void executeSuspend();    // 0x57C
    void initMenu();          // 0x5EC
    void executeMenu();       // 0x5F4
    void initResume();        // 0x720
    void executeResume();     // 0x72C
    void updateState();       // 0x79C: switches to mNextState
    void executeState();      // 0x820
    void startTalk(u16 code); // 0xB84

    /* 0x0BC dMsg::Rcpt_c */
    /* 0x120 */ int mState; // State_e
    /* 0x124 */ int mPrevState;
    /* 0x128 */ int mNextState; // STATE_NONE: no change
    /* 0x12C */ dBGCF::mvbg_c mBg;
    /* 0x1C8 */ dObjc::dpdBox_c mBox;
}; // size 0x538

#endif
