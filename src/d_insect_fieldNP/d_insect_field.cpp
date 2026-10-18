// dInsectField_c: the insects in the town (REL d_insect_fieldNP). See
// include/game/game/d_insect_field.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_insect_field.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_fi_mdl_mgr.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/game/d_bk_unit_search_cand.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_a_snowman.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_a_str.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_fgobj_manager.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_heap.hpp>
#include <revolution/OS.h>
#include <string.h>
#include <stdio.h>
#include <game/game/d_shadow.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_world.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_str_manager.hpp>
#include <game/mLib/m_mtx.hpp>
// The kind classes, in .text order (their strong vtables are in reverse).
#include <game/game/d_ins_fld_butterfly.hpp>
#include <game/game/d_ins_fld_beetle.hpp>
#include <game/game/d_ins_fld_grasshopper.hpp>
#include <game/game/d_ins_fld_flower.hpp>
#include <game/game/d_ins_fld_cicada.hpp>
#include <game/game/d_ins_fld_dragonfly.hpp>
#include <game/game/d_ins_fld_moth.hpp>
#include <game/game/d_ins_fld_bee.hpp>
#include <game/game/d_ins_fld_hornet.hpp>
#include <game/game/d_ins_fld_ant.hpp>
#include <game/game/d_ins_fld_strider.hpp>
#include <game/game/d_ins_fld_diving_beetle.hpp>
#include <game/game/d_ins_fld_mole_cricket.hpp>
#include <game/game/d_ins_fld_tumble_bug.hpp>
#include <game/game/d_ins_fld_firefly.hpp>
#include <game/game/d_ins_fld_flea.hpp>
#include <game/game/d_ins_fld_pill_bug.hpp>
#include <game/game/d_ins_fld_mosquito.hpp>
#include <game/game/d_ins_fld_spider.hpp>
#include <game/game/d_ins_fld_tarantula.hpp>
#include <game/game/d_ins_fld_stump.hpp>
#include <game/game/d_ins_fld_leaf.hpp>
// Defined after the kinds: its vtable is the first one in .data.
#include <game/game/d_bk_attr_search_cand.hpp>

// .rodata 0: the firefly glow's offset from its root node (executeHold).
static const Vec l_glowOfs = {0.0f, -3.0f, 6.0f};

namespace dInsectField {

void *dInsectField_c_classInit(); // 0xD8

fProf::fBaseProfile_c g_profile_INSECT_FIELD = {
    &dInsectField_c_classInit, fProf::INSECT_FIELD,
    fProf::DRAW_ORDER::INSECT_FIELD};

} // namespace dInsectField

// The kinds, in .text order (see build/insect_work/core_notes.md "Source
// layout"): their data precedes the manager's, their code gets one -sym section
// each, and create (new string literals) then lands after Leaf's code.
#include "d_ins_fld_butterfly.inc"
#include "d_ins_fld_beetle.inc"
#include "d_ins_fld_grasshopper.inc"
#include "d_ins_fld_flower.inc"
#include "d_ins_fld_cicada.inc"
#include "d_ins_fld_dragonfly.inc"
#include "d_ins_fld_moth.inc"
#include "d_ins_fld_bee.inc"
#include "d_ins_fld_hornet.inc"
#include "d_ins_fld_ant.inc"
#include "d_ins_fld_strider.inc"
#include "d_ins_fld_diving_beetle.inc"
#include "d_ins_fld_mole_cricket.inc"
#include "d_ins_fld_tumble_bug.inc"
#include "d_ins_fld_firefly.inc"
#include "d_ins_fld_flea.inc"
#include "d_ins_fld_pill_bug.inc"
#include "d_ins_fld_mosquito.inc"
#include "d_ins_fld_spider.inc"
#include "d_ins_fld_tarantula.inc"
#include "d_ins_fld_stump.inc"
#include "d_ins_fld_leaf.inc"

namespace dInsectField {

// 0xCC
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

void *dInsectField_c_classInit() {
    return new dInsectField_c();
}

// 0x108
void heapList_c::create(ulong size, EGG::Heap *parent, int num,
                        const char *name) {
    mHeaps = new (parent, 4) EGG::FrmHeap *[num];
    mUsed = new (parent, 4) u8[num];
    mNum = num;
    for (u32 i = 0; i < mNum; i++) {
        mHeaps[i] =
            mHeap::createFrmHeap(size, parent, name, 0x20, mHeap::OPT_NONE);
        mUsed[i] = 0;
    }
}

// 0x1C0
void heapList_c::destroy() {
    for (u32 i = 0; i < mNum; i++) {
        if (mHeaps[i] != NULL) {
            mHeap::destroyFrmHeap(mHeaps[i]);
            mHeaps[i] = NULL;
        }
    }
    delete[] mHeaps;
    delete[] mUsed;
}

} // namespace dInsectField

// 0x250
dInsectField_c::dInsectField_c()
    : mSpawnTimer(SPAWN_INTERVAL), mCreated(0), mDeleting(0) {
}

// 0x4B0
dInsectField_c::~dInsectField_c() {
}

// 0x25EE4 (in the trailing section: it makes new string literals, see
// notes/matching_tips.md "-sym on: string literals")
int dInsectField_c::create() {
    if (dScene::getCurrentScene() == dScene::SCENE_CHECK_FIELD) {
        deleteRequest();
        return SUCCEEDED;
    }
    if (dWeather_c::sInstance == NULL) {
        return NOT_READY;
    }
    if (!mCreated) {
        EGG::FrmHeap *heap = mHeap::createFrmHeap(
            0x69400, dHeap::localHeap_p, "dInsectField_c::m_parentHeap", 0x20,
            mHeap::OPT_NONE);
        m_parentHeap = heap;
        heap->getTotalFreeSize();
        for (int i = 0; i < INSECT_FIELD_BANK_NUM; i++) {
            m_ResHeap[i] =
                mHeap::createFrmHeap(0x5800, heap, "dInsectField_c::m_ResHeap",
                                     0x20, mHeap::OPT_NONE);
            mBanks[i].setHeap(m_ResHeap[i]);
        }
        dInsFldBank_c::sBanks = mBanks;
        dInsFldBank_c::sBankNum = INSECT_FIELD_BANK_NUM;
        mNextId = 0;
        mActive = NULL;
        mPending = NULL;
        mDying = NULL;
        mNetInsects[0] = NULL;
        mNetInsects[1] = NULL;
        mNetInsects[2] = NULL;
        mNetInsects[3] = NULL;
        m_objHeap.create(0x1000, heap, INSECT_NET_SLOT_NUM,
                         "dInsectField_c::m_objHeap");
        m_objHeapPermanent.create(0x1400, heap, INSECT_FIELD_PERMANENT_NUM,
                                  "dInsectField_c::m_objHeapPermanent");
        m_objHeapPlayer.create(0x1000, heap, INSECT_UTIL_PLAYER_NUM,
                               "dInsectField_c::m_objHeapPlayer");
        int w = dField::getInfo(FD_ID_CURRENT)->getBlockW();
        int h = dField::getInfo(FD_ID_CURRENT)->getBlockH();
        for (int i = 0; i < INSECT_FIELD_SEARCH_NUM; i++) {
            mSearch[i] = new (heap, 4) dFdBkSearchCand_c(w, h);
            mSearch[i]->setAll();
            mSearch[i]->removeBorder();
        }
        g_playActorMng = reinterpret_cast<dPlayActorMng_c *>(this);
        initSpawn();
        mCreated = TRUE;
    }
    if (createPending()) {
        lbl_8074E510 = dInsectField::recvCatchMsg;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// 0x52C
int dInsectField_c::execute() {
    BOOL owner = TRUE;
    BOOL becameOwner = FALSE;
    if (dNet::isSessionActive() && !dNet::isSceneOwner()) {
        owner = FALSE;
        initSlots();
    }
    if (dNet::isSessionActive() && !mWasOwner && owner) {
        initSlots();
        becameOwner = TRUE;
    }
    mWasOwner = owner;
    dInsFldBase_c *insect;
    dInsFldBase_c *next;
    insect = mActive;
    while (insect != NULL) {
        next = insect->mNext;
        if (!owner && insect->mHeapList == &m_objHeap) {
            dInsectCatchData_c *data = insect->getCatchData();
            if (data->getReq() == INSECT_FIELD_REQ_HELD && !insect->isHeld() &&
                !insect->isDeleteRequested()) {
                insect->setHeld(data->getState());
            }
        }
        if (!owner && isNetChanged(insect)) {
            removeInsect(insect);
            insect = next;
            continue;
        }
        if (insect->isHeld() && !insect->mHoldStarted) {
            insect->mHoldStarted = TRUE;
            insect->onHold();
        }
        if (becameOwner) {
            insect->onBecomeOwner();
        }
        insect->mSound.setPos(&insect->mPos);
        dInsFldBase_c::ProcFunc proc = insect->mHoldExecute;
        if (proc) {
            (insect->*proc)();
        } else if (insect->mActive && !insect->isDeleteRequested()) {
            insect->execute();
        }
        if (insect->isDeleteReady() || isLifeOver(insect)) {
            removeInsect(insect);
        }
        insect = next;
    }
    calcSpawn();
    createPending();
    destroyAllDying();
    dInsectInfo::clearPos();
    if (dNet::isSessionActive() && dNet::isSceneOwner()) {
        sendSlots();
    }
    return SUCCEEDED;
}

// 0x7A0
int dInsectField_c::draw() {
    for (dInsFldBase_c *insect = mActive; insect != NULL;
         insect = insect->mNext) {
        if (dInsectField::isInView(&dCamera_c::sCenter, &insect->mPos)) {
            dInsFldBase_c::ProcFunc proc = insect->mHoldDraw;
            if (proc) {
                (insect->*proc)();
            } else if (!insect->isDeleteRequested()) {
                insect->draw();
            }
        }
    }
    return SUCCEEDED;
}

// 0x85C
int dInsectField_c::doDelete() {
    if (dScene::getCurrentScene() == dScene::SCENE_CHECK_FIELD) {
        return SUCCEEDED;
    }
    if (!mDeleting) {
        if (!createPending()) {
            return NOT_READY;
        }
        if (!deleteAll()) {
            return NOT_READY;
        }
        mDeleting = TRUE;
    }
    if (!destroyAllDying()) {
        return NOT_READY;
    }
    for (int i = 0; i < INSECT_FIELD_BANK_NUM; i++) {
        if (m_ResHeap[i] != NULL) {
            mHeap::destroyFrmHeap(m_ResHeap[i]);
            m_ResHeap[i] = NULL;
        }
    }
    for (int i = 0; i < INSECT_FIELD_SEARCH_NUM; i++) {
        if (mSearch[i] != NULL) {
            delete mSearch[i];
            mSearch[i] = NULL;
        }
    }
    m_objHeap.destroy();
    m_objHeapPermanent.destroy();
    m_objHeapPlayer.destroy();
    m_parentHeap->destroy();
    lbl_8074E510 = NULL;
    g_playActorMng = NULL;
    return SUCCEEDED;
}

// 0x9B4
void dInsectField_c::addActive(dInsFldBase_c *insect) {
    dInsFldBase_c **link = &mActive;
    while (*link != NULL) {
        link = &(*link)->mNext;
    }
    *link = insect;
    insect->mNext = NULL;
    insect->mId = mNextId++;
}

// 0x9EC
void dInsectField_c::removeActive(dInsFldBase_c *insect) {
    dInsFldBase_c **link = &mActive;
    while (*link != insect) {
        link = &(*link)->mNext;
    }
    if (*link == insect) {
        *link = insect->mNext;
        insect->mNext = NULL;
    }
}

// 0xA1C
void dInsectField_c::addPending(dInsFldBase_c *insect) {
    dInsFldBase_c **link = &mPending;
    while (*link != NULL) {
        link = &(*link)->mNext;
    }
    *link = insect;
    insect->mNext = NULL;
}

// 0xA44
void dInsectField_c::removePending(dInsFldBase_c *insect) {
    dInsFldBase_c **link = &mPending;
    while (*link != insect) {
        link = &(*link)->mNext;
    }
    if (*link == insect) {
        *link = insect->mNext;
        insect->mNext = NULL;
    }
}

// 0xA74
void dInsectField_c::addDying(dInsFldBase_c *insect) {
    dInsFldBase_c **link = &mDying;
    while (*link != NULL) {
        link = &(*link)->mNext;
    }
    *link = insect;
    insect->mNext = NULL;
}

// 0xA9C
void dInsectField_c::removeDying(dInsFldBase_c *insect) {
    dInsFldBase_c **link = &mDying;
    while (*link != insect) {
        link = &(*link)->mNext;
    }
    if (*link == insect) {
        *link = insect->mNext;
        insect->mNext = NULL;
    }
}

// 0xACC
BOOL dBkAttrSearchCand_c::check(int x, int z) {
    return dField::getInfo(FD_ID_CURRENT)->getBlock(x, z)->hasFlag(mMask);
}

// 0xB28
BOOL dInsectField_c::addInsect(int type, const mVec3_c &pos,
                               const mAng3_c *angle,
                               dInsectField::heapList_c *list) {
    int idx = -1;
    if (list == NULL) {
        list = &m_objHeap;
    }
    if (!list->getFree(&idx)) {
        return FALSE;
    }
    list->mUsed[idx] = TRUE;
    dInsectCatchData_c *data = &mCatchData[idx];
    data->setType(type);
    data->setSerial(data->getSerial() + 1);
    data->setPosX(pos.x);
    data->setPosZ(pos.z);
    data->setState(INSECT_NET_STATE_NORMAL);
    EGG::FrmHeap *heap = list->mHeaps[idx];
    dInsFldBase_c *insect = dInsectField::sParamTable[type]->mCreate(heap);
    insect->mPos = insect->mInitPos = pos;
    insect->mAngle = insect->mInitAngle = *angle;
    insect->mAllocator.attach(heap, 0x20);
    insect->mPos.y += 2.0f;
    insect->mHeapIdx = idx;
    insect->mType = type;
    insect->mSerial = data->getSerial();
    insect->mSpawnKind = dInsFldBase_c::SPAWN_FIELD;
    insect->mHeapList = list;
    addPending(insect);
    return TRUE;
}

// 0xDB0
BOOL dInsectField_c::addInsectFromNet(int type, int idx,
                                      dInsectField::heapList_c *list) {
    if (list == NULL) {
        list = &m_objHeap;
    }
    if (list->mUsed[idx]) {
        return FALSE;
    }
    list->mUsed[idx] = TRUE;
    const dInsectCatchData_c *data = &mCatchData[idx];
    mVec3_c pos(data->getPosX() * 4, 0.0f, data->getPosZ() * 4);
    pos.y = dBGCF::getGroundY(&pos, 0);
    EGG::FrmHeap *heap = list->mHeaps[idx];
    dInsFldBase_c *insect = dInsectField::sParamTable[type]->mCreate(heap);
    insect->mPos = insect->mInitPos = pos;
    insect->mInitAngle.set(0, 0, 0);
    insect->mAngle.set(0, 0, 0);
    insect->mAllocator.attach(heap, 0x20);
    insect->mPos.y += 2.0f;
    insect->mHeapIdx = idx;
    insect->mType = type;
    insect->mSerial = mCatchData[idx].getSerial();
    insect->mSpawnKind = dInsFldBase_c::SPAWN_FIELD;
    insect->mHeapList = list;
    addPending(insect);
    return TRUE;
}

// 0xF7C
dInsFldBase_c *dInsectField_c::addPlayerInsect(int type, int mode,
                                               const mVec3_c *pos, int player) {
    if (!m_objHeapPlayer.mUsed[player]) {
        m_objHeapPlayer.mUsed[player] = TRUE;
        EGG::FrmHeap *heap = m_objHeapPlayer.mHeaps[player];
        dInsFldBase_c *insect;
        if (mode == PLAYER_INSECT_SHOW) {
            insect = new (heap) dInsFldShow_c();
        } else {
            insect = dInsectField::sParamTable[type]->mCreate(heap);
        }
        insect->mPos = *pos;
        insect->mInitPos = *pos;
        insect->mAngle.set(0, 0, 0);
        insect->mAllocator.attach(heap, 0x20);
        insect->mHeapIdx = player;
        insect->mType = type;
        insect->mHeapList = &m_objHeapPlayer;
        insect->mSpawnKind = dInsFldBase_c::SPAWN_PLAYER;
        switch (mode) {
        case PLAYER_INSECT_SHOW: {
            insect->mPlayer = player;
            nw4r::g3d::ResFile res;
            insect->create(res);
            addActive(insect);
            break;
        }
        case PLAYER_INSECT_RELEASE:
        default:
            insect->mPlayer = player;
            addPending(insect);
            break;
        }
        return insect;
    }
    return NULL;
}

// 0x10F8
BOOL dInsectField_c::createPending() {
    dInsFldBase_c *insect;
    dInsFldBase_c **link = &mPending;
    BOOL done = TRUE;
    while ((insect = *link) != NULL) {
        int idx = insect->mHeapIdx;
        int bank = idx;
        if (insect->mHeapList == &m_objHeapPermanent) {
            bank = idx + INSECT_NET_SLOT_NUM;
        }
        int type = insect->mType;
        if (insect->mSpawnKind == dInsFldBase_c::SPAWN_PLAYER) {
            if (dFIMdlMgr_c::sInstance->request(
                    insect->mPlayer, dFIMdlBank_c::TYPE_INSECT, type, FALSE,
                    dFIMdlBank_c::MODE_BOOK)) {
                removePending(insect);
                nw4r::g3d::ResFile res(
                    dFIMdlMgr_c::sInstance->mBanks[idx].getData());
                insect->create(res);
                insect->release();
                addActive(insect);
            } else {
                done = FALSE;
                link = &(*link)->mNext;
            }
        } else if (mBanks[bank].request(type)) {
            removePending(insect);
            nw4r::g3d::ResFile res(mBanks[bank].getData());
            insect->create(res);
            if (dNet::isSessionActive() && !dNet::isSceneOwner() &&
                insect->mHeapList == &m_objHeap) {
                dInsectCatchData_c *data = insect->getCatchData();
                if (data->getReq() == INSECT_FIELD_REQ_HELD) {
                    insect->setHeld(data->getState());
                }
            }
            addActive(insect);
        } else {
            done = FALSE;
            link = &(*link)->mNext;
        }
    }
    return done;
}

// 0x12AC
BOOL dInsectField_c::destroyAllDying() {
    dInsFldBase_c **link = &mDying;
    BOOL done = TRUE;
    while (*link != NULL) {
        if (!destroyDying(*link)) {
            done = FALSE;
            link = &(*link)->mNext;
        }
    }
    return done;
}

// 0x131C
BOOL dInsectField_c::destroyDying(dInsFldBase_c *insect) {
    BOOL done = TRUE;
    int idx = insect->mHeapIdx;
    int bank = idx;
    if (insect->mHeapList == &m_objHeapPermanent) {
        bank = idx + INSECT_NET_SLOT_NUM;
    }
    if (insect->mSpawnKind == dInsFldBase_c::SPAWN_PLAYER) {
        removeDying(insect);
        insect->~dInsFldBase_c();
        insect->mHeapList->mHeaps[idx]->free(3);
        insect->mHeapList->mUsed[idx] = FALSE;
    } else if (mBanks[bank].release()) {
        removeDying(insect);
        insect->~dInsFldBase_c();
        insect->mHeapList->mHeaps[idx]->free(3);
        insect->mHeapList->mUsed[idx] = FALSE;
    } else {
        done = FALSE;
    }
    return done;
}

// 0x144C
BOOL dInsectField_c::deleteInsect(dInsFldBase_c *insect) {
    dInsFldBase_c **link = &mPending;
    if (insect->mSpawnKind == dInsFldBase_c::SPAWN_PLAYER) {
        dInsFldBase_c *p;
        while ((p = *link) != NULL) {
            if (insect == p) {
                removePending(insect);
                if (insect->mPlayer != INSECT_FIELD_PLAYER_NONE) {
                    dFIMdlMgr_c::sInstance->release(insect->mPlayer);
                    if (mNetInsects[insect->mPlayer] == insect) {
                        mNetInsects[insect->mPlayer] = NULL;
                    }
                    insect->mPlayer = INSECT_FIELD_PLAYER_NONE;
                }
                addDying(insect);
                return destroyDying(insect);
            }
            link = &p->mNext;
        }
    }
    BOOL ret = FALSE;
    if (removeInsect(insect) && destroyDying(insect)) {
        ret = TRUE;
    }
    return ret;
}

// 0x1550
void dInsectField_c::calcSpawn() {
    if ((!dNet::isSessionActive() || dNet::isSceneOwner()) &&
        sLib::calcTimer(&mSpawnTimer) == 0) {
        mSpawnTimer = SPAWN_INTERVAL;
        spawn(m_objHeap.getFreeNum(), TRUE);
    }
    if (dNet::isSessionActive() && !dNet::isSceneOwner()) {
        addFromNet();
    }
}

// 0x1608
void dInsectField_c::addFromNet() {
    for (int i = 0; i < INSECT_NET_SLOT_NUM; i++) {
        int type = mCatchData[i].getType();
        if (!m_objHeap.mUsed[i] && type != INSECT_NET_TYPE_NONE) {
            addInsectFromNet(type, i, &m_objHeap);
        }
    }
}

// 0x1688
BOOL dInsectField_c::removeInsect(dInsFldBase_c *insect) {
    if (insect != NULL) {
        int idx = insect->mHeapIdx;
        if (insect->doDelete()) {
            if (dNet::isSessionActive() && dNet::isSceneOwner() &&
                insect->mHeapList == &m_objHeap) {
                dInsectCatchData_c *data = &mCatchData[idx];
                data->setType(INSECT_NET_TYPE_NONE);
                data->setState(INSECT_NET_STATE_NORMAL);
                data->clearReq();
            }
            removeActive(insect);
            addDying(insect);
            return TRUE;
        }
        dItem::Item item;
        item.setFromIndex(dItem::ITEM_IDX_COMMON_BUTTERFLY, insect->mType,
                          FALSE);
        dItem::infoBank_c::get()->getBITM(item);
    }
    return FALSE;
}

// 0x17A0
BOOL dInsectField_c::isLifeOver(dInsFldBase_c *insect) {
    bool inView = !dInsectField::isOutOfView(&insect->mPos);
    if (inView) {
        insect->mLifeTimer = dInsFldBase_c::LIFE_TIME;
    } else if (insect->mHeapList == &m_objHeap &&
               sLib::calcTimer(&insect->mLifeTimer) == 0 &&
               (!dNet::isSessionActive() || dNet::isSceneOwner())) {
        return TRUE;
    }
    return FALSE;
}

// 0x1838
BOOL dInsectField_c::isNetChanged(dInsFldBase_c *insect) {
    if (insect->mHeapList == &m_objHeap) {
        const dInsectCatchData_c *data = &mCatchData[insect->mHeapIdx];
        if (data->getType() != insect->getInsectType() ||
            (u16)(data->mSerialLow + data->mSerial * 2) !=
                insect->getSerial()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 0x1898
void dInsectField_c::spawn(int num, BOOL awayFromPlayers) {
    if (!dNet::isSessionActive()) {
        mVec3_c pos;
        if (fgMngProc_getRafflesiaPos(&pos)) {
            if ((dInsectField::isOutOfView(&pos) || !awayFromPlayers) &&
                !dInsectUtil::isRaining() && !dInsectUtil::isSnowing()) {
                for (int i = 0; i < num; i++) {
                    mAng3_c angle(0, 0, 0);
                    addInsect(INSECT_FLY, pos, &angle, NULL);
                }
            }
            if (dInsFldAnt_c::sAntNum == 0 && !dInsectUtil::isRaining()) {
                mVec3_c antPos;
                if (dInsectField::findAntPos(INSECT_ANT, &antPos,
                                             awayFromPlayers)) {
                    mAng3_c angle(0, 0, 0);
                    addInsect(INSECT_ANT, antPos, &angle, &m_objHeapPermanent);
                }
            }
        } else {
            if (dInsFldAnt_c::sAntNum == 0) {
                mAng3_c angle(0, 0, 0);
                mVec3_c antPos;
                if (dInsectField::findAntPos(INSECT_ANT, &antPos,
                                             awayFromPlayers) &&
                    !dInsectUtil::isRaining()) {
                    addInsect(INSECT_ANT, antPos, &angle, &m_objHeapPermanent);
                }
                if (!dInsectUtil::isRaining() && !dInsectUtil::isSnowing() &&
                    num > 0 && cM::rndF(100.0f) < 20.0f) {
                    mAng3_c angle(0, 0, 0);
                    mVec3_c flyPos;
                    if (dInsectField::findFlyPos(INSECT_FLY, &flyPos,
                                                 awayFromPlayers) &&
                        addInsect(INSECT_FLY, flyPos, &angle, NULL)) {
                        num--;
                    }
                }
            }
        }
    }
    for (int i = 0; i < num; i++) {
        dInsectSpawn_c *entry = dInsectInfo::getRandomSpawn();
        if (entry == NULL) {
            continue;
        }
        int type = entry->mType;
        if (dNet::isSessionActive() == true && !dInsectInfo::isOfTerm(type)) {
            continue;
        }
        if (dInsectUtil::isRaining() == TRUE &&
            !dInsectInfo::canSpawnInRain(type)) {
            continue;
        }
        if (dInsectUtil::isSnowing() == TRUE &&
            !dInsectInfo::canSpawnInSnow(type)) {
            continue;
        }
        if (type == INSECT_SNAIL && !dInsectUtil::isRaining()) {
            continue;
        }
        if (type == INSECT_FLEA) {
            continue;
        }
        int place = dInsectField::getPlace(type);
        if (place >= dInsectField::PLACE_NUM) {
            continue;
        }
        mVec3_c spawnPos;
        if (dInsectField::sPlaceFinders[place](type, &spawnPos,
                                               awayFromPlayers)) {
            mAng3_c angle(0, 0, 0);
            addInsect(type, spawnPos, &angle, NULL);
        }
    }
}

// 0x1B94
void dInsectField_c::initSpawn() {
    if (dNet::isSessionActive()) {
        initSlots();
        addFromNet();
    }
    if (dNet::isSessionActive() && !dNet::isSceneOwner()) {
        mWasOwner = FALSE;
        return;
    }
    if (!dNet::isSessionActive() || g_insectRespawn) {
        int num = m_objHeap.getFreeNum();
        if (num == INSECT_NET_SLOT_NUM) {
            spawn(num, FALSE);
        }
        g_insectRespawn = 0;
    }
    mWasOwner = TRUE;
}

// 0x1C74
void dInsectField_c::addHoneycombBees(const mVec3_c *pos) {
    if (!dInsFldHornet_c::isBeesOut()) {
        mAng3_c angle(0, 0, 0);
        addInsect(INSECT_BEE, *pos, &angle, &m_objHeapPermanent);
    }
}

// 0x1CDC
BOOL dInsectField_c::addFlea(const mVec3_c *pos) {
    if (dInsectField::sFlea == NULL) {
        mAng3_c angle(0, 0, 0);
        return addInsect(INSECT_FLEA, *pos, &angle, &m_objHeapPermanent);
    }
    return FALSE;
}

// 0x1D34
dInsFldBase_c *dInsectField_c::getFlea() {
    return dInsectField::sFlea;
}

// 0x1D40
BOOL dInsectField_c::deleteAll() {
    dInsFldBase_c *insect = mActive;
    while (insect != NULL) {
        if (insect->mHoldDraw || insect->mHoldExecute) {
            insect->clearHoldProcs();
        }
        if (!removeInsect(insect)) {
            return FALSE;
        }
        insect = mActive;
    }
    return TRUE;
}
namespace dInsectField {

// What a place check is told (the user of dBkUnitSearchCandCb_c).
struct placeUser_c {
    /* 0x0 */ int mType;
    /* 0x4 */ u8 mAwayFromPlayers;
};

// 0x1DCC: open ground (the mosquito only once the part-time job is done)
static BOOL checkGround(int blockX, int blockZ, int unitX, int unitZ,
                        void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    u16 id;
    int x;
    int z = (blockZ << 4) + unitZ;
    x = (blockX << 4) + unitX;
    if (dBGCF::getAttr(x, z) == BG_ATTR_STONE_BUS) {
        return FALSE;
    }
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    id = item->mId;
    if (u->mType == INSECT_MOSQUITO) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (player == NULL || player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
            return FALSE;
        }
    }
    if (id == dItem::ITEM_ID_NONE) {
        if (dField::getInfo(FD_ID_CURRENT)->_24->getAttr(x, z) &
            dFdUnitAttr_c::STR_COL) {
            return FALSE;
        }
        dBGCF::groundChk_c chk(x, z, dBGCF::LAYER_TOP, 0, 0);
        if (chk.mWater != BG_WATER_SEA) {
            return TRUE;
        }
    }
    return FALSE;
}

// 0x1F58
BOOL findGroundPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_GROUND];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkGround, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x20F0: flowers (two butterflies only on some colours)
static BOOL checkFlower(int blockX, int blockZ, int unitX, int unitZ,
                        void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    u16 id = item->mId;
    if (item->isFlower() == TRUE || id == dItem::FG_DANDELION) {
        if (u->mType == INSECT_TIGER_BUTTERFLY) {
            switch (id) {
            case dItem::FG_TULIP_RED:
            case dItem::FG_TULIP_PINK:
            case dItem::FG_PANSY_RED:
            case dItem::FG_COSMOS_RED:
            case dItem::FG_COSMOS_PINK:
            case dItem::FG_ROSE_RED:
            case dItem::FG_ROSE_PINK:
                return TRUE;
            default:
                return FALSE;
            }
        } else if (u->mType == INSECT_PEACOCK_BUTTERFLY) {
            switch (id) {
            case dItem::FG_TULIP_PURPLE:
            case dItem::FG_TULIP_BLACK:
            case dItem::FG_PANSY_PURPLE:
            case dItem::FG_PANSY_BLUE:
            case dItem::FG_COSMOS_BLACK:
            case dItem::FG_ROSE_PURPLE:
            case dItem::FG_ROSE_BLACK:
            case dItem::FG_ROSE_BLUE:
                return TRUE;
            default:
                return FALSE;
            }
        } else {
            return TRUE;
        }
    }
    return FALSE;
}

// 0x2258
BOOL findFlowerPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_FLOWER];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkFlower, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x23F0: flowers (the orchid mantis only on some)
static BOOL checkMantisFlower(int blockX, int blockZ, int unitX, int unitZ,
                              void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    u16 id = item->mId;
    if (item->isFlower() == TRUE || id == dItem::FG_DANDELION) {
        if (u->mType == INSECT_ORCHID_MANTIS) {
            switch (id) {
            case dItem::FG_TULIP_WHITE:
            case dItem::FG_PANSY_WHITE:
            case dItem::FG_COSMOS_WHITE:
            case dItem::FG_ROSE_WHITE:
            case dItem::FG_JACOBS_LADDER:
                return TRUE;
            default:
                return FALSE;
            }
        } else {
            return TRUE;
        }
    }
    return FALSE;
}

// 0x251C
BOOL findMantisFlowerPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_MANTIS_FLOWER];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkMantisFlower, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x26B4: thick grass
static BOOL checkGrass(int blockX, int blockZ, int unitX, int unitZ,
                       void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    if (item->mId == dItem::ITEM_ID_NONE) {
        mVec3_c pos;
        dFdBase_c::getUnitCenterPos(&pos, blockX, blockZ, unitX, unitZ);
        if (dField::getInfo(FD_ID_CURRENT)->getGrassWear(&pos, TRUE) >= 0x80) {
            if (dField::getInfo(FD_ID_CURRENT)
                    ->_24->getAttr((blockX << 4) + unitX,
                                   (blockZ << 4) + unitZ) &
                dFdUnitAttr_c::STR_COL) {
                return FALSE;
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 0x2804
BOOL findGrassPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_GRASS];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkGrass, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x299C: river water
static BOOL checkRiver(int blockX, int blockZ, int unitX, int unitZ,
                       void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, blockX, blockZ, unitX, unitZ);
    if (dBGCF::isNearFall(&pos) == TRUE) {
        return FALSE;
    }
    dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_TOP, 0, 0);
    if (chk.mWater == BG_WATER_RIVER) {
        return TRUE;
    }
    return FALSE;
}

// 0x2A8C
BOOL findRiverPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_RIVER];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkRiver, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x2C24: river or pond water
static BOOL checkWater(int blockX, int blockZ, int unitX, int unitZ,
                       void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, blockX, blockZ, unitX, unitZ);
    if (dBGCF::isNearFall(&pos) == TRUE) {
        return FALSE;
    }
    dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_TOP, 0, 0);
    if (chk.mWater == BG_WATER_POND || chk.mWater == BG_WATER_RIVER) {
        return TRUE;
    }
    return FALSE;
}

// 0x2D1C
BOOL findWaterPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_WATER];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkWater, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x2EB4: spoiled turnips or candy lying around
static BOOL checkAntFood(int blockX, int blockZ, int unitX, int unitZ,
                         void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    if (item->getKind() == dItem::KIND_BAD_KABU &&
        !dField::getInfo(FD_ID_TOWN)->isBuried(blockX, blockZ, unitX, unitZ)) {
        return TRUE;
    }
    if (item->getKind() == dItem::KIND_CANDY &&
        !dField::getInfo(FD_ID_TOWN)->isBuried(blockX, blockZ, unitX, unitZ)) {
        return TRUE;
    }
    return FALSE;
}

// 0x2FF4
BOOL findAntPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_ANT];
    if (!dInsectInfo::isAntsAttracted()) {
        return FALSE;
    }
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = awayFromPlayers ? 0 : blocks->getCount();
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkAntFood, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            dInsectInfo::clearAntsAttracted();
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    if (blocks->getCount() == 0) {
        dInsectInfo::clearAntsAttracted();
    }
    return FALSE;
}

// 0x31C4: trash or spoiled turnips lying around
static BOOL checkFlyFood(int blockX, int blockZ, int unitX, int unitZ,
                         void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    if ((item->isDust() || item->getKind() == dItem::KIND_BAD_KABU) &&
        !dField::getInfo(FD_ID_TOWN)->isBuried(blockX, blockZ, unitX, unitZ)) {
        return TRUE;
    }
    return FALSE;
}

// 0x32D8
BOOL findFlyPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    if (!dInsectInfo::isFliesAttracted()) {
        return FALSE;
    }
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_FLY];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = awayFromPlayers ? 0 : blocks->getCount();
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkFlyFood, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            dInsectInfo::clearFliesAttracted();
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    if (blocks->getCount() == 0) {
        dInsectInfo::clearFliesAttracted();
    }
    return FALSE;
}

// 0x34A8: bare soil (the walking leaf: any dry soil; the tarantula and the
// scorpion once the part-time job is done)
static BOOL checkSoil(int blockX, int blockZ, int unitX, int unitZ,
                      void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    if (item->mId == dItem::ITEM_ID_NONE) {
        int x = (blockX << 4) + unitX;
        int z = (blockZ << 4) + unitZ;
        if (dField::getInfo(FD_ID_CURRENT)->_24->getAttr(x, z) &
            dFdUnitAttr_c::STR_COL) {
            return FALSE;
        }
        dBGCF::groundChk_c chk(x, z, dBGCF::LAYER_TOP, 0, 0);
        int attr = chk.mUnitAttr;
        if (attr == BG_ATTR_SAND) {
            return FALSE;
        }
        const int type = u->mType;
        if (u->mType == INSECT_TARANTULA || type == INSECT_SCORPION) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            if (player == NULL ||
                player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
                return FALSE;
            }
        }
        if (u->mType == INSECT_WALKING_LEAF) {
            if (chk.mWater == BG_WATER_NONE &&
                dField::getInfo(FD_ID_CURRENT)->getDigType(x, z) ==
                    BG_DIG_SOIL) {
                return TRUE;
            }
        } else {
            switch (attr) {
            case BG_ATTR_SOIL:
            case BG_ATTR_SOILX:
                if (dField::getInfo(FD_ID_CURRENT)->getDigType(x, z) ==
                    BG_DIG_SOIL) {
                    return TRUE;
                }
                return FALSE;
            default:
                return FALSE;
            }
        }
    }
    return FALSE;
}

// 0x36A8
BOOL findSoilPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_SOIL];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkSoil, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x3840: rocks
static BOOL checkRock(int blockX, int blockZ, int unitX, int unitZ,
                      void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    BOOL rock = FALSE;
    if (item->mId >= dItem::FG_STONE_FIRST &&
        item->mId <= dItem::FG_STONE_LAST) {
        rock = TRUE;
    }
    return rock == TRUE;
}

// 0x393C
BOOL findRockPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_ROCK];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkRock, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

} // namespace dInsectField

namespace dInsectField {

// 0x3AD4
BOOL checkHouseLight(int blockX, int blockZ, int unitX, int unitZ, void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour < 19 && (now.hour < 0 || now.hour > 3)) {
        return FALSE;
    }
    dSTR::getTownUnitAttr();
    int x = (blockX << 4) + unitX;
    int z = (blockZ << 4) + unitZ;
    int hx = 0;
    int hz = 0;
    int houseOfsX[2] = {-1, -2};
    int houseOfsZ[2] = {2, 2};
    u32 type[2];
    for (u32 i = 0; i < ANIMAL_NUM; i++) {
        if (!dSaveBuildingList_c::get()->getNpcHousePos(&hx, &hz, i)) {
            continue;
        }
        if (!dSTR::getNpcHouseType(type, i) || type[0] == 1) {
            continue;
        }
        for (u32 j = 0; j < 2; j++) {
            if (x == hx + houseOfsX[j] && z == hz + houseOfsZ[j]) {
                dItem::Item id(dSaveBuildingList_c::getNpcHouseId(i));
                daSTR_c *house =
                    dSTR::getAcList()->searchBuilding(id.mId, TRUE);
                if (house != NULL && house->isWindowLit()) {
                    sLightHouse = i;
                    sLightHouseFound = TRUE;
                    return TRUE;
                }
            }
        }
    }
    int hallOfsX[6] = {-1, -2, -3, 1, 2, 3};
    int hallOfsZ[6] = {2, 2, 1, 2, 2, 1};
    dItem::Item item((u16)BUILDING_TOWN_HALL);
    if (dSaveBuildingList_c::get()->getPos(&hx, &hz, item, 1)) {
        for (int j = 0; j < 6; j++) {
            if (x == hx + hallOfsX[j] && z == hz + hallOfsZ[j]) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 0x3E64
BOOL findHouseLightPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_HOUSE_LIGHT];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkHouseLight, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// A palm tree (the range is FG_PALM_FIRST..FG_PALM_LAST only, despite the
// name).
inline BOOL isFruitTree(u16 id) {
    BOOL ret = FALSE;
    if (id >= dItem::FG_PALM_FIRST && id <= dItem::FG_PALM_LAST) {
        ret = TRUE;
    }
    return ret;
}

// 0x3FFC: a grown tree, no fruit tree, cedar or bamboo
static BOOL checkTree(int blockX, int blockZ, int unitX, int unitZ,
                      void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    const dItem::FgInfo *info = item->getFgInfo();
    if (info == NULL) {
        return FALSE;
    }
    if (info->mTreeStage > 3) {
        u16 id = item->mId;
        if (!isFruitTree(id)) {
            switch (id) {
            case dItem::FG_TREE_FTR:
            case dItem::FG_TREE_BEES:
            case dItem::FG_TREE_BELLS:
            case dItem::FG_MONEY_TREE:
            case dItem::FG_CEDAR_FTR:
            case dItem::FG_CEDAR_BEES:
            case dItem::FG_CEDAR_BELLS:
            case dItem::FG_CEDAR_LIGHTS:
                break;
            default:
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 0x4164
BOOL findTreePos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_TREE];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkTree, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x42FC: a grown fruit tree
static BOOL checkPalm(int blockX, int blockZ, int unitX, int unitZ,
                      void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    const dItem::FgInfo *info = item->getFgInfo();
    if (info == NULL) {
        return FALSE;
    }
    if (info->mTreeStage > 3 && isFruitTree(item->mId)) {
        return TRUE;
    }
    return FALSE;
}

// 0x4434
BOOL findPalmPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_PALM];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkPalm, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x45CC: a grown tree that bears no fruit
static BOOL checkTreeTrunk(int blockX, int blockZ, int unitX, int unitZ,
                           void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    const dItem::FgInfo *info = item->getFgInfo();
    if (info == NULL) {
        return FALSE;
    }
    if (info->mTreeStage > 3) {
        u16 id = item->mId;
        if (!isFruitTree(id)) {
            if (u->mType == INSECT_BEE) {
                if (id == dItem::FG_TREE_BEES && id == dItem::FG_CEDAR_BEES) {
                    return TRUE;
                }
                return FALSE;
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 0x4730
BOOL findTreeTrunkPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_TREE_TRUNK];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkTreeTrunk, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x48C8: any grown tree
static BOOL checkGrownTree(int blockX, int blockZ, int unitX, int unitZ,
                           void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    const dItem::FgInfo *info = item->getFgInfo();
    if (info == NULL) {
        return FALSE;
    }
    return info->mTreeStage > 3;
}

// 0x49D8
BOOL findGrownTreePos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_GROWN_TREE];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkGrownTree, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x4B70: tree stumps
static BOOL checkStump(int blockX, int blockZ, int unitX, int unitZ,
                       void *user) {
    placeUser_c *u = static_cast<placeUser_c *>(user);
    if (u->mAwayFromPlayers &&
        !isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    if (!isUnitFree(blockX, blockZ, unitX, unitZ)) {
        return FALSE;
    }
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return FALSE;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return FALSE;
    }
    switch (item->mId) {
    case dItem::FG_STUMP:
    case dItem::FG_CEDAR_STUMP:
    case dItem::FG_PALM_STUMP:
        return TRUE;
    default:
        return FALSE;
    }
}

// 0x4C78
BOOL findStumpPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    dSearchCandXZCore_c *blocks =
        dInsectField_c::getInstance()->mSearch[PLACE_STUMP];
    int blockX, blockZ;
    while (!blocks->getRandomXZ(&blockX, &blockZ)) {
        blocks->setAll();
        blocks->removeBorder();
    }
    int tries = 0;
    do {
        placeUser_c user;
        user.mType = type;
        user.mAwayFromPlayers = awayFromPlayers;
        dBkUnitSearchCandCb_c units(blockX, blockZ, checkStump, &user);
        units.clear();
        units.search();
        int unitX, unitZ;
        if (units.getRandomXZ(&unitX, &unitZ)) {
            dFdBase_c::getUnitCenterPos(pos, blockX, blockZ, unitX, unitZ);
            return TRUE;
        }
        blocks->remove(blockX + blockZ * blocks->mWidth);
    } while (tries-- != 0 && blocks->getRandomXZ(&blockX, &blockZ));
    return FALSE;
}

// 0x4E10: a snowman (the dung beetle); its index is marked in
// dInsFldTumbleBug_c::sBallTaken
BOOL findSnowmanPos(int type, mVec3_c *pos, BOOL awayFromPlayers) {
    int idx = 0;
    mVec3_c found(0.0f, 0.0f, 0.0f);
    if (daSnowMan_c::find(&found, &idx, (void *)awayFromPlayers,
                          dInsFldTumbleBug_c::canSpawn) == TRUE) {
        *pos = found;
        dInsFldTumbleBug_c::sBallTaken[idx] = TRUE;
        return TRUE;
    }
    return FALSE;
}

// 0x4EA8
int getPlace(int type) {
    int place = PLACE_GRASS;
    switch (type) {
    case INSECT_OAK_SILK_MOTH:
    case INSECT_BROWN_CICADA:
    case INSECT_ROBUST_CICADA:
    case INSECT_WALKER_CICADA:
    case INSECT_EVENING_CICADA:
    case INSECT_LANTERN_FLY:
    case INSECT_WALKING_STICK:
    case INSECT_DRONE_BEETLE:
    case INSECT_SCARAB_BEETLE:
    case INSECT_JEWEL_BEETLE:
    case INSECT_MIYAMA_STAG:
    case INSECT_SAW_STAG:
    case INSECT_GIANT_STAG:
    case INSECT_RAINBOW_STAG:
    case INSECT_DYNASTID_BEETLE:
        place = PLACE_TREE_TRUNK;
        break;
    case INSECT_BAGWORM:
    case INSECT_SPIDER:
        place = PLACE_TREE;
        break;
    case INSECT_CYCLOMMATUS_STAG:
    case INSECT_GOLDEN_STAG:
    case INSECT_ATLAS_BEETLE:
    case INSECT_ELEPHANT_BEETLE:
    case INSECT_HERCULES_BEETLE:
    case INSECT_GOLIATH_BEETLE:
        place = PLACE_PALM;
        break;
    case INSECT_COMMON_BUTTERFLY:
    case INSECT_YELLOW_BUTTERFLY:
    case INSECT_TIGER_BUTTERFLY:
    case INSECT_PEACOCK_BUTTERFLY:
    case INSECT_MONARCH_BUTTERFLY:
    case INSECT_EMPEROR_BUTTERFLY:
    case INSECT_AGRIAS_BUTTERFLY:
    case INSECT_BIRDWING_BUTTERFLY:
    case INSECT_HONEYBEE:
        place = PLACE_FLOWER;
        break;
    case INSECT_LONG_LOCUST:
    case INSECT_MIGRATORY_LOCUST:
    case INSECT_CRICKET:
    case INSECT_BELL_CRICKET:
    case INSECT_GRASSHOPPER:
        place = PLACE_GRASS;
        break;
    case INSECT_MANTIS:
    case INSECT_ORCHID_MANTIS:
    case INSECT_SNAIL:
    case INSECT_LADYBUG:
        place = PLACE_MANTIS_FLOWER;
        break;
    case INSECT_RED_DRAGONFLY:
    case INSECT_DARNER_DRAGONFLY:
    case INSECT_BANDED_DRAGONFLY:
    case INSECT_GIANT_PETALTAIL_DRAGONFLY:
    case INSECT_MOSQUITO:
        place = PLACE_GROUND;
        break;
    case INSECT_RAJA_BROOKE:
    case INSECT_POND_SKATER:
    case INSECT_DIVING_BEETLE:
    case INSECT_FIREFLY:
        place = PLACE_WATER;
        break;
    case INSECT_MOLE_CRICKET:
    case INSECT_WALKING_LEAF:
    case INSECT_TARANTULA:
    case INSECT_SCORPION:
        place = PLACE_SOIL;
        break;
    case INSECT_PILL_BUG:
    case INSECT_CENTIPEDE:
        place = PLACE_ROCK;
        break;
    case INSECT_MOTH:
        place = PLACE_HOUSE_LIGHT;
        break;
    case INSECT_BEE:
        place = PLACE_TREE_TRUNK;
        break;
    case INSECT_FLY:
        place = PLACE_FLY;
        break;
    case INSECT_ANT:
        place = PLACE_ANT;
        break;
    case INSECT_FLEA:
        place = PLACE_NUM;
        break;
    case INSECT_VIOLIN_BEETLE:
    case INSECT_LONGHORN_BEETLE:
        place = PLACE_STUMP;
        break;
    case INSECT_DUNG_BEETLE:
        place = PLACE_SNOWMAN;
        break;
    }
    return place;
}

// .data 0x1BD8
PlaceFinder sPlaceFinders[PLACE_NUM] = {
    findGroundPos,    findFlowerPos, findMantisFlowerPos, findGrassPos,
    findRiverPos,     findWaterPos,  findAntPos,          findFlyPos,
    findSoilPos,      findRockPos,   findHouseLightPos,   findStumpPos,
    findSnowmanPos,   findTreePos,   findPalmPos,         findTreeTrunkPos,
    findGrownTreePos,
};

// 0x4F58: pos is in the view of the camera at center
BOOL isInView(const mVec3_c *center, const mVec3_c *pos) {
    f32 minX = -320.0f + center->x;
    f32 maxX = 320.0f + center->x;
    f32 minZ = -512.0f + center->z;
    f32 maxZ = 192.0f + center->z;
    if (sLib::isInRange(pos->x, minX, maxX) &&
        sLib::isInRange(pos->z, minZ, maxZ)) {
        return TRUE;
    }
    return FALSE;
}

// 0x5004
BOOL isOutOfView(const mVec3_c *pos) {
    int blockX, blockZ, unitX, unitZ;
    dFdBase_c::posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, pos);
    return isAwayFromPlayers(blockX, blockZ, unitX, unitZ);
}

// Within the view window around a player's (or the camera's) unit.
inline BOOL isNearUnit(int x, int z, int cx, int cz) {
    int dx = x - cx;
    int dz = z - cz;
    BOOL ret;
    if (dx >= -10 && dx <= 10 && dz >= -16 && dz <= 6) {
        ret = TRUE;
    } else {
        ret = FALSE;
    }
    return ret;
}

// 0x504C
BOOL isAwayFromPlayers(int blockX, int blockZ, int unitX, int unitZ) {
    if (dNet::isSessionActive()) {
        int x = (blockX << 4) + unitX;
        int z = (blockZ << 4) + unitZ;
        for (int i = 0; i < INSECT_UTIL_PLAYER_NUM; i++) {
            u32 px, pz;
            if (dPlayerMgr::getUnitCoordinates(&px, &pz, i) &&
                isNearUnit(x, z, px, pz)) {
                return FALSE;
            }
        }
        return TRUE;
    }
    int cx = worldToUnit(dCamera_c::sCenter.x);
    int cz = worldToUnit(dCamera_c::sCenter.z);
    return !isNearUnit((blockX << 4) + unitX, (blockZ << 4) + unitZ, cx, cz);
}

} // namespace dInsectField

// 0x51AC
void dInsectField_c::sendSlots() {
    for (int i = 0; i < INSECT_NET_SLOT_NUM; i++) {
        dNet::setRecord(i + NET_REC_INSECT, &mCatchData[i], 0);
    }
}

// 0x5200
void dInsectField_c::initSlots() {
    for (int i = 0; i < INSECT_NET_SLOT_NUM; i++) {
        cLib::memCpy(&mCatchData[i], dNet::getRecord(i + NET_REC_INSECT),
                     sizeof(dInsectCatchData_c));
    }
}

// 0x525C
BOOL dInsectField_c::acceptCatch(int member, dInsectField::catchMsg_c *msg) {
    dInsectCatchData_c *data = &mCatchData[msg->getIdx()];
    if (data->getState() == INSECT_NET_STATE_NORMAL &&
        data->getType() == msg->getValue() &&
        data->getSerial() == msg->getSerial()) {
        msg->setMember(member);
        return g_playActorMng->findActorIf(dInsectField::tryCatchCb, msg);
    }
    return FALSE;
}

namespace dInsectField {

// 0x52F8
BOOL tryCatchCb(dPlayActor_c *actor, void *arg) {
    dInsFldBase_c *insect = reinterpret_cast<dInsFldBase_c *>(actor);
    catchMsg_c *msg = static_cast<catchMsg_c *>(arg);
    if (insect->mHeapIdx == msg->getIdx()) {
        insect->tryCatch(msg->getMember());
        return TRUE;
    }
    return FALSE;
}

// 0x5340
BOOL resetCatchCb(dPlayActor_c *actor, void *arg) {
    dInsFldBase_c *insect = reinterpret_cast<dInsFldBase_c *>(actor);
    catchMsg_c *msg = static_cast<catchMsg_c *>(arg);
    if (insect->mHeapIdx == msg->getIdx()) {
        insect->mCatchState = dInsFldBase_c::CATCH_STATE_NONE;
        return TRUE;
    }
    return FALSE;
}

// 0x536C
BOOL caughtCb(dPlayActor_c *actor, void *arg) {
    dInsFldBase_c *insect = reinterpret_cast<dInsFldBase_c *>(actor);
    catchMsg_c *msg = static_cast<catchMsg_c *>(arg);
    if (insect->mHeapIdx == msg->getIdx()) {
        insect->mCatchState = dInsFldBase_c::CATCH_STATE_CAUGHT;
        return TRUE;
    }
    return FALSE;
}

// 0x5398
BOOL missedCb(dPlayActor_c *actor, void *arg) {
    dInsFldBase_c *insect = reinterpret_cast<dInsFldBase_c *>(actor);
    catchMsg_c *msg = static_cast<catchMsg_c *>(arg);
    if (insect->mHeapIdx == msg->getIdx()) {
        insect->mCatchState = dInsFldBase_c::CATCH_STATE_MISSED;
        return TRUE;
    }
    return FALSE;
}

// 0x53C4: NET_PACKET_INSECT_FIELD (lbl_8074E510 while the field exists)
void recvCatchMsg(int member, const u8 *data, u32 size, u32 arg) {
    catchMsg_c msg;
    cLib::memCpy(&msg, data, sizeof(msg));
    u32 type = msg.getType();
    if (type == catchMsg_c::TYPE_REQUEST) {
        if (dInsectField_c::getInstance()->acceptCatch(member, &msg)) {
            catchMsg_c out;
            out.setType(catchMsg_c::TYPE_CAUGHT);
            out.setIdx(msg.getIdx());
            out.setValue(msg.getValue());
            out.setSerial(msg.getSerial());
            out.setMember(0);
            if (member == dNet::getMyNo()) {
                recvCatchMsg(dNet::getMyNo(), (const u8 *)&out, sizeof(out),
                             member);
            } else {
                dNet::beginPacket();
                dNet::addPacket(&out, sizeof(out));
                dNet::sendPacket(NET_PACKET_INSECT_FIELD, member);
            }
        } else {
            catchMsg_c out;
            out.setType(catchMsg_c::TYPE_MISSED);
            out.setIdx(msg.getIdx());
            out.setValue(msg.getValue());
            out.setSerial(msg.getSerial());
            out.setMember(0);
            if (member == dNet::getMyNo()) {
                recvCatchMsg(dNet::getMyNo(), (const u8 *)&out, sizeof(out),
                             member);
            } else {
                dNet::beginPacket();
                dNet::addPacket(&out, sizeof(out));
                dNet::sendPacket(NET_PACKET_INSECT_FIELD, member);
            }
        }
    } else if (type == catchMsg_c::TYPE_RESET) {
        g_playActorMng->findActorIf(resetCatchCb, &msg);
    } else if (type == catchMsg_c::TYPE_CAUGHT) {
        g_playActorMng->findActorIf(caughtCb, &msg);
    } else {
        g_playActorMng->findActorIf(missedCb, &msg);
    }
}

} // namespace dInsectField

namespace dInsectField {

inline bool isTulip(u16 id) {
    u8 ret = FALSE;
    if (id >= dItem::FG_TULIP_RED && id <= dItem::FG_TULIP_BLACK) {
        ret = TRUE;
    }
    return ret;
}
inline bool isPansy(u16 id) {
    u8 ret = FALSE;
    if (id >= dItem::FG_PANSY_WHITE && id <= dItem::FG_PANSY_BLUE) {
        ret = TRUE;
    }
    return ret;
}
inline bool isCosmos(u16 id) {
    u8 ret = FALSE;
    if (id >= dItem::FG_COSMOS_WHITE && id <= dItem::FG_COSMOS_BLACK) {
        ret = TRUE;
    }
    return ret;
}
inline bool isRose(u16 id) {
    u8 ret = FALSE;
    if (id >= dItem::FG_ROSE_RED && id <= dItem::FG_ROSE_GOLD) {
        ret = TRUE;
    }
    return ret;
}
inline bool isCarnation(u16 id) {
    u8 ret = FALSE;
    if (id >= dItem::FG_CARNATION_RED && id <= dItem::FG_CARNATION_WHITE) {
        ret = TRUE;
    }
    return ret;
}
inline bool isJacobsLadder(u16 id) {
    u8 ret = FALSE;
    if (id >= dItem::FG_JACOBS_LADDER && id <= dItem::FG_JACOBS_LADDER) {
        ret = TRUE;
    }
    return ret;
}
inline bool isRafflesia(u16 id) {
    u8 ret = FALSE;
    if (id >= dItem::FG_RAFFLESIA && id <= dItem::FG_RAFFLESIA) {
        ret = TRUE;
    }
    return ret;
}
inline bool isStone(u16 id) {
    u8 ret = FALSE;
    if (id >= dItem::FG_STONE_FIRST && id <= dItem::FG_STONE_LAST) {
        ret = TRUE;
    }
    return ret;
}
inline bool isMoneyRock(u16 id) {
    u8 ret = FALSE;
    if (id >= dItem::FG_MONEY_ROCK_FIRST && id <= dItem::FG_MONEY_ROCK_LAST) {
        ret = TRUE;
    }
    return ret;
}
inline int getStoneIdx(u16 id) {
    if (isStone(id)) {
        return id - dItem::FG_STONE_FIRST;
    }
    return -1;
}
inline u32 getMoneyRockIdx(u16 id) {
    if (isMoneyRock(id)) {
        return id - dItem::FG_MONEY_ROCK_FIRST;
    }
    return -1;
}

// 0x55B4
void getItemOffset(mVec3_c *out, const dItem::Item *item) {
    u16 id = item->mId;
    out->set(0.0f, 0.0f, 0.0f);
    if (isTulip(id) == TRUE) {
        out->set(0.0f, 13.0f, 0.0f);
    } else if (isPansy(id)) {
        out->set(0.0f, 12.0f, 0.0f);
    } else if (isCosmos(id)) {
        out->set(0.0f, 12.0f, 0.0f);
    } else if (isRose(id)) {
        out->set(0.0f, 14.0f, 0.0f);
    } else if (isCarnation(id)) {
        out->set(0.0f, 13.0f, 0.0f);
    } else if (isJacobsLadder(id)) {
        out->set(0.0f, 16.0f, 0.0f);
    } else if (id == dItem::FG_DANDELION) {
        out->set(0.0f, 13.0f, 0.0f);
    }
    id = item->mId;
    if (isStone(id) == TRUE || isMoneyRock(id)) {
        int idx;
        if (isStone(id)) {
            idx = getStoneIdx(id);
        } else {
            idx = getMoneyRockIdx(id) % 5;
        }
        switch (idx) {
        case 0:
            out->set(0.0f, 20.5f, 1.5f);
            break;
        case 1:
            out->set(4.0f, 19.0f, 0.0f);
            break;
        case 2:
            out->set(8.0f, 19.0f, 0.0f);
            break;
        case 3:
            out->set(0.0f, 19.0f, -4.5f);
            break;
        case 4:
            out->set(-1.5f, 19.2f, 2.0f);
            break;
        }
    }
    u16 id2;
    u16 id1;
    u16 id0;
    id0 = dItem::Item(dItem::ITEM_IDX_EMPTY_CAN).mId;
    id1 = dItem::Item(dItem::ITEM_IDX_BOOT).mId;
    id2 = dItem::Item(dItem::ITEM_IDX_OLD_TIRE).mId;
    const u16 id3 = dItem::Item(dItem::ITEM_IDX_SPOILED_TURNIPS).mId;
    id = item->mId;
    if (id0 == id) {
        out->set(0.0f, 8.0f, 3.5f);
    } else if (id1 == id) {
        out->set(3.0f, 10.0f, 0.5f);
    } else if (id2 == id) {
        out->set(0.0f, 13.0f, 0.5f);
    } else if (id3 == id) {
        out->set(0.0f, 10.0f, 0.0f);
    } else if (isRafflesia(id)) {
        out->set(0.0f, 8.0f, 0.0f);
    }
}

// 0x59F0
BOOL isUnitFree(int blockX, int blockZ, int unitX, int unitZ) {
    dInsFldBase_c *insect;
    dInsectField_c *mng = reinterpret_cast<dInsectField_c *>(g_playActorMng);
    int bx = 0;
    int bz = 0;
    int ux = 0;
    int uz = 0;
    dInsFldBase_c **link = &mng->mActive;
    while ((insect = *link) != NULL) {
        dFdBase_c::posToBlockUnit(&bx, &bz, &ux, &uz, &insect->mPos);
        if (blockX == bx && blockZ == bz && unitX == ux && unitZ == uz) {
            return FALSE;
        }
        link = &insect->mNext;
    }
    link = &mng->mPending;
    while ((insect = *link) != NULL) {
        dFdBase_c::posToBlockUnit(&bx, &bz, &ux, &uz, &insect->mPos);
        if (blockX == bx && blockZ == bz && unitX == ux && unitZ == uz) {
            return FALSE;
        }
        link = &insect->mNext;
    }
    return TRUE;
}

} // namespace dInsectField

namespace dInsectField {

// 0x5B1C: what registObjc adds to the radius.
f32 getObjcRadiusAdd() {
    return 4.0f;
}

} // namespace dInsectField

u32 dInsFldBank_c::sBankNum = -1;     // .data 0x1C1C
dInsFldBank_c *dInsFldBank_c::sBanks; // .bss 0x44
char dInsFldBank_c::sPath[0x20];      // .bss 0x48

// 0x5B28
dInsFldBank_c *dInsFldBank_c::find(int type) {
    dInsFldBank_c *bank = sBanks;
    for (u32 i = 0; i < sBankNum; i++, bank++) {
        if (type == bank->mType && (u32)bank->mState <= STATE_LOADED) {
            return bank;
        }
    }
    return NULL;
}

// 0x5B74
BOOL dInsFldBank_c::request(int type) {
    if (mState == STATE_LOADED) {
        return TRUE;
    }
    if (mState == STATE_FREE) {
        dInsFldBank_c *src = find(type);
        if (src != NULL) {
            if (src->mState == STATE_LOADING) {
                src->addSharer(this);
                mShared = src;
                mState = STATE_SHARED;
            } else {
                copyFrom(src);
                mState = STATE_LOADED;
            }
        } else {
            mState = STATE_LOADING;
        }
    }
    if (mState == STATE_LOADING) {
        snprintf(sPath, sizeof(sPath), "/Insect/%s.brres",
                 dInsectInfo::getModelName(type));
        mType = type;
        if (load(sPath, mpBankHeap, 0)) {
            mState = STATE_LOADED;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// 0x5C94
BOOL dInsFldBank_c::release() {
    if (mState == STATE_SHARED) {
        return FALSE;
    }
    if (unload(FALSE)) {
        mState = STATE_FREE;
        mType = -1;
        mShared = NULL;
        return TRUE;
    }
    return FALSE;
}

// 0x5D04
void dInsFldBank_c::setHeap(EGG::FrmHeap *heap) {
    mpBankHeap = heap;
}

// 0x5D0C
void dInsFldBank_c::freeData() {
    if (mpBankHeap != NULL && mpData != NULL) {
        mpBankHeap->free(3);
    }
}

// 0x5D34
void dInsFldBank_c::onLoaded() {
    while (mSharers != NULL) {
        mSharers->copyFrom(this);
        removeSharer(mSharers);
    }
    dDvd::brresBank_c::onLoaded();
}

// 0x5DAC
void dInsFldBank_c::addSharer(dInsFldBank_c *bank) {
    dInsFldBank_c **link = &mSharers;
    while (*link != NULL) {
        link = &(*link)->mSharers;
    }
    *link = bank;
}

// 0x5DCC
void dInsFldBank_c::removeSharer(dInsFldBank_c *bank) {
    dInsFldBank_c **link = &mSharers;
    dInsFldBank_c *p;
    while ((p = *link) != bank && p != NULL) {
        link = &p->mSharers;
    }
    if (p != NULL) {
        *link = bank->mSharers;
        bank->mSharers = NULL;
    }
}

// 0x5E08
void dInsFldBank_c::copyFrom(dInsFldBank_c *bank) {
    s32 size = bank->getSize();
    mpData = mpBankHeap->alloc(size, 0x20);
    memcpy(mpData, bank->mpData, size);
    DCFlushRange(mpData, size);
    dDvd::brresBank_c::onLoaded();
    mState = STATE_LOADED;
}

// 0x5EBC
dInsFldBase_c::dInsFldBase_c()
    : mInitPos(0.0f, 0.0f, 0.0f), mPos(0.0f, 0.0f, 0.0f), mInitAngle(0, 0, 0),
      mAngle(0, 0, 0), mFrame(0), mNext(NULL), mAlpha(0xFF),
      mCatchState(INSECT_NET_STATE_NORMAL), mVelocity(0.0f, 0.0f, 0.0f),
      mShadowSize(0.0f), mScareRadius(0.0f), mBaseRadius(0.0f), mBgRadius(0.0f),
      mHoldExecute(NULL), mHoldDraw(NULL), mpRef(NULL), mLinks(NULL),
      mLifeTimer(LIFE_TIME), mDeleteReq(FALSE), mRemoteDeleteReq(FALSE),
      mReleased(FALSE), mHoldStarted(FALSE), mActive(TRUE), mMdlCreated(FALSE),
      mPlayer(INSECT_FIELD_PLAYER_NONE),
      mBgFlags(dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_SET_POS) {
}

// 0x60B4
dInsFldBase_c::~dInsFldBase_c() {
}

// 0x6198
BOOL dInsFldBase_c::create(const nw4r::g3d::ResFile &res) {
    initParam();
    mHomePos = mPos;
    createMdl(res);
    if (dInsectField_c::getInstance()->getLifecycleState() !=
        fBase_c::DELETING) {
        setGroundPos();
        initState();
    }
    return TRUE;
}

// 0x6240
void dInsFldBase_c::calcMtx() {
    setMtxFromAngle();
}

// 0x6244
int dInsFldBase_c::execute() {
    mNpcCheck.setPos(mPos);
    mPlayerCheck.setPos(mPos);
    mOldPos = mPos;
    executeState();
    setAlpha(mAlpha);
    mFrame++;
    if (dNet::isSessionActive() == true) {
        if (mBgCheck == TRUE && dNet::isSceneOwner() == TRUE) {
            mAcch.checkSteps(mBgRadius, &mPos, &mOldPos, mAngle.y, 0, mBgFlags);
        }
    } else if (mBgCheck == TRUE) {
        mAcch.checkSteps(mBgRadius, &mPos, &mOldPos, mAngle.y, 0, mBgFlags);
    }
    calcScare();
    mMdl.play();
    return TRUE;
}

// 0x6378
int dInsFldBase_c::draw() {
    if (!mActive) {
        return TRUE;
    }
    calcMtx();
    mMdl.calc(false);
    mMdl.entry();
    if (!isZero(mShadowSize)) {
        f32 alpha = 0.5f * (mAlpha / 255.0f);
        mAng angle(0);
        dShadow::requestRound(&mPos, 0, &angle, mShadowSize, 1.0f, alpha, 0.0f,
                              1.0f);
    }
    return TRUE;
}

// .rodata 0xE84: the round shadow's alpha scale in the kinds' draw().
const f32 dInsFldBase_c::sShadowAlphaScale = 0.5f;

// 0x6470
BOOL dInsFldBase_c::doDelete() {
    if (mHoldExecute || mHoldDraw) {
        return FALSE;
    }
    releasePlayer();
    if (mpRef != NULL) {
        *mpRef = NULL;
        mpRef = NULL;
    }
    if (mMdlCreated) {
        mMdl.remove();
        mAnmChr.remove();
        mAnmVis.remove();
    }
    while (mLinks != NULL) {
        dInsectField::removeNpcLink(this, mLinks);
    }
    return TRUE;
}

namespace dInsectField {

// 0x6584
void addNpcLink(void *insect, void *link) {
    dInsFldBase_c *self = static_cast<dInsFldBase_c *>(insect);
    dInsectLink_c *node = static_cast<dInsectLink_c *>(link);
    dInsectLink_c **p = &self->mLinks;
    while (*p != NULL) {
        p = &(*p)->mpNext;
    }
    *p = node;
    node->mpNext = NULL;
    node->mpInsect = self;
}

// 0x65B0
void removeNpcLink(void *insect, void *link) {
    dInsFldBase_c *self = static_cast<dInsFldBase_c *>(insect);
    dInsectLink_c *node = static_cast<dInsectLink_c *>(link);
    dInsectLink_c **p = &self->mLinks;
    while (*p != node) {
        p = &(*p)->mpNext;
    }
    if (*p == node) {
        *p = node->mpNext;
        node->mpNext = NULL;
        node->mpInsect = NULL;
    }
}

} // namespace dInsectField

// 0x65E4
BOOL dInsFldBase_c::isActive() {
    return TRUE;
}

// 0x65EC
void dInsFldBase_c::onBecomeOwner() {
    if (mPos.y < dBGCF::getGroundY(&mPos, TRUE)) {
        mVec3_c pos;
        pos.x = getCatchData()->getPosX() * 4;
        pos.z = getCatchData()->getPosZ() * 4;
        pos.y = dBGCF::getGroundY(&pos, 0);
        mPos = pos;
        mOldPos = mPos;
    }
}

// 0x66E8
void dInsFldBase_c::onHold() {
}

// 0x66EC
void dInsFldBase_c::setHeldPos(const mVec3_c *pos) {
    mPos = *pos;
}

// 0x6708
void dInsFldBase_c::setHeldScale(const mVec3_c *scale) {
    dFIObj_c *obj = dFIMdlMgr_c::sInstance->getObj(mPlayer);
    if (obj != NULL) {
        obj->mMdl.setScale(*scale);
    }
}

// 0x675C
void dInsFldBase_c::cancelHold() {
    clearHoldProcs();
    releasePlayer();
    if (mSpawnKind == SPAWN_PLAYER) {
        reinterpret_cast<dInsectField_c *>(g_playActorMng)->deleteInsect(this);
    } else {
        requestDelete();
    }
}

// 0x67F8
void dInsFldBase_c::release() {
    if (mMdlCreated == TRUE && !mReleased) {
        clearHoldProcs();
        mActive = TRUE;
        mFrame = 0;
        mVelocity.set(0.0f, 0.0f, 0.0f);
        mAngle.set(0, 0, 0);
        mAngle.y = mPlayerCheck.getPlayerAngle(mPlayer);
        const mAng angle = mAngle.y;
        if (angle >= -0x555 && angle <= 0x555) {
            mAngle.y += 0xAAB;
        } else if (angle <= -0x7AAB || angle >= 0x7AAB) {
            mAngle.y += 0xAAB;
        }
        onRelease();
        mVec3_c *hand = dPlayerMgr::getHandPosition(mPlayer);
        if (hand != NULL) {
            mPos = *hand;
        }
        mReleased = TRUE;
    }
}

// 0x6900
void dInsFldBase_c::initHold() {
    mActive = FALSE;
    mShadowSize = 0.0f;
    initHoldMdl();
}

// 0x6918
void dInsFldBase_c::initHoldMdl() {
    mAngle.set(0, 0, 0);
    mHoldExecute = &dInsFldBase_c::waitHoldMdl;
    waitHoldMdl();
}

// 0x6948
void dInsFldBase_c::waitHoldMdl() {
    if (dFIMdlMgr_c::sInstance->request(mPlayer, dFIMdlBank_c::TYPE_INSECT_MENU,
                                        mType, TRUE, dFIMdlBank_c::MODE_GET)) {
        setHeldScale(&mVec3_c::Zero);
        startHold();
    }
}

// 0x69B0
void dInsFldBase_c::startHold() {
    mActive = TRUE;
    mHoldExecute = &dInsFldBase_c::executeHold;
    mHoldDraw = &dInsFldBase_c::drawHold;
    executeHold();
}

// 0x69F4
void dInsFldBase_c::executeHold() {
    dFIObj_c *obj = dFIMdlMgr_c::sInstance->getObj(mPlayer);
    mVec3_c *hand = dPlayerMgr::getHandPosition(mPlayer);
    if (hand != NULL) {
        mPos = *hand;
    }
    mAngle.set(-0x1770, 0, 0);
    mMtx_c mtx;
    dActor_c::makeMtx(&mtx, &mPos, 0);
    nw4r::g3d::ResFile res(dFIMdlMgr_c::sInstance->mBanks[mPlayer].getData());
    if (!res.GetResAnmChr("get").IsValid()) {
        mtx.YrotM(-DEG_TO_ANGLE(180));
        mtx.XrotM(-DEG_TO_ANGLE(90));
    }
    mMtx_c rot;
    rot.XrotS(mAngle.x);
    rot.ZrotM(mAngle.z);
    PSMTXConcat(mtx, rot, mtx);
    obj->mMdl.setLocalMtx(&mtx);
    obj->mMdl.play();
    if (obj->mAnmTexSrt.IsBound()) {
        obj->mAnmTexSrt.play();
    }
    obj->mMdl.calc(false);
    if (mType == INSECT_FIREFLY) {
        mMtx_c node;
        obj->mMdl.getNodeWorldMtx(0, &node);
        mVec3_c col0;
        mVec3_c col1;
        mVec3_c ofs(l_glowOfs.x, l_glowOfs.y, l_glowOfs.z);
        mMtx_c trans;
        PSMTXTrans(trans, ofs.x, ofs.y, ofs.z);
        PSMTXConcat(node, trans, node);
        col0.set(node.m[0][0], node.m[1][0], node.m[2][0]);
        col1.set(node.m[0][1], node.m[1][1], node.m[2][1]);
        f32 scale = PSVECMag(col1);
        f32 mag0 = PSVECMag(col0);
        scale = mag0 > scale ? scale : mag0;
        node.m[0][0] = node.m[1][1] = node.m[2][2] = scale;
        node.m[0][1] = node.m[0][2] = node.m[1][0] = node.m[1][2] =
            node.m[2][0] = node.m[2][1] = 0.0f;
        mEffect.create("af_ins_hotaru_glow", &node, 1);
    }
}

// 0x6C94
void dInsFldBase_c::drawHold() {
    dFIObj_c *obj = dFIMdlMgr_c::sInstance->getObj(mPlayer);
    obj->mMdl.entry();
    if (mType == INSECT_FIREFLY && dEffectLight_c::sInstance != NULL) {
        nw4r::ut::Color color;
        color.Set(0x40, 0xFF, 0x80, 0xFF);
        dEffectLight_c::sInstance->addGlow(&color, &mPos, 32.0f);
    }
}

// 0x6D60
void dInsFldBase_c::clearHoldProcs() {
    mHoldExecute = NULL;
    mHoldDraw = NULL;
}

// 0x6D98
void dInsFldBase_c::initParam() {
    mAcch.init();
    mScare.clear();
    mScare.setLimit(getParam()->mScareLimit);
    mScare.setEscapeLimit(getParam()->mEscapeLimit);
    mShadowSize = getParam()->mShadowSize;
    mScareRadius = getParam()->mScareRadius;
    mBgRadius = mBaseRadius = getParam()->mRadius;
    mBgCheck = TRUE;
}

// 0x6E3C
void dInsFldBase_c::setGroundPos() {
    mPos.y += getGroundY();
}

// 0x6E74
void dInsFldBase_c::createMdl(const nw4r::g3d::ResFile &res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    if (res.HasResAnmVis() == TRUE) {
        if (res.HasResAnmTexSrt() == TRUE) {
            mMdl.create(mdl, &mAllocator,
                        nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR |
                            nw4r::g3d::ScnMdl::ANM_VIS |
                            nw4r::g3d::ScnMdl::ANM_TEXSRT,
                        1, NULL);
        } else {
            mMdl.create(mdl, &mAllocator,
                        nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR |
                            nw4r::g3d::ScnMdl::ANM_VIS,
                        1, NULL);
        }
    } else {
        mMdl.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR, 1,
                    NULL);
    }
    mAnmChrWait = res.GetResAnmChr("wait");
    mAnmChrFly = res.GetResAnmChr("fly");
    mAnmChrStop = res.GetResAnmChr("stop");
    if (mAnmChrWait.IsValid()) {
        mAnmChr.create(mdl, mAnmChrWait, &mAllocator, NULL);
    } else if (mAnmChrFly.IsValid()) {
        mAnmChr.create(mdl, mAnmChrFly, &mAllocator, NULL);
    }
    if (res.HasResAnmVis() == TRUE) {
        mAnmVisWait = res.GetResAnmVis("wait");
        mAnmVisFly = res.GetResAnmVis("fly");
        mAnmVisStop = res.GetResAnmVis("stop");
        if (mAnmVisWait.IsValid()) {
            mAnmVis.create(mdl, mAnmVisWait, &mAllocator, NULL);
        } else {
            mAnmVis.create(mdl, mAnmVisFly, &mAllocator, NULL);
        }
    }
    mMdlCreated = TRUE;
}

// 0x708C
void dInsFldBase_c::setAnmChr(const nw4r::g3d::ResAnmChr &anm, f32 blend) {
    if (anm.IsValid()) {
        mAnmChr.setAnm(mMdl, anm, m3d::PLAYMODE_INHERIT);
        mMdl.setAnm(mAnmChr, blend);
    }
}

// 0x70F4
void dInsFldBase_c::setAnmVis(const nw4r::g3d::ResAnmVis &anm) {
    if (anm.IsValid()) {
        mAnmVis.setAnm(mMdl, anm, m3d::PLAYMODE_INHERIT);
        mMdl.setAnm(mAnmVis);
    }
}

// 0x7158
void dInsFldBase_c::setAnmRate(f32 rate) {
    if (mAnmChr.IsBound() == true) {
        if (rate <= mAnmChr.mFrameMax) {
            mAnmChr.setRate(rate);
        } else {
            mAnmChr.setRate(mAnmChr.mFrameMax);
        }
    }
    if (mAnmVis.IsBound() == true) {
        if (rate <= mAnmVis.mFrameMax) {
            mAnmVis.setRate(rate);
        } else {
            mAnmVis.setRate(mAnmVis.mFrameMax);
        }
    }
}

// 0x7204
void dInsFldBase_c::setAnmFrame(f32 frame) {
    if (mAnmChr.IsBound() == true) {
        mAnmChr.setFrame(frame);
    }
    if (mAnmVis.IsBound() == true) {
        mAnmVis.setFrame(frame);
    }
}

// 0x7270
void dInsFldBase_c::setAlpha(u8 alpha) {
    mMdl.setDrawMode(nw4r::g3d::RESMDL_DRAWMODE_SORT_XLU_Z);
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat("m1");
    if (mat.IsValid()) {
        nw4r::g3d::ScnMdl *scnMdl =
            nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mMdl.getScn());
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

// 0x7398
bool dInsFldBase_c::isUnderGround() {
    return mPos.y < getGroundY();
}

// 0x73D4
bool dInsFldBase_c::isUnderWater() {
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 1, 0);
    if (chk.isUnderWater(mPos.y) == TRUE) {
        return true;
    }
    return false;
}

// 0x7434
f32 dInsFldBase_c::getGroundY() {
    return dBGCF::getGroundY(&mPos, 0);
}

// 0x7440
void dInsFldBase_c::fadeOut() {
    if (mFrame >= 24) {
        f32 alpha = mAlpha;
        sLib::addCalc(&alpha, 0.0f, 28.0f, 8.0f, 255.0f);
        mAlpha = alpha;
    }
}

// 0x74C8
BOOL dInsFldBase_c::isHeld() const {
    return mPlayer != INSECT_FIELD_PLAYER_NONE;
}

// Speculative stand-in. The REL's constant pool creates 1.3f and -0.08f here
// (after fadeOut's 28.0f, before fly's -0.8f) but nothing references them: a
// function the linker stripped made them.
f32 dInsFldBase_strippedMoveRate(f32 speed) {
    return speed * -0.08f + 1.3f;
}

// 0x74E0
void dInsFldBase_c::moveForward(f32 speed, f32 accelY) {
    f32 vy = mVelocity.y;
    mVelocity.z = speed;
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.rotY(mAngle.y);
    mVelocity.y = vy + accelY;
    mVec3_c pos = mPos;
    mPos += mVelocity;
    mAcch.checkSteps(10.0f, &mPos, &pos, mAngle.y, 0,
                     dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL |
                         dBGCF::CHECK_SET_POS);
    fadeOut();
    if (mAlpha == 0) {
        requestDelete();
    }
}

// 0x75F0
BOOL dInsFldBase_c::isUnitTaken(const mVec3_c *pos) {
    dInsFldBase_c *insect = dInsectField_c::getInstance()->mActive;
    int bx = 0, bz = 0, ux = 0, uz = 0;
    dFdBase_c::posToBlockUnit(&bx, &bz, &ux, &uz, pos);
    int obx = 0, obz = 0, oux = 0, ouz = 0;
    for (; insect != NULL; insect = insect->mNext) {
        if (this != insect) {
            dFdBase_c::posToBlockUnit(&obx, &obz, &oux, &ouz, &insect->mPos);
            if (bx == obx && bz == obz && ux == oux && uz == ouz) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 0x76EC
bool dInsFldBase_c::isSceneChanging() {
    if (dInsectField::getSceneChange()->isChanging() == TRUE) {
        return true;
    }
    return dStrm_c::sDoorEntered == 1;
}

// 0x7740
void dInsFldBase_c::calcScare() {
    int player = mPlayerCheck.getFastest(mScareRadius);
    if (player < INSECT_UTIL_PLAYER_NUM) {
        f32 rate = 1.0f;
        if (mPlayerCheck.isNear(player, mScareRadius / 2.0f) == true) {
            rate = 3.0f;
        }
        mScare.add(mPlayerCheck.getPlayerSpeed(player), rate);
    } else if (mNpcCheck.isNear(mScareRadius) == true) {
        // Discarded near-rate predicate, likely residue of a stripped debug
        // check.
        (void)(mNpcCheck.isNear(mScareRadius / 2.0f) == true ? 3.0f : 1.0f);
        mScare.add(mNpcCheck.getNpcSpeed(), 1.0f);
    } else {
        mScare.add(0.0f, 1.0f);
    }
}

// 0x7840
dItem::Item dInsFldBase_c::getItem() {
    dItem::Item item;
    f32 z = mPos.z;
    f32 y = mPos.y;
    f32 x = mPos.x;
    mVec3_c pos(x, y, z);
    dItem::Item *p = dInsectUtil::getItem(&pos);
    if (p != NULL) {
        item = *p;
    }
    return item;
}

// 0x78A4
void dInsFldBase_c::setMtxFromAngle() {
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

// 0x795C
void dInsFldBase_c::setMtxFromAxis() {
    mMtx_c mtx;
    dActor_c::makeMtx(&mtx, &mPos, 0);
    mMtx_c rot;
    mMtx_c axis;
    dInsectUtil::getMtx(axis, mAxis);
    rot = axis;
    PSMTXConcat(mtx, rot, mtx);
    mMdl.setLocalMtx(&mtx);
}

// 0x7A24
void dInsFldBase_c::fly(f32 lift, f32 accel, f32 drag, f32 turn) {
    mVec3_c toHome = mHomePos - mPos;
    toHome.y = 0.0f;
    toHome.normalize();
    mVec3_c front = mAxis.getFront();
    mVec3_c side(0.0f, 0.0f, 0.0f);
    if (front.x * toHome.x + front.z * toHome.z < -0.8f) {
        side = mAxis.getSide();
    } else {
        side = toHome - front;
    }
    front += side * turn;
    front.normalize();
    mAxis.setFront(front);
    mVec3_c move = front * accel;
    mVelocity.x *= drag;
    mVelocity.z *= drag;
    mVelocity.y += lift;
    mVelocity += move;
    mPos += mVelocity;
}

// 0x7C30
const dInsectField::param_c *dInsFldBase_c::getParam() const {
    return dInsectField::sParamTable[mType];
}

// 0x7C48
void dInsFldBase_c::requestDelete() {
    if (dNet::isSessionActive() && !dNet::isSceneOwner() &&
        mHeapList == &dInsectField_c::getInstance()->m_objHeap) {
        mRemoteDeleteReq = TRUE;
    } else {
        mDeleteReq = TRUE;
    }
    releasePlayer();
    while (mLinks != NULL) {
        dInsectField::removeNpcLink(this, mLinks);
    }
}

// 0x7D14
BOOL dInsFldBase_c::isDeleteRequested() const {
    BOOL ret = FALSE;
    if (mDeleteReq || mRemoteDeleteReq) {
        ret = TRUE;
    }
    return ret;
}

// 0x7D3C
BOOL dInsFldBase_c::isDeleteReady() {
    BOOL ret = FALSE;
    if (mDeleteReq ||
        (dNet::isSessionActive() && dNet::isSceneOwner() && mRemoteDeleteReq)) {
        ret = TRUE;
    }
    return ret;
}

// 0x7DA8
void dInsFldBase_c::registObjc(const mVec3_c *pos, f32 radius, BOOL enable) {
    mObjc.regist(pos, radius + 4.0f, (u32)this, enable);
}

// 0x7DC4
int dInsFldBase_c::tryCatch(int player) {
    int ret = CATCH_MISSED;
    if (dNet::isSessionActive()) {
        if (mCatchState == INSECT_NET_STATE_NORMAL && !dNet::isSceneOwner()) {
            mCatchState = CATCH_STATE_REQUESTED;
            mActive = FALSE;
            dInsectField::catchMsg_c msg;
            msg.setType(dInsectField::catchMsg_c::TYPE_REQUEST);
            msg.setIdx(mHeapIdx);
            msg.setValue(mType);
            msg.setSerial(mSerial);
            msg.setMember(0);
            dNet::beginPacket();
            dNet::addPacket(&msg, sizeof(msg));
            dNet::sendPacket(NET_PACKET_INSECT_FIELD, NET_DEST_SCENE_OWNER);
        }
        if (mCatchState == CATCH_STATE_REQUESTED) {
            return CATCH_PENDING;
        }
        if (mCatchState == CATCH_STATE_MISSED) {
            return CATCH_MISSED;
        }
        if (!dNet::isSceneOwner()) {
            getCatchData();
            return isHeld() ? CATCH_CAUGHT : CATCH_PENDING;
        }
    }
    if (mPlayer == INSECT_FIELD_PLAYER_NONE) {
        ret = CATCH_CAUGHT;
        attachToPlayer(player);
        if (dNet::isSessionActive() && dNet::isSceneOwner()) {
            dInsectCatchData_c *data = getCatchData();
            if (data != NULL) {
                data->setState(player);
                data->setReq(INSECT_FIELD_REQ_HELD);
            }
        }
    } else if (player == mPlayer) {
        ret = CATCH_CAUGHT;
    }
    return ret;
}

// 0x7F60
void dInsFldBase_c::setHeld(int player) {
    mPlayer = player;
    dFIMdlMgr_c::sInstance->request(player, dFIMdlBank_c::TYPE_INSECT_MENU,
                                    mType, TRUE, dFIMdlBank_c::MODE_GET);
    mActive = FALSE;
    reinterpret_cast<dInsectField_c *>(g_playActorMng)->mNetInsects[player] =
        this;
}

// 0x7FD0
void dInsFldBase_c::attachToPlayer(int player) {
    mPlayer = player;
    mActive = FALSE;
    dInsectField_c::getInstance()->mNetInsects[player] = this;
}

// 0x7FF4
void *dInsFldBase_c::operator new(size_t size, EGG::Heap *heap) {
    void *p = EGG::Heap::alloc(size, 4, heap);
    if (p == NULL) {
        return NULL;
    }
    memset(p, 0, size);
    return p;
}

// 0x8054
void dInsFldBase_c::checkPtclLanding(nw4r::ef::Particle *ptcl, u32 arg) {
    if (ptcl == NULL) {
        return;
    }
    mVec3_c pos;
    mVec3_c ptclPos(ptcl->mParameter.mPosition);
    dWorld::toFieldPosition(&pos, &ptclPos);
    dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_TOP, 0, 0);
    f32 groundY = chk.getHeight(0);
    if (16.0f <= groundY - pos.y) {
        if (ptcl->mParticleManager != NULL) {
            ptcl->mParticleManager->RetireParticle(ptcl);
        }
    } else if (chk.mWater != BG_WATER_NONE) {
        if (chk.isUnderWater(pos.y) == TRUE) {
            pos.y = chk.getWaterY();
            dEffect::create("af_hny_hamon_S01", &pos, NULL, NULL);
            if (ptcl->mParticleManager != NULL) {
                ptcl->mParticleManager->RetireParticle(ptcl);
            }
        }
    } else if (pos.y <= chk.getHeight(0)) {
        if (ptcl->mParticleManager != NULL) {
            ptcl->mParticleManager->RetireParticle(ptcl);
        }
    }
}

// 0x8188
dInsectCatchData_c *dInsFldBase_c::getCatchData() const {
    dInsectField_c *mng = dInsectField_c::getInstance();
    if (mHeapList == &mng->m_objHeap) {
        return &mng->mCatchData[mHeapIdx];
    }
    return NULL;
}

// 0x81BC
BOOL dInsFldShow_c::create(const nw4r::g3d::ResFile &res) {
    initParam();
    initHold();
    return TRUE;
}

// 0x8200
int dInsFldShow_c::execute() {
    return TRUE;
}

// 0x8208
int dInsFldShow_c::draw() {
    return TRUE;
}

// 0x8210
dInsFldShow_c::~dInsFldShow_c() {
}

#ifdef MUST_MATCH
// Unused (dead-stripped by the linker): queues anmChr_c::getType ahead of the
// end-of-TU vtable pass that adds anmVis_c::getType (target .text order, as in
// d_fgobj_manager.cpp).
static void orderM3dWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif

#include "d_insect_field_param.inc"
