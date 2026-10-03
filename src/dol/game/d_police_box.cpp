// The police station's lost and found. .text 801538F8..80153E9C.
#include <game/game/d_police_box.hpp>
#include <game/game/d_item.hpp>
#include <game/cLib/c_math.hpp>

// Dependencies whose owners are not recovered yet.
extern "C" {
// Picks a random item (also used by dPrivateData_c for presents).
void fn_800C60B4(dItem::Item *item, int, s32 *range, int, void *, int, int, int);
}
extern u8 lbl_8059FF80[];

// 801538F8
void dPoliceBox_c::clear() {
    for (int i = 0; i < POLICE_BOX_ITEM_NUM; i++) {
        mItems[i] = dItem::ITEM_ID_NONE;
    }
}

// 80153934
void dPoliceBox_c::init() {
    clear();
    // 804766C0
    s32 categories[] = {9, 3, 4};
    s32 range[2];
    range[0] = 9;
    range[1] = 0;
    dItem::Item item;
    for (int i = 0; i < 3; i++) {
        range[0] = categories[i];
        fn_800C60B4(&item, 1, range, 1, lbl_8059FF80, 0, 0, 0);
        mItems[i] = item.mId;
    }
}

// 80153A04
void dPoliceBox_c::refill(int days) {
    compact();
    // 804766CC
    s32 categories[] = {9, 3, 4};
    int n = 0;
    for (int i = 0; i < POLICE_BOX_ITEM_NUM; i++) {
        if (mItems[i] != dItem::ITEM_ID_NONE) {
            n++;
        }
    }
    s32 range[2];
    range[0] = 9;
    range[1] = 0;
    dItem::Item item;
    for (int day = 0; day < days; day++) {
        if (n >= 10) {
            break;
        }
        for (int i = 0; i < POLICE_BOX_ITEM_NUM; i++) {
            if (mItems[i] == dItem::ITEM_ID_NONE) {
                int roll = cM::rndInt(100);
                int category = 0xFF;
                if (roll >= 50 && roll < 100) {
                } else if (roll >= 30 && roll < 50) {
                    mItems[i] = dItem::Item(0xA).mId;
                    n++;
                } else {
                    if (roll >= 20 && roll < 30) {
                        category = 1;
                    } else if (roll >= 10 && roll < 20) {
                        category = 2;
                    } else if (roll >= 0 && roll < 10) {
                        category = 0;
                    }
                    range[0] = categories[category];
                    fn_800C60B4(&item, 1, range, 1, lbl_8059FF80, 0, 0, 0);
                    mItems[i] = item.mId;
                    n++;
                }
                break;
            }
        }
    }
}

// 80153C14
void dPoliceBox_c::push(u16 item) {
    if (!add(item)) {
        for (int i = 0; i < POLICE_BOX_ITEM_NUM - 1; i++) {
            mItems[i] = mItems[i + 1];
        }
        mItems[POLICE_BOX_ITEM_NUM - 1] = item;
    }
}

// 80153CB0
int dPoliceBox_c::count() {
    int n = 0;
    for (int i = 0; i < POLICE_BOX_ITEM_NUM; i++) {
        if (mItems[i] != dItem::ITEM_ID_NONE) {
            n++;
        }
    }
    return n;
}

// 80153D34
u16 dPoliceBox_c::get(int slot) {
    return mItems[slot];
}

// 80153D40
void dPoliceBox_c::set(int slot, u16 item) {
    mItems[slot] = item;
}

// 80153D4C
BOOL dPoliceBox_c::add(u16 item) {
    compact();
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item(item));
    if (bitm != NULL) {
        int kind = bitm->getKind();
        if (bitm->getKindFlag4() && kind != dItem::KIND_FISH && kind != dItem::KIND_INSECT) {
            for (int i = 0; i < POLICE_BOX_ITEM_NUM; i++) {
                if (mItems[i] == dItem::ITEM_ID_NONE) {
                    mItems[i] = item;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// 80153E14
void dPoliceBox_c::compact() {
    BOOL moved;
    for (int i = 0; i < POLICE_BOX_ITEM_NUM; i++) {
        moved = FALSE;
        if (mItems[i] == dItem::ITEM_ID_NONE) {
            for (int j = i + 1; j < POLICE_BOX_ITEM_NUM; j++) {
                if (mItems[j] != dItem::ITEM_ID_NONE) {
                    mItems[i] = mItems[j];
                    mItems[j] = dItem::ITEM_ID_NONE;
                    moved = TRUE;
                    break;
                }
            }
            if (!moved) {
                return;
            }
        }
    }
}
