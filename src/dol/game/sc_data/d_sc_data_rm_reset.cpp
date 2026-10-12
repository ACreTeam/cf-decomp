// The scene data of RM_RESET (dSceneData_c sScDataRmReset; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dSceneBaseData_c l_base[] = {{0x00A3, 0}};
static u16 l_resources[] = {0x0099, 0x009A, 0x0018};
static PlayerData_c l_player = {{224.0f, 0.0f, 384.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {0x0099, 0, {272.0f, 0.0f, 336.0f}, {0, 0, 0}, 0, (void *)0x8050},
    {0x009A, 0, {336.0f, 0.0f, 336.0f}, {0, 0, 0}, 0, (void *)0x8051},
};
static u32 l_setting0[] = {0x525294FF, 0x7B382030, 0xFF010000, 0x3DCCCCCD, 0x3D4CCCCD}; // scene setting kind 6
static u32 l_setting1[] = {0xF7F7CEFF, 0xDAAB0000, 0x403038FF, 0x01000000, 0x3DCCCCCD, 0x3D4CCCCD}; // scene setting kind 0xA
static u32 l_setting2[] = {0xFF3000FF, 0xD0000000, 0xC02000FF, 0x3D088889, 0x3D088889, 0x08000000}; // scene setting kind 0xF
static Scene3Entry_c l_scene3[] = {{6, l_setting0}, {0xA, l_setting1}, {0xF, l_setting2}};
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
    {{0x2D, 0x2D, 0x2D, 0x2D}, {1424.0f, 0.0f, 768.0f}, -0x4000, 0, 8, 4, 0, -0x4000},
};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x0194, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataRmReset = &dScData::l_scene.mData;
