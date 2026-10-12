// The scene data of RM_HAPPY_ENT (dSceneData_c sScDataRmHappyEnt; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static EntParam_c l_param0(0x33, 10, 9, 21, 0);
static dSceneBaseData_c l_base[] = {{0x00A3, 0}};
static ColorByte_c l_palette0 = {{0x52, 0x52, 0x94, 0xFF}, 0xCE};
static u16 l_resources[] = {0x0073, 0x000E};
static PlayerData_c l_player = {{256.0f, 0.0f, 464.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {0x0073, 0, {256.0f, 0.0f, 320.0f}, {0, 0, 0}, 0, (void *)0x8018},
    {0x000E, 0, {256.0f, 0.0f, 272.0f}, {0, 0, 0}, 0, &l_param0},
};
static u32 l_setting0[] = {0xF7F7CEFF, 0xD0000000, 0x4242C6FF, 0x01000000, 0x3D088889, 0x3D088889}; // scene setting kind 0xA
static Scene3Entry_c l_scene3[] = {{5, &l_palette0}, {0xA, l_setting0}};
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
    {{0x44, 0x44, 0x44, 0x44}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x33, 0x33, 0x33, 0x33}, {256.0f, 0.0f, 464.0f}, -0x8000, 0, 0x4A, 3, 0, -0x8000},
};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01A4, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataRmHappyEnt = &dScData::l_scene.mData;
