// The cockroaches of a player's house room (REL d_cockroach_managerNP). See
// include/game/game/d_cockroach_manager.hpp.
#include <game/game/d_cockroach_manager.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_cockroach.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/mLib/m_allocator_dummy_heap.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/core/eggExpHeap.h>

namespace dCockroachManager {

BASE_PROFILE(COCKROACH_MANAGER, dCockroachManager_c);

} // namespace dCockroachManager

dCockroachManager_c *dCockroachManager_c::sInstance;

static const u32 l_roomSceneAttr[] = {
    dScene::SCENE_ATTR_HOUSE0 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE1 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE2 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE3 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    0,
};

// 0x8C
dCockroachManager_c::dCockroachManager_c()
    : mpHeap(NULL), mHouse(0), mRoom(0), mTalkRequested(FALSE), mAllKilled(FALSE), mAreaMin(0.0f, 0.0f, 0.0f),
      mAreaMax(0.0f, 0.0f, 0.0f), mSpawned(FALSE), mTalkStep(0) {
    for (int i = 0; i < COCKROACH_ROOM_NUM; i++) {
        mpCockroach[i] = NULL;
    }
}

// 0x154
dCockroachManager_c::~dCockroachManager_c() {}

// 0x1D0
int dCockroachManager_c::create() {
    if (dNet::isSessionActive() == true) {
        setDemoID((u16)dNet::getMyNo());
    } else {
        setDemoID(0);
    }
    mRoom = dScene::getSceneAttrTableIndex(dScene::getCurrentScene(), l_roomSceneAttr, &mHouse);
    dBGCF::getArea(&mAreaMin.x, &mAreaMax.x, &mAreaMin.z, &mAreaMax.z);
    mAreaMax.z = 448.0f;
    if (!mAllocator.hasHeap()) {
        mpHeap = mHeap::createExpHeap(0x2400, dHeap::localHeap_p, "dCockroachManager_c::m_common_heap", 0x20,
                                      mHeap::OPT_NONE);
        static_cast<EGG::ExpHeap *>(mpHeap)->setAllocMode(0);
        mAllocator.attach(mpHeap, 0x20);
    }
    mRecords.create();
    if (sInstance == NULL) {
        sInstance = this;
    }
    if (dNet::isSessionActive() == true) {
        if (dNet::isSceneOwner() == TRUE && dSceneCheck::isRoomOpen(mHouse, mRoom) == TRUE) {
            dSceneCheck::clearRoomOpen(mHouse, mRoom);
            spawn();
        }
    } else {
        spawn();
    }
    return TRUE;
}

// 0x324
int dCockroachManager_c::execute() {
    mRecords.execute();
    if (dNet::isSessionActive() == true) {
        spawnRemote();
    }
    if (mAllKilled == TRUE && !mTalkRequested && dPlayerMgr::getPlayerActor(dNet::getMyNo()) != NULL) {
        if ((u32)dPlayerMgr::getState(dNet::getMyNo()) <= 1) {
            dDemo::requestSpeak(this, NULL);
        } else {
            mAllKilled = FALSE;
        }
    }
    if (mTalkRequested == TRUE && !mTalkEnded) {
        executeTalk();
    }
    return TRUE;
}

// 0x3DC
int dCockroachManager_c::draw() {
    return TRUE;
}

// 0x3E4
int dCockroachManager_c::doDelete() {
    mRecords.doDelete();
    sInstance = NULL;
    if (mpHeap != NULL) {
        mpHeap->destroy();
        mpHeap = NULL;
    }
    return TRUE;
}

// 0x44C
void dCockroachManager_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_SPEAK:
        mTalkRequested = TRUE;
        startPlayerReaction();
        break;
    case DEMO_CMD_END:
        mTalkEnded = TRUE;
        break;
    }
}

// 0x478
mAllocator_c *dCockroachManager_c::getAllocator() {
    return &mAllocator;
}

// 0x480
daCockroachBase_c *dCockroachManager_c::createCockroach(const mVec3_c *pos, const mAng3_c *angle, u8 idx) {
    return static_cast<daCockroachBase_c *>(dActor_c::construct(fProf::A_COCKROACH, this, idx, pos, angle));
}

// 0x49C
void dCockroachManager_c::kill(u8 idx) {
    u8 i = idx - (mHouse * COCKROACH_HOUSE_RECORD_NUM + mRoom * COCKROACH_ROOM_NUM);
    getRoom()->clearFlag(i);
    if (!hasCockroaches()) {
        mAllKilled = TRUE;
        dHome_c *home = dSaveData_c::getTown()->mHomes.getHome(mHouse);
        home->_15BA = 30;
    }
    if (dNet::isSessionActive() == true) {
        u8 packet = makeKillPacket(mHouse, mRoom, i);
        dNet::beginPacket();
        dNet::addPacket(&packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_COCKROACH_REMOVE, 4);
    }
}

// 0x570
BOOL dCockroachManager_c::hasCockroaches() {
    return dSaveData_c::getTown()->mHomes.getHome(mHouse)->hasFlags();
}

// 0x5B0
u8 dCockroachManager_c::isSpawned() {
    return mSpawned;
}

// 0x5B8
dCockroachRecord_c *dCockroachManager_c::getRecord(u8 idx) {
    return mRecords.get(idx);
}

// 0x5C0
void dCockroachManager_c::setRecord(const dCockroachRecord_c *record) {
    mRecords.set(record);
}

// 0x5C8
mVec3_c dCockroachManager_c::getAreaMin() const {
    return mAreaMin;
}

// 0x5E4
mVec3_c dCockroachManager_c::getAreaMax() const {
    return mAreaMax;
}

// 0x600
u32 dCockroachManager_c::isOutside(const mVec3_c &pos) {
    if (pos.x < mAreaMin.x || mAreaMax.x < pos.x || pos.z < mAreaMin.z || mAreaMax.z < pos.z) {
        return TRUE;
    }
    return FALSE;
}

// 0x648
u32 dCockroachManager_c::isOnFurniture(const mVec3_c &pos) {
    f32 groundY = dBGCF::getGroundY(&pos, FALSE);
    return groundY != dBGCF::getGroundY(&pos, NULL, 0x31);
}

// 0x6A8
void dCockroachManager_c::spawn() {
    mSpawned = TRUE;
    dFdBase_c *field = getCurrentField();
    int units[UT_TOTAL_NUM];
    int num = 0;
    for (int z = 0; z < UT_Z_NUM; z++) {
        for (int x = 0; x < UT_X_NUM; x++) {
            if (field->canPutItem(x, z) == TRUE) {
                // a local makes getUnitBaseY run before getRoomGroundY, as in the target
                f32 baseY = dBGCF::getUnitBaseY(x, z);
                if (baseY == dFTR::getRoomGroundY(x, z)) {
                    units[num] = x * UT_Z_NUM + z;
                    num++;
                }
            }
        }
    }
    if (num == 0) {
        mSpawned = FALSE;
        return;
    }
    dHomeRoom_c *room = getRoom();
    for (u8 i = 0; i < COCKROACH_ROOM_NUM; i++) {
        if (room->isFlag(i) == TRUE) {
            int k = cM::rndRange<int>(0, num - 1);
            int unit = units[k];
            mVec3_c pos;
            dFdBase_c::getUnitGroundPos(&pos, unit / UT_Z_NUM, unit % UT_Z_NUM);
            // Record i, not getRecordIdx(i): a bug in the original (setRecord files it under its new index).
            dCockroachRecord_c *record = getRecord(i);
            record->setIdx(getRecordIdx(i));
            record->setState(daCockroach_c::STATE_WAIT);
            record->setX(pos.x);
            record->setZ(pos.z);
            record->setPlayer(dCockroachRecord_c::PLAYER_NONE);
            setRecord(record);
            mAng3_c angle(0, 0, 0);
            mpCockroach[i] = createCockroach(&pos, &angle, getRecordIdx(i));
            units[k] = units[num - 1];
            if (--num <= 0) {
                num = 1;
            }
        }
    }
    if (getRoom()->countFlags() == 0) {
        mSpawned = FALSE;
    }
}

// 0x928
void dCockroachManager_c::spawnRemote() {
    for (u8 i = 0; i < COCKROACH_ROOM_NUM; i++) {
        dCockroachRecord_c *record = getRecord(getRecordIdx(i));
        if (0.0f != record->getX() && record->getPlayer() == dCockroachRecord_c::PLAYER_NONE &&
            mpCockroach[i] == NULL) {
            mVec3_c pos(record->getX(), 0.0f, record->getZ());
            mAng3_c angle(0, 0, 0);
            mpCockroach[i] = createCockroach(&pos, &angle, getRecordIdx(i));
            mpCockroach[i]->mRecordIdx = getRecordIdx(i);
            mSpawned = TRUE;
        }
    }
}

// 0xAA8
u8 dCockroachManager_c::getRecordIdx(u8 i) {
    return mHouse * COCKROACH_HOUSE_RECORD_NUM + mRoom * COCKROACH_ROOM_NUM + i;
}

// 0xAD0
u8 dCockroachManager_c::makeKillPacket(int house, int room, int i) {
    return ((house & 3) << 6) | ((room & 3) << 4) | (i & 3);
}

// 0xAE0
dHomeRoom_c *dCockroachManager_c::getRoom() {
    return dSaveData_c::getTown()->mHomes.getHome(mHouse)->getRoom(mRoom);
}

// 0xB24
bool dCockroachManager_c::startPlayerReaction() {
    return dPlayerMgr::reactToCockroaches() == 1;
}

// 0xB50
void dCockroachManager_c::executeTalk() {
    if (dPlayerMgr::isCockroachReactionFinished() == TRUE) {
        switch (mTalkStep) {
        case 0:
            attachDemoActor(this);
            setMessageLabel("sys_MESS/SYS_Player");
            setMessageCode(9);
            mpController->requestStart(0);
            mTalkStep = 1;
            break;
        case 1: {
            dMsg::Comp_c *comp = getController();
            if (comp != NULL) {
                if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
                    mTalkStep = 2;
                }
            }
            break;
        }
        case 2: {
            dMsg::Comp_c *comp = getController();
            if (comp != NULL) {
                if (comp->isState(&dMsg::Comp_c::executeHidden)) {
                    detachDemoActor(this);
                    dDemo::finishTalk(this);
                }
            }
            break;
        }
        }
    }
}

// 0xCB0
dCockroachRecordList_c::dCockroachRecordList_c() {}

// 0xE0C
dCockroachRecordList_c::~dCockroachRecordList_c() {}

// 0xE4C
int dCockroachRecordList_c::create() {
    clear();
    if (dNet::isSessionActive() == true) {
        receive();
    }
    return TRUE;
}

// 0xE90
int dCockroachRecordList_c::execute() {
    if (dNet::isSessionActive() == true) {
        if (dNet::isSceneOwner() == TRUE) {
            send();
        } else {
            receive();
        }
    }
    return TRUE;
}

// 0xEE8
int dCockroachRecordList_c::doDelete() {
    return TRUE;
}

// 0xEF0
void dCockroachRecordList_c::set(const dCockroachRecord_c *record) {
    u8 idx = record->getIdx();
    if (idx >= COCKROACH_RECORD_NUM) {
        idx = 0;
    }
    mRecords[idx] = *record;
}

// 0xF40
dCockroachRecord_c *dCockroachRecordList_c::get(u8 idx) {
    // u32 copy: the target indexes without a clrlwi
    u32 i = idx;
    if (i < COCKROACH_RECORD_NUM) {
        return &mRecords[i];
    }
    return mRecords;
}

// 0xF58
void dCockroachRecordList_c::clear() {
    for (u8 i = 0; i < COCKROACH_RECORD_NUM; i++) {
        mRecords[i].setIdx(dCockroachRecord_c::RECORD_IDX_NONE);
        mRecords[i].setState(0);
        mRecords[i].setX(0);
        mRecords[i].setZ(0);
        mRecords[i].setPlayer(0);
    }
}

// 0x102C
void dCockroachRecordList_c::send() {
    int house = 0;
    int room = dScene::getSceneAttrTableIndex(dScene::getCurrentScene(), l_roomSceneAttr, &house);
    u8 base = house * COCKROACH_HOUSE_RECORD_NUM + room * COCKROACH_ROOM_NUM;
    if (base >= COCKROACH_RECORD_NUM - 2) {
        base = 0;
    }
    for (u8 i = 0; i < COCKROACH_ROOM_NUM; i++) {
        dNet::setRecord(base + i + NET_REC_HOUSE, &mRecords[base + i], 0);
    }
}

// 0x10DC
void dCockroachRecordList_c::receive() {
    int house = 0;
    int room = dScene::getSceneAttrTableIndex(dScene::getCurrentScene(), l_roomSceneAttr, &house);
    u8 base = house * COCKROACH_HOUSE_RECORD_NUM + room * COCKROACH_ROOM_NUM;
    if (base >= COCKROACH_RECORD_NUM - 2) {
        base = 0;
    }
    for (u8 i = 0; i < COCKROACH_ROOM_NUM; i++) {
        cLib::memCpy(&mRecords[base + i], dNet::getRecord(base + i + NET_REC_HOUSE), sizeof(dCockroachRecord_c));
    }
}
