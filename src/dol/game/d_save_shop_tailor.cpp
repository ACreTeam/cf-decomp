// Able Sisters' stock (dSaveShopTailor_c). .text 80147ECC..801483D8.
#include <game/game/d_save_shop_tailor.hpp>
#include <game/game/d_shop_layout.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_field_info.hpp>
#include <string.h>

extern u8 lbl_8059FF80[]; // the random item filter (fn_800C60B4)

extern "C" {
int fn_800C60B4(dItem::Item *out, int num, const int *range, int rangeNum, const void *filter, const dItem::Item *exclude,
                int excludeNum, int); // 800C60B4: random items
BOOL fn_800DCEDC();                   // 800DCEDC: an online session is active
u32 fn_800DCF30();                    // 800DCF30: players in the session
void fn_800DD4C8();                   // 800DD4C8
void fn_800DD518(const void *data, u32 size); // 800DD518
void fn_800DD588(int type, int arg);  // 800DD588
void *fn_800A9058();                  // 800A9058
int fn_800A8F98(void *, int unitX, int unitZ, int); // 800A8F98: -1 when none
void fn_800ABAF4(int);                // 800ABAF4
}

// 80147ECC
void dSaveShopTailor_c::clear() {
    memset(this, 0, sizeof(dSaveShopTailor_c));
    for (int i = 0; i < SHOP_TAILOR_DESIGN_NUM; i++) {
        mDesigns[i].clear();
    }
}

// 80147F24
void dSaveShopTailor_c::loadDesigns() {
    for (int i = 0; i < SHOP_TAILOR_DESIGN_NUM; i++) {
        mDesigns[i].setFromItem(dItem::ITEM_IDX_CHECKED_CLOTH + i);
        mDesigns[i].loadTextureB(i);
    }
}

// 80147F80
dDesign_c *dSaveShopTailor_c::getDesign(int idx) {
    return &mDesigns[idx % SHOP_TAILOR_DESIGN_NUM];
}

// 80147FA0
u16 dSaveShopTailor_c::getItem(int slot) {
    if (slot < 0 || slot >= SHOP_TAILOR_STOCK_NUM) {
        return dItem::ITEM_ID_NONE;
    }
    if (!mStock[slot].mSold) {
        return mStock[slot].mItem;
    }
    return dItem::Item(dItem::ITEM_IDX_SOLD_OUT_SIGN_00).mId;
}

// 80148004
void dSaveShopTailor_c::update() {
    for (int i = 0; i < SHOP_TAILOR_STOCK_NUM; i++) {
        mStock[i].mItem = dItem::ITEM_ID_NONE;
        mStock[i].mSold = FALSE;
    }

    {
        dItem::Item item;
        int range[2];
        range[0] = 0x38;
        range[1] = 8;
        fn_800C60B4(&item, 1, range, 1, lbl_8059FF80, NULL, 0, 0);
        mStock[0].mItem = item.mId;
    }
    {
        dItem::Item items[3];
        int range[2];
        range[0] = 4;
        range[1] = 0;
        fn_800C60B4(items, 3, range, 1, lbl_8059FF80, NULL, 0, 0);
        mStock[1].mItem = items[0].mId;
        mStock[2].mItem = items[1].mId;
        mStock[3].mItem = items[2].mId;
    }
    for (int i = 0; i < 8; i++) {
        mStock[4 + i].mItem = dItem::Item(dItem::ITEM_IDX_TA_CLOTH_00, i, FALSE).mId;
    }
    {
        dItem::Item item;
        int range[2];
        range[0] = 5;
        range[1] = 8;
        fn_800C60B4(&item, 1, range, 1, lbl_8059FF80, NULL, 0, 0);
        mStock[12].mItem = item.mId;
    }
    {
        dItem::Item item;
        int range[2];
        range[0] = 6;
        range[1] = 8;
        fn_800C60B4(&item, 1, range, 1, lbl_8059FF80, NULL, 0, 0);
        mStock[13].mItem = item.mId;
    }
}

// 80148210
void dSaveShopTailor_c::sellAndNotify(int scene, int slot, int unitX, int unitZ, int price, int flag) {
    if ((u32)slot != 0xFF) {
        sell(scene, slot, unitX, unitZ);
    }
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        dShopSale_c sale;
        sale.mSlot = slot;
        sale.mUnitX = unitX;
        sale.mUnitZ = unitZ;
        sale.mPrice = price;
        sale.mFlag = flag;
        sale.mScene = scene;
        fn_800DD4C8();
        fn_800DD518(&sale, sizeof(dShopSale_c));
        fn_800DD588(0x42, 4);
    }
}

// 801482BC
void dSaveShopTailor_c::sell(int scene, int slot, int unitX, int unitZ) {
    if (slot >= 0 && slot < SHOP_TAILOR_STOCK_NUM) {
    } else {
        return;
    }
    if (!isSold(slot)) {
        mStock[slot].mSold = TRUE;
    } else {
        return;
    }

    if ((u32)scene != getCurrentScene()) {
        return;
    }
    int idx = fn_800A8F98(fn_800A9058(), unitX, unitZ, 0);
    if (idx != -1) {
        fn_800ABAF4(idx);
        return;
    }
    dItem::Item sold(dItem::ITEM_IDX_SOLD_OUT_SIGN_00);
    dFdBase_c *fd = fn_80190C44(0);
    if (fd != NULL) {
        fd->setItem(&sold, 0, 0, unitX, unitZ, 0);
    }
}

// 801483A4
BOOL dSaveShopTailor_c::isSold(int slot) {
    if (slot < 0 || slot >= SHOP_TAILOR_STOCK_NUM) {
        return TRUE;
    }
    return mStock[slot].mSold != 0;
}
