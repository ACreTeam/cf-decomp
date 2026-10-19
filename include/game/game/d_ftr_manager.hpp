#pragma once

// The furniture manager (RTTI dFtrm_c, profile FTR_MANAGER, REL d_ftr_managerNP).

#include <game/framework/f_base.hpp>
#include <game/game/d_audio_obj.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_rec_bank.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/g3d.h>

class daFTR_c;
class dActor_c;

// A furniture or table-top item in reach of the player (dFtrm_c::findPutUnit).
struct putUnit_c {
    /* 0x0 */ int mX;
    /* 0x4 */ int mZ;
    /* 0x8 */ int mLayer;
    /* 0xC */ dItem::Item mItem;
    /* 0xE */ dItem::Item mDirItem;
}; // size 0x10

class dFtrm_c : public fBase_c {
public:
    // checkPutUnit / findPutUnit: the furniture's dFTR::list_c handle, or one of these.
    enum {
        PUT_UNIT_NONE = -1,
        PUT_UNIT_ITEM = -2, // an item lying on a table
    };

    // checkPut / checkPutRot / findPutPos and the putOrg* functions.
    enum {
        PUT_OK,
        PUT_INVALID, // not a furniture, or not in the player's house
        PUT_FULL,    // dFTR::list_c is full
        PUT_BLOCKED, // no free unit
    };

    // checkStand.
    enum {
        STAND_OK,
        STAND_BLOCKED,  // furniture, a step or a door
        STAND_OCCUPIED, // another actor
    };

    // getSeatPos / daFtrBed_c::checkSeat.
    enum {
        SEAT_BLOCKED,
        SEAT_BED,   // the bed itself, or an adjacent bed facing the same way
        SEAT_FLOOR, // walkable furniture or level floor
    };

    // Ftr/roomAlways.arc: the shared furniture resources.
    class loader_c : public dDvd::arcBank_c {
    public:
        virtual void onLoaded();

        void *loadFile(const char *name, EGG::Heap *heap);
        void *getFile(const char *name);
        BOOL load(void *heap);
    };

    class pool_c {
    public:
        pool_c() { clear(); }
        ~pool_c() {}

        void clear();
        BOOL create(u32 num);
        void *alloc();
        BOOL free(void *p);
        BOOL destroy();

        /* 0x00 */ u32 mNum;
        /* 0x04 */ u32 mSize;
        /* 0x08 */ u8 *mpData;
        /* 0x0C */ u8 mUsed[8];
    }; // size 0x14

    // The furniture being replaced by a newly made one (the catalog order box).
    struct current_c {
        current_c() { clear(); }

        void clear();
        void remove(daFTR_c *ftr);
        BOOL init();
        BOOL destroy();
        BOOL create(const dItem::Item *item);
        daFTR_c *get();
        void replace(daFTR_c *ftr);
        void removeAll();

        /* 0x00 */ daFTR_c *mOld;
        /* 0x04 */ daFTR_c *mNew;
        /* 0x08 */ u32 mItem;
    }; // size 0xC

    // The push / pull movement of a furniture (int_push / int_pull of int_common.brres).
    class transAnm_c {
    public:
        class nodeCallBack_c : public m3d::mdl_c::callback_c {
        public:
            virtual ~nodeCallBack_c() {}
            virtual void timingB(ulong nodeId, nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl resMdl);

            /* 0x04 */ mVec3_c mPos;
        }; // size 0x10

        transAnm_c();

        void create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmChr push, nw4r::g3d::ResAnmChr pull,
                    mAllocator_c *allocator);
        void remove();
        BOOL start(nw4r::g3d::ResAnmChr anm, const u32 *owner, const mVec3_c *pos, mAng angle);
        BOOL startPush(const u32 *owner, const mVec3_c *pos, mAng angle);
        BOOL startPull(const u32 *owner, const mVec3_c *pos, mAng angle);
        BOOL calc(const u32 *owner, mVec3_c *pos);

        /* 0x00 */ m3d::mdl_c mMdl;
        /* 0x40 */ m3d::anmChr_c mAnmChr;
        /* 0x78 */ nw4r::g3d::ResMdl mResMdl;
        /* 0x7C */ nw4r::g3d::ResAnmChr mPush;
        /* 0x80 */ nw4r::g3d::ResAnmChr mPull;
        /* 0x84 */ nodeCallBack_c mNodeCallBack;
        /* 0x94 */ const u32 *mpOwner;
    }; // size 0x98

    // An item popping out of a furniture (appearItem).
    struct appear_c {
        /* 0x00 */ mVec3_c mPos;
        /* 0x0C */ mVec3_c mTarget;
        /* 0x18 */ mVec3_c mSpeed;
        /* 0x24 */ f32 mScale;
        /* 0x28 */ dItem::Item mItem;
    }; // size 0x2C

    class appearList_c {
    public:
        appearList_c() { clear(); }

        void clear();
        void add(u16 item, const mVec3_c *pos);
        void calc();
        void draw();

        /* 0x00 */ appear_c mAppear[4];
    }; // size 0xB0

    // The TV programme texture: copies the frame's texture and palette into the TV's texture.
    struct texPatAnm_c {
        texPatAnm_c() { clear(); }

        void clear();
        void set(nw4r::g3d::ResFile file, nw4r::g3d::ResAnmTexPat anm);
        BOOL calc();

        /* 0x0 */ u32 mFrame;
        /* 0x4 */ nw4r::g3d::ResFile mFile;
        /* 0x8 */ nw4r::g3d::ResAnmTexPat mAnm;
        /* 0xC */ u32 mLoop;
    }; // size 0x10

    // The TV programmes (/Ftr/tv_program*.brres), double buffered.
    class tvPlayer_c {
    public:
        tvPlayer_c();

        static int getProgram();
        BOOL create();
        BOOL execute();
        BOOL remove();
        dSndTv_c *getSnd(u32 i) { return &mSnd[i]; }

        /* 0x00 */ u8 mEnabled;
        /* 0x01 */ u8 mBuffer;
        /* 0x02 */ u8 _02[0x06 - 0x02];
        /* 0x06 */ s16 mNext;
        /* 0x08 */ s16 mProgram;
        /* 0x0C */ u32 mForecast;
        /* 0x10 */ dSndTv_c mSnd[2];
        /* 0x20 */ EGG::FrmHeap *m_frmHeap_p[2];
        /* 0x28 */ dDvd::brresBank_c mBank[2];
        /* 0xD8 */ texPatAnm_c mAnm;
    }; // size 0xE8

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    static BOOL isFtrScene();
    BOOL loadRes();
    BOOL deleteRes();
    void checkFtr();
    void checkTalk();
    void init();
    void createChildren();
    void registerCatalog();
    static BOOL isLampScene();
    void initLight();
    void createFrmHeap();
    void destroyFrmHeap();
    void executeTv();
    void setAuctionFtr();
    void putDefaultFtr();
    void removeDefaultFtr();
    void createResetObj();
    void putLoftFtr();
    void createAllFtr();
    void createCockroach();

    static u16 getRotateSpeed();
    static u16 getRotateWait();
    static u16 getHoldWait();
    static u16 getHoldWaitNpc();
    static f32 getSeatCenterDist();
    static BOOL isDebug();
    static f32 getPushRate();
    static f32 getPullRate();
    static f32 getDpdHeight();
    static s16 getPushAngle();
    static f32 getPushDist();
    static int getDir(mAng angle);
    static dActor_c *createWallFtr(u32 idx, const dItem::Item &item);
    static dActor_c *createFtr(int x, int z, dItem::Item item, int rot, u8 layer, int kind);
    static dActor_c *createFtr(u32 info);
    static int checkPutUnit(putUnit_c *out, int x, int z, int layer, const mVec3_c *pos);
    static int findPutUnit(putUnit_c *out, const mVec3_c *pos, mAng angle, BOOL frontOnly);
    static int findPutUnit(putUnit_c *out, BOOL frontOnly);
    static int sendEvent(int handle);
    static BOOL canPass(int x, int z, int toX, int toZ);
    static int checkPutRot(u32 *info, int x, int z, int rot, dItem::Item item, u8 layer, const mVec3_c *pos);
    static int checkPut(u32 *info, int x, int z, int rot, dItem::Item item, u32 layer, int playerX,
                        int playerZ);
    static int findPutPos(u32 *info, dItem::Item item, const mVec3_c *pos, mAng angle, int kind);
    static int findPutPos(u32 *info, dItem::Item item, int kind);
    static int getPlayerIdx();
    static int putOrgItem(int base, u32 *info, int slot, int kind);
    static int putOrgCloth(u32 *info, int slot);
    static int putOrgEasel(u32 *info, int slot);
    static int putOrgUmb(u32 *info, int slot);
    static int putOrgCap(u32 *info, int slot);
    static BOOL getDirUnit(int *x, int *z, const mVec3_c *pos, mAng angle, int dir);
    static int checkStand(const mVec3_c *pos, mAng angle, BOOL snap, BOOL player, f32 dist);
    static BOOL canSit(const mVec3_c *pos, mAng angle);
    static BOOL canStandLeft(const mVec3_c *pos, mAng angle);
    static BOOL canStandRight(const mVec3_c *pos, mAng angle);
    static int getSeatPos(const mVec3_c *pos, mAng angle, const mVec3_c *ofs, mVec3_c *out);
    static int getSeatPosLeft(const mVec3_c *pos, mAng angle, mVec3_c *out);
    static int getSeatPosRight(const mVec3_c *pos, mAng angle, mVec3_c *out);
    static void turnOnSeatSound(const mVec3_c *pos);
    static void updateSeatSound(const mVec3_c *pos);
    static void turnOffSeatSound(const mVec3_c *pos);
    static BOOL canTalk(daFTR_c *ftr);
    static BOOL canTalk(int handle);
    static mVec3_c *getEffectPos(int handle);
    static void setEffectPos(const mVec3_c *pos);
    static pool_c *getPool();
    static void getOfsPos(mVec3_c *out, const mVec3_c *pos, mAng angle, f32 dist);
    static void appearItem(u16 item, const mVec3_c *pos);
    static BOOL getWallUnit(int *x, int *z, u32 i);
    static u16 getTimer();
    static current_c *getCurrent();
    static BOOL switchOn(daFTR_c *ftr);
    static BOOL switchOff(daFTR_c *ftr);

    /* 0x064 */ u8 mActive;
    /* 0x065 */ u8 mInited;
    /* 0x068 */ EGG::FrmHeap *m_frmHeap_p;
    /* 0x06C */ mAllocator_c mAllocator;
    /* 0x088 */ appearList_c mAppear;
    /* 0x138 */ u16 mTimer;
    /* 0x13C */ tvPlayer_c mTvPlayer;

    static const u32 sRecSize;
    static const u32 sMinResSize;
    static u8 sLampOn;
    static dRecBank::bankTbl_c sRecBank;
    static loader_c sLoader;
    static transAnm_c sTransAnm;
}; // size 0x224
