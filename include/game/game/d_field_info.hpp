#pragma once

// Field (town map) dimensions. The City Folk counterpart of the GameCube Animal Crossing
// m_field_make.h / m_field_info.h; the macro names follow the GC ones.
//
// A block (acre) is UT_X_NUM x UT_Z_NUM units. The town is BLOCK_X_NUM x BLOCK_Z_NUM blocks
// including the border acres around it; FG_BLOCK_X_NUM x FG_BLOCK_Z_NUM of them are the usable
// field (CF has no extra rows for the train tracks / ocean like GC's 7 x 10).

#include <types.h>

// Units per block.
#define UT_BASE_NUM 16
#define UT_X_NUM UT_BASE_NUM // units per block in x
#define UT_Z_NUM UT_BASE_NUM // units per block in z
#define UT_TOTAL_NUM (UT_X_NUM * UT_Z_NUM)

// Blocks (acres), including the border acres.
#define BLOCK_X_NUM 7
#define BLOCK_Z_NUM 7
#define BLOCK_TOTAL_NUM (BLOCK_X_NUM * BLOCK_Z_NUM)

// Usable field blocks (the border acres excluded).
#define FG_BLOCK_X_NUM (BLOCK_X_NUM - 2) // 5
#define FG_BLOCK_Z_NUM (BLOCK_Z_NUM - 2) // 5
#define FG_BLOCK_TOTAL_NUM (FG_BLOCK_X_NUM * FG_BLOCK_Z_NUM)

// World size of a unit: positions are multiplied by 1 / 32 to get unit indices (fn_8006CC64)
// and by 1 / 512 for block indices (fn_8006CCD8); the same for x and z. GC uses 40.
#define mFI_UNIT_BASE_SIZE 32
#define mFI_UNIT_BASE_SIZE_F ((f32)mFI_UNIT_BASE_SIZE)
#define mFI_UT_WORLDSIZE_X mFI_UNIT_BASE_SIZE
#define mFI_UT_WORLDSIZE_Z mFI_UNIT_BASE_SIZE
#define mFI_UT_WORLDSIZE_X_F ((f32)mFI_UT_WORLDSIZE_X)
#define mFI_UT_WORLDSIZE_Z_F ((f32)mFI_UT_WORLDSIZE_Z)
#define mFI_UT_WORLDSIZE_HALF_X_F (mFI_UT_WORLDSIZE_X_F / 2.0f) // 16, a unit's centre
#define mFI_UT_WORLDSIZE_HALF_Z_F (mFI_UT_WORLDSIZE_Z_F / 2.0f)
#define mFI_BK_WORLDSIZE_BASE (mFI_UNIT_BASE_SIZE * UT_BASE_NUM) // 512
#define mFI_BK_WORLDSIZE_X mFI_BK_WORLDSIZE_BASE
#define mFI_BK_WORLDSIZE_Z mFI_BK_WORLDSIZE_BASE
#define mFI_BK_WORLDSIZE_X_F ((f32)mFI_BK_WORLDSIZE_X)
#define mFI_BK_WORLDSIZE_Z_F ((f32)mFI_BK_WORLDSIZE_Z)

// ---------------------------------------------------------------------------------------------
// Field info objects (src/dol/game/d_field_info.cpp, .text 8008BD2C..8008EDEC). Class names are
// from the RTTI; method names are inferred. See notes/d_field_info.txt.
//
// dBGCF::clmcb_c                  collision callback (vtable 804A6BA8, code around 80076688)
//   dFdBase_c                     a grid of units with per-unit data, registered with dBGCF
//     dFdInfo_c                   the town field
//     dFdInfoNpcHs_c              a villager's house room
//     dFdInfoSvMdlRm_c            the model room

#include <lib/egg/core/eggHeap.h>

struct dHomeRoom_c;

namespace dBGCF {

// Collision callback base. The real class lives elsewhere (dtor 80076690, getAttr 80076688); our weak
// copies are dropped at link.
class clmcb_c {
public:
    virtual ~clmcb_c() {}
    virtual BOOL getAttr(f32 *height, f32 *param, int *attr, int x, int z); // 0x0C: 80076688
};

} // namespace dBGCF

namespace dItem {
struct Item;
}
namespace nw4r {
namespace math {
struct VEC3;
}
}

// A block's entry in the save's or a room's block list.
struct dFdBlockId_c {
    u16 mId : 15;  // block (acre) type
    u16 mFlag : 1;
};

// Per-block (acre) data of a dFdBase_c. Its functions are around 80080DDC..800811D8.
struct dFdBlock_c {
    static u32 getAllocSize(int num, int align);                             // 80080DDC: ROUND_UP(num * 0x24, align)
    static dFdBlock_c *create(int num, EGG::Heap *heap, int align);          // 80080DF4: heap array, each constructed
    void release(EGG::Heap *heap, BOOL items, BOOL data);                    // 80080E88: frees mItems[] / mBgData
    void set(int type, dItem::Item *items0, dItem::Item *items1, u16 *flagsA, u16 *flagsB, void *bgData,
             int blockX, int blockZ, int flag, int bg);                      // 80080F38: then fn_800756F4
    BOOL hasFlag(int mask) const;                                            // 80080F80: flags of mType
    BOOL setItem(const dItem::Item *item, int unitX, int unitZ, int layer);  // 80080FC8
    dItem::Item *getItem(int unitX, int unitZ, int layer) const;             // 800810CC
    BOOL setFlagA(int unitX, int unitZ);                                     // 800810D0
    BOOL clearFlagA(int unitX, int unitZ);                                   // 800810E8
    BOOL isFlagA(int unitX, int unitZ) const;                                // 80081100
    BOOL setFlagB(int unitX, int unitZ);                                     // 80081160
    BOOL isFlagB(int unitX, int unitZ) const;                                // 80081178
    void clearFlagsB();                                                      // 800811D8

    /* 0x00 */ int mType;              // block (acre) type, or the room's BG id
    /* 0x04 */ dItem::Item *mItems[2]; // 16 x 16 items per layer
    /* 0x0C */ u16 *mFlagsA;           // one u16 bit row per unit z
    /* 0x10 */ u16 *mFlagsB;
    /* 0x14 */ void *mBgData;          // 0xA00 bytes loaded by fn_80069680
    /* 0x18 */ int mBlockX;
    /* 0x1C */ int mBlockZ;
    /* 0x20 */ int mFlag;              // the save's block flag
}; // size 0x24

// Per-unit attribute map (r3 of 80167CD8..80167DE4; class not recovered).
class dFdUnitAttr_c {
public:
    u32 getAttr(int unitX, int unitZ);   // 80167CD8
    BOOL isOpen(int unitX, int unitZ);   // 80167D8C: (attr & 5) == 0
    BOOL isOpen1(int unitX, int unitZ);  // 80167DB8: (attr & 1) == 0
    BOOL isAttr2(int unitX, int unitZ);  // 80167DE4: attr bit 1
};

class dFdBase_c : public dBGCF::clmcb_c {
public:
    enum Kind_e {
        KIND_FIELD = 0,
        KIND_NPC_HOUSE = 3,
        KIND_MODEL_ROOM = 4,
    };

    dFdBase_c()
        : mBlocks(NULL), mBlockW(0), mBlockH(0), mUnitW(0), mUnitH(0), mWorldW(0.0f), mWorldH(0.0f), mBg(0),
          _24(NULL) {}
    virtual ~dFdBase_c() {}                                                              // 8008ECE4
    virtual BOOL getAttr(f32 *height, f32 *param, int *attr, int x, int z);              // 8008D3A0
    virtual void release(EGG::Heap *heap) {}                                             // 8008ED24
    virtual int getKind() = 0;

    // Functions used across the TU (names fixed; more members follow in the cpp's order).
    BOOL setup(int blockW, int blockH, int bg);                                          // 8008BD2C
    void clearWorldSize();                                                               // stripped
    BOOL isValid(u32 blockX, u32 blockZ, u32 unitX, u32 unitZ);                          // 8008BDC0
    dFdBlock_c *getBlock(int blockX, int blockZ);                                        // 8008C16C
    const dFdBlock_c *getBlock(int blockX, int blockZ) const;                            // 8008C1E8
    dItem::Item *getItem(int blockX, int blockZ, int unitX, int unitZ, int layer) const; // 8008C7F4
    dItem::Item *getItem(int unitX, int unitZ, int layer) const;                         // 8008C850
    static void getUnitCenterPos(nw4r::math::VEC3 *pos, int unitX, int unitZ);          // 8008BED0
    int bgCall_80072D54(int unitX, int unitZ) const;                                     // 8008D2F0
    int bgCall_80072F80(int unitX, int unitZ) const;                                     // 8008D348
    int bgCall_80076260(int unitX, int unitZ);                                           // 8008CDF4
    void setBorderUnits();                                                               // 8008CE4C: bgCall_80076260 on the border ring

    static void posToBlockUnit(int *blockX, int *blockZ, int *unitX, int *unitZ, const nw4r::math::VEC3 *pos); // 8008BDF8
    static void snapToUnitCenter(nw4r::math::VEC3 *out, const nw4r::math::VEC3 *pos);  // 8008BE54
    static void getUnitCenterPos(nw4r::math::VEC3 *pos, int blockX, int blockZ, int unitX, int unitZ); // 8008BEBC
    static void snapToUnitGround(nw4r::math::VEC3 *out, const nw4r::math::VEC3 *pos);  // 8008BF30
    static void getUnitGroundPos(nw4r::math::VEC3 *pos, int blockX, int blockZ, int unitX, int unitZ); // 8008BF78
    static void getUnitGroundPos(nw4r::math::VEC3 *pos, int unitX, int unitZ);          // 8008BF8C
    static void getBlockCenterPos(nw4r::math::VEC3 *pos, int blockX, int blockZ);       // 8008C010
    static void getUnitPos(nw4r::math::VEC3 *pos, int blockX, int blockZ, int unitX, int unitZ); // 8008C0AC
    dFdBlock_c *findBlock(int mask);                                                     // 8008C264
    const dFdBlock_c *findBlock(int mask) const;                                         // 8008C300
    BOOL hasBlockFlag(int blockX, int blockZ, int mask) const;                           // 8008C39C
    BOOL getRaccoSpot(int *unitX, int *unitZ, s16 *angle);                               // 8008C3E0
    static f32 getRaccoDist(u32 idx);                                                    // 8008C4E4
    static u32 getOtherRaccoIdx(u32 idx);                                                // 8008C518
    static f32 getRaccoParamA();                                                         // 8008C570
    static f32 getRaccoParamB();                                                         // 8008C578
    nw4r::math::VEC3 getRaccoPos(int blockX, int blockZ) const;                          // 8008C580
    BOOL isBlockVariant(int blockX, int blockZ) const;                                   // 8008C674
    void clearUnknownItems();                                                            // 8008C6C8
    dItem::Item *getItem(const nw4r::math::VEC3 *pos, int layer) const;                  // 8008C878
    BOOL setItem(const dItem::Item *item, int blockX, int blockZ, int unitX, int unitZ, int layer); // 8008C8F4
    BOOL setItem(const dItem::Item *item, int unitX, int unitZ, int layer);              // 8008C970
    BOOL setItem(const dItem::Item *item, const nw4r::math::VEC3 *pos, int layer);       // 8008C998
    BOOL setFlagA(int blockX, int blockZ, int unitX, int unitZ);                         // 8008CA24
    BOOL setFlagA(int unitX, int unitZ);                                                 // 8008CA78
    BOOL clearFlagA(int blockX, int blockZ, int unitX, int unitZ);                       // 8008CA9C
    BOOL clearFlagA(int unitX, int unitZ);                                               // 8008CAF0
    BOOL isFlagA(int blockX, int blockZ, int unitX, int unitZ) const;                    // 8008CB14
    BOOL isFlagA(int unitX, int unitZ) const;                                            // 8008CB68
    BOOL isFlagA(const nw4r::math::VEC3 *pos) const;                                     // 8008CB8C
    BOOL setFlagB(int blockX, int blockZ, int unitX, int unitZ);                         // 8008CBF8
    BOOL setFlagB(int unitX, int unitZ);                                                 // 8008CC4C
    BOOL isFlagB(int blockX, int blockZ, int unitX, int unitZ) const;                    // 8008CC70
    BOOL isFlagB(int unitX, int unitZ) const;                                            // 8008CCC4
    void clearFlagsB();                                                                  // 8008CCE8
    int bgCall_80073D2C(void *arg, int unitX, int unitZ, int arg2);                      // 8008CD5C
    int bgCall_80073158(int unitX, int unitZ) const;                                     // 8008CEE4
    int bgCall_80073158(const nw4r::math::VEC3 *pos);                                    // 8008CF6C
    int bgCall_80073208(int unitX, int unitZ) const;                                     // 8008CFB4
    int bgCall_800733F0(int unitX, int unitZ) const;                                     // 8008D03C
    int bgCall_80073260(int unitX, int unitZ) const;                                     // 8008D0C4
    int bgCall_800732B8(int unitX, int unitZ) const;                                     // 8008D14C
    int bgCall_800732B8(const nw4r::math::VEC3 *pos);                                    // 8008D1D4
    int bgCall_80073314(int unitX, int unitZ) const;                                     // 8008D21C
    int bgCall_80073314(int blockX, int blockZ, int unitX, int unitZ);                   // 8008D2DC

    int bgCall_80072D54(const nw4r::math::VEC3 *pos);                                    // 8008D610
    void fn_8008D658(int unitX, int unitZ, int a, int b);                                // 8008D658
    void fn_8008D6B4(const nw4r::math::VEC3 *pos, int a, int b);                         // 8008D6B4
    void fn_8008D70C(const nw4r::math::VEC3 *pos, int a);                                // 8008D70C
    u8 fn_8008D760(const nw4r::math::VEC3 *pos, BOOL onlyAttr16);                        // 8008D760
    dItem::Item getUnitItem(int unitX, int unitZ) const;                                 // 8008D858
    void registerBg();                                                                   // 8008D99C
    static int getHomeRoomBg(u32 home, int room);                                        // 8008E054: 0x10 if none
    BOOL buildHomeRoom(u32 home, int room, EGG::Heap *heap);                             // 8008E0B8
    void reloadHomeRoom(u32 home, int room);                                             // 8008E238
    static int getBgDataSize();                                                          // 8008E2D0

    /* 0x04 */ dFdBlock_c *mBlocks; // mBlockW * mBlockH
    /* 0x08 */ int mBlockW;
    /* 0x0C */ int mBlockH;
    /* 0x10 */ int mUnitW;   // mBlockW * UT_X_NUM
    /* 0x14 */ int mUnitH;   // mBlockH * UT_Z_NUM
    /* 0x18 */ f32 mWorldW;  // mBlockW * block world size
    /* 0x1C */ f32 mWorldH;
    /* 0x20 */ int mBg;      // dBGCF slot (fn_80075658 / fn_800755A0 / fn_8007589C)
    /* 0x24 */ dFdUnitAttr_c *_24; // optional per-unit attributes
}; // size 0x28

// 80190C44: the field info of the current scene (0) or of the town (1).
extern "C" dFdBase_c *fn_80190C44(int idx);
enum {
    FD_ID_CURRENT = 0,
    FD_ID_TOWN = 1,
};

// Scenes (fn_80162548: the current one) and their attribute masks (fn_80162594(scene, mask):
// (sSceneAttr[scene] & mask) == mask, the 0x44-entry table 80479BE0).
#define SCENE_NUM 0x44
#define SCENE_ATTR_TOWN 0x05          // outdoors (scenes 0, 0x38, 0x42)
#define SCENE_ATTR_PLAYER_HOUSE 0x450 // the 16 player house scenes 1..0x10

class dFdInfo_c : public dFdBase_c {
public:
    virtual ~dFdInfo_c() {}                                                              // 8008ED80
    virtual void release(EGG::Heap *heap);                                               // 8008DC84
    virtual int getKind() { return KIND_FIELD; }                                         // 8008ED78

    BOOL create(const dFdBlockId_c *blockIds, int blockW, int blockH, EGG::Heap *heap,
                dFdUnitAttr_c *unitAttr);                                                // 8008DA44
    static u32 getHeapSize();                                                            // 8008DD3C: the town (7 x 7)
    BOOL createTown(EGG::Heap *heap);                                                    // 8008DD70
    BOOL updateTown();                                                                   // 8008DEF4
    static u32 getRoomHeapSize();                                                        // 8008E024: one block
};

class dFdInfoNpcHs_c : public dFdBase_c {
public:
    static void *operator new(size_t size, void *p) { return p; }

    virtual ~dFdInfoNpcHs_c() {}                                                         // 8008ED38
    virtual int getKind() { return KIND_NPC_HOUSE; }                                     // 8008ED30

    static void *getBgData(int bgId);                                                    // 8008E2D8: shared, loaded once
    static BOOL incCount();                                                              // 8008E354
    static BOOL decCount();                                                              // 8008E37C
    static BOOL isCountZero();                                                           // 8008E3A0
    BOOL build(int animalIdx, EGG::Heap *heap);                                          // 8008E3B0
    void destroy(EGG::Heap *heap);                                                       // 8008E614
    static dFdInfoNpcHs_c *create(int animalIdx, EGG::Heap *heap);                       // 8008E6A0
    static void remove(dFdInfoNpcHs_c **info, EGG::Heap *heap);                          // 8008E798
};

class dFdInfoSvMdlRm_c : public dFdBase_c {
public:
    static void *operator new(size_t size, void *p) { return p; }

    dFdInfoSvMdlRm_c();                                                                  // 8008E818
    virtual ~dFdInfoSvMdlRm_c();                                                         // 8008E854
    virtual int getKind() { return KIND_MODEL_ROOM; }                                    // 8008ED28

    BOOL build(dHomeRoom_c *room, int bgId, EGG::Heap *heap);                            // 8008E894
    void destroy(EGG::Heap *heap);                                                       // 8008E9DC
    static dFdInfoSvMdlRm_c *create(dHomeRoom_c *room, int bgId, EGG::Heap *heap);       // 8008EA5C
    static dFdInfoSvMdlRm_c *createWithBg0(dHomeRoom_c *room, EGG::Heap *heap);          // 8008EB1C
    static dFdInfoSvMdlRm_c *createWithBg15(dHomeRoom_c *room, EGG::Heap *heap);         // 8008EB28
    static void remove(dFdInfoSvMdlRm_c **info, EGG::Heap *heap);                        // 8008EB34
};
