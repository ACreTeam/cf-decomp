// The scene data of DM_SAVE (dScene::dSceneData_c sScDataDmSave; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dScene::dSceneBaseData_c l_base[] = {{fProf::SV_MGR, 0}};
static dScene::dSceneDataEntry_c l_entries[] = {
    {dScene::SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {dScene::SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {dScene::SCENE_DATA_BASE, dScData::g_roomBaseNum, dScData::g_roomBase},
    {dScene::SCENE_DATA_RES, dScData::g_roomResNum, dScData::g_roomRes},
    {dScene::SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
};
static ExitData_c l_exits[] = {
    {{0x4B, 0x4B, 0x4B, 0x4B}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x4A, 0x4A, 0x4A, 0x4A}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
};
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              NULL};

} // namespace dScData

dScene::dSceneData_c *sScDataDmSave = &dScData::l_scene.mData;
