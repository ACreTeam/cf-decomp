// Field info: world position <-> block/unit conversion, the per-block data of the town field and
// the rooms (dFdBase_c and its subclasses), and the Racco spot search. See
// include/game/game/d_field_info.hpp and notes/d_field_info.txt.
// .text 8008BD2C..8008EDEC, .rodata 8046EEA8..8046EEC0, .data 804DF990..804DFB10,
// .bss 80587818..80587840, .sdata 80749EE0..80749F20, .sbss 8074E300..8074E310,
// .sdata2 807505A0..807505C0.
#include <game/game/d_field_info.hpp>
#include <game/game/d_search_cand.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_fg_data.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/core/eggHeap.h>
#include <nw4r/math.h>

// Ext item data (0xD000 ids), see d_fg_item.cpp.
struct dFdExtInfo_c {
    /* 0x00 */ u8 _00[0xF];
    /* 0x0F */ u8 mValueA;
    /* 0x10 */ u8 mValueB;
    /* 0x11 */ u8 mFlag;
};

// Result of the ground check fn_8006E1BC.
struct dFdGroundCheck_c {
    /* 0x00 */ u8 _00[0x34];
    /* 0x34 */ int mAttr;
    /* 0x38 */ u8 _38[0x50];
}; // size 0x88

// The furniture interface the unit's furniture object exposes (base at +0x158 of fn_800A8FC4's result).
class dFdFtrIf_c {
public:
    virtual void vf08();
    virtual void vf0C();
    virtual void vf10();
    virtual void vf14();
    virtual void vf18();
    virtual void vf1C();
    virtual void vf20();
    virtual void vf24();
    virtual void vf28();
    virtual void vf2C();
    virtual void vf30();
    virtual void vf34();
    virtual void vf38();
    virtual void vf3C();
    virtual void vf40();
    virtual void vf44();
    virtual void vf48();
    virtual void vf4C();
    virtual void vf50();
    virtual void vf54();
    virtual dItem::Item getItem(); // 0x58
};
struct dFdFtrPad_c {
    u8 _000[0x158];
};
class dFdFtr_c : public dFdFtrPad_c, public dFdFtrIf_c {};

// Not split yet (C linkage keeps the target names).
extern "C" {
extern const f32 lbl_80750520; // 80750520: block world size in x (512)
extern const f32 lbl_80750524; // 80750524: block world size in z (512)
extern const f32 lbl_80750528; // 80750528: half a block in x (256)
extern const f32 lbl_8075052C; // 8075052C: half a block in z (256)
extern EGG::Heap *lbl_8074E3F4; // 8074E3F4

BOOL fn_80075658(int blockW, int blockH, dBGCF::clmcb_c *cb, int bg); // 80075658: register a BG slot
void fn_800755A0(int bg);                                              // 800755A0: select the current BG slot
void fn_8007589C(int bg);                                              // 8007589C: release a BG slot
void fn_800756F4(int blockX, int blockZ, void *data, int flag, int bg); // 800756F4
f32 fn_80074D64(int unitX, int unitZ);                                 // 80074D64: ground height of a unit
void fn_8006CD90(nw4r::math::VEC3 *pos, int unitX, int unitZ);         // 8006CD90: unit -> world position
void fn_8006CD4C(int *blockX, int *blockZ, int unitX, int unitZ);      // 8006CD4C: block of a unit
f32 fn_80074974(const nw4r::math::VEC3 *pos, int a);                   // 80074974: ground height
int fn_80081514(int type);                                             // 80081514: the bridge variant of a block type
int fn_80073D2C(void *arg, int unitX, int unitZ, int arg2);            // 80073D2C
int fn_80076260(int unitX, int unitZ);                                 // 80076260
int fn_80073158(int unitX, int unitZ);                                 // 80073158
int fn_80073208(int unitX, int unitZ);                                 // 80073208
int fn_800733F0(int unitX, int unitZ);                                 // 800733F0
int fn_80073260(int unitX, int unitZ);                                 // 80073260
int fn_800732B8(int unitX, int unitZ);                                 // 800732B8
int fn_80073314(int unitX, int unitZ);                                 // 80073314
int fn_80072D54(int unitX, int unitZ);                                 // 80072D54
int fn_80072F80(int unitX, int unitZ);                                 // 80072F80: unit attribute
void *fn_8014B6C8();                                                   // 8014B6C8
dItem::Item fn_8014B034(void *fg, int x, int z, int);                  // 8014B034
dFdExtInfo_c *fn_80167FB4(int index);                                  // 80167FB4
void *fn_800A835C();                                                   // 800A835C
void fn_800A7F14(void *, const nw4r::math::VEC3 *pos, int, int, int);  // 800A7F14
void fn_800A7F9C(void *, const nw4r::math::VEC3 *pos, int, int);       // 800A7F9C
int fn_8006E1BC(dFdGroundCheck_c *check, const nw4r::math::VEC3 *pos, int, int, int); // 8006E1BC
u32 fn_802B6258(void *, int x, int z);                                 // 802B6258
void *fn_800A9058();                                                   // 800A9058
dFdFtr_c *fn_800A8FC4(void *, int x, int z, int);                      // 800A8FC4
void *fn_80069978();                                                   // 80069978: BG data loader
void fn_80069680(void *loader, void *buf, int id);                     // 80069680: loads a BG into buf
dFdUnitAttr_c *fn_801683D8();                                          // 801683D8
}

// Units where Racco can appear (RTTI raccoCand_c; vtable 804DF990): units of BG attribute
// 0x6C..0x70 in block (mBlockX, mBlockZ), or anywhere when both are -1, excluding the block saved at
// dSaveData_c+0x735CD/E when the flag at +0x735CC is 1.
class raccoCand_c : public dFdGutSearchCand_c {
public:
    raccoCand_c(const dFdBase_c *fd, int blockX, int blockZ) : dFdGutSearchCand_c(fd) {
        mInfo = (dFdBase_c *)fd;
        mBlockX = blockX;
        mBlockZ = blockZ;
    }
    virtual BOOL check(int x, int z); // 8008EBA0

    /* 0x640 */ int mBlockX; // -1: any
    /* 0x644 */ int mBlockZ;
}; // size 0x648

// 8046EEA8: the facing for unit attributes 0x6C..0x70.
static const s16 sRaccoAngle[] = {-0x4000, 0, 0x4000, -0x2000, 0x2000};
// 8046EEB4
static const f32 sRaccoDist[] = {32.0f, 48.0f, 64.0f};

// Unit / block index -> world coordinate.
static inline f32 unitToWorld(int u) { return u * mFI_UT_WORLDSIZE_X; }
static inline f32 blockToWorld(int b) { int u = unitToWorld(b); return u * UT_X_NUM; }

// 8008BD2C
BOOL dFdBase_c::setup(int blockW, int blockH, int bg) {
    mBlockW = blockW;
    mBlockH = blockH;
    mUnitW = blockW * UT_X_NUM;
    mUnitH = blockH * UT_Z_NUM;
    mWorldW = blockW * lbl_80750520;
    mWorldH = blockH * lbl_80750524;
    mBg = bg;
    return fn_80075658(blockW, blockH, this, bg);
}

// Stripped by the linker (placeholder name and body); its 0.0f pools that constant before the
// ones of getUnitCenterPos, as in the target's .sdata2.
void dFdBase_c::clearWorldSize() {
    mWorldW = 0.0f;
    mWorldH = 0.0f;
}

// 8008BDC0
BOOL dFdBase_c::isValid(u32 blockX, u32 blockZ, u32 unitX, u32 unitZ) {
    BOOL valid = FALSE;
    if (blockX < mBlockW && blockZ < mBlockH && unitX < UT_X_NUM && unitZ < UT_Z_NUM) {
        valid = TRUE;
    }
    return valid;
}

// 8008BDF8
void dFdBase_c::posToBlockUnit(int *blockX, int *blockZ, int *unitX, int *unitZ, const nw4r::math::VEC3 *pos) {
    *blockX = (int)pos->x >> 9;
    *blockZ = (int)pos->z >> 9;
    *unitX = ((int)pos->x >> 5) & (UT_X_NUM - 1);
    *unitZ = ((int)pos->z >> 5) & (UT_Z_NUM - 1);
}

// 8008BE54
void dFdBase_c::snapToUnitCenter(nw4r::math::VEC3 *out, const nw4r::math::VEC3 *pos) {
    getUnitCenterPos(out, (int)pos->x >> 5, (int)pos->z >> 5);
    out->y = pos->y;
}

// 8008BEBC
void dFdBase_c::getUnitCenterPos(nw4r::math::VEC3 *pos, int blockX, int blockZ, int unitX, int unitZ) {
    int x = blockX * UT_X_NUM;
    int z = blockZ * UT_Z_NUM;
    getUnitCenterPos(pos, x + unitX, z + unitZ);
}

// 8008BED0
void dFdBase_c::getUnitCenterPos(nw4r::math::VEC3 *pos, int unitX, int unitZ) {
    f32 x = mFI_UT_WORLDSIZE_HALF_X_F + unitToWorld(unitX);
    f32 z = mFI_UT_WORLDSIZE_HALF_Z_F + unitToWorld(unitZ);
    pos->y = 0.0f;
    pos->x = x;
    pos->z = z;
}

// 8008BF30
void dFdBase_c::snapToUnitGround(nw4r::math::VEC3 *out, const nw4r::math::VEC3 *pos) {
    getUnitGroundPos(out, (int)pos->x >> 5, (int)pos->z >> 5);
}

// 8008BF78
void dFdBase_c::getUnitGroundPos(nw4r::math::VEC3 *pos, int blockX, int blockZ, int unitX, int unitZ) {
    int x = blockX * UT_X_NUM;
    int z = blockZ * UT_Z_NUM;
    getUnitGroundPos(pos, x + unitX, z + unitZ);
}

// 8008BF8C
void dFdBase_c::getUnitGroundPos(nw4r::math::VEC3 *pos, int unitX, int unitZ) {
    f32 x = mFI_UT_WORLDSIZE_HALF_X_F + unitToWorld(unitX);
    f32 z = mFI_UT_WORLDSIZE_HALF_Z_F + unitToWorld(unitZ);
    pos->x = x;
    pos->z = z;
    pos->y = fn_80074D64(unitX, unitZ);
}

// 8008C010
void dFdBase_c::getBlockCenterPos(nw4r::math::VEC3 *pos, int blockX, int blockZ) {
    f32 x = blockToWorld(blockX);
    f32 z = blockToWorld(blockZ);
    pos->x = x + lbl_80750528;
    pos->z = z + lbl_8075052C;
}

// 8008C0AC
static inline void getBlockPos(nw4r::math::VEC3 *pos, int blockX, int blockZ) {
    f32 x = blockToWorld(blockX);
    f32 z = blockToWorld(blockZ);
    pos->x = x;
    pos->z = z;
}
void dFdBase_c::getUnitPos(nw4r::math::VEC3 *pos, int blockX, int blockZ, int unitX, int unitZ) {
    getBlockPos(pos, blockX, blockZ);
    pos->x += unitToWorld(unitX);
    pos->z += unitToWorld(unitZ);
}

// 8008C16C
dFdBlock_c *dFdBase_c::getBlock(int blockX, int blockZ) {
    if (isValid(blockX, blockZ, 0, 0) && mBlocks != NULL) {
        return &mBlocks[blockX + blockZ * mBlockW];
    }
    return NULL;
}

// 8008C1E8
const dFdBlock_c *dFdBase_c::getBlock(int blockX, int blockZ) const {
    if (((dFdBase_c *)this)->isValid(blockX, blockZ, 0, 0) && mBlocks != NULL) {
        return &mBlocks[blockX + blockZ * mBlockW];
    }
    return NULL;
}

// 8008C264
dFdBlock_c *dFdBase_c::findBlock(int mask) {
    dFdBlock_c *block;
    int x;
    int z;
    int w = mBlockW;
    int h = mBlockH;
    for (z = 0; z < h; z++) {
        for (x = 0; x < w; x++) {
            block = getBlock(x, z);
            if (block != NULL && block->hasFlag(mask)) {
                return block;
            }
        }
    }
    return NULL;
}

// 8008C300
const dFdBlock_c *dFdBase_c::findBlock(int mask) const {
    const dFdBlock_c *block;
    int x;
    int z;
    int w = mBlockW;
    int h = mBlockH;
    for (z = 0; z < h; z++) {
        for (x = 0; x < w; x++) {
            block = getBlock(x, z);
            if (block != NULL && block->hasFlag(mask)) {
                return block;
            }
        }
    }
    return NULL;
}

// 8008C39C
BOOL dFdBase_c::hasBlockFlag(int blockX, int blockZ, int mask) const {
    const dFdBlock_c *block = getBlock(blockX, blockZ);
    if (block != NULL) {
        return block->hasFlag(mask);
    }
    return FALSE;
}

// 8008C3E0
BOOL dFdBase_c::getRaccoSpot(int *unitX, int *unitZ, s16 *angle) {
    raccoCand_c cand(this, -1, -1);
    cand.clear();
    cand.search();
    if (cand.getRandomXZ(unitX, unitZ)) {
        u32 idx = bgCall_80072F80(*unitX, *unitZ) - 0x6C;
        if (idx < ARRAY_SIZE(sRaccoAngle)) {
            *angle = sRaccoAngle[idx];
            return TRUE;
        }
    }
    return FALSE;
}

// 8008C4E4
f32 dFdBase_c::getRaccoDist(u32 idx) {
    if (idx < ARRAY_SIZE(sRaccoDist)) {
        return mFI_UT_WORLDSIZE_HALF_X_F + mFI_UT_WORLDSIZE_X_F * sRaccoDist[idx];
    }
    return 0.0f;
}

// 8008C518
u32 dFdBase_c::getOtherRaccoIdx(u32 idx) {
    if (idx < ARRAY_SIZE(sRaccoDist)) {
        return cM::rndInt(ARRAY_SIZE(sRaccoDist));
    }
    u32 other = cM::rndInt(ARRAY_SIZE(sRaccoDist));
    return other < idx ? other : other + 1;
}

// 8008C570
f32 dFdBase_c::getRaccoParamA() {
    return 256.0f;
}

// 8008C578
f32 dFdBase_c::getRaccoParamB() {
    return 3328.0f;
}

// 8008C580
nw4r::math::VEC3 dFdBase_c::getRaccoPos(int blockX, int blockZ) const {
    nw4r::math::VEC3 pos;
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = 0.0f;
    raccoCand_c cand(this, blockX, blockZ);
    cand.clear();
    cand.search();
    int unitX;
    int unitZ;
    if (cand.getRandomXZ(&unitX, &unitZ)) {
        fn_800755A0(1);
        fn_8006CD90(&pos, unitX, unitZ);
        fn_800755A0(0);
        pos.y = fn_80074974(&pos, 0);
    }
    return pos;
}

// 8008C674
BOOL dFdBase_c::isBlockVariant(int blockX, int blockZ) const {
    const dFdBlock_c *block = getBlock(blockX, blockZ);
    if (block != NULL) {
        int type = block->mType;
        return fn_80081514(type) != type;
    }
    return FALSE;
}

static inline BOOL clearBlockItem(dFdBlock_c *block, int ux, int uz, int layer) {
    dItem::Item none((u16)dItem::ITEM_ID_NONE);
    return block->setItem(&none, ux, uz, layer);
}

// 8008C6C8: removes items that have no item data.
void dFdBase_c::clearUnknownItems() {
    int w = mBlockW;
    int h = mBlockH;
    for (int bz = 0; bz < h; bz++) {
        for (int bx = 0; bx < w; bx++) {
            dFdBlock_c *block = getBlock(bx, bz);
            if (block == NULL) {
                continue;
            }
            for (int layer = 0; layer < 2; layer++) {
                for (int uz = 0; uz < UT_Z_NUM; uz++) {
                    for (int ux = 0; ux < UT_X_NUM; ux++) {
                        dItem::Item *item = block->getItem(ux, uz, layer);
                        if (item != NULL && dItem::isRealItemId(item->mId) && dItem::getBITM(item->mId) == NULL) {
                            clearBlockItem(block, ux, uz, layer);
                        }
                    }
                }
            }
        }
    }
}

// 8008C7F4
dItem::Item *dFdBase_c::getItem(int blockX, int blockZ, int unitX, int unitZ, int layer) const {
    const dFdBlock_c *block = getBlock(blockX, blockZ);
    if (block != NULL) {
        return block->getItem(unitX, unitZ, layer);
    }
}

// 8008C850
dItem::Item *dFdBase_c::getItem(int unitX, int unitZ, int layer) const {
    return getItem(unitX >> 4, unitZ >> 4, unitX - (unitX >> 4) * UT_X_NUM, unitZ - (unitZ >> 4) * UT_Z_NUM, layer);
}

// 8008C878
dItem::Item *dFdBase_c::getItem(const nw4r::math::VEC3 *pos, int layer) const {
    int blockX = 0;
    int blockZ = 0;
    int unitX = 0;
    int unitZ = 0;
    posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, pos);
    return getItem(blockX, blockZ, unitX, unitZ, layer);
}

// 8008C8F4
BOOL dFdBase_c::setItem(const dItem::Item *item, int blockX, int blockZ, int unitX, int unitZ, int layer) {
    dFdBlock_c *block = getBlock(blockX, blockZ);
    if (block != NULL) {
        return block->setItem(item, unitX, unitZ, layer);
    }
    return FALSE;
}

// 8008C970
BOOL dFdBase_c::setItem(const dItem::Item *item, int unitX, int unitZ, int layer) {
    return setItem(item, unitX >> 4, unitZ >> 4, unitX - (unitX >> 4) * UT_X_NUM, unitZ - (unitZ >> 4) * UT_Z_NUM,
                   layer);
}

// 8008C998
BOOL dFdBase_c::setItem(const dItem::Item *item, const nw4r::math::VEC3 *pos, int layer) {
    int blockX = 0;
    int blockZ = 0;
    int unitX = 0;
    int unitZ = 0;
    posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, pos);
    return setItem(item, blockX, blockZ, unitX, unitZ, layer);
}

// 8008CA24
BOOL dFdBase_c::setBuried(int blockX, int blockZ, int unitX, int unitZ) {
    dFdBlock_c *block = getBlock(blockX, blockZ);
    if (block != NULL) {
        return block->setBuried(unitX, unitZ);
    }
    return FALSE;
}

// 8008CA78
BOOL dFdBase_c::setBuried(int unitX, int unitZ) {
    return setBuried(unitX >> 4, unitZ >> 4, unitX - (unitX >> 4) * UT_X_NUM, unitZ - (unitZ >> 4) * UT_Z_NUM);
}

// 8008CA9C
BOOL dFdBase_c::clearBuried(int blockX, int blockZ, int unitX, int unitZ) {
    dFdBlock_c *block = getBlock(blockX, blockZ);
    if (block != NULL) {
        return block->clearBuried(unitX, unitZ);
    }
    return FALSE;
}

// 8008CAF0
BOOL dFdBase_c::clearBuried(int unitX, int unitZ) {
    return clearBuried(unitX >> 4, unitZ >> 4, unitX - (unitX >> 4) * UT_X_NUM, unitZ - (unitZ >> 4) * UT_Z_NUM);
}

// 8008CB14
BOOL dFdBase_c::isBuried(int blockX, int blockZ, int unitX, int unitZ) const {
    const dFdBlock_c *block = getBlock(blockX, blockZ);
    if (block != NULL) {
        return block->isBuried(unitX, unitZ);
    }
    return FALSE;
}

// 8008CB68
BOOL dFdBase_c::isBuried(int unitX, int unitZ) const {
    return isBuried(unitX >> 4, unitZ >> 4, unitX - (unitX >> 4) * UT_X_NUM, unitZ - (unitZ >> 4) * UT_Z_NUM);
}

// 8008CB8C
BOOL dFdBase_c::isBuried(const nw4r::math::VEC3 *pos) const {
    int blockX = 0;
    int blockZ = 0;
    int unitX = 0;
    int unitZ = 0;
    posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, pos);
    return isBuried(blockX, blockZ, unitX, unitZ);
}

// 8008CBF8
BOOL dFdBase_c::setWatered(int blockX, int blockZ, int unitX, int unitZ) {
    dFdBlock_c *block = getBlock(blockX, blockZ);
    if (block != NULL) {
        return block->setWatered(unitX, unitZ);
    }
    return FALSE;
}

// 8008CC4C
BOOL dFdBase_c::setWatered(int unitX, int unitZ) {
    return setWatered(unitX >> 4, unitZ >> 4, unitX - (unitX >> 4) * UT_X_NUM, unitZ - (unitZ >> 4) * UT_Z_NUM);
}

// 8008CC70
BOOL dFdBase_c::isWatered(int blockX, int blockZ, int unitX, int unitZ) const {
    const dFdBlock_c *block = getBlock(blockX, blockZ);
    if (block != NULL) {
        return block->isWatered(unitX, unitZ);
    }
    return FALSE;
}

// 8008CCC4
BOOL dFdBase_c::isWatered(int unitX, int unitZ) const {
    return isWatered(unitX >> 4, unitZ >> 4, unitX - (unitX >> 4) * UT_X_NUM, unitZ - (unitZ >> 4) * UT_Z_NUM);
}

// 8008CCE8
void dFdBase_c::clearWatered() {
    int x;
    int z;
    for (z = 0; z < BLOCK_Z_NUM; z++) {
        for (x = 0; x < BLOCK_X_NUM; x++) {
            dFdBlock_c *block = getBlock(x, z);
            if (block != NULL) {
                block->clearWatered();
            }
        }
    }
}

// 8008CD5C
int dFdBase_c::bgCall_80073D2C(void *arg, int unitX, int unitZ, int arg2) {
    if (_24 != NULL && !_24->isGroundFree(unitX, unitZ)) {
        return 0;
    }
    fn_800755A0(mBg);
    int res = fn_80073D2C(arg, unitX, unitZ, arg2);
    fn_800755A0(0);
    return res;
}

// 8008CDF4
int dFdBase_c::bgCall_80076260(int unitX, int unitZ) {
    fn_800755A0(mBg);
    int res = fn_80076260(unitX, unitZ);
    fn_800755A0(0);
    return res;
}

// 8008CE4C
void dFdBase_c::setBorderUnits() {
    for (int x = UT_X_NUM; x < mUnitW - UT_X_NUM; x++) {
        bgCall_80076260(x, UT_Z_NUM);
    }
    for (int z = UT_Z_NUM; z < mUnitH; z++) {
        bgCall_80076260(UT_X_NUM, z);
        bgCall_80076260(mUnitW - UT_X_NUM - 1, z);
    }
}

// 8008CEE4
int dFdBase_c::canPutItem(int unitX, int unitZ) const {
    if (_24 != NULL && !_24->isGroundFree(unitX, unitZ)) {
        return 0;
    }
    fn_800755A0(mBg);
    int res = fn_80073158(unitX, unitZ);
    fn_800755A0(0);
    return res;
}

// 8008CF6C
int dFdBase_c::canPutItem(const nw4r::math::VEC3 *pos) {
    f32 x = pos->x;
    f32 z = pos->z;
    return canPutItem((int)x >> 5, (int)z >> 5);
}

// 8008CFB4
int dFdBase_c::isGrassGround(int unitX, int unitZ) const {
    if (_24 != NULL && !_24->isGroundFree(unitX, unitZ)) {
        return 0;
    }
    fn_800755A0(mBg);
    int res = fn_80073208(unitX, unitZ);
    fn_800755A0(0);
    return res;
}

// 8008D03C
int dFdBase_c::isBeachGround(int unitX, int unitZ) const {
    if (_24 != NULL && !_24->isGroundFree(unitX, unitZ)) {
        return 0;
    }
    fn_800755A0(mBg);
    int res = fn_800733F0(unitX, unitZ);
    fn_800755A0(0);
    return res;
}

// 8008D0C4
int dFdBase_c::canNpcPutItem(int unitX, int unitZ) const {
    if (_24 != NULL && !_24->isNotStrCol(unitX, unitZ)) {
        return 0;
    }
    fn_800755A0(mBg);
    int res = fn_80073260(unitX, unitZ);
    fn_800755A0(0);
    return res;
}

// 8008D14C
int dFdBase_c::getDigType(int unitX, int unitZ) const {
    if (_24 != NULL && !_24->isGroundFree(unitX, unitZ)) {
        return 2;
    }
    fn_800755A0(mBg);
    int res = fn_800732B8(unitX, unitZ);
    fn_800755A0(0);
    return res;
}

// 8008D1D4
int dFdBase_c::getDigType(const nw4r::math::VEC3 *pos) {
    f32 x = pos->x;
    f32 z = pos->z;
    return getDigType((int)x >> 5, (int)z >> 5);
}

// 8008D21C
int dFdBase_c::getPlantType(int unitX, int unitZ) const {
    if (_24 != NULL && !_24->isGroundFree(unitX, unitZ)) {
        return 0;
    }
    fn_800755A0(mBg);
    int res = fn_80073314(unitX, unitZ);
    fn_800755A0(0);
    if (res == 2 && _24 != NULL && _24->isNoPlantUnit(unitX, unitZ)) {
        return 1;
    }
    return res;
}

// 8008D2DC
int dFdBase_c::getPlantType(int blockX, int blockZ, int unitX, int unitZ) {
    int x = blockX * UT_X_NUM;
    int z = blockZ * UT_Z_NUM;
    return getPlantType(x + unitX, z + unitZ);
}

// 8008D2F0
int dFdBase_c::bgCall_80072D54(int unitX, int unitZ) const {
    fn_800755A0(mBg);
    int res = fn_80072D54(unitX, unitZ);
    fn_800755A0(0);
    return res;
}

// 8008D348
int dFdBase_c::bgCall_80072F80(int unitX, int unitZ) const {
    fn_800755A0(mBg);
    int res = fn_80072F80(unitX, unitZ);
    fn_800755A0(0);
    return res;
}

// 8008D3A0
BOOL dFdBase_c::getAttr(f32 *height, f32 *param, int *attr, int x, int z) {
    dItem::Item fg;
    dItem::Item *item = getItem(x, z, 0);
    if (item != NULL) {
        if (item->isExtId()) {
            f32 value = item->getExtValueA();
            BOOL valid = value != 0.0f;
            if (valid && (_24 == NULL || !_24->getAttr(x, z))) {
                *height = value;
                *param = item->getExtValueB();
                *attr = 0x15;
                return TRUE;
            }
        } else if (getCurrentScene() == SCENE_TOWN) {
            fg = fn_8014B034(fn_8014B6C8(), x, z, 2);
            if (fg.isExtId()) {
                dFdExtInfo_c *info = fn_80167FB4(fg.getExtIndex());
                f32 value = (int)info->mValueA;
                if (value != 0.0f) {
                    f32 p = (int)info->mValueB;
                    *height = value;
                    *param = p;
                    *attr = 0x15;
                    return TRUE;
                }
            }
        } else if (item->mId == 0x7003) {
            *height = 16.0f;
            *param = 48.0f;
            *attr = 0x13;
            return TRUE;
        }
        dItem::FgInfo *info = item->getFgInfo();
        if (info != NULL) {
            f32 value = (s8)info->_08[0];
            if (value != 0.0f) {
                *height = value;
                *param = (s8)info->_08[1];
                *attr = (s8)info->_08[2];
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8008D610
int dFdBase_c::bgCall_80072D54(const nw4r::math::VEC3 *pos) {
    f32 x = pos->x;
    f32 z = pos->z;
    return bgCall_80072D54((int)x >> 5, (int)z >> 5);
}

// 8008D658
void dFdBase_c::fn_8008D658(int unitX, int unitZ, int a, int b) {
    nw4r::math::VEC3 pos;
    getUnitCenterPos(&pos, unitX, unitZ);
    fn_8008D6B4(&pos, a, b);
}

// 8008D6B4
void dFdBase_c::fn_8008D6B4(const nw4r::math::VEC3 *pos, int a, int b) {
    if (_24 != NULL) {
        fn_800A7F14(fn_800A835C(), pos, -a, 0, 1);
    }
}

// 8008D70C
void dFdBase_c::fn_8008D70C(const nw4r::math::VEC3 *pos, int a) {
    if (_24 != NULL) {
        fn_800A7F9C(fn_800A835C(), pos, a, 0);
    }
}

// 8008D760
u8 dFdBase_c::fn_8008D760(const nw4r::math::VEC3 *pos, BOOL onlyAttr16) {
    if (_24 != NULL) {
        f32 x = pos->x;
        f32 z = pos->z;
        int unitX = (int)x >> 5;
        int unitZ = (int)z >> 5;
        fn_800755A0(mBg);
        dFdGroundCheck_c check;
        fn_8006E1BC(&check, pos, 0, 0, 0);
        int attr = check.mAttr;
        fn_800755A0(0);
        if (onlyAttr16) {
            if (attr == 0x16) {
                return fn_802B6258((u8 *)fn_800A835C() + 0x58, unitX, unitZ);
            }
            return 0;
        }
        return fn_802B6258((u8 *)fn_800A835C() + 0x58, unitX, unitZ);
    }
    return 0xFF;
}

// 8008D858
dItem::Item dFdBase_c::getUnitItem(int unitX, int unitZ) const {
    static dItem::Item sExcluded(0x39);
    dItem::Item result;
    dItem::Item *item = getItem(unitX, unitZ, 0);
    if (item != NULL && item->isNotSame(sExcluded)) {
        if (item->isUsable()) {
            dFdFtrIf_c *ftr = fn_800A8FC4(fn_800A9058(), unitX, unitZ, 0);
            result = ftr->getItem();
        } else {
            u16 id = item->mId;
            if (dItem::isRealItemId(id)) {
                result.mId = id;
            }
        }
    }
    return result;
}

// 8008D99C
void dFdBase_c::registerBg() {
    int w = mBlockW;
    int h = mBlockH;
    fn_80075658(w, h, this, 0);
    int x, z;
    for (z = 0; z < h; z++) {
        for (x = 0; x < w; x++) {
            dFdBlock_c *block = getBlock(x, z);
            if (block != NULL) {
                fn_800756F4(block->mBlockX, block->mBlockZ, block->mBgData, block->mFlag, 0);
            }
        }
    }
}

// 8008DA44
BOOL dFdInfo_c::create(const dFdBlockId_c *blockIds, int blockW, int blockH, EGG::Heap *heap, dFdUnitAttr_c *unitAttr) {
    _24 = unitAttr;
    if (mBlocks == NULL) {
        mBlocks = dFdBlock_c::create(blockW * blockH, heap, 4);
        setup(blockW, blockH, 0);
    }
    void *data;
    dItem::Item *items;
    dFdBlock_c *block;
    int x, z;
    block = mBlocks;
    if (block != NULL) {
        for (z = 0; z < mBlockH; z++) {
            for (x = 0; x < mBlockW; x++) {
                if (block->mBgData == NULL) {
                    data = heap->alloc(0xA00, 4);
                    if (data != NULL) {
                        fn_80069680(fn_80069978(), data, blockIds->mId);
                    }
                    items = NULL;
                    if (block->getItem(0, 0, 0) == NULL) {
                        items = (dItem::Item *)heap->alloc(UT_TOTAL_NUM * sizeof(dItem::Item), 4);
                        if (items != NULL) {
                            for (int i = 0; i < UT_TOTAL_NUM; i++) {
                                new (&items[i]) dItem::Item;
                            }
                            if (!dFgData_getLayout0((u16 *)items, blockIds->mId)) {
                                heap->free(items);
                                items = NULL;
                            }
                        }
                    }
                    block->set(blockIds->mId, items, NULL, NULL, NULL, data, x, z, blockIds->mFlag, mBg);
                }
                block++;
                blockIds++;
            }
        }
    }
    return TRUE;
}

// 8008DC84
void dFdInfo_c::release(EGG::Heap *heap) {
    dFdBlock_c *block = mBlocks;
    if (block != NULL) {
        int x, z;
        for (z = 0; z < mBlockH; z++) {
            for (x = 0; x < mBlockW; x++) {
                block->release(heap, TRUE, TRUE);
                block++;
            }
        }
        heap->free(mBlocks);
        mBlocks = NULL;
    }
    _24 = NULL;
    fn_8007589C(mBg);
}

// 8008DD3C
u32 dFdInfo_c::getHeapSize() {
    return ROUND_UP(dFdBlock_c::getAllocSize(BLOCK_TOTAL_NUM, 4) + BLOCK_TOTAL_NUM * 0xA00 + sizeof(dFdInfo_c), 4);
}

// 8008DD70
BOOL dFdInfo_c::createTown(EGG::Heap *heap) {
    dSaveMainField_c *field;
    dFdBlock_c *block;
    const dFdBlockId_c *blockIds;
    void *data;
    int x, z;
    if (mBlocks == NULL) {
        mBlocks = dFdBlock_c::create(BLOCK_TOTAL_NUM, heap, 4);
        setup(BLOCK_X_NUM, BLOCK_Z_NUM, 1);
    }
    block = mBlocks;
    if (block != NULL) {
        field = &dSaveData_c::getTown()->mMainField;
        blockIds = field->mFieldBlockData[0];
        for (z = 0; z < mBlockH; z++) {
            for (x = 0; x < mBlockW; x++) {
                if (block->mBgData == NULL) {
                    data = heap->alloc(0xA00, 4);
                    if (data != NULL) {
                        fn_80069680(fn_80069978(), data, blockIds->mId);
                    }
                    block->set(blockIds->mId, field->getBlockItems(x, z), NULL, (u16 *)field->getBlockBuried(x, z),
                               (u16 *)field->getBlockWater(x, z), data, x, z, blockIds->mFlag, mBg);
                }
                block++;
                blockIds++;
            }
        }
    }
    _24 = fn_801683D8();
    return TRUE;
}

// 8008DEF4
BOOL dFdInfo_c::updateTown() {
    dSaveMainField_c *field;
    void *data;
    dFdBlock_c *block;
    const dFdBlockId_c *blockIds;
    int x, z;
    block = mBlocks;
    if (block != NULL) {
        field = &dSaveData_c::getTown()->mMainField;
        blockIds = field->mFieldBlockData[0];
        for (z = 0; z < mBlockH; z++) {
            for (x = 0; x < mBlockW; x++) {
                data = block->mBgData;
                if (data != NULL) {
                    fn_80069680(fn_80069978(), data, blockIds->mId);
                }
                block->set(blockIds->mId, field->getBlockItems(x, z), NULL, (u16 *)field->getBlockBuried(x, z),
                           (u16 *)field->getBlockWater(x, z), data, x, z, blockIds->mFlag, mBg);
                block++;
                blockIds++;
            }
        }
    }
    _24 = fn_801683D8();
    setBorderUnits();
    return TRUE;
}

// 8008E024
u32 dFdInfo_c::getRoomHeapSize() {
    return ROUND_UP(dFdBlock_c::getAllocSize(1, 4) + 0xA00 + sizeof(dFdInfo_c), 4);
}

// 8008E054
int dFdBase_c::getHomeRoomBg(u32 home, int room) {
    if (home < 4 && dHome_c::isValidRoom(room)) {
        return home * 3 + room + 2;
    }
    return 0x10;
}

// 8008E0B8
BOOL dFdBase_c::buildHomeRoom(u32 home, int room, EGG::Heap *heap) {
    dHome_c *homeData;
    int bg;
    dFdBlock_c *block;
    void *buf;
    int roomId;
    dHomeRoom_c *roomData;
    homeData = dSaveData_c::getTown()->mHomes.getHome(home);
    bg = getHomeRoomBg(home, room);
    if (homeData == NULL || bg == 0x10) {
        return FALSE;
    }
    if (mBlocks == NULL) {
        mBlocks = dFdBlock_c::create(1, heap, 4);
        setup(1, 1, bg);
    }
    block = mBlocks;
    if (block != NULL && block->mBgData == NULL) {
        roomId = homeData->getRoomId(room);
        roomData = homeData->getRoom(room);
        buf = heap->alloc(0xA00, 4);
        if (buf != NULL) {
            fn_80069680(fn_80069978(), buf, roomId);
        }
        block->set(roomId, (dItem::Item *)roomData->getLayer(0), (dItem::Item *)roomData->getLayer(1), 0, 0, buf, 0, 0, 0, mBg);
    }
    _24 = NULL;
    return TRUE;
}

// 8008E238
void dFdBase_c::reloadHomeRoom(u32 home, int room) {
    // The room index becomes the room id.
    room = dSaveData_c::getTown()->mHomes.getHome(home)->getRoomId(room);
    dFdBlock_c *block = getBlock(0, 0);
    if (block != NULL) {
        block->mType = room;
        void *buf = block->mBgData;
        if (buf != NULL) {
            fn_80069680(fn_80069978(), buf, room);
        }
    }
}

// 8008E2D0
int dFdBase_c::getBgDataSize() {
    return 0xA00;
}

static void *sNpcHsBgData; // 8074E308
static u32 sNpcHsCount;    // 8074E30C: live dFdInfoNpcHs_c objects

// 8008E2D8
void *dFdInfoNpcHs_c::getBgData(int bgId) {
    if (sNpcHsBgData == NULL) {
        void *buf = lbl_8074E3F4->alloc(0xA00, 4);
        sNpcHsBgData = buf;
        if (buf != NULL) {
            fn_80069680(fn_80069978(), buf, bgId);
        }
    }
    return sNpcHsBgData;
}

// 8008E354
BOOL dFdInfoNpcHs_c::incCount() {
    if (sNpcHsCount < 0xFFFFFFFF) {
        sNpcHsCount++;
        return TRUE;
    }
    return FALSE;
}

// 8008E37C
BOOL dFdInfoNpcHs_c::decCount() {
    if (sNpcHsCount != 0) {
        sNpcHsCount--;
        return TRUE;
    }
    return FALSE;
}

// 8008E3A0
BOOL dFdInfoNpcHs_c::isCountZero() {
    return sNpcHsCount == 0;
}

// 8008E3B0
BOOL dFdInfoNpcHs_c::build(int animalIdx, EGG::Heap *heap) {
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(animalIdx);
    if (animal == NULL) {
        return FALSE;
    }
    int type = 1;
    int layout = animal->getRoomLayout(&type);
    if (layout == -1) {
        return FALSE;
    }
    if (mBlocks == NULL) {
        mBlocks = dFdBlock_c::create(1, heap, 4);
        setup(1, 1, 0xE);
    }
    dFdBlock_c *block = mBlocks;
    BOOL ok = TRUE;
    if (block != NULL) {
        if (block->mBgData == NULL) {
            void *bgData = getBgData(0xC1);
            if (block->getItem(0, 0, 0) == NULL) {
                dItem::Item *items = (dItem::Item *)heap->alloc(0x200, 4);
                if (items != NULL) {
                    for (int i = 0; i < 0x100; i++) {
                        new (&items[i]) dItem::Item;
                    }
                    BOOL found;
                    if (type == 1) {
                        found = dFgData_getLayout1((u16 *)items, layout);
                    } else {
                        found = dFgData_getLayout2((u16 *)items, layout);
                    }
                    if (found) {
                        animal->applyRoomFtr(items);
                    } else {
                        heap->free(items);
                        items = NULL;
                        ok = FALSE;
                    }
                } else {
                    ok = FALSE;
                }
                if (ok) {
                    block->set(0xC1, items, NULL, 0, 0, bgData, 0, 0, 0, mBg);
                }
            }
        }
    } else {
        ok = FALSE;
    }
    _24 = NULL;
    return ok;
}

// 8008E614
void dFdInfoNpcHs_c::destroy(EGG::Heap *heap) {
    if (mBlocks != NULL) {
        mBlocks->release(heap, 1, 0);
        heap->free(mBlocks);
        mBlocks = NULL;
    }
    _24 = NULL;
    if (isCountZero()) {
        fn_8007589C(mBg);
    }
}

// 8008E6A0
dFdInfoNpcHs_c *dFdInfoNpcHs_c::create(int animalIdx, EGG::Heap *heap) {
    dFdInfoNpcHs_c *info = (dFdInfoNpcHs_c *)heap->alloc(sizeof(dFdInfoNpcHs_c), 4);
    if (info != NULL) {
        new (info) dFdInfoNpcHs_c;
        if (info->build(animalIdx, heap)) {
            incCount();
        } else {
            info->destroy(heap);
            heap->free(info);
            info = NULL;
        }
    }
    return info;
}

// 8008E798
void dFdInfoNpcHs_c::remove(dFdInfoNpcHs_c **info, EGG::Heap *heap) {
    if (info != NULL && *info != NULL && decCount()) {
        (*info)->destroy(heap);
        heap->free(*info);
        *info = NULL;
    }
}

// 8008E818
dFdInfoSvMdlRm_c::dFdInfoSvMdlRm_c() {}

// 8008E854
dFdInfoSvMdlRm_c::~dFdInfoSvMdlRm_c() {}

// 8008E894
BOOL dFdInfoSvMdlRm_c::build(dHomeRoom_c *room, int bgId, EGG::Heap *heap) {
    if (mBlocks == NULL) {
        mBlocks = dFdBlock_c::create(1, heap, 4);
        setup(1, 1, bgId);
    }
    dFdBlock_c *block = mBlocks;
    BOOL ok = TRUE;
    if (block != NULL) {
        if (block->mBgData == NULL) {
            int roomId = static_cast<dModelRoom_c *>(room)->getBgId(); // room is a model room
            void *buf = heap->alloc(0xA00, 4);
            if (buf != NULL) {
                fn_80069680(fn_80069978(), buf, roomId);
            } else {
                ok = FALSE;
            }
            if (ok) {
                block->set(roomId, (dItem::Item *)room->getLayer(0), (dItem::Item *)room->getLayer(1), 0, 0, buf, 0, 0, 0, mBg);
            }
        }
    } else {
        ok = FALSE;
    }
    _24 = NULL;
    return ok;
}

// 8008E9DC
void dFdInfoSvMdlRm_c::destroy(EGG::Heap *heap) {
    if (mBlocks != NULL) {
        mBlocks->release(heap, 0, 1);
        heap->free(mBlocks);
        mBlocks = NULL;
    }
    _24 = NULL;
    fn_8007589C(mBg);
}

// 8008EA5C
dFdInfoSvMdlRm_c *dFdInfoSvMdlRm_c::create(dHomeRoom_c *room, int bgId, EGG::Heap *heap) {
    dFdInfoSvMdlRm_c *info = (dFdInfoSvMdlRm_c *)heap->alloc(sizeof(dFdInfoSvMdlRm_c), 4);
    if (info != NULL) {
        new (info) dFdInfoSvMdlRm_c;
        if (!info->build(room, bgId, heap)) {
            info->destroy(heap);
            heap->free(info);
            info = NULL;
        }
    }
    return info;
}

// 8008EB1C
dFdInfoSvMdlRm_c *dFdInfoSvMdlRm_c::createWithBg0(dHomeRoom_c *room, EGG::Heap *heap) {
    return create(room, 0, heap);
}

// 8008EB28
dFdInfoSvMdlRm_c *dFdInfoSvMdlRm_c::createWithBg15(dHomeRoom_c *room, EGG::Heap *heap) {
    return create(room, 0xF, heap);
}

// 8008EB34
void dFdInfoSvMdlRm_c::remove(dFdInfoSvMdlRm_c **info, EGG::Heap *heap) {
    if (info != NULL && *info != NULL) {
        (*info)->destroy(heap);
        heap->free(*info);
        *info = NULL;
    }
}

// 8008EBA0
BOOL raccoCand_c::check(int x, int z) {
    if (mBlockX != -1 || mBlockZ != -1) {
        int blockX, blockZ;
        fn_8006CD4C(&blockX, &blockZ, x, z);
        if (blockX != mBlockX || blockZ != mBlockZ) {
            return FALSE;
        }
    }
    switch (mInfo->bgCall_80072F80(x, z)) {
    case 0x6C:
    case 0x6D:
    case 0x6E:
    case 0x6F:
    case 0x70:
        if ((s8)dSaveData_c::getTown()->mNewConstruction == NEW_CONSTRUCTION_BRIDGE && mBlockX == -1 && mBlockZ == -1) {
            int blockX, blockZ;
            fn_8006CD4C(&blockX, &blockZ, x, z);
            dSaveTown_c *save1 = dSaveData_c::getTown();
            if (blockX == save1->mNewConstructionBlockX) {
                dSaveTown_c *save2 = dSaveData_c::getTown();
                if (blockZ == save2->mNewConstructionBlockZ) {
                    return FALSE;
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}
