// The scene data of RM_SHOP2 (dSceneData_c sScDataRmShop2; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dSceneBaseData_c l_base[] = {{0x00A3, 0}};
static u16 l_resources[] = {0x005E, 0x0016};
static PlayerData_c l_player = {{256.0f, 0.0f, 464.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {0x005E, 0, {256.0f, 0.0f, 400.0f}, {0, 0, 0}, 0, (void *)0x8002},
    {0x0016, 0, {336.0f, 0.0f, 432.0f}, {0, -0x4000, 0}, 0, 0},
};
static dSceneDataEntry_c l_entries[] = {
    {SCENE_DATA_PLAYER, 1, &l_player},
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
};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x018E, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataRmShop2 = &dScData::l_scene.mData;
