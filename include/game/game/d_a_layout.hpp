#ifndef GAME_D_A_LAYOUT_HPP
#define GAME_D_A_LAYOUT_HPP

// A layout test actor (REL d_a_layoutNP, profile A_LAYOUT). A leftover: it
// loads "Layout/bal.arc", builds the speech balloon layout "balloon_00.brlyt"
// and plays its "balloon_00_blnIn.brlan" animation, then draws it. Class name
// from the RTTI ("dAcLayout_c"); everything else is inferred.
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>

class dAcLayout_c : public fBase_c {
  public:
    virtual int create();   // .text:0xC0
    virtual int doDelete(); // .text:0x168
    virtual int execute();  // .text:0x198
    virtual int draw();     // .text:0x1E0
    virtual ~dAcLayout_c(); // .text:0x208

    /* 0x064 */ m2d::ResAccLoader_c mResAcc;
    /* 0x138 */ m2d::Simple_c mLyt;
    /* 0x1D0 */ m2d::Anm_c mAnm;
}; // size 0x1F4

#endif
