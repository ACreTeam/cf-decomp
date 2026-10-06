#pragma once

#include <types.h>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

// Scenes (rooms, the town, demos) and the scene manager: the current / previous scene, the scene
// attribute table, scene changes and the scene's actor list. Source: src/dol/game/d_scene.cpp
// (.text 80161CC4..80163AA0). The SCENE_ names are the game's own scene names (the name table
// at 804F0A18); the other names are inferred.

class fBase_c;
class dTime_c;

enum Scene_e {
    SCENE_FIELD = 0x00, // the player's town
    SCENE_RM_HS0_F1 = 0x01, // player house 0: main floor, upstairs, basement, loft
    SCENE_RM_HS0_F2 = 0x02,
    SCENE_RM_HS0_BM = 0x03,
    SCENE_RM_HS0_LOFT = 0x04,
    SCENE_RM_HS1_F1 = 0x05,
    SCENE_RM_HS1_F2 = 0x06,
    SCENE_RM_HS1_BM = 0x07,
    SCENE_RM_HS1_LOFT = 0x08,
    SCENE_RM_HS2_F1 = 0x09,
    SCENE_RM_HS2_F2 = 0x0A,
    SCENE_RM_HS2_BM = 0x0B,
    SCENE_RM_HS2_LOFT = 0x0C,
    SCENE_RM_HS3_F1 = 0x0D,
    SCENE_RM_HS3_F2 = 0x0E,
    SCENE_RM_HS3_BM = 0x0F,
    SCENE_RM_HS3_LOFT = 0x10,
    SCENE_RM_NPC0 = 0x11, // villager houses
    SCENE_RM_NPC1 = 0x12,
    SCENE_RM_NPC2 = 0x13,
    SCENE_RM_NPC3 = 0x14,
    SCENE_RM_NPC4 = 0x15,
    SCENE_RM_NPC5 = 0x16,
    SCENE_RM_NPC6 = 0x17,
    SCENE_RM_NPC7 = 0x18,
    SCENE_RM_NPC8 = 0x19,
    SCENE_RM_NPC9 = 0x1A,
    SCENE_RM_OFFICE = 0x1B, // town hall
    SCENE_RM_CHKP = 0x1C, // gate (check point)
    SCENE_RM_CHKP_NET = 0x1D, // gate, online
    SCENE_RM_MM_ENT = 0x1E, // museum
    SCENE_RM_MM_FISH = 0x1F,
    SCENE_RM_MM_INS = 0x20,
    SCENE_RM_MM_FOSSIL0 = 0x21,
    SCENE_RM_MM_FOSSIL1 = 0x22,
    SCENE_RM_MM_PIC0 = 0x23,
    SCENE_RM_MM_PIC1 = 0x24,
    SCENE_RM_MM_CAFE = 0x25,
    SCENE_RM_MM_ASTRO = 0x26,
    SCENE_RM_SHOP0 = 0x27, // Nook's store, one per stage
    SCENE_RM_SHOP1 = 0x28,
    SCENE_RM_SHOP2 = 0x29,
    SCENE_RM_SHOP3_F1 = 0x2A,
    SCENE_RM_SHOP3_F2 = 0x2B,
    SCENE_RM_TAILOR = 0x2C,
    SCENE_TOWN = 0x2D, // the city
    SCENE_RM_BROKER = 0x2E,
    SCENE_RM_BARBER = 0x2F,
    SCENE_RM_FORTUNE = 0x30,
    SCENE_RM_GRACE = 0x31,
    SCENE_RM_HAPPY_ENT = 0x32,
    SCENE_RM_HAPPY_MDL = 0x33, // the model room
    SCENE_RM_THEATER_ENT = 0x34,
    SCENE_RM_THEATER_SHOW = 0x35,
    SCENE_RM_AUCTION = 0x36,
    SCENE_RM_RESET = 0x37,
    SCENE_DM_TITLE = 0x38,
    SCENE_DM_SAVE = 0x39,
    SCENE_DM_LOAD = 0x3A,
    SCENE_DM_PL_SEL = 0x3B,
    SCENE_DM_BUS_PL_CRT = 0x3C,
    SCENE_DM_BUS_TO_TOWN = 0x3D,
    SCENE_DM_BUS_TO_LAND = 0x3E,
    SCENE_DM_CHKP_CNNCT = 0x3F,
    SCENE_DM_CHKP_DCNNCT = 0x40,
    SCENE_DM_CHKP_MISS = 0x41,
    SCENE_CHECK_FIELD = 0x42,
    SCENE_RM_SAMPLE = 0x43,

    SCENE_NUM = 0x44,

    // Not scenes: getSceneData() takes SCENE_NUM for the current scene; scene exits use 0x44..0x4B
    // for the exits of buildings in the town (see dSceneChange_c::getExitDest).
    SCENE_EXIT_BUILDING = 0x44, // the exit of the current scene's building
    SCENE_EXIT_CHANGE = 0x45, // the exit of building dSceneChange_c::mBuilding
    SCENE_EXIT_HOUSE0 = 0x46, // the exit of player house 0..3
    SCENE_EXIT_HOUSE1 = 0x47,
    SCENE_EXIT_HOUSE2 = 0x48,
    SCENE_EXIT_HOUSE3 = 0x49,
    SCENE_EXIT_GATE = 0x4A, // the exit of the gate
    SCENE_NONE = 0x4B, // no scene change pending; as an exit: back to gSceneReturnExit
};

// Scene attributes (getSceneAttr; the table sSceneAttr). isSceneAttr(scene, mask) is true when
// the scene has all bits of the mask.
enum {
    SCENE_ATTR_OUTDOOR = 0x1, // the town, the city, the title demo, the check field
    SCENE_ATTR_CHECK_FIELD = 0x2,
    SCENE_ATTR_MY_TOWN = 0x4, // the player's town (with SCENE_ATTR_OUTDOOR)
    SCENE_ATTR_CITY = 0x8,
    SCENE_ATTR_ROOM = 0x10, // indoors
    SCENE_ATTR_DEMO = 0x20,
    SCENE_ATTR_FTR_ROOM = 0x40, // a room with a furniture layout (houses, museum exhibits, model room)
    SCENE_ATTR_FACILITY = 0x80, // a staffed room (town hall, shops, city facilities)
    SCENE_ATTR_CHKP = 0x100, // the gate and its connection demos
    SCENE_ATTR_SHOP = 0x200,
    SCENE_ATTR_HOUSE = 0x400, // a player house room
    SCENE_ATTR_NPC_HOUSE = 0x800, // a villager house
    SCENE_ATTR_STORE = 0x1000, // Nook's store
    SCENE_ATTR_MUSEUM = 0x2000,
    SCENE_ATTR_BUS = 0x4000,
    SCENE_ATTR_HOUSE0 = 0x01000000, // a room of player house 0..3
    SCENE_ATTR_HOUSE1 = 0x02000000,
    SCENE_ATTR_HOUSE2 = 0x04000000,
    SCENE_ATTR_HOUSE3 = 0x08000000,
    SCENE_ATTR_STORE0 = 0x10000000, // Nook's store stage 0..3
    SCENE_ATTR_STORE1 = 0x20000000,
    SCENE_ATTR_STORE2 = 0x40000000,
    SCENE_ATTR_STORE3 = 0x80000000,

    // Combinations used as masks.
    SCENE_ATTR_TOWN = SCENE_ATTR_OUTDOOR | SCENE_ATTR_MY_TOWN, // 0x5
    SCENE_ATTR_CITY_FIELD = SCENE_ATTR_OUTDOOR | SCENE_ATTR_CITY, // 0x9
    SCENE_ATTR_PLAYER_HOUSE = SCENE_ATTR_ROOM | SCENE_ATTR_FTR_ROOM | SCENE_ATTR_HOUSE, // 0x450
    SCENE_ATTR_VILLAGER_HOUSE = SCENE_ATTR_ROOM | SCENE_ATTR_FTR_ROOM | SCENE_ATTR_NPC_HOUSE, // 0x850
    SCENE_ATTR_SHOP_ROOM = SCENE_ATTR_ROOM | SCENE_ATTR_SHOP, // 0x210
    SCENE_ATTR_STORE_ROOM = SCENE_ATTR_ROOM | SCENE_ATTR_FACILITY | SCENE_ATTR_SHOP | SCENE_ATTR_STORE, // 0x1290
    SCENE_ATTR_CHKP_ROOM = SCENE_ATTR_ROOM | SCENE_ATTR_DEMO | SCENE_ATTR_CHKP, // 0x130
};

// Scene seasons (getSaveSeason, getSeasonFromTime: one per dTime_c term) and their resource
// variants (getSeasonBres).
#define SCENE_SEASON_NUM 22
#define SCENE_SEASON_BRES_SNOW 9

// ---------------------------------------------------------------------------------------------
// Scene data: per scene, a list of creation steps (actors, the player, bases, ...) and the exits.

// SCENE_DATA_ACTOR: dActor_c::construct per entry.
struct dSceneActorData_c {
    /* 0x00 */ u16 mProfile;
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ mAng3_c mAngle;
    /* 0x18 */ u32 mParam;
}; // size 0x1C

// SCENE_DATA_PLAYER: the default player position.
struct dScenePlayerData_c {
    /* 0x00 */ mVec3_c mPos;
    /* 0x0C */ mAng3_c mAngle;
    /* 0x14 */ u32 mType;
}; // size 0x18

// SCENE_DATA_BASE: dBase_c::createBase per entry.
struct dSceneBaseData_c {
    /* 0x00 */ u16 mProfile;
    /* 0x04 */ u32 mParam;
}; // size 0x8

enum {
    SCENE_DATA_ACTOR,
    SCENE_DATA_PLAYER,
    SCENE_DATA_BASE,
    SCENE_DATA_3,
    SCENE_DATA_RES, // resource ids (u16)
    SCENE_DATA_5,
};

struct dSceneDataEntry_c {
    /* 0x00 */ u16 mType; // SCENE_DATA_*
    /* 0x02 */ u16 mNum;
    /* 0x04 */ void *mData;
}; // size 0x8

// An exit of a scene.
struct dSceneExitData_c {
    /* 0x00 */ u8 mScenes[4]; // the destination, per player house (0 outside houses), or SCENE_EXIT_*
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ s16 mAngle;
    /* 0x14 */ u32 mType;
    /* 0x18 */ u8 mPosType;
    /* 0x1A */ s16 mPosAngle;
}; // size 0x1C

struct dSceneExitList_c {
    /* 0x00 */ dSceneExitData_c *mExits;
    /* 0x04 */ u32 mNum;
};

struct dSceneData_c {
    /* 0x00 */ u16 mNum;
    /* 0x04 */ dSceneDataEntry_c *mEntries;
    /* 0x08 */ dSceneExitList_c *mExits;
};

// ---------------------------------------------------------------------------------------------

// dSceneChange_c::getExitDest / getExit results.
enum {
    SCENE_EXIT_DEST_NONE, // a plain exit (getExit reads it from the exit data)
    SCENE_EXIT_DEST_RETURN, // back to gSceneReturnExit
    SCENE_EXIT_DEST_BUILDING, // out of a building in the town (also kept in gSceneBuildingExit)
    SCENE_EXIT_DEST_INVALID, // getExit: no such exit
};

// A scene and a position in it to go back to (gSceneReturnExit, gSceneBuildingExit).
class dSceneExit_c {
public:
    dSceneExit_c() { init(); }
    ~dSceneExit_c(); // 80161CD0

    void init(); // 80162C1C: the town, (768, 0, 768)

    void set(u8 scene, const mVec3_c *pos, u32 type, s16 angle, int blockX, int blockZ); // 80162CAC

    /* 0x00 */ mVec3_c mPos;
    /* 0x0C */ u32 mType;
    /* 0x10 */ s16 mAngle;
    /* 0x12 */ u8 mScene;
    /* 0x13 */ s8 mBlockX;
    /* 0x14 */ s8 mBlockZ;
}; // size 0x18

// The pending scene change (gSceneChange, getSceneChange()).
class dSceneChange_c {
public:
    dSceneChange_c(); // 80162CDC

    u8 getScene(); // 80162D9C
    void setScene(u8 scene) { mScene = scene; }
    void setChanging(BOOL changing) { mChanging = changing; }
    BOOL request(u8 scene, int wipeIn, int wipeOut, BOOL force); // 80162DA4
    BOOL request(u8 scene, const mVec3_c *pos, u32 type, s16 angle, int wipeIn, int wipeOut); // 80162F40
    int getExitDest(u32 idx, u8 *scene, mVec3_c *pos, u32 *type, s16 *angle, int *blockX, int *blockZ); // 80163154
    int getExit(u32 idx, u8 *scene, mVec3_c *pos, u32 *type, s16 *angle, int *blockX, int *blockZ); // 80163380
    BOOL requestExit(u32 idx, int wipeIn, int wipeOut); // 801634AC
    BOOL getExitPos(u32 idx, u32 *type, mVec3_c *pos, s16 *angle, const mVec3_c *ref); // 801635B8
    BOOL setReturnExit(u8 scene, const mVec3_c *pos, u32 type, s16 angle); // 801636FC
    void setReturnExitHere(f32 dz); // 80163740: the player's position

    /* 0x00 */ u8 mScene; // SCENE_NONE: none
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ s16 mAngle;
    /* 0x14 */ u32 mType;
    /* 0x18 */ u8 _18;
    /* 0x1A */ u16 _1A;
    /* 0x1C */ u16 mBuilding; // for SCENE_EXIT_CHANGE
    /* 0x1E */ u8 mUseDefaultPos; // the scene's default player position instead of mPos
    /* 0x1F */ u8 mChanging;
}; // size 0x20

extern dSceneChange_c gSceneChange; // 805FAF98
extern dSceneExit_c gSceneReturnExit; // 805FAFC8
extern dSceneExit_c gSceneBuildingExit; // 805FAFF0

dSceneChange_c *getSceneChange(); // 80161CC4
dSceneData_c *getSceneData(u8 scene); // 80161D10: SCENE_NUM for the current scene
dSceneExitList_c *getSceneExits(u8 scene); // 80161D34
BOOL isSceneResLoaded(); // 80162158: the next scene's resources
BOOL releaseSceneRes(); // 80162214
u32 getSeasonFromTime(const dTime_c *time); // 801622D4
void updateCurrentScene(); // 80162374: the pending scene becomes the current one
BOOL createScene(fBase_c *parent); // 801623A4
void destroyScene(); // 801624D8
u32 getSceneAttr(u8 scene); // 80162524
u8 getCurrentScene(); // 80162548
u8 getPrevScene(); // 80162550
bool isOutdoorScene(u8 scene); // 80162558
void clearSceneCreated(); // 80162588
BOOL isSceneAttr(u8 scene, u32 attr); // 80162594
BOOL isCurrentSceneAttr(u32 attr); // 801625D0
int getSceneAttrIndex(u8 scene, int attr); // 80162608: among the scenes with attr; -1 if none
int getSceneAttrTableIndex(u8 scene, const u32 *table, int *index); // 801626A8
BOOL isCurrentSceneMyHouse(); // 80162744
BOOL isMyHouseScene(u8 scene); // 8016276C
u16 getSceneBuilding(u8 scene); // 801627CC: its building item (0xD0xx) in the town, or 0xFFF1
int getSceneParamA(u8 scene); // 801627F4
u8 getSceneParamB(u8 scene); // 80162828
u32 getSceneParamC(u8 scene); // 80162848
bool isCityScene(u8 scene); // 8016286C
u32 findSceneByBuilding(u16 building); // 80162898
void updateSaveSeason(); // 801628F8
u32 getSaveSeason(); // 8016295C
u32 getSeason(const dTime_c *time); // 80162984
u32 getSeasonBres(u32 season); // 80162988
u32 getCurrentSeasonBres(); // 801629A8
s8 getSeasonType(); // 801629CC: 'S' or 'W'
BOOL isWinterSeasonType(); // 80162A20
BOOL isSnowSeason(const dTime_c *time); // 80162A50
BOOL isLateSeason(); // 80162A80
BOOL isLateSeason(const dTime_c *time); // 80162AB8
BOOL isSeason2(); // 80162AF0
BOOL requestSceneChange(u8 scene, int wipeIn, int wipeOut); // 80162B1C
BOOL forceSceneChange(u8 scene, int wipeIn, int wipeOut); // 80162B74
u8 getNextScene(); // 80162BCC
void setSceneE848(u32 value); // 80162BF0
u32 getSceneE848(); // 80162BF8
void *getScenePlayer(); // 80162C00
fBase_c *getSceneParent(); // 80162C08
mVec3_c *getScenePlayerPos(); // 80162C10
