#pragma once

// d_shop_fg (REL d_shop_fgNP, profile SHOP_FG): puts a shop's stock on the
// floor of its room when the room is entered (dShopFg_c, its RTTI name). By the
// current scene it picks the save's shop (Nook's store by stage, Redd's
// gallery, the tailor, Gracie), puts each stock slot's item on the next free
// display unit of the room's layout (dShopLayout_findFreeUnit) and deletes
// itself on its first execute. Member names other than the fBase_c overrides
// are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_fg_item.hpp>

class dShopFg_c : public fBase_c {
  public:
    virtual int create();  // 0xA8
    virtual int execute(); // 0x2CC

    void putItem(const dItem::Item &item, int unitX,
                 int unitZ); // 0x2F0: nothing for ITEM_ID_NONE
    void putShopItems();     // 0x360: Nook's store
    void putBrokerItems();   // 0x630: Redd's (dSaveShopGallery_c)
    void putTailorItems();   // 0x6C0: the Able Sisters
    void putGraceItems();    // 0x850: Gracie's
    void putBarberItems();   // 0xB04: Shampoodle (nothing)
    void putCafeItems();     // 0xB08: The Roost (nothing)
}; // size 0x64
