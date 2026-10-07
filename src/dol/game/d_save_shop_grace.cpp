// GracieGrace's stock (dSaveShopGrace_c). .text 80148C40..80149608, .rodata 804764A8..80476648,
// .data 804EF360..804EF388 (jumptable), .sdata2 80750CA8..80750CB0.
#include <game/game/d_save_shop_grace.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_random.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_def.hpp>
#include <game/cLib/c_math.hpp>
#include <string.h>

static const int sMonthType[12] = {3, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3}; // the last month of each type has the sale
static const int sSeries[4] = {18, 21, 20, 19}; // the series of slots 0..11

// 1-based index in the series of slots 0..9.
static const int sOrder[4][10] = {
    {7, 1, 8, 3, 2, 10, 4, 6, 5, 9},
    {3, 7, 2, 1, 6, 10, 5, 4, 8, 9},
    {2, 3, 5, 9, 8, 10, 7, 1, 4, 6},
    {1, 8, 5, 3, 2, 10, 4, 6, 9, 7},
};

// The variant of slots 0..9.
static const int sVariant[4][10] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 0},
    {0, 0, 0, 0, 3, 0, 0, 3, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 0, 3, 1, 3, 1},
};

static const int sFromA[4] = {70, 71, 72, 73}; // fromCandCB_c of slots 12..20
static const int sFromB[4] = {85, 86, 87, 88}; // fromCandCB_c of slots 21..22

// 80148C40
void dSaveShopGrace_c::clear() {
    memset(this, 0, sizeof(dSaveShopGrace_c));
    mDate.reset();
}

// 80148C7C
u16 dSaveShopGrace_c::getItem(int slot, BOOL showSold) {
    if (slot < 0 || slot >= SHOP_GRACE_STOCK_NUM) {
        return dItem::ITEM_ID_NONE;
    }
    if (!showSold || !mStock[slot].mSold) {
        return mStock[slot].mItem;
    }
    return dItem::Item(dItem::ITEM_IDX_SOLD_OUT_SIGN_01).mId;
}

// 80148CF0
int dSaveShopGrace_c::getSaleStage() {
    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();
    switch (today.get().month) {
    case 0:
    case 3:
    case 6:
    case 9:
        if (today.get().mday >= 26) {
            return 3;
        }
        if (today.get().mday >= 21) {
            return 2;
        }
        return today.get().mday >= 15;
    }
    return 0;
}

// 80148DC0
int dSaveShopGrace_c::getLayoutType(int month) {
    return sMonthType[month];
}

// 80148DD4
void dSaveShopGrace_c::update() {
    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();

    dSaveData_c::getTown()->clearFlag(0x17);
    if (getSaleStage() == 0) {
        dTime_c now = *dTime_c::getCurrent();
        dRandom_c rnd(0x9D);
        rnd.initFromDateWithSalt(now, 1234);
        if (rnd.rndF(100.0f) < 10.0f) {
            dSaveData_c::getTown()->setFlag(0x17);
        }
    }

    if (!mDate.isNone()) {
        if (mDate.diffDays(today.getTicks(), FALSE, FALSE) < 0) {
            if (getSaleStage() != 0) {
                for (int i = 0; i < SHOP_GRACE_STOCK_NUM; i++) {
                    mStock[i].mSold = TRUE;
                }
                mDate.set(today.getTicks());
                return;
            }
            if (getLayoutType(today.get().month) == getLayoutType(mDate.get().month)) {
                mDate.set(today.getTicks());
                return;
            }
        } else if (getLayoutType(today.get().month) == getLayoutType(mDate.get().month)) {
            sellOff();
            mDate.set(today.getTicks());
            return;
        }
    }

    mDate.set(today.getTicks());
    for (int i = 0; i < SHOP_GRACE_STOCK_NUM; i++) {
        mStock[i].mItem = dItem::ITEM_ID_NONE;
        mStock[i].mSold = FALSE;
    }

    for (int i = 0; i < 10; i++) {
        mStock[i].mItem = dItem::Item(dItem::seeker_c::get()->getNthInSeries(
                                          sOrder[getLayoutType(today.get().month)][i] - 1,
                                          sSeries[getLayoutType(today.get().month)], 3))
                              .withVariant(sVariant[getLayoutType(today.get().month)][i])
                              .mId;
    }

    const int *const series = sSeries;
    mStock[10].mItem =
        dItem::seeker_c::get()->getNthInSeries(0, series[getLayoutType(today.get().month)], 1).mId;
    mStock[11].mItem =
        dItem::seeker_c::get()->getNthInSeries(0, series[getLayoutType(today.get().month)], 2).mId;

    dItem::fromCandCB_c fromA(sFromA[getLayoutType(today.get().month)]);
    dItem::seeker_c::get()->search(dItem::KIND_CLOTH, 6, &fromA);
    for (int i = 0; i < 6; i++) {
        mStock[12 + i].mItem = dItem::seeker_c::get()->getNth(i).mId;
    }
    dItem::seeker_c::get()->search(dItem::KIND_CAP, 6, &fromA);
    mStock[18].mItem = dItem::seeker_c::get()->getNth(0).mId;
    dItem::seeker_c::get()->search(dItem::KIND_ACC, 6, &fromA);
    mStock[19].mItem = dItem::seeker_c::get()->getNth(0).mId;
    dItem::seeker_c::get()->search(dItem::KIND_UMBRELLA, 6, &fromA);
    mStock[20].mItem = dItem::seeker_c::get()->getNth(0).mId;

    dItem::fromCandCB_c fromB(sFromB[getLayoutType(today.get().month)]);
    dItem::seeker_c::get()->search(dItem::KIND_CLOTH, 6, &fromB);
    mStock[21].mItem = dItem::seeker_c::get()->getNth(0).withVariant(1).mId;
    dItem::seeker_c::get()->search(dItem::KIND_CAP, 6, &fromB);
    if (dItem::seeker_c::get()->mCount == 1) {
        mStock[22].mItem = dItem::seeker_c::get()->getNth(0).withVariant(1).mId;
    } else {
        dItem::seeker_c::get()->search(dItem::KIND_ACC, 6, &fromB);
        mStock[22].mItem = dItem::seeker_c::get()->getNth(0).withVariant(1).mId;
    }
    sellOff();
}

// 80149494
BOOL dSaveShopGrace_c::isSold(int slot) {
    if (slot < 0 || slot >= SHOP_GRACE_STOCK_NUM) {
        return TRUE;
    }
    return mStock[slot].mSold != 0;
}

// 801494C8
BOOL dSaveShopGrace_c::isSoldOut() {
    for (int i = 0; i < SHOP_GRACE_STOCK_NUM; i++) {
        if (!mStock[i].mSold) {
            return FALSE;
        }
    }
    return TRUE;
}

// 801494F4
void dSaveShopGrace_c::sellOff() {
    int sold = 0;
    for (int i = 0; i < SHOP_GRACE_STOCK_NUM; i++) {
        if (mStock[i].mSold) {
            sold++;
        }
    }

    int target = 0;
    switch (getSaleStage()) {
    case 0:
        return;
    case 1:
        target = 7;
        break;
    case 2:
        target = 10;
        break;
    case 3:
        target = 14;
        break;
    }
    if (sold >= target) {
        return;
    }

    while (sold < target) {
        int n = cM::rndInt(SHOP_GRACE_STOCK_NUM - sold) + 1;
        int unsold = 0;
        for (int i = 0; i < SHOP_GRACE_STOCK_NUM; i++) {
            if (!mStock[i].mSold) {
                unsold++;
                if (unsold == n) {
                    mStock[i].mSold = TRUE;
                    break;
                }
            }
        }
        sold++;
    }
}
