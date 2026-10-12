// The scene data of DM_SAVE (dSceneData_c sScDataDmSave; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dSceneBaseData_c l_base[] = {{0x00A4, 0}};
static dSceneDataEntry_c l_entries[] = {
    {SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {SCENE_DATA_BASE, dScData::g_roomBaseNum, dScData::g_roomBase},
    {SCENE_DATA_RES, dScData::g_roomResNum, dScData::g_roomRes},
    {SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
};
static ExitData_c l_exits[] = {
    {{0x4B, 0x4B, 0x4B, 0x4B}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x4A, 0x4A, 0x4A, 0x4A}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, NULL};

} // namespace dScData

dSceneData_c *sScDataDmSave = &dScData::l_scene.mData;
