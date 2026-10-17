#pragma once

// The museum fish tanks (REL d_fish_museumNP): the manager, the tanks, and the fish and decoration bases.

#include <types.h>
#include <nw4r/g3d.h>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_frustum.hpp>
#include <game/framework/f_base.hpp>
#include <game/framework/f_profile.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_world.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/math/eggQuat.h>
#include <game/game/d_snd_obj.hpp>

class dFishMusBase_c;
class dFishMus_c;

struct dFishMusData_c {
    enum HitType_e {
        HIT_CYLINDER, // radius mHitLength, height mHitHeight
        HIT_CAPSULE,  // along the heading, half length mHitLength - mHitWidth
        HIT_BOX,      // the 4 corners of dFishMusEel_c (in the tanks: half sizes mHitWidth / mHitLength)
        HIT_NONE,
    };

    /* 0x00 */ int mAquariumIdx;   // dFishMuseum::sAquariums
    /* 0x04 */ int mHitType;       // HitType_e
    /* 0x08 */ f32 mHitWidth;
    /* 0x0C */ f32 mHitHeight;
    /* 0x10 */ f32 mHitLength;
    /* 0x14 */ f32 mSwimY;
    /* 0x18 */ f32 _18;
    /* 0x1C */ f32 mSpeed;
    /* 0x20 */ int mSwimTimeMax;   // dFishMusCommonMove_c::setSwimTimer
    /* 0x24 */ int mSwimTimeMin;
    /* 0x28 */ int mWaitTimeMax;   // dFishMusCommonMove_c::setWaitTimer
    /* 0x2C */ int mWaitTimeMin;
    /* 0x30 */ s16 mTurnAngle;
    /* 0x34 */ f32 mSwimYMax;
    /* 0x38 */ f32 mSwimYMin;
    /* 0x3C */ s16 mPitchDeg;
    /* 0x40 */ f32 mAnmRateFast;
    /* 0x44 */ f32 mAnmRateSlow;
    /* 0x48 */ f32 mMdlOffsetZ;    // dFishMusBase_c::mMdlOffset.z
    /* 0x4C */ f32 mCenterOffsetZ; // dFishMusBase_c::mCenterOffset.z
    /* 0x50 */ f32 mTrailStep;     // dFishMusMdl_c::mTrailStep
    /* 0x54 */ f32 mWeight;        // share of a push between two fish
    /* 0x58 */ f32 mCullRadius;

    void getHalfSizeXZ(f32 cosY, f32 &halfX, f32 &halfZ) const {
        f32 c = nw4r::math::FAbs(cosY);
        halfX = mHitWidth * c + mHitLength * (1.0f - c);
        halfZ = mHitWidth * (1.0f - c) + mHitLength * c;
    }
}; // size 0x5C

enum dFishMusKind_e {
    FISH_TANAGO,
    FISH_OIKAWA,
    FISH_FUNA,
    FISH_UGUI,
    FISH_NIGOI,
    FISH_KOI,
    FISH_NISHIKI,
    FISH_KINGYO,
    FISH_DEMEKIN,
    FISH_MEDAKA,
    FISH_ZARIGANI,
    FISH_KAERU,
    FISH_DONKO,
    FISH_DOJOU,
    FISH_NAMAZU,
    FISH_UNAGI,
    FISH_RAIGYO,
    FISH_BLUE,
    FISH_YELLOW,
    FISH_BLACK,
    FISH_PIKE,
    FISH_WAKASAGI,
    FISH_AYU,
    FISH_YAMAME,
    FISH_OOIWANA,
    FISH_NIJIMASU,
    FISH_ITOU,
    FISH_SAKE,
    FISH_KING,
    FISH_GUPPI,
    FISH_ANGEL,
    FISH_NEON,
    FISH_PIRANIA,
    FISH_AROWANA,
    FISH_DORADO,
    FISH_GA,
    FISH_PIRARUKU,
    FISH_KURIONE,
    FISH_KURAGE,
    FISH_TATSU,
    FISH_KUMANOMI,
    FISH_NANYOU,
    FISH_CHOUCHOU,
    FISH_NAPOLEON,
    FISH_MINOKASAGO,
    FISH_HARISEN,
    FISH_AJI,
    FISH_ISHIDAI,
    FISH_SUZUKI,
    FISH_TAI,
    FISH_KAREI,
    FISH_HIRAME,
    FISH_IKA,
    FISH_TAKO,
    FISH_LOBSTER,
    FISH_UTSUBO,
    FISH_ANKOU,
    FISH_MAGURO,
    FISH_KAJIKI,
    FISH_EI,
    FISH_MANBOU,
    FISH_SHUMOKU,
    FISH_SAME,
    FISH_SHIIRA,
    FISH_KIND_NUM
};

class dAquariumCorrectObj_c;
class dAquarium_c;

class dFishMusBank_c : public dDvd::brresBank_c {
public:
    BOOL loadFish(EGG::Heap *heap, int fishIdx);
    BOOL loadObj(EGG::Heap *heap, int objKind);
}; // size 0x58

class dFishMusMdl_c : public m3d::smdl_c {
public:
    class mdlCallback_c : public nw4r::g3d::ICalcWorldCallback {
    public:
        mdlCallback_c(dFishMusMdl_c *owner);
        virtual ~mdlCallback_c();
        virtual void ExecCallbackB(nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl mdl,
                                   nw4r::g3d::FuncObjCalcWorld *funcObj);

        /* 0x4 */ dFishMusMdl_c *mOwner;
    }; // size 0x8

    enum { TRAIL_NUM = 32 };

    dFishMusMdl_c();
    virtual ~dFishMusMdl_c();

    BOOL create(nw4r::g3d::ResMdl mdl, mAllocator_c *allocator, int nodeNum, int startNode, int,
                ulong bufferOption, int viewCount, size_t *objSize);
    void shiftTrail(const mVec3_c &move);
    void calcNode(nw4r::g3d::WorldMtxManip *manip, ulong nodeId);

    /* 0x00C */ mdlCallback_c mCallback;
    /* 0x014 */ nw4r::math::MTX34 *mNodeMtx; // [mNodeNum]
    /* 0x018 */ EGG::Quatf *mNodeRot;        // [mNodeNum]
    /* 0x01C */ mVec3_c mTrail[TRAIL_NUM];
    /* 0x19C */ mVec3_c mRootPos;
    /* 0x1A8 */ mVec3_c mRootDir;
    /* 0x1B4 */ int mTrailHead;
    /* 0x1B8 */ int mTrailTail;
    /* 0x1BC */ int mTrailIdx;
    /* 0x1C0 */ int mTrailCount;
    /* 0x1C4 */ f32 mTrailDist[TRAIL_NUM];
    /* 0x244 */ f32 mDist;
    /* 0x248 */ f32 mTrailStep;
    /* 0x24C */ f32 mBendRate;
    /* 0x250 */ int mNodeNum;
    /* 0x254 */ int mStartNode;
    /* 0x258 */ u8 mMoved;
}; // size 0x25C

class dFishMusBaseObj_c {
public:
    static void *operator new(size_t size, EGG::Heap *heap);

    dFishMusBaseObj_c();
    virtual ~dFishMusBaseObj_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual BOOL draw();
    virtual BOOL doDelete();

    BOOL isOutOfView();

    /* 0x04 */ mAllocator_c mAllocator;
    /* 0x20 */ m3d::anmChr_c mAnm;
    /* 0x58 */ f32 mAnmRate;
    /* 0x5C */ f32 mAnmFrameMax; // random start frame below this
    /* 0x60 */ mVec3_c mPos;
    /* 0x6C */ mAng3_c mHomeAngle;
    /* 0x72 */ mAng3_c mAngle;
    /* 0x78 */ f32 mCullRadius;
    /* 0x7C */ m3d::smdl_c mMdl;
    /* 0x88 */ int mSlot;                // dFishMus_c heap slot
    /* 0x8C */ dFishMusBaseObj_c *mNext; // dFishMus_c's decoration list
}; // size 0x90

class dFishMusBase_c {
public:
    static void *operator new(size_t size, EGG::Heap *heap);

    dFishMusBase_c();
    virtual ~dFishMusBase_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual BOOL draw();
    virtual BOOL doDelete();
    virtual void addPos(const mVec3_c &push);
    virtual void onHitFish(dFishMusBase_c *other, const mVec3_c &push);
    virtual void onHitObj(dAquariumCorrectObj_c *obj, const mVec3_c &push);
    virtual int getHitType();

    void calcMtx();
    void getCenterPos(mVec3_c &center) const {
        center = mPos;
        mVec3_c ofs = mCenterOffset;
        ofs.rotX(mAngle.x);
        ofs.rotY(mAngle.y);
        center += ofs;
    }
    dAquarium_c *getAquarium();
    void setTankArea();
    BOOL isNearWall();
    BOOL isOutOfView();

    /* 0x004 */ dFishMusMdl_c mMdl;
    /* 0x260 */ m3d::anmChr_c mAnm;
    /* 0x298 */ mAllocator_c mAllocator;
    /* 0x2B4 */ f32 mAnmRate;
    /* 0x2B8 */ dFishMusMdl_c *mpMdl; // &mMdl once created
    /* 0x2BC */ mVec3_c mHomePos;
    /* 0x2C8 */ mVec3_c mPos;
    /* 0x2D4 */ mVec3_c mCenterOffset; // hit / view centre, in the fish's frame
    /* 0x2E0 */ mVec3_c mMdlOffset;    // in the fish's frame
    /* 0x2EC */ mVec3_c mVelocity;
    /* 0x2F8 */ f32 mSpeed;
    /* 0x2FC */ mAng3_c mHomeAngle;
    /* 0x302 */ mAng3_c mAngle;
    /* 0x308 */ int mTimer;
    /* 0x30C */ int mHitFrames; // frames since the last hit of another fish (dFishMus_c counts)
    /* 0x310 */ dFishMusData_c *mpData;
    /* 0x314 */ mVec3_c mTankMin;
    /* 0x320 */ mVec3_c mTankMax;
    /* 0x32C */ mVec3_c mHeadPos;
    /* 0x338 */ int mHitFishIdx;  // mFishIdx of the last fish hit
    /* 0x33C */ int mSlot;        // dFishMus_c heap slot
    /* 0x340 */ int mFishIdx;     // dFishMuseum::sFishData
    /* 0x344 */ int mAquariumIdx; // dFishMuseum::sAquariums
    /* 0x348 */ mVec3_c mPush;    // moved by addPos / onHitFish / onHitObj this frame (dFishMus_c clears it)
    /* 0x354 */ dFishMusBase_c *mNext;     // dFishMus_c's fish list
    /* 0x358 */ dFishMusBase_c *mHitNext;  // dAquarium_c::mHitList
    /* 0x35C */ dFishMusBase_c *mTankNext; // dAquarium_c::mFishList
}; // size 0x360

class dAquariumCorrectObj_c {
public:
    dAquariumCorrectObj_c() : mNext(NULL) {}
    virtual ~dAquariumCorrectObj_c() {}

    virtual void init();
    virtual BOOL hitCheck(dFishMusBase_c *fish, mVec3_c *push) = 0;
    virtual mVec3_c getPos() = 0;
    virtual f32 getHeight() const = 0;

    /* 0x04 */ dAquariumCorrectObj_c *mNext; // dAquarium_c::mObjList
}; // size 0x8

class dAquariumCorrectObjCyl_c : public dAquariumCorrectObj_c {
public:
    dAquariumCorrectObjCyl_c(const mVec3_c &pos, f32 radius, f32 height);
    dAquariumCorrectObjCyl_c(const char *nodeName, f32 radius, f32 height);
    dAquariumCorrectObjCyl_c(const char *nodeName, const mVec3_c &offset, f32 radius, f32 height);
    virtual ~dAquariumCorrectObjCyl_c() {}

    virtual void init();
    virtual BOOL hitCheck(dFishMusBase_c *fish, mVec3_c *push);
    virtual mVec3_c getPos() { return mPos; }
    virtual f32 getHeight() const { return mHeight; }

    /* 0x08 */ mVec3_c mPos; // bottom centre
    /* 0x14 */ mVec3_c mOffset;
    /* 0x20 */ f32 mRadius;
    /* 0x24 */ f32 mHeight;
    /* 0x28 */ const char *mNodeName;
    /* 0x2C */ bool mUseNode; // mPos = the node's position + mOffset
}; // size 0x30

class dAquariumEffBubble_c {
public:
    dAquariumEffBubble_c(const char *effName, const mVec3_c &offset, int aquariumIdx);
    virtual ~dAquariumEffBubble_c();

    void init();
    void execute();

    /* 0x04 */ const char *mEffName;
    /* 0x08 */ mVec3_c mPos;
    /* 0x14 */ mVec3_c mOffset; // from the tank's centre
    /* 0x20 */ int mAquariumIdx;
    /* 0x24 */ int mTimer;
}; // size 0x28

class dAquarium_c {
public:
    enum Type_e {
        TYPE_BOX,
        TYPE_CYLINDER,
    };
    // dFishMuseum::sAquariums
    enum Idx_e {
        IDX_RIVER,
        IDX_POND,
        IDX_SEA,
        IDX_CYL_LEFT,
        IDX_CYL_RIGHT,
        IDX_NUM,
    };

    dAquarium_c(const char *nodeName) : mHitList(NULL), mFishList(NULL), mObjList(NULL), mNodeName(nodeName) {}
    ~dAquarium_c() {}

    virtual void init();
    virtual int getType() const = 0;
    virtual BOOL correctFish(dFishMusBase_c *fish) = 0; // keeps the fish inside: TRUE if moved

    BOOL hitFish(dFishMusBase_c *a, dFishMusBase_c *b);
    void hitObjs(dFishMusBase_c *fish);
    void addFish(dFishMusBase_c *fish);
    void addHitFish(dFishMusBase_c *fish);
    void addObj(dAquariumCorrectObj_c *obj);
    void removeFish(dFishMusBase_c *fish);
    void removeHitFish(dFishMusBase_c *fish);
    void execute();

    /* 0x04 */ dFishMusBase_c *mHitList;
    /* 0x08 */ dFishMusBase_c *mFishList;
    /* 0x0C */ dAquariumCorrectObj_c *mObjList;
    /* 0x10 */ const char *mNodeName; // locator node of the tank in the room model
}; // size 0x14

class dAquariumBox_c : public dAquarium_c {
public:
    dAquariumBox_c(const char *nodeName, const mVec3_c &halfSize);
    dAquariumBox_c(const char *nodeName, const mVec3_c &halfSize, const mVec3_c &offset);

    virtual void init();
    virtual int getType() const { return TYPE_BOX; }
    virtual BOOL correctFish(dFishMusBase_c *fish);

    mVec3_c getMin() const { return mMin; }
    mVec3_c getMax() const { return mMax; }
    mVec3_c getCenter() const { return mCenter; }

    /* 0x14 */ mVec3_c mMin;
    /* 0x20 */ mVec3_c mMax;
    /* 0x2C */ mVec3_c mCenter;
    /* 0x38 */ mVec3_c mHalfSize;
    /* 0x44 */ mVec3_c mOffset;
}; // size 0x50

class dAquariumCyl_c : public dAquarium_c {
public:
    dAquariumCyl_c(const char *nodeName, f32 radius, f32 height)
        : dAquarium_c(nodeName), mRadius(radius), mHeight(height) {}

    virtual void init();
    virtual int getType() const { return TYPE_CYLINDER; }
    virtual BOOL correctFish(dFishMusBase_c *fish);

    mVec3_c getCenter() const { return mCenter; }

    /* 0x14 */ mVec3_c mCenter; // bottom centre
    /* 0x20 */ f32 mRadius;
    /* 0x24 */ f32 mHeight;
}; // size 0x28

class dFishMus_c : public fBase_c {
public:
    enum {
        FISH_NUM = 64,
        OBJ_KIND_NUM = 7,
        OBJ_NUM = 37,
        FISH_SLOT_NUM = 128,
    };
    enum ObjKind_e {
        OBJ_LOTUS,
        OBJ_WAKAME00,
        OBJ_WAKAME01,
        OBJ_WATER_WEED00,
        OBJ_WATER_WEED01,
        OBJ_SEA_ANEMONE,
        OBJ_OCTOPUS_POD,
    };

    dFishMus_c();
    virtual ~dFishMus_c();
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    void addFish(dFishMusBase_c *fish);
    void removeFish(dFishMusBase_c *fish);
    void addObj(dFishMusBaseObj_c *obj);
    void removeObj(dFishMusBaseObj_c *obj);
    BOOL createFish(int fishIdx, int aquariumIdx);
    BOOL createAllFish();
    BOOL deleteFish(dFishMusBase_c *fish);
    void deleteAllFish();
    mVec3_c getLocatorPos(const char *name, int idx);
    BOOL createLotus(int kind, int *objIdx);
    BOOL createWakame00(int kind, int *objIdx);
    BOOL createWakame01(int kind, int *objIdx);
    BOOL createWaterWeed00(int kind, int *objIdx);
    BOOL createWaterWeed01(int kind, int *objIdx);
    BOOL createSeaAnemone(int kind, int *objIdx);
    BOOL createOctopusPod(int kind, int *objIdx);
    BOOL createAllObj();
    BOOL deleteObj(dFishMusBaseObj_c *obj);
    void deleteAllObj();

    static dFishMus_c *sInstance;

    /* 0x0064 */ dFishMusBank_c mFishBanks[FISH_NUM];
    /* 0x1664 */ dFishMusBank_c mObjBanks[OBJ_NUM]; // only OBJ_KIND_NUM used
    /* 0x231C */ EGG::FrmHeap *mObjChildHeap[OBJ_NUM];
    /* 0x23B0 */ EGG::FrmHeap *mResHeap;
    /* 0x23B4 */ EGG::FrmHeap *mChildHeap[FISH_SLOT_NUM];
    /* 0x25B4 */ bool mFishSlotUsed[FISH_SLOT_NUM];
    /* 0x2634 */ bool mFirstSpawn[FISH_NUM]; // the tank floor is not added to sFishData's heights yet
    /* 0x2674 */ bool mObjSlotUsed[OBJ_NUM];
    /* 0x269C */ dFishMusBase_c *mFishList;
    /* 0x26A0 */ dFishMusBaseObj_c *mObjList;
    /* 0x26A4 */ bool _26A4;
    /* 0x26A5 */ bool _26A5;
}; // size 0x26A8

namespace dFishMuseum {

extern dFishMusData_c sFishData[FISH_KIND_NUM];
extern char sDebugFishName[]; // unreferenced

s16 degToAng(f32 deg);

} // namespace dFishMuseum
