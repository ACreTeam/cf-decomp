#pragma once

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_time_stamp.hpp>

#define SHOP_GALLERY_STOCK_NUM 3

// A slot of Redd's stock.
struct dSaveShopGalleryStock_c { // 0x4
    /* 0x0 */ u16 mItem;
    /* 0x2 */ u8 mSold;
    /* 0x3 */ u8 mChecked;
};

// Redd's (NH SaveShopGallery): dSaveShops_c::mShopGallery. Restocked once a week (weeks start on
// Wednesday): slot 0 a painting (genuine with getPictureRate()%, else a fake), slots 1..2 furniture.
// Source: src/dol/game/d_save_shop_gallery.cpp.
class dSaveShopGallery_c { // 0x14
public:
    void clear();              // 801483D8
    u16 getItem(int slot);     // 80148414: ITEM_IDX_SOLD_OUT_SIGN_00 once sold
    dTime_c getNextWednesday(); // 80148478
    dTime_c getLastWednesday(); // 80148520: today if it is Wednesday
    void update();             // 801485C8: daily, restocks in a new week
    void sell(int slot);       // 80148998
    BOOL isSold(int slot);     // 801489D0: TRUE for slots out of range
    int getUnsoldNum();        // 80148A04
    int getPictureRate();      // 80148A40: 50 + fn_800AC5F8
    int getFtrRate();          // 80148A64: 50 + fn_800AC6B4
    BOOL isChecked(int slot);  // 80148A88
    void check(int slot);      // 80148ABC: every unsold slot unless the player has flag0 0x14
    void clearChecked();       // 80148B70
    BOOL isOpen();             // 80148BAC: closed from 1:00 to 10:00

    /* 0x00 */ dTimeStamp_c mDate; // the last update
    /* 0x08 */ dSaveShopGalleryStock_c mStock[SHOP_GALLERY_STOCK_NUM];
};
