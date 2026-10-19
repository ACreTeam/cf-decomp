#ifndef D_A_RMOBJ_THEATER_HPP
#define D_A_RMOBJ_THEATER_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (m3d
// animations, fBase_c, dDvd, dActor_c, dMsg::Rcpt_c, dDemoActor_c, then
// daRmobj_c).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>

// The stage of the theater (REL d_a_rmobj_theaterNP, profile A_RMOBJ_THEATER):
// the curtain
// ("obj_tw_theater_dontyou", donchou = stage curtain) and the stage lights
// ("obj_tw_theater_light"). Opening and closing the curtain fades the theater
// light (dFixSpotTheaterLight_c) with it. Dr. Shrunk (d_a_npc_sp_sisyoNP)
// drives the curtain through the static functions. Class name from RTTI
// ("daRmobjTheater_c").
class daRmobjTheater_c : public daRmobj_c {
  public:
    // The curtain states (mState).
    enum State_e {
        STATE_CLOSED,  // closed, the light off
        STATE_OPENING, // the light fades in with the curtain animation
        STATE_OPEN,    // open, the light on
        STATE_CLOSING, // the light fades out with the curtain animation
        STATE_NUM
    };

    // The curtain's animations (both kinds of every model_c animation, by
    // index).
    enum CurtainAnm_e { CURTAIN_ANM_OPEN, CURTAIN_ANM_CLOSE };

    typedef BOOL (daRmobjTheater_c::*InitProc)();
    typedef void (daRmobjTheater_c::*ExecuteProc)();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    BOOL changeState(int state);
    void executeState();

    BOOL initClosed();
    void executeClosed();
    BOOL initOpening();
    void executeOpening();
    BOOL initOpen();
    void executeOpen();
    BOOL initClosing();
    void executeClosing();

    // Called by Dr. Shrunk (d_a_npc_sp_sisyoNP). All are false without a
    // theater.
    static BOOL isClosed();
    static BOOL isOpen();
    static BOOL open();
    static BOOL close();

    static daRmobjTheater_c *sInstance;

    /* 0x15C */ model_c mCurtain;
    /* 0x308 */ model_c mLight;
    /* 0x4B4 */ bool
        mLightOn; // the "obj_tw_tht_light_on" animation was set (first opening)
    /* 0x4B8 */ int mState;
}; // size 0x4BC

#endif
