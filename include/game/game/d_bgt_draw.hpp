#pragma once
#include <types.h>
#include <game/mLib/m_3d.hpp>
#include <game/game/d_m3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggFrmHeap.h>

// The city (街) bg drawer, d_bgt_drawNP (REL, profile BGT_DRAW). It loads the
// bg model of every block of the city's field ("/BgData/BgModel/%03d.brres",
// the block's bg id), plays its animations and draws it rolled onto the drum
// (dWorld) by its block row. Class name from RTTI ("dBgtDraw_c"); the rest are
// inferred.
class dBgtDraw_c : public fBase_c {
  public:
    // The model of one block.
    class block_c {
      public:
        BOOL load(void *heap, int blockX, int blockZ);
        BOOL create(mAllocator_c *allocator);
        BOOL draw(mAllocator_c *allocator);
        void remove(mAllocator_c *allocator);

        /* 0x000 */ u8 mBlockX;
        /* 0x001 */ u8 mBlockZ;
        /* 0x004 */ int mBgId;
        /* 0x008 */ dDvd::brresBank_c mBank;
        /* 0x060 */ m3d::mdl_c mMdl;
        /* 0x0A0 */ m3d::anmChr_c mAnmChr;
        /* 0x0D8 */ m3d::anmMatClr_c mAnmClr;
        /* 0x104 */ m3d::anmTexPat_c mAnmPat;
        /* 0x130 */ m3d::anmTexSrt_c mAnmSrt;
        /* 0x15C */ m3d::anmVis_c mAnmVis;
    }; // size 0x194

    // The blocks of the city.
    class blockMng_c {
      public:
        enum { BLOCK_X_MAX = 3, BLOCK_Z_MAX = 2 };

        bool load(void *heap);
        bool unload();
        bool create(mAllocator_c *allocator);
        bool execute(mAllocator_c *allocator);
        bool draw(mAllocator_c *allocator);
        void remove(mAllocator_c *allocator);

        /* 0x000 */ block_c mBlocks[BLOCK_Z_MAX][BLOCK_X_MAX];
    }; // size 0x978

    dBgtDraw_c() {
    }
    virtual ~dBgtDraw_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    void initHeap(EGG::Heap *parent);
    void deleteHeap();

    /* 0x064 */ blockMng_c mBlocks;
    /* 0x9DC */ EGG::FrmHeap *m_frmHeap_p;
    /* 0x9E0 */ mAllocator_c mAllocator;
    /* 0x9FC */ mVec3_c mCenter; // dBGCF::getNextCenter(0) once created
}; // size 0xA08
