#ifndef D_A_RMOBJ_CHECKP_HPP
#define D_A_RMOBJ_CHECKP_HPP

// Include order: the RTTI/vtable group order in .data (reverse class
// completion).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_bgcf.hpp>

// The gate of the check point (REL d_a_rmobj_checkpNP, profile A_RMOBJ_CHECKP).
// Class name from RTTI
// ("daRmobjChkp_c"). The door model ("obj_chk_door") plays animation 1 to open
// and 0 to close; a moving bg box blocks the passage while it is closed. The
// gatekeeper drives it through the functions below; the time-of-day light
// colour of material "m_light_anim" is set every draw.
class daRmobjChkp_c : public daRmobj_c {
  public:
    enum State_e {
        STATE_CLOSED,
        STATE_OPENING,
        STATE_OPEN,
        STATE_CLOSING,
        STATE_NUM
    };

    // The door animations (index into the model's animations).
    enum Anm_e {
        ANM_CLOSE,
        ANM_OPEN,
    };

    typedef BOOL (daRmobjChkp_c::*InitFunc)();
    typedef void (daRmobjChkp_c::*ExecFunc)();

    virtual int create();   // 0x3B8
    virtual int doDelete(); // 0x718
    virtual int execute();  // 0x46C
    virtual int draw();     // 0x510

    void setBgHeight(f32 height); // 0x770
    void updateBg();              // 0x780
    void removeBg();              // 0x830
    BOOL changeState(int state);  // 0x870
    void executeState();          // 0x8E4
    BOOL initClosed();            // 0x970
    void executeClosed();         // 0x988
    BOOL initOpening();           // 0x994
    void executeOpening();        // 0x9F4
    BOOL initOpen();              // 0xAC0
    void executeOpen();           // 0xAD8
    BOOL initClosing();           // 0xB3C
    void executeClosing();        // 0xB9C

    int getState() const {
        return mState;
    }

    static daRmobjChkp_c *s_instance;

    // The collision box of the closed gate (no RTTI; a member struct: the
    // dtor's double null check of the inlined mvbg_c dtor).
    struct bg_c {
        /* 0x00 */ dBGCF::mvbg_c mMvbg;
        /* 0x9C */ f32 mHeight; // the box's z offset (0 closed, -32 open)
        /* 0xA0 */ u8 mDirty;   // set by setBgHeight: updateBg re-adds the box
    }; // size 0xA4

    /* 0x15C */ bg_c mBg;
    /* 0x200 */ model_c mDoorMdl;
    /* 0x3AC */ int mState;
}; // size 0x3B0

// The gate controls of the gatekeeper: they act on the gate instance (NULL:
// FALSE).
namespace dARmobjCheckp {
BOOL isGateClosed(); // 0xC6C: STATE_CLOSED
BOOL isGateOpen();   // 0xC94: STATE_OPEN
BOOL openGate();     // 0xCC0: changeState(STATE_OPENING)
BOOL closeGate();    // 0xCE0: changeState(STATE_CLOSING)
} // namespace dARmobjCheckp

#endif
