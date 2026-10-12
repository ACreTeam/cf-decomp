// The scene data of RM_THEATER_SHOW (dSceneData_c sScDataRmTheaterShow; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static ColorByte_c l_palette0 = {{0x58, 0x48, 0x40, 0xFF}, 0x6B};
static u16 l_resources[] = {0x0072, 0x001C, 0x0053};
static PlayerData_c l_player = {{144.0f, 0.0f, 432.0f}, {0, -0x8000, 0}, 0, 8};
static dSceneBaseData_c l_base[] = {{0x00A3, 0}, {0x00AD, 0}};
static ActorData_c l_actors[] = {
    {0x0072, 0, {256.0f, 0.0f, 224.0f}, {0, 0, 0}, 0, (void *)0x8020},
    {0x001C, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, 0},
};
static u32 l_setting0[] = {0x806840FF, 0xE0000000, 0x484020FF, 0x10000000, 0x01000000, 0x3C888889, 0x3BB60B61}; // scene setting kind 0xB
static u32 l_setting1[] = {0xFFFFC0FF, 0x43800000, 0x43200000, 0x43A00000, 0xD0000000, 0x3EC00000, 0x00000000, 0x00000000, 0x420C0000, 0x00000004}; // scene setting kind 0xE
static Scene3Entry_c l_scene3[] = {{5, &l_palette0}, {0xB, l_setting0}, {0xE, l_setting1}};
static dSceneDataEntry_c l_entries[] = {
    {SCENE_DATA_PLAYER, 1, &l_player},
    {SCENE_DATA_3, ARRAY_SIZE(l_scene3), l_scene3},
    {SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {SCENE_DATA_BASE, dScData::g_roomBaseNum, dScData::g_roomBase},
    {SCENE_DATA_RES, dScData::g_roomResNum, dScData::g_roomRes},
    {SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
    {SCENE_DATA_ACTOR, ARRAY_SIZE(l_actors), l_actors},
    {SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x34, 0x34, 0x34, 0x34}, {208.0f, 0.0f, 272.0f}, 0, 0, 0x4A, 3, 0, 0},
    {{0x34, 0x34, 0x34, 0x34}, {304.0f, 0.0f, 272.0f}, 0, 0, 0x4A, 3, 0, 0},
};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01AE, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataRmTheaterShow = &dScData::l_scene.mData;
