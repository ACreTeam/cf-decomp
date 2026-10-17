// Furniture: the furniture actors' shared state, room unit map and online sync.
#include <game/game/d_ftr.hpp>
#include <game/game/d_a_ftr.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_home_room_map.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <nw4r/g3d.h>
#include <string.h>

namespace dFTR {

struct shapeData_c {
    /* 0x00 */ const int (*mOfs[4])[2];
    /* 0x10 */ u8 mNum;
};

static const int l_ofs1x1[][2] = {{0, 0}};
static const shapeData_c l_shape1x1 = {{l_ofs1x1, l_ofs1x1, l_ofs1x1, l_ofs1x1}, 1};
static const int l_ofs1x2_0[][2] = {{0, 0}, {1, 0}};
static const int l_ofs1x2_1[][2] = {{0, 0}, {0, -1}};
static const int l_ofs1x2_2[][2] = {{0, 0}, {-1, 0}};
static const int l_ofs1x2_3[][2] = {{0, 0}, {0, 1}};
static const shapeData_c l_shape1x2 = {{l_ofs1x2_0, l_ofs1x2_1, l_ofs1x2_2, l_ofs1x2_3}, 2};
static const int l_ofs2x2[][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};
static const shapeData_c l_shape2x2 = {{l_ofs2x2, l_ofs2x2, l_ofs2x2, l_ofs2x2}, 4};
static const int l_dir[][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

static const shapeData_c *l_shapeData[] = {&l_shape1x1, &l_shape1x2, &l_shape2x2};

inline lockList_c::~lockList_c() {}

inline lock_c::lock_c() {
    clear();
}

void lock_c::clear() {
    mActive = FALSE;
    mPos.set(0.0f, 0.0f, 0.0f);
    mScene = 0;
}

BOOL lock_c::set(u8 scene, const mVec3_c &pos) {
    mActive = TRUE;
    mPos = pos;
    mScene = scene;
    return TRUE;
}

BOOL lock_c::reset() {
    mActive = FALSE;
    return TRUE;
}

void lockList_c::clear() {
    for (u32 i = 0; i < 4; i++) {
        mLock[i].clear();
    }
}

BOOL lockList_c::set(int member, u8 scene, const mVec3_c &pos) {
    if (member < 4) {
        return mLock[member].set(scene, pos);
    }
    return TRUE;
}

BOOL lockList_c::reset(int member) {
    if (member < 4) {
        return mLock[member].reset();
    }
    return FALSE;
}

BOOL lockList_c::check(u32 member, u8 scene, const mVec3_c &pos) {
    BOOL online = FALSE;
    if (dNet::isSessionActive() && dNet::getMemberNum() >= 2) {
        online = TRUE;
    }
    u32 num = online ? 4 : 1;
    if (member < num) {
        for (u32 i = 0; i < num; i++) {
            if (i != member && mLock[i].mActive && scene == mLock[i].mScene) {
                mVec3_c dist = pos - mLock[i].mPos;
                if (dist.x * dist.x + dist.z * dist.z < 576.0f) {
                    return FALSE;
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

BOOL lockList_c::lock(int member, u8 scene, const mVec3_c &pos) {
    if (check(member, scene, pos)) {
        return set(member, scene, pos);
    }
    return FALSE;
}

shape_c *shape_c::set(dItem::Item item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm != NULL) {
        mSize = bitm->getFtrSize();
        mRot = item.mId & 3;
    } else {
        mSize = 0;
        mRot = 0;
    }
    return this;
}

u32 shape_c::getNum() const {
    if (mSize < 3) {
        return l_shapeData[mSize]->mNum;
    }
    return 0;
}

const int *shape_c::getOfs(u32 i) const {
    static const int l_zero[2] = {0, 0};
    if (mSize < 3 && i < getNum()) {
        return l_shapeData[mSize]->mOfs[mRot & 3][i];
    }
    return l_zero;
}

void shape_c::getCenterPos(mVec3_c *pos, int x, int z) {
    mVec3_c sum(0.0f, 0.0f, 0.0f);
    u32 num = getNum();
    for (u32 i = 0; i < num; i++) {
        int ux = x + getOfs(i)[0];
        int uz = z + getOfs(i)[1];
        mVec3_c unitPos;
        dFdBase_c::getUnitCenterPos(&unitPos, ux, uz);
        sum += unitPos;
    }
    f32 inv = 1.0f / num;
    sum *= inv;
    *pos = sum;
}

void unitMap_c::clear() {
    for (u32 layer = 0; layer < 2; layer++) {
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                mHandle[layer][z][x] = 0xFF;
            }
        }
    }
}

BOOL unitMap_c::set(daFTR_c *ftr, int x, int z, u8 layer) {
    u8 &handle = mHandle[layer & 1][z & 0xF][x & 0xF];
    int idx = getList()->find(ftr);
    if (idx != -1) {
        handle = idx;
        return TRUE;
    }
    return FALSE;
}

BOOL unitMap_c::remove(daFTR_c *ftr, int x, int z, u8 layer) {
    u8 &handle = mHandle[layer & 1][z & 0xF][x & 0xF];
    int idx = getList()->find(ftr);
    if (idx != -1) {
        handle = 0xFF;
        return TRUE;
    }
    return FALSE;
}

int unitMap_c::getHandle(int x, int z, int layer) {
    u8 handle = mHandle[layer & 1][z & 0xF][x & 0xF];
    if (handle == 0xFF) {
        return -1;
    }
    return handle;
}

daFTR_c *unitMap_c::get(int x, int z, int layer) {
    int handle = getHandle(x, z, layer);
    if (handle != -1) {
        return getList()->get(handle);
    }
    return NULL;
}

daFTR_c *unitMap_c::get(const mVec3_c *pos, int layer) {
    f32 x = pos->x;
    f32 z = pos->z;
    return get((int)x >> 5, (int)z >> 5, layer);
}

static lockList_c l_lockList;
static unitMap_c l_unitMap;

unitMap_c *getUnitMap() {
    return &l_unitMap;
}

void list_c::clear() {
    for (int i = 0; i < 64; i++) {
        mFtr[i] = NULL;
    }
}

int list_c::add(daFTR_c *ftr) {
    u32 num = dScene::getSceneParamA(dScene::getCurrentScene());
    for (u32 i = 0; i < num; i++) {
        if (mFtr[i] == ftr) {
            return i;
        }
    }
    for (u32 i = 0; i < num; i++) {
        if (mFtr[i] == NULL) {
            mFtr[i] = ftr;
            return i;
        }
    }
    return -1;
}

BOOL list_c::remove(daFTR_c *ftr) {
    u32 num = dScene::getSceneParamA(dScene::getCurrentScene());
    for (u32 i = 0; i < num; i++) {
        if (mFtr[i] == ftr) {
            mFtr[i] = NULL;
            return TRUE;
        }
    }
    return FALSE;
}

u32 list_c::getNum() {
    u32 num = dScene::getSceneParamA(dScene::getCurrentScene());
    u32 count = 0;
    for (u32 i = 0; i < num; i++) {
        if (mFtr[i] != NULL) {
            count++;
        }
    }
    return count;
}

u32 list_c::getFreeNum() {
    u32 num = dScene::getSceneParamA(dScene::getCurrentScene());
    return num - getNum();
}

int list_c::find(daFTR_c *ftr) {
    u32 num = dScene::getSceneParamA(dScene::getCurrentScene());
    for (u32 i = 0; i < num; i++) {
        if (mFtr[i] == ftr) {
            return i;
        }
    }
    return -1;
}

daFTR_c *list_c::get(u32 handle) {
    u32 num = dScene::getSceneParamA(dScene::getCurrentScene());
    if (num <= 64) {
        if (handle < num) {
            return mFtr[handle];
        }
        return NULL;
    }
    return NULL;
}

static list_c l_list;

list_c *getList() {
    return &l_list;
}

static int l_holdState;

void recvHoldRequest(const void *data, int member) {
    const u8 *buf = (const u8 *)data;
    mVec3_c pos;
    pos.y = 0.0f;
    pos.x = buf[0] * 2;
    pos.z = buf[1] * 2;
    if (l_lockList.lock(member, buf[2], pos)) {
        dNet::beginPacket();
        dNet::sendPacket(NET_PACKET_FTR_HOLD_OK, member);
    } else {
        dNet::beginPacket();
        dNet::sendPacket(NET_PACKET_FTR_HOLD_NG, member);
    }
}

void recvHoldAnswer(BOOL ok) {
    int state = HOLD_STATE_REJECTED;
    if (ok) {
        state = HOLD_STATE_ACCEPTED;
    }
    l_holdState = state;
}

void recvHoldRelease(int member, int arg) {
    l_lockList.reset(member);
}

BOOL isGyroid(daFTR_c *ftr) {
    alwaysAc_c *ac = ftr;
    if (ac != NULL) {
        return ac->isGyroid();
    }
    return FALSE;
}

void setOwnerSwitch(u8 scene, int x, int z, u8 layer, BOOL arg) {
    u8 current = dScene::getCurrentScene();
    if (scene == current && dScene::isSceneAttr(current, dScene::SCENE_ATTR_ROOM)) {
        alwaysAc_c *ac = getUnitMap()->get(x, z, layer);
        if (ac != NULL) {
            if (!ac->getSwitch()) {
                if (countSwitch(isGyroid, 1) >= 8) {
                    daFTR_c *ftr = switchOffRandom(isGyroid, 0);
                    if (ftr != NULL) {
                        setSwitch(ftr, 0, 0xFF, arg, TRUE);
                    }
                }
                setSwitch(x, z, layer, 1, 0xFF, arg, TRUE);
            } else {
                setSwitch(x, z, layer, 0, 0xFF, arg, TRUE);
            }
        }
    }
}

struct ownerSwitchPacket_c {
    u16 mScene : 7;
    u16 mLayer : 1;
    u16 mX : 4;
    u16 mZ : 4;
};

void recvOwnerSwitch(const void *data) {
    const ownerSwitchPacket_c *packet = (const ownerSwitchPacket_c *)data;
    setOwnerSwitch(packet->mScene, packet->mX, packet->mZ, packet->mLayer, TRUE);
}

void sendOwnerSwitch(u8 scene, int x, int z, u8 layer) {
    if (!dNet::isSessionActive() || dNet::isSceneOwner()) {
        setOwnerSwitch(scene, x, z, layer, TRUE);
    } else {
        ownerSwitchPacket_c packet;
        packet.mScene = scene;
        packet.mLayer = layer;
        packet.mX = x;
        packet.mZ = z;
        dNet::beginPacket();
        dNet::addPacket(&packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_FTR_OWNER, NET_DEST_SCENE_OWNER);
    }
}

void sendOwnerSwitch(u8 scene, const mVec3_c *pos, u8 layer) {
    f32 x = pos->x;
    f32 z = pos->z;
    sendOwnerSwitch(scene, (int)x >> 5, (int)z >> 5, layer);
}

BOOL applySwitch(u8 scene, int x, int z, int layer, int sw, BOOL on, BOOL music, BOOL arg, u8 param) {
    if (scene == dScene::getCurrentScene()) {
        alwaysAc_c *ac = getUnitMap()->get(x, z, (u8)layer);
        if (ac != NULL && ac->canSetSwitch()) {
            return ac->setSwitch(sw, param);
        }
    }
    dFdBase_c *fd = dField::getInfo(dField::getSceneFieldId(scene));
    if (fd != NULL) {
        dItem::Item *item = fd->getItem(x, z, 0);
        if (item != NULL) {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
            if (bitm != NULL && bitm->getKind() == dItem::KIND_HANIWA) {
                if (on) {
                    addRoomGyroid(x, z, param & 0xF, scene);
                } else {
                    removeRoomGyroid(x, z, scene);
                }
            }
        }
    }
    setRoomMap(x, z, layer, on, scene);
    if (music) {
        if (on) {
            dItem::Item song(0xEE, param, 0);
            setRoomSong(scene, song);
        } else if (!arg) {
            clearRoomSong(scene);
        }
    }
    return TRUE;
}

struct switchPacket_c {
    u32 mParam : 8;
    u32 mX : 4;
    u32 mZ : 4;
    u32 mScene : 7;
    u32 mLayer : 1;
    u32 mOn : 1;
    u32 mSw : 4;
    u32 mRelay : 1;
    u32 mMusic : 1;
    u32 mArg : 1;
};

BOOL sendSwitch(int x, int z, int layer, int sw, u8 param, BOOL arg, int member) {
    bool relay = member != 4;
    alwaysAc_c *ac = getUnitMap()->get(x, z, layer);
    if (ac == NULL) {
        return FALSE;
    }
    if (dNet::isSessionActive() && ac != NULL && !ac->isSyncDisabled()) {
        switchPacket_c packet;
        BOOL on = ac->getSwitchBit(sw);
        packet.mScene = dScene::getCurrentScene();
        packet.mLayer = layer;
        packet.mOn = on;
        packet.mX = x;
        packet.mZ = z;
        packet.mSw = sw;
        packet.mParam = param;
        packet.mArg = arg;
        packet.mRelay = relay;
        packet.mMusic = ac->isMusic();
        dNet::beginPacket();
        dNet::addPacket(&packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_FTR_ACTION, member);
    }
    return TRUE;
}

BOOL setSwitch(daFTR_c *ftr, int sw, int param, BOOL arg, BOOL send) {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        alwaysAc_c *ac = ftr;
        int x;
        int z;
        if (ac->getUnit(&x, &z, NULL, mAng(0))) {
            return setSwitch(x, z, ac->getLayer(), sw, param, arg, send);
        }
    }
    return FALSE;
}

BOOL setSwitch(int x, int z, u8 layer, int sw, int param, BOOL arg, BOOL send) {
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        return FALSE;
    }
    daFTR_c *ftr = getUnitMap()->get(x, z, layer);
    if (ftr == NULL) {
        return FALSE;
    }
    alwaysAc_c *ac = ftr;
    ac->setSwitch(sw, param);
    if (send) {
        if (ac != NULL) {
            param = ac->getSwitchParam();
        }
        return sendSwitch(x, z, layer, sw, param, arg, 4);
    }
    return TRUE;
}

BOOL requestSwitch(daFTR_c *ftr, int sw, u8 param, BOOL arg, BOOL send) {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        alwaysAc_c *ac = ftr;
        int x;
        int z;
        if (ac->getUnit(&x, &z, NULL, mAng(0))) {
            return requestSwitch(x, z, ac->getLayer(), sw, param, arg, send);
        }
    }
    return FALSE;
}

BOOL requestSwitch(int x, int z, u8 layer, int sw, u8 param, BOOL arg, BOOL send) {
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        return FALSE;
    }
    daFTR_c *ftr = getUnitMap()->get(x, z, layer);
    if (ftr == NULL) {
        return FALSE;
    }
    if (dNet::isHost() || !dNet::isSessionActive()) {
        return setSwitch(x, z, layer, sw, param, arg, send);
    }
    if (send) {
        if (sendSwitch(x, z, layer, sw, param, arg, 0)) {
            ftr->onSwitchSent();
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

void recvSwitch(const void *data) {
    const switchPacket_c *packet = (const switchPacket_c *)data;
    switch (packet->mRelay) {
    case 1:
        applySwitch(packet->mScene, packet->mX, packet->mZ, packet->mLayer, packet->mSw, packet->mOn,
                    packet->mMusic, packet->mArg, packet->mParam);
        switchPacket_c relay = *packet;
        relay.mRelay = 0;
        dNet::beginPacket();
        dNet::addPacket(&relay, sizeof(relay));
        dNet::sendPacket(NET_PACKET_FTR_ACTION, 4);
        break;
    default:
        applySwitch(packet->mScene, packet->mX, packet->mZ, packet->mLayer, packet->mSw, packet->mOn,
                    packet->mMusic, packet->mArg, packet->mParam);
        break;
    }
}

struct roomFlagPacket_c {
    u16 mScene : 7;
    u16 mOn : 1;
    u16 mRelay : 1;
};

void sendRoomFlag(u8 scene, BOOL on, int member) {
    if (dNet::isSessionActive()) {
        roomFlagPacket_c packet;
        packet.mScene = scene;
        packet.mOn = on;
        packet.mRelay = member != 4;
        dNet::beginPacket();
        dNet::addPacket(&packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_FTR_ACTION2, member);
    }
}

BOOL setRoomFlag(u8 scene, u8 on) {
    int room;
    int home = dHomeList_c::getHomeFromScene(&room, scene);
    if (home != -1) {
        dHome_c *house = dSaveData_c::getTown()->mHomes.getHome(home);
        if (dHome_c::isValidRoom(room)) {
            dHomeRoom_c *homeRoom = house->getRoom(room);
            if (homeRoom != NULL) {
                homeRoom->_456 = on;
                return TRUE;
            }
        }
    } else if (dScene::getCurrentScene() == dScene::SCENE_RM_HAPPY_MDL) {
        dSaveData_c::getTown()->mModelRoom._456 = on;
        return TRUE;
    }
    return FALSE;
}

void requestRoomFlag(u8 scene, u8 on) {
    if (setRoomFlag(scene, on)) {
        sendRoomFlag(scene, on, 4);
    }
}

void recvRoomFlag(const void *data) {
    const roomFlagPacket_c *packet = (const roomFlagPacket_c *)data;
    u8 scene = packet->mScene;
    bool on = packet->mOn;
    switch (packet->mRelay) {
    case 1:
        setRoomFlag(scene, on);
        roomFlagPacket_c relay = *packet;
        relay.mRelay = 0;
        dNet::beginPacket();
        dNet::addPacket(&relay, sizeof(relay));
        dNet::sendPacket(NET_PACKET_FTR_ACTION2, 4);
        break;
    default:
        setRoomFlag(scene, on);
        break;
    }
}

struct unitItemPacket_c {
    u32 mItem : 16;
    u32 mX : 4;
    u32 mZ : 4;
};

void setUnitItem(u8 scene, int x, int z, dItem::Item *item, BOOL send) {
    u8 current = dScene::getCurrentScene();
    BOOL room = dScene::isSceneAttr(current, dScene::SCENE_ATTR_ROOM);
    dFdBase_c *fd = NULL;
    if (room && current == scene) {
        fd = dField::getInfo(0);
    } else {
        int room;
        int home = dHomeList_c::getHomeFromScene(&room, current);
        if (home != -1 && room < 3) {
            fd = dField::getInfo(home * 3 + room + 2);
        }
    }
    if (fd != NULL && fd->setItem(item, x, z, 1) && send && dNet::isSessionActive()) {
        unitItemPacket_c packet;
        packet.mX = x;
        packet.mZ = z;
        packet.mItem = item->mId;
        dNet::beginPacket();
        dNet::addPacket(&packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_FTR_SET_UNIT, 4);
    }
}

void recvUnitItem(u8 scene, const void *data) {
    const unitItemPacket_c *packet = (const unitItemPacket_c *)data;
    dItem::Item item((u16)packet->mItem);
    setUnitItem(scene, packet->mX, packet->mZ, &item, FALSE);
}

struct putPacket_c {
    u32 mItem : 16;
    u32 mX : 4;
    u32 mZ : 4;
    u32 mGyroidValue : 4;
    u32 mLayer : 1;
    u32 mOn : 1;
    u32 mGyroid : 1;
};

void putFtr(u8 scene, int x, int z, u8 layer, BOOL on, BOOL gyroid, u8 gyroidValue, const dItem::Item *item,
            bool send) {
    dScene::isSceneAttr(scene, dScene::SCENE_ATTR_ROOM);
    dItem::Item ftrItem = *item;
    dFdBase_c *fd;
    u8 current = dScene::getCurrentScene();
    if (current == scene) {
        fd = dField::getInfo(0);
    } else {
        int id = dField::getSceneFieldId(scene);
        fd = dField::getInfo(id);
    }
    if (fd != NULL) {
        fd->getKind();
        shape_c shape;
        shape.set(*item);
        for (u32 i = 0; i < shape.getNum(); i++) {
            int ux = x + shape.getOfs(i)[0];
            int uz = z + shape.getOfs(i)[1];
            dItem::Item unitItem = i == 0 ? ftrItem : dItem::Item((u16)0xF030);
            if (!fd->setItem(&unitItem, ux, uz, layer)) {
                return;
            }
            setRoomMap(ux, uz, layer, on, scene);
            if (gyroid && on && i == 0) {
                addRoomGyroid(ux, uz, gyroidValue, scene);
            }
        }
        if (dNet::isSessionActive() && send) {
            putPacket_c packet;
            packet.mX = x;
            packet.mZ = z;
            packet.mGyroidValue = gyroidValue;
            packet.mLayer = layer;
            packet.mOn = on;
            packet.mGyroid = gyroid;
            packet.mItem = ftrItem.mId;
            dNet::beginPacket();
            dNet::addPacket(&packet, sizeof(packet));
            dNet::sendPacket(NET_PACKET_FTR_PUT, 4);
        }
    }
}

void recvPutFtr(u8 scene, const void *data) {
    const putPacket_c *packet = (const putPacket_c *)data;
    dItem::Item item((u16)packet->mItem);
    putFtr(scene, packet->mX, packet->mZ, packet->mLayer, packet->mOn, packet->mGyroid, packet->mGyroidValue, &item,
           FALSE);
}

struct removePacket_c {
    u32 mItem : 16;
    u32 mX : 4;
    u32 mZ : 4;
    u32 mLayer : 1;
    u32 mUpper : 1;
    u32 mOn : 1;
};

static inline bool clearItemFailed(dFdBase_c *fd, int x, int z, int layer) {
    dItem::Item none((u16)dItem::ITEM_ID_NONE);
    return !fd->setItem(&none, x, z, layer);
}

void removeFtr(u8 scene, int x, int z, int layer, BOOL on, const dItem::Item *item, BOOL upper, BOOL send) {
    dItem::Item ftrItem = *item;
    dFdBase_c *fd;
    u8 current = dScene::getCurrentScene();
    if (current == scene) {
        fd = dField::getInfo(0);
    } else {
        int fieldId = dField::getSceneFieldId(scene);
        fd = dField::getInfo(fieldId);
    }
    if (fd == NULL) {
        return;
    }
    shape_c shape;
    shape.set(*item);
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    BOOL gyroid = bitm != NULL ? dItem::clampField(bitm->m_ftrFunc, dItem::FTR_FUNC_COUNT, dItem::FTR_FUNC_PLAIN) == dItem::FTR_FUNC_GYROID : FALSE;
    for (u32 i = 0; i < shape.getNum(); i++) {
        int ux = x + shape.getOfs(i)[0];
        int uz = z + shape.getOfs(i)[1];
        if (clearItemFailed(fd, ux, uz, layer)) {
            return;
        }
        if (upper && layer == 0) {
            if (clearItemFailed(fd, ux, uz, 1)) {
                return;
            }
        }
        setRoomMap(ux, uz, layer, on, scene);
        if (upper && layer == 0) {
            setRoomMap(ux, uz, 1, on, scene);
        }
        if (gyroid && on) {
            removeRoomGyroid(ux, uz, scene);
        }
    }
    if (dNet::isSessionActive() && send) {
        removePacket_c packet;
        packet.mLayer = layer;
        packet.mX = x;
        packet.mZ = z;
        packet.mUpper = upper;
        packet.mOn = on;
        packet.mItem = ftrItem.mId;
        dNet::beginPacket();
        dNet::addPacket(&packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_FTR_REMOVE, 4);
    }
}

void recvRemoveFtr(u8 scene, const void *data) {
    const removePacket_c *packet = (const removePacket_c *)data;
    dItem::Item item((u16)packet->mItem);
    removeFtr(scene, packet->mX, packet->mZ, packet->mLayer, packet->mOn, &item, packet->mUpper, FALSE);
}

f32 getGroundY(const mVec3_c *pos) {
    int x;
    int z;
    dBGCF::posToUnit(&x, &z, pos);
    return getGroundY(x, z);
}

f32 getGroundY(int x, int z) {
    daFTR_c *ftr = getUnitMap()->get(x, z, 0);
    alwaysAc_c *ac = ftr;
    if (ftr != NULL) {
        if (ac->canWalkOn()) {
            return ftr->getPosP()->y;
        }
        return ac->getTopY();
    }
    mVec3_c pos;
    dBGCF::unitToPos(&pos, x, z);
    dBGCF::groundChk_c chk(&pos, 0, 0, 0);
    return chk.getHeight(FALSE);
}

f32 getRoomGroundY(int x, int z) {
    dFdBase_c *fd = dField::getInfo(0);
    if (fd != NULL) {
        for (int uz = z - 1; uz <= z + 1; uz++) {
            for (int ux = x - 1; ux <= x + 1; ux++) {
                dItem::Item *item = fd->getItem(ux, uz, 0);
                if (item != NULL && item->hasFtrFunc()) {
                    shape_c shape;
                    shape.set(*item);
                    for (u32 i = 0; i < shape.getNum(); i++) {
                        const int *ofs = shape.getOfs(i);
                        int ox = ux + ofs[0];
                        int oz = uz + ofs[1];
                        if (x == ox && z == oz) {
                            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                            if (bitm != NULL) {
                                mVec3_c pos;
                                dBGCF::unitToPos(&pos, x, z);
                                dBGCF::groundChk_c chk(&pos, 0, 0, 0);
                                f32 y = chk.getHeight(FALSE);
                                if (!bitm->m_ftrPassThru) {
                                    mVec3_c topPos;
                                    dBGCF::unitToPos(&topPos, x, z);
                                    dBGCF::groundChk_c topChk(&topPos, 0, 0, 0);
                                    return 32.0f + y;
                                }
                                return y;
                            }
                        }
                    }
                }
            }
        }
        mVec3_c pos;
        dBGCF::unitToPos(&pos, x, z);
        dBGCF::groundChk_c chk(&pos, 0, 0, 0);
        return chk.getHeight(FALSE);
    }
    return 0.0f;
}

BOOL isEmptyOnTop(int x, int z) {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        dFdBase_c *fd = dField::getInfo(0);
        daFTR_c *ftr = getUnitMap()->get(x, z, 0);
        if (fd != NULL && ftr != NULL) {
            dItem::Item *item = fd->getItem(x, z, 1);
            if (ftr->getTopKind() == 1 && item != NULL) {
                if (item->isSame(dItem::Item((u16)dItem::ITEM_ID_NONE))) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

inline hold_c::hold_c() {}

void hold_c::clear() {
    mHandle = -1;
    mPlayerLastPos.set(0.0f, 0.0f, 0.0f);
    mPlayerPos.set(0.0f, 0.0f, 0.0f);
    mDist = 0.0f;
    mCenterPos.set(0.0f, 0.0f, 0.0f);
    mPushPos.set(0.0f, 0.0f, 0.0f);
    mPushAngle = 0;
    mSide = 0;
}

void hold_c::set(int handle, const mVec3_c &playerLastPos, const mVec3_c &playerPos, f32 dist,
                 const mVec3_c &centerPos, const mVec3_c &pushPos, s16 pushAngle, u8 side) {
    if (handle == -1) {
        return;
    }
    mHandle = handle;
    mPlayerLastPos = playerLastPos;
    mPlayerPos = playerPos;
    mDist = dist;
    mCenterPos = centerPos;
    mPushPos = pushPos;
    mPushAngle = pushAngle;
    mSide = side;
}

static holdList_c l_holdList;

holdList_c *getHoldList() {
    return &l_holdList;
}

hold_c *holdList_c::get(int layer) {
    if (isHoldScene()) {
        hold_c *hold = &mHold[layer & 1];
        if (hold->mHandle != -1) {
            return hold;
        }
    }
    return NULL;
}

hold_c *holdList_c::find(daFTR_c *ftr) {
    int handle = getList()->find(ftr);
    if (handle != -1) {
        for (int i = 0; i < 2; i++) {
            if (handle == mHold[i].mHandle) {
                return &mHold[i];
            }
        }
    }
    return NULL;
}

daFTR_c *holdList_c::getFtr(int layer) {
    hold_c *hold = get(layer);
    if (hold != NULL) {
        return getList()->get(hold->mHandle);
    }
    return NULL;
}

void holdList_c::startHold() {
    alwaysAc_c *upper = getFtr(1);
    hold_c *upperHold = get(1);
    if (upper != NULL && upperHold != NULL) {
        BOOL ok = TRUE;
        if (upper->getBITM()->_188_2 && upperHold->mSide) {
            ok = FALSE;
        }
        if (ok && upper->startHold()) {
            return;
        }
    }
    alwaysAc_c *lower = getFtr(0);
    hold_c *lowerHold = get(0);
    if (lower != NULL && lowerHold != NULL) {
        BOOL ok = TRUE;
        if (lower->getBITM()->_188_2 && lowerHold->mSide) {
            ok = FALSE;
        }
        if (ok && lower->startHold()) {
            return;
        }
    }
}

BOOL holdList_c::isIdle() {
    alwaysAc_c *ac = getFtr(0);
    if (ac != NULL) {
        return ac->isIdle();
    }
    return FALSE;
}

BOOL holdList_c::canRotate(s16 angle) {
    alwaysAc_c *ac = getFtr(0);
    if (ac != NULL) {
        return ac->canRotate(mAng(angle));
    }
    return FALSE;
}

BOOL holdList_c::startRotate(s16 angle) {
    alwaysAc_c *ac = getFtr(0);
    if (ac != NULL) {
        return ac->startRotate(mAng(angle));
    }
    return FALSE;
}

BOOL holdList_c::canRotateLeft() {
    return canRotate(-0x4000);
}

BOOL holdList_c::startRotateLeft() {
    return startRotate(-0x4000);
}

BOOL holdList_c::canRotateRight() {
    return canRotate(0x4000);
}

BOOL holdList_c::startRotateRight() {
    return startRotate(0x4000);
}

BOOL holdList_c::canMove(s16 angle) {
    hold_c *hold = get(0);
    alwaysAc_c *ac = getFtr(0);
    if (hold != NULL && ac != NULL) {
        return ac->canMove(getDir(angle + hold->mPushAngle));
    }
    return FALSE;
}

mVec3_c *holdList_c::getDir(s16 angle) {
    static mVec3_c dir;
    dir.set(0.0f, 0.0f, 32.0f);
    dir.rotY(mAng(angle));
    return &dir;
}

BOOL holdList_c::canPush() {
    return canMove(0);
}

BOOL holdList_c::startPush() {
    hold_c *hold = get(0);
    alwaysAc_c *ac = getFtr(0);
    if (hold != NULL && ac != NULL) {
        return ac->startPush(mAng(hold->mPushAngle));
    }
    return FALSE;
}

BOOL holdList_c::canPull() {
    return canMove(-0x8000);
}

BOOL holdList_c::startPull() {
    hold_c *hold = get(0);
    alwaysAc_c *ac = getFtr(0);
    if (hold != NULL && ac != NULL) {
        return ac->startPull(mAng(hold->mPushAngle));
    }
    return FALSE;
}

void holdList_c::clear() {
    for (u32 i = 0; i < 2; i++) {
        mHold[i].clear();
    }
}

BOOL holdList_c::set(int handle, f32 dist, const mVec3_c &playerLastPos, const mVec3_c &playerPos,
                     const mVec3_c &centerPos, const mVec3_c &pushPos, s16 pushAngle, u8 side) {
    if (handle != -1) {
        alwaysAc_c *ac = getList()->get(handle);
        if (ac != NULL) {
            mHold[ac->getLayer() & 1].set(handle, playerLastPos, playerPos, dist, centerPos, pushPos, pushAngle, side);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

BOOL holdList_c::isHoldScene() {
    u8 scene = dScene::getCurrentScene();
    if (isHoldDisabled()) {
        return FALSE;
    }
    if (scene == dScene::SCENE_RM_THEATER_SHOW || scene == dScene::SCENE_RM_BARBER ||
        scene == dScene::SCENE_RM_MM_CAFE) {
        return TRUE;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_PLAYER_HOUSE)) {
        int room;
        if (dHomeList_c::getHomeFromScene(&room, scene) != -1 && room == 3) {
            return FALSE;
        }
        return TRUE;
    }
    return dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE);
}

BOOL canHoldInScene() {
    if (isHoldDisabledOnly()) {
        return FALSE;
    }
    return !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_SHOP_ROOM);
}

void resetHold() {
    l_holdState = HOLD_STATE_PENDING;
    l_lockList.clear();
}

void requestHold(const mVec3_c *pos) {
    l_holdState = HOLD_STATE_PENDING;
    if (!dNet::isSessionActive() || dNet::isHost()) {
        if (l_lockList.lock(0, dScene::getCurrentScene(), *pos)) {
            l_holdState = HOLD_STATE_ACCEPTED;
        } else {
            l_holdState = HOLD_STATE_REJECTED;
        }
    } else {
        u8 buf[3];
        buf[0] = (int)pos->x >> 1;
        buf[1] = (int)pos->z >> 1;
        buf[2] = dScene::getCurrentScene();
        dNet::beginPacket();
        dNet::addPacket(buf, sizeof(buf));
        dNet::sendPacket(NET_PACKET_FTR_HOLD_REQUEST, 0);
    }
}

int getHoldState() {
    return l_holdState;
}

void requestPush(const mVec3_c *pos) {
    requestHold(pos);
}

int getPushState() {
    return getHoldState();
}

void releaseHold() {
    l_holdState = HOLD_STATE_PENDING;
    if (!dNet::isSessionActive() || dNet::isHost()) {
        l_lockList.reset(0);
    } else {
        dNet::beginPacket();
        dNet::sendPacket(NET_PACKET_FTR_HOLD_RELEASE, 0);
    }
}

BOOL canNpcWalk(int x, int z) {
    if (dBGCF::getOpenNext(x, z) == -1) {
        daFTR_c *ftr = getUnitMap()->get(x, z, 0);
        if (ftr != NULL && !ftr->canWalkOn()) {
            return FALSE;
        }
        if (dBGCF::canNpcPutItem(x, z)) {
            return TRUE;
        }
    }
    return FALSE;
}

void requestPull(const mVec3_c *pos) {
    requestHold(pos);
}

int getPullState() {
    return getHoldState();
}

void requestRotate(const mVec3_c *pos) {
    requestHold(pos);
}

int getRotateState() {
    return getHoldState();
}

void releaseRotate() {
    releaseHold();
}

void requestMove(const mVec3_c *pos) {
    requestHold(pos);
}

int getMoveState() {
    return getHoldState();
}

// Nothing in the DOL calls it, yet the original link kept it.
#pragma force_active on
BOOL isHoldDisabledScene() {
    return FALSE;
}
#pragma force_active reset

static u8 l_homeFlag[4];

void clearHomeFlags() {
    l_homeFlag[0] = 0;
    l_homeFlag[1] = 0;
    l_homeFlag[2] = 0;
    l_homeFlag[3] = 0;
}

// Nothing in the DOL calls it, yet the original link kept it.
#pragma force_active on
void clearHomeFlag() {
    int home = dHomeList_c::getHomeFromScene(NULL, dScene::getCurrentScene());
    if (home != -1) {
        l_homeFlag[home & 3] = 0;
    }
}
#pragma force_active reset

void setSwitchAll(u32 sw, filter_t filter, int arg) {
    for (u32 i = 0; i < dScene::getSceneParamA(dScene::getCurrentScene()); i++) {
        daFTR_c *ftr = getList()->get(i);
        if (ftr != NULL && (filter == NULL || filter(ftr)) && sw != ftr->getSwitch()) {
            ftr->changeSwitch(sw, arg);
        }
    }
}

u32 countSwitch(filter_t filter, u32 sw) {
    u32 count = 0;
    for (u32 i = 0; i < dScene::getSceneParamA(dScene::getCurrentScene()); i++) {
        daFTR_c *ftr = getList()->get(i);
        if (ftr != NULL && (filter == NULL || filter(ftr)) && sw == ftr->getSwitch()) {
            count++;
        }
    }
    return count;
}

daFTR_c *switchOffRandom(filter_t filter, int arg) {
    u32 num = countSwitch(filter, 1);
    if (num != 0) {
        u32 pick = cM::rndF(num);
        u32 count = 0;
        for (u32 i = 0; i < dScene::getSceneParamA(dScene::getCurrentScene()); i++) {
            daFTR_c *ftr = getList()->get(i);
            if (ftr != NULL && (filter == NULL || filter(ftr)) && ftr->getSwitch()) {
                if (count == pick) {
                    ftr->changeSwitch(0, arg);
                    return ftr;
                }
                count++;
            }
        }
    }
    return NULL;
}

BOOL switchOffAll() {
    for (u32 i = 0; i < dScene::getSceneParamA(dScene::getCurrentScene()); i++) {
        daFTR_c *ftr = getList()->get(i);
        if (ftr != NULL && ftr->isMusic() && ftr->getSwitch()) {
            return setSwitch(ftr, 0, 0xFF, FALSE, TRUE);
        }
    }
    return TRUE;
}

BOOL playMusic(const dItem::Item *item) {
    for (u32 i = 0; i < dScene::getSceneParamA(dScene::getCurrentScene()); i++) {
        daFTR_c *ftr = getList()->get(i);
        if (ftr != NULL && ftr->isMusic() && !ftr->getSwitch() && ftr->isSwitchable() &&
            item->getKind() == dItem::KIND_MUSIC) {
            return setSwitch(ftr, 1, (u8)dItem::seeker_c::get()->findLike(*item), FALSE, TRUE);
        }
    }
    return TRUE;
}

int sendEvent(int handle) {
    alwaysAc_c *ac = getList()->get(handle);
    if (ac != NULL) {
        return ac->sendEvent(5);
    }
    return 0;
}

int sendSellEvent(int handle) {
    return sendEvent(handle);
}

BOOL alwaysBank_c::load(void *heap) {
    return bank_c::load("/Ftr/always.arc", heap, 0);
}

void *alwaysBank_c::getFile(u32 idx) {
    static const char *l_name[] = {"dsnCloth.brres", "dsnUmb.brres", "dsnCap.brres", "capStand.brres", "dsnCat.brres"};
    if (idx < 5) {
        u32 size;
        return arcBank_c::getFile(l_name[idx], &size);
    }
    return NULL;
}

void alwaysBank_c::onLoaded() {
    arcBank_c::onLoaded();
    for (u32 i = 0; i < 5; i++) {
        nw4r::g3d::ResFile res(getFile(i));
        res.Init();
        res.Bind(res);
    }
}

alwaysBank_c *alwaysBank_c::get() {
    static alwaysBank_c bank;
    return &bank;
}

static u8 l_holdDisabled;
static u8 l_holdDisabledOnly;

void disableHold() {
    l_holdDisabled = TRUE;
    l_holdDisabledOnly = TRUE;
}

void disableHoldOnly() {
    l_holdDisabled = TRUE;
}

void enableHold() {
    l_holdDisabled = FALSE;
    l_holdDisabledOnly = FALSE;
}

BOOL isHoldDisabled() {
    return l_holdDisabled;
}

BOOL isHoldDisabledOnly() {
    return l_holdDisabledOnly;
}

BOOL canWalk(int x, int z) {
    if (!dBGCF::canPutItem(x, z)) {
        return FALSE;
    }
    daFTR_c *ftr = getUnitMap()->get(x, z, 0);
    if (ftr != NULL && !ftr->canWalkOn()) {
        return FALSE;
    }
    return TRUE;
}

BOOL isExit(int x, int z) {
    return z >= 13 && (x == 7 || x == 8);
}

BOOL canReachExit() {
    static u8 l_reach[16][16];
    int room;
    if (dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene()) == -1) {
        return TRUE;
    }
    if (room >= 3) {
        return TRUE;
    }
    memset(l_reach, 0, sizeof(l_reach));
    dPlayerActor_c *player = dPlayerMgr::getPlayerActor(4);
    if (player != NULL) {
        int x;
        int z;
        dBGCF::posToUnit(&x, &z, player->getPosP());
        l_reach[z & 0xF][x & 0xF] = TRUE;
        if (isExit(x, z)) {
            return TRUE;
        }
        int added;
        do {
            added = 0;
            for (int uz = 0; uz < 16; uz++) {
                for (int ux = 0; ux < 16; ux++) {
                    if (!l_reach[uz & 0xF][ux & 0xF]) {
                        continue;
                    }
                    for (const int(*dir)[2] = l_dir; dir < l_dir + 4; dir++) {
                        int nx = ux + (*dir)[0];
                        int nz = uz + (*dir)[1];
                        if (!l_reach[nz & 0xF][nx & 0xF] && canWalk(nx, nz)) {
                            l_reach[nz & 0xF][nx & 0xF] = TRUE;
                            if (isExit(nx, nz)) {
                                return TRUE;
                            }
                            added++;
                        }
                    }
                }
            }
        } while (added != 0);
        return FALSE;
    }
    return FALSE;
}

} // namespace dFTR
