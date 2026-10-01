#pragma once

#include <types.h>

// The 2-byte item value used everywhere an item can sit: field-object ids
// below 0xE5 (fg_treeA_x, fg_apple, ... in the 804E3C40 table) and encoded
// BITM ids (categories 9..12 in bits 12-15, 0x9000 + (baseId << 2), with
// the low 2 bits as a variant).
//
// The class has no virtuals, so there is no RTTI and its real name is
// unknown. Its out-of-line members live in this TU (800A5D3C..800A70AC).
// The user-declared destructor makes MWCC return it through a hidden pointer.
namespace dItem {

enum {
    ITEM_ID_NONE = 0xFFF1,

    // Field objects index the fg info table directly.
    FG_COUNT = 0xE5,
};

inline bool isEncodedId(u16 id) {
    int category = (id >> 12) & 0xF;
    return category >= 9 && category <= 12;
}

// One field object (804E3C40 table, 0x20 bytes each).
struct FgInfo {
    u16 _00; // 0x00
    u16 mDropItem; // 0x02: item a grown tree drops (or ITEM_ID_NONE)
    u16 _04; // 0x04
    s16 mPlantItem; // 0x06: item that plants this object (or ITEM_ID_NONE)
    u8 _08[5]; // 0x08
    s8 mTreeStage; // 0x0D: growth stage, negative for non-trees
    u8 _0E; // 0x0E
    char mName[17]; // 0x0F: model resource name
}; // sizeof = 0x20

struct Item {
    Item() : mId(ITEM_ID_NONE) {}
    Item(u16 id) : mId(id) {}
    // From an item index (not an id); leaves mId to setFromIndex.
    explicit Item(int index); // 800A5D3C
    Item(int base, int offset, BOOL skipCheck); // 800A5D6C
    ~Item() {}

    void setFromIndex(int index); // 800A5D9C
    void setFromIndex(int base, int offset, BOOL skipCheck); // 800A5E08

    // Same item, ignoring the 2 variant bits of encoded ids.
    BOOL isSame(const Item &other) const; // 800A5E64

    bool isFg() {
        return mId < FG_COUNT;
    }
    int getFgIndex() {
        return isFg() ? mId & 0xFFF : -1;
    }
    FgInfo *getFgInfo(); // 800A5EF0

    BOOL isMoney(); // 800A5F20
    BOOL isKabu(); // 800A5F48
    int getPrice() const; // 800A5F74
    // Price after the current shop discount.
    int getShopPrice(); // 800A60F8
    // Money item closest to `amount` (rounding up or down); the leftover
    // goes to `remainder`.
    static u16 getMoneyItem(int amount, BOOL roundUp, int *remainder); // 800A622C

    // Ids 0xD000..0xD044 index a table owned by the TU at 80167FB4.
    bool isExtId() const {
        return mId >= 0xD000 && mId < 0xD045;
    }
    int getExtIndex() const {
        return isExtId() ? mId & 0xFFF : -1;
    }
    f32 getExtValueA() const; // 800A6370
    f32 getExtValueB() const; // 800A6404
    BOOL getExtFlag() const; // 800A6498

    BOOL isOrgDesign() const; // 800A651C
    // BITM kind, or KIND_NONE for unencoded ids or missing entries.
    int getKind() const; // 800A658C
    int getFrom() const; // 800A6608
    int getFashion() const; // 800A6684
    int getStyle() const; // 800A6700
    int getClothStyle(); // 800A677C: tail-calls getStyle
    int getColorA() const; // 800A6780
    int getColorB() const; // 800A680C
    int getPartA() const; // 800A689C
    int getHideBone() const; // 800A6918
    int getSeries(); // 800A6998
    int getSeriesGroup(); // 800A69F0
    BOOL hasNoFtrFunc() const; // 800A6A34
    BOOL hasFtrFunc() const; // 800A6A8C
    BOOL isUsable() const; // 800A6B14

    BOOL isFlower() const; // 800A6B5C
    BOOL isWiltedFlower(); // 800A6B80
    BOOL isTree(); // 800A6BA4
    BOOL isFruitTree(); // 800A6BE4
    BOOL isFg74(); // 800A6C08
    BOOL isInsect(); // 800A6C2C
    BOOL isFish(); // 800A6C58
    BOOL isDust(); // 800A6C84
    BOOL isMushroom(); // 800A6CB0
    BOOL isShell(); // 800A6CDC
    BOOL isAnyFlower(); // 800A6D08
    BOOL is7000(); // 800A6D5C
    int get7000Index(); // 800A6D7C
    // Field object planted by this item, or the id itself if none.
    u16 getPlantedFg(); // 800A6D88
    int getFruitTreeType(); // 800A6E00
    // Item given when this object is picked up.
    Item getPickItem() const; // 800A6F20

    Item getVariant(int variant); // 800A7094: tail-calls withVariant
    // Copy with the variant bits replaced by `variant & 3`.
    Item withVariant(int variant) const; // 800A7098

    u16 mId;
};

// Never inlined (they return an Item); d_fg_item's copies are the linked ones.
inline Item Item::getVariant(int variant) {
    return withVariant(variant);
}

inline Item Item::withVariant(int variant) const {
    return Item(static_cast<u16>((variant & 3) | (mId & 0xFFFC)));
}


} // namespace dItem
