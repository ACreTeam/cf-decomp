// A shop room's floor stock (dShopFg_c). See include/game/game/d_shop_fg.hpp.
#include <game/game/d_shop_fg.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shop.hpp>
#include <game/game/d_save_shop_gallery.hpp>
#include <game/game/d_save_shop_grace.hpp>
#include <game/game/d_save_shop_tailor.hpp>
#include <game/game/d_save_shops.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shop_layout.hpp>

namespace dShopFg {
// .text 0x5C
BASE_PROFILE(SHOP_FG, dShopFg_c);
} // namespace dShopFg

// .text 0xA8
int dShopFg_c::create() {
    switch (dScene::getCurrentScene()) {
    case dScene::SCENE_RM_SHOP0:
        if (dSaveData_c::getTown()->mShops.mShop.mStage != SHOP_STAGE_CRANNY) {
            dSaveData_c::getTown()->mShops.mShop.setStage(SHOP_STAGE_CRANNY);
            dSaveData_c::getTown()->mShops.mShop.update();
        }
        putShopItems();
        break;
    case dScene::SCENE_RM_SHOP1:
        if (dSaveData_c::getTown()->mShops.mShop.mStage !=
            SHOP_STAGE_NOOK_N_GO) {
            dSaveData_c::getTown()->mShops.mShop.setStage(SHOP_STAGE_NOOK_N_GO);
            dSaveData_c::getTown()->mShops.mShop.update();
        }
        putShopItems();
        break;
    case dScene::SCENE_RM_SHOP2:
        if (dSaveData_c::getTown()->mShops.mShop.mStage != SHOP_STAGE_NOOKWAY) {
            dSaveData_c::getTown()->mShops.mShop.setStage(SHOP_STAGE_NOOKWAY);
            dSaveData_c::getTown()->mShops.mShop.update();
        }
        putShopItems();
        break;
    case dScene::SCENE_RM_SHOP3_F1:
    case dScene::SCENE_RM_SHOP3_F2:
        if (dScene::getCurrentScene() == dScene::SCENE_RM_SHOP3_F1) {
            dBGCF::setRoute(9, 4, TRUE);
            dBGCF::setRoute(9, 7, TRUE);
            dBGCF::setRoute(9, 11, TRUE);
            dBGCF::setRoute(9, 12, TRUE);
            dBGCF::setRoute(9, 13, TRUE);
        } else {
            dBGCF::setRoute(10, 5, FALSE);
            dBGCF::setRoute(10, 6, FALSE);
        }
        if (dSaveData_c::getTown()->mShops.mShop.mStage !=
            SHOP_STAGE_NOOKINGTONS) {
            dSaveData_c::getTown()->mShops.mShop.setStage(
                SHOP_STAGE_NOOKINGTONS);
            dSaveData_c::getTown()->mShops.mShop.update();
        }
        putShopItems();
        break;
    case dScene::SCENE_RM_BROKER:
        putBrokerItems();
        break;
    case dScene::SCENE_RM_TAILOR:
        putTailorItems();
        break;
    case dScene::SCENE_RM_GRACE:
        putGraceItems();
        break;
    case dScene::SCENE_RM_BARBER:
        putBarberItems();
        break;
    case dScene::SCENE_RM_MM_CAFE:
        putCafeItems();
        break;
    }
    return SUCCEEDED;
}

// .text 0x2CC
int dShopFg_c::execute() {
    deleteRequest();
    return SUCCEEDED;
}

// .text 0x2F0
void dShopFg_c::putItem(const dItem::Item &item, int unitX, int unitZ) {
    if (item.isValid()) {
        dField::getInfo(0)->setItem(&item, 0, 0, unitX, unitZ, 0);
    }
}

// .text 0x360
void dShopFg_c::putShopItems() {
    dSaveShop_c *shop = &dSaveData_c::getTown()->mShops.mShop;
    int unitX, unitZ;
    for (int i = 0; i < SHOP_SLOT_TOOL_NUM; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_COUNT, &unitX, &unitZ)) {
            putItem(shop->getItem(i + SHOP_SLOT_TOOL), unitX, unitZ);
        }
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_MEDICINE, &unitX, &unitZ)) {
        putItem(shop->getItem(SHOP_SLOT_MEDICINE), unitX, unitZ);
    }
    for (int i = 0; i < SHOP_SLOT_SEED_NUM; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_SEED, &unitX, &unitZ)) {
            putItem(shop->getItem(i + SHOP_SLOT_SEED), unitX, unitZ);
        }
    }
    for (int i = 0; i < SHOP_SLOT_SAPLING_NUM; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_SEEDLING, &unitX, &unitZ)) {
            putItem(shop->getItem(i + SHOP_SLOT_SAPLING), unitX, unitZ);
        }
    }
    for (int i = 0; i < SHOP_SLOT_FTR_NUM; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_FTR, &unitX, &unitZ)) {
            putItem(shop->getItem(i + SHOP_SLOT_FTR), unitX, unitZ);
        }
    }
    for (int i = 0; i < SHOP_SLOT_WALL_NUM; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_WALL, &unitX, &unitZ)) {
            putItem(shop->getItem(i + SHOP_SLOT_WALL), unitX, unitZ);
        }
    }
    for (int i = 0; i < SHOP_SLOT_CARPET_NUM; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_CARPET, &unitX, &unitZ)) {
            putItem(shop->getItem(i + SHOP_SLOT_CARPET), unitX, unitZ);
        }
    }
    for (int i = 0; i < SHOP_SLOT_PAPER_NUM; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_PAPER, &unitX, &unitZ)) {
            putItem(shop->getItem(i + SHOP_SLOT_PAPER), unitX, unitZ);
        }
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_PAINT, &unitX, &unitZ)) {
        putItem(shop->getItem(SHOP_SLOT_PAINT), unitX, unitZ);
    }
}

// .text 0x630
void dShopFg_c::putBrokerItems() {
    dSaveShopGallery_c *shop = &dSaveData_c::getTown()->mShops.mShopGallery;
    int unitX, unitZ;
    for (int i = 0; i < 3; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_FTR, &unitX, &unitZ)) {
            putItem(shop->getItem(i), unitX, unitZ);
        }
    }
}

// .text 0x6C0
void dShopFg_c::putTailorItems() {
    dSaveShopTailor_c *shop = &dSaveData_c::getTown()->mShops.mShopTailor;
    int unitX, unitZ;
    if (dShopLayout_findFreeUnit(dItem::KIND_COUNT, &unitX, &unitZ)) {
        putItem(shop->getItem(0), unitX, unitZ);
    }
    for (int i = 0; i < 3; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_CLOTH, &unitX, &unitZ)) {
            putItem(shop->getItem(i + 1), unitX, unitZ);
        }
    }
    for (int i = 0; i < 8; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_TA_CLOTH, &unitX, &unitZ)) {
            putItem(shop->getItem(i + 4), unitX, unitZ);
        }
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_CAP, &unitX, &unitZ)) {
        putItem(shop->getItem(12), unitX, unitZ);
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_ACC, &unitX, &unitZ)) {
        putItem(shop->getItem(13), unitX, unitZ);
    }
}

// .text 0x850
void dShopFg_c::putGraceItems() {
    dSaveShopGrace_c *shop = &dSaveData_c::getTown()->mShops.mShopGrace;
    int unitX, unitZ;
    for (int i = 0; i < 10; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_FTR, &unitX, &unitZ)) {
            putItem(shop->getItem(i, TRUE), unitX, unitZ);
        }
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_WALL, &unitX, &unitZ)) {
        putItem(shop->getItem(10, TRUE), unitX, unitZ);
    }
    for (int i = 0; i < 3; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_CARPET, &unitX, &unitZ)) {
            putItem(shop->getItem(i + 11, TRUE), unitX, unitZ);
        }
    }
    for (int i = 0; i < 6; i++) {
        if (dShopLayout_findFreeUnit(dItem::KIND_CLOTH, &unitX, &unitZ)) {
            putItem(shop->getItem(i + 12, TRUE), unitX, unitZ);
        }
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_CAP, &unitX, &unitZ)) {
        putItem(shop->getItem(18, TRUE), unitX, unitZ);
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_ACC, &unitX, &unitZ)) {
        putItem(shop->getItem(19, TRUE), unitX, unitZ);
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_COUNT, &unitX, &unitZ)) {
        putItem(shop->getItem(20, TRUE), unitX, unitZ);
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_ORG_CLOTH, &unitX, &unitZ)) {
        putItem(shop->getItem(21, TRUE), unitX, unitZ);
    }
    if (dShopLayout_findFreeUnit(dItem::KIND_ORG_CAP, &unitX, &unitZ)) {
        putItem(shop->getItem(22, TRUE), unitX, unitZ);
    }
}

// .text 0xB04
void dShopFg_c::putBarberItems() {
}

// .text 0xB08
void dShopFg_c::putCafeItems() {
}
