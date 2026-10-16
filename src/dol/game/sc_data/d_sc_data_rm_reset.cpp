// The scene data of RM_RESET (dScene::dSceneData_c sScDataRmReset; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dScene::dSceneBaseData_c l_base[] = {{fProf::SV_DEMO, 0}};
static u16 l_resources[] = {fProf::A_NPC_SP_DEMO_RESETSAN_099,
                            fProf::A_NPC_SP_DEMO_RESETSAN_09A,
                            fProf::A_RMOBJ_RESET};
static PlayerData_c l_player = {{224.0f, 0.0f, 384.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {fProf::A_NPC_SP_DEMO_RESETSAN_099,
     0,
     {272.0f, 0.0f, 336.0f},
     {0, 0, 0},
     0,
     (void *)0x8050},
    {fProf::A_NPC_SP_DEMO_RESETSAN_09A,
     0,
     {336.0f, 0.0f, 336.0f},
     {0, 0, 0},
     0,
     (void *)0x8051},
};
static u32 l_setting0[] = {0x525294FF, 0x7B382030, 0xFF010000, 0x3DCCCCCD,
                           0x3D4CCCCD}; // scene setting kind 6
static u32 l_setting1[] = {0xF7F7CEFF, 0xDAAB0000, 0x403038FF, 0x01000000,
                           0x3DCCCCCD, 0x3D4CCCCD}; // scene setting kind 0xA
static u32 l_setting2[] = {0xFF3000FF, 0xD0000000, 0xC02000FF, 0x3D088889,
                           0x3D088889, 0x08000000}; // scene setting kind 0xF
static Scene3Entry_c l_scene3[] = {
    {6, l_setting0}, {0xA, l_setting1}, {0xF, l_setting2}};
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
    {{0x2D, 0x2D, 0x2D, 0x2D},
     {1424.0f, 0.0f, 768.0f},
     -0x4000,
     0,
     8,
     4,
     0,
     -0x4000},
};
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x0194, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmReset = &dScData::l_scene.mData;
