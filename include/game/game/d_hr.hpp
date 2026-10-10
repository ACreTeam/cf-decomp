#pragma once

// d_hr: the house rating (Happy Room Academy). TU src/dol/game/d_hr.cpp (.text 800AC260..800B4DA0).
// Class names come from the RTTI (dHR::fdCb_c, dHR::fdMdlCb_c, dHR::fdPlCb_c, dHR::fdNpcCb_c,
// dHR::searchSeries_c, dHR::searchLucky_c, dHR::searchCat_c; searchNew_c, searchOld_c, searchAdult_c and
// searchKiddy_c have no dHR:: prefix in their RTTI). Everything below class level is inferred.
// Free functions are in namespace dHR; stubs of unknown purpose keep the target names (extern "C").

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_model_room.hpp>
#include <game/game/d_item_name.hpp>

#define HR_THEME_UNIT_MSG 0x33 // sys_STRING/STR_Unit: the first model room theme name (+ getModelRoomTheme())

// Result flags (result_c::mFlags, set by rate and the rating aspects). Names follow the letters and remarks;
// bits 0..12 select the letter HR_MAIL_SERIES_COMPLETE + bit.
enum {
    HR_RESULT_SERIES_COMPLETE = 1 << 0,          // a basic series, all 12 parts (30000 points)
    HR_RESULT_SERIES_ALMOST = 1 << 1,            // a basic series, furniture + wallpaper or carpet (25000)
    HR_RESULT_SERIES_NEED_WALL_FLOOR = 1 << 2,   // a basic series, furniture only
    HR_RESULT_THEME = 1 << 3,                    // a theme series
    HR_RESULT_SET = 1 << 4,                      // set series points >= 3000
    HR_RESULT_COMFY = 1 << 5,                    // all five furniture parts (seriesParts_c::getBonus: "comfortable life")
    HR_RESULT_FENG_SHUI = 1 << 6,                // feng shui points >= 500
    HR_RESULT_COLOR = 1 << 7,                    // colour points >= 2000
    HR_RESULT_COHESIVE = 1 << 8,                 // new/old + adult/kiddy points (calcGenrePoints) >= 2000
    HR_RESULT_COLLECTION = 1 << 9,               // category points >= 3000 ("sentimental value")
    HR_RESULT_LUCKY = 1 << 10,                   // lucky points >= 7000
    HR_RESULT_MESSY = 1 << 11,                   // a real non-furniture item lying around (result_c::mRealItem)
    HR_RESULT_FACING_WALL = 1 << 12,             // furniture facing the wall (result_c::mWallItem)
    HR_RESULT_MANY_PLANTS = 1 << 14,             // 8+ of FTR_PART_B_PLANT
    HR_RESULT_MANY_ART = 1 << 15,                // 8+ of FTR_PART_B_ART
    HR_RESULT_MANY_INSTRUMENTS = 1 << 16,        // 8+ of FTR_PART_B_INSTRUMENT
    HR_RESULT_MANY_MODELS = 1 << 17,             // 8+ of FTR_PART_B_MODEL
    HR_RESULT_MANY_DOLLS = 1 << 18,              // 8+ of FTR_PART_B_DOLL
    HR_RESULT_COLOR_MATCH = 1 << 19,             // colour points > 0
    HR_RESULT_THEME_WALL_FLOOR = 1 << 20,        // a theme with its wallpaper and carpet, not complete
    HR_RESULT_SET_SERIES = 1 << 21,              // set series points > 0
};

// quickRate layout flags (getPlayerRank; d_npc_talk_quest_q08 picks its remark and present from them).
enum {
    HR_LAYOUT_MESSY = 1 << 0,           // a real non-furniture item lying around
    HR_LAYOUT_FACING_WALL = 1 << 1,     // furniture facing the wall
    HR_LAYOUT_COMFY = 1 << 2,           // all five furniture parts
    HR_LAYOUT_THEME_OR_SET = 1 << 3,    // a theme or set series
    HR_LAYOUT_SERIES = 1 << 4,          // a basic series
    HR_LAYOUT_SERIES_PARTS = 1 << 5,    // no basic series, but 5+ parts of one
    HR_LAYOUT_NO_THEME_OR_SET = 1 << 6, // not both a theme and a set series
};

// Letters: entries of sys_MAIL1 MAIL_ETC_Happyroom.bmg (the dMail_c kind). 2..6 are empty.
enum {
    HR_MAIL_WELCOME = 1,                      // the welcome letter (PRIVATE_FLAG0_HRA_JOINED)
    HR_MAIL_EXTRA_SERIES_COMPLETE = 7,        // 7..10: as HR_MAIL_SERIES_COMPLETE..HR_MAIL_THEME_COMPLETE for an
    HR_MAIL_EXTRA_SERIES_ALMOST = 8,          // extra series (isExtraSeries: no series name)
    HR_MAIL_EXTRA_SERIES_NEED_WALL_FLOOR = 9,
    HR_MAIL_EXTRA_THEME_COMPLETE = 10,
    HR_MAIL_MODEL_ROOM_THEME = 11,            // this month's model room theme (STR_Unit HR_THEME_UNIT_MSG + theme)
    HR_MAIL_SCORE_ZERO = 12,                  // zero points
    HR_MAIL_SIZE_0 = 13,                      // 13..17: under 20000 points, by house size
    HR_MAIL_SCORE_LOW = 18,                   // 20000..69999 points
    HR_MAIL_SCORE_MID = 19,                   // 70000..99999 points
    HR_MAIL_SCORE_HIGH = 20,                  // 100000 points and more
    HR_MAIL_SERIES_COMPLETE = 21,             // 21..33: HR_MAIL_SERIES_COMPLETE + result flag bit
    HR_MAIL_SERIES_ALMOST = 22,
    HR_MAIL_SERIES_NEED_WALL_FLOOR = 23,
    HR_MAIL_THEME_COMPLETE = 24,
    HR_MAIL_SET_MATCH = 25,
    HR_MAIL_COMFY = 26,
    HR_MAIL_FENG_SHUI = 27,
    HR_MAIL_COLOR = 28,
    HR_MAIL_COHESIVE = 29,
    HR_MAIL_COLLECTION = 30,
    HR_MAIL_LUCKY_BONUS = 31,
    HR_MAIL_MESSY = 32,
    HR_MAIL_FACING_WALL = 33,
    HR_MAIL_PRESENT_70000 = 34,               // the house model present
    HR_MAIL_PRESENT_100000 = 35,              // the wide house model present
    HR_MAIL_PRESENT_150000 = 36,              // the two-story model present

    HR_MAIL_POINT_NUM = HR_MAIL_FACING_WALL - HR_MAIL_SERIES_COMPLETE + 1, // letters selected by a result bit
};

// Model room remarks: entries of Lyle's NPC_honma.bmg (remark_c::mMsg). The SERIES_HALLOWEEN..SERIES_BUNNY_DAY,
// FESTIVE_TREE, CHOCOLATE and *_ITEM festival entries are season branches to the next one or two entries.
enum {
    HR_REMARK_THEME_NONE = 0x51,
    HR_REMARK_THEME_ADULT = 0x52,
    HR_REMARK_THEME_KIDDY = 0x53,
    HR_REMARK_THEME_NEW = 0x54,
    HR_REMARK_THEME_OLD = 0x55,
    HR_REMARK_FENG_SHUI_YELLOW_WEST = 0x56,
    HR_REMARK_FENG_SHUI_RED_EAST = 0x57,
    HR_REMARK_FENG_SHUI_GREEN_SOUTH = 0x58,
    HR_REMARK_SERIES_HALLOWEEN = 0x5F,
    HR_REMARK_SERIES_JINGLE = 0x62,
    HR_REMARK_SERIES_HARVEST = 0x65,
    HR_REMARK_SERIES_BUNNY_DAY = 0x68,
    HR_REMARK_SERIES_GRACIE = 0x6B,
    HR_REMARK_SERIES_COMPLETE = 0x6C,
    HR_REMARK_SERIES_ALMOST = 0x6D,
    HR_REMARK_SERIES_NEED_WALL_FLOOR = 0x6E,
    HR_REMARK_THEME_SERIES = 0x6F,
    HR_REMARK_LUCKY_ITEM = 0x70,
    HR_REMARK_SET_SERIES = 0x71,
    HR_REMARK_COLOR_MATCH = 0x79,
    HR_REMARK_THEME_WALL_FLOOR = 0x7A,
    HR_REMARK_COMFY = 0x7B,
    HR_REMARK_MANY_INSTRUMENTS = 0x7C,
    HR_REMARK_MANY_PLANTS = 0x7D,
    HR_REMARK_MANY_ART = 0x7E,
    HR_REMARK_MANY_MODELS = 0x7F,
    HR_REMARK_MANY_DOLLS = 0x80,
    HR_REMARK_MESSY_ITEM = 0x87,
    HR_REMARK_FACING_WALL = 0x88,
    HR_REMARK_MISMATCH = 0x89,
    HR_REMARK_LAYOUT_SPARSE = 0x8A,
    HR_REMARK_LAYOUT_GOOD = 0x8B,
    HR_REMARK_LAYOUT_PACKED = 0x8C,
    HR_REMARK_SNOWMAN = 0x91,
    HR_REMARK_BIRTHDAY_CAKE = 0x92,
    HR_REMARK_PORTRAIT = 0x93,
    HR_REMARK_FESTIVE_TREE = 0x94,
    HR_REMARK_CHOCOLATE = 0x97,
    HR_REMARK_HALLOWEEN_ITEM = 0x9A,
    HR_REMARK_JINGLE_ITEM = 0x9D,
    HR_REMARK_HARVEST_ITEM = 0xA0,
    HR_REMARK_BUNNY_DAY_ITEM = 0xA3,
    HR_REMARK_CLOTHES = 0xAA,
    HR_REMARK_ODD_ITEM = 0xAB,
    HR_REMARK_AQUARIUM = 0xAC,
    HR_REMARK_GYROID = 0xAD,
    HR_REMARK_GYROIDS = 0xAE,
    HR_REMARK_FOSSIL = 0xAF,
    HR_REMARK_FOSSILS = 0xB0,
    HR_REMARK_SOME_INSTRUMENTS = 0xB1,
    HR_REMARK_SOME_PLANTS = 0xB2,
    HR_REMARK_SOME_PAINTINGS = 0xB3,
    HR_REMARK_LAMPS = 0xB9,
    HR_REMARK_CLOCKS = 0xBA,
    HR_REMARK_CHAIRS = 0xBB,
    HR_REMARK_DRESSERS = 0xBC,
    HR_REMARK_STEREOS = 0xBD,
    HR_REMARK_TVS = 0xBE,
    HR_REMARK_BEDS = 0xBF,
    HR_REMARK_CREATURES = 0xC0,
};

class dFdBase_c;
class dDemo_c;
struct dNpcFtrShape_c;

// Vtables are emitted in reverse class-completion order: the target's .data has searchCat_c,
// searchLucky_c, searchSeries_c, searchKiddy_c, searchAdult_c, searchOld_c, searchNew_c, fdNpcCb_c,
// fdPlCb_c, fdMdlCb_c, dItem::nameSeries_c, so the classes are completed in the opposite order.

namespace dHR {

// Where a rated room's field, wallpaper and carpet come from. No virtual dtor (3-entry vtable).
class fdCb_c {
public:
    virtual dFdBase_c *getField(int room) = 0;
    virtual dItem::Item getWallpaper(int room) = 0;
    virtual dItem::Item getCarpet(int room) = 0;
};

// A model room (dSvMdlRm_c): its wallpaper and carpet copied at construction, the field given.
class fdMdlCb_c : public fdCb_c {
public:
    fdMdlCb_c(const dHomeRoom_c *room, dFdBase_c *field); // 800ACB48
    virtual dFdBase_c *getField(int room);                // 800ACBD0
    virtual dItem::Item getWallpaper(int room);           // 800ACBD8
    virtual dItem::Item getCarpet(int room);              // 800ACBE4

    /* 0x4 */ dItem::Item mWallpaper;
    /* 0x6 */ dItem::Item mCarpet;
    /* 0x8 */ dFdBase_c *mField;
}; // size 0xC

// A player's house (mHome: dHomeList_c index): the room's field id is 2 + home * 3 + room.
class fdPlCb_c : public fdCb_c {
public:
    fdPlCb_c(u32 home) : mHome(home & 3) {}
    virtual dFdBase_c *getField(int room);      // 800ACBF0
    virtual dItem::Item getWallpaper(int room); // 800ACC6C: default ITEM_IDX_EXOTIC_WALL without a room
    virtual dItem::Item getCarpet(int room);    // 800ACCC4: default ITEM_IDX_EXOTIC_RUG without a room
    dHomeRoom_c *getRoom(int room);             // 800ACC08

    /* 0x4 */ u32 mHome;
}; // size 0x8

// A villager's house.
class fdNpcCb_c : public fdCb_c {
public:
    fdNpcCb_c(dFdBase_c *field, dItem::Item carpet, dItem::Item wallpaper); // 800ACD1C
    virtual dFdBase_c *getField(int room);      // 800ACE30
    virtual dItem::Item getWallpaper(int room); // 800ACE38
    virtual dItem::Item getCarpet(int room);    // 800ACE44

    /* 0x4 */ dFdBase_c *mField;
    /* 0x8 */ dItem::Item mCarpet;
    /* 0xA */ dItem::Item mWallpaper;
}; // size 0xC



// The feng shui rating of a house (4 bytes, 80597340[4] by dHomeList_c index): the three colour
// scores of fnShui_c summed over the rooms, and whether it is set. getRate returns mScore.
struct rate_c {
    rate_c() { clear(); } // weak 800B4BA0 (for the __construct_array in __sinit)
    void clear() {
        mScore[0] = mScore[1] = mScore[2] = 0;
        mValid = 0;
    }

    /* 0x0 */ u8 mScore[3];
    /* 0x3 */ u8 mValid;
}; // size 0x4

// The feng shui scores of one room: furniture of colour 1 by the left wall, 4 by the back wall and
// 2 by the right wall (fn_800AC798), with the last matching item of each.
struct fnShui_c {
    fnShui_c(); // 800AC770
    void calc(dFdBase_c *field); // 800AC99C
    u8 count(dFdBase_c *field, u32 x0, u32 x1, u32 z0, u32 z1, int color, dItem::Item *out); // 800AC798

    /* 0x0 */ u8 mScore[3];
    /* 0x4 */ dItem::Item mItemL;
    /* 0x6 */ dItem::Item mItemB;
    /* 0x8 */ dItem::Item mItemR;
}; // size 0xA

// 80597760: per series (dItem::SeriesId) the furniture part bits seen (fn_800ACA60), all ORed in mAll.
struct seriesParts_c {
    seriesParts_c() { clear(); }
    void clear() {
        for (int i = 0; i < dItem::SERIES_COUNT; i++) {
            mParts[i] = 0;
        }
        mAll = 0;
    }
    void add(dItem::Item item); // 800ACA60
    // The part bits of series (0 out of range).
    s32 get(u32 series) const { return series < dItem::SERIES_COUNT ? mParts[series] : 0; }
    // All five parts seen (quickRate).
    BOOL hasAllParts() const { return (mAll & 0x1F) == 0x1F; }
    // The parts bonus of rate: 1000 with all five parts seen, 5000 if one series has them all.
    int getBonus() const {
        int bonus = 0;
        if (hasAllParts()) {
            BOOL complete = FALSE;
            for (int i = 0; i < dItem::SERIES_COUNT; i++) {
                if ((get(i) & 0x1F) == 0x1F) {
                    complete = TRUE;
                    break;
                }
            }
            bonus = 1000;
            if (complete) {
                bonus = 5000;
            }
        }
        return bonus;
    }

    /* 0x000 */ s32 mParts[dItem::SERIES_COUNT];
    /* 0x210 */ s32 mAll;
}; // size 0x214

// l_seriesScores: the best score of each series (checkBasicSeries / checkThemeSeries / checkSetSeries).
struct seriesScores_c {
    seriesScores_c() { clear(); }
    void clear() { memset(this, 0, sizeof(*this)); }
    // The score of series (0 out of range).
    u32 get(int series) const { return series < dItem::SERIES_COUNT ? mScores[series] : 0; }

    // Raises the score of series to score; FALSE if it already had as much (checkBasicSeries..checkSetSeries).
    BOOL raise(int series, u32 score) {
        if (series < dItem::SERIES_COUNT && mScores[series] < score) {
            mScores[series] = score;
            return TRUE;
        }
        return FALSE;
    }

    /* 0x000 */ u32 mScores[dItem::SERIES_COUNT];
}; // size 0x210

// 80597560 and locals of calcColorPoints/calcGenrePoints: one bit per item base id (0x1000), to count
// distinct furniture (addFtr adds). size 0x200.
struct ftrSet_c {
    ftrSet_c() { clear(); }
    void clear() { memset(this, 0, sizeof(*this)); }
    bool test(u16 baseId) const {
        if (baseId < 0x1000) {
            return (mBits[baseId >> 3] & (1 << (baseId & 7))) != 0;
        }
        return false;
    }
    u32 count() const {
        u32 n = 0;
        for (u32 i = 0; i < 0x1000; i++) {
            if (test(i)) {
                n++;
            }
        }
        return n;
    }

    /* 0x000 */ u8 mBits[0x1000 / 8];
}; // size 0x200

// One remark of the rating (805981F4[5] / makeRemarks): an item, its series and a message.
struct remark_c {
    remark_c(); // 800B24F8
    void clear(); // 800B2514: item ITEM_IDX_APPLE (the placeholder)
    dItem::Item getItem() const; // 800B255C: the item, ITEM_IDX_APPLE if it is not a real item
    void setWords(dDemo_c *demo) const; // 800B25C4: word 0 the series name (if any), word 1 the item name

    /* 0x0 */ dItem::Item mItem;
    /* 0x4 */ u32 mSeries;
    /* 0x8 */ u16 mMsg;
}; // size 0xC

// The five remarks of a rating (805981F4; makeRemarks passes them to makeModelRoomRemarks). Five members, not an
// array: the implicit ctor / dtor then call no __construct_array / __destroy_arr.
struct remarks_c {
    /* 0x00 */ remark_c mTheme;
    /* 0x0C */ remark_c mSeries;
    /* 0x18 */ remark_c mParts;
    /* 0x24 */ remark_c mNotable;
    /* 0x30 */ remark_c mExtra;
}; // size 0x3C

} // namespace dHR

// Room item searches for dSvMdlRm_c::getRandomRoomItem (all constructed in makeModelRoomRemarks).
class searchNew_c : public dSvMdlRm_c::searchCB_c {
public:
    virtual BOOL check(const dItem::Item *item); // 800B49B8
};
class searchOld_c : public dSvMdlRm_c::searchCB_c {
public:
    virtual BOOL check(const dItem::Item *item); // 800B4960
};
class searchAdult_c : public dSvMdlRm_c::searchCB_c {
public:
    virtual BOOL check(const dItem::Item *item); // 800B4908
};
class searchKiddy_c : public dSvMdlRm_c::searchCB_c {
public:
    virtual BOOL check(const dItem::Item *item); // 800B48B0: BITM adult/kiddy == 2
};

namespace dHR {

// Furniture of series mSeries that is not in mExclude[0..mExcludeNum).
class searchSeries_c : public dSvMdlRm_c::searchCB_c {
public:
    // In the body, mSeries before mExcludeNum: makeModelRoomRemarks stores 0xC before 0x8.
    searchSeries_c(const dItem::Item *exclude, u32 excludeNum, int series) {
        mExclude = exclude;
        mSeries = series;
        mExcludeNum = excludeNum;
    }
    virtual BOOL check(const dItem::Item *item); // 800B47D4

    /* 0x4 */ const dItem::Item *mExclude;
    /* 0x8 */ u32 mExcludeNum;
    /* 0xC */ int mSeries;
}; // size 0x10
// Lucky items (BITM::m_hraLuckyBonus) that are not in mExclude[0..mExcludeNum).
class searchLucky_c : public dSvMdlRm_c::searchCB_c {
public:
    searchLucky_c(const dItem::Item *exclude, u32 excludeNum) : mExclude(exclude), mExcludeNum(excludeNum) {}
    virtual BOOL check(const dItem::Item *item); // 800B4710

    /* 0x4 */ const dItem::Item *mExclude;
    /* 0x8 */ u32 mExcludeNum;
}; // size 0xC
// Furniture whose part B (BITM::m_ftrPartB, dItem::FtrPartB) is mPart.
class searchCat_c : public dSvMdlRm_c::searchCB_c {
public:
    searchCat_c(int part) : mPart(part) {}
    virtual BOOL check(const dItem::Item *item); // 800B4694

    /* 0x4 */ int mPart;
}; // size 0x8

// (Defined by C; owners may rename fields once their meaning is known.)
// The rated room: furniture bounds in units (inclusive, from calcBounds) and the room's wallpaper and
// carpet (from the fdCb_c). First argument of rate and of the per-aspect checks. size 0x14.
struct area_c {
    /* 0x00 */ int mMinX;
    /* 0x04 */ int mMinZ;
    /* 0x08 */ int mMaxX;
    /* 0x0C */ int mMaxZ;
    /* 0x10 */ dItem::Item mWallpaper;
    /* 0x12 */ dItem::Item mCarpet;
};

// What the rating found (second argument of rate). size 0x2C.
struct result_c {
    // All in the body, the two items reset last in reverse order: the target stores the words
    // first and then 0x2A before 0x28, with 0 and 0x10000 (for 0xFFF1) in this register order
    // (selectModelRooms; B found the same form for ratePlayer).
    result_c() {
        mFlags = 0;
        mCategory = 0;
        mNewOld = 0;
        mAdultKiddy = 0;
        mThemePartSeries = 0x65;
        mBasicSeries = 0x65;
        mBasicSeries2 = 0x65;
        mBasicSeries3 = 0x65;
        mThemeSeries = 0x65;
        mSetSeries = 0x65;
        mWallItem = dItem::Item();
        mRealItem = dItem::Item();
    }
    // Weak (not inlined: the Item argument has a destructor).
    void setWallItem(dItem::Item item) { mWallItem = item; } // 800B4BB8
    void setRealItem(dItem::Item item) { mRealItem = item; } // 800B4BC4
    // const: makeModelRoomRemarks loads mFlags twice where one plain read would be CSE'd.
    BOOL isFlag(u32 flag) const { return mFlags & flag; }

    /* 0x00 */ u32 mFlags;
    /* 0x04 */ u32 mCategory;       // calcCategoryPoints: BITM ftrPartB group with >= 8 pieces
    /* 0x08 */ u32 mNewOld;         // calcGenrePoints
    /* 0x0C */ u32 mAdultKiddy;     // calcGenrePoints
    // The series are signed: makeModelRoomRemarks compares them with cmpw / cmpwi.
    /* 0x10 */ int mThemePartSeries; // checkThemeSeries: theme with wallpaper+carpet but not complete (flag 0x100000)
    /* 0x14 */ int mBasicSeries;    // checkBasicSeries: basic series complete incl. wallpaper and carpet
    /* 0x18 */ int mBasicSeries2;   // checkBasicSeries: all furniture + wallpaper or carpet
    /* 0x1C */ int mBasicSeries3;   // checkBasicSeries: all furniture
    /* 0x20 */ int mThemeSeries;    // checkThemeSeries: complete theme
    /* 0x24 */ int mSetSeries;      // checkSetSeries: complete set
    /* 0x28 */ dItem::Item mRealItem;
    /* 0x2A */ dItem::Item mWallItem;
};

} // namespace dHR

namespace dHR {
// ---- Feng shui ratings of the player houses ----
// 800AC28C: the rating record of player house home (< 4); home 4 = the current player's house (a zeroed
// static record when the player has none). Returns rate_c::mScore; d_npc_talk_rollan reads bytes 1 and 2.
const u8 *getRate(u32 home);
const u8 *getAverageRate();             // 800AC320: the average over the rated houses
void updateRates();                     // 800AC4A4: updateRate for the four houses (d_sv_proc)
void updateRate(u32 home);              // 800AC4E0: rates player house home's feng shui into l_rates[home]
u32 getPictureRate();                   // 800AC5F8: the three scores / (10 * rated houses) (d_save_shop_gallery)
u32 getFtrRate();                       // 800AC6B4: scores 1 + 2 / (10 * rated houses) (d_save_shop_gallery)
BOOL isExtraSeries(int series);         // 800AC260: series >= SERIES_EXTRA_BASIC_1 (no STR_Furniture name)

// ---- Ratings ----
// 800ACE50: quick rating of room 0 of cb (no score, no letter): returns layout flags, *rank = 1..5.
int quickRate(area_c *area, fdCb_c *cb, u32 *rank);
// 800AD118: quickRate on villager animalIdx's house (*rank 3 and 0 without the villager).
int quickRateAnimal(area_c *area, int animalIdx, u32 *rank);
// 800AD1FC: quickRate on player house home.
int quickRatePlayer(area_c *area, u32 home, u32 *rank);
// 800AD234: the full rating (rate) of player house home.
int ratePlayer(area_c *area, u32 home, int mode, int type, int start, int count, u8 *sent, bool letter);
// 800AD2E0: the full rating of villager animalIdx's house.
int rateAnimal(area_c *area, int animalIdx, result_c *result, int type);
// 800AD3D4: the full rating of a model room (0 without an owner).
int rateModelRoom(area_c *area, result_c *result, dSvMdlRm_c *room, int type);
// 800AD514: one of rooms start..start+count-1 has a custom design (wallpaper, carpet or furniture).
BOOL hasOrgDesign(area_c *area, fdCb_c *cb, int start, int count);
// 800AD680: the full rating of rooms start..start+count-1 of cb: returns the points (-1 with a custom
// design unless type 6), fills result; mode 1: the current player's weekly HRA letter.
int rate(area_c *area, result_c *result, fdCb_c *cb, int start, int count, int mode, int type, u8 *sent,
         bool letter);
// 800B2480: rating of player house home (dHomeList_c index) into *rank (through quickRatePlayer); returns
// layout flags (d_npc_talk_quest_q08 picks its "Q08_Layout" remark and present from them).
int getPlayerRank(int home, u32 *rank);
// 800B24BC: rating of villager animalIdx's house into *rank (through quickRateAnimal); d_npc_talk_quest_q09
// clamps it to 1..5 to pick the present.
int getAnimalRank(int animalIdx, u32 *rank);

// ---- Rating aspects ----
// 800AE70C: basic (group 0) series into l_seriesScores: 12 parts (10 furniture, wallpaper 0x400, carpet
// 0x800); *maxParts = the most parts of one basic series. Returns the series, SERIES_COUNT if none.
u32 checkBasicSeries(area_c *area, result_c *result, dFdBase_c *field, u32 *maxParts);
BOOL checkThemeSeries(area_c *area, result_c *result, dFdBase_c *field); // 800AEBC8: theme (group 1) series
BOOL checkSetSeries(area_c *area, result_c *result, dFdBase_c *field);   // 800AF038: set (group 2) series
void collectParts(const area_c *area, dFdBase_c *field);            // 800AF3F4: furniture parts into l_seriesParts
int calcFromPoints(const area_c *area, dFdBase_c *field);           // 800AF4AC: "from" points of furniture with a function
int calcFengShuiPoints(const area_c *area, const fnShui_c *fnShui); // 800AF58C: feng shui points
u32 calcColorPoints(const area_c *area, dFdBase_c *field);          // 800AF5A8: colour points
void addFtr(ftrSet_c *set, dItem::Item item);                       // 800AF9A0: adds a furniture item (with a function)
// 800AFA18: new/old and adult/kiddy points (theme 6: any).
u32 calcGenrePoints(const area_c *area, dFdBase_c *field, u32 *newOld, u32 *adultKiddy, int theme);
// 800B003C: category points (8 or more of one category).
u32 calcCategoryPoints(const area_c *area, result_c *result, dFdBase_c *field, u32 *category);
// 800B0204: wall items (against the edge they face) and real non-furniture items.
int calcWallPoints(const area_c *area, result_c *result, dFdBase_c *field, u8 *wall, u8 *real);
void collectLucky(const area_c *area, dFdBase_c *field);            // 800B0424: lucky items into l_luckyItems
BOOL calcBounds(int *minX, int *maxX, int *minZ, int *maxZ, dFdBase_c *field); // 800B0844: the room's bounds

// ---- Letters ----
// 800B04F8: the HRA letter (MAIL_ETC_Happyroom); hold: only add it to the post office.
BOOL sendRatingLetter(const area_c *area, int kind, int points, u32 series, u32 category, BOOL hold);
BOOL sendModelPresent(const area_c *area, int points); // 800B0698: the house model presents (70000 / 100000 / 150000)

// ---- Model rooms ----
// 800B0954: ratePlayer on the current player's house; selectAllModelRooms if sent (d_private_data).
void rateCurrentPlayer(bool letter, int mode);
int rateAnimalHome(int animalIdx, result_c *result, int type);         // 800B09E4: rateAnimal
int rateModelRoomResult(result_c *result, dSvMdlRm_c *room, int type); // 800B0A28: rateModelRoom (result may be NULL)
void selectAllModelRooms();                                            // 800B0AB0: selectModelRooms(TRUE, TRUE)
// 800B0ABC: picks this month's model rooms (the kept one and the candidate in dSaveTown_c): re-rates the
// kept one for a new theme, then rates every room of the players' houses (players) and every villager's
// house (animals). d_sv_proc: (FALSE, TRUE).
void selectModelRooms(BOOL players, BOOL animals);
// 800B12F4: a model room received over WiiConnect24 (d_wifi): rated for this month's theme, it replaces
// the kept one if it scores better (or ties against a villager's or another town's room).
void receiveModelRoom(dSvMdlRm_c *room);

// ---- Remarks on this month's model room ----
remarks_c *getRemarks();              // 800B2674: l_remarks
void updateRemarks();                 // 800B2680: makeRemarks(l_remarks)
void makeRemarks(remarks_c *remarks); // 800B268C: makeModelRoomRemarks with the five remarks (a local when NULL)
// 800B2740: the five remarks on the kept model room (NULL outputs use locals): the theme item (or feng
// shui), the series, the furniture parts, a notable item or collection, and one more (theme opposite / wall
// item / room size). Each is a message id and an item.
void makeModelRoomRemarks(remark_c *outTheme, remark_c *outSeries, remark_c *outParts, remark_c *outNotable,
                          remark_c *outExtra);

int isBusy(); // 800B4684: stub, returns 0 (d_s_boot_static)
} // namespace dHR

// Stubs of unknown purpose (target names until known).
extern "C" {
BOOL fn_800AC278();                 // 800AC278: returns TRUE (d_bg_debug)
void fn_800AC280();                 // 800AC280 (d_bg_debug)
BOOL fn_800AC284();                 // 800AC284: returns TRUE (d_bg_debug)
BOOL fn_800B094C();                 // 800B094C: returns FALSE (no callers)
void fn_800B468C();                 // 800B468C (no callers)
void fn_800B4690(dSvMdlRm_c *room); // 800B4690 (receiveModelRoom passes its room)
}
