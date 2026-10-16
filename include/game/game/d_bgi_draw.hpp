#pragma once
#include <types.h>
#include <cstdio>
#include <game/mLib/m_3d.hpp>
#include <game/game/d_m3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_bg.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggFrmHeap.h>

// The interior (room) bg drawer, d_bgi_drawNP (REL, profile BGI_DRAW). It draws
// the room model of the current scene ("/BgData/BgModel/%03d.brres", the bg id
// of field block 0, 0) with its animations, the sun window model and the clock,
// the models of the other blocks of the room, and the wallpaper / carpet
// textures, which it streams in when they change (request_c). It registers
// itself with dBgUtil::setBgiDraw (d_bg_util.cpp) while it exists. Class names
// from RTTI
// ("dBgiDraw_c", "dBgiDraw_c::loader_c", "dBgiDraw_c::nodeCallBack_c",
// "dBgiDraw_c::sunNodeCallBack_c", "dBgiDraw_hostIO_c"); the rest are inferred.

namespace dBGI {

// The room's wallpaper / carpet and model queries that the DOL reaches through
// dBgUtil (RTTI "dBGI::alwaysAc_c").
class alwaysAc_c {
  public:
    // The wallpaper; NONE while it is an original design.
    virtual dItem::Item getWallpaper() const = 0;
    // Request a new wallpaper: wrap is the texture wrap mode (replaced for
    // wallpaper / carpet items).
    virtual BOOL setWallpaper(dItem::Item item, int wrap, int flag,
                              int playSe) = 0;
    // No wallpaper change pending.
    virtual BOOL isWallpaperDone() const = 0;
    // The carpet; NONE while it is an original design.
    virtual dItem::Item getCarpet() const = 0;
    // The carpet as set.
    virtual dItem::Item getCarpetRaw() const = 0;
    virtual BOOL setCarpet(dItem::Item item, int wrap, int flag,
                           int playSe) = 0;
    virtual BOOL isCarpetDone() const = 0;
    // World position of a model node of a block (0, 0 is the room model).
    virtual BOOL getNodePos(mVec3_c *pos, int blockX, int blockZ,
                            const char *name) = 0;
    // The node count of a block's model.
    virtual int getNodeNum(int blockX, int blockZ) = 0;
    // The name of node idx of a block's model; with pos, also its world
    // position.
    virtual const char *getNodeName(u32 idx, int blockX, int blockZ,
                                    mVec3_c *pos) = 0;
    virtual EGG::FrmHeap *getFrmHeap() = 0;
    // The room model is ready.
    virtual BOOL isReady() = 0;
};

} // namespace dBGI

class dBgiDraw_c : public fBase_c,
                   public dDvd::brresBank_c,
                   public dBGI::alwaysAc_c {
  public:
    // A wallpaper / carpet texture loader with its own heap.
    class loader_c : public dItem::resLoader_c {
      public:
        BOOL load(dItem::Item item);
        nw4r::g3d::ResTex getTex();
        nw4r::g3d::ResPltt getPltt();
        void freeHeap();

        /* 0x70 */ EGG::FrmHeap *mpFrmHeap;
    }; // size 0x74

    // The wallpaper or the carpet: the current item and a pending change,
    // double-buffered.
    class request_c {
      public:
        enum { LOADER_NUM = 2 };

        request_c();
        void init();
        bool createHeap(EGG::Heap *parent, u32 size);
        void destroyHeap();
        BOOL set(dItem::Item item, int wrap, int flag, int playSe);
        void finish();
        bool release();
        void playSe();
        void next();

        dItem::Item getCurrent() const {
            return mCurrent;
        }
        void setCurrent(dItem::Item item) {
            mCurrent = item;
        }
        dItem::Item getPending() const {
            return mPending;
        }
        int getWrap() const {
            return mWrap;
        }
        loader_c *getLoader(u32 idx) {
            return &mLoaders[idx % LOADER_NUM];
        }

        /* 0x00 */ u8 mLoaded;
        /* 0x01 */ u8 mBuf; // the loader of the current item
        /* 0x02 */ dItem::Item mCurrent;
        /* 0x04 */ dItem::Item mPending;
        /* 0x08 */ int mWrap; // GXTexWrapMode
        /* 0x0C */ u8 mPlaySe;
        /* 0x10 */ loader_c mLoaders[LOADER_NUM];
        /* 0xF8 */ u8 mFlag; // hides the material's KColor1 alpha
    }; // size 0xFC

    // The clock hands ("kh_j", "km_j") of a model.
    class nodeCallBack_c : public m3d::mdl_c::callback_c {
      public:
        nodeCallBack_c() {
            mHourNode = mMinuteNode = -1;
        }
        virtual ~nodeCallBack_c() {
        }
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes,
                             nw4r::g3d::ResMdl resMdl);

        /* 0x04 */ int mHourNode;
        /* 0x08 */ int mMinuteNode;
    }; // size 0xC

    // Keeps the sun model's nodes at their bind positions.
    class sunNodeCallBack_c : public m3d::mdl_c::callback_c {
      public:
        virtual ~sunNodeCallBack_c() {
        }
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes,
                             nw4r::g3d::ResMdl resMdl);
    }; // size 0x4

    // The model of another block of the room.
    class block_c {
      public:
        BOOL load(void *heap, int blockX, int blockZ);
        BOOL create(mAllocator_c *allocator);
        BOOL execute(mAllocator_c *allocator);
        BOOL draw(mAllocator_c *allocator);
        void remove(mAllocator_c *allocator);
        BOOL getNodePos(mVec3_c *pos, const char *name) const;
        int getNodeNum();
        const char *getNodeName(u32 idx);

        /* 0x000 */ u8 mBlockX;
        /* 0x001 */ u8 mBlockZ;
        /* 0x004 */ int mBgId;
        /* 0x008 */ dDvd::brresBank_c mBank;
        /* 0x060 */ m3d::mdl_c mMdl;
        /* 0x0A0 */ m3d::anmChr_c mAnmChr;
        /* 0x0D8 */ m3d::anmMatClr_c mAnmClr;
        /* 0x104 */ m3d::anmTexSrt_c mAnmSrt;
        /* 0x130 */ m3d::anmVis_c mAnmVis;
        /* 0x168 */ m3d::anmTexPat_c mAnmPat;
    }; // size 0x194

    // The blocks of the room other than 0, 0 (that is the room model).
    class blockMng_c {
      public:
        enum { BLOCK_X_MAX = 3, BLOCK_Z_MAX = 2 };

        BOOL isValid(int blockX, int blockZ);
        bool isReady();
        BOOL load(void *heap);
        bool unload();
        bool create(mAllocator_c *allocator);
        bool execute(mAllocator_c *allocator);
        bool draw(mAllocator_c *allocator);
        void remove(mAllocator_c *allocator);
        BOOL getNodePos(mVec3_c *pos, int blockX, int blockZ, const char *name);
        int getNodeNum(int blockX, int blockZ);
        const char *getNodeName(u32 idx, int blockX, int blockZ);

        /* 0x000 */ block_c mBlocks[BLOCK_Z_MAX][BLOCK_X_MAX];
        /* 0x978 */ int mNumX;
        /* 0x97C */ int mNumZ;
    }; // size 0x980

    dBgiDraw_c() {
    }
    virtual ~dBgiDraw_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    virtual dItem::Item getWallpaper() const;
    virtual dItem::Item getCarpet() const;
    virtual dItem::Item getCarpetRaw() const;
    virtual BOOL setWallpaper(dItem::Item item, int wrap, int flag,
                              int playSe) {
        return mWallReq.set(item, wrap, flag, playSe);
    }
    virtual BOOL isWallpaperDone() const {
        return mWallReq.getPending() == dItem::ITEM_ID_NONE;
    }
    virtual BOOL setCarpet(dItem::Item item, int wrap, int flag, int playSe) {
        return mCarpetReq.set(item, wrap, flag, playSe);
    }
    virtual BOOL isCarpetDone() const {
        return mCarpetReq.getPending() == dItem::ITEM_ID_NONE;
    }
    virtual BOOL getNodePos(mVec3_c *pos, int blockX, int blockZ,
                            const char *name);
    virtual int getNodeNum(int blockX, int blockZ);
    virtual const char *getNodeName(u32 idx, int blockX, int blockZ,
                                    mVec3_c *pos);
    virtual BOOL isReady();
    virtual EGG::FrmHeap *getFrmHeap() {
        return m_frmHeap_p;
    }

    nw4r::g3d::ResFile getResFile();
    nw4r::g3d::ResMdl getResMdl();
    nw4r::g3d::ResMat getResMat(const char *name);
    u32 getTexIdx(nw4r::g3d::ResMat mat, const char *name);
    void setTexSrt(nw4r::g3d::ResMat mat, const char *name, f32 scaleX,
                   f32 scaleY, f32 transX, f32 transY);
    void setTex(nw4r::g3d::ResTex tex, nw4r::g3d::ResPltt pltt, int wrapS,
                int wrapT, const char *matName, const char *texName,
                const char *plttName, f32 scaleX, f32 scaleY, f32 transX,
                f32 transY);
    void setWallTex(nw4r::g3d::ResTex tex, nw4r::g3d::ResPltt pltt, int wrapS,
                    int wrapT, f32 scaleX, f32 scaleY, f32 transX, f32 transY);
    void setCarpetTex(nw4r::g3d::ResTex tex, nw4r::g3d::ResPltt pltt, int wrapS,
                      int wrapT, f32 scaleX, f32 scaleY, f32 transX,
                      f32 transY);
    BOOL loadRoomItems();
    void calcRequests();
    bool releaseRequests();
    void debugDraw();
    void initHeap();
    void deleteHeap();
    void init();
    mColor getSunColor();
    void setSunColor(const mColor &color);
    void calcSun(const mColor &color);
    void calcClock();
    BOOL isTownBuildingRoom();

    static void getModelPath(char *buf, u16 bgId) {
        sprintf(buf, "/BgData/BgModel/%03d.brres", bgId);
    }
    BOOL loadModel(void *heap) {
        char path[0x34];
        getModelPath(path, mBgId);
        return load(path, heap, 0);
    }

    /* 0x0C0 */ dSndObjSimple_c mSndObj; // the clock's tick
    /* 0x0F0 */ mVec3_c mSndPos;
    /* 0x0FC */ u8 mSndStarted;
    /* 0x100 */ int mLastSecond;
    /* 0x104 */ u8 mIsPlayerHouse;
    /* 0x105 */ u8 mWallFlag;
    /* 0x106 */ u8 mCarpetFlag;
    /* 0x107 */ u8 mIsTownBuildingRoom;
    /* 0x108 */ dBG::packBank_c mPackBank;
    /* 0x164 */ dBG::roomAlwaysBank_c mRoomAlwaysBank;
    /* 0x1D8 */ int mBgId;
    /* 0x1DC */ u8 mHasField;
    /* 0x1E0 */ EGG::FrmHeap *m_frmHeap_p;
    /* 0x1E4 */ mAllocator_c mAllocator;
    /* 0x200 */ m3d::mdl_c mMdl;
    /* 0x240 */ m3d::anmChr_c mAnmChr;
    /* 0x278 */ m3d::anmMatClr_c mAnmClr;
    /* 0x2A4 */ m3d::anmTexPat_c mAnmPat;
    /* 0x2D0 */ m3d::anmTexSrt_c mAnmSrt;
    /* 0x2FC */ m3d::anmVis_c mAnmVis;
    /* 0x334 */ m3d::mdl_c mSunMdl;
    /* 0x374 */ m3d::anmChr_c mSunAnmChr;
    /* 0x3AC */ m3d::anmVis_c mSunAnmVis;
    /* 0x3E4 */ m3d::anmTexSrt_c mSunAnmSrt;
    /* 0x410 */ u8
        mAnmPatFixed; // the house size picks the texture pattern frame
    /* 0x414 */ m3d::mdl_c mClockMdl;
    /* 0x454 */ m3d::anmChr_c mClockAnmChr;
    /* 0x48C */ request_c mWallReq;
    /* 0x588 */ request_c mCarpetReq;
    /* 0x684 */ nodeCallBack_c mNodeCallBack;
    /* 0x690 */ nodeCallBack_c mClockNodeCallBack;
    /* 0x69C */ sunNodeCallBack_c mSunNodeCallBack;
    /* 0x6A0 */ blockMng_c mBlocks;
}; // size 0x1020

namespace dBgiDraw {

dItem::Item getStoreWallpaper();
dItem::Item getSceneWallpaper(int *wrap);
dItem::Item getGraceItem(int kind);
dItem::Item getGraceCarpet();
dItem::Item getGraceWallpaper();
dItem::Item getStoreCarpet();
dItem::Item getSceneCarpet(int *wrap);

} // namespace dBgiDraw
