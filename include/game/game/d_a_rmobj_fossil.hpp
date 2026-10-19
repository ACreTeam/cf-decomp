#ifndef D_A_RMOBJ_FOSSIL_HPP
#define D_A_RMOBJ_FOSSIL_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (see
// d_a_rmobj_sample.hpp).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_bgcf.hpp>

// The fossil exhibits of the museum (REL d_a_rmobj_fossilNP, profile
// A_RMOBJ_FOSSIL): one model per fossil room ("obj_ms_fossil_a" in
// SCENE_RM_MM_FOSSIL0, "obj_ms_fossil_b" otherwise) whose nodes are the fossil
// parts; the ones not donated yet are hidden. Class name from RTTI
// ("daRmobjFossil_c").
class daRmobjFossil_c : public daRmobj_c {
  public:
    virtual int create();   // 0x4CC
    virtual int doDelete(); // 0x634
    virtual int execute();  // 0x604
    virtual int draw();     // 0x60C

    /* 0x15C */ dBGCF::mvbg_c mBg0; // never added in this build
    /* 0x1F8 */ dBGCF::mvbg_c mBg1;
    /* 0x294 */ model_c mModel;
}; // size 0x440

#endif
