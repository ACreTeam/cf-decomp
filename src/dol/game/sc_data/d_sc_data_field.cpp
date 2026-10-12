// The scene data of FIELD (dScene::dSceneData_c sScDataField; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static PlayerData_c l_player = {{544.0f, 0.0f, 544.0f}, {0, 0, 0}, 0, 8};
static dScene::dSceneBaseData_c l_base[] = {{0x00A3, 0}, {0x00AD, 0}};
static u16 l_resources[] = {0x004C, 0x005D, 0x0058, 0x007A, 0x0061, 0x006F, 0x0070, 0x006A, 0x006B, 0x006E, 0x0077, 0x0078, 0x0079, 0x0095, 0x0096, 0x009B, 0x0097, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x007C, 0x007D, 0x007E, 0x007F, 0x0094, 0x009D, 0x009C, 0x009E, 0x009F};
static dScene::dSceneDataEntry_c l_entries[] = {
    {dScene::SCENE_DATA_PLAYER, 1, &l_player},
    {dScene::SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {dScene::SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {dScene::SCENE_DATA_BASE, dScData::g_fieldBaseNum, dScData::g_fieldBase},
    {dScene::SCENE_DATA_RES, dScData::g_fieldResNum, dScData::g_fieldRes},
    {dScene::SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
    {dScene::SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x4B, 0x4B, 0x4B, 0x4B}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
};
static dScene::dSceneExitList_c l_exitList = {(dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, NULL};

} // namespace dScData

dScene::dSceneData_c *sScDataField = &dScData::l_scene.mData;
