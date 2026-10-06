// Per-player storage boxes in dSaveExtra_c: letter items, letters and designs.
// .text 80112BB8..80113158.
#include <game/game/d_save_box.hpp>
#include <revolution/OS/OSCache.h>

// 80112BB8
void dSaveItemBox_c::clear() {
    for (int i = 0; i < SAVE_MAILBOX_NUM; i++) {
        mItems[i] = dItem::ITEM_ID_NONE;
    }
}

// 80112C74
u16 dSaveItemBox_c::set(int page, int slot, const dItem::Item *item) {
    u16 old = mItems[slot + page * SAVE_MAILBOX_PAGE_SLOTS];
    mItems[slot + page * SAVE_MAILBOX_PAGE_SLOTS] = item->mId;
    return old;
}

// 80112C94
u16 dSaveItemBox_c::get(int page, int slot) {
    return mItems[slot + page * SAVE_MAILBOX_PAGE_SLOTS];
}

// 80112CA8
void dSaveMailBox_c::clear() {
    for (int i = 0; i < SAVE_MAILBOX_NUM; i++) {
        mMails[i].clear();
    }
}

// 80112CF4
void dSaveMailBox_c::set(int page, int slot, const dMail_c *mail) {
    mMails[slot + page * SAVE_MAILBOX_PAGE_SLOTS].copy(mail);
}

// 80112D10
dMail_c *dSaveMailBox_c::get(int page, int slot) {
    return &mMails[slot + page * SAVE_MAILBOX_PAGE_SLOTS];
}

// 80112D24
void dSaveDesignBox_c::init() {
    for (int i = 0; i < SAVE_DESIGNBOX_NUM; i++) {
        mDesigns[i].initDefault();
    }
    mUsed[0] = 0;
    mUsed[1] = 0;
}

// 80112D8C
void dSaveDesignBox_c::remove(int page, int slot) {
    mDesigns[slot + page * SAVE_DESIGNBOX_PAGE_SLOTS].clear();
    clearUsed(page, slot);
}

// 80112DF0
void dSaveDesignBox_c::set(int page, int slot, const dDesign_c *design) {
    mDesigns[slot + page * SAVE_DESIGNBOX_PAGE_SLOTS] = *design;
    DCStoreRangeNoSync(&mDesigns[slot + page * SAVE_DESIGNBOX_PAGE_SLOTS], sizeof(dDesign_c));
    setUsed(page, slot);
}

// 8011305C
// "== FALSE" (not "!isUsed()") keeps the NULL path first.
dDesign_c *dSaveDesignBox_c::get(int page, int slot) {
    if (isUsed(page, slot) == FALSE) {
        return NULL;
    }
    return &mDesigns[slot + page * SAVE_DESIGNBOX_PAGE_SLOTS];
}

// 801130C0
void dSaveDesignBox_c::setUsed(int page, int slot) {
    u32 idx = slot + page * SAVE_DESIGNBOX_PAGE_SLOTS;
    u32 *word = &mUsed[idx >> 5];
    *word |= 1 << (idx & 31);
}

// 801130F0
void dSaveDesignBox_c::clearUsed(int page, int slot) {
    u32 idx = slot + page * SAVE_DESIGNBOX_PAGE_SLOTS;
    u32 *word = &mUsed[idx >> 5];
    *word &= ~(1 << (idx & 31));
}

// 80113120
BOOL dSaveDesignBox_c::isUsed(int page, int slot) {
    u32 idx = slot + page * SAVE_DESIGNBOX_PAGE_SLOTS;
    u32 *word = &mUsed[idx >> 5];
    return (*word & (1 << (idx & 31))) != 0;
}
