// The scene data of RM_HS0_F1 (dSceneData_c sScDataRmHs0F1; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static u16 l_resources[] = {0x004F, 0x0050, 0x001F, 0x0020};
static PlayerData_c l_player = {{256.0f, 0.0f, 464.0f}, {0, -0x8000, 0}, 0, 8};
static dSceneBaseData_c l_base[] = {{0x00A3, 0}, {0x00AD, 0}};
static dSceneDataEntry_c l_entries[] = {
    {SCENE_DATA_PLAYER, 1, &l_player},
    {SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {SCENE_DATA_BASE, dScData::g_roomBaseNum, dScData::g_roomBase},
    {SCENE_DATA_RES, dScData::g_roomResNum, dScData::g_roomRes},
    {SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
    {SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x46, 0x47, 0x48, 0x49}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x04, 0x08, 0x0C, 0x10}, {240.0f, 0.0f, 464.0f}, 0x4000, 0, 0x81, 1, 0, -0x4000},
    {{0x02, 0x06, 0x0A, 0x0E}, {240.0f, 0.0f, 464.0f}, 0x4000, 0, 0x81, 1, 0, -0x4000},
    {{0x03, 0x07, 0x0B, 0x0F}, {272.0f, 0.0f, 464.0f}, -0x4000, 0, 0x83, 2, 0, 0x4000},
    {{0x4B, 0x4B, 0x4B, 0x4B}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x0170, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataRmHs0F1 = &dScData::l_scene.mData;
