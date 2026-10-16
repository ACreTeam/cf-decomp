// REL .text 0xCC..0x5694, .ctors 0x0..0x4, .rodata 0x0..0x550, .data 0x0..0x7B0, .bss 0x8..0x308.
// Header order sets the weak vtable/RTTI order in .data (reverse class completion).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_a_ftr.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_catalog.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_fg_draw.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_model_room.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_sv_auc.hpp>
#include <game/game/d_time_stamp.hpp>
#include <game/game/d_weather.hpp>
#include <game/framework/f_profile.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <revolution/MEM.h>
#include <revolution/MTX.h>
#include <revolution/OS.h>
#include <stdio.h>
#include <string.h>

extern "C" {
BOOL fn_16_5C58(daFTR_c *ftr, const mVec3_c *pos, dItem::Item item, const mVec3_c *ofs, mVec3_c *out);
void fn_16_A1F0(daFTR_c *ftr);
BOOL fn_16_D740(daFTR_c *ftr);
BOOL fn_16_D754(daFTR_c *ftr);
BOOL fn_16_122CC(daFTR_c *ftr);
BOOL fn_16_122E0(daFTR_c *ftr);
}

struct tvEntry_c {
    /* 0x0 */ s16 mProgram;
    /* 0x2 */ u8 mHour;
    /* 0x3 */ u8 mMin;
}; // size 0x4

struct tvDay_c {
    /* 0x0 */ const tvEntry_c *mEntries;
    /* 0x4 */ int mNum;
}; // size 0x8

class hostIO_c {
public:
    hostIO_c() {
        for (u16 i = 0; i < ARRAY_SIZE(_04); i++) {
            _04[i] = 0;
        }
        _80 = 0;
        _81 = 0;
        _9C = 0;
        _9E = 100;
        _A0 = 0;
        mRotateWait = 6;
        mRotateSpeed = 26;
        mHoldWait = 21;
        mHoldWaitNpc = 21;
        mSeatCenterDist = 6.4f;
        _90 = 0;
        _91 = 0;
        _96 = 0;
        _A2 = 0;
        _A3 = 0;
        _92 = 90;
        _94 = 0x222E;
        _98 = -1;
        mPushAngle = 60;
        mPushDist = 2.0f;
        mDpdHeight = 11.2f;
        mRegion = -1;
        _7C = -1;
        _74 = -1;
    }
    virtual ~hostIO_c() {}

    /* 0x04 */ u32 _04[28];
    /* 0x74 */ s32 _74;
    /* 0x78 */ s32 mRegion;
    /* 0x7C */ s32 _7C;
    /* 0x80 */ u8 _80;
    /* 0x81 */ u8 _81;
    /* 0x82 */ u16 mRotateWait;
    /* 0x84 */ u16 mRotateSpeed;
    /* 0x86 */ u16 mHoldWait;
    /* 0x88 */ u16 mHoldWaitNpc;
    /* 0x8C */ f32 mSeatCenterDist;
    /* 0x90 */ u8 _90;
    /* 0x91 */ u8 _91;
    /* 0x92 */ u16 _92;
    /* 0x94 */ u16 _94;
    /* 0x96 */ u8 _96;
    /* 0x98 */ s32 _98;
    /* 0x9C */ u8 _9C;
    /* 0x9E */ u16 _9E;
    /* 0xA0 */ u16 _A0;
    /* 0xA2 */ u8 _A2;
    /* 0xA3 */ u8 _A3;
    /* 0xA4 */ s16 mPushAngle;
    /* 0xA8 */ f32 mPushDist;
    /* 0xAC */ f32 mDpdHeight;
}; // size 0xB0

static inline u32 makeInfo(int x, int z, const dItem::Item &item, int rot, int layer, int kind) {
    daFTR_c::info_c info;
    info.mX = x;
    info.mZ = z;
    info.mItem = item.getId();
    info.mRot = rot;
    info.mLayer = layer;
    info.mKind = kind;
    return info.mValue;
}

static const u32 l_lampSceneAttr[] = {
    dScene::SCENE_ATTR_HOUSE0 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE1 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE2 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE3 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    0,
};

const u32 dFtrm_c::sRecSize = 0x8400;
const u32 dFtrm_c::sMinResSize = 0x6C00;

// 0xCC
namespace dFtrManager {
BASE_PROFILE(FTR_MANAGER, dFtrm_c)
}

static const char *l_resName[] = {
    "catalog_carpet.brres",
    "catalog_wall.brres",
    "int_anim.brres",
    "int_common.brres",
    "int_dummy.brres",
    "int_hnw.brres",
    "int_kob_oth_dummytv.brres",
    "int_mannequin.brres",
    "int_mannequin_grace.brres",
    "room_goki.brres",
    "standA.brres",
    "standB.brres",
    "standC.brres",
    "standD.brres",
};

static dFtrm_c *l_instance;
u8 dFtrm_c::sLampOn;
dRecBank::bankTbl_c dFtrm_c::sRecBank;
dFtrm_c::loader_c dFtrm_c::sLoader;
static dFtrm_c::current_c l_current;
static hostIO_c l_hostIO;

// 0x16C
void dFtrm_c::loader_c::onLoaded() {
    // Unreferenced; only its .rodata slot is known.
    static const int _548 = 23;
    static const u32 resourceCount = ARRAY_SIZE(l_resName);
    ARCInitHandle(mpData, &mHandle);
    mArcReady = 1;
    for (u32 i = 0; i < resourceCount; i++) {
        nw4r::g3d::ResFile res(dDvd::arcBank_c::getFile(l_resName[i], NULL));
        res.Init();
        res.Bind(res);
    }
}

// 0x204
void *dFtrm_c::loader_c::loadFile(const char *name, EGG::Heap *heap) {
    if (heap != NULL) {
        void *p = heap->alloc(getFileSize(name), 0x20);
        if (bindFile(p, name)) {
            return p;
        }
    }
    return NULL;
}

// 0x2AC
void *dFtrm_c::loader_c::getFile(const char *name) {
    u32 size;
    return dDvd::arcBank_c::getFile(name, &size);
}

// 0x2D0
BOOL dFtrm_c::loader_c::load(void *heap) {
    return dDvd::bank_c::load("Ftr/roomAlways.arc", heap, 0);
}

// 0x2E4
u16 dFtrm_c::getRotateSpeed() {
    return l_hostIO.mRotateSpeed;
}

// 0x2F4
u16 dFtrm_c::getRotateWait() {
    return l_hostIO.mRotateWait;
}

// 0x304
u16 dFtrm_c::getHoldWait() {
    return l_hostIO.mHoldWait;
}

// 0x314
u16 dFtrm_c::getHoldWaitNpc() {
    return l_hostIO.mHoldWaitNpc;
}

// 0x324
f32 dFtrm_c::getSeatCenterDist() {
    // Truncated to whole units through u16, as in the original.
    return (u16)l_hostIO.mSeatCenterDist;
}

// 0x368
BOOL dFtrm_c::isDebug() {
    return FALSE;
}

// 0x370
f32 dFtrm_c::getPushRate() {
    return 1.0f;
}

// 0x37C
f32 dFtrm_c::getPullRate() {
    return 1.0f;
}

// 0x388
f32 dFtrm_c::getDpdHeight() {
    return l_hostIO.mDpdHeight;
}

// 0x398
s16 dFtrm_c::getPushAngle() {
    return l_hostIO.mPushAngle * 0xB6;
}

// 0x3B0
f32 dFtrm_c::getPushDist() {
    return l_hostIO.mPushDist;
}

// 0x3C0
BOOL dFtrm_c::isFtrScene() {
    BOOL ftr = FALSE;
    if (dField::getInfo(0) != NULL && dScene::getSceneParamA(dScene::getCurrentScene()) != 0) {
        ftr = TRUE;
    }
    return ftr;
}

// 0x414
int dFtrm_c::create() {
    mActive = isFtrScene();
    if (!mActive) {
        return SUCCEEDED;
    }
    init();
    if (loadRes()) {
        createChildren();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// 0x480
int dFtrm_c::execute() {
    if (!mActive) {
        return SUCCEEDED;
    }
    mAppear.calc();
    mTvPlayer.execute();
    executeTv();
    mTimer = (mTimer + 1) % 120;
    checkTalk();
    checkFtr();
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_PLAYER_HOUSE)) {
        u32 num = dFTR::countSwitch(daFTR_c::isLampOn, 1);
        if (sLampOn) {
            if (num == 0) {
                dSubLightRoom_c::sLightOn = 0;
                dSubLightRoom_c::sLight.set(0, 1, dSubLightRoom_c::sLightMode == 1, 0.1f);
                sLampOn = FALSE;
            }
        } else if (num != 0) {
            dSubLightRoom_c::sLightMode = 0;
            dSubLightRoom_c::sLightOn = 1;
            dSubLightRoom_c::sLight.set(1, 1, 0, 0.1f);
            dFTR::requestRoomFlag(dScene::getCurrentScene(), 1);
            sLampOn = TRUE;
        }
    }
    return SUCCEEDED;
}

// 0x5F0
int dFtrm_c::draw() {
    if (!mActive) {
        return SUCCEEDED;
    }
    mAppear.draw();
    return SUCCEEDED;
}

// 0x62C
int dFtrm_c::doDelete() {
    if (deleteRes()) {
        l_instance = NULL;
        getCurrent()->destroy();
        getPool()->destroy();
        sRecBank.destroy();
        dFTR::getList()->clear();
        dFTR::getUnitMap()->clear();
        sTransAnm.remove();
        dFTR::getHoldList()->clear();
        destroyFrmHeap();
        removeDefaultFtr();
        dFTR::enableHold();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// 0x6CC
BOOL dFtrm_c::loadRes() {
    if (sLoader.load(dHeap::localHeap_p)) {
        bool ok = true;
        ok &= mTvPlayer.execute();
        return ok;
    }
    return FALSE;
}

// 0x724
BOOL dFtrm_c::deleteRes() {
    bool ok = true;
    ok &= mTvPlayer.remove();
    if (ok && sLoader.closeArc()) {
        return TRUE;
    }
    return FALSE;
}

// 0x770
void dFtrm_c::checkFtr() {
    int x;
    int z;
    dFdBase_c *field;
    u32 layer;
    BOOL found;
    BOOL idle;
    dItem::Item *item;
    u32 i;
    daFTR_c *ftr;
    field = dField::getInfo(0);
    if (field != NULL) {
        for (layer = 0; layer < 2; layer++) {
            for (z = 0; z < 16; z++) {
                for (x = 0; x < 16; x++) {
                    item = field->getItem(x, z, 0);
                    if (item != NULL && item->hasFtrFunc() && dFTR::getUnitMap()->get(x, z, 0) == NULL) {
                        if (dBGCF::getAttr(x, z) == BG_ATTR_WALL) {
                            if (!dScene::isCurrentSceneModelRoom()) {
                                continue;
                            }
                            if (x != 15U || z != 15U) {
                                continue;
                            }
                        }
                        createFtr(x, z, *item, item->getId() & 3, layer, FTR_KIND_PLACED);
                    }
                }
            }
        }
        for (i = 0; i < dFTR::list_c::MAX; i++) {
            ftr = dFTR::getList()->get(i);
            found = FALSE;
            idle = FALSE;
            if (ftr != NULL && ftr->canSetSwitch() && ftr->isIdle() && ftr->isParentIdle()) {
                idle = TRUE;
            }
            int ftrX;
            int ftrZ;
            if (idle) {
                if (ftr->getUnit(&ftrX, &ftrZ, NULL, mAng(0))) {
                    found = TRUE;
                }
            }
            if (found) {
                dItem::Item *ftrItem = field->getItem(ftrX, ftrZ, ftr->getLayer());
                if (ftrItem != NULL) {
                    dFTR::shape_c shape;
                    shape.set(*ftrItem);
                    for (u32 j = 0; j < shape.getNum(); j++) {
                        int unitX = ftrX + shape.getOfs(j)[0];
                        int unitZ = ftrZ + shape.getOfs(j)[1];
                        dItem::Item *unitItem = field->getItem(unitX, unitZ, ftr->getLayer());
                        if (unitItem != NULL && !unitItem->isUsable()) {
                            ftr->deleteRequest();
                        }
                    }
                }
            }
        }
    }
}

// 0xA20
void dFtrm_c::checkTalk() {
    dFdBase_c *field;
    BOOL talk = FALSE;
    const dObjc::dpdHit_c *ftrHit = NULL;
    if (!dFTR::isHoldDisabled() && !dFTR::isHoldDisabledOnly() && !dPlayerMgr::getChairPosition(NULL, 4) &&
        !dPlayerMgr::getBedPosition(NULL, 4)) {
        if (mPad::g_currentCore->getDpdValidFlag() > 0) {
            const dObjc::dpdHit_c *hit = dObjc::getDpdHit();
            if (hit != NULL) {
                u32 type = hit->mType;
                if (type == 0x80000001) {
                    if (dPad::isATriggered()) {
                        talk = TRUE;
                    }
                } else if (type == dObjc::HIT_FTR || type == dObjc::HIT_FTR_ROOM) {
                    if (dPlayerMgr::fn_800FFA50(&hit->mPos, 0x3000, 64.0f)) {
                        ftrHit = hit;
                        talk = TRUE;
                    }
                }
            }
            if (dPad::isConfirmTriggered() && !dPad::isATriggered()) {
                talk = TRUE;
            }
        } else if (dPad::isConfirmTriggered()) {
            talk = TRUE;
        }
    }

    field = dField::getInfo(0);
    if (dScene::isCurrentSceneModelRoom() && talk && dPad::isConfirmTriggered() && field != NULL) {
        mVec3_c pos = mVec3_c::Zero;
        if (dPlayerMgr::getGroundPositionAhead(&pos)) {
            int x;
            int z;
            dBGCF::posToUnit(&x, &z, &pos);
            dItem::Item *item = field->getItem(x, z, 0);
            BOOL ok = TRUE;
            if (ftrHit != NULL && ftrHit->mBase.get() == NULL) {
                int hitX;
                int hitZ;
                dBGCF::posToUnit(&hitX, &hitZ, &ftrHit->mPos);
                if (x != hitX || z != hitZ) {
                    ok = FALSE;
                }
            }
            if (ok && item != NULL && dItem::isRealItemId(item->getId()) && !item->hasFtrFunc()) {
                for (u32 i = 0; i < dFTR::list_c::MAX; i++) {
                    daFTR_c *ftr = dFTR::getList()->get(i);
                    if (ftr != NULL && !ftr->mFlags.mSpeakDemo && !ftr->mFlags.mSpeak &&
                        ftr->mState != daFTR_c::STATE_TALK && ftr->mState != daFTR_c::STATE_TALK_END) {
                        mVec3_c demoPos;
                        dBGCF::snapToUnit(&demoPos, &pos);
                        demoPos.y = pos.y;
                        ftr->mFlags.mSpeakDemo = 1;
                        ftr->mDemoPos = demoPos;
                        ftr->mMsgItem0 = *item;
                        ftr->mMsgItem1 = dItem::Item();
                        ftr->mMsgItem2 = dItem::Item();
                        return;
                    }
                }
            }
        }

        putUnit_c unit;
        int ret = findPutUnit(&unit, FALSE);
        if (ret == PUT_UNIT_ITEM) {
            BOOL ok = TRUE;
            if (ftrHit != NULL && ftrHit->mBase.get() == NULL) {
                int hitX;
                int hitZ;
                dBGCF::posToUnit(&hitX, &hitZ, &ftrHit->mPos);
                if (unit.mX != hitX || unit.mZ != hitZ) {
                    ok = FALSE;
                }
            }
            if (ok) {
                daFTR_c *ftr = dFTR::getUnitMap()->get(unit.mX, unit.mZ, 0);
                if (ftr != NULL && !ftr->mFlags.mSpeakDemo && !ftr->mFlags.mSpeak &&
                    ftr->mState != daFTR_c::STATE_TALK && ftr->mState != daFTR_c::STATE_TALK_END) {
                    mVec3_c demoPos;
                    dBGCF::unitToPos(&demoPos, unit.mX, unit.mZ);
                    ftr->mDemoPos = demoPos;
                    ftr->mFlags.mSpeakDemo = 1;
                    ftr->mMsgItem1 = unit.mItem;
                    ftr->mMsgItem2 = dItem::Item();
                    ftr->mMsgItem0 = ftr->getItem();
                }
            }
        } else if (ret != PUT_UNIT_NONE) {
            daFTR_c *ftr = dFTR::getUnitMap()->get(unit.mX, unit.mZ, (u8)unit.mLayer);
            if (ftr != NULL) {
                BOOL ok = TRUE;
                if (ftrHit != NULL) {
                    fBase_c *base = ftrHit->mBase.get();
                    if (base != NULL && base != ftr) {
                        ok = FALSE;
                    }
                }
                if (ok) {
                    ftr->mMsgItem2 = dItem::Item();
                    ftr->mFlags.mSpeak = 1;
                    daFTR_c *lower = dFTR::getUnitMap()->get(unit.mX, unit.mZ, 0);
                    if (unit.mLayer == 1) {
                        mVec3_c demoPos;
                        dBGCF::unitToPos(&demoPos, unit.mX, unit.mZ);
                        ftr->mDemoPos = demoPos;
                    }
                    if (ftr->isMusic() && ftr->getSwitch()) {
                        dSaveTown_c *town = dSaveData_c::getTown();
                        ftr->mMsgItem2 = town->mModelRoom.getSong();
                    }
                    if (lower != NULL) {
                        ftr->mMsgItem0 = lower->getItem();
                        dItem::Item *item = field->getItem(unit.mX, unit.mZ, 1);
                        if (item != NULL && dItem::isRealItemId(item->getId())) {
                            ftr->mMsgItem1 = *item;
                        }
                    }
                }
            }
        }
    }
}

// 0xFD4
void dFtrm_c::init() {
    if (!mInited) {
        l_instance = this;
        dFTR::getList()->clear();
        dFTR::getUnitMap()->clear();
        dFTR::getHoldList()->clear();
        createFrmHeap();
        u32 num = dScene::getSceneParamA(dScene::getCurrentScene());
        getPool()->create(num);
        sRecBank.create(dHeap::localHeap_p, sRecSize, dScene::getSceneParamA(dScene::getCurrentScene()));
        mTvPlayer.create();
        getCurrent()->init();
        initLight();
    }
    mInited = TRUE;
}

// 0x10A0
void dFtrm_c::createChildren() {
    createCockroach();
    nw4r::g3d::ResFile res(sLoader.getFile("int_dummy.brres"));
    sTransAnm.create(res.GetResMdl("int_dummy"), res.GetResAnmChr("int_push"), res.GetResAnmChr("int_pull"),
                     &mAllocator);
    createResetObj();
    putLoftFtr();
    setAuctionFtr();
    createAllFtr();
    registerCatalog();
}

// 0x1164
void dFtrm_c::registerCatalog() {
    u8 scene = dScene::getCurrentScene();
    if (dScene::isMyHouseScene(scene)) {
        int room;
        int home = dHomeList_c::getHomeFromScene(&room, scene);
        dItem::Item items[3];
        dHomeRoom_c::getDefaultItems((u16 *)items, home, room);
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (player != NULL) {
            u16 item = items[1].getId();
            if (item != dItem::ITEM_ID_NONE && !player->mCatalog.isRegistered(item)) {
                player->mCatalog.registerItem(items[1].getId(), FALSE);
            }
            item = items[2].getId();
            if (item != dItem::ITEM_ID_NONE && !player->mCatalog.isRegistered(item)) {
                player->mCatalog.registerItem(items[2].getId(), FALSE);
            }
            item = items[0].getId();
            if (item != dItem::ITEM_ID_NONE && !player->mCatalog.isRegistered(item)) {
                player->mCatalog.registerItem(items[0].getId(), FALSE);
            }
        }
    }
}

// 0x127C
BOOL dFtrm_c::isLampScene() {
    int room;
    int home = dScene::getSceneAttrTableIndex(dScene::getCurrentScene(), l_lampSceneAttr, &room);
    BOOL lamp = FALSE;
    if ((home != -1 && dHome_c::isValidRoom(home)) || dScene::getCurrentScene() == dScene::SCENE_RM_HAPPY_MDL) {
        lamp = TRUE;
    }
    return lamp;
}

// 0x12E8
void dFtrm_c::initLight() {
    if (isLampScene()) {
        dSubLightRoom_c::sLightOn = 0;
    }
}

// 0x131C
void dFtrm_c::createFrmHeap() {
    if (m_frmHeap_p == NULL) {
        m_frmHeap_p = mHeap::createFrmHeap(0x32000, dHeap::localHeap_p, "dFtrm_c::m_frmHeap_p", 0x20, mHeap::OPT_NONE);
        mAllocator.attach(m_frmHeap_p, 0x20);
    }
}

// 0x1388
void dFtrm_c::destroyFrmHeap() {
    if (m_frmHeap_p != NULL) {
        mHeap::destroyFrmHeap(m_frmHeap_p);
        m_frmHeap_p = NULL;
    }
}

// 0x13CC
dActor_c *dFtrm_c::createWallFtr(u32 idx, dItem::Item item) {
    int x;
    int z;
    if (getWallUnit(&x, &z, idx)) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
        if (bitm != NULL && bitm->getFtrSize() == dItem::FTR_SIZE_1x2) {
            z++;
        }
        return createFtr(x, z, item, 0, 0, FTR_KIND_APPEAR);
    }
    return NULL;
}

// 0x1484
dActor_c *dFtrm_c::createFtr(int x, int z, dItem::Item item, int rot, u8 layer, int kind) {
    return createFtr(makeInfo(x, z, item, rot, layer, kind));
}

// 0x14C8
dActor_c *dFtrm_c::createFtr(u32 info) {
    if (dFTR::getList()->getFreeNum() != 0) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item((u16)(info >> 16)));
        if (bitm != NULL) {
            if (bitm->getFtrFunc() == 0) {
                return NULL;
            }
            return dActor_c::construct(bitm->getFtrFuncType(), l_instance, info, NULL, NULL);
        }
    }
    return NULL;
}

static const f32 l_reach[] = {48.0f, 49.6f};

// 0x156C
int dFtrm_c::checkPutUnit(putUnit_c *out, int x, int z, int layer, const mVec3_c *pos) {
    putUnit_c tmp;
    putUnit_c *unit = out != NULL ? out : &tmp;
    dFdBase_c *field = dField::getInfo(0);
    mVec3_c center;
    dFdBase_c::getUnitCenterPos(center, x, z);
    if (center.xzDistTo(*pos) <= l_reach[layer & 1]) {
        daFTR_c *ftr = dFTR::getUnitMap()->get(x, z, (u8)layer);
        unit->mX = x;
        unit->mZ = z;
        int posX;
        int posZ;
        dBGCF::posToUnit(&posX, &posZ, pos);
        if (layer != 1 && !canPass(posX, posZ, x, z)) {
            return PUT_UNIT_NONE;
        }
        if (ftr != NULL && ftr->isWait() && !ftr->hasTopItem()) {
            unit->mLayer = layer;
            unit->mItem = ftr->getPutItem();
            unit->mDirItem = ftr->getDirItem(mAng(0));
            return dFTR::getList()->find(ftr);
        }
        if (layer == 1 && ftr == NULL && field != NULL) {
            dItem::Item *item = field->getItem(x, z, layer);
            if (item != NULL && dItem::isRealItemId(item->getId())) {
                unit->mLayer = layer;
                unit->mItem = *item;
                unit->mDirItem = *item;
                return PUT_UNIT_ITEM;
            }
        }
    }
    return PUT_UNIT_NONE;
}

// 0x1780
int dFtrm_c::findPutUnit(putUnit_c *out, const mVec3_c *pos, mAng angle, BOOL frontOnly) {
    if (dFTR::isHoldDisabled()) {
        return PUT_UNIT_NONE;
    }
    int room;
    if (dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene()) != -1 && room == HOME_SCENE_LOFT) {
        return PUT_UNIT_NONE;
    }
    int x;
    int z;
    if (frontOnly) {
        if (getDirUnit(&x, &z, pos, angle, 0)) {
            int ret = checkPutUnit(out, x, z, 1, pos);
            if (ret != PUT_UNIT_NONE) {
                return ret;
            }
        }
        if (getDirUnit(&x, &z, pos, angle, 0)) {
            int ret = checkPutUnit(out, x, z, 0, pos);
            if (ret != PUT_UNIT_NONE) {
                return ret;
            }
        }
    } else {
        for (u32 dir = 0; dir < 4; dir++) {
            for (int l = 1; l >= 0; l--) {
                if (getDirUnit(&x, &z, pos, angle, dir)) {
                    int ret = checkPutUnit(out, x, z, l, pos);
                    if (ret != PUT_UNIT_NONE) {
                        return ret;
                    }
                }
            }
        }
    }
    return PUT_UNIT_NONE;
}

// 0x190C
int dFtrm_c::findPutUnit(putUnit_c *out, BOOL frontOnly) {
    dPlayerActor_c *player = dPlayerMgr::getPlayerActor(4);
    if (player != NULL) {
        return findPutUnit(out, &player->mPos, player->mAngle.y.mAngle, frontOnly);
    }
    return PUT_UNIT_NONE;
}

// 0x1978
int dFtrm_c::sendEvent(int handle) {
    return dFTR::sendEvent(handle);
}

// 0x197C
BOOL dFtrm_c::canPass(int x, int z, int toX, int toZ) {
    static const f32 checkRadius = 3.2f;
    int dx = cM::absI(x - toX);
    int dz = cM::absI(z - toZ);
    int dist = dx + dz;
    if (dist == 0) {
        return TRUE;
    }
    if (dist == 1) {
        return TRUE;
    }
    mVec3_c from;
    dBGCF::unitToPos(&from, x, z);
    from.y = dBGCF::getUnitBaseY(x, z);
    // acch2 is never used, but the original constructs it.
    dBGCF::acch_c acch1;
    dBGCF::acch_c acch2;
    mVec3_c mid;
    dBGCF::unitToPos(&mid, toX, z);
    mid.y = dBGCF::getUnitBaseY(toX, z);
    mVec3_c p0(mid);
    acch1.check(checkRadius, &p0, &from, 0, 0,
                dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_STEP_EXT_48 | dBGCF::CHECK_SET_POS, TRUE);
    if (!(acch1.isWallHit() || acch1.isStepWallHit())) {
        return TRUE;
    }
    dBGCF::unitToPos(&mid, x, toZ);
    mid.y = dBGCF::getUnitBaseY(x, toZ);
    mVec3_c p1(mid);
    acch1.check(checkRadius, &p1, &from, 0, 0,
                dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_STEP_EXT_48 | dBGCF::CHECK_SET_POS, TRUE);
    if (!(acch1.isWallHit() || acch1.isStepWallHit())) {
        return TRUE;
    }
    return FALSE;
}

// 0x1B54
int dFtrm_c::checkPutRot(u32 *info, int x, int z, int rot, dItem::Item item, u8 layer, const mVec3_c *pos) {
    int posX = worldToUnit(pos->x);
    int posZ = worldToUnit(pos->z);
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm == NULL) {
        return PUT_INVALID;
    }
    if (bitm->getFtrFunc() == 0) {
        return PUT_INVALID;
    }
    int size = bitm->getFtrSize();
    int ret = checkPut(info, x, z, rot, item, layer, posX, posZ);
    if (ret == PUT_OK && size == dItem::FTR_SIZE_1x2) {
        dFTR::shape_c shape;
        dItem::Item dirItem = item.makeDir(rot);
        shape.set(dirItem);
        mVec3_c center;
        shape.getCenterPos(&center, x, z);
        mVec3_c diff = *pos - center;
        if (labs((s16)(cM::atan2s(diff.x, diff.z) - (s16)(rot << 14))) > 0x4000) {
            u32 other;
            int flipRet = checkPut(&other, x + shape.getOfs(1)[0], z + shape.getOfs(1)[1], (rot + 2) & 3, item, layer,
                                   posX, posZ);
            if (flipRet == PUT_OK) {
                *info = other;
                return PUT_OK;
            }
        }
    }
    return ret;
}

// 0x1D48
int dFtrm_c::checkPut(u32 *info, int x, int z, int rot, dItem::Item item, u32 layer, int playerX, int playerZ) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm == NULL) {
        return PUT_INVALID;
    }
    dItem::Item dirItem = item.makeDir(rot);
    BOOL passThru = bitm->m_ftrPassThru;
    int size = bitm->getFtrSize();
    int table = bitm->getFtrTable();
    dFTR::shape_c shape;
    shape.set(dirItem);
    u32 num = shape.getNum();
    dFdBase_c *field = dField::getInfo(0);
    for (u32 i = 0; i < num; i++) {
        const int *ofs = shape.getOfs(i);
        int unitX = x + ofs[0];
        int unitZ = z + ofs[1];
        dItem::Item *unitItem = field->getItem(unitX, unitZ, layer);
        if ((!passThru && unitX == playerX && unitZ == playerZ) || unitItem == NULL ||
            unitItem->getId() != dItem::ITEM_ID_NONE || !dBGCF::canPutItem(unitX, unitZ)) {
            return PUT_BLOCKED;
        }
        if (layer == 1) {
            daFTR_c *lower = dFTR::getUnitMap()->get(unitX, unitZ, 0);
            if (lower == NULL || table != dItem::FTR_TABLE_TOP || lower->isBusy()) {
                return PUT_BLOCKED;
            }
            if (size != dItem::FTR_SIZE_1x1) {
                return PUT_INVALID;
            }
            const dItem::BITM *lowerBitm = dItem::infoBank_c::get()->getBITM(dItem::Item((u16)(lower->mParam >> 16)));
            if (lowerBitm != NULL && lowerBitm->getFtrTable() != dItem::FTR_TABLE_ITEM) {
                return PUT_BLOCKED;
            }
        }
    }
    *info = makeInfo(x, z, item, rot, layer, FTR_KIND_APPEAR);
    return PUT_OK;
}

static const struct {
    int x;
    int z;
} l_putOfs[] = {
    {0, 0},
    {-1, 0},
    {0, -1},
    {-1, -1},
};

static const u8 l_rotOrder[] = {0, 2, 1, 3};

// 0x1F98
int dFtrm_c::findPutPos(u32 *info, dItem::Item item, const mVec3_c *pos, mAng angle, int kind) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm == NULL) {
        return PUT_INVALID;
    }
    if (bitm->getFtrFunc() == 0) {
        return PUT_INVALID;
    }
    if (dFTR::getList()->getFreeNum() == 0) {
        return PUT_FULL;
    }
    if (kind == FTR_KIND_HIDDEN || kind == FTR_KIND_DUMMY) {
        daFTR_c::info_c putInfo;
        putInfo.mX = 0;
        putInfo.mZ = 0;
        putInfo.mItem = item.getId();
        putInfo.mRot = 0;
        putInfo.mLayer = 0;
        putInfo.mKind = kind;
        *info = putInfo.mValue;
        return PUT_OK;
    }
    if (!dScene::isCurrentSceneMyHouse()) {
        return PUT_INVALID;
    }
    int size = bitm->getFtrSize();
    int dir = getDir(mAng(angle.mAngle + 0x8000));
    int posX = worldToUnit(pos->x);
    int posZ = worldToUnit(pos->z);
    for (u32 d = 0; d < 4; d++) {
        for (int l = 1; l >= 0; l--) {
            int x;
            int z;
            if (getDirUnit(&x, &z, pos, angle, d)) {
                mVec3_c center;
                dFdBase_c::getUnitCenterPos(center, x, z);
                if (l == 1 || canPass(posX, posZ, x, z)) {
                    for (int r = 0; r < 4; r++) {
                        int rot = (dir + l_rotOrder[r]) & 3;
                        u32 num = size == dItem::FTR_SIZE_2x2 ? 4 : 1;
                        for (u32 k = 0; k < num; k++) {
                            int ret = checkPutRot(info, x + l_putOfs[k].x, z + l_putOfs[k].z, rot, item, l, pos);
                            if (ret == PUT_OK) {
                                return PUT_OK;
                            }
                        }
                    }
                }
            }
        }
    }
    return PUT_BLOCKED;
}

// 0x2204
int dFtrm_c::findPutPos(u32 *info, dItem::Item item, int kind) {
    dPlayerActor_c *player = dPlayerMgr::getPlayerActor(4);
    if (player != NULL) {
        return findPutPos(info, item, &player->mPos, player->getAngleY(), kind);
    }
    return PUT_INVALID;
}

// 0x2284
int dFtrm_c::getPlayerIdx() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL) {
        return player->findInSave();
    }
    return -1;
}

// 0x22B8
int dFtrm_c::putOrgItem(int base, u32 *info, int slot, int kind) {
    int idx = getPlayerIdx();
    if (idx != -1) {
        dItem::Item item(base + idx * 8 + (slot & 7));
        return findPutPos(info, item, kind);
    }
    return PUT_INVALID;
}

// 0x2348
int dFtrm_c::putOrgCloth(u32 *info, int slot) {
    return putOrgItem(dItem::ITEM_IDX_ORG_CLOTH_00, info, slot, FTR_KIND_APPEAR);
}

// 0x235C
int dFtrm_c::putOrgEasel(u32 *info, int slot) {
    return putOrgItem(dItem::ITEM_IDX_ORG_EASEL_00, info, slot, FTR_KIND_APPEAR);
}

// 0x2370
int dFtrm_c::putOrgUmb(u32 *info, int slot) {
    return putOrgItem(dItem::ITEM_IDX_ORG_UMB_00, info, slot, FTR_KIND_APPEAR);
}

// 0x2384
int dFtrm_c::putOrgCap(u32 *info, int slot) {
    return putOrgItem(dItem::ITEM_IDX_ORG_CAP_00, info, slot, FTR_KIND_APPEAR);
}

static const struct {
    int x;
    int z;
} l_dirOfs[] = {
    {0, 1},  {1, 1},   {1, 1},   {1, 0},   {1, 0},   {1, -1}, {1, -1}, {0, -1},
    {0, -1}, {-1, -1}, {-1, -1}, {-1, 0},  {-1, 0},  {-1, 1}, {-1, 1}, {0, 1},
};

static const s16 l_dirStep[] = {0, 0, 2, -2};

// 0x2398
BOOL dFtrm_c::getDirUnit(int *x, int *z, const mVec3_c *pos, mAng angle, int dir) {
    static const int directionCount = ARRAY_SIZE(l_dirStep);
    s16 ang = angle;
    *x = worldToUnit(pos->x);
    *z = worldToUnit(pos->z);
    if (dir == 1) {
        return TRUE;
    }
    if (dir < directionCount) {
        int i = (((ang >> 12) & 0xF) + l_dirStep[dir]) & 0xF;
        *x += l_dirOfs[i].x;
        *z += l_dirOfs[i].z;
        return TRUE;
    }
    return FALSE;
}

// 0x2444
int dFtrm_c::getDir(mAng angle) {
    u16 ang = angle.mAngle;
    if (ang >= 0xE000 || ang < 0x2000) {
        return 0;
    }
    if (ang < 0x6000) {
        return 1;
    }
    return ang < 0xA000 ? 2 : 3;
}

// 0x248C
int dFtrm_c::checkStand(const mVec3_c *pos, mAng angle, BOOL snap, BOOL player, f32 dist) {
    mVec3_c stand;
    getOfsPos(&stand, pos, angle, dist);
    if (snap) {
        dBGCF::snapToUnit(&stand, &stand);
    }
    daFTR_c *ftr = dFTR::getUnitMap()->get(&stand, 0);
    if (ftr != NULL && !ftr->canWalkOn()) {
        return STAND_BLOCKED;
    }
    f32 groundY = dBGCF::getGroundY(pos, FALSE);
    f32 standY = dBGCF::getGroundY(&stand, FALSE);
    if (standY != groundY) {
        return STAND_BLOCKED;
    }
    groundY = dBGCF::getGroundY(&stand, FALSE);
    standY = dFTR::getGroundY(&stand);
    if (standY != groundY) {
        return STAND_BLOCKED;
    }
    if (!daFTR_c::isFree(&stand, FALSE, daFTR_c::sFreeRadius, daFTR_c::sFreeRadius, daFTR_c::sFreeRadius, 0.0f)) {
        return STAND_OCCUPIED;
    }
    if (dBGCF::getOpenNext(&stand) != -1) {
        return STAND_BLOCKED;
    }
    return STAND_OK;
}

// 0x25D8
BOOL dFtrm_c::canSit(const mVec3_c *pos, mAng angle) {
    daFTR_c *ftr = dFTR::getUnitMap()->get(pos, 0);
    if (ftr != NULL && (ftr->isFtrFunc36() || ftr->isFtrFunc39())) {
        return FALSE;
    }
    if (checkStand(pos, (s16)angle, FALSE, TRUE, 32.0f) == STAND_OK) {
        return TRUE;
    }
    return FALSE;
}

// 0x268C
BOOL dFtrm_c::canStandLeft(const mVec3_c *pos, mAng angle) {
    daFTR_c *ftr = dFTR::getUnitMap()->get(pos, 0);
    if (ftr != NULL) {
        if (ftr->isFtrFunc39()) {
            return FALSE;
        }
        if (ftr->isFtrFuncD()) {
            return FALSE;
        }
        if (ftr->getFtrSize() == dItem::FTR_SIZE_1x2) {
            mVec3_c center;
            ftr->getCenter(&center);
            if (pos->xzDistTo(center) <= getSeatCenterDist()) {
                return FALSE;
            }
        }
    }
    return checkStand(pos, angle.mAngle + 0x4000, TRUE, TRUE, 32.0f) == STAND_OK;
}

// 0x27B0
BOOL dFtrm_c::canStandRight(const mVec3_c *pos, mAng angle) {
    daFTR_c *ftr = dFTR::getUnitMap()->get(pos, 0);
    if (ftr != NULL) {
        if (ftr->isFtrFunc36()) {
            return FALSE;
        }
        if (ftr->isFtrFuncD()) {
            return FALSE;
        }
        if (ftr->getFtrSize() == dItem::FTR_SIZE_1x2) {
            mVec3_c center;
            ftr->getCenter(&center);
            if (pos->xzDistTo(center) <= getSeatCenterDist()) {
                return FALSE;
            }
        }
    }
    return checkStand(pos, angle.mAngle + 0xC000, TRUE, TRUE, 32.0f) == STAND_OK;
}

// 0x28D8
BOOL dFtrm_c::getSeatPos(const mVec3_c *pos, dItem::Item item, const mVec3_c *ofs, mVec3_c *out) {
    daFTR_c *ftr = dFTR::getUnitMap()->get(pos, 0);
    if (ftr != NULL && ftr->mProfName == fProf::A_FTR_ACT_02A) {
        return fn_16_5C58(ftr, pos, item, ofs, out);
    }
    return FALSE;
}

// 0x296C
BOOL dFtrm_c::getSeatPosLeft(const mVec3_c *pos, dItem::Item item, mVec3_c *out) {
    static mVec3_c l_ofs(-32.0f, 0.0f, 0.0f);
    return getSeatPos(pos, item, &l_ofs, out);
}

// 0x2A1C
BOOL dFtrm_c::getSeatPosRight(const mVec3_c *pos, dItem::Item item, mVec3_c *out) {
    static mVec3_c l_ofs(32.0f, 0.0f, 0.0f);
    return getSeatPos(pos, item, &l_ofs, out);
}

// 0x2ACC
void dFtrm_c::turnOnSeatSound(const mVec3_c *pos) {
    daFTR_c *ftr = dFTR::getUnitMap()->get(pos, 0);
    if (ftr != NULL && ftr->isSeat()) {
        ftr->turnOnSound();
    }
}

// 0x2B24
void dFtrm_c::updateSeatSound(const mVec3_c *pos) {
    daFTR_c *ftr = dFTR::getUnitMap()->get(pos, 0);
    if (ftr != NULL && ftr->isSeat()) {
        ftr->updateSound();
    }
}

// 0x2B7C
void dFtrm_c::turnOffSeatSound(const mVec3_c *pos) {
    daFTR_c *ftr = dFTR::getUnitMap()->get(pos, 0);
    if (ftr != NULL && ftr->isSeat()) {
        ftr->turnOffSound();
    }
}

dFtrm_c::transAnm_c dFtrm_c::sTransAnm;

// 0x2BD4
dFtrm_c::transAnm_c::transAnm_c() {
    mpOwner = NULL;
}

// 0x2C44
void dFtrm_c::transAnm_c::nodeCallBack_c::timingB(ulong nodeId, nw4r::g3d::WorldMtxManip *manip,
                                                  nw4r::g3d::ResMdl resMdl) {
    if (nodeId == 0) {
        mMtx_c mtx;
        manip->GetMatrix(&mtx);
        mVec3_c zero(0.0f, 0.0f, 0.0f);
        PSMTXMultVec(mtx, zero, mPos);
    }
}

// 0x2CA8
void dFtrm_c::transAnm_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmChr push, nw4r::g3d::ResAnmChr pull,
                                 mAllocator_c *allocator) {
    mResMdl = mdl;
    mPush = push;
    mPull = pull;
    mMdl.create(mdl, allocator, 0, 1, NULL);
    mMdl.setCallback(&mNodeCallBack);
    mAnmChr.create(mdl, push, allocator, NULL);
    mAnmChr.mPlayMode = m3d::FORWARD_ONCE;
    mMdl.setAnm(mAnmChr);
}

// 0x2D80
void dFtrm_c::transAnm_c::remove() {
    mMdl.remove();
    mAnmChr.remove();
}

// 0x2DCC
BOOL dFtrm_c::transAnm_c::start(nw4r::g3d::ResAnmChr anm, const u32 *owner, const mVec3_c *pos, mAng angle) {
    if (mpOwner == NULL) {
        mAnmChr.setAnm(mMdl, anm, m3d::FORWARD_ONCE);
        mMdl.setAnm(mAnmChr);
        mpOwner = owner;
        mNodeCallBack.mPos = *pos;
        mMtx_c mtx;
        mtx.trans(mNodeCallBack.mPos);
        mtx.YrotM(angle);
        mMdl.setLocalMtx(&mtx);
        return TRUE;
    }
    return FALSE;
}

// 0x2EA8
BOOL dFtrm_c::transAnm_c::startPush(const u32 *owner, const mVec3_c *pos, mAng angle) {
    return start(mPush, owner, pos, angle);
}

// 0x2EEC
BOOL dFtrm_c::transAnm_c::startPull(const u32 *owner, const mVec3_c *pos, mAng angle) {
    return start(mPull, owner, pos, angle);
}

// 0x2F30
BOOL dFtrm_c::transAnm_c::calc(const u32 *owner, mVec3_c *pos) {
    if (mpOwner != NULL && mpOwner == owner) {
        mAnmChr.play();
        mMdl.calc(false);
        *pos = mNodeCallBack.mPos;
        if (mAnmChr.isStop()) {
            mpOwner = NULL;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// 0x2FD8
BOOL dFtrm_c::canTalk(daFTR_c *ftr) {
    BOOL ok = FALSE;
    if (ftr != NULL && ftr->isWait() && !ftr->hasTopItem()) {
        ok = TRUE;
    }
    return ok;
}

// 0x3044
BOOL dFtrm_c::canTalk(int handle) {
    if (handle >= 0) {
        daFTR_c *ftr = dFTR::getList()->get(handle);
        if (ftr != NULL) {
            return canTalk(ftr);
        }
    }
    return FALSE;
}

static mVec3_c l_effectPos(0.0f, 0.0f, 0.0f);

// 0x3094
mVec3_c *dFtrm_c::getEffectPos(int handle) {
    daFTR_c *ftr = dFTR::getList()->get(handle);
    if (ftr == NULL) {
        return &l_effectPos;
    }
    mVec3_c scale = ftr->getScale();
    if (scale.x < 0.3f) {
        return &l_effectPos;
    }
    return NULL;
}

// 0x3118
void dFtrm_c::setEffectPos(const mVec3_c *pos) {
    l_effectPos = *pos;
}

static dFtrm_c::pool_c l_pool;

// 0x313C
dFtrm_c::pool_c *dFtrm_c::getPool() {
    return &l_pool;
}

// 0x3148
void dFtrm_c::pool_c::clear() {
    mNum = 0;
    mSize = 0;
    mpData = NULL;
    memset(mUsed, 0, sizeof(mUsed));
}

static const u32 l_ftrSize[] = {
    0x8D0, 0x8D4, 0x964, 0x968, 0x8D4, 0x8D8, 0x8D4, 0x96C, 0x8D0, 0x8E0, 0x8DC, 0x924, 0x96C,
    0x968, 0x968, 0x8CC, 0x934, 0x8D4, 0x8D4, 0x990, 0x944, 0x8E0, 0x908, 0x8D0, 0x8D0, 0x944,
    0x914, 0x968, 0x908, 0x8E4, 0x8CC, 0x8CC, 0x8CC, 0x8CC, 0x8D4, 0x8F4, 0x8CC,
};

// 0x3168
BOOL dFtrm_c::pool_c::create(u32 num) {
    static const u32 typeCount = ARRAY_SIZE(l_ftrSize);
    if (mNum == 0) {
        clear();
        mSize = 0;
        for (const u32 *p = l_ftrSize; p < l_ftrSize + typeCount; p++) {
            if (*p > mSize) {
                mSize = *p;
            }
        }
        mSize = ROUND_UP(mSize, 0x20);
        u32 size = mSize * num;
        if (size != 0) {
            mpData = (u8 *)dHeap::localHeap_p->alloc(size, 0x20);
            if (mpData != NULL) {
                mNum = num;
                return TRUE;
            }
            return FALSE;
        }
    }
    return TRUE;
}

// 0x324C
void *dFtrm_c::pool_c::alloc() {
    for (u32 i = 0; i < mNum; i++) {
        u32 byte = i >> 3;
        u32 bit = i & 7;
        if (!((mUsed[byte] >> bit) & 1)) {
            mUsed[byte] |= 1 << bit;
            return mpData + mSize * i;
        }
    }
    return NULL;
}

// 0x32B0
BOOL dFtrm_c::pool_c::free(void *p) {
    for (u32 i = 0; i < mNum; i++) {
        u32 byte = i >> 3;
        u32 bit = i & 7;
        if (mpData + mSize * i == p) {
            mUsed[byte] &= ~(1 << bit);
            return TRUE;
        }
    }
    return FALSE;
}

// 0x3314
BOOL dFtrm_c::pool_c::destroy() {
    if (mpData != NULL) {
        dHeap::localHeap_p->free(mpData);
        clear();
        return TRUE;
    }
    return FALSE;
}

// 0x3374
void dFtrm_c::getOfsPos(mVec3_c *out, const mVec3_c *pos, mAng angle, f32 dist) {
    mVec3_c ofs(0.0f, 0.0f, dist);
    ofs.rotY(angle);
    *out = *pos + ofs;
}

// 0x33F8
void dFtrm_c::appearItem(u16 item, const mVec3_c *pos) {
    if (l_instance != NULL) {
        l_instance->mAppear.add(item, pos);
    }
}

// 0x341C
void dFtrm_c::appearList_c::clear() {
    for (int i = 0; i < ARRAY_SIZE(mAppear); i++) {
        mAppear[i].mItem = dItem::ITEM_ID_NONE;
    }
}

// 0x3438
void dFtrm_c::appearList_c::add(u16 item, const mVec3_c *pos) {
    static const f32 travelFrames = 40.0f;
    dPlayerActor_c *player = dPlayerMgr::getPlayerActor(4);
    if (player != NULL) {
        appear_c *appear = mAppear;
        for (int i = 0; i < ARRAY_SIZE(mAppear); i++, appear++) {
            if (appear->mItem.getId() == dItem::ITEM_ID_NONE) {
                appear->mPos = player->mPos;
                appear->mPos.y += 16.0f;
                appear->mTarget = *pos;
                appear->mSpeed.x = (pos->x - appear->mPos.x) / travelFrames;
                appear->mSpeed.z = (pos->z - appear->mPos.z) / travelFrames;
                appear->mScale = 0.0f;
                f32 fall = 0.0f;
                f32 speed = 0.0f;
                for (u32 n = 0; n < travelFrames; n++) {
                    fall += speed;
                    speed -= 0.17777778f;
                }
                appear->mSpeed.y = (appear->mTarget.y - appear->mPos.y - fall) / 41.0f;
                appear->mItem = item;
                return;
            }
        }
    }
}

// 0x3574
void dFtrm_c::appearList_c::calc() {
    appear_c *appear = mAppear;
    for (u32 i = 0; i < ARRAY_SIZE(mAppear); i++, appear++) {
        if (appear->mItem.getId() != dItem::ITEM_ID_NONE) {
            sLib::chase(&appear->mScale, 1.0f, 0.025f);
            appear->mPos += appear->mSpeed;
            if (appear->mSpeed.y < 0.0f && appear->mPos.y < appear->mTarget.y) {
                appear->mPos.y = appear->mTarget.y;
            }
            appear->mSpeed.y -= 0.17777778f;
            f32 dist = appear->mPos.x - appear->mTarget.x;
            if (dist < 0.0f) {
                dist = -dist;
            }
            bool arrived = dist < 0.001f;
            if (arrived) {
                appear->mItem = dItem::ITEM_ID_NONE;
            }
        }
    }
}

// 0x36B8
void dFtrm_c::appearList_c::draw() {
    for (u32 i = 0; i < ARRAY_SIZE(mAppear); i++) {
        appear_c *appear = &mAppear[i];
        if (appear->mItem.getId() != dItem::ITEM_ID_NONE) {
            mVec3_c scale(appear->mScale, appear->mScale, appear->mScale);
            dFgDraw::add(appear->mItem.getId(), appear->mPos, scale, FALSE, FALSE, mAng3_c(0, 0, 0));
        }
    }
}

// 0x374C
void dFtrm_c::texPatAnm_c::clear() {
    mFrame = 0;
    mFile = nw4r::g3d::ResFile();
    mAnm = nw4r::g3d::ResAnmTexPat();
}

// 0x3760
void dFtrm_c::texPatAnm_c::set(nw4r::g3d::ResFile file, nw4r::g3d::ResAnmTexPat anm) {
    mFrame = 0;
    mFile = file;
    mAnm = anm;
    mLoop = 0;
}

// 0x3780
BOOL dFtrm_c::texPatAnm_c::calc() {
    static const u32 paletteSize = 0x20;
    if (!mAnm.IsValid() || !mFile.IsValid()) {
        return FALSE;
    }
    u32 num = mAnm.GetNumFrame();
    if (mFrame >= num) {
        mFrame %= num;
        if (++mLoop >= 0x100) {
            mLoop = 0x100;
        }
    }
    nw4r::g3d::TexPatAnmResult result;
    mAnm.GetAnmResult(&result, 0, mFrame);
    const void *tex = result.tex[0].GetTexData();
    const void *pltt = result.pltt[0].GetPlttData();
    nw4r::g3d::ResTex dstTex = mFile.GetResTex(0);
    void *dstTexData = dstTex.GetTexData();
    u16 *dstPlttData = mFile.GetResPltt(0).GetPlttData();
    u32 size = dstTex.GetHeight() * dstTex.GetWidth() / 2U;
    memcpy(dstTexData, tex, size);
    DCStoreRangeNoSync(dstTexData, size);
    memcpy(dstPlttData, pltt, paletteSize);
    DCStoreRangeNoSync(dstPlttData, paletteSize);
    mFrame++;
    return TRUE;
}

// 0x3B38
dFtrm_c::tvPlayer_c::tvPlayer_c() {
    mEnabled = 0;
    mBuffer = 0;
    m_frmHeap_p[1] = NULL;
    m_frmHeap_p[0] = NULL;
    mProgram = -1;
    mNext = -1;
    mForecast = 0;
}

static const tvEntry_c l_tvFri[] = {
    {TV_PROGRAM_VARIETY, 0, 0}, {TV_PROGRAM_TRAVEL, 1, 0}, {TV_PROGRAM_COLOR, 3, 0}, {TV_PROGRAM_STORM, 4, 0},
    {TV_PROGRAM_COLOR, 5, 0}, {TV_PROGRAM_NEWS, 6, 0}, {TV_PROGRAM_WEATHER, 6, 45}, {TV_PROGRAM_NEWS, 7, 0},
    {TV_PROGRAM_WEATHER, 7, 45}, {TV_PROGRAM_DRAMA, 8, 0}, {TV_PROGRAM_TRAVEL, 8, 30}, {TV_PROGRAM_HERO, 10, 0},
    {TV_PROGRAM_NEWS, 11, 0}, {TV_PROGRAM_WEATHER, 11, 45}, {TV_PROGRAM_VARIETY, 12, 0}, {TV_PROGRAM_DRAMA, 13, 0},
    {TV_PROGRAM_TRAVEL, 15, 0}, {TV_PROGRAM_HERO, 16, 0}, {TV_PROGRAM_VARIETY, 17, 0}, {TV_PROGRAM_NEWS, 18, 0},
    {TV_PROGRAM_WEATHER, 18, 45}, {TV_PROGRAM_SOCCER, 19, 0}, {TV_PROGRAM_DRAMA, 21, 0}, {TV_PROGRAM_NEWS, 22, 0},
    {TV_PROGRAM_WEATHER, 22, 45}, {TV_PROGRAM_QUIZ, 23, 0},
};
static const tvEntry_c l_tvMon[] = {
    {TV_PROGRAM_COLOR, 0, 0}, {TV_PROGRAM_STORM, 1, 0}, {TV_PROGRAM_COLOR, 5, 0}, {TV_PROGRAM_NEWS, 6, 0},
    {TV_PROGRAM_WEATHER, 6, 45}, {TV_PROGRAM_NEWS, 7, 0}, {TV_PROGRAM_WEATHER, 7, 45}, {TV_PROGRAM_DRAMA, 8, 0},
    {TV_PROGRAM_TRAVEL, 8, 30}, {TV_PROGRAM_HERO, 10, 0}, {TV_PROGRAM_NEWS, 11, 0}, {TV_PROGRAM_WEATHER, 11, 45},
    {TV_PROGRAM_VARIETY, 12, 0}, {TV_PROGRAM_DRAMA, 13, 0}, {TV_PROGRAM_VARIETY, 13, 30},
    {TV_PROGRAM_TRAVEL, 15, 0}, {TV_PROGRAM_HERO, 16, 0}, {TV_PROGRAM_VARIETY, 17, 0}, {TV_PROGRAM_NEWS, 18, 0},
    {TV_PROGRAM_WEATHER, 18, 45}, {TV_PROGRAM_VARIETY, 19, 0}, {TV_PROGRAM_QUIZ, 20, 0}, {TV_PROGRAM_DRAMA, 21, 0},
    {TV_PROGRAM_NEWS, 22, 0}, {TV_PROGRAM_WEATHER, 22, 45}, {TV_PROGRAM_VARIETY, 23, 0},
};
static const tvEntry_c l_tvSat[] = {
    {TV_PROGRAM_SONG, 0, 0}, {TV_PROGRAM_MOVIE, 1, 0}, {TV_PROGRAM_COLOR, 3, 0}, {TV_PROGRAM_STORM, 4, 0},
    {TV_PROGRAM_COLOR, 5, 0}, {TV_PROGRAM_NEWS, 6, 0}, {TV_PROGRAM_WEATHER, 6, 45}, {TV_PROGRAM_NEWS, 7, 0},
    {TV_PROGRAM_WEATHER, 7, 45}, {TV_PROGRAM_VARIETY, 8, 0}, {TV_PROGRAM_TRAVEL, 9, 0}, {TV_PROGRAM_HERO, 10, 0},
    {TV_PROGRAM_NEWS, 11, 0}, {TV_PROGRAM_WEATHER, 11, 45}, {TV_PROGRAM_VARIETY, 12, 0}, {TV_PROGRAM_TRAVEL, 14, 0},
    {TV_PROGRAM_SOCCER, 15, 0}, {TV_PROGRAM_VARIETY, 17, 0}, {TV_PROGRAM_NEWS, 18, 0}, {TV_PROGRAM_WEATHER, 18, 45},
    {TV_PROGRAM_QUIZ, 19, 0}, {TV_PROGRAM_VARIETY, 20, 0}, {TV_PROGRAM_MOVIE, 21, 0}, {TV_PROGRAM_WEATHER, 22, 45},
    {TV_PROGRAM_NEWS, 23, 0},
};
static const tvEntry_c l_tvSun[] = {
    {TV_PROGRAM_COLOR, 0, 0}, {TV_PROGRAM_STORM, 1, 0}, {TV_PROGRAM_COLOR, 5, 0}, {TV_PROGRAM_NEWS, 6, 0},
    {TV_PROGRAM_WEATHER, 6, 45}, {TV_PROGRAM_HERO, 7, 0}, {TV_PROGRAM_TRAVEL, 8, 0}, {TV_PROGRAM_QUIZ, 9, 0},
    {TV_PROGRAM_VARIETY, 10, 0}, {TV_PROGRAM_NEWS, 11, 0}, {TV_PROGRAM_WEATHER, 11, 45}, {TV_PROGRAM_HERO, 12, 0},
    {TV_PROGRAM_SOCCER, 13, 0}, {TV_PROGRAM_QUIZ, 15, 0}, {TV_PROGRAM_TRAVEL, 16, 0}, {TV_PROGRAM_VARIETY, 17, 0},
    {TV_PROGRAM_NEWS, 18, 0}, {TV_PROGRAM_WEATHER, 18, 45}, {TV_PROGRAM_SOCCER, 19, 0}, {TV_PROGRAM_MOVIE, 21, 0},
    {TV_PROGRAM_WEATHER, 22, 45}, {TV_PROGRAM_NEWS, 23, 0},
};
static const tvEntry_c l_tvThu[] = {
    {TV_PROGRAM_VARIETY, 0, 0}, {TV_PROGRAM_SOCCER, 1, 0}, {TV_PROGRAM_COLOR, 3, 0}, {TV_PROGRAM_STORM, 4, 0},
    {TV_PROGRAM_COLOR, 5, 0}, {TV_PROGRAM_NEWS, 6, 0}, {TV_PROGRAM_WEATHER, 6, 45}, {TV_PROGRAM_NEWS, 7, 0},
    {TV_PROGRAM_WEATHER, 7, 45}, {TV_PROGRAM_DRAMA, 8, 0}, {TV_PROGRAM_TRAVEL, 8, 30}, {TV_PROGRAM_HERO, 10, 0},
    {TV_PROGRAM_NEWS, 11, 0}, {TV_PROGRAM_WEATHER, 11, 45}, {TV_PROGRAM_VARIETY, 12, 0}, {TV_PROGRAM_DRAMA, 13, 0},
    {TV_PROGRAM_VARIETY, 13, 30}, {TV_PROGRAM_TRAVEL, 15, 0}, {TV_PROGRAM_HERO, 16, 0}, {TV_PROGRAM_VARIETY, 17, 0},
    {TV_PROGRAM_NEWS, 18, 0}, {TV_PROGRAM_WEATHER, 18, 45}, {TV_PROGRAM_VARIETY, 19, 0}, {TV_PROGRAM_TRAVEL, 20, 0},
    {TV_PROGRAM_DRAMA, 21, 0}, {TV_PROGRAM_NEWS, 22, 0}, {TV_PROGRAM_WEATHER, 22, 45}, {TV_PROGRAM_VARIETY, 23, 0},
};
static const tvEntry_c l_tvTue[] = {
    {TV_PROGRAM_VARIETY, 0, 0}, {TV_PROGRAM_MOVIE, 1, 0}, {TV_PROGRAM_COLOR, 3, 0}, {TV_PROGRAM_STORM, 4, 0},
    {TV_PROGRAM_COLOR, 5, 0}, {TV_PROGRAM_NEWS, 6, 0}, {TV_PROGRAM_WEATHER, 6, 45}, {TV_PROGRAM_NEWS, 7, 0},
    {TV_PROGRAM_WEATHER, 7, 45}, {TV_PROGRAM_DRAMA, 8, 0}, {TV_PROGRAM_TRAVEL, 8, 30}, {TV_PROGRAM_HERO, 10, 0},
    {TV_PROGRAM_NEWS, 11, 0}, {TV_PROGRAM_WEATHER, 11, 45}, {TV_PROGRAM_VARIETY, 12, 0}, {TV_PROGRAM_DRAMA, 13, 0},
    {TV_PROGRAM_TRAVEL, 15, 0}, {TV_PROGRAM_HERO, 16, 0}, {TV_PROGRAM_VARIETY, 17, 0}, {TV_PROGRAM_NEWS, 18, 0},
    {TV_PROGRAM_WEATHER, 18, 45}, {TV_PROGRAM_SOCCER, 19, 0}, {TV_PROGRAM_DRAMA, 21, 0}, {TV_PROGRAM_NEWS, 22, 0},
    {TV_PROGRAM_WEATHER, 22, 45}, {TV_PROGRAM_TRAVEL, 23, 0},
};
static const tvEntry_c l_tvWed[] = {
    {TV_PROGRAM_TRAVEL, 0, 0}, {TV_PROGRAM_SONG, 1, 0}, {TV_PROGRAM_COLOR, 3, 0}, {TV_PROGRAM_STORM, 4, 0},
    {TV_PROGRAM_COLOR, 5, 0}, {TV_PROGRAM_NEWS, 6, 0}, {TV_PROGRAM_WEATHER, 6, 45}, {TV_PROGRAM_NEWS, 7, 0},
    {TV_PROGRAM_WEATHER, 7, 45}, {TV_PROGRAM_DRAMA, 8, 0}, {TV_PROGRAM_TRAVEL, 8, 30}, {TV_PROGRAM_HERO, 10, 0},
    {TV_PROGRAM_NEWS, 11, 0}, {TV_PROGRAM_WEATHER, 11, 45}, {TV_PROGRAM_VARIETY, 12, 0}, {TV_PROGRAM_DRAMA, 13, 0},
    {TV_PROGRAM_VARIETY, 13, 30}, {TV_PROGRAM_TRAVEL, 15, 0}, {TV_PROGRAM_HERO, 16, 0}, {TV_PROGRAM_VARIETY, 17, 0},
    {TV_PROGRAM_NEWS, 18, 0}, {TV_PROGRAM_WEATHER, 18, 45}, {TV_PROGRAM_HERO, 19, 0}, {TV_PROGRAM_SONG, 20, 0},
    {TV_PROGRAM_DRAMA, 21, 0}, {TV_PROGRAM_NEWS, 22, 0}, {TV_PROGRAM_WEATHER, 22, 45}, {TV_PROGRAM_QUIZ, 23, 0},
};

static const tvDay_c l_tvWeek[] = {
    {l_tvSun, ARRAY_SIZE(l_tvSun)}, {l_tvMon, ARRAY_SIZE(l_tvMon)}, {l_tvTue, ARRAY_SIZE(l_tvTue)},
    {l_tvWed, ARRAY_SIZE(l_tvWed)}, {l_tvThu, ARRAY_SIZE(l_tvThu)}, {l_tvFri, ARRAY_SIZE(l_tvFri)},
    {l_tvSat, ARRAY_SIZE(l_tvSat)},
};

// 0x3C1C
int dFtrm_c::tvPlayer_c::getProgram() {
    int program = -1;
    dTimeStamp_c stamp;
    stamp.setNow();
    dTime_c now = *dTime_c::getCurrent();
    const tvDay_c *day = &l_tvWeek[stamp.getWeekday()];
    for (int i = day->mNum - 1; i >= 0; i--) {
        const tvEntry_c *entry = &day->mEntries[i];
        if (now.hour > entry->mHour) {
            program = entry->mProgram;
            break;
        }
        if (now.hour == entry->mHour && now.min >= entry->mMin) {
            program = entry->mProgram;
            break;
        }
    }
    return program < 0 ? TV_PROGRAM_STORM : program;
}

// 0x3D30
BOOL dFtrm_c::tvPlayer_c::create() {
    mEnabled = dScene::getSceneParamA(dScene::getCurrentScene()) != 0 &&
               !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_DEMO) &&
               !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_CHKP);
    if (!mEnabled) {
        return TRUE;
    }
    BOOL ok = TRUE;
    for (u32 i = 0; i < ARRAY_SIZE(m_frmHeap_p); i++) {
        m_frmHeap_p[i] = mHeap::createFrmHeap(
            0x32000, dHeap::localHeap_p,
            "dFtrm_c::tvPlayer_c::m_frmHeap_p \x83\x65\x83\x8C\x83\x72\x83\x41\x83\x6A\x83\x81\x83\x71\x81\x5B\x83\x76",
            0x20, mHeap::OPT_NONE);
        if (m_frmHeap_p[i] == NULL) {
            ok = FALSE;
        }
    }
    return ok;
}

static const char *l_tvWeather[] = {
    "/Ftr/tv_program_ff.brres", "/Ftr/tv_program_cc.brres", "/Ftr/tv_program_rr.brres",
    "/Ftr/tv_program_fc.brres", "/Ftr/tv_program_cf.brres", "/Ftr/tv_program_fr.brres",
    "/Ftr/tv_program_cr.brres", "/Ftr/tv_program_rc.brres", "/Ftr/tv_program_ss.brres",
    "/Ftr/tv_program_cs.brres", "/Ftr/tv_program_sc.brres",
};

static const char *l_regionEU = "EU";
static const char *l_regionUS = "US";

// 0x3E00
BOOL dFtrm_c::tvPlayer_c::execute() {
    static const u32 forecastCount = ARRAY_SIZE(l_tvWeather);
    if (!mEnabled) {
        return TRUE;
    }
    if (mNext < 0) {
        int program = getProgram();
        if (program >= 0 && program != mProgram) {
            mNext = program;
            EGG::FrmHeap *heap = m_frmHeap_p[(mBuffer + 1) & 1];
            if (heap != NULL) {
                heap->free(MEM_FRM_HEAP_FREE_ALL);
            }
        }
    }
    if (mNext >= 0) {
        u32 next = (mBuffer + 1) & 1;
        char path[100];
        if (mNext == TV_PROGRAM_WEATHER) {
            dTime_c time = *dTime_c::getCurrent();
            time.add(1, 0, 0, 0);
            mForecast = dWeather::getForecast(&time);
            if (mForecast >= forecastCount) {
                mForecast = 0;
            }
            snprintf(path, sizeof(path), "%s", l_tvWeather[mForecast]);
        } else if (mNext == TV_PROGRAM_COLOR) {
            const char *region = dSTR::getTownRegion() == CONSOLE_REGION_EU ? l_regionEU : l_regionUS;
            if (l_hostIO.mRegion != -1) {
                region = l_hostIO.mRegion != 0 ? l_regionEU : l_regionUS;
            }
            snprintf(path, sizeof(path), "/Ftr/tv_program%d_%s.brres", mNext, region);
        } else {
            snprintf(path, sizeof(path), "/Ftr/tv_program%d.brres", mNext);
        }
        dSndTv_c *snd = getSnd(next);
        if (snd->mObj == NULL) {
            snd->create(dHeap::localHeap_p, mNext);
        }
        if (mBank[next].load(path, m_frmHeap_p[next], 0) && snd->isReady()) {
            snd->reset();
            nw4r::g3d::ResFile dummy(sLoader.getFile("int_kob_oth_dummytv.brres"));
            nw4r::g3d::ResFile file(mBank[next].getData());
            mAnm.set(dummy, file.GetResAnmTexPat(0));
            u32 cur = (next + 1) & 1;
            m_frmHeap_p[cur]->free(MEM_FRM_HEAP_FREE_ALL);
            mBank[cur].unload(FALSE);
            dSndTv_c *curSnd = &mSnd[cur];
            if (curSnd->mObj != NULL) {
                curSnd->stop();
                curSnd->clear();
            }
            mProgram = mNext;
            mNext = -1;
            mBuffer++;
        }
    }
    return mAnm.calc();
}

// 0x4110
BOOL dFtrm_c::tvPlayer_c::remove() {
    bool ok = true;
    for (u32 i = 0; i < ARRAY_SIZE(mBank); i++) {
        ok &= mBank[i].unload(FALSE);
        if (mSnd[i].mObj != NULL && !mSnd[i].mOn) {
            bool ready = mSnd[i].isReady();
            if (ready) {
                mSnd[i].reset();
            }
            ok &= ready;
        }
        if (mSnd[i].mOn) {
            mSnd[i].stop();
            mSnd[i].clear();
        }
    }
    if (ok) {
        for (u32 i = 0; i < ARRAY_SIZE(mBank); i++) {
            if (m_frmHeap_p[i] != NULL) {
                mHeap::destroyFrmHeap(m_frmHeap_p[i]);
                m_frmHeap_p[i] = NULL;
            }
        }
        return TRUE;
    }
    return FALSE;
}

static f32 l_tvDistSq = lbl_80750524 * lbl_80750524;

// 0x422C
void dFtrm_c::executeTv() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    BOOL mute = TRUE;
    dPlayerActor_c *player = dPlayerMgr::getPlayerActor(4);
    if (player != NULL) {
        mVec3_c playerPos = player->mPos;
        int nearest = -1;
        f32 minDist = l_tvDistSq;
        for (u32 i = 0; i < dFTR::list_c::MAX; i++) {
            daFTR_c *ftr = dFTR::getList()->get(i);
            if (ftr != NULL && ftr->isTvOn()) {
                mVec3_c center;
                ftr->getCenter(&center);
                f32 dist = (playerPos - center).xzLenSq();
                if (dist < minDist) {
                    nearest = i;
                    minDist = dist;
                }
            }
        }
        if (nearest != -1) {
            daFTR_c *ftr = dFTR::getList()->get(nearest);
            if (ftr != NULL) {
                fn_16_A1F0(ftr);
                mVec3_c center;
                ftr->getCenter(&center);
                mute = FALSE;
                pos = center;
            }
        }
    }
    dSndTv_c *snd = &mTvPlayer.mSnd[mTvPlayer.mBuffer & 1];
    if (snd->mObj != NULL) {
        int frame = mTvPlayer.mAnm.mFrame;
        frame--;
        if (frame < 0) {
            frame = 0;
        }
        if (mute) {
            snd->mute();
            mTvPlayer.mSnd[mTvPlayer.mBuffer & 1].execute(frame, NULL);
        } else {
            snd->setOn(FALSE);
            mTvPlayer.mSnd[mTvPlayer.mBuffer & 1].execute(frame, &pos);
        }
    }
}

static const struct {
    int x;
    int z;
} l_wallUnit[] = {
    {4, 5}, {7, 5}, {10, 5}, {4, 8}, {11, 8}, {4, 10}, {11, 10}, {4, 12}, {11, 12},
};

// 0x43F0
BOOL dFtrm_c::getWallUnit(int *x, int *z, u32 i) {
    static const u32 unitCount = ARRAY_SIZE(l_wallUnit);
    if (i < unitCount) {
        *x = l_wallUnit[i].x;
        *z = l_wallUnit[i].z;
        return TRUE;
    }
    return FALSE;
}

// 0x4428
void dFtrm_c::setAuctionFtr() {
    dFdBase_c *field = dField::getInfo(0);
    if (dScene::getCurrentScene() == dScene::SCENE_RM_AUCTION) {
        for (u32 i = 0; i < ARRAY_SIZE(l_wallUnit); i++) {
            dItem::Item *item = &dSvAuc_c::get()->getItem(i)->mItem;
            if (item->hasFtrFunc()) {
                int x;
                int z;
                if (getWallUnit(&x, &z, i)) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                    if (bitm != NULL && bitm->getFtrSize() == dItem::FTR_SIZE_1x2) {
                        z++;
                    }
                    field->setItem(item, x, z, 0);
                }
            }
        }
    }
}

// 0x4520
void dFtrm_c::putDefaultFtr() {
    if (dScene::isCurrentSceneModelRoom()) {
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            dItem::Item item(dItem::ITEM_IDX_NOT_USED_FTR_06);
            field->setItem(&item, 15, 15, 0);
            createFtr(15, 15, item, 0, 0, FTR_KIND_PLACED);
        }
    }
}

// 0x45B8
void dFtrm_c::removeDefaultFtr() {
    if (dScene::isCurrentSceneModelRoom()) {
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            dItem::Item item;
            field->setItem(&item, 15, 15, 0);
        }
    }
}

// 0x461C
void dFtrm_c::createResetObj() {
    if (dScene::getCurrentScene() == dScene::SCENE_RM_RESET) {
        dActor_c::construct(fProf::A_RMOBJ_RESET, this, 0, NULL, NULL);
    }
}

// 0x466C
void dFtrm_c::putLoftFtr() {
    static const int unitX = 7;
    static const int unitZ = 11;
    int room;
    int home = dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene());
    if (home != -1 && room == HOME_SCENE_LOFT) {
        dHome_c *h = dSaveData_c::getTown()->mHomes.getHome(home);
        if (h != NULL) {
            dItem::Item item = h->getLoftFtr();
            item = item.makeDir(3);
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
            if (bitm != NULL) {
                int size = bitm->getFtrSize();
                dFdBase_c *field = dField::getInfo(0);
                if (field != NULL) {
                    if (size == dItem::FTR_SIZE_1x2) {
                        field->setItem(&item, unitX, unitZ, 0);
                    } else if (size == dItem::FTR_SIZE_2x2) {
                        field->setItem(&item, unitX, unitZ, 0);
                    }
                }
            }
        }
    }
}

// 0x477C
void dFtrm_c::createAllFtr() {
    int num = 0;
    dFdBase_c *field = dField::getInfo(0);
    dFdBlock_c *block = field != NULL ? field->getBlock(0, 0) : NULL;
    if (block != NULL) {
        for (u32 layer = 0; layer < 2; layer++) {
            for (u32 z = 0; z < 16; z++) {
                for (u32 x = 0; x < 16; x++) {
                    dItem::Item *item = block->getItem(x, z, layer);
                    if (item != NULL && item->hasFtrFunc() && dBGCF::getAttr(x, z) != BG_ATTR_WALL) {
                        if (createFtr(x, z, *item, item->getId() & 3, layer, FTR_KIND_PLACED)) {
                            num++;
                        }
                    }
                }
            }
        }
        if (num == 0) {
            putDefaultFtr();
        }
    }
}

// 0x4894
u16 dFtrm_c::getTimer() {
    if (l_instance != NULL) {
        return l_instance->mTimer;
    }
    return 0;
}

// 0x48B4
dFtrm_c::current_c *dFtrm_c::getCurrent() {
    return &l_current;
}

// 0x48C0
void dFtrm_c::current_c::clear() {
    mOld = NULL;
    mNew = NULL;
    mItem = 0;
}

// 0x48D4
void dFtrm_c::current_c::remove(daFTR_c *ftr) {
    if (ftr == mNew) {
        mNew = NULL;
    }
    if (ftr == mOld) {
        mOld = NULL;
    }
}

// 0x4900
BOOL dFtrm_c::current_c::init() {
    clear();
    return TRUE;
}

// 0x4924
BOOL dFtrm_c::current_c::destroy() {
    removeAll();
    return TRUE;
}

// 0x4948
BOOL dFtrm_c::current_c::create(const dItem::Item *item) {
    dItem::Item put = *item;
    u32 data = 0;
    if (mOld != NULL) {
        replace(NULL);
    }
    mOld = mNew;
    mNew = NULL;
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm != NULL) {
        switch (bitm->getKind()) {
        case dItem::KIND_WALL:
            put = dItem::Item(dItem::ITEM_IDX_CATALOG_ONLY_00);
            data = item->getId();
            break;
        case dItem::KIND_CARPET:
            put = dItem::Item(dItem::ITEM_IDX_CATALOG_ONLY_00);
            data = item->getId();
            break;
        }
        u32 info;
        if (findPutPos(&info, put, FTR_KIND_HIDDEN) == PUT_OK) {
            mItem = data;
            return dynamic_cast<daFTR_c *>(createFtr(info)) != NULL;
        }
    }
    return FALSE;
}

// 0x4A8C
daFTR_c *dFtrm_c::current_c::get() {
    if (mNew == NULL || !mNew->canSetSwitch()) {
        return mOld;
    }
    return mNew;
}

// 0x4AE8
void dFtrm_c::current_c::replace(daFTR_c *ftr) {
    if (mOld != NULL && (ftr == NULL || ftr != mOld)) {
        mOld->deleteRequest();
        mOld = NULL;
    }
}

// 0x4B3C
void dFtrm_c::current_c::removeAll() {
    replace(NULL);
    if (mNew != NULL) {
        mNew->deleteRequest();
        mNew = NULL;
    }
}

// 0x4B84
BOOL dFtrm_c::switchOn(daFTR_c *ftr) {
    if (ftr != NULL) {
        switch (ftr->getFtrFunc()) {
        case 0x27:
            if (ftr != NULL) {
                return fn_16_D740(ftr);
            }
            break;
        case 0x2A:
            if (ftr != NULL) {
                dItem::Item item = ftr->getItem();
                const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
                if (bitm != NULL && (bitm->getHideBone() == 0 || bitm->getHideBone() == 2)) {
                    ftr->mFlags.mShadow = 0;
                }
                return fn_16_122CC(ftr);
            }
            break;
        }
    }
    return FALSE;
}

// 0x4C88
BOOL dFtrm_c::switchOff(daFTR_c *ftr) {
    if (ftr != NULL) {
        switch (ftr->getFtrFunc()) {
        case 0x27:
            if (ftr != NULL) {
                return fn_16_D754(ftr);
            }
            break;
        case 0x2A:
            if (ftr != NULL) {
                ftr->mFlags.mShadow = 1;
                return fn_16_122E0(ftr);
            }
            break;
        }
    }
    return FALSE;
}

// The last items put by the debug menu.
struct ftrLog_c {
    struct entry_c {
        /* 0x0 */ dItem::Item mItem;
        /* 0x2 */ s8 mFlag;
        /* 0x4 */ int mValue;
    }; // size 0x8

    ftrLog_c() { clear(); }

    void clear();
    void print();

    /* 0x00 */ entry_c mEntries[10];
    /* 0x50 */ int mNum;
    /* 0x54 */ f32 mAverage;
    /* 0x58 */ dItem::Item mLastItem;
    /* 0x5C */ int mLastValue;
}; // size 0x60

// 0x4D18
void ftrLog_c::clear() {
    for (entry_c *entry = mEntries; entry < mEntries + ARRAY_SIZE(mEntries); entry++) {
        entry->mItem = dItem::ITEM_ID_NONE;
        entry->mFlag = -1;
        entry->mValue = 0;
    }
    mNum = 0;
    mAverage = 0.0f;
    mLastItem = dItem::Item();
    mLastValue = 0;
}

// Stripped by the linker; leaves its messages in .data.
void ftrLog_c::print() {
    // Unreferenced; only their .rodata slots are known.
    static const int _51C = 20;
    static const int _520 = 30;
    for (int i = 0; i < ARRAY_SIZE(mEntries); i++) {
        entry_c *entry = &mEntries[i];
        if (entry->mItem.isValid()) {
            OSReport("%02d: %s %d", i, entry->mItem.getFgInfo()->mName, entry->mValue);
        } else if (entry->mFlag >= 0) {
            OSReport("%02d: ADD ITEM", i);
        } else {
            OSReport("%02d: empty", i);
        }
    }
    OSReport("AVERAGE %d", (int)mAverage);
    if (mNum == 0) {
        OSReport("Last: \x82\xC8\x82\xB5");
    } else if (mLastItem.isValid()) {
        OSReport("Last: %s %d", mLastItem.getFgInfo()->mName, mLastValue);
    } else {
        OSReport("Last: ADD ITEM %d", mLastValue);
    }
}

// 0x4E38
void dFtrm_c::createCockroach() {
    int room;
    int home = dScene::getSceneAttrTableIndex(dScene::getCurrentScene(), l_lampSceneAttr, &room);
    if (room != -1 && dHome_c::isValidRoom(home)) {
        dBase_c::createBase(fProf::COCKROACH_MANAGER, this, 0, 0);
    }
}

static ftrLog_c l_log;
