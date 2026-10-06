// Insect data. See include/game/game/d_insect_info.hpp and notes/d_insect_info.txt.
// .text 800BD450..800BDDE0, .ctors 804656B0..804656B4, .rodata 80471A00..80471B40,
// .data 804E7478..804E89A8, .bss 8059AD80..8059AD98, .sdata 8074A4F8..8074A520,
// .sbss 8074E488..8074E490, .sdata2 807507B0..807507C8.
#include <game/game/d_insect_info.hpp>
#include <game/game/d_search_cand.hpp>
#include <game/game/d_item.hpp>
#include <game/cLib/c_math.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/sLib/s_lib.hpp>

// Not split yet (C linkage keeps the target name).
extern "C" BOOL fn_800DCEDC(); // 800DCEDC: an online session is active

// 80471A00: the time slots each insect is active in, INSECT_TIME_MASK bits (e.g. 0x0C for 8:00..16:59,
// 0x21 for 19:00..3:59, 0x3F all day). Read by the insect museum (d_insect_museumNP).
static const u8 sActiveTimeMask[INSECT_NUM] = {
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x21, 0x21, 0x0C, 0x0E, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x1A, 0x3F, 0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1C, 0x1C, 0x1C, 0x33, 0x33,
    0x0C, 0x3F, 0x0C, 0x1E, 0x38, 0x0C, 0x33, 0x1E, 0x23, 0x21, 0x33, 0x03, 0x0C, 0x23, 0x23, 0x23,
    0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x21, 0x21,
};

// 804E7478..804E83D0: spawn lists per month and time slot, then per month the slot table.
static dInsectSpawn_c sJan_0[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 100},
    {INSECT_PILL_BUG, 250},
};
static dInsectSpawn_c sJan_1[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 100},
    {INSECT_PILL_BUG, 250},
};
static dInsectSpawn_c sJan_2[4] = { // one more slot than the list uses
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_PILL_BUG, 230},
};
static dInsectSpawn_c sJan_3[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_PILL_BUG, 230},
    {INSECT_CENTIPEDE, 280},
};
static dInsectSpawn_c sJan_4[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 90},
    {INSECT_PILL_BUG, 240},
    {INSECT_CENTIPEDE, 290},
};
static dInsectSpawn_c sJan_5[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 90},
    {INSECT_PILL_BUG, 240},
    {INSECT_CENTIPEDE, 290},
};
static dInsectSpawn_c sFeb_0[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 100},
    {INSECT_PILL_BUG, 250},
};
static dInsectSpawn_c sFeb_1[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 100},
    {INSECT_PILL_BUG, 250},
};
static dInsectSpawn_c sFeb_2[4] = { // one more slot than the list uses
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_PILL_BUG, 230},
};
static dInsectSpawn_c sFeb_3[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_PILL_BUG, 230},
    {INSECT_CENTIPEDE, 280},
};
static dInsectSpawn_c sFeb_4[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 90},
    {INSECT_PILL_BUG, 240},
    {INSECT_CENTIPEDE, 290},
};
static dInsectSpawn_c sFeb_5[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 90},
    {INSECT_PILL_BUG, 240},
    {INSECT_CENTIPEDE, 290},
};
static dInsectSpawn_c sMar_0[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_FLEA, 40},
    {INSECT_PILL_BUG, 190},
    {INSECT_SPIDER, 240},
};
static dInsectSpawn_c sMar_1[] = {
    {INSECT_COMMON_BUTTERFLY, 40},
    {INSECT_YELLOW_BUTTERFLY, 80},
    {INSECT_TIGER_BUTTERFLY, 100},
    {INSECT_PEACOCK_BUTTERFLY, 120},
    {INSECT_MOLE_CRICKET, 150},
    {INSECT_FLEA, 160},
    {INSECT_PILL_BUG, 310},
    {INSECT_SPIDER, 360},
};
static dInsectSpawn_c sMar_2[] = {
    {INSECT_COMMON_BUTTERFLY, 120},
    {INSECT_YELLOW_BUTTERFLY, 240},
    {INSECT_TIGER_BUTTERFLY, 300},
    {INSECT_PEACOCK_BUTTERFLY, 350},
    {INSECT_HONEYBEE, 450},
    {INSECT_MOLE_CRICKET, 480},
    {INSECT_LADYBUG, 600},
    {INSECT_FLEA, 610},
    {INSECT_PILL_BUG, 760},
    {INSECT_SPIDER, 810},
};
static dInsectSpawn_c sMar_3[] = {
    {INSECT_COMMON_BUTTERFLY, 120},
    {INSECT_YELLOW_BUTTERFLY, 240},
    {INSECT_TIGER_BUTTERFLY, 300},
    {INSECT_PEACOCK_BUTTERFLY, 350},
    {INSECT_HONEYBEE, 410},
    {INSECT_MOLE_CRICKET, 440},
    {INSECT_LADYBUG, 520},
    {INSECT_FLEA, 530},
    {INSECT_PILL_BUG, 680},
    {INSECT_SPIDER, 730},
};
static dInsectSpawn_c sMar_4[] = {
    {INSECT_COMMON_BUTTERFLY, 40},
    {INSECT_YELLOW_BUTTERFLY, 80},
    {INSECT_TIGER_BUTTERFLY, 100},
    {INSECT_PEACOCK_BUTTERFLY, 120},
    {INSECT_MOLE_CRICKET, 150},
    {INSECT_FLEA, 160},
    {INSECT_PILL_BUG, 310},
    {INSECT_SPIDER, 360},
};
static dInsectSpawn_c sMar_5[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_FLEA, 40},
    {INSECT_PILL_BUG, 190},
    {INSECT_SPIDER, 240},
};
static dInsectSpawn_c sApr_0[] = {
    {INSECT_SNAIL, 40},
    {INSECT_MOLE_CRICKET, 70},
    {INSECT_FLEA, 80},
    {INSECT_PILL_BUG, 170},
    {INSECT_SPIDER, 260},
};
static dInsectSpawn_c sApr_1[] = {
    {INSECT_COMMON_BUTTERFLY, 50},
    {INSECT_YELLOW_BUTTERFLY, 100},
    {INSECT_TIGER_BUTTERFLY, 130},
    {INSECT_PEACOCK_BUTTERFLY, 160},
    {INSECT_SNAIL, 200},
    {INSECT_MOLE_CRICKET, 230},
    {INSECT_FLEA, 240},
    {INSECT_PILL_BUG, 330},
    {INSECT_SPIDER, 420},
};
static dInsectSpawn_c sApr_2[] = {
    {INSECT_COMMON_BUTTERFLY, 220},
    {INSECT_YELLOW_BUTTERFLY, 440},
    {INSECT_TIGER_BUTTERFLY, 520},
    {INSECT_PEACOCK_BUTTERFLY, 570},
    {INSECT_HONEYBEE, 690},
    {INSECT_MANTIS, 740},
    {INSECT_ORCHID_MANTIS, 750},
    {INSECT_SNAIL, 780},
    {INSECT_MOLE_CRICKET, 810},
    {INSECT_LADYBUG, 930},
    {INSECT_FLEA, 940},
    {INSECT_PILL_BUG, 950},
    {INSECT_SPIDER, 1000},
};
static dInsectSpawn_c sApr_3[] = {
    {INSECT_COMMON_BUTTERFLY, 200},
    {INSECT_YELLOW_BUTTERFLY, 400},
    {INSECT_TIGER_BUTTERFLY, 480},
    {INSECT_PEACOCK_BUTTERFLY, 530},
    {INSECT_HONEYBEE, 610},
    {INSECT_MANTIS, 660},
    {INSECT_ORCHID_MANTIS, 670},
    {INSECT_SNAIL, 710},
    {INSECT_MOLE_CRICKET, 740},
    {INSECT_LADYBUG, 840},
    {INSECT_FLEA, 850},
    {INSECT_PILL_BUG, 920},
    {INSECT_SPIDER, 1000},
};
static dInsectSpawn_c sApr_4[] = {
    {INSECT_COMMON_BUTTERFLY, 50},
    {INSECT_YELLOW_BUTTERFLY, 100},
    {INSECT_TIGER_BUTTERFLY, 130},
    {INSECT_PEACOCK_BUTTERFLY, 160},
    {INSECT_SNAIL, 200},
    {INSECT_MOLE_CRICKET, 230},
    {INSECT_FLEA, 240},
    {INSECT_PILL_BUG, 330},
    {INSECT_SPIDER, 420},
};
static dInsectSpawn_c sApr_5[] = {
    {INSECT_SNAIL, 40},
    {INSECT_MOLE_CRICKET, 70},
    {INSECT_FLEA, 80},
    {INSECT_PILL_BUG, 170},
    {INSECT_SPIDER, 260},
};
static dInsectSpawn_c sMay_0[] = {
    {INSECT_MOTH, 50},
    {INSECT_SNAIL, 110},
    {INSECT_MOLE_CRICKET, 140},
    {INSECT_FLEA, 150},
    {INSECT_PILL_BUG, 240},
    {INSECT_SPIDER, 330},
};
static dInsectSpawn_c sMay_1[] = {
    {INSECT_COMMON_BUTTERFLY, 50},
    {INSECT_YELLOW_BUTTERFLY, 100},
    {INSECT_TIGER_BUTTERFLY, 130},
    {INSECT_PEACOCK_BUTTERFLY, 160},
    {INSECT_SNAIL, 220},
    {INSECT_MOLE_CRICKET, 250},
    {INSECT_FLEA, 260},
    {INSECT_PILL_BUG, 350},
    {INSECT_SPIDER, 440},
};
static dInsectSpawn_c sMay_2[] = {
    {INSECT_COMMON_BUTTERFLY, 180},
    {INSECT_YELLOW_BUTTERFLY, 360},
    {INSECT_TIGER_BUTTERFLY, 490},
    {INSECT_PEACOCK_BUTTERFLY, 570},
    {INSECT_RAJA_BROOKE, 580},
    {INSECT_HONEYBEE, 680},
    {INSECT_LONG_LOCUST, 700},
    {INSECT_MANTIS, 750},
    {INSECT_ORCHID_MANTIS, 760},
    {INSECT_POND_SKATER, 780},
    {INSECT_DIVING_BEETLE, 790},
    {INSECT_SNAIL, 830},
    {INSECT_MOLE_CRICKET, 850},
    {INSECT_LADYBUG, 950},
    {INSECT_FLEA, 960},
    {INSECT_PILL_BUG, 970},
    {INSECT_SPIDER, 1000},
};
static dInsectSpawn_c sMay_3[18] = { // one more slot than the list uses
    {INSECT_COMMON_BUTTERFLY, 180},
    {INSECT_YELLOW_BUTTERFLY, 330},
    {INSECT_TIGER_BUTTERFLY, 430},
    {INSECT_PEACOCK_BUTTERFLY, 490},
    {INSECT_RAJA_BROOKE, 500},
    {INSECT_HONEYBEE, 600},
    {INSECT_LONG_LOCUST, 620},
    {INSECT_MANTIS, 670},
    {INSECT_ORCHID_MANTIS, 680},
    {INSECT_POND_SKATER, 700},
    {INSECT_DIVING_BEETLE, 710},
    {INSECT_SNAIL, 770},
    {INSECT_MOLE_CRICKET, 790},
    {INSECT_LADYBUG, 890},
    {INSECT_FLEA, 900},
    {INSECT_PILL_BUG, 910},
    {INSECT_SPIDER, 980},
};
static dInsectSpawn_c sMay_4[] = {
    {INSECT_COMMON_BUTTERFLY, 50},
    {INSECT_YELLOW_BUTTERFLY, 100},
    {INSECT_TIGER_BUTTERFLY, 130},
    {INSECT_PEACOCK_BUTTERFLY, 160},
    {INSECT_LONG_LOCUST, 210},
    {INSECT_SNAIL, 270},
    {INSECT_MOLE_CRICKET, 300},
    {INSECT_FLEA, 310},
    {INSECT_PILL_BUG, 400},
    {INSECT_SPIDER, 490},
};
static dInsectSpawn_c sMay_5[] = {
    {INSECT_MOTH, 30},
    {INSECT_SNAIL, 90},
    {INSECT_MOLE_CRICKET, 120},
    {INSECT_FLEA, 130},
    {INSECT_PILL_BUG, 220},
    {INSECT_SPIDER, 310},
};
static dInsectSpawn_c sJun_0[] = {
    {INSECT_EMPEROR_BUTTERFLY, 10},
    {INSECT_MOTH, 170},
    {INSECT_OAK_SILK_MOTH, 190},
    {INSECT_LANTERN_FLY, 220},
    {INSECT_SNAIL, 260},
    {INSECT_FIREFLY, 760},
    {INSECT_RAINBOW_STAG, 770},
    {INSECT_GOLIATH_BEETLE, 780},
    {INSECT_FLEA, 790},
    {INSECT_PILL_BUG, 840},
    {INSECT_MOSQUITO, 880},
    {INSECT_SPIDER, 930},
    {INSECT_TARANTULA, 935},
};
static dInsectSpawn_c sJun_1[] = {
    {INSECT_COMMON_BUTTERFLY, 50},
    {INSECT_YELLOW_BUTTERFLY, 100},
    {INSECT_TIGER_BUTTERFLY, 130},
    {INSECT_PEACOCK_BUTTERFLY, 160},
    {INSECT_EMPEROR_BUTTERFLY, 170},
    {INSECT_LANTERN_FLY, 200},
    {INSECT_SNAIL, 350},
    {INSECT_VIOLIN_BEETLE, 390},
    {INSECT_RAINBOW_STAG, 400},
    {INSECT_GOLIATH_BEETLE, 410},
    {INSECT_FLEA, 420},
    {INSECT_PILL_BUG, 510},
    {INSECT_SPIDER, 600},
};
static dInsectSpawn_c sJun_2[] = {
    {INSECT_COMMON_BUTTERFLY, 140},
    {INSECT_YELLOW_BUTTERFLY, 280},
    {INSECT_TIGER_BUTTERFLY, 410},
    {INSECT_PEACOCK_BUTTERFLY, 500},
    {INSECT_AGRIAS_BUTTERFLY, 510},
    {INSECT_RAJA_BROOKE, 520},
    {INSECT_BIRDWING_BUTTERFLY, 525},
    {INSECT_HONEYBEE, 625},
    {INSECT_LONG_LOCUST, 645},
    {INSECT_MANTIS, 665},
    {INSECT_ORCHID_MANTIS, 675},
    {INSECT_COMMON_DRAGONFLY, 735},
    {INSECT_POND_SKATER, 775},
    {INSECT_DIVING_BEETLE, 795},
    {INSECT_SNAIL, 875},
    {INSECT_LADYBUG, 915},
    {INSECT_VIOLIN_BEETLE, 955},
    {INSECT_FLEA, 965},
    {INSECT_PILL_BUG, 975},
    {INSECT_SPIDER, 995},
};
static dInsectSpawn_c sJun_3[] = {
    {INSECT_COMMON_BUTTERFLY, 140},
    {INSECT_YELLOW_BUTTERFLY, 280},
    {INSECT_TIGER_BUTTERFLY, 380},
    {INSECT_PEACOCK_BUTTERFLY, 460},
    {INSECT_AGRIAS_BUTTERFLY, 470},
    {INSECT_RAJA_BROOKE, 480},
    {INSECT_HONEYBEE, 550},
    {INSECT_LONG_LOCUST, 570},
    {INSECT_MANTIS, 590},
    {INSECT_ORCHID_MANTIS, 600},
    {INSECT_LANTERN_FLY, 610},
    {INSECT_COMMON_DRAGONFLY, 730},
    {INSECT_POND_SKATER, 770},
    {INSECT_DIVING_BEETLE, 790},
    {INSECT_SNAIL, 870},
    {INSECT_LADYBUG, 910},
    {INSECT_VIOLIN_BEETLE, 950},
    {INSECT_FLEA, 960},
    {INSECT_PILL_BUG, 970},
    {INSECT_CENTIPEDE, 990},
    {INSECT_SPIDER, 1000},
};
static dInsectSpawn_c sJun_4[] = {
    {INSECT_COMMON_BUTTERFLY, 50},
    {INSECT_YELLOW_BUTTERFLY, 100},
    {INSECT_TIGER_BUTTERFLY, 130},
    {INSECT_PEACOCK_BUTTERFLY, 160},
    {INSECT_RAJA_BROOKE, 170},
    {INSECT_LONG_LOCUST, 220},
    {INSECT_LANTERN_FLY, 250},
    {INSECT_POND_SKATER, 350},
    {INSECT_SNAIL, 450},
    {INSECT_VIOLIN_BEETLE, 490},
    {INSECT_FLEA, 500},
    {INSECT_PILL_BUG, 590},
    {INSECT_MOSQUITO, 790},
    {INSECT_CENTIPEDE, 810},
    {INSECT_SPIDER, 900},
};
static dInsectSpawn_c sJun_5[] = {
    {INSECT_EMPEROR_BUTTERFLY, 10},
    {INSECT_MOTH, 110},
    {INSECT_OAK_SILK_MOTH, 120},
    {INSECT_SNAIL, 160},
    {INSECT_FIREFLY, 760},
    {INSECT_RAINBOW_STAG, 770},
    {INSECT_GOLIATH_BEETLE, 780},
    {INSECT_FLEA, 790},
    {INSECT_PILL_BUG, 840},
    {INSECT_MOSQUITO, 910},
    {INSECT_CENTIPEDE, 930},
    {INSECT_SPIDER, 980},
    {INSECT_TARANTULA, 985},
};
static dInsectSpawn_c sJul_0[] = {
    {INSECT_EMPEROR_BUTTERFLY, 20},
    {INSECT_MOTH, 180},
    {INSECT_OAK_SILK_MOTH, 200},
    {INSECT_LANTERN_FLY, 210},
    {INSECT_SNAIL, 260},
    {INSECT_LONGHORN_BEETLE, 310},
    {INSECT_DRONE_BEETLE, 460},
    {INSECT_SCARAB_BEETLE, 480},
    {INSECT_MIYAMA_STAG, 530},
    {INSECT_SAW_STAG, 560},
    {INSECT_GIANT_STAG, 580},
    {INSECT_RAINBOW_STAG, 590},
    {INSECT_CYCLOMMATUS_STAG, 600},
    {INSECT_GOLDEN_STAG, 605},
    {INSECT_DYNASTID_BEETLE, 675},
    {INSECT_ATLAS_BEETLE, 685},
    {INSECT_ELEPHANT_BEETLE, 695},
    {INSECT_HERCULES_BEETLE, 700},
    {INSECT_GOLIATH_BEETLE, 720},
    {INSECT_FLEA, 730},
    {INSECT_PILL_BUG, 750},
    {INSECT_MOSQUITO, 770},
    {INSECT_SPIDER, 820},
    {INSECT_TARANTULA, 825},
    {INSECT_SCORPION, 830},
};
static dInsectSpawn_c sJul_1[] = {
    {INSECT_PEACOCK_BUTTERFLY, 10},
    {INSECT_EMPEROR_BUTTERFLY, 50},
    {INSECT_EVENING_CICADA, 300},
    {INSECT_LANTERN_FLY, 320},
    {INSECT_SNAIL, 370},
    {INSECT_WALKING_STICK, 390},
    {INSECT_LONGHORN_BEETLE, 440},
    {INSECT_DRONE_BEETLE, 590},
    {INSECT_SCARAB_BEETLE, 610},
    {INSECT_MIYAMA_STAG, 660},
    {INSECT_SAW_STAG, 690},
    {INSECT_GIANT_STAG, 710},
    {INSECT_RAINBOW_STAG, 720},
    {INSECT_CYCLOMMATUS_STAG, 730},
    {INSECT_GOLDEN_STAG, 735},
    {INSECT_DYNASTID_BEETLE, 805},
    {INSECT_ATLAS_BEETLE, 815},
    {INSECT_ELEPHANT_BEETLE, 825},
    {INSECT_HERCULES_BEETLE, 830},
    {INSECT_GOLIATH_BEETLE, 840},
    {INSECT_FLEA, 850},
    {INSECT_SPIDER, 930},
};
static dInsectSpawn_c sJul_2[] = {
    {INSECT_TIGER_BUTTERFLY, 10},
    {INSECT_PEACOCK_BUTTERFLY, 20},
    {INSECT_AGRIAS_BUTTERFLY, 30},
    {INSECT_RAJA_BROOKE, 40},
    {INSECT_BIRDWING_BUTTERFLY, 45},
    {INSECT_HONEYBEE, 65},
    {INSECT_LONG_LOCUST, 85},
    {INSECT_MANTIS, 95},
    {INSECT_ORCHID_MANTIS, 105},
    {INSECT_BROWN_CICADA, 405},
    {INSECT_ROBUST_CICADA, 705},
    {INSECT_WALKER_CICADA, 755},
    {INSECT_COMMON_DRAGONFLY, 795},
    {INSECT_DARNER_DRAGONFLY, 805},
    {INSECT_POND_SKATER, 845},
    {INSECT_DIVING_BEETLE, 865},
    {INSECT_SNAIL, 895},
    {INSECT_WALKING_LEAF, 900},
    {INSECT_LONGHORN_BEETLE, 920},
    {INSECT_DRONE_BEETLE, 950},
    {INSECT_JEWEL_BEETLE, 970},
    {INSECT_MIYAMA_STAG, 980},
    {INSECT_SAW_STAG, 990},
    {INSECT_FLEA, 1000},
};
static dInsectSpawn_c sJul_3[] = {
    {INSECT_TIGER_BUTTERFLY, 20},
    {INSECT_PEACOCK_BUTTERFLY, 40},
    {INSECT_AGRIAS_BUTTERFLY, 50},
    {INSECT_RAJA_BROOKE, 70},
    {INSECT_LONG_LOCUST, 90},
    {INSECT_MANTIS, 100},
    {INSECT_ORCHID_MANTIS, 110},
    {INSECT_BROWN_CICADA, 310},
    {INSECT_ROBUST_CICADA, 410},
    {INSECT_WALKER_CICADA, 460},
    {INSECT_EVENING_CICADA, 660},
    {INSECT_LANTERN_FLY, 670},
    {INSECT_COMMON_DRAGONFLY, 700},
    {INSECT_DARNER_DRAGONFLY, 720},
    {INSECT_POND_SKATER, 760},
    {INSECT_DIVING_BEETLE, 770},
    {INSECT_SNAIL, 800},
    {INSECT_GRASSHOPPER, 840},
    {INSECT_WALKING_LEAF, 850},
    {INSECT_LONGHORN_BEETLE, 870},
    {INSECT_DRONE_BEETLE, 910},
    {INSECT_JEWEL_BEETLE, 940},
    {INSECT_MIYAMA_STAG, 970},
    {INSECT_SAW_STAG, 980},
    {INSECT_FLEA, 990},
    {INSECT_CENTIPEDE, 1000},
};
static dInsectSpawn_c sJul_4[] = {
    {INSECT_LONG_LOCUST, 20},
    {INSECT_EVENING_CICADA, 420},
    {INSECT_LANTERN_FLY, 440},
    {INSECT_POND_SKATER, 490},
    {INSECT_SNAIL, 530},
    {INSECT_WALKING_STICK, 550},
    {INSECT_LONGHORN_BEETLE, 570},
    {INSECT_DRONE_BEETLE, 670},
    {INSECT_MIYAMA_STAG, 690},
    {INSECT_SAW_STAG, 700},
    {INSECT_RAINBOW_STAG, 705},
    {INSECT_CYCLOMMATUS_STAG, 710},
    {INSECT_GOLDEN_STAG, 715},
    {INSECT_DYNASTID_BEETLE, 745},
    {INSECT_ATLAS_BEETLE, 750},
    {INSECT_ELEPHANT_BEETLE, 755},
    {INSECT_HERCULES_BEETLE, 760},
    {INSECT_GOLIATH_BEETLE, 770},
    {INSECT_FLEA, 780},
    {INSECT_MOSQUITO, 980},
    {INSECT_CENTIPEDE, 1000},
};
static dInsectSpawn_c sJul_5[] = {
    {INSECT_EMPEROR_BUTTERFLY, 10},
    {INSECT_MOTH, 160},
    {INSECT_OAK_SILK_MOTH, 170},
    {INSECT_LANTERN_FLY, 180},
    {INSECT_SNAIL, 240},
    {INSECT_DRONE_BEETLE, 390},
    {INSECT_MIYAMA_STAG, 440},
    {INSECT_SAW_STAG, 470},
    {INSECT_RAINBOW_STAG, 480},
    {INSECT_CYCLOMMATUS_STAG, 490},
    {INSECT_GOLDEN_STAG, 495},
    {INSECT_DYNASTID_BEETLE, 545},
    {INSECT_ATLAS_BEETLE, 555},
    {INSECT_ELEPHANT_BEETLE, 565},
    {INSECT_HERCULES_BEETLE, 570},
    {INSECT_GOLIATH_BEETLE, 590},
    {INSECT_FLEA, 600},
    {INSECT_PILL_BUG, 680},
    {INSECT_MOSQUITO, 880},
    {INSECT_CENTIPEDE, 910},
    {INSECT_SPIDER, 990},
    {INSECT_TARANTULA, 995},
    {INSECT_SCORPION, 1000},
};
static dInsectSpawn_c sAug_0[26] = { // one more slot than the list uses
    {INSECT_EMPEROR_BUTTERFLY, 10},
    {INSECT_MOTH, 170},
    {INSECT_OAK_SILK_MOTH, 190},
    {INSECT_LANTERN_FLY, 220},
    {INSECT_SNAIL, 270},
    {INSECT_LONGHORN_BEETLE, 320},
    {INSECT_DRONE_BEETLE, 470},
    {INSECT_SCARAB_BEETLE, 490},
    {INSECT_MIYAMA_STAG, 540},
    {INSECT_SAW_STAG, 570},
    {INSECT_GIANT_STAG, 590},
    {INSECT_RAINBOW_STAG, 600},
    {INSECT_CYCLOMMATUS_STAG, 610},
    {INSECT_GOLDEN_STAG, 615},
    {INSECT_DYNASTID_BEETLE, 685},
    {INSECT_ATLAS_BEETLE, 695},
    {INSECT_ELEPHANT_BEETLE, 705},
    {INSECT_HERCULES_BEETLE, 710},
    {INSECT_GOLIATH_BEETLE, 730},
    {INSECT_FLEA, 740},
    {INSECT_PILL_BUG, 760},
    {INSECT_MOSQUITO, 780},
    {INSECT_SPIDER, 830},
    {INSECT_TARANTULA, 835},
    {INSECT_SCORPION, 840},
};
static dInsectSpawn_c sAug_1[] = {
    {INSECT_EMPEROR_BUTTERFLY, 40},
    {INSECT_EVENING_CICADA, 290},
    {INSECT_LANTERN_FLY, 320},
    {INSECT_SNAIL, 370},
    {INSECT_WALKING_STICK, 390},
    {INSECT_LONGHORN_BEETLE, 440},
    {INSECT_DRONE_BEETLE, 590},
    {INSECT_SCARAB_BEETLE, 610},
    {INSECT_MIYAMA_STAG, 680},
    {INSECT_SAW_STAG, 730},
    {INSECT_GIANT_STAG, 750},
    {INSECT_RAINBOW_STAG, 760},
    {INSECT_CYCLOMMATUS_STAG, 780},
    {INSECT_GOLDEN_STAG, 785},
    {INSECT_DYNASTID_BEETLE, 855},
    {INSECT_ATLAS_BEETLE, 865},
    {INSECT_ELEPHANT_BEETLE, 885},
    {INSECT_HERCULES_BEETLE, 890},
    {INSECT_GOLIATH_BEETLE, 910},
    {INSECT_FLEA, 920},
};
static dInsectSpawn_c sAug_2[] = {
    {INSECT_TIGER_BUTTERFLY, 10},
    {INSECT_PEACOCK_BUTTERFLY, 20},
    {INSECT_AGRIAS_BUTTERFLY, 30},
    {INSECT_RAJA_BROOKE, 40},
    {INSECT_BIRDWING_BUTTERFLY, 45},
    {INSECT_LONG_LOCUST, 75},
    {INSECT_MIGRATORY_LOCUST, 95},
    {INSECT_MANTIS, 115},
    {INSECT_ORCHID_MANTIS, 125},
    {INSECT_BROWN_CICADA, 425},
    {INSECT_ROBUST_CICADA, 675},
    {INSECT_WALKER_CICADA, 775},
    {INSECT_COMMON_DRAGONFLY, 805},
    {INSECT_DARNER_DRAGONFLY, 825},
    {INSECT_POND_SKATER, 845},
    {INSECT_DIVING_BEETLE, 855},
    {INSECT_SNAIL, 865},
    {INSECT_GRASSHOPPER, 885},
    {INSECT_WALKING_LEAF, 895},
    {INSECT_LONGHORN_BEETLE, 915},
    {INSECT_DRONE_BEETLE, 950},
    {INSECT_JEWEL_BEETLE, 970},
    {INSECT_MIYAMA_STAG, 980},
    {INSECT_SAW_STAG, 990},
    {INSECT_FLEA, 1000},
};
static dInsectSpawn_c sAug_3[] = {
    {INSECT_TIGER_BUTTERFLY, 20},
    {INSECT_PEACOCK_BUTTERFLY, 40},
    {INSECT_AGRIAS_BUTTERFLY, 50},
    {INSECT_RAJA_BROOKE, 70},
    {INSECT_LONG_LOCUST, 120},
    {INSECT_MIGRATORY_LOCUST, 140},
    {INSECT_MANTIS, 170},
    {INSECT_ORCHID_MANTIS, 190},
    {INSECT_BROWN_CICADA, 240},
    {INSECT_ROBUST_CICADA, 300},
    {INSECT_WALKER_CICADA, 470},
    {INSECT_EVENING_CICADA, 580},
    {INSECT_LANTERN_FLY, 590},
    {INSECT_COMMON_DRAGONFLY, 670},
    {INSECT_DARNER_DRAGONFLY, 690},
    {INSECT_POND_SKATER, 710},
    {INSECT_DIVING_BEETLE, 720},
    {INSECT_SNAIL, 730},
    {INSECT_GRASSHOPPER, 830},
    {INSECT_WALKING_LEAF, 840},
    {INSECT_LONGHORN_BEETLE, 860},
    {INSECT_DRONE_BEETLE, 910},
    {INSECT_JEWEL_BEETLE, 940},
    {INSECT_MIYAMA_STAG, 960},
    {INSECT_SAW_STAG, 970},
    {INSECT_FLEA, 980},
    {INSECT_CENTIPEDE, 1000},
};
static dInsectSpawn_c sAug_4[] = {
    {INSECT_LONG_LOCUST, 20},
    {INSECT_MIGRATORY_LOCUST, 40},
    {INSECT_EVENING_CICADA, 340},
    {INSECT_LANTERN_FLY, 360},
    {INSECT_BANDED_DRAGONFLY, 365},
    {INSECT_POND_SKATER, 415},
    {INSECT_SNAIL, 455},
    {INSECT_WALKING_STICK, 475},
    {INSECT_LONGHORN_BEETLE, 495},
    {INSECT_DRONE_BEETLE, 640},
    {INSECT_MIYAMA_STAG, 670},
    {INSECT_SAW_STAG, 680},
    {INSECT_RAINBOW_STAG, 685},
    {INSECT_CYCLOMMATUS_STAG, 690},
    {INSECT_GOLDEN_STAG, 695},
    {INSECT_DYNASTID_BEETLE, 725},
    {INSECT_ATLAS_BEETLE, 730},
    {INSECT_ELEPHANT_BEETLE, 735},
    {INSECT_HERCULES_BEETLE, 740},
    {INSECT_GOLIATH_BEETLE, 750},
    {INSECT_FLEA, 760},
    {INSECT_MOSQUITO, 980},
    {INSECT_CENTIPEDE, 1000},
};
static dInsectSpawn_c sAug_5[] = {
    {INSECT_EMPEROR_BUTTERFLY, 10},
    {INSECT_MOTH, 140},
    {INSECT_OAK_SILK_MOTH, 150},
    {INSECT_LANTERN_FLY, 160},
    {INSECT_SNAIL, 220},
    {INSECT_DRONE_BEETLE, 370},
    {INSECT_MIYAMA_STAG, 430},
    {INSECT_SAW_STAG, 460},
    {INSECT_RAINBOW_STAG, 470},
    {INSECT_CYCLOMMATUS_STAG, 480},
    {INSECT_GOLDEN_STAG, 485},
    {INSECT_DYNASTID_BEETLE, 535},
    {INSECT_ATLAS_BEETLE, 555},
    {INSECT_ELEPHANT_BEETLE, 565},
    {INSECT_HERCULES_BEETLE, 570},
    {INSECT_GOLIATH_BEETLE, 590},
    {INSECT_FLEA, 600},
    {INSECT_PILL_BUG, 680},
    {INSECT_MOSQUITO, 880},
    {INSECT_CENTIPEDE, 910},
    {INSECT_SPIDER, 990},
    {INSECT_TARANTULA, 995},
    {INSECT_SCORPION, 1000},
};
static dInsectSpawn_c sSep_0[16] = { // one more slot than the list uses
    {INSECT_EMPEROR_BUTTERFLY, 10},
    {INSECT_MOTH, 110},
    {INSECT_OAK_SILK_MOTH, 120},
    {INSECT_LANTERN_FLY, 140},
    {INSECT_SNAIL, 180},
    {INSECT_CRICKET, 510},
    {INSECT_BELL_CRICKET, 760},
    {INSECT_DRONE_BEETLE, 820},
    {INSECT_RAINBOW_STAG, 830},
    {INSECT_GOLIATH_BEETLE, 840},
    {INSECT_FLEA, 850},
    {INSECT_PILL_BUG, 900},
    {INSECT_MOSQUITO, 940},
    {INSECT_SPIDER, 995},
    {INSECT_SCORPION, 1000},
};
static dInsectSpawn_c sSep_1[] = {
    {INSECT_MONARCH_BUTTERFLY, 60},
    {INSECT_EMPEROR_BUTTERFLY, 70},
    {INSECT_LANTERN_FLY, 80},
    {INSECT_SNAIL, 130},
    {INSECT_CRICKET, 430},
    {INSECT_BELL_CRICKET, 650},
    {INSECT_WALKING_STICK, 680},
    {INSECT_VIOLIN_BEETLE, 720},
    {INSECT_DRONE_BEETLE, 780},
    {INSECT_RAINBOW_STAG, 790},
    {INSECT_GOLIATH_BEETLE, 800},
    {INSECT_FLEA, 810},
    {INSECT_PILL_BUG, 900},
    {INSECT_SPIDER, 990},
};
static dInsectSpawn_c sSep_2[] = {
    {INSECT_COMMON_BUTTERFLY, 10},
    {INSECT_YELLOW_BUTTERFLY, 20},
    {INSECT_TIGER_BUTTERFLY, 40},
    {INSECT_PEACOCK_BUTTERFLY, 60},
    {INSECT_MONARCH_BUTTERFLY, 200},
    {INSECT_AGRIAS_BUTTERFLY, 210},
    {INSECT_RAJA_BROOKE, 220},
    {INSECT_BIRDWING_BUTTERFLY, 225},
    {INSECT_LONG_LOCUST, 365},
    {INSECT_MIGRATORY_LOCUST, 465},
    {INSECT_MANTIS, 525},
    {INSECT_ORCHID_MANTIS, 555},
    {INSECT_WALKER_CICADA, 575},
    {INSECT_RED_DRAGONFLY, 635},
    {INSECT_POND_SKATER, 655},
    {INSECT_DIVING_BEETLE, 675},
    {INSECT_SNAIL, 715},
    {INSECT_GRASSHOPPER, 825},
    {INSECT_WALKING_LEAF, 835},
    {INSECT_WALKING_STICK, 855},
    {INSECT_VIOLIN_BEETLE, 895},
    {INSECT_FLEA, 905},
    {INSECT_PILL_BUG, 915},
    {INSECT_SPIDER, 925},
};
static dInsectSpawn_c sSep_3[] = {
    {INSECT_MONARCH_BUTTERFLY, 130},
    {INSECT_AGRIAS_BUTTERFLY, 140},
    {INSECT_RAJA_BROOKE, 160},
    {INSECT_LONG_LOCUST, 230},
    {INSECT_MIGRATORY_LOCUST, 260},
    {INSECT_MANTIS, 300},
    {INSECT_ORCHID_MANTIS, 320},
    {INSECT_WALKER_CICADA, 330},
    {INSECT_LANTERN_FLY, 340},
    {INSECT_RED_DRAGONFLY, 710},
    {INSECT_POND_SKATER, 740},
    {INSECT_DIVING_BEETLE, 750},
    {INSECT_SNAIL, 780},
    {INSECT_GRASSHOPPER, 890},
    {INSECT_WALKING_LEAF, 900},
    {INSECT_WALKING_STICK, 920},
    {INSECT_VIOLIN_BEETLE, 950},
    {INSECT_FLEA, 960},
    {INSECT_PILL_BUG, 970},
    {INSECT_CENTIPEDE, 980},
    {INSECT_SPIDER, 990},
};
static dInsectSpawn_c sSep_4[] = {
    {INSECT_LONG_LOCUST, 100},
    {INSECT_MIGRATORY_LOCUST, 180},
    {INSECT_LANTERN_FLY, 200},
    {INSECT_RED_DRAGONFLY, 250},
    {INSECT_BANDED_DRAGONFLY, 255},
    {INSECT_POND_SKATER, 285},
    {INSECT_SNAIL, 325},
    {INSECT_CRICKET, 575},
    {INSECT_BELL_CRICKET, 775},
    {INSECT_WALKING_STICK, 805},
    {INSECT_FLEA, 815},
    {INSECT_PILL_BUG, 875},
    {INSECT_MOSQUITO, 925},
    {INSECT_CENTIPEDE, 945},
    {INSECT_SPIDER, 985},
};
static dInsectSpawn_c sSep_5[16] = { // one more slot than the list uses
    {INSECT_EMPEROR_BUTTERFLY, 10},
    {INSECT_MOTH, 60},
    {INSECT_OAK_SILK_MOTH, 70},
    {INSECT_SNAIL, 110},
    {INSECT_CRICKET, 460},
    {INSECT_BELL_CRICKET, 710},
    {INSECT_DRONE_BEETLE, 740},
    {INSECT_RAINBOW_STAG, 750},
    {INSECT_GOLIATH_BEETLE, 760},
    {INSECT_FLEA, 770},
    {INSECT_PILL_BUG, 830},
    {INSECT_MOSQUITO, 900},
    {INSECT_CENTIPEDE, 920},
    {INSECT_SPIDER, 980},
    {INSECT_SCORPION, 985},
};
static dInsectSpawn_c sOct_0[] = {
    {INSECT_CRICKET, 320},
    {INSECT_BELL_CRICKET, 540},
    {INSECT_BAGWORM, 590},
    {INSECT_FLEA, 600},
    {INSECT_PILL_BUG, 690},
    {INSECT_SPIDER, 740},
};
static dInsectSpawn_c sOct_1[10] = { // one more slot than the list uses
    {INSECT_MONARCH_BUTTERFLY, 30},
    {INSECT_CRICKET, 280},
    {INSECT_BELL_CRICKET, 440},
    {INSECT_WALKING_STICK, 470},
    {INSECT_BAGWORM, 520},
    {INSECT_VIOLIN_BEETLE, 560},
    {INSECT_FLEA, 570},
    {INSECT_PILL_BUG, 660},
    {INSECT_SPIDER, 710},
};
static dInsectSpawn_c sOct_2[] = {
    {INSECT_MONARCH_BUTTERFLY, 330},
    {INSECT_LONG_LOCUST, 470},
    {INSECT_MIGRATORY_LOCUST, 570},
    {INSECT_MANTIS, 630},
    {INSECT_ORCHID_MANTIS, 660},
    {INSECT_RED_DRAGONFLY, 720},
    {INSECT_WALKING_STICK, 740},
    {INSECT_LADYBUG, 840},
    {INSECT_VIOLIN_BEETLE, 880},
    {INSECT_FLEA, 890},
    {INSECT_PILL_BUG, 910},
    {INSECT_SPIDER, 930},
};
static dInsectSpawn_c sOct_3[] = {
    {INSECT_MONARCH_BUTTERFLY, 250},
    {INSECT_LONG_LOCUST, 300},
    {INSECT_MIGRATORY_LOCUST, 320},
    {INSECT_MANTIS, 360},
    {INSECT_ORCHID_MANTIS, 380},
    {INSECT_RED_DRAGONFLY, 890},
    {INSECT_WALKING_STICK, 910},
    {INSECT_LADYBUG, 930},
    {INSECT_VIOLIN_BEETLE, 960},
    {INSECT_FLEA, 970},
    {INSECT_PILL_BUG, 990},
    {INSECT_CENTIPEDE, 1000},
};
static dInsectSpawn_c sOct_4[12] = { // one more slot than the list uses
    {INSECT_LONG_LOCUST, 90},
    {INSECT_MIGRATORY_LOCUST, 150},
    {INSECT_RED_DRAGONFLY, 380},
    {INSECT_BANDED_DRAGONFLY, 385},
    {INSECT_CRICKET, 635},
    {INSECT_BELL_CRICKET, 795},
    {INSECT_WALKING_STICK, 825},
    {INSECT_BAGWORM, 875},
    {INSECT_FLEA, 885},
    {INSECT_PILL_BUG, 975},
    {INSECT_CENTIPEDE, 995},
};
static dInsectSpawn_c sOct_5[] = {
    {INSECT_CRICKET, 350},
    {INSECT_BELL_CRICKET, 570},
    {INSECT_BAGWORM, 620},
    {INSECT_FLEA, 630},
    {INSECT_PILL_BUG, 720},
    {INSECT_CENTIPEDE, 740},
};
static dInsectSpawn_c sNov_0[6] = { // one more slot than the list uses
    {INSECT_CRICKET, 150},
    {INSECT_MOLE_CRICKET, 180},
    {INSECT_BAGWORM, 260},
    {INSECT_FLEA, 270},
    {INSECT_PILL_BUG, 420},
};
static dInsectSpawn_c sNov_1[] = {
    {INSECT_CRICKET, 90},
    {INSECT_MOLE_CRICKET, 120},
    {INSECT_WALKING_STICK, 150},
    {INSECT_BAGWORM, 230},
    {INSECT_FLEA, 240},
    {INSECT_PILL_BUG, 390},
};
static dInsectSpawn_c sNov_2[12] = { // one more slot than the list uses
    {INSECT_MONARCH_BUTTERFLY, 200},
    {INSECT_LONG_LOCUST, 300},
    {INSECT_MIGRATORY_LOCUST, 380},
    {INSECT_MANTIS, 440},
    {INSECT_ORCHID_MANTIS, 480},
    {INSECT_MOLE_CRICKET, 510},
    {INSECT_WALKING_STICK, 530},
    {INSECT_BAGWORM, 680},
    {INSECT_VIOLIN_BEETLE, 720},
    {INSECT_FLEA, 730},
    {INSECT_PILL_BUG, 880},
};
static dInsectSpawn_c sNov_3[] = {
    {INSECT_MONARCH_BUTTERFLY, 150},
    {INSECT_LONG_LOCUST, 210},
    {INSECT_MIGRATORY_LOCUST, 240},
    {INSECT_MANTIS, 280},
    {INSECT_ORCHID_MANTIS, 300},
    {INSECT_MOLE_CRICKET, 330},
    {INSECT_WALKING_STICK, 350},
    {INSECT_BAGWORM, 500},
    {INSECT_VIOLIN_BEETLE, 550},
    {INSECT_FLEA, 560},
    {INSECT_PILL_BUG, 710},
    {INSECT_CENTIPEDE, 730},
};
static dInsectSpawn_c sNov_4[10] = { // one more slot than the list uses
    {INSECT_LONG_LOCUST, 70},
    {INSECT_MIGRATORY_LOCUST, 110},
    {INSECT_CRICKET, 200},
    {INSECT_MOLE_CRICKET, 230},
    {INSECT_WALKING_STICK, 260},
    {INSECT_BAGWORM, 410},
    {INSECT_FLEA, 420},
    {INSECT_PILL_BUG, 570},
    {INSECT_CENTIPEDE, 590},
};
static dInsectSpawn_c sNov_5[] = {
    {INSECT_CRICKET, 150},
    {INSECT_MOLE_CRICKET, 180},
    {INSECT_BAGWORM, 260},
    {INSECT_FLEA, 270},
    {INSECT_PILL_BUG, 420},
    {INSECT_CENTIPEDE, 440},
};
static dInsectSpawn_c sDec_0[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 100},
    {INSECT_PILL_BUG, 250},
};
static dInsectSpawn_c sDec_1[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 100},
    {INSECT_PILL_BUG, 250},
};
static dInsectSpawn_c sDec_2[4] = { // one more slot than the list uses
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 130},
    {INSECT_PILL_BUG, 280},
};
static dInsectSpawn_c sDec_3[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 130},
    {INSECT_PILL_BUG, 280},
    {INSECT_CENTIPEDE, 300},
};
static dInsectSpawn_c sDec_4[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 130},
    {INSECT_DUNG_BEETLE, 140},
    {INSECT_PILL_BUG, 290},
    {INSECT_CENTIPEDE, 310},
};
static dInsectSpawn_c sDec_5[] = {
    {INSECT_MOLE_CRICKET, 30},
    {INSECT_BAGWORM, 80},
    {INSECT_DUNG_BEETLE, 90},
    {INSECT_PILL_BUG, 240},
    {INSECT_CENTIPEDE, 260},
};
static dInsectSpawnList_c sSpawn_Jan[INSECT_TIME_NUM] = {
    {sJan_0, ARRAY_SIZE(sJan_0)}, {sJan_1, ARRAY_SIZE(sJan_1)}, {sJan_2, 3}, {sJan_3, ARRAY_SIZE(sJan_3)}, {sJan_4, ARRAY_SIZE(sJan_4)}, {sJan_5, ARRAY_SIZE(sJan_5)},
};
static dInsectSpawnList_c sSpawn_Feb[INSECT_TIME_NUM] = {
    {sFeb_0, ARRAY_SIZE(sFeb_0)}, {sFeb_1, ARRAY_SIZE(sFeb_1)}, {sFeb_2, 3}, {sFeb_3, ARRAY_SIZE(sFeb_3)}, {sFeb_4, ARRAY_SIZE(sFeb_4)}, {sFeb_5, ARRAY_SIZE(sFeb_5)},
};
static dInsectSpawnList_c sSpawn_Mar[INSECT_TIME_NUM] = {
    {sMar_0, ARRAY_SIZE(sMar_0)}, {sMar_1, ARRAY_SIZE(sMar_1)}, {sMar_2, ARRAY_SIZE(sMar_2)}, {sMar_3, ARRAY_SIZE(sMar_3)}, {sMar_4, ARRAY_SIZE(sMar_4)}, {sMar_5, ARRAY_SIZE(sMar_5)},
};
static dInsectSpawnList_c sSpawn_Apr[INSECT_TIME_NUM] = {
    {sApr_0, ARRAY_SIZE(sApr_0)}, {sApr_1, ARRAY_SIZE(sApr_1)}, {sApr_2, ARRAY_SIZE(sApr_2)}, {sApr_3, ARRAY_SIZE(sApr_3)}, {sApr_4, ARRAY_SIZE(sApr_4)}, {sApr_5, ARRAY_SIZE(sApr_5)},
};
static dInsectSpawnList_c sSpawn_May[INSECT_TIME_NUM] = {
    {sMay_0, ARRAY_SIZE(sMay_0)}, {sMay_1, ARRAY_SIZE(sMay_1)}, {sMay_2, ARRAY_SIZE(sMay_2)}, {sMay_3, 17}, {sMay_4, ARRAY_SIZE(sMay_4)}, {sMay_5, ARRAY_SIZE(sMay_5)},
};
static dInsectSpawnList_c sSpawn_Jun[INSECT_TIME_NUM] = {
    {sJun_0, ARRAY_SIZE(sJun_0)}, {sJun_1, ARRAY_SIZE(sJun_1)}, {sJun_2, ARRAY_SIZE(sJun_2)}, {sJun_3, ARRAY_SIZE(sJun_3)}, {sJun_4, ARRAY_SIZE(sJun_4)}, {sJun_5, ARRAY_SIZE(sJun_5)},
};
static dInsectSpawnList_c sSpawn_Jul[INSECT_TIME_NUM] = {
    {sJul_0, ARRAY_SIZE(sJul_0)}, {sJul_1, ARRAY_SIZE(sJul_1)}, {sJul_2, ARRAY_SIZE(sJul_2)}, {sJul_3, ARRAY_SIZE(sJul_3)}, {sJul_4, ARRAY_SIZE(sJul_4)}, {sJul_5, ARRAY_SIZE(sJul_5)},
};
static dInsectSpawnList_c sSpawn_Aug[INSECT_TIME_NUM] = {
    {sAug_0, 25}, {sAug_1, ARRAY_SIZE(sAug_1)}, {sAug_2, ARRAY_SIZE(sAug_2)}, {sAug_3, ARRAY_SIZE(sAug_3)}, {sAug_4, ARRAY_SIZE(sAug_4)}, {sAug_5, ARRAY_SIZE(sAug_5)},
};
static dInsectSpawnList_c sSpawn_Sep[INSECT_TIME_NUM] = {
    {sSep_0, 15}, {sSep_1, ARRAY_SIZE(sSep_1)}, {sSep_2, ARRAY_SIZE(sSep_2)}, {sSep_3, ARRAY_SIZE(sSep_3)}, {sSep_4, ARRAY_SIZE(sSep_4)}, {sSep_5, 15},
};
static dInsectSpawnList_c sSpawn_Oct[INSECT_TIME_NUM] = {
    {sOct_0, ARRAY_SIZE(sOct_0)}, {sOct_1, 9}, {sOct_2, ARRAY_SIZE(sOct_2)}, {sOct_3, ARRAY_SIZE(sOct_3)}, {sOct_4, 11}, {sOct_5, ARRAY_SIZE(sOct_5)},
};
static dInsectSpawnList_c sSpawn_Nov[INSECT_TIME_NUM] = {
    {sNov_0, 5}, {sNov_1, ARRAY_SIZE(sNov_1)}, {sNov_2, 11}, {sNov_3, ARRAY_SIZE(sNov_3)}, {sNov_4, 9}, {sNov_5, ARRAY_SIZE(sNov_5)},
};
static dInsectSpawnList_c sSpawn_Dec[INSECT_TIME_NUM] = {
    {sDec_0, ARRAY_SIZE(sDec_0)}, {sDec_1, ARRAY_SIZE(sDec_1)}, {sDec_2, 3}, {sDec_3, ARRAY_SIZE(sDec_3)}, {sDec_4, ARRAY_SIZE(sDec_4)}, {sDec_5, ARRAY_SIZE(sDec_5)},
};

// 804E83D0
static dInsectSpawnList_c *sSpawnTable[12] = {
    sSpawn_Jan, sSpawn_Feb, sSpawn_Mar, sSpawn_Apr, sSpawn_May, sSpawn_Jun,
    sSpawn_Jul, sSpawn_Aug, sSpawn_Sep, sSpawn_Oct, sSpawn_Nov, sSpawn_Dec,
};

// 804E8400: Bug-Off score (5..100). The Bug-Off host (d_a_npc_sp_bugNP) and the Bug-Off entry code
// (80114930..) turn it into a rank: <=20, <=40, <=60, <=80, <=99, 100 (fn_80114BAC).
static u8 sBugOffScore[INSECT_NUM] = {
    20, 25, 45, 60, 25, 75, 75, 75, 90, 5, 85, 30, 70, 20, 40, 55,
    70, 5, 10, 15, 20, 70, 30, 40, 65, 90, 5, 25, 50, 35, 20, 25,
    35, 60, 50, 55, 30, 30, 55, 60, 70, 50, 30, 85, 60, 60, 70, 80,
    85, 90, 100, 65, 85, 80, 100, 90, 5, 5, 5, 10, 65, 50, 75, 80,
};

// 804E8440: whether a villager can catch it for the Bug-Off (getRandomNpcCatch). FALSE for the peacock
// butterfly, Raja Brooke, orchid mantis, lantern fly, snail, violin beetle, longhorn beetle and centipede,
// which need a special spot (flowers, stumps, rocks, rain).
static u8 sNpcCatchable[INSECT_NUM] = {
    1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
};

// 804E8760
static const char *sModelName[INSECT_NUM] = {
    "ins_monsiro", "ins_monki", "ins_ageha", "ins_karasu",
    "ins_ookaba", "ins_morpho", "ins_miiro", "ins_akaeri",
    "ins_alex", "ins_ga", "ins_ymmga", "ins_mitsu",
    "ins_suzume", "ins_syouryou", "ins_tonosama", "ins_kama",
    "ins_hanakama", "ins_abura", "ins_minmin", "ins_tsuku",
    "ins_higu", "ins_biwa", "ins_akane", "ins_ginyan",
    "ins_oniyan", "ins_mukashi", "ins_ari", "ins_amenbo",
    "ins_gengorou", "ins_kata", "ins_koorogi", "ins_suzu",
    "ins_kirigiri", "ins_okera", "ins_konoha", "ins_nana",
    "ins_mino", "ins_tentou", "ins_violin", "ins_kamikiri",
    "ins_hunkoro", "ins_hotaru", "ins_kogane", "ins_plakoga",
    "ins_tama", "ins_miyakwa", "ins_nokokwa", "ins_ookwa",
    "ins_nijikwa", "ins_hosokwa", "ins_ougon", "ins_kbt",
    "ins_kokbt", "ins_zokbt", "ins_helkbt", "ins_gorias",
    "ins_nomi", "ins_dango", "ins_ka", "ins_hae",
    "ins_mukade", "ins_kumo", "ins_taran", "ins_sasori",
};

// 804E8860 / 804E8874: spawn rate range per dInsectRarity_e.
static int sRarityMin[INSECT_RARITY_NUM] = {150, 100, 40, 20, 0};
static int sRarityMax[INSECT_RARITY_NUM] = {1000, 149, 99, 39, 19};

// 8074E488..8074E48A. Set by fgMngProc_attractInsects (offline, outdoors) when candy / spoiled turnips
// (ants) or trash / spoiled turnips (flies) lie in town; d_insect_fieldNP spawns the insect near one
// and clears the flag.
static u8 sAntsAttracted;
static u8 sFliesAttracted;
static u8 sHasPos;
// 8059AD8C
static mVec3_c sPos(0.0f, 0.0f, 0.0f);

namespace dInsectInfo {

inline int getTimeSlot(int hour) {
    if (hour < 4) {
        return INSECT_TIME_NIGHT;
    }
    if (hour < 8) {
        return INSECT_TIME_MORNING;
    }
    if (hour < 16) {
        return INSECT_TIME_DAY;
    }
    if (hour < 17) {
        return INSECT_TIME_AFTERNOON;
    }
    if (hour < 19) {
        return INSECT_TIME_EVENING;
    }
    if (hour < 23) {
        return INSECT_TIME_LATE;
    }
    return INSECT_TIME_NIGHT;
}

// 800BD450
dInsectSpawnList_c *getSpawnList(int month, int slot) {
    return &sSpawnTable[month][slot];
}

// 800BD46C
dInsectSpawnList_c *getSpawnList(int slot) {
    dTime_c now = *dTime_c::getCurrent();
    return getSpawnList(now.month, slot);
}

// 800BD4FC
dInsectSpawnList_c *getSpawnList() {
    return getSpawnList(getTimeSlot(dTime_c::getCurrent()->hour));
}

// 800BD58C
BOOL isFleaSpawn() {
    if (fn_800DCEDC()) {
        return FALSE;
    }
    dInsectSpawn_c *spawn = getRandomSpawn();
    if (spawn != NULL && spawn->mType == INSECT_FLEA) {
        return TRUE;
    }
    return FALSE;
}

// 800BD5E0
BOOL isOfTerm(int type) {
    BOOL ret = TRUE;
    switch (type) {
    case INSECT_MOTH:
    case INSECT_HONEYBEE:
    case INSECT_BEE:
    case INSECT_ANT:
    case INSECT_POND_SKATER:
    case INSECT_WALKING_LEAF:
    case INSECT_BAGWORM:
    case INSECT_DUNG_BEETLE:
    case INSECT_FLEA:
    case INSECT_MOSQUITO:
    case INSECT_FLY:
    case INSECT_SPIDER:
    case INSECT_TARANTULA:
    case INSECT_SCORPION:
        ret = FALSE;
        break;
    }
    return ret;
}

// 800BD610
dInsectSpawn_c *getRandomSpawn() {
    dInsectSpawnList_c *spawnList = getSpawnList();
    int num = spawnList->mNum;
    dInsectSpawn_c *list = spawnList->mList;
    f32 roll = cM::rndF(1000.0f);
    for (int i = 0; i < num; i++) {
        if (roll < list[i].mRate) {
            return &list[i];
        }
    }
    return NULL;
}

// 800BD6A8
int getRandomByRarity(const dTime_c &time, int rarity, int maxRarity, BOOL allDay) {
    dSearchCand_c<INSECT_NUM> cand;
    cand.clear();
    if (maxRarity == INSECT_RARITY_SAME) {
        maxRarity = rarity;
    }
    int min = sRarityMin[rarity];
    int max = sRarityMax[maxRarity];
    int timeStart;
    int timeEnd;
    if (allDay) {
        timeStart = 0;
        timeEnd = INSECT_TIME_NUM - 1;
    } else {
        timeStart = getTimeSlot(time.hour);
        timeEnd = getTimeSlot(time.hour);
    }
    BOOL down = TRUE;
    int res;
    while (true) {
        for (int t = timeStart; t <= timeEnd; t++) {
            dInsectSpawnList_c spawnList = sSpawnTable[time.month][t];
            int prev = 0;
            for (int i = 0; i < spawnList.mNum; i++) {
                int rate = spawnList.mList[i].mRate - prev;
                prev = spawnList.mList[i].mRate;
                if (sLib::isInRange(rate, min, max)) {
                    cand.add(spawnList.mList[i].mType);
                }
            }
        }
        res = cand.getRandom();
        if (res >= 0) {
            break;
        }
        if (down) {
            if (--maxRarity < 0) {
                maxRarity = 1;
                down = FALSE;
            }
        } else if (++maxRarity >= INSECT_RARITY_NUM) {
            break;
        }
        min = sRarityMin[maxRarity];
        max = sRarityMax[maxRarity];
    }
    return res;
}

// 800BD8F8
int getRandomNpcCatch(const dTime_c &time, int rarity, int maxRarity, BOOL allDay) {
    int res = getRandomByRarity(time, rarity, maxRarity, allDay);
    if (!isNpcCatchable(res)) {
        return -1;
    }
    return res;
}

// 800BD938
u8 getRandomOfTerm() {
    dTime_c time = *dTime_c::getCurrent();
    time.add(0, -6, 0, 0);
    dSearchCand_c<INSECT_NUM> cand;
    cand.clear();
    for (int t = 0; t < INSECT_TIME_NUM; t++) {
        dInsectSpawnList_c *spawnList = getSpawnList(time.month, t);
        int num = spawnList->mNum;
        dInsectSpawn_c *list = spawnList->mList;
        for (int i = 0; i < num; i++) {
            if (list[i].mRate > 0 && isOfTerm(list[i].mType)) {
                cand.add(list[i].mType);
            }
        }
    }
    return cand.getRandom();
}

// 800BDA74
u8 getBugOffScore(int type) {
    return sBugOffScore[type];
}

// 800BDA84
BOOL isNpcCatchable(int type) {
    return sNpcCatchable[type];
}

// 800BDA94
u8 getBugOffScore(const dItem::Item *item) {
    return getBugOffScore(dItem::seeker_c::get()->findLike(*item));
}

// 800BDAC8
const char *getModelName(int type) {
    return sModelName[type];
}

// 800BDADC
void setAntsAttracted() {
    sAntsAttracted = TRUE;
}

// 800BDAE8
u8 isAntsAttracted() {
    return sAntsAttracted;
}

// 800BDAF0
void clearAntsAttracted() {
    sAntsAttracted = FALSE;
}

// 800BDAFC
void setFliesAttracted() {
    sFliesAttracted = TRUE;
}

// 800BDB08
u8 isFliesAttracted() {
    return sFliesAttracted;
}

// 800BDB10
void clearFliesAttracted() {
    sFliesAttracted = FALSE;
}

// 800BDB1C
void setPos(const mVec3_c &pos) {
    sHasPos = TRUE;
    sPos = pos;
}

// 800BDB48
u8 hasPos() {
    return sHasPos;
}

// 800BDB50
mVec3_c *getPos() {
    return &sPos;
}

// 800BDB5C
void clearPos() {
    sHasPos = FALSE;
}

// 800BDB68
u8 getActiveTimeMask(int type) {
    return sActiveTimeMask[type];
}

// 800BDB78: FALSE for the insects that do not spawn while it rains (weather 3/4, fn_800C0420); the
// snail needs rain (d_insect_fieldNP checks that separately).
BOOL canSpawnInRain(int type) {
    const int list[32] = {
        INSECT_COMMON_BUTTERFLY, INSECT_YELLOW_BUTTERFLY, INSECT_TIGER_BUTTERFLY, INSECT_PEACOCK_BUTTERFLY,
        INSECT_MONARCH_BUTTERFLY, INSECT_EMPEROR_BUTTERFLY, INSECT_AGRIAS_BUTTERFLY, INSECT_RAJA_BROOKE,
        INSECT_BIRDWING_BUTTERFLY, INSECT_MOTH, INSECT_HONEYBEE, INSECT_FLY,
        INSECT_LONG_LOCUST, INSECT_MIGRATORY_LOCUST, INSECT_CRICKET, INSECT_BELL_CRICKET,
        INSECT_GRASSHOPPER, INSECT_MANTIS, INSECT_ORCHID_MANTIS, INSECT_LADYBUG,
        INSECT_RED_DRAGONFLY, INSECT_COMMON_DRAGONFLY, INSECT_DARNER_DRAGONFLY, INSECT_BANDED_DRAGONFLY,
        INSECT_ANT, INSECT_POND_SKATER, INSECT_VIOLIN_BEETLE, INSECT_LONGHORN_BEETLE,
        INSECT_FIREFLY, INSECT_MOSQUITO, INSECT_TARANTULA, INSECT_SCORPION,
    };
    for (int i = 0; i < ARRAY_SIZE(list); i++) {
        if (type == list[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

// 800BDC70: FALSE for the insects that do not spawn while it snows (weather 5/6, fn_800C0454). Same
// list as canSpawnInRain, with the snail instead of the ant.
BOOL canSpawnInSnow(int type) {
    const int list[32] = {
        INSECT_COMMON_BUTTERFLY, INSECT_YELLOW_BUTTERFLY, INSECT_TIGER_BUTTERFLY, INSECT_PEACOCK_BUTTERFLY,
        INSECT_MONARCH_BUTTERFLY, INSECT_EMPEROR_BUTTERFLY, INSECT_AGRIAS_BUTTERFLY, INSECT_RAJA_BROOKE,
        INSECT_BIRDWING_BUTTERFLY, INSECT_MOTH, INSECT_HONEYBEE, INSECT_FLY,
        INSECT_LONG_LOCUST, INSECT_MIGRATORY_LOCUST, INSECT_CRICKET, INSECT_BELL_CRICKET,
        INSECT_GRASSHOPPER, INSECT_MANTIS, INSECT_ORCHID_MANTIS, INSECT_SNAIL,
        INSECT_LADYBUG, INSECT_RED_DRAGONFLY, INSECT_COMMON_DRAGONFLY, INSECT_DARNER_DRAGONFLY,
        INSECT_BANDED_DRAGONFLY, INSECT_POND_SKATER, INSECT_VIOLIN_BEETLE, INSECT_LONGHORN_BEETLE,
        INSECT_FIREFLY, INSECT_MOSQUITO, INSECT_TARANTULA, INSECT_SCORPION,
    };
    for (int i = 0; i < ARRAY_SIZE(list); i++) {
        if (type == list[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

// 800BDD68
void initCatchData(dInsectCatchData_c *data) {
    dInsectCatchData_c init;
    init.mTypeSerial = 0;
    init.mSerial = 0;
    init.mPosXZ = 0;
    init.mPosZ = 0;
    init.mState = 0;
    init.mTypeSerial |= INSECT_NET_TYPE_NONE;
    init.mState |= INSECT_NET_STATE_NORMAL;
    init.mPosX = 0;
    cLib::memCpy(data, &init, 6);
}

} // namespace dInsectInfo
