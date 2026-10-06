// Player houses: rooms, the house itself and the four houses in the save data.
// .text 8013CB30..8013E618. Notes: notes/d_home.txt.
#include <game/game/d_home.hpp>
#include <game/game/d_catalog.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <cstring>

// Dependencies whose owners are not recovered yet.
extern "C" {
dItem::Item fn_80077730(const dHomeRoom_c *room); // room->mWallpaper
dItem::Item fn_80077748(const dHomeRoom_c *room); // room->mCarpet
void *fn_801683D8();
void *fn_80167BAC(void *);
void fn_801911AC(void *);
void fn_801911E8(int home);
}

// 80476258: house scene attribute per house (getSceneAttrTableIndex)
static const u32 sHomeScenes[] = {
    SCENE_ATTR_HOUSE0 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE1 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE2 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE3 | SCENE_ATTR_PLAYER_HOUSE,
    0,
    0,
};

// 80476270: default mWallpaper per style and room
static const int sDefaultWallpapers[4][HOME_ROOM_NUM] = {
    {dItem::ITEM_IDX_WOOD_PANELING, dItem::ITEM_IDX_COMMON_WALL, dItem::ITEM_IDX_CONCRETE_WALL},
    {dItem::ITEM_IDX_OLD_BRICK_WALL, dItem::ITEM_IDX_COMMON_WALL, dItem::ITEM_IDX_CONCRETE_WALL},
    {dItem::ITEM_IDX_STONE_WALL, dItem::ITEM_IDX_COMMON_WALL, dItem::ITEM_IDX_CONCRETE_WALL},
    {dItem::ITEM_IDX_ORNATE_WALL, dItem::ITEM_IDX_COMMON_WALL, dItem::ITEM_IDX_CONCRETE_WALL},
};

// 804762A0: default mCarpet per style and room
static const int sDefaultCarpets[4][HOME_ROOM_NUM] = {
    {dItem::ITEM_IDX_OPULENT_RUG, dItem::ITEM_IDX_COMMON_FLOOR, dItem::ITEM_IDX_BASEMENT_FLOOR},
    {dItem::ITEM_IDX_CONCRETE_FLOOR, dItem::ITEM_IDX_COMMON_FLOOR, dItem::ITEM_IDX_BASEMENT_FLOOR},
    {dItem::ITEM_IDX_SHANTY_MAT, dItem::ITEM_IDX_COMMON_FLOOR, dItem::ITEM_IDX_BASEMENT_FLOOR},
    {dItem::ITEM_IDX_PLANK_FLOORING, dItem::ITEM_IDX_COMMON_FLOOR, dItem::ITEM_IDX_BASEMENT_FLOOR},
};

// 804762D0: default item placed in the main room per style
static const int sDefaultItems0[4] = {dItem::ITEM_IDX_CANDLE, dItem::ITEM_IDX_TABLE_LAMP, dItem::ITEM_IDX_MINI_LAMP, dItem::ITEM_IDX_DESK_LIGHT};

// 804762E0: dHome_c::_15B8 per style
static const int sHomeItems[4] = {dItem::ITEM_IDX_BASIC_RED_BED, dItem::ITEM_IDX_BASIC_BLUE_BED, dItem::ITEM_IDX_BASIC_YELLOW_BED, dItem::ITEM_IDX_BASIC_GREEN_BED};

// 804762F0: rooms available per house size
static const u8 sHasRoom[HOME_SIZE_NUM][HOME_ROOM_NUM] = {
    {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1},
};

// 80476300: per house size
static const int sExteriorIds[HOME_SIZE_NUM] = {0xBF, 0xBF, 0xBF, 0xC0, 0xC0};

// 80476314: main room per house size
static const int sMainRoomIds[HOME_SIZE_NUM] = {0xB8, 0xB9, 0xBA, 0xBB, 0xBC};

// 8013CB30
void dHomeLayer_c::clear() {
    dItem::Item *p = &mItems[0][0];
    for (int i = 0; i < 16 * 16; i++) {
        p[i].mId = dItem::ITEM_ID_NONE;
    }
}

// 8013CBCC
void dHomeRoom_c::clear() {
    for (int i = 0; i < HOME_LAYER_NUM; i++) {
        mLayers[i].clear();
    }
    mMap.clear();
    mWallpaper = dItem::Item(dItem::ITEM_IDX_EXOTIC_WALL);
    _454 = 1;
    mCarpet = dItem::Item(dItem::ITEM_IDX_EXOTIC_RUG);
    _455 = 2;
    mSong.mId = dItem::ITEM_ID_NONE;
    _456 = 0;
}

// 8013CC78
void dHomeRoom_c::recycleItems() {
    for (int i = 0; i < HOME_LAYER_NUM; i++) {
        dItem::Item *items = (dItem::Item *)getLayer(i);
        if (items != NULL) {
            for (dItem::Item *p = items; p < items + 0x100; p++) {
                if (!p->isOrgDesign()) {
                    switch (p->getKind()) {
                    case dItem::KIND_GOLD_FISHINGROD:
                    case dItem::KIND_GOLD_SCOOP:
                    case dItem::KIND_GOLD_AXE:
                    case dItem::KIND_GOLD_WATERING:
                    case dItem::KIND_GOLD_NET:
                    case dItem::KIND_GOLD_PACHINKO:
                        dItem::Item item = *p;
                        dSaveData_c::addToRecycleBin(item);
                        p->mId = dItem::ITEM_ID_NONE;
                        break;
                    }
                } else {
                    p->mId = dItem::ITEM_ID_NONE;
                }
            }
        }
    }
    for (int i = 0; i < HOME_LAYER_NUM; i++) {
        dItem::Item *items = (dItem::Item *)getLayer(i);
        if (items != NULL) {
            for (dItem::Item *p = items; p < items + 0x100; p++) {
                if (!p->isOrgDesign()) {
                    switch (p->getKind()) {
                    case dItem::KIND_SILVER_FISHINGROD:
                    case dItem::KIND_SILVER_SCOOP:
                    case dItem::KIND_SILVER_AXE:
                    case dItem::KIND_SILVER_WATERING:
                    case dItem::KIND_SILVER_NET:
                    case dItem::KIND_SILVER_PACHINKO:
                        dItem::Item item = *p;
                        dSaveData_c::addToRecycleBin(item);
                        p->mId = dItem::ITEM_ID_NONE;
                        break;
                    }
                } else {
                    p->mId = dItem::ITEM_ID_NONE;
                }
            }
        }
    }
    for (int i = 0; i < HOME_LAYER_NUM; i++) {
        dItem::Item *items = (dItem::Item *)getLayer(i);
        if (items != NULL) {
            for (dItem::Item *p = items; p < items + 0x100; p++) {
                if (!p->isOrgDesign()) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*p);
                    if (bitm != NULL && bitm->m_noPurchase) {
                        dItem::Item item = *p;
                        dSaveData_c::addToRecycleBin(item);
                        p->mId = dItem::ITEM_ID_NONE;
                    }
                } else {
                    p->mId = dItem::ITEM_ID_NONE;
                }
            }
        }
    }
    for (int i = 0; i < HOME_LAYER_NUM; i++) {
        dItem::Item *items = (dItem::Item *)getLayer(i);
        if (items != NULL) {
            for (dItem::Item *p = items; p < items + 0x100; p++) {
                if (!p->isOrgDesign()) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*p);
                    if (bitm != NULL && !bitm->m_noPurchase) {
                        dItem::Item item = *p;
                        dSaveData_c::addToRecycleBin(item);
                        p->mId = dItem::ITEM_ID_NONE;
                    }
                } else {
                    p->mId = dItem::ITEM_ID_NONE;
                }
            }
        }
    }
    for (int i = 0; i < HOME_LAYER_NUM; i++) {
        mLayers[i].clear();
    }
    mMap.clear();
    mSong.mId = dItem::ITEM_ID_NONE;
    _456 = 0;
}

// 8013CF7C
void dHomeRoom_c::getDefaultItems(u16 *out, u32 style, u32 room) {
    if (style >= 4 || room >= HOME_ROOM_NUM) {
        out[0] = dItem::ITEM_ID_NONE;
        out[1] = dItem::ITEM_ID_NONE;
        out[2] = dItem::ITEM_ID_NONE;
        return;
    }
    if (room == 0) {
        out[0] = dItem::Item(sDefaultItems0[style & 3]).mId;
    } else {
        out[0] = dItem::ITEM_ID_NONE;
    }
    out[1] = dItem::Item(sDefaultWallpapers[style & 3][room]).mId;
    out[2] = dItem::Item(sDefaultCarpets[style & 3][room]).mId;
}

// 8013D06C
void dHomeRoom_c::init(int style, int room) {
    clear();
    if ((style != -1 || room != -1) && (u32)style < 4 && (u32)room < HOME_ROOM_NUM) {
    u16 items[3];
    items[0] = dItem::ITEM_ID_NONE;
    items[1] = dItem::ITEM_ID_NONE;
    items[2] = dItem::ITEM_ID_NONE;
    getDefaultItems(items, style, room);
    if ((u32)room < HOME_ROOM_NUM) {
        mWallpaper.mId = items[1];
        mCarpet.mId = items[2];
        if (room == 0) {
            dHomeLayer_c *floor = getLayer(0);
            dHomeLayer_c *top = getLayer(1);
            floor->set(10, 9, dItem::Item(dItem::ITEM_IDX_TAPE_DECK));
            floor->set(10, 6, dItem::Item(dItem::ITEM_IDX_CARDBOARD_BOX));
            top->set(10, 6, dItem::Item(items[0]));
            mMap.set(9, 10, 0, FALSE);
            mMap.set(6, 10, 1, FALSE);
            _456 = 0;
        }
    }
    mFlags = 0;
    }
}

// 8013D1A4
BOOL dHomeRoom_c::isValidLayer(int layer) {
    BOOL ret = FALSE;
    if (layer >= 0 && layer < HOME_LAYER_NUM) {
        ret = TRUE;
    }
    return ret;
}

// 8013D1C0
dHomeLayer_c *dHomeRoom_c::getLayer(int layer) {
    if (isValidLayer(layer)) {
        return &mLayers[layer];
    }
    return NULL;
}

// 8013D210
const dHomeLayer_c *dHomeRoom_c::getLayer(int layer) const {
    if (const_cast<dHomeRoom_c *>(this)->isValidLayer(layer)) {
        return &mLayers[layer];
    }
    return NULL;
}

// 8013D260
BOOL dHomeRoom_c::hasInvalidItem() const {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(fn_80077730(this));
    if (bitm == NULL) {
        return TRUE;
    }
    bitm = dItem::infoBank_c::get()->getBITM(fn_80077748(this));
    if (bitm == NULL) {
        return TRUE;
    }
    for (u32 i = 0; i < HOME_LAYER_NUM; i++) {
        const dHomeLayer_c *layer = getLayer(i);
        if (layer != NULL) {
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    dItem::Item item = layer->get(z, x);

                    // TODO: should be an inline or something
                    int type = (u16)ITEM_NAME_TYPE(item.mId);
                    BOOL check = FALSE;
                    if (type >= 9 && type <= 12) {
                        check = TRUE;
                    }
                    if (check) {
                        bitm = dItem::infoBank_c::get()->getBITM(item);
                        if (bitm == NULL) {
                            return TRUE;
                        }
                    }
                }
            }
        }
    }
    return FALSE;
}

// 8013D384
dHomeLayer_c *dHomeRoom_c::getLayerItems(int layer) {
    if (isValidLayer(layer)) {
        return &mLayers[layer];
    }
    return NULL;
}

// 8013D3D4
u8 dHomeRoom_c::countFlags() {
    u8 flags = mFlags;
    u8 n = ((flags >> 2) & 1) + ((flags >> 1) & 1);
    return (u8)(n + (flags & 1));
}

// 8013D3F8
void dHomeRoom_c::clearFlag(int flag) {
    u8 masks[] = {1, 2, 4};
    mFlags &= ~masks[flag];
}

// 8013D430
BOOL dHomeRoom_c::isFlag(int flag) const {
    u8 masks[] = {1, 2, 4};
    return (mFlags & masks[flag]) != 0;
}

// 8013D470
void dHomeRoom_c::setFlags(u8 flags) {
    mFlags = flags;
}

// 8013D478
void dHome_c::clear() {
    mOwner.clear();
    for (int i = 0; i < HOME_ROOM_NUM; i++) {
        mRooms[i].clear();
    }
    mNextSize = 0;
    mSize = 0;
    _15B7 = 0;
    _15B6 = 0;
    _15B8.mId = dItem::ITEM_ID_NONE;
    _15BB.mRaw = 0;
}

// 8013D4FC
void dHome_c::init(u32 style) {
    // 80750BE8
    static const u8 sStyleSizes[4] = {0, 1, 2, 3};
    clear();
    mDesign.clear();
    mDesign.setFromItem(dItem::ITEM_IDX_RED_LEAF + (style & 3));
    mDesign.loadTextureD(style);
    for (int i = 0; i < HOME_ROOM_NUM; i++) {
        mRooms[i].init(style, i);
    }
    u8 size = sStyleSizes[style & 3];
    _15B7 = size;
    _15B6 = size;
    _15B8 = dItem::Item(sHomeItems[style & 3]);
    _15BA = 30;
}

// 8013D5B8
BOOL dHome_c::hasRoom(u32 room) {
    if (room < HOME_ROOM_NUM && mSize < HOME_SIZE_NUM) {
        return sHasRoom[mSize][room] != 0;
    }
    return FALSE;
}

// 8013D5FC
void dHome_c::recycleRoomItems(int room) {
    dHomeRoom_c *r = getRoom(room);
    if (r != NULL) {
        r->recycleItems();
    }
}

// 8013D628
BOOL dHome_c::setOwner(dPrivateData_c *player) {
    if (!mOwner.isValid()) {
        mOwner = player->mPID;
        player->mCatalog.registerItem(_15B8.mId, FALSE);
        mDesign.setLandFromTown();
        return TRUE;
    }
    return FALSE;
}

// 8013D744
BOOL dHome_c::setOwnerToCurrent() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        return setOwner(player);
    }
    return FALSE;
}

// 8013D78C
BOOL dHome_c::isOwner(const dPersonalID_c *pid) {
    if (!mOwner.isValid()) {
        return FALSE;
    }
    return *pid == mOwner;
}

// 8013D838
BOOL dHome_c::isValidRoom(int room) {
    return (u32)room < HOME_ROOM_NUM;
}

// 8013D850
dHomeRoom_c *dHome_c::getRoom(int room) {
    if (isValidRoom(room)) {
        return &mRooms[room];
    }
    return NULL;
}

// 8013D8A8
int dHome_c::getExteriorId() {
    int size = mSize;
    if (size < HOME_SIZE_NUM) {
        return sExteriorIds[mSize];
    }
    return 0xC0;
}

// 8013D8D0
int dHome_c::getRoomId(int room) {
    switch (room) {
    case 0: {
        int size = mSize;
        if (size < HOME_SIZE_NUM) {
            return sMainRoomIds[mSize];
        }
        return 0xBC;
    }
    case 1:
        return 0xBD;
    default:
        return 0xBE;
    }
}

// 8013D920
void dHome_c::update(int days, BOOL isCurrent) {
    if (days != 0) {
        if (isPending()) {
            _15BB.mRaw = (u8)(_15BB.mRaw | 2) & 0xFE;
        }
        if (mSize != mNextSize && mNextSize < HOME_SIZE_NUM) {
            mSize = mNextSize;
            fn_801911AC(fn_80167BAC(fn_801683D8()));
        }
        _15B6 = _15B7;
    }
    if (mOwner.isValid() == 1U) {
        updateFlags(days, isCurrent);
    }
}

// 8013D9D4
BOOL dHome_c::requestUpgrade() {
    if (mSize < 4 && mSize == mNextSize) {
        mNextSize = mSize + 1;
        return TRUE;
    }
    return FALSE;
}

// 8013DA04
BOOL dHome_c::hasRoom456() {
    for (int i = 0; i < HOME_ROOM_NUM; i++) {
        dHomeRoom_c *room = getRoom(i);
        if (i != 2 && room != NULL && room->_456 != 0) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8013DA78
BOOL dHome_c::hasFlags() {
    u8 a = mRooms[0].countFlags();
    u8 b = mRooms[1].countFlags();
    u8 c = mRooms[2].countFlags();
    return (u8)(c + (a + b)) != 0;
}

// 8013DAE8
void dHome_c::clearFlags() {
    mRooms[0].clearFlag(0);
    mRooms[0].clearFlag(1);
    mRooms[0].clearFlag(2);
    mRooms[1].clearFlag(0);
    mRooms[1].clearFlag(1);
    mRooms[1].clearFlag(2);
    mRooms[2].clearFlag(0);
    mRooms[2].clearFlag(1);
    mRooms[2].clearFlag(2);
}

// 8013DB7C
void dHome_c::updateFlags(int days, u32 isCurrent) {
    if (days < 0) {
        return;
    }
    u8 resetDays[] = {30, 7, 7, 7, 7, 7, 7, 7, 7, 7};
    u8 a = mRooms[0].countFlags();
    u8 b = mRooms[1].countFlags();
    u8 c = mRooms[2].countFlags();
    int count = a + b + c;
    int maxCounts[] = {3, 3, 3, 6, 9, 9};
    int max = maxCounts[mSize];
    int left = _15BA;
    if (count == max) {
        return;
    }
    u32 passed = FALSE;
    int rest;
    if (left <= days) {
        rest = days - left;
        passed = TRUE;
        count++;
    } else {
        left -= days;
        rest = 0;
    }
    if (count == 0 && isCurrent == TRUE) {
        left = 30;
    }
    count += rest / 7;
    if (count > max) {
        count = max;
    }
    if (passed == TRUE) {
        left = resetDays[count] - rest % 7;
    } else {
        left -= rest;
    }
    int v = left < 0 ? 0 : left;
    if (count == max) {
        v = resetDays[count];
    }
    _15BA = v;
    setFlagCount(count);
}

// 8013DD5C
void dHome_c::setFlagCount(int count) {
    int b = count - 3;
    int c = count - 6;
    int a = count < 0 ? 0 : (count > 3 ? 3 : count);
    b = b < 0 ? 0 : (b > 3 ? 3 : b);
    c = c < 0 ? 0 : (c > 3 ? 3 : c);
    mRooms[0].setFlags(getFlagMask(a));
    mRooms[1].setFlags(getFlagMask(b));
    mRooms[2].setFlags(getFlagMask(c));
    // @BUG - Writes one room past the end (into the next house).
    mRooms[3].setFlags(0);
}

// 8013DE5C
u8 dHome_c::getFlagMask(u8 count) {
    u8 masks[] = {0, 1, 3, 7};
    return masks[count];
}

// 8013DE90
void dHomeList_c::initAll() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        mHomes[i].init(i);
    }
}

// 8013DEE0
void dHomeList_c::initPlayerHome(u32 player) {
    int home = findPlayer(player);
    if (home != -1) {
        mHomes[home].init(home);
    }
}

// 8013DF24
int dHomeList_c::findOwner(dPrivateData_c *player) {
    if (player->mPID.isValid()) {
        for (int i = 0; i < PLAYER_NUM; i++) {
            if (const_cast<dHome_c *>(static_cast<const dHomeList_c *>(this)->getHome(i))->isOwner(&player->mPID)) {
                return i;
            }
        }
    }
    return -1;
}

// 8013DFA8
int dHomeList_c::findPlayer(u32 player) {
    if (player < PLAYER_NUM) {
        dPrivateData_c *data = dPlayerMgr_c::getPlayerRaw(player);
        if (data != NULL) {
            return findOwner(data);
        }
        return -1;
    }
    return -1;
}

// 8013E004
int dHomeList_c::findCurrentPlayer() {
    dPrivateData_c *data = dPlayerMgr_c::getCurrentPlayerRaw();
    if (data != NULL) {
        return findOwner(data);
    }
    return -1;
}

// 8013E04C
int dHomeList_c::getHomeFromScene(int *room, u8 scene) {
    int home = -1;
    int dummy = -1;
    if (room == NULL) {
        room = &dummy;
    }
    *room = getSceneAttrTableIndex(scene, sHomeScenes, &home);
    return home;
}

// 8013E0A8
dHome_c *dHomeList_c::getHome(u32 home) {
    if (home < PLAYER_NUM) {
        return &mHomes[home];
    }
    return NULL;
}

// 8013E0C4
const dHome_c *dHomeList_c::getHome(u32 home) const {
    if (home < PLAYER_NUM) {
        return &mHomes[home];
    }
    return NULL;
}

// 8013E0E0
int dHomeList_c::getPlayerOfHome(u32 home) {
    if (home < PLAYER_NUM) {
        for (u32 i = 0; i < PLAYER_NUM; i++) {
            int h = dSaveData_c::getRaw()->mHomes.findPlayer(i);
            if (h >= 0 && (int)home == h) {
                return i;
            }
        }
    }
    return -1;
}

// 8013E158
dPrivateData_c *dHomeList_c::getHomePlayer(u32 home) {
    int player = getPlayerOfHome(home);
    if (player >= 0) {
        return dPlayerMgr_c::getPlayer(player);
    }
    return NULL;
}

// 8013E18C
dPrivateData_c *dHomeList_c::getHomePlayerRaw(u32 home) {
    int player = getPlayerOfHome(home);
    if (player >= 0) {
        return dPlayerMgr_c::getPlayerRaw(player);
    }
    return NULL;
}

// 8013E1C0
int dHomeList_c::countVacant() const {
    int count = 0;
    for (int i = 0; i < PLAYER_NUM; i++) {
        const dHome_c *home = getHome(i);
        if (home != NULL && !home->mOwner.isValid()) {
            count++;
        }
    }
    return count;
}

// 8013E238
void dHomeList_c::updateAll(int days) {
    for (int i = 0; i < PLAYER_NUM; i++) {
        dHome_c *home = getHome(i);
        if (home != NULL) {
            dHome_c *cur = getHome(findCurrentPlayer());
            home->update(days, home == cur);
        }
        fn_801911E8(i);
    }
}

// 8013E2DC
dItem::Item getCurrentRoomSong() {
    int room;
    u8 scene = getCurrentScene();
    int home = dHomeList_c::getHomeFromScene(&room, scene);
    if (home != -1 && dHome_c::isValidRoom(room)) {
        dHome_c *h = dSaveData_c::getTown()->mHomes.getHome(home);
        if (h != NULL) {
            dHomeRoom_c *r = h->getRoom(room);
            if (r != NULL) {
                return getRoomSong(r);
            }
        }
    }
    return dItem::Item();
}

// 8013E384
BOOL setRoomSong(u8 scene, dItem::Item item) {
    int room;
    int home = dHomeList_c::getHomeFromScene(&room, scene);
    if (home != -1 && dHome_c::isValidRoom(room)) {
        dHome_c *h = dSaveData_c::getTown()->mHomes.getHome(home);
        if (h != NULL) {
            dHomeRoom_c *r = h->getRoom(room);
            if (r != NULL) {
                r->setSong(item);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8013E424
BOOL setCurrentRoomSong(dItem::Item item) {
    return setRoomSong(getCurrentScene(), item);
}

// 8013E458
BOOL clearRoomSong(u8 scene) {
    return setRoomSong(scene, dItem::Item());
}

// 8013E488
BOOL clearCurrentRoomSong() {
    return setCurrentRoomSong(dItem::Item());
}

// 8013E4B8
BOOL isSongInOtherRoom(u8 scene, const dItem::Item &item) {
    int room;
    int home = dHomeList_c::getHomeFromScene(&room, scene);
    if (home != -1 && room != -1) {
        dHome_c *h = dSaveData_c::getTown()->mHomes.getHome(home);
        dHomeRoom_c *self = h->getRoom(room);
        for (int i = 0; i < HOME_ROOM_NUM; i++) {
            dHomeRoom_c *r = h->getRoom(i);
            if (r != NULL && r != self) {
                dItem::Item other = getRoomSong(r);
                if (item.isSame(other)) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// 8013E598
BOOL isSongInOtherCurrentRoom(const dItem::Item &item) {
    return isSongInOtherRoom(getCurrentScene(), item);
}

// 8013E5D0
int getRoomFromScene(u8 scene) {
    int room;
    if (dHomeList_c::getHomeFromScene(&room, scene) != -1) {
        return room;
    }
    return -1;
}

// 8013E60C
dItem::Item getRoomSong(const dHomeRoom_c *room) {
    return room->mSong;
}
