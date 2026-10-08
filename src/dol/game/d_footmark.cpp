// Footmarks: the grass wear texture (dFootmark::Editor_c). .text 800A773C..800A858C, .ctors,
// .rodata 8046F228..8046F268, .data 804E5B10..804E5C38, .bss 8058B760..80596A20,
// .sdata 8074A3E0..8074A400, .sbss 8074E380..8074E388. See include/game/game/d_footmark.hpp and
// notes/d_footmark.txt.
#include <game/game/d_footmark.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_save_data.hpp>
#include <game/mLib/m_heap.hpp>
#include <revolution/OS/OSCache.h>
#include <cstring>

namespace dFootmark {

// 8074E380: a byte cleared by the __sinit before the editor is built; nothing reads it.
class flag_c {
public:
    flag_c() : mFlag(0) {}

    u8 mFlag;
};
static flag_c l_flag;

// The units of a 7 x 7 block field, one value each (the daily update's growth per unit).
class unitBuf_c {
public:
    enum {
        SIZE = BLOCK_X_NUM * UT_X_NUM, // 112
    };

    unitBuf_c() { clear(); }

    void clear() { memset(mBuf, 0, sizeof(mBuf)); }
    BOOL isIn(u32 x, u32 z) const {
        if (x < SIZE && z < SIZE) {
            return TRUE;
        }
        return FALSE;
    }
    void set(int x, int z, s16 value) {
        if (isIn(x, z)) {
            mBuf[z][x] = value;
        }
    }
    s16 get(int x, int z) const { return isIn(x, z) ? mBuf[z][x] : 0; }

private:
    s16 mBuf[SIZE][SIZE];
}; // size 0x6200

// The eight neighbours of a unit.
struct offset_c {
    int x;
    int z;
};
static const offset_c l_around[8] = {
    {-1, -1}, {0, -1}, {1, -1},
    {-1, 0},           {1, 0},
    {-1, 1},  {0, 1},  {1, 1},
};

// 800A773C
void Editor_c::clearMarks() {
    memset(mMarks, 0, sizeof(mMarks));
    memset(mMarks2, 0, sizeof(mMarks2));
}

// 800A7784
void Editor_c::clearMarks2() {
    memset(mMarks2, 0, sizeof(mMarks2));
}

// 800A7794
void Editor_c::clearPixels() {
    memset(mPixels, 0xFF, sizeof(mPixels));
}

// 800A77A4
void Editor_c::init() {
    mFieldId = FD_ID_TOWN;
    clearPixels();
    memset(mMarks, 0, sizeof(mMarks));
    mTex::edit8b_c::init(TEX_SIZE, TEX_SIZE, mPixels);
}

// 800A77FC
BOOL Editor_c::isMarked(int x, int z) const {
    u32 idx = x + z * TEX_SIZE;
    if (idx < TEX_SIZE * TEX_SIZE) {
        return (mMarks[idx >> 3] >> (idx & 7)) & 1;
    }
    return FALSE;
}

// 800A7830
void Editor_c::setMarked(int x, int z) {
    u32 idx = x + z * TEX_SIZE;
    if (idx < TEX_SIZE * TEX_SIZE) {
        mMarks[idx >> 3] |= 1 << (idx & 7);
    }
}

// The town's outer ring of blocks (the cliffs and the sea around the playable 5 x 5).
inline BOOL Editor_c::isBorderBlock(u32 blockX, u32 blockZ) const {
    BOOL border = FALSE;
    if (blockX == 0 || blockZ == 0 || blockX == BLOCK_X_NUM - 1 || blockZ == BLOCK_Z_NUM - 1) {
        if (mFieldId == FD_ID_TOWN) {
            border = TRUE;
        }
    }
    return border;
}

// 800A7864
void Editor_c::setBlock(const u8 *units, u32 blockX, u32 blockZ) {
    int z;
    int x;
    int unitX;
    int unitZ;
    BOOL border = isBorderBlock(blockX, blockZ);
    if (units != NULL) {
        mTex::edit8b_c tex(UT_X_NUM, UT_Z_NUM, (void *)units);
        for (z = 0; z < UT_Z_NUM; z++) {
            unitZ = z + blockZ * UT_Z_NUM;
            for (x = 0; x < UT_X_NUM; x++) {
                unitX = x + blockX * UT_X_NUM;
                set(unitX, unitZ, border ? 0 : tex.get(x, z), FALSE);
            }
        }
    }
}

// 800A7994
u8 *Editor_c::getSaveUnits(int blockX, int blockZ) const {
    return dSaveData_c::getTown()->mMainField.getBlockGrassWearUnits(blockX, blockZ);
}

static inline dFdBlock_c *getFdBlock(dFdBase_c *fd, int blockX, int blockZ) {
    return fd != NULL ? fd->getBlock(blockX, blockZ) : NULL;
}

static inline int getBlockType(dFdBlock_c *block) {
    return block != NULL ? block->mType : 0;
}

// 800A79E0
void Editor_c::onLoaded() {
    dDvd::brresBank_c::onLoaded();
    init();
    dFdBase_c *fd = fn_80190C44(mFieldId);
    for (int blockZ = 0; blockZ < fd->mBlockH; blockZ++) {
        for (int blockX = 0; blockX < fd->mBlockW; blockX++) {
            u8 *units = getTexImage(getBlockType(getFdBlock(fd, blockX, blockZ)));
            if (units != NULL) {
                u8 *save = getSaveUnits(blockX, blockZ);
                if (save != NULL) {
                    memcpy(save, units, UT_X_NUM * UT_Z_NUM);
                }
                setBlock(units, blockX, blockZ);
            }
        }
    }
    clampAll();
    DCStoreRangeNoSync(mPixels, sizeof(mPixels));
}

// 800A7B18
BOOL Editor_c::create(EGG::Heap *heap, int fieldId) {
    if (m_frmHeap_p == NULL) {
        m_frmHeap_p = mHeap::createFrmHeap(0xF700, heap, "dFootmark::Editor_c::m_frmHeap_p : 草はげテクスチャバッファ", 0x20,
                                           mHeap::OPT_NONE);
    }
    mFieldId = fieldId;
    if (!load(m_frmHeap_p)) {
        return FALSE;
    }
    return TRUE;
}

// 800A7B9C
BOOL Editor_c::destroy() {
    if (unload(FALSE)) {
        if (m_frmHeap_p != NULL) {
            mHeap::destroyFrmHeap(m_frmHeap_p);
            m_frmHeap_p = NULL;
        }
        return TRUE;
    }
    return FALSE;
}

// 800A7BF8
void Editor_c::loadSave() {
    init();
    for (u32 blockZ = 0; blockZ < BLOCK_Z_NUM; blockZ++) {
        for (u32 blockX = 0; blockX < BLOCK_X_NUM; blockX++) {
            setBlock(getSaveUnits(blockX, blockZ), blockX, blockZ);
        }
    }
    DCStoreRangeNoSync(mPixels, sizeof(mPixels));
}

// 800A7C84
void Editor_c::setUnit(int x, int z, int value, BOOL flush) {
    int min = getMin(x, z);
    int max = getMax(x, z);
    if (value < min) {
        value = min;
    } else if (value > max) {
        value = max;
    }
    int blockX, blockZ;
    dBGCF::unitToBlock(&blockX, &blockZ, x, z);
    u8 *save = getSaveUnits(blockX, blockZ);
    if (save != NULL) {
        mTex::edit8b_c tex(UT_X_NUM, UT_Z_NUM, save);
        tex.set(x & (UT_X_NUM - 1), z & (UT_Z_NUM - 1), value, FALSE);
    }
    mTex::edit8b_c::set(x, z, value, flush);
}

inline void Editor_c::setCurrentBg() const {
    if (mFieldId == FD_ID_TOWN) {
        dBGCF::setCurrentBg(1);
    } else {
        dBGCF::setCurrentBg(0);
    }
}

// 800A7DA0
u8 Editor_c::getMin(int x, int z) const {
    setCurrentBg();
    int min = dBGCF::getGrassMin(x, z);
    dBGCF::setCurrentBg(0);
    return min;
}

// 800A7E10
u8 Editor_c::getMax(int x, int z) const {
    setCurrentBg();
    int max = dBGCF::getGrassMax(x, z);
    dBGCF::setCurrentBg(0);
    return max;
}

// 800A7E80
void Editor_c::addUnit(int x, int z, int delta, BOOL force, BOOL flush) {
    if (force || !isMarked(x, z)) {
        u8 value = get(x, z);
        setUnit(x, z, value + delta, flush);
        setMarked(x, z);
    }
}

// 800A7F14
void Editor_c::addPos(const mVec3_c *pos, int delta, BOOL force, BOOL flush) {
    int x, z;
    dBGCF::posToUnit(&x, &z, pos);
    if (x >= 0 && z >= 0) {
        addUnit(x, z, delta, force, flush);
    }
}

// 800A7F9C
void Editor_c::updateDay(int days, int arg, BOOL flush) {
    if (days < 1) {
        return;
    }
    dFdBase_c *fd = fn_80190C44(mFieldId);
    if (fd == NULL) {
        return;
    }
    int maxX = (fd->mBlockW - 1) * UT_X_NUM - 1;
    int maxZ = (fd->mBlockH - 1) * UT_Z_NUM - 1;
    clearMarks();
    for (int z = UT_Z_NUM; z <= maxZ; z++) {
        for (int x = UT_X_NUM; x <= maxX; x++) {
            dItem::Item *item = fd->getItem(x, z, 0);
            if (item != NULL) {
                dItem::FgInfo *info = item->getFgInfo();
                if (info != NULL) {
                    addUnit(x, z, info->mGrassGrowth, TRUE, FALSE);
                }
            }
        }
    }

    static unitBuf_c s_growth;
    s_growth.clear();
    for (int z = UT_Z_NUM; z <= maxZ; z++) {
        for (int x = UT_X_NUM; x <= maxX; x++) {
            int sum = 0;
            for (const offset_c *ofs = l_around; ofs < l_around + 8; ofs++) {
                sum += get(x + ofs->x, z + ofs->z);
            }
            s16 growth = sum / 8 / 85 + 1;
            s_growth.set(x, z, growth);
        }
    }
    for (int z = UT_Z_NUM; z <= maxZ; z++) {
        for (int x = UT_X_NUM; x <= maxX; x++) {
            addUnit(x, z, s_growth.get(x, z), TRUE, FALSE);
        }
    }
    if (flush) {
        mTex::edit8b_c::flush();
    }
    clearMarks();
}

// 800A822C
void Editor_c::clampAll() {
    dFdBase_c *fd = fn_80190C44(mFieldId);
    int blockX;
    int blockZ;
    int z;
    int x;
    int unitX;
    int unitZ;
    for (blockX = 0; blockX < fd->mBlockW; blockX++) {
        // The zero-trip test before the loop is what the target's code has (see notes).
        blockZ = 0;
        if (fd->mBlockH > 0) {
            for (; blockZ < fd->mBlockH; blockZ++) {
                for (z = 0; z < UT_Z_NUM; z++) {
                    unitZ = (blockZ << 4) + z;
                    for (x = 0; x < UT_X_NUM; x++) {
                        unitX = (blockX << 4) + x;
                        int min = getMin(unitX, unitZ);
                        int max = getMax(unitX, unitZ);
                        int value = get(unitX, unitZ);
                        if (value < min) {
                            setUnit(unitX, unitZ, min, FALSE);
                        } else if (value > max) {
                            setUnit(unitX, unitZ, max, FALSE);
                        }
                    }
                }
            }
        }
    }
}

static Editor_c l_editor;

// 800A835C
Editor_c *getEditor() {
    return &l_editor;
}

} // namespace dFootmark
