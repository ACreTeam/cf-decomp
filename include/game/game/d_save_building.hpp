#pragma once

// Town and city buildings: the item ids 0xD000..0xD044 (dItem::Item::isExtId). Each indexes the
// 0x40-byte building table at 80479FC8 (fn_80167FB4) by its low 12 bits. The names come from that
// table: the model name and the Japanese name (quoted, so no line ends on a Shift-JIS lead byte:
// MWCC reads the sources as SJIS).

#include <types.h>

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
