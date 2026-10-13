#pragma once

// d_fd_block_type (DOL TU d_fd_block_type.cpp, .text 800812C8..80081618, .rodata 8046D460..8046D5A0, .data
// 804A76B8..804A7780): the kind of each block (acre) type and the flags of each kind (d_block_kind.hpp).
// Namespace from the file name; function names inferred from what they do and from their callers
// (d_random_field, d_field_block, d_field_info, d_save_main_field, d_field_assessment, d_npc_mng).

#include <types.h>
#include <game/game/d_block_kind.hpp>

#define BLOCK_TYPE_NUM 0xB8 // block types (RF_TYPE_NUM)

namespace dFdBlockType {

int getKind(int type);                 // 800812C8: BLOCK_KIND_* of a block type (0 for an invalid type)
int findKind(int flags);               // 800812E8: the kind with exactly these flags, or BLOCK_KIND_NONE
int getTypeFlags(int type);            // 80081324: getKindFlags(getKind(type))
u32 getKindFlags(int kind);            // 80081348: a kind's BLOCK_KIND_FLAG_* (0 for an invalid kind)
u8 getRiverNum(int kind);              // 8008136C: how many BLOCK_KIND_FLAG_RIVER_* bits the kind has
int getTypeIndex(int type);            // 80081410: index of type among the types with its flags, or -1
int getNthType(int flags, u32 index);  // 80081494: the index-th type with exactly these flags, or -1
int getBridgeType(int type);           // 80081514: the bridge variant of a block type (or the type itself)

} // namespace dFdBlockType
