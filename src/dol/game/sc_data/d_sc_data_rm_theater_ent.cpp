// The scene data of RM_THEATER_ENT (dScene::dSceneData_c sScDataRmTheaterEnt; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static EntParam_c l_param0(0x35, 11, 10, 23, 0);
static EntParam_c l_param1(0x35, 11, 10, 23, 1);
static dScene::dSceneBaseData_c l_base[] = {{0x00A3, 0}};
static ColorByte_c l_palette0 = {{0x52, 0x52, 0x94, 0xFF}, 0xCE};
static u16 l_resources[] = {0x001E, 0x000E};
static PlayerData_c l_player = {{256.0f, 0.0f, 464.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {0x001E, 0, {256.0f, 0.0f, 272.0f}, {0, 0, 0}, 0, 0},
    {0x000E, 0, {200.0f, 0.0f, 296.0f}, {0, 0, 0}, 0, &l_param0},
    {0x000E, 0, {312.0f, 0.0f, 296.0f}, {0, 0, 0}, 0, &l_param1},
};
static u32 l_setting0[] = {0xF7F7CEFF, 0xE5560000, 0x4242C6FF, 0x01000000, 0x3D088889, 0x3D088889}; // scene setting kind 0xA
static Scene3Entry_c l_scene3[] = {{5, &l_palette0}, {0xA, l_setting0}};
static dScene::dSceneDataEntry_c l_entries[] = {
    {dScene::SCENE_DATA_PLAYER, 1, &l_player},
    {dScene::SCENE_DATA_3, ARRAY_SIZE(l_scene3), l_scene3},
    {dScene::SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {dScene::SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {dScene::SCENE_DATA_BASE, dScData::g_roomBaseNum, dScData::g_roomBase},
    {dScene::SCENE_DATA_RES, dScData::g_roomResNum, dScData::g_roomRes},
    {dScene::SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
    {dScene::SCENE_DATA_ACTOR, ARRAY_SIZE(l_actors), l_actors},
    {dScene::SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x44, 0x44, 0x44, 0x44}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x35, 0x35, 0x35, 0x35}, {208.0f, 0.0f, 464.0f}, -0x8000, 0, 0x4A, 3, 0, -0x8000},
    {{0x35, 0x35, 0x35, 0x35}, {304.0f, 0.0f, 464.0f}, -0x8000, 0, 0x4A, 3, 0, -0x8000},
};
static dScene::dSceneExitList_c l_exitList = {(dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01AC, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmTheaterEnt = &dScData::l_scene.mData;
