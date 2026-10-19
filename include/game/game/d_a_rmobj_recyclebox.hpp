#ifndef D_A_RMOBJ_RECYCLEBOX_HPP
#define D_A_RMOBJ_RECYCLEBOX_HPP

// Include order: the RTTI/vtable group order in .data (reverse class
// completion).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_objc.hpp>

// The recycle box of the town hall (REL d_a_rmobj_recycleboxNP, profile
// A_RMOBJ_RECYCLEBOX). Class name from RTTI ("daRmobjRB_c"). Talking to it
// shows a message (npc_SP/NPC_obj 0x15), opens the lid and the recycle box
// menu, then closes the lid. The lid state is shared over the net (record
// NET_REC_RECYCLEBOX_LID; written by the console in the talk, followed by the
// others).
class daRmobjRB_c : public daRmobj_c {
  public:
    // The model's animations.
    enum Anm_e {
        ANM_LID, // played forward to open, backward to close
    };

    enum LidState_e { LID_CLOSED, LID_OPENING, LID_OPEN, LID_CLOSING, LID_NUM };

    enum TalkState_e {
        TALK_NONE,
        TALK_MESSAGE, // the message is shown
        TALK_MESSAGE_END,
        TALK_OPEN,  // the lid opens
        TALK_MENU,  // the recycle box menu
        TALK_CLOSE, // the lid closes, then the talk ends
        TALK_NUM
    };

    // npc_SP/NPC_obj.
    enum {
        MSG_RECYCLING_BIN = 0x15, // "Recycling Bin / Pick-up time is 6 a.m.
                                  // each Monday and Thursday."
    };

    typedef BOOL (daRmobjRB_c::*InitFunc)();
    typedef void (daRmobjRB_c::*ExecFunc)();

    virtual int create();                          // 0x368
    virtual int doDelete();                        // 0x4F0
    virtual int execute();                         // 0x424
    virtual int draw();                            // 0x4C8
    virtual bool canInteract(dDemoActor_c *actor); // 0x53C
    virtual void onDemoCmd(int cmd, u8 member);    // 0x5DC

    virtual void onMessageStart(int arg); // 0xBF8
    virtual void onMessageEnd(int kind);  // 0xBFC
    virtual void onAnswer(int arg);       // 0xC0C

    BOOL changeLidState(u8 state);   // 0x604
    void executeLidState();          // 0x67C
    BOOL initLidClosed();            // 0x6F4
    void executeLidClosed();         // 0x760
    BOOL initLidOpening();           // 0x764
    void executeLidOpening();        // 0x7C4
    BOOL initLidOpen();              // 0x820
    void executeLidOpen();           // 0x838
    BOOL initLidClosing();           // 0x83C
    void executeLidClosing();        // 0x898
    BOOL changeTalkState(int state); // 0x920
    void executeTalkState();         // 0x98C
    BOOL initTalkNone();             // 0x9CC
    void executeTalkNone();          // 0x9D4
    BOOL initTalkMessage();          // 0x9D8
    void executeTalkMessage();       // 0xA44
    BOOL initTalkMessageEnd();       // 0xAB8
    void executeTalkMessageEnd();    // 0xAC0
    BOOL initTalkOpen();             // 0xB4C
    void executeTalkOpen();          // 0xB54
    BOOL initTalkMenu();             // 0xB6C
    void executeTalkMenu();          // 0xB9C
    BOOL initTalkClose();            // 0xBDC
    void executeTalkClose();         // 0xBE4
    void addBg();                    // 0xC10
    void removeBg();                 // 0xC6C
    void setLidRecord(u8 state);     // 0xCA0
    u8 getLidRecord();               // 0xCDC

    /* 0x15C */ dBGCF::mvbg_c mBg;
    /* 0x1F8 */ dObjc::dpdBox_c mBox;
    /* 0x568 */ model_c mMdl;
    /* 0x714 */ u8 mLidState;
    /* 0x715 */ u8
        mCloseSePlayed; // no SE_RecycleBox_Close on entering LID_CLOSED
    /* 0x718 */ int mTalkState;
}; // size 0x71C

#endif
