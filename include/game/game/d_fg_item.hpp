#pragma once

#include <types.h>
#include <game/game/d_item_def.hpp>
#include <game/game/d_ftr_list.hpp>

#define ITEM_NAME_LEN 16

// The 2-byte item value used everywhere an item can sit: field-object ids
// below 0xE5 (fg_treeA_x, fg_apple, ... in the 804E3C40 table) and encoded
// BITM ids (categories 9..12 in bits 12-15, 0x9000 + (baseId << 2), with
// the low 2 bits as a variant).
//
// The class has no virtuals, so there is no RTTI and its real name is
// unknown. Its out-of-line members live in this TU (800A5D3C..800A70AC).
// The user-declared destructor makes MWCC return it through a hidden pointer.
namespace dItem {

enum {
    ITEM_ID_NONE = 0xFFF1,

    // Field objects index the fg info table directly.
    FG_COUNT = 0xE5,
};

// Field object ("fg") ids: dItem::Item::mId values below FG_COUNT (0xE5), indices into the
// FgInfo table (d_fg_item.cpp). Each comment gives the FgInfo model name and a short note.
enum FgId {
    FG_UNK_00                          = 0x00, // fg_apple, empty FgInfo entry (placeholder model)
    FG_DEAD_SAPLING                    = 0x01, // fg_treeA_x, killTree of FG_TREE_SAPLING / FG_MONEY_TREE_SAPLING
    FG_DEAD_FRUIT_SAPLING              = 0x02, // fg_treeA_x, killTree of a fruit-tree sapling
    FG_DEAD_CEDAR_SAPLING              = 0x03, // fg_treeB_x, killTree of FG_CEDAR_SAPLING
    FG_DEAD_PALM_SAPLING               = 0x04, // fg_treeC_x, killTree of FG_PALM_SAPLING
    FG_STUMP_S1                        = 0x05, // fg_treeA_1_b, stump (FgInfo _00) of a stage 1 tree / fruit tree / money tree
    FG_STUMP_S2                        = 0x06, // fg_treeA_2_b, stump (FgInfo _00) of a stage 2 tree / fruit tree / money tree
    FG_STUMP_S3                        = 0x07, // fg_treeA_3_b, stump (FgInfo _00) of a stage 3 tree / fruit tree / money tree
    FG_STUMP                           = 0x08, // fg_treeA_4_b, stump of a grown tree / fruit tree / money tree
    FG_CEDAR_STUMP_S1                  = 0x09, // fg_treeB_1_b, stump (FgInfo _00) of a stage 1 cedar
    FG_CEDAR_STUMP_S2                  = 0x0A, // fg_treeB_2_b, stump (FgInfo _00) of a stage 2 cedar
    FG_CEDAR_STUMP_S3                  = 0x0B, // fg_treeB_3_b, stump (FgInfo _00) of a stage 3 cedar
    FG_CEDAR_STUMP                     = 0x0C, // fg_treeB_4_b, stump of a grown cedar
    FG_PALM_STUMP_S1                   = 0x0D, // fg_treeC_1_b, stump (FgInfo _00) of a stage 1 palm
    FG_PALM_STUMP_S2                   = 0x0E, // fg_treeC_2_b, stump (FgInfo _00) of a stage 2 palm
    FG_PALM_STUMP_S3                   = 0x0F, // fg_treeC_3_b, stump (FgInfo _00) of a stage 3 palm
    FG_PALM_STUMP                      = 0x10, // fg_treeC_4_b, stump of a grown palm
    FG_TREE_SAPLING                    = 0x11, // fg_treeA_0, planted by ITEM_IDX_SAPLING; buried money (golden shovel)
    FG_TREE_S1                         = 0x12, // fg_treeA_1, stage 1
    FG_TREE_S2                         = 0x13, // fg_treeA_2, stage 2
    FG_TREE_S3                         = 0x14, // fg_treeA_3, stage 3
    FG_TREE                            = 0x15, // fg_treeA_4, grown tree
    FG_TREE_FTR                        = 0x16, // fg_treeA_4, shakes out a random item; 2 per town (plantTreesA): probably furniture
    FG_TREE_BEES                       = 0x17, // fg_treeA_4, one per block column (plantTreesB): probably bees (hedged)
    FG_TREE_BELLS                      = 0x18, // fg_treeA_4, shakes out 100 (300) bells; one per block (plantTreesC)
    FG_APPLE_TREE_SAPLING              = 0x19, // fg_treeA_0, planted by ITEM_IDX_APPLE
    FG_APPLE_TREE_S1                   = 0x1A, // fg_treeA_1, stage 1
    FG_APPLE_TREE_S2                   = 0x1B, // fg_treeA_2, stage 2
    FG_APPLE_TREE_S3                   = 0x1C, // fg_treeA_3, stage 3
    FG_APPLE_TREE_NOFRUIT_0            = 0x1D, // fg_treeA_4, grown, no fruit (+1 a day)
    FG_APPLE_TREE_NOFRUIT_1            = 0x1E, // fg_treeA_4
    FG_APPLE_TREE_NOFRUIT_2            = 0x1F, // fg_treeA_4
    FG_APPLE_TREE_FRUIT                = 0x20, // fg_treeA_4, grown with fruit, drops ITEM_IDX_APPLE
    FG_ORANGE_TREE_SAPLING             = 0x21, // fg_treeA_0, planted by ITEM_IDX_ORANGE
    FG_ORANGE_TREE_S1                  = 0x22, // fg_treeA_1, stage 1
    FG_ORANGE_TREE_S2                  = 0x23, // fg_treeA_2, stage 2
    FG_ORANGE_TREE_S3                  = 0x24, // fg_treeA_3, stage 3
    FG_ORANGE_TREE_NOFRUIT_0           = 0x25, // fg_treeA_4, grown, no fruit (+1 a day)
    FG_ORANGE_TREE_NOFRUIT_1           = 0x26, // fg_treeA_4
    FG_ORANGE_TREE_NOFRUIT_2           = 0x27, // fg_treeA_4
    FG_ORANGE_TREE_FRUIT               = 0x28, // fg_treeA_4, grown with fruit, drops ITEM_IDX_ORANGE
    FG_PEAR_TREE_SAPLING               = 0x29, // fg_treeA_0, planted by ITEM_IDX_PEAR
    FG_PEAR_TREE_S1                    = 0x2A, // fg_treeA_1, stage 1
    FG_PEAR_TREE_S2                    = 0x2B, // fg_treeA_2, stage 2
    FG_PEAR_TREE_S3                    = 0x2C, // fg_treeA_3, stage 3
    FG_PEAR_TREE_NOFRUIT_0             = 0x2D, // fg_treeA_4, grown, no fruit (+1 a day)
    FG_PEAR_TREE_NOFRUIT_1             = 0x2E, // fg_treeA_4
    FG_PEAR_TREE_NOFRUIT_2             = 0x2F, // fg_treeA_4
    FG_PEAR_TREE_FRUIT                 = 0x30, // fg_treeA_4, grown with fruit, drops ITEM_IDX_PEAR
    FG_PEACH_TREE_SAPLING              = 0x31, // fg_treeA_0, planted by ITEM_IDX_PEACH
    FG_PEACH_TREE_S1                   = 0x32, // fg_treeA_1, stage 1
    FG_PEACH_TREE_S2                   = 0x33, // fg_treeA_2, stage 2
    FG_PEACH_TREE_S3                   = 0x34, // fg_treeA_3, stage 3
    FG_PEACH_TREE_NOFRUIT_0            = 0x35, // fg_treeA_4, grown, no fruit (+1 a day)
    FG_PEACH_TREE_NOFRUIT_1            = 0x36, // fg_treeA_4
    FG_PEACH_TREE_NOFRUIT_2            = 0x37, // fg_treeA_4
    FG_PEACH_TREE_FRUIT                = 0x38, // fg_treeA_4, grown with fruit, drops ITEM_IDX_PEACH
    FG_CHERRY_TREE_SAPLING             = 0x39, // fg_treeA_0, planted by ITEM_IDX_CHERRY
    FG_CHERRY_TREE_S1                  = 0x3A, // fg_treeA_1, stage 1
    FG_CHERRY_TREE_S2                  = 0x3B, // fg_treeA_2, stage 2
    FG_CHERRY_TREE_S3                  = 0x3C, // fg_treeA_3, stage 3
    FG_CHERRY_TREE_NOFRUIT_0           = 0x3D, // fg_treeA_4, grown, no fruit (+1 a day)
    FG_CHERRY_TREE_NOFRUIT_1           = 0x3E, // fg_treeA_4
    FG_CHERRY_TREE_NOFRUIT_2           = 0x3F, // fg_treeA_4
    FG_CHERRY_TREE_FRUIT               = 0x40, // fg_treeA_4, grown with fruit, drops ITEM_IDX_CHERRY
    FG_PALM_SAPLING                    = 0x41, // fg_treeC_0, planted by ITEM_IDX_COCONUT
    FG_PALM_S1                         = 0x42, // fg_treeC_1, stage 1
    FG_PALM_S2                         = 0x43, // fg_treeC_2, stage 2
    FG_PALM_S3                         = 0x44, // fg_treeC_3, stage 3
    FG_PALM_NOFRUIT_0                  = 0x45, // fg_treeC_4, grown, no fruit (+1 a day)
    FG_PALM_NOFRUIT_1                  = 0x46, // fg_treeC_4
    FG_PALM_NOFRUIT_2                  = 0x47, // fg_treeC_4
    FG_PALM_FRUIT                      = 0x48, // fg_treeC_4, grown with fruit, drops ITEM_IDX_COCONUT
    FG_MONEY_TREE_SAPLING              = 0x49, // fg_treeA_0, buried money with the golden shovel (by chance)
    FG_MONEY_TREE_S1                   = 0x4A, // fg_treeA_1, stage 1
    FG_MONEY_TREE_S2                   = 0x4B, // fg_treeA_2, stage 2
    FG_MONEY_TREE_S3                   = 0x4C, // fg_treeA_3, stage 3
    FG_MONEY_TREE                      = 0x4D, // fg_treeA_4, grown, drops ITEM_IDX_30000_BELLS (getFruitTreeType 6)
    FG_CEDAR_SAPLING                   = 0x4E, // fg_treeB_0, planted by ITEM_IDX_CEDAR_SAPLING
    FG_CEDAR_S1                        = 0x4F, // fg_treeB_1, stage 1
    FG_CEDAR_S2                        = 0x50, // fg_treeB_2, stage 2
    FG_CEDAR_S3                        = 0x51, // fg_treeB_3, stage 3
    FG_CEDAR                           = 0x52, // fg_treeB_4, grown cedar
    FG_CEDAR_FTR                       = 0x53, // fg_treeB_4, cedar FG_TREE_FTR
    FG_CEDAR_BEES                      = 0x54, // fg_treeB_4, cedar FG_TREE_BEES (hedged)
    FG_CEDAR_BELLS                     = 0x55, // fg_treeB_4, cedar FG_TREE_BELLS
    FG_CEDAR_LIGHTS                    = 0x56, // fg_treeB_4, FG_CEDAR decorated Dec 15 .. Jan 3 (updateFg56)
    FG_WEED_A                          = 0x57, // fg_grassA
    FG_WEED_B                          = 0x58, // fg_grassB
    FG_WEED_C                          = 0x59, // fg_grassC
    FG_WEED_D                          = 0x5A, // fg_grassD
    FG_STONE_A                         = 0x5B, // fg_stoneA
    FG_STONE_B                         = 0x5C, // fg_stoneB
    FG_STONE_C                         = 0x5D, // fg_stoneC
    FG_STONE_D                         = 0x5E, // fg_stoneD
    FG_STONE_E                         = 0x5F, // fg_stoneE
    FG_MONEY_ROCK_P0_A                 = 0x60, // fg_stoneA, player 0's money rock (changeStones; (id - 0x60) / 5 is the player), reverts to FG_STONE_A
    FG_MONEY_ROCK_P0_B                 = 0x61, // fg_stoneB
    FG_MONEY_ROCK_P0_C                 = 0x62, // fg_stoneC
    FG_MONEY_ROCK_P0_D                 = 0x63, // fg_stoneD
    FG_MONEY_ROCK_P0_E                 = 0x64, // fg_stoneE
    FG_MONEY_ROCK_P1_A                 = 0x65, // fg_stoneA, player 1's money rock (changeStones; (id - 0x60) / 5 is the player), reverts to FG_STONE_A
    FG_MONEY_ROCK_P1_B                 = 0x66, // fg_stoneB
    FG_MONEY_ROCK_P1_C                 = 0x67, // fg_stoneC
    FG_MONEY_ROCK_P1_D                 = 0x68, // fg_stoneD
    FG_MONEY_ROCK_P1_E                 = 0x69, // fg_stoneE
    FG_MONEY_ROCK_P2_A                 = 0x6A, // fg_stoneA, player 2's money rock (changeStones; (id - 0x60) / 5 is the player), reverts to FG_STONE_A
    FG_MONEY_ROCK_P2_B                 = 0x6B, // fg_stoneB
    FG_MONEY_ROCK_P2_C                 = 0x6C, // fg_stoneC
    FG_MONEY_ROCK_P2_D                 = 0x6D, // fg_stoneD
    FG_MONEY_ROCK_P2_E                 = 0x6E, // fg_stoneE
    FG_MONEY_ROCK_P3_A                 = 0x6F, // fg_stoneA, player 3's money rock (changeStones; (id - 0x60) / 5 is the player), reverts to FG_STONE_A
    FG_MONEY_ROCK_P3_B                 = 0x70, // fg_stoneB
    FG_MONEY_ROCK_P3_C                 = 0x71, // fg_stoneC
    FG_MONEY_ROCK_P3_D                 = 0x72, // fg_stoneD
    FG_MONEY_ROCK_P3_E                 = 0x73, // fg_stoneE
    FG_DESIGN_P0_0                     = 0x74, // fg_apple, a custom design laid on the ground: 0x74 + player * 8 + slot (KIND_PUT_DESIGN)
    FG_DESIGN_P0_1                     = 0x75, // fg_apple
    FG_DESIGN_P0_2                     = 0x76, // fg_apple
    FG_DESIGN_P0_3                     = 0x77, // fg_apple
    FG_DESIGN_P0_4                     = 0x78, // fg_apple
    FG_DESIGN_P0_5                     = 0x79, // fg_apple
    FG_DESIGN_P0_6                     = 0x7A, // fg_apple
    FG_DESIGN_P0_7                     = 0x7B, // fg_apple
    FG_DESIGN_P1_0                     = 0x7C, // fg_apple
    FG_DESIGN_P1_1                     = 0x7D, // fg_apple
    FG_DESIGN_P1_2                     = 0x7E, // fg_apple
    FG_DESIGN_P1_3                     = 0x7F, // fg_apple
    FG_DESIGN_P1_4                     = 0x80, // fg_apple
    FG_DESIGN_P1_5                     = 0x81, // fg_apple
    FG_DESIGN_P1_6                     = 0x82, // fg_apple
    FG_DESIGN_P1_7                     = 0x83, // fg_apple
    FG_DESIGN_P2_0                     = 0x84, // fg_apple
    FG_DESIGN_P2_1                     = 0x85, // fg_apple
    FG_DESIGN_P2_2                     = 0x86, // fg_apple
    FG_DESIGN_P2_3                     = 0x87, // fg_apple
    FG_DESIGN_P2_4                     = 0x88, // fg_apple
    FG_DESIGN_P2_5                     = 0x89, // fg_apple
    FG_DESIGN_P2_6                     = 0x8A, // fg_apple
    FG_DESIGN_P2_7                     = 0x8B, // fg_apple
    FG_DESIGN_P3_0                     = 0x8C, // fg_apple
    FG_DESIGN_P3_1                     = 0x8D, // fg_apple
    FG_DESIGN_P3_2                     = 0x8E, // fg_apple
    FG_DESIGN_P3_3                     = 0x8F, // fg_apple
    FG_DESIGN_P3_4                     = 0x90, // fg_apple
    FG_DESIGN_P3_5                     = 0x91, // fg_apple
    FG_DESIGN_P3_6                     = 0x92, // fg_apple
    FG_DESIGN_P3_7                     = 0x93, // fg_apple
    FG_HOLE                            = 0x94, // fg_apple, probably a dug hole (shovel finds it in front)
    FG_RED_TURNIP_WILTED               = 0x95, // fg_r_kabu_x, picks as ITEM_IDX_SPOILED_TURNIPS; unwatered plant
    FG_RED_TURNIP_0                    = 0x96, // fg_r_kabu_p, picks as ITEM_IDX_RED_TURNIP_00
    FG_RED_TURNIP_1                    = 0x97, // fg_r_kabu_p, picks as ITEM_IDX_RED_TURNIP_01
    FG_RED_TURNIP_2                    = 0x98, // fg_r_kabu_p, picks as ITEM_IDX_RED_TURNIP_02
    FG_RED_TURNIP_3                    = 0x99, // fg_r_kabu_p, picks as ITEM_IDX_RED_TURNIP_03
    FG_RED_TURNIP_4                    = 0x9A, // fg_r_kabu_p, picks as ITEM_IDX_RED_TURNIP_04
    FG_RED_TURNIP_5                    = 0x9B, // fg_r_kabu_p, picks as ITEM_IDX_RED_TURNIP_05
    FG_RED_TURNIP_6                    = 0x9C, // fg_r_kabu_p, picks as ITEM_IDX_RED_TURNIP_06
    FG_RED_TURNIP_WILTED_WATERED       = 0x9D, // fg_r_kabu_x, silver can on FG_RED_TURNIP_WILTED; -> FG_RED_TURNIP_0
    FG_TULIP_RED                       = 0x9E, // flw_tulip_rd
    FG_TULIP_WHITE                     = 0x9F, // flw_tulip_wt
    FG_TULIP_YELLOW                    = 0xA0, // flw_tulip_ye
    FG_TULIP_PINK                      = 0xA1, // flw_tulip_pi
    FG_TULIP_PURPLE                    = 0xA2, // flw_tulip_vi
    FG_TULIP_BLACK                     = 0xA3, // flw_tulip_bk
    FG_PANSY_WHITE                     = 0xA4, // flw_pansy_wt
    FG_PANSY_YELLOW                    = 0xA5, // flw_pansy_ye
    FG_PANSY_RED                       = 0xA6, // flw_pansy_rd
    FG_PANSY_PURPLE                    = 0xA7, // flw_pansy_vi
    FG_PANSY_ORANGE                    = 0xA8, // flw_pansy_ry, ITEM_IDX_ORANGE_PANSIES
    FG_PANSY_BLUE                      = 0xA9, // flw_pansy_bl
    FG_COSMOS_WHITE                    = 0xAA, // flw_cosmos_wt
    FG_COSMOS_RED                      = 0xAB, // flw_cosmos_rd
    FG_COSMOS_YELLOW                   = 0xAC, // flw_cosmos_ye
    FG_COSMOS_PINK                     = 0xAD, // flw_cosmos_pi
    FG_COSMOS_ORANGE                   = 0xAE, // flw_cosmos_or
    FG_COSMOS_BLACK                    = 0xAF, // flw_cosmos_bk
    FG_ROSE_RED                        = 0xB0, // flw_rose_rd
    FG_ROSE_WHITE                      = 0xB1, // flw_rose_wt
    FG_ROSE_YELLOW                     = 0xB2, // flw_rose_ye
    FG_ROSE_PINK                       = 0xB3, // flw_rose_pi
    FG_ROSE_ORANGE                     = 0xB4, // flw_rose_or
    FG_ROSE_PURPLE                     = 0xB5, // flw_rose_vi
    FG_ROSE_BLACK                      = 0xB6, // flw_rose_bk
    FG_ROSE_BLUE                       = 0xB7, // flw_rose_bl
    FG_ROSE_GOLD                       = 0xB8, // flw_rose_gl, no plant item, never wilts randomly
    FG_CARNATION_RED                   = 0xB9, // flw_carna_rd
    FG_CARNATION_PINK                  = 0xBA, // flw_carna_pi
    FG_CARNATION_WHITE                 = 0xBB, // flw_carna_wt
    FG_JACOBS_LADDER                   = 0xBC, // flw_lily, ITEM_IDX_JACOBS_LADDER
    FG_RAFFLESIA                       = 0xBD, // flw_rafflesia
    FG_WILTED_TULIP_RED                = 0xBE, // flw_tulip_rd_x
    FG_WILTED_TULIP_WHITE              = 0xBF, // flw_tulip_wt_x
    FG_WILTED_TULIP_YELLOW             = 0xC0, // flw_tulip_ye_x
    FG_WILTED_TULIP_PINK               = 0xC1, // flw_tulip_pi_x
    FG_WILTED_TULIP_PURPLE             = 0xC2, // flw_tulip_vi_x
    FG_WILTED_TULIP_BLACK              = 0xC3, // flw_tulip_bk_x
    FG_WILTED_PANSY_WHITE              = 0xC4, // flw_pansy_wt_x
    FG_WILTED_PANSY_YELLOW             = 0xC5, // flw_pansy_ye_x
    FG_WILTED_PANSY_RED                = 0xC6, // flw_pansy_rd_x
    FG_WILTED_PANSY_PURPLE             = 0xC7, // flw_pansy_vi_x
    FG_WILTED_PANSY_ORANGE             = 0xC8, // flw_pansy_ry_x
    FG_WILTED_PANSY_BLUE               = 0xC9, // flw_pansy_bl_x
    FG_WILTED_COSMOS_WHITE             = 0xCA, // flw_cosmos_wt_x
    FG_WILTED_COSMOS_RED               = 0xCB, // flw_cosmos_rd_x
    FG_WILTED_COSMOS_YELLOW            = 0xCC, // flw_cosmos_ye_x
    FG_WILTED_COSMOS_PINK              = 0xCD, // flw_cosmos_pi_x
    FG_WILTED_COSMOS_ORANGE            = 0xCE, // flw_cosmos_or_x
    FG_WILTED_COSMOS_BLACK             = 0xCF, // flw_cosmos_bk_x
    FG_WILTED_ROSE_RED                 = 0xD0, // flw_rose_rd_x
    FG_WILTED_ROSE_WHITE               = 0xD1, // flw_rose_wt_x
    FG_WILTED_ROSE_YELLOW              = 0xD2, // flw_rose_ye_x
    FG_WILTED_ROSE_PINK                = 0xD3, // flw_rose_pi_x
    FG_WILTED_ROSE_ORANGE              = 0xD4, // flw_rose_or_x
    FG_WILTED_ROSE_PURPLE              = 0xD5, // flw_rose_vi_x
    FG_WILTED_ROSE_BLACK               = 0xD6, // flw_rose_bk_x
    FG_WILTED_ROSE_BLUE                = 0xD7, // flw_rose_bl_x
    FG_WILTED_ROSE_GOLD                = 0xD8, // flw_rose_bk_x, model reuses the black one
    FG_WILTED_CARNATION_RED            = 0xD9, // flw_carna_rd_x
    FG_WILTED_CARNATION_PINK           = 0xDA, // flw_carna_pi_x
    FG_WILTED_CARNATION_WHITE          = 0xDB, // flw_carna_wt_x
    FG_WILTED_JACOBS_LADDER            = 0xDC, // flw_lily_x
    FG_WILTED_RAFFLESIA                = 0xDD, // flw_rafflesia_x
    FG_DANDELION                       = 0xDE, // flw_dandelion0
    FG_DANDELION_PUFF                  = 0xDF, // flw_dandelion1
    FG_CLOVER                          = 0xE0, // fg_clover
    FG_LUCKY_CLOVER                    = 0xE1, // fg_clover, 1% instead of FG_CLOVER; ITEM_IDX_LUCKY_CLOVER plants it
    FG_SEED                            = 0xE2, // fg_tane, base fg while a seed bag / flower / red turnip is planted
    FG_HONEYCOMB                       = 0xE3, // fg_honeycomb, not used by the fg manager code
    FG_4LEAF_CLOVER                    = 0xE4, // fg_4leaf_clover, not used by the fg manager code (cf. FG_LUCKY_CLOVER)

    // Ranges the code tests (inclusive).
    FG_DEAD_SAPLING_FIRST              = FG_DEAD_SAPLING, // 0x01
    FG_DEAD_SAPLING_LAST               = FG_DEAD_PALM_SAPLING, // 0x04
    FG_STUMP_FIRST                     = FG_STUMP_S1, // 0x05
    FG_STUMP_LAST                      = FG_PALM_STUMP, // 0x10
    FG_PALM_STUMP_FIRST                = FG_PALM_STUMP_S1, // 0x0D
    FG_PALM_STUMP_LAST                 = FG_PALM_STUMP, // 0x10
    FG_TREE_FIRST                      = FG_TREE_SAPLING, // 0x11
    FG_TREE_LAST                       = FG_TREE_BELLS, // 0x18
    FG_FRUIT_TREE_FIRST                = FG_APPLE_TREE_SAPLING, // 0x19
    FG_FRUIT_TREE_LAST                 = FG_PALM_FRUIT, // 0x48
    FG_APPLE_TREE_FIRST                = FG_APPLE_TREE_SAPLING, // 0x19
    FG_APPLE_TREE_LAST                 = FG_APPLE_TREE_FRUIT, // 0x20
    FG_ORANGE_TREE_FIRST               = FG_ORANGE_TREE_SAPLING, // 0x21
    FG_ORANGE_TREE_LAST                = FG_ORANGE_TREE_FRUIT, // 0x28
    FG_PEAR_TREE_FIRST                 = FG_PEAR_TREE_SAPLING, // 0x29
    FG_PEAR_TREE_LAST                  = FG_PEAR_TREE_FRUIT, // 0x30
    FG_PEACH_TREE_FIRST                = FG_PEACH_TREE_SAPLING, // 0x31
    FG_PEACH_TREE_LAST                 = FG_PEACH_TREE_FRUIT, // 0x38
    FG_CHERRY_TREE_FIRST               = FG_CHERRY_TREE_SAPLING, // 0x39
    FG_CHERRY_TREE_LAST                = FG_CHERRY_TREE_FRUIT, // 0x40
    FG_PALM_FIRST                      = FG_PALM_SAPLING, // 0x41
    FG_PALM_LAST                       = FG_PALM_FRUIT, // 0x48
    FG_MONEY_TREE_FIRST                = FG_MONEY_TREE_SAPLING, // 0x49
    FG_MONEY_TREE_LAST                 = FG_MONEY_TREE, // 0x4D
    FG_CEDAR_FIRST                     = FG_CEDAR_SAPLING, // 0x4E
    FG_CEDAR_LAST                      = FG_CEDAR_LIGHTS, // 0x56
    FG_WEED_FIRST                      = FG_WEED_A, // 0x57
    FG_WEED_LAST                       = FG_WEED_D, // 0x5A
    FG_STONE_FIRST                     = FG_STONE_A, // 0x5B
    FG_STONE_LAST                      = FG_STONE_E, // 0x5F
    FG_MONEY_ROCK_FIRST                = FG_MONEY_ROCK_P0_A, // 0x60
    FG_MONEY_ROCK_LAST                 = FG_MONEY_ROCK_P3_E, // 0x73
    FG_DESIGN_FIRST                    = FG_DESIGN_P0_0, // 0x74
    FG_DESIGN_LAST                     = FG_DESIGN_P3_7, // 0x93
    FG_RED_TURNIP_FIRST                = FG_RED_TURNIP_WILTED, // 0x95
    FG_RED_TURNIP_LAST                 = FG_RED_TURNIP_WILTED_WATERED, // 0x9D
    FG_FLOWER_FIRST                    = FG_TULIP_RED, // 0x9E
    FG_FLOWER_LAST                     = FG_JACOBS_LADDER, // 0xBC
    FG_CB_FLOWER_LAST                  = FG_CARNATION_WHITE, // 0xBB (cross-breeding range end)
    FG_WILTED_FLOWER_FIRST             = FG_WILTED_TULIP_RED, // 0xBE
    FG_WILTED_FLOWER_LAST              = FG_WILTED_RAFFLESIA, // 0xDD
};

#define ITEM_NAME_TYPE(id) (((id) >> 12) & 0xF)
#define ITEM_NAME_INDEX(id) ((id) & 0xFFF) // index within ITEM_NAME_TYPE(id)

inline bool isRealItemId(u16 id) {
    int category = ITEM_NAME_TYPE(id);
    return category >= 0x9 && category <= 0xC;
}

// One field object (804E3C40 table, 0x20 bytes each).
struct FgInfo {
    u16 _00; // 0x00
    s16 mDropItem; // 0x02: item a grown tree drops (or ITEM_ID_NONE)
    u16 _04; // 0x04
    s16 mPlantItem; // 0x06: item that plants this object (or ITEM_ID_NONE)
    u8 _08[5]; // 0x08
    s8 mTreeStage; // 0x0D: growth stage, negative for non-trees
    u8 mGrassGrowth; // 0x0E: grass wear value the unit under the item regains each day (dFootmark)
    char mName[ITEM_NAME_LEN+1]; // 0x0F: model resource name
}; // sizeof = 0x20

struct Item {
    Item() : mId(ITEM_ID_NONE) {}
    Item(u16 id) : mId(id) {}
    Item(const Item &other) : mId(other.mId) {}
    Item &operator=(const Item &other) {
        mId = other.mId;
        return *this;
    }
    Item &operator=(u16 id) {
        mId = id;
        return *this;
    }
    bool operator==(u16 o) const { return getId() == o; }
    // friend bool operator==(u16 id, const Item& item) { return item.getId() == id; }
    // From an item index (not an id); leaves mId to setFromIndex.
    explicit Item(int index); // 800A5D3C
    Item(int base, int offset, BOOL skipCheck); // 800A5D6C
    ~Item() {}
    static void *operator new(size_t, void *p) { return p; } // in-place arrays (d_field_info)

    void setFromIndex(int index); // 800A5D9C
    void setFromIndex(int base, int offset, BOOL skipCheck); // 800A5E08

    // Same item, ignoring the 2 variant bits of encoded ids.
    BOOL isSame(const Item &other) const; // 800A5E64
    BOOL isNotSame(const Item &other) const { return !isSame(other); }
    BOOL isSame(u16 id) const { return isSame(Item(id)); }

    bool isValid() const { return mId != ITEM_ID_NONE; }
    u16 getId() const { return mId; }

    bool isFg() const {
        return mId < FG_COUNT;
    }
    int getFgIndex() const {
        return isFg() ? mId & 0xFFF : -1;
    }
    FgInfo *getFgInfo() const; // 800A5EF0

    BOOL isMoney() const; // 800A5F20
    BOOL isKabu(); // 800A5F48
    int getPrice() const; // 800A5F74
    // Price after the current shop discount.
    int getShopPrice(); // 800A60F8
    // Money item closest to `amount` (rounding up or down); the leftover
    // goes to `remainder`.
    static u16 getMoneyItem(int amount, BOOL roundUp, int *remainder); // 800A622C

    // Ids 0xD000..0xD044 index a table owned by the TU at 80167FB4.
    bool isExtId() const {
        return mId >= 0xD000 && mId < 0xD045;
    }
    int getExtIndex() const {
        return isExtId() ? mId & 0xFFF : -1;
    }
    f32 getExtValueA() const; // 800A6370
    f32 getExtValueB() const; // 800A6404
    BOOL getExtFlag() const; // 800A6498

    BOOL isOrgDesign() const; // 800A651C
    // BITM kind, or KIND_NONE for unencoded ids or missing entries.
    int getKind() const; // 800A658C
    int getFrom() const; // 800A6608
    int getFashion() const; // 800A6684
    int getStyle() const; // 800A6700
    int getClothStyle(); // 800A677C: tail-calls getStyle
    int getColorA() const; // 800A6780
    int getColorB() const; // 800A680C
    int getPartA() const; // 800A689C
    int getHideBone() const; // 800A6918
    int getSeries(); // 800A6998
    int getSeriesGroup(); // 800A69F0
    BOOL hasNoFtrFunc() const; // 800A6A34
    BOOL hasFtrFunc() const; // 800A6A8C
    BOOL isUsable() const; // 800A6B14

    BOOL isFlower() const; // 800A6B5C
    BOOL isWiltedFlower(); // 800A6B80
    BOOL isTree(); // 800A6BA4
    BOOL isFruitTree(); // 800A6BE4
    BOOL isFg74(); // 800A6C08
    BOOL isInsect(); // 800A6C2C
    BOOL isFish(); // 800A6C58
    BOOL isDust(); // 800A6C84
    BOOL isMushroom(); // 800A6CB0
    BOOL isShell(); // 800A6CDC
    BOOL isAnyFlower(); // 800A6D08
    BOOL isSnowman(); // 800A6D5C
    int getSnowmanIdx(); // 800A6D7C
    // Field object planted by this item, or the id itself if none.
    u16 getPlantedFg(); // 800A6D88
    int getFruitTreeType(); // 800A6E00
    // Item given when this object is picked up.
    Item getPickItem() const; // 800A6F20

    Item getVariant(int variant); // 800A7094: tail-calls withVariant
    // Copy with the variant bits replaced by `variant & 3`.
    Item withVariant(int variant) const; // 800A7098

    inline bool operator==(const Item& other) const {
        return mId == other.mId;
    }

    inline bool operator!=(const Item& other) const {
        return !(*this == other);
    }

    u16 mId;
};

// Never inlined (they return an Item); d_fg_item's copies are the linked ones.
inline Item Item::getVariant(int variant) {
    return withVariant(variant);
}

inline Item Item::withVariant(int variant) const {
    return Item(static_cast<u16>((variant & 3) | (mId & 0xFFFC)));
}


} // namespace dItem
