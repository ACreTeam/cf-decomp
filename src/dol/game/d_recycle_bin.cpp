// The town hall's recycle bin. .text 80153E9C..80154080, no data.
#include <game/game/d_recycle_bin.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_time_stamp.hpp>

// 80153E9C
void dRecycleBin_c::clear() {
    for (int i = 0; i < RECYCLE_BIN_ITEM_NUM; i++) {
        mItems[i] = dItem::ITEM_ID_NONE;
    }
}

// 80153ED8
void dRecycleBin_c::update(dTime_c *now, int days) {
    if (days <= 0) {
        return;
    }
    if (days >= 4) {
        clear();
        return;
    }
    dTimeStamp_c time(now);
    time.toDayStart();
    switch (time.getWeekday()) {
    case 1:
    case 4:
        clear();
        break;
    case 2:
    case 5:
        if (days >= 2) {
            clear();
        }
        break;
    case 3:
    case 6:
        if (days >= 3) {
            clear();
        }
        break;
    }
}

// 80153FA0
u16 dRecycleBin_c::get(int slot) {
    return mItems[slot];
}

// 80153FAC
void dRecycleBin_c::set(int slot, u16 item) {
    mItems[slot] = item;
}

// 80153FB8
BOOL dRecycleBin_c::add(u16 item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item(item));
    if (bitm != NULL) {
        int kind = bitm->getKind();
        if (bitm->getKindFlag4() && kind != dItem::KIND_FISH && kind != dItem::KIND_INSECT) {
            for (int i = 0; i < RECYCLE_BIN_ITEM_NUM; i++) {
                if (mItems[i] == dItem::ITEM_ID_NONE) {
                    mItems[i] = item;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// 8015407C
u16 *dRecycleBin_c::getItems() {
    return mItems;
}
