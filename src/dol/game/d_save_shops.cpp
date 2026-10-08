// The town's shops in the town save (dSaveShops_c). .text 80143ECC..80143FE8.
#include <game/game/d_save_shops.hpp>

// 80143ECC
void dSaveShops_c::clear() {
    mShop.clear();
    mStalkMarket.clear();
    mCatherine.clear();
    mShopTailor.clear();
    mShopGallery.clear();
    mShopGrace.clear();
    mCafeGuest.clear();
    mDate.clear();
}

// 80143F3C
void dSaveShops_c::loadTailorDesigns() {
    mShopTailor.loadDesigns();
}

// 80143F40
void dSaveShops_c::processDays(int days) {
    mShop.processDays(days);
}

// 80143F48
void dSaveShops_c::update() {
    BOOL reset = FALSE;
    if (mDate.year == 0 || mDate.day == 0) {
        reset = TRUE;
    }
    if (!reset && mDate.isToday(TRUE)) {
        return;
    }
    mDate.setToday(TRUE);
    mShop.update();
    mShopTailor.update();
    mShopGallery.update();
    mShopGrace.update();
    mCafeGuest.update();
}

// 80143FE0
void dSaveShops_c::decideStalkPrices() {
    mStalkMarket.decidePrices();
}
