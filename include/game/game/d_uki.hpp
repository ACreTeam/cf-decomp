#pragma once

// The fishing float ("uki") and the slingshot shot.

#include <types.h>
#include <game/game/d_objc.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/game/d_dvd.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>

class dFishFldShadow_c;
namespace dBGCF {
class groundChk_c;
}
namespace dItem {
class Item;
}
namespace nw4r {
namespace ef {
class Particle;
}
} // namespace nw4r
namespace EGG {
class FrmHeap;
}

#define UKI_SLOT_NUM 9
#define UKI_SHOT_NUM 3

class dUki_c {
public:
    enum Kind_e {
        KIND_ROD,
        KIND_SILVER_ROD,
        KIND_GOLD_ROD,
        KIND_PACHINKO,
        KIND_NONE,
    };

    enum State_e {
        STATE_NONE,
        STATE_HOLD,
        STATE_RETURN,
        STATE_SWING,
        STATE_CAST,
        STATE_FLOAT,
        STATE_DROP,
        STATE_BITE,
        STATE_HOOKED,
        STATE_REEL,
        STATE_REEL_EMPTY,
        STATE_SHOT_HOLD,
        STATE_SHOT,
    };

    dUki_c();  // 8016DE60
    ~dUki_c(); // 8016DF98

    void init(int slot);
    void remove();
    void create(int kind, const dItem::Item *item);
    void putAway();
    void calc(const mMtx_c *mtx);
    void draw();
    void setMdlMtx(const mMtx_c *mtx);
    void removeMdl();
    void reset();

    void initHold();
    void executeHold(mMtx_c *mtx);
    void initReturn();
    void executeReturn(mMtx_c *mtx);
    void initSwing();
    void executeSwing(mMtx_c *mtx);
    void initCast();
    void executeCast(mMtx_c *mtx);
    void initFloat();
    void executeFloat(mMtx_c *mtx);
    void exitFloat();
    void initDrop();
    void executeDrop(mMtx_c *mtx);
    void initBite();
    void executeBite(mMtx_c *mtx);
    void exitBite(u32 next);
    void initHooked();
    void executeHooked(mMtx_c *mtx);
    void initReel();
    void executeReel(mMtx_c *mtx);
    void initReelEmpty();
    void executeReelEmpty(mMtx_c *mtx);
    void initShotHold();
    void executeShotHold();
    void initShot();
    void executeShot(mMtx_c *mtx);

    void setState(u32 state);
    void nibble();
    bool isInWater() const;
    BOOL isJustLanded() const;
    void shoot(const mVec3_c *pos);
    void setFishPos(const mVec3_c *pos);
    dFishFldShadow_c *getFish() const;
    void setTarget(const mVec3_c *pos);
    void setPos(const mVec3_c *pos);
    void setRodMtx(const mMtx_c *mtx);
    void setPos(f32 x, f32 y, f32 z);

    BOOL calcCast();
    BOOL calcReel();
    BOOL calcReelEmpty();
    BOOL calcReturn();
    BOOL calcSwing();
    void initTipPos();
    void calcTipMtx(mMtx_c *mtx);
    void updateTip();
    BOOL setMatAlpha(bool markDirty, const dBGCF::groundChk_c *chk);
    BOOL calcShot();
    static void particleCallback(nw4r::ef::Particle *particle, u32 arg);
    void createWaterEffect(const char *name, const mVec3_c *pos);
    void createSplashEffect(const char *name, const mVec3_c *pos);
    void createSplash(const mVec3_c *pos);
    u8 getFallAlpha(const mVec3_c *pos);

    static dUki_c *getFloat(int player, dFishFldShadow_c *fish);
    static void setFish(int player, dFishFldShadow_c *fish);
    static u32 getBankHeapSize();
    static BOOL loadRes();
    static int getHeapSize();
    static void *getResData();

    /* 0x000 */ dSndObjSimple_c mSound;
    /* 0x030 */ mAllocator_c mAllocator;
    /* 0x04C */ m3d::mdl_c mRodMdl[2];
    /* 0x0CC */ m3d::mdl_c mShotMdl[UKI_SHOT_NUM];
    /* 0x18C */ dObjc::pos_c mObjc[UKI_SHOT_NUM];
    /* 0x270 */ mVec3_c mPos[UKI_SHOT_NUM];
    /* 0x294 */ mVec3_c mReelPos;
    /* 0x2A0 */ mVec3_c mTarget;
    /* 0x2AC */ mVec3_c mTip;
    /* 0x2B8 */ mVec3_c mHookPos;
    /* 0x2C4 */ mVec3_c mFishPos;
    /* 0x2D0 */ mMtx_c mRodMtx;
    /* 0x300 */ dLevelEffect_c mEffect0;
    /* 0x394 */ dLevelEffect_c mEffect1;
    /* 0x428 */ EGG::FrmHeap *mHeap;
    /* 0x42C */ int mSlot;
    /* 0x430 */ f32 mCastRate;
    /* 0x434 */ f32 mPullRadius;
    /* 0x438 */ u32 mTimer;
    /* 0x43C */ s16 mAngY;
    /* 0x43E */ s16 mAngX;
    /* 0x440 */ s16 mAngXTarget;
    /* 0x442 */ s16 _442;
    /* 0x444 */ s16 mPlayerYaw;
    /* 0x446 */ u8 mKind;
    /* 0x447 */ u8 mState;
    /* 0x448 */ u8 mNum;
    /* 0x449 */ u8 mMdlIdx;
    /* 0x44A */ u8 mAlpha;
    /* 0x44B */ u8 mFollowTip;
    /* 0x44C */ u8 mRipple;
    /* 0x44D */ u8 mFade;
}; // size 0x450 (dHmnToolBank_c: 0x1C..0x46C)

class dUkiBank_c {
public:
    dUkiBank_c();  // 80170E70
    ~dUkiBank_c(); // 80170EC8

    BOOL load();
    u32 getHeapSize();
    void setUki(int slot, dUki_c *uki);
    dUki_c *getUki(int player, dFishFldShadow_c *fish);

    /* 0x00 */ dDvd::loader_c mLoader;
    /* 0x14 */ dUki_c *mUki[UKI_SLOT_NUM];
    /* 0x38 */ dFishFldShadow_c *mFish[UKI_SLOT_NUM];
    /* 0x5C */ void *mResData;
}; // size 0x60
