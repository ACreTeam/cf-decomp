// The room locators, the manager process and the insect base; the insects are #included in the middle (.text order).
// 0xEB0
void InsectRoomLocator_c::getPos(mVec3_c *pos) const {
    *pos = mPos;
}

// 0xECC
InsectRoomLocatorList_c::InsectRoomLocatorList_c() : mHead(NULL), mCur(NULL) {}

// 0xEDC
InsectRoomLocatorList_c::~InsectRoomLocatorList_c() {}

// 0xF1C
void InsectRoomLocatorList_c::getPos(mVec3_c *pos, const char *name) const {
    InsectRoomLocator_c *node = mHead;
    while (node != NULL) {
        if (strcmp(node->mName, name) == 0) {
            node->getPos(pos);
            return;
        }
        node = node->mNext;
    }
}

// 0xF90
InsectRoomLocatorManager_c::InsectRoomLocatorManager_c()
    : mFlower(NULL), mPond(NULL), mWasteland(NULL), mTropical(NULL), mF1(NULL), mF2(NULL), mF3(NULL),
      mFrmHeap(NULL) {}

// 0xFEC
InsectRoomLocatorManager_c::~InsectRoomLocatorManager_c() {}

// 0x1054
void InsectRoomLocatorManager_c::remove() {
    freeList(&mFlower);
    freeList(&mPond);
    freeList(&mWasteland);
    freeList(&mTropical);
    freeList(&mF1);
    freeList(&mF2);
    freeList(&mF3);
    if (mFrmHeap != NULL) {
        mHeap::destroyFrmHeap(mFrmHeap);
        mFrmHeap = NULL;
    }
}

// 0x10E4
InsectRoomLocatorList_c *InsectRoomLocatorManager_c::setFlower() {
    mList.set(mFlower);
    return &mList;
}

// 0x10F4
InsectRoomLocatorList_c *InsectRoomLocatorManager_c::setPond() {
    mList.set(mPond);
    return &mList;
}

// 0x1104
InsectRoomLocatorList_c *InsectRoomLocatorManager_c::setWasteland() {
    mList.set(mWasteland);
    return &mList;
}

// 0x1114
InsectRoomLocatorList_c *InsectRoomLocatorManager_c::setTropical() {
    mList.set(mTropical);
    return &mList;
}

// 0x1124
InsectRoomLocatorList_c *InsectRoomLocatorManager_c::setF1() {
    mList.set(mF1);
    return &mList;
}

// 0x1134
InsectRoomLocatorList_c *InsectRoomLocatorManager_c::setF2() {
    mList.set(mF2);
    return &mList;
}

// 0x1144
InsectRoomLocatorList_c *InsectRoomLocatorManager_c::setF3() {
    mList.set(mF3);
    return &mList;
}

// 0x1154
void InsectRoomLocatorManager_c::setHeap(EGG::Heap *heap) {
    if (!mAllocator.hasHeap()) {
        mAllocator.attach(heap, 0x20);
    }
}

// 0x11B4
void InsectRoomLocatorManager_c::freeList(InsectRoomLocator_c **list) {
    InsectRoomLocator_c *node = *list;
    while (node != NULL) {
        InsectRoomLocator_c *next = node->mNext;
        mAllocator.free(node);
        node = next;
    }
    *list = NULL;
}

// 0x1224
InsectRoomLocator_c *InsectRoomLocatorManager_c::getLast(InsectRoomLocator_c *node) {
    while (node->mNext != NULL) {
        node = node->mNext;
    }
    return node;
}

// 0x1240
BOOL InsectRoomLocatorManager_c::isBgReady() {
    return dBgUtil::isBgiReady() == TRUE;
}

// 0x126C
void InsectRoomLocatorManager_c::getNodePos(mVec3_c *pos, const char *name) {
    if (!dBgUtil::getNodePos(pos, name, -1, -1)) {
        pos->set(0.0f, 0.0f, 0.0f);
    }
}

namespace dInsectMuseum {

// 0x12C4
void *dInsectMus_c_classInit() {
    return new dInsectMus_c();
}

fProf::fBaseProfile_c g_profile_INSECT_MUSEUM = {&dInsectMus_c_classInit, fProf::INSECT_MUSEUM,
                                                 fProf::DRAW_ORDER::INSECT_MUSEUM};

} // namespace dInsectMuseum

#include "d_ins_mus_beetle.inc"
#include "d_ins_mus_butterfly.inc"
#include "d_ins_mus_cicada.inc"
#include "d_ins_mus_dragonfly.inc"
#include "d_ins_mus_grasshopper.inc"
#include "d_ins_mus_flower.inc"
#include "d_ins_mus_bee.inc"
#include "d_ins_mus_moth.inc"
#include "d_ins_mus_strider.inc"
#include "d_ins_mus_firefly.inc"
#include "d_ins_mus_mole_cricket.inc"
#include "d_ins_mus_flea.inc"
#include "d_ins_mus_pill_bug.inc"
#include "d_ins_mus_ant.inc"
#include "d_ins_mus_tumble_bug.inc"
#include "d_ins_mus_hornet.inc"
#include "d_ins_mus_mosquito.inc"
#include "d_ins_mus_spider.inc"
#include "d_ins_mus_tarantula.inc"
#include "d_ins_mus_diving_beetle.inc"
#include "d_ins_mus_stump.inc"
#include "d_ins_mus_leaf.inc"
#include "d_ins_mus_beetle_battle.inc"

// 0x12F4
dInsMusBrresLoader_c::dInsMusBrresLoader_c() {}

// 0x1330
dInsMusBrresLoader_c::~dInsMusBrresLoader_c() {}

// 0x1390
BOOL dInsMusBrresLoader_c::loadInsect(EGG::Heap *heap, int type) {
    static const u8 pathLen = 0x20;
    static char path[pathLen];
    if (type == INSECT_ANT) {
        snprintf(path, pathLen, "/Insect/m_ins_march.brres");
    } else if (type == dInsectMus_c::BANK_BALL) {
        snprintf(path, pathLen, "/Insect/m_ins_ball.brres");
    } else {
        snprintf(path, pathLen, "/Insect/m_%s.brres", dInsectInfo::getModelName(type));
    }
    return load(path, heap, 0);
}

dInsectMus_c *dInsectMus_c::sInstance;

namespace dInsectMuseum {

InsectRoomLocatorManager_c sLocatorMgr;

} // namespace dInsectMuseum

// 0x1458
dInsectMus_c::dInsectMus_c() : mResHeap(NULL), mInsectList(NULL), mFrame(0) {}

// 0x14D4
dInsectMus_c::~dInsectMus_c() {}

// 0x155C
BOOL dInsectMus_c::createInsect(int type) {
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        for (int slot = 0; slot < SLOT_NUM; slot++) {
            if (!mSlotUsed[slot]) {
                mSlotUsed[slot] = true;
                dInsMusBase_c *insect = sInsectData[type]->mCreate(mChildHeap[slot]);
                mVec3_c pos(0.0f, 0.0f, 0.0f);
                sLocatorMgr.getNodePos(&pos, sInsectPlace[type].mLocator);
                insect->mPos = insect->mHomePos = pos;
                insect->mAngle = insect->mHomeAngle = player->getAngle();
                insect->mAllocator.attach(mChildHeap[slot], 0x20);
                insect->mPos.y += 2.0f;
                insect->mSlot = slot;
                insect->mType = type;
                nw4r::g3d::ResFile res(mBanks[type].getData());
                insect->create(res);
                addInsect(insect);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 0x16EC
BOOL dInsectMus_c::deleteInsect(dInsMusBase_c *insect) {
    if (insect != NULL) {
        int slot = insect->mSlot;
        if (insect->doDelete() == TRUE) {
            mSlotUsed[slot] = false;
            removeInsect(insect);
            insect->~dInsMusBase_c();
            mChildHeap[slot]->free(MEM_FRM_HEAP_FREE_ALL);
            return TRUE;
        }
    }
    return FALSE;
}

// 0x17A0
void dInsectMus_c::deleteAllInsects() {
    dInsMusBase_c *insect = mInsectList;
    do {
        deleteInsect(insect);
        insect = mInsectList;
    } while (insect != NULL);
    dInsMusBeetleBattle_c::getInstance()->freeHeap();
    dInsMusBeetleBattle_c::getInstance()->remove();
}

// 0x17F0
void dInsectMus_c::addInsect(dInsMusBase_c *insect) {
    dInsMusBase_c **link = &mInsectList;
    while (*link != NULL) {
        link = &(*link)->mNext;
    }
    *link = insect;
}

// 0x1810
void dInsectMus_c::removeInsect(dInsMusBase_c *insect) {
    dInsMusBase_c **link = &mInsectList;
    while (*link != insect) {
        link = &(*link)->mNext;
    }
    if (*link == insect) {
        *link = insect->mNext;
    }
}

namespace dInsectMuseum {

// Stripped by the linker; its heap name precedes create()'s in .data.
EGG::FrmHeap *createBattleHeap(EGG::Heap *parent) {
    return mHeap::createFrmHeap(0x1000, parent, "dInsMusBeetleBattle_c::m_heap_p", 0x20, mHeap::OPT_NONE);
}

} // namespace dInsectMuseum

// 0x1838
int dInsectMus_c::create() {
    EGG::Heap *parent = dHeap::localHeap_p;
    if (mResHeap == NULL) {
        mResHeap = mHeap::createFrmHeap(0x140000, parent, "dInsectMus_c::m_ResHeap", 0x20, mHeap::OPT_NONE);
    }
    BOOL ok = TRUE;
    for (u32 i = 0; i < BANK_NUM; i++) {
        if (!mBanks[i].loadInsect(mResHeap, i)) {
            ok = FALSE;
        }
    }
    if (!ok) {
        return NOT_READY;
    }
    if (!sLocatorMgr.create()) {
        return NOT_READY;
    }
    for (u32 i = 0; i < SLOT_NUM; i++) {
        mChildHeap[i] = mHeap::createFrmHeap(0x2000, parent, "dInsectMus_c::m_childHeap", 0x20, mHeap::OPT_NONE);
        mSlotUsed[i] = false;
    }
    sInstance = this;
    sTimeMask = getTimeMask();
    mAreaMgr.init();
    createAllInsects();
    if (isBattleReady() == TRUE && !dInsMusBeetleBattle_c::getInstance()->mIsActive) {
        nw4r::g3d::ResFile beetleRes(mBanks[INSECT_HERCULES_BEETLE].getData());
        nw4r::g3d::ResFile stagRes(mBanks[INSECT_GOLDEN_STAG].getData());
        EGG::FrmHeap *heap =
            mHeap::createFrmHeap(0x1000, parent, "dInsMusBeetleBattle_c::m_heap_p", 0x20, mHeap::OPT_NONE);
        dInsMusBeetleBattle_c::getInstance()->setHeap(heap);
        dInsMusBeetleBattle_c::getInstance()->create(beetleRes, stagRes);
    }
    return SUCCEEDED;
}

// 0x19D8
int dInsectMus_c::execute() {
    mFrame++;
    for (dInsMusBase_c **link = &mInsectList; *link != NULL; link = &(*link)->mNext) {
        (*link)->mSnd.setPos(&(*link)->mPos);
        (*link)->execute();
    }
    if (dInsMusBeetleBattle_c::getInstance()->mIsActive == true) {
        dInsMusBeetleBattle_c::getInstance()->execute();
    }
    mAreaMgr.execute();
    return SUCCEEDED;
}

// 0x1A74
int dInsectMus_c::draw() {
    for (dInsMusBase_c **link = &mInsectList; *link != NULL; link = &(*link)->mNext) {
        (*link)->draw();
    }
    if (dInsMusBeetleBattle_c::getInstance()->mIsActive == true) {
        dInsMusBeetleBattle_c::getInstance()->draw();
    }
    return SUCCEEDED;
}

// 0x1AE0
int dInsectMus_c::doDelete() {
    for (u32 i = 0; i < BANK_NUM; i++) {
        if (!mBanks[i].unload(FALSE)) {
            return NOT_READY;
        }
    }
    if (mResHeap != NULL) {
        mHeap::destroyFrmHeap(mResHeap);
        mResHeap = NULL;
    }
    deleteAllInsects();
    if (dInsMusBeetleBattle_c::getInstance()->mIsActive == true) {
        dInsMusBeetleBattle_c::getInstance()->freeHeap();
        dInsMusBeetleBattle_c::getInstance()->remove();
    }
    for (u32 i = 0; i < SLOT_NUM; i++) {
        if (mChildHeap[i] != NULL) {
            mHeap::destroyFrmHeap(mChildHeap[i]);
            mChildHeap[i] = NULL;
        }
    }
    sLocatorMgr.remove();
    sInstance = NULL;
    return SUCCEEDED;
}

// 0x1BD4
u32 dInsectMus_c::getTimeMask() {
    dTime_c now = *dTime_c::getCurrent();
    int hour = now.hour;
    if (hour >= 23 || (hour >= 0 && hour <= 3)) {
        return INSECT_TIME_MASK(INSECT_TIME_NIGHT);
    }
    if (hour >= 4 && hour <= 7) {
        return INSECT_TIME_MASK(INSECT_TIME_MORNING);
    }
    if (hour >= 8 && hour <= 15) {
        return INSECT_TIME_MASK(INSECT_TIME_DAY);
    }
    if (hour == 16) {
        return INSECT_TIME_MASK(INSECT_TIME_AFTERNOON);
    }
    if (hour >= 17 && hour <= 18) {
        return INSECT_TIME_MASK(INSECT_TIME_EVENING);
    }
    if (hour >= 19 && hour <= 22) {
        return INSECT_TIME_MASK(INSECT_TIME_LATE);
    }
    return INSECT_TIME_MASK(INSECT_TIME_NIGHT);
}

// 0x1CD8
void dInsectMus_c::createAllInsects() {
    dMuseum_c *museum = &dSaveData_c::getTown()->mMuseum;
    u32 battle = isBattleReady();
    for (u32 type = 0; type < INSECT_NUM; type++) {
        dItem::Item item(dItem::ITEM_IDX_COMMON_BUTTERFLY, type, FALSE);
        if (type == INSECT_GOLDEN_STAG || type == INSECT_HERCULES_BEETLE) {
            if (!battle && museum->isDonated(item) == TRUE) {
                createInsect(type);
            }
        } else if (museum->isDonated(item) == TRUE) {
            createInsect(type);
        }
    }
}

// 0x1DB0
u32 dInsectMus_c::isBattleReady() {
    dMuseum_c *museum = &dSaveData_c::getTown()->mMuseum;
    dItem::Item stag(dItem::ITEM_IDX_COMMON_BUTTERFLY, INSECT_GOLDEN_STAG, FALSE);
    dItem::Item beetle(dItem::ITEM_IDX_COMMON_BUTTERFLY, INSECT_HERCULES_BEETLE, FALSE);
    u32 stagDonated = museum->isDonated(stag);
    u32 beetleDonated = museum->isDonated(beetle);
    return stagDonated && beetleDonated;
}

namespace dInsectMuseum {

u32 sTimeMask = INSECT_TIME_MASK(INSECT_TIME_NIGHT);

} // namespace dInsectMuseum

// 0x1E48
dInsMusBase_c::dInsMusBase_c()
    : mHomePos(0.0f, 0.0f, 0.0f), mPos(0.0f, 0.0f, 0.0f), mHomeAngle(0, 0, 0), mAngle(0, 0, 0), mTimer(0),
      mAlpha(0xFF), mNext(NULL), mShadowSize(0.0f), mScareRadius(0.0f), mCreated(false),
      mBgFlags(dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_SET_POS), mSlot(0), mType(0), mCheckBg(true) {}

// 0x1F94
dInsMusBase_c::~dInsMusBase_c() {}

// 0x2040
void *dInsMusBase_c::operator new(size_t size, EGG::Heap *heap) {
    void *p = EGG::Heap::alloc(size, 4, heap);
    if (p == NULL) {
        return NULL;
    }
    cLib::memSet(p, 0, size);
    return p;
}

// 0x20A0
void dInsMusBase_c::calcMtx() {
    setAngleMtx();
}

// 0x20A4
void dInsMusBase_c::setAlpha(u8 alpha) {
    mMdl.setDrawMode(nw4r::g3d::RESMDL_DRAWMODE_SORT_XLU_Z);
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat("m1");
    if (mat.IsValid()) {
        nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mMdl.getScn());
        nw4r::g3d::ScnMdl::CopiedMatAccess access(scnMdl, mat.ptr()->id);
        if (access.IsValid()) {
            GXColor color;
            nw4r::g3d::ResMatTevColor tev = access.GetResMatTevColor();
            tev.GXGetTevKColor(GX_KCOLOR1, &color);
            color.a = alpha;
            tev.GXSetTevKColor(GX_KCOLOR1, color);
            tev.DCStore(false);
        }
    }
}

// 0x21CC
BOOL dInsMusBase_c::create(nw4r::g3d::ResFile &res) {
    mShadowSize = getData()->mShadowSize;
    mScareRadius = getData()->mScareRadius;
    mScare.init();
    mScare.setLimit(getData()->mScareLimit);
    mScare.setEscapeLimit(getData()->mEscapeLimit);
    mBgRadius = mShadowSize;
    createModel(res);
    init();
    initState();
    mAcch.init();
    return TRUE;
}

// 0x22A8
BOOL dInsMusBase_c::execute() {
    mPlayerCheck.setPos(mPos);
    mVec3_c oldPos = mPos;
    executeState();
    if (mCheckBg == true) {
        mAcch.check(mBgRadius, &mPos, &oldPos, mAngle.y, 0, mBgFlags, TRUE);
    }
    setAlpha(mAlpha);
    mTimer++;
    if (mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT, mScareRadius) == true) {
        mScare.add(mPlayerCheck.getPlayerSpeed(dPlayerMgr::PLAYER_INDEX_CURRENT), 1.0f);
    } else {
        mScare.add(0.0f, 1.0f);
    }
    mMdl.play();
    return FALSE;
}

// 0x23C0
BOOL dInsMusBase_c::draw() {
    mMdl.calc(false);
    calcMtx();
    mMdl.entry();
    if (!isZero(mShadowSize)) {
        f32 alpha = 0.5f * (mAlpha / 255.0f);
        mAng angle(0);
        dShadow::requestRound(&mPos, 0, &angle, mShadowSize, 1.0f, alpha, 0.0f, 1.0f);
    }
    return TRUE;
}

// 0x24A8
BOOL dInsMusBase_c::doDelete() {
    if (mCreated == true) {
        mMdl.remove();
        mAnmChr.remove();
        mAnmVis.remove();
    }
    return TRUE;
}

// 0x2518
void dInsMusBase_c::createModel(nw4r::g3d::ResFile &res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    if (res.HasResAnmVis() == true) {
        mMdl.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR | nw4r::g3d::ScnMdl::ANM_VIS, 1, NULL);
    } else {
        mMdl.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR, 1, NULL);
    }
    mAnmChrWait = res.GetResAnmChr("wait");
    mAnmChrFly = res.GetResAnmChr("fly");
    mAnmChrStop = res.GetResAnmChr("stop");
    if (mAnmChrWait.IsValid()) {
        mAnmChr.create(mdl, mAnmChrWait, &mAllocator, NULL);
    } else if (mAnmChrFly.IsValid()) {
        mAnmChr.create(mdl, mAnmChrFly, &mAllocator, NULL);
    }
    if (res.HasResAnmVis() == true) {
        mAnmVisWait = res.GetResAnmVis("wait");
        mAnmVisFly = res.GetResAnmVis("fly");
        mAnmVisStop = res.GetResAnmVis("stop");
        if (mAnmVisWait.IsValid()) {
            mAnmVis.create(mdl, mAnmVisWait, &mAllocator, NULL);
        } else {
            mAnmVis.create(mdl, mAnmVisFly, &mAllocator, NULL);
        }
    }
    mCreated = true;
}

// 0x26FC
void dInsMusBase_c::setAnmChr(const nw4r::g3d::ResAnmChr &anm, f32 blend) {
    if (anm.IsValid()) {
        mAnmChr.setAnm(mMdl, anm, m3d::PLAYMODE_INHERIT);
        mMdl.setAnm(mAnmChr, blend);
    }
}

// 0x2764
void dInsMusBase_c::setAnmVis(const nw4r::g3d::ResAnmVis &anm) {
    if (anm.IsValid()) {
        mAnmVis.setAnm(mMdl, anm, m3d::PLAYMODE_INHERIT);
        mMdl.setAnm(mAnmVis);
    }
}

// 0x27C8
void dInsMusBase_c::setAnmRate(f32 rate) {
    if (mAnmChr.IsBound() == true) {
        mAnmChr.setRate(rate);
    }
    if (mAnmVis.IsBound() == true) {
        mAnmVis.setRate(rate);
    }
}

// 0x2834
void dInsMusBase_c::setAnmFrame(f32 frame) {
    if (mAnmChr.IsBound() == true) {
        mAnmChr.setFrame(frame);
    }
    if (mAnmVis.IsBound() == true) {
        mAnmVis.setFrame(frame);
    }
}

// 0x28A0
bool dInsMusBase_c::isUnderGround() {
    return mPos.y < getGroundY();
}

// 0x28DC
f32 dInsMusBase_c::getGroundY() {
    return dBGCF::getGroundY(&mPos, FALSE);
}

// 0x28E8
const dInsMusData_c *dInsMusBase_c::getData() {
    return sInsectData[mType];
}

// 0x2900
bool dInsMusBase_c::isActiveTime() const {
    u8 mask = dInsectInfo::getActiveTimeMask(mType);
    return (mask & sTimeMask) != 0;
}

// 0x2940
const char *dInsMusBase_c::getLocatorName() {
    return sInsectPlace[mType].mLocator;
}

// 0x2958
void dInsMusBase_c::setScale(f32 scale) {
    mVec3_c s(scale, scale, scale);
    mMdl.setScale(s);
}

// 0x298C
void dInsMusBase_c::setAngleMtx() {
    mMtx_c mtx;
    mMtx_c rotY;
    mMtx_c rotX;
    mMtx_c rotZ;
    rotX.XrotS(mAngle.x);
    rotY.YrotS(mAngle.y);
    rotZ.ZrotS(mAngle.z);
    dActor_c::makeMtx(&mtx, &mPos, 0);
    PSMTXConcat(mtx, rotY, mtx);
    PSMTXConcat(mtx, rotX, mtx);
    PSMTXConcat(mtx, rotZ, mtx);
    mMdl.setLocalMtx(&mtx);
}

// 0x2A44
void dInsMusBase_c::setAxisMtx() {
    mMtx_c mtx;
    mtx = getAxisMtx();
    mtx.m[0][3] = mPos.x;
    mtx.m[1][3] = mPos.y;
    mtx.m[2][3] = mPos.z;
    mMdl.setLocalMtx(&mtx);
}

// 0x2AFC
void dInsMusBase_c::flyToTarget(f32 liftY, f32 speed, f32 brake, f32 turnRate) {
    mVec3_c dir = mTargetPos - mPos;
    dir.y = 0.0f;
    dir.normalize();
    mVec3_c front = mAxis.getFront();
    mVec3_c turn(0.0f, 0.0f, 0.0f);
    if (dotXZ(front, dir) < -0.8f) {
        turn = mAxis.getSide();
    } else {
        turn = dir - front;
    }
    front += turn * turnRate;
    front.normalize();
    mAxis.setFront(front);
    mVec3_c move = front * speed;
    mSpeed *= brake;
    mSpeed.y += liftY;
    mSpeed += move;
    mPos += mSpeed;
}

// 0x2D0C
InsectRoomLocatorList_c *dInsMusBase_c::getRoomLocators() {
    switch (sInsectPlace[mType].mRoom) {
    case ROOM_FLOWER:
        return sLocatorMgr.setFlower();
    case ROOM_POND:
        return sLocatorMgr.setPond();
    case ROOM_WASTELAND:
        return sLocatorMgr.setWasteland();
    case ROOM_TROPICAL:
        return sLocatorMgr.setTropical();
    case ROOM_F1:
        return sLocatorMgr.setF1();
    case ROOM_F2:
        return sLocatorMgr.setF2();
    case ROOM_F3:
        return sLocatorMgr.setF3();
    }
    return NULL;
}

// 0x2DA0
void dInsMusBase_c::getLocatorPos(mVec3_c *pos) {
    InsectRoomLocatorList_c *locators = getRoomLocators();
    locators->getPos(pos, getLocatorName());
}

namespace dInsectMuseum {

const char l_monsiro[] = "ins_monsiro";
const char l_monki[] = "ins_monki";
const char l_ageha[] = "ins_ageha";
const char l_karasu[] = "ins_karasu";
const char l_ookaba[] = "ins_ookaba";
const char l_morpho[] = "ins_morpho";
const char l_miiro[] = "ins_miiro";
const char l_akaeri[] = "ins_akaeri";
const char l_alex[] = "ins_alex";
const char l_ga[] = "ins_ga";
const char l_ymmga[] = "ins_ymmga";
const char l_mitsu[] = "ins_mitsu";
const char l_suzume[] = "ins_suzume";
const char l_syouryou[] = "ins_syouryou";
const char l_tonosama[] = "ins_tonosama";
const char l_kama[] = "ins_kama";
const char l_hanakama[] = "ins_hanakama";
const char l_abura[] = "ins_abura";
const char l_minmin[] = "ins_minmin";
const char l_tsuku[] = "ins_tsuku";
const char l_higu[] = "ins_higu";
const char l_biwa[] = "ins_biwa";
const char l_akane[] = "ins_akane";
const char l_ginyan[] = "ins_ginyan";
const char l_oniyan[] = "ins_oniyan";
const char l_mukashi[] = "ins_mukashi";
const char l_ari[] = "ins_ari";
const char l_amenbo[] = "ins_amenbo";
const char l_gengorou[] = "ins_gengorou";
const char l_kata[] = "ins_kata";
const char l_koorogi[] = "ins_koorogi";
const char l_suzu[] = "ins_suzu";
const char l_kirigiri[] = "ins_kirigiri";
const char l_okera[] = "ins_okera";
const char l_konoha[] = "ins_konoha";
const char l_nana[] = "ins_nana";
const char l_mino[] = "ins_mino";
const char l_tentou[] = "ins_tentou";
const char l_violin[] = "ins_violin";
const char l_kamikiri[] = "ins_kamikiri";
const char l_hunkoro[] = "ins_hunkoro";
const char l_hotaru[] = "ins_hotaru";
const char l_kogane[] = "ins_kogane";
const char l_plakoga[] = "ins_plakoga";
const char l_tama[] = "ins_tama";
const char l_miyakwa[] = "ins_miyakwa";
const char l_nokokwa[] = "ins_nokokwa";
const char l_ookwa[] = "ins_ookwa";
const char l_nijikwa[] = "ins_nijikwa";
const char l_hosokwa[] = "ins_hosokwa";
const char l_ougon[] = "ins_ougon";
const char l_kbt[] = "ins_kbt";
const char l_kokbt[] = "ins_kokbt";
const char l_zokbt[] = "ins_zokbt";
const char l_helkbt[] = "ins_helkbt";
const char l_gorias[] = "ins_gorias";
const char l_nomi[] = "ins_nomi";
const char l_dango[] = "ins_dango";
const char l_ka[] = "ins_ka";
const char l_hae[] = "ins_hae";
const char l_mukade[] = "ins_mukade";
const char l_kumo[] = "ins_kumo";
const char l_taran[] = "ins_taran";
const char l_sasori[] = "ins_sasori";

dInsMusData_c sInsectDataTable[INSECT_NUM] = {
    {createInsect<dInsMusButterfly_c, INSECT_COMMON_BUTTERFLY>, l_monsiro, 150, 0, 80.0f, 6.0f},
    {createInsect<dInsMusButterfly_c, INSECT_YELLOW_BUTTERFLY>, l_monki, 150, 0, 80.0f, 6.0f},
    {createInsect<dInsMusButterfly_c, INSECT_TIGER_BUTTERFLY>, l_ageha, 120, 0, 100.0f, 8.0f},
    {createInsect<dInsMusButterfly_c, INSECT_PEACOCK_BUTTERFLY>, l_karasu, 120, 0, 100.0f, 8.0f},
    {createInsect<dInsMusButterfly_c, INSECT_MONARCH_BUTTERFLY>, l_ookaba, 120, 0, 80.0f, 7.0f},
    {createInsect<dInsMusButterfly_c, INSECT_EMPEROR_BUTTERFLY>, l_morpho, 80, 0, 100.0f, 9.0f},
    {createInsect<dInsMusButterfly_c, INSECT_AGRIAS_BUTTERFLY>, l_miiro, 120, 0, 100.0f, 8.0f},
    {createInsect<dInsMusButterfly_c, INSECT_RAJA_BROOKE>, l_akaeri, 120, 0, 150.0f, 8.0f},
    {createInsect<dInsMusButterfly_c, INSECT_BIRDWING_BUTTERFLY>, l_alex, 80, 0, 220.0f, 13.0f},
    {createInsect<dInsMusMoth_c, INSECT_MOTH>, l_ga, 400, 0, 50.0f, 6.5f},
    {createInsect<dInsMusBeetle_c, INSECT_OAK_SILK_MOTH>, l_ymmga, 80, 0, 120.0f, 12.0f},
    {createInsect<dInsMusBee_c, INSECT_HONEYBEE>, l_mitsu, 200, 0, 50.0f, 3.0f},
    {createInsect<dInsMusHornet_c, INSECT_BEE>, l_suzume, 0, 0, 0.0f, 10.0f},
    {createInsect<dInsMusGrasshopper_c, INSECT_LONG_LOCUST>, l_syouryou, 100, 0, 80.0f, 5.0f},
    {createInsect<dInsMusGrasshopper_c, INSECT_MIGRATORY_LOCUST>, l_tonosama, 70, 0, 80.0f, 6.0f},
    {createInsect<dInsMusFlower_c, INSECT_MANTIS>, l_kama, 20, 400, 80.0f, 4.0f},
    {createInsect<dInsMusFlower_c, INSECT_ORCHID_MANTIS>, l_hanakama, 20, 300, 80.0f, 3.0f},
    {createInsect<dInsMusCicada_c, INSECT_BROWN_CICADA>, l_abura, 40, 200, 60.0f, 5.0f},
    {createInsect<dInsMusCicada_c, INSECT_ROBUST_CICADA>, l_minmin, 40, 200, 60.0f, 5.0f},
    {createInsect<dInsMusCicada_c, INSECT_WALKER_CICADA>, l_tsuku, 40, 200, 60.0f, 5.0f},
    {createInsect<dInsMusCicada_c, INSECT_EVENING_CICADA>, l_higu, 40, 200, 60.0f, 5.0f},
    {createInsect<dInsMusCicada_c, INSECT_LANTERN_FLY>, l_biwa, 30, 80, 120.0f, 7.5f},
    {createInsect<dInsMusDragonfly_c, INSECT_RED_DRAGONFLY>, l_akane, 100, 0, 80.0f, 5.0f},
    {createInsect<dInsMusDragonfly_c, INSECT_DARNER_DRAGONFLY>, l_ginyan, 100, 0, 80.0f, 6.0f},
    {createInsect<dInsMusDragonfly_c, INSECT_BANDED_DRAGONFLY>, l_oniyan, 100, 0, 120.0f, 8.0f},
    {createInsect<dInsMusDragonfly_c, INSECT_GIANT_PETALTAIL_DRAGONFLY>, l_mukashi, 100, 0, 200.0f, 11.0f},
    {createInsect<dInsMusAnt_c, INSECT_ANT>, l_ari, 0, 0, 0.0f, 0.0f},
    {createInsect<dInsMusStrider_c, INSECT_POND_SKATER>, l_amenbo, 0, 0, 0.0f, 0.0f},
    {createInsect<dInsMusDivingBeetle_c, INSECT_DIVING_BEETLE>, l_gengorou, 0, 0, 0.0f, 0.0f},
    {createInsect<dInsMusFlower_c, INSECT_SNAIL>, l_kata, 100, 400, 40.0f, 4.0f},
    {createInsect<dInsMusGrasshopper_c, INSECT_CRICKET>, l_koorogi, 30, 100, 80.0f, 3.5f},
    {createInsect<dInsMusGrasshopper_c, INSECT_BELL_CRICKET>, l_suzu, 30, 100, 80.0f, 4.0f},
    {createInsect<dInsMusGrasshopper_c, INSECT_GRASSHOPPER>, l_kirigiri, 30, 100, 80.0f, 5.0f},
    {createInsect<dInsMusMoleCricket_c, INSECT_MOLE_CRICKET>, l_okera, 0, 0, 0.0f, 5.0f},
    {createInsect<dInsMusLeaf_c, INSECT_WALKING_LEAF>, l_konoha, 5, 0, 50.0f, 7.0f},
    {createInsect<dInsMusBeetle_c, INSECT_WALKING_STICK>, l_nana, 200, 0, 100.0f, 4.0f},
    {createInsect<dInsMusSpider_c, INSECT_BAGWORM>, l_mino, 0, 0, 0.0f, 0.0f},
    {createInsect<dInsMusFlower_c, INSECT_LADYBUG>, l_tentou, 400, 0, 40.0f, 4.5f},
    {createInsect<dInsMusStump_c, INSECT_VIOLIN_BEETLE>, l_violin, 200, 0, 100.0f, 4.0f},
    {createInsect<dInsMusStump_c, INSECT_LONGHORN_BEETLE>, l_kamikiri, 200, 0, 100.0f, 4.0f},
    {createInsect<dInsMusTumbleBug_c, INSECT_DUNG_BEETLE>, l_hunkoro, 150, 0, 100.0f, 6.0f},
    {createInsect<dInsMusFirefly_c, INSECT_FIREFLY>, l_hotaru, 70, 0, 80.0f, 7.0f},
    {createInsect<dInsMusBeetle_c, INSECT_DRONE_BEETLE>, l_kogane, 200, 0, 60.0f, 5.0f},
    {createInsect<dInsMusBeetle_c, INSECT_SCARAB_BEETLE>, l_plakoga, 80, 0, 90.0f, 5.0f},
    {createInsect<dInsMusBeetle_c, INSECT_JEWEL_BEETLE>, l_tama, 150, 0, 90.0f, 4.5f},
    {createInsect<dInsMusBeetle_c, INSECT_MIYAMA_STAG>, l_miyakwa, 200, 0, 60.0f, 6.5f},
    {createInsect<dInsMusBeetle_c, INSECT_SAW_STAG>, l_nokokwa, 150, 0, 90.0f, 6.5f},
    {createInsect<dInsMusBeetle_c, INSECT_GIANT_STAG>, l_ookwa, 100, 0, 90.0f, 7.0f},
    {createInsect<dInsMusBeetle_c, INSECT_RAINBOW_STAG>, l_nijikwa, 100, 0, 90.0f, 6.0f},
    {createInsect<dInsMusBeetle_c, INSECT_CYCLOMMATUS_STAG>, l_hosokwa, 100, 0, 90.0f, 7.5f},
    {createInsect<dInsMusBeetle_c, INSECT_GOLDEN_STAG>, l_ougon, 80, 0, 90.0f, 7.0f},
    {createInsect<dInsMusBeetle_c, INSECT_DYNASTID_BEETLE>, l_kbt, 150, 0, 90.0f, 7.0f},
    {createInsect<dInsMusBeetle_c, INSECT_ATLAS_BEETLE>, l_kokbt, 100, 0, 90.0f, 8.0f},
    {createInsect<dInsMusBeetle_c, INSECT_ELEPHANT_BEETLE>, l_zokbt, 100, 0, 90.0f, 7.5f},
    {createInsect<dInsMusBeetle_c, INSECT_HERCULES_BEETLE>, l_helkbt, 80, 0, 90.0f, 8.0f},
    {createInsect<dInsMusBeetle_c, INSECT_GOLIATH_BEETLE>, l_gorias, 120, 0, 90.0f, 8.0f},
    {createInsect<dInsMusFlea_c, INSECT_FLEA>, l_nomi, 0, 0, 0.0f, 0.0f},
    {createInsect<dInsMusPillBug_c, INSECT_PILL_BUG>, l_dango, 50, 0, 64.0f, 3.0f},
    {createInsect<dInsMusMosquito_c, INSECT_MOSQUITO>, l_ka, 0, 0, 0.0f, 0.0f},
    {createInsect<dInsMusBee_c, INSECT_FLY>, l_hae, 100, 0, 100.0f, 3.0f},
    {createInsect<dInsMusPillBug_c, INSECT_CENTIPEDE>, l_mukade, 70, 0, 100.0f, 3.0f},
    {createInsect<dInsMusSpider_c, INSECT_SPIDER>, l_kumo, 0, 0, 0.0f, 0.0f},
    {createInsect<dInsMusTarantula_c, INSECT_TARANTULA>, l_taran, 40, 80, 120.0f, 10.0f},
    {createInsect<dInsMusTarantula_c, INSECT_SCORPION>, l_sasori, 40, 80, 120.0f, 10.0f},
};

const dInsMusData_c *sInsectData[INSECT_NUM] = {
    &sInsectDataTable[0], &sInsectDataTable[1], &sInsectDataTable[2], &sInsectDataTable[3],
    &sInsectDataTable[4], &sInsectDataTable[5], &sInsectDataTable[6], &sInsectDataTable[7],
    &sInsectDataTable[8], &sInsectDataTable[9], &sInsectDataTable[10], &sInsectDataTable[11],
    &sInsectDataTable[12], &sInsectDataTable[13], &sInsectDataTable[14], &sInsectDataTable[15],
    &sInsectDataTable[16], &sInsectDataTable[17], &sInsectDataTable[18], &sInsectDataTable[19],
    &sInsectDataTable[20], &sInsectDataTable[21], &sInsectDataTable[22], &sInsectDataTable[23],
    &sInsectDataTable[24], &sInsectDataTable[25], &sInsectDataTable[26], &sInsectDataTable[27],
    &sInsectDataTable[28], &sInsectDataTable[29], &sInsectDataTable[30], &sInsectDataTable[31],
    &sInsectDataTable[32], &sInsectDataTable[33], &sInsectDataTable[34], &sInsectDataTable[35],
    &sInsectDataTable[36], &sInsectDataTable[37], &sInsectDataTable[38], &sInsectDataTable[39],
    &sInsectDataTable[40], &sInsectDataTable[41], &sInsectDataTable[42], &sInsectDataTable[43],
    &sInsectDataTable[44], &sInsectDataTable[45], &sInsectDataTable[46], &sInsectDataTable[47],
    &sInsectDataTable[48], &sInsectDataTable[49], &sInsectDataTable[50], &sInsectDataTable[51],
    &sInsectDataTable[52], &sInsectDataTable[53], &sInsectDataTable[54], &sInsectDataTable[55],
    &sInsectDataTable[56], &sInsectDataTable[57], &sInsectDataTable[58], &sInsectDataTable[59],
    &sInsectDataTable[60], &sInsectDataTable[61], &sInsectDataTable[62], &sInsectDataTable[63],
};

// Unused; anmChrPart_c's vtable puts anmChr_c::getType before anmVis_c::getType among the weak functions.
static void unusedPartAnm() {
    m3d::anmChrPart_c anm;
}

} // namespace dInsectMuseum
