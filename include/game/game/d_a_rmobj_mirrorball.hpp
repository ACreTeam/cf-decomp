#ifndef D_A_RMOBJ_MIRRORBALL_HPP
#define D_A_RMOBJ_MIRRORBALL_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (here
// dDvd before dActor_c, unlike d_a_rmobj_sample).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>

// The stage of the Roost with its mirror ball (REL d_a_rmobj_mirrorballNP,
// profile A_RMOBJ_MIRRORBALL, model "obj_cafe_stage"). Its two light materials
// ("m_cf_mlight", "m_cf_mlight1") fade in and out (the alpha of konst color 1).
// Class name from RTTI ("daRmobjMirrorBall_c").
class daRmobjMirrorBall_c : public daRmobj_c {
  public:
    virtual int create();   // 0x1C4
    virtual int doDelete(); // 0x44C
    virtual int execute();  // 0x298
    virtual int draw();     // 0x424

    /* 0x15C */ model_c mModel;
    /* 0x308 */ f32 mLight0; // brightness of the first light (m_cf_mlight1)
    /* 0x30C */ f32 mLight1; // ... of the others (m_cf_mlight)
    /* 0x310 */ u8 mLight0On;
    /* 0x311 */ u8 mLight1On;

    static daRmobjMirrorBall_c *sInstance; // the stage in the scene
}; // size 0x314

#endif
