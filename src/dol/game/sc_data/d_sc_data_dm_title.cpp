// The scene data of DM_TITLE (dScene::dSceneData_c sScDataDmTitle; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static u16 l_resources[] = {fProf::TITLE_DEMO, fProf::A_NPC_OUT};
static dScene::dSceneBaseData_c l_base[] = {{fProf::TITLE_DEMO, 0},
                                            {fProf::NPC_MNG, 0}};
static dScene::dSceneDataEntry_c l_entries[] = {
    {dScene::SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {dScene::SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {dScene::SCENE_DATA_BASE, dScData::g_fieldBaseNum, dScData::g_fieldBase},
    {dScene::SCENE_DATA_RES, dScData::g_fieldResNum, dScData::g_fieldRes},
    {dScene::SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
    {dScene::SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, NULL}, NULL};

} // namespace dScData

dScene::dSceneData_c *sScDataDmTitle = &dScData::l_scene.mData;
