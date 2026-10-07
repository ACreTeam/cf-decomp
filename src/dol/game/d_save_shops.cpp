// The town's shops in the town save (dSaveShops_c). .text 80143ECC..80143FE8.
#include <game/game/d_save_shops.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
void fn_8014CCC4(dYMD_c *date, BOOL gameDay); // date = today
BOOL fn_8014CD88(dYMD_c *date, BOOL gameDay); // date is today
}

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
    if (!reset && fn_8014CD88(&mDate, TRUE)) {
        return;
    }
    fn_8014CCC4(&mDate, TRUE);
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
