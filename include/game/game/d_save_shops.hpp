#pragma once

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_save_shop.hpp>
#include <game/game/d_save_shop_tailor.hpp>
#include <game/game/d_save_shop_gallery.hpp>
#include <game/game/d_save_shop_grace.hpp>
#include <game/game/d_save_shop_stalk_market.hpp>
#include <game/game/d_save_catherine.hpp>
#include <game/game/d_save_cafe_guest.hpp>

// The town's shops in the town save: dSaveData_c::mShops (+0x5EC80). Source:
// src/dol/game/d_save_shops.cpp. Names follow NH's SaveShop* types where they correspond.
class dSaveShops_c { // 0x4660 (32-aligned by dDesign_c)
public:
    void clear();             // 80143ECC
    void loadTailorDesigns(); // 80143F3C
    void processDays(int days); // 80143F40: Nook's
    void update();            // 80143F48: once a day, restocks the shops
    void decideStalkPrices(); // 80143FE0

    /* 0x0000 */ dSaveShopTailor_c mShopTailor;
    /* 0x4440 */ dSaveShop_c mShop;
    /* 0x4504 */ dSaveShopGallery_c mShopGallery;
    /* 0x4518 */ dSaveShopGrace_c mShopGrace;
    /* 0x457C */ dYMD_c mDate; // the last update
    /* 0x4580 */ dSaveStalkMarket_c mStalkMarket;
    /* 0x45C8 */ dSaveCatherine_c mCatherine;
    /* 0x4648 */ dSaveCafeGuest_c mCafeGuest;
};
