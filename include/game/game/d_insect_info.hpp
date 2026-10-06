#pragma once

// Insect data: which insects spawn in each month and time slot (cumulative spawn rates out of
// 1000), the rarity classes, and per-insect tables (model names, ...). The insect counterpart of
// d_fish_info. Source: src/dol/game/d_insect_info.cpp (.text 800BD450..800BDDE0).
// All names are inferred (enum names from the ins_* model names). See notes/d_insect_info.txt.

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/mLib/m_vec.hpp>

// Insect types: the index of every per-insect table.
enum dInsectType_e {
    INSECT_COMMON_BUTTERFLY,        // ins_monsiro
    INSECT_YELLOW_BUTTERFLY,        // ins_monki
    INSECT_TIGER_BUTTERFLY,         // ins_ageha
    INSECT_PEACOCK_BUTTERFLY,       // ins_karasu
    INSECT_MONARCH_BUTTERFLY,       // ins_ookaba
    INSECT_EMPEROR_BUTTERFLY,       // ins_morpho
    INSECT_AGRIAS_BUTTERFLY,        // ins_miiro
    INSECT_RAJA_BROOKE,             // ins_akaeri
    INSECT_BIRDWING_BUTTERFLY,      // ins_alex
    INSECT_MOTH,                    // ins_ga
    INSECT_OAK_SILK_MOTH,           // ins_ymmga
    INSECT_HONEYBEE,                // ins_mitsu
    INSECT_BEE,                     // ins_suzume
    INSECT_LONG_LOCUST,             // ins_syouryou
    INSECT_MIGRATORY_LOCUST,        // ins_tonosama
    INSECT_MANTIS,                  // ins_kama
    INSECT_ORCHID_MANTIS,           // ins_hanakama
    INSECT_BROWN_CICADA,            // ins_abura
    INSECT_ROBUST_CICADA,           // ins_minmin
    INSECT_WALKER_CICADA,           // ins_tsuku
    INSECT_EVENING_CICADA,          // ins_higu
    INSECT_LANTERN_FLY,             // ins_biwa
    INSECT_RED_DRAGONFLY,           // ins_akane
    INSECT_DARNER_DRAGONFLY,        // ins_ginyan
    INSECT_BANDED_DRAGONFLY,        // ins_oniyan
    INSECT_GIANT_PETALTAIL_DRAGONFLY, // ins_mukashi
    INSECT_ANT,                     // ins_ari
    INSECT_POND_SKATER,             // ins_amenbo
    INSECT_DIVING_BEETLE,           // ins_gengorou
    INSECT_SNAIL,                   // ins_kata
    INSECT_CRICKET,                 // ins_koorogi
    INSECT_BELL_CRICKET,            // ins_suzu
    INSECT_GRASSHOPPER,             // ins_kirigiri
    INSECT_MOLE_CRICKET,            // ins_okera
    INSECT_WALKING_LEAF,            // ins_konoha
    INSECT_WALKING_STICK,           // ins_nana
    INSECT_BAGWORM,                 // ins_mino
    INSECT_LADYBUG,                 // ins_tentou
    INSECT_VIOLIN_BEETLE,           // ins_violin
    INSECT_LONGHORN_BEETLE,         // ins_kamikiri
    INSECT_DUNG_BEETLE,             // ins_hunkoro
    INSECT_FIREFLY,                 // ins_hotaru
    INSECT_DRONE_BEETLE,            // ins_kogane
    INSECT_SCARAB_BEETLE,           // ins_plakoga
    INSECT_JEWEL_BEETLE,            // ins_tama
    INSECT_MIYAMA_STAG,             // ins_miyakwa
    INSECT_SAW_STAG,                // ins_nokokwa
    INSECT_GIANT_STAG,              // ins_ookwa
    INSECT_RAINBOW_STAG,            // ins_nijikwa
    INSECT_CYCLOMMATUS_STAG,        // ins_hosokwa
    INSECT_GOLDEN_STAG,             // ins_ougon
    INSECT_DYNASTID_BEETLE,         // ins_kbt
    INSECT_ATLAS_BEETLE,            // ins_kokbt
    INSECT_ELEPHANT_BEETLE,         // ins_zokbt
    INSECT_HERCULES_BEETLE,         // ins_helkbt
    INSECT_GOLIATH_BEETLE,          // ins_gorias
    INSECT_FLEA,                    // ins_nomi
    INSECT_PILL_BUG,                // ins_dango
    INSECT_MOSQUITO,                // ins_ka
    INSECT_FLY,                     // ins_hae
    INSECT_CENTIPEDE,               // ins_mukade
    INSECT_SPIDER,                  // ins_kumo
    INSECT_TARANTULA,               // ins_taran
    INSECT_SCORPION,                // ins_sasori

    INSECT_NUM // 0x40
};

// Time slots (dInsectInfo::getTimeSlot).
enum dInsectTime_e {
    INSECT_TIME_NIGHT,     // 23:00..3:59
    INSECT_TIME_MORNING,   // 4:00..7:59
    INSECT_TIME_DAY,       // 8:00..15:59
    INSECT_TIME_AFTERNOON, // 16:00..16:59
    INSECT_TIME_EVENING,   // 17:00..18:59
    INSECT_TIME_LATE,      // 19:00..22:59

    INSECT_TIME_NUM
};

// One bit per dInsectTime_e (dInsectInfo::getActiveTimeMask). The insect museum REL tests the bit of
// the current slot to show an insect as active.
#define INSECT_TIME_MASK(slot) (1 << (slot))

// Rarity classes by spawn rate (the entry's own share, out of 1000).
enum dInsectRarity_e {
    INSECT_RARITY_COMMON, // 150..1000
    INSECT_RARITY_1,      // 100..149
    INSECT_RARITY_2,      // 40..99
    INSECT_RARITY_3,      // 20..39
    INSECT_RARITY_RARE,   // 0..19

    INSECT_RARITY_NUM,
    INSECT_RARITY_SAME = INSECT_RARITY_NUM, // dInsectInfo::getRandomByRarity: max class = min class
};

// One spawn list entry.
struct dInsectSpawn_c {
    /* 0x0 */ s16 mType; // dInsectType_e
    /* 0x2 */ s16 mRate; // cumulative spawn rate out of 1000
}; // size 0x4

struct dInsectSpawnList_c {
    /* 0x0 */ dInsectSpawn_c *mList;
    /* 0x4 */ int mNum;
}; // size 0x8

// One of the 12 insect slots shared with the other players while online (net data ids 0x10..0x1B,
// cleared through the init table at 80474338). d_insect_fieldNP keeps the 12 slots at +0x787 of its
// manager, writes one when it spawns an insect and empties it when the insect goes away.
// The fields are packed LSB-first across the bytes (game code shifts and masks them by hand):
//   bits  0..6  type (dInsectType_e; INSECT_NET_TYPE_NONE = empty slot)
//   bits  7..15 serial, 9 bits, +1 every time the slot is reused (matches the actor to the slot)
//   bits 16..19 unknown, cleared when the slot is emptied
//   bits 20..29 x / 4 (world units)
//   bits 30..39 z / 4
//   bits 40..42 state (INSECT_NET_STATE_*)
//   bits 43..47 unused
struct dInsectCatchData_c {
    /* 0x0 */ u8 mTypeSerial; // type (INSECT_NET_TYPE_MASK), serial bit 0 (INSECT_NET_SERIAL_LO)
    /* 0x1 */ u8 mSerial;     // serial bits 1..8
    /* 0x2 */ u8 mPosX;       // bits 0..3 unknown, bits 4..7 x/4 bits 0..3
    /* 0x3 */ u8 mPosXZ;      // bits 0..5 x/4 bits 4..9, bits 6..7 z/4 bits 0..1
    /* 0x4 */ u8 mPosZ;       // z/4 bits 2..9
    /* 0x5 */ u8 mState;      // INSECT_NET_STATE_MASK
}; // size 0x6

#define INSECT_NET_SLOT_NUM 12
#define INSECT_NET_TYPE_MASK 0x7F
#define INSECT_NET_TYPE_NONE INSECT_NUM // empty slot
#define INSECT_NET_SERIAL_LO 0x80       // mTypeSerial: serial bit 0
#define INSECT_NET_STATE_MASK 0x07
// The only state value the game writes (init, spawn, removal); another player's catch request is
// only accepted in this state (d_insect_fieldNP fn_116_525C).
#define INSECT_NET_STATE_NORMAL 4

namespace dInsectInfo {

dInsectSpawnList_c *getSpawnList(int month, int slot);                 // 800BD450
dInsectSpawnList_c *getSpawnList(int slot);                            // 800BD46C: this month
dInsectSpawnList_c *getSpawnList();                                    // 800BD4FC: now
BOOL isFleaSpawn();                                                    // 800BD58C: offline and the roll is a flea
BOOL isOfTerm(int type);                                               // 800BD5E0: picked by getRandomOfTerm
dInsectSpawn_c *getRandomSpawn();                                      // 800BD610
int getRandomByRarity(const dTime_c &time, int rarity, int maxRarity, BOOL allDay); // 800BD6A8: -1 if none
int getRandomNpcCatch(const dTime_c &time, int rarity, int maxRarity, BOOL allDay); // 800BD8F8: getRandomByRarity, -1 unless isNpcCatchable (villager Bug-Off entries)
u8 getRandomOfTerm();                                                  // 800BD938: any insect of the month (6 hours ago)
u8 getBugOffScore(int type);                                           // 800BDA74: 5..100
BOOL isNpcCatchable(int type);                                         // 800BDA84: FALSE for insects with a special spawn spot
u8 getBugOffScore(const dItem::Item *item);                            // 800BDA94: of an insect item (Bug-Off host REL)
const char *getModelName(int type);                                    // 800BDAC8
void setAntsAttracted();                                               // 800BDADC: candy or spoiled turnips lying around
u8 isAntsAttracted();                                                  // 800BDAE8: d_insect_fieldNP spawns an ant
void clearAntsAttracted();                                             // 800BDAF0
void setFliesAttracted();                                              // 800BDAFC: trash or spoiled turnips lying around
u8 isFliesAttracted();                                                 // 800BDB08: d_insect_fieldNP spawns a fly
void clearFliesAttracted();                                            // 800BDB10
void setPos(const mVec3_c &pos);                                       // 800BDB1C: dFgMngProc_c::setUnitItem
u8 hasPos();                                                           // 800BDB48
mVec3_c *getPos();                                                     // 800BDB50
void clearPos();                                                       // 800BDB5C
u8 getActiveTimeMask(int type);                                        // 800BDB68: INSECT_TIME_MASK(slot) bits
BOOL canSpawnInRain(int type);                                         // 800BDB78: d_insect_fieldNP skips the rest while raining
BOOL canSpawnInSnow(int type);                                         // 800BDC70: d_insect_fieldNP skips the rest while snowing
void initCatchData(dInsectCatchData_c *data);                          // 800BDD68: empty net slot (type none, state normal)

} // namespace dInsectInfo
