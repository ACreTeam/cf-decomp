#include <game/game/d_fgobj_manager.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_fg_draw.hpp>
#include <game/game/d_fg_mng_task.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_bgcf.hpp>
#include <lib/nw4r/ef/ef_particlemanager.h>
#include <game/game/d_a_snowman.hpp>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_ground_attr.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_activity_log.hpp>
#include <game/game/d_field_info.hpp>
#include <lib/egg/math/eggMath.h>
#include <game/game/d_fg_obj.hpp>
#include <string.h>
#include <game/game/d_bgm.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_insect_field.hpp>
#include <game/game/d_a_player.hpp>

namespace dFgobjManager {

// The manager while it exists (sFgObjMgr).
inline dFgObjMng_c *getMgr() {
    return (dFgObjMng_c *)fgMngProc_getFgObjMgr();
}

// .text:0xCC
void *create() {
    return new dFgObjMng_c;
}

fProf::fBaseProfile_c g_profile_FGOBJ_MANAGER = {
    &create, fProf::FGOBJ_MANAGER, fProf::DRAW_ORDER::FGOBJ_MANAGER};

} // namespace dFgobjManager

// The tree effect tables, by effect kind (dFgObjAnm_c::mEffKind: tree stage -
// 1, +4 palm-like, +8 cedar-like) or by effect index (effect_c::mIdx: the kind
// + 12 * the tree animation group).
static const f32 sTreeEffectScale[12] = {0.4f, 0.5f, 0.7f, 1.0f, 0.4f, 0.5f,
                                         0.7f, 1.0f, 0.4f, 0.5f, 0.7f, 1.0f};
static const f32 sTreeEffectHeight[12] = {6.0f, 15.0f, 23.0f, 27.0f,
                                          7.0f, 16.0f, 22.0f, 30.0f,
                                          5.0f, 13.0f, 23.0f, 30.0f};
static const f32 sTreeEffectRate[12] = {1.0f, 0.6f, 0.4f, 0.0f, 1.0f, 0.6f,
                                        0.4f, 0.0f, 1.0f, 0.6f, 0.4f, 0.0f};
static const u8 sTreeEffectCount[0x48] = {
    0,  80, 80, 80, 0,  80, 80, 80, 0,  80, 80, 80, 0, 1, 1,  1,  0, 1,
    1,  1,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0,  0, 0,
    25, 45, 52, 60, 25, 45, 52, 60, 25, 45, 52, 60, 0, 0, 66, 70, 0, 0,
    66, 70, 0,  0,  66, 70, 1,  1,  1,  1,  1,  1,  1, 1, 1,  1,  1, 1,
};
static const f32 sTreeEffectParamA[0x48] = {
    0.0f,  0.08f, 0.12f, 0.2f,  0.0f,  0.08f, 0.12f, 0.2f,  0.0f,  0.08f, 0.12f,
    0.2f,  0.0f,  4.0f,  6.0f,  8.0f,  0.0f,  4.0f,  6.0f,  8.0f,  0.0f,  4.0f,
    6.0f,  8.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    0.0f,  0.0f,  0.0f,  0.08f, 0.08f, 0.12f, 0.2f,  0.08f, 0.08f, 0.12f, 0.2f,
    0.08f, 0.08f, 0.12f, 0.15f, 0.0f,  0.0f,  0.12f, 0.2f,  0.0f,  0.0f,  0.12f,
    0.2f,  0.0f,  0.0f,  0.12f, 0.15f, 3.0f,  4.0f,  6.0f,  8.0f,  3.0f,  4.0f,
    6.0f,  8.0f,  3.0f,  4.0f,  6.0f,  8.0f,
};
static const f32 sTreeEffectParamB[0x48] = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.2f, 0.3f, 0.5f, 0.5f, 0.2f, 0.3f, 0.5f, 0.5f, 0.2f, 0.3f, 0.5f, 0.5f,
};
static const f32 sTreeShakeFrame[12] = {25.0f, 45.0f, 52.0f, 60.0f,
                                        25.0f, 45.0f, 52.0f, 60.0f,
                                        25.0f, 45.0f, 52.0f, 60.0f};
static const f32 sTreeChopFrame[12] = {0.0f, 0.0f, 66.0f, 70.0f,
                                       0.0f, 0.0f, 66.0f, 70.0f,
                                       0.0f, 0.0f, 66.0f, 70.0f};
static const f32 sTreeSnowRate[12] = {1.2f, 1.0f, 0.6f, 0.0f, 1.2f, 1.0f,
                                      0.6f, 0.0f, 1.2f, 1.0f, 0.6f, 0.0f};
static const u8 sTreeSnowCount[0x40] = {
    0, 80, 80, 80, 0,  80, 80, 80, 0,  80, 80, 80, 0,  1,  1,  1,
    0, 1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1, 1,  1,  1,  45, 45, 52, 60, 45, 45, 52, 60, 45, 45, 52, 60,
    0, 0,  66, 70, 0,  0,  66, 70, 0,  0,  66, 70, 0,  0,  0,  0,
};
static const f32 sTreeSnowParam[0x3C] = {
    0.0f,  0.03f, 0.06f, 0.1f, 0.0f,  0.03f, 0.06f, 0.1f,  0.0f,  0.03f,
    0.06f, 0.1f,  0.0f,  4.0f, 5.0f,  6.0f,  0.0f,  4.0f,  5.0f,  6.0f,
    0.0f,  4.0f,  5.0f,  6.0f, 2.0f,  2.0f,  3.0f,  4.0f,  2.0f,  2.0f,
    3.0f,  4.0f,  2.0f,  2.0f, 3.0f,  4.0f,  0.03f, 0.03f, 0.06f, 0.1f,
    0.03f, 0.03f, 0.06f, 0.1f, 0.03f, 0.03f, 0.06f, 0.1f,  0.0f,  0.0f,
    0.06f, 0.1f,  0.0f,  0.0f, 0.06f, 0.1f,  0.0f,  0.0f,  0.06f, 0.1f,
};

// 0x90 bytes of floats and ints that nothing in the REL reads (the user was
// stripped); kept for the .rodata layout.
#ifdef MUST_MATCH
extern const u32 lUnusedTreeParams[36];
const u32 lUnusedTreeParams[36] = {
    0x3E400000, 0x3F500000, 0x40400000, 0x40700000, 0x3FC00000, 0x3DCCCCCD,
    0x3FC00000, 0x3E19999A, 0x3FF00000, 0x0000004B, 0x0000001E, 0x40400000,
    0x3F000000, 0x00000009, 0x00000018, 0x00000001, 0x3B83126F, 0x00000032,
    0x0000000C, 0x00000008, 0x00000024, 0x3D4CCCCD, 0x0000002D, 0x00000028,
    0x0000003C, 0x0000000C, 0x41700000, 0x40A00000, 0x3F800000, 0x41800000,
    0x41000000, 0x40800000, 0x40800000, 0x04000000, 0x00000015, 0x00000038,
};
#endif

// ---------------------------------------------------------------------------------------------
// .text 0x1A4 .. 0xF18: helpers, constructors / destructors and the virtuals of
// the object classes.

// .text:0x1A4
static f32 roundUp(f32 x) {
    if ((int)x == x) {
        return x;
    }
    return (int)(x + 1.0f);
}

// The procs of the object types (dFgObj_c::mType; dFgObjAnm_c from type 15).
static void (dFgObj_c::*const sObjProcs[])() = {
    &dFgObj_c::procDrop,       &dFgObj_c::procGrow,
    &dFgObj_c::procGrow,       &dFgObj_c::procShrink,
    &dFgObj_c::procPickUp,     &dFgObj_c::procToss,
    &dFgObj_c::procSlide,      &dFgObj_c::procKnockOff,
    &dFgObj_c::procBuryMoney,  &dFgObj_c::procFillHole,
    &dFgObj_c::procSpin,       &dFgObj_c::procDrop,
    &dFgObj_c::procShrinkFast, &dFgObj_c::procPop,
    &dFgObj_c::procThrow,
};
static void (dFgObjAnm_c::*const sAnmProcs[])() = {
    &dFgObjAnm_c::procTree,
    &dFgObjAnm_c::procHoneycomb,
    &dFgObjAnm_c::procFellTree,
};
static void (dFgObjAnm_c::*const sAnmDrawProcs[])() = {
    &dFgObjAnm_c::drawTree,
    &dFgObjAnm_c::drawHoneycomb,
    &dFgObjAnm_c::drawFellTree,
};

// .text:0x214
dFgObj_c::dFgObj_c() {
    clear();
}

// .text:0x2D4
dFgObjAnm_c::dFgObjAnm_c() {
}

// .text:0x3F4
dFgObjSwingPlant_c::dFgObjSwingPlant_c() {
}

// .text:0x440
dFgDecoAnm_c::dFgDecoAnm_c() {
    clear();
    mTreeAnmKind = -1;
    mEndCount = 0;
}

// .text:0x538
dFgObj_c::~dFgObj_c() {
}

// .text:0x594
dFgObjAnm_c::~dFgObjAnm_c() {
}

// .text:0x6A0
dFgObjSwingPlant_c::~dFgObjSwingPlant_c() {
}

// .text:0x714
dFgDecoAnm_c::~dFgDecoAnm_c() {
}

// .text:0x7A4
void dFgObj_c::remove() {
    clear();
}

// .text:0x828
void dFgObjAnm_c::remove() {
    dFgObj_c::remove();
    mMdl.remove();
    mAnmChr.remove();
    mAnmClr.remove();
    mAnmVis.remove();
    EGG::FrmHeap *heap = dFgobjManager::getAnmHeap(mIdx);
    if (heap != NULL) {
        heap->free(3);
    }
    mHideSlot = -1;
    if (mDeco != NULL) {
        mDeco->endTreeAnm();
    }
    mDeco = NULL;
}

// .text:0x8DC
void dFgObjSwingPlant_c::remove() {
    dFgObj_c::remove();
    mMdl.remove();
    EGG::FrmHeap *heap = dFgobjManager::getSwingHeap(mIdx);
    if (heap != NULL) {
        heap->free(3);
    }
    mHiddenUnit = -1;
}

// .text:0x93C
void dFgDecoAnm_c::remove() {
    clear();
    mMdl.remove();
    mAnmChr.remove();
    mAnmClr.remove();
    mAnmSrt.remove();
    EGG::FrmHeap *heap = dFgobjManager::getDecoHeap(mIdx);
    if (heap != NULL) {
        heap->free(3);
    }
    mTreeAnmKind = -1;
    mEndCount = 0;
}

// .text:0xA00
void dFgObj_c::setup(int idx) {
    mIdx = idx;
}

// .text:0xA08
void dFgObjAnm_c::setup(int idx) {
    dFgObj_c::setup(idx);
    mAllocator.attach(dFgobjManager::getAnmHeap(idx), 0x20);
    mCallback.mpOwner = this;
    mTopPos = mVec3_c::Zero;
}

// .text:0xA7C
void dFgObjSwingPlant_c::setup(int idx) {
    dFgObj_c::setup(idx);
    mAllocator.attach(dFgobjManager::getSwingHeap(idx), 0x20);
}

// .text:0xACC
void dFgDecoAnm_c::setup(int idx) {
    mIdx = idx;
    mAllocator.attach(dFgobjManager::getDecoHeap(idx), 0x20);
}

// .text:0xB10
void dFgObj_c::execute() {
    if (mActive) {
        mSnd.setPos(&mSndPos);
        (this->*sObjProcs[mType])();
    }
}

// .text:0xB70
void dFgObjAnm_c::execute() {
    if (mActive) {
        mSnd.setPos(&mSndPos);
        (this->*sAnmProcs[mType - 15])();
        mTopPos = mVec3_c::Zero;
    }
}

// .text:0xBF4
void dFgObjSwingPlant_c::execute() {
    if (mActive) {
        mSnd.setPos(&mSndPos);
        calcSwing();
    }
}

// .text:0xC3C
void dFgDecoAnm_c::execute() {
    if (mEndCount >= 2) {
        remove();
    }
    if (mActive) {
        mVec3_c pos;
        mAng ang = dWorld::curvePosition(&pos, &mPos);
        mMtx_c m;
        PSMTXTrans(m, pos.x, pos.y, pos.z);
        m.XrotM(ang);
        mMdl.setLocalMtx(&m);
        if (mAnmClr.IsBound()) {
            mAnmClr.play();
        }
        if (mAnmSrt.IsBound()) {
            mAnmSrt.play();
        }
        mMdl.play();
    }
}

// .text:0xD30
void dFgObj_c::draw() {
    switch (mDrawMode) {
    case 1:
        dFgDraw::add(mItem, mDrawPos, mDrawScale, FALSE, FALSE,
                     mAng3_c(0, 0, 0));
        break;
    case 2:
        dFgDraw::add(mItem, mDrawPos, mDrawScale, TRUE, FALSE,
                     mAng3_c(0, 0, 0));
        break;
    case 3:
        dFgDraw::addAtUnit(mItem, mDrawPos, mUnitX, mUnitZ, mDrawScale);
        break;
    }
}

// .text:0xDF0
void dFgObjAnm_c::draw() {
    if (mActive) {
        (this->*sAnmDrawProcs[mType - 15])();
    }
}

// .text:0xE38
void dFgObjSwingPlant_c::draw() {
    if (mActive) {
        drawSwing();
    }
}

// .text:0xE4C
void dFgDecoAnm_c::draw() {
    if (mActive) {
        switch (mTreeAnmKind) {
        case -1:
            dCamera_c *camera = (dCamera_c *)dScene::getSceneE848();
            if (camera == NULL || !camera->isPastForeground(&mPos, 32.0f)) {
                mMdl.entry();
            }
            break;
        }
    }
}

// .text:0xEC8
void dFgObj_c::finishTask() {
    if (mProcTask) {
        mProcTask = 0;
        fgMngTask_procAt(mUnitX, mUnitZ, mLayer);
    } else if (mReleaseTask) {
        mReleaseTask = 0;
        fgMngTask_releaseAt(mUnitX, mUnitZ, mLayer);
    }
}

// ---------------------------------------------------------------------------------------------
// .text 0xF18 .. 0x5798: the per-type procs of the object classes.

// .text:0xF18
void dFgObj_c::init(int player, int x, int z, mVec3_c pos, int type, u16 item,
                    dFdBase_c *fd, s16 angle, int arg, u8 layer) {
    mActive = TRUE;
    mPlayer = player;
    mType = type;
    mUnitX = x;
    mUnitZ = z;
    mAngleY = angle;
    _56 = 0;
    mPos = pos;
    mLayer = layer;
    mProcTask = 1;
    mReleaseTask = 0;
    _28.x = 0.0f;
    _28.y = 0.0f;
    _28.z = 0.0f;
    mItem = mPrevItem = item;
    dFdAsPos_c unit(x, z);
    int task = fgMngTask_find(dScene::getCurrentScene(), &unit, layer);
    if (task >= 0) {
        mPrevItem = fgMngTask_get(task)->mNewItem;
    } else {
        mPrevItem = dItem::ITEM_ID_NONE;
    }
    mStep = 0;
    mFrame = 0;
    mTimer = 0;
    mSeCount = 0;
    mParam = 0.0f;
    switch (type) {
    case 0:
    case 11:
        initDrop(arg);
        break;
    case 1:
        initDigUp(fd, arg);
        break;
    case 2:
        initPull();
        break;
    case 3:
        initDigHole(fd);
        break;
    case 4:
    case 13:
        initPickUp();
        break;
    case 5:
        initToss(arg);
        break;
    case 6:
        initSlide();
        break;
    case 7:
        initKnockOff(fd);
        break;
    case 8:
        initBuryMoney();
        break;
    case 9:
        initFillHole(fd);
        break;
    case 10:
        initClear(fd);
        break;
    case 12:
        initDigHoleTask(fd);
        break;
    case 14:
        initThrow();
        break;
    case 15:
        static_cast<dFgObjAnm_c *>(this)->initTree(arg);
        break;
    case 16:
        static_cast<dFgObjAnm_c *>(this)->initHoneycomb();
        break;
    case 17:
        static_cast<dFgObjAnm_c *>(this)->initFellTree(fd);
        break;
    case 19:
        static_cast<dFgObjSwingPlant_c *>(this)->initSwing();
        break;
    }
}

// .text:0x1134
void dFgObj_c::kill() {
    fgMngProc_isSelfMember(mPlayer);
    finishTask();
    remove();
}

// .text:0x1180
void dFgObj_c::playLandSound() {
    dItem::Item item(mItem);
    int se;
    if (item.isMoney()) {
        if (item.getPrice() < 1000) {
            se = 0x17A0;
        } else {
            se = 0x173B;
        }
    } else {
        se = dGroundAttr::getAttr(&mSndPos);
        if (item.isSame(dItem::Item(0x212)) &&
            (se == 0x68 || se == 0x19 || se == 0x13)) {
            se = 0x1750;
            mSnd.startSound(se);
        } else {
            se = 0x174F;
        }
    }
    if (dWeather_c::isRainKind(dWeather_c::sInstance->mWeather)) {
        switch (se) {
        case 0x17A0:
            se = 0x182B;
            break;
        case 0x173B:
            se = 0x1795;
            break;
        }
    }
    mSnd.startSound(se);
}

// .text:0x12B8
void dFgObj_c::playDropSound() {
    dItem::Item item(mItem);
    if (item.isMoney()) {
        mSnd.startSound(0x1742);
    } else {
        mSnd.startSound(0x1743);
    }
}

// .text:0x1314
void dFgObj_c::setDraw(const mVec3_c *pos, const mVec3_c *scale, u8 mode) {
    mDrawPos = *pos;
    mDrawScale = *scale;
    mDrawMode = mode;
}

namespace dFgobjManager {

// .text:0x134C
void moveOffWater(mVec3_c *pos, BOOL onWater) {
    if (dField::getInfo(1)->canPutItem(pos) == 1) {
        return;
    }
    static const f32 offsets[8] = {0.0f, -15.0f, 15.0f,  0.0f,
                                   0.0f, 15.0f,  -15.0f, 0.0f};
    u8 water = 0;
    mVec3_c base(*pos);
    mVec3_c check(base);
    for (int i = 0; i < 4; i++) {
        check.x = base.x + offsets[i * 2];
        check.z = base.z + offsets[i * 2 + 1];
        dBGCF::groundChk_c chk(&check, 0, 0, 0);
        if (chk.mWater != 0) {
            water |= 1 << i;
        }
    }
    switch (water) {
    case 0:
        return;
    case 3:
        if (onWater) {
            pos->x += 16.0f;
            pos->z -= 16.0f;
        } else {
            pos->x -= 2.0f;
            pos->z += 2.0f;
        }
        break;
    case 6:
        if (onWater) {
            pos->x += 16.0f;
            pos->z += 16.0f;
        } else {
            pos->x -= 2.0f;
            pos->z -= 2.0f;
        }
        break;
    case 9:
        if (onWater) {
            pos->x -= 16.0f;
            pos->z -= 16.0f;
        } else {
            pos->x += 2.0f;
            pos->z += 2.0f;
        }
        break;
    case 12:
        if (onWater) {
            pos->x -= 16.0f;
            pos->z += 16.0f;
        } else {
            pos->x += 2.0f;
            pos->z -= 2.0f;
        }
        break;
    case 15:
        break;
    }
    if (onWater) {
        dBGCF::groundChk_c chk(pos, 0, 0, 0);
        pos->y = chk.getWaterY();
    } else {
        dBGCF::groundChk_c chk(pos, 0, 0, 0);
        pos->y = chk.getHeight(FALSE);
    }
}

} // namespace dFgobjManager

// .text:0x15F0
void dFgObj_c::initDrop(int effect) {
    mVec3_c target;
    dFgDraw::getItemPosition(&target, mItem, mUnitX, mUnitZ, mLayer);
    f32 dx = 0.8125f * (target.x - mPos.x);
    f32 dz = 0.8125f * (target.z - mPos.z);
    target.x = mPos.x + dx;
    target.z = mPos.z + dz;
    f32 vy;
    if (mLayer == 1) {
        mParam = target.y = dFTR::getGroundY(mUnitX, mUnitZ);
        vy = 3.75f;
    } else {
        vy = 3.0f;
        mParam = target.y;
        target.y = dBGCF::getUnitY(&target);
    }
    f32 g = 5.3333335f;
    f32 vv = vy * vy;
    f32 h = target.y - mPos.y;
    if (vv < 0.375f * h) {
        f32 t = roundUp(EGG::Math<f32>::sqrt(g * (2.0f * h)) - 1.0f);
        mSpeed.x = dx / t;
        mSpeed.y = 0.1875f * t;
        mSpeed.z = dz / t;
    } else {
        f32 t =
            roundUp(g * (vy + EGG::Math<f32>::sqrt(vv - 0.375f * h)) - 1.0f);
        mSpeed.y = vy;
        mSpeed.x = dx / t;
        mSpeed.z = dz / t;
    }
    mSndPos = mPos;
    mScale.x = 0.0f;
    mScale.y = 1.0f;
    mScale.z = 0.0f;
    mSnd.setPos(&mSndPos);
    if (mType == 11) {
        if (effect == 1) {
            dEffect::create("af_fg_rock_lucky", &mSndPos, NULL, NULL);
        }
        dItem::Item item(mItem);
        if (item.isMoney() && item.getPrice() >= 1000) {
            mSnd.startSound(0x17DC);
        }
    } else {
        playDropSound();
    }
}

// .text:0x1838
void dFgObj_c::initDigUp(dFdBase_c *fd, BOOL release) {
    mScale.x = 0.0f;
    mScale.y = 1.0f;
    mScale.z = 0.0f;
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    dFdBase_c::getUnitGroundPos(&mSndPos, mUnitX, mUnitZ);
    dItem::Item *item = fd->getItem(mUnitX, mUnitZ, 0);
    if (item != NULL) {
        BOOL buried = fd->isBuried(mUnitX, mUnitZ);
        if (!item->isAnyFlower() && !item->isWeed() && !item->isClover() &&
            !((item->getId() == dItem::FG_RED_TURNIP_WILTED) |
              (item->getId() == dItem::FG_RED_TURNIP_WILTED_WATERED)) &&
            (buried || item->isRedTurnip()) && release) {
            mProcTask = 0;
            mReleaseTask = 1;
        }
        if (buried) {
            dFgMngProc_c::setUnitItem(fd, mUnitX, mUnitZ, dItem::ITEM_ID_NONE,
                                      FALSE);
        }
        dBGCF::setUnitAnm(mUnitX, mUnitZ, 9,
                          dFgobjManager::getFgAnmParam(mItem), 0.0f);
    }
}

// .text:0x19D8
void dFgObj_c::initPull() {
    mScale.x = 0.0f;
    mScale.y = 1.0f;
    mScale.z = 0.0f;
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    dFdBase_c::getUnitGroundPos(&mSndPos, mUnitX, mUnitZ);
    dBGCF::setUnitAnm(mUnitX, mUnitZ, 9, dFgobjManager::getFgAnmParam(mItem),
                      0.0f);
    mProcTask = 0;
    mReleaseTask = 1;
}

// .text:0x1A6C
void dFgObj_c::initDigHole(dFdBase_c *fd) {
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    dFdBase_c::getUnitGroundPos(&mSndPos, mUnitX, mUnitZ);
    mItem = dItem::FG_HOLE;
    dBGCF::setUnitAnm(mUnitX, mUnitZ, 0x18, 0.0f,
                      dFgobjManager::getFgAnmParam(mItem));
    dItem::Item none;
    fd->setItem(&none, mUnitX, mUnitZ, 0);
}

// .text:0x1B18
void dFgObj_c::initPickUp() {
    mScale.x = 0.0f;
    mScale.y = 0.0f;
    mScale.z = 0.0f;
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    dFdBase_c::getUnitGroundPos(&mSndPos, mUnitX, mUnitZ);
    dItem::Item item;
    item = mItem;
    mSnd.setPos(&mSndPos);
    if (item.isFg74()) {
        mSnd.startSound(0x17BF);
    } else if (mItem == dItem::FG_LUCKY_CLOVER) {
        mSnd.startSound(0x17DA);
    }
}

// .text:0x1BC8
void dFgObj_c::initToss(BOOL quiet) {
    mPrevItem = mItem;
    mSndPos = mPos;
    mSpeed.x = 0.0f;
    mSpeed.y = 1.5f;
    mSpeed.z = 0.15f;
    if (mAngleY != 0) {
        mProcTask = 0;
    }
    if (!quiet) {
        dItem::Item item;
        mSnd.setPos(&mSndPos);
        item = mItem;
        if (item.isMoney()) {
            mSnd.startSound(0x1742);
        } else {
            mSnd.startSound(0x17A4);
        }
    }
}

// .text:0x1C98
void dFgObj_c::initSlide() {
    mSndPos = mPos;
    mVec3_c dst;
    dFdBase_c::getUnitGroundPos(&dst, mUnitX, mUnitZ);
    mSpeed.set((dst.x - mPos.x) / 75.0f, (dst.y - mPos.y) / 75.0f,
               (dst.z - mPos.z) / 75.0f);
    if (mAngleY != 0) {
        finishTask();
    }
    mSnd.setPos(&mSndPos);
    mSnd.startSound(0x17BB);
}

// .text:0x1D58
void dFgObj_c::initKnockOff(dFdBase_c *fd) {
    dFdBase_c::getUnitGroundPos(&mSndPos, mUnitX, mUnitZ);
    mSpeed.set(0.5f * mAng(mAngleY).sin(), 3.0f, 0.5f * mAng(mAngleY).cos());
    mProcTask = 0;
    dItem::Item none;
    fd->setItem(&none, mUnitX, mUnitZ, 0);
}

// .text:0x1E38
void dFgObj_c::initBuryMoney() {
    mSndPos = mPos;
    mScale.x = 0.0f;
    mScale.y = 1.0f;
    mScale.z = 0.0f;
    mProcTask = 0;
    mVec3_c dst;
    dFdBase_c::getUnitGroundPos(&dst, mUnitX, mUnitZ);
    mSpeed.set((dst.x - mSndPos.x) / 30.0f, 2.8125f,
               (dst.z - mSndPos.z) / 30.0f);
    playDropSound();
    BOOL planted = FALSE;
    BOOL money = FALSE;
    int player = mPlayer;
    if (player == dNet::getMyNo()) {
        dItem::Item item(mItem);
        if (item.isMoney()) {
            money = TRUE;
        }
    }
    if (money) {
        dItem::Item bag(0x9AA);
        if (fgMngProc_getMemberHeldItem(mPlayer) == bag.getId()) {
            planted = TRUE;
        }
    }
    if (planted) {
        dActivityLog::onPlantMoneyTree(mPlayer);
    }
}

// .text:0x1F74
void dFgObj_c::initFillHole(dFdBase_c *fd) {
    dFdBase_c::getUnitGroundPos(&mSndPos, mUnitX, mUnitZ);
    mItem = dItem::FG_HOLE;
    mParam = dFgobjManager::getFgAnmParam(mItem);
    dBGCF::setUnitAnm(mUnitX, mUnitZ, 2, mParam, mParam);
    dItem::Item none;
    fd->setItem(&none, mUnitX, mUnitZ, 0);
    finishTask();
}

// .text:0x2010
void dFgObj_c::initClear(dFdBase_c *fd) {
    dFdBase_c::getUnitGroundPos(&mSndPos, mUnitX, mUnitZ);
    dItem::Item none;
    fd->setItem(&none, mUnitX, mUnitZ, 0);
}

// .text:0x2078
void dFgObj_c::initDigHoleTask(dFdBase_c *fd) {
    dFdBase_c::getUnitGroundPos(&mSndPos, mUnitX, mUnitZ);
    mItem = dItem::FG_HOLE;
    dBGCF::setUnitAnm(mUnitX, mUnitZ, 0x18, 0.0f,
                      dFgobjManager::getFgAnmParam(mItem));
    dItem::Item none;
    fd->setItem(&none, mUnitX, mUnitZ, 0);
    dFdAsPos_c unit(mUnitX, mUnitZ);
    int task = fgMngTask_find(dScene::getCurrentScene(), &unit, 0);
    if (task >= 0) {
        dFgMngTask_c *t = fgMngTask_get(task);
        t->mNewItem = dItem::ITEM_ID_NONE;
    }
}

// .text:0x214C
void dFgObj_c::initThrow() {
    mSndPos = mPos;
    mVec3_c dst;
    dFdBase_c::snapToUnitCenter(&dst, &mSndPos);
    dFgobjManager::moveOffWater(&dst, FALSE);
    dst.y = dBGCF::getGroundY(&dst, TRUE);
    f32 dy = mPos.y - dst.y;
    f32 g = 5.3333335f;
    int time = EGG::Math<f32>::sqrt(2.0f * dy * g);
    if (time > 26) {
        int n = 26;
        dy -= n * (0.1875f * n) / 2.0f;
        time = n + roundUp(dy / 5.0f);
    }
    mSpeed.set((dst.x - mSndPos.x) / time, 0.0f, (dst.z - mSndPos.z) / time);
}

// .text:0x22D8
void dFgObjAnm_c::initTree(int kind) {
    mHideSlot = -1;
    mVec3_c pos;
    dFdBase_c::getUnitGroundPos(&pos, mUnitX, mUnitZ);
    mPos = pos;
    mSndPos = mPos;
    char mdlName[0x14];
    char chrName[0x20];
    char clrName[0x18];
    memset(mdlName, 0, sizeof(mdlName));
    memset(chrName, 0, sizeof(chrName));
    memset(clrName, 0, sizeof(clrName));
    dItem::Item item(mItem);
    strncpy(mdlName, item.getFgInfo()->mName, 0x11);
    strncat(mdlName, "_t", 2);
    strncpy(chrName, mdlName, 0x13);
    strncpy(clrName, mdlName, 0x13);
    dFgDraw::appendTreeVariant(item, mUnitX, mUnitZ, mdlName);
    nw4r::g3d::ResFile res(g_fgObjRes.getData());
    nw4r::g3d::ResMdl mdl = res.GetResMdl(mdlName);
    mMdl.create(mdl, &mAllocator, 0x148, 1, NULL);
    strncat(clrName, "_cut", 4);
    f32 frame = 0.0f;
    int se = -1;
    switch (kind) {
    case 0:
        strncat(chrName, "_shakeM", 7);
        se = 0x17AD;
        break;
    case 1:
        strncat(chrName, "_shakeS", 7);
        se = 0x17A9;
        break;
    case 2:
        strncat(chrName, "_shakeL_wait", 12);
        se = 0x17AD;
        frame = 15.0f;
        break;
    case 3:
        strncat(chrName, "_shakeL", 7);
        if (mTimer == 0) {
            se = 0x17AD;
        }
        break;
    case 4:
        strncat(chrName, "_cut0_L", 7);
        se = 0x1818;
        break;
    case 5:
        strncat(chrName, "_cut0_R", 7);
        se = 0x1818;
        break;
    case 6:
        strncat(chrName, "_cut1_L", 7);
        se = 0x1818;
        break;
    case 7:
        strncat(chrName, "_cut1_R", 7);
        se = 0x1818;
        break;
    }
    if (se != -1) {
        if (dWeather_c::isRainKind(dWeather_c::sInstance->mWeather)) {
            switch (se) {
            case 0x17A9:
                se = 0x1821;
                break;
            case 0x17AD:
                se = 0x1820;
                break;
            case 0x1818:
                se = 0x181E;
                break;
            }
        }
        mSnd.setPos(&mSndPos);
        mSnd.startSound(se);
    }
    nw4r::g3d::ResAnmChr chr = res.GetResAnmChr(chrName);
    nw4r::g3d::ResAnmClr clr = res.GetResAnmClr(clrName);
    nw4r::g3d::ResAnmVis vis = res.GetResAnmVis(clrName);
    mAnmChr.create(mdl, chr, &mAllocator, NULL);
    mMdl.setAnm(mAnmChr, frame);
    mTreeAnm = kind;
    if (kind >= 4) {
        mAnmClr.create(mdl, clr, &mAllocator, NULL, 1);
        mAnmVis.create(mdl, vis, &mAllocator, NULL);
        mMdl.setAnm(mAnmClr);
        mMdl.setAnm(mAnmVis);
    }
    dFdBase_c *fd = dField::getInfo(1);
    mStep = 0;
    switch (mTreeAnm) {
    case 2:
        mAnmChr.setFrame(0.0f);
    case 1:
        mHideSlot = dFgDraw::hideUnit(mUnitX, mUnitZ);
        break;
    case 0:
    case 3: {
        dFdAsPos_c unit(mUnitX, mUnitZ);
        if (fgMngTask_find(dScene::getCurrentScene(), &unit, 0) >= 0) {
            dropShakeItems(fd->getItem(mUnitX, mUnitZ, 0));
            fgMngTask_procOwn(dScene::getCurrentScene(), &unit, mPlayer, 0);
            mItem = mPrevItem;
            dItem::Item prev(mPrevItem);
            fd->setItem(&prev, mUnitX, mUnitZ, 0);
        }
        mHideSlot = dFgDraw::hideUnit(mUnitX, mUnitZ);
        break;
    }
    default: {
        dFdAsPos_c unit(mUnitX, mUnitZ);
        dropShakeItems(fd->getItem(mUnitX, mUnitZ, 0));
        fgMngTask_procOwn(dScene::getCurrentScene(), &unit, mPlayer, 0);
        mStep = 1;
        break;
    }
    }
    if (item.mId == dItem::FG_CEDAR_LIGHTS) {
        dFgDecoAnm_c *deco = dFgobjManager::getMgr()->findDeco(mUnitX, mUnitZ);
        if (deco == NULL) {
            dFgobjManager::startDecoAnm(mUnitX, mUnitZ, 0.0f);
            deco = dFgobjManager::getMgr()->findDeco(mUnitX, mUnitZ);
            if (deco == NULL) {
                return;
            }
        }
        deco->setTreeAnm(mTreeAnm);
        mDeco = deco;
    }
    startEffects(FALSE);
}

// .text:0x286C
void dFgDecoAnm_c::setTreeAnm(int kind) {
    f32 frame = 0.0f;
    if (mAnmSrt.IsBound()) {
        frame = mAnmSrt.getFrame(0);
    }
    mIsCut = 0;
    mMdl.remove();
    mAnmChr.remove();
    mAnmClr.remove();
    mAnmSrt.remove();
    EGG::FrmHeap *heap = dFgobjManager::getDecoHeap(mIdx);
    if (heap != NULL) {
        heap->free(3);
    }
    char chrName[0x19];
    memset(chrName, 0, sizeof(chrName));
    static const char *mdlName = "fg_treeB_d_t";
    nw4r::g3d::ResFile res(g_fgObjRes.getData());
    nw4r::g3d::ResMdl mdl = res.GetResMdl(mdlName);
    mMdl.create(mdl, &mAllocator, 0x34C, 1, NULL);
    static const char *chrBase = "fg_treeB_4_t";
    strncpy(chrName, chrBase, 12);
    f32 chrFrame = 0.0f;
    switch (kind) {
    case 0:
        strncat(chrName, "_shakeM", 7);
        break;
    case 1:
        strncat(chrName, "_shakeS", 7);
        break;
    case 2:
        strncat(chrName, "_shakeL_wait", 12);
        chrFrame = 15.0f;
        break;
    case 3:
        strncat(chrName, "_shakeL", 7);
        break;
    case 4:
        strncat(chrName, "_cut0_L", 7);
        break;
    case 5:
        strncat(chrName, "_cut0_R", 7);
        break;
    case 6:
        strncat(chrName, "_cut1_L", 7);
        break;
    case 7:
        strncat(chrName, "_cut1_R", 7);
        break;
    }
    mTreeAnmKind = kind;
    if (kind >= 4) {
        mIsCut = 1;
    }
    nw4r::g3d::ResAnmChr chr = res.GetResAnmChr(chrName);
    static const char *clrName = "fg_treeB_4_t_cut";
    nw4r::g3d::ResAnmClr clr = res.GetResAnmClr(clrName);
    nw4r::g3d::ResAnmTexSrt srt = res.GetResAnmTexSrt(mdlName);
    mAnmChr.create(mdl, chr, &mAllocator, NULL);
    mAnmSrt.create(mdl, srt, &mAllocator, NULL, 1);
    mAnmSrt.setFrame(frame, 0);
    mMdl.setAnm(mAnmChr, chrFrame);
    mMdl.setAnm(mAnmSrt);
    if (mTreeAnmKind >= 4) {
        mAnmClr.create(mdl, clr, &mAllocator, NULL, 1);
        mMdl.setAnm(mAnmClr);
    } else if (mTreeAnmKind == 2) {
        mAnmChr.setFrame(0.0f);
    }
}

// .text:0x2BC0
void dFgDecoAnm_c::endTreeAnm() {
    if (mTreeAnmKind >= 0) {
        if (mTreeAnmKind >= 4) {
            remove();
        } else {
            mEndCount = 1;
            f32 frame = 0.0f;
            if (mAnmSrt.IsBound()) {
                frame = mAnmSrt.getFrame(0);
            }
            dFgobjManager::startDecoAnm(mUnitX, mUnitZ, frame);
        }
    }
}

// .text:0x2C60
void dFgDecoAnm_c::init(int x, int z, f32 frame) {
    mActive = TRUE;
    mUnitX = x;
    mUnitZ = z;
    dFdBase_c::getUnitGroundPos(&mPos, x, z);
    static const char *name = "fg_treeB_d";
    nw4r::g3d::ResFile res(g_fgObjRes.getData());
    nw4r::g3d::ResMdl mdl = res.GetResMdl(name);
    mMdl.create(mdl, &mAllocator, 0x204, 1, NULL);
    mAnmSrt.create(mdl, res.GetResAnmTexSrt(name), &mAllocator, NULL, 1);
    mMdl.setAnm(mAnmSrt);
    mAnmSrt.setFrame(frame, 0);
}

// .text:0x2D58
BOOL dFgDecoAnm_c::isPlayingTreeAnm() {
    BOOL playing = FALSE;
    if (mAnmChr.IsBound() || mAnmClr.IsBound()) {
        playing = TRUE;
    }
    return playing;
}

// .text:0x2DB4
void dFgObjAnm_c::initHoneycomb() {
    mSpeed.x = 0.0f;
    mSpeed.y = 1.5f;
    mSpeed.z = 0.15f;
    mSndPos = mPos;
    mSnd.setPos(&mSndPos);
    mSnd.startSound(0x17A4);
    finishTask();
    fgMngProc_setBusy();
    static const char *name = "fg_honeycomb";
    nw4r::g3d::ResFile res(g_fgObjRes.getData());
    nw4r::g3d::ResMdl mdl = res.GetResMdl(name);
    mMdl.create(mdl, &mAllocator, 0x148, 1, NULL);
    nw4r::g3d::ResAnmClr clr = res.GetResAnmClr(name);
    nw4r::g3d::ResAnmVis vis = res.GetResAnmVis(name);
    mAnmClr.create(mdl, clr, &mAllocator, NULL, 1);
    mAnmVis.create(mdl, vis, &mAllocator, NULL);
    mMdl.setAnm(mAnmClr);
    mMdl.setAnm(mAnmVis);
    mMdl.setCallback(NULL);
}

// .text:0x2F2C
void dFgObjAnm_c::initFellTree(dFdBase_c *fd) {
    dFdBase_c::getUnitGroundPos(&mSndPos, mUnitX, mUnitZ);
    mSpeed.set(0.5f * mAng(mAngleY).sin(), 3.0f, 0.5f * mAng(mAngleY).cos());
    mProcTask = 0;
    fd->setItem(dItem::Item(), mUnitX, mUnitZ, 0);
    char mdlName[0x16];
    memset(mdlName, 0, sizeof(mdlName));
    dItem::Item item(mItem);
    strncpy(mdlName, item.getFgInfo()->mName, 0x11);
    dFgDraw::appendTreeVariant(item, mUnitX, mUnitZ, mdlName);
    strncat(mdlName, "_a", 2);
    nw4r::g3d::ResFile res(g_fgObjRes.getData());
    nw4r::g3d::ResMdl mdl = res.GetResMdl(mdlName);
    mMdl.create(mdl, &mAllocator, 0x148, 1, NULL);
    static const char *anmName = "v_dig";
    nw4r::g3d::ResAnmClr clr = res.GetResAnmClr(anmName);
    nw4r::g3d::ResAnmVis vis = res.GetResAnmVis(anmName);
    mAnmClr.create(mdl, clr, &mAllocator, NULL, 1);
    mAnmVis.create(mdl, vis, &mAllocator, NULL);
    mMdl.setAnm(mAnmClr);
    mMdl.setAnm(mAnmVis);
    mMdl.setCallback(NULL);
}

// .text:0x314C
void dFgObjSwingPlant_c::initSwing() {
    dFgobjManager::getMgr()->addSwing();
    mHiddenUnit = dFgDraw::hideUnit(mUnitX, mUnitZ);
    dItem::Item item(mItem);
    dItem::FgInfo *info = item.getFgInfo();
    mSndPos = mPos;
    nw4r::g3d::ResFile res(g_fgObjRes.getData());
    char name[0x14];
    memset(name, 0, sizeof(name));
    strncpy(name, info->mName, 0x11);
    dFgDraw::appendTreeVariant(item, mUnitX, mUnitZ, name);
    mMdl.create(res.GetResMdl(name), &mAllocator, 0, 1, NULL);
    if (item.isDeadSapling() || item.isWiltedFlower()) {
        mStep = 1;
    } else {
        mStep = 0;
    }
}

// .text:0x327C
void dFgObjAnm_c::startEffects(BOOL follow) {
    dItem::Item item(mItem);
    int kind = dFgobjManager::getTreeAnmKind(&item, mUnitX, mUnitZ);
    dItem::FgInfo *info = item.getFgInfo();
    mVec3_c pos(mPos);
    int stage = info->mTreeStage - 1;
    switch (kind) {
    case 3:
        mEffKind = stage + 4;
        pos.y += 16.0f;
        break;
    case 4:
        mEffKind = stage + 8;
        pos.y += 13.0f;
        break;
    default:
        mEffKind = stage;
        pos.y += 16.0f;
        break;
    }
    f32 h = sTreeEffectHeight[mEffKind];
    pos.y += h;
    mCallback.mOffsetY = h;
    if (follow) {
        pos = mTopPos;
    }
    if (item.isCedarTree()) {
        mCallback.mNodeId = 2;
    } else if (item.isPalmTree()) {
        mCallback.mNodeId = 4;
    } else {
        mCallback.mNodeId = 2;
    }
    mMdl.setCallback(&mCallback);
    mEffect.start(mTreeAnm, kind, dScene::getSaveSeason(), mEffKind, &pos,
                  follow);
}

// .text:0x344C
void dFgObjAnm_c::updateEffects(BOOL restart) {
    if (restart) {
        mEffect.restart();
    }
    mEffect.update(&mTopPos, mEffKind);
}

// .text:0x3494
dFgObjAnm_c::mdlCallback_c::mdlCallback_c() : mpOwner(NULL) {
}

// .text:0x34AC
dFgObjAnm_c::mdlCallback_c::~mdlCallback_c() {
    mpOwner = NULL;
}

// .text:0x34F4
void dFgObjAnm_c::mdlCallback_c::timingB(ulong nodeId,
                                         nw4r::g3d::WorldMtxManip *manip,
                                         nw4r::g3d::ResMdl resMdl) {
    if (mpOwner != NULL && nodeId == mNodeId) {
        mMtx_c m;
        manip->GetMatrix(&m);
        nw4r::math::MTX34 t;
        PSMTXTrans(t, 0.0f, mOffsetY, 0.0f);
        PSMTXConcat(m, t, m);
        mVec3_c top(m._03, m._13, m._23);
        mVec3_c field;
        if (!top.isZero()) {
            dWorld::toFieldPosition(&field, &top);
        } else {
            field = mVec3_c::Zero;
        }
        mpOwner->mTopPos = field;
    }
}

// .text:0x3604
void dFgObj_c::procDrop() {
    mScale.x += 0.1f;
    if (mScale.x >= 1.0f) {
        mScale.x = 1.0f;
    }
    mScale.y = mScale.z = mScale.x;
    mSpeed.y -= 0.1875f;
    mSndPos.x += mSpeed.x;
    mSndPos.y += mSpeed.y;
    mSndPos.z += mSpeed.z;
    setDraw(&mSndPos, &mScale, 1);
    f32 ground;
    if (mLayer == 0) {
        ground = dBGCF::getUnitY(&mSndPos);
    } else {
        ground = mParam;
    }
    if (mSpeed.y > 0.0f || mSndPos.y > ground) {
        return;
    }
    mSndPos.y = ground;
    switch (mStep) {
    case 0: {
        mVec3_c dst;
        dFgDraw::getItemPosition(&dst, mItem, mUnitX, mUnitZ, mLayer);
        f32 dx = dst.x - mSndPos.x;
        f32 dz = dst.z - mSndPos.z;
        f32 v = 0.375f * (dst.y - mSndPos.y);
        f32 g = 5.3333335f;
        if (1.5f * 1.5f < v) {
            f32 t = roundUp(
                EGG::Math<f32>::sqrt(g * (2.0f * (dst.y - mSndPos.y))) - 1.0f);
            mSpeed.x = dx / t;
            mSpeed.y = 0.1875f * t;
            mSpeed.z = dz / t;
        } else {
            f32 t = roundUp(g * (1.5f + EGG::Math<f32>::sqrt(1.5f * 1.5f - v)) -
                            1.0f);
            mSpeed.y = 1.5f;
            mSpeed.x = dx / t;
            mSpeed.z = dz / t;
        }
        playLandSound();
        mStep = 1;
        break;
    }
    case 1:
        kill();
        break;
    }
}

// .text:0x382C
void dFgObj_c::procGrow() {
    setDraw(&mSndPos, &mScale, 1);
    mScale.x += 0.11111111f;
    if (mScale.x >= 1.0f) {
        mScale.x = 1.0f;
        kill();
    }
    mScale.y = mScale.z = mScale.x;
}

// .text:0x38A4
void dFgObj_c::procShrink() {
    setDraw(&mSndPos, &mScale, 1);
    if (++mFrame >= 50) {
        mScale.x -= 0.8f;
    } else {
        mScale.x -= 0.004f;
    }
    if (mScale.x <= 0.0f) {
        mScale.x = 0.0f;
        // flowers, dandelions, the lucky clover, saplings and red turnips are
        // planted back
        dItem::Item prev(mPrevItem);
        dItem::FgInfo *info = prev.getFgInfo();
        if (prev.isFlower() || prev.isDandelion() ||
            mPrevItem == dItem::FG_LUCKY_CLOVER ||
            (info != NULL && info->mTreeStage == 0) || prev.isRedTurnip()) {
            if (dFgobjManager::spawnPickUp(mPlayer, mPrevItem, mUnitX,
                                           mUnitZ)) {
                mProcTask = 0;
                kill();
            }
        } else {
            kill();
        }
    }
    mScale.y = mScale.z = mScale.x;
}

// .text:0x3A10
void dFgObj_c::procShrinkFast() {
    setDraw(&mSndPos, &mScale, 1);
    mScale.x -= 0.041666668f;
    if (mScale.x <= 0.0f) {
        mScale.x = 0.0f;
        // flowers, dandelions, the lucky clover, saplings and red turnips are
        // planted back
        dItem::Item prev(mPrevItem);
        dItem::FgInfo *info = prev.getFgInfo();
        if (prev.isFlower() || prev.isDandelion() ||
            mPrevItem == dItem::FG_LUCKY_CLOVER ||
            (info != NULL && info->mTreeStage == 0) || prev.isRedTurnip()) {
            if (dFgobjManager::spawnPickUp(mPlayer, mPrevItem, mUnitX,
                                           mUnitZ)) {
                mProcTask = 0;
                kill();
            }
        } else {
            kill();
        }
    }
    mScale.y = mScale.z = mScale.x;
}

// .text:0x3B4C
void dFgObj_c::procPickUp() {
    static const f32 sScaleXZ[10] = {0.5f,     0.75f,  0.875f,   1.0f,
                                     1.1875f,  1.375f, 1.28125f, 1.1875f,
                                     1.09375f, 1.0f};
    static const f32 sScaleY[10] = {0.0f,  1.375f,  1.21875f, 1.0625f, 0.90625f,
                                    0.75f, 0.8125f, 0.875f,   0.9375f, 1.0f};
    if (mFrame >= 46) {
        if (mFrame == 46) {
            mSnd.startSound(0x17DA);
        }
        mScale.x = sScaleXZ[mFrame - 46];
        mScale.z = sScaleXZ[mFrame - 46];
        mScale.y = sScaleY[mFrame - 46];
        setDraw(&mSndPos, &mScale, 1);
        if (mFrame >= 55) {
            kill();
        }
    } else {
        mScale.x = 0.0f;
        mScale.y = 0.0f;
        mScale.z = 0.0f;
    }
    mFrame++;
}

// .text:0x3C10
void dFgObj_c::procPop() {
    static const f32 sScale[10] = {0.5f,   0.75f,    0.875f,  1.0f,     1.1875f,
                                   1.375f, 1.28125f, 1.1875f, 1.09375f, 1.0f};
    f32 s = sScale[mFrame];
    mScale.x = s;
    mScale.y = s;
    mScale.z = s;
    setDraw(&mSndPos, &mScale, 1);
    if (mFrame >= 9) {
        kill();
    }
    mFrame++;
}

// .text:0x3C88
void dFgObj_c::procThrow() {
    mSpeed.y -= 0.1875f;
    if (mSpeed.y <= -5.0f) {
        mSpeed.y = -5.0f;
    }
    mSndPos.x += mSpeed.x;
    mSndPos.y += mSpeed.y;
    mSndPos.z += mSpeed.z;
    setDraw(&mSndPos, &mScale, 1);
    f32 ground = dBGCF::getGroundY(&mSndPos, TRUE);
    if (mSpeed.y > 0.0f || mSndPos.y > ground) {
        return;
    }
    mSndPos.y = ground;
    dBGCF::groundChk_c chk(&mSndPos, 0, 0, 0);
    switch (mStep) {
    case 0: {
        dFdAsPos_c unit(mUnitX, mUnitZ);
        if (fgMngTask_findMarked(dScene::getCurrentScene(), &unit, 0) >= 0 ||
            chk.mWater == 0) {
            mVec3_c dst;
            dFdBase_c::getUnitGroundPos(&dst, mUnitX, mUnitZ);
            dFgobjManager::moveOffWater(&dst, TRUE);
            f32 dx = dst.x - mSndPos.x;
            f32 dz = dst.z - mSndPos.z;
            f32 v = 0.375f * (dst.y - mSndPos.y);
            f32 g = 5.3333335f;
            if (1.875f * 1.875f < v) {
                f32 t = roundUp(
                    EGG::Math<f32>::sqrt(g * (2.0f * (dst.y - mSndPos.y))) -
                    1.0f);
                mSpeed.x = dx / t;
                mSpeed.y = 0.1875f * t;
                mSpeed.z = dz / t;
            } else {
                f32 t = roundUp(
                    g * (1.875f + EGG::Math<f32>::sqrt(1.875f * 1.875f - v)) -
                    1.0f);
                mSpeed.y = 1.875f;
                mSpeed.x = dx / t;
                mSpeed.z = dz / t;
            }
            mSnd.startSound(0x17C6);
            mStep = 1;
            break;
        }
    }
    case 1:
        if (chk.mWater != 0) {
            mVec3_c pos(mSndPos);
            pos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_L", &pos, NULL, NULL);
            dEffect::create("af_hny_mizutama_L", &pos, NULL, NULL,
                            &dFgobjManager::rippleParticleCallback, 0);
            dPlayerMgr::landBalloonPresent(&pos);
            mSnd.startSound(0x17C7);
        } else {
            dPlayerMgr::landBalloonPresent(NULL);
        }
        kill();
        break;
    }
}

// .text:0x3F60
void dFgObj_c::procToss() {
    mVec3_c dst;
    mSpeed.y -= 0.1875f;
    mSndPos.x += mSpeed.x;
    mSndPos.y += mSpeed.y;
    mSndPos.z += mSpeed.z;
    mVec3_c scale(1.0f, 1.0f, 1.0f);
    setDraw(&mSndPos, &scale, 1);
    f32 ground = dBGCF::getUnitY(&mSndPos);
    if (!(mSndPos.y > ground)) {
        mSndPos.y = ground;
        switch (mStep) {
        case 0: {
            dFdBase_c::getUnitGroundPos(&dst, mUnitX, mUnitZ);
            f32 dx = dst.x - mSndPos.x;
            f32 dz = dst.z - mSndPos.z;
            f32 v = 0.375f * (dst.y - mSndPos.y);
            f32 g = 5.3333335f;
            if (1.875f * 1.875f < v) {
                f32 t = roundUp(
                    EGG::Math<f32>::sqrt(g * (2.0f * (dst.y - mSndPos.y))) -
                    1.0f);
                mSpeed.x = dx / t;
                mSpeed.y = 0.1875f * t;
                mSpeed.z = dz / t;
            } else {
                f32 t = roundUp(
                    g * (1.875f + EGG::Math<f32>::sqrt(1.875f * 1.875f - v)) -
                    1.0f);
                mSpeed.y = 1.875f;
                mSpeed.x = dx / t;
                mSpeed.z = dz / t;
            }
            playLandSound();
            mStep = 1;
            break;
        }
        case 1:
            playLandSound();
            kill();
            break;
        }
    }
}

// .text:0x413C
void dFgObj_c::procSlide() {
    mVec3_c pos;
    pos = mSndPos;
    pos.x += 8.0f * nw4r::math::SinIdx(mFrame += 0x369);
    mVec3_c scale(1.0f, 1.0f, 1.0f);
    setDraw(&pos, &scale, 1);
    mSndPos.x += mSpeed.x;
    mSndPos.y += mSpeed.y;
    mSndPos.z += mSpeed.z;
    if (mSndPos.y <= dBGCF::getUnitBaseY(mUnitX, mUnitZ)) {
        kill();
    }
}

// .text:0x4238
void dFgObj_c::procKnockOff() {
    if (mItem == dItem::FG_TREE_SAPLING) {
        mVec3_c scale(1.0f, 1.0f, 1.0f);
        setDraw(&mSndPos, &scale, 3);
    } else {
        mVec3_c scale(1.0f, 1.0f, 1.0f);
        setDraw(&mSndPos, &scale, 1);
    }
    mSpeed.y -= 0.1875f;
    mSndPos.x += mSpeed.x;
    mSndPos.y += mSpeed.y;
    mSndPos.z += mSpeed.z;
    if (mSndPos.y <= dBGCF::getUnitBaseY(&mSndPos)) {
        kill();
    }
}

// .text:0x4318
void dFgObj_c::procBuryMoney() {
    mScale.x += 0.1f;
    if (mScale.x >= 1.0f) {
        mScale.x = 1.0f;
    }
    mScale.y = mScale.z = mScale.x;
    mSpeed.y -= 0.1875f;
    mSndPos.x += mSpeed.x;
    mSndPos.y += mSpeed.y;
    mSndPos.z += mSpeed.z;
    if (mItem == dItem::FG_TREE_SAPLING) {
        setDraw(&mSndPos, &mScale, 3);
    } else {
        setDraw(&mSndPos, &mScale, 1);
    }
    if (mSpeed.y < 0.0f && mSndPos.y <= dBGCF::getUnitBaseY(&mSndPos)) {
        playLandSound();
        kill();
    }
}

// .text:0x442C
void dFgObj_c::procFillHole() {
    setDraw(&mSndPos, &mScale, 1);
    mFrame++;
    if ((mFrame >= 12 && mFrame <= 20) || mFrame >= 36) {
        mScale.x -= 0.05f;
    }
    if (mScale.x <= 0.0f) {
        mScale.x = 0.0f;
        kill();
    }
    mScale.y = mScale.z = mScale.x;
    f32 frame = mParam * mScale.x;
    dBGCF::setUnitAnm(mUnitX, mUnitZ, 2, frame, frame);
}

// .text:0x44EC
void dFgObj_c::procSpin() {
    mVec3_c pos;
    pos = mSndPos;
    f32 radius = 8.0f * nw4r::math::SinIdx(mTimer += 0x1000);
    pos.x += radius * mAng(mAngleY).sin();
    pos.y = mSndPos.y;
    pos.z += radius * mAng(mAngleY).cos();
    setDraw(&pos, &mScale, 2);
    if (mTimer < 0) {
        kill();
    }
}

// .text:0x45F4
void dFgObjAnm_c::procTree() {
    mFrame++;
    mVec3_c pos;
    mAng ang = dWorld::curvePosition(&pos, &mSndPos);
    mMtx_c m;
    PSMTXTrans(m, pos.x, pos.y, pos.z);
    m.XrotM(ang);
    mMdl.setLocalMtx(&m);
    u32 se = -1;
    if (mAnmChr.IsBound()) {
        BOOL restart = FALSE;
        switch (mTreeAnm) {
        case 2:
            if (++mTimer >= 12u) {
                mTimer -= 12;
                se = 0x17AD;
            }
            break;
        case 3:
            if (mSeCount < 4) {
                if (++mTimer >= 12u) {
                    mTimer -= 12;
                    mSeCount++;
                    se = 0x17AD;
                }
            }
            break;
        case 4:
        case 5:
            if (mAnmChr.checkFrame(sTreeShakeFrame[mEffKind])) {
                se = 0x17BA;
                restart = TRUE;
            }
            break;
        case 6:
        case 7:
            if (mAnmChr.checkFrame(45.0f)) {
                se = 0x17BA;
            } else if (mAnmChr.checkFrame(sTreeChopFrame[mEffKind] - 1.0f)) {
                se = 0x17BA;
                restart = TRUE;
            }
            break;
        }
        if (se == 0x17BA && (mEffKind & 3) == 0) {
            se = -1;
        }
        if (dWeather_c::isRainKind(dWeather_c::sInstance->mWeather)) {
            switch (se) {
            case 0x17BA:
                se = 0x181D;
                break;
            case 0x17AD:
                se = 0x1820;
                break;
            }
        }
        if (se != -1) {
            mSnd.startSound(se);
        }
        if (restart) {
            startEffects(TRUE);
        } else {
            updateEffects(mTreeAnm == 2);
        }
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.play();
    }
    if (mAnmVis.IsBound()) {
        mAnmVis.play();
    }
    mMdl.play();
    if (mAnmChr.isStop()) {
        if (mStep == 0) {
            dFgDraw::showUnit(mHideSlot);
            mStep++;
        } else {
            remove();
        }
    }
}

// .text:0x4900
void dFgObjAnm_c::endLoop() {
    mAnmChr.mPlayMode = m3d::FORWARD_ONCE;
    if (mDeco != NULL) {
        mDeco->endLoop();
    }
}

// .text:0x4920 (dtk merged it into 0x4900: the call above is a tail call)
void dFgDecoAnm_c::endLoop() {
    mAnmChr.mPlayMode = m3d::FORWARD_ONCE;
}

// .text:0x492C
void dFgObjAnm_c::updatePrevItem(const dFdAsPos_c *pos) {
    int task = fgMngTask_find(dScene::getCurrentScene(), pos, 0);
    if (task >= 0) {
        mPrevItem = fgMngTask_get(task)->mNewItem;
    } else {
        mPrevItem = mItem;
    }
}

// .text:0x4994
void dFgObjAnm_c::cancelTree() {
    mMdl.remove();
    mAnmChr.remove();
    mAnmClr.remove();
    mAnmVis.remove();
    EGG::FrmHeap *heap = dFgobjManager::getAnmHeap(mIdx);
    if (heap != NULL) {
        heap->free(3);
    }
    dFgDraw::showUnit(mHideSlot);
    if (mDeco != NULL) {
        mDeco->endTreeAnm();
    }
}

// .text:0x4A3C
void dFgObjAnm_c::finishTree() {
    dFgMngProc_c::setUnitItem(dField::getInfo(1), mUnitX, mUnitZ, mItem, FALSE);
    dFgDraw::showUnit(mHideSlot);
    remove();
}

// .text:0x4A9C
void dFgObjAnm_c::drawTree() {
    if (mTreeAnm < 4) {
        dItem::Item item(mItem);
        dItem::FgInfo *info = item.getFgInfo();
        dFgDraw::add((u16)(s32)info->mStump, mSndPos, mScale, FALSE, FALSE,
                     mAng3_c(0, 0, 0));
    }
    if (mDeco != NULL) {
        mMdl.entry();
        mDeco->mMdl.entry();
    } else {
        mMdl.entry();
    }
}

// .text:0x4B5C
void dFgObjAnm_c::dropShakeItems(const dItem::Item *tree) {
    dFgobjManager::dropTreeItems(tree, mPlayer, mUnitX, mUnitZ);
}

// .text:0x4B74
void dFgObjAnm_c::procHoneycomb() {
    mVec3_c pos;
    mAng ang = dWorld::curvePosition(&pos, &mSndPos);
    mMtx_c m;
    PSMTXTrans(m, pos.x, pos.y, pos.z);
    m.XrotM(ang);
    mMdl.setLocalMtx(&m);
    switch (mStep) {
    case 0: {
        f32 ground = dBGCF::getUnitY(&mSndPos);
        mSpeed.y -= 0.1875f;
        mSndPos.x += mSpeed.x;
        mSndPos.y += mSpeed.y;
        mSndPos.z += mSpeed.z;
        if (mSndPos.y <= ground) {
            mSndPos.y = ground;
            mSpeed.y = 1.875f;
            mStep = 1;
        }
        break;
    }
    case 1: {
        f32 ground = dBGCF::getUnitY(&mSndPos);
        mSpeed.y -= 0.1875f;
        mSndPos.x += mSpeed.x;
        mSndPos.y += mSpeed.y;
        mSndPos.z += mSpeed.z;
        if (mSndPos.y <= ground) {
            mSndPos.y = ground;
            mSpeed.y = 0.0f;
            mStep = 2;
            mTimer = 40;
            dBgm::l_mgr.mEvent.mute18();
        }
        break;
    }
    case 2:
        if (--mTimer < 0) {
            if (g_playActorMng != NULL) {
                fn_116_1C74(g_playActorMng, &mSndPos);
            }
            mTimer = 60;
            mStep = 3;
        }
        break;
    case 3:
        if (--mTimer < 0) {
            mStep = 4;
        }
        break;
    default:
        if (mAnmClr.IsBound()) {
            mAnmClr.play();
        }
        if (mAnmVis.IsBound()) {
            mAnmVis.play();
        }
        if (mAnmClr.isStop(0)) {
            remove();
        }
        break;
    }
}

// .text:0x4DD8
void dFgObjAnm_c::drawHoneycomb() {
    mMdl.entry();
}

// .text:0x4DE8
void dFgObjAnm_c::procFellTree() {
    if (mAnmClr.IsBound()) {
        mAnmClr.play();
    }
    if (mAnmVis.IsBound()) {
        mAnmVis.play();
    }
    mVec3_c pos;
    mAng ang = dWorld::curvePosition(&pos, &mSndPos);
    mMtx_c m;
    PSMTXTrans(m, pos.x, pos.y, pos.z);
    m.XrotM(ang);
    mMdl.setLocalMtx(&m);
    mSpeed.y -= 0.1875f;
    mSndPos.x += mSpeed.x;
    mSndPos.y += mSpeed.y;
    mSndPos.z += mSpeed.z;
    if (mSndPos.y <= dBGCF::getUnitBaseY(&mSndPos)) {
        remove();
    }
}

// .text:0x4F0C
void dFgObjAnm_c::drawFellTree() {
    mMdl.entry();
}

// .text:0x4F1C
void dFgObjSwingPlant_c::calcSwing() {
    dItem::Item *item = dField::getInfo(0)->getItem(mUnitX, mUnitZ, 0);
    if (mItem != item->mId) {
        endSwing();
        return;
    }
    mFrame++;
    if (++mTimer >= 21) {
        mTimer = 0;
    }
    mVec3_c pos;
    mAng ang = dWorld::curvePosition(&pos, &mSndPos);
    mMtx_c m;
    PSMTXTrans(m, pos.x, pos.y, pos.z);
    m.XrotM(ang);
    f32 amp = ((56 - mFrame) << 10) / 56;
    if (mStep == 1) {
        amp *= 0.5f;
    }
    int rot = amp * nw4r::math::SinIdx(mTimer * 0x10000 / 21);
    f32 c = nw4r::math::CosIdx(rot);
    f32 s = nw4r::math::SinIdx(rot);
    f32 len = EGG::Math<f32>::sqrt(m._11 * m._11 + m._21 * m._21);
    m._01 = -s * len;
    m._11 *= c;
    m._21 *= c;
    mMdl.setLocalMtx(&m);
    if (mFrame >= 56) {
        endSwing();
    }
}

// .text:0x513C
void dFgObjSwingPlant_c::drawSwing() {
    mMdl.entry();
}

// .text:0x514C
void dFgObjSwingPlant_c::endSwing() {
    dFgobjManager::getMgr()->removeSwing();
    dFgDraw::showUnit(mHiddenUnit);
    remove();
}

// The tree effects: the leaves (af_tree*) and the snow (af_snow_tree*) a shaken
// or felled tree drops.
static const char *sTreeEffectNames[] = {
    "af_treeA0",     "af_treeA1",     "af_treeA2",     "af_treeB",
    "af_treeC",      "af_treeA0_chr", "af_treeA1_chr", "af_treeA2_chr",
    "af_treeA0_snw", "af_treeA1_snw", "af_treeA2_snw", "af_treeB_snw",
    "af_treeC_snw",
};
static const char *sTreeSnowEffectNames[] = {"af_snow_treeA", "af_snow_treeB",
                                             "af_snow_treeC"};

// .text:0x5198
void dFgObjAnm_c::effect_c::start(int treeAnm, int kind, int season,
                                  int effKind, const mVec3_c *pos,
                                  BOOL follow) {
    mSnowName = 3;
    switch (kind) {
    case 0:
        if (season >= 0x13) {
            mName = 8;
            mSnowName = 0;
        } else {
            mName = 0;
        }
        break;
    case 1:
        if (season >= 0x13) {
            mName = 9;
            mSnowName = 0;
        } else if (season == 1 || season == 2) {
            mName = 6;
        } else {
            mName = 1;
        }
        break;
    case 2:
        if (season >= 0x13) {
            mName = 10;
            mSnowName = 0;
        } else if (season == 2) {
            mName = 7;
        } else {
            mName = 2;
        }
        break;
    case 3:
        if (season >= 0x13) {
            mName = 11;
            mSnowName = 1;
        } else {
            mName = 3;
        }
        break;
    case 4:
        if (season >= 0x13) {
            mName = 12;
            mSnowName = 2;
        } else {
            mName = 4;
        }
        break;
    }
    int ofs;
    switch (treeAnm) {
    case 1:
        ofs = 24;
        break;
    case 0:
        ofs = 12;
        break;
    case 2:
    case 3:
        ofs = 0;
        break;
    case 4:
    case 5:
        ofs = follow ? 60 : 36;
        break;
    case 6:
    case 7:
        ofs = follow ? 60 : 48;
        break;
    }
    mIdx = effKind + ofs;
    mCount = sTreeEffectCount[mIdx] - 1;
    f32 scale = sTreeEffectScale[effKind];
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    if (mCount >= 0 &&
        mEffect.create(sTreeEffectNames[mName], pos, NULL, &mScale, 0)) {
        mEffect.vf3C(sTreeEffectRate[effKind]);
        mEffect.vf2C(sTreeEffectParamA[mIdx]);
        mEffect.vf38(sTreeEffectParamB[mIdx]);
    }
    if (mSnowName != 3 && !follow) {
        mSnowCount = sTreeSnowCount[mIdx] - 1;
        if (mSnowCount >= 0 &&
            mSnowEffect.create(sTreeSnowEffectNames[mSnowName], pos, NULL,
                               &mScale, 0)) {
            mSnowEffect.vf3C(sTreeSnowRate[effKind]);
            mSnowEffect.vf2C(sTreeSnowParam[mIdx]);
        }
    } else {
        mSnowCount = -1;
    }
}

// .text:0x54F0
void dFgObjAnm_c::effect_c::update(const mVec3_c *pos, int effKind) {
    if (mCount > 0) {
        mEffect.create(sTreeEffectNames[mName], pos, NULL, &mScale, 0);
        mCount--;
        mEffect.vf3C(sTreeEffectRate[effKind]);
        mEffect.vf2C(sTreeEffectParamA[mIdx]);
        mEffect.vf38(sTreeEffectParamB[mIdx]);
    }
    if (mSnowCount > 0) {
        mSnowEffect.create(sTreeSnowEffectNames[mSnowName], pos, NULL, &mScale,
                           0);
        mSnowCount--;
        mSnowEffect.vf3C(sTreeSnowRate[effKind]);
        mSnowEffect.vf2C(sTreeSnowParam[mIdx]);
    }
}

// ---------------------------------------------------------------------------------------------
// .text 0x5670 .. 0x5798

// .text:0x5670
void dFgObjAnm_c::effect_c::restart() {
    if (mCount >= 0) {
        mCount = sTreeEffectCount[mIdx] - 1;
    }
    if (mSnowCount >= 0) {
        mSnowCount = sTreeSnowCount[mIdx] - 1;
    }
}

namespace dFgobjManager {

// .text:0x56BC
EGG::FrmHeap *getAnmHeap(int idx) {
    return getMgr()->mAnmHeaps[idx];
}

// .text:0x56F8
EGG::FrmHeap *getSwingHeap(int idx) {
    return getMgr()->mSwingHeaps[idx];
}

// .text:0x5734
EGG::FrmHeap *getDecoHeap(int idx) {
    return getMgr()->mDecoHeaps[idx];
}

} // namespace dFgobjManager

// .text:0x5770
void dFgObjMng_c::addSwing() {
    mSwingNum++;
}

// .text:0x5784
void dFgObjMng_c::removeSwing() {
    mSwingNum--;
}

// ---------------------------------------------------------------------------------------------
// .text 0x5798 .. 0x5D74: dFgObjMng_c.

// .text:0x5798
int dFgObjMng_c::create() {
    int i;
    sFgObjMgr = this;
    for (i = 0; i < OBJ_NUM; i++) {
        mObjs[i].setup(i);
    }
    for (i = 0; i < ANM_NUM; i++) {
        mAnmHeaps[i] =
            mHeap::createFrmHeap(0xE00, dHeap::fgHeap_p, "dFgObjMng_c::m_heap",
                                 0x20, mHeap::OPT_NONE);
        mAnmObjs[i].setup(i);
    }
    for (i = 0; i < SWING_NUM; i++) {
        mSwingHeaps[i] = mHeap::createFrmHeap(0x220, dHeap::fgHeap_p,
                                              "dFgObjMng_c::m_heapSwing", 0x20,
                                              mHeap::OPT_NONE);
        mSwingObjs[i].setup(i);
    }
    for (i = 0; i < DECO_NUM; i++) {
        mDecoHeaps[i] = mHeap::createFrmHeap(0xE00, dHeap::fgHeap_p,
                                             "dFgObjMng_c::m_heapDeco", 0x20,
                                             mHeap::OPT_NONE);
        mDecoObjs[i].setup(i);
    }
    mSwingNum = 0;
    fgMngProc_setFgObjCallback(&dFgobjManager::fgObjCallback);
    return 1;
}

// .text:0x5940
int dFgObjMng_c::execute() {
    int i;
    procCommands();
    swayPlantsNearPlayer();
    for (i = 0; i < OBJ_NUM; i++) {
        mObjs[i].mDrawMode = 0;
        mObjs[i].execute();
    }
    for (i = 0; i < ANM_NUM; i++) {
        mAnmObjs[i].execute();
    }
    for (i = 0; i < SWING_NUM; i++) {
        mSwingObjs[i].execute();
    }
    for (i = 0; i < DECO_NUM; i++) {
        mDecoObjs[i].execute();
    }
    return 1;
}

// .text:0x5A48
int dFgObjMng_c::draw() {
    for (int i = 0; i < OBJ_NUM; i++) {
        mObjs[i].draw();
    }
    for (int i = 0; i < ANM_NUM; i++) {
        mAnmObjs[i].draw();
    }
    for (int i = 0; i < SWING_NUM; i++) {
        mSwingObjs[i].draw();
    }
    for (int i = 0; i < DECO_NUM; i++) {
        mDecoObjs[i].draw();
        if (mDecoObjs[i].isEnding()) {
            mDecoObjs[i].mEndCount++;
        }
    }
    return 1;
}

// .text:0x5B5C
int dFgObjMng_c::doDelete() {
    int i;
    fgMngProc_setFgObjCallback(NULL);
    for (i = 0; i < ANM_NUM; i++) {
        mAnmObjs[i].remove();
        if (mAnmHeaps[i] != NULL) {
            mHeap::destroyFrmHeap(mAnmHeaps[i]);
            mAnmHeaps[i] = NULL;
        }
    }
    for (i = 0; i < SWING_NUM; i++) {
        mSwingObjs[i].remove();
        if (mSwingHeaps[i] != NULL) {
            mHeap::destroyFrmHeap(mSwingHeaps[i]);
            mSwingHeaps[i] = NULL;
        }
    }
    for (i = 0; i < DECO_NUM; i++) {
        mDecoObjs[i].remove();
        if (mDecoHeaps[i] != NULL) {
            mHeap::destroyFrmHeap(mDecoHeaps[i]);
            mDecoHeaps[i] = NULL;
        }
    }
    sFgObjMgr = NULL;
    return 1;
}

// .text:0x5CA4
dFgObj_c *dFgObjMng_c::getFreeObj() {
    dFgObj_c *obj = mObjs;
    int i = 0;
    do {
        if (!obj->mActive) {
            return obj;
        }
        obj++;
        i++;
    } while (i < OBJ_NUM);
    return NULL;
}

// .text:0x5CD0
dFgObjAnm_c *dFgObjMng_c::getFreeAnmObj() {
    dFgObjAnm_c *obj = mAnmObjs;
    int i = 0;
    do {
        if (!obj->mActive) {
            return obj;
        }
        obj++;
        i++;
    } while (i < ANM_NUM);
    return NULL;
}

// .text:0x5CFC
dFgObjSwingPlant_c *dFgObjMng_c::getFreeSwingObj() {
    dFgObjSwingPlant_c *obj = mSwingObjs;
    int i = 0;
    do {
        if (!obj->mActive) {
            return obj;
        }
        obj++;
        i++;
    } while (i < SWING_NUM);
    return NULL;
}

// .text:0x5D2C
dFgDecoAnm_c *dFgObjMng_c::getFreeDecoObj() {
    dFgDecoAnm_c *deco = mDecoObjs;
    for (int i = 0; i < DECO_NUM; i++) {
        if (!deco->mActive) {
            return &mDecoObjs[i];
        }
        deco++;
    }
    return NULL;
}

// ---------------------------------------------------------------------------------------------
// .text 0x5D74 .. 0x8000: spawning objects (the requests from other modules).

// .text:0x5D74
BOOL dFgObjMng_c::spawn(int player, int type, u16 item, int x, int z,
                        mVec3_c pos, s16 angle, int arg, u8 layer) {
    dFdBase_c *fd = dField::getInfo(0);
    if (fd == NULL) {
        return FALSE;
    }
    if (type == 0x13) {
        dFgObjSwingPlant_c *obj = getFreeSwingObj();
        if (obj == NULL) {
            return FALSE;
        }
        obj->init(player, x, z, pos, type, item, fd, angle, arg, layer);
    } else if (type >= 0xF) {
        dFgObjAnm_c *obj = getFreeAnmObj();
        if (obj == NULL) {
            return FALSE;
        }
        obj->init(player, x, z, pos, type, item, fd, angle, arg, layer);
    } else {
        dFgObj_c *obj = getFreeObj();
        if (obj == NULL) {
            return FALSE;
        }
        obj->init(player, x, z, pos, type, item, fd, angle, arg, layer);
    }
    return TRUE;
}

namespace dFgobjManager {

// .text:0x5F14
BOOL killObjAt(int x, int z) {
    BOOL ret = FALSE;
    dFgObj_c *obj = getMgr()->mObjs;
    for (int i = 0; i < dFgObjMng_c::OBJ_NUM; i++, obj++) {
        if (obj->mActive && isObjAt(obj, x, z)) {
            if (obj->mProcTask) {
                ret = TRUE;
            }
            obj->kill();
            return ret;
        }
    }
    dFgObjAnm_c *anm = getMgr()->mAnmObjs;
    for (int i = 0; i < dFgObjMng_c::ANM_NUM; i++, anm++) {
        if (anm->mActive && isObjAt(anm, x, z)) {
            if (anm->mProcTask) {
                ret = TRUE;
            }
            anm->kill();
            return ret;
        }
    }
    dFgObjSwingPlant_c *swing = getMgr()->mSwingObjs;
    for (int i = 0; i < dFgObjMng_c::SWING_NUM; i++, swing++) {
        if (swing->mActive && isObjAt(swing, x, z)) {
            if (swing->mProcTask) {
                ret = TRUE;
            }
            swing->kill();
            return ret;
        }
    }
    return FALSE;
}

// .text:0x6094
void dropFruit(const dItem::Item *tree, int player, int x, int z) {
    mVec3_c ground;
    dFdBase_c::getUnitGroundPos(ground, x, z);
    dItem::Item fruit(sFruit[((dItem::Item *)tree)->getFruitTreeType()]);
    u16 fruitId = fruit.getId();
    int kind;
    u32 num;
    if (tree->isCedarTree()) {
        num = 2;
        kind = 1;
    } else if (tree->isPalmTree()) {
        num = 2;
        kind = 2;
    } else {
        num = 3;
        kind = 0;
    }
    for (u32 i = 0; i < num; i++) {
        mVec3_c pos;
        mVec3_c ofs;
        ofs = *dFgDraw::getFruitOffset(i, kind);
        int idx = fgMngTask_findGroup((u8)player, (u8)i, TRUE);
        BOOL drop = TRUE;
        pos.x = ground.x + ofs.x;
        pos.y = ground.y + ofs.y;
        pos.z = ground.z + ofs.z;
        int dx;
        int dz;
        if (idx >= 0) {
            dFgMngTask_c *task = fgMngTask_get(idx);
            dx = task->getX();
            dz = task->getZ();
            if (i == task->mDropIdx) {
                drop = FALSE;
                task->mHold = 1;
            }
        }
        if (drop == TRUE) {
            u8 ofsXZ = sDirOffset[sFruitDir[i]];
            dx = x + (ofsXZ >> 4) - 8;
            dz = z + (ofsXZ & 0xF) - 8;
        }
        spawnFruitDrop(player, fruitId, dx, dz, &pos, drop, i);
    }
}

// .text:0x6278
void dropHoneycomb(int player, int x, int z) {
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!dNet::isSessionActive() && priv != NULL && !priv->isFlag0(0xD)) {
        mVec3_c ground;
        dFdBase_c::getUnitGroundPos(ground, x, z);
        mVec3_c pos;
        mVec3_c ofs;
        ofs = *dFgDraw::getFruitOffset(
            fgMngState_isPlayerLeftOf(player, ground), 0);
        pos = ground + ofs;
        spawnHoneycomb(player, &pos);
    }
}

// .text:0x6360
void dropShakeItem(const dItem::Item *tree, int player, int x, int z) {
    u16 id = tree->mId;
    BOOL bells = FALSE;
    int kind;
    if (tree->isBroadleafTree()) {
        kind = 0;
        if (id == dItem::FG_TREE_FTR) {
            bells = TRUE;
        }
    } else if (tree->isCedarTree()) {
        kind = 1;
        if (id == dItem::FG_CEDAR_FTR) {
            bells = TRUE;
        }
    }
    mVec3_c ground;
    dFdBase_c::getUnitGroundPos(ground, x, z);
    int dir = fgMngState_isPlayerLeftOf(player, ground);
    mVec3_c pos;
    mVec3_c ofs;
    ofs = *dFgDraw::getFruitOffset(dir, kind);
    int idx = fgMngTask_findGroup((u8)player, 0, FALSE);
    pos.x = ground.x + ofs.x;
    pos.y = ground.y + ofs.y;
    pos.z = ground.z + ofs.z;
    u16 item;
    BOOL drop;
    if (idx >= 0) {
        dFgMngTask_c *task = fgMngTask_get(idx);
        drop = FALSE;
        item = task->mNewItem;
        x = task->getX();
        z = task->getZ();
    } else {
        if (!bells) {
            u8 ofsXZ = sDirOffset[sFruitDir[dir + 3]];
            x = x + (ofsXZ >> 4) - 8;
            z = z + (ofsXZ & 0xF) - 8;
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mFortune ==
                FORTUNE_MONEY) {
                item = dItem::Item(0x55).mId;
            } else {
                item = dItem::Item(0x53).mId;
            }
        } else {
            u8 ofsXZ = sDirOffset[sFruitDir[dir]];
            x = x + (ofsXZ >> 4) - 8;
            z = z + (ofsXZ & 0xF) - 8;
            item = dItem::Item(0x640).mId;
        }
        drop = TRUE;
    }
    if (!bells) {
        spawnFruitDrop(player, item, x, z, &pos, drop, 0);
    } else {
        spawnShakeItemDrop(player, item, x, z, &pos, drop);
    }
}

// .text:0x65A4
void dropTreeItems(const dItem::Item *tree, int player, int x, int z) {
    u16 id = tree->mId;
    if (tree->isPeachTree() || tree->isAppleTree() || tree->isOrangeTree() ||
        tree->isPearTree() || tree->isCherryTree() || tree->isPalmTree() ||
        tree->isMoneyTree()) {
        dItem::FgInfo *info = tree->getFgInfo();
        if (info != NULL && (u16)(int)info->mDropItem != dItem::ITEM_ID_NONE) {
            dropFruit(tree, player, x, z);
        }
    } else if (id == dItem::FG_TREE_BEES || id == dItem::FG_CEDAR_BEES) {
        if (fgMngProc_canEditField()) {
            dropHoneycomb(player, x, z);
        }
    } else if (id == dItem::FG_TREE_FTR || id == dItem::FG_TREE_BELLS ||
               id == dItem::FG_CEDAR_FTR || id == dItem::FG_CEDAR_BELLS) {
        dropShakeItem(tree, player, x, z);
    }
}

// .text:0x6770
void fgObjCallback(int item, int pos, int scale, mAng3_c *ang, int present) {
    if (getMgr() != NULL) {
        getMgr()->addFgDraw(item, (const mVec3_c *)pos, (const mVec3_c *)scale,
                            ang, present);
    }
}

} // namespace dFgobjManager

// .text:0x67D8
void dFgObjMng_c::getDrawItem(dItem::Item *item) {
    u16 id = item->mId;
    if (dItem::Item(id).isRedTurnip()) {
        item->setFromIndex(0x18B, id - dItem::FG_RED_TURNIP_0, FALSE);
        return;
    }
    BOOL replace = TRUE;
    if (id != dItem::FG_LUCKY_CLOVER) {
        BOOL match = FALSE;
        if (dScene::isSceneAttr(dScene::getCurrentScene(), 5)) {
            if (item->mId == dItem::Item(0x5BE).mId) {
                match = TRUE;
            }
        }
        if (!match) {
            replace = FALSE;
        }
    }
    if (replace) {
        item->mId = dItem::FG_4LEAF_CLOVER;
    }
}

// .text:0x68B0
void dFgObjMng_c::addFgDraw(int id, const mVec3_c *pos, const mVec3_c *scale,
                            mAng3_c *ang, BOOL present) {
    dItem::Item item((u16)id);
    getDrawItem(&item);
    if (item.isFg74()) {
        dFgDraw::addFlat(item.mId, *pos, *scale);
    } else {
        dFgDraw::add(item.mId, *pos, *scale, FALSE, present, *ang);
    }
}

namespace dFgobjManager {

// .text:0x6948
int getTreeAnmKind(const dItem::Item *tree, int a, int b) {
    if (tree->isBroadleafTree()) {
        return ((a ^ b) & 1) + 1;
    }
    if (tree->isCedarTree()) {
        return 3;
    }
    return tree->isPalmTree() ? 4 : 0;
}

// .text:0x69D0
BOOL spawnDrop(int player, u16 item, int x, int z, const mVec3_c *pos,
               u8 layer) {
    return getMgr()->spawn(player, 0, item, x, z, *pos, 0, 0, layer);
}

// .text:0x6A58
f32 getFgAnmParam(u16 id) {
    f32 ret = 0.0f;
    dItem::FgInfo *info = dItem::Item(id).getFgInfo();
    if (info != NULL) {
        ret = (s8)info->_08[0];
    }
    return ret;
}

// .text:0x6ACC
BOOL spawnDigUp(int player, u16 item, int x, int z, int arg) {
    return getMgr()->spawn(player, 1, item, x, z, mVec3_c(0.0f, 0.0f, 0.0f), 0,
                           arg, 0) == TRUE;
}

// .text:0x6B5C
BOOL requestPitfallEnter(int player, int x, int z) {
    return getMgr()->spawn(player, 2, dItem::FG_HOLE, x, z,
                           mVec3_c(0.0f, 0.0f, 0.0f), 0, 0, 0) == TRUE;
}

// .text:0x6BEC
BOOL spawnDigHole(int player, int x, int z) {
    return getMgr()->spawn(player, 3, dItem::ITEM_ID_NONE, x, z,
                           mVec3_c(0.0f, 0.0f, 0.0f), 0, 0, 0) == TRUE;
}

// .text:0x6C84
BOOL spawnPickUp(int player, u16 item, int x, int z) {
    if (item == dItem::FG_LUCKY_CLOVER) {
        return getMgr()->spawn(player, 0xD, item, x, z,
                               mVec3_c(0.0f, 0.0f, 0.0f), 0, 0, 0);
    } else {
        return getMgr()->spawn(player, 4, item, x, z, mVec3_c(0.0f, 0.0f, 0.0f),
                               0, 0, 0);
    }
}

// .text:0x6D64
BOOL spawnPickUpSpecial(int player, u16 item, int x, int z) {
    return getMgr()->spawn(player, 0xD, item, x, z, mVec3_c(0.0f, 0.0f, 0.0f),
                           0, 0, 0);
}

// .text:0x6DF4
void spawnFruitDrop(int player, u16 item, int x, int z, const mVec3_c *pos,
                    s16 drop, int idx) {
    getMgr()->spawn(player, 5, item, x, z, *pos, drop, idx, 0);
}

// .text:0x6E80
BOOL landBalloonItem(u16 item, const mVec3_c *pos) {
    int x;
    int z;
    BOOL ret = findDropSpot(pos, &x, &z);
    if (ret) {
        dFdAsPos_c unit(x, z);
        clearRequest(requestAction(&sFgMngState, 0, FG_MNG_KIND_BALLOON, unit,
                                   item, item, 0, 0, 0, -1));
    }
    getMgr()->spawn(0, 0xE, item, x, z, *pos, 0, 0, 0);
    return ret;
}

// .text:0x6F78
void spawnShakeItemDrop(int player, u16 item, int x, int z, const mVec3_c *pos,
                        s16 drop) {
    getMgr()->spawn(player, 6, item, x, z, *pos, drop, 0, 0);
}

// .text:0x7000
void spawnHoneycomb(int player, const mVec3_c *pos) {
    getMgr()->spawn(player, 0x10, dItem::FG_HONEYCOMB, 0, 0, *pos, 0, 0, 0);
}

// .text:0x7070
void spawnFellTree(int player, u16 item, int x, int z, s16 param) {
    getMgr()->spawn(player, 0x11, item, x, z, mVec3_c(0.0f, 0.0f, 0.0f), param,
                    0, 0);
}

// .text:0x70F4
void spawnBuryMoney(int player, u16 item, int x, int z, const mVec3_c *pos) {
    getMgr()->spawn(player, 8, item, x, z, *pos, 0, 0, 0);
}

// .text:0x7188
BOOL spawnFillHole(int player, int x, int z) {
    return getMgr()->spawn(player, 9, dItem::ITEM_ID_NONE, x, z,
                           mVec3_c(0.0f, 0.0f, 0.0f), 0, 0, 0) == TRUE;
}

// .text:0x7220
void spawnClear(int player, u16 item, int x, int z, s16 param) {
    getMgr()->spawn(player, 0xA, item, x, z, mVec3_c(0.0f, 0.0f, 0.0f), param,
                    0, 0);
}

// .text:0x72A4
void spawnDropAt(int player, u16 item, int x, int z, const mVec3_c *pos,
                 int arg) {
    getMgr()->spawn(player, 0xB, item, x, z, *pos, 0, arg, 0);
}

// .text:0x732C
void procChopResult(int x, int z, s16 param) {
    int player = dNet::getMyNo();
    int idx = fgMngTask_findGroup((u8)player, 4, FALSE);
    if (idx >= 0) {
        spawnClear(player, fgMngTask_get(idx)->mNewItem, x, z, param);
    }
    idx = fgMngTask_findGroup((u8)player, 3, TRUE);
    if (idx >= 0) {
        dFgMngTask_c *task = fgMngTask_get(idx);
        mVec3_c pos;
        dFdBase_c::getUnitGroundPos(pos, x, z);
        spawnDropAt(player, task->mNewItem, task->getX(), task->getZ(), &pos,
                    task->mStage);
        task->mHold = 1;
    }
}

// .text:0x7414
BOOL requestPitfallExit(int player, int x, int z) {
    return getMgr()->spawn(player, 0xC, dItem::FG_HOLE, x, z,
                           mVec3_c(0.0f, 0.0f, 0.0f), 0, 0, 0) == TRUE;
}

// .text:0x74A4
BOOL hasStumpTask(int player, int x, int z) {
    BOOL ret = FALSE;
    dFdAsPos_c pos(x, z);
    int idx = fgMngTask_findOwn(player, 0, &pos, 0);
    if (idx >= 0) {
        dItem::Item item;
        item = fgMngTask_get(idx)->mNewItem;
        if (item.isStump()) {
            ret = TRUE;
        }
    }
    return ret;
}

// .text:0x751C
BOOL checkTreeVisible(int player, int x, int z, int kind) {
    BOOL ret = FALSE;
    if (getMgr() != NULL) {
        dItem::Item *item = dField::getInfo(1)->getItem(x, z, 0);
        if (item != NULL) {
            dItem::FgInfo *info = item->getFgInfo();
            if (info != NULL && info->mTreeStage > 0) {
                mVec3_c pos;
                dFdBase_c::getUnitGroundPos(pos, x, z);
                dCamera_c *camera = (dCamera_c *)dScene::getSceneE848();
                if (camera != NULL && !camera->isPastForeground(&pos, 32.0f)) {
                    ret = TRUE;
                }
            }
        }
    }
    if (!ret) {
        dItem::Item *item = dField::getInfo(1)->getItem(x, z, 0);
        if (item != NULL) {
            switch (kind) {
            case 0:
            case 3:
            case 4:
            case 5:
                dropTreeItems(item, player, x, z);
                break;
            case 2:
                return ret;
            }
        }
        dFdAsPos_c pos(x, z);
        fgMngTask_procOwn(dScene::getCurrentScene(), &pos, player, 0);
    }
    return ret;
}

// .text:0x7660
int restartTreeAnm(int player, int x, int z, int kind) {
    if (getMgr() != NULL) {
        for (int i = 0; i < dFgObjMng_c::ANM_NUM; i++) {
            dFgObjAnm_c *obj = getMgr()->getAnmObj(i);
            if (obj->mType == 15 && isObjAt(obj, x, z)) {
                int anm = obj->mTreeAnm;
                if (kind == 1 || (kind == 2 && anm != 1)) {
                    return -1;
                }
                if (kind == 3 && anm == 2) {
                    dFdAsPos_c pos(x, z);
                    obj->cancelTree();
                    obj->updatePrevItem(&pos);
                    if (player != obj->mPlayer) {
                        obj->mPlayer = player;
                    }
                    obj->initTree(kind);
                    return 0;
                }
                obj->finishTree();
                break;
            }
        }
    }
    return 1;
}

// .text:0x7784
BOOL startShakeTree(int player, int x, int z, int kind) {
    if (kind == 1 && !fgMngProc_isSelfMember(player)) {
        return FALSE;
    }
    int ret = restartTreeAnm(player, x, z, kind);
    if (ret == -1) {
        return FALSE;
    }
    if (ret == 0) {
        return TRUE;
    }
    if (!checkTreeVisible(player, x, z, kind)) {
        return FALSE;
    }
    dItem::Item *item = dField::getInfo(1)->getItem(x, z, 0);
    if (item == NULL) {
        return FALSE;
    }
    u16 id = item->mId;
    return getMgr()->spawn(player, 15, id, x, z, mVec3_c(0.0f, 0.0f, 0.0f), 0,
                           kind, 0);
}

// .text:0x78A4
BOOL stopShakeTree(int player, int x, int z) {
    if (getMgr() != NULL) {
        for (int i = 0; i < dFgObjMng_c::ANM_NUM; i++) {
            dFgObjAnm_c *obj = &getMgr()->mAnmObjs[i];
            if (player == obj->mPlayer && obj->mType == 15 &&
                isObjAt(obj, x, z) && obj->mTreeAnm == 2) {
                obj->endLoop();
                return TRUE;
            }
        }
    }
    return FALSE;
}

// .text:0x7968
BOOL startChopTree(int player, int x, int z, int kind) {
    if (restartTreeAnm(player, x, z, kind) < 0) {
        return FALSE;
    }
    if (!checkTreeVisible(player, x, z, kind)) {
        return FALSE;
    }
    u16 id;
    dFdBase_c *fd = dField::getInfo(1);
    dItem::Item *item = fd->getItem(x, z, 0);
    if (item == NULL) {
        return FALSE;
    }
    id = item->mId;
    dFdAsPos_c pos(x, z);
    if (!hasStumpTask(player, x, z)) {
        kind = 0;
    } else {
        u8 scene = dScene::getCurrentScene();
        if (fgMngTask_findOwn(player, scene, &pos, 0) >= 0) {
            dItem::FgInfo *info = item->getFgInfo();
            mVec3_c center;
            dFdBase_c::getUnitCenterPos(center, x, z);
            f32 y = dBGCF::getUnitY(&center);
            switch (kind) {
            case 4: {
                dFdBase_c::getUnitCenterPos(center, x - 2, z);
                f32 sideY = dBGCF::getUnitY(&center);
                if (info->mTreeStage > 2 &&
                    (isBlockedUnit(fd, x - 2, z) || sideY > y + 20.0f)) {
                    kind = 6;
                }
                break;
            }
            case 5: {
                dFdBase_c::getUnitCenterPos(center, x + 2, z);
                f32 sideY = dBGCF::getUnitY(&center);
                if (info->mTreeStage > 2 &&
                    (isBlockedUnit(fd, x + 2, z) || sideY > y + 20.0f)) {
                    kind = 7;
                }
                break;
            }
            }
        } else {
            kind = 0;
        }
    }
    return getMgr()->spawn(player, 15, id, x, z, mVec3_c(0.0f, 0.0f, 0.0f), 0,
                           kind, 0);
}

// .text:0x7BC4
void startDecoAnm(int x, int z, f32 frame) {
    getMgr()->startDecoAnm(x, z, frame);
}

} // namespace dFgobjManager

// .text:0x7C18
void dFgObjMng_c::startDecoAnm(int x, int z, f32 frame) {
    if (findDecoIdx(x, z) == -1) {
        dFgDecoAnm_c *deco = getFreeDecoObj();
        if (deco != NULL) {
            deco->init(x, z, frame);
        }
    }
}

namespace dFgobjManager {

// .text:0x7C90
void endDecoAnm(int x, int z) {
    getMgr()->endDecoAnm(x, z);
}

} // namespace dFgobjManager

// .text:0x7CD4
void dFgObjMng_c::endDecoAnm(int x, int z) {
    dFgDecoAnm_c *deco = findDeco(x, z);
    if (deco == NULL || deco->isPlayingTreeAnm() == TRUE) {
        return;
    }
    deco->remove();
}

// .text:0x7D2C
int dFgObjMng_c::findDecoIdx(int x, int z) {
    dFgDecoAnm_c *deco = mDecoObjs;
    for (int i = 0; i < DECO_NUM; deco++, i++) {
        if (dFgobjManager::isDecoAt(deco, x, z) && deco->mEndCount == 0) {
            return i;
        }
    }
    return -1;
}

// .text:0x7E30
dFgDecoAnm_c *dFgObjMng_c::findDeco(int x, int z) {
    for (int i = 0; i < DECO_NUM; i++) {
        if (dFgobjManager::isDecoAt(&mDecoObjs[i], x, z) &&
            mDecoObjs[i].mEndCount == 0) {
            return &mDecoObjs[i];
        }
    }
    return NULL;
}

namespace dFgobjManager {

// .text:0x7FAC
BOOL swayPlantAt(int player, int x, int z, const mVec3_c *pos, f32 radius) {
    dFdBase_c *fd = dField::getInfo(0);
    if (fd == NULL) {
        return FALSE;
    }
    dItem::Item *item = fd->getItem(x, z, 0);
    if (item == NULL) {
        return FALSE;
    }
    mVec3_c itemPos;
    dFgDraw::getItemPosition(&itemPos, item->mId, x, z, FALSE);
    itemPos.y = dBGCF::getUnitY(&itemPos);
    f32 size;
    if (item->isFg()) {
        if (item->isRafflesia() || item->isWiltedRafflesia()) {
            size = 16.0f;
        } else if (item->isFlower() || item->isDandelion() ||
                   item->isWiltedFlower()) {
            size = 8.0f;
        } else if (item->isWeed()) {
            size = 4.0f;
        } else if (item->getFgInfo()->mTreeStage == 0 ||
                   item->isDeadSapling()) {
            size = 4.0f;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
    if (EGG::Mathf::sqrt(PSVECSquareDistance(itemPos, *pos)) >= size + radius) {
        return FALSE;
    }
    dFgObjMng_c *mgr = getMgr();
    if (mgr == NULL) {
        return FALSE;
    }
    for (int i = 0; i < dFgObjMng_c::SWING_NUM; i++) {
        dFgObjSwingPlant_c *obj = &mgr->mSwingObjs[i];
        if (obj->mType == 0x13 && isObjAt(obj, x, z)) {
            obj->mFrame = 0;
            return TRUE;
        }
    }
    return getMgr()->spawn(player, 0x13, item->mId, x, z, itemPos, 0, 0, 0);
}

} // namespace dFgobjManager

// ---------------------------------------------------------------------------------------------
// .text 0x8000 .. 0xBDC8: player / NPC field requests, pickups.

// .text:0x82F8: lets the plants on the 3 x 3 units around this console's
// (moving) player sway.
void dFgObjMng_c::swayPlantsNearPlayer() {
    int x, z, unitZ;
    int player = dNet::getMyNo();
    dAcPlayer_c *actor = dPlayerMgr::getPlayerActor(player);
    if (actor != NULL && actor->getSpeedF() >= 1.0f) {
        mVec3_c pos = actor->getPos();
        x = (int)pos.x >> 5;
        z = (int)pos.z >> 5;
        for (int dz = -1; dz <= 1; dz++) {
            unitZ = z + dz;
            for (int dx = -1; dx <= 1; dx++) {
                dFgobjManager::swayPlantAt(player, x + dx, unitZ, &pos, 16.0f);
            }
        }
    }
}

namespace dFgobjManager {

// .text:0x83C8
BOOL isSwingPlantAt(int x, int z) {
    dFgObjMng_c *mgr = (dFgObjMng_c *)fgMngProc_getFgObjMgr();
    if (mgr->mSwingNum == 0) {
        return FALSE;
    }
    for (int i = 0; i < dFgObjMng_c::SWING_NUM; i++) {
        dFgObjSwingPlant_c *obj = &mgr->mSwingObjs[i];
        if (obj->mType == 0x13 && isObjAt(obj, x, z)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x84EC: a particle that reaches water leaves a ripple and disappears.
void rippleParticleCallback(nw4r::ef::Particle *particle, u32 arg) {
    if (particle != NULL) {
        mVec3_c pos;
        if (dScene::isOutdoorScene(dScene::getCurrentScene())) {
            mVec3_c src;
            src = particle->mParameter.mPosition;
            dWorld::toFieldPosition(&pos, &src);
        } else {
            pos = particle->mParameter.mPosition;
        }
        dBGCF::groundChk_c chk(&pos, 0, 0, 0);
        if (chk.mWater != 0 && chk.isUnderWater(pos.y)) {
            pos.y = chk.getWaterY();
            dEffect::create("af_hny_hamon_S01", &pos, NULL, NULL);
            if (particle->mParticleManager != NULL) {
                particle->mParticleManager->RetireParticle(particle);
            }
        }
    }
}

// .text:0x85DC: finishes the task at the unit, unless an object still animates
// there.
void procTaskAt(u32 scene, const dFdAsPos_c *pos, u32 layer) {
    int idx = fgMngTask_find(scene, pos, layer);
    if (idx >= 0) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[idx];
        if (!killObjAt(task->getX(), task->getZ())) {
            switch (task->mKind) {
            case FG_MNG_KIND_PICK_UP_FULL:
            case FG_MNG_KIND_DIG_UP_FULL:
            case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
            case FG_MNG_KIND_PITFALL:
                task->release();
                break;
            default:
                task->proc();
                break;
            }
        }
    }
}

// .text:0x8678: finishes the tasks of a command (its unit and its drops).
void procCmdTasks(dFgMngCmd_c *cmd) {
    dFdAsPos_c pos(cmd->mPos >> 8, cmd->mPos & 0xFF);
    switch (cmd->mKind) {
    case FG_MNG_KIND_BLOCKED:
    case FG_MNG_KIND_HIT_ROCK:
    case FG_MNG_KIND_WATER:
    case FG_MNG_KIND_MISS:
        return;
    default:
        break;
    }
    procTaskAt(cmd->mScene, &pos, cmd->mLayer);
    if (cmd->mHasDrops) {
        for (int i = 0; i < 3; i++) {
            pos = dFdAsPos_c(*(u16 *)&cmd->mDropPos[i] >> 8,
                             *(u16 *)&cmd->mDropPos[i] & 0xFF);
            if (cmd->mDropPos[i].mPos != FG_MNG_POS_NONE) {
                procTaskAt(cmd->mScene, &pos, cmd->mLayer);
            }
        }
    }
}

// .text:0x8778: writes a command's result into the field data.
void applyCmdToField(dFgMngCmd_c *cmd) {
    switch (cmd->mKind) {
    case FG_MNG_KIND_BLOCKED:
    case FG_MNG_KIND_HIT_ROCK:
    case FG_MNG_KIND_WATER:
    case FG_MNG_KIND_MISS:
        break;
    default:
        dFdBase_c *fd = dField::getInfo(dField::getSceneFieldId(cmd->mScene));
        if (dScene::isSceneAttr(cmd->mScene, dScene::SCENE_ATTR_TOWN)) {
            dFgMngProc_c::setUnitItem(fd, cmd->mPos >> 8, cmd->mPos & 0xFF,
                                      cmd->mNewItem, cmd->mStage);
            if (cmd->mHasDrops) {
                u16 item = cmd->mDropItem;
                for (int i = 0; i < 3; i++) {
                    if (cmd->mDropPos[i].mPos != FG_MNG_POS_NONE) {
                        dFgMngProc_c::setUnitItem(
                            fd, *(u16 *)&cmd->mDropPos[i] >> 8,
                            *(u16 *)&cmd->mDropPos[i] & 0xFF, item, 0);
                    }
                }
            }
        } else {
            dItem::Item item(cmd->mNewItem);
            fd->setItem(&item, cmd->mPos >> 8, cmd->mPos & 0xFF, cmd->mLayer);
        }
        break;
    }
}

// .text:0x889C: a command received from the net (or made here).
void procCmd(dFgMngCmd_c *cmd) {
    if (dNet::getMyNo() == cmd->getPlayer()) {
        if (cmd->mLocal) {
            playCmdEffect(cmd);
        }
    } else if (cmd->mLocal) {
        u8 scene = cmd->mScene;
        if (scene == dScene::getCurrentScene()) {
            procRemoteCmd(cmd);
        } else {
            applyCmdToField(cmd);
            procCmdTasks(cmd);
        }
    }
}

// .text:0x8934: another player's command in this scene.
void procRemoteCmd(dFgMngCmd_c *cmd) {
    playCmdEffect(cmd);
    switch (cmd->mKind) {
    case FG_MNG_KIND_PUT:
    case FG_MNG_KIND_SWAP:
    case FG_MNG_KIND_PUT_PENDING: {
        dItem::Item item(cmd->mNewItem);
        if (item.isFlower() || item.isDandelion()) {
            spawnPickUp(cmd->mPlayer, cmd->mNewItem, cmd->mPos >> 8,
                        cmd->mPos & 0xFF);
            break;
        }
        mVec3_c pos;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&pos.x, &pos.z, &angle,
                                           cmd->mPlayer)) {
            mVec3_c playerPos = *dPlayerMgr::getPosition(cmd->mPlayer);
            mVec3_c center;
            dFdBase_c::getUnitCenterPos(&center, cmd->mPos >> 8,
                                        cmd->mPos & 0xFF);
            center.y = 0.0f;
            playerPos.y = 0.0f;
            pos.y = 0.0f;
            f32 dist = getDistXZSq(center, pos);
            f32 dist2 = getDistXZSq(center, playerPos);
            if (dist > dist2) {
                dist = dist2;
                pos = playerPos;
            }
            if (dist > 2560.0f) {
                mVec3_c dir = pos - center;
                dir.normalize();
                pos = center + dir * 1.6f * 32.0f;
            }
            pos.y = dBGCF::getUnitY(&pos);
            spawnDrop(cmd->mPlayer, cmd->mItem, cmd->mPos >> 8,
                      cmd->mPos & 0xFF, &pos, cmd->mLayer);
        } else {
            applyCmdToField(cmd);
            procCmdTasks(cmd);
        }
        break;
    }
    case FG_MNG_KIND_BURY:
        if (cmd->mNewItem != 0xFFF1) {
            dAcPlayer_c *actor = dPlayerMgr::getPlayerActor(cmd->mPlayer);
            if (actor != NULL) {
                spawnBuryMoney(cmd->mPlayer, cmd->mItem, cmd->mPos >> 8,
                               cmd->mPos & 0xFF, actor->getPosP());
            }
        }
        break;
    }
}

// .text:0x8BC8: the effect of a command on this console.
void playCmdEffect(dFgMngCmd_c *cmd) {
    switch (cmd->mKind) {
    case FG_MNG_KIND_BLOCKED:
    case FG_MNG_KIND_HIT_ROCK:
    case FG_MNG_KIND_WATER:
    case FG_MNG_KIND_MISS:
        return;
    case FG_MNG_KIND_PLANT:
        spawnPickUp(cmd->mPlayer, cmd->mNewItem, cmd->mPos >> 8,
                    cmd->mPos & 0xFF);
        return;
    case FG_MNG_KIND_PUT_DESIGN:
        spawnPickUpSpecial(cmd->mPlayer, cmd->mNewItem, cmd->mPos >> 8,
                           cmd->mPos & 0xFF);
        return;
    }
}

} // namespace dFgobjManager

namespace dFgobjManager {

// The coordinates of a unit position (by value).
inline int getUnitX(dFdAsPos_c pos) {
    return pos.mX;
}

inline int getUnitZ(dFdAsPos_c pos) {
    return pos.mZ;
}

// Packs a unit position into a command (x in the high byte, z in the low).
inline void setCmdPos(dFgMngCmd_c *cmd, dFdAsPos_c pos) {
    ((u8 *)&cmd->mPos)[0] = getUnitX(pos);
    ((u8 *)&cmd->mPos)[1] = pos.mZ;
}

// Plans the drops of a request processed offline.
inline void planReqCmd(dFgMngReq_c *req, dFgMngPlayerState_c *st) {
    dFgMngCmd_c cmd;
    setCmdPos(&cmd, st->getPos());
    cmd.plan(req, 0);
}

// Adds the task of a request processed offline.
inline BOOL addTask(const dFdAsPos_c &pos, dFdAsPos_c link, dFgMngReq_c *req,
                    s8 day) {
    return fgMngTask_add(0, dScene::getCurrentScene(), pos, link, req->mItem,
                         req->mTarget, req->mKind, req->mStage, req->mB2,
                         dFgMngTask_c::DROP_IDX_MAIN, req->mLayer, day);
}

// addTask for a player state slot's request.
inline BOOL addReqTask(dFgMngReq_c *req, dFgMngPlayerState_c *st,
                       dFdAsPos_c link) {
    return addTask(st->mPos, link, req, st->mDay);
}

// .text:0x8C30: starts a field request of the player in a free player state
// slot (-1: none free, or the pocket is full and the player already has a task
// waiting).
int requestAction(dFgMngState_c *state, int player, int kind, dFdAsPos_c pos,
                  u16 item, u16 baseItem, u8 stage, u8 layer, int b2, s8 day) {
    int ret = -1;
    for (int i = 0; i < 3; i++) {
        dFgMngPlayerState_c *st = &state->mPlayers[i];
        if (st->mState == dFgMngPlayerState_c::STATE_NONE &&
            st->mKind == FG_MNG_KIND_DROP) {
            switch (kind) {
            case FG_MNG_KIND_PICK_UP:
            case FG_MNG_KIND_DIG_UP:
            case FG_MNG_KIND_PICK_UP_SPECIAL:
                if (dItem::Item(baseItem).isFg74() == FALSE) {
                    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(
                            0) < 0) {
                        if (fgMngTask_findActive(player) >= 0) {
                            return -1;
                        }
                        kind++;
                    }
                }
                break;
            }
            st->mPlayer = player;
            st->_04 = 0;
            st->mState = dFgMngPlayerState_c::STATE_WAIT;
            st->mKind = kind;
            st->mItem = item;
            st->mBaseItem = baseItem;
            st->setPos(pos);
            st->mLayer = layer;
            st->mStage = stage;
            st->mB2 = b2;
            st->mDay = day;
            sendRequest(i, b2);
            ret = i;
            break;
        }
    }
    fgMngProc_isSelfMember(player);
    return ret;
}

// .text:0x8D8C: sends the request of a player state slot (offline: processes it
// at once).
void sendRequest(u8 idx, int b2) {
    dFdAsPos_c link;
    dFdBase_c *fd = dField::getInfo(0);
    dFgMngPlayerState_c *st = &sFgMngState.mPlayers[idx];
    if (fd == NULL) {
        st->mState = dFgMngPlayerState_c::STATE_REJECTED;
        return;
    }
    dFdAsPos_c pos = st->getPos();
    dItem::Item *item = fd->getItem(pos.mX, pos.mZ, st->mLayer);
    if (item == NULL) {
        st->mState = dFgMngPlayerState_c::STATE_REJECTED;
        return;
    }
    if (!fgMngState_canAct(idx, pos.mX, pos.mZ)) {
        st->mState = dFgMngPlayerState_c::STATE_REJECTED;
        return;
    }
    dFgMngReq_c req;
    req.mMember = st->mPlayer;
    req.mPlayer = idx;
    req.mStage = st->mStage;
    req.mKind = st->mKind;
    ((u8 *)&req.mPos)[0] = pos.mX;
    ((u8 *)&req.mPos)[1] = pos.mZ;
    req.mTarget = item->mId;
    req.mItem = st->mItem;
    req.mB2 = b2;
    req.mLayer = st->mLayer;
    switch (req.mKind) {
    case FG_MNG_KIND_SHAKE_TREE:
    case FG_MNG_KIND_SHAKE_TREE_SPECIAL:
    case FG_MNG_KIND_PICK_UP:
    case FG_MNG_KIND_PICK_UP_FULL:
    case FG_MNG_KIND_PULL:
    case FG_MNG_KIND_CHOP_TREE:
    case FG_MNG_KIND_CHOP_TREE_SPECIAL:
    case FG_MNG_KIND_DIG_HOLE:
    case FG_MNG_KIND_DIG_UP:
    case FG_MNG_KIND_DIG_UP_FULL:
    case FG_MNG_KIND_DIG_OUT:
    case FG_MNG_KIND_BLOCKED:
    case FG_MNG_KIND_HIT_ROCK:
    case FG_MNG_KIND_HIT_MONEY_ROCK:
    case FG_MNG_KIND_WATER:
    case FG_MNG_KIND_BURY:
    case FG_MNG_KIND_FILL_HOLE:
    case FG_MNG_KIND_PICK_UP_SPECIAL:
    case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
    case FG_MNG_KIND_MISS:
    case FG_MNG_KIND_PITFALL:
    case FG_MNG_KIND_BALLOON:
        req.mSend = FALSE;
        break;
    case FG_MNG_KIND_PUT:
    case FG_MNG_KIND_SWAP:
    case FG_MNG_KIND_PUT_PENDING:
    case FG_MNG_KIND_PLANT:
    case FG_MNG_KIND_PUT_DESIGN:
        req.mSend = TRUE;
        break;
    }
    switch (req.mKind) {
    case FG_MNG_KIND_PUT:
    case FG_MNG_KIND_SWAP:
    case FG_MNG_KIND_PUT_PENDING:
    case FG_MNG_KIND_BURY:
        req.mItem2 = st->mBaseItem;
        break;
    }
    if (dNet::isSessionActive()) {
        if (dNet::isHost()) {
            BOOL local =
                fgMngProc_isChangeRequestValid(&req, dScene::getCurrentScene());
            req.send(local, dNet::getMyNo(), dScene::getCurrentScene());
        } else {
            dNet::beginPacket();
            dNet::addPacket(&req, sizeof(dFgMngReq_c));
            dNet::sendPacket(NET_PACKET_FG_MNG_REQ, 0);
        }
    } else {
        st->mState = dFgMngPlayerState_c::STATE_ACCEPTED;
        if (!req.proc(st)) {
            int active = fgMngTask_findActive(0);
            if (active >= 0) {
                dFgMngTask_c *task = &sFgMngTaskList.mTasks[active];
                link = dFdAsPos_c(task->getX(), task->getZ());
            } else {
                link = dFdAsPos_c(-1, -1);
            }
            if (!addReqTask(&req, st, link)) {
                st->mState = dFgMngPlayerState_c::STATE_REJECTED;
                return;
            }
            planReqCmd(&req, st);
        }
    }
}

// .text:0x90C4: the player uses a tool on the unit.
int useTool(int player, int x, int z, int tool, int b2, u16 heldItem) {
    dFdBase_c *fd;
    int ret = -1;
    fd = dField::getInfo(0);
    if (fd != NULL) {
        dItem::Item *item = fd->getItem(x, z, 0);
        if (item != NULL) {
            dFdAsPos_c pos(x, z);
            switch (tool) {
            case 0:
                ret = shakeTree(player, item, &pos, b2);
                break;
            case 1:
                ret = chopTree(player, item, &pos, b2);
                break;
            case 2:
                ret = dig(player, fd, item, pos, heldItem, b2);
                break;
            case 4:
                ret = water(player, item, &pos, b2);
                break;
            }
        }
    }
    return ret;
}

// .text:0x91E8: the player uses a tool on the unit of its active task.
int useToolAtTask(int player, int tool, int b2, u16 heldItem) {
    int ret = -1;
    if (!fgMngProc_isSelfMember(player)) {
        return -1;
    }
    u16 *pos = fgMngTask_getActive(player);
    if (pos != NULL) {
        ret = useTool(player, *pos >> 8, *pos & 0xFF, tool, b2, heldItem);
    }
    return ret;
}

// Field object kinds by id (u16 forms of the dItem::Item range tests).
inline bool isStoneFg(u16 id) {
    return id >= dItem::FG_STONE_A && id <= dItem::FG_STONE_E;
}

inline bool isExtIdFg(u16 id) {
    return id >= 0xD000 && id < 0xD045;
}

inline bool isCedarFg(u16 id) {
    return id >= dItem::FG_CEDAR_SAPLING && id <= dItem::FG_CEDAR_LIGHTS;
}
inline bool isMoneyTreeFg(u16 id) {
    return id >= dItem::FG_MONEY_TREE_SAPLING && id <= dItem::FG_MONEY_TREE;
}
inline bool isAppleTreeFg(u16 id) {
    return id >= dItem::FG_APPLE_TREE_SAPLING &&
           id <= dItem::FG_APPLE_TREE_FRUIT;
}
inline bool isOrangeTreeFg(u16 id) {
    return id >= dItem::FG_ORANGE_TREE_SAPLING &&
           id <= dItem::FG_ORANGE_TREE_FRUIT;
}
inline bool isPearTreeFg(u16 id) {
    return id >= dItem::FG_PEAR_TREE_SAPLING && id <= dItem::FG_PEAR_TREE_FRUIT;
}
inline bool isPeachTreeFg(u16 id) {
    return id >= dItem::FG_PEACH_TREE_SAPLING &&
           id <= dItem::FG_PEACH_TREE_FRUIT;
}
inline bool isCherryTreeFg(u16 id) {
    return id >= dItem::FG_CHERRY_TREE_SAPLING &&
           id <= dItem::FG_CHERRY_TREE_FRUIT;
}
inline bool isPalmTreeFg(u16 id) {
    return id >= dItem::FG_PALM_SAPLING && id <= dItem::FG_PALM_FRUIT;
}

// .text:0x9274: shaking a tree (tool type 0).
int shakeTree(int player, const dItem::Item *item, const dFdAsPos_c *pos,
              int b2) {
    int ret = -1;
    dItem::FgInfo *info = item->getFgInfo();
    if (info != NULL) {
        if (info->mTreeStage > 1) {
            int stage = info->mTreeStage;
            dItem::Item fg = *item;
            int kind = FG_MNG_KIND_SHAKE_TREE;
            u16 newItem;
            if (fg.isCedarTree()) {
                newItem = dItem::FG_CEDAR_SAPLING + stage;
            } else {
                newItem = dItem::FG_TREE_SAPLING + stage;
            }
            if (fg.mId == dItem::FG_TREE_BEES ||
                fg.mId == dItem::FG_CEDAR_BEES) {
                if (fgMngProc_canEditField()) {
                    kind = FG_MNG_KIND_SHAKE_TREE_SPECIAL;
                } else {
                    newItem = item->mId;
                }
            } else if (fg.isPeachTree() || fg.isAppleTree() ||
                       fg.isOrangeTree() || fg.isPearTree() ||
                       fg.isCherryTree() || fg.isPalmTree()) {
                if ((u16)(int)info->mDropItem != dItem::ITEM_ID_NONE) {
                    newItem = fg.mId - 3;
                } else {
                    newItem = fg.mId;
                }
            } else if (fg.isMoneyTree()) {
                if ((u16)(int)info->mDropItem == dItem::ITEM_ID_NONE) {
                    newItem = fg.mId;
                }
            } else if (fg.mId == dItem::FG_CEDAR_LIGHTS) {
                newItem = fg.mId;
            }
            ret = requestAction(&sFgMngState, player, kind, *pos, newItem,
                                item->mId, 0, 0, b2, -1);
        }
    }
    return ret;
}

// .text:0x94AC: chopping with the axe (tool type 1). The unit flag counts the
// hits.
int chopTree(int player, const dItem::Item *item, const dFdAsPos_c *pos,
             int b2) {
    static const int sChopHits[] = {0, 1, 2, 2};
    dItem::FgInfo *info;
    u16 id;
    int kind = 0;
    u16 newItem;
    u8 hits;
    int ret;
    dFdUnitAttr_c *attr = dField::getInfo(1)->getUnitAttr();
    id = item->mId;
    hits = 0;
    ret = -1;
    info = item->getFgInfo();
    newItem = item->getId();
    if (info != NULL && info->mTreeStage > 0) {
        int stage = info->mTreeStage - 1;
        kind = FG_MNG_KIND_CHOP_TREE;
        hits = sFgMngState.getUnitFlag(pos);
        if (b2 != 2) {
            if (hits >= sChopHits[stage]) {
                newItem = (int)info->mStump;
            } else {
                if (isCedarFg(item->getId())) {
                    newItem = dItem::FG_CEDAR_S1 + stage;
                } else {
                    newItem = dItem::FG_TREE_S1 + stage;
                }
                if (id == dItem::FG_TREE_BEES || id == dItem::FG_CEDAR_BEES) {
                    if (fgMngProc_canEditField()) {
                        kind = FG_MNG_KIND_CHOP_TREE_SPECIAL;
                    } else {
                        newItem = id;
                    }
                } else if (isMoneyTreeFg(item->getId())) {
                    if ((u16)(int)info->mDropItem == dItem::ITEM_ID_NONE) {
                        newItem = item->getId();
                    }
                } else if (isPeachTreeFg(item->getId()) ||
                           isAppleTreeFg(item->getId()) ||
                           isOrangeTreeFg(item->getId()) ||
                           isPearTreeFg(item->getId()) ||
                           isCherryTreeFg(item->getId()) ||
                           isPalmTreeFg(item->getId())) {
                    if ((u16)(int)info->mDropItem != dItem::ITEM_ID_NONE) {
                        newItem = item->getId() - 3;
                    } else {
                        newItem = item->getId();
                    }
                } else if (id == dItem::FG_CEDAR_LIGHTS) {
                    newItem = item->getId();
                }
            }
        }
    } else if (isStoneFg(newItem) || isExtIdFg(newItem) ||
               (attr != NULL &&
                (attr->getAttr(pos->mX, pos->mZ) & dFdUnitAttr_c::STR_COL))) {
        kind = FG_MNG_KIND_HIT_ROCK;
    } else if (item->isMoneyRock()) {
        kind = FG_MNG_KIND_HIT_ROCK;
        if (!dNet::isSessionActive()) {
            int owner = (u32)(item->mId - dItem::FG_MONEY_ROCK_FIRST) / 5;
            if (owner == dPlayerMgr::getCurrentPlayerSlot() && b2 != 2) {
                kind = FG_MNG_KIND_HIT_MONEY_ROCK;
            }
        }
    } else if (item->mId < dItem::FG_COUNT) {
        kind = FG_MNG_KIND_MISS;
    } else if (dItem::isRealItemId(item->mId) || item->hasFtrFunc()) {
        newItem = item->mId;
        kind = FG_MNG_KIND_BLOCKED;
    } else if (item->mId == dItem::ITEM_ID_NONE) {
        kind = FG_MNG_KIND_MISS;
    }
    if (kind != 0) {
        ret = requestAction(&sFgMngState, player, kind, *pos, newItem,
                            item->mId, hits, 0, b2, -1);
    }
    return ret;
}

// .text:0x9880: digging with the shovel (tool type 2); heldItem is buried in a
// hole.
int dig(int player, dFdBase_c *fd, const dItem::Item *item, dFdAsPos_c pos,
        u16 heldItem, int b2) {
    int kind = FG_MNG_KIND_MISS;
    int ret = -1;
    dItem::FgInfo *info = item->getFgInfo();
    dItem::Item fg = *item;
    u16 newItem = dItem::FG_HOLE;
    u8 stage = 0;
    u16 baseItem = fg.mId;
    int x;
    BOOL snowman = FALSE;
    for (int i = 0; i < daSnowMan_c::BALL_NUM; i++) {
        daSnowMan_c *ball = daSnowMan_c::getBall(i);
        if (ball != NULL) {
            f32 ballZ = ball->getPos().z;
            x = (int)ball->getPos().x >> 5;
            int z = (int)ballZ >> 5;
            if (x == pos.mX && z == pos.mZ) {
                snowman = TRUE;
                break;
            }
        }
    }
    if (snowman) {
        newItem = fg.mId;
        kind = FG_MNG_KIND_BLOCKED;
    } else if (fg.mId == dItem::ITEM_ID_NONE) {
        switch (fd->getDigType(pos.mX, pos.mZ)) {
        case 0:
        case 1:
            kind = FG_MNG_KIND_DIG_HOLE;
            break;
        case 2:
            newItem = fg.mId;
            kind = FG_MNG_KIND_BLOCKED;
            break;
        case 3:
            newItem = fg.mId;
            kind = FG_MNG_KIND_MISS;
            break;
        }
    } else if (fg.isStone() || fg.isExtId()) {
        newItem = fg.mId;
        kind = FG_MNG_KIND_HIT_ROCK;
    } else if (fg.isMoneyRock()) {
        newItem = fg.mId;
        kind = FG_MNG_KIND_HIT_ROCK;
        if (!dNet::isSessionActive()) {
            int owner = (u32)(item->mId - dItem::FG_MONEY_ROCK_FIRST) / 5;
            if (owner == dPlayerMgr::getCurrentPlayerSlot()) {
                kind = FG_MNG_KIND_HIT_MONEY_ROCK;
            }
        }
    } else if (fg.isHole()) {
        newItem = heldItem;
        kind = FG_MNG_KIND_BURY;
        baseItem = heldItem;
        if (heldItem != dItem::ITEM_ID_NONE) {
            fgMngProc_getBuryFg(player, &newItem, &baseItem, &stage, heldItem);
        }
    } else if (fg.isStump()) {
        kind = FG_MNG_KIND_DIG_OUT;
    } else if (isBlockedUnit(fd, pos.mX, pos.mZ) ||
               fg.mId == dItem::FG_RAFFLESIA ||
               fg.mId == dItem::FG_WILTED_RAFFLESIA) {
        newItem = item->mId;
        kind = FG_MNG_KIND_BLOCKED;
    } else if (item->isDeadSapling() || fg.mId == dItem::FG_RED_TURNIP_WILTED ||
               fg.mId == dItem::FG_RED_TURNIP_WILTED_WATERED ||
               (info != NULL && info->mTreeStage == 0)) {
        kind = FG_MNG_KIND_DIG_OUT;
    } else if (item->isWeed() || (fg.mId >= dItem::FG_FLOWER_FIRST &&
                                  fg.mId <= dItem::FG_LUCKY_CLOVER)) {
        kind = FG_MNG_KIND_DIG_HOLE;
    } else if (fd->isBuried(pos.mX, pos.mZ)) {
        kind = FG_MNG_KIND_DIG_UP;
    } else if (item->isRedTurnip()) {
        kind = FG_MNG_KIND_DIG_UP;
        baseItem = dItem::Item(dItem::ITEM_IDX_RED_TURNIP_00,
                               fg.mId - dItem::FG_RED_TURNIP_0, FALSE)
                       .mId;
    } else if (dItem::isRealItemId(item->getId()) || item->hasFtrFunc()) {
        newItem = item->mId;
        kind = FG_MNG_KIND_BLOCKED;
    }
    if (kind != 0) {
        ret = requestAction(&sFgMngState, player, kind, pos, newItem, baseItem,
                            stage, 0, b2, -1);
    }
    return ret;
}

// .text:0x9CE8: watering (tool type 4).
int water(int player, const dItem::Item *item, const dFdAsPos_c *pos, int b2) {
    int ret = -1;
    if (fgMngProc_isSelfMember(player)) {
        ret = requestAction(&sFgMngState, player, FG_MNG_KIND_WATER, *pos,
                            item->getId(), item->getId(), 0, 0, b2, -1);
    }
    return ret;
}

// .text:0x9D80: picking up what lies on the unit.
int pickUp(int player, int x, int z) {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_TOWN)) {
        return pickUpTown(player, x, z);
    }
    return pickUpRoom(player, x, z);
}

// .text:0x9DFC
int pickUpTown(int player, int x, int z) {
    u16 id;
    int ret;
    dItem::Item *item;
    dFdBase_c *fd = dField::getInfo(1);
    ret = -1;
    item = fd->getItem(x, z, 0);
    if (item != NULL) {
        id = item->mId;
        int kind = 0;
        const dItem::BITM *bitm =
            dItem::infoBank_c::get()->getBITM(dItem::Item(id));
        if (id == dItem::ITEM_ID_NONE) {
            ret = -1;
        } else if (bitm != NULL && bitm->getKind() == dItem::KIND_MUSH_FTR) {
            kind = FG_MNG_KIND_PICK_UP_SPECIAL;
        } else if (item->isWeed() || item->getId() == dItem::FG_CLOVER) {
            kind = FG_MNG_KIND_PULL;
        } else if ((dItem::isRealItemId(item->getId()) &&
                    !fd->isBuried(x, z)) ||
                   item->isFlower() || id == dItem::FG_DANDELION ||
                   id == dItem::FG_DANDELION_PUFF ||
                   (item->hasFtrFunc() && !fd->isBuried(x, z)) ||
                   item->isDesignP0() || item->isDesignP1() ||
                   item->isDesignP2() || item->isDesignP3()) {
            kind = FG_MNG_KIND_PICK_UP;
        } else if (item->isWiltedFlower() && id != dItem::FG_WILTED_RAFFLESIA) {
            kind = FG_MNG_KIND_PULL;
        } else if (id == dItem::FG_LUCKY_CLOVER) {
            kind = FG_MNG_KIND_PICK_UP_SPECIAL;
        }
        if (kind != 0) {
            dFdAsPos_c pos(x, z);
            ret = requestAction(&sFgMngState, player, kind, pos,
                                dItem::ITEM_ID_NONE, id, 0, 0, 0, -1);
        }
    }
    return ret;
}

// .text:0xA088: picking up an item indoors (from the floor, else from the
// furniture top layer).
int pickUpRoom(int player, int x, int z) {
    dFdBase_c *fd = dField::getInfo(0);
    int ret = -1;
    if (fd != NULL) {
        dItem::Item *item = fd->getItem(x, z, 0);
        if (item != NULL) {
            int kind = 0;
            u16 baseItem;
            u8 layer;
            if (dItem::isRealItemId(item->mId) && !item->hasFtrFunc()) {
                baseItem = item->mId;
                kind = FG_MNG_KIND_PICK_UP;
                layer = 0;
            } else {
                dItem::Item *top = fd->getItem(x, z, 1);
                if (top != NULL && dItem::isRealItemId(top->mId) &&
                    !top->hasFtrFunc()) {
                    baseItem = top->mId;
                    kind = FG_MNG_KIND_PICK_UP;
                    layer = 1;
                }
            }
            if (kind != 0) {
                dFdAsPos_c pos(x, z);
                ret = requestAction(&sFgMngState, player, kind, pos,
                                    dItem::ITEM_ID_NONE, baseItem, 0, layer, 0,
                                    -1);
            }
        }
    }
    return ret;
}

// .text:0xA204: filling a hole.
int fillHole(int player, int x, int z, int b2) {
    dFdBase_c *fd = dField::getInfo(0);
    int ret = -1;
    if (fd != NULL) {
        dItem::Item *item = fd->getItem(x, z, 0);
        if (item != NULL) {
            u16 newItem = dItem::ITEM_ID_NONE;
            int kind = 0;
            if (item->isHole() && b2 != 2) {
                newItem = dItem::ITEM_ID_NONE;
                kind = FG_MNG_KIND_FILL_HOLE;
            }
            if (kind != 0) {
                dFdAsPos_c pos(x, z);
                ret = requestAction(&sFgMngState, player, kind, pos, newItem,
                                    newItem, 1, 0, 0, -1);
            }
        }
    }
    return ret;
}

} // namespace dFgobjManager

namespace dFgobjManager {

// .text:0xA300: the player digs up a buried pitfall seed (it becomes a hole).
int requestPitfall(int player, int x, int z) {
    BOOL pitfall;
    u16 id;
    dFdBase_c *fd;
    int ret;
    if (!dScene::isSceneAttr(dScene::getCurrentScene(),
                             dScene::SCENE_ATTR_TOWN)) {
        return -1;
    }
    fd = dField::getInfo(0);
    ret = -1;
    if (fd != NULL) {
        dItem::Item *item = fd->getItem(x, z, 0);
        if (item != NULL) {
            id = item->mId;
            dItem::Item seed(dItem::ITEM_IDX_PITFALL_SEED);
            pitfall = FALSE;
            if (id == seed.mId) {
                if (fd->isBuried(x, z)) {
                    pitfall = TRUE;
                }
            }
            if (pitfall) {
                dFdAsPos_c pos(x, z);
                ret = requestAction(&sFgMngState, player, FG_MNG_KIND_PITFALL,
                                    pos, dItem::FG_HOLE, dItem::ITEM_ID_NONE, 0,
                                    0, 0, -1);
            }
        }
    }
    return ret;
}

// .text:0xA424: an NPC falls into a pitfall (d_a_npc_out); the request is made
// for player 0 and carries the NPC in its day field. Returns the player state
// slot, -1 if none.
int requestNpcPitfall(int npc, int x, int z) {
    BOOL pitfall;
    u16 id;
    dFdBase_c *fd;
    int ret;
    if (!dScene::isSceneAttr(dScene::getCurrentScene(),
                             dScene::SCENE_ATTR_TOWN)) {
        return -1;
    }
    fd = dField::getInfo(0);
    ret = -1;
    if (fd != NULL) {
        dItem::Item *item = fd->getItem(x, z, 0);
        if (item != NULL) {
            id = item->mId;
            dItem::Item seed(dItem::ITEM_IDX_PITFALL_SEED);
            pitfall = FALSE;
            if (id == seed.mId) {
                if (fd->isBuried(x, z)) {
                    pitfall = TRUE;
                }
            }
            if (pitfall) {
                dFdAsPos_c pos(x, z);
                ret = requestAction(&sFgMngState, 0, FG_MNG_KIND_PITFALL, pos,
                                    dItem::FG_HOLE, dItem::ITEM_ID_NONE, 0, 0,
                                    0, npc);
            }
        }
    }
    return ret;
}

// The item on a unit position (by value).
inline dItem::Item *getUnitItem(dFdBase_c *fd, dFdAsPos_c pos, int layer) {
    return fd->getItem(pos.mX, pos.mZ, layer);
}

// A player state slot.
inline dFgMngPlayerState_c *getPlayerState(int idx) {
    return &sFgMngState.mPlayers[idx];
}

// The field of the current scene.
inline dFdBase_c *getFieldInfo() {
    return dField::getInfo(0);
}

// Copies a player state slot's unit position into *out.
inline const dFdAsPos_c *getRequestPosition(dFgMngPlayerState_c *st,
                                            dFdAsPos_c *out) {
    *out = st->mPos;
    return out;
}

// The z of a player state slot's unit position.
inline void getRequestZ(int &z, dFgMngPlayerState_c *st) {
    z = st->getPos().mZ;
}

// .text:0xA548: the result of a player state slot's request: 0 waiting, 1
// accepted (its effect is played here), 2 rejected or nothing pending.
int getRequestResult(int idx) {
    dFdAsPos_c pickupPos;
    int ret;
    int z;
    dFdBase_c *fd;
    dFgMngPlayerState_c *st;
    fd = getFieldInfo();
    st = getPlayerState(idx);
    ret = 0;
    if (st->mState == dFgMngPlayerState_c::STATE_NONE ||
        st->mKind == FG_MNG_KIND_DROP ||
        (dNet::isSessionActive() && st->getPlayer() != dNet::getMyNo())) {
        return 2;
    }
    if (fd == NULL) {
        return 0;
    }
    getRequestZ(z, st);
    dFdAsPos_c pos = st->mPos;
    dItem::Item *item = getUnitItem(fd, pos, 0);
    if (item == NULL) {
        return 0;
    }
    switch (st->mState) {
    case dFgMngPlayerState_c::STATE_ACCEPTED:
        switch (st->mKind) {
        case FG_MNG_KIND_BURY:
            if (st->mItem != dItem::ITEM_ID_NONE) {
                dAcPlayer_c *actor = dPlayerMgr::getPlayerActor(4);
                if (actor != NULL) {
                    const mVec3_c *actorPos = actor->getPosP();
                    u16 base = st->mBaseItem;
                    dItem::Item planted(st->mItem);
                    dItem::FgInfo *info = planted.getFgInfo();
                    if (info != NULL && info->mTreeStage == 0) {
                        int type = planted.getFruitTreeType();
                        if (type < 7) {
                            base = dItem::Item(sFruit[type]).mId;
                        }
                    }
                    spawnBuryMoney(st->mPlayer, base, pos.mX, z, actorPos);
                }
            }
            ret = 1;
            break;
        case FG_MNG_KIND_PICK_UP:
        case FG_MNG_KIND_PICK_UP_SPECIAL:
            if (item->mId == dItem::ITEM_ID_NONE) {
                const dFdAsPos_c *pickup = getRequestPosition(st, &pickupPos);
                fd->setItem(dItem::Item(st->mBaseItem), pickup->mX, pickup->mZ,
                            st->mLayer);
                ret = 1;
            } else {
                ret = 1;
            }
            break;
        case FG_MNG_KIND_PLANT:
            spawnPickUp(st->mPlayer, st->mItem, pos.mX, z);
            ret = 1;
            break;
        case FG_MNG_KIND_PUT_DESIGN:
            spawnPickUpSpecial(st->mPlayer, st->mItem, pos.mX, z);
            ret = 1;
            break;
        default:
            ret = 1;
            break;
        }
        break;
    case dFgMngPlayerState_c::STATE_REJECTED:
        ret = 2;
        break;
    }
    return ret;
}

// .text:0xA788: puts an item down on the unit in front of the player (kind PUT
// / SWAP / PLANT ...).
int putItem(int player, int kind, u16 item) {
    int ret = -1;
    dFdAsPos_c pos;
    u8 layer;
    if (findPutSpot(kind, &pos.mX, &pos.mZ, &layer)) {
        u16 newItem;
        u16 baseItem;
        switch (kind) {
        case FG_MNG_KIND_PLANT:
            if (!fgMngProc_getPlantedFg(&newItem, &baseItem, item, player)) {
                return -1;
            }
            break;
        default:
            newItem = item;
            baseItem = item;
            break;
        }
        ret = requestAction(&sFgMngState, player, kind, pos, newItem, baseItem,
                            0, layer, 0, -1);
    }
    return ret;
}

// .text:0xA878: puts the held item down (a design item is laid out as a
// design).
int putHeldItem(int player, u16 item) {
    return putItem(player,
                   dItem::Item(item).isFg74() ? FG_MNG_KIND_PUT_DESIGN
                                              : FG_MNG_KIND_PUT,
                   item);
}

// .text:0xA8D4: puts the held item down at the unit of the active (pocket-full)
// task.
int swapItem(int player, u16 item) {
    int ret = -1;
    if (!fgMngProc_isSelfMember(player)) {
        return -1;
    }
    u16 *task = fgMngTask_getActive(player);
    if (task != NULL) {
        dFdAsPos_c pos(*task >> 8, *task & 0xFF);
        ret = requestAction(&sFgMngState, player, FG_MNG_KIND_SWAP, pos, item,
                            item, 0, 0, 0, -1);
    }
    return ret;
}

// .text:0xA994: puts an item down; elsewhere when the player has an active
// task.
int putItemAside(int player, u16 item) {
    if (!fgMngProc_isSelfMember(player)) {
        return -1;
    }
    u16 *task = fgMngTask_getActive(player);
    return putItem(
        player, task != NULL ? FG_MNG_KIND_PUT_PENDING : FG_MNG_KIND_PUT, item);
}

// .text:0xAA00: lays out one of the current player's designs (d_menu_desi00).
int putDesign(int player, int slot) {
    return putItem(player, FG_MNG_KIND_PUT_DESIGN,
                   dItem::FG_DESIGN_P0_0 +
                       dPlayerMgr::getCurrentPlayerSlot() * 8 + slot);
}

// Position forms of the spawn functions.
inline BOOL spawnPickUp(int player, u16 item, dFdAsPos_c pos) {
    int x = getUnitX(pos);
    int z = getUnitZ(pos);
    return spawnPickUp(player, item, x, z);
}

inline BOOL spawnPickUpSpecial(int player, u16 item, dFdAsPos_c pos) {
    int x = getUnitX(pos);
    int z = getUnitZ(pos);
    return spawnPickUpSpecial(player, item, x, z);
}

inline BOOL spawnDrop(int player, u16 item, dFdAsPos_c pos, const mVec3_c *from,
                      u8 layer) {
    int x = getUnitX(pos);
    int z = getUnitZ(pos);
    return spawnDrop(player, item, x, z, from, layer);
}

// .text:0xAA54: plays the effect of an accepted put request: 1 played, 2
// rejected, 0 otherwise.
int playPutEffect(int idx) {
    dFdBase_c *fd = dField::getInfo(0);
    dFgMngPlayerState_c *st = &sFgMngState.mPlayers[idx];
    int ret = 0;
    if (fd == NULL) {
        return 0;
    }
    switch (st->mState) {
    case dFgMngPlayerState_c::STATE_ACCEPTED: {
        dAcPlayer_c *actor = dPlayerMgr::getPlayerActor(4);
        if (fd != NULL && actor != NULL) {
            const mVec3_c *actorPos = actor->getPosP();
            if (dScene::isSceneAttr(dScene::getCurrentScene(),
                                    dScene::SCENE_ATTR_TOWN)) {
                dItem::Item item(st->mItem);
                if (item.isFg74()) {
                    if (spawnPickUpSpecial(st->mPlayer, st->mItem,
                                           st->getPos())) {
                        ret = 1;
                    }
                } else if (item.isFlower() || item.isDandelion() ||
                           item.isSame((u16)dItem::FG_LUCKY_CLOVER)) {
                    if (spawnPickUp(st->mPlayer, st->mItem, st->getPos())) {
                        ret = 1;
                    }
                } else if (dItem::infoBank_c::get()->getBITM(item)->getKind() ==
                           dItem::KIND_MUSH_FTR) {
                    if (spawnDrop(st->mPlayer, item.mId, st->getPos(), actorPos,
                                  0)) {
                        ret = 1;
                    }
                } else {
                    if (spawnDrop(st->mPlayer, st->mItem, st->getPos(),
                                  actorPos, 0)) {
                        ret = 1;
                    }
                }
            } else {
                if (spawnDrop(st->mPlayer, st->mItem, st->getPos(), actorPos,
                              st->mLayer)) {
                    ret = 1;
                }
            }
        }
        break;
    }
    case dFgMngPlayerState_c::STATE_REJECTED:
        ret = 2;
        break;
    }
    return ret;
}

// .text:0xACF8: frees a player state slot (d_a_npc_out after getRequestResult).
void clearRequest(int idx) {
    if (idx >= 0) {
        sFgMngState.clearPlayer(idx);
    }
}

// .text:0xAD14: the item on a unit an item of the given kind can be put on
// (empty and allowed), NULL if none.
dItem::Item *getPutSpotItem(int kind, dFdBase_c *fd, int x, int z,
                            const mVec3_c *playerPos) {
    dItem::Item *item = fd->getItem(x, z, 0);
    if (item != NULL) {
        if (item->mId != dItem::ITEM_ID_NONE) {
            item = NULL;
        } else {
            switch (kind) {
            case FG_MNG_KIND_PUT:
            case FG_MNG_KIND_SWAP:
            case FG_MNG_KIND_PUT_PENDING:
            case FG_MNG_KIND_BALLOON:
                if (!fd->bgCall_80073D2C((void *)playerPos, x, z, 0)) {
                    item = NULL;
                }
                break;
            case FG_MNG_KIND_PLANT: {
                mVec3_c pos = *playerPos;
                int quarter = dBGCF::getUnitQuarter(&pos);
                f32 posZ = pos.z;
                if (!dBGCF::checkWalkable((int)pos.x >> 5, (int)posZ >> 5,
                                          quarter, x, z)) {
                    item = NULL;
                } else {
                    switch (fd->getPlantType(x, z)) {
                    case 1:
                    case 2:
                        break;
                    default:
                        item = NULL;
                        break;
                    }
                }
                break;
            }
            case FG_MNG_KIND_PUT_DESIGN:
                if (!fd->bgCall_80073D2C((void *)playerPos, x, z, 1)) {
                    item = NULL;
                }
                break;
            }
        }
    }
    return item;
}

// The unit and its 8 neighbours ((dx + 8) << 4 | (dz + 8)).
static const u8 sUnitOffset[] = {0x88, 0x87, 0x77, 0x97, 0x78,
                                 0x98, 0x79, 0x99, 0x89};

// .text:0xAEA4: finds the unit around the player to put an item of the given
// kind on, in the order of sPutOrder for the player's direction (indoors, the
// top of furniture in front comes first).
BOOL findPutSpot(int kind, int *outX, int *outZ, u8 *outLayer) {
    // The sUnitOffset entries to try, per player direction
    // (dFgMngProc_c::getDirection).
    static const int sPutOrder[8][9] = {
        {0, 8, 6, 7, 4, 5, 2, 3, 1}, {0, 7, 8, 5, 6, 3, 4, 1, 2},
        {0, 5, 7, 3, 8, 1, 6, 2, 4}, {0, 3, 5, 1, 7, 2, 8, 4, 6},
        {0, 1, 3, 2, 5, 4, 7, 6, 8}, {0, 2, 1, 4, 3, 6, 5, 8, 7},
        {0, 4, 2, 6, 1, 8, 3, 7, 5}, {0, 6, 8, 4, 7, 2, 5, 1, 3},
    };
    dFdBase_c *fd;
    const mVec3_c *pos;
    BOOL ret;
    const int *order;
    fd = dField::getInfo(0);
    dAcPlayer_c *actor = dPlayerMgr::getPlayerActor(4);
    ret = FALSE;
    if (fd != NULL && actor != NULL) {
        pos = actor->getPosP();
        order =
            sPutOrder[dFgMngProc_c::getDirection(actor->getAngle().y.mAngle)];
        *outX = (int)pos->x >> 5;
        *outZ = (int)pos->z >> 5;
        dFdAsPos_c unit;
        if (!dScene::isSceneAttr(dScene::getCurrentScene(),
                                 dScene::SCENE_ATTR_TOWN)) {
            s32 dx = (sUnitOffset[order[1]] >> 4) - 8;
            s32 dz = (sUnitOffset[order[1]] & 0xF) - 8;
            unit.mX = *outX + dx;
            unit.mZ = *outZ + dz;
            if (dFTR::isEmptyOnTop(unit.mX, unit.mZ)) {
                *outX = unit.mX;
                *outZ = unit.mZ;
                if (outLayer != NULL) {
                    *outLayer = 1;
                }
                dItem::Item none((u16)0xFFFF);
                fd->setItem(&none, unit.mX, unit.mZ, 1);
                return TRUE;
            }
        }
        for (int i = 0; i < 9; i++) {
            s32 dx = (sUnitOffset[order[i]] >> 4) - 8;
            s32 dz = (sUnitOffset[order[i]] & 0xF) - 8;
            unit.mX = *outX + dx;
            unit.mZ = *outZ + dz;
            dItem::Item *item = getPutSpotItem(kind, fd, unit.mX, unit.mZ, pos);
            if (item != NULL && item->mId == dItem::ITEM_ID_NONE &&
                fgMngLock_find(dScene::getCurrentScene(), &unit, 0) < 0 &&
                fgMngTask_find(dScene::getCurrentScene(), &unit, 0) < 0 &&
                fgMngState_isFreeOfPlayers(unit.mX, unit.mZ, dNet::getMyNo()) &&
                fgMngState_isFreeOfNpcs(unit.mX, unit.mZ)) {
                *outX = unit.mX;
                *outZ = unit.mZ;
                if (outLayer != NULL) {
                    *outLayer = 0;
                }
                ret = TRUE;
                break;
            }
            if (!dScene::isSceneAttr(dScene::getCurrentScene(),
                                     dScene::SCENE_ATTR_TOWN) &&
                dFTR::isEmptyOnTop(unit.mX, unit.mZ)) {
                *outX = unit.mX;
                *outZ = unit.mZ;
                if (outLayer != NULL) {
                    *outLayer = 1;
                }
                dItem::Item none((u16)0xFFFF);
                fd->setItem(&none, unit.mX, unit.mZ, 1);
                ret = TRUE;
                break;
            }
        }
    }
    return ret;
}

} // namespace dFgobjManager

namespace dFgobjManager {

// Whether an item can't be put on the unit at pos because it lies in water.
inline BOOL isWaterSpot(dFdBase_c *fd, const mVec3_c *pos) {
    BOOL water = FALSE;
    if (!fd->canPutItem(pos)) {
        dBGCF::groundChk_c chk(pos, 0, 0, 0);
        if (chk.mWater != 0) {
            water = TRUE;
        }
    }
    return water;
}

// .text:0xB178: finds where an item dropped at pos lands (d_a_balloon): TRUE
// with a free unit around it; FALSE (with *x / *z set to a fallback unit, if
// any) otherwise.
BOOL findDropSpot(const mVec3_c *pos, int *x, int *z) {
    static const int sDropOrder[] = {0, 4, 5, 2, 6, 3, 7, 1, 8};
    int x0;
    int z0;
    dFdBase_c *fd;
    f32 posZ = pos->z;
    *x = (int)pos->x >> 5;
    *z = (int)posZ >> 5;
    mVec3_c ground;
    dFdBase_c::getUnitGroundPos(&ground, *x, *z);
    moveOffWater(&ground, FALSE);
    fd = dField::getInfo(0);
    if (isWaterSpot(fd, &ground)) {
        return FALSE;
    }
    z0 = *z;
    x0 = *x;
    dFdAsPos_c unit;
    for (int i = 0; i < 9; i++) {
        s32 dx = (sUnitOffset[sDropOrder[i]] >> 4) - 8;
        s32 dz = (sUnitOffset[sDropOrder[i]] & 0xF) - 8;
        unit.mX = x0 + dx;
        unit.mZ = z0 + dz;
        dItem::Item *item =
            getPutSpotItem(FG_MNG_KIND_BALLOON, fd, unit.mX, unit.mZ, &ground);
        if (item != NULL && item->mId == dItem::ITEM_ID_NONE &&
            fgMngTask_findMarked(dScene::getCurrentScene(), &unit, 0) < 0 &&
            fgMngState_isFreeOfPlayers(unit.mX, unit.mZ, dNet::getMyNo())) {
            *x = unit.mX;
            *z = unit.mZ;
            return TRUE;
        }
    }
    for (int i = 0; i < 9; i++) {
        s32 dx = (sUnitOffset[sDropOrder[i]] >> 4) - 8;
        s32 dz = (sUnitOffset[sDropOrder[i]] & 0xF) - 8;
        unit.mX = x0 + dx;
        unit.mZ = z0 + dz;
        dBGCF::groundChk_c chk(unit.mX, unit.mZ, 0, 0, 0);
        if (chk.mWater != 0) {
            *x = unit.mX;
            *z = unit.mZ;
            return FALSE;
        }
    }
    dItem::Item *item = fd->getItem(*x, *z, 0);
    if (item != NULL) {
        dItem::FgInfo *info = item->getFgInfo();
        if (info != NULL && (s8)info->_08[0] > 0) {
            for (int i = 0; i < 9; i++) {
                s32 dx = (sUnitOffset[sDropOrder[i]] >> 4) - 8;
                s32 dz = (sUnitOffset[sDropOrder[i]] & 0xF) - 8;
                unit.mX = x0 + dx;
                unit.mZ = z0 + dz;
                dItem::Item *other = fd->getItem(unit.mX, unit.mZ, 0);
                if (other != NULL) {
                    dItem::FgInfo *otherInfo = other->getFgInfo();
                    if (otherInfo == NULL || (s8)otherInfo->_08[0] == 0) {
                        *x = unit.mX;
                        *z = unit.mZ;
                        return FALSE;
                    }
                }
            }
        }
    }
    return FALSE;
}

} // namespace dFgobjManager

// .text:0xB464: online, runs the received field commands (retrying the failed
// ones).
void dFgObjMng_c::procCommands() {
    if (dNet::isSessionActive()) {
        dFgMngCmd_c *cmd;
        for (int i = 0; i < FG_MNG_CMD_NUM; i++) {
            cmd = fgMngCmd_get(i);
            if (cmd->mNewItem != dFgMngCmd_c::ITEM_FREE) {
                if (fgMngCmd_isRetry(i)) {
                    if (!cmd->exec()) {
                        continue;
                    }
                    fgMngCmd_clearRetry(i);
                }
                dFgobjManager::procCmd(cmd);
                cmd->init();
            }
        }
    }
}

namespace dFgobjManager {

// .text:0xB4FC: the leaves of a weed / clover / flower on the unit fly
// (d_a_player walking through).
inline bool isWeedFg(u16 id) {
    return id >= dItem::FG_WEED_A && id <= dItem::FG_WEED_D;
}
inline bool isCloverFg(u16 id) {
    return id >= dItem::FG_CLOVER && id <= dItem::FG_LUCKY_CLOVER;
}
inline bool isDandelionFg(u16 id) {
    return id >= dItem::FG_DANDELION && id <= dItem::FG_DANDELION_PUFF;
}

void playGrassEffect(int x, int z, u16 id, const mAng3_c *ang, BOOL wind) {
    dItem::Item item(id);
    if (isWeedFg(id) || isCloverFg(id) || item.isWiltedFlower() ||
        item.isFlower() || isDandelionFg(item.getId())) {
        mVec3_c pos;
        dFgDraw::getItemPosition(&pos, id, x, z, FALSE);
        switch (id) {
        case dItem::FG_WEED_A:
            pos.x -= 5.0f;
            pos.z += 5.0f;
            break;
        case dItem::FG_WEED_B:
            pos.x += 5.0f;
            pos.z += 5.0f;
            break;
        case dItem::FG_WEED_C:
            pos.x -= 5.0f;
            pos.z -= 5.0f;
            break;
        case dItem::FG_WEED_D:
            pos.x += 5.0f;
            pos.z -= 5.0f;
            break;
        }
        dEffect_c effect;
        mVec3_c dir(-1.0f, 0.0f, 0.0f);
        mAng curve = dWorld::getCurveAngle(&pos);
        if (ang != NULL && wind) {
            dir.rotY(ang->y);
            dir.rotX(curve);
        }
        u16 fg = item.getId();
        if (isWeedFg(fg)) {
            if (effect.createEffect("af_fg_weed", &pos, ang, NULL) && wind) {
                effect.vf44(&dir);
            }
        } else if (isCloverFg(fg)) {
            if (effect.createEffect("af_fg_crover", &pos, ang, NULL) && wind) {
                effect.vf44(&dir);
            }
        } else {
            fgMngProc_playFlowerFallEffect(&item, x, z, 2, ang, wind);
        }
    }
}

// .text:0xB7EC: puts a spaceship part (the shot down UFO, d_a_ufo) on a random
// free unit of the block row of z that the camera doesn't show, on the given
// side of the player.
BOOL dropUfoPart(int unused, int z, u32 side) {
    u16 rows[0x70];
    int columns[0x70];
    int bits[16];
    dFdBase_c *fd;
    int z0;
    int x;
    int unitZ;
    int xEnd;
    int zEnd;
    int num;
    const mVec3_c *center;
    mVec3_c *pos = dPlayerMgr::getPosition(4);
    if (pos == NULL) {
        return FALSE;
    }
    center = &dCamera_c::sCenter;
    num = 0;
    fd = dField::getInfo(1);
    z0 = z & 0xF0;
    zEnd = (z0 + 0x10) & 0xF0;
    xEnd = (int)pos->x >> 5;
    if (side == 1) {
        x = xEnd + 1;
        xEnd = 0x60;
    } else {
        x = 0x10;
    }
    for (; x < xEnd; x++) {
        rows[x] = 0;
        for (unitZ = z0; unitZ < zEnd; unitZ++) {
            if (!isInCameraView(x, unitZ, center) &&
                !isBadDropSpot(fd, x, unitZ)) {
                rows[x] |= 1 << (unitZ & 0xF);
            }
        }
        if (rows[x] != 0) {
            columns[num] = x;
            num++;
        }
    }
    if (num == 0) {
        return FALSE;
    }
    int col;
    int n = 0;
    col = columns[cM::rndInt(num)];
    u16 row = rows[col];
    int *p = bits;
    for (int i = 0; i < 16; i++) {
        if (row & (1 << i)) {
            *p++ = i;
            n++;
        }
    }
    z = z0 + bits[cM::rndInt(n)];
    dFgMngProc_c::setUnitItem(
        fd, col, z, dItem::Item(dItem::ITEM_IDX_SPACESHIP_PART).mId, 0);
    return TRUE;
}

// .text:0xBA98: whether the unit lies within the camera's view around center.
BOOL isInCameraView(int x, int z, const mVec3_c *center) {
    mVec3_c pos;
    dFdBase_c::getUnitGroundPos(&pos, x, z);
    static mVec3_c sMin(-(32.0f * (u32)l_8074FEE0), 0.0f,
                        -(32.0f * (u32)l_8074FEE8));
    static mVec3_c sMax(32.0f * (u32)l_8074FEE4, 0.0f, 32.0f * (u32)l_8074FEEC);
    if (pos.x > center->x + sMin.x && pos.x < center->x + sMax.x &&
        pos.z > center->z + sMin.z && pos.z < center->z + sMax.z) {
        return TRUE;
    }
    return FALSE;
}

// .text:0xBC54: whether nothing can be dropped on the unit.
BOOL isBadDropSpot(dFdBase_c *fd, int x, int z) {
    dItem::Item *item = fd->getItem(x, z, 0);
    if (!fd->canPutItem(x, z) || item == NULL ||
        item->mId != dItem::ITEM_ID_NONE) {
        return TRUE;
    }
    int below = z + 1;
    if (isBlockedUnit(fd, x, below) == TRUE ||
        isPondEdgeUnit(fd, x, below) == TRUE) {
        return TRUE;
    }
    return FALSE;
}

// .text:0xBD1C: the player steps on the unit at pos (d_a_player): the plants
// around it sway and a flower on it is damaged.
void stepOnUnit(int player, const mVec3_c *pos) {
    int x, z, unitZ;
    f32 posZ = pos->z;
    x = (int)pos->x >> 5;
    z = (int)posZ >> 5;
    for (int dz = -1; dz <= 1; dz++) {
        unitZ = z + dz;
        for (int dx = -1; dx <= 1; dx++) {
            swayPlantAt(player, x + dx, unitZ, pos, 16.0f);
        }
    }
    fgMngProc_damageFlowerAt(x, z, 1);
}

} // namespace dFgobjManager

// .text:0xBDC8
dFgObjMng_c::~dFgObjMng_c() {
}

#ifdef MUST_MATCH
// .rodata 0x918: seven ints nothing references, after the last literals. Most
// likely folded function-local static consts (kind values such as
// FG_MNG_KIND_PICK_UP_SPECIAL / _MISS) whose functions are unknown; kept for
// the .rodata layout.
extern const int lUnusedRodataTail[7];
const int lUnusedRodataTail[7] = {-1, 0x15, 0xC, 0x18, 0x13, 0x1F, 0x17};
#endif

#ifdef MUST_MATCH
// Unused (dead-stripped by the linker): only needs m3d's weak inlines in the
// target's order (callback_c::timingC, timingA, anmChr_c::getType), ahead of
// the end-of-TU vtable pass that adds anmVis_c::getType and timingB.
static void orderM3dWeakInlines(m3d::mdl_c::callback_c *cb, m3d::anmChr_c *anm,
                                nw4r::g3d::ResMdl resMdl) {
    cb->m3d::mdl_c::callback_c::timingC(NULL, resMdl);
    cb->m3d::mdl_c::callback_c::timingA(0, NULL, resMdl);
    anm->m3d::anmChr_c::getType();
}
#endif
