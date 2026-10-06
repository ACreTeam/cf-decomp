#pragma once

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item.hpp>

namespace EGG {
class Heap;
}

#define DL_ITEM_SLOT_NUM 0x100 // = dItem::DL_ITEM_COUNT

// One downloaded item in the save (0x2000 bytes): its item definition (a BITM whose
// m_addItem is the slot), an archive at +0x18C (the model, decompressed by loadArchive) and a
// checksum over the rest at +0x1FFC. Source: src/dol/game/d_save_dl_item.cpp
// (.text 80115380..80115CDC). Names are inferred.
class dSaveDLItem_c {
public:
    dSaveDLItem_c();                                  // 80115380: zeroes it

    dItem::Item getItem();                            // 801153B8: ITEM_ID_NONE if not an item
    const dItem::BITM *getValidBITM() const;                      // 80115420: NULL without the BITM magic
    void *getArchive();                               // 80115464: NULL unless its state is 2
    u32 getArchiveSize();                             // 801154A8
    void *loadArchive(EGG::Heap *heap);               // 801154DC: decompressed into a block of heap
    const dItem::BITM *getBITM() const;                           // 80115460
    BOOL isBITM() const;                                    // 80115588: the 'BITM' magic
    BOOL isUsed();                                    // 801155C0: a BITM with a base id below 0x1000
    BOOL isGood();                                    // 80115624: checksum, isUsed and BITM::isValid
    u32 getChecksum();                                // 8011569C
    u32 calcChecksum();                               // 801156A4
    void copy(const dSaveDLItem_c *src);              // 801156AC

    union {
        dItem::BITM mBITM;
        u8 mData[0x1FFC];
    };
    /* 0x1FFC */ u32 mChecksum;
}; // size 0x2000

// All downloaded items of the save (dSaveData_c::mDLItems, save + 0x20F320): a CRC32 over the
// slots and the DL_ITEM_SLOT_NUM slots (slot i holds item index dItem::DL_ITEM_FIRST + i).
class dSaveDLItemList_c {
public:
    // getAt / find come in two copies (const and not); the const ones are used on get(), the
    // others on getRaw().
    dSaveDLItemList_c();                                          // 801156B4
    void updateChecksum();                                        // 80115718
    int getNum();                                                 // 80115748: used slots
    dItem::Item add(const dSaveDLItem_c *item, int arg);          // 801157B4
    dSaveDLItem_c *find(dItem::Item item) const;                  // 80115910
    dSaveDLItem_c *find(dItem::Item item);                        // 801159B8
    s32 getSlot(dItem::Item *item);                               // 80115A60: -1 if none
    dSaveDLItem_c *getAt(u32 slot) const;                         // 80115B10
    dSaveDLItem_c *getAt(u32 slot);                               // 80115B30
    BOOL isSlotTaken(const dSaveDLItem_c *item);                  // 80115B50
    dItem::Item getItemAt(u32 slot);                              // 80115BC0
    u32 calcChecksum();                                           // 80115BE8
    BOOL isChecksumOK();                                          // 80115C00

    static dSaveDLItemList_c *get();                              // 80115CA4: dSaveData_c::getDLData()
    static dSaveDLItemList_c *getRaw();                           // 80115CA8

    /* 0x000000 */ u32 mChecksum;
    /* 0x000004 */ dSaveDLItem_c mItems[DL_ITEM_SLOT_NUM];
    /* 0x200004 */ u8 _200004[0x1C];
}; // size 0x200020

namespace dItem {

// The town's item rarity (dSaveData_c::mItemRarity, save + 0x735AF): for each of 8 item
// categories, which of the 6 orders of 60 / 30 / 10 percent applies to the item groups A, B and C
// (FROM_GROUP_A..C), i.e. which group is common, uncommon and rare in this town. Rolled once when
// the town is made (fn_80155214); read by the random item picker (fn_800C60B4, fn_800C6EB8,
// fn_800C6F6C, with the 6 x 3 rate table 80472DE0 of that TU). The name is ours. Its two
// functions are compiled in this TU in the target (between dSaveDLItemList_c::isChecksumOK and
// get, before the __sinit), though nothing else here uses it.
class dSaveItemRarity_c {
public:
    void randomize();         // 80115C3C
    u8 get(int category);     // 80115C98: 0..5 (callers take it mod 6)

    /* 0x0 */ u8 mOrder[8];
}; // size 0x8

} // namespace dItem
