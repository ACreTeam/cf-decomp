// The scene data of RM_HS0_LOFT (dScene::dSceneData_c sScDataRmHs0Loft; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dScene::dSceneBaseData_c l_base[] = {{fProf::SV_DEMO, 1}};
static ColorByte_c l_palette0 = {{0x52, 0x52, 0x94, 0xFF}, 0x6B};
static u32 l_setting0[] = {0x84846BFF, 0xE5560000}; // scene setting kind 9
static u16 l_resources[] = {fProf::A_RMOBJ_TEL, fProf::A_NPC_SP_YUUTARO_LOFT};
static PlayerData_c l_player = {{256.0f, 0.0f, 464.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {fProf::A_RMOBJ_TEL, 0, {304.0f, 56.0f, 368.0f}, {0, 0, 0}, 0, 0},
    {fProf::A_NPC_SP_YUUTARO_LOFT,
     0,
     {256.0f, 56.0f, 368.0f},
     {0, 0, 0},
     0,
     (void *)0x805E},
};
static Scene3Entry_c l_scene3[] = {
    {5, &l_palette0}, {9, l_setting0}, {2, (void *)0}};
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
    {{0x01, 0x05, 0x09, 0x0D},
     {240.0f, 0.0f, 464.0f},
     0x4000,
     0,
     0x83,
     2,
     0,
     -0x4000},
    {{0x02, 0x06, 0x0A, 0x0E},
     {272.0f, 0.0f, 464.0f},
     -0x4000,
     0,
     0x83,
     2,
     0,
     0x4000},
};
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x017E, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmHs0Loft = &dScData::l_scene.mData;
