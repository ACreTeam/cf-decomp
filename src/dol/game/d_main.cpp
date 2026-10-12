// main() and the game thread, the process stop reasons, the debug town map (dMain). .text 800CDD2C..800CE43C,
// .bss 805A20F8..805A24E8, .sbss 8074E4D8..8074E4E8, .sdata2 80750870..80750890. See include/game/game/d_main.hpp.
#include <game/game/d_main.hpp>
#include <game/framework/f_manager.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_sys.hpp>
#include <game/mLib/m_tex.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/revolution/OS/OSThread.h>

namespace dMain {

static OSThread sThread;
u32 sStopReason;

} // namespace dMain

#ifdef MUST_MATCH
// 8074E4DC: nothing in the DOL references it (an unused object, kept by the original link).
#pragma force_active on
u8 lbl_8074E4DC[4];
#pragma force_active reset
#endif

namespace dMain {

// 800CDD2C
BOOL isStopped(u32 mask) {
    return (sStopReason & mask) != 0;
}

// 800CDD44
void addStop(u32 mask) {
    u32 reason = sStopReason;
    if (reason == 0) {
        fManager_c::m_StopProcInf |= fManager_c::PROC_FLAG_CONNECT | fManager_c::PROC_FLAG_CREATE |
                                     fManager_c::PROC_FLAG_EXECUTE | fManager_c::PROC_FLAG_DELETE;
    }
    sStopReason = reason | mask;
}

// 800CDD68
void removeStop(u32 mask) {
    sStopReason &= ~mask;
    if (sStopReason == 0) {
        fManager_c::m_StopProcInf &= fManager_c::PROC_FLAG_DRAW;
    }
}

// 800CDD88
void init() {
    fn_8016BA7C();
    fn_8016B97C(0);
}

// 800CDDB0
void loop() {
    while (true) {
        fn_8016C3C0();
    }
}

// 800CDDC4
void *threadMain(void *arg) {
    init();
    loop();
    return NULL;
}

} // namespace dMain

// 800CDDEC
void main() {
    u8 stack[0xF000] ALIGN(32);
    OSThread *thread = OSGetCurrentThread();
    OSCreateThread(&dMain::sThread, dMain::threadMain, NULL, stack + sizeof(stack), sizeof(stack), 16, 0);
    OSResumeThread(&dMain::sThread);
    OSSetThreadPriority(thread, 31);
    OSSuspendThread(thread);
}

namespace dMain {

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

    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
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
    int lastZ = blockH - 1;
    int blockX = column + 1;
    int blockZ = 1;
    if (lastZ > 1) {
        for (; blockZ < lastZ; blockZ++) {
            dFdBlock_c *block = fd->getBlock(blockX, blockZ);
            BOOL hill = block->hasFlag(0xFE00000);
            BOOL cliff = block->hasFlag(0x10000000);
            bool noFlag = block->mFlag == 0;
            for (int i = 0; i < 16; i++) {
                int unitZ = blockZ * 16 + i;
                int texZ = (unitZ - 16) * 3;
                for (int j = 0; j < 16; j++) {
                    int unitX = blockX * 16 + j;
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

} // namespace dMain
