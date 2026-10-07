// Redd's stock (dSaveShopGallery_c). .text 801483D8..80148C40, .sdata2 80750CA0..80750CA8.
#include <game/game/d_save_shop_gallery.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/cLib/c_math.hpp>
#include <string.h>

extern u8 lbl_8059FF80[]; // the random item filter (fn_800C60B4)

extern "C" {
int fn_800C60B4(dItem::Item *out, int num, const int *range, int rangeNum, const void *filter, const dItem::Item *exclude,
                int excludeNum, int); // 800C60B4: random items
int fn_800AC5F8();                    // 800AC5F8
int fn_800AC6B4();                    // 800AC6B4
}

// 801483D8
void dSaveShopGallery_c::clear() {
    memset(this, 0, sizeof(dSaveShopGallery_c));
    mDate.reset();
}

// 80148414
u16 dSaveShopGallery_c::getItem(int slot) {
    if (slot < 0 || slot >= SHOP_GALLERY_STOCK_NUM) {
        return dItem::ITEM_ID_NONE;
    }
    if (!mStock[slot].mSold) {
        return mStock[slot].mItem;
    }
    return dItem::Item(dItem::ITEM_IDX_SOLD_OUT_SIGN_00).mId;
}

// 80148478
dTime_c dSaveShopGallery_c::getNextWednesday() {
    dTimeStamp_c day(dTime_c::getCurrent());
    day.toDayStart();
    if (day.get().wday < 3) {
        day.addDays(-day.get().wday + 3);
    } else {
        day.addDays(-day.get().wday + 10);
    }
    return day.get();
}

// 80148520
dTime_c dSaveShopGallery_c::getLastWednesday() {
    dTimeStamp_c day(dTime_c::getCurrent());
    day.toDayStart();
    if (day.get().wday < 3) {
        day.addDays(-day.get().wday - 4);
    } else {
        day.addDays(-day.get().wday + 3);
    }
    return day.get();
}

// 801485C8
void dSaveShopGallery_c::update() {
    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();

    if (!mDate.isNone()) {
        if (mDate.diffDays(today.getTicks(), FALSE, FALSE) < 0) {
            for (int i = 0; i < SHOP_GALLERY_STOCK_NUM; i++) {
                mStock[i].mSold = TRUE;
                mStock[i].mChecked = TRUE;
            }
            mDate.set(today.getTicks());
            return;
        }

        dTimeStamp_c last = mDate;
        const dTime_c &wednesday = getLastWednesday();
        dTimeStamp_c thisWeek(&wednesday);
        if (last.get().wday < 3) {
            last.addDays(-last.get().wday - 4);
        } else {
            last.addDays(-last.get().wday + 3);
        }
        if (last.diffDays(thisWeek.getTicks(), FALSE, FALSE) <= 0) {
            mDate.set(today.getTicks());
            return;
        }
    }

    mDate.set(today.getTicks());
    for (int i = 0; i < SHOP_GALLERY_STOCK_NUM; i++) {
        mStock[i].mItem = dItem::ITEM_ID_NONE;
        mStock[i].mSold = FALSE;
        mStock[i].mChecked = FALSE;
    }

    dItem::Item items[SHOP_GALLERY_STOCK_NUM];
    if (cM::rndInt(100) < getPictureRate()) {
        int range[2];
        range[0] = dItem::KIND_PICTURE;
        range[1] = 0x24;
        fn_800C60B4(&items[0], 1, range, 1, lbl_8059FF80, NULL, 0, 0);
    } else {
        int range[2];
        range[0] = dItem::KIND_FAKE_PICTURE_BEFORE;
        range[1] = 0x25;
        fn_800C60B4(&items[0], 1, range, 1, lbl_8059FF80, NULL, 0, 0);
    }

    f32 rate = getFtrRate();
    int rare = 0;
    int common = 0;
    for (int i = 1; i < SHOP_GALLERY_STOCK_NUM; i++) {
        BOOL isRare = cM::rndInt(100) < rate;
        if (isRare) {
            rare++;
        }
        if (!isRare) {
            common++;
        }
    }
    if (rare > 0) {
        int range[2];
        range[0] = dItem::KIND_FTR;
        range[1] = 7;
        fn_800C60B4(&items[1], rare, range, 1, lbl_8059FF80, NULL, 0, 0);
    }
    if (common > 0) {
        int range[2];
        range[0] = dItem::KIND_FTR;
        range[1] = 0;
        fn_800C60B4(&items[1 + rare], common, range, 1, lbl_8059FF80, NULL, 0, 0);
    }

    mStock[0].mItem = items[0].mId;
    mStock[1].mItem = items[1].mId;
    mStock[2].mItem = items[2].mId;
}

// 80148998
void dSaveShopGallery_c::sell(int slot) {
    if (slot < 0 || slot >= SHOP_GALLERY_STOCK_NUM) {
        return;
    }
    if (mStock[slot].mSold) {
        return;
    }
    mStock[slot].mSold = TRUE;
    mStock[slot].mChecked = TRUE;
}

// 801489D0
BOOL dSaveShopGallery_c::isSold(int slot) {
    if (slot < 0 || slot >= SHOP_GALLERY_STOCK_NUM) {
        return TRUE;
    }
    return mStock[slot].mSold != 0;
}

// 80148A04
int dSaveShopGallery_c::getUnsoldNum() {
    int num = 0;
    for (int i = 0; i < SHOP_GALLERY_STOCK_NUM; i++) {
        if (!mStock[i].mSold) {
            num++;
        }
    }
    return num;
}

// 80148A40
int dSaveShopGallery_c::getPictureRate() {
    return fn_800AC5F8() + 50;
}

// 80148A64
int dSaveShopGallery_c::getFtrRate() {
    return fn_800AC6B4() + 50;
}

// 80148A88
BOOL dSaveShopGallery_c::isChecked(int slot) {
    if (slot < 0 || slot >= SHOP_GALLERY_STOCK_NUM) {
        return FALSE;
    }
    return mStock[slot].mChecked != 0;
}

// 80148ABC
void dSaveShopGallery_c::check(int slot) {
    if (dPlayerMgr_c::getCurrentPlayerRaw()->isFlag0(0x14)) {
        if (slot < 0 || slot >= SHOP_GALLERY_STOCK_NUM) {
            return;
        }
        if (!mStock[slot].mSold) {
            mStock[slot].mChecked = TRUE;
        }
    } else {
        for (int i = 0; i < SHOP_GALLERY_STOCK_NUM; i++) {
            if (!mStock[i].mSold) {
                mStock[i].mChecked = TRUE;
            }
        }
    }
}

// 80148B70
void dSaveShopGallery_c::clearChecked() {
    for (int i = 0; i < SHOP_GALLERY_STOCK_NUM; i++) {
        if (!mStock[i].mSold) {
            mStock[i].mChecked = FALSE;
        }
    }
}

// 80148BAC
BOOL dSaveShopGallery_c::isOpen() {
    dTime_c now = *dTime_c::getCurrent();
    BOOL open = TRUE;
    if (now.hour < 10 && now.hour >= 1) {
        open = FALSE;
    }
    return open;
}
