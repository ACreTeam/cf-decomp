#pragma once

// Block (acre) kinds and their flags. Each of the 0xB8 block types has a kind (fn_800812C8, the
// table at 804A76B8), and each kind has flags (fn_80081348, 0x4F u32 at 8046D460; fn_80081324 gives a
// type's flags, which dFdBlock_c::hasFlag tests; fn_800812E8 finds the kind with exactly some flags).
// The random field generator (d_random_field) builds the town in kinds and then picks a type per kind.
//
// Names are inferred from the flags, the block models (BgData/BgModel/<type, decimal>_0.brres) and the
// FgData layouts (the facilities' buildings). The river bits are named by the step the river takes
// through them in dRF's direction table (S = +z, E = +x, W = -x); the seven cliff shapes are only
// numbered.

enum {
    BLOCK_KIND_BORDER_N = 0x00, // the north border row; types 0x64
    BLOCK_KIND_BORDER_N_GATE = 0x01, // above the gate acre (m_Es_G_soil); types 0x65
    BLOCK_KIND_BORDER_N_RIVER = 0x02, // where the river enters, with a waterfall; types 0x66
    BLOCK_KIND_BORDER_W = 0x03, // the west border column; types 0x61
    BLOCK_KIND_BORDER_W_CLIFF = 0x04, // types 0x62
    BLOCK_KIND_BORDER_W_BEACH = 0x05, // the bottom of the west column; types 0x63
    BLOCK_KIND_BORDER_E = 0x06, // the east border column; types 0x69
    BLOCK_KIND_BORDER_E_CLIFF = 0x07, // types 0x6A
    BLOCK_KIND_BORDER_E_BEACH = 0x08, // the bottom of the east column; types 0x6B
    BLOCK_KIND_BORDER_NW = 0x09, // types 0x67
    BLOCK_KIND_BORDER_NE = 0x0A, // types 0x68
    BLOCK_KIND_PLAIN = 0x0B, // types 0x30..0x35; 0x33..0x35 have a pond
    BLOCK_KIND_GATE = 0x0C, // gate + bus stop (FgData layout); types 0x36, 0x37, 0x38
    BLOCK_KIND_UNUSED_0D = 0x0D, // flag 0x1, no block types
    BLOCK_KIND_MUSEUM = 0x0E, // museum; types 0x39, 0x3A, 0x3B
    BLOCK_KIND_TOWN_HALL = 0x0F, // town hall + bulletin board; types 0x3F, 0x40, 0x41
    BLOCK_KIND_SHOP = 0x10, // Nook's; types 0x3C, 0x3D, 0x3E
    BLOCK_KIND_TAILOR = 0x11, // Able Sisters; types 0x42, 0x43, 0x44
    BLOCK_KIND_RIVER_S0_E1 = 0x12, // types 0x95, 0x96
    BLOCK_KIND_RIVER_E0_S1 = 0x13, // types 0x7F
    BLOCK_KIND_RIVER_W0_S2 = 0x14, // types 0xB7
    BLOCK_KIND_RIVER_S0_W1 = 0x15, // types 0xA3, 0xA4
    BLOCK_KIND_RIVER_E1_W1 = 0x16, // types 0xA1, 0xA2
    BLOCK_KIND_RIVER_FALL = 0x17, // the river below the north waterfall; types 0x86, 0x87, 0x88
    BLOCK_KIND_RIVER_S0 = 0x18, // types 0x80, 0x81, 0x82
    BLOCK_KIND_RIVER_E0 = 0x19, // types 0x6D, 0x6E, 0x6F
    BLOCK_KIND_RIVER_W0 = 0x1A, // types 0xA5, 0xA6, 0xA7
    BLOCK_KIND_RIVER_E1 = 0x1B, // types 0x8B, 0x8C, 0x8D, 0x8E
    BLOCK_KIND_RIVER_S1 = 0x1C, // types 0x75, 0x76, 0x77, 0x78
    BLOCK_KIND_RIVER_W1 = 0x1D, // types 0x97, 0x98, 0x99, 0x9A
    BLOCK_KIND_RIVER_S2 = 0x1E, // types 0xAD, 0xAE, 0xAF, 0xB0
    BLOCK_KIND_BRIDGE_S0 = 0x1F, // types 0x83, 0x84, 0x85
    BLOCK_KIND_BRIDGE_E0 = 0x20, // types 0x70, 0x71, 0x72
    BLOCK_KIND_BRIDGE_W0 = 0x21, // types 0xA8, 0xA9, 0xAA
    BLOCK_KIND_BRIDGE_E1 = 0x22, // types 0x8F, 0x90, 0x91, 0x92
    BLOCK_KIND_BRIDGE_S1 = 0x23, // types 0x79, 0x7A, 0x7B, 0x7C
    BLOCK_KIND_BRIDGE_W1 = 0x24, // types 0x9B, 0x9C, 0x9D, 0x9E
    BLOCK_KIND_BRIDGE_S2 = 0x25, // types 0xB1, 0xB2, 0xB3, 0xB4
    BLOCK_KIND_RACCO_S0 = 0x26, // types 0x89, 0x8A
    BLOCK_KIND_RACCO_E0 = 0x27, // types 0x73, 0x74
    BLOCK_KIND_RACCO_W0 = 0x28, // types 0xAB, 0xAC
    BLOCK_KIND_RACCO_E1 = 0x29, // types 0x93, 0x94
    BLOCK_KIND_RACCO_S1 = 0x2A, // types 0x7D, 0x7E
    BLOCK_KIND_RACCO_W1 = 0x2B, // types 0x9F, 0xA0
    BLOCK_KIND_RACCO_S2 = 0x2C, // types 0xB5, 0xB6
    BLOCK_KIND_BEACH = 0x2D, // types 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A
    BLOCK_KIND_BEACH_RIVER_S0 = 0x2E, // types 0x4F, 0x50, 0x51
    BLOCK_KIND_BEACH_RIVER_E1 = 0x2F, // types 0x55, 0x56
    BLOCK_KIND_BEACH_RIVER_S1 = 0x30, // types 0x4B, 0x4C
    BLOCK_KIND_BEACH_RIVER_W1 = 0x31, // types 0x59, 0x5A
    BLOCK_KIND_BEACH_RIVER_S2 = 0x32, // types 0x5D, 0x5E
    BLOCK_KIND_BEACH_BRIDGE_S0 = 0x33, // types 0x52, 0x53, 0x54
    BLOCK_KIND_BEACH_BRIDGE_E1 = 0x34, // types 0x57, 0x58
    BLOCK_KIND_BEACH_BRIDGE_S1 = 0x35, // types 0x4D, 0x4E
    BLOCK_KIND_BEACH_BRIDGE_W1 = 0x36, // types 0x5B, 0x5C
    BLOCK_KIND_BEACH_BRIDGE_S2 = 0x37, // types 0x5F, 0x60
    BLOCK_KIND_SEA = 0x38, // the south row (type 0x6C, no model); types 0x6C
    BLOCK_KIND_NONE = 0x39, // no kind (RF_KIND_NONE); types none
    BLOCK_KIND_CLIFF_0 = 0x3A, // types 0x0D, 0x0E, 0x0F, 0x10
    BLOCK_KIND_CLIFF_1 = 0x3B, // types 0x06, 0x07, 0x08, 0x09
    BLOCK_KIND_CLIFF_2 = 0x3C, // types 0x00, 0x01, 0x02
    BLOCK_KIND_CLIFF_3 = 0x3D, // types 0x15, 0x16, 0x17, 0x18
    BLOCK_KIND_CLIFF_4 = 0x3E, // types 0x1C, 0x1D, 0x1E, 0x1F
    BLOCK_KIND_CLIFF_5 = 0x3F, // types 0x23, 0x24, 0x25
    BLOCK_KIND_CLIFF_6 = 0x40, // types 0x28, 0x29, 0x2A, 0x2B
    BLOCK_KIND_RAMP_0 = 0x41, // types 0x13, 0x14
    BLOCK_KIND_RAMP_1 = 0x42, // types 0x0B, 0x0C
    BLOCK_KIND_RAMP_2 = 0x43, // types 0x04, 0x05
    BLOCK_KIND_RAMP_3 = 0x44, // types 0x1A, 0x1B
    BLOCK_KIND_RAMP_4 = 0x45, // types 0x21, 0x22
    BLOCK_KIND_RAMP_5 = 0x46, // types 0x26, 0x27
    BLOCK_KIND_RAMP_6 = 0x47, // types 0x2D, 0x2E
    BLOCK_KIND_FALL_C0_S0 = 0x48, // types 0x11, 0x12
    BLOCK_KIND_FALL_C1_S0 = 0x49, // types 0x0A
    BLOCK_KIND_CLIFF3_RIVER_S0 = 0x4A, // types 0x19
    BLOCK_KIND_FALL_C4_S0 = 0x4B, // types 0x20
    BLOCK_KIND_CLIFF6_RIVER_S0 = 0x4C, // types 0x2C
    BLOCK_KIND_FALL_C2_E0 = 0x4D, // types 0x03
    BLOCK_KIND_FALL_C5_W0 = 0x4E, // types 0x2F

    BLOCK_KIND_NUM = 0x4F,
};

// Kind flags (fn_80081348).
enum {
    BLOCK_KIND_FLAG_UNUSED_0D = 0x1,      // only BLOCK_KIND_UNUSED_0D
    BLOCK_KIND_FLAG_SHOP = 0x2,
    BLOCK_KIND_FLAG_TAILOR = 0x4,
    BLOCK_KIND_FLAG_BRIDGE = 0x8,         // a river acre with a bridge; each river shape has a bridge kind
    BLOCK_KIND_FLAG_BEACH = 0x10,
    BLOCK_KIND_FLAG_SEA = 0x20,
    BLOCK_KIND_FLAG_BORDER_N = 0x40,      // the border acres around the town
    BLOCK_KIND_FLAG_BORDER_W = 0x80,
    BLOCK_KIND_FLAG_BORDER_E = 0x100,
    BLOCK_KIND_FLAG_RACCO = 0x200,        // one river acre per town (dRF); inferred: Pascal's spot (BG_ATTR_BR_RACCO_*)
    BLOCK_KIND_FLAG_TOWN_HALL = 0x400,
    BLOCK_KIND_FLAG_GATE = 0x800,         // gate + bus stop; the fountain goes at its unit (7, 8)
    BLOCK_KIND_FLAG_MUSEUM = 0x1000,
    BLOCK_KIND_FLAG_RIVER_S0 = 0x2000,    // river shapes, by their step: (0, +1)
    BLOCK_KIND_FLAG_RIVER_E0 = 0x4000,    // (+1, 0)
    BLOCK_KIND_FLAG_RIVER_W0 = 0x8000,    // (-1, 0)
    BLOCK_KIND_FLAG_RIVER_E1 = 0x10000,   // (+1, 0)
    BLOCK_KIND_FLAG_RIVER_S1 = 0x20000,   // (0, +1)
    BLOCK_KIND_FLAG_RIVER_W1 = 0x40000,   // (-1, 0)
    BLOCK_KIND_FLAG_RIVER_S2 = 0x80000,   // (0, +1)
    BLOCK_KIND_FLAG_FALL = 0x100000,      // a waterfall (the river drops here)
    BLOCK_KIND_FLAG_CLIFF_0 = 0x200000,   // cliff shapes
    BLOCK_KIND_FLAG_CLIFF_1 = 0x400000,
    BLOCK_KIND_FLAG_CLIFF_2 = 0x800000,
    BLOCK_KIND_FLAG_CLIFF_3 = 0x1000000,
    BLOCK_KIND_FLAG_CLIFF_4 = 0x2000000,
    BLOCK_KIND_FLAG_CLIFF_5 = 0x4000000,
    BLOCK_KIND_FLAG_CLIFF_6 = 0x8000000,
    BLOCK_KIND_FLAG_RAMP = 0x10000000,    // a ramp up the cliff (m_cliff_slope)

    BLOCK_KIND_FLAG_RIVER = 0x000FE000,   // any river shape
    BLOCK_KIND_FLAG_CLIFF = 0x0FE00000,   // any cliff shape
};
