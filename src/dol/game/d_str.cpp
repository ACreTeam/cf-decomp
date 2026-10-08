// dSTR: town structures (banks, flatness/copy checks, collision bank) and the per-unit field
// attribute map dFdUnitAttr_c. .text 80166990..8016AE68.
// TODO: only dFdUnitAttr_c (80167C08..80167E08) is decompiled. The rest of the TU, the dSTR code
// before and after it plus its data and __sinit (8016AB1C), still has to be done.
#include <game/game/d_field_info.hpp>

extern "C" void fn_8006CD4C(int *blockX, int *blockZ, int unitX, int unitZ); // 8006CD4C: block of a unit

// 80167C08
void dFdUnitAttr_c::setAttr(int unitX, int unitZ, u32 attr) {
    int blockX;
    int blockZ;
    fn_8006CD4C(&blockX, &blockZ, unitX, unitZ);
    if (blockX >= 0 && (u32)blockX < BLOCK_X_NUM && blockZ >= 0 && (u32)blockZ < BLOCK_Z_NUM) {
        int idx = (unitX & (UT_X_NUM - 1)) + (unitZ & (UT_Z_NUM - 1)) * UT_X_NUM;
        mBlocks[blockZ][blockX].set(idx, attr);
    }
}

// 80167CD8
u32 dFdUnitAttr_c::getAttr(int unitX, int unitZ) {
    int blockX;
    int blockZ;
    fn_8006CD4C(&blockX, &blockZ, unitX, unitZ);
    if (blockX >= 0 && (u32)blockX < BLOCK_X_NUM && blockZ >= 0 && (u32)blockZ < BLOCK_Z_NUM) {
        int idx = (unitX & (UT_X_NUM - 1)) + (unitZ & (UT_Z_NUM - 1)) * UT_X_NUM;
        return mBlocks[blockZ][blockX].get(idx);
    }
    return 0;
}

// 80167D8C
BOOL dFdUnitAttr_c::isGroundFree(int unitX, int unitZ) {
    return (getAttr(unitX, unitZ) & (STR_COL | STR_ENT)) == 0;
}

// 80167DB8
BOOL dFdUnitAttr_c::isNotStrCol(int unitX, int unitZ) {
    return !(getAttr(unitX, unitZ) & STR_COL);
}

// 80167DE4
BOOL dFdUnitAttr_c::isNoPlantUnit(int unitX, int unitZ) {
    return (getAttr(unitX, unitZ) & STR_DEL) != 0;
}
