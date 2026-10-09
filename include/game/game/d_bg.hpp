#pragma once
#include <types.h>
#include <game/game/d_dvd.hpp>

// Background (field model) resources, namespace dBG (src/dol/game/d_bg.cpp, .text
// 800684A4..80069D2C; see notes/d_bg.txt). The class names come from the RTTI
// ("dBG::bkLoader_c", "dBG::packBank_c", "dBG::alwaysBank_c", "dBG::roomAlwaysBank_c",
// "dBG::grassBank_c") and the heap name ("dBG::mdlBank_c::m_heap_p"); everything else is inferred.
//
// - mdlBank_c: one bkLoader_c per block of the field, each loading the block's model
//   ("/BgData/BgModel/%03d_%d.brres"). The blocks around the player are loaded first.
// - packBank_c: the season's grass/pattern brres; its grass texture is the footmark texture.
// - alwaysBank_c / roomAlwaysBank_c: archives kept loaded (the block collision data
//   "always/blockCol.bin" used by dBGCF, the shadow, the room gradient and sun animation).
// - grassBank_c: each block type's starting grass wear texture (used by dFootmark::Editor_c).

class mVec3_c;
namespace EGG {
class Heap;
class FrmHeap;
} // namespace EGG
namespace dBGCF {
struct unitDat_c;
}

namespace dBG {

// "sound.bsd" from the External folder of a block model's brres: points of the block, each with an
// id and a position relative to the block's corner.
class bsd_c {
public:
    struct point_s {
        s16 mId;
        s16 mX;
        s16 mZ;
    };
    struct data_s {
        u16 mNum;
        point_s mPoints[1];
    };

    bsd_c() : mpData(NULL) {}

    int getNum() const;                                                       // 800684A4
    // The point's position in the block; its id, or 0xFFFF if idx is out of range.
    int getPoint(f32 *x, f32 *z, u32 idx) const;                              // 800684C0
    // The same in world coordinates for the block (blockX, blockZ).
    int getWorldPoint(f32 *x, f32 *z, int blockX, int blockZ, u32 idx) const; // 80068574

    /* 0x0 */ data_s *mpData;
}; // size 0x4

// The model of one block (acre) of the field.
class bkLoader_c : public dDvd::brresBank_c {
public:
    enum {
        KIND_FIRST = 't1st',  // in view of the player: loaded first
        KIND_SECOND = 't2nd', // the other blocks
        KIND_NONE = -1,
        ID_NONE = 0xFFFF,
    };

    bkLoader_c() { init(); }
    virtual ~bkLoader_c();   // 80069998
    virtual void onLoaded(); // 80068AA8: binds the brres, colours the grass for the season, sets the texture matrix

    void init();                                                             // 800688E0
    BOOL set(int id, u32 kind, int blockX, int blockZ, u8 variant);          // 8006890C: only once after init
    void setTexMtx(nw4r::g3d::ResFile res, int blockX, int blockZ);          // 80068940
    BOOL load(void *heap, u32 kind, bkLoader_c *same);                       // 80068BD0: if mKind is kind (KIND_NONE: any)
    BOOL unload();                                                           // 80068C90
    bsd_c *getBsd();                                                         // 80068CDC: NULL if the brres has none

    bool isLoaded() const { return getData() != NULL; }

    /* 0x58 */ u32 mKind;      // KIND_*
    /* 0x5C */ int mId;        // the BgModel number (the block type), ID_NONE if unused
    /* 0x60 */ int mBlockX;
    /* 0x64 */ int mBlockZ;
    /* 0x68 */ int _68;        // set to -1 by init, not read here
    /* 0x6C */ u8 mRequested;  // load was called
    /* 0x6D */ u8 mVariant;    // bit 0: the "_1" model (dFdBlock_c::mFlag)
    /* 0x70 */ bsd_c mBsd;
}; // size 0x74

// The models of the field's blocks (one bkLoader_c per block) and their heap.
class mdlBank_c {
public:
    mdlBank_c() { init(); }

    void init();                                    // 80068D4C
    bkLoader_c *find(int id, u32 variant);          // 80068D64: a requested loader of the same model
    BOOL create(EGG::Heap *heap);                   // 80068DD8: the heap and a loader per block of the town
    BOOL load(u32 kind);                            // 80068F58: requests every block's model of the kind
    void onAllLoaded();                             // 80069208
    BOOL destroy();                                 // 8006920C
    bkLoader_c *getLoader(u32 blockX, u32 blockZ);  // 800692C4: NULL unless loaded
    BOOL isReady();                                 // 80069308: always TRUE outside the town

    /* 0x0000 */ bkLoader_c mLoaders[7][7];
    /* 0x1634 */ EGG::FrmHeap *m_heap_p;
    /* 0x1638 */ u8 mAllLoaded;
    /* 0x1639 */ u8 mBlockW;
    /* 0x163A */ u8 mBlockH;
}; // size 0x163C

// The grass textures: one brres with a 16 x 16 grass wear texture per block type.
class grassBank_c : public dDvd::brresBank_c {
public:
    grassBank_c() {}
    virtual ~grassBank_c() {}

    BOOL load(void *heap);     // 8006934C: bank_c::load of the grass brres
    u8 *getTexImage(int type); // 8006935C: the texel data of the block type's texture, NULL if not loaded
};

// The season's brres of the town's grass type ("/BgData/Pack/pat%dseason%02d.brres").
class packBank_c : public dDvd::brresBank_c {
public:
    packBank_c();            // 800693BC
    virtual ~packBank_c();   // 800699F8
    virtual void onLoaded(); // 80069514: points the "grassBrd" texture at the footmark texture

    BOOL load(void *heap); // 80069400
    BOOL unload();         // 800694C8

    /* 0x58 */ int mSeason; // getCurrentSeasonBres when loaded, -1 if not
}; // size 0x5C

// "/BgData/Always/always.arc": the block collision data and the shadow.
class alwaysBank_c : public dDvd::arcBank_c {
public:
    alwaysBank_c() {}
    virtual ~alwaysBank_c(); // 80069A58
    virtual void onLoaded(); // 80069724

    BOOL load(void *heap);                    // 80069670
    BOOL copyBlockCol(void *dst, int type);   // 80069680: the block type's unit data (0xA00 bytes)
    dBGCF::unitDat_c *getBlockCol(int type);  // 800696D4
};

// "/BgData/Always/roomAlways.arc": the rooms' gradient texture and sun animation.
class roomAlwaysBank_c : public dDvd::arcBank_c {
public:
    roomAlwaysBank_c() {}
    virtual ~roomAlwaysBank_c(); // 80069AB8
    virtual void onLoaded();     // 800695CC

    BOOL load(void *heap); // 800695BC
};

// A rectangle of blocks (inclusive).
class area_c {
public:
    void set(const mVec3_c *center);                     // 80069794: the blocks in view around center
    void setView();                                      // 80069834
    BOOL isIn(int blockX, int blockZ) const;             // 8006989C
    u8 getPriority(int blockX, int blockZ, const mVec3_c *pos) const; // 800698DC

    /* 0x0 */ int mMinX;
    /* 0x4 */ int mMinZ;
    /* 0x8 */ int mMaxX;
    /* 0xC */ int mMaxZ;
}; // size 0x10

// 807503F0: 56, one block level up (dBGCF::blockInfo_c); d_scene puts the town's building exits there.
extern const f32 cGroundY;

mdlBank_c *getMdlBank();               // 8006996C
alwaysBank_c *getAlwaysBank();         // 80069978
packBank_c *getPackBank();             // 80069984
roomAlwaysBank_c *getRoomAlwaysBank(); // 80069990: always NULL

} // namespace dBG
