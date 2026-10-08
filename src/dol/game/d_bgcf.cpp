// The bg check (namespace dBGCF, d_bgcf.hpp). .text 8006C620..800767A8, .ctors 80465640,
// .rodata 8046CE20..8046D2B0, .data 804A5588..804A6CE0, .bss 80575EB0..80582E60,
// .sdata 80749C60..80749CC0, .sbss 8074E1E8..8074E270, .sdata2 80750418..807504B0.
// See notes/d_bgcf.txt. Function, member and most class names are inferred.
#include <game/game/d_bgcf.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_scene.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/math/eggMath.h>
#include <revolution/MTX.h>
#include <cstring>

namespace dBGCF {

const f32 cWaterY0 = 14.0f;
const f32 cWaterY1 = 14.0f;
const f32 cWaterY2 = 70.0f;

// One entry of the BG attribute table (see notes/bg_attributes.txt and d_bg_attr.hpp).
struct attr_s {
    s8 getWater() const { return (m27 >> 6) & 3; }
    s8 getPlant() const { return (m27 >> 4) & 3; }
    s8 getDig() const { return (m27 >> 2) & 3; }
    int canNpcPut() const { return (m27 >> 1) & 1; }
    int canPut() const { return m27 & 1; }
    int canPutNoBridge() const { return (m28 >> 7) & 1; }
    int isGrass() const { return (m28 >> 6) & 1; }
    int isBeach() const { return (m28 >> 5) & 1; }
    int isRiver() const { return (m28 >> 4) & 1; }
    int isSea() const { return (m28 >> 3) & 1; }

    /* 0x00 */ s8 mNext;         // NEXTn: n, else -1
    /* 0x01 */ u8 mGrassMin;     // limits of the unit's grass wear value
    /* 0x02 */ u8 mGrassMax;
    /* 0x03 */ s8 mTriAttr0[4];  // attribute of each triangle, LAYER_TOP
    /* 0x07 */ s8 mTriAttr1[4];  // attribute of each triangle, LAYER_WATER
    /* 0x0B */ u8 mMapColor[9];  // town map pixels, 3 x 3 (z rows, then x)
    /* 0x14 */ char mName[0x12]; // unused by the code
    /* 0x26 */ u8 mDir;          // low nibble: 1 + the slope direction (eighths of a turn)
    /* 0x27 */ u8 m27;           // bits 6..7: bgWater_e, 4..5: bgPlant_e, 2..3: bgDig_e, 1: npc put, 0: put
    /* 0x28 */ u8 m28;           // bit 7: put but not on a bridge, 6: grass, 5: beach, 4: river, 3: sea
}; // size 0x29

// The attribute data as converted from its file: a 0x20-byte header (record count and size), the
// records, and padding to a multiple of 32 bytes.
struct attrData_s {
    /* 0x0000 */ int mNum;
    /* 0x0004 */ int mSize;
    /* 0x0008 */ u8 _08[0x18];
    /* 0x0020 */ attr_s mAttrs[BG_ATTR_NUM];
    /* 0x1596 */ u8 _1596[0xA];
}; // size 0x15A0

} // namespace dBGCF

// 804A5588 (records at 804A55A8)
dBGCF::attrData_s lbl_804A5588 = {BG_ATTR_NUM, sizeof(dBGCF::attr_s), {0}, {
    {-1, 0xFF, 0xFF, {0, 0, 0, 0}, {0, 0, 0, 0}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "NONE", 0x00, 0x08, 0x00},
    {-1, 0xFF, 0xFF, {1, 1, 1, 1}, {1, 1, 1, 1}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "ATTRW", 0x00, 0x08, 0x00},
    {-1, 0xFF, 0xFF, {2, 2, 2, 2}, {2, 2, 2, 2}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "OBJ", 0x00, 0x08, 0x00},
    {-1, 0xFF, 0xFF, {3, 3, 3, 3}, {3, 3, 3, 3}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "STR_COL", 0x00, 0x08, 0x00},
    {-1, 0xFF, 0xFF, {4, 4, 4, 4}, {4, 4, 4, 4}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "STR_ENT", 0x00, 0x08, 0x00},
    {-1, 0xFF, 0xFF, {5, 5, 5, 5}, {5, 5, 5, 5}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "STR_INS", 0x00, 0x09, 0x80},
    {-1, 0xFF, 0xFF, {6, 6, 6, 6}, {6, 6, 6, 6}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "STR_DEL", 0x00, 0x09, 0x80},
    {-1, 0xFF, 0xFF, {4, 4, 4, 4}, {4, 4, 4, 4}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "STR_ENT_TO_RU", 0x00, 0x08, 0x00},
    {-1, 0xFF, 0xFF, {4, 4, 4, 4}, {4, 4, 4, 4}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "STR_ENT_TO_LU", 0x00, 0x08, 0x00},
    {0, 0xFF, 0xFF, {20, 20, 20, 20}, {20, 20, 20, 20}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "NEXT0", 0x00, 0x0A, 0x00},
    {1, 0xFF, 0xFF, {20, 20, 20, 20}, {20, 20, 20, 20}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "NEXT1", 0x00, 0x0A, 0x00},
    {2, 0xFF, 0xFF, {20, 20, 20, 20}, {20, 20, 20, 20}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "NEXT2", 0x00, 0x0A, 0x00},
    {3, 0xFF, 0xFF, {20, 20, 20, 20}, {20, 20, 20, 20}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "NEXT3", 0x00, 0x0A, 0x00},
    {4, 0xFF, 0xFF, {20, 20, 20, 20}, {20, 20, 20, 20}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "NEXT4", 0x00, 0x0A, 0x00},
    {5, 0xFF, 0xFF, {20, 20, 20, 20}, {20, 20, 20, 20}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "NEXT5", 0x00, 0x0A, 0x00},
    {6, 0xFF, 0xFF, {20, 20, 20, 20}, {20, 20, 20, 20}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "NEXT6", 0x00, 0x0A, 0x00},
    {-1, 0xFF, 0xFF, {20, 20, 20, 20}, {20, 20, 20, 20}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "WALL", 0x00, 0x08, 0x00},
    {-1, 0x00, 0x00, {17, 17, 17, 17}, {17, 17, 17, 17}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "CHANGE", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x00, {18, 18, 18, 18}, {18, 18, 18, 18}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "WATER", 0x00, 0xCC, 0x00},
    {-1, 0x00, 0xFF, {19, 19, 19, 19}, {19, 19, 19, 19}, {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}, "STONE", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x00, {20, 20, 20, 20}, {20, 20, 20, 20}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "FLOOR", 0x00, 0x0A, 0x00},
    {-1, 0x00, 0x00, {21, 21, 21, 21}, {21, 21, 21, 21}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "WOOD", 0x00, 0x0A, 0x00},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, "SOIL", 0x00, 0x23, 0xC0},
    {-1, 0x00, 0xFF, {23, 23, 23, 23}, {23, 23, 23, 23}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, "SAND", 0x00, 0x17, 0x80},
    {-1, 0x00, 0x80, {24, 24, 24, 24}, {24, 24, 24, 24}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "SEA", 0x01, 0x8C, 0x08},
    {-1, 0x00, 0x00, {25, 25, 25, 25}, {25, 25, 25, 25}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BRIDGE", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x00, {26, 26, 26, 26}, {26, 26, 26, 26}, {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, "WAVE", 0x00, 0x88, 0x00},
    {-1, 0x00, 0x80, {27, 27, 27, 27}, {27, 27, 27, 27}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "RIVER_N", 0x05, 0x4C, 0x10},
    {-1, 0x00, 0x80, {28, 28, 28, 28}, {28, 28, 28, 28}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "RIVER_NW", 0x06, 0x4C, 0x10},
    {-1, 0x00, 0x80, {29, 29, 29, 29}, {29, 29, 29, 29}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "RIVER_W", 0x07, 0x4C, 0x10},
    {-1, 0x00, 0x80, {30, 30, 30, 30}, {30, 30, 30, 30}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "RIVER_SW", 0x08, 0x4C, 0x10},
    {-1, 0x00, 0x80, {31, 31, 31, 31}, {31, 31, 31, 31}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "RIVER_S", 0x01, 0x4C, 0x10},
    {-1, 0x00, 0x80, {32, 32, 32, 32}, {32, 32, 32, 32}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "RIVER_SE", 0x02, 0x4C, 0x10},
    {-1, 0x00, 0x80, {33, 33, 33, 33}, {33, 33, 33, 33}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "RIVER_E", 0x03, 0x4C, 0x10},
    {-1, 0x00, 0x80, {34, 34, 34, 34}, {34, 34, 34, 34}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "RIVER_NE", 0x04, 0x4C, 0x10},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, "SOILX", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0x80, {26, 26, 26, 26}, {26, 26, 26, 26}, {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, "WAVE_S", 0x50, 0x17, 0xA0},
    {-1, 0x00, 0xC0, {23, 23, 26, 26}, {23, 23, 26, 26}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x04, 0x06, 0x04, 0x04}, "WAVE_SE2", 0x00, 0x17, 0xA0},
    {-1, 0x00, 0xC0, {23, 26, 26, 23}, {23, 26, 26, 23}, {0x06, 0x06, 0x06, 0x04, 0x06, 0x06, 0x04, 0x04, 0x06}, "WAVE_SW2", 0x00, 0x17, 0xA0},
    {-1, 0x00, 0x80, {26, 26, 24, 24}, {26, 26, 24, 24}, {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00}, "WAVE_SE", 0x60, 0x0C, 0x00},
    {-1, 0x00, 0x80, {26, 24, 24, 26}, {26, 24, 24, 26}, {0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00, 0x00, 0x04}, "WAVE_SW", 0x40, 0x0C, 0x00},
    {-1, 0x00, 0xC0, {22, 22, 24, 24}, {22, 22, 24, 24}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x06, 0x00, 0x00}, "SOIL_NW_SEA_SE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xC0, {22, 24, 24, 22}, {22, 24, 24, 22}, {0x06, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00, 0x00, 0x06}, "SOIL_NE_SEA_SW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xC0, {28, 28, 22, 22}, {28, 28, 22, 22}, {0x00, 0x00, 0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06}, "WATER_NW_SOILX_SE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xC0, {22, 30, 30, 22}, {22, 30, 30, 22}, {0x06, 0x06, 0x06, 0x00, 0x06, 0x06, 0x06, 0x00, 0x06}, "WATER_SW_SOILX_NE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xC0, {22, 22, 32, 32}, {22, 22, 32, 32}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x06, 0x00, 0x00}, "WATER_SE_SOILX_NW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xC0, {34, 22, 22, 34}, {34, 22, 22, 34}, {0x06, 0x00, 0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x06}, "WATER_NE_SOILX_SW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {27, 27, 27, 27}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RIVER_N", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {29, 29, 29, 29}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RIVER_W", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {31, 31, 31, 31}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RIVER_S", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {33, 33, 33, 33}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RIVER_E", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {28, 28, 28, 28}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RIVER_NW", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {30, 30, 30, 30}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RIVER_SW", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {32, 32, 32, 32}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RIVER_SE", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {34, 34, 34, 34}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RIVER_NE", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0xC0, {25, 25, 22, 22}, {28, 28, 22, 22}, {0x03, 0x03, 0x06, 0x03, 0x06, 0x06, 0x06, 0x06, 0x06}, "BR_NW_SO_SE", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0xC0, {22, 25, 25, 22}, {22, 30, 30, 22}, {0x06, 0x06, 0x06, 0x03, 0x06, 0x06, 0x03, 0x03, 0x06}, "BR_SW_SO_NE", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0xC0, {22, 22, 25, 25}, {22, 22, 32, 32}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x03, 0x06, 0x03, 0x03}, "BR_SE_SO_NW", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0xC0, {25, 22, 22, 25}, {34, 22, 22, 34}, {0x06, 0x03, 0x03, 0x06, 0x06, 0x03, 0x06, 0x06, 0x06}, "BR_NE_SO_SW", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 28, 28}, {28, 28, 28, 28}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x00, 0x00}, "BR_NW_RIVER_NW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {30, 25, 25, 30}, {30, 30, 30, 30}, {0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03}, "BR_SW_RIVER_SW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {32, 32, 25, 25}, {32, 32, 32, 32}, {0x00, 0x00, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_SE_RIVER_SE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 34, 34, 25}, {34, 34, 34, 34}, {0x03, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03}, "BR_NE_RIVER_NE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 25, 32, 32}, {32, 32, 32, 32}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x00, 0x00}, "BR_NW_RIVER_SE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {34, 25, 25, 34}, {34, 34, 34, 34}, {0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03}, "BR_SW_RIVER_NE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {28, 28, 25, 25}, {28, 28, 28, 28}, {0x00, 0x00, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_SE_RIVER_NW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 30, 30, 25}, {30, 30, 30, 30}, {0x03, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03}, "BR_NE_RIVER_SW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 25, 27, 27}, {27, 27, 27, 27}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x00, 0x00}, "BR_NW_RIVER_N", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {31, 25, 25, 31}, {31, 31, 31, 31}, {0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03}, "BR_SW_RIVER_S", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {31, 31, 25, 25}, {31, 31, 31, 31}, {0x00, 0x00, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_SE_RIVER_S", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 27, 27, 25}, {27, 27, 27, 27}, {0x03, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03}, "BR_NE_RIVER_N", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 25, 31, 31}, {31, 31, 31, 31}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x00, 0x00}, "BR_NW_RIVER_S", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {27, 25, 25, 27}, {27, 27, 27, 27}, {0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03}, "BR_SW_RIVER_N", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {27, 27, 25, 25}, {27, 27, 27, 27}, {0x00, 0x00, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_SE_RIVER_N", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 31, 31, 25}, {31, 31, 31, 31}, {0x03, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03}, "BR_NE_RIVER_S", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 25, 29, 29}, {29, 29, 29, 29}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x00, 0x00}, "BR_NW_RIVER_W", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {29, 25, 25, 29}, {29, 29, 29, 29}, {0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03}, "BR_SW_RIVER_W", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {33, 33, 25, 25}, {33, 33, 33, 33}, {0x00, 0x00, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_SE_RIVER_E", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 33, 33, 25}, {33, 33, 33, 33}, {0x03, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03}, "BR_NE_RIVER_E", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 25, 33, 33}, {33, 33, 33, 33}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x00, 0x00}, "BR_NW_RIVER_E", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {33, 25, 25, 33}, {33, 33, 33, 33}, {0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03}, "BR_SW_RIVER_E", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {29, 29, 25, 25}, {29, 29, 29, 29}, {0x00, 0x00, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_SE_RIVER_W", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x80, {25, 29, 29, 25}, {29, 29, 29, 29}, {0x03, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03}, "BR_NE_RIVER_W", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xC0, {19, 19, 22, 22}, {19, 19, 22, 22}, {0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x01, 0x06, 0x06}, "STONE_NW", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0xC0, {22, 19, 19, 22}, {22, 19, 19, 22}, {0x01, 0x06, 0x06, 0x01, 0x01, 0x06, 0x01, 0x01, 0x01}, "STONE_SW", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0xC0, {22, 22, 19, 19}, {22, 22, 19, 19}, {0x06, 0x06, 0x01, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01}, "STONE_SE", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0xC0, {19, 22, 22, 19}, {19, 22, 22, 19}, {0x01, 0x01, 0x01, 0x06, 0x01, 0x01, 0x06, 0x06, 0x01}, "STONE_NE", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, "CLIFF_N", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x05, 0x06, 0x06, 0x05, 0x06, 0x06, 0x05, 0x06, 0x06}, "CLIFF_W", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05}, "CLIFF_S", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x06, 0x06, 0x05, 0x06, 0x06, 0x05, 0x06, 0x06, 0x05}, "CLIFF_E", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x06, 0x06, 0x05, 0x06, 0x05, 0x06, 0x05, 0x06, 0x06}, "CLIFF_NW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x05, 0x06, 0x06, 0x06, 0x05, 0x06, 0x06, 0x06, 0x05}, "CLIFF_NE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x05, 0x06, 0x06, 0x06, 0x05, 0x06, 0x06, 0x06, 0x05}, "CLIFF_SW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x06, 0x06, 0x05, 0x06, 0x05, 0x06, 0x05, 0x06, 0x06}, "CLIFF_SE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x05, 0x06, 0x06}, "CLIFF_NW2", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x05, 0x05, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x05}, "CLIFF_NE2", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x05, 0x06, 0x06, 0x05, 0x06, 0x06, 0x05, 0x05, 0x05}, "CLIFF_SW2", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x06, 0x06, 0x05, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05}, "CLIFF_SE2", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0xFF, {22, 22, 23, 23}, {22, 22, 23, 23}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, "SAND_SE", 0x00, 0x17, 0x80},
    {-1, 0x00, 0xFF, {22, 23, 23, 22}, {22, 23, 23, 22}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, "SAND_SW", 0x00, 0x17, 0x80},
    {-1, 0x00, 0xFF, {19, 19, 19, 19}, {19, 19, 19, 19}, {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}, "STONE_BUS", 0x00, 0x09, 0x80},
    {-1, 0x00, 0xFF, {22, 22, 22, 22}, {22, 22, 22, 22}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, "LIGHTHOUSE", 0x00, 0x13, 0xC0},
    {-1, 0x00, 0x00, {21, 21, 21, 21}, {21, 21, 21, 21}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "TAILOR", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x00, {104, 104, 104, 104}, {104, 104, 104, 104}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "IRON", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x00, {105, 105, 105, 105}, {105, 105, 105, 105}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "MUSHIRO", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x00, {106, 106, 106, 106}, {106, 106, 106, 106}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "INDOOR_SAND", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x00, {107, 107, 107, 107}, {107, 107, 107, 107}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "CARPET", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {29, 29, 29, 29}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RACCO_W", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {31, 31, 31, 31}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RACCO_S", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {33, 33, 33, 33}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RACCO_E", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {30, 30, 30, 30}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RACCO_SW", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x80, {25, 25, 25, 25}, {32, 32, 32, 32}, {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, "BR_RACCO_SE", 0x00, 0x0B, 0x00},
    {-1, 0x00, 0x00, {113, 113, 113, 113}, {113, 113, 113, 113}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "COBBLE", 0x00, 0x0B, 0x80},
    {0, 0x00, 0x80, {19, 19, 19, 19}, {19, 19, 19, 19}, {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}, "GOTO_RESET", 0x00, 0x09, 0x80},
    {-1, 0x00, 0x00, {24, 24, 24, 24}, {24, 24, 24, 24}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "UFO", 0x01, 0x8C, 0x08},
    {-1, 0x00, 0x00, {116, 116, 116, 116}, {116, 116, 116, 116}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "INDOOR_SOIL", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x00, {117, 117, 117, 117}, {117, 117, 117, 117}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "INDOOR_GRASS", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x80, {31, 31, 31, 31}, {31, 31, 31, 31}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "FALL_S", 0x01, 0x4C, 0x10},
    {-1, 0x00, 0x80, {30, 30, 30, 30}, {30, 30, 30, 30}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "FALL_SW", 0x08, 0x4C, 0x10},
    {-1, 0x00, 0x80, {32, 32, 32, 32}, {32, 32, 32, 32}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "FALL_SE", 0x02, 0x4C, 0x10},
    {-1, 0x00, 0xC0, {24, 22, 22, 24}, {24, 22, 22, 24}, {0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x06, 0x00, 0x00}, "SOIL_SW_SEA_NE", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0xC0, {24, 24, 22, 22}, {24, 24, 22, 22}, {0x06, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00, 0x00, 0x06}, "SOIL_SE_SEA_NW", 0x00, 0x0C, 0x00},
    {-1, 0x00, 0x00, {123, 123, 123, 123}, {123, 123, 123, 123}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "SEA_SW", 0x08, 0x8C, 0x08},
    {-1, 0x00, 0x00, {124, 124, 124, 124}, {124, 124, 124, 124}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "SEA_SE", 0x02, 0x8C, 0x08},
    {-1, 0x00, 0x80, {19, 19, 19, 19}, {19, 19, 19, 19}, {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}, "TOWN", 0x00, 0x0B, 0x80},
    {-1, 0x00, 0x80, {19, 19, 19, 19}, {19, 19, 19, 19}, {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}, "TOWN_NO_NPC", 0x00, 0x09, 0x80},
    {0, 0xFF, 0xFF, {107, 107, 107, 107}, {107, 107, 107, 107}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "CARPET_NEXT0", 0x00, 0x0A, 0x00},
    {1, 0xFF, 0xFF, {107, 107, 107, 107}, {107, 107, 107, 107}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "CARPET_NEXT1", 0x00, 0x0A, 0x00},
    {2, 0xFF, 0xFF, {107, 107, 107, 107}, {107, 107, 107, 107}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "CARPET_NEXT2", 0x00, 0x0A, 0x00},
    {3, 0xFF, 0xFF, {107, 107, 107, 107}, {107, 107, 107, 107}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "CARPET_NEXT3", 0x00, 0x0A, 0x00},
    {4, 0xFF, 0xFF, {107, 107, 107, 107}, {107, 107, 107, 107}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "CARPET_NEXT4", 0x00, 0x0A, 0x00},
    {5, 0xFF, 0xFF, {107, 107, 107, 107}, {107, 107, 107, 107}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "CARPET_NEXT5", 0x00, 0x0A, 0x00},
    {6, 0xFF, 0xFF, {107, 107, 107, 107}, {107, 107, 107, 107}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "CARPET_NEXT6", 0x00, 0x0A, 0x00},
}};

namespace dBGCF {

static attr_s *sAttrTable = lbl_804A5588.mAttrs;

// 8046CE20: per attribute, the footstep sound and the furniture move sound.
static const int sAttrSe[BG_ATTR_NUM][2] = {
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {6454, 6277}, {6466, 6281}, {6506, 6280}, {6446, 6276}, {6482, -1},
    {-1, -1}, {6454, -1}, {6474, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {6494, 6283}, {6502, 6279}, {6450, 6282}, {6498, 6278},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {6454, 6277},
    {6454, -1}, {-1, -1}, {6514, 6275}, {6510, 6276}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {6454, -1},
    {6454, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1},
    {-1, -1}, {-1, -1},
};

static inline const attr_s *getAttrData(int attr) {
    return &sAttrTable[attr];
}

// One block of a bg slot: its unit data and base height.
class blockInfo_c {
public:
    blockInfo_c() { clear(); }

    void clear();                     // 8006CE3C
    void set(unitDat_c *data, u32 y); // 8006CE50

    /* 0x0 */ unitDat_c *mData;
    /* 0x4 */ f32 mY;
}; // size 0x8

// 16 x 16 unit bits.
class unitBits_c {
public:
    unitBits_c() { memset(mBits, 0, sizeof(mBits)); }

    /* 0x00 */ u16 mBits[16];
}; // size 0x20

// One bit per unit of a 7 x 7 block area.
class unitFlags_c {
public:
    void clear();             // 8006C620
    BOOL check(int x, int z); // 8006C69C
    void on(int x, int z);    // 8006C734
    void off(int x, int z);   // 8006C7CC

    /* 0x000 */ unitBits_c mBlocks[7][7];
}; // size 0x620

// A bg slot: up to 7 x 7 blocks.
class bg_c {
public:
    bg_c() {
        mUnitFlags.clear();
        init();
    }
    ~bg_c() {}

    BOOL checkFlag(int bit);             // 8006CE84
    void clearFlags();                   // 8006CE94
    void setFlags();                     // 8006CEA0
    void init();                         // 8006CEAC
    blockInfo_c *getBlock(u32 x, u32 z); // 8006CF40
    BOOL isLoaded();                     // 8006CF74
    BOOL onFlag(int bit);                // 8006CFDC
    BOOL offFlag(int bit);               // 8006D008

    /* 0x000 */ blockInfo_c mBlocks[7][7];
    /* 0x188 */ clmcb_c *mCallback;
    /* 0x18C */ u32 mBlockW;
    /* 0x190 */ u32 mBlockH;
    /* 0x194 */ u32 mFlags; // NEXTn exits that are locked
    /* 0x198 */ u8 mLoaded;
    /* 0x19A */ unitFlags_c mUnitFlags;
    /* 0x7BC */ f32 mPrevY;
    /* 0x7C0 */ f32 mY;
}; // size 0x7C4

// Short height animations of units.
class unitAnm_c {
public:
    struct rec_s {
        /* 0x00 */ u16 mTimer;
        /* 0x04 */ int mX;
        /* 0x08 */ int mZ;
        /* 0x0C */ f32 mFrom;
        /* 0x10 */ f32 mTo;
        /* 0x14 */ u32 mTime;
    }; // size 0x18

    unitAnm_c() { clear(); }

    void clear();                                       // 8006D034
    BOOL set(int x, int z, u32 time, f32 to, f32 from); // 8006D040
    int find(int x, int z);                             // 8006D0FC
    void update();                                      // 8006D2C0
    f32 getY(int x, int z);                             // 8006D340

    /* 0x000 */ u32 mFlags;
    /* 0x004 */ rec_s mRecs[32];
}; // size 0x304

// One unit of the check grid: its 4 triangles and its data.
class cell_c {
public:
    cell_c() {
        mDat = NULL;
        mType = 0;
    }
    cell_c(const mVec3_c *pos, int type);
    ~cell_c() {}

    void make(const int *xz, int mode, int type);
    void makeTriangles(const int *xz, int blockX, int blockZ, int mode, int type);

    u8 getAttr() const { return mDat != NULL ? mDat->mAttr : 0; }
    void setTriY(int idx, f32 y) {
        for (mVec3_c *p = mTri[idx]; p < mTri[idx] + 3; p++) {
            p->y = y;
        }
    }
    void setTriWaterY(int idx, f32 y) {
        for (mVec3_c *p = mTri[idx]; p < mTri[idx] + 3; p++) {
            p->y = y + mDat->getTri(idx & 3).getWaterY();
        }
    }

    /* 0x00 */ mVec3_c mTri[4][3];
    /* 0x90 */ unitDat_c *mDat;
    /* 0x94 */ u8 mType;
}; // size 0x98

// The 7 x 7 units around the mover.
class grid_c {
public:
    grid_c() { mMinX = mMinZ = mMaxX = mMaxZ = 0; }

    void make(const int *min, const int *max, int mode, int type); // 8006D584
    cell_c *getCell(const int *xz);                                // 8006DDBC

    /* 0x0000 */ cell_c mCells[7][7];
    /* 0x1D18 */ int mMinX;
    /* 0x1D1C */ int mMinZ;
    /* 0x1D20 */ int mMaxX;
    /* 0x1D24 */ int mMaxZ;
}; // size 0x1D28

// All the bg check state.
class manager_c {
public:
    manager_c();              // 8006DE18
    void initDefault();       // 8006E11C
    BOOL setCurrent(u32 idx); // 8006E170

    /* 0x0000 */ u32 mCurrent;
    /* 0x0004 */ bg_c mBg[16];
    /* 0x7C44 */ wallList_c mWalls;
    /* 0x9148 */ floorList_c mFloors;
    /* 0xA7CC */ columnList_c mColumns;
    /* 0xAB90 */ mvbgList_c mMvbgs;
    /* 0xAB94 */ grid_c mGrid;
    /* 0xC8BC */ unitAnm_c mUnitAnm;
    /* 0xCBC0 */ unitDat_c mDefault; // for units without data
}; // size 0xCBCC

static mVec3_c l_up(0.0f, 1.0f, 0.0f);    // 80575EBC
static mVec3_c l_box(8.0f, 8.0f, 8.0f);   // 80575ED4: margin around a check
static mVec3_c l_front(0.0f, 0.0f, 1.0f); // 80575EEC
static mVec3_c l_zero(0.0f, 0.0f, 0.0f);  // 80575F04
static manager_c l_mgr;                   // 80575F1C

static bg_c *sCurBg; // 8074E1E8

// 8006C620
void unitFlags_c::clear() {
    for (u32 z = 0; z < 7; z++) {
        for (u32 x = 0; x < 7; x++) {
            memset(mBlocks[z][x].mBits, 0, sizeof(mBlocks[z][x].mBits));
        }
    }
}

// 8006C69C
BOOL unitFlags_c::check(int x, int z) {
    int blockX, blockZ;
    unitToBlock(&blockX, &blockZ, x, z);
    if ((u32)blockX < 7 && (u32)blockZ < 7) {
        return (mBlocks[blockZ][blockX].mBits[z & 15] >> (x & 15)) & 1;
    }
    return FALSE;
}

// 8006C734
void unitFlags_c::on(int x, int z) {
    int blockX, blockZ;
    unitToBlock(&blockX, &blockZ, x, z);
    if ((u32)blockX < 7 && (u32)blockZ < 7) {
        mBlocks[blockZ][blockX].mBits[z & 15] |= 1 << (x & 15);
    }
}

// 8006C7CC
void unitFlags_c::off(int x, int z) {
    int blockX, blockZ;
    unitToBlock(&blockX, &blockZ, x, z);
    if ((u32)blockX < 7 && (u32)blockZ < 7) {
        mBlocks[blockZ][blockX].mBits[z & 15] &= ~(1 << (x & 15));
    }
}

// 8006C864
void unitDat_c::calcTri0(mVec3_c *out, unitDat_c *dat, const mVec3_c &base) const {
    f32 y = 7.0f * dat->getHeight();
    out[0].set(16.0f, y + dat->getY0(0), 16.0f);
    out[0] += base;
    out[1].set(32.0f, y + dat->getY1(0), 0.0f);
    out[1] += base;
    out[2].set(0.0f, y + dat->getY2(0), 0.0f);
    out[2] += base;
}

// 8006C95C
void unitDat_c::calcTri1(mVec3_c *out, unitDat_c *dat, const mVec3_c &base) const {
    f32 y = 7.0f * dat->getHeight();
    out[0].set(16.0f, y + dat->getY0(1), 16.0f);
    out[0] += base;
    out[1].set(0.0f, y + dat->getY1(1), 0.0f);
    out[1] += base;
    out[2].set(0.0f, y + dat->getY2(1), 32.0f);
    out[2] += base;
}

// 8006CA54
void unitDat_c::calcTri2(mVec3_c *out, unitDat_c *dat, const mVec3_c &base) const {
    f32 y = 7.0f * dat->getHeight();
    out[0].set(16.0f, y + dat->getY0(2), 16.0f);
    out[0] += base;
    out[1].set(0.0f, y + dat->getY1(2), 32.0f);
    out[1] += base;
    out[2].set(32.0f, y + dat->getY2(2), 32.0f);
    out[2] += base;
}

// 8006CB4C
void unitDat_c::calcTri3(mVec3_c *out, unitDat_c *dat, const mVec3_c &base) const {
    f32 y = 7.0f * dat->getHeight();
    out[0].set(16.0f, y + dat->getY0(3), 16.0f);
    out[0] += base;
    out[1].set(32.0f, y + dat->getY1(3), 32.0f);
    out[1] += base;
    out[2].set(32.0f, y + dat->getY2(3), 0.0f);
    out[2] += base;
}

// 8006CC44
unitDat_c *getUnitDat(unitDat_c *data, int idx) {
    return &data[(u8)idx];
}

// 8006CC54
unitDat_c *getUnitDat(unitDat_c *data, int x, int z) {
    return getUnitDat(data, (x & 15) + ((z & 15) << 4));
}

// 8006CC64
void posToUnit(int *unitX, int *unitZ, const mVec3_c *pos) {
    *unitX = pos->x / 32.0f;
    *unitZ = pos->z / 32.0f;
    if (pos->x < 0.0f) {
        (*unitX)--;
    }
    if (pos->z < 0.0f) {
        (*unitZ)--;
    }
}

// 8006CCD8
void posToBlock(int *blockX, int *blockZ, const mVec3_c *pos) {
    *blockX = pos->x / 512.0f;
    *blockZ = pos->z / 512.0f;
    if (pos->x < 0.0f) {
        (*blockX)--;
    }
    if (pos->z < 0.0f) {
        (*blockZ)--;
    }
}

// 8006CD4C
void unitToBlock(int *blockX, int *blockZ, int unitX, int unitZ) {
    *blockX = unitX / 16;
    *blockZ = unitZ / 16;
    if (unitX < 0) {
        (*blockX)--;
    }
    if (unitZ < 0) {
        (*blockZ)--;
    }
}

// 8006CD90
void unitToPos(mVec3_c *pos, int unitX, int unitZ) {
    pos->set(16.0f + 32.0f * unitX, 0.0f, 16.0f + 32.0f * unitZ);
}

// 8006CDF4
void snapToUnit(mVec3_c *out, const mVec3_c *pos) {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    unitToPos(out, unitX, unitZ);
}

// 8006CE3C
void blockInfo_c::clear() {
    mData = NULL;
    mY = 0.0f;
}

// 8006CE50
void blockInfo_c::set(unitDat_c *data, u32 y) {
    mData = data;
    mY = 56.0f * y;
}

// 8006CE84
BOOL bg_c::checkFlag(int bit) {
    return (mFlags >> bit) & 1;
}

// 8006CE94
void bg_c::clearFlags() {
    mFlags = 0;
}

// 8006CEA0
void bg_c::setFlags() {
    mFlags = 0xFFFFFFFF;
}

// 8006CEAC
void bg_c::init() {
    mBlockW = 0;
    mBlockH = 0;
    mCallback = NULL;
    mPrevY = 0.0f;
    mY = 0.0f;
    mFlags = 0;
    mLoaded = 0;
    for (int z = 0; z < 7; z++) {
        for (int x = 0; x < 7; x++) {
            mBlocks[z][x].clear();
        }
    }
    mUnitFlags.clear();
}

// 8006CF40
blockInfo_c *bg_c::getBlock(u32 x, u32 z) {
    if (x < mBlockW && z < mBlockH) {
        return &mBlocks[z][x];
    }
    return NULL;
}

// 8006CF74
BOOL bg_c::isLoaded() {
    for (int z = mBlockH - 1; z >= 0; z--) {
        for (int x = mBlockW - 1; x >= 0; x--) {
            if (mBlocks[z][x].mData == NULL) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

// 8006CFDC
BOOL bg_c::onFlag(int bit) {
    if (bit >= 0) {
        mFlags |= 1 << bit;
        return TRUE;
    }
    return FALSE;
}

// 8006D008
BOOL bg_c::offFlag(int bit) {
    if (bit >= 0) {
        mFlags &= ~(1 << bit);
        return TRUE;
    }
    return FALSE;
}

// 8006D034
void unitAnm_c::clear() {
    mFlags = 0;
}

// 8006D040
BOOL unitAnm_c::set(int x, int z, u32 time, f32 to, f32 from) {
    int idx = find(x, z);
    if (idx != -1) {
        rec_s *rec = &mRecs[idx];
        mFlags |= 1 << idx;
        rec->mX = (s8)x;
        rec->mZ = (s8)z;
        rec->mTimer = 0;
        rec->mTime = time;
        rec->mFrom = from;
        rec->mTo = to;
        return TRUE;
    }
    return FALSE;
}

// 8006D0FC
int unitAnm_c::find(int x, int z) {
    for (int i = 0; i < 32; i++) {
        rec_s *rec = &mRecs[i];
        if (((mFlags >> i) & 1) && rec->mX == x && rec->mZ == z) {
            return i;
        }
    }
    for (int i = 0; i < 32; i++) {
        if (!((mFlags >> i) & 1)) {
            return i;
        }
    }
    return -1;
}

// 8006D2C0
void unitAnm_c::update() {
    int i = 0;
    for (rec_s *rec = mRecs; rec < &mRecs[32]; rec++) {
        if (((mFlags >> i) & 1) && rec->mTimer < rec->mTime) {
            rec->mTimer++;
        } else {
            rec->mTimer = 0;
            mFlags &= ~(1 << i);
        }
        i++;
    }
}

// 8006D340
f32 unitAnm_c::getY(int x, int z) {
    int i = 0;
    for (rec_s *rec = mRecs; rec < &mRecs[32]; rec++) {
        if (((mFlags >> i) & 1) && rec->mTimer < rec->mTime && x == rec->mX && z == rec->mZ) {
            f32 t = (f32)rec->mTimer / (f32)(rec->mTime - 1);
            if (t > 1.0f) {
                t = 1.0f;
            }
            f32 from = rec->mFrom;
            return from + t * (rec->mTo - from);
        }
        i++;
    }
    return 0.0f;
}

// The attribute of each triangle (by index) of a unit, cell type 1.
static inline int getTriAttr1_0(int attr) {
    return attr < BG_ATTR_NUM ? getAttrData(attr)->mTriAttr1[0] : 0;
}

static inline int getTriAttr1_1(int attr) {
    return attr < BG_ATTR_NUM ? getAttrData(attr)->mTriAttr1[1] : 0;
}

static inline int getTriAttr1_2(int attr) {
    return attr < BG_ATTR_NUM ? getAttrData(attr)->mTriAttr1[2] : 0;
}

static inline int getTriAttr1_3(int attr) {
    return attr < BG_ATTR_NUM ? getAttrData(attr)->mTriAttr1[3] : 0;
}

extern int (*const sGetTriAttr1[4])(int attr);

// 8006D414 (the four getTriAttr1_n follow it, 3 first)
int getTriAttr1(int attr, int idx) {
    return sGetTriAttr1[idx & 3](attr);
}

// 8046D250
int (*const sGetTriAttr1[4])(int attr) = {getTriAttr1_0, getTriAttr1_1, getTriAttr1_2, getTriAttr1_3};

// The attribute of each triangle (by index) of a unit, cell type 0.
static inline int getTriAttr0_0(int attr) {
    return attr < BG_ATTR_NUM ? getAttrData(attr)->mTriAttr0[0] : 0;
}

static inline int getTriAttr0_1(int attr) {
    return attr < BG_ATTR_NUM ? getAttrData(attr)->mTriAttr0[1] : 0;
}

static inline int getTriAttr0_2(int attr) {
    return attr < BG_ATTR_NUM ? getAttrData(attr)->mTriAttr0[2] : 0;
}

static inline int getTriAttr0_3(int attr) {
    return attr < BG_ATTR_NUM ? getAttrData(attr)->mTriAttr0[3] : 0;
}

extern int (*const sGetTriAttr0[4])(int attr);

// 8006D4CC (the four getTriAttr0_n follow it, 3 first)
int getTriAttr0(int attr, int idx) {
    return sGetTriAttr0[idx & 3](attr);
}

// 8046D260
int (*const sGetTriAttr0[4])(int attr) = {getTriAttr0_0, getTriAttr0_1, getTriAttr0_2, getTriAttr0_3};

// 80750458: by cell_c::mType.
static int (*const sGetTriAttr[2])(int attr, int idx) = {getTriAttr0, getTriAttr1};

static inline s8 getWater(int attr) {
    if (attr < BG_ATTR_NUM) {
        return getAttrData(attr)->getWater();
    }
    return BG_WATER_NONE;
}

// Fills one cell from the unit (x, z). mode 2: a unit marked in the slot's unit flags is flat; a non-zero
// type makes the water triangles flat at the water level.
inline void cell_c::makeTriangles(const int *xz, int blockX, int blockZ, int mode, int type) {
    blockInfo_c *block = sCurBg->getBlock(blockX, blockZ);
    f32 y;
    if (block != NULL) {
        y = block->mY;
    } else {
        y = 0.0f;
    }
    if (mDat == NULL) {
        return;
    }
    mVec3_c base(32.0f * xz[0], y, 32.0f * xz[1]);
    mDat->calcTri0(mTri[0], mDat, base);
    mDat->calcTri1(mTri[1], mDat, base);
    mDat->calcTri2(mTri[2], mDat, base);
    mDat->calcTri3(mTri[3], mDat, base);
    if (mode == 2) {
        if (sCurBg == NULL || !sCurBg->mUnitFlags.check(xz[0], xz[1])) {
            return;
        }
        f32 h = y + mDat->getBaseY();
        for (u32 i = 0; i < 4; i++) {
            setTriY(i, h);
        }
    } else if (type != 0) {
        f32 h = y + mDat->getBaseY() >= 56.0f ? 56.0f : 0.0f;
        for (u32 i = 0; i < 4; i++) {
            int idx = i;
            if (getWater(sGetTriAttr[mType](getAttr(), idx)) != BG_WATER_RIVER) {
                continue;
            }
            if (isWaterAttr(sGetTriAttr[LAYER_TOP](getAttr(), idx), 0)) {
                setTriWaterY(idx, h);
            } else {
                setTriY(idx, h);
            }
        }
    }
}

inline void cell_c::make(const int *xz, int mode, int type) {
    mDat = getUnitDat(xz[0], xz[1]);
    mType = type;
    int blockX, blockZ;
    unitToBlock(&blockX, &blockZ, xz[0], xz[1]);
    if (sCurBg == NULL) {
        return;
    }
    makeTriangles(xz, blockX, blockZ, mode, type);
}

inline cell_c::cell_c(const mVec3_c *pos, int type) {
    int xz[2];
    posToUnit(&xz[0], &xz[1], pos);
    make(xz, 0, type);
}

// The cells of the grid around [mMinX..mMaxX] x [mMinZ..mMaxZ]. mode 2: units marked in the slot's
// unit flags are flat; a non-zero type makes the water triangles flat at the water level.
// 8006D584
void grid_c::make(const int *min, const int *max, int mode, int type) {
    int xz[2];
    u32 sizeX = max[0] - min[0] + 1;
    u32 sizeZ = max[1] - min[1] + 1;
    if (sizeX <= 7 && sizeZ <= 7) {
        mMinX = min[0];
        mMinZ = min[1];
        mMaxX = max[0];
        mMaxZ = max[1];
        for (xz[1] = mMinZ; xz[1] <= mMaxZ; xz[1]++) {
            for (xz[0] = mMinX; xz[0] <= mMaxX; xz[0]++) {
                cell_c *cell = &mCells[xz[1] - mMinZ][xz[0] - mMinX];
                cell->make(xz, mode, type);
            }
        }
    } else {
        mMinX = min[0];
        mMinZ = min[1];
        mMaxX = min[0] + 6;
        mMaxZ = min[1] + 6;
        for (xz[1] = mMinZ; xz[1] <= mMaxZ; xz[1]++) {
            for (xz[0] = mMinX; xz[0] <= mMaxX; xz[0]++) {
                cell_c *cell = &mCells[xz[1] - mMinZ][xz[0] - mMinX];
                cell->make(xz, mode, type);
            }
        }
    }
}

// 8006DDBC
cell_c *grid_c::getCell(const int *xz) {
    if (xz[0] >= mMinX && xz[0] <= mMaxX && xz[1] >= mMinZ && xz[1] <= mMaxZ) {
        return &mCells[xz[1] - mMinZ][xz[0] - mMinX];
    }
    return NULL;
}

// 8006DE18
manager_c::manager_c() {
    initDefault();
    sCurBg = NULL;
    for (bg_c *bg = mBg; bg < &mBg[16]; bg++) {
        bg->init();
    }
}

// 8006E11C
void manager_c::initDefault() {
    memset(&mDefault, -1, sizeof(mDefault));
    mDefault.mAttr = BG_ATTR_WALL;
    mDefault.mFlags &= ~UNIT_ROUTE;
}

// 8006E170
BOOL manager_c::setCurrent(u32 idx) {
    if (idx < 16) {
        mCurrent = idx;
        return TRUE;
    }
    return FALSE;
}

// 8006E18C
addDat_c::addDat_c() {
    mAttr = 0;
    mMvbg = NULL;
}

// 8006E19C
void addDat_c::set(int attr, mvbg_c *mvbg) {
    mAttr = attr;
    mMvbg = mvbg;
}

// 8006E1A8
void addDat_c::set(const addDat_c &other) {
    mAttr = other.mAttr;
    mMvbg = other.mMvbg;
}

// 8006E1BC
groundChk_c::groundChk_c(const mVec3_c *pos, int type, int mode, u8 arg) {
    mArg = arg;
    check(pos, type, mode);
}

// 8006E238
groundChk_c::groundChk_c(int unitX, int unitZ, int type, int mode, u8 arg) {
    mVec3_c pos;
    unitToPos(&pos, unitX, unitZ);
    mArg = arg;
    check(&pos, type, mode);
}

// 8006E2C8
BOOL groundChk_c::isContinuous() {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, &mPos);
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        return dat->isContinuous();
    }
    return TRUE;
}

// 8006E31C
f32 groundChk_c::getHeight(BOOL withColumn) {
    f32 y = mFloor.calcY(mPos);
    if (!withColumn) {
        return y;
    }
    f32 radius, height;
    int attr;
    if (getColumnAttr(mUnitX, mUnitZ, &radius, &height, &attr) && attr != BG_ATTR_ATTRW) {
        mVec3_c center;
        unitToPos(&center, mUnitX, mUnitZ);
        mVec3_c d = mPos - center;
        if (d.x * d.x + d.z * d.z <= radius * radius) {
            return y + height;
        }
    }
    return y;
}

// 8006E400
BOOL groundChk_c::isUnderWater(f32 y) {
    if (mWater != BG_WATER_NONE) {
        return y <= mWaterY;
    }
    return FALSE;
}

// 8006E42C
void groundChk_c::setSlope(f32 prevY, f32 y, mAng ang) {
    f32 d = y - prevY;
    EGG::Mathf::abs(d); // Unused result, likely left over from a stripped assertion on the slope.
    mDir = l_front;
    s16 add = d > 0.0f ? 0 : 0x8000;
    mDir.rotY(mAng(add + ang.mAngle));
}

// 8006E4AC
void groundChk_c::check(const mVec3_c *pos, int type, int mode) {
    if (sCurBg == NULL) {
        return;
    }
    f32 prevY = sCurBg->mPrevY;
    f32 bgY = sCurBg->mY;
    mSlope = 0;
    mPos = *pos;
    posToUnit(&mUnitX, &mUnitZ, pos);
    int quarter = getUnitQuarter(pos);
    mUnitAttr = getUnitAttr(mUnitX, mUnitZ);
    mAttr = sGetTriAttr[type](mUnitAttr, quarter);
    mWaterY = -32.0f;
    mWater = getWater(mAttr);
    mDir = l_front;

    cell_c cell(pos, type);

    int attr = sGetTriAttr[cell.mType](cell.getAttr(), quarter);
    mVec3_c *tri = cell.mTri[quarter & 3];
    mFloor.set(tri[0], tri[1], tri[2], attr, NULL);
    if (isWaterAttr(mAttr, mode)) {
        mVec3_c center;
        if (mUnitAttr == BG_ATTR_WAVE_SW2) {
            snapToUnit(&center, pos);
            mVec3_c v0(center.x - 16.0f, center.y, 16.0f + center.z);
            mVec3_c v1(v0.x, center.y, v0.z - 32.0f * bgY);
            mVec3_c v2(v0.x + 32.0f * bgY, center.y, v0.z);
            dBGC::poly_c poly(v1, v0, v2, l_up);
            if (poly.checkInsideXZ(*pos) && bgY != 0.0f) {
                mAttr = BG_ATTR_WAVE;
                setSlope(prevY, bgY, mAng(0x6000));
            } else {
                mWater = BG_WATER_NONE;
                mAttr = BG_ATTR_SAND;
            }
            mSlope = 1;
            mSlopeCenter.set(0.5f * (v1.x + v2.x), 0.5f * (v1.y + v2.y), 0.5f * (v1.z + v2.z));
        } else if (mUnitAttr == BG_ATTR_WAVE_SW) {
            snapToUnit(&center, pos);
            f32 t = 1.0f - bgY;
            mVec3_c v0(16.0f + center.x, center.y, center.z - 16.0f);
            mVec3_c v1(v0.x, center.y, v0.z + 32.0f * t);
            mVec3_c v2(v0.x - 32.0f * t, center.y, v0.z);
            dBGC::poly_c poly(v0, v2, v1, l_up);
            if (!poly.checkInsideXZ(*pos) || t == 0.0f) {
                mAttr = BG_ATTR_WAVE;
                setSlope(prevY, bgY, mAng(0x6000));
            } else {
                mWater = BG_WATER_NONE;
                mAttr = BG_ATTR_SAND;
            }
            mSlope = 1;
            mSlopeCenter.set(0.5f * (v1.x + v2.x), 0.5f * (v1.y + v2.y), 0.5f * (v1.z + v2.z));
        } else if (mUnitAttr == BG_ATTR_WAVE_SE2) {
            snapToUnit(&center, pos);
            mVec3_c v0(16.0f + center.x, center.y, 16.0f + center.z);
            mVec3_c v1(v0.x, center.y, v0.z - 32.0f * bgY);
            mVec3_c v2(v0.x - 32.0f * bgY, center.y, v0.z);
            dBGC::poly_c poly(v1, v2, v0, l_up);
            if (poly.checkInsideXZ(*pos) && bgY != 0.0f) {
                mAttr = BG_ATTR_WAVE;
                setSlope(prevY, bgY, mAng(-0x6000));
            } else {
                mWater = BG_WATER_NONE;
                mAttr = BG_ATTR_SAND;
            }
            mSlope = 1;
            mSlopeCenter.set(0.5f * (v1.x + v2.x), 0.5f * (v1.y + v2.y), 0.5f * (v1.z + v2.z));
        } else if (mUnitAttr == BG_ATTR_WAVE_SE) {
            snapToUnit(&center, pos);
            f32 t = 1.0f - bgY;
            mVec3_c v0(center.x - 16.0f, center.y, center.z - 16.0f);
            mVec3_c v1(v0.x, center.y, v0.z + 32.0f * t);
            mVec3_c v2(v0.x + 32.0f * t, center.y, v0.z);
            dBGC::poly_c poly(v0, v1, v2, l_up);
            if (!poly.checkInsideXZ(*pos) || t == 0.0f) {
                mAttr = BG_ATTR_WAVE;
                setSlope(prevY, bgY, mAng(-0x6000));
            } else {
                mWater = BG_WATER_NONE;
                mAttr = BG_ATTR_SAND;
            }
            mSlope = 1;
            mSlopeCenter.set(0.5f * (v1.x + v2.x), 0.5f * (v1.y + v2.y), 0.5f * (v1.z + v2.z));
        } else if (mUnitAttr == BG_ATTR_WAVE_S) {
            snapToUnit(&center, pos);
            f32 z = 16.0f + center.z - 32.0f * bgY;
            if (z > pos->z) {
                mWater = BG_WATER_NONE;
                mAttr = BG_ATTR_SAND;
            } else {
                mAttr = BG_ATTR_WAVE;
                setSlope(prevY, bgY, mAng(-0x8000));
            }
            mSlope = 1;
            mSlopeCenter.x = center.x;
            mSlopeCenter.y = 0.0f;
            mSlopeCenter.z = z;
        } else {
            int dir = mAttr < BG_ATTR_NUM ? getAttrData(mAttr)->mDir & 0xF : 0;
            if (dir != 0) {
                mDir.rotY(mAng((dir - 1) << 13));
            }
        }

        if (mWater == BG_WATER_RIVER || mWater == BG_WATER_POND) {
            f32 y = mFloor.calcY(*pos);
            if (type != 0) {
                if (y < 14.0f) {
                    mWaterY = 14.0f;
                } else {
                    mWaterY = 70.0f;
                }
            } else if (y > 14.0f) {
                mWaterY = 70.0f;
            } else {
                mWaterY = 14.0f;
            }
        } else if (mWater == BG_WATER_SEA) {
            mWaterY = 14.0f;
        }
    } else {
        mWater = BG_WATER_NONE;
    }
}

// 8006EFA4
BOOL floor_c::set(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2, int attr, mvbg_c *mvbg) {
    addDat_c::set(attr, mvbg);
    dBGC::poly_c::set(p0, p1, p2);
    return TRUE;
}

static inline int getTriAttr0Inline(int attr, int idx) {
    if (attr < BG_ATTR_NUM) {
        return getAttrData(attr)->mTriAttr0[idx];
    }
    return 0;
}

// 8006F018
void floorList_c::make(grid_c *grid, const int *min, const int *max) {
    int xz[2];
    for (xz[1] = min[1]; xz[1] <= max[1]; xz[1]++) {
        for (xz[0] = min[0]; xz[0] <= max[0]; xz[0]++) {
            cell_c *cell = grid->getCell(xz);
            if (cell == NULL) {
                continue;
            }
            mVec3_c center;
            unitToPos(&center, xz[0], xz[1]);
            f32 x1 = 16.0f + center.x;
            f32 x0 = center.x - 16.0f;
            f32 z1 = 16.0f + center.z;
            f32 z0 = center.z - 16.0f;
            if (cell->mDat != NULL ? cell->mDat->isFlat() : TRUE) {
                int attr = cell->getAttr();
                int a0 = getTriAttr0Inline(attr, 0);
                BOOL flat = getTriAttr0Inline(attr, 1) == a0;
                if (flat) {
                    flat = getTriAttr0Inline(attr, 2) == a0;
                }
                if (flat) {
                    flat = getTriAttr0Inline(attr, 3) == a0;
                }
                if (flat) {
                    f32 y = cell->mTri[0][0].y;
                    mVec3_c p0(x0, y, z0);
                    mVec3_c p1(x0, y, z1);
                    mVec3_c p2(x1, y, z1);
                    mVec3_c p3(x1, y, z0);
                    add(p0, p1, p2, sGetTriAttr[cell->mType](cell->getAttr(), 0), NULL);
                    add(p0, p2, p3, sGetTriAttr[cell->mType](cell->getAttr(), 0), NULL);
                    continue;
                }
            }
            add(cell->mTri[0][0], cell->mTri[0][1], cell->mTri[0][2], sGetTriAttr[cell->mType](cell->getAttr(), 0), NULL);
            add(cell->mTri[1][0], cell->mTri[1][1], cell->mTri[1][2], sGetTriAttr[cell->mType](cell->getAttr(), 1), NULL);
            add(cell->mTri[2][0], cell->mTri[2][1], cell->mTri[2][2], sGetTriAttr[cell->mType](cell->getAttr(), 2), NULL);
            add(cell->mTri[3][0], cell->mTri[3][1], cell->mTri[3][2], sGetTriAttr[cell->mType](cell->getAttr(), 3), NULL);
        }
    }
}

// 8006F3F8
BOOL floorList_c::add(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2, int attr, mvbg_c *mvbg) {
    if (getNum() < 0x5A) {
        return mFloors[mNum++].set(p0, p1, p2, attr, mvbg);
    }
    return FALSE;
}

// 8006F424
wall_c::wall_c(const wallDat_s &dat, const dBGC::vecXZ_c &normal, mvbg_c *mvbg) : mNoFace(0) {
    set(dat, normal, mvbg);
}

// 8006F4CC
dBGC::vecXZ_c wall_c::getCenter() const {
    return dBGC::vecXZ_c(0.5f * (mStart.x + mEnd.x), 0.5f * (mStart.z + mEnd.z));
}

// 8006F50C
BOOL wall_c::set(const wall_c &other) {
    addDat_c::set(other);
    dBGC::lineXZ_c::set(other.mStart, other.mEnd, other.mNormal);
    f32 y0 = other.mY0;
    f32 y1 = other.mY1;
    mType = other.mType;
    mY0 = y0;
    mY1 = y1;
    mY1Low = y1 < y0;
    mNoFace = other.mNoFace != 0;
    return TRUE;
}

// 8006F5A0
BOOL wall_c::set(const wallDat_s &dat, const dBGC::vecXZ_c &normal, mvbg_c *mvbg) {
    addDat_c::set(dat.mAttr, mvbg);
    dBGC::lineXZ_c::set(dat.mStart, dat.mEnd, normal);
    mType = dat.mType;
    mY0 = dat.mY0;
    mY1 = dat.mY1;
    mNoFace = dat.mNoFace;
    mY1Low = mY1 < mY0;
    return TRUE;
}

// 8006F638
f32 wall_c::calcCorrectR(const mVec3_c &pos, BOOL noCheck, f32 r) const {
    if (noCheck) {
        return r;
    }
    if (pos.y > (mY1Low ? mY0 : mY1)) {
        return 0.0f;
    }
    mVec3_c a(mStart.x, getY0(), mStart.z);
    mVec3_c b(mEnd.x, getY1(), mEnd.z);
    mVec3_c c(mStart.x, getY0(), mStart.z);
    c.x += mNormal.x;
    c.z += mNormal.z;
    dBGC::poly_c poly(a, b, c);
    f32 y = poly.calcY(pos);
    if (y > (mY1Low ? mY0 : mY1)) {
        y = mY1Low ? mY0 : mY1;
    } else if (y < (mY1Low ? mY1 : mY0)) {
        y = mY1Low ? mY1 : mY0;
    }
    if (pos.y > y) {
        return 0.0f;
    }
    f32 d = y - pos.y;
    if (d < r) {
        return std::fabs(d);
    }
    return r;
}

static f32 sWallR[0x60];   // 80582AE8
static u8 sWallDone[0x60]; // 80582C68

// 8006F7D0
BOOL wallList_c::correct(mVec3_c *pos, const mVec3_c &old, f32 r, acch_c *acch, u32 flags, int arg,
                         BOOL noCheck) {
    dBGC::vecXZ_c p(pos->x, pos->z);
    dBGC::vecXZ_c o(old.x, old.z);
    u32 num;
    f32 *wr;
    wall_c *end;
    BOOL hit;
    BOOL crossHit;
    int type;
    u8 *done;
    hit = FALSE;
    num = mNum;
    memset(sWallR, 0, sizeof(sWallR));
    wr = sWallR;
    for (wall_c *wall = mWalls; wall < &mWalls[mNum]; wall++) {
        *wr++ = wall->calcCorrectR(*pos, noCheck, r);
    }
    crossHit = FALSE;
    memset(sWallDone, 0, sizeof(sWallDone));
    end = &mWalls[num];

    wr = sWallR;
    done = sWallDone;
    for (wall_c *wall = mWalls; wall < end; wr++, done++, wall++) {
        if (*done == 0 && *wr != 0.0f) {
            getCurrentScene();
            if (wall->correctCross(&p, o, *wr)) {
                type = wall->getType();
                if (type != WALL_TYPE_COLUMN) {
                    int attr = wall->getAttr();
                    acch->mWall.add(mAng(cM::atan2s(wall->mNormal.x, wall->mNormal.z)), type, attr, FALSE);
                    if (wall->mMvbg != NULL) {
                        wall->mMvbg->onHit(wall, arg, *wr);
                    }
                    hit = TRUE;
                    crossHit = TRUE;
                }
                *done = TRUE;
            }
        }
    }

    done = sWallDone;
    wr = sWallR;
    for (wall_c *wall = mWalls; wall < end; wr++, done++, wall++) {
        if (*done == 0 && *wr != 0.0f) {
            getCurrentScene();
            if (wall->correctCross(&p, o, *wr)) {
                type = wall->getType();
                if (type != WALL_TYPE_COLUMN) {
                    int attr = wall->getAttr();
                    acch->mWall.add(mAng(cM::atan2s(wall->mNormal.x, wall->mNormal.z)), type, attr, FALSE);
                    if (wall->mMvbg != NULL) {
                        wall->mMvbg->onHit(wall, arg, *wr);
                    }
                    hit = TRUE;
                    crossHit = TRUE;
                }
                *done = TRUE;
            }
        }
    }

    wr = sWallR;
    for (wall_c *wall = mWalls; wall < end; wr++, wall++) {
        if (*wr != 0.0f && !wall->mNoFace && wall->correctFace(&p, o, *wr)) {
            type = wall->getType();
            if (type != WALL_TYPE_COLUMN) {
                int attr = wall->getAttr();
                acch->mWall.add(mAng(cM::atan2s(wall->mNormal.x, wall->mNormal.z)), type, attr, FALSE);
                if (wall->mMvbg != NULL) {
                    wall->mMvbg->onHit(wall, arg, *wr);
                }
                hit = TRUE;
            }
        }
    }

    if (!crossHit) {
        wr = sWallR;
        for (wall_c *wall = mWalls; wall < end; wr++, wall++) {
            if (*wr != 0.0f && !wall->mNoFace && wall->correctEdge(&p, o, *wr)) {
                type = wall->getType();
                if (type != WALL_TYPE_COLUMN) {
                    int attr = wall->getAttr();
                    acch->mWall.add(mAng(cM::atan2s(wall->mNormal.x, wall->mNormal.z)), type, attr,
                                    !(flags & CHECK_EDGE_WALLS));
                    if (wall->mMvbg != NULL) {
                        wall->mMvbg->onHit(wall, arg, *wr);
                    }
                    hit = TRUE;
                }
            }
        }
    }
    pos->x = p.x;
    pos->z = p.z;
    return hit;
}

// 8006FC0C
BOOL wallList_c::add(const wall_c &wall) {
    if (getNum() < 0x60) {
        mWalls[mNum++].set(wall);
        return TRUE;
    }
    return FALSE;
}

// 8006FC58
BOOL wallList_c::add(const wallDat_s &dat, const dBGC::vecXZ_c &normal, mvbg_c *mvbg) {
    wall_c wall(dat, normal, mvbg);
    return add(wall);
}

// 8006FC94
void wallList_c::makeStepWalls(grid_c *grid, const int *xz, f32 h) {
    BOOL ext = h != 0.0f;
    cell_c *cell = grid->getCell(xz);
    if (cell == NULL) {
        return;
    }
    if (sCurBg == NULL) {
        return;
    }
    int right[2];
    int down[2];
    down[0] = xz[0];
    right[1] = xz[1];
    right[0] = xz[0] + 1;
    down[1] = xz[1] + 1;
    cell_c *cellR = grid->getCell(right);
    cell_c *cellD = grid->getCell(down);
    mVec3_c base;
    unitToPos(&base, xz[0], xz[1]);
    BOOL flag = sCurBg->mUnitFlags.check(xz[0], xz[1]);
    BOOL flagR = sCurBg->mUnitFlags.check(right[0], right[1]);
    BOOL flagD = sCurBg->mUnitFlags.check(down[0], down[1]);

    wallDat_s dat;
    dat.mNoFace = 0;
    if (cellR != NULL) {
        static dBGC::vecXZ_c sNormalX(1.0f, 0.0f);
        static dBGC::vecXZ_c sNormalNX(-1.0f, 0.0f);
        dBGC::vecXZ_c a(16.0f + base.x, base.z - 16.0f);
        dBGC::vecXZ_c b(16.0f + base.x, 16.0f + base.z);
        if (cell->mTri[3][1].y > cellR->mTri[1][2].y || cell->mTri[3][2].y > cellR->mTri[1][1].y) {
            dat.mStart.set(a);
            dat.mY0 = cell->mTri[3][2].y;
            dat.mEnd.set(b);
            dat.mY1 = cell->mTri[3][1].y;
            dat.mType = WALL_TYPE_NORMAL;
            dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 3);
            add(dat, sNormalX, NULL);
            if (ext && !flag) {
                dat.mY0 += h;
                dat.mY1 += h;
                dat.mType = WALL_TYPE_STEP_EXT;
                dat.mAttr = BG_ATTR_ATTRW;
                add(dat, sNormalNX, NULL);
            }
        } else if (cell->mTri[3][1].y < cellR->mTri[1][2].y || cell->mTri[3][2].y < cellR->mTri[1][1].y) {
            dat.mStart.set(a);
            dat.mY0 = cellR->mTri[1][1].y;
            dat.mEnd.set(b);
            dat.mY1 = cellR->mTri[1][2].y;
            dat.mType = WALL_TYPE_NORMAL;
            dat.mAttr = sGetTriAttr[cellR->mType](cellR->getAttr(), 1);
            add(dat, sNormalNX, NULL);
            if (ext && !flagR) {
                dat.mY0 += h;
                dat.mY1 += h;
                dat.mType = WALL_TYPE_STEP_EXT;
                dat.mAttr = BG_ATTR_ATTRW;
                add(dat, sNormalX, NULL);
            }
        }
    }
    if (cellD != NULL) {
        static dBGC::vecXZ_c sNormalZ(0.0f, 1.0f);
        static dBGC::vecXZ_c sNormalNZ(0.0f, -1.0f);
        dBGC::vecXZ_c a(base.x - 16.0f, 16.0f + base.z);
        dBGC::vecXZ_c b(16.0f + base.x, 16.0f + base.z);
        if (cell->mTri[2][1].y > cellD->mTri[0][2].y || cell->mTri[2][2].y > cellD->mTri[0][1].y) {
            dat.mStart.set(a);
            dat.mY0 = cell->mTri[2][1].y;
            dat.mEnd.set(b);
            dat.mY1 = cell->mTri[2][2].y;
            dat.mType = WALL_TYPE_NORMAL;
            dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 2);
            add(dat, sNormalZ, NULL);
            if (ext && !flag) {
                dat.mY0 += h;
                dat.mY1 += h;
                dat.mType = WALL_TYPE_STEP_EXT;
                dat.mAttr = BG_ATTR_ATTRW;
                add(dat, sNormalNZ, NULL);
            }
        } else if (cell->mTri[2][1].y < cellD->mTri[0][2].y || cell->mTri[2][2].y < cellD->mTri[0][1].y) {
            dat.mStart.set(a);
            dat.mY0 = cellD->mTri[0][2].y;
            dat.mEnd.set(b);
            dat.mY1 = cellD->mTri[0][1].y;
            dat.mType = WALL_TYPE_NORMAL;
            dat.mAttr = sGetTriAttr[cellD->mType](cellD->getAttr(), 0);
            add(dat, sNormalNZ, NULL);
            if (ext && !flagD) {
                dat.mY0 += h;
                dat.mY1 += h;
                dat.mType = WALL_TYPE_STEP_EXT;
                dat.mAttr = BG_ATTR_ATTRW;
                add(dat, sNormalZ, NULL);
            }
        }
    }

    static dBGC::vecXZ_c sNormalPP(0.70711f, 0.70711f);
    static dBGC::vecXZ_c sNormalPN(0.70711f, -0.70711f);
    static dBGC::vecXZ_c sNormalNN(-0.70711f, -0.70711f);
    static dBGC::vecXZ_c sNormalNP(-0.70711f, 0.70711f);
    dBGC::vecXZ_c center(base.x, base.z);
    dBGC::vecXZ_c nw(base.x - 16.0f, base.z - 16.0f);
    dBGC::vecXZ_c sw(base.x - 16.0f, 16.0f + base.z);
    dBGC::vecXZ_c ne(16.0f + base.x, base.z - 16.0f);
    dBGC::vecXZ_c se(16.0f + base.x, 16.0f + base.z);
    if (cell->mTri[0][0].y > cell->mTri[3][0].y || cell->mTri[0][1].y > cell->mTri[3][2].y) {
        dat.mStart.set(center);
        dat.mY0 = cell->mTri[0][0].y;
        dat.mEnd.set(ne);
        dat.mY1 = cell->mTri[0][1].y;
        dat.mType = WALL_TYPE_NORMAL;
        dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 0);
        add(dat, sNormalPP, NULL);
        if (ext && !flag) {
            dat.mY0 += h;
            dat.mY1 += h;
            dat.mType = WALL_TYPE_STEP_EXT;
            dat.mAttr = BG_ATTR_ATTRW;
            add(dat, sNormalNN, NULL);
        }
    } else if (cell->mTri[0][0].y < cell->mTri[3][0].y || cell->mTri[0][1].y < cell->mTri[3][2].y) {
        dat.mStart.set(center);
        dat.mY0 = cell->mTri[3][0].y;
        dat.mEnd.set(ne);
        dat.mY1 = cell->mTri[3][2].y;
        dat.mType = WALL_TYPE_NORMAL;
        dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 3);
        add(dat, sNormalNN, NULL);
        if (ext && !flag) {
            dat.mY0 += h;
            dat.mY1 += h;
            dat.mType = WALL_TYPE_STEP_EXT;
            dat.mAttr = BG_ATTR_ATTRW;
            add(dat, sNormalPP, NULL);
        }
    }
    if (cell->mTri[3][0].y > cell->mTri[2][0].y || cell->mTri[3][1].y > cell->mTri[2][2].y) {
        dat.mStart.set(center);
        dat.mY0 = cell->mTri[3][0].y;
        dat.mEnd.set(se);
        dat.mY1 = cell->mTri[3][1].y;
        dat.mType = WALL_TYPE_NORMAL;
        dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 3);
        add(dat, sNormalNP, NULL);
        if (ext && !flag) {
            dat.mY0 += h;
            dat.mY1 += h;
            dat.mType = WALL_TYPE_STEP_EXT;
            dat.mAttr = BG_ATTR_ATTRW;
            add(dat, sNormalPN, NULL);
        }
    } else if (cell->mTri[3][0].y < cell->mTri[2][0].y || cell->mTri[3][1].y < cell->mTri[2][2].y) {
        dat.mStart.set(center);
        dat.mY0 = cell->mTri[2][0].y;
        dat.mEnd.set(se);
        dat.mY1 = cell->mTri[2][2].y;
        dat.mType = WALL_TYPE_NORMAL;
        dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 2);
        add(dat, sNormalPN, NULL);
        if (ext && !flag) {
            dat.mY0 += h;
            dat.mY1 += h;
            dat.mType = WALL_TYPE_STEP_EXT;
            dat.mAttr = BG_ATTR_ATTRW;
            add(dat, sNormalNP, NULL);
        }
    }
    if (cell->mTri[1][0].y > cell->mTri[2][0].y || cell->mTri[1][2].y > cell->mTri[2][1].y) {
        dat.mStart.set(center);
        dat.mY0 = cell->mTri[1][0].y;
        dat.mEnd.set(sw);
        dat.mY1 = cell->mTri[1][2].y;
        dat.mType = WALL_TYPE_NORMAL;
        dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 1);
        add(dat, sNormalPP, NULL);
        if (ext && !flag) {
            dat.mY0 += h;
            dat.mY1 += h;
            dat.mType = WALL_TYPE_STEP_EXT;
            dat.mAttr = BG_ATTR_ATTRW;
            add(dat, sNormalNN, NULL);
        }
    } else if (cell->mTri[1][0].y < cell->mTri[2][0].y || cell->mTri[1][2].y < cell->mTri[2][1].y) {
        dat.mStart.set(center);
        dat.mY0 = cell->mTri[2][0].y;
        dat.mEnd.set(sw);
        dat.mY1 = cell->mTri[2][1].y;
        dat.mType = WALL_TYPE_NORMAL;
        dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 2);
        add(dat, sNormalNN, NULL);
        if (ext && !flag) {
            dat.mY0 += h;
            dat.mY1 += h;
            dat.mType = WALL_TYPE_STEP_EXT;
            dat.mAttr = BG_ATTR_ATTRW;
            add(dat, sNormalPP, NULL);
        }
    }
    if (cell->mTri[1][0].y > cell->mTri[0][0].y || cell->mTri[1][1].y > cell->mTri[0][2].y) {
        dat.mStart.set(center);
        dat.mY0 = cell->mTri[1][0].y;
        dat.mEnd.set(nw);
        dat.mY1 = cell->mTri[1][1].y;
        dat.mType = WALL_TYPE_NORMAL;
        dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 1);
        add(dat, sNormalPN, NULL);
        if (ext && !flag) {
            dat.mY0 += h;
            dat.mY1 += h;
            dat.mType = WALL_TYPE_STEP_EXT;
            dat.mAttr = BG_ATTR_ATTRW;
            add(dat, sNormalNP, NULL);
        }
    } else if (cell->mTri[1][0].y < cell->mTri[0][0].y || cell->mTri[1][1].y < cell->mTri[0][2].y) {
        dat.mStart.set(center);
        dat.mY0 = cell->mTri[0][0].y;
        dat.mEnd.set(nw);
        dat.mY1 = cell->mTri[0][2].y;
        dat.mType = WALL_TYPE_NORMAL;
        dat.mAttr = sGetTriAttr[cell->mType](cell->getAttr(), 0);
        add(dat, sNormalNP, NULL);
        if (ext && !flag) {
            dat.mY0 += h;
            dat.mY1 += h;
            dat.mType = WALL_TYPE_STEP_EXT;
            dat.mAttr = BG_ATTR_ATTRW;
            add(dat, sNormalPN, NULL);
        }
    }
}

// 80070A0C
void wallList_c::makeSteps(grid_c *grid, const int *min, const int *max, u32 flags) {
    f32 h = 0.0f;
    if (flags & CHECK_STEP_EXT_96) {
        h = 96.0f;
    } else if (flags & CHECK_STEP_EXT_48) {
        h = 48.0f;
    }
    int xz[2];
    for (xz[1] = max[1]; xz[1] >= min[1]; xz[1]--) {
        for (xz[0] = max[0]; xz[0] >= min[0]; xz[0]--) {
            makeStepWalls(grid, xz, h);
        }
    }
}

// 80070AEC
void column_c::set(const mVec3_c &pos, int type, int attr, f32 r, f32 h) {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, &pos);
    mUnitX = unitX;
    mUnitZ = unitZ;
    mCenter.x = pos.x;
    mCenter.y = pos.y;
    mCenter.z = pos.z;
    mRadius = r;
    mHeight = h;
    mType = type;
    addDat_c::set(attr, NULL);
}

// 80070BB4
BOOL column_c::makeWall(wallList_c *walls, const column_c &other) const {
    int dx = other.mUnitX - mUnitX;
    int dz = other.mUnitZ - mUnitZ;
    if ((dx == 0 && dz == 1) || (dx == 1 && (dz == -1 || dz == 0 || dz == 1))) {
        dBGC::vecXZ_c p0(mCenter.x, mCenter.z);
        dBGC::vecXZ_c p1(other.mCenter.x, other.mCenter.z);
        dBGC::vecXZ_c dir = p1 - p0;
        dir.normalize();
        f32 r0 = mRadius;
        dBGC::vecXZ_c start(p0.x - dir.x * r0, p0.z - dir.z * r0);
        f32 r1 = other.mRadius;
        dBGC::vecXZ_c end(p1.x + dir.x * r1, p1.z + dir.z * r1);
        dBGC::vecXZ_c normal;
        normal.setNormal(p0, p1);
        dBGC::vecXZ_c back(-normal.x, -normal.z);
        wallDat_s dat;
        dat.mNoFace = 1;
        dat.mStart.set(start);
        dat.mY0 = mCenter.y + getHeight();
        dat.mEnd.set(end);
        dat.mY1 = other.mCenter.y + other.getHeight();
        dat.mType = WALL_TYPE_COLUMN;
        dat.mAttr = BG_ATTR_NONE;
        walls->add(dat, normal, NULL);
        walls->add(dat, back, NULL);
        return TRUE;
    }
    return FALSE;
}

// 80070D94
BOOL columnList_c::correctSide(mVec3_c *pos, f32 r, acch_c *acch, int arg, clmcb_c *cb) {
    column_c *columns = mColumns;
    u8 attr;
    int type;
    BOOL hit = FALSE;
    for (column_c *column = columns; column < &columns[mNum]; column++) {
        mVec3_c prev;
        prev = *pos;
        if (column->correctSide(pos, r)) {
            mVec3_c d = *pos - column->mCenter;
            attr = column->mAttr;
            type = column->mType;
            acch->mWall.add(mAng(cM::atan2s(d.x, d.z)), type, attr, FALSE);
            hit = TRUE;
        }
    }
    return hit;
}

// 80070E80
BOOL columnList_c::correctTop(mVec3_c *pos, const mVec3_c &old, int *attr, int arg, clmcb_c *cb) {
    column_c *columns = mColumns;
    for (column_c *column = columns; column < &columns[mNum]; column++) {
        mVec3_c prev;
        prev = *pos;
        if (column->mType == COLUMN_TYPE_STAND && column->correctTop(pos, old)) {
            *attr = column->mAttr;
            return TRUE;
        }
    }
    return FALSE;
}

// 80070F30
BOOL columnList_c::add(f32 r, f32 h, const mVec3_c &pos, int type, int attr) {
    if (getNum() < 0x18) {
        mColumns[mNum].set(pos, type, attr, r, h);
        mNum++;
        return TRUE;
    }
    return FALSE;
}

// 80070F90
void columnList_c::make(clmcb_c *cb, const int *min, const int *max, BOOL anm) {
    for (int z = min[1]; z <= max[1]; z++) {
        for (int x = min[0]; x <= max[0]; x++) {
            f32 radius, height;
            int attr;
            if (cb->getAttr(&radius, &height, &attr, x, z)) {
                if (attr == BG_ATTR_ATTRW) {
                    if (anm) {
                        mVec3_c pos;
                        unitToPos(&pos, x, z);
                        pos.y = getUnitBaseY(x, z);
                        add(radius, height, pos, COLUMN_TYPE_ANM, attr);
                    }
                } else {
                    mVec3_c pos;
                    unitToPos(&pos, x, z);
                    pos.y = getUnitBaseY(x, z);
                    add(radius, height, pos, COLUMN_TYPE_STAND, attr);
                }
            } else if (anm && l_mgr.mUnitAnm.mFlags != 0) {
                f32 y = l_mgr.mUnitAnm.getY(x, z);
                if (y != 0.0f) {
                    mVec3_c pos;
                    unitToPos(&pos, x, z);
                    pos.y = getUnitBaseY(x, z);
                    add(y, 16.0f, pos, COLUMN_TYPE_ANM, BG_ATTR_ATTRW);
                }
            }
        }
    }
}

// 8046D270: the 8 neighbours of a unit.
static const int sNeighbors[8][2] = {
    {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1},
};

// 80071160
void columnList_c::makeWalls(wallList_c *walls, u32 flags, clmcb_c *cb) {
    BOOL flat;
    BOOL flat2;
    BOOL flag0;
    column_c *end = &mColumns[mNum];
    for (column_c *a = mColumns; a < end; a++) {
        for (column_c *b = mColumns; b < end; b++) {
            a->makeWall(walls, *b);
        }
    }
    if (flags & CHECK_STEP_EXT_48) {
        for (column_c *column = mColumns; column < end; column++) {
            int unitX, unitZ;
            posToUnit(&unitX, &unitZ, &column->mCenter);
            f32 y = getUnitBaseY(unitX, unitZ);
            unitDat_c *dat = getUnitDat(unitX, unitZ);
            flat = FALSE;
            if (dat != NULL && dat->isFlat()) {
                flat = TRUE;
            }
            for (const int *ofs = sNeighbors[0]; ofs < sNeighbors[8]; ofs += 2) {
                int x = unitX + ofs[0];
                int z = unitZ + ofs[1];
                f32 y2 = getUnitBaseY(x, z);
                unitDat_c *dat2 = getUnitDat(x, z);
                flat2 = FALSE;
                if (dat2 != NULL && dat2->isFlat()) {
                    flat2 = TRUE;
                }
                flag0 = FALSE;
                if (dat2 != NULL && dat2->isContinuous()) {
                    flag0 = TRUE;
                }
                f32 radius, height;
                int attr;
                BOOL straight = ((ofs[0] + ofs[1]) & 1) ^ 1;
                if (cb->getAttr(&radius, &height, &attr, x, z)) {
                    continue;
                }
                if ((y != y2 && flat && flat2) || (!flag0 && !straight)) {
                    mVec3_c pos;
                    unitToPos(&pos, x, z);
                    mVec3_c mid;
                    if (!flag0) {
                        mid = pos;
                    } else {
                        mid = column->mCenter + pos;
                        mid *= 0.5f;
                    }
                    mid.y = column->mCenter.y;
                    wallDat_s dat;
                    dat.mNoFace = 1;
                    dBGC::vecXZ_c a(column->mCenter.x, column->mCenter.z);
                    dBGC::vecXZ_c b(mid.x, mid.z);
                    dBGC::vecXZ_c normal;
                    normal.setNormal(a, b);
                    dBGC::vecXZ_c back(-normal.x, -normal.z);
                    dat.mStart.set(a);
                    dat.mY0 = mid.y + column->getHeight();
                    dat.mEnd.set(b);
                    dat.mY1 = dat.mY0;
                    dat.mType = WALL_TYPE_COLUMN;
                    dat.mAttr = BG_ATTR_NONE;
                    walls->add(dat, normal, NULL);
                    walls->add(dat, back, NULL);
                }
            }
        }
    }
}

// 800714C8
void acchWall_c::clear() {
    mNum = 0;
    mFlags = 0;
    for (int i = 0; i < 2; i++) {
        mAngle[i] = 0;
        mAttr[i] = 0;
        mType[i] = 0;
    }
}

// 800714F0
BOOL acchWall_c::add(mAng ang, int type, int attr, BOOL ignore) {
    mAng *angle = mAngle;
    int *wallType = mType;
    for (int i = 0; i < mNum; i++, angle++, wallType++) {
        if (angle->mAngle == ang.mAngle && *wallType == type) {
            return FALSE;
        }
    }
    if (mNum < 2) {
        mAngle[mNum] = ang.mAngle;
        mType[mNum] = type;
        mAttr[mNum] = attr;
        if (ignore) {
            mFlags |= 1 << mNum;
        }
        mNum++;
        return TRUE;
    }
    return FALSE;
}

// 800715A8
void acch_c::clear() {
    mWall.clear();
    mGroundAttr = 0;
    mPrevHitFlags = mHitFlags;
    mHitFlags = 0;
    mPushOut = l_zero;
    mGroundNormal = l_up;
}

// 8007162C
void acch_c::init() {
    mHitFlags = 0;
    mPrevHitFlags = 0;
    mGroundAttr = 0;
    mPushOut = l_zero;
    mGroundNormal = l_up;
    _00 = 1;
    mStepHeight = 0.0f;
}

// 80071690
void acch_c::calcWallFlags(const mAng ang) {
    int back;
    s16 a;
    int i;

    a = ang.mAngle;
    back = a + 0x8000;
    for (i = 0; i < mWall.getNum(); i++) {
        if ((mWall.mFlags >> i) & 1) {
            continue;
        }
        u16 d = mWall.getAngle(i).mAngle - back;
        if (d < 0x2100 || d >= 0xDF00) {
            switch (mWall.getType(i)) {
            case WALL_TYPE_STEP_EXT:
                mHitFlags |= HIT_STEP_EXT_FRONT;
                break;
            default:
                mHitFlags |= HIT_WALL_FRONT;
                break;
            }
        } else if (d < 0x6000) {
            switch (mWall.getType(i)) {
            case WALL_TYPE_STEP_EXT:
                mHitFlags |= HIT_STEP_EXT_LEFT;
                break;
            default:
                mHitFlags |= HIT_WALL_LEFT;
                break;
            }
        } else if (d < 0xA000) {
            switch (mWall.getType(i)) {
            case WALL_TYPE_STEP_EXT:
                mHitFlags |= HIT_STEP_EXT_BACK;
                break;
            default:
                mHitFlags |= HIT_WALL_BACK;
                break;
            }
        } else {
            switch (mWall.getType(i)) {
            case WALL_TYPE_STEP_EXT:
                mHitFlags |= HIT_STEP_EXT_RIGHT;
                break;
            default:
                mHitFlags |= HIT_WALL_RIGHT;
                break;
            }
        }
    }
    if (mWall.getNum() == 2) {
        int sum = mWall.getAngle(0).mAngle;
        sum += mWall.getAngle(1).mAngle;
        s16 mid = (sum >> 1) + 0x7FFF;
        int diff = a - mid;
        f32 abs = (f32)diff < 0.0f ? -(f32)diff : (f32)diff;
        if (abs < 8192.0f) {
            mHitFlags |= HIT_CORNER;
        }
        if ((s16)sum == 0) {
            mHitFlags |= HIT_FLAG12;
        }
    }
}

} // namespace dBGCF

// The callback of acch_c::check: corrects the position against walls and columns.
class dtcbCorrect_c : public dBGCF::dtcb_c {
public:
    dtcbCorrect_c(dBGCF::acch_c *acch, mVec3_c *pos, const mVec3_c &old, mAng ang, f32 r, int arg,
                  u32 flags); // 80071888

    virtual void checkWall(dBGCF::wallList_c *walls);           // 800718CC
    virtual void checkFloor(dBGCF::floorList_c *floors);        // 80071900
    virtual void checkColumn(dBGCF::columnList_c *columns);     // 80071904

    /* 0x04 */ dBGCF::acch_c *mAcch;
    /* 0x08 */ mVec3_c *mPos;
    /* 0x0C */ mVec3_c mOld;
    /* 0x18 */ mAng mAngle;
    /* 0x1C */ f32 mRadius;
    /* 0x20 */ int mArg;
    /* 0x24 */ u32 mFlags;
}; // size 0x28

// The callback of checkLine: finds the first piece of geometry the segment mStart..*mEnd crosses.
class dtcbLineChk_c : public dBGCF::dtcb_c {
public:
    dtcbLineChk_c(mVec3_c *end, const mVec3_c &start, u32 flags); // 800719B4

    virtual void checkWall(dBGCF::wallList_c *walls);       // 80071A34
    virtual void checkFloor(dBGCF::floorList_c *floors);    // 80071C48
    virtual void checkColumn(dBGCF::columnList_c *columns); // 80071D74

    /* 0x04 */ mVec3_c *mEnd;
    /* 0x08 */ mVec3_c mStart;
    /* 0x14 */ u32 mFlags;
    /* 0x18 */ u8 mHit;
    /* 0x1C */ dBGCF::addDat_c mDat;
    /* 0x24 */ mVec3_c mNormal;
    /* 0x30 */ int mAttr;
    /* 0x34 */ u32 mWallNum;
    /* 0x38 */ u32 mFloorNum;
    /* 0x3C */ u32 mColumnNum;
}; // size 0x40

// 80071888
dtcbCorrect_c::dtcbCorrect_c(dBGCF::acch_c *acch, mVec3_c *pos, const mVec3_c &old, mAng ang, f32 r, int arg,
                             u32 flags)
    : mAcch(acch), mPos(pos), mOld(old), mAngle(ang), mRadius(r), mArg(arg), mFlags(flags) {}

// 800718CC
void dtcbCorrect_c::checkWall(dBGCF::wallList_c *walls) {
    walls->correct(mPos, mOld, mRadius, mAcch, mFlags, mArg, (mFlags & dBGCF::CHECK_STEP_EXT) != 0);
}

// 80071900
void dtcbCorrect_c::checkFloor(dBGCF::floorList_c *floors) {}

// 80071904
void dtcbCorrect_c::checkColumn(dBGCF::columnList_c *columns) {
    if (mFlags & dBGCF::CHECK_FLOOR) {
        int attr;
        if (columns->correctTop(mPos, mOld, &attr, mArg, dBGCF::getClmcb())) {
            mAcch->mHitFlags |= dBGCF::HIT_GROUND;
            mAcch->mGroundAttr = attr;
        }
    }
    if (mFlags & dBGCF::CHECK_WALLS) {
        columns->correctSide(mPos, mRadius, mAcch, mArg, dBGCF::getClmcb());
    }
}

// 800719B4
dtcbLineChk_c::dtcbLineChk_c(mVec3_c *end, const mVec3_c &start, u32 flags)
    : mEnd(end), mStart(start), mFlags(flags), mHit(0) {
    mAttr = 0;
    mWallNum = 0;
    mFloorNum = 0;
    mColumnNum = 0;
}

static inline BOOL isFront(const dBGC::poly_c *poly, const mVec3_c &p) {
    return poly->calcDist(p) >= 0.0f;
}

// 80071A34
void dtcbLineChk_c::checkWall(dBGCF::wallList_c *walls) {
    mVec3_c cross;
    mWallNum = walls->mNum;
    for (dBGCF::wall_c *wall = walls->mWalls; wall < &walls->mWalls[mWallNum]; wall++) {
        mVec3_c normal(wall->mNormal.x, 0.0f, wall->mNormal.z);
        mVec3_c a(wall->mStart.x, wall->getY0(), wall->mStart.z);
        mVec3_c b(wall->mStart.x, -48.0f, wall->mStart.z);
        mVec3_c c(wall->mEnd.x, -48.0f, wall->mEnd.z);
        mVec3_c d(wall->mEnd.x, wall->getY0(), wall->mEnd.z);
        dBGC::poly_c polys[2] = {dBGC::poly_c(a, b, c, normal), dBGC::poly_c(a, c, d, normal)};
        for (dBGC::poly_c *poly = polys; poly < polys + 2; poly++) {
            if (isFront(poly, mStart) && !isFront(poly, *mEnd)) {
                if (poly->crossSeg(&cross, mStart, *mEnd)) {
                    *mEnd = mVec3_c(cross.x, mEnd->y, cross.z);
                    mDat.set(*wall);
                    mNormal = poly->mNormal;
                    mAttr = wall->mAttr;
                    mHit = 1;
                }
            }
        }
    }
}

// 80071C48
void dtcbLineChk_c::checkFloor(dBGCF::floorList_c *floors) {
    mFloorNum = floors->mNum;
    for (dBGCF::floor_c *floor = floors->mFloors; floor < &floors->mFloors[mFloorNum]; floor++) {
        if (isFront(floor, mStart) && !isFront(floor, *mEnd)) {
            mVec3_c cross;
            if (floor->crossSeg(&cross, mStart, *mEnd)) {
                *mEnd = cross;
                mDat.set(*floor);
                mNormal = floor->mNormal;
                mAttr = floor->mAttr;
                mHit = 1;
            }
        }
    }
}

// 80071D74
void dtcbLineChk_c::checkColumn(dBGCF::columnList_c *columns) {
    mColumnNum = columns->mNum;
    dBGCF::column_c *base = columns->mColumns;
    for (dBGCF::column_c *column = base; column < &base[mColumnNum]; column++) {
        mVec3_c prev;
        prev = *mEnd;
        if (column->crossTop(mEnd, mStart)) {
            mDat.set(*column);
            mNormal = dBGCF::l_up;
            mAttr = column->mAttr;
            mHit = 1;
        }
        prev = *mEnd;
        if (column->crossSide(mEnd, mStart)) {
            mDat.set(*column);
            mNormal.set(mEnd->x - column->mCenter.x, 0.0f, mEnd->z - column->mCenter.z);
            mNormal.normalizeRS();
            mAttr = column->mAttr;
            mHit = 1;
        }
    }
}

namespace dBGCF {

// 80071EC8
void mvbg_c::init() {
    mPointNum = 0;
    mPos = l_zero;
    mSizeX = 0.0f;
    mSizeZ = 0.0f;
    mHeight = 0.0f;
    mScale.set(1.0f, 1.0f, 1.0f);
    mAngle = 0;
    mNext = NULL;
    mActive = 0;
    for (int i = 0; i < 4; i++) {
        mPoint[i] = l_zero;
        mNormal[i].set(0.0f, 0.0f);
    }
}

// 80071FA0
BOOL mvbg_c::set(f32 sizeX, f32 sizeZ, f32 height, const mVec3_c &pos, mAng ang, const mVec3_c &scale) {
    mSizeX = sizeX;
    mSizeZ = sizeZ;
    mHeight = height;
    l_mgr.mMvbgs.calc(this, pos, ang, scale);
    mPos = pos;
    mScale = scale;
    mAngle = ang.mAngle;
    mActive = 1;
    return TRUE;
}

// 80072068
void mvbg_c::onHit(wall_c *wall, int arg, f32 dist) {}

// 8007206C
mvbgList_c *getMvbgList() {
    return &l_mgr.mMvbgs;
}

// 80072080
void mvbgList_c::clear() {
    mHead = NULL;
}

void vecMax(mVec3_c *a, const mVec3_c &b); // 80072A28
void vecMin(mVec3_c *a, const mVec3_c &b); // 80072A68

// 8007208C
BOOL mvbgList_c::calc(mvbg_c *mvbg, const mVec3_c &pos, mAng ang, const mVec3_c &scale) {
    int a = ang.mAngle;
    BOOL changed = TRUE;
    BOOL moved = TRUE;
    if (mvbg->isAngle(ang)) {
        if (!(mvbg->mPos != pos)) {
            moved = FALSE;
        }
    }
    if (!moved && !(mvbg->mScale != scale)) {
        changed = FALSE;
    }
    if (changed) {
        f32 hx = 0.5f * mvbg->mSizeX;
        f32 hz = 0.5f * mvbg->mSizeZ;
        mvbg->mPos = pos;
        mvbg->mScale = scale;
        mvbg->mAngle = ang.mAngle;
        f32 y = mvbg->mHeight;
        mVec3_c corners[4];
        corners[0].set(-hx, y, hz);
        corners[1].set(hx, y, hz);
        corners[2].set(hx, y, -hz);
        corners[3].set(-hx, y, -hz);
        mMtx_c mtx;
        PSMTXTrans(mtx, pos.x, pos.y, pos.z);
        mtx.YrotM(ang);
        mMtx_c scaleMtx = mMtx_c::createScale(scale.x, scale.y, scale.z);
        PSMTXConcat(mtx, scaleMtx, mtx);
        mVec3_c max;
        mVec3_c min;
        mVec3_c *corner = corners;
        mVec3_c *point = mvbg->mPoint;
        dBGC::vecXZ_c *normal = mvbg->mNormal;
        if (mvbg->mSizeX == 0.0f || mvbg->mSizeZ == 0.0f) {
            int odd = 1;
            if (0.0f == mvbg->mSizeZ) {
                odd = 0;
            }
            mvbg->mPointNum = MVBG_POINTS_LINE;
            for (int i = 0; i < 4; i++, corner++, a += 0x4000) {
                if ((i & 1) == odd) {
                    fn_803911A4(mtx.m, *corner, *point);
                    if (i == odd) {
                        max = *point;
                        min = *point;
                    } else {
                        vecMax(&max, *point);
                        vecMin(&min, *point);
                    }
                    normal->x = 0.0f;
                    normal->z = 1.0f;
                    normal->rotY(a);
                    point++;
                    normal++;
                }
            }
        } else {
            mvbg->mPointNum = MVBG_POINTS_BOX;
            for (int i = 0; i < mvbg->mPointNum; i++) {
                fn_803911A4(mtx.m, *corner, *point);
                if (i == 0) {
                    max = *point;
                    min = *point;
                } else {
                    vecMax(&max, *point);
                    vecMin(&min, *point);
                }
                normal->x = 0.0f;
                normal->z = 1.0f;
                normal->rotY(a);
                point++;
                corner++;
                normal++;
                a += 0x4000;
            }
        }
        mvbg->mCenter.set(0.5f * (max.x + min.x), 0.5f * (max.y + min.y), 0.5f * (max.z + min.z));
        mvbg->mHalf.set(max.x - mvbg->mCenter.x, max.y - mvbg->mCenter.y, max.z - mvbg->mCenter.z);
        return TRUE;
    }
    return FALSE;
}

// 80072478
BOOL mvbgList_c::add(f32 sizeX, f32 sizeZ, f32 height, mvbg_c *mvbg, const mVec3_c &pos, mAng ang,
                     const mVec3_c *scale) {
    mVec3_c s(1.0f, 1.0f, 1.0f);
    if (scale != NULL) {
        s = *scale;
    }
    if (mvbg->set(sizeX, sizeZ, height, pos, ang, s)) {
        mvbg->mNext = mHead;
        mHead = mvbg;
        return TRUE;
    }
    return FALSE;
}

// 80072518
BOOL mvbgList_c::remove(mvbg_c *mvbg) {
    mvbg_c *cur = mHead;
    mvbg_c *prev = NULL;
    while (cur != NULL) {
        if (cur == mvbg) {
            if (prev != NULL) {
                prev->mNext = cur->mNext;
            } else {
                mHead = cur->mNext;
            }
            mvbg->init();
            return TRUE;
        }
        prev = cur;
        cur = cur->mNext;
    }
    return FALSE;
}

static inline f32 absF(f32 v) {
    if (v < 0.0f) {
        v = -v;
    }
    return v;
}

static inline int outCode(const mVec3_c *p, const mVec3_c &min, const mVec3_c &max) {
    int code = 0;
    f32 x = p->x;
    f32 minX = min.x;
    f32 maxX = max.x;
    if (x < minX) {
        code |= 1;
    } else if (x > maxX) {
        code |= 2;
    }
    f32 z = p->z;
    f32 minZ = min.z;
    f32 maxZ = max.z;
    if (z < minZ) {
        code |= 4;
    } else if (z > maxZ) {
        code |= 8;
    }
    return code;
}

// 80072588
BOOL mvbgList_c::isOutside(const mVec3_c &center, const mVec3_c &half, const mVec3_c *a, const mVec3_c *b) {
    const mVec3_c *pts[2] = {a, b};
    mVec3_c min = center - half;
    mVec3_c max = center + half;
    u32 code = 0xFF;
    for (int i = 0; i < 2; i++) {
        const mVec3_c *p = pts[i];
        int c = outCode(p, min, max);
        if (c == 0) {
            return FALSE;
        }
        code &= c;
    }
    if (code == 1) {
        return TRUE;
    }
    if (code == 2) {
        return TRUE;
    }
    if (code == 4) {
        return TRUE;
    }
    return code == 8;
}

// 800726E0
void mvbgList_c::makeGeometry(const mVec3_c &center, const mVec3_c &half, wallList_c *walls, floorList_c *floors,
                              u32 flags, BOOL below) {
    BOOL doWalls = flags & CHECK_WALL;
    BOOL doFloors = flags & CHECK_FLOOR;
    for (mvbg_c *mvbg = mHead; mvbg != NULL; mvbg = mvbg->mNext) {
        BOOL skip = FALSE;
        if (!below) {
            if (mvbg->mPos.y <= 0.0f) {
                skip = TRUE;
            }
        } else if (mvbg->mPos.y > 0.0f) {
            skip = TRUE;
        }
        if (skip) {
            continue;
        }
        wallDat_s dat;
        dat.mNoFace = 0;
        mVec3_c c(mvbg->mCenter);
        mVec3_c d = c - center;
        if (!(absF(d.x) < mvbg->mHalf.x + half.x)) {
            continue;
        }
        if (!(absF(d.z) < mvbg->mHalf.z + half.z)) {
            continue;
        }
        if (doWalls) {
            u8 out[4];
            u8 *o = out;
            for (int n = 0; n < mvbg->mPointNum; n++) {
                *o++ = isOutside(center, half, &mvbg->getPoint(n), &mvbg->getPoint(n + 1));
            }
            f32 top = mvbg->mPos.y + mvbg->mHeight * mvbg->mScale.y;
            for (int n = 0; n < mvbg->mPointNum; n++) {
                if (out[n & 3] == 0) {
                    const mVec3_c &p0 = mvbg->getPoint(n);
                    dBGC::vecXZ_c a(p0.x, p0.z);
                    const mVec3_c &p1 = mvbg->getPoint(n + 1);
                    dBGC::vecXZ_c b(p1.x, p1.z);
                    dat.mStart.set(a);
                    dat.mY0 = top;
                    dat.mEnd.set(b);
                    dat.mY1 = top;
                    dat.mType = WALL_TYPE_NORMAL;
                    dat.mAttr = BG_ATTR_OBJ;
                    walls->add(dat, mvbg->getNormal(n), mvbg);
                }
            }
        }
        if (doFloors) {
            floors->add(mvbg->getPoint(0), mvbg->getPoint(1), mvbg->getPoint(3), BG_ATTR_OBJ, mvbg);
            floors->add(mvbg->getPoint(1), mvbg->getPoint(2), mvbg->getPoint(3), BG_ATTR_OBJ, mvbg);
        }
    }
}

// 80072A00
BOOL isLoaded() {
    return l_mgr.mBg[l_mgr.mCurrent].mLoaded != 0;
}

// 80072A28
void vecMax(mVec3_c *a, const mVec3_c &b) {
    if (b.x > a->x) {
        a->x = b.x;
    }
    if (b.y > a->y) {
        a->y = b.y;
    }
    if (b.z > a->z) {
        a->z = b.z;
    }
}

// 80072A68
void vecMin(mVec3_c *a, const mVec3_c &b) {
    if (b.x < a->x) {
        a->x = b.x;
    }
    if (b.y < a->y) {
        a->y = b.y;
    }
    if (b.z < a->z) {
        a->z = b.z;
    }
}

// 80072AA8
unitDat_c *getBlockData(int blockX, int blockZ) {
    if (sCurBg != NULL) {
        blockInfo_c *block = sCurBg->getBlock(blockX, blockZ);
        if (block != NULL) {
            return block->mData;
        }
        return NULL;
    }
    return NULL;
}

// 80072B00
unitDat_c *getUnitDat(int unitX, int unitZ) {
    int blockX, blockZ;
    unitToBlock(&blockX, &blockZ, unitX, unitZ);
    unitDat_c *data = getBlockData(blockX, blockZ);
    if (data != NULL) {
        return getUnitDat(data, unitX, unitZ);
    }
    return &l_mgr.mDefault;
}

// 80072B7C
unitDat_c *getUnitDat(const mVec3_c *pos) {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    return getUnitDat(unitX, unitZ);
}

// 80072BB4
int getUnitAttr(int unitX, int unitZ) {
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        return dat->mAttr;
    }
    return 0;
}

// 80072BE8
clmcb_c *getClmcb() {
    static clmcb_c sDefault;
    if (sCurBg != NULL) {
        clmcb_c *cb = sCurBg->mCallback;
        if (cb == NULL) {
            cb = &sDefault;
        }
        return cb;
    }
    return &sDefault;
}

// 80072C60
BOOL isWaterAttr(int attr, int mode) {
    if (getWater(attr) != BG_WATER_NONE) {
        switch (mode) {
        case 1:
            if (attr == BG_ATTR_WAVE) {
                return FALSE;
            }
            break;
        case 2:
            if (attr == BG_ATTR_WAVE) {
                return FALSE;
            }
            break;
        }
        return TRUE;
    }
    return FALSE;
}

// 80072CD8
BOOL isWaterToLand(const mVec3_c *from, const mVec3_c *to, acch_c *acch, int arg) {
    groundChk_c chkFrom(from, LAYER_TOP, 1, 0);
    if (chkFrom.mWater != BG_WATER_NONE) {
        groundChk_c chkTo(to, LAYER_TOP, 1, 0);
        if (chkTo.mWater == BG_WATER_NONE) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80072D54
int getRouteDirs(int unitX, int unitZ) {
    int dirs = 0;
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat == NULL) {
        return 0;
    }
    if (!dat->isRoute()) {
        return 0;
    }
    dat = getUnitDat(unitX, unitZ - 1);
    if (dat != NULL && dat->isRoute()) {
        dirs |= 1 << QUARTER_NEG_Z;
    }
    dat = getUnitDat(unitX - 1, unitZ);
    if (dat != NULL && dat->isRoute()) {
        dirs |= 1 << QUARTER_NEG_X;
    }
    dat = getUnitDat(unitX, unitZ + 1);
    if (dat != NULL && dat->isRoute()) {
        dirs |= 1 << QUARTER_POS_Z;
    }
    dat = getUnitDat(unitX + 1, unitZ);
    if (dat != NULL && dat->isRoute()) {
        dirs |= 1 << QUARTER_POS_X;
    }
    return dirs;
}

// 80072E50
void setRoute(int unitX, int unitZ, int on) {
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        dat->mFlags = (on << 7) | (dat->mFlags & 0x7F);
    }
}

// 80072E94
int getUnitQuarter(const mVec3_c *pos) {
    mVec3_c p(*pos);
    mVec3_c center;
    snapToUnit(&center, &p);
    p -= center;
    f32 a = p.x + p.z;
    f32 b = -p.x + p.z;
    if (a > 0.0f) {
        if (b > 0.0f) {
            return QUARTER_POS_Z;
        }
        return QUARTER_POS_X;
    }
    if (b > 0.0f) {
        return QUARTER_NEG_X;
    }
    return QUARTER_NEG_Z;
}

// 80072F48
int isFlat(const mVec3_c *pos) {
    unitDat_c *dat = getUnitDat(pos);
    if (dat != NULL) {
        return dat->isFlat();
    }
    return 0;
}

// 80072F80
int getAttr(int unitX, int unitZ) {
    return getUnitAttr(unitX, unitZ);
}

// 80072F84
int getGroundAttr(const mVec3_c *pos, int type) {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    if (l_mgr.mBg[l_mgr.mCurrent].mUnitFlags.check(unitX, unitZ)) {
        f32 y = getUnitY(pos);
        if (y != getGroundY(pos, FALSE)) {
            return BG_ATTR_STONE;
        }
    }
    int quarter = getUnitQuarter(pos);
    return sGetTriAttr[type](getUnitAttr(unitX, unitZ), quarter);
}

// 80073054
int getWaterKind(int unitX, int unitZ) {
    if (isLoaded()) {
        int attr = getUnitAttr(unitX, unitZ);
        if (attr < BG_ATTR_NUM) {
            return getAttrData(attr)->getWater();
        }
        return BG_WATER_NONE;
    }
    return BG_WATER_NONE;
}

// 800730D0
int isRiver(int unitX, int unitZ) {
    int attr = getUnitAttr(unitX, unitZ);
    if (attr < BG_ATTR_NUM) {
        return getAttrData(attr)->isRiver();
    }
    return 0;
}

// 80073114
int isSea(int unitX, int unitZ) {
    int attr = getUnitAttr(unitX, unitZ);
    if (attr < BG_ATTR_NUM) {
        return getAttrData(attr)->isSea();
    }
    return 0;
}

// 80073158
int canPutItem(int unitX, int unitZ) {
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        int attr = dat->mAttr;
        if (attr < BG_ATTR_NUM) {
            return getAttrData(attr)->canPut();
        }
        return 0;
    }
    return 0;
}

// 800731B0
int canPutItemNoBridge(int unitX, int unitZ) {
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        int attr = dat->mAttr;
        if (attr < BG_ATTR_NUM) {
            return getAttrData(attr)->canPutNoBridge();
        }
        return 0;
    }
    return 0;
}

// 80073208
int isGrassGround(int unitX, int unitZ) {
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        int attr = dat->mAttr;
        if (attr < BG_ATTR_NUM) {
            return getAttrData(attr)->isGrass();
        }
        return 0;
    }
    return 0;
}

// 80073260
int canNpcPutItem(int unitX, int unitZ) {
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        int attr = dat->mAttr;
        if (attr < BG_ATTR_NUM) {
            return getAttrData(attr)->canNpcPut();
        }
        return 0;
    }
    return 0;
}

// 800732B8
int getDigType(int unitX, int unitZ) {
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        int attr = dat->mAttr;
        if (attr < BG_ATTR_NUM) {
            return getAttrData(attr)->getDig();
        }
        return BG_DIG_OTHER;
    }
    return BG_DIG_OTHER;
}

// 80073314
int getPlantType(int unitX, int unitZ) {
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        int attr = dat->mAttr;
        if (attr < BG_ATTR_NUM) {
            return getAttrData(attr)->getPlant();
        }
        return BG_PLANT_NONE;
    }
    return BG_PLANT_NONE;
}

// 80073370
int getGrassMin(int unitX, int unitZ) {
    int attr = getUnitAttr(unitX, unitZ);
    if (attr < BG_ATTR_NUM) {
        return getAttrData(attr)->mGrassMin;
    }
    return 0;
}

// 800733B0
int getGrassMax(int unitX, int unitZ) {
    int attr = getUnitAttr(unitX, unitZ);
    if (attr < BG_ATTR_NUM) {
        return getAttrData(attr)->mGrassMax;
    }
    return 0xFF;
}

// 800733F0
int isBeachGround(int unitX, int unitZ) {
    int attr = getUnitAttr(unitX, unitZ);
    if (attr < BG_ATTR_NUM) {
        return getAttrData(attr)->isBeach();
    }
    return 0;
}

// 80073434
BOOL getMapColors(u8 *out, int unitX, int unitZ) {
    int attr = getUnitAttr(unitX, unitZ);
    if (attr < BG_ATTR_NUM) {
        out[0] = sAttrTable[attr].mMapColor[0];
        out[1] = sAttrTable[attr].mMapColor[1];
        out[2] = sAttrTable[attr].mMapColor[2];
        out[3] = sAttrTable[attr].mMapColor[3];
        out[4] = sAttrTable[attr].mMapColor[4];
        out[5] = sAttrTable[attr].mMapColor[5];
        out[6] = sAttrTable[attr].mMapColor[6];
        out[7] = sAttrTable[attr].mMapColor[7];
        out[8] = sAttrTable[attr].mMapColor[8];
        return TRUE;
    }
    return FALSE;
}

// 80073510
f32 getUnitY(const mVec3_c *pos) {
    int cur = l_mgr.mCurrent;
    if (cur == 1 && isCurrentSceneAttr(5)) {
        setCurrentBg(0);
    }
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    int blockX, blockZ;
    unitToBlock(&blockX, &blockZ, unitX, unitZ);
    blockInfo_c *block = sCurBg->getBlock(blockX, blockZ);
    f32 y = block != NULL ? block->mY : 0.0f;
    if (l_mgr.mBg[l_mgr.mCurrent].mUnitFlags.check(unitX, unitZ)) {
        unitDat_c *dat = getUnitDat(unitX, unitZ);
        if (dat != NULL) {
            f32 h = 7.0f * dat->getHeight();
            setCurrentBg(cur);
            return y + h;
        }
        setCurrentBg(cur);
        return 0.0f;
    }
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        int quarter = getUnitQuarter(pos);
        mVec3_c base(32.0f * unitX, y, 32.0f * unitZ);
        mVec3_c tri[3];
        switch (quarter) {
        case QUARTER_NEG_Z:
            dat->calcTri0(tri, dat, base);
            break;
        case QUARTER_NEG_X:
            dat->calcTri1(tri, dat, base);
            break;
        case QUARTER_POS_Z:
            dat->calcTri2(tri, dat, base);
            break;
        default:
            dat->calcTri3(tri, dat, base);
            break;
        }
        dBGC::poly_c poly(tri[0], tri[1], tri[2]);
        setCurrentBg(cur);
        return poly.calcY(*pos);
    }
    setCurrentBg(cur);
    return 0.0f;
}

// 80073798
int getFootstepSe(int attr) {
    if (attr < BG_ATTR_NUM) {
        return sAttrSe[attr][0];
    }
    return -1;
}

// 800737BC
int getFtrSe(int attr) {
    if (attr < BG_ATTR_NUM) {
        return sAttrSe[attr][1];
    }
    return -1;
}

static inline int absI(int v) {
    return v < 0 ? -v : v;
}

// 800737E4
BOOL checkWalkable(int unitX, int unitZ, int dir, int toX, int toZ) {
    mVec3_c pos;
    unitToPos(&pos, unitX, unitZ);
    switch (dir & 3) {
    case QUARTER_NEG_Z:
        pos.z -= 8.0f;
        break;
    case QUARTER_NEG_X:
        pos.x -= 8.0f;
        break;
    case QUARTER_POS_Z:
        pos.z += 8.0f;
        break;
    default:
        pos.x += 8.0f;
        break;
    }
    groundChk_c chk(&pos, LAYER_TOP, 0, 0);
    pos.y = chk.getHeight(FALSE);
    int x, z;
    posToUnit(&x, &z, &pos);
    int dx = absI(x - toX);
    if (dx > 1) {
        return FALSE;
    }
    int dz = absI(z - toZ);
    if (dz > 1) {
        return FALSE;
    }
    int dist = dx + dz;
    mVec3_c from;
    mVec3_c to;
    if (chk.isContinuous()) {
        unitToPos(&from, x, z);
        to.y = getUnitBaseY(x, z);
    } else {
        from = pos;
    }
    unitToPos(&to, toX, toZ);
    to.y = getUnitBaseY(toX, toZ);
    if (dist == 0) {
        acch_c acch;
        acch.check(3.2f, &to, &from, 0, 0, CHECK_FLOOR | CHECK_WALL | CHECK_STEP_EXT_48 | CHECK_SET_POS, TRUE);
        if (acch.isWallHit() || acch.isStepWallHit()) {
            return FALSE;
        }
        return TRUE;
    }
    if (dist == 1) {
        acch_c acch;
        mVec3_c p(to);
        acch.check(3.2f, &p, &from, 0, 0, CHECK_FLOOR | CHECK_WALL | CHECK_STEP_EXT_48 | CHECK_SET_POS, TRUE);
        if (acch.isWallHit() || acch.isStepWallHit()) {
            return FALSE;
        }
        return TRUE;
    }
    if (dist == 2) {
        acch_c acch1;
        acch_c acch2;
        mVec3_c mid;
        unitToPos(&mid, toX, z);
        mid.y = getUnitBaseY(toX, z);
        mVec3_c p0(mid);
        acch1.check(3.2f, &p0, &from, 0, 0, CHECK_FLOOR | CHECK_WALL | CHECK_STEP_EXT_48 | CHECK_SET_POS, TRUE);
        if (!(acch1.isWallHit() || acch1.isStepWallHit())) {
            p0 = mid;
            mVec3_c p1(to);
            acch2.check(3.2f, &p1, &p0, 0, 0, CHECK_FLOOR | CHECK_WALL | CHECK_STEP_EXT_48 | CHECK_SET_POS, TRUE);
            if (!(acch2.isWallHit() || acch2.isStepWallHit())) {
                return TRUE;
            }
        }
        unitToPos(&mid, x, toZ);
        mid.y = getUnitBaseY(x, toZ);
        mVec3_c p3(mid);
        acch1.check(3.2f, &p3, &from, 0, 0, CHECK_FLOOR | CHECK_WALL | CHECK_STEP_EXT_48 | CHECK_SET_POS, TRUE);
        if (!(acch1.isWallHit() || acch1.isStepWallHit())) {
            p3 = mid;
            mVec3_c p4(to);
            mVec3_c p6(to);
            acch2.check(3.2f, &p4, &p3, 0, 0, CHECK_FLOOR | CHECK_WALL | CHECK_STEP_EXT_48 | CHECK_SET_POS, TRUE);
            if (!(acch2.isWallHit() || acch2.isStepWallHit())) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80073C94
BOOL checkPut(int unitX, int unitZ, int dir, int toX, int toZ, BOOL noBridge) {
    if (noBridge) {
        if (!canPutItemNoBridge(toX, toZ)) {
            return FALSE;
        }
    } else if (!canPutItem(toX, toZ)) {
        return FALSE;
    }
    return checkWalkable(unitX, unitZ, dir, toX, toZ);
}

// 80073D2C
BOOL checkPut(const mVec3_c *pos, int toX, int toZ, BOOL noBridge) {
    int dir = getUnitQuarter(pos);
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    return checkPut(unitX, unitZ, dir, toX, toZ, noBridge);
}

// 80073D9C
BOOL isWaterArea(const mVec3_c *pos, f32 *waterY, int mode, f32 r) {
    groundChk_c chk(pos, LAYER_TOP, mode, 0);
    if (chk.mWater != BG_WATER_NONE) {
        mVec3_c ofs[8] = {
            mVec3_c(-r, 0.0f, -r), mVec3_c(-r, 0.0f, r),   mVec3_c(r, 0.0f, r),   mVec3_c(r, 0.0f, -r),
            mVec3_c(-r, 0.0f, 0.0f), mVec3_c(r, 0.0f, 0.0f), mVec3_c(0.0f, 0.0f, r), mVec3_c(0.0f, 0.0f, -r),
        };
        f32 y;
        for (int i = 0; i < 8; i++) {
            mVec3_c p = *pos + ofs[i];
            groundChk_c chk2(&p, LAYER_TOP, mode, 0);
            if (chk2.mWater == BG_WATER_NONE) {
                return FALSE;
            }
            y = chk.mWaterY;
        }
        if (waterY != NULL) {
            *waterY = y;
        }
        return TRUE;
    }
    return FALSE;
}

// 80073EE4
BOOL findWaterPos(mVec3_c *out, const mVec3_c *pos, mAng ang, u32 num, int mode, f32 dist, f32 r) {
    if (num >= 1) {
        f32 step = dist / num;
        mVec3_c dir(0.0f, 0.0f, 1.0f);
        dir.rotY(ang);
        for (int i = num; i >= 0; i--) {
            f32 d = i * step;
            mVec3_c p(pos->x + dir.x * d, pos->y + dir.y * d, pos->z + dir.z * d);
            f32 y;
            if (isWaterArea(&p, &y, mode, r)) {
                *out = p;
                out->y = y;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80074050
BOOL isWaterArea(const mVec3_c *pos, f32 r) {
    return isWaterArea(pos, NULL, 2, r);
}

inline f32 distXZSq(const mVec3_c &a, const mVec3_c &b) {
    mVec3_c d = a - b;
    return d.x * d.x + d.z * d.z;
}

inline f32 distXZ(const mVec3_c &a, const mVec3_c &b) {
    return EGG::Mathf::sqrt(distXZSq(a, b));
}

// 8007405C
void acch_c::checkSteps(f32 r, mVec3_c *pos, const mVec3_c *old, mAng ang, int arg, u32 flags) {
    BOOL first = FALSE;
    f32 len = distXZ(*pos, *old);
    if (len >= 16.0f) {
        u32 num = (u32)(len / 14.4f) + 1;
        mVec3_c step = *pos - *old;
        f32 s = 1.0f / num;
        step.x *= s;
        step.y *= s;
        step.z *= s;
        mVec3_c cur(*old);
        for (u32 i = 0; i < num; i++) {
            mVec3_c prev(cur);
            cur = prev + step;
            check(r, &cur, &prev, ang, arg, flags, !first);
            first = TRUE;
        }
        *pos = cur;
    } else {
        check(r, pos, old, ang, arg, flags, TRUE);
    }
}

// 80074234
void acch_c::check(f32 r, mVec3_c *pos, const mVec3_c *old, mAng ang, int arg, u32 flags, BOOL doClear) {
    mVec3_c p(*pos);
    mVec3_c o(*old);
    f32 ext = 2.0f * (l_box.x + r);
    if (ext + EGG::Mathf::abs(p.x - old->x) > 192.0f || ext + EGG::Mathf::abs(p.z - old->z) > 192.0f) {
        o = p;
    }
    mVec3_c rr(r, r, r);
    mVec3_c max(o);
    mVec3_c min(o);
    vecMax(&max, p);
    vecMin(&min, p);
    max += rr;
    min -= rr;
    dtcbCorrect_c cb(this, &p, o, ang, r, arg, flags);
    if (doClear) {
        clear();
    }
    dBGCF::check(&min, &max, &cb, flags, 0, (mPrevHitFlags >> 1) & 1);
    if (flags & CHECK_FLOOR) {
        groundChk_c chk(&p, (mPrevHitFlags >> 1) & 1, (flags >> 8) & 1, 0);
        if (p.y <= chk.getHeight(FALSE)) {
            mHitFlags |= HIT_GROUND;
            mGroundAttr = chk.mAttr;
            p.y = chk.getHeight(FALSE);
        } else {
            if (p.y <= chk.getHeight(FALSE) + getStepHeight()) {
                mHitFlags |= HIT_GROUND;
                mGroundAttr = chk.mAttr;
            }
        }
        if (chk.isUnderWater(p.y)) {
            mHitFlags |= HIT_UNDER_WATER;
            if (chk.mSlope) {
                mHitFlags |= HIT_WATER_SLOPE;
            }
        } else if (chk.mWater != BG_WATER_NONE) {
            mHitFlags |= HIT_WATER;
            if (chk.mSlope) {
                mHitFlags |= HIT_WATER_SLOPE;
            }
        }
        mGroundNormal = chk.mFloor.mNormal;
    }
    calcWallFlags(ang);
    mPushOut = p - *pos;
    if (flags & CHECK_STEP_EXT_48) {
        if (isWaterToLand(&p, old, this, arg)) {
            p = *old;
        }
    }
    if (flags & CHECK_SET_POS) {
        *pos = p;
    }
    _00 = 0;
}

// 80074630
BOOL checkLineSteps(lineChk_s *out, mVec3_c *end, const mVec3_c *start, u32 flags) {
    u32 num = (u32)(distXZ(*end, *start) / 48.0f);
    if (num == 0) {
        return checkLine(out, end, start, flags);
    }
    mVec3_c step = *end - *start;
    f32 s = 1.0f / num;
    step.x *= s;
    step.y *= s;
    step.z *= s;
    for (int i = 0; i < num; i++) {
        f32 t0 = i;
        f32 t1 = 0.1f + (i + 1);
        mVec3_c a(start->x + step.x * t0, start->y + step.y * t0, start->z + step.z * t0);
        mVec3_c b(start->x + step.x * t1, start->y + step.y * t1, start->z + step.z * t1);
        if (checkLine(out, &b, &a, flags)) {
            *end = b;
            return TRUE;
        }
    }
    return FALSE;
}

// 8007481C
BOOL checkLine(lineChk_s *out, mVec3_c *end, const mVec3_c *start, u32 flags) {
    mVec3_c e(*end);
    mVec3_c min(*start);
    mVec3_c max(*start);
    vecMin(&min, e);
    vecMax(&max, e);
    dtcbLineChk_c cb(&e, *start, flags);
    check(&min, &max, &cb, flags, 1, LAYER_TOP);
    if (flags & CHECK_SET_POS) {
        *end = e;
    }
    out->mNormal = cb.mNormal;
    out->mDat.set(cb.mDat);
    return cb.mHit;
}

// 80074974
f32 getGroundY(const mVec3_c *pos, BOOL withColumn) {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    if (isFlat(pos)) {
        f32 y = getUnitBaseY(pos);
        if (!withColumn) {
            return y;
        }
        posToUnit(&unitX, &unitZ, pos);
        f32 radius, height;
        int attr;
        if (getColumnAttr(unitX, unitZ, &radius, &height, &attr) && attr != BG_ATTR_ATTRW) {
            mVec3_c center;
            unitToPos(&center, unitX, unitZ);
            if (distXZSq(*pos, center) <= radius * radius) {
                return y + height;
            }
        }
        return y;
    }
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL && sCurBg != NULL) {
        int blockX, blockZ;
        unitToBlock(&blockX, &blockZ, unitX, unitZ);
        blockInfo_c *block = sCurBg->getBlock(blockX, blockZ);
        f32 by = block != NULL ? block->mY : 0.0f;
        int quarter = getUnitQuarter(pos);
        mVec3_c base(32.0f * unitX, by, 32.0f * unitZ);
        mVec3_c tri[3];
        switch (quarter) {
        case QUARTER_NEG_Z:
            dat->calcTri0(tri, dat, base);
            break;
        case QUARTER_NEG_X:
            dat->calcTri1(tri, dat, base);
            break;
        case QUARTER_POS_Z:
            dat->calcTri2(tri, dat, base);
            break;
        default:
            dat->calcTri3(tri, dat, base);
            break;
        }
        dBGC::poly_c poly(tri[0], tri[1], tri[2]);
        f32 y = poly.calcY(*pos);
        if (!withColumn) {
            return y;
        }
        f32 radius, height;
        int attr;
        if (getColumnAttr(unitX, unitZ, &radius, &height, &attr) && attr != BG_ATTR_ATTRW) {
            mVec3_c center;
            unitToPos(&center, unitX, unitZ);
            if (distXZSq(*pos, center) <= radius * radius) {
                return y + height;
            }
        }
        return y;
    }
    return 0.0f;
}

// 80074C8C
f32 getGroundY(const mVec3_c *pos, int *attr, u32 flags) {
    lineChk_s res;
    checkFlags_e lineFlags = (checkFlags_e)(flags & ~CHECK_WALLS);
    mVec3_c start(*pos);
    mVec3_c end(*pos);
    start.y = 3200.0f;
    end.y = -start.y;
    if (checkLine(&res, &end, &start, lineFlags)) {
        if (attr != NULL) {
            *attr = res.mDat.mAttr;
        }
        return end.y;
    }
    groundChk_c chk(pos, LAYER_TOP, 0, 0);
    if (attr != NULL) {
        *attr = chk.mAttr;
    }
    return chk.getHeight(FALSE);
}

// 80074D64
f32 getUnitBaseY(int unitX, int unitZ) {
    if (sCurBg != NULL) {
        int blockX, blockZ;
        unitToBlock(&blockX, &blockZ, unitX, unitZ);
        blockInfo_c *block = sCurBg->getBlock(blockX, blockZ);
        f32 y = block != NULL ? block->mY : 0.0f;
        unitDat_c *dat = getUnitDat(unitX, unitZ);
        if (dat != NULL) {
            f32 base = 7.0f * dat->getHeight();
            return y + base + dat->getY0(2);
        }
    }
    return 0.0f;
}

// 80074E5C
f32 getUnitBaseY(const mVec3_c *pos) {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    return getUnitBaseY(unitX, unitZ);
}

// 80074E94
f32 getEdgeGroundY(const mVec3_c *pos) {
    if (sCurBg != NULL) {
        int unitX, unitZ;
        posToUnit(&unitX, &unitZ, pos);
        int blockX, blockZ;
        unitToBlock(&blockX, &blockZ, unitX, unitZ);
        if (blockX <= 0) {
            mVec3_c p;
            unitToPos(&p, 16, unitZ);
            return getUnitY(&p);
        }
        if (blockX >= sCurBg->mBlockW - 1) {
            mVec3_c p;
            unitToPos(&p, (sCurBg->mBlockW - 1) * 16 - 1, unitZ);
            return getUnitY(&p);
        }
        groundChk_c chk(pos, LAYER_TOP, 0, 0);
        if (chk.mWater != BG_WATER_NONE) {
            return 28.0f + chk.getHeight(FALSE);
        }
        return getUnitY(pos);
    }
    return 0.0f;
}

// 80074F94
f32 getUnitMinY(int unitX, int unitZ) {
    if (sCurBg == NULL) {
        return 0.0f;
    }
    int blockX, blockZ;
    unitToBlock(&blockX, &blockZ, unitX, unitZ);
    blockInfo_c *block = sCurBg->getBlock(blockX, blockZ);
    f32 y = block != NULL ? block->mY : 0.0f;
    unitDat_c *dat = getUnitDat(unitX, unitZ);
    if (dat != NULL) {
        f32 h2 = dat->getY0(2);
        f32 h1 = dat->getY0(1);
        f32 h0 = dat->getY0(0);
        f32 h3 = dat->getY0(3);
        f32 min = 0.0f;
        if (h2 < min) {
            min = h2;
        }
        if (h1 < min) {
            min = h1;
        }
        if (h0 < min) {
            min = h0;
        }
        if (h3 < min) {
            min = h3;
        }
        return min + (y + 7.0f * dat->getHeight());
    }
    return 0.0f;
}

// 80075114
BOOL getColumnAttr(int x, int z, f32 *radius, f32 *height, int *attr) {
    if (sCurBg != NULL && sCurBg->mCallback != NULL) {
        f32 r, h;
        int a;
        if (sCurBg->mCallback->getAttr(&r, &h, &a, x, z)) {
            if (radius != NULL) {
                *radius = r;
            }
            if (height != NULL) {
                *height = h;
            }
            if (attr != NULL) {
                *attr = a;
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 800751D8
BOOL checkColumn(const mVec3_c *pos, f32 *radius, f32 *height, int *attr) {
    int unitX, unitZ;
    f32 r = 0.0f;
    f32 h = 0.0f;
    int a = 0;
    posToUnit(&unitX, &unitZ, pos);
    if (radius == NULL) {
        radius = &r;
    }
    if (height == NULL) {
        height = &h;
    }
    if (attr == NULL) {
        attr = &a;
    }
    if (getColumnAttr(unitX, unitZ, radius, height, attr)) {
        mVec3_c center;
        unitToPos(&center, unitX, unitZ);
        return distXZSq(*pos, center) <= *radius * *radius;
    }
    return FALSE;
}

// 800752E0
void fn_800752E0() {}

// 800752E4
void fn_800752E4() {}

// 800752E8
void check(const mVec3_c *min, const mVec3_c *max, dtcb_c *cb, u32 flags, int mode, int type) {
    BOOL anm = (flags & CHECK_STEP_EXT) != 0;
    anm = (flags & CHECK_NO_ANM) ? FALSE : anm;
    mVec3_c lo = *min - l_box;
    mVec3_c hi = *max + l_box;
    int unitMin[2];
    posToUnit(&unitMin[0], &unitMin[1], &lo);
    int unitMax[2];
    posToUnit(&unitMax[0], &unitMax[1], &hi);
    mVec3_c center = lo + hi;
    center.x *= 0.5f;
    center.y *= 0.5f;
    center.z *= 0.5f;
    mVec3_c half = hi - lo;
    half.x *= 0.5f;
    half.y *= 0.5f;
    half.z *= 0.5f;
    l_mgr.mColumns.mNum = 0;
    l_mgr.mWalls.mNum = 0;
    l_mgr.mFloors.mNum = 0;
    l_mgr.mGrid.make(unitMin, unitMax, mode, type);
    if ((flags & (CHECK_FLOOR | CHECK_WALLS)) && !(flags & CHECK_NO_COLUMNS)) {
        l_mgr.mColumns.make(getClmcb(), unitMin, unitMax, anm);
    }
    if (!(flags & CHECK_NO_MVBG)) {
        l_mgr.mMvbgs.makeGeometry(center, half, &l_mgr.mWalls, &l_mgr.mFloors, flags, FALSE);
        l_mgr.mMvbgs.makeGeometry(center, half, &l_mgr.mWalls, &l_mgr.mFloors, flags, TRUE);
    }
    if (flags & CHECK_WALLS) {
        if (!(flags & CHECK_NO_COLUMN_WALLS)) {
            l_mgr.mColumns.makeWalls(&l_mgr.mWalls, flags, getClmcb());
        }
        l_mgr.mWalls.makeSteps(&l_mgr.mGrid, unitMin, unitMax, flags);
    }
    if (mode != 0 && flags != 0) {
        l_mgr.mFloors.make(&l_mgr.mGrid, unitMin, unitMax);
    }
    cb->checkColumn(&l_mgr.mColumns);
    cb->checkWall(&l_mgr.mWalls);
    cb->checkFloor(&l_mgr.mFloors);
}

// 800755A0
void setCurrentBg(int idx) {
    if (!l_mgr.setCurrent(idx)) {
        if (idx < 0) {
            l_mgr.setCurrent(0);
            sCurBg = &l_mgr.mBg[l_mgr.mCurrent];
        } else {
            l_mgr.setCurrent(15);
            sCurBg = &l_mgr.mBg[l_mgr.mCurrent];
        }
    } else {
        sCurBg = &l_mgr.mBg[l_mgr.mCurrent];
    }
}

// 80075658
BOOL entryBg(u32 blockW, u32 blockH, clmcb_c *cb, int idx) {
    bg_c *bg = &l_mgr.mBg[idx];
    BOOL ok = TRUE;
    resetBg(idx);
    if (blockW <= 7) {
        bg->mBlockW = (u8)blockW;
    } else {
        ok = FALSE;
    }
    if (blockH <= 7) {
        bg->mBlockH = (u8)blockH;
    } else {
        ok = FALSE;
    }
    bg->mCallback = cb;
    bg->mLoaded = 0;
    setCurrentBg(0);
    return ok;
}

// 800756F4
BOOL setBlock(int blockX, int blockZ, unitDat_c *data, u32 y, int idx) {
    blockInfo_c *block = l_mgr.mBg[idx].getBlock(blockX, blockZ);
    if (block != NULL) {
        block->set(data, y);
        l_mgr.mBg[idx].mLoaded = l_mgr.mBg[idx].isLoaded();
        return TRUE;
    }
    return FALSE;
}

// 80075780
void resetBg(int idx) {
    bg_c *bg = &l_mgr.mBg[idx];
    bg->init();
    if (idx == 0) {
        l_mgr.mWalls.mNum = 0;
        l_mgr.mFloors.mNum = 0;
        l_mgr.mColumns.mNum = 0;
        l_mgr.mMvbgs.clear();
        l_mgr.mUnitAnm.clear();
        sCurBg = bg;
    }
}

// 80075808
void clearBlocks(int idx) {
    bg_c *bg = &l_mgr.mBg[idx];
    for (u32 z = 0; z < bg->mBlockH; z++) {
        for (u32 x = 0; x < bg->mBlockW; x++) {
            blockInfo_c *block = bg->getBlock(x, z);
            if (block != NULL) {
                block->clear();
            }
        }
    }
}

// 8007589C
void releaseBg(int idx) {
    clearBlocks(idx);
    resetBg(idx);
    if (idx == 0) {
        sCurBg = NULL;
    }
}

// 800758E0
BOOL setUnitAnm(int x, int z, u32 time, f32 to, f32 from) {
    return l_mgr.mUnitAnm.set(x, z, time, to, from);
}

// 80075908
void setBgY(f32 y) {
    if (sCurBg != NULL && sCurBg->mY != y) {
        sCurBg->mPrevY = sCurBg->mY;
        sCurBg->mY = y;
    }
}

// 80075930
int getBlockUnitAttr(unitDat_c *data, int x, int z) {
    return getUnitDat(data, x, z)->mAttr;
}

// 80075954
void copyUnits(unitDat_c *src, int unitX, int unitZ, copyChk_c *cb) {
    if (sCurBg == NULL) {
        return;
    }
    unitDat_c *s = src;
    for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
            int ux = unitX + x;
            int uz = unitZ + z;
            u8 copyTri, copyAttr, copyRoute;
            cb->check(getBlockUnitAttr(src, x, z), &copyTri, &copyAttr, &copyRoute);
            unitDat_c *dat = getUnitDat(ux, uz);
            if (dat != NULL) {
                if (copyTri) {
                    dat->mTri[0] = s->mTri[0];
                    dat->mTri[1] = s->mTri[1];
                    dat->mTri[2] = s->mTri[2];
                    dat->mTri[3] = s->mTri[3];
                    dat->mFlags = (s->mFlags & UNIT_CONTINUOUS) | (dat->mFlags & 0xFE);
                    dat->mFlags = (s->mFlags & UNIT_FLAT) | (dat->mFlags & ~UNIT_FLAT);
                    sCurBg->mUnitFlags.on(ux, uz);
                }
                if (copyAttr) {
                    dat->mAttr = s->mAttr;
                }
                if (copyRoute) {
                    dat->mFlags = (s->mFlags & UNIT_ROUTE) | (dat->mFlags & 0x7F);
                }
            }
            s++;
        }
    }
}

// 80075AB0
void flattenUnits(unitDat_c *src, int unitX, int unitZ, flatChk_c *cb) {
    if (sCurBg == NULL) {
        return;
    }
    for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
            int ux = unitX + x;
            int uz = unitZ + z;
            if (cb->check(getBlockUnitAttr(src, x, z))) {
                unitDat_c *dat = getUnitDat(ux, uz);
                if (dat != NULL) {
                    dat->mTri[0].clear();
                    dat->mTri[1].clear();
                    dat->mTri[2].clear();
                    dat->mTri[3].clear();
                    dat->mFlags = (dat->mFlags & ~UNIT_FLAT) | UNIT_FLAT;
                    sCurBg->mUnitFlags.off(ux, uz);
                }
            }
        }
    }
}

// 80075B9C
BOOL isSlopeStep(const mVec3_c *pos) {
    if (sCurBg != NULL) {
        int unitX, unitZ;
        posToUnit(&unitX, &unitZ, pos);
        if (sCurBg->mUnitFlags.check(unitX, unitZ)) {
            groundChk_c chk(pos, LAYER_TOP, 0, 0);
            getUnitDat(unitX, unitZ);
            if (chk.isContinuous()) {
                return TRUE;
            }
            f32 y = getUnitMinY(unitX, unitZ);
            if (!dBGC::isZero(y - getGroundY(pos, FALSE))) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80075C98
int getNext(int unitX, int unitZ) {
    int attr = getUnitAttr(unitX, unitZ);
    if (attr < BG_ATTR_NUM) {
        return sAttrTable[attr].mNext;
    }
    return -1;
}

// 80075CD8
int getOpenNext(int unitX, int unitZ) {
    int next = getNext(unitX, unitZ);
    if (sCurBg != NULL && next != -1) {
        if (sCurBg->checkFlag(next)) {
            return -1;
        }
        return next;
    }
    return -1;
}

// 80075D3C
int getOpenNext(const mVec3_c *pos) {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    return getOpenNext(unitX, unitZ);
}

// 80075D74
BOOL lockNext(int idx) {
    if (sCurBg != NULL) {
        if (idx == -1) {
            sCurBg->setFlags();
            return TRUE;
        }
        return sCurBg->onFlag(idx);
    }
    return FALSE;
}

// 80075DC8
BOOL isNextLocked(const mVec3_c *pos) {
    if (sCurBg != NULL) {
        int unitX, unitZ;
        posToUnit(&unitX, &unitZ, pos);
        int next = getNext(unitX, unitZ);
        if (next >= 0) {
            return sCurBg->checkFlag(next);
        }
        return FALSE;
    }
    return FALSE;
}

// 80075E30
int getLockedNext(int unitX, int unitZ) {
    if (sCurBg != NULL) {
        int next = getNext(unitX, unitZ);
        if (next >= 0 && sCurBg->checkFlag(next)) {
            return next;
        }
        return -1;
    }
    return -1;
}

// 80075E98
int getLockedNext(const mVec3_c *pos) {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    return getLockedNext(unitX, unitZ);
}

// 80075ED0
BOOL unlockNext(int idx) {
    if (sCurBg != NULL) {
        if (idx == -1) {
            sCurBg->clearFlags();
            return TRUE;
        }
        return sCurBg->offFlag(idx);
    }
    return FALSE;
}

// 80075F24
mVec3_c getNextCenter(int idx) {
    int unitX;
    u32 blockZ;
    int baseZ;
    u32 blockX;
    int baseX;
    u32 z;
    u32 x;
    int unitZ;
    if (sCurBg != NULL && idx != -1) {
        mVec3_c sum(mVec3_c::Zero);
        f32 num = 0.0f;
        for (blockZ = 0, baseZ = 0; blockZ < sCurBg->mBlockH; baseZ += 16, blockZ++) {
            for (blockX = 0, baseX = 0; blockX < sCurBg->mBlockW; baseX += 16, blockX++) {
                for (z = 0, unitZ = baseZ; z < 16; z++, unitZ++) {
                    for (x = 0, unitX = baseX; x < 16; x++, unitX++) {
                        int next = getOpenNext(unitX, unitZ);
                        if (next == idx) {
                            mVec3_c pos;
                            unitToPos(&pos, unitX, unitZ);
                            num += 1.0f;
                            sum += pos;
                        }
                    }
                }
            }
        }
        if (0.0f != num) {
            f32 s = 1.0f / num;
            sum.x *= s;
            sum.y *= s;
            sum.z *= s;
        }
        sum.y = getGroundY(&sum, FALSE);
        return sum;
    } else {
        return mVec3_c::Zero;
    }
}

// 80076104
BOOL getArea(f32 *minX, f32 *maxX, f32 *minZ, f32 *maxZ) {
    if (isLoaded()) {
        int x0 = 0xA0;
        int x1 = 0;
        int z0 = 0xA0;
        int z1 = 0;
        for (int z = 0; z < 0x20; z++) {
            for (int x = 0; x < 0x30; x++) {
                if (getUnitAttr(x, z) != BG_ATTR_WALL) {
                    if (x < x0) {
                        x0 = x;
                    } else if (x > x1) {
                        x1 = x;
                    }
                    if (z < z0) {
                        z0 = z;
                    } else if (z > z1) {
                        z1 = z;
                    }
                }
            }
        }
        if (sCurBg != NULL) {
            z1 = sCurBg->mBlockH * 16;
        }
        mVec3_c p0;
        unitToPos(&p0, x0, z0);
        mVec3_c p1;
        unitToPos(&p1, x1, z1);
        *minX = p0.x - 16.0f;
        *maxX = 16.0f + p1.x;
        *minZ = p0.z - 16.0f;
        if (maxZ != NULL) {
            *maxZ = 16.0f + p1.z;
        }
        return TRUE;
    }
    *minZ = 0.0f;
    *maxX = 0.0f;
    *minX = 0.0f;
    if (maxZ != NULL) {
        *maxZ = 0.0f;
    }
    return FALSE;
}

// 80076260
BOOL setSoilX(int unitX, int unitZ) {
    if (getUnitAttr(unitX, unitZ) == BG_ATTR_SOIL) {
        unitDat_c *dat = getUnitDat(unitX, unitZ);
        if (dat != NULL) {
            dat->mAttr = BG_ATTR_SOILX;
            return TRUE;
        }
    }
    return FALSE;
}

// 800762C8
void fn_800762C8() {}

// 800762CC
void updateUnitAnm() {
    l_mgr.mUnitAnm.update();
}

// 800762E0
void fn_800762E0() {}

// 800762E4
BOOL isNearFall(int unitX, int unitZ) {
    for (int z = unitZ - 1; z <= unitZ + 1; z++) {
        for (int x = unitX - 1; x <= unitX + 1; x++) {
            switch (getUnitAttr(x, z)) {
            case BG_ATTR_FALL_S:
            case BG_ATTR_FALL_SW:
            case BG_ATTR_FALL_SE:
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80076370
BOOL isNearFall(const mVec3_c *pos) {
    int unitX, unitZ;
    posToUnit(&unitX, &unitZ, pos);
    return isNearFall(unitX, unitZ);
}

} // namespace dBGCF

#ifdef MUST_MATCH
// 80582CD8: nothing in the DOL references it (an unused object, kept by the original link).
#pragma force_active on
u8 lbl_80582CD8[0x188];
#pragma force_active reset
#endif
