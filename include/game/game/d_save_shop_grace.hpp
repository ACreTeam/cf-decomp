#pragma once

#include <types.h>
#include <game/game/d_time_stamp.hpp>
#include <game/game/d_save_shop.hpp>

#define SHOP_GRACE_STOCK_NUM 23

// GracieGrace: dSaveShops_c::mShopGrace. The stock follows the season (4 layout types, by month); in the
// last month of a season she has a sale from the 15th (stages 1..3 from the 15th / 21st / 26th) during
// which more and more of the stock is sold off. Source: src/dol/game/d_save_shop_grace.cpp.
class dSaveShopGrace_c { // 0x64
public:
    void clear();                          // 80148C40
    u16 getItem(int slot, BOOL showSold);  // 80148C7C: ITEM_IDX_SOLD_OUT_SIGN_01 for sold slots if showSold
    int getSaleStage();                    // 80148CF0: 0 = no sale
    int getLayoutType(int month);          // 80148DC0: 0..3 (dShopLayout_get)
    void update();                         // 80148DD4: daily
    BOOL isSold(int slot);                 // 80149494: TRUE for slots out of range
    BOOL isSoldOut();                      // 801494C8
    void sellOff();                        // 801494F4: sells random slots up to the sale stage's count

    /* 0x00 */ dTimeStamp_c mDate; // the last update
    /* 0x08 */ dSaveShopStock_c mStock[SHOP_GRACE_STOCK_NUM];
};
