#pragma once

// The insect rooms of the museum (REL d_insect_museumNP): the manager, the room areas and locators, the insect base.

#include <types.h>
#include <nw4r/g3d.h>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_color.hpp>
#include <game/framework/f_base.hpp>
#include <game/framework/f_profile.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/game/d_insect_util.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/core/eggHeap.h>

class dInsMusBase_c;
class InsectRoomLocatorList_c;

struct dInsMusData_c {
    typedef dInsMusBase_c *(*CreateFunc)(EGG::Heap *heap);

    /* 0x00 */ CreateFunc mCreate;
    /* 0x04 */ const char *mModelName;
    /* 0x08 */ int mScareLimit;
    /* 0x0C */ int mEscapeLimit;
    /* 0x10 */ f32 mScareRadius;
    /* 0x14 */ f32 mShadowSize;
}; // size 0x18

class dInsMusBrresLoader_c : public dDvd::brresBank_c {
public:
    dInsMusBrresLoader_c();
    virtual ~dInsMusBrresLoader_c();
    BOOL loadInsect(EGG::Heap *heap, int type);
}; // size 0x58

class dInsMusBase_c {
public:
    enum Room_e {
        ROOM_FLOWER,
        ROOM_POND,
        ROOM_WASTELAND,
        ROOM_TROPICAL,
        ROOM_F1,
        ROOM_F2,
        ROOM_F3,
    };

    static void *operator new(size_t size, EGG::Heap *heap);

    dInsMusBase_c();
    virtual ~dInsMusBase_c();
    virtual void calcMtx();
    virtual BOOL create(nw4r::g3d::ResFile &res);
    virtual BOOL execute();
    virtual BOOL draw();
    virtual BOOL doDelete();
    virtual void createModel(nw4r::g3d::ResFile &res);
    virtual void init() {}
    virtual void initState() {
        setAnmChr(mAnmChrWait, 0.0f);
        setAnmVis(mAnmVisWait);
    }
    virtual void executeState() {}

    void setAlpha(u8 alpha);
    void setAnmChr(const nw4r::g3d::ResAnmChr &anm, f32 blend);
    void setAnmVis(const nw4r::g3d::ResAnmVis &anm);
    void setAnmRate(f32 rate);
    void setAnmFrame(f32 frame);
    bool isUnderGround();
    f32 getGroundY();
    const dInsMusData_c *getData();
    bool isActiveTime() const;
    const char *getLocatorName();
    void setScale(f32 scale);
    void setAngleMtx();
    void setAxisMtx();
    void flyToTarget(f32 liftY, f32 speed, f32 brake, f32 turnRate);
    InsectRoomLocatorList_c *getRoomLocators();
    void getLocatorPos(mVec3_c *pos);

    mMtx_c getAxisMtx() const {
        mMtx_c mtx;
        dInsectUtil::getMtx(mtx, mAxis);
        return mtx;
    }

    void changeAnm(const nw4r::g3d::ResAnmChr &chr, const nw4r::g3d::ResAnmVis &vis) {
        f32 frame = mAnmChr.getFrame();
        setAnmChr(chr, 0.0f);
        setAnmVis(vis);
        setAnmRate(1.0f);
        setAnmFrame(frame);
    }

    /* 0x004 */ m3d::mdl_c mMdl;
    /* 0x044 */ m3d::anmChr_c mAnmChr;
    /* 0x07C */ m3d::anmVis_c mAnmVis;
    /* 0x0B4 */ mAllocator_c mAllocator;
    /* 0x0D0 */ dSndObjInsect_c mSnd;
    /* 0x100 */ mVec3_c mHomePos;
    /* 0x10C */ mVec3_c mPos;
    /* 0x118 */ mAng3_c mHomeAngle;
    /* 0x11E */ mAng3_c mAngle;
    /* 0x124 */ u32 mTimer;
    /* 0x128 */ u8 mAlpha;
    /* 0x12C */ mVec3_c mSpeed;
    /* 0x138 */ mVec3_c mTargetPos;
    /* 0x144 */ mVec3_c mPrevTargetPos;
    /* 0x150 */ dInsectUtil::axis_c mAxis;
    /* 0x174 */ dInsMusBase_c *mNext;
    /* 0x178 */ nw4r::g3d::ResAnmChr mAnmChrWait;
    /* 0x17C */ nw4r::g3d::ResAnmChr mAnmChrFly;
    /* 0x180 */ nw4r::g3d::ResAnmChr mAnmChrStop;
    /* 0x184 */ nw4r::g3d::ResAnmVis mAnmVisWait;
    /* 0x188 */ nw4r::g3d::ResAnmVis mAnmVisFly;
    /* 0x18C */ nw4r::g3d::ResAnmVis mAnmVisStop;
    /* 0x190 */ f32 mShadowSize;
    /* 0x194 */ f32 mScareRadius;
    /* 0x198 */ f32 mBgRadius;
    /* 0x19C */ dInsectUtil::scare_c mScare;
    /* 0x1A8 */ dInsectUtil::playerCheck_c mPlayerCheck;
    /* 0x1B4 */ dBGCF::acch_c mAcch;
    /* 0x1FC */ bool mCreated;
    /* 0x200 */ u32 mBgFlags;
    /* 0x204 */ int mSlot;
    /* 0x208 */ int mType;
    /* 0x20C */ bool mCheckBg;
}; // size 0x210

class InsectRoomArea_c {
public:
    InsectRoomArea_c();
    ~InsectRoomArea_c();
    void set(const mVec3_c &center, f32 radiusX, f32 radiusZ);
    void clampTarget(mVec3_c *out, const mVec3_c *from, const mVec3_c *target) const;
    bool isInside(const mVec3_c *pos) const;
    void calcMtx();
    f32 getRadiusX() const { return mRadiusX; }
    f32 getRadiusZ() const { return mRadiusZ; }

    /* 0x00 */ mColor mDrawColor; // unused; needed for ~mColor's place among the weak functions
    /* 0x04 */ mVec3_c mCenter;
    /* 0x10 */ f32 mRadiusX;
    /* 0x14 */ f32 mRadiusZ;
    /* 0x18 */ mMtx_c mMtx;
    /* 0x48 */ mMtx_c mInvMtx;
}; // size 0x78

class InsectRoomAreaManager_c {
public:
    enum { AREA_NUM = 25 };

    InsectRoomAreaManager_c();
    ~InsectRoomAreaManager_c();
    void init();
    void execute();

    /* 0x000 */ InsectRoomArea_c mAreas[AREA_NUM];
}; // size 0xBB8

struct InsectRoomLocator_c {
    enum { NAME_LEN = 0x20 };

    void getPos(mVec3_c *pos) const;

    /* 0x00 */ InsectRoomLocator_c *mNext;
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ char mName[NAME_LEN];
}; // size 0x30

class InsectRoomLocatorList_c {
public:
    InsectRoomLocatorList_c();
    ~InsectRoomLocatorList_c();
    void set(InsectRoomLocator_c *list) {
        mHead = list;
        mCur = list;
    }
    void getPos(mVec3_c *pos, const char *name) const;

    /* 0x0 */ InsectRoomLocator_c *mHead;
    /* 0x4 */ InsectRoomLocator_c *mCur;
}; // size 0x8

class InsectRoomLocatorManager_c {
public:
    InsectRoomLocatorManager_c();
    ~InsectRoomLocatorManager_c();
    BOOL create();
    void addLocators(const char *name, InsectRoomLocator_c **list, bool numbered);
    void createFlower();
    void createPond();
    void createWasteland();
    void createTropical();
    void createF1();
    void createF2();
    void createF3();
    void remove();
    InsectRoomLocatorList_c *setFlower();
    InsectRoomLocatorList_c *setPond();
    InsectRoomLocatorList_c *setWasteland();
    InsectRoomLocatorList_c *setTropical();
    InsectRoomLocatorList_c *setF1();
    InsectRoomLocatorList_c *setF2();
    InsectRoomLocatorList_c *setF3();
    void setHeap(EGG::Heap *heap);
    void freeList(InsectRoomLocator_c **list);
    InsectRoomLocator_c *getLast(InsectRoomLocator_c *node);
    BOOL isBgReady();
    void getNodePos(mVec3_c *pos, const char *name);

    /* 0x00 */ InsectRoomLocatorList_c mList;
    /* 0x08 */ mAllocator_c mAllocator;
    /* 0x24 */ InsectRoomLocator_c *mFlower;
    /* 0x28 */ InsectRoomLocator_c *mPond;
    /* 0x2C */ InsectRoomLocator_c *mWasteland;
    /* 0x30 */ InsectRoomLocator_c *mTropical;
    /* 0x34 */ InsectRoomLocator_c *mF1;
    /* 0x38 */ InsectRoomLocator_c *mF2;
    /* 0x3C */ InsectRoomLocator_c *mF3;
    /* 0x40 */ EGG::FrmHeap *mFrmHeap;
}; // size 0x44

class dInsectMus_c : public fBase_c {
public:
    enum {
        SLOT_NUM = 64,
        BANK_BALL = INSECT_NUM,
        BANK_NUM,
    };

    dInsectMus_c();
    virtual ~dInsectMus_c();
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    BOOL createInsect(int type);
    BOOL deleteInsect(dInsMusBase_c *insect);
    void deleteAllInsects();
    void addInsect(dInsMusBase_c *insect);
    void removeInsect(dInsMusBase_c *insect);
    u32 getTimeMask();
    void createAllInsects();
    u32 isBattleReady();

    static dInsectMus_c *sInstance;

    /* 0x0064 */ bool mSlotUsed[SLOT_NUM];
    /* 0x00A4 */ EGG::FrmHeap *mChildHeap[SLOT_NUM];
    /* 0x01A4 */ dInsMusBrresLoader_c mBanks[BANK_NUM];
    /* 0x17FC */ EGG::FrmHeap *mResHeap;
    /* 0x1800 */ dInsMusBase_c *mInsectList;
    /* 0x1804 */ InsectRoomAreaManager_c mAreaMgr;
    /* 0x23BC */ u32 mFrame;
}; // size 0x23C0

namespace dInsectMuseum {

struct insectPlace_s {
    /* 0x0 */ const char *mLocator;
    /* 0x4 */ dInsMusBase_c::Room_e mRoom;
}; // size 0x8

extern const insectPlace_s sInsectPlace[INSECT_NUM];
extern InsectRoomLocatorManager_c sLocatorMgr;
extern u32 sTimeMask;
extern dInsMusData_c sInsectDataTable[INSECT_NUM];
extern const dInsMusData_c *sInsectData[INSECT_NUM];

void *dInsectMus_c_classInit();

} // namespace dInsectMuseum
