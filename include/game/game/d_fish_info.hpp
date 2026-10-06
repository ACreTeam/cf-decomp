#pragma once

// Fish data: which fish spawn in each half-month term and time slot (river and sea lists with
// cumulative spawn rates), the rarity classes, fishing tourney fish, and per-fish tables (model
// names, lengths, ...). Source: src/dol/game/d_fish_info.cpp (.text 80090E2C..8009176C).
// The function names are inferred. See notes/d_fish_info.txt.

#include <types.h>
#include <game/game/d_date.hpp>

// Fish types: the index of every per-fish table (named after the model names, fsh_*).
enum dFishType_e {
    FISH_BITTERLING,        // fsh_tanago
    FISH_PALE_CHUB,         // fsh_oikawa
    FISH_CRUCIAN_CARP,      // fsh_funa
    FISH_DACE,              // fsh_ugui
    FISH_BARBEL_STEED,      // fsh_nigoi
    FISH_CARP,              // fsh_koi
    FISH_KOI,               // fsh_nishiki
    FISH_GOLDFISH,          // fsh_kingyo
    FISH_POPEYED_GOLDFISH,  // fsh_demekin
    FISH_KILLIFISH,         // fsh_medaka
    FISH_CRAWFISH,          // fsh_zarigani
    FISH_FROG,              // fsh_kaeru
    FISH_FRESHWATER_GOBY,   // fsh_donko
    FISH_LOACH,             // fsh_dojou
    FISH_CATFISH,           // fsh_namazu
    FISH_EEL,               // fsh_unagi
    FISH_GIANT_SNAKEHEAD,   // fsh_raigyo
    FISH_BLUEGILL,          // fsh_blue
    FISH_YELLOW_PERCH,      // fsh_yellow
    FISH_BLACK_BASS,        // fsh_black
    FISH_PIKE,              // fsh_pike
    FISH_POND_SMELT,        // fsh_wakasagi
    FISH_SWEETFISH,         // fsh_ayu
    FISH_CHERRY_SALMON,     // fsh_yamame
    FISH_CHAR,              // fsh_ooiwana
    FISH_RAINBOW_TROUT,     // fsh_nijimasu
    FISH_STRINGFISH,        // fsh_itou
    FISH_SALMON,            // fsh_sake
    FISH_KING_SALMON,       // fsh_king
    FISH_GUPPY,             // fsh_guppi
    FISH_ANGELFISH,         // fsh_angel
    FISH_NEON_TETRA,        // fsh_neon
    FISH_PIRANHA,           // fsh_pirania
    FISH_AROWANA,           // fsh_arowana
    FISH_DORADO,            // fsh_dorado
    FISH_GAR,               // fsh_ga
    FISH_ARAPAIMA,          // fsh_piraruku
    FISH_SEA_BUTTERFLY,     // fsh_kurione
    FISH_JELLYFISH,         // fsh_kurage
    FISH_SEAHORSE,          // fsh_tatsu
    FISH_CLOWNFISH,         // fsh_kumanomi
    FISH_SURGEONFISH,       // fsh_nanyou
    FISH_BUTTERFLY_FISH,    // fsh_chouchou
    FISH_NAPOLEONFISH,      // fsh_napoleon
    FISH_ZEBRA_TURKEYFISH,  // fsh_minokasago
    FISH_PUFFER_FISH,       // fsh_harisen
    FISH_HORSE_MACKEREL,    // fsh_aji
    FISH_BARRED_KNIFEJAW,   // fsh_ishidai
    FISH_SEA_BASS,          // fsh_suzuki
    FISH_RED_SNAPPER,       // fsh_tai
    FISH_DAB,               // fsh_karei
    FISH_OLIVE_FLOUNDER,    // fsh_hirame
    FISH_SQUID,             // fsh_ika
    FISH_OCTOPUS,           // fsh_tako
    FISH_LOBSTER,           // fsh_lobster
    FISH_MORAY_EEL,         // fsh_utsubo
    FISH_FOOTBALL_FISH,     // fsh_ankou
    FISH_TUNA,              // fsh_maguro
    FISH_BLUE_MARLIN,       // fsh_kajiki
    FISH_RAY,               // fsh_ei
    FISH_OCEAN_SUNFISH,     // fsh_manbou
    FISH_HAMMERHEAD_SHARK,  // fsh_shumoku
    FISH_SHARK,             // fsh_same
    FISH_COELACANTH,        // fsh_siira

    FISH_NUM,               // 0x40

    // Trash at the end of every spawn list (rates 98, 99, 100); which is which is not known.
    FISH_TRASH_0 = FISH_NUM,
    FISH_TRASH_1,
    FISH_TRASH_2,

    FISH_TYPE_NUM = 0x4B,   // size of the model name table (0x40..0x4A have no model); also "none"
};

// Half-month spawn terms (dFishInfo::getTerm): one per month, August and September in halves.
enum dFishTerm_e {
    FISH_TERM_JAN,
    FISH_TERM_FEB,
    FISH_TERM_MAR,
    FISH_TERM_APR,
    FISH_TERM_MAY,
    FISH_TERM_JUN,
    FISH_TERM_JUL,
    FISH_TERM_AUG_1, // 1st..14th
    FISH_TERM_AUG_2,
    FISH_TERM_SEP_1,
    FISH_TERM_SEP_2,
    FISH_TERM_OCT,
    FISH_TERM_NOV,
    FISH_TERM_DEC,

    FISH_TERM_NUM
};

// Time slots (dFishInfo::getTimeSlot).
enum dFishTime_e {
    FISH_TIME_MORNING_EVENING, // 4:00..8:59 and 16:00..20:59
    FISH_TIME_DAY,             // 9:00..15:59
    FISH_TIME_NIGHT,           // 21:00..3:59

    FISH_TIME_NUM
};

// Spawn lists of a time slot.
enum dFishWater_e {
    FISH_WATER_RIVER,
    FISH_WATER_SEA,

    FISH_WATER_NUM
};

// Rarity classes by spawn rate (percent points of a spawn list entry).
enum dFishRarity_e {
    FISH_RARITY_COMMON,    // 15..100
    FISH_RARITY_1,         // 10..14
    FISH_RARITY_2,         // 6..9
    FISH_RARITY_3,         // 3..5
    FISH_RARITY_RARE,      // 0..2

    FISH_RARITY_NUM,
    FISH_RARITY_SAME = FISH_RARITY_NUM, // dFishInfo::getRandomByRarity: max class = min class
};

// One spawn list entry.
struct dFishSpawn_c {
    // Where it spawns (cf. ac-decomp's aSOG_SPAWN_AREA_*, which has the same areas).
    enum Place_e {
        PLACE_RIVER,       // most river fish and the river trash
        PLACE_POOL,        // river pool: catfish, giant snakehead, gar
        PLACE_WATERFALL,   // char
        PLACE_POND,        // killifish, crawfish, frog
        PLACE_RIVER_MOUTH, // salmon, king salmon (also listed as PLACE_RIVER in some terms)
        PLACE_OFFING,      // deep sea: coelacanth
        PLACE_SEA,         // sea fish and the sea trash

        PLACE_NUM
    };

    /* 0x0 */ u8 mType;  // dFishType_e, or trash (>= FISH_NUM)
    /* 0x1 */ u8 mPlace; // Place_e
    /* 0x2 */ u8 mRate;  // cumulative spawn rate in percent; the list ends at 100
}; // size 0x3

struct dFishSpawnList_c {
    /* 0x0 */ dFishSpawn_c *mList;
    /* 0x4 */ int mNum;
}; // size 0x8

// Catch data written by dFishInfo::initCatchData (only its first 8 bytes are copied).
struct dFishCatchData_c {
    /* 0x0 */ u8 mType;
    /* 0x1 */ u8 _01;
    /* 0x2 */ u8 _02;
    /* 0x3 */ u8 _03;
    /* 0x4 */ u8 _04;
    /* 0x5 */ u8 _05;
    /* 0x6 */ u8 _06;
    /* 0x7 */ u8 _07;
    /* 0x8 */ u8 _08;
    /* 0x9 */ u8 _09;
    /* 0xC */ int _0C;
}; // size 0x10

namespace dFishInfo {

int getTerm(const dTime_c &time);                                       // 80090E2C: dFishTerm_e
int getTimeSlot(const dTime_c &time);                                   // 80090ED8: dFishTime_e
const dFishSpawnList_c *getSpawnLists();                                // 80090F2C: now, [FISH_WATER_NUM]
const dFishSpawn_c *getRandomSpawn(int water);                          // 80090F88
const dFishSpawn_c *getRandomSpawnBoosted(int water, int type);         // 80091054: type twice as likely
int getTourneyFish(const dTime_c &time);                                // 80091200
int getRandomByRarity(const dTime_c &time, int rarity, int maxRarity, BOOL allDay); // 80091370: -1 if none
u8 getRandomOfTerm();                                                   // 80091518: any fish of the term (6 hours ago)
const char *getModelName(int type);                                     // 8009166C
s16 getLength(int type);                                                // 80091680: in cm
BOOL isLarge(int type);                                                 // 80091694
f32 getHoldParam(int type);                                             // 800916D8: used by the player holding it
f32 getModelScale(int type);                                            // 800916EC
void initCatchData(dFishCatchData_c *data);                             // 80091700

} // namespace dFishInfo
