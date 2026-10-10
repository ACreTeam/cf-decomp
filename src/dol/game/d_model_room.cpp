// Happy Room Academy model rooms (dSvMdlRm_c) and the theme of the month. .text 80111A2C..80112BB8,
// .rodata 80475CF8..80475D10, .bss 805ECB58..805ED1D0, .sbss 8074E6E8..8074E6F0, .sdata2 80750AD8..80750AE0.
#include <game/game/d_model_room.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_random.hpp>
#include <game/game/d_save_data.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/core/eggHeap.h>
#include <game/game/d_ftr.hpp>

// 80475CF8: BG per room type
static const u32 sBgIds[] = {0xD3, 0xD4, 0xD5, 0xD5, 0xD5, 0};

// 80111A2C
int getModelRoomRnd(const dTime_c &time, u32 max) {
    dRandom_c rnd(0x9D);
    rnd.initFromDateWithSalt(time, 0x85775358);
    u32 r = (int)(max * rnd.rnd());
    if (r >= max) {
        r = 0;
    }
    return r;
}

// 80111ABC
dTime_c getModelRoomDate() {
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour < TIME_DAY_START_HOUR) {
        now.add(-1, 0, 0, 0);
    }
    dTime_c date = now;
    int wday = dTime_c::getWeekday(now.year, now.month, now.mday);
    date.add(-wday, 0, 0, 0);
    while (date.mday >= 8) {
        date.add(-7, 0, 0, 0);
    }
    return date;
}

// 80111C40
int getModelRoomTheme() {
    dTime_c date = getModelRoomDate();
    return (date.month + getModelRoomRnd(date, 6)) % 6;
}

// 80111CEC
void dSvMdlRm_c::clearFlags() {
    mFlags = 0;
}

// 80111CF8
void dSvMdlRm_c::clear() {
    clearFlags();
    mScore = 0;
    mRoomType = 0;
    mTheme = 0;
    _54B = 0;
    init(-1, -1);
    mPlayerID.clear();
    mAnimalID.clear();
}

// 80111D58
void dSvMdlRm_c::setOwner(const dPersonalID_c *pid) {
    mPlayerID = *pid;
    mFlags |= MODEL_ROOM_OWNED;
    mFlags &= ~MODEL_ROOM_VILLAGER;
}

// 80111E44
void dSvMdlRm_c::setOwner(const dAnmPersonalID_c *aid) {
    mAnimalID = *aid;
    mFlags |= MODEL_ROOM_OWNED;
    mFlags |= MODEL_ROOM_VILLAGER;
}

// 80111F38
int dSvMdlRm_c::getBgId() {
    if (mRoomType < 5) {
        return sBgIds[mRoomType];
    }
    return 0xBA;
}

// 80111F60
BOOL dSvMdlRm_c::isFromThisTown() {
    const dPersonalID_c *pid = getPlayerID();
    if (pid != NULL) {
        return pid->land == dSaveData_c::getTown()->mLandID;
    }
    const dAnmPersonalID_c *aid = getAnimalID();
    if (aid != NULL) {
        return aid->mLand == dSaveData_c::getTown()->mLandID;
    }
    return FALSE;
}

// 80112114
BOOL dSvMdlRm_c::setFromAnimal(u32 animalIdx) {
    if (animalIdx < ANIMAL_NUM) {
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(animalIdx);
        if (animal != NULL) {
            EGG::Heap *heap = EGG::Heap::getCurrentHeap();
            dFdInfoNpcHs_c *info = dFdInfoNpcHs_c::create(animalIdx, heap);
            if (info != NULL) {
                clear();

                static dHomeRoom_c sRoom;
                sRoom.clear();
                setOwner(&animal->mID);
                mRoomType = 1;
                info->clearUnknownItems();
                dItem::Item *dst = sRoom.getLayer(0)->mItems[0];
                for (int z = 0; z < 16; z++) {
                    for (int x = 0; x < 16; x++) {
                        dItem::Item *item = info->getItem(x, z, 0);
                        if (item != NULL) {
                            *dst = *item;
                        }
                        dst++;
                    }
                }
                sRoom.setSong(animal->getMusic());
                if (dItem::infoBank_c::get()->getBITM(*animal->getWall())) {
                    sRoom.setWallpaper(*animal->getWall());
                } else {
                    sRoom.setWallpaper(dItem::Item(dItem::ITEM_IDX_EXOTIC_WALL));
                }
                if (dItem::infoBank_c::get()->getBITM(*animal->getCarpet())) {
                    sRoom.setCarpet(*animal->getCarpet());
                } else {
                    // The default rug goes through the wallpaper setter.
                    sRoom.setWallpaper(dItem::Item(dItem::ITEM_IDX_EXOTIC_RUG));
                }
                dHomeRoom_c::operator=(sRoom);
                dFdInfoNpcHs_c::remove(&info, heap);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80112640
BOOL dSvMdlRm_c::setFromHome(u32 home, int room) {
    if (home < PLAYER_NUM) {
        // A signed copy: using home directly moves its register copy into the prologue.
        int idx = home;
        dPrivateData_c *player = dSaveData_c::getTown()->mHomes.getHomePlayer(idx);
        if (player != NULL) {
            clear();
            dHome_c *h = dSaveData_c::getTown()->mHomes.getHome(idx);
            dHomeRoom_c *r = h->getRoom(room);
            if (r != NULL) {
                setRoom(r);
                setOwner(&player->mPID);
                if (room == 1) {
                    mRoomType = 1;
                } else if (room == 2) {
                    mRoomType = 2;
                } else {
                    mRoomType = h->mSize;
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 801129A8
dItem::Item dSvMdlRm_c::getRandomRoomItem(dSvMdlRm_c::searchCB_c &check) {
    static dItem::Item sItems[256];
    dItem::Item *dst = sItems;
    u32 num = 0;
    for (u32 l = 0; l < HOME_LAYER_NUM; l++) {
        dItem::Item *item = getLayerItems(l)->mItems[0];
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                if (check.check(item) && num < 256) {
                    num++;
                    *dst++ = *item;
                }
                item++;
            }
        }
    }
    if (num != 0) {
        return sItems[cM::rndInt(num)];
    }
    return dItem::Item();
}

// 80112B08
int dSvMdlRm_c::countRoomFtrTiles() {
    dNpcFtrShape_c shape;
    int num = 0;
    dItem::Item *item = getLayerItems(0)->mItems[0];
    for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
            if (item->hasFtrFunc()) {
                fn_800A8B28(&shape, *item);
                num += fn_800A8BB8(&shape);
            }
            item++;
        }
    }
    return num;
}
