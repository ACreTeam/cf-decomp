#pragma once
#include <types.h>
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_field_info.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <nw4r/g3d/res/g3d_resfile.h>

// The town's field (outdoor bg) drawer, d_bgo_drawNP (REL, profile BGO_DRAW).
// It draws the model of every loaded field block (dBG::mdlBank_c) with its
// river / wave animations, keeps the grass, soil, stone and ocean materials'
// texture scrolls, colours and z modes in step with the camera, the season and
// the time of day, places the waterfall / pond / river sounds at the block's
// sound points, and works out the bg y level and the distance to the sea. Class
// names from RTTI
// ("dBgoDraw_c", "dBgoDraw_HIO_c"); the rest are inferred.
class dBgoDraw_c : public fBase_c {
  public:
    enum {
        SEA_UNIT_NUM = 0x70, // field units along x whose sea edge z is kept
    };

    // The model of one field block.
    class block_c {
      public:
        enum STATE_e { STATE_CREATE, STATE_EXECUTE, STATE_NUM };
        enum { SOUND_NUM = 6 };

        // The block's sound points (dBG::bsd_c ids) and their sounds.
        enum SOUND_e {
            SOUND_POINT_FALL = 5,
            SOUND_POINT_POND = 0xF,
            SOUND_POINT_RIVER = 0x11,
        };

        block_c() {
            mState = STATE_CREATE;
        }

        void setMaterials(u32 blockX, u32 blockZ, u32 flags);
        void create(u32 blockX, u32 blockZ, nw4r::g3d::ResFile pack,
                    mAllocator_c *allocator, int init);
        void execute(u32 blockX, u32 blockZ, nw4r::g3d::ResFile pack,
                     mAllocator_c *allocator, int init);
        void proc(u32 blockX, u32 blockZ, nw4r::g3d::ResFile pack,
                  mAllocator_c *allocator, int init);
        void draw(u32 blockX, u32 blockZ, nw4r::g3d::ResFile pack);
        void remove(u32 blockX, u32 blockZ, nw4r::g3d::ResFile pack);
        int getSoundPoint(f32 *x, f32 *z, u32 blockX, u32 blockZ, u32 idx);

        /* 0x000 */ u32 mAnmFrameNum;
        /* 0x004 */ u8 _4;
        /* 0x005 */ u8 mState;
        /* 0x008 */ m3d::mdl_c mMdl;
        /* 0x048 */ m3d::anmTexSrt_c mAnmSrt;
        /* 0x074 */ m3d::anmMatClr_c mAnmClr;
        /* 0x0A0 */ u8 mIsEdge;
        /* 0x0A1 */ u8 mPrioOpa;
        /* 0x0A2 */ u8 mPrioXlu;
        /* 0x0A4 */ mVec3_c mSoundPos[SOUND_NUM];
        /* 0x0EC */ u32 mSoundId[SOUND_NUM];
        /* 0x104 */ void *mpSound[SOUND_NUM];
        /* 0x11C */ int mSoundNum;
    }; // size 0x120

    // The models of the town's blocks.
    class blockMng_c {
      public:
        void proc(nw4r::g3d::ResFile pack, mAllocator_c *allocator, int init);
        void draw(nw4r::g3d::ResFile pack);
        void remove(nw4r::g3d::ResFile pack);

        /* 0x0000 */ block_c mBlocks[BLOCK_Z_NUM][BLOCK_X_NUM];
    }; // size 0x3720

    dBgoDraw_c() {
    }
    virtual ~dBgoDraw_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    BOOL initHeap(EGG::Heap *parent);
    void calcBgY();
    void calcSeaColor();
    void calcGrassTrans();
    void initSeaZ();
    void calcSeaDist();
    u8 getScene();
    f32 getSeaZ(u32 unitX);

    /* 0x0064 */ blockMng_c mBlocks;
    /* 0x3784 */ EGG::FrmHeap *m_g3dHeap_p;
    /* 0x3788 */ mAllocator_c mAllocator;
    /* 0x37A4 */ f32 mSeaZ[SEA_UNIT_NUM];
}; // size 0x3964
