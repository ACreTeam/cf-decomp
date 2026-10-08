#pragma once

#include <types.h>
#include <game/game/d_item.hpp>

#define CATALOG_BIT_NUM 0x1000 // indexed by item base ID

// 0x200; ctor/clear fn_80139D2C (memset 0). Lives at dPrivateData_c+0x83FA.
// Bit for base ID n is mBits[n >> 3] & (1 << (n & 7)).
class dCatalog_c {
public:
    void clear();                                                // 80139D2C
    dCatalog_c() { init(); }
    void init();                                                 // 80139D38
    void registerDefaults(int set);                              // 80139D3C; 0x88D, 0x8BE, then 3 items from fn_8013CF7C unless set == -1
    static u16 getNth(int kind, u32 n, dItem::seeker_c::candCB_c *cb); // 80139E04; kind 5 walks lbl_80476238
    u16 getNthRegistered(int kind, u32 n);                       // 80139ED8
    u16 getNthAny(int kind, u32 n);                              // 80139F20
    BOOL isNthRegistered(int kind, u32 n);                       // 80139F68
    bool isRegistered(u16 item);                                 // 80139FA4; also requires BITM byte 0x187 bit 1 (catalogable)
    BOOL isRegistered(const dItem::Item &item);                  // 8013A03C
    BOOL registerItem(u16 item, BOOL force);                     // 8013A044; unless force, skips BITM kinds 7 and 8
    u32 countRegistered(int kind);                               // 8013A10C; kind 5 also counts kind 6
    u32 countAll(int kind);                                      // 8013A1B4
    BOOL isComplete(int kind);                                   // 8013A25C

    u16 getBit(u16 idx) { return (mBits[(idx >> 3) & 0x1FF] >> (idx & 7)) & 1; }
    void setBit(u16 idx) { mBits[(idx >> 3) & 0x1FF] |= 1 << (idx & 7); }

    u8 mBits[CATALOG_BIT_NUM / 8];
};

// RTTI name "catalogCandCB_c", vtable lbl_804EF22C, check fn_8013CAE8.
// Built on the stack by fn_80139ED8 (mOnlyRegistered = 1) and fn_80139F20 (= 0).
class catalogCandCB_c : public dItem::seeker_c::candCB_c {
public:
    catalogCandCB_c(dCatalog_c *catalog, BOOL onlyRegistered) : mpCatalog(catalog), mOnlyRegistered(onlyRegistered) {}

    // Fails if mpCatalog is NULL; passes everything if !mOnlyRegistered,
    // otherwise only items registered in mpCatalog.
    virtual BOOL check(const dItem::BITM *bitm, dItem::Item *item) const; // 8013CAE8

    dCatalog_c *mpCatalog; // 0x04
    BOOL mOnlyRegistered;  // 0x08
};
