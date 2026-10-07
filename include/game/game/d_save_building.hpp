#pragma once

// The town's buildings in the town save (dSaveTown_c::mBuilding, +0x5E260): the town flag, the gate and
// house looks, and where every building stands. Source: src/dol/game/d_save_building.cpp
// (.text 80149B70..8014BD88).
//
// Town and city buildings: the item ids 0xD000..0xD044 (dItem::Item::isExtId). Each indexes the
// 0x40-byte building table at 80479FC8 (fn_80167FB4) by its low 12 bits. The names come from that
// table: the model name and the Japanese name (quoted, so no line ends on a Shift-JIS lead byte:
// MWCC reads the sources as SJIS).

#include <types.h>
#include <game/game/d_item.hpp>
#include <game/game/d_dsn.hpp>

enum {
    BUILDING_BUILD_SITE = 0xD000,       // obj_buildsite "建設予定地" (construction site)

    BUILDING_PLAYER_HOUSE_0 = 0xD001,   // obj_myhome0 "プレイヤの家０..３"
    BUILDING_PLAYER_HOUSE_1 = 0xD002,
    BUILDING_PLAYER_HOUSE_2 = 0xD003,
    BUILDING_PLAYER_HOUSE_3 = 0xD004,
    BUILDING_MAILBOX_0 = 0xD005,        // obj_post "ポスト０..３" (the players' mailboxes)
    BUILDING_MAILBOX_1 = 0xD006,
    BUILDING_MAILBOX_2 = 0xD007,
    BUILDING_MAILBOX_3 = 0xD008,
    BUILDING_NPC_HOUSE_0 = 0xD009,      // obj_myhome0 "ＮＰＣの家０..９" (villager houses)
    BUILDING_NPC_HOUSE_1 = 0xD00A,
    BUILDING_NPC_HOUSE_2 = 0xD00B,
    BUILDING_NPC_HOUSE_3 = 0xD00C,
    BUILDING_NPC_HOUSE_4 = 0xD00D,
    BUILDING_NPC_HOUSE_5 = 0xD00E,
    BUILDING_NPC_HOUSE_6 = 0xD00F,
    BUILDING_NPC_HOUSE_7 = 0xD010,
    BUILDING_NPC_HOUSE_8 = 0xD011,
    BUILDING_NPC_HOUSE_9 = 0xD012,
    BUILDING_TOWN_HALL = 0xD013,        // obj_office "役所"
    BUILDING_GATE = 0xD014,             // obj_myhome0 "関所" (no model of its own)
    BUILDING_SHOP = 0xD015,             // obj_nc "お店" (Nook's)
    BUILDING_TAILOR = 0xD016,           // obj_tailor "仕立て屋" (Able Sisters)
    BUILDING_MUSEUM = 0xD017,           // obj_museum "博物館"
    BUILDING_BULLETIN_BOARD = 0xD018,   // obj_bbs "掲示板"
    BUILDING_COUNTDOWN_BOARD = 0xD019,  // obj_cdbbs "カウントダウン" (New Year's countdown)
    BUILDING_FISH_TENT = 0xD01A,        // obj_tent0 "魚テント" (fishing tourney)
    BUILDING_INSECT_TENT = 0xD01B,      // obj_tent1 "虫テント" (Bug-Off)
    BUILDING_LIGHTHOUSE = 0xD01C,       // obj_lighthouse "灯台" (public works)
    BUILDING_WINDMILL = 0xD01D,         // obj_windmill "風車" (public works)
    BUILDING_FOUNTAIN = 0xD01E,         // obj_fountain "噴水" (public works)
    BUILDING_HARVEST_0 = 0xD01F,        // obj_harvest0 "ハーベスト０" (Harvest Festival)
    BUILDING_HARVEST_1 = 0xD020,        // obj_harvest1 "ハーベスト１"
    BUILDING_UFO = 0xD021,              // ufo1 UFO

    // The city (obj_tw_*)
    BUILDING_CITY_BARBER = 0xD022,      // obj_tw_cut "床屋" (Shampoodle)
    BUILDING_CITY_HAPPY_ROOM = 0xD023,  // obj_tw_HRA "ハッピールーム" (HRA)
    BUILDING_CITY_BROKER = 0xD024,      // obj_tw_tsunekichi "ブローカー" (Crazy Redd's)
    BUILDING_CITY_FORTUNE = 0xD025,     // obj_tw_fortune "ハッケミィ" (Katrina)
    BUILDING_CITY_GRACE = 0xD026,       // obj_tw_grace "グレース" (GracieGrace)
    BUILDING_CITY_AUCTION = 0xD027,     // obj_tw_auction "オークション"
    BUILDING_CITY_THEATER = 0xD028,     // obj_tw_theater "劇場"
    BUILDING_CITY_ATM = 0xD029,         // obj_tw_ATM ATM
    BUILDING_CITY_SHOE_SHINE = 0xD02A,  // obj_tw_shoeblack "靴磨き"
    BUILDING_CITY_ETC_0 = 0xD02B,       // obj_tw_etc00..04 "その他０..４"
    BUILDING_CITY_ETC_1 = 0xD02C,
    BUILDING_CITY_ETC_2 = 0xD02D,
    BUILDING_CITY_ETC_3 = 0xD02E,
    BUILDING_CITY_ETC_4 = 0xD02F,
    BUILDING_CITY_RESET_CENTER = 0xD030, // obj_tw_reset "リセットセンター入り口" (its entrance)
    BUILDING_CITY_FOUNTAIN = 0xD031,    // obj_tw_fountain "街の噴水"
    BUILDING_CITY_LAMP_A = 0xD032,      // obj_tw_lamp "街灯Ａ..Ｌ"
    BUILDING_CITY_LAMP_B = 0xD033,
    BUILDING_CITY_LAMP_C = 0xD034,
    BUILDING_CITY_LAMP_D = 0xD035,
    BUILDING_CITY_LAMP_E = 0xD036,
    BUILDING_CITY_LAMP_F = 0xD037,
    BUILDING_CITY_LAMP_G = 0xD038,
    BUILDING_CITY_LAMP_H = 0xD039,
    BUILDING_CITY_LAMP_I = 0xD03A,
    BUILDING_CITY_LAMP_J = 0xD03B,
    BUILDING_CITY_LAMP_K = 0xD03C,
    BUILDING_CITY_LAMP_L = 0xD03D,
    BUILDING_CITY_ORGAN = 0xD03E,       // obj_tw_organ "自動オルガン" (street organ)

    BUILDING_BUS = 0xD03F,              // obj_bus "バス"
    BUILDING_CITY_BUS = 0xD040,         // obj_bus "バス(街)"
    BUILDING_BUS_STOP = 0xD041,         // obj_busstop0 "バス停"
    BUILDING_CITY_BUS_STOP = 0xD042,    // obj_busstop1 "街のバス停"
    BUILDING_CITY_DRESSER = 0xD043,     // obj_tw_organ "タンス" (dresser; reuses the organ's model)
    BUILDING_CARNIVAL = 0xD044,         // obj_carnival_bord "カーニバル" (Carnival board)

    BUILDING_END = 0xD045,
    BUILDING_NUM = BUILDING_END - BUILDING_BUILD_SITE,
};

// An entry of the 0xD000 building table (fn_80167FB4; 0x45 entries of 0x40 bytes at 80479FC8).
struct dBuildingInfo_c {
    /* 0x00 */ const char *mModel;  // obj_*
    /* 0x04 */ const char *mName;   // the Japanese name
    /* 0x08 */ u8 _08[6];
    /* 0x0E */ u8 mField;           // 1: the town, 2: the city
    /* 0x0F */ u8 mValueA;          // dItem::Item::getExtValueA (the collision height)
    /* 0x10 */ u8 mValueB;          // dItem::Item::getExtValueB
    /* 0x11 */ u8 mFlag;            // dItem::Item::getExtFlag
    /* 0x12 */ u8 _12[0x2E];
}; // size 0x40

extern "C" dBuildingInfo_c *fn_80167FB4(int idx); // the table entry of a building index (not split yet)

// A building's unit position; (0, 0) = none.
struct dSaveBuildingPos_c {
    /* 0x0 */ s8 mX;
    /* 0x1 */ s8 mZ;
}; // size 0x2

#define BUILD_SITE_NUM 100

// Where each building stands (dSaveBuilding_c::mList, dSaveTown_c+0x5EB04). Its CRC is checked by
// dSaveData_c::isExtraGood.
class dSaveBuildingList_c {
public:
    dSaveBuildingList_c() { init(); }

    static int getRangeIndex(int idx, int first, int last); // 80149B70: idx - first, or -1
    static BOOL isPlayerHouse(int idx);                    // 80149B90
    static int getPlayerHouseNo(int idx);                  // 80149BC0: building index 1..4 -> 0..3
    static BOOL isNpcHouse(int idx);                       // 80149BCC
    static int getNpcHouseNo(int idx);                     // 80149BFC: building index 9..0x12 -> 0..9

    void init();                                           // 80149E38: no positions
    void clear();                                          // 80149FB4
    static u16 getNpcHouseId(u32 no);                      // 80149FEC
    static u16 getPlayerHouseId(u32 no);                   // 8014A010
    BOOL getPlayerHousePos(int *x, int *z, int no) const;        // 8014A034
    BOOL setNpcHouse(u32 no);                              // 8014A098: picks a spot
    BOOL removeNpcHouse(u32 no);                           // 8014A4F4
    BOOL getNpcHousePos(int *x, int *z, u32 no) const;           // 8014A544
    BOOL hasNpcHouse(u32 no) const;                              // 8014A5B8
    BOOL setFountain();                                    // 8014A5E4: in the plaza block
    BOOL setAtLighthouseSpot(const dItem::Item &id);       // 8014A660: lighthouse / windmill
    BOOL setAtUfoSpot(const dItem::Item &id);              // 8014A770
    BOOL setAtReserveSpot(const dItem::Item &id);          // 8014A880: any free build site
    void create();                                         // 8014A978: a new town's layout
    void setCityBuildings();                               // 8014A9D4: the city's fixed layout
    BOOL setPlayerHouses();                                // 8014ADB0
    void setFromField();                                   // 8014AEF4: the buildings placed in the town field
    dItem::Item getAt(int x, int z, int mask) const;             // 8014B034: the building at a unit, or none
    BOOL getPos(int *x, int *z, const dItem::Item *id, int mask) const; // 8014B0F0
    BOOL setPos(int x, int z, dItem::Item id, BOOL updateCrc); // 8014B1C0
    BOOL setPos(int x, int z, dItem::Item id);             // 8014B2CC
    BOOL remove(dItem::Item id);                           // 8014B2FC
    u32 calcChecksum() const;                                    // 8014B3CC
    BOOL addBuildSite(int x, int z, BOOL updateCrc);       // 8014B3E4
    BOOL isBuildSite(int x, int z) const;                        // 8014B474
    BOOL getBuildSite(int *x, int *z, u32 n) const;              // 8014B4E4
    int countHouses(int blockX, int blockZ) const;               // 8014B55C: player and villager houses in an acre
    static dSaveBuildingList_c *get();                     // 8014B6C8

    // The building id of a table index (the build site for out-of-range indices).
    static u16 getId(u32 idx) {
        return idx < BUILDING_NUM ? BUILDING_BUILD_SITE + idx : BUILDING_BUILD_SITE;
    }

    /* 0x000 */ u32 mChecksum;
    /* 0x004 */ dSaveBuildingPos_c mPos[BUILDING_NUM];     // by building index
    /* 0x08E */ dSaveBuildingPos_c mSites[BUILD_SITE_NUM];  // build sites (BUILDING_BUILD_SITE in the field)
    /* 0x156 */ u8 _156[2];
}; // size 0x158

// The town's buildings (dSaveTown_c::mBuilding, +0x5E260).
class dSaveBuilding_c {
public:
    void clear();                                          // 80149C08: also picks the gate and house looks
    void setGateType();                                    // 80149C50: 0..2
    void initTownFlag();                                   // 80149C88: the default flag, by this town
    static dSaveBuilding_c *get();                         // 8014B6A0

    /* 0x000 */ dDesign_c mTownFlag;
    /* 0x880 */ u8 mGateType;                              // 0..2
    /* 0x881 */ u8 _881[0x1F];
    /* 0x8A0 */ u8 mHouseVariant;                          // 0..4: the villager houses' look
    /* 0x8A1 */ u8 _8A1[3];
    /* 0x8A4 */ dSaveBuildingList_c mList;
    /* 0x9FC */ u8 _9FC[4];
}; // size 0xA00
