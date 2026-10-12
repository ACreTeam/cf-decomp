// The scene data of RM_FORTUNE (dScene::dSceneData_c sScDataRmFortune; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dScene::dSceneBaseData_c l_base[] = {{0x00A3, 0}};
static ColorByte_c l_palette0 = {{0x10, 0x39, 0x31, 0xFF}, 0x7B};
static u32 l_setting0[] = {0x084210FF, 0xEFF70000}; // scene setting kind 9
static u16 l_resources[] = {0x0071};
static PlayerData_c l_player = {{256.0f, 0.0f, 464.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {0x0071, 0, {256.0f, 0.0f, 336.0f}, {0, 0, 0}, 0, (void *)0x8022},
};
static u32 l_setting1[] ALIGN(8) = {0xFF6321FF, 0x43800000, 0x42800000, 0x43C00000, 0xE0000000, 0x3F000000, 0x3BCCCCCD, 0x389D4952, 0x42B40000}; // scene setting kind 0xC
static Scene3Entry_c l_scene3[] = {{5, &l_palette0}, {9, l_setting0}, {0xC, l_setting1}};
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
};
static dScene::dSceneExitList_c l_exitList = {(dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01A0, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmFortune = &dScData::l_scene.mData;
