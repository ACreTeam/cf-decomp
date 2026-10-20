#pragma once
#include <game/mLib/m_mtx.hpp>
#include <types.h>
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_item.hpp>
#include <game/mLib/m_allocator.hpp>
#include <lib/egg/core/eggFrmHeap.h>

// The original design objects, d_org_objNP (REL, profile ORG_OBJ): the models
// the design editor (d_menu_editNP) shows a design on, from dFTR's always.arc:
// the shirt ("dsnCloth.brres"), the umbrella ("dsnUmb.brres"), the hat on its
// stand ("dsnCap.brres", "capStand.brres") and "dsnCat.brres". Their textures
// come from the design item's resLoader_c (replaceTexture swaps in the design
// being edited); the editor reaches the object through sInstance. Class names
// from RTTI ("dOrgObj_c", "dOrgObj_hostIO_c"); the rest are inferred.
class dOrgObj_c : public fBase_c {
  public:
    // The draw bits (mDrawFlags).
    enum {
        DRAW_CLOTH = 0,
        DRAW_UMB = 1,
        DRAW_CAP = 2,
        DRAW_CAT = 3,
    };

    // The shirt: its texture scroll / colour animation picks the sleeve
    // pattern (setClothAnm).
    class cloth_c {
      public:
        /* 0x00 */ m3d::mdl_c mMdl;
        /* 0x40 */ m3d::anmTexSrt_c mAnmSrt;
        /* 0x6C */ m3d::anmMatClr_c mAnmClr;
        /* 0x98 */ int mAnm; // < 0: no animation
    }; // size 0x9C

    // The umbrella.
    class umb_c {
      public:
        /* 0x00 */ m3d::mdl_c mMdl;
        /* 0x40 */ m3d::anmChr_c mAnmChr;
    }; // size 0x78

    // The hat, placed at the "set" node of its stand.
    class cap_c {
      public:
        /* 0x00 */ m3d::mdl_c mMdl;
        /* 0x40 */ m3d::mdl_c mStandMdl;
    }; // size 0x80

    // "dsnCat.brres".
    class cat_c {
      public:
        /* 0x00 */ m3d::mdl_c mMdl;
    }; // size 0x40

    dOrgObj_c() {
    }
    virtual ~dOrgObj_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    void replaceTexture(void *tex, void *pltt);
    void setLight(m3d::bmdl_c *mdl);
    BOOL loadRes();
    bool releaseRes();
    void initHeap();
    void deleteHeap();

    void bindCloth();
    BOOL createCloth();
    BOOL executeCloth();
    void calcCloth(const mMtx_c *mtx);
    BOOL drawCloth();
    BOOL removeCloth();
    void setClothAnm(int anm);

    void bindUmb();
    BOOL createUmb();
    BOOL executeUmb();
    void calcUmb(const mMtx_c *mtx);
    BOOL drawUmb();
    BOOL removeUmb();

    void bindCap();
    BOOL createCap();
    BOOL executeCap();
    void calcCap(const mMtx_c *mtx);
    BOOL drawCap();
    BOOL removeCap();

    void bindCat();
    BOOL createCat();
    BOOL executeCat();
    BOOL drawCat();
    void calcCat(const mMtx_c *mtx);
    BOOL removeCat();

    // Places every model at mtx (NULL: the debug position).
    void calc(const mMtx_c *mtx);

    BOOL isDraw(int kind) const {
        return (mDrawFlags >> kind) & 1;
    }

    static dOrgObj_c *sInstance;

    /* 0x064 */ u32 mDrawFlags; // DRAW_* bits, set by the design editor
    /* 0x068 */ EGG::FrmHeap *m_frmHeap_p;
    /* 0x06C */ mAllocator_c mAllocator;
    /* 0x088 */ dItem::resLoader_c mLoader; // the design item 0x372
    /* 0x0F8 */ cloth_c mCloth;
    /* 0x194 */ umb_c mUmb;
    /* 0x20C */ cap_c mCap;
    /* 0x28C */ cat_c mCat;
}; // size 0x2CC
