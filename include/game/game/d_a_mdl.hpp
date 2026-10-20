#ifndef GAME_D_A_MDL_HPP
#define GAME_D_A_MDL_HPP

// A generic model actor (REL d_a_mdlNP, profile A_MDL). A leftover test actor:
// it loads "Sample/cat.brres" into its own frame heap, shows the model with a
// "wait" animation and a "cat_0" texture pattern animation, and each press of
// the A button of the current controller cycles a part animation of the legs
// and left arm ("walk", then "run", then none). Class name from the RTTI
// ("dAcMdl_c"); everything else is inferred.

// The include order sets the weak RTTI order in the data section: m_3d (banm_c,
// fanm_c, anmChr_c), then d_actor (dActor_c, fBase_c), then d_m3d
// (anmChrPart_c).
#include <game/game/d_dvd.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_m3d.hpp>

class dAcMdl_c : public dActor_c {
  public:
    virtual int create();   // .text:0x13C
    virtual int doDelete(); // .text:0x1D4
    virtual int execute();  // .text:0x1F8
    virtual int draw();     // .text:0x380
    virtual ~dAcMdl_c();    // .text:0x69C

    BOOL createFrmHeap();  // .text:0x3B0
    void destroyFrmHeap(); // .text:0x418
    void setupModel();     // .text:0x4A8
    void setupMtx();       // .text:0x628

    /* 0x0A0 */ EGG::FrmHeap *mpFrmHeap;
    /* 0x0A4 */ mAllocator_c mAllocator;
    /* 0x0C0 */ void *mpRes; // the loaded .brres
    /* 0x0C4 */ nw4r::g3d::ResFile mRes;
    /* 0x0C8 */ dDvd::loader_c mLoader;
    /* 0x0DC */ m3d::mdlEx_c mMdl;
    /* 0x108 */ m3d::anmChr_c mAnmChr;
    /* 0x140 */ m3d::anmChrPart_c mAnmChrPart;
    /* 0x178 */ m3d::anmTexPat_c mAnmTexPat;
}; // size 0x1A4

#endif
