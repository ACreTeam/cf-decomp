#ifndef D_A_RMOBJ_PIC_HPP
#define D_A_RMOBJ_PIC_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (see
// d_a_rmobj_sample.hpp).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>

// The paintings of the museum's art gallery (REL d_a_rmobj_picNP, profile
// A_RMOBJ_PIC): one model per gallery room ("obj_art_A" in
// SCENE_RM_MM_PIC0, "obj_art_B" otherwise) whose nodes "art_*" are the 23
// paintings; the ones not donated yet are hidden. Class name from RTTI
// ("daRmobjPic_c").
class daRmobjPic_c : public daRmobj_c {
  public:
    virtual int create();   // 0x1C4
    virtual int doDelete(); // 0x300
    virtual int execute();  // 0x2D0
    virtual int draw();     // 0x2D8

    /* 0x15C */ model_c mModel;
}; // size 0x308

#endif
