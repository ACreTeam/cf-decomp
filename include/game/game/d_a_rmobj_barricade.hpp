#ifndef D_A_RMOBJ_BARRICADE_HPP
#define D_A_RMOBJ_BARRICADE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (m3d
// animations, fBase_c, dActor_c, dDvd, dMsg::Rcpt_c, dDemoActor_c, then
// daRmobj_c), as d_a_rmobj_sample.hpp.
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_objc.hpp>

// The barrier poles in front of a closed building entrance (REL
// d_a_rmobj_barricadeNP, profile A_RMOBJ_BARRICADE): "obj_tw_theater_ent_pole"
// in front of the theater (SCENE_RM_THEATER_ENT), "obj_tw_HRA_ent_pole" in
// front of the Happy Room Academy elsewhere. It is not placed while the
// building is open (Param_s hours) or when the player just came out of it;
// otherwise it blocks the way (moving bg collision and a pointer box) and shows
// the "closed" door message when talked to. The ticket counter
// (d_a_ticket_counterNP) checks isPlaced. Class name from RTTI
// ("daRmobjBarricade_c").
class daRmobjBarricade_c : public daRmobj_c {
  public:
    // The placement data mParam points to.
    struct Param_s {
        /* 0x00 */ u32
            mScene; // not placed when coming from this scene (the building)
        /* 0x04 */ u32 mMsgCode;   // the door message (sys_MESS/SYS_Door)
        /* 0x08 */ u32 mOpenHour;  // not placed from this hour...
        /* 0x0C */ u32 mCloseHour; // ...to this one (exclusive)
        /* 0x10 */ u32 mDemoID;
        /* 0x14 */ f32 mSizeX; // the collision box
        /* 0x18 */ f32 mSizeZ;
    }; // size 0x1C

    // The states (mState).
    enum State_e {
        STATE_WAIT,
        STATE_TALK_START, // the door message was requested (DEMO_CMD_TALK)
        STATE_TALK,       // the message is shown; ends the talk when it closes
        STATE_NUM
    };

    typedef BOOL (daRmobjBarricade_c::*InitProc)();
    typedef void (daRmobjBarricade_c::*ExecuteProc)();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual bool canInteract(dDemoActor_c *actor);
    virtual void onDemoCmd(int cmd, u8 member);

    virtual void onMessageStart(int arg);
    virtual void onMessageEnd(int kind);
    virtual void onAnswer(int arg);

    // Sets the pointer box (32 high) at the position.
    void setBox(f32 sizeX, f32 sizeZ) {
        mBox.set(mPos, sizeX, sizeZ, 32.0f, mAngle.y, dObjc::HIT_RMOBJ, this,
                 0);
    }

    BOOL changeState(int state);
    void executeState();

    BOOL initWait();
    void executeWait();
    BOOL initTalkStart();
    void executeTalkStart();
    BOOL initTalk();
    void executeTalk();

    // TRUE while a barricade is placed (d_a_ticket_counterNP).
    static bool isPlaced();

    static bool sPlaced;

    /* 0x15C */ dBGCF::mvbg_c mBg;
    /* 0x1F8 */ model_c mModel;
    /* 0x3A4 */ dObjc::dpdBox_c mBox;
    /* 0x714 */ int mState;
}; // size 0x718

#endif
