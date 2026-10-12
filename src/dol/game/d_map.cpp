// The debug town map texture maker (dMap). .text 800CDEA0..800CE43C, .bss 805A2410..805A24E8,
// .sbss 8074E4E0..8074E4E8, .sdata2 80750870..80750890. See include/game/game/d_map.hpp.
#include <game/game/d_map.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_field_info.hpp>
#include <game/mLib/m_tex.hpp>
#include <game/mLib/m_vec.hpp>

namespace dMap {

// 800CDEA0
u8 getHeightColor(u8 color, u32 levels, f32 y) {
    if (levels > 2) {
        f32 step = 56.0f / (levels - 1);
        f32 level = 84.0f;
        for (u32 i = 0; i < levels; i++) {
            if (y >= level) {
                return color + i;
            }
            level -= step;
        }
        return color + levels - 1;
    }
    return color;
}

// 800CDF1C
BOOL makeMapColumn(void *tex, int column) {
    if (column < 0 || column >= MAP_COLUMN_NUM) {
        return FALSE;
    }

    int lastZ;
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    int baseZ;
    if (fd == NULL) {
        return FALSE;
    }

    static mVec3_c offsets[9] = {
        mVec3_c(-8.0f, 0.0f, -8.0f), mVec3_c(0.0f, 0.0f, -8.0f), mVec3_c(8.0f, 0.0f, -8.0f),
        mVec3_c(-8.0f, 0.0f, 0.0f),  mVec3_c(0.0f, 0.0f, 0.0f),  mVec3_c(8.0f, 0.0f, 0.0f),
        mVec3_c(-8.0f, 0.0f, 8.0f),  mVec3_c(0.0f, 0.0f, 8.0f),  mVec3_c(8.0f, 0.0f, 8.0f),
    };

    mTex::edit4b_c edit(0x100, 0x100, tex);
    int blockH = fd->getBlockH();
    dBGCF::setCurrentBg(1);
    lastZ = blockH - 1;
    int blockX = column + 1;
    int blockZ = 1;
    if (lastZ > 1) {
        int texZ;
        bool noFlag;
        int baseX = blockX * 16;
        for (baseZ = 16; blockZ < lastZ; baseZ += 16, blockZ++) {
            dFdBlock_c *block = fd->getBlock(blockX, blockZ);
            BOOL hill = block->hasFlag(0xFE00000);
            BOOL cliff = block->hasFlag(0x10000000);
            noFlag = block->mFlag == 0;
            int j, unitX, unitZ;
            for (int i = 0; i < 16; i++) {
                unitZ = baseZ + i;
                texZ = (unitZ - 16) * 3;
                for (j = 0; j < 16; j++) {
                    unitX = baseX + j;
                    int texX = (unitX - 16) * 3;
                    u8 colors[9];
                    dBGCF::getMapColors(colors, unitX, unitZ);
                    u8 *color = colors;
                    mVec3_c *offset = offsets;
                    for (int dz = 0; dz < 3; dz++) {
                        for (int dx = 0; dx < 3; dx++) {
                            int x = texX + dx;
                            int z = texZ + dz;
                            u8 c = *color;
                            if (c == 1) {
                                if (!cliff) {
                                    if (noFlag) {
                                        c = 2;
                                    }
                                } else {
                                    c = getHeightColor(c, 2, dBGCF::getUnitBaseY(unitX, unitZ));
                                }
                            } else if (c == 6) {
                                if (!hill) {
                                    if (noFlag) {
                                        c = 11;
                                    }
                                } else {
                                    mVec3_c pos;
                                    dBGCF::unitToPos(&pos, unitX, unitZ);
                                    mVec3_c p = pos + *offset;
                                    c = getHeightColor(c, 6, dBGCF::getUnitY(&p));
                                }
                            }
                            edit.set(x, z, c, FALSE);
                            color++;
                            offset++;
                        }
                    }
                }
            }
        }
    }
    edit.flush();
    dBGCF::setCurrentBg(0);
    return TRUE;
}

// 800CE334
BOOL makeMap(void *tex, u32 column) {
    if (column == MAP_COLUMN_ALL) {
        bool ok = true;
        ok &= makeMapColumn(tex, 0);
        ok &= makeMapColumn(tex, 1);
        ok &= makeMapColumn(tex, 2);
        ok &= makeMapColumn(tex, 3);
        ok &= makeMapColumn(tex, 4);
        return ok;
    }
    if (column < MAP_COLUMN_NUM) {
        return makeMapColumn(tex, column);
    }
    return FALSE;
}

} // namespace dMap
