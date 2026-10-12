// The scene data of TOWN (dSceneData_c sScDataTown; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static u16 l_resources[] = {0x007B, 0x00A1, 0x0051, 0x011F};
static PlayerData_c l_player = {{768.0f, 0.0f, 640.0f}, {0, 0, 0}, 0, 8};
static dSceneBaseData_c l_base[] = {{0x00A3, 0}, {0x00AD, 0}};
static ActorData_c l_actors[] = {
    {0x007B, 0, {1084.8f, 0.0f, 385.6f}, {0, -0x2000, 0}, 0, (void *)0x804B},
    {0x00A1, 0, {656.0f, 0.0f, 512.0f}, {0, 0, 0}, 0, (void *)0x805F},
};
static dSceneDataEntry_c l_entries[] = {
    {SCENE_DATA_PLAYER, 1, &l_player},
    {SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {SCENE_DATA_BASE, dScData::g_townBaseNum, dScData::g_townBase},
    {SCENE_DATA_RES, dScData::g_townResNum, dScData::g_townRes},
    {SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
    {SCENE_DATA_ACTOR, ARRAY_SIZE(l_actors), l_actors},
    {SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x37, 0x37, 0x37, 0x37}, {144.0f, 0.0f, 416.0f}, 0x4000, 0, 0x4A, 3, 0, 0x4000},
    {{0x4B, 0x4B, 0x4B, 0x4B}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
};
static u16 l_exitInfoData[] = {0x01E0, 0x01D6, 0x01D8, 0x01DA, 0x01DC, 0x01DE, 0x0000};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x03020000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataTown = &dScData::l_scene.mData;
