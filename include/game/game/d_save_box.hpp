#pragma once

#include <types.h>
#include <game/game/d_dsn.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_mail.hpp>

// Per-player storage boxes in dSaveExtra_c. Source: src/dol/game/d_save_box.cpp
// (.text 80112BB8..80113158). Slots are addressed as (page, slot). Names are inferred.

#define SAVE_MAILBOX_PAGE_SLOTS 10
#define SAVE_MAILBOX_NUM 160
#define SAVE_DESIGNBOX_PAGE_SLOTS 4
#define SAVE_DESIGNBOX_NUM 64

// One item id per stored-letter slot, same paging as dSaveMailBox_c (ctor/dtor fn_8010EE70 /
// fn_8010EE74, both empty). Read by the letter menu REL for the inventory's letter slots.
struct dSaveItemBox_c {
    dSaveItemBox_c() {}
    ~dSaveItemBox_c() {}

    void clear();                                          // 80112BB8: all ITEM_ID_NONE
    u16 set(int page, int slot, const dItem::Item *item);  // 80112C74: returns the old id
    u16 get(int page, int slot);                           // 80112C94

    /* 0x000 */ u16 mItems[SAVE_MAILBOX_NUM];
}; // size 0x140

// Stored letters (ctor fn_8010EDC4, dtor fn_8010EE0C).
struct dSaveMailBox_c {
    void clear();                                        // 80112CA8
    void set(int page, int slot, const dMail_c *mail);   // 80112CF4
    dMail_c *get(int page, int slot);                    // 80112D10

    /* 0x0000 */ dMail_c mMails[SAVE_MAILBOX_NUM];
}; // size 0x23A00

// Stored designs with a used bit per slot (ctor fn_8010ED28).
struct dSaveDesignBox_c {
    void init();                                           // 80112D24: default designs, none used
    void remove(int page, int slot);                       // 80112D8C
    void set(int page, int slot, const dDesign_c *design); // 80112DF0
    dDesign_c *get(int page, int slot);                    // 8011305C: NULL when the slot is unused
    void setUsed(int page, int slot);                      // 801130C0
    void clearUsed(int page, int slot);                    // 801130F0
    BOOL isUsed(int page, int slot);                       // 80113120

    /* 0x00000 */ dDesign_c mDesigns[SAVE_DESIGNBOX_NUM];
    /* 0x22000 */ u32 mUsed[2];
}; // size 0x22020 (alignment 32)
