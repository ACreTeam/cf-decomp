// The scene data of RM_OFFICE (dScene::dSceneData_c sScDataRmOffice; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dScene::dSceneBaseData_c l_base[] = {{fProf::SV_DEMO, 0}};
static u16 l_resources[] = {fProf::A_NPC_SP_PERIKO_05A, fProf::A_NPC_SP_SONCHO,
                            fProf::A_RMOBJ_ATM, fProf::A_RMOBJ_RECYCLEBOX};
static PlayerData_c l_player = {{256.0f, 0.0f, 464.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {fProf::A_NPC_SP_PERIKO_05A,
     0,
     {240.0f, 0.0f, 304.0f},
     {0, 0, 0},
     0,
     (void *)0x800A},
    {fProf::A_NPC_SP_SONCHO,
     0,
     {304.0f, 0.0f, 378.0f},
     {0, 0, 0},
     0,
     (void *)0x800D},
    {fProf::A_RMOBJ_ATM, 0, {144.0f, 0.0f, 432.0f}, {0, 0x4000, 0}, 0, 0},
    {fProf::A_RMOBJ_RECYCLEBOX,
     0,
     {368.0f, 0.0f, 416.0f},
     {0, -0x4000, 0},
     0,
     0},
};
static dScene::dSceneDataEntry_c l_entries[] = {
    {dScene::SCENE_DATA_PLAYER, 1, &l_player},
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
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x0186, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmOffice = &dScData::l_scene.mData;
