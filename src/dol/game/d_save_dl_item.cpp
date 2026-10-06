// The downloaded items of the save (dSaveDLItem_c, dSaveDLItemList_c). .text 80115380..80115CDC,
// .ctors 8046571C..80465720, .bss 805ED570..805EF570, .sdata2 80750B60..80750B68.
#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_save_data.hpp>
#include <game/sLib/s_crc.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/core/eggHeap.h>
#include <revolution/OS/OSCache.h>
#include <string.h>

dItem::Item makeItemFromBaseId(u16 baseId); // d_item.cpp

// Not split yet (C linkage keeps the target names).
extern "C" {
int fn_8043F258(const void *archive);            // 8043F258: archive state (2: ready)
u32 fn_8043F2DC(const void *archive);            // 8043F2DC: decompressed size
void fn_8043F020(const void *archive, void *dst); // 8043F020: decompress
u32 fn_800C5278(const void *data, u32 size);     // 800C5278: CRC32 with the item seed
}

// 805ED570: a 4-byte aligned copy for add().
static dSaveDLItem_c sAlignedItem;

// 80115380
dSaveDLItem_c::dSaveDLItem_c() {
    memset(this, 0, sizeof(dSaveDLItem_c));
}

// 801153B8
dItem::Item dSaveDLItem_c::getItem() {
    const dItem::BITM *bitm = getValidBITM();
    if (bitm != NULL && (u16)bitm->m_baseId <= 0xFFF) {
        dItem::Item item = makeItemFromBaseId(bitm->m_baseId);
        return item;
    }
    return dItem::Item();
}

// 80115420
const dItem::BITM *dSaveDLItem_c::getValidBITM() const {
    if (isBITM()) {
        return &mBITM;
    }
    return NULL;
}

// 80115460
const dItem::BITM *dSaveDLItem_c::getBITM() const {
    return &mBITM;
}

// 80115464
void *dSaveDLItem_c::getArchive() {
    void *archive = mData + sizeof(dItem::BITM);
    if (fn_8043F258(archive) == 2) {
        return archive;
    }
    return NULL;
}

// 801154A8
u32 dSaveDLItem_c::getArchiveSize() {
    void *archive = getArchive();
    if (archive != NULL) {
        return fn_8043F2DC(archive);
    }
    return 0;
}

// 801154DC
void *dSaveDLItem_c::loadArchive(EGG::Heap *heap) {
    void *archive = getArchive();
    u32 size = getArchiveSize();
    if (archive != NULL && size != 0) {
        void *data = heap->alloc(size, 0x20);
        if (data != NULL) {
            fn_8043F020(archive, data);
            DCFlushRange(data, size);
            return data;
        }
    }
    return NULL;
}

// 80115588
BOOL dSaveDLItem_c::isBITM() const {
    return getBITM()->m_magic == 'BITM';
}

// 801155C0
BOOL dSaveDLItem_c::isUsed() {
    if (isBITM()) {
        int baseId = getBITM()->m_baseId;
        return (u16)baseId <= 0xFFF;
    }
    return FALSE;
}

// 80115624
BOOL dSaveDLItem_c::isGood() {
    u32 checksum = getChecksum();
    if (checksum == calcChecksum() && isUsed() && getBITM()->isValid()) {
        return TRUE;
    }
    return FALSE;
}

// 8011569C
u32 dSaveDLItem_c::getChecksum() {
    return mChecksum;
}

// 801156A4
u32 dSaveDLItem_c::calcChecksum() {
    return fn_800C5278(this, offsetof(dSaveDLItem_c, mChecksum));
}

// 801156AC
void dSaveDLItem_c::copy(const dSaveDLItem_c *src) {
    memcpy(this, src, sizeof(dSaveDLItem_c));
}

// 801156B4
dSaveDLItemList_c::dSaveDLItemList_c() {
    updateChecksum();
}

// 80115718
void dSaveDLItemList_c::updateChecksum() {
    mChecksum = calcChecksum();
}

// 80115748
int dSaveDLItemList_c::getNum() {
    int num = 0;
    for (dSaveDLItem_c *item = mItems; item < mItems + DL_ITEM_SLOT_NUM; item++) {
        if (item->isUsed()) {
            num++;
        }
    }
    return num;
}

// 801157B4: stores a received item in its slot (unless the slot already holds it) and returns
// its item, or ITEM_ID_NONE when it isn't a good downloaded item.
dItem::Item dSaveDLItemList_c::add(const dSaveDLItem_c *item, int arg) {
    const dItem::BITM *bitm;
    u16 baseId;
    dSaveDLItem_c *src;
    src = (dSaveDLItem_c *)item;
    if ((u32)item & 3) {
        sAlignedItem.copy(item);
        src = &sAlignedItem;
    }
    if (!src->isUsed()) {
        return dItem::Item();
    }
    if (!src->isGood()) {
        return dItem::Item();
    }
    bitm = src->getBITM();
    int rawId = bitm->m_baseId;
    baseId = rawId;
    if (!bitm->isValid()) {
        return dItem::Item();
    }
    u8 slot = bitm->m_addItem;
    if (dItem::infoBank_c::get()->isBuiltinBaseId(baseId)) {
        dItem::Item builtin = makeItemFromBaseId(baseId);
        return builtin;
    }
    if (!isSlotTaken(src)) {
        mItems[slot].copy(item);
        dItem::infoBank_c::get()->setDlItem(baseId, slot);
    }
    if (baseId <= 0xFFF) {
        dItem::Item added = makeItemFromBaseId(baseId);
        return added;
    }
    return dItem::Item();
}


// 80115910
dSaveDLItem_c *dSaveDLItemList_c::find(dItem::Item item) const {
    if (dItem::isRealItemId(item.mId)) {
        for (const dSaveDLItem_c *slot = mItems; slot < mItems + DL_ITEM_SLOT_NUM; slot++) {
            if (item.isSame(((dSaveDLItem_c *)slot)->getItem())) {
                return (dSaveDLItem_c *)slot;
            }
        }
    }
    return NULL;
}

// 801159B8
dSaveDLItem_c *dSaveDLItemList_c::find(dItem::Item item) {
    if (dItem::isRealItemId(item.mId)) {
        for (dSaveDLItem_c *slot = mItems; slot < mItems + DL_ITEM_SLOT_NUM; slot++) {
            if (item.isSame(slot->getItem())) {
                return slot;
            }
        }
    }
    return NULL;
}

// 80115A60
s32 dSaveDLItemList_c::getSlot(dItem::Item *item) {
    if (dItem::isRealItemId(item->mId)) {
        for (dSaveDLItem_c *slot = mItems; slot < mItems + DL_ITEM_SLOT_NUM; slot++) {
            if (item->isSame(slot->getItem())) {
                return slot->getBITM()->m_addItem;
            }
        }
    }
    return -1;
}

// 80115B10
dSaveDLItem_c *dSaveDLItemList_c::getAt(u32 slot) const {
    if (slot < DL_ITEM_SLOT_NUM) {
        return (dSaveDLItem_c *)&mItems[slot];
    }
    return NULL;
}

// 80115B30
dSaveDLItem_c *dSaveDLItemList_c::getAt(u32 slot) {
    if (slot < DL_ITEM_SLOT_NUM) {
        return &mItems[slot];
    }
    return NULL;
}

// 80115B50
BOOL dSaveDLItemList_c::isSlotTaken(const dSaveDLItem_c *item) {
    const dItem::BITM *bitm = ((dSaveDLItem_c *)item)->getValidBITM();
    if (bitm != NULL) {
        return getItemAt(bitm->m_addItem).mId != dItem::ITEM_ID_NONE;
    }
    return TRUE;
}

// 80115BC0
dItem::Item dSaveDLItemList_c::getItemAt(u32 slot) {
    if (slot < DL_ITEM_SLOT_NUM) {
        return mItems[slot].getItem();
    }
    return dItem::Item();
}

// 80115BE8
u32 dSaveDLItemList_c::calcChecksum() {
    return sCrc::calcCRC32(mItems, sizeof(mItems), 0x04201018, -1);
}

// 80115C00
BOOL dSaveDLItemList_c::isChecksumOK() {
    return calcChecksum() == mChecksum;
}

// 80115C3C
void dItem::dSaveItemRarity_c::randomize() {
    for (u32 i = 0; i < 8; i++) {
        mOrder[i] = cM::rndF(6.0f);
    }
}

// 80115C98
u8 dItem::dSaveItemRarity_c::get(int category) {
    return mOrder[category & 7];
}

// 80115CA4
dSaveDLItemList_c *dSaveDLItemList_c::get() {
    return dSaveData_c::getDLData();
}

// 80115CA8
dSaveDLItemList_c *dSaveDLItemList_c::getRaw() {
    return &dSaveData_c::getRaw()->mDLItems;
}
