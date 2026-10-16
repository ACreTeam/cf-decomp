#ifndef D_FG_DRAW_HPP
#define D_FG_DRAW_HPP

#include <string.h>
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/mLib/m_mtx.hpp>

// Names other than the three RTTI class names are inferred from behavior.
namespace dFgDraw {
enum {
    DESIGN_COUNT = 32,
    BLOCK_COUNT = 7,
    UNITS_PER_BLOCK = 16,
    DRAW_CAPACITY = 640,
    HIDDEN_CAPACITY = 20
};
inline f32 getCullRadius(u8 kind) {
    static const f32 radii[] = {0.0f, 38.4f, 51.2f};
    return radii[kind];
}

enum DrawKind { DRAW_MODEL, DRAW_TEXTURE };
enum CullKind { CULL_NONE, CULL_TEXTURE, CULL_MODEL };
enum DrawFlag {
    EXPLICIT_POS = 1, TREE = 2, HIDDEN = 4, FLAT_TEXTURE = 8,
    EXPLICIT_MTX = 16, PRESENT = 32, SPECIAL_GROUND = 64
};

class designLoaders_c {
public:
    nw4r::g3d::ResTex getTexture(int index) { return nw4r::g3d::ResFile(mLoaders[index].getData()).GetResTex(0); }
    nw4r::g3d::ResPltt getPalette(int index) { return nw4r::g3d::ResFile(mLoaders[index].getData()).GetResPltt(0); }
    BOOL load(void *heap); // .text 0148
    BOOL release(); // .text 01D4
    /* 0x0000 */ dItem::resLoader_c mLoaders[DESIGN_COUNT];
}; // size 0xE00

class blockMask_c {
public:
    blockMask_c() { memset(mRows, 0, sizeof(mRows)); }
    int test(int x, int z); // .text 025C
    void set(int x, int z); // .text 0274
    void clear(int x, int z); // .text 0294
    /* 0x00 */ u16 mRows[UNITS_PER_BLOCK];
}; // size 0x20

class fieldMask_c {
public:
    fieldMask_c() { reset(); }
    void reset(); // .text 02B4
    int test(int x, int z); // .text 0330
    void set(int x, int z); // .text 0380
    void clear(int x, int z); // .text 03CC
    /* 0x000 */ blockMask_c mBlocks[BLOCK_COUNT][BLOCK_COUNT];
}; // size 0x620

class entry_c {
public:
    static void *operator new(size_t, void *ptr) { return ptr; }
    entry_c() : mModel(NULL), mFruitModel(NULL), mTexture(NULL), mPalette(NULL) { reset(); }
    bool hasShadow() const { return mShadowRadius != 0.0f; }
    bool isSpecialGround() const { return (mFlags & SPECIAL_GROUND) != 0; }
    int getUnitX() const { return mUnitX; }
    int getUnitZ() const { return mUnitZ; }
    void reset(); // .text 08DC
    void restoreVisibility(); // .text 0948
    void setupResources(); // .text 09B4
    BOOL init(const dItem::Item &item, int x, int z, int kind, BOOL upper); // .text 103C
    BOOL initAt(u16 item, const mVec3_c &pos, const mVec3_c &scale, BOOL shadow,
                BOOL present, const mAng3_c &rot); // .text 1144
    BOOL initFlat(u16 item, const mVec3_c &pos, const mVec3_c &scale); // .text 12BC
    BOOL initAtUnit(u16 item, const mVec3_c &pos, int x, int z, const mVec3_c &scale); // .text 1314
    BOOL initWithMtx(u16 item, const mVec3_c &pos, const mMtx_c &mtx); // .text 13EC
    BOOL isNeeded(int x, int z, int height); // .text 152C
    BOOL release(); // .text 1784
    void drawModel(const mMtx_c &view); // .text 17F0
    void drawTexture(const mMtx_c &view); // .text 1DA4
    BOOL draw(const mMtx_c &view, int kind); // .text 2184
    const char *getSoldOutTexture(int language); // .text 22B4
    const char *getGraceSoldOutTexture(int language); // .text 2334

    /* 0x00 */ nw4r::ut::Link mLink;
    /* 0x08 */ int mKind;
    /* 0x0C */ nw4r::g3d::ResMdl mModel;
    /* 0x10 */ nw4r::g3d::ResMdl mFruitModel;
    /* 0x14 */ mVec3_c mPos;
    /* 0x20 */ mVec3_c mScale;
    /* 0x2C */ mAng3_c mRot;
    /* 0x32 */ u8 _0032[2];
    /* 0x34 */ f32 mCullRadius;
    /* 0x38 */ f32 mShadowRadius;
    /* 0x3C */ f32 mShadowScale;
    /* 0x40 */ int mShadowKind;
    /* 0x44 */ u8 mFlags;
    /* 0x45 */ s8 mUnitX;
    /* 0x46 */ s8 mHeight;
    /* 0x47 */ s8 mUnitZ;
    /* 0x48 */ u8 mCullKind;
    /* 0x49 */ u8 _0049;
    /* 0x4A */ u16 mItem;
    /* 0x4C */ nw4r::g3d::ResTex mTexture;
    /* 0x50 */ nw4r::g3d::ResPltt mPalette;
    /* 0x54 */ s16 mTexWidth;
    /* 0x56 */ s16 mTexHeight;
    /* 0x58 */ mMtx_c mMtx;
}; // size 0x88

class manager_c {
public:
    BOOL add(u16 item, const mVec3_c &pos, const mVec3_c &scale, BOOL shadow,
             BOOL present, const mAng3_c &rot); // .text 23B4
    BOOL addWithMtx(u16 item, const mVec3_c &pos, const mMtx_c &mtx); // .text 2460
    BOOL addFlat(u16 item, const mVec3_c &pos, const mVec3_c &scale); // .text 24F4
    BOOL addAtUnit(u16 item, const mVec3_c &pos, int x, int z, const mVec3_c &scale); // .text 2588
    BOOL init(entry_c *entries, int count, bool drawFurniture); // .text 262C
    BOOL execute(); // .text 26C8
    BOOL draw(const mMtx_c &view, int kind); // .text 296C
    BOOL release(); // .text 29E8
    BOOL setHidden(int x, int z, BOOL hidden); // .text 29F0
    void insert(nw4r::ut::List *list, entry_c *entry); // .text 2AA4
    /* 0x000 */ bool mDrawFurniture;
    /* 0x004 */ nw4r::ut::List mFree;
    /* 0x010 */ nw4r::ut::List mActive;
    /* 0x01C */ fieldMask_c mUsed;
}; // size 0x63C

struct hiddenUnit_c {
    void reset(); // .text 38F0
    bool isAt(int x, int z) const { return x == mX && z == mZ; }
    /* 0x0 */ s8 mX;
    /* 0x1 */ s8 mZ;
    /* 0x2 */ s8 mRefs;
}; // size 3

void *create(); // .text 00CC
int getCullKind(const dItem::Item &item); // .text 0418
BOOL inModelRange(int x, int z, int unitX, int unitZ, int height, int unitHeight); // .text 0490
BOOL inTextureRange(int x, int z, int unitX, int unitZ, int height, int unitHeight); // .text 04D4
BOOL inNoRange(int, int, int, int, int, int); // .text 0518
BOOL inRange(int x, int z, int unitX, int unitZ, u8 kind, int height, int unitHeight); // .text 0520
void setupTexture(const mMtx_c &view, nw4r::g3d::ResTex tex, nw4r::g3d::ResPltt pltt,
                  int width, int height); // .text 0540
void calcTexCoord(mVec3_c *pos, f32 *s, f32 *t, const mVec3_c &origin, mVec3_c scale); // .text 0804
dItem::Item getUnitItem(int x, int z, int *height, BOOL *upper, BOOL filterFurniture); // .text 1608
void add(u16 item, const mVec3_c &pos, const mVec3_c &scale, BOOL shadow, BOOL present,
         const mAng3_c &rot); // .text 2B2C
BOOL addWithMtx(u16 item, const mVec3_c &pos, const mMtx_c &mtx); // .text 2B88
BOOL addFlat(u16 item, const mVec3_c &pos, const mVec3_c &scale); // .text 2BA8
BOOL addAtUnit(u16 item, const mVec3_c &pos, int x, int z, const mVec3_c &scale); // .text 2BC8
BOOL setHidden(int x, int z, BOOL hidden); // .text 2BF8
mVec3_c *getFruitOffset(u32 index, int kind); // .text 2C18
void getScatterOffset(mVec3_c *pos, int x, int z); // .text 2E40
void getItemPosition(mVec3_c *pos, u16 item, int x, int z, BOOL upper); // .text 2E84
BOOL isHidden(int x, int z); // .text 3908
int hideUnit(int x, int z); // .text 39B8
void showUnit(int slot); // .text 3AD4
void getModelName(const dItem::Item &item, int x, int z, char *name); // .text 3B40
void appendTreeVariant(const dItem::Item &item, int x, int z, char *name); // .text 3BB8
hiddenUnit_c *getHiddenUnit(int slot); // .text 3C3C
extern manager_c sManager;
}

// RTTI explicitly places these classes at global scope.
class dFgDraw_c : public fBase_c, public m3d::proc_c {
public:
    void bindTreeResources(); // .text 2F9C
    virtual int create(); // .text 36BC
    virtual int execute(); // .text 3820
    virtual int draw(); // .text 3828
    virtual int doDelete(); // .text 3864
    virtual ~dFgDraw_c(); // .text 4064
    virtual void drawOpa(); // .text 35A8 (thunk at 4208)
    /* 0x6C */ mAllocator_c mAllocator;
    /* 0x88 */ dFgDraw::entry_c *mpEntries;
}; // size 0x8C

class fgHostIO_c {
public:
    virtual ~fgHostIO_c(); // .text 4024
};

class drawUtDtcb_c : public dBGCF::dtcb_c {
public:
    virtual ~drawUtDtcb_c() {}
    drawUtDtcb_c(const mVec3_c &pos, mVec3_c scale, nw4r::g3d::ResTex tex,
                 nw4r::g3d::ResPltt pltt, const mMtx_c &view, s16 width, s16 height, bool special); // 2114
    virtual void checkFloor(dBGCF::floorList_c *floors); // .text 3C54
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ mVec3_c mScale;
    /* 0x1C */ nw4r::g3d::ResTex mTexture;
    /* 0x20 */ nw4r::g3d::ResPltt mPalette;
    /* 0x24 */ const mMtx_c *mpView;
    /* 0x28 */ s16 mWidth;
    /* 0x2A */ s16 mHeight;
    /* 0x2C */ bool mSpecial;
}; // size 0x30

#endif
