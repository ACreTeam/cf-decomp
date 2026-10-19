#ifndef D_A_RMOBJ_GRACE_HPP
#define D_A_RMOBJ_GRACE_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (as d_a_rmobj_sample.hpp).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_hmn_palette_mng.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_allocator.hpp>

namespace EGG {
class FrmHeap;
}

// GracieGrace's mannequin (REL d_a_rmobj_graceNP, profile A_RMOBJ_GRACE): a
// mannequin wearing the outfit received from a friend town
// (dSaveExtra_c::mOutfit; Gracie's own top and hat without one). The body
// model gets the shirt's, the shoes' and the palette's textures; the head model
// depends on the hat (BITM::m_hideBone); the hat and the accessory are item
// models on the head node. Class name from RTTI ("daRmobjGrace_c").
class daRmobjGrace_c : public daRmobj_c {
  public:
    // The body model's chr animation (its pose).
    enum Anm_e { ANM_POSE };

    virtual int create();   // 0x4DC
    virtual int doDelete(); // 0x88C
    virtual int execute();  // 0x770
    virtual int draw();     // 0x798

    BOOL loadRes(); // 0x9B4: the items and the mannequin models; FALSE until
                    // all loaded
    void createMdlHeap(); // 0xC7C: m_frmHeap_p for the item models
    void deleteMdlHeap(); // 0xCE4
    void
    bindBodyTex(); // 0xD28: the shirt, shoes and palette textures on the body

    /* 0x15C */ dItem::resLoader_c mShirtRes;
    /* 0x1CC */ dItem::resLoader_c mHatRes;
    /* 0x23C */ dItem::resLoader_c mAccRes;
    /* 0x2AC */ m3d::mdl_c mHatMdl;
    /* 0x2EC */ m3d::mdl_c mAccMdl;
    /* 0x32C */ dHmnPaletteMng_c mPalette;
    /* 0x330 */ EGG::FrmHeap *m_frmHeap_p; // 0x7800 in dHeap::localHeap_p
    /* 0x334 */ mAllocator_c mAllocator;   // on m_frmHeap_p
    /* 0x350 */ dObjc::dpdBox_c mDpdBox;
    /* 0x6C0 */ model_c mModel;     // not used
    /* 0x86C */ model_c mBodyModel; // "g_mnqn_body_boy" / "_girl"
    /* 0xA18 */ model_c
        mHeadModel; // "g_mnqn_head_boy_a" / "_girl_a" (no full mask)
    /* 0xBC4 */ model_c
        mCapHeadModel; // "g_mnqn_head_boy_b" / "_girl_b" (a cap)
    /* 0xD70 */ model_c mBaldHeadModel; // "g_mnqn_head_hage" (the other hats)
}; // size 0xF1C

#endif
