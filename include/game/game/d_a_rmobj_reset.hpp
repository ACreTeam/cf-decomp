#ifndef D_A_RMOBJ_RESET_HPP
#define D_A_RMOBJ_RESET_HPP

// Include order: the RTTI/vtable group order in .data (reverse class
// completion).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_effect.hpp>
#include <game/mLib/m_mtx.hpp>

// The Reset Surveillance Center's machinery (REL d_a_rmobj_resetNP, profile
// A_RMOBJ_RESET). Class name from RTTI ("daRmobjReset_c"). Two models: the
// monitor wall ("obj_tw_reset_TV", textured with the furniture's dummy TV file)
// and the room ("obj_tw_reset_R": the seat that launches Mr. Resetti, its gear,
// the warning light and the door). d_a_npc_sp_demo_resetsan drives it through
// s_instance (changeState, lightOn, lightOff).
class daRmobjReset_c : public daRmobj_c {
  public:
    enum State_e {
        STATE_NONE,
        STATE_NONE2,
        STATE_LAUNCH, // the gear turns: the lever, the seat starts, steam
        STATE_WAIT,   // the seat waits (loop)
        STATE_BACK,   // the seat comes back: sparks, then fire
        STATE_BACK_END,
        STATE_DOOR_CLOSE,
        STATE_END,
        STATE_NUM
    };

    typedef BOOL (daRmobjReset_c::*InitFunc)();
    typedef void (daRmobjReset_c::*ExecFunc)();

    virtual int create();   // 0x308
    virtual int doDelete(); // 0x5E4
    virtual int execute();  // 0x41C
    virtual int draw();     // 0x5A8

    const mVec3_c *
    getUnusedSndPos(); // dead-stripped: only its static is left in .bss
    void
    lightOn(); // 0x644: the warning light (srt animation "obj_tw_rst_light")
    void lightOff();             // 0x6E4
    BOOL changeState(int state); // 0x784
    void executeState();         // 0x7F0
    BOOL initNone();             // 0x830
    void executeNone();          // 0x838
    BOOL initLaunch();           // 0x83C
    void executeLaunch();        // 0x89C
    BOOL initWait();             // 0x9E8
    void executeWait();          // 0xA04
    BOOL initBack();             // 0xA10
    void executeBack();          // 0xA7C
    BOOL initBackEnd();          // 0xB3C
    void executeBackEnd();       // 0xB44
    BOOL initDoorClose();        // 0xB58
    void executeDoorClose();     // 0xBBC
    BOOL initEnd();              // 0xC00
    void executeEnd();           // 0xC08
    void updateChair(); // 0xC0C: mChairPos / mChairMtx from node "chair0"

    static daRmobjReset_c *s_instance;

    /* 0x15C */ int mState;
    /* 0x160 */ u8 _160[8];
    /* 0x168 */ model_c mTvMdl;
    /* 0x314 */ model_c mRoomMdl;
    /* 0x4C0 */ mVec3_c
        mFirePos; // where the fire starts (the chair, when the seat came back)
    /* 0x4CC */ mVec3_c mChairPos;
    /* 0x4D8 */ mMtx_c mChairMtx;
    /* 0x508 */ dSndObjSimple_c mDoorSnd;
    /* 0x538 */ dSndObjSimple_c mTvSnd;
    /* 0x568 */ u8 mLightOn;
    /* 0x569 */ u8 mLightUsed; // set by lightOn, never cleared: the TV plays
                               // its scramble sound
    /* 0x56C */ dLevelEffect_c mSparkEffect;
    /* 0x600 */ u8
        mStartEffect; // initLaunch: executeLaunch creates the start effect
    /* 0x601 */ u8 mBackEffect; // initBack: executeBack creates the sparks
    /* 0x604 */ int mFireTimer; // frames to the fire effect (-1: none)
}; // size 0x608

#endif
