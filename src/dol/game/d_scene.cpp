// The scene manager: the current / previous scene, the scene attribute and parameter tables, the
// scene's creation steps, scene changes and exits, and the town season. .text 80161CC4..80163AA0,
// .ctors 80465754, .rodata 80479BB0..80479F70, .data 804F0768..804F0C50, .bss 805FAF58..805FB008,
// .sdata 8074B2C0..8074B340, .sbss 8074E848..8074E858, .sdata2 80750EB8..80750EE0.
#include <game/game/d_scene.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_save_data.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
BOOL fn_800CCDA4(void *data, int num);
BOOL fn_80086DD0(u16 id);
BOOL fn_80086E00(u16 id);
int fn_80086E20(u16 id);
BOOL fn_800DCEDC();
BOOL fn_800DCEE0();
u32 fn_800DCF30();
int fn_800DCF58();
BOOL fn_800DD6B4();
void fn_800DD6D8(int arg);
void fn_80082794();
void fn_8008279C();
void fn_800C7704();
void fn_800C7708();
void fn_801A6584();
void fn_801A670C();
void fn_80186708();
void fn_80186260();
void *fn_80100234(int player, int arg, u32 type, const mVec3_c *pos, const mAng3_c *angle);
int fn_8008B91C();
int fn_8008B924();
int fn_80169124();
void fn_800FE688(u32 *type, mVec3_c *pos, s16 *angle);
BOOL fn_8014B0F0(void *fg, int *x, int *z, dItem::Item *item, int flags);
struct dSceneBuildingInfo_c {
    u8 _00[0xE];
    u8 mFlags; // 0x0E
};
dSceneBuildingInfo_c *fn_80167FB4(int index);
void *fn_801683CC();
struct dSceneBuilding_c {
    u8 _00[0x1A];
    u16 mAngle; // 0x1A
};
dSceneBuilding_c *fn_80167734(void *mgr, u16 id);
void fn_801681B8(mVec3_c *out, dSceneBuilding_c *bld, u16 id, int arg, f32 dist);
}

// The creation steps of each scene (other TUs' .sdata), named after the first scene using them.
extern dSceneData_c *sScDataField;
extern dSceneData_c *sScDataRmHs0F1;
extern dSceneData_c *sScDataRmHs0F2;
extern dSceneData_c *sScDataRmHs0Bm;
extern dSceneData_c *sScDataRmHs0Loft;
extern dSceneData_c *sScDataRmNpc0;
extern dSceneData_c *sScDataRmOffice;
extern dSceneData_c *sScDataRmChkp;
extern dSceneData_c *sScDataRmMmEnt;
extern dSceneData_c *sScDataRmMmFish;
extern dSceneData_c *sScDataRmMmIns;
extern dSceneData_c *sScDataRmMmFossil0;
extern dSceneData_c *sScDataRmMmFossil1;
extern dSceneData_c *sScDataRmMmPic0;
extern dSceneData_c *sScDataRmMmPic1;
extern dSceneData_c *sScDataRmMmCafe;
extern dSceneData_c *sScDataRmMmAstro;
extern dSceneData_c *sScDataRmShop0;
extern dSceneData_c *sScDataRmShop1;
extern dSceneData_c *sScDataRmShop2;
extern dSceneData_c *sScDataRmShop3F1;
extern dSceneData_c *sScDataRmShop3F2;
extern dSceneData_c *sScDataRmTailor;
extern dSceneData_c *sScDataTown;
extern dSceneData_c *sScDataRmBroker;
extern dSceneData_c *sScDataRmBarber;
extern dSceneData_c *sScDataRmFortune;
extern dSceneData_c *sScDataRmGrace;
extern dSceneData_c *sScDataRmHappyEnt;
extern dSceneData_c *sScDataRmHappyMdl;
extern dSceneData_c *sScDataRmTheaterEnt;
extern dSceneData_c *sScDataRmTheaterShow;
extern dSceneData_c *sScDataRmAuction;
extern dSceneData_c *sScDataRmReset;
extern dSceneData_c *sScDataDmTitle;
extern dSceneData_c *sScDataDmSave;
extern dSceneData_c *sScDataDmPlSel;
extern dSceneData_c *sScDataDmBusPlCrt;
extern dSceneData_c *sScDataCheckField;
extern dSceneData_c *sScDataRmSample;

// The scene wipe (other TUs' data).
extern u16 lbl_8074B1B6; // wipe out type
extern u16 lbl_8074B1B4; // wipe in type
extern u16 lbl_8074B1B0;
extern int lbl_8074E804;
extern int lbl_8074E808;
extern u8 lbl_8074EACD;
extern const f32 lbl_807503F0; // the city's ground height

// 80479BB0: the house attribute per player house
static const u32 sHomeScenes[] = {
    SCENE_ATTR_HOUSE0 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE1 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE2 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE3 | SCENE_ATTR_PLAYER_HOUSE,
    0,
};

// 80479BC4: the scene season per dTime_c term
static const u8 sTermSeasons[0x1C] = {
    0x13, 0x14, 0x15, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
};

#define HOUSE_ROOM(n) (SCENE_ATTR_HOUSE##n | SCENE_ATTR_PLAYER_HOUSE)
#define MUSEUM_ROOM (SCENE_ATTR_MUSEUM | SCENE_ATTR_ROOM)
#define MUSEUM_EXHIBIT (SCENE_ATTR_MUSEUM | SCENE_ATTR_FTR_ROOM | SCENE_ATTR_ROOM)
#define STORE_ROOM(n) (SCENE_ATTR_STORE##n | SCENE_ATTR_STORE_ROOM)
#define SHOP_ROOM (SCENE_ATTR_SHOP | SCENE_ATTR_FACILITY | SCENE_ATTR_ROOM)
#define FACILITY_ROOM (SCENE_ATTR_FACILITY | SCENE_ATTR_ROOM)
#define BUS_DEMO (SCENE_ATTR_BUS | SCENE_ATTR_DEMO)

// 80479BE0
static const u32 sSceneAttr[SCENE_NUM] = {
    SCENE_ATTR_TOWN, // SCENE_FIELD
    HOUSE_ROOM(0), HOUSE_ROOM(0), HOUSE_ROOM(0), HOUSE_ROOM(0),
    HOUSE_ROOM(1), HOUSE_ROOM(1), HOUSE_ROOM(1), HOUSE_ROOM(1),
    HOUSE_ROOM(2), HOUSE_ROOM(2), HOUSE_ROOM(2), HOUSE_ROOM(2),
    HOUSE_ROOM(3), HOUSE_ROOM(3), HOUSE_ROOM(3), HOUSE_ROOM(3),
    SCENE_ATTR_VILLAGER_HOUSE, SCENE_ATTR_VILLAGER_HOUSE, SCENE_ATTR_VILLAGER_HOUSE, SCENE_ATTR_VILLAGER_HOUSE,
    SCENE_ATTR_VILLAGER_HOUSE, SCENE_ATTR_VILLAGER_HOUSE, SCENE_ATTR_VILLAGER_HOUSE, SCENE_ATTR_VILLAGER_HOUSE,
    SCENE_ATTR_VILLAGER_HOUSE, SCENE_ATTR_VILLAGER_HOUSE, // SCENE_RM_NPC0..9
    FACILITY_ROOM, // SCENE_RM_OFFICE
    SCENE_ATTR_CHKP | SCENE_ATTR_ROOM, // SCENE_RM_CHKP
    SCENE_ATTR_CHKP | SCENE_ATTR_ROOM, // SCENE_RM_CHKP_NET
    MUSEUM_ROOM, // SCENE_RM_MM_ENT
    MUSEUM_EXHIBIT, MUSEUM_EXHIBIT, MUSEUM_EXHIBIT, MUSEUM_EXHIBIT, MUSEUM_EXHIBIT, MUSEUM_EXHIBIT,
    MUSEUM_ROOM, // SCENE_RM_MM_CAFE
    MUSEUM_ROOM, // SCENE_RM_MM_ASTRO
    STORE_ROOM(0), STORE_ROOM(1), STORE_ROOM(2), STORE_ROOM(3), STORE_ROOM(3),
    SHOP_ROOM, // SCENE_RM_TAILOR
    SCENE_ATTR_CITY_FIELD, // SCENE_TOWN
    SHOP_ROOM, // SCENE_RM_BROKER
    FACILITY_ROOM, // SCENE_RM_BARBER
    FACILITY_ROOM, // SCENE_RM_FORTUNE
    SHOP_ROOM, // SCENE_RM_GRACE
    SCENE_ATTR_ROOM, // SCENE_RM_HAPPY_ENT
    SCENE_ATTR_FTR_ROOM | SCENE_ATTR_ROOM, // SCENE_RM_HAPPY_MDL
    FACILITY_ROOM, // SCENE_RM_THEATER_ENT
    SCENE_ATTR_ROOM, // SCENE_RM_THEATER_SHOW
    SHOP_ROOM, // SCENE_RM_AUCTION
    SCENE_ATTR_ROOM, // SCENE_RM_RESET
    SCENE_ATTR_DEMO | SCENE_ATTR_TOWN, // SCENE_DM_TITLE
    SCENE_ATTR_DEMO, // SCENE_DM_SAVE
    SCENE_ATTR_DEMO, // SCENE_DM_LOAD
    SCENE_ATTR_DEMO, // SCENE_DM_PL_SEL
    BUS_DEMO, BUS_DEMO, BUS_DEMO, // SCENE_DM_BUS_*
    SCENE_ATTR_CHKP_ROOM, SCENE_ATTR_CHKP_ROOM, SCENE_ATTR_CHKP_ROOM, // SCENE_DM_CHKP_*
    SCENE_ATTR_TOWN | SCENE_ATTR_CHECK_FIELD, // SCENE_CHECK_FIELD
    SCENE_ATTR_ROOM, // SCENE_RM_SAMPLE
};

// 80479CF0: the building (item id) of each scene in the town
static const u16 sSceneBuildings[SCENE_NUM] = {
    0xFFF1, // SCENE_FIELD
    0xD001, 0xD001, 0xD001, 0xD001, // SCENE_RM_HS0_*
    0xD002, 0xD002, 0xD002, 0xD002,
    0xD003, 0xD003, 0xD003, 0xD003,
    0xD004, 0xD004, 0xD004, 0xD004,
    0xD009, 0xD00A, 0xD00B, 0xD00C, 0xD00D, 0xD00E, 0xD00F, 0xD010, 0xD011, 0xD012, // SCENE_RM_NPC0..9
    0xD013, // SCENE_RM_OFFICE
    0xD014, 0xD014, // SCENE_RM_CHKP, SCENE_RM_CHKP_NET
    0xD017, 0xD017, 0xD017, 0xD017, 0xD017, 0xD017, 0xD017, 0xD017, 0xD017, // SCENE_RM_MM_*
    0xD015, 0xD015, 0xD015, 0xD015, 0xD015, // SCENE_RM_SHOP*
    0xD016, // SCENE_RM_TAILOR
    0xFFF1, // SCENE_TOWN
    0xD024, // SCENE_RM_BROKER
    0xD022, // SCENE_RM_BARBER
    0xD025, // SCENE_RM_FORTUNE
    0xD026, // SCENE_RM_GRACE
    0xD023, 0xD023, // SCENE_RM_HAPPY_*
    0xD028, 0xD028, // SCENE_RM_THEATER_*
    0xD027, // SCENE_RM_AUCTION
    0xFFF1, // SCENE_RM_RESET
    0xFFF1, 0xFFF1, 0xFFF1, 0xFFF1, 0xFFF1, 0xFFF1, 0xFFF1, // SCENE_DM_TITLE..SCENE_DM_BUS_TO_LAND
    0xD014, 0xD014, 0xD014, // SCENE_DM_CHKP_*
    0xFFF1, // SCENE_CHECK_FIELD
    0xFFF1, // SCENE_RM_SAMPLE
};

// 80479D78
static const u8 sSceneParamA[SCENE_NUM] = {
    0x00, 0x40, 0x40, 0x40, 0x0A, 0x40, 0x40, 0x40, 0x0A, 0x40, 0x40, 0x40, 0x0A, 0x40, 0x40, 0x40,
    0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x14, 0x14,
    0x0A, 0x20, 0x0A, 0x40, 0x05, 0x10, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

// 80479DBC
static const u8 sSceneParamB[SCENE_NUM] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

// 80479E00
static const u32 sSceneParamC[SCENE_NUM] = {
    0x00000101, // SCENE_FIELD
    0x00000202, 0x00000202, 0x00000202, 0x00010202, // SCENE_RM_HS0_*
    0x00000202, 0x00000202, 0x00000202, 0x00010202,
    0x00000202, 0x00000202, 0x00000202, 0x00010202,
    0x00000202, 0x00000202, 0x00000202, 0x00010202,
    0x00020202, 0x00020202, 0x00020202, 0x00020202, 0x00020202, // SCENE_RM_NPC0..9
    0x00020202, 0x00020202, 0x00020202, 0x00020202, 0x00020202,
    0x00000203, // SCENE_RM_OFFICE
    0x00000204, 0x00000204, // SCENE_RM_CHKP, SCENE_RM_CHKP_NET
    0x00000205, // SCENE_RM_MM_ENT
    0x00010305, 0x00020305, 0x00030305, 0x00030305, 0x00040305, 0x00040305,
    0x00000306, // SCENE_RM_MM_CAFE
    0x00050305, // SCENE_RM_MM_ASTRO
    0x00000207, 0x00010207, 0x00020207, 0x00030207, 0x00030207, // SCENE_RM_SHOP*
    0x00000208, // SCENE_RM_TAILOR
    0x00000109, // SCENE_TOWN
    0x0000020A, // SCENE_RM_BROKER
    0x0000020B, // SCENE_RM_BARBER
    0x0000020C, // SCENE_RM_FORTUNE
    0x0000020D, // SCENE_RM_GRACE
    0x0000020E, // SCENE_RM_HAPPY_ENT
    0x00030202, // SCENE_RM_HAPPY_MDL
    0x0000020F, 0x0001020F, // SCENE_RM_THEATER_*
    0x00000210, // SCENE_RM_AUCTION
    0x00000211, // SCENE_RM_RESET
    0x00000012, // SCENE_DM_TITLE
    0x00000013, // SCENE_DM_SAVE
    0x00000014, // SCENE_DM_LOAD
    0x00000015, // SCENE_DM_PL_SEL
    0x00000016, 0x00000016, 0x00000016, // SCENE_DM_BUS_*
    0x00000004, 0x00000004, 0x00000004, // SCENE_DM_CHKP_*
    0x00000000, // SCENE_CHECK_FIELD
    0x00000000, // SCENE_RM_SAMPLE
};

// 80479F10: the city and its buildings
static const u8 sCityScenes[SCENE_NUM] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0,
};

// 80479F54: the resource variant per scene season
static const u8 sSeasonBres[0x1C] = {
    0, 1, 1, 1, 2, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 8, 8, 9, 9, 9,
};

struct dSceneListEntry_c {
    const char *mName;
    dSceneData_c *mData;
};

// 804F0A18
static dSceneListEntry_c sSceneList[SCENE_NUM] = {
    {"FIELD", sScDataField},
    {"RM_HS0_F1", sScDataRmHs0F1},
    {"RM_HS0_F2", sScDataRmHs0F2},
    {"RM_HS0_BM", sScDataRmHs0Bm},
    {"RM_HS0_LOFT", sScDataRmHs0Loft},
    {"RM_HS1_F1", sScDataRmHs0F1},
    {"RM_HS1_F2", sScDataRmHs0F2},
    {"RM_HS1_BM", sScDataRmHs0Bm},
    {"RM_HS1_LOFT", sScDataRmHs0Loft},
    {"RM_HS2_F1", sScDataRmHs0F1},
    {"RM_HS2_F2", sScDataRmHs0F2},
    {"RM_HS2_BM", sScDataRmHs0Bm},
    {"RM_HS2_LOFT", sScDataRmHs0Loft},
    {"RM_HS3_F1", sScDataRmHs0F1},
    {"RM_HS3_F2", sScDataRmHs0F2},
    {"RM_HS3_BM", sScDataRmHs0Bm},
    {"RM_HS3_LOFT", sScDataRmHs0Loft},
    {"RM_NPC0", sScDataRmNpc0},
    {"RM_NPC1", sScDataRmNpc0},
    {"RM_NPC2", sScDataRmNpc0},
    {"RM_NPC3", sScDataRmNpc0},
    {"RM_NPC4", sScDataRmNpc0},
    {"RM_NPC5", sScDataRmNpc0},
    {"RM_NPC6", sScDataRmNpc0},
    {"RM_NPC7", sScDataRmNpc0},
    {"RM_NPC8", sScDataRmNpc0},
    {"RM_NPC9", sScDataRmNpc0},
    {"RM_OFFICE", sScDataRmOffice},
    {"RM_CHKP", sScDataRmChkp},
    {"RM_CHKP_NET", sScDataRmChkp},
    {"RM_MM_ENT", sScDataRmMmEnt},
    {"RM_MM_FISH", sScDataRmMmFish},
    {"RM_MM_INS", sScDataRmMmIns},
    {"RM_MM_FOSSIL0", sScDataRmMmFossil0},
    {"RM_MM_FOSSIL1", sScDataRmMmFossil1},
    {"RM_MM_PIC0", sScDataRmMmPic0},
    {"RM_MM_PIC1", sScDataRmMmPic1},
    {"RM_MM_CAFE", sScDataRmMmCafe},
    {"RM_MM_ASTRO", sScDataRmMmAstro},
    {"RM_SHOP0", sScDataRmShop0},
    {"RM_SHOP1", sScDataRmShop1},
    {"RM_SHOP2", sScDataRmShop2},
    {"RM_SHOP3_F1", sScDataRmShop3F1},
    {"RM_SHOP3_F2", sScDataRmShop3F2},
    {"RM_TAILOR", sScDataRmTailor},
    {"TOWN", sScDataTown},
    {"RM_BROKER", sScDataRmBroker},
    {"RM_BARBER", sScDataRmBarber},
    {"RM_FORTUNE", sScDataRmFortune},
    {"RM_GRACE", sScDataRmGrace},
    {"RM_HAPPY_ENT", sScDataRmHappyEnt},
    {"RM_HAPPY_MDL", sScDataRmHappyMdl},
    {"RM_THEATER_ENT", sScDataRmTheaterEnt},
    {"RM_THEATER_SHOW", sScDataRmTheaterShow},
    {"RM_AUCTION", sScDataRmAuction},
    {"RM_RESET", sScDataRmReset},
    {"DM_TITLE", sScDataDmTitle},
    {"DM_SAVE", sScDataDmSave},
    {"DM_LOAD", sScDataDmSave},
    {"DM_PL_SEL", sScDataDmPlSel},
    {"DM_BUS_PL_CRT", sScDataDmBusPlCrt},
    {"DM_BUS_TO_TOWN", sScDataDmBusPlCrt},
    {"DM_BUS_TO_LAND", sScDataDmBusPlCrt},
    {"DM_CHKP_CNNCT", sScDataRmChkp},
    {"DM_CHKP_DCNNCT", sScDataRmChkp},
    {"DM_CHKP_MISS", sScDataRmChkp},
    {"CHECK_FIELD", sScDataCheckField},
    {"RM_SAMPLE", sScDataRmSample},
};

static BOOL createSceneActors(dSceneDataEntry_c *entry);
static BOOL createScenePlayer(dSceneDataEntry_c *entry);
static BOOL createSceneBases(dSceneDataEntry_c *entry);
static BOOL createScene3(dSceneDataEntry_c *entry);

typedef BOOL (*dSceneCreateFunc)(dSceneDataEntry_c *entry);

// 804F0C38: per SCENE_DATA_*
static dSceneCreateFunc sCreateFuncs[] = {
    createSceneActors, createScenePlayer, createSceneBases, createScene3, NULL, NULL,
};

// 805FAF64: where the player was created
static mVec3_c sPlayerPos(0.0f, 0.0f, 0.0f);

static s32 sSeasonOverride = -1; // 8074B338: -1: from the date
static u8 sCurScene = SCENE_NUM; // 8074B33C
static u8 sPrevScene = SCENE_NUM; // 8074B33D

static u32 sE848; // 8074E848
static void *sPlayer; // 8074E84C
static fBase_c *sParent; // 8074E850
static u8 sCreated; // 8074E854

// 80161CC4
dSceneChange_c *getSceneChange() {
    return &gSceneChange;
}

// 80161CD0
dSceneExit_c::~dSceneExit_c() {}

// 80161D10
dSceneData_c *getSceneData(u8 scene) {
    if (scene == SCENE_NUM) {
        scene = sCurScene;
    }
    return sSceneList[scene].mData;
}

// 80161D34
dSceneExitList_c *getSceneExits(u8 scene) {
    dSceneData_c *data = getSceneData(scene);
    if (data != NULL) {
        return data->mExits;
    }
    return NULL;
}

// 80161D68
static BOOL createSceneActors(dSceneDataEntry_c *entry) {
    const dSceneActorData_c *data = (const dSceneActorData_c *)entry->mData;
    for (int i = 0; i < entry->mNum; i++) {
        mVec3_c pos = data->mPos;
        mAng3_c angle = data->mAngle;
        if (dActor_c::construct(data->mProfile, sParent, data->mParam, &pos, &angle) == NULL) {
            return FALSE;
        }
        data++;
    }
    return TRUE;
}

// 80161E1C
static BOOL createScenePlayer(dSceneDataEntry_c *entry) {
    u8 scene = getCurrentScene();
    if (fn_800DCEDC() && isCurrentSceneAttr(SCENE_ATTR_CHKP_ROOM) && scene != SCENE_DM_CHKP_MISS) {
        return TRUE;
    }

    const dScenePlayerData_c *data = (const dScenePlayerData_c *)entry->mData;
    mVec3_c pos = data->mPos;
    mAng3_c angle = data->mAngle;
    u32 type = data->mType;
    if (isCurrentSceneAttr(SCENE_ATTR_CHKP_ROOM)) {
        if (scene == SCENE_DM_CHKP_CNNCT) {
            pos.set(256.0f, 0.0f, 48.0f);
            angle.set(0, 0, 0);
        } else if (scene == SCENE_DM_CHKP_MISS) {
            pos.set(256.0f, 0.0f, 48.0f);
            angle.set(0, 0, 0);
        }
    }

    if (!getSceneChange()->mUseDefaultPos) {
        // Each component through its own copy of the position (the target makes three).
        const mVec3_c &p1 = getSceneChange()->mPos;
        pos.x = mVec3_c(p1.x, p1.y, p1.z).x;
        const mVec3_c &p2 = getSceneChange()->mPos;
        pos.y = mVec3_c(p2.x, p2.y, p2.z).y;
        const mVec3_c &p3 = getSceneChange()->mPos;
        pos.z = mVec3_c(p3.x, p3.y, p3.z).z;
        angle.set(0, getSceneChange()->mAngle, 0);
        type = getSceneChange()->mType;
        if (!isCityScene(scene) && !isSceneAttr(scene, SCENE_ATTR_OUTDOOR) && type != 0xB && type != 0x2E &&
            type != 0x38) {
            mVec3_c offset(-4.0f + 2.0f * (fn_800DCF58() & 3), 0.0f, 0.0f);
            offset.rotY(angle.y);
            pos += offset;
        }
    }

    sPlayerPos = pos;
    sPlayer = fn_80100234(fn_800DCF58(), 1, type, &pos, &angle);
    if (sPlayer == NULL) {
        return FALSE;
    }
    return TRUE;
}

// 801620D8
static BOOL createSceneBases(dSceneDataEntry_c *entry) {
    const dSceneBaseData_c *data = (const dSceneBaseData_c *)entry->mData;
    for (int i = 0; i < entry->mNum; i++) {
        dBase_c::createBase(data->mProfile, sParent, data->mParam, 0);
        data++;
    }
    return TRUE;
}

// 80162148
static BOOL createScene3(dSceneDataEntry_c *entry) {
    return fn_800CCDA4(entry->mData, entry->mNum);
}

// 80162158
BOOL isSceneResLoaded() {
    const u16 *ids;
    int i;
    int j;
    BOOL loaded;
    dSceneDataEntry_c *entry;
    dSceneData_c *data;

    data = getSceneData(getSceneChange()->mScene);
    loaded = TRUE;
    for (i = 0; i < data->mNum; i++) {
        entry = &data->mEntries[i];
        if (entry->mType == SCENE_DATA_RES) {
            ids = (const u16 *)entry->mData;
            for (j = 0; j < entry->mNum; j++) {
                if (!fn_80086DD0(*ids) && fn_80086E20(*ids) != 1) {
                    loaded = FALSE;
                }
                ids++;
            }
        }
    }
    return loaded;
}

// 80162214
BOOL releaseSceneRes() {
    const u16 *ids;
    int i;
    int j;
    dSceneDataEntry_c *entry;
    dSceneData_c *data;

    data = getSceneData(sCurScene);
    for (i = 0; i < data->mNum; i++) {
        entry = &data->mEntries[i];
        if (entry->mType == SCENE_DATA_RES) {
            j = entry->mNum - 1;
            ids = &((const u16 *)entry->mData)[j];
            for (; j >= 0; j--) {
                if (fn_80086DD0(*ids) && !fn_80086E00(*ids)) {
                    return FALSE;
                }
                ids--;
            }
        }
    }
    return TRUE;
}

// 801622D4
u32 getSeasonFromTime(const dTime_c *time) {
    dTime_c t = *time;
    if (t.hour < TIME_DAY_START_HOUR) {
        t.add(-1, 0, 0, 0);
    }
    return sTermSeasons[t.getTerm()];
}

// 80162374
void updateCurrentScene() {
    sPrevScene = sCurScene;
    sCurScene = getSceneChange()->mScene;
}

// 801623A4
BOOL createScene(fBase_c *parent) {
    getSceneChange()->setChanging(FALSE);
    BOOL skipSeason = FALSE;
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        skipSeason = TRUE;
    }
    if (!skipSeason) {
        updateSaveSeason();
    }
    sParent = parent;
    sCreated = TRUE;
    getSceneChange()->setScene(SCENE_NONE);
    isOutdoorScene(sCurScene);
    fn_80082794();
    dFgMngProc_c::create();
    fn_800C7704();
    fn_801A6584();

    int i;
    dSceneData_c *data = getSceneData(SCENE_NUM);
    BOOL ok = TRUE;
    for (i = 0; i < data->mNum; i++) {
        dSceneCreateFunc func = sCreateFuncs[data->mEntries[i].mType];
        if (func != NULL) {
            ok = func(&data->mEntries[i]);
        }
        if (!ok) {
            return FALSE;
        }
    }
    fn_80186708();
    fn_80186260();
    if (getCurrentScene() != SCENE_FIELD && getCurrentScene() != SCENE_RM_OFFICE) {
        lbl_8074EACD = 0;
    }
    return ok;
}

// 801624D8
void destroyScene() {
    sE848 = 0;
    sPlayer = NULL;
    sParent = NULL;
    fn_8008279C();
    dFgMngProc_c::destroy();
    fn_800C7708();
    fn_801A670C();
    getSceneChange()->setChanging(FALSE);
}

// 80162524
u32 getSceneAttr(u8 scene) {
    if (scene < SCENE_NUM) {
        return sSceneAttr[scene];
    }
    return 0;
}

// 80162548
u8 getCurrentScene() {
    return sCurScene;
}

// 80162550
u8 getPrevScene() {
    return sPrevScene;
}

// 80162558
bool isOutdoorScene(u8 scene) {
    return isSceneAttr(scene, SCENE_ATTR_OUTDOOR);
}

// 80162588
void clearSceneCreated() {
    sCreated = FALSE;
}

// 80162594
BOOL isSceneAttr(u8 scene, u32 attr) {
    return (getSceneAttr(scene) & attr) == attr;
}

// 801625D0
BOOL isCurrentSceneAttr(u32 attr) {
    return isSceneAttr(getCurrentScene(), attr);
}

// 80162608
int getSceneAttrIndex(u8 scene, int attr) {
    if ((attr & (int)getSceneAttr(scene)) != attr) {
        return -1;
    }
    int idx = 0;
    for (u32 i = 0; i < SCENE_NUM; i++) {
        int a = getSceneAttr(i);
        if (scene == (u8)i) {
            return idx;
        }
        if ((attr & a) == attr) {
            idx++;
        }
    }
    return -1;
}

// 801626A8
int getSceneAttrTableIndex(u8 scene, const u32 *table, int *index) {
    if (index != NULL) {
        *index = -1;
    }
    int i = 0;
    while (*table != 0) {
        const u32 &attr = *table; // reloaded: not merged with the loop test's load
        int res = getSceneAttrIndex(scene, attr);
        if (res != -1) {
            if (index != NULL) {
                *index = i;
            }
            return res;
        }
        i++;
        table++;
    }
    return -1;
}

// 80162744
BOOL isCurrentSceneMyHouse() {
    return isMyHouseScene(getCurrentScene());
}

// 8016276C
BOOL isMyHouseScene(u8 scene) {
    int home;
    getSceneAttrTableIndex(scene, sHomeScenes, &home);
    if (home != -1) {
        return dSaveData_c::getTown()->mHomes.findCurrentPlayer() == home;
    }
    return FALSE;
}

// 801627CC
u16 getSceneBuilding(u8 scene) {
    if (scene < SCENE_NUM) {
        return sSceneBuildings[scene];
    }
    return 0xFFF1;
}

// 801627F4
int getSceneParamA(u8 scene) {
    if (scene < SCENE_NUM) {
        u8 value = sSceneParamA[scene];
        return value > 0x40 ? 0x20 : value;
    }
    return 0;
}

// 80162828
u8 getSceneParamB(u8 scene) {
    if (scene < SCENE_NUM) {
        return sSceneParamB[scene];
    }
    return 0;
}

// 80162848
u32 getSceneParamC(u8 scene) {
    if (scene < SCENE_NUM) {
        return sSceneParamC[scene];
    }
    return 0;
}

// 8016286C
bool isCityScene(u8 scene) {
    if (scene < SCENE_NUM) {
        return sCityScenes[scene];
    }
    return false;
}

// 80162898
u32 findSceneByBuilding(u16 building) {
    for (u32 i = 0; i < SCENE_NUM; i++) {
        if (building == getSceneBuilding(i)) {
            return i;
        }
    }
    return SCENE_FIELD;
}

// 801628F8
void updateSaveSeason() {
    int season = sSeasonOverride;
    if (season == -1) {
        season = getSeasonFromTime(dTime_c::getCurrent());
    }
    u8 s = season;
    if (s < SCENE_SEASON_NUM) {
        dSaveData_c::getTown()->mMainField.setSeason(s);
    } else {
        dSaveData_c::getTown()->mMainField.setSeason(0);
    }
}

// 8016295C
u32 getSaveSeason() {
    return dSaveData_c::getRaw()->mMainField.mSeason;
}

// 80162984
u32 getSeason(const dTime_c *time) {
    return getSeasonFromTime(time);
}

// 80162988
u32 getSeasonBres(u32 season) {
    if (season < SCENE_SEASON_NUM) {
        return sSeasonBres[season];
    }
    return 0;
}

// 801629A8
u32 getCurrentSeasonBres() {
    return getSeasonBres(getSaveSeason());
}

// 801629CC
s8 getSeasonType() {
    if (fn_80169124() != -1) {
        return (fn_80169124() & 1) ? 'W' : 'S';
    }
    return getCurrentSeasonBres() == SCENE_SEASON_BRES_SNOW ? 'W' : 'S';
}

// 80162A20
BOOL isWinterSeasonType() {
    return getSeasonType() == 'W';
}

// 80162A50
BOOL isSnowSeason(const dTime_c *time) {
    return getSeasonBres(getSeason(time)) == SCENE_SEASON_BRES_SNOW;
}

// 80162A80
BOOL isLateSeason() {
    return getSaveSeason() >= 0x11;
}

// 80162AB8
BOOL isLateSeason(const dTime_c *time) {
    return getSeason(time) >= 0x11;
}

// 80162AF0
BOOL isSeason2() {
    return getSaveSeason() == 2;
}

// 80162B1C
BOOL requestSceneChange(u8 scene, int wipeIn, int wipeOut) {
    return getSceneChange()->request(scene, wipeIn, wipeOut, FALSE);
}

// 80162B74
BOOL forceSceneChange(u8 scene, int wipeIn, int wipeOut) {
    return getSceneChange()->request(scene, wipeIn, wipeOut, TRUE);
}

// 80162BCC
u8 getNextScene() {
    return getSceneChange()->getScene();
}

// 80162BF0
void setSceneE848(u32 value) {
    sE848 = value;
}

// 80162BF8
u32 getSceneE848() {
    return sE848;
}

// 80162C00
void *getScenePlayer() {
    return sPlayer;
}

// 80162C08
fBase_c *getSceneParent() {
    return sParent;
}

// 80162C10
mVec3_c *getScenePlayerPos() {
    return &sPlayerPos;
}

// 80162C1C
void dSceneExit_c::init() {
    static mVec3_c sDefaultPos(768.0f, 0.0f, 768.0f);
    set(SCENE_FIELD, &sDefaultPos, 8, 0, -1, -1);
}

// 80162CAC
void dSceneExit_c::set(u8 scene, const mVec3_c *pos, u32 type, s16 angle, int blockX, int blockZ) {
    mScene = scene;
    mPos = *pos;
    mType = type;
    mAngle = angle;
    mBlockX = blockX;
    mBlockZ = blockZ;
}

// 80162CDC
dSceneChange_c::dSceneChange_c() {
    mBuilding = 0xFFF1;
    mScene = SCENE_NONE;
    mPos = mVec3_c::Zero;
    mType = 0;
    mAngle = 0;
    mUseDefaultPos = TRUE;
    _1A = 0;
    _18 = 0;
    mChanging = FALSE;
}

// 80162D34
static u8 adjustChkpScene(u8 scene) {
    BOOL online = fn_800DCEDC();
    if (scene == SCENE_RM_CHKP) {
        if (online && !fn_800DCEE0()) {
            scene = SCENE_RM_CHKP_NET;
        }
    } else if (scene == SCENE_RM_CHKP_NET) {
        if (!online) {
            scene = SCENE_RM_CHKP;
        }
    }
    return scene;
}

// 80162D9C
u8 dSceneChange_c::getScene() {
    return mScene;
}

// 80162DA4
BOOL dSceneChange_c::request(u8 scene, int wipeIn, int wipeOut, BOOL force) {
    if ((!fn_800DD6B4() && mScene == SCENE_NONE) || force) {
        mScene = adjustChkpScene(scene);
        u16 outType = 0x1E;
        u16 inType = 0x1E;
        mUseDefaultPos = TRUE;
        if (fn_8008B91C() == 0) {
            switch (wipeIn) {
            case 2:
                outType = 0x26;
                break;
            case 3:
            case 4:
                outType = 0x2A;
                break;
            case 0:
                outType = 0x36;
                break;
            case 1:
                outType = 0x3C;
                break;
            case 5:
                outType = 1;
                break;
            }
        } else {
            outType = fn_8008B91C();
        }
        if (fn_8008B924() == 0) {
            switch (wipeOut) {
            case 2:
                inType = 0x26;
                break;
            case 3:
            case 4:
                inType = 0x2A;
                break;
            case 0:
                inType = 0x36;
                break;
            case 1:
                inType = 0xF;
                break;
            case 5:
                inType = 1;
                break;
            }
        } else {
            inType = fn_8008B924();
        }
        lbl_8074B1B6 = outType;
        lbl_8074B1B4 = inType;
        lbl_8074B1B0 = 1;
        lbl_8074E804 = wipeOut;
        lbl_8074E808 = wipeIn;
        if (fn_800DCEDC() && !fn_800DCEE0()) {
            fn_800DD6D8(1);
        }
        getSceneChange()->setChanging(TRUE);
        return TRUE;
    }
    return FALSE;
}

// 80162F40
BOOL dSceneChange_c::request(u8 scene, const mVec3_c *pos, u32 type, s16 angle, int wipeIn, int wipeOut) {
    if (request(scene, wipeIn, wipeOut, FALSE)) {
        mPos = *pos;
        mType = type;
        mAngle = angle;
        mUseDefaultPos = FALSE;
        return TRUE;
    }
    return FALSE;
}

// 80162FD8
static BOOL getBuildingExit(u16 building, u8 *scene, mVec3_c *pos, u32 *type, s16 *angle, int *blockX,
                            int *blockZ) {
    u8 flags = 1;
    if (dItem::Item(building).isExtId()) {
        flags = fn_80167FB4(dItem::Item(building).getExtIndex())->mFlags;
    }
    dItem::Item item(building);
    int x, z;
    dSaveData_c *save = dSaveData_c::getTown();
    if (fn_8014B0F0(&save->_05EB04, &x, &z, &item, flags)) {
        dSceneBuilding_c *bld = fn_80167734(fn_801683CC(), building);
        if (bld != NULL) {
            if (flags & 1) {
                *scene = SCENE_FIELD;
            } else if (flags & 2) {
                *scene = SCENE_TOWN;
            }
            mVec3_c p;
            fn_801681B8(&p, bld, building, 1, 32.0f);
            *pos = p;
            if (flags & 2) {
                pos->y = lbl_807503F0;
            }
            *type = 0xB;
            *angle = (u16)bld->mAngle;
            *blockX = x;
            *blockZ = z;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// 80163154
int dSceneChange_c::getExitDest(u32 idx, u8 *scene, mVec3_c *pos, u32 *type, s16 *angle, int *blockX,
                                int *blockZ) {
    dSceneExitList_c *exits = getSceneExits(sCurScene);
    if (exits != NULL && exits->mExits != NULL && idx < exits->mNum) {
        dSceneExitData_c *exit = &exits->mExits[idx];
        int home;
        getSceneAttrTableIndex(sCurScene, sHomeScenes, &home);
        if (home < 0) {
            home = 0;
        }
        home &= 3;
        switch (exit->mScenes[home]) {
        case SCENE_EXIT_HOUSE0:
        case SCENE_EXIT_HOUSE1:
        case SCENE_EXIT_HOUSE2:
        case SCENE_EXIT_HOUSE3:
            return getBuildingExit(0xD001 + ((exit->mScenes[home] - SCENE_EXIT_HOUSE0) & 3), scene, pos, type, angle,
                                   blockX, blockZ)
                       ? SCENE_EXIT_DEST_BUILDING
                       : SCENE_EXIT_DEST_NONE;
        case SCENE_EXIT_GATE:
            return getBuildingExit(0xD014, scene, pos, type, angle, blockX, blockZ) ? SCENE_EXIT_DEST_BUILDING
                                                                                   : SCENE_EXIT_DEST_NONE;
        case SCENE_NONE: {
            dSceneExit_c *ret = &gSceneReturnExit;
            *scene = ret->mScene;
            *pos = ret->mPos;
            *type = ret->mType;
            *angle = ret->mAngle;
            *blockX = ret->mBlockX;
            *blockZ = ret->mBlockZ;
            return SCENE_EXIT_DEST_RETURN;
        }
        case SCENE_EXIT_BUILDING:
            return getBuildingExit(getSceneBuilding(getCurrentScene()), scene, pos, type, angle, blockX, blockZ)
                       ? SCENE_EXIT_DEST_BUILDING
                       : SCENE_EXIT_DEST_NONE;
        case SCENE_EXIT_CHANGE:
            return getBuildingExit(mBuilding, scene, pos, type, angle, blockX, blockZ) ? SCENE_EXIT_DEST_BUILDING
                                                                                      : SCENE_EXIT_DEST_NONE;
        }
    }
    return SCENE_EXIT_DEST_NONE;
}

// 80163380
int dSceneChange_c::getExit(u32 idx, u8 *scene, mVec3_c *pos, u32 *type, s16 *angle, int *blockX, int *blockZ) {
    int tmpX;
    int tmpZ;
    if (blockX == NULL) {
        blockX = &tmpX;
    }
    if (blockZ == NULL) {
        blockZ = &tmpZ;
    }
    int res = getExitDest(idx, scene, pos, type, angle, blockX, blockZ);
    if (res == SCENE_EXIT_DEST_NONE) {
        int home;
        getSceneAttrTableIndex(sCurScene, sHomeScenes, &home);
        if (home < 0) {
            home = 0;
        }
        home &= 3;
        dSceneExitList_c *exits = getSceneExits(sCurScene);
        if (exits != NULL && exits->mExits != NULL && idx < exits->mNum) {
            dSceneExitData_c *exit = &exits->mExits[idx];
            if (exit->mScenes[home] < SCENE_NUM) {
                *scene = exit->mScenes[home];
            } else {
                *scene = SCENE_FIELD;
            }
            *pos = exit->mPos;
            *type = exit->mType;
            *angle = exit->mAngle;
            return res;
        }
        return SCENE_EXIT_DEST_INVALID;
    }
    return res;
}

// 801634AC
BOOL dSceneChange_c::requestExit(u32 idx, int wipeIn, int wipeOut) {
    u8 scene;
    s16 angle;
    u32 type;
    int blockX;
    int blockZ;
    mVec3_c pos;
    int res = getExit(idx, &scene, &pos, &type, &angle, &blockX, &blockZ);
    if (res != SCENE_EXIT_DEST_INVALID && request(scene, &pos, type, angle, wipeIn, wipeOut)) {
        switch (res) {
        case SCENE_EXIT_DEST_NONE:
            break;
        case SCENE_EXIT_DEST_RETURN:
            gSceneReturnExit.set(scene, &pos, type, angle, blockX, blockZ);
            break;
        case SCENE_EXIT_DEST_BUILDING:
            gSceneBuildingExit.set(scene, &pos, type, angle, blockX, blockZ);
            break;
        }
        return TRUE;
    }
    return FALSE;
}

// 801635B8
BOOL dSceneChange_c::getExitPos(u32 idx, u32 *type, mVec3_c *pos, s16 *angle, const mVec3_c *ref) {
    s16 tmpAngle;
    u32 tmpType;
    mVec3_c tmpPos;
    if (type == NULL) {
        type = &tmpType;
    }
    if (angle == NULL) {
        angle = &tmpAngle;
    }
    if (pos == NULL) {
        pos = &tmpPos;
    }
    *type = 0;
    *angle = 0;
    *pos = *ref;
    dSceneExitList_c *exits = getSceneExits(sCurScene);
    if (exits != NULL && exits->mExits != NULL && idx < exits->mNum) {
        mVec3_c center;
        dFdBase_c::snapToUnitCenter(&center, ref);
        dSceneExitData_c *exit = &exits->mExits[idx];
        *type = exit->mPosType;
        *angle = exit->mPosAngle;
        pos->x = center.x;
        pos->y = ref->y;
        pos->z = center.z;
        if (*angle == 0 || *angle == -0x8000) {
            pos->x = ref->x;
            return TRUE;
        }
        if (*angle == 0x4000 || *angle == -0x4000) {
            pos->z = ref->z;
            return TRUE;
        }
        return TRUE;
    }
    return FALSE;
}

// 801636FC
BOOL dSceneChange_c::setReturnExit(u8 scene, const mVec3_c *pos, u32 type, s16 angle) {
    if (scene < SCENE_NUM) {
        gSceneReturnExit.set(scene, pos, type, angle, -1, -1);
        return TRUE;
    }
    return FALSE;
}

// 80163740
void dSceneChange_c::setReturnExitHere(f32 dz) {
    s16 angle;
    u32 type;
    mVec3_c pos;
    fn_800FE688(&type, &pos, &angle);
    pos.z += dz;
    gSceneReturnExit.set(getCurrentScene(), &pos, type, angle, -1, -1);
}

dSceneChange_c gSceneChange; // 805FAF98
dSceneExit_c gSceneReturnExit; // 805FAFC8
dSceneExit_c gSceneBuildingExit; // 805FAFF0
