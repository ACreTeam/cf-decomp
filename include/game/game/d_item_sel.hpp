#pragma once

#include <game/game/d_fg_item.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_save_data.hpp>
#include <types.h>

class dCatalog_c;

namespace dItemSel {

// RTTI supplies unqRnd_c's original name. Other names are inferred.
class unqRnd_c {
  public:
    unqRnd_c();                  // 800C6B58
    virtual u32 getRnd(u32 max); // 800C5C30
}; // size 0x4

struct Range_c {
    Range_c(int kind, int from) : mKind(kind), mFrom(from) {
    }
    int getKind() const {
        return mKind;
    }
    int getFrom() const {
        return mFrom;
    }
    /* 0x0 */ int mKind;
    /* 0x4 */ int mFrom;
}; // size 0x8

enum FilterFlags {
    EXCLUDE_NOT_FOR_SALE = 1 << 0,
    ONLY_REGISTERED = 1 << 1,
    KEEP_FROM = 1 << 2,
    REVERSE_RARITY = 1 << 3,
    UNIQUE_FTR_PART = 1 << 4,
    NO_FALLBACK = 1 << 5,
};

class Filter_c {
  public:
    Filter_c(dCatalog_c *catalog = NULL, unqRnd_c *rnd = NULL, u32 flags = 0)
        : mpCatalog(catalog), mpRnd(rnd), mFlags(flags) {
    }
    void setCatalog(dCatalog_c *catalog); // 800C5C6C
    /* 0x0 */ dCatalog_c *mpCatalog;
    /* 0x4 */ unqRnd_c *mpRnd;
    /* 0x8 */ u32 mFlags;
}; // size 0xC (target .bss includes four bytes of alignment padding)

extern Filter_c sDefaultFilter; // 8059FF80
extern int sCandidateKind;      // 8074A9B8

u32 searchCandidates(int kind, int from, const dItem::Item *items, int num,
                     const dItem::Item *exclude, int excludeNum,
                     const Filter_c *filter,
                     int season);       // 800C5F50
u16 getCandidate(u32 index);            // 800C5FC0
int getClothSeason(u8 month, u8 day); // 800C6024
BOOL select(dItem::Item *out, u32 num, const Range_c *ranges, u32 rangeNum,
            const Filter_c *filter, const dItem::Item *exclude, int excludeNum,
            int *selectedFrom); // 800C60B4
int resolveFrom(int kind, int from, unqRnd_c *rnd, BOOL keepFrom,
                BOOL reverse);          // 800C6B68
u8 getFengShuiBonus();                  // 800C6E54
int getFromRate(int kind, int rate);    // 800C6EB8
int getRarity(const dItem::Item *item); // 800C6F6C
extern "C" int
fn_800C769C(); // 800C769C: unused constant-zero stub, purpose unknown

} // namespace dItemSel
