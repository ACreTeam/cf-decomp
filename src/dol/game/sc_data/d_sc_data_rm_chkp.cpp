// The scene data of RM_CHKP (dScene::dSceneData_c sScDataRmChkp; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static u32 l_setting0[] = {0x42426BFF, 0x6B000000}; // scene setting kind 7
static u32 l_setting1[] = {0x9CBDCEFF, 0xDAAB0000}; // scene setting kind 0x10
static PlayerData_c l_player = {{256.0f, 0.0f, 464.0f}, {0, -0x8000, 0}, 0, 8};
static dScene::dSceneBaseData_c l_base[] = {{fProf::SV_DEMO, 0},
                                            {fProf::SKY, 0}};
static ActorData_c l_actors[] = {
    {fProf::A_NPC_SP_GATEKEEPER_A,
     0,
     {320.0f, 0.0f, 304.0f},
     {0, 0, 0},
     0,
     (void *)0x8026},
    {fProf::A_NPC_SP_GATEKEEPER_B,
     0,
     {192.0f, 0.0f, 304.0f},
     {0, 0, 0},
     0,
     (void *)0x8027},
    {fProf::A_NPC_SP_MISSING_CHILD,
     0,
     {240.0f, 0.0f, 496.0f},
     {0, -0x8000, 0},
     0,
     (void *)0x8019},
    {fProf::A_NPC_SP_MISSING_MOTHER,
     0,
     {240.0f, 0.0f, 416.0f},
     {0, -0x8000, 0},
     0,
     (void *)0x801A},
    {fProf::A_RMOBJ_CHECKP, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, 0},
    {fProf::A_SAMPLE2, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, 0},
    {fProf::A_SAMPLE3, 0, {256.0f, 0.0f, 384.0f}, {0, 0, 0}, 0, 0},
};
static u32 l_setting2[] ALIGN(8) = {
    0x9CBDCEFF, 0x43800000, 0x42700000, 0x438C0000, 0xE0000000,
    0x3F800000, 0x00000000, 0x38D1B717, 0x42B40000}; // scene setting kind 0xC
static Scene3Entry_c l_scene3[] = {
    {7, l_setting0}, {0x10, l_setting1}, {0xC, l_setting2}};
static u16 l_resources[] = {fProf::SKY,
                            fProf::A_NPC_SP_GATEKEEPER_A,
                            fProf::A_NPC_SP_GATEKEEPER_B,
                            fProf::A_NPC_SP_MISSING_CHILD,
                            fProf::A_NPC_SP_MISSING_MOTHER,
                            fProf::A_RMOBJ_CHECKP,
                            fProf::A_SAMPLE2,
                            fProf::A_SAMPLE3};
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
    {{0x44, 0x44, 0x44, 0x44}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x4B, 0x4B, 0x4B, 0x4B}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
};
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x0188, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmChkp = &dScData::l_scene.mData;
