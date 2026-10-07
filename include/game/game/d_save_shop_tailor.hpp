#pragma once

#include <types.h>
#include <game/game/d_dsn.hpp>
#include <game/game/d_save_shop.hpp>

#define SHOP_TAILOR_DESIGN_NUM 8
#define SHOP_TAILOR_STOCK_NUM 14

// Able Sisters' (NH SaveShopTailor): dSaveShops_c::mShopTailor. The displayed designs come first.
// Stock: 0 a random item of kind range 0x38, 1..3 of range 4, 4..11 the 8 TA_CLOTH designs, 12 of
// range 5, 13 of range 6. Source: src/dol/game/d_save_shop_tailor.cpp.
class dSaveShopTailor_c { // 0x4440
public:
    void clear();                                                        // 80147ECC
    void loadDesigns();                                                  // 80147F24: the checked-cloth designs
    dDesign_c *getDesign(int idx);                                       // 80147F80: idx % 8
    u16 getItem(int slot);                                               // 80147FA0: ITEM_IDX_SOLD_OUT_SIGN_00 once sold
    void update();                                                       // 80148004: daily, restocks
    void sellAndNotify(int scene, int slot, int unitX, int unitZ, int price, int flag); // 80148210
    void sell(int scene, int slot, int unitX, int unitZ);                // 801482BC: marks the slot sold, clears its unit
    BOOL isSold(int slot);                                               // 801483A4: TRUE for slots out of range

    /* 0x0000 */ dDesign_c mDesigns[SHOP_TAILOR_DESIGN_NUM];
    /* 0x4400 */ dSaveShopStock_c mStock[SHOP_TAILOR_STOCK_NUM];
    /* 0x4438 */ u8 _4438[8];
};
