#pragma once

// Field assessment (town rating): per-block counts of trees, flowers, weeds, items and so on, a
// per-block point score, and the town rank from the block scores. The City Folk counterpart of the
// GameCube Animal Crossing m_field_assessment (mFAs_*). Part of the dFgMngProc_c TU
// (src/dol/game/d_field_assessment.cpp, 80091E40..80093238); see notes/d_field_assessment.txt.
//
// All class, member and function names here are inferred (no RTTI: the classes have no virtuals).

#include <types.h>
#include <game/game/d_field_info.hpp>
#include <game/game/d_item_def.hpp>

// A unit offset packed in a byte: ((dx + 8) << 4) | (dz + 8).
#define FG_UNIT_OFS(dx, dz) ((((dx) + 8) << 4) | ((dz) + 8))

// Field layout of the usable blocks (rows are 0-based usable rows; field rows are usable + 1).
#define FG_BEACH_BLOCK_Z (FG_BLOCK_Z_NUM - 1) // 4: the beach row (shells, coconuts)
#define BEACH_BLOCK_Z (FG_BEACH_BLOCK_Z + 1)  // 5: the beach row in field coordinates
#define FG_CEDAR_BLOCK_Z_NUM 2                // the cedar rows (cedar saplings die further south)
#define FG56_BLOCK_MAX 3                      // growFg56: at most 3 decorated cedars per block
#define FG56_UNIT_NUM (FG_BLOCK_X_NUM * FG_CEDAR_BLOCK_Z_NUM * FG56_BLOCK_MAX) // 30: sFg56Units
#define FG_ALL_BLOCKS_MASK ((1 << FG_BLOCK_TOTAL_NUM) - 1) // a bit per usable block
#define FG_UNIT_MASK_NUM (FG_BLOCK_X_NUM * UT_X_NUM * FG_BLOCK_Z_NUM) // 400: sFgMngUnitMask rows

// Item counts (ranges of d_item_def.hpp).
#define FG_SHELL_KIND_NUM (dItem::ITEM_IDX_SAND_DOLLAR - dItem::ITEM_IDX_PEARL_OYSTER + 1)          // 9
#define FG_MUSHROOM_KIND_NUM (dItem::ITEM_IDX_RARE_MUSHROOM - dItem::ITEM_IDX_ELEGANT_MUSHROOM + 1) // 5
#define FG_MUSH_FTR_NUM (dItem::ITEM_IDX_MUSH_TV - dItem::ITEM_IDX_FOREST_WALL + 1)                 // 12
#define FG_GYROID_NUM (dItem::ITEM_IDX_MINI_RUSTOID - dItem::ITEM_IDX_MEGA_CLANKOID + 1)            // 127
#define FG_TOWN_MUSHROOM_NUM 5 // putMushrooms tops the town up to 5 mushrooms ...
#define FG_TOWN_MUSH_FTR_NUM 1 // ... and 1 mushroom furniture item

// Flower kinds (getFlowerKind; also the index of the falling petal / leaf effect tables).
enum dFgFlowerKind_e {
    FLOWER_KIND_TULIP,
    FLOWER_KIND_PANSY,
    FLOWER_KIND_COSMOS,
    FLOWER_KIND_ROSE,
    FLOWER_KIND_CARNATION,
    FLOWER_KIND_LILY, // Jacob's ladder (flw_lily)
    FLOWER_KIND_DANDELION,
    FLOWER_KIND_DANDELION_PUFF,

    FLOWER_KIND_NUM,
    FLOWER_KIND_NONE = FLOWER_KIND_NUM,
};

// sFgMngProcFlags (8074E33D).
enum {
    FG_MNG_PROC_FLAG_SAVE_FLAG6 = 1 << 0, // fgMngProc_setSaveFlag6 deferred it to the day change
    FG_MNG_PROC_FLAG_DAY_CHANGE = 1 << 1, // fgMngProc_updateFrame noticed 6:00 passing
    FG_MNG_PROC_FLAG_THREAD = 1 << 2,     // startFgMngProcThread
};

#define FG_MNG_THREAD_STACK_SIZE 0x3000 // startFgMngProcThread (from the "createGrowUpHeap" heap)
#define FG_MNG_BUFFER_SIZE 0x32000      // dFgMngProc_c::m_heap and mBuffer

// A unit / block position; invalid positions are (-1, -1).
struct dFdAsPos_c {
    dFdAsPos_c() {}
    dFdAsPos_c(int x, int z) : mX(x), mZ(z) {}
    BOOL isInvalid() const {
        BOOL invalid = FALSE;
        if (mX < 0 && mZ < 0) {
            invalid = TRUE;
        }
        return invalid;
    }

    /* 0x00 */ int mX;
    /* 0x04 */ int mZ;
}; // size 0x8

// Assessment of one usable block (acre): 16 x 16 units.
class dFdAsBlock_c {
public:
    // Block ranks from the point score (getBlockRank).
    enum Rank_e {
        RANK_GREAT, // > 100
        RANK_GOOD,  // 95..100
        RANK_FAIR,  // 80..94
        RANK_POOR,  // 75..79
        RANK_BAD,   // < 75
        RANK_NUM,
    };

    // calcPoint / getBlockRank.
    enum {
        POINT_BASE = 100,       // the score before penalties and bonuses
        POINT_MAX = 200,        // clamp
        TREE_NUM_MAX = 15,      // more trees: -2 per tree (REASON_TREE_MANY)
        TREE_NUM_MIN = 12,      // fewer trees: -2 per tree (REASON_TREE_FEW)
        DUST_PENALTY = 10,      // per trash / spoiled turnips
        RAFFLESIA_PENALTY = 50,
        POINT_POOR = 75,        // getBlockRank: below POINT_POOR is RANK_BAD, ...
        POINT_FAIR = 80,
        POINT_GOOD = 95,
        POINT_GREAT = 101,
    };

    void assess(dFdBase_c *fd, int blockX, int blockZ);    // 80091E40: day-change field (dFgMngProc_c)
    void assessLive(dFdBase_c *fd, int blockX, int blockZ); // 80092420: the live town field
    void calcPoint();                                       // 800928E8

    dFdAsPos_c getRafflesia() { return mRafflesia; }
    int getTreeNum() { return mTreeNum[1][1] + mTreeNum[0][1] + (mTreeNum[0][0] + mTreeNum[1][0]); }

    /* 0x00 */ int mPoint;          // 100 +/- the penalties and bonus below, 0..200
    /* 0x04 */ u8 mTreeNum[2][2];   // trees per block quarter [unitX / 8][unitZ / 8]
    /* 0x08 */ u16 mGrassNum;       // weeds (fg_grassA..D)
    /* 0x0A */ u16 mItemNum;        // items lying on the ground (not buried)
    /* 0x0C */ u16 mDustNum;        // trash and spoiled turnips, buried or not
    /* 0x0E */ u16 mFlowerNum;      // flowers, counted wilted flowers and dandelions
    /* 0x10 */ dFdAsPos_c mRafflesia; // unit of the rafflesia
    /* 0x18 */ u16 mStoneNum;       // fg_stoneA..E 0x5B..0x5F
    /* 0x1A */ u16 mCedarB56Num;    // fg id 0x56 (a fully grown cedar variant)
    /* 0x1C */ u8 mShellNum;        // shells lying on the ground
    /* 0x1D */ u8 mFossilNum;       // buried fossils
    /* 0x1E */ u8 mMushroomNum;     // mushrooms (assess only)
    /* 0x1F */ u8 mMushFtrNum;      // KIND_MUSH_FTR items lying on the ground (assess only)
    /* 0x20 */ u8 _20[4];
    /* 0x24 */ union {
        u32 mRaw;
        struct {
            u32 mHasRafflesia : 1;  // 0x80000000
            u32 mHasLily : 1;       // 0x40000000
            u32 mHasFlower : 1;     // 0x20000000
            u32 mHasTreeB : 1;      // 0x10000000: fg 0x17 / 0x54
            u32 mHasTreeA : 1;      // 0x08000000: fg 0x16 / 0x53
            u32 mHasTreeC : 1;      // 0x04000000: fg 0x18 / 0x55
            u32 mStoneKinds : 4;    // 0x03C00000: one bit per fg 0x60..0x73 group of 5
            u32 mHasCoconut : 1;    // 0x00200000: a coconut lying on the ground (assess only)
            u32 mHasTree : 1;       // 0x00100000: a grown fg 0x11..0x15 tree
            u32 mHasCedar : 1;      // 0x00080000: a cedar (0x4E..0x56) at stage 3+ (4 for assessLive)
            u32 mHasPitfall : 1;    // 0x00040000: a buried pitfall seed
            u32 mHasBadKabu : 1;    // 0x00020000: spoiled turnips lying on the ground
            u32 mHasDust : 1;       // 0x00010000: trash lying on the ground
            u32 mHasCandy : 1;      // 0x00008000: KIND_CANDY lying on the ground
            u32 _bits17 : 15;
        } mBits;
    } mFlags;
}; // size 0x28

// The block with the lowest point score and the main reason for it.
class dFdAsWorst_c {
public:
    enum Reason_e {
        REASON_NONE = -1,
        REASON_TREE_MANY,
        REASON_TREE_FEW,
        REASON_GRASS,
        REASON_ITEM,
        REASON_DUST,
        REASON_RAFFLESIA,

        REASON_NUM,
    };

    dFdAsWorst_c();  // 800929E8
    ~dFdAsWorst_c(); // 80092A14
    void set(dFdAsPos_c pos, dFdAsBlock_c *block); // 80092A54
    void setPos(dFdAsPos_c pos) { mPos = pos; }

    /* 0x00 */ dFdAsPos_c mPos; // block (0-based in the usable field)
    /* 0x08 */ int mReason;
}; // size 0xC

// Town assessment: the usable field's FG_BLOCK_X_NUM x FG_BLOCK_Z_NUM blocks.
class dFdAssess_c {
public:
    enum { PERFECT_GREAT_BLOCK_NUM = 8 }; // getTownRank: TOWN_RANK_PERFECT needs 8+ RANK_GREAT blocks

    enum TownRank_e {
        TOWN_RANK_PERFECT, // no block below RANK_FAIR and 8+ RANK_GREAT blocks
        TOWN_RANK_GOOD,
        TOWN_RANK_FAIR,    // a RANK_FAIR block
        TOWN_RANK_POOR,    // a RANK_POOR or RANK_BAD block
        TOWN_RANK_BAD,     // every block RANK_BAD
    };

    void assessTown(dFdBase_c *fd, int blockW, int blockH); // 80092B70: dFdAsBlock_c::assess
    int assessLiveTown();                                   // 80092E88: assessLive over fn_80190C44(1); mRank
    int getTownRank(int *blockRankNum, int blockW, int blockH); // 8009316C
    int getBlockRank(int point);                            // 800931D0
    void assessLiveBlock(dFdBase_c *fd, int blockX, int blockZ); // 80093218
    dFdAsBlock_c *getBlock(int blockX, int blockZ) { return &mBlocks[blockX][blockZ]; }
    dFdAsPos_c getRafflesiaBlock0() const { return mRafflesiaBlock; }
    dFdAsPos_c getRafflesiaUnit0() const { return mRafflesiaUnit; }
    dFdAsPos_c getRafflesiaBlock() const { dFdAsPos_c pos = getRafflesiaBlock0(); return pos; }
    dFdAsPos_c getRafflesiaUnit() const { dFdAsPos_c pos = getRafflesiaUnit0(); return pos; }

    /* 0x000 */ int mRank;                        // TownRank_e
    /* 0x004 */ int mBlockRankNum[dFdAsBlock_c::RANK_NUM];
    /* 0x018 */ dFdAsWorst_c mWorst;
    /* 0x024 */ u8 mLilyBlockNum;
    /* 0x025 */ u8 mFlowerBlockNum;
    /* 0x026 */ u16 mFlowerNum;
    /* 0x028 */ u16 mGrassNum;
    /* 0x02A */ u16 mTreeNum;
    /* 0x02C */ u8 mTreeABlockNum;
    /* 0x02D */ u8 mFossilNum;
    /* 0x02E */ u8 mStoneNum;
    /* 0x02F */ u8 mMushroomNum;
    /* 0x030 */ u8 mMushFtrNum;
    /* 0x032 */ union {
        u16 mRaw;
        struct {
            u16 mStoneKinds : 4; // 0xF000
            u16 mHasCoconut : 1; // 0x0800
            u16 mHasPitfall : 1; // 0x0400
            u16 mHasBadKabu : 1; // 0x0200
            u16 mHasDust : 1;    // 0x0100
            u16 mHasCandy : 1;   // 0x0080
            u16 _bits9 : 7;
        } mBits;
    } mFlags;
    /* 0x034 */ dFdAsPos_c mRafflesiaBlock;
    /* 0x03C */ dFdAsPos_c mRafflesiaUnit;
    /* 0x044 */ dFdAsBlock_c mBlocks[FG_BLOCK_X_NUM][FG_BLOCK_Z_NUM]; // [blockX][blockZ]
}; // size 0x42C

extern dFdAssess_c sFdAssess; // 8058909C

// ---------------------------------------------------------------------------------------------
// The FG manager process (dFgMngProc_c, the object at 80589080; name from "dFgMngProc_c::m_heap"):
// the day-change processing of the field. Names are inferred.

#include <game/game/d_date.hpp>

namespace EGG {
class FrmHeap;
}
namespace dItem {
struct Item;
}

// A unit position packed into two bytes (written as a 2-byte struct, read as a u16).
struct dFgMngPos8_c {
    /* 0x0 */ u8 mX;
    /* 0x1 */ u8 mZ;
}; // size 0x2

union dFgMngPos16_c {
    u16 mRaw; // (x << 8) | z
    dFgMngPos8_c mPos;
};

#define FG_MNG_LOCK_NUM 0x40

// 80589D80: 0x40 unit locks: a unit (scene, position, layer) and a bit per player who still has
// to finish its task there (bit 0 when offline).
struct dFgMngLock_c {
    /* 0x0 */ u8 mPendingPlayers : 4;
    /* 0x0 */ u8 mLayer : 1;
    /* 0x0 */ u8 _00_5 : 3;
    /* 0x1 */ u8 mScene;
    /* 0x2 */ dFgMngPos16_c mPos;
}; // size 0x4

class dFgMngProc_c {
public:
    typedef BOOL (*UnitFunc)(dFdBase_c *fd, int x, int z);

    dFgMngProc_c() : m_heap(NULL), mBuffer(NULL) {}
    ~dFgMngProc_c() {} // 800A5C84

    // 80093238..800968E0
    void destroyHeap();                                                              // 80093994
    void processDays(dTime_c *now, dTime_c *last, int days, BOOL flag, BOOL arg);    // 800939E8
    void removeDeadSaplings(dFdBase_c *fd);                                          // 80094034
    BOOL isLastTime(int hour, int min);                                              // 80094120
    void setLastTime(int hour, int min);                                             // 8009414C
    u16 getRandomShell();                                                            // 8009415C
    void putShellOnBeach(dFdBase_c *fd, int blockX, BOOL avoidPlayer);               // 800941E4
    void trySpawnShellOffline();                                                     // 800943A0
    void trySpawnShellOnline();                                                      // 800944B8
    void updateShellSpawn();                                                         // 80094658
    void fillBeachShells(dFdBase_c *fd);                                             // 800946A0
    void markFlowerUnits(dFdBase_c *fd, int w, int h);                               // 800947A4
    BOOL isFlowerOrRedKabu(dItem::Item *item);                                       // 8009488C
    void killBadSaplings(dFdBase_c *fd, int *size);                                  // 80094934
    void checkPalmSapling(dFdBase_c *fd, dItem::Item *item, int *size, int x, int z); // 80094A74
    void checkCedarSapling(dFdBase_c *fd, dItem::Item *item, int *size, int x, int z); // 80094A94
    void checkSapling(dFdBase_c *fd, dItem::Item *item, int *size, int x, int z);    // 80094AB4
    BOOL killSaplingsAround(dFdBase_c *fd, int *size, int *pos);                     // 80094C1C
    BOOL isSaplingSpaceFree(dFdBase_c *fd, int *size, int x, int z);                 // 80094C38
    void thinOutSaplings(dFdBase_c *fd, int *size);                                  // 80094C78
    void killCrowdedSaplings(dFdBase_c *fd, int *size);                              // 80094D50
    void updateTrees(dFdBase_c *fd, int w, int h);                                   // 80094E84
    void growTrees(dFdBase_c *fd, int *size);                                        // 80094F08
    void spreadWeeds(dFdBase_c *fd, int days);                                       // 800954CC
    void removeWeeds(dFdBase_c *fd);                                                 // 80095920
    void plantClovers(dFdBase_c *fd, dTime_c time, int days, int w, int h);          // 800959E8
    void plantDandelions(dFdBase_c *fd, dTime_c time, int days, int w, int h);       // 80095BE4
    void updateFlowers(dFdBase_c *fd);                                               // 80095D4C
    void updateFlowersLaterDay(dFdBase_c *fd, int day);                              // 80095F30
    u32 getWiltChance(u16 id);                                                       // 800960D4
    void plantRandomFlower(dFdBase_c *fd, int w, int h);                             // 800960F8
    void crossBreedFlowers(dFdBase_c *fd);                                           // 80096284

    // 800968E0..8009936C
    static u16 getCrossBreed(const dItem::Item *a, const dItem::Item *b);                   // 800968E0
    static BOOL putAround(dFdBase_c *fd, const dFdAsPos_c *size, const dFdAsPos_c *center, u16 item); // 80096C98
    static BOOL put(dFdBase_c *fd, const dFdAsPos_c *pos, u16 item);                        // 80096D8C
    void putLily(dFdBase_c *fd, int blockW, int blockH);                                    // 80096E34
    dFdAsPos_c findLilyBlock(dFdBase_c *fd, int blockW, int blockH);                        // 80096EA0
    void tryPutLily(dFdBase_c *fd, int blockW, int blockH);                                 // 80096FE8
    void putRafflesia(dFdBase_c *fd, int blockW, int blockH);                               // 8009708C
    void wiltRafflesia(dFdBase_c *fd);                                                      // 80097148
    void tryPutRafflesia(dFdBase_c *fd, int blockW, int blockH);                            // 80097234
    void removeTreesInWater(dFdBase_c *fd, int blockW, int blockH);                         // 800972A8
    void procEvents(dFdBase_c *fd, const dTime_c *time);                                    // 800973B0
    void endEvents(dFdBase_c *fd);                                                          // 80097568
    void procDay(dFdBase_c *fd, dTime_c *time);                                             // 80097640
    void addEvent(int id);                                                                  // 80097760
    void procLiveDay(BOOL arg);                                                             // 8009779C
    void plantTreesB(dFdBase_c *fd, int blockW, int blockH);                                // 80097848
    void plantTreesA(dFdBase_c *fd, int blockW, int blockH);                                // 80097934
    void plantTreesC(dFdBase_c *fd, int blockW, int blockH);                                // 80097A00
    void plantTrees(dFdBase_c *fd, int blockW, int blockH);                                 // 80097AA4
    void buryFossils(dFdBase_c *fd, int blockW, int blockH);                                // 80097B1C
    void buryPitfall(dFdBase_c *fd, int blockW, int blockH);                                // 80097CA4
    BOOL changeStone(dFdBase_c *fd, dFdAsBlock_c *block, int kind, int blockX, int blockZ); // 80097D84
    BOOL changeStoneOf(dFdBase_c *fd, int blockW, int blockH, int kind, int num);           // 80097EB4
    void resetStones(dFdBase_c *fd, int blockW, int blockH);                                // 80097FA4
    void changeStones(dFdBase_c *fd, int blockW, int blockH);                               // 800980B4
    void growRedKabu(dFdBase_c *fd, int num);                                               // 80098164
    void putCoconut(dFdBase_c *fd, int blockW);                                             // 80098370
    void buryGyroids(dFdBase_c *fd, int blockW, int blockH, BOOL bury);                     // 80098434
    BOOL isMushroomSeason(const dTime_c *time);                                             // 800985E4
    void putMushrooms(dFdBase_c *fd);                                                       // 80098614
    BOOL findMushroomUnit(dFdBase_c *fd, const dFdAsPos_c *block, dFdAsPos_c *out);         // 800987FC
    u16 getMushroomKind();                                                                  // 80098A40
    void makeGoldenShovels(dFdBase_c *fd, int blockW, int blockH);                              // 80098AE0
    static BOOL isGrassUnit(dFdBase_c *fd, int x, int z);                                   // 80098C10
    static BOOL isGroundUnit(dFdBase_c *fd, int x, int z);                                  // 80098C64
    static BOOL isLilyUnit(dFdBase_c *fd, int x, int z);                                    // 80098CD0
    static BOOL isRafflesiaUnit(dFdBase_c *fd, int x, int z);                               // 80098E70
    static BOOL isFlowerUnit(dFdBase_c *fd, int x, int z);                                  // 80098F6C
    static BOOL isDigUnit(dFdBase_c *fd, int x, int z);                                     // 80098F70
    static BOOL isBeachUnit(dFdBase_c *fd, int x, int z);                                   // 80098FDC
    static BOOL isSandUnit(dFdBase_c *fd, int x, int z);                                    // 80098FE0
    BOOL plantTree(dFdBase_c *fd, int blockX, int blockZ, int kind);                        // 8009901C
    BOOL putInBlock(dFdBase_c *fd, int blockX, int blockZ, u16 item, UnitFunc func, BOOL flag); // 80099170
    int collectUnits(dFdAsPos_c *out, dFdBase_c *fd, int blockX, int blockZ, UnitFunc func); // 800991FC
    int putRandom(dFdBase_c *fd, int num, const dFdAsPos_c *units, u16 item, BOOL flag);    // 800992D4

    // 8009936C..8009B47C
    int plantGrass(dFdBase_c *fd, int spotNum, dFdAsPos_c *spots, int num);                         // 8009936C
    static void setUnitItem(dFdBase_c *fd, int blockX, int blockZ, int unitX, int unitZ, u16 id, BOOL flagA); // 80099514
    static void setUnitItem(dFdBase_c *fd, int unitX, int unitZ, u16 id, BOOL flagA);               // 80099538
    static BOOL isUnitFree(int unitX, int unitZ);                                                   // 800995E0: sFgMngUnitMask bit clear
    static void clearFg94();                                                                        // 80099648
    void spoilKabu(dFdBase_c *fd);                                                                  // 80099744
    void spoilPocketKabu();                                                                         // 80099838
    void spoilTownKabu();                                                                           // 800998F0
    void spoilRoomKabu();                                                                           // 8009992C
    void spoilRecycleBinKabu();                                                                     // 80099980
    void spoilPoliceBoxKabu();                                                                      // 80099A10
    void spoilAllKabu();                                                                            // 80099AA0
    void takeEgg(int *pick, int *kind, int *blockEggs, int *total, int *kinds, int *left, int *kindNum); // 80099B88
    void updateEggs();                                                                              // 80099C0C
    void collectEggs(dFdBase_c *fd);                                                                // 80099C60
    void hideEggs(dFdBase_c *fd);                                                                   // 80099F4C
    void removeEggs(dFdBase_c *fd);                                                                 // 8009A820
    void endEggs(dFdBase_c *fd);                                                                    // 8009A938
    void updateFg56();                                                                              // 8009AB14
    void growFg56(dFdBase_c *fd, int blockX, int blockZ);                                           // 8009AC04
    void revertFg56(dFdBase_c *fd);                                                                 // 8009AD4C
    BOOL buryLamp(dFdBase_c *fd);                                                                   // 8009AF1C
    void removeLamps(dFdBase_c *fd);                                                                // 8009B13C
    static int getDirection(int angle);                                                             // 8009B200
    static void create();                                                                           // 8009B288
    static void execute();                                                                          // 8009B2DC
    static void destroy();                                                                          // 8009B30C
    static void setupUnitMask(dFdBase_c *fd, u16 *buf);                                             // 8009B360

    // the -sym on tail (d_fg_mng_task.inc)
    void createHeap();                                                                              // 800A5900

    /* 0x00 */ u8 mBusy;               // fgMngProc_isBusy / fgMngProc_setBusy / fgMngProc_clearBusy
    /* 0x01 */ u8 _01;
    /* 0x02 */ dFgMngPos16_c mLastTime; // (hour << 8) | minute
    /* 0x04 */ u8 mSaveFlag;            // fgMngProc_backupSaveFlag1A / fgMngProc_restoreSaveFlag1A: save flag 0x1A
    /* 0x05 */ u8 _05[3];
    /* 0x08 */ EGG::FrmHeap *m_heap;   // 0x32000 bytes, created by createHeap
    /* 0x0C */ void *mBuffer;          // 0x32000 bytes from m_heap
}; // size 0x10

// ---------------------------------------------------------------------------------------------
// The FG manager's functions for the rest of the game (other TUs and the RELs call these).

class mAng3_c;
struct dFgMngReq_c;

// The flags fgMngProc_attractInsects acts on (8074E344), latched from the assessment.
struct dFgMngLitterFlags_c {
    /* 0x0 */ u8 mHasDust;    // trash
    /* 0x1 */ u8 mHasBadKabu; // spoiled turnips
    /* 0x2 */ u8 mHasCandy;
}; // size 0x3

// The d_fgobj_manager callback (8074E358).
typedef void (*dFgObjCallback)(int a, int b, int c, mAng3_c *ang, int d);

extern "C" void *fn_80091AF8();                 // 80091AF8: &lbl_805FAF98 (unknown 0x30-byte object)
int fgMngProc_getGrowUpHeapSize();              // 80091B04: 0x3400, the "createGrowUpHeap" ExpHeap (thread stack)
int fgMngProc_getFgHeapSize();                  // 80091B0C: 0x2617C0, the "createFgHeap" FrmHeap (m_heap)
int fgMngProc_getElapsedDays();                 // 80091B18: days since the last processed day (at 6:00)
void *fgMngProc_getFgObjMgr();                  // 80091C24: the d_fgobj_managerNP actor (sFgObjMgr)
void fgMngProc_resetSync();                     // 80091C2C: unblocks the day change, resets the locks / commands
void fgMngTask_resetAll();                      // 80091E34: resets the 32 tasks
void fgMngProc_procDayChange(BOOL live);        // 8009B47C: the 6:00 day change of the whole save (live: also procLiveDay)
void fgMngProc_updateFrame();                   // 8009BCDC: per frame (d_s_stage): notices 6:00 passing, spawns shells
void fgMngProc_threadMain();                    // 8009BEF0: the work of the FG manager thread
void fgMngProc_collectFg56Units();              // 8009C0CC: records the fg 0x56 units of the first two block rows
void fgMngProc_initTownField();                 // 8009C244: one-off field setup (probably for a new town)
void fgMngProc_initOnCreate();                  // 8009C41C: from dFgMngProc_c::create
int fgMngProc_assessLiveTown();                 // 8009C520: sFdAssess.assessLiveTown()
dFdAsWorst_c *fgMngProc_getWorstBlock();        // 8009C52C: the lowest-rated block and its main fault (Pelly)
u16 fgMngProc_getWeedNum();                     // 8009C53C: re-assesses the live town, returns its weed count
BOOL fgMngProc_isSelfMember(int member);        // 8009C568: TRUE offline, else whether `member` is this console
BOOL fgMngProc_canEditField();                  // 8009C5B4
u16 fgMngProc_getMemberHeldItem(int member);    // 8009C638: ITEM_ID_NONE without a player
void fgMngProc_getBuriedMoneyFg(u16 *outFg, u8 *outFlag, u16 itemId); // 8009C678: money buried with the golden shovel
BOOL fgMngProc_getPlantedFg(u16 *outFg, u16 *outBase, u16 itemId, void *obj); // 8009C7FC: FALSE if it can't be planted
void fgMngProc_getBuryFg(void *obj, u16 *outFg, u16 *outBase, u8 *outFlag, u16 item); // 8009CA58: planted or buried
u8 fgMngProc_isBusy();                          // 8009CAE0: sFgMngProc.mBusy
void fgMngProc_setBusy();                       // 8009CAEC
void fgMngProc_clearBusy();                     // 8009CAFC
void fgMngProc_resetUnitState();                // 8009CB0C: offline: clears the unit flags and the fg 0x94
void fgMngProc_waterAround(int member, const dFdAsPos_c *pos); // 8009CB44: the watering can (golden / silver: 3 x 3)
dFdAsPos_c *fgMngProc_getFg56Units();           // 8009CD3C: the 30 fg 0x56 units ((-1, -1): free)
void fgMngProc_addFg56Unit(int x, int z);       // 8009CD48
void fgMngProc_clearFg56Units();                // 8009CE70
BOOL fgMngProc_isClockSetBack();                // 8009CF84: the save's time stamp lies after now
BOOL fgMngProc_getRafflesiaPos(nw4r::math::VEC3 *pos); // 8009D06C: FALSE without a rafflesia
int fgMngProc_trampleFlowerAt(int x, int z);    // 8009D0E8: d_a_player: petals (1) or, 1 in 8, destroyed (0); 3: nothing
void fgMngProc_damageFlower(dItem::Item *item, int x, int z, int mode); // 8009D1B4: mode 0 removes it
void fgMngProc_damageFlowerAt(int x, int z, int mode); // 8009D248
void fgMngProc_playFlowerFallEffect(dItem::Item *item, int x, int z, int mode, const mAng3_c *ang,
                                    BOOL wind); // 8009D2C0: the afm_*_fall / *_leaf_fall effects
BOOL fgMngProc_isUnitSpecialGround(int x, int z); // 8009D68C: ground attribute 0x17 (meaning unknown)
BOOL fgMngProc_isDayUpToDate();                 // 8009D6F4: the day change already ran today
void fgMngProc_setFgObjCallback(dFgObjCallback func); // 8009D874
void fgMngProc_callFgObjCallback(int a, int b, int c, int d); // 8009D87C
BOOL fgMngProc_findHoleInFront(int *outX, int *outZ); // 8009D8C4: with a shovel, a fg 0x94 (hole?) in front
BOOL fgMngProc_isChangeRequestValid(const dFgMngReq_c *req, int member); // 8009DB80: a request from the net
void fgMngProc_setUnitFgSync(int x, int z, u16 fg); // 8009DD40: online, also sends it (packet 0x2B)
void fgMngProc_recvUnitFg(const u16 *data);     // 8009DDD0: a received packet 0x2B
BOOL fgMngProc_canCheckDayChange();             // 8009DEE4: FALSE while fading or in some scenes
void fgMngProc_blockDayChange();                // 8009DFBC: events of special NPCs
void fgMngProc_unblockDayChange();              // 8009DFE4
dTime_c fgMngProc_getLastDayTime();             // 8009DFF0: the last processed day at 6:00
void fgMngProc_setSaveFlag6();                  // 8009E0D4: deferred while a day change is pending
BOOL fgMngProc_isSaveFlag6();                   // 8009E11C
void fgMngProc_clearFg94();                     // 8009E158: dFgMngProc_c::clearFg94()
void fgMngProc_getUnitGroundPos(nw4r::math::VEC3 *pos, int x, int z); // 8009E15C: of a town unit
void fgMngProc_getLitterFlags(dFgMngLitterFlags_c *flags); // 8009E1BC: from the assessment
void fgMngProc_attractInsects(dFgMngLitterFlags_c *flags); // 8009E238: offline outdoors (ants / flies?)
void fgMngProc_updateLitterFlags();             // 8009E2CC: re-assesses the live town (d_insect_field)
void fgMngProc_seedRandom(const dTime_c *time, int salt); // 8009E2F4: the day change's random numbers
f32 fgMngProc_rndF(f32 max);                    // 8009E308: with the day change's random numbers, else cM::rndF
void fgMngProc_backupSaveFlag1A();              // 8009E31C
void fgMngProc_restoreSaveFlag1A();             // 8009E350
void fgMngProc_stashFg94();                     // 8009E39C: removes the town's fg 0x94, remembering them
void fgMngProc_restoreFg94();                   // 8009E4D8: puts them back

// ---------------------------------------------------------------------------------------------
// Cross-breeding parameters (HostIO objects of d_field_assessment.cpp; class names from the RTTI).
// CB<Flower><A><B>HostIO_c holds the chance in percent of each flower color when parents of colors
// A and B cross-breed; the base class (declared here, its dtor is a header inline) zeroes them.

// The common view of the CB*HostIO_c objects (the cross-breeding tables point at them).
struct dFgCBRate_c {
    /* 0x00 */ void *mVtbl;
    /* 0x04 */ f32 mRate[9]; // up to 8 used
}; // size 0x28

class CBTulipBaseHostIO_c {
public:
    enum Color_e { // the afm_tulip_fall_*_st effects: rd wt ye pi vi bk
        TULIP_RED,
        TULIP_WHITE,
        TULIP_YELLOW,
        TULIP_PINK,
        TULIP_PURPLE,
        TULIP_BLACK,

        TULIP_COLOR_NUM
    };

    CBTulipBaseHostIO_c() {
        mRate[TULIP_RED] = 0.0f;
        mRate[TULIP_WHITE] = 0.0f;
        mRate[TULIP_YELLOW] = 0.0f;
        mRate[TULIP_PINK] = 0.0f;
        mRate[TULIP_PURPLE] = 0.0f;
        mRate[TULIP_BLACK] = 0.0f;
    }
    virtual ~CBTulipBaseHostIO_c() {}

    /* 0x04 */ f32 mRate[TULIP_COLOR_NUM]; // percent per child color
}; // size 0x1C

class CBTulipRedRedHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipRedRedHostIO_c() {
        mRate[TULIP_RED] = 80.0f;
        mRate[TULIP_BLACK] = 20.0f;
    }
};

class CBTulipRedWhiteHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipRedWhiteHostIO_c() {
        mRate[TULIP_RED] = 35.0f;
        mRate[TULIP_WHITE] = 35.0f;
        mRate[TULIP_PINK] = 30.0f;
    }
};

class CBTulipRedYellowHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipRedYellowHostIO_c() {
        mRate[TULIP_RED] = 40.0f;
        mRate[TULIP_YELLOW] = 40.0f;
        mRate[TULIP_PURPLE] = 20.0f;
    }
};

class CBTulipRedPinkHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipRedPinkHostIO_c() {
        mRate[TULIP_RED] = 50.0f;
        mRate[TULIP_PINK] = 50.0f;
    }
};

class CBTulipRedPurpleHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipRedPurpleHostIO_c() {
        mRate[TULIP_RED] = 50.0f;
        mRate[TULIP_PURPLE] = 50.0f;
    }
};

class CBTulipRedBlackHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipRedBlackHostIO_c() {
        mRate[TULIP_RED] = 50.0f;
        mRate[TULIP_BLACK] = 50.0f;
    }
};

class CBTulipWhiteWhiteHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipWhiteWhiteHostIO_c() {
        mRate[TULIP_WHITE] = 100.0f;
    }
};

class CBTulipWhiteYellowHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipWhiteYellowHostIO_c() {
        mRate[TULIP_WHITE] = 50.0f;
        mRate[TULIP_YELLOW] = 50.0f;
    }
};

class CBTulipWhitePinkHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipWhitePinkHostIO_c() {
        mRate[TULIP_WHITE] = 50.0f;
        mRate[TULIP_PINK] = 50.0f;
    }
};

class CBTulipWhitePurpleHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipWhitePurpleHostIO_c() {
        mRate[TULIP_WHITE] = 50.0f;
        mRate[TULIP_PURPLE] = 50.0f;
    }
};

class CBTulipWhiteBlackHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipWhiteBlackHostIO_c() {
        mRate[TULIP_WHITE] = 50.0f;
        mRate[TULIP_BLACK] = 50.0f;
    }
};

class CBTulipYellowYellowHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipYellowYellowHostIO_c() {
        mRate[TULIP_YELLOW] = 80.0f;
        mRate[TULIP_BLACK] = 20.0f;
    }
};

class CBTulipYellowPinkHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipYellowPinkHostIO_c() {
        mRate[TULIP_YELLOW] = 50.0f;
        mRate[TULIP_PINK] = 50.0f;
    }
};

class CBTulipYellowPurpleHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipYellowPurpleHostIO_c() {
        mRate[TULIP_YELLOW] = 50.0f;
        mRate[TULIP_PURPLE] = 50.0f;
    }
};

class CBTulipYellowBlackHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipYellowBlackHostIO_c() {
        mRate[TULIP_YELLOW] = 50.0f;
        mRate[TULIP_BLACK] = 50.0f;
    }
};

class CBTulipPinkPinkHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipPinkPinkHostIO_c() {
        mRate[TULIP_PINK] = 100.0f;
    }
};

class CBTulipPinkPurpleHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipPinkPurpleHostIO_c() {
        mRate[TULIP_PINK] = 50.0f;
        mRate[TULIP_PURPLE] = 50.0f;
    }
};

class CBTulipPinkBlackHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipPinkBlackHostIO_c() {
        mRate[TULIP_PINK] = 50.0f;
        mRate[TULIP_BLACK] = 50.0f;
    }
};

class CBTulipPurplePurpleHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipPurplePurpleHostIO_c() {
        mRate[TULIP_PURPLE] = 100.0f;
    }
};

class CBTulipPurpleBlackHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipPurpleBlackHostIO_c() {
        mRate[TULIP_PURPLE] = 50.0f;
        mRate[TULIP_BLACK] = 50.0f;
    }
};

class CBTulipBlackBlackHostIO_c : public CBTulipBaseHostIO_c {
public:
    CBTulipBlackBlackHostIO_c() {
        mRate[TULIP_BLACK] = 100.0f;
    }
};

class CBPansyBaseHostIO_c {
public:
    enum Color_e { // the afm_pansy_fall_*_st effects: wt ye rd vi ry bl
        PANSY_WHITE,
        PANSY_YELLOW,
        PANSY_RED,
        PANSY_PURPLE,
        PANSY_ORANGE,
        PANSY_BLUE,

        PANSY_COLOR_NUM
    };

    CBPansyBaseHostIO_c() {
        mRate[PANSY_WHITE] = 0.0f;
        mRate[PANSY_YELLOW] = 0.0f;
        mRate[PANSY_RED] = 0.0f;
        mRate[PANSY_PURPLE] = 0.0f;
        mRate[PANSY_ORANGE] = 0.0f;
        mRate[PANSY_BLUE] = 0.0f;
    }
    virtual ~CBPansyBaseHostIO_c() {}

    /* 0x04 */ f32 mRate[PANSY_COLOR_NUM]; // percent per child color
}; // size 0x1C

class CBPansyWhiteWhiteHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyWhiteWhiteHostIO_c() {
        mRate[PANSY_WHITE] = 80.0f;
        mRate[PANSY_BLUE] = 20.0f;
    }
};

class CBPansyWhiteYellowHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyWhiteYellowHostIO_c() {
        mRate[PANSY_WHITE] = 50.0f;
        mRate[PANSY_YELLOW] = 50.0f;
    }
};

class CBPansyWhiteRedHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyWhiteRedHostIO_c() {
        mRate[PANSY_WHITE] = 50.0f;
        mRate[PANSY_RED] = 50.0f;
    }
};

class CBPansyWhitePurpleHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyWhitePurpleHostIO_c() {
        mRate[PANSY_WHITE] = 50.0f;
        mRate[PANSY_PURPLE] = 50.0f;
    }
};

class CBPansyWhiteOrangeHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyWhiteOrangeHostIO_c() {
        mRate[PANSY_WHITE] = 50.0f;
        mRate[PANSY_ORANGE] = 50.0f;
    }
};

class CBPansyWhiteBlueHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyWhiteBlueHostIO_c() {
        mRate[PANSY_WHITE] = 50.0f;
        mRate[PANSY_BLUE] = 50.0f;
    }
};

class CBPansyYellowYellowHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyYellowYellowHostIO_c() {
        mRate[PANSY_YELLOW] = 100.0f;
    }
};

class CBPansyYellowRedHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyYellowRedHostIO_c() {
        mRate[PANSY_YELLOW] = 30.0f;
        mRate[PANSY_RED] = 30.0f;
        mRate[PANSY_ORANGE] = 40.0f;
    }
};

class CBPansyYellowPurpleHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyYellowPurpleHostIO_c() {
        mRate[PANSY_YELLOW] = 50.0f;
        mRate[PANSY_PURPLE] = 50.0f;
    }
};

class CBPansyYellowOrangeHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyYellowOrangeHostIO_c() {
        mRate[PANSY_YELLOW] = 50.0f;
        mRate[PANSY_ORANGE] = 50.0f;
    }
};

class CBPansyYellowBlueHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyYellowBlueHostIO_c() {
        mRate[PANSY_YELLOW] = 50.0f;
        mRate[PANSY_BLUE] = 50.0f;
    }
};

class CBPansyRedRedHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyRedRedHostIO_c() {
        mRate[PANSY_RED] = 70.0f;
        mRate[PANSY_PURPLE] = 30.0f;
    }
};

class CBPansyRedPurpleHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyRedPurpleHostIO_c() {
        mRate[PANSY_RED] = 50.0f;
        mRate[PANSY_PURPLE] = 50.0f;
    }
};

class CBPansyRedOrangeHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyRedOrangeHostIO_c() {
        mRate[PANSY_RED] = 50.0f;
        mRate[PANSY_ORANGE] = 50.0f;
    }
};

class CBPansyRedBlueHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyRedBlueHostIO_c() {
        mRate[PANSY_RED] = 50.0f;
        mRate[PANSY_BLUE] = 50.0f;
    }
};

class CBPansyPurplePurpleHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyPurplePurpleHostIO_c() {
        mRate[PANSY_PURPLE] = 100.0f;
    }
};

class CBPansyPurpleOrangeHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyPurpleOrangeHostIO_c() {
        mRate[PANSY_PURPLE] = 50.0f;
        mRate[PANSY_ORANGE] = 50.0f;
    }
};

class CBPansyPurpleBlueHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyPurpleBlueHostIO_c() {
        mRate[PANSY_PURPLE] = 50.0f;
        mRate[PANSY_BLUE] = 50.0f;
    }
};

class CBPansyOrangeOrangeHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyOrangeOrangeHostIO_c() {
        mRate[PANSY_ORANGE] = 100.0f;
    }
};

class CBPansyOrangeBlueHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyOrangeBlueHostIO_c() {
        mRate[PANSY_ORANGE] = 50.0f;
        mRate[PANSY_BLUE] = 50.0f;
    }
};

class CBPansyBlueBlueHostIO_c : public CBPansyBaseHostIO_c {
public:
    CBPansyBlueBlueHostIO_c() {
        mRate[PANSY_BLUE] = 100.0f;
    }
};

class CBCosmosBaseHostIO_c {
public:
    enum Color_e { // the afm_cosmos_fall_*_st effects: wt rd ye pi or bk
        COSMOS_WHITE,
        COSMOS_RED,
        COSMOS_YELLOW,
        COSMOS_PINK,
        COSMOS_ORANGE,
        COSMOS_BLACK,

        COSMOS_COLOR_NUM
    };

    CBCosmosBaseHostIO_c() {
        mRate[COSMOS_WHITE] = 0.0f;
        mRate[COSMOS_RED] = 0.0f;
        mRate[COSMOS_YELLOW] = 0.0f;
        mRate[COSMOS_PINK] = 0.0f;
        mRate[COSMOS_ORANGE] = 0.0f;
        mRate[COSMOS_BLACK] = 0.0f;
    }
    virtual ~CBCosmosBaseHostIO_c() {}

    /* 0x04 */ f32 mRate[COSMOS_COLOR_NUM]; // percent per child color
}; // size 0x1C

class CBCosmosWhiteWhiteHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosWhiteWhiteHostIO_c() {
        mRate[COSMOS_WHITE] = 100.0f;
    }
};

class CBCosmosWhiteRedHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosWhiteRedHostIO_c() {
        mRate[COSMOS_WHITE] = 35.0f;
        mRate[COSMOS_RED] = 35.0f;
        mRate[COSMOS_PINK] = 30.0f;
    }
};

class CBCosmosWhiteYellowHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosWhiteYellowHostIO_c() {
        mRate[COSMOS_WHITE] = 50.0f;
        mRate[COSMOS_YELLOW] = 50.0f;
    }
};

class CBCosmosWhitePinkHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosWhitePinkHostIO_c() {
        mRate[COSMOS_WHITE] = 50.0f;
        mRate[COSMOS_PINK] = 50.0f;
    }
};

class CBCosmosWhiteOrangeHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosWhiteOrangeHostIO_c() {
        mRate[COSMOS_WHITE] = 50.0f;
        mRate[COSMOS_ORANGE] = 50.0f;
    }
};

class CBCosmosWhiteBlackHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosWhiteBlackHostIO_c() {
        mRate[COSMOS_WHITE] = 80.0f;
        mRate[COSMOS_BLACK] = 20.0f;
    }
};

class CBCosmosRedRedHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosRedRedHostIO_c() {
        mRate[COSMOS_RED] = 80.0f;
        mRate[COSMOS_BLACK] = 20.0f;
    }
};

class CBCosmosRedYellowHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosRedYellowHostIO_c() {
        mRate[COSMOS_RED] = 35.0f;
        mRate[COSMOS_YELLOW] = 35.0f;
        mRate[COSMOS_ORANGE] = 30.0f;
    }
};

class CBCosmosRedPinkHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosRedPinkHostIO_c() {
        mRate[COSMOS_RED] = 50.0f;
        mRate[COSMOS_PINK] = 50.0f;
    }
};

class CBCosmosRedOrangeHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosRedOrangeHostIO_c() {
        mRate[COSMOS_RED] = 50.0f;
        mRate[COSMOS_ORANGE] = 50.0f;
    }
};

class CBCosmosRedBlackHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosRedBlackHostIO_c() {
        mRate[COSMOS_RED] = 80.0f;
        mRate[COSMOS_BLACK] = 20.0f;
    }
};

class CBCosmosYellowYellowHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosYellowYellowHostIO_c() {
        mRate[COSMOS_YELLOW] = 100.0f;
    }
};

class CBCosmosYellowPinkHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosYellowPinkHostIO_c() {
        mRate[COSMOS_YELLOW] = 50.0f;
        mRate[COSMOS_PINK] = 50.0f;
    }
};

class CBCosmosYellowOrangeHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosYellowOrangeHostIO_c() {
        mRate[COSMOS_YELLOW] = 50.0f;
        mRate[COSMOS_ORANGE] = 50.0f;
    }
};

class CBCosmosYellowBlackHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosYellowBlackHostIO_c() {
        mRate[COSMOS_YELLOW] = 80.0f;
        mRate[COSMOS_BLACK] = 20.0f;
    }
};

class CBCosmosPinkPinkHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosPinkPinkHostIO_c() {
        mRate[COSMOS_PINK] = 100.0f;
    }
};

class CBCosmosPinkOrangeHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosPinkOrangeHostIO_c() {
        mRate[COSMOS_PINK] = 50.0f;
        mRate[COSMOS_ORANGE] = 50.0f;
    }
};

class CBCosmosPinkBlackHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosPinkBlackHostIO_c() {
        mRate[COSMOS_PINK] = 80.0f;
        mRate[COSMOS_BLACK] = 20.0f;
    }
};

class CBCosmosOrangeOrangeHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosOrangeOrangeHostIO_c() {
        mRate[COSMOS_ORANGE] = 100.0f;
    }
};

class CBCosmosOrangeBlackHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosOrangeBlackHostIO_c() {
        mRate[COSMOS_ORANGE] = 80.0f;
        mRate[COSMOS_BLACK] = 20.0f;
    }
};

class CBCosmosBlackBlackHostIO_c : public CBCosmosBaseHostIO_c {
public:
    CBCosmosBlackBlackHostIO_c() {
        mRate[COSMOS_RED] = 50.0f;
        mRate[COSMOS_BLACK] = 50.0f;
    }
};

class CBRoseBaseHostIO_c {
public:
    enum Color_e { // the afm_rose_fall_*_st effects: rd wt ye pi or vi bk bl
        ROSE_RED,
        ROSE_WHITE,
        ROSE_YELLOW,
        ROSE_PINK,
        ROSE_ORANGE,
        ROSE_PURPLE,
        ROSE_BLACK,
        ROSE_BLUE,

        ROSE_COLOR_NUM,
        ROSE_GOLD = ROSE_COLOR_NUM, // getFlowerColor of the gold rose (no cross-breeding entry)
    };

    CBRoseBaseHostIO_c() {
        mRate[ROSE_RED] = 0.0f;
        mRate[ROSE_WHITE] = 0.0f;
        mRate[ROSE_YELLOW] = 0.0f;
        mRate[ROSE_PINK] = 0.0f;
        mRate[ROSE_ORANGE] = 0.0f;
        mRate[ROSE_PURPLE] = 0.0f;
        mRate[ROSE_BLACK] = 0.0f;
        mRate[ROSE_BLUE] = 0.0f;
    }
    virtual ~CBRoseBaseHostIO_c() {}

    /* 0x04 */ f32 mRate[ROSE_COLOR_NUM]; // percent per child color
}; // size 0x24

class CBRoseRedRedHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseRedRedHostIO_c() {
        mRate[ROSE_RED] = 80.0f;
        mRate[ROSE_BLACK] = 20.0f;
    }
};

class CBRoseRedWhiteHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseRedWhiteHostIO_c() {
        mRate[ROSE_RED] = 35.0f;
        mRate[ROSE_WHITE] = 35.0f;
        mRate[ROSE_PINK] = 30.0f;
    }
};

class CBRoseRedYellowHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseRedYellowHostIO_c() {
        mRate[ROSE_RED] = 35.0f;
        mRate[ROSE_YELLOW] = 35.0f;
        mRate[ROSE_ORANGE] = 30.0f;
    }
};

class CBRoseRedPinkHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseRedPinkHostIO_c() {
        mRate[ROSE_RED] = 50.0f;
        mRate[ROSE_PINK] = 50.0f;
    }
};

class CBRoseRedOrangeHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseRedOrangeHostIO_c() {
        mRate[ROSE_RED] = 50.0f;
        mRate[ROSE_ORANGE] = 50.0f;
    }
};

class CBRoseRedPurpleHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseRedPurpleHostIO_c() {
        mRate[ROSE_RED] = 80.0f;
        mRate[ROSE_PURPLE] = 20.0f;
    }
};

class CBRoseRedBlackHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseRedBlackHostIO_c() {
        mRate[ROSE_RED] = 50.0f;
        mRate[ROSE_BLACK] = 50.0f;
    }
};

class CBRoseRedBlueHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseRedBlueHostIO_c() {
        mRate[ROSE_RED] = 100.0f;
    }
};

class CBRoseWhiteWhiteHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseWhiteWhiteHostIO_c() {
        mRate[ROSE_WHITE] = 80.0f;
        mRate[ROSE_PURPLE] = 20.0f;
    }
};

class CBRoseWhiteYellowHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseWhiteYellowHostIO_c() {
        mRate[ROSE_WHITE] = 50.0f;
        mRate[ROSE_YELLOW] = 50.0f;
    }
};

class CBRoseWhitePinkHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseWhitePinkHostIO_c() {
        mRate[ROSE_RED] = 35.0f;
        mRate[ROSE_WHITE] = 35.0f;
        mRate[ROSE_PINK] = 30.0f;
    }
};

class CBRoseWhiteOrangeHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseWhiteOrangeHostIO_c() {
        mRate[ROSE_WHITE] = 50.0f;
        mRate[ROSE_ORANGE] = 50.0f;
    }
};

class CBRoseWhitePurpleHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseWhitePurpleHostIO_c() {
        mRate[ROSE_WHITE] = 50.0f;
        mRate[ROSE_PURPLE] = 50.0f;
    }
};

class CBRoseWhiteBlackHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseWhiteBlackHostIO_c() {
        mRate[ROSE_WHITE] = 50.0f;
        mRate[ROSE_BLACK] = 50.0f;
    }
};

class CBRoseWhiteBlueHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseWhiteBlueHostIO_c() {
        mRate[ROSE_WHITE] = 80.0f;
        mRate[ROSE_BLUE] = 20.0f;
    }
};

class CBRoseYellowYellowHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseYellowYellowHostIO_c() {
        mRate[ROSE_YELLOW] = 100.0f;
    }
};

class CBRoseYellowPinkHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseYellowPinkHostIO_c() {
        mRate[ROSE_YELLOW] = 50.0f;
        mRate[ROSE_PINK] = 50.0f;
    }
};

class CBRoseYellowOrangeHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseYellowOrangeHostIO_c() {
        mRate[ROSE_RED] = 35.0f;
        mRate[ROSE_YELLOW] = 35.0f;
        mRate[ROSE_ORANGE] = 30.0f;
    }
};

class CBRoseYellowPurpleHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseYellowPurpleHostIO_c() {
        mRate[ROSE_YELLOW] = 50.0f;
        mRate[ROSE_PURPLE] = 50.0f;
    }
};

class CBRoseYellowBlackHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseYellowBlackHostIO_c() {
        mRate[ROSE_YELLOW] = 80.0f;
        mRate[ROSE_BLACK] = 20.0f;
    }
};

class CBRoseYellowBlueHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseYellowBlueHostIO_c() {
        mRate[ROSE_YELLOW] = 100.0f;
    }
};

class CBRosePinkPinkHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRosePinkPinkHostIO_c() {
        mRate[ROSE_PINK] = 100.0f;
    }
};

class CBRosePinkOrangeHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRosePinkOrangeHostIO_c() {
        mRate[ROSE_PINK] = 50.0f;
        mRate[ROSE_ORANGE] = 50.0f;
    }
};

class CBRosePinkPurpleHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRosePinkPurpleHostIO_c() {
        mRate[ROSE_PINK] = 80.0f;
        mRate[ROSE_PURPLE] = 20.0f;
    }
};

class CBRosePinkBlackHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRosePinkBlackHostIO_c() {
        mRate[ROSE_PINK] = 70.0f;
        mRate[ROSE_BLACK] = 30.0f;
    }
};

class CBRosePinkBlueHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRosePinkBlueHostIO_c() {
        mRate[ROSE_PINK] = 80.0f;
        mRate[ROSE_BLUE] = 20.0f;
    }
};

class CBRoseOrangeOrangeHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseOrangeOrangeHostIO_c() {
        mRate[ROSE_ORANGE] = 100.0f;
    }
};

class CBRoseOrangePurpleHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseOrangePurpleHostIO_c() {
        mRate[ROSE_ORANGE] = 50.0f;
        mRate[ROSE_PURPLE] = 50.0f;
    }
};

class CBRoseOrangeBlackHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseOrangeBlackHostIO_c() {
        mRate[ROSE_ORANGE] = 50.0f;
        mRate[ROSE_BLACK] = 50.0f;
    }
};

class CBRoseOrangeBlueHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseOrangeBlueHostIO_c() {
        mRate[ROSE_ORANGE] = 80.0f;
        mRate[ROSE_BLUE] = 20.0f;
    }
};

class CBRosePurplePurpleHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRosePurplePurpleHostIO_c() {
        mRate[ROSE_PURPLE] = 100.0f;
    }
};

class CBRosePurpleBlackHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRosePurpleBlackHostIO_c() {
        mRate[ROSE_PURPLE] = 40.0f;
        mRate[ROSE_BLACK] = 40.0f;
        mRate[ROSE_BLUE] = 20.0f;
    }
};

class CBRosePurpleBlueHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRosePurpleBlueHostIO_c() {
        mRate[ROSE_PURPLE] = 80.0f;
        mRate[ROSE_BLUE] = 20.0f;
    }
};

class CBRoseBlackBlackHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseBlackBlackHostIO_c() {
        mRate[ROSE_RED] = 50.0f;
        mRate[ROSE_BLACK] = 50.0f;
    }
};

class CBRoseBlackBlueHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseBlackBlueHostIO_c() {
        mRate[ROSE_BLACK] = 80.0f;
        mRate[ROSE_BLUE] = 20.0f;
    }
};

class CBRoseBlueBlueHostIO_c : public CBRoseBaseHostIO_c {
public:
    CBRoseBlueBlueHostIO_c() {
        mRate[ROSE_PURPLE] = 40.0f;
        mRate[ROSE_BLACK] = 40.0f;
        mRate[ROSE_BLUE] = 20.0f;
    }
};

class CBCarnationBaseHostIO_c {
public:
    enum Color_e { // the afm_carnation_fall_*_st effects: rd pi wt
        CARNATION_RED,
        CARNATION_PINK,
        CARNATION_WHITE,

        CARNATION_COLOR_NUM
    };

    CBCarnationBaseHostIO_c() {
        mRate[CARNATION_RED] = 0.0f;
        mRate[CARNATION_PINK] = 0.0f;
        mRate[CARNATION_WHITE] = 0.0f;
    }
    virtual ~CBCarnationBaseHostIO_c() {}

    /* 0x04 */ f32 mRate[CARNATION_COLOR_NUM]; // percent per child color
}; // size 0x10

class CBCarnationRedRedHostIO_c : public CBCarnationBaseHostIO_c {
public:
    CBCarnationRedRedHostIO_c() {
        mRate[CARNATION_RED] = 100.0f;
    }
};

class CBCarnationRedPinkHostIO_c : public CBCarnationBaseHostIO_c {
public:
    CBCarnationRedPinkHostIO_c() {
        mRate[CARNATION_RED] = 40.0f;
        mRate[CARNATION_PINK] = 40.0f;
        mRate[CARNATION_WHITE] = 20.0f;
    }
};

class CBCarnationRedWhiteHostIO_c : public CBCarnationBaseHostIO_c {
public:
    CBCarnationRedWhiteHostIO_c() {
        mRate[CARNATION_RED] = 50.0f;
        mRate[CARNATION_WHITE] = 50.0f;
    }
};

class CBCarnationPinkPinkHostIO_c : public CBCarnationBaseHostIO_c {
public:
    CBCarnationPinkPinkHostIO_c() {
        mRate[CARNATION_PINK] = 100.0f;
    }
};

class CBCarnationPinkWhiteHostIO_c : public CBCarnationBaseHostIO_c {
public:
    CBCarnationPinkWhiteHostIO_c() {
        mRate[CARNATION_PINK] = 50.0f;
        mRate[CARNATION_WHITE] = 50.0f;
    }
};

class CBCarnationWhiteWhiteHostIO_c : public CBCarnationBaseHostIO_c {
public:
    CBCarnationWhiteWhiteHostIO_c() {
        mRate[CARNATION_WHITE] = 100.0f;
    }
};

// Parameter groups of the HostIO tree (no fields; only their vtables / RTTI exist).

class crossBreedTulipHostIO_c {
public:
    crossBreedTulipHostIO_c() {}
    virtual ~crossBreedTulipHostIO_c() {}
};

class crossBreedPansyHostIO_c {
public:
    crossBreedPansyHostIO_c() {}
    virtual ~crossBreedPansyHostIO_c() {}
};

class crossBreedCosmosHostIO_c {
public:
    crossBreedCosmosHostIO_c() {}
    virtual ~crossBreedCosmosHostIO_c() {}
};

class crossBreedRoseHostIO_c {
public:
    crossBreedRoseHostIO_c() {}
    virtual ~crossBreedRoseHostIO_c() {}
};

class crossBreedCarnationHostIO_c {
public:
    crossBreedCarnationHostIO_c() {}
    virtual ~crossBreedCarnationHostIO_c() {}
};

// Parameter groups of the HostIO tree (no fields; only their vtables / RTTI exist).
class crossBreedHostIO_c {
public:
    crossBreedHostIO_c() {}
    virtual ~crossBreedHostIO_c() {}
};

// Shell parameters: the weight of each of the 9 shells (dFgMngProc_c::getRandomShell) and their
// total; fgMngProc_updateFrame fills them in once from sShellWeights.
class shellHostIO_c {
public:
    shellHostIO_c() {
        mWeightTotal = 0.0f;
        for (int i = 0; i < 9; i++) {
            mWeight[i] = 0.0f;
        }
        mInit = 0;
    }
    virtual ~shellHostIO_c() {}

    /* 0x04 */ f32 mWeight[9];
    /* 0x28 */ f32 mWeightTotal;
    /* 0x2C */ u8 mInit;
}; // size 0x30
