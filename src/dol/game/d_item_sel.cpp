#include <game/cLib/c_math.hpp>
#include <game/game/d_catalog.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_save_data.hpp>

// RTTI places this callback at global scope.
class myCandCB_c : public dItem::seeker_c::candCB_c {
  public:
    myCandCB_c(int from, const dItem::Item *items, int num,
               const dItem::Item *exclude, int excludeNum,
               const dItemSel::Filter_c *filter, int season) {
        mpFilter = filter;
        mFrom = from;
        mpItems = items;
        mNum = num;
        mpExclude = exclude;
        mExcludeNum = excludeNum;
        mSeason = season;
    }
    virtual BOOL check(const dItem::BITM *bitm,
                       dItem::Item *item) const; // 800C5C74
    /* 0x04 */ const dItemSel::Filter_c *mpFilter;
    /* 0x08 */ const dItem::Item *mpItems;
    /* 0x0C */ int mNum;
    /* 0x10 */ const dItem::Item *mpExclude;
    /* 0x14 */ int mExcludeNum;
    /* 0x18 */ int mFrom;
    /* 0x1C */ int mSeason;
};

namespace dItemSel {
enum {
    RARITY_GROUP_COUNT = dItem::FROM_GROUP_C - dItem::FROM_GROUP_A + 1,
    RARITY_ORDER_COUNT = 6,
};

static const int sRates[RARITY_ORDER_COUNT][RARITY_GROUP_COUNT] = {
    {60, 30, 10}, {60, 10, 30}, {30, 60, 10},
    {10, 60, 30}, {30, 10, 60}, {10, 30, 60},
};
static const int sRandomGroups[RARITY_GROUP_COUNT] = {
    dItem::FROM_GROUP_A,
    dItem::FROM_GROUP_B,
    dItem::FROM_GROUP_C,
};

int sCandidateKind = dItem::KIND_NONE;
Filter_c sDefaultFilter;

// 800C5C30
u32 unqRnd_c::getRnd(u32 max) {
    return cM::rndF(max);
}

// 800C5C6C
void Filter_c::setCatalog(dCatalog_c *catalog) {
    mpCatalog = catalog;
}
} // namespace dItemSel

// 800C5C74
BOOL myCandCB_c::check(const dItem::BITM *bitm, dItem::Item *item) const {
    if (mpFilter != NULL) {
        u32 flags = mpFilter->mFlags;
        int from = (u32)bitm->m_from < dItem::FROM_COUNT ? bitm->m_from
                                                         : dItem::FROM_NONE;
        int kind = bitm->getKind();
        if (kind == dItem::KIND_CLOTH &&
            (from == dItem::FROM_GROUP_A || from == dItem::FROM_GROUP_B ||
             from == dItem::FROM_GROUP_C)) {
            int season = (u32)bitm->m_season < dItem::CLOTH_SEASON_COUNT
                             ? bitm->m_season
                             : (int)dItem::CLOTH_SEASON_ALL;
            if (season != mSeason && season != dItem::CLOTH_SEASON_ALL) {
                return FALSE;
            }
        }
        const dItem::Item *items = mpItems;
        int num = mNum;
        int excludeNum;
        const dItem::Item *itemsPartEnd;
        const dItem::Item *excludePartEnd;
        const dItem::Item *excludeEnd;
        if (items != NULL && num != 0) {
            const dItem::Item *itemsEnd = items + num;
            for (const dItem::Item *p = items; p < itemsEnd; ++p) {
                if (p->isSame(*item)) {
                    return FALSE;
                }
            }
        }
        const dItem::Item *exclude = mpExclude;
        excludeNum = mExcludeNum;
        if (exclude != NULL && excludeNum != 0) {
            excludeEnd = exclude + excludeNum;
            for (const dItem::Item *p = exclude; p < excludeEnd; ++p) {
                if (p->isSame(*item)) {
                    return FALSE;
                }
            }
        }
        if (mFrom != dItem::FROM_COUNT) {
            if (mFrom == dItem::FROM_GROUP_ABC_ALL) {
                if (from != dItem::FROM_GROUP_A &&
                    from != dItem::FROM_GROUP_B &&
                    from != dItem::FROM_GROUP_C) {
                    return FALSE;
                }
            } else if (from != mFrom) {
                return FALSE;
            }
        }
        if ((flags & dItemSel::EXCLUDE_NOT_FOR_SALE) && bitm->isNotForSale()) {
            return FALSE;
        }
        if (flags & dItemSel::UNIQUE_FTR_PART) {
            int part = bitm->getFtrPartA();
            if (part == dItem::FTR_PART_A_NONE) {
                return FALSE;
            }
            if (items != NULL && num != 0) {
                const dItem::Item *p = items;
                itemsPartEnd = items + num;
                for (; p < itemsPartEnd; ++p) {
                    if (part == p->getPartA()) {
                        return FALSE;
                    }
                }
            }
            if (exclude != NULL && excludeNum != 0) {
                const dItem::Item *p = exclude;
                excludePartEnd = exclude + excludeNum;
                for (; p < excludePartEnd; ++p) {
                    if (part == p->getPartA()) {
                        return FALSE;
                    }
                }
            }
        }
        dCatalog_c *catalog = mpFilter->mpCatalog;
        if (catalog != NULL) {
            if (flags & dItemSel::ONLY_REGISTERED) {
                if (!catalog->isRegistered(dItem::Item(*item))) {
                    return FALSE;
                }
            } else {
                if (catalog->isRegistered(dItem::Item(*item))) {
                    return FALSE;
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

namespace dItemSel {
// 800C5F50
u32 searchCandidates(int kind, int from, const dItem::Item *items, int num,
                     const dItem::Item *exclude, int excludeNum,
                     const Filter_c *filter, int season) {
    myCandCB_c cb(from, items, num, exclude, excludeNum, filter, season);
    dItem::seeker_c::get()->search(kind,
                                   dItem::seeker_c::SEARCH_BUILTIN |
                                       dItem::seeker_c::SEARCH_DOWNLOAD,
                                   &cb);
    sCandidateKind = kind;
    return dItem::seeker_c::get()->mCount;
}

// 800C5FC0
u16 getCandidate(u32 index) {
    dItem::Item item = dItem::seeker_c::get()->getNth(index);
    if (sCandidateKind == dItem::KIND_PAPER) {
        return item.withVariant(dItem::PAPER_VARIANT_FULL_PACK).mId;
    }
    return item.mId;
}

// 800C6024
int getClothSeason(u8 month, u8 day) {
    switch (month) {
    case MONTH_JANUARY:
        return SEASON_WINTER;
    case MONTH_FEBRUARY: {
        int season = SEASON_WINTER;
        if (day > 24) {
            season = SEASON_SPRING;
        }
        return season;
    }
    case MONTH_MARCH:
    case MONTH_APRIL:
        return SEASON_SPRING;
    case MONTH_MAY:
        return day > 26 ? SEASON_SUMMER : SEASON_SPRING;
    case MONTH_JUNE:
    case MONTH_JULY:
        return SEASON_SUMMER;
    case MONTH_AUGUST: {
        int season = SEASON_SUMMER;
        if (day > 26) {
            season = SEASON_FALL;
        }
        return season;
    }
    case MONTH_SEPTEMBER:
    case MONTH_OCTOBER:
        return SEASON_FALL;
    case MONTH_NOVEMBER: {
        int season = SEASON_FALL;
        if (day > 26) {
            season = SEASON_WINTER;
        }
        return season;
    }
    default:
        return SEASON_WINTER;
    }
}

// Placeholder for a possible stripped function; its original name and body are
// unknown. The target pools the signed int-to-float conversion constant before
// the 10.0f used by getFengShuiBonus. This unreferenced function establishes
// that order; the linker strips its code.
float unusedIntToFloat(int value) {
    return value;
}

// 800C60B4
BOOL select(dItem::Item *out, u32 num, const Range_c *ranges, u32 rangeNum,
            const Filter_c *filter, const dItem::Item *exclude, int excludeNum,
            int *selectedFrom) {
    unqRnd_c defaultRnd;
    unqRnd_c *rnd = filter->mpRnd != NULL ? filter->mpRnd : &defaultRnd;
    dTime_c time = *dTime_c::getCurrent();
    int season = getClothSeason((u8)time.month, (u8)time.mday);
    for (u32 i = 0; i < num; ++i) {
        out[i] = dItem::ITEM_ID_NONE;
    }
    dSaveData_c::getTown();
    bool result = true;
    u32 i = 0;
    while (i < num) {
        u32 idx = rnd->getRnd(rangeNum);
        int kind = ranges[idx].getKind();
        int from = resolveFrom(kind, ranges[idx].getFrom(), rnd,
                               filter->mFlags & KEEP_FROM,
                               (filter->mFlags & REVERSE_RARITY) != 0);
        if (selectedFrom != NULL) {
            *selectedFrom = from;
        }
        u32 count = searchCandidates(kind, from, out, num, exclude, excludeNum,
                                     filter, season);
        if (count != 0) {
            u16 item = getCandidate(rnd->getRnd(count));
            if (item != dItem::ITEM_ID_NONE) {
                out[i++] = item;
                continue;
            }
        }
        if (!(filter->mFlags & NO_FALLBACK)) {
            count =
                searchCandidates(kind, from, NULL, 0, NULL, 0, filter, season);
            if (count != 0) {
                u16 item = getCandidate(rnd->getRnd(count));
                if (item != dItem::ITEM_ID_NONE) {
                    out[i++] = item;
                    continue;
                }
            }
            Filter_c fallback = *filter;
            fallback.setCatalog(NULL);
            if (filter->mpCatalog != NULL) {
                count = searchCandidates(kind, from, NULL, 0, NULL, 0,
                                         &fallback, season);
                if (count != 0) {
                    u16 item = getCandidate(rnd->getRnd(count));
                    if (item != dItem::ITEM_ID_NONE) {
                        out[i++] = item;
                        continue;
                    }
                }
            }
            count = searchCandidates(kind, dItem::FROM_COUNT, NULL, 0, NULL, 0,
                                     &fallback, season);
            if (count != 0) {
                u16 item = getCandidate(rnd->getRnd(count));
                if (item != dItem::ITEM_ID_NONE) {
                    out[i++] = item;
                }
            }
        } else {
            out[i++] = dItem::ITEM_ID_NONE;
            result = false;
        }
    }
    return result;
}

// 800C6B58
inline unqRnd_c::unqRnd_c() {
}

// 800C6B68
inline int resolveFrom(int kind, int from, unqRnd_c *rnd, BOOL keepFrom,
                       BOOL reverse) {

    if (keepFrom) {
        return from;
    }
    u8 common = 60 - getFengShuiBonus();
    u8 rare = 70 - common;
    if (from == dItem::FROM_GROUP_ABC) {
        unqRnd_c defaultRnd;
        rnd = rnd != NULL ? rnd : &defaultRnd;
        u32 n = rnd->getRnd(100);
        if (n < common) {
            if (reverse) {
                return resolveFrom(kind, dItem::FROM_GROUP_C, rnd, FALSE,
                                   FALSE);
            }
            return resolveFrom(kind, dItem::FROM_GROUP_A, rnd, FALSE, FALSE);
        } else if (n < common + rare) {
            if (reverse) {
                return resolveFrom(kind, dItem::FROM_GROUP_A, rnd, FALSE,
                                   FALSE);
            }
            return resolveFrom(kind, dItem::FROM_GROUP_C, rnd, FALSE, FALSE);
        } else {
            return resolveFrom(kind, dItem::FROM_GROUP_B, rnd, FALSE, FALSE);
        }
    } else if (from == dItem::FROM_GROUP_ABC_AVERAGE) {
        unqRnd_c defaultRnd;
        rnd = rnd != NULL ? rnd : &defaultRnd;
        return resolveFrom(kind,
                           sRandomGroups[rnd->getRnd(99) % RARITY_GROUP_COUNT],
                           rnd, FALSE, FALSE);
    } else if (from == dItem::FROM_GROUP_A) {
        if (common >= 30 && common >= rare) {
            return getFromRate(kind, 60);
        }
        if (common <= 30 && rare <= 30) {
            return getFromRate(kind, 30);
        }
        return getFromRate(kind, 10);
    } else if (from == dItem::FROM_GROUP_B) {
        if ((common <= 30 && common >= rare) ||
            (common >= 30 && common <= rare)) {
            return getFromRate(kind, 60);
        }
        if ((common >= 30 && rare <= 30) || (common <= 30 && rare >= 30)) {
            return getFromRate(kind, 30);
        }
        return getFromRate(kind, 10);
    } else if (from == dItem::FROM_GROUP_C) {
        if (common <= 30 && common <= rare) {
            return getFromRate(kind, 60);
        }
        if (common >= 30 && rare >= 30) {
            return getFromRate(kind, 30);
        }
        return getFromRate(kind, 10);
    }
    return from;
}

// 800C6E54
inline u8 getFengShuiBonus() {
    const u8 *rate = dHR::getRate(dHR::RATE_CURRENT_HOME);
    float sum = rate[dHR::FENG_SHUI_RED] + rate[dHR::FENG_SHUI_GREEN];
    int bonus = sum / 10.0f;
    return (u8)bonus;
}

// 800C6EB8
inline int getFromRate(int kind, int rate) {

    u32 order = dSaveData_c::getTown()->mItemRarity.get(kind);
    if (order >= RARITY_ORDER_COUNT) {
        order %= RARITY_ORDER_COUNT;
    }
    const int *rates = sRates[order];
    for (int i = 0; i < RARITY_GROUP_COUNT; ++i) {
        if (rate == rates[i]) {
            return i + dItem::FROM_GROUP_A;
        }
    }
    return dItem::FROM_GROUP_A;
}

inline int getRarityGroup(u32 index) {
    static const int groups[RARITY_GROUP_COUNT] = {
        dItem::FROM_GROUP_A,
        dItem::FROM_GROUP_B,
        dItem::FROM_GROUP_C,
    };
    return groups[index];
}

// 800C6F6C
int getRarity(const dItem::Item *item) {
    const dItem::BITM *bitm =
        dItem::infoBank_c::get()->getBITM(dItem::Item(*item));
    if (bitm != NULL) {
        int kind = bitm->getKind();
        int from = dItem::clampField(bitm->m_from, dItem::FROM_COUNT,
                                     dItem::FROM_NONE);
        for (u32 i = 0; i < RARITY_GROUP_COUNT; ++i) {
            if (from ==
                resolveFrom(kind, getRarityGroup(i), NULL, FALSE, FALSE)) {
                return getRarityGroup(i);
            }
        }
        return from;
    }
    return dItem::FROM_NOUSE;
}

// 800C769C: unused stub; no callers to establish its purpose.
int fn_800C769C() {
    return 0;
}
} // namespace dItemSel
