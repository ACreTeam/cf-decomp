#pragma once

#include <types.h>
#include <cstring>
#include <game/game/d_dvd.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_string.hpp>
#include <lib/nw4r/ut/ut_list.h>
#include <nw4r/g3d/res/g3d_resfile.h>

// Item definition tables loaded from /Item/item.arc.
// RTTI names place these under the dItem namespace.
namespace dItem {

enum {
    // Entries stored in item/item.bin; lower item indices address the table.
    ITEM_COUNT = 0xA15,
    // Indices [DL_ITEM_FIRST, DL_ITEM_END) resolve through the save's
    // downloadable-item blocks instead of item.bin.
    DL_ITEM_FIRST = 0xA15,
    DL_ITEM_END = 0xB15,
    DL_ITEM_COUNT = DL_ITEM_END - DL_ITEM_FIRST,

    BASE_ID_COUNT = 0x1000,
    SERIES_COUNT = 0x84,

    BITM_VERSION = 0x1701,

    // Item IDs: categories 9..12 (bits 12-15) encode 0x9000 + (baseId << 2).
    // ITEM_ID_NONE is declared with Item in d_fg_item.hpp.
    INDEX_NONE = 0xFFFF,
};

enum Language {
    LANG_JP,
    LANG_US,
    LANG_MX,
    LANG_QC,
    LANG_EN,
    LANG_ES,
    LANG_FR,
    LANG_IT,
    LANG_DE,
    LANG_KR,
};

// Item kinds (BITM::m_kind). Names follow the game's resource-name table
// (804E9C80, read by getKindName).
enum Kind {
    KIND_MONEY, // 0x00
    KIND_WALL,
    KIND_CARPET,
    KIND_FTR,
    KIND_CLOTH,
    KIND_CAP,
    KIND_ACC,
    KIND_INSECT,
    KIND_FISH,
    KIND_PAPER,
    KIND_BEFORE_FOSSIL,
    KIND_FOSSIL,
    KIND_HANIWA,
    KIND_PICTURE,
    KIND_MUSIC,
    KIND_FRUIT,
    KIND_SEED,
    KIND_FAKE_PICTURE_BEFORE,
    KIND_FAKE_PICTURE_AFTER,
    KIND_ORG_CLOTH,
    KIND_ORG_UMB,
    KIND_ORG_CAP,
    KIND_ORG_WC,
    KIND_ORG_EASEL,
    KIND_TA_CLOTH,
    KIND_SEEDLING,
    KIND_KABU,
    KIND_BAD_KABU,
    KIND_RKABU,
    KIND_RKABU_SEED,
    KIND_PAINT,
    KIND_SHELL,
    KIND_SOLD_OUT,
    KIND_BOTTLE_BEFORE,
    KIND_BOTTLE_AFTER,
    KIND_PITFALL_SEED,
    KIND_PBOX,
    KIND_HAND_LETTER,
    KIND_PAPER_BAG,
    KIND_MEDICINE,
    KIND_DUST,
    KIND_NONE, // 0x29
    KIND_DUMMY,
    KIND_EGG_FAKE_BEFORE,
    KIND_EGG_FAKE_AFTER,
    KIND_EGG_BINGO_BEFORE,
    KIND_EGG_BINGO_AFTER,
    KIND_KNIFE_AND_FORK,
    KIND_CANDY,
    KIND_HANABI,
    KIND_CHOCOLATE,
    KIND_CRACKER,
    KIND_CATALOG_ONLY,
    KIND_KEY,
    KIND_MUSHROOM,
    KIND_TIMER,
    KIND_UMBRELLA,
    KIND_FLOWER,
    KIND_FISHINGROD,
    KIND_SILVER_FISHINGROD,
    KIND_GOLD_FISHINGROD,
    KIND_SCOOP,
    KIND_SILVER_SCOOP,
    KIND_GOLD_SCOOP,
    KIND_AXE,
    KIND_SILVER_AXE,
    KIND_GOLD_AXE,
    KIND_WATERING,
    KIND_SILVER_WATERING,
    KIND_GOLD_WATERING,
    KIND_NET,
    KIND_SILVER_NET,
    KIND_GOLD_NET,
    KIND_PACHINKO,
    KIND_GOLD_PACHINKO,
    KIND_SILVER_PACHINKO,
    KIND_BALLOON,
    KIND_SYABON,
    KIND_WINDMILL,
    KIND_CREDIT_CARD,
    KIND_LAMP,
    KIND_UFO_PARTS,
    KIND_DSN_DATA_PLAYER,
    KIND_DSN_DATA_TA,
    KIND_DSN_DATA_FLAG,
    KIND_DSN_DATA_SEIICHI,
    KIND_MUSH_FTR,

    KIND_COUNT, // 0x57
};

// Furniture footprint (BITM::m_ftrSize).
enum FtrSize {
    FTR_SIZE_1x1, // 0x00
    FTR_SIZE_1x2,
    FTR_SIZE_2x2,

    FTR_SIZE_COUNT, // 0x03
};

// Item source groups (BITM::m_from). Names follow the game's own string table
// (804EA114, owned by the preceding TU).
enum From {
    FROM_GROUP_ABC, // 0x00
    FROM_GROUP_ABC_AVERAGE,
    FROM_GROUP_ABC_ALL,
    FROM_GROUP_A,
    FROM_GROUP_B,
    FROM_GROUP_C,
    FROM_EYE_CATCHER,
    FROM_FOX,
    FROM_TAILOR,
    FROM_PRESENT,
    FROM_HANIWA,
    FROM_FLOWER,
    FROM_INSECT,
    FROM_FISH,
    FROM_NORMAL_FOSSIL,
    FROM_NICE_FOSSIL,
    FROM_SP_PRESENT,
    FROM_SNOW,
    FROM_JONNY,
    FROM_FORTUNE,
    FROM_RAKKO,
    FROM_PICTURE,
    FROM_ORIGINAL,
    FROM_NOUSE,
    FROM_DONGURI,
    FROM_LOST,
    FROM_WARASIBE,
    FROM_SAVING,
    FROM_FISHING,
    FROM_FISHING_SP,
    FROM_BUGCATCHING,
    FROM_GARDENING,
    FROM_GRACE,
    FROM_ROLAN,
    FROM_SHELL,
    FROM_BED_DEFAULT,
    FROM_FOX_PICTURE,
    FROM_FORGED,
    FROM_HAPPY_ROOM,
    FROM_POINT_CHANGE,
    FROM_POINT_PRESENT,
    FROM_AFTER_FORGED,
    FROM_LIMITED1,
    FROM_LIMITED2,
    FROM_NONE, // 0x2C
    FROM_JINGLE,
    FROM_SETSUBUN,
    FROM_HINA,
    FROM_KODOMO,
    FROM_TSUKIMI_JP,
    FROM_TSUKIMI_EU,
    FROM_TSUKIMI_KR,
    FROM_GROUNDHOG,
    FROM_EARTH_DAY,
    FROM_LABOR_DAY,
    FROM_COLUMBUS_DAY,
    FROM_HARVESTMOON,
    FROM_MIDSUMMER,
    FROM_ST_NICHOLAS_DAY,
    FROM_MIDWINTER,
    FROM_OLD_NEWYEAR,
    FROM_PLANTING_DAY,
    FROM_MASTERS_DAY,
    FROM_TANABATA,
    FROM_NEWYEAR,
    FROM_COUNTDOWN,
    FROM_HARVESTFESTIVAL,
    FROM_HALLOWEEN,
    FROM_FIREWORKS,
    FROM_EASTER,
    FROM_GRACE_SPR,
    FROM_GRACE_SUM,
    FROM_GRACE_AUT,
    FROM_GRACE_WIN,
    FROM_CARNIVAL,
    FROM_MUSIC_GOKIGEN,
    FROM_MUSIC_FUKIGEN,
    FROM_MUSIC_MATTARI,
    FROM_MUSIC_BLUE,
    FROM_MUSIC_UNKNOWN,
    FROM_MUSIC_SECRET,
    FROM_APRIL_FOOL,
    FROM_LIMITED3,
    FROM_LIMITED4,
    FROM_BALLOON,
    FROM_GRACE_SPR_FASHION,
    FROM_GRACE_SUM_FASHION,
    FROM_GRACE_AUT_FASHION,
    FROM_GRACE_WIN_FASHION,
    FROM_BALLOON_MAN,
    FROM_MUSHROOM,
    FROM_MUSIC_HAZURE,

    FROM_COUNT, // 0x5C
};

// item/item.bin header; the BITM entries follow it.
struct BinHeader {
    u32 mCount; // 0x00: ITEM_COUNT
    u32 mEntrySize; // 0x04: sizeof(BITM)
    u8 _08[0x18];
}; // sizeof = 0x20

// Range check used by the BITM field accessors: an out-of-range value
// falls back to a default.
inline int clampField(int value, u32 count, int def) {
    return static_cast<u32>(value) < count ? value : def;
}

// One item definition. Field order and names follow community research;
// offsets are verified against item.bin and the dItem accessors.
struct BITM {
    // Normal or silver tool of a type; gold tools are excluded.
    bool isAxe() const; // 800C1AD4
    bool isNet() const; // 800C1B0C
    bool isFishingrod() const; // 800C1B44
    bool isWatering() const; // 800C1B7C
    int getNewOld() const; // 800C1BB4
    int getAdultKiddy() const; // 800C1BE0
    BOOL isNotForSale() const; // 800C1C08
    static u16 getVersion(); // 800C1C80
    BOOL isValid() const; // 800C1C88
    int getFtrFuncType() const; // 800C1CCC
    const wchar_t *getName() const; // 800C1CF8
    int getDefArticle() const; // 800C1D90
    int getIndefArticle() const; // 800C1E48
    int getGender() const; // 800C1F00
    BOOL isFurnitureLike() const; // 800C1FB0
    BOOL isAvailableInRegion() const; // 800C2008
    int getSeriesGroup() const; // 800C2068
    BOOL canAddToCatalog() const; // 800C20DC
    int getKindFlag6() const; // 800C214C
    int getKindFlag5() const; // 800C21AC
    int getKindFlag4() const; // 800C220C
    const char *getKindName() const; // 800C226C
    const char *getFgObjName(BOOL alt) const; // 800C22CC
    u16 getFromNameIndex() const; // 800C2318
    int resolveColor(int color, BOOL usePlayer) const; // 800C2354

    int getKind() const {
        int kind = m_kind;
        return static_cast<u32>(kind) < KIND_COUNT ? kind : KIND_NONE;
    }

    int getFtrSize() const {
        int size = static_cast<s8>(m_ftrSize);
        return static_cast<u32>(size) < FTR_SIZE_COUNT ? static_cast<FtrSize>(size) : FTR_SIZE_1x1;
    }

    int getFashion() const {
        int fashion = 0;
        if (static_cast<u32>(m_fashion) < 0x39) {
            fashion = m_fashion;
        }

        return fashion;
    }

    u32 m_magic; // 0x000: 'BITM'
    s32 m_price; // 0x004
    s16 m_baseId; // 0x008
    u16 m_icon; // 0x00A
    s16 m_ftrKind; // 0x00C
    s16 _E; // 0x00E
    s16 m_version; // 0x010
    wchar_t m_nameJp[17]; // 0x012
    wchar_t m_nameUs[17]; // 0x034
    wchar_t m_nameMx[17]; // 0x056
    wchar_t m_nameQc[17]; // 0x078
    wchar_t m_nameEn[17]; // 0x09A
    wchar_t m_nameDe[17]; // 0x0BC
    wchar_t m_nameIt[17]; // 0x0DE
    wchar_t m_nameEs[17]; // 0x100
    wchar_t m_nameFr[17]; // 0x122
    wchar_t m_nameKr[17]; // 0x144
    s8 m_kind; // 0x166
    u8 m_addItem; // 0x167
    s8 m_fgobj; // 0x168
    s8 m_series; // 0x169
    s8 m_from; // 0x16A
    u8 m_carpetSfx; // 0x16B
    s8 m_fashion; // 0x16C
    u8 m_designCreatorName; // 0x16D
    u8 m_designCreatorFrom; // 0x16E
    u8 m_catalogScale; // 0x16F
    u8 _170; // 0x170
    s8 m_ftrFunc; // 0x171
    u8 m_ftrInstr; // 0x172
    u8 m_ftrSfx; // 0x173
    s8 m_fossil; // 0x174
    u8 m_ftrHeight; // 0x175
    // MWCC allocates u8 bit-fields from the most significant bit.
    u8 m_region : 4; // 0x176
    u8 m_season : 4;
    u8 m_hideBone : 4; // 0x177
    u8 m_style : 4;
    u8 m_textPltt : 4; // 0x178
    u8 m_designPltt : 4;
    u8 m_ftrSize : 4; // 0x179
    u8 m_ftrColorA : 4;
    u8 m_ftrColorB : 4; // 0x17A
    u8 m_ftrPartA : 4;
    u8 m_ftrPartB : 4; // 0x17B
    u8 m_defUs : 4;
    u8 m_indefUs : 4; // 0x17C
    u8 m_defMx : 4;
    u8 m_indefMx : 4; // 0x17D
    u8 m_genderMx : 4;
    u8 m_defQc : 4; // 0x17E
    u8 m_indefQc : 4;
    u8 m_genderQc : 4; // 0x17F
    u8 m_defEn : 4;
    u8 m_indefEn : 4; // 0x180
    u8 m_defDe : 4;
    u8 m_indefDe : 4; // 0x181
    u8 m_genderDe : 4;
    u8 m_defIt : 4; // 0x182
    u8 m_indefIt : 4;
    u8 m_genderIt : 4; // 0x183
    u8 m_defEs : 4;
    u8 m_indefEs : 4; // 0x184
    u8 m_genderEs : 4;
    u8 m_defFr : 4; // 0x185
    u8 m_indefFr : 4;
    u8 m_genderFr : 4; // 0x186
    u8 m_edibility : 2;
    u8 m_ftrTable : 2;
    u8 m_ftrLamp : 2; // 0x187
    u8 m_hasRes : 1;
    u8 m_noPurchase : 1;
    u8 _187_3 : 1;
    u8 m_proDesign : 1;
    u8 m_catalogStore : 1;
    // The genre pairs straddle bytes and are read as separate bits
    // (0x187 bit 0 / 0x188 bit 7, 0x188 bits 6 and 5).
    u8 m_ftrGenreA0 : 1;
    u8 m_ftrGenreA1 : 1; // 0x188
    u8 m_ftrGenreB0 : 1;
    u8 m_ftrGenreB1 : 1;
    u8 m_hraWallCheck : 1;
    u8 m_hraLuckyBonus : 1;
    u8 _188_2 : 1;
    u8 m_ftrPassThru : 1;
    u8 _188_0 : 1;
    u8 _189_7 : 1; // 0x189
}; // sizeof = 0x18C

#ifdef __MWERKS__ // wchar_t is 16-bit on MWCC
typedef char BitmSizeCheck[sizeof(BITM) == 0x18C ? 1 : -1];
#endif
typedef char BinHeaderSizeCheck[sizeof(BinHeader) == 0x20 ? 1 : -1];

// Base-ID to item-index map plus per-kind index ranges.
// Global at 8059AD98; built from item.bin and the downloadable items.
class indexTable_c {
public:
    u16 getKindFirst(int kind) const; // 800C142C
    u16 getKindLast(int kind) const; // 800C145C
    int getIndexInKind(u16 index) const; // 800C148C
    u16 getIndex(u16 baseId) const; // 800C150C
    void build(); // 800C1530
    void addDlItems(); // 800C1790
    void setDlItem(u16 baseId, u16 slot); // 800C1864

    u16 mIndex[BASE_ID_COUNT]; // 0x0000
    u16 mKindFirst[KIND_COUNT]; // 0x2000
    u16 mKindLast[KIND_COUNT]; // 0x20AE
}; // sizeof = 0x215C

// kind.bin entry (one byte per kind). MWCC allocates from the top bit.
struct KindInfo {
    u8 mCheckNoPurchase : 1; // bit 7: BITM::m_noPurchase applies
    u8 mFlag6 : 1;
    u8 mFlag5 : 1;
    u8 mFlag4 : 1;
    u8 _low : 4;
};

// series.bin entry.
struct Series {
    s8 mId; // 0x00
    u8 _01[0x21];
    u8 mGroup : 4; // 0x22
    u8 _22_lo : 4;
}; // sizeof = 0x23

// npcMsg.bin entry (3 bytes) and npcMsgBullfest.bin entry (6 bytes).
struct NpcMsg {
    u8 mMsg; // 0x00 (read as s8)
    // Per-slot enables for the two message sets (A = first, B = second).
    u8 mA0 : 1; // 0x01
    u8 mB0 : 1;
    u8 mA1 : 1;
    u8 mB1 : 1;
    u8 mA2 : 1;
    u8 mB2 : 1;
    u8 mA3 : 1;
    u8 mB3 : 1;
    u8 mA4 : 1; // 0x02
    u8 mB4 : 1;
    u8 mA5 : 1;
    u8 mB5 : 1;
    u8 _02_lo : 4;
};
struct NpcMsgBullfest {
    u8 mMsg[6];
};

class infoBank_c : public dDvd::arcBank_c {
public:
    infoBank_c(); // 800C24D8
    // The implicit destructor is emitted at 800C5564.
    virtual void onLoaded(); // 800C2590

    static infoBank_c *get(); // 800C24CC
    BOOL load(void *heap); // 800C257C
    void loadDlItems(); // 800C2670
    void setDlItem(u16 baseId, u16 slot); // 800C2684
    void setPalettes(nw4r::g3d::ResFile file); // 800C2690
    const BITM *getBITM(u16 index) const; // 800C2704
    const BITM *getBITM(Item item) const; // 800C27E0
    Series *getSeries(u32 series); // 800C2870
    NpcMsg *getNpcMsg(u16 index); // 800C2890
    NpcMsgBullfest *getNpcMsgBullfest(u16 index); // 800C28C0
    KindInfo *getKindInfo(u32 kind); // 800C28EC
    u16 getIndexFromBaseId(u16 baseId) const; // 800C2910
    u16 getBaseId(u16 index) const; // 800C291C
    u16 getItemId(u16 index); // 800C2998
    u16 getItemIdFromIndex(int index); // 800C29C8
    u16 getBaseIdFromItemId(u16 id) const; // 800C29D0
    BOOL isBuiltinBaseId(u16 baseId); // 800C2A10
    BOOL isBuiltinItemId(u16 id); // 800C2A3C
    u16 getIndexFromItemId(u16 id) const; // 800C2AA8

    u8 mDlLoaded; // 0x74
    BITM *mpItems; // 0x78
    Series *mpSeries; // 0x7C
    NpcMsg *mpNpcMsg; // 0x80
    u32 mNpcMsgCount; // 0x84
    NpcMsgBullfest *mpNpcMsgBullfest; // 0x88
    u32 mNpcMsgBullfestCount; // 0x8C
    KindInfo *mpKindInfo; // 0x90
    void *mpPalettes[16]; // 0x94
}; // sizeof = 0xD4

class dsnPlttLoader_c : public dDvd::brresBank_c {
public:
    // The implicit destructor is emitted at 800C5674.
    virtual void onLoaded(); // 800C249C

    static dsnPlttLoader_c *get(); // 800C242C
}; // sizeof = 0x58

// Candidate set over item indices 0..DL_ITEM_END.
class seeker_c {
public:
    // Filters candidates during search.
    class candCB_c {
    public:
        virtual BOOL check(const BITM *bitm, Item *item); // 800C56D4
    };

    seeker_c() { memset(this, 0, sizeof(seeker_c)); }

    static seeker_c *get(); // 800C2B34
    void add(u16 index); // 800C2B40
    BOOL contains(u16 index) const; // 800C2B80
    void searchItem(const Item &item, int flags, candCB_c *cb); // 800C2BC0
    void search(int kind, int flags, candCB_c *cb); // 800C2C58
    Item getNth(u32 n) const; // 800C2E6C
    Item getRandom() const; // 800C2F34
    int find(const Item &item) const; // 800C2F9C
    int findLike(Item item); // 800C303C
    int findInSeries(Item item); // 800C3088
    u32 searchSeries(int series, int kind); // 800C3180
    Item getNthInSeries(u32 n, int series, int kind); // 800C31E8
    int findFossil(Item item); // 800C3278
    u32 searchFossil(int fossil); // 800C32E0
    Item getRandomFossil(int fossil); // 800C3330

    inline int findFromId(u16 id) {
        Item item(id);
        return findLike(item);
    }

    u8 mBits[(DL_ITEM_END + 7) / 8]; // 0x000
    u32 mCount; // 0x164
}; // sizeof = 0x168

class clothCandCB_c : public seeker_c::candCB_c {
public:
    clothCandCB_c(Item item, int style, int prevStyle, BOOL excludeNoSale); // 800C3394
    void set(Item item, int style, int prevStyle, BOOL excludeNoSale); // 800C33E8
    virtual BOOL check(const BITM *bitm, Item *item); // 800C3400

    Item mExclude; // 0x04
    int mStyle; // 0x08
    int mPrevStyle; // 0x0C
    u8 mExcludeNoSale; // 0x10
};

class musicCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item); // 800C34D0

    int mExcludeFrom; // 0x04
    int mFrom[5]; // 0x08
    u8 *mpOwned; // 0x1C
};

class fromCandCB_c : public seeker_c::candCB_c {
public:
    fromCandCB_c(int from) : mFrom(from) {}
    virtual BOOL check(const BITM *bitm, Item *item); // 800C36DC

    int mFrom; // 0x04
};

class fossilCandCB_c : public seeker_c::candCB_c {
public:
    fossilCandCB_c(int fossil) : mFossil(fossil) {}
    fossilCandCB_c(Item item); // 800C5744
    void set(Item item); // 800C3718
    virtual BOOL check(const BITM *bitm, Item *item); // 800C3784

    int mFossil; // 0x04
};

class colorCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item); // 800C37F0

    int mColor; // 0x04
};

class seriesCandCB_c : public seeker_c::candCB_c {
public:
    seriesCandCB_c(int series, Item exclude, BOOL excludeNoSale); // 800C56DC
    void set(int series, Item exclude, BOOL excludeNoSale); // 800C5730
    virtual BOOL check(const BITM *bitm, Item *item); // 800C38AC

    int mSeries; // 0x04
    Item mExclude; // 0x08
    u8 mExcludeNoSale; // 0x0A
};

class sizeCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item); // 800C3954

    int mSize; // 0x04
};

class ftrSeCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item); // 800C3984

    u32 mSfx; // 0x04
};

class newOldCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item); // 800C399C

    int mValue; // 0x04
};

class adultKiddyCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item); // 800C39DC

    int mValue; // 0x04
};

class categoryQ5CandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item); // 800C3A1C

    int mCategory; // 0x04
};

class resLoader_c : public dDvd::brresBank_c {
public:
    resLoader_c(); // 800C3B40
    virtual ~resLoader_c(); // 800C3B84
    virtual void onLoaded(); // 800C440C

    void link(u16 index); // 800C3A5C
    void unlink(); // 800C3AC4
    void reset(); // 800C3B10
    BOOL loadFromMemory(const void *src, u32 size, void *heap); // 800C3BFC
    BOOL loadDesign(void *design, void *heap, s16 width, s16 height); // 800C3CC0
    BOOL loadDesignInPlace(void *design, void *heap, s16 width, s16 height); // 800C3D30
    BOOL loadTexture(const void *tex, const void *pltt, void *heap, s16 width, s16 height); // 800C3DA0
    BOOL bindTexture(void *tex, void *pltt, void *heap, s16 width, s16 height); // 800C3F1C
    BOOL loadItem(Item item, void *heap); // 800C4054
    BOOL loadIndex(u16 index, void *heap); // 800C40B4
    BOOL release(); // 800C43B8
    s32 getDataSize(); // 800C4488
    void replaceTexture(void *tex, void *pltt, u32 texIdx, u32 plttIdx, u16 width, u16 height); // 800C44C0

    nw4r::ut::Link mLink; // 0x58
    u16 mIndex; // 0x60
    const BITM *mpBITM; // 0x64
    u16 mItemId; // 0x68 (a raw id: the constructor leaves it unset)
    u8 mFromArchive; // 0x6A
    s32 mSize; // 0x6C
}; // sizeof = 0x70

// Shared state for loaded resLoader_c objects. Global at 8059D308.
struct resList_c {
    nw4r::ut::List mList; // 0x00
    u8 mState[0xA24]; // 0x0C: per-index load state (1 = loading, 2 = loaded)
}; // sizeof = 0xA30

// Enumerates items to send over the network.
class makeSendData_c {
public:
    // Declared before the first virtual, so the vtable pointer lands at 0x08.
    u32 mCount; // 0x00
    u8 *mpExclude; // 0x04

    makeSendData_c(u32 count, u8 *exclude); // 800C45B8
    u32 countSendable(); // 800C45D0
    void *getNthSendable(u32 n); // 800C4698

    virtual Item getItem(u32 index) = 0;
};

class makePlEquipSendData_c : public makeSendData_c {
public:
    makePlEquipSendData_c(void *player) : makeSendData_c(2, NULL), mpPlayer(player) {}
    virtual Item getItem(u32 index); // 800C4760

    void *mpPlayer; // 0x0C
};

class makeToCstmSendData_c : public makeSendData_c {
public:
    makeToCstmSendData_c(u8 *exclude) : makeSendData_c(DL_ITEM_COUNT, exclude) {}
    virtual Item getItem(u32 index); // 800C48C8
};

// View over downloadable-item blocks in the save (0x2000 bytes each).
struct dlBlockList_c {
    dlBlockList_c(u8 *blocks, u32 count); // 800C4AE8
    BOOL add(void *block); // 800C4AF4
    BOOL addItem(Item item); // 800C4BC8
    void clear(); // 800C4C60
    int addFromPlayer(void *player); // 800C4C8C
    int countValid(); // 800C4DA0
    int countOwned(void *player); // 800C4E48

    u8 *mpBlocks; // 0x00
    u32 mCount; // 0x04
};

// Item name holder: a word with room for one BITM name (17 characters).
// Vtable 8049E4EC; its functions live in the TU around 8000C14C.
class name_c : public dScript::Word_c {
public:
    virtual ~name_c(); // 8000DDF4
    virtual u32 getBufferSize(); // 8000DE54: returns sizeof(mBuffer)
    virtual wchar_t *getBuffer(); // 8000DE4C

    wchar_t mBuffer[17]; // 0x24
}; // sizeof = 0x48

// Category-specific names (RTTI-only so far). Each adds nothing but its
// own destructor on top of dString::Word_c.
class nameFashion_c : public dString::Word_c {
public:
    virtual ~nameFashion_c(); // 80036274
};
class nameLook_c : public dString::Word_c {
public:
    virtual ~nameLook_c(); // 800362CC
};
class nameLookQ4_c : public dString::Word_c {
public:
    virtual ~nameLookQ4_c(); // 8003616C
};
class nameSeries_c : public dString::Word_c {
public:
    virtual ~nameSeries_c(); // 800B4D48
};
class nameSeriesQ5_c : public dString::Word_c {
public:
    virtual ~nameSeriesQ5_c(); // 8003621C
};
class nameCategoryQ5_c : public dString::Word_c {
public:
    virtual ~nameCategoryQ5_c(); // 800361C4
};

static inline int Item_getIdxInKind(const Item &item) {
    return seeker_c::get()->findLike(item);
}

static inline const dItem::BITM *getBITM(u16 id) {
    Item item(id);
    return infoBank_c::get()->getBITM(item);
}

} // namespace dItem
