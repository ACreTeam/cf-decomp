// Fish data. See include/game/game/d_fish_info.hpp and notes/d_fish_info.txt.
// .text 80090E2C..8009176C, .rodata 8046EEF0..8046EFA0, .data 804DFBA0..804E17F0,
// .sdata 80749F70..80749FB8, .sdata2 80750630..80750658.
#include <game/game/d_fish_info.hpp>
#include <game/game/d_search_cand.hpp>
#include <game/cLib/c_math.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/game/d_fireworks.hpp>

// Not split yet (C linkage keeps the target name).
extern "C" BOOL fn_800DCEDC(); // 800DCEDC: an online session is active (spawn rolls use 0..95 then)

// 8046EEF0: length in cm.
static const s16 sLength[FISH_NUM] = {
    10,  15,  30,  35,  50,  80,  80,  15,  15,  4,   12,  12,  15,  20,  60,  100,
    85,  25,  35,  50,  130, 15,  25,  35,  50,  80,  150, 90,  160, 4,   12,  2,
    30,  70,  100, 190, 300, 3,   25,  8,   15,  31,  18,  200, 30,  35,  40,  60,
    100, 90,  50,  80,  35,  60,  50,  80,  60,  230, 220, 120, 300, 250, 540, 150,
};

// 80749F70: one bit per type: the stringfish, arapaima, napoleonfish, ocean sunfish, sharks and
// coelacanth.
static u32 sLarge[2] = {0x04000000, 0xF0000810};

// 804DFBA0..804E10F8: spawn lists per term and time slot (river lists first, then sea lists),
// then per term the time slot pairs and their table.
static dFishSpawn_c sRiver_Jan_MornEve[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 23},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 33},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 38},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 41},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 44},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 45},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 52},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 56},
    {FISH_POND_SMELT, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_STRINGFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Jan_Day[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 23},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 30},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 35},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 38},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 40},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 51},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 57},
    {FISH_POND_SMELT, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Jan_Night[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 15},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 30},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 45},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 49},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 50},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 51},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 56},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 62},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 66},
    {FISH_POND_SMELT, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_STRINGFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Feb_MornEve[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 23},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 33},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 38},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 41},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 44},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 45},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 52},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 56},
    {FISH_POND_SMELT, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_STRINGFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Feb_Day[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 23},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 30},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 35},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 38},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 40},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 51},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 57},
    {FISH_POND_SMELT, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Feb_Night[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 15},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 30},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 45},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 49},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 50},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 51},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 56},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 62},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 66},
    {FISH_POND_SMELT, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_STRINGFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Mar_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 24},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 27},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 29},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 31},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 32},
    {FISH_LOACH, dFishSpawn_c::PLACE_RIVER, 57},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 62},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 68},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 78},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 86},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Mar_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 7},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 29},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 36},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 40},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 42},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_LOACH, dFishSpawn_c::PLACE_RIVER, 78},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 84},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 90},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Mar_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 16},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 32},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 48},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 52},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 53},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 55},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 60},
    {FISH_LOACH, dFishSpawn_c::PLACE_RIVER, 85},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Apr_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 9},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 14},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 19},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 32},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 34},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 36},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 39},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 51},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 52},
    {FISH_LOACH, dFishSpawn_c::PLACE_RIVER, 62},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 69},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 78},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 86},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Apr_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 29},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 34},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 37},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 40},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 43},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 59},
    {FISH_LOACH, dFishSpawn_c::PLACE_RIVER, 80},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 86},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Apr_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 15},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 29},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 57},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 58},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 60},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 63},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 75},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 79},
    {FISH_LOACH, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_May_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 8},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 12},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 16},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 27},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 29},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 31},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 34},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 46},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 56},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 57},
    {FISH_LOACH, dFishSpawn_c::PLACE_RIVER, 60},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 62},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 77},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 82},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 89},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_May_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 28},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 41},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 46},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 56},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 66},
    {FISH_LOACH, dFishSpawn_c::PLACE_RIVER, 71},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 77},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_May_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 12},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 20},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 28},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 40},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 42},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 45},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 56},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 66},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 70},
    {FISH_LOACH, dFishSpawn_c::PLACE_RIVER, 73},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 79},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Jun_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 7},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 13},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 21},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 23},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 24},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 28},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 39},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 49},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 50},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 55},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 60},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 76},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 80},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 86},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 90},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Jun_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 27},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 34},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 37},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 41},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 46},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 56},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 64},
    {FISH_GIANT_SNAKEHEAD, dFishSpawn_c::PLACE_POOL, 66},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 76},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 86},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Jun_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 5},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 8},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 14},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 20},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 21},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 26},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 37},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 47},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 51},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 68},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 78},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 91},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Jul_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 7},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 14},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 20},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 23},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 26},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 36},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 46},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 47},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 52},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 57},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 73},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 91},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Jul_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 5},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 15},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 21},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 24},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 26},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 28},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 31},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 40},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 50},
    {FISH_GIANT_SNAKEHEAD, dFishSpawn_c::PLACE_POOL, 52},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 62},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 71},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 86},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Jul_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 5},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 8},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 14},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 17},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 19},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 22},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 31},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 41},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 45},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 61},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 73},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 85},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 90},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Aug1_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 7},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 14},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 19},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 21},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 25},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 35},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 45},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 46},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 51},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 56},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 72},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 91},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Aug1_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 6},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 16},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 25},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 27},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 29},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 32},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 41},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 51},
    {FISH_GIANT_SNAKEHEAD, dFishSpawn_c::PLACE_POOL, 53},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 63},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 70},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 86},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Aug1_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 5},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 8},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 14},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 19},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 20},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 23},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 32},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 42},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 46},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 63},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 74},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 85},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 90},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Aug2_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 7},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 14},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 19},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 21},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 25},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 35},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 45},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 46},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 51},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 56},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 72},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 91},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Aug2_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 6},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 16},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 25},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 27},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 29},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 32},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 41},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 51},
    {FISH_GIANT_SNAKEHEAD, dFishSpawn_c::PLACE_POOL, 53},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 63},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 70},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 86},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Aug2_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 5},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 8},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 14},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 19},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 20},
    {FISH_KILLIFISH, dFishSpawn_c::PLACE_POND, 23},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 32},
    {FISH_FROG, dFishSpawn_c::PLACE_POND, 42},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 46},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 63},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 74},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 85},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 90},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Sep1_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 8},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 11},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 15},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 20},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 24},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 35},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 36},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 41},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 58},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 59},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 64},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 73},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 80},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 91},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Sep1_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 25},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 32},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 35},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 37},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 49},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 59},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 69},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 70},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 78},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 81},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 83},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 86},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Sep1_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 16},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 21},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 27},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 30},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 31},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 33},
    {FISH_CRAWFISH, dFishSpawn_c::PLACE_POND, 45},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 49},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 66},
    {FISH_EEL, dFishSpawn_c::PLACE_RIVER, 71},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 85},
    {FISH_SWEETFISH, dFishSpawn_c::PLACE_RIVER, 90},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_PIRANHA, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_AROWANA, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Sep2_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 4},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 7},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 13},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 15},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 17},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 21},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 31},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 32},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 37},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 41},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 46},
    {FISH_SALMON, dFishSpawn_c::PLACE_RIVER, 86},
    {FISH_KING_SALMON, dFishSpawn_c::PLACE_RIVER, 91},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 92},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Sep2_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 6},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 14},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 21},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 23},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 24},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 32},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 40},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 45},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 48},
    {FISH_SALMON, dFishSpawn_c::PLACE_RIVER, 88},
    {FISH_KING_SALMON, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_DORADO, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Sep2_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 7},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 15},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 19},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 21},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 25},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 38},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 48},
    {FISH_SALMON, dFishSpawn_c::PLACE_RIVER, 87},
    {FISH_KING_SALMON, dFishSpawn_c::PLACE_RIVER, 93},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_GAR, dFishSpawn_c::PLACE_POOL, 95},
    {FISH_ARAPAIMA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Oct_MornEve[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 11},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 16},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 37},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 41},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 42},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 44},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 49},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 64},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 65},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 76},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 84},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Oct_Day[] = {
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 32},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 40},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 45},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 47},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 48},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 58},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 64},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 77},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 78},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 84},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 88},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Oct_Night[] = {
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 21},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 33},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 48},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 63},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 64},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 66},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 71},
    {FISH_CATFISH, dFishSpawn_c::PLACE_POOL, 77},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 80},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_ANGELFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Nov_MornEve[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 5},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 19},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 25},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 31},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 46},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 48},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 50},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 51},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 58},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 73},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 74},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 82},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 88},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Nov_Day[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 6},
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 16},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 31},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 39},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 54},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 56},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 57},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 67},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 75},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 85},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 86},
    {FISH_CHERRY_SALMON, dFishSpawn_c::PLACE_RIVER, 89},
    {FISH_CHAR, dFishSpawn_c::PLACE_WATERFALL, 91},
    {FISH_RAINBOW_TROUT, dFishSpawn_c::PLACE_RIVER, 94},
    {FISH_GUPPY, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_NEON_TETRA, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Nov_Night[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 2},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 22},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 37},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 52},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 67},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 68},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 70},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 75},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 81},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Dec_MornEve[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 23},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 33},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 38},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 41},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 45},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 46},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 53},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 57},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 58},
    {FISH_POND_SMELT, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_STRINGFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Dec_Day[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 18},
    {FISH_PALE_CHUB, dFishSpawn_c::PLACE_RIVER, 23},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 30},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 36},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 40},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 42},
    {FISH_POPEYED_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 43},
    {FISH_BLUEGILL, dFishSpawn_c::PLACE_RIVER, 46},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 54},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 60},
    {FISH_PIKE, dFishSpawn_c::PLACE_RIVER, 61},
    {FISH_POND_SMELT, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sRiver_Dec_Night[] = {
    {FISH_BITTERLING, dFishSpawn_c::PLACE_RIVER, 10},
    {FISH_CRUCIAN_CARP, dFishSpawn_c::PLACE_RIVER, 17},
    {FISH_DACE, dFishSpawn_c::PLACE_RIVER, 32},
    {FISH_BARBEL_STEED, dFishSpawn_c::PLACE_RIVER, 47},
    {FISH_CARP, dFishSpawn_c::PLACE_RIVER, 51},
    {FISH_KOI, dFishSpawn_c::PLACE_RIVER, 52},
    {FISH_GOLDFISH, dFishSpawn_c::PLACE_RIVER, 54},
    {FISH_FRESHWATER_GOBY, dFishSpawn_c::PLACE_RIVER, 59},
    {FISH_YELLOW_PERCH, dFishSpawn_c::PLACE_RIVER, 65},
    {FISH_BLACK_BASS, dFishSpawn_c::PLACE_RIVER, 69},
    {FISH_POND_SMELT, dFishSpawn_c::PLACE_RIVER, 95},
    {FISH_STRINGFISH, dFishSpawn_c::PLACE_RIVER, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_RIVER, 98},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_RIVER, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_RIVER, 100},
};
static dFishSpawn_c sSea_Jan_MornEve[] = {
    {FISH_SEA_BUTTERFLY, dFishSpawn_c::PLACE_SEA, 10},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 30},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 46},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 50},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 63},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 70},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 78},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 86},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 87},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Jan_Day[] = {
    {FISH_SEA_BUTTERFLY, dFishSpawn_c::PLACE_SEA, 9},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 33},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 55},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 69},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 77},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Jan_Night[] = {
    {FISH_SEA_BUTTERFLY, dFishSpawn_c::PLACE_SEA, 12},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 32},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 47},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 64},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 79},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 84},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Feb_MornEve[] = {
    {FISH_SEA_BUTTERFLY, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 28},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 44},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 48},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 62},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 70},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Feb_Day[] = {
    {FISH_SEA_BUTTERFLY, dFishSpawn_c::PLACE_SEA, 7},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 31},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 50},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 54},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 67},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 87},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Feb_Night[] = {
    {FISH_SEA_BUTTERFLY, dFishSpawn_c::PLACE_SEA, 11},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 31},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 46},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 50},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 64},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 72},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 82},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 84},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Mar_MornEve[] = {
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 24},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 27},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 46},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 53},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 65},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 81},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Mar_Day[] = {
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 30},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 33},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 53},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 60},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 77},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 87},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Mar_Night[] = {
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 25},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 28},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 44},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 63},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 69},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 79},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 89},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 96},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Apr_MornEve[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 4},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 10},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 12},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 17},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 37},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 40},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 56},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 64},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 74},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 78},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 86},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Apr_Day[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 6},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 12},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 14},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 18},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 24},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 44},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 47},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 57},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 65},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 73},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 77},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Apr_Night[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 3},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 5},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 7},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 9},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 12},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 32},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 35},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 50},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 58},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 68},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 72},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 86},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_May_MornEve[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 6},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 13},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 15},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 22},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 30},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 45},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 48},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 64},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 72},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 76},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 84},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_May_Day[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 14},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 16},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 20},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 30},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 48},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 67},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 79},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_May_Night[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 5},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 13},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 15},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 23},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 28},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 44},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 47},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 63},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 88},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Jun_MornEve[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 6},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 13},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 15},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 22},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 30},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 45},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 48},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 63},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Jun_Day[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 15},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 17},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 24},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 33},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 48},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 67},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 79},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Jun_Night[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 5},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 13},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 15},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 23},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 30},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 46},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 49},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 65},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 73},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 77},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Jul_MornEve[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 6},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 14},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 16},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 21},
    {FISH_NAPOLEONFISH, dFishSpawn_c::PLACE_SEA, 22},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 30},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 35},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 45},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 49},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 66},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 74},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 77},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Jul_Day[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 16},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 18},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 23},
    {FISH_NAPOLEONFISH, dFishSpawn_c::PLACE_SEA, 24},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 34},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 41},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 52},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 56},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 70},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 78},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 81},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 86},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Jul_Night[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 5},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 13},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 15},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 20},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 26},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 32},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 44},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 48},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 64},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 72},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Aug1_MornEve[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 16},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 18},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 23},
    {FISH_NAPOLEONFISH, dFishSpawn_c::PLACE_SEA, 24},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 32},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 43},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 53},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 57},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 78},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 80},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 87},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 89},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Aug1_Day[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 16},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 18},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 23},
    {FISH_NAPOLEONFISH, dFishSpawn_c::PLACE_SEA, 24},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 34},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 46},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 60},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 64},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 77},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 81},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 88},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Aug1_Night[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 5},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 13},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 15},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 20},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 27},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 39},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 49},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 53},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 78},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 80},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 88},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Aug2_MornEve[] = {
    {FISH_JELLYFISH, dFishSpawn_c::PLACE_SEA, 40},
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 43},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 47},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 49},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_NAPOLEONFISH, dFishSpawn_c::PLACE_SEA, 52},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 56},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 60},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 69},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 72},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 82},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 87},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 89},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Aug2_Day[] = {
    {FISH_JELLYFISH, dFishSpawn_c::PLACE_SEA, 40},
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 44},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 48},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 50},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 53},
    {FISH_NAPOLEONFISH, dFishSpawn_c::PLACE_SEA, 54},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 59},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 63},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 72},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 86},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 88},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Aug2_Night[] = {
    {FISH_JELLYFISH, dFishSpawn_c::PLACE_SEA, 40},
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 43},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 45},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 47},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 49},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 52},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 63},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 72},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 86},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 88},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Sep1_MornEve[] = {
    {FISH_SALMON, dFishSpawn_c::PLACE_RIVER_MOUTH, 40},
    {FISH_KING_SALMON, dFishSpawn_c::PLACE_RIVER_MOUTH, 46},
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 49},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 52},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 54},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 58},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 64},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 73},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 76},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 82},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 87},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 89},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Sep1_Day[] = {
    {FISH_SALMON, dFishSpawn_c::PLACE_RIVER_MOUTH, 39},
    {FISH_KING_SALMON, dFishSpawn_c::PLACE_RIVER_MOUTH, 45},
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 48},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 50},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 53},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 58},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 65},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 74},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 77},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 84},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 87},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 89},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Sep1_Night[] = {
    {FISH_SALMON, dFishSpawn_c::PLACE_RIVER_MOUTH, 40},
    {FISH_KING_SALMON, dFishSpawn_c::PLACE_RIVER_MOUTH, 46},
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 49},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 52},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 54},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 57},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 64},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 73},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 76},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 86},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 88},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Sep2_MornEve[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 7},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 9},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 10},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 12},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 19},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 25},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 50},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 54},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 78},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 80},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 87},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 89},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Sep2_Day[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 9},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 11},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 12},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 14},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 23},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 31},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 53},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 57},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 74},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 81},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 83},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_OCEAN_SUNFISH, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Sep2_Night[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 9},
    {FISH_CLOWNFISH, dFishSpawn_c::PLACE_SEA, 11},
    {FISH_SURGEONFISH, dFishSpawn_c::PLACE_SEA, 12},
    {FISH_BUTTERFLY_FISH, dFishSpawn_c::PLACE_SEA, 16},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 24},
    {FISH_BLOWFISH, dFishSpawn_c::PLACE_SEA, 30},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 51},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 55},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 82},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 84},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 88},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 90},
    {FISH_BLUE_MARLIN, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_HAMMERHEAD_SHARK, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_SHARK, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Oct_MornEve[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 6},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 15},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 35},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 39},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 60},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 67},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 77},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 81},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Oct_Day[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 16},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 40},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 44},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 61},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 68},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 78},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 82},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Oct_Night[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 5},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 11},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 35},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 39},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 64},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 81},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_MORAY_EEL, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Nov_MornEve[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 4},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 32},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 35},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 56},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 59},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 71},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 77},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 87},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 88},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Nov_Day[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 5},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 10},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 35},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 38},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 63},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 66},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 76},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 82},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_RAY, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Nov_Night[] = {
    {FISH_SEAHORSE, dFishSpawn_c::PLACE_SEA, 3},
    {FISH_LIONFISH, dFishSpawn_c::PLACE_SEA, 6},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 32},
    {FISH_BARRED_KNIFEJAW, dFishSpawn_c::PLACE_SEA, 35},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 59},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 62},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 73},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 79},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 85},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 86},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Dec_MornEve[] = {
    {FISH_SEA_BUTTERFLY, dFishSpawn_c::PLACE_SEA, 8},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 27},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 41},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 44},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 58},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 66},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 74},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 84},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 86},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 92},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 94},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Dec_Day[] = {
    {FISH_SEA_BUTTERFLY, dFishSpawn_c::PLACE_SEA, 7},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 31},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 52},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 55},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 67},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 75},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 81},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};
static dFishSpawn_c sSea_Dec_Night[] = {
    {FISH_SEA_BUTTERFLY, dFishSpawn_c::PLACE_SEA, 11},
    {FISH_HORSE_MACKEREL, dFishSpawn_c::PLACE_SEA, 29},
    {FISH_SEA_BASS, dFishSpawn_c::PLACE_SEA, 44},
    {FISH_RED_SNAPPER, dFishSpawn_c::PLACE_SEA, 47},
    {FISH_DAB, dFishSpawn_c::PLACE_SEA, 60},
    {FISH_OLIVE_FLOUNDER, dFishSpawn_c::PLACE_SEA, 68},
    {FISH_SQUID, dFishSpawn_c::PLACE_SEA, 76},
    {FISH_OCTOPUS, dFishSpawn_c::PLACE_SEA, 82},
    {FISH_LOBSTER, dFishSpawn_c::PLACE_SEA, 84},
    {FISH_FOOTBALL_FISH, dFishSpawn_c::PLACE_SEA, 91},
    {FISH_TUNA, dFishSpawn_c::PLACE_SEA, 93},
    {FISH_COELACANTH, dFishSpawn_c::PLACE_OFFING, 95},
    {FISH_TRASH_0, dFishSpawn_c::PLACE_SEA, 97},
    {FISH_TRASH_1, dFishSpawn_c::PLACE_SEA, 99},
    {FISH_TRASH_2, dFishSpawn_c::PLACE_SEA, 100},
};

static dFishSpawnList_c sSpawn_Jan_MornEve[FISH_WATER_NUM] = {{sRiver_Jan_MornEve, ARRAY_SIZE(sRiver_Jan_MornEve)}, {sSea_Jan_MornEve, ARRAY_SIZE(sSea_Jan_MornEve)}};
static dFishSpawnList_c sSpawn_Jan_Day[FISH_WATER_NUM] = {{sRiver_Jan_Day, ARRAY_SIZE(sRiver_Jan_Day)}, {sSea_Jan_Day, ARRAY_SIZE(sSea_Jan_Day)}};
static dFishSpawnList_c sSpawn_Jan_Night[FISH_WATER_NUM] = {{sRiver_Jan_Night, ARRAY_SIZE(sRiver_Jan_Night)}, {sSea_Jan_Night, ARRAY_SIZE(sSea_Jan_Night)}};
static dFishSpawnList_c *sSpawn_Jan[] = {sSpawn_Jan_MornEve, sSpawn_Jan_Day, sSpawn_Jan_Night, NULL};

static dFishSpawnList_c sSpawn_Feb_MornEve[FISH_WATER_NUM] = {{sRiver_Feb_MornEve, ARRAY_SIZE(sRiver_Feb_MornEve)}, {sSea_Feb_MornEve, ARRAY_SIZE(sSea_Feb_MornEve)}};
static dFishSpawnList_c sSpawn_Feb_Day[FISH_WATER_NUM] = {{sRiver_Feb_Day, ARRAY_SIZE(sRiver_Feb_Day)}, {sSea_Feb_Day, ARRAY_SIZE(sSea_Feb_Day)}};
static dFishSpawnList_c sSpawn_Feb_Night[FISH_WATER_NUM] = {{sRiver_Feb_Night, ARRAY_SIZE(sRiver_Feb_Night)}, {sSea_Feb_Night, ARRAY_SIZE(sSea_Feb_Night)}};
static dFishSpawnList_c *sSpawn_Feb[] = {sSpawn_Feb_MornEve, sSpawn_Feb_Day, sSpawn_Feb_Night, NULL};

static dFishSpawnList_c sSpawn_Mar_MornEve[FISH_WATER_NUM] = {{sRiver_Mar_MornEve, ARRAY_SIZE(sRiver_Mar_MornEve)}, {sSea_Mar_MornEve, ARRAY_SIZE(sSea_Mar_MornEve)}};
static dFishSpawnList_c sSpawn_Mar_Day[FISH_WATER_NUM] = {{sRiver_Mar_Day, ARRAY_SIZE(sRiver_Mar_Day)}, {sSea_Mar_Day, ARRAY_SIZE(sSea_Mar_Day)}};
static dFishSpawnList_c sSpawn_Mar_Night[FISH_WATER_NUM] = {{sRiver_Mar_Night, ARRAY_SIZE(sRiver_Mar_Night)}, {sSea_Mar_Night, ARRAY_SIZE(sSea_Mar_Night)}};
static dFishSpawnList_c *sSpawn_Mar[] = {sSpawn_Mar_MornEve, sSpawn_Mar_Day, sSpawn_Mar_Night, NULL};

static dFishSpawnList_c sSpawn_Apr_MornEve[FISH_WATER_NUM] = {{sRiver_Apr_MornEve, ARRAY_SIZE(sRiver_Apr_MornEve)}, {sSea_Apr_MornEve, ARRAY_SIZE(sSea_Apr_MornEve)}};
static dFishSpawnList_c sSpawn_Apr_Day[FISH_WATER_NUM] = {{sRiver_Apr_Day, ARRAY_SIZE(sRiver_Apr_Day)}, {sSea_Apr_Day, ARRAY_SIZE(sSea_Apr_Day)}};
static dFishSpawnList_c sSpawn_Apr_Night[FISH_WATER_NUM] = {{sRiver_Apr_Night, ARRAY_SIZE(sRiver_Apr_Night)}, {sSea_Apr_Night, ARRAY_SIZE(sSea_Apr_Night)}};
static dFishSpawnList_c *sSpawn_Apr[] = {sSpawn_Apr_MornEve, sSpawn_Apr_Day, sSpawn_Apr_Night, NULL};

static dFishSpawnList_c sSpawn_May_MornEve[FISH_WATER_NUM] = {{sRiver_May_MornEve, ARRAY_SIZE(sRiver_May_MornEve)}, {sSea_May_MornEve, ARRAY_SIZE(sSea_May_MornEve)}};
static dFishSpawnList_c sSpawn_May_Day[FISH_WATER_NUM] = {{sRiver_May_Day, ARRAY_SIZE(sRiver_May_Day)}, {sSea_May_Day, ARRAY_SIZE(sSea_May_Day)}};
static dFishSpawnList_c sSpawn_May_Night[FISH_WATER_NUM] = {{sRiver_May_Night, ARRAY_SIZE(sRiver_May_Night)}, {sSea_May_Night, ARRAY_SIZE(sSea_May_Night)}};
static dFishSpawnList_c *sSpawn_May[] = {sSpawn_May_MornEve, sSpawn_May_Day, sSpawn_May_Night, NULL};

static dFishSpawnList_c sSpawn_Jun_MornEve[FISH_WATER_NUM] = {{sRiver_Jun_MornEve, ARRAY_SIZE(sRiver_Jun_MornEve)}, {sSea_Jun_MornEve, ARRAY_SIZE(sSea_Jun_MornEve)}};
static dFishSpawnList_c sSpawn_Jun_Day[FISH_WATER_NUM] = {{sRiver_Jun_Day, ARRAY_SIZE(sRiver_Jun_Day)}, {sSea_Jun_Day, ARRAY_SIZE(sSea_Jun_Day)}};
static dFishSpawnList_c sSpawn_Jun_Night[FISH_WATER_NUM] = {{sRiver_Jun_Night, ARRAY_SIZE(sRiver_Jun_Night)}, {sSea_Jun_Night, ARRAY_SIZE(sSea_Jun_Night)}};
static dFishSpawnList_c *sSpawn_Jun[] = {sSpawn_Jun_MornEve, sSpawn_Jun_Day, sSpawn_Jun_Night, NULL};

static dFishSpawnList_c sSpawn_Jul_MornEve[FISH_WATER_NUM] = {{sRiver_Jul_MornEve, ARRAY_SIZE(sRiver_Jul_MornEve)}, {sSea_Jul_MornEve, ARRAY_SIZE(sSea_Jul_MornEve)}};
static dFishSpawnList_c sSpawn_Jul_Day[FISH_WATER_NUM] = {{sRiver_Jul_Day, ARRAY_SIZE(sRiver_Jul_Day)}, {sSea_Jul_Day, ARRAY_SIZE(sSea_Jul_Day)}};
static dFishSpawnList_c sSpawn_Jul_Night[FISH_WATER_NUM] = {{sRiver_Jul_Night, ARRAY_SIZE(sRiver_Jul_Night)}, {sSea_Jul_Night, ARRAY_SIZE(sSea_Jul_Night)}};
static dFishSpawnList_c *sSpawn_Jul[] = {sSpawn_Jul_MornEve, sSpawn_Jul_Day, sSpawn_Jul_Night, NULL};

static dFishSpawnList_c sSpawn_Aug1_MornEve[FISH_WATER_NUM] = {{sRiver_Aug1_MornEve, ARRAY_SIZE(sRiver_Aug1_MornEve)}, {sSea_Aug1_MornEve, ARRAY_SIZE(sSea_Aug1_MornEve)}};
static dFishSpawnList_c sSpawn_Aug1_Day[FISH_WATER_NUM] = {{sRiver_Aug1_Day, ARRAY_SIZE(sRiver_Aug1_Day)}, {sSea_Aug1_Day, ARRAY_SIZE(sSea_Aug1_Day)}};
static dFishSpawnList_c sSpawn_Aug1_Night[FISH_WATER_NUM] = {{sRiver_Aug1_Night, ARRAY_SIZE(sRiver_Aug1_Night)}, {sSea_Aug1_Night, ARRAY_SIZE(sSea_Aug1_Night)}};
static dFishSpawnList_c *sSpawn_Aug1[] = {sSpawn_Aug1_MornEve, sSpawn_Aug1_Day, sSpawn_Aug1_Night, NULL};

static dFishSpawnList_c sSpawn_Aug2_MornEve[FISH_WATER_NUM] = {{sRiver_Aug2_MornEve, ARRAY_SIZE(sRiver_Aug2_MornEve)}, {sSea_Aug2_MornEve, ARRAY_SIZE(sSea_Aug2_MornEve)}};
static dFishSpawnList_c sSpawn_Aug2_Day[FISH_WATER_NUM] = {{sRiver_Aug2_Day, ARRAY_SIZE(sRiver_Aug2_Day)}, {sSea_Aug2_Day, ARRAY_SIZE(sSea_Aug2_Day)}};
static dFishSpawnList_c sSpawn_Aug2_Night[FISH_WATER_NUM] = {{sRiver_Aug2_Night, ARRAY_SIZE(sRiver_Aug2_Night)}, {sSea_Aug2_Night, ARRAY_SIZE(sSea_Aug2_Night)}};
static dFishSpawnList_c *sSpawn_Aug2[] = {sSpawn_Aug2_MornEve, sSpawn_Aug2_Day, sSpawn_Aug2_Night, NULL};

static dFishSpawnList_c sSpawn_Sep1_MornEve[FISH_WATER_NUM] = {{sRiver_Sep1_MornEve, ARRAY_SIZE(sRiver_Sep1_MornEve)}, {sSea_Sep1_MornEve, ARRAY_SIZE(sSea_Sep1_MornEve)}};
static dFishSpawnList_c sSpawn_Sep1_Day[FISH_WATER_NUM] = {{sRiver_Sep1_Day, ARRAY_SIZE(sRiver_Sep1_Day)}, {sSea_Sep1_Day, ARRAY_SIZE(sSea_Sep1_Day)}};
static dFishSpawnList_c sSpawn_Sep1_Night[FISH_WATER_NUM] = {{sRiver_Sep1_Night, ARRAY_SIZE(sRiver_Sep1_Night)}, {sSea_Sep1_Night, ARRAY_SIZE(sSea_Sep1_Night)}};
static dFishSpawnList_c *sSpawn_Sep1[] = {sSpawn_Sep1_MornEve, sSpawn_Sep1_Day, sSpawn_Sep1_Night, NULL};

static dFishSpawnList_c sSpawn_Sep2_MornEve[FISH_WATER_NUM] = {{sRiver_Sep2_MornEve, ARRAY_SIZE(sRiver_Sep2_MornEve)}, {sSea_Sep2_MornEve, ARRAY_SIZE(sSea_Sep2_MornEve)}};
static dFishSpawnList_c sSpawn_Sep2_Day[FISH_WATER_NUM] = {{sRiver_Sep2_Day, ARRAY_SIZE(sRiver_Sep2_Day)}, {sSea_Sep2_Day, ARRAY_SIZE(sSea_Sep2_Day)}};
static dFishSpawnList_c sSpawn_Sep2_Night[FISH_WATER_NUM] = {{sRiver_Sep2_Night, ARRAY_SIZE(sRiver_Sep2_Night)}, {sSea_Sep2_Night, ARRAY_SIZE(sSea_Sep2_Night)}};
static dFishSpawnList_c *sSpawn_Sep2[] = {sSpawn_Sep2_MornEve, sSpawn_Sep2_Day, sSpawn_Sep2_Night, NULL};

static dFishSpawnList_c sSpawn_Oct_MornEve[FISH_WATER_NUM] = {{sRiver_Oct_MornEve, ARRAY_SIZE(sRiver_Oct_MornEve)}, {sSea_Oct_MornEve, ARRAY_SIZE(sSea_Oct_MornEve)}};
static dFishSpawnList_c sSpawn_Oct_Day[FISH_WATER_NUM] = {{sRiver_Oct_Day, ARRAY_SIZE(sRiver_Oct_Day)}, {sSea_Oct_Day, ARRAY_SIZE(sSea_Oct_Day)}};
static dFishSpawnList_c sSpawn_Oct_Night[FISH_WATER_NUM] = {{sRiver_Oct_Night, ARRAY_SIZE(sRiver_Oct_Night)}, {sSea_Oct_Night, ARRAY_SIZE(sSea_Oct_Night)}};
static dFishSpawnList_c *sSpawn_Oct[] = {sSpawn_Oct_MornEve, sSpawn_Oct_Day, sSpawn_Oct_Night, NULL};

static dFishSpawnList_c sSpawn_Nov_MornEve[FISH_WATER_NUM] = {{sRiver_Nov_MornEve, ARRAY_SIZE(sRiver_Nov_MornEve)}, {sSea_Nov_MornEve, ARRAY_SIZE(sSea_Nov_MornEve)}};
static dFishSpawnList_c sSpawn_Nov_Day[FISH_WATER_NUM] = {{sRiver_Nov_Day, ARRAY_SIZE(sRiver_Nov_Day)}, {sSea_Nov_Day, ARRAY_SIZE(sSea_Nov_Day)}};
static dFishSpawnList_c sSpawn_Nov_Night[FISH_WATER_NUM] = {{sRiver_Nov_Night, ARRAY_SIZE(sRiver_Nov_Night)}, {sSea_Nov_Night, ARRAY_SIZE(sSea_Nov_Night)}};
static dFishSpawnList_c *sSpawn_Nov[] = {sSpawn_Nov_MornEve, sSpawn_Nov_Day, sSpawn_Nov_Night, NULL};

static dFishSpawnList_c sSpawn_Dec_MornEve[FISH_WATER_NUM] = {{sRiver_Dec_MornEve, ARRAY_SIZE(sRiver_Dec_MornEve)}, {sSea_Dec_MornEve, ARRAY_SIZE(sSea_Dec_MornEve)}};
static dFishSpawnList_c sSpawn_Dec_Day[FISH_WATER_NUM] = {{sRiver_Dec_Day, ARRAY_SIZE(sRiver_Dec_Day)}, {sSea_Dec_Day, ARRAY_SIZE(sSea_Dec_Day)}};
static dFishSpawnList_c sSpawn_Dec_Night[FISH_WATER_NUM] = {{sRiver_Dec_Night, ARRAY_SIZE(sRiver_Dec_Night)}, {sSea_Dec_Night, ARRAY_SIZE(sSea_Dec_Night)}};
static dFishSpawnList_c *sSpawn_Dec[] = {sSpawn_Dec_MornEve, sSpawn_Dec_Day, sSpawn_Dec_Night, NULL};

// 804E10F8
static dFishSpawnList_c **sSpawnTable[FISH_TERM_NUM] = {
    sSpawn_Jan, sSpawn_Feb, sSpawn_Mar, sSpawn_Apr, sSpawn_May, sSpawn_Jun, sSpawn_Jul, sSpawn_Aug1, sSpawn_Aug2, sSpawn_Sep1, sSpawn_Sep2, sSpawn_Oct, sSpawn_Nov, sSpawn_Dec,
};

// 804E1130 / 804E1230
static f32 sHoldParam[FISH_NUM] = {
    1.5f, 1.5f, 2.0f, 3.0f, 1.5f, 1.5f, 1.5f, 2.0f,
    2.0f, 3.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 4.0f,
    1.5f, 2.0f, 3.5f, 1.5f, 1.5f, 2.5f, 1.5f, 1.5f,
    1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 4.0f, 3.0f, 3.0f,
    1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 3.0f, 1.5f, 2.5f,
    1.5f, 1.5f, 3.0f, 1.5f, 1.5f, 4.0f, 3.0f, 2.5f,
    1.5f, 2.5f, 3.0f, 3.0f, 1.5f, 2.0f, 1.5f, 2.0f,
    1.5f, 1.5f, 1.5f, 1.5f, 3.0f, 4.0f, 1.0f, 1.5f,
};
static f32 sModelScale[FISH_NUM] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 0.7f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.8f, 1.0f, 0.8f, 0.7f, 1.0f,
};

// 804E15FC
static const char *sModelName[FISH_TYPE_NUM] = {
    "fsh_tanago", "fsh_oikawa", "fsh_funa", "fsh_ugui",
    "fsh_nigoi", "fsh_koi", "fsh_nishiki", "fsh_kingyo",
    "fsh_demekin", "fsh_medaka", "fsh_zarigani", "fsh_kaeru",
    "fsh_donko", "fsh_dojou", "fsh_namazu", "fsh_unagi",
    "fsh_raigyo", "fsh_blue", "fsh_yellow", "fsh_black",
    "fsh_pike", "fsh_wakasagi", "fsh_ayu", "fsh_yamame",
    "fsh_ooiwana", "fsh_nijimasu", "fsh_itou", "fsh_sake",
    "fsh_king", "fsh_guppi", "fsh_angel", "fsh_neon",
    "fsh_pirania", "fsh_arowana", "fsh_dorado", "fsh_ga",
    "fsh_piraruku", "fsh_kurione", "fsh_kurage", "fsh_tatsu",
    "fsh_kumanomi", "fsh_nanyou", "fsh_chouchou", "fsh_napoleon",
    "fsh_minokasago", "fsh_harisen", "fsh_aji", "fsh_ishidai",
    "fsh_suzuki", "fsh_tai", "fsh_karei", "fsh_hirame",
    "fsh_ika", "fsh_tako", "fsh_lobster", "fsh_utsubo",
    "fsh_ankou", "fsh_maguro", "fsh_kajiki", "fsh_ei",
    "fsh_manbou", "fsh_shumoku", "fsh_same", "fsh_siira",
};

// 804E1728 / 804E173C: spawn rate range per dFishRarity_e.
static int sRarityMin[FISH_RARITY_NUM] = {15, 10, 6, 3, 0};
static int sRarityMax[FISH_RARITY_NUM] = {100, 14, 9, 5, 2};

namespace dFishInfo {

// 80090E2C
int getTerm(const dTime_c &time) {
    int term;
    switch (time.month) {
    case MONTH_JANUARY:
        term = FISH_TERM_JAN;
        break;
    case MONTH_FEBRUARY:
        term = FISH_TERM_FEB;
        break;
    case MONTH_MARCH:
        term = FISH_TERM_MAR;
        break;
    case MONTH_APRIL:
        term = FISH_TERM_APR;
        break;
    case MONTH_MAY:
        term = FISH_TERM_MAY;
        break;
    case MONTH_JUNE:
        term = FISH_TERM_JUN;
        break;
    case MONTH_JULY:
        term = FISH_TERM_JUL;
        break;
    case MONTH_AUGUST:
        if (time.mday < 15) {
            term = FISH_TERM_AUG_1;
        } else {
            term = FISH_TERM_AUG_2;
        }
        break;
    case MONTH_SEPTEMBER:
        if (time.mday < 15) {
            term = FISH_TERM_SEP_1;
        } else {
            term = FISH_TERM_SEP_2;
        }
        break;
    case MONTH_OCTOBER:
        term = FISH_TERM_OCT;
        break;
    case MONTH_NOVEMBER:
        term = FISH_TERM_NOV;
        break;
    case MONTH_DECEMBER:
        term = FISH_TERM_DEC;
        break;
    }
    return term;
}

// 80090ED8
int getTimeSlot(const dTime_c &time) {
    int hour = time.hour;
    if (hour < 4) {
        return FISH_TIME_NIGHT;
    }
    if (hour < 9) {
        return FISH_TIME_MORNING_EVENING;
    }
    if (hour < 16) {
        return FISH_TIME_DAY;
    }
    if (hour < 21) {
        return FISH_TIME_MORNING_EVENING;
    }
    return FISH_TIME_NIGHT;
}

// 80090F2C
const dFishSpawnList_c *getSpawnLists() {
    const dTime_c *now = dTime_c::getCurrent();
    int term = getTerm(*now);
    int slot = getTimeSlot(*now);
    return sSpawnTable[term][slot];
}

// 80090F88
const dFishSpawn_c *getRandomSpawn(int water) {
    const dFishSpawnList_c *lists = getSpawnLists();
    f32 range = 100.0f;
    int num = lists[water].mNum;
    dFishSpawn_c *list = lists[water].mList;
    if (fn_800DCEDC()) {
        range -= 5.0f;
    }
    f32 roll = cM::rndF(range);
    for (int i = 0; i < num; i++) {
        if (roll < list[i].mRate) {
            return &list[i];
        }
    }
    return NULL;
}

// 80091054
const dFishSpawn_c *getRandomSpawnBoosted(int water, int type) {
    const dFishSpawnList_c *lists = getSpawnLists();
    f32 bonus = 0.0f;
    dFishSpawn_c *list = lists[water].mList;
    int num = lists[water].mNum;
    f32 add = bonus;
    f32 prev = bonus;
    for (int i = 0; i < num; i++) {
        if (type == list[i].mType) {
            f32 rate = list[i].mRate - prev;
            bonus = (100.0f * rate) / (100.0f - 2.0f * rate);
            break;
        }
        prev = list[i].mRate;
    }
    f32 range = 100.0f + bonus;
    if (fn_800DCEDC()) {
        range -= 5.0f;
    }
    f32 roll = cM::rndF(range);
    for (int i = 0; i < num; i++) {
        if (type == list[i].mType) {
            add = bonus;
        }
        if (roll < add + list[i].mRate) {
            return &list[i];
        }
    }
    return NULL;
}

// 80091200
int getTourneyFish(const dTime_c &time) {
    const int fish[4][3] = {
        {FISH_POND_SMELT, FISH_HORSE_MACKEREL, FISH_DAB},
        {FISH_LOACH, FISH_SEA_BASS, FISH_CRUCIAN_CARP},
        {FISH_RED_SNAPPER, FISH_SEA_BASS, FISH_BLACK_BASS},
        {FISH_HORSE_MACKEREL, FISH_CARP, FISH_BLACK_BASS},
    };
    int idx = 0;
    switch (time.month) {
    case MONTH_JANUARY:
        idx = 0;
        break;
    case MONTH_MARCH:
        idx = 1;
        break;
    case MONTH_MAY:
        idx = 2;
        break;
    case MONTH_NOVEMBER:
        idx = 3;
        break;
    }
    const int *cand = fish[idx];
    int type = cand[0];
    for (int i = 1; i < 3; i++) {
        if (randomFloat(0.0f, 1.0f) <= 1.0f / (i + 1)) {
            type = cand[i];
        }
    }
    return type;
}

// 80091370
int getRandomByRarity(const dTime_c &time, int rarity, int maxRarity, BOOL allDay) {
    int term = getTerm(time);
    dSearchCand_c<64> cand;
    cand.clear();
    if (maxRarity == FISH_RARITY_SAME) {
        maxRarity = rarity;
    }
    int timeStart;
    int timeEnd;
    if (allDay) {
        timeStart = 0;
        timeEnd = FISH_TIME_NUM - 1;
    } else {
        timeEnd = getTimeSlot(time);
        timeStart = timeEnd;
    }
    int min = sRarityMin[rarity];
    int max = sRarityMax[maxRarity];
    BOOL down = TRUE;
    int res;
    while (true) {
        for (int t = timeStart; t <= timeEnd; t++) {
            dFishSpawnList_c *lists = sSpawnTable[term][t];
            for (int w = 0; w < FISH_WATER_NUM; w++) {
                int num = lists[w].mNum;
                int prev = 0;
                dFishSpawn_c *list = lists[w].mList;
                for (int i = 0; i < num; i++) {
                    int rate = list[i].mRate - prev;
                    prev = list[i].mRate;
                    if (list[i].mType < FISH_NUM && list[i].mType != FISH_COELACANTH &&
                        sLib::isInRange(rate, min, max)) {
                        cand.add(list[i].mType);
                    }
                }
            }
        }
        res = cand.getRandom();
        if (res >= 0) {
            break;
        }
        if (down) {
            if (--maxRarity < 0) {
                down = FALSE;
                maxRarity = 1;
            }
        } else if (++maxRarity >= FISH_RARITY_NUM) {
            break;
        }
        min = sRarityMin[maxRarity];
        max = sRarityMax[maxRarity];
    }
    return res;
}

// 80091518
u8 getRandomOfTerm() {
    dTime_c time = *dTime_c::getCurrent();
    time.add(0, -6, 0, 0);
    int term = getTerm(time);
    dSearchCand_c<64> cand;
    cand.clear();
    for (int t = 0; t < FISH_TIME_NUM; t++) {
        dFishSpawnList_c *lists = sSpawnTable[term][t];
        for (int w = 0; w < FISH_WATER_NUM; w++) {
            int num = lists[w].mNum;
            dFishSpawn_c *list = lists[w].mList;
            for (int i = 0; i < num; i++) {
                if (list[i].mRate != 0 && list[i].mType < FISH_NUM) {
                    cand.add(list[i].mType);
                }
            }
        }
    }
    return cand.getRandom();
}

// 8009166C
const char *getModelName(int type) {
    return sModelName[type];
}

// 80091680
s16 getLength(int type) {
    return sLength[type];
}

// 80091694
BOOL isLarge(int type) {
    return (sLarge[type / 32] & (1 << (type % 32))) != 0;
}

// 800916D8
f32 getHoldParam(int type) {
    return sHoldParam[type];
}

// 800916EC
f32 getModelScale(int type) {
    return sModelScale[type];
}

// 80091700
void initCatchData(dFishCatchData_c *data) {
    dFishCatchData_c init;
    init.mType = 0;
    init._02 = 0;
    init._03 = 0;
    init._04 = 0;
    init._05 = 0;
    init._06 = 0;
    init._07 = 0;
    init._08 = 0;
    init._09 = 0;
    init._0C = -1;
    init.mType |= FISH_TYPE_NUM;
    init._01 = 0;
    init._07 |= 4;
    cLib::memCpy(data, &init, 8);
}

} // namespace dFishInfo
