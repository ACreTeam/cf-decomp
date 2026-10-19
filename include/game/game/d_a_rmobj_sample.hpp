#ifndef D_A_RMOBJ_SAMPLE_HPP
#define D_A_RMOBJ_SAMPLE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (m3d animations, fBase_c, dActor_c, dDvd, dMsg::Rcpt_c, dDemoActor_c, then
// daRmobj_c::model_c).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>

// The sample room object (REL d_a_rmobj_sampleNP, profile A_RMOBJ_SAMPLE): a
// box model ("obj_r_box") playing its chr animation. The template of the
// rmobj actors. Class name from RTTI ("daRmobjSample_c").
class daRmobjSample_c : public daRmobj_c {
  public:
    virtual int create();   // 0x1C4
    virtual int doDelete(); // 0x298
    virtual int execute();  // 0x248
    virtual int draw();     // 0x270

    /* 0x15C */ model_c mModel;
}; // size 0x308

#endif
