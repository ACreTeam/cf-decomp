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
