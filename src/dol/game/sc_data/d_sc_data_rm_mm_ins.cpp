// The scene data of RM_MM_INS (dScene::dSceneData_c sScDataRmMmIns; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static u32 l_count0 = 6;
static u32 l_count1 = 8;
static u32 l_count2 = 7;
static u32 l_count3 = 8;
static u32 l_count4 = 5;
static u32 l_count5 = 7;
static u32 l_count6 = 8;
static u32 l_count7 = 7;
static u32 l_count8 = 8;
static u16 l_resources[] = {fProf::INSECT_MUSEUM, fProf::A_NAMEPLATE_MUSEUM};
static PlayerData_c l_player = {{352.0f, 0.0f, 912.0f}, {0, -0x8000, 0}, 0, 8};
static dScene::dSceneBaseData_c l_base[] = {{fProf::SV_DEMO, 0},
                                            {fProf::INSECT_MUSEUM, 0}};
static u32 l_ids0[] = {7, 0xB, 0x1B, 0x1C, 0x25, 0x29};
static u32 l_ids1[] = {0xD, 0xE, 0x16, 0x17, 0x18, 0x21, 0x28, 0x39};
static u32 l_ids2[] = {0, 1, 2, 3, 5, 6, 0x10};
static u32 l_ids3[] = {0x31, 0x34, 0x35, 0x38, 0x3B, 0x3C, 0x3E, 0x3F};
static u32 l_ids4[] = {8, 0xF, 0x1D, 0x37, 0x3A};
static u32 l_ids5[] = {0x13, 0x14, 0x1A, 0x1E, 0x1F, 0x32, 0x36};
static u32 l_ids6[] = {0xA, 0x11, 0x12, 0x20, 0x22, 0x26, 0x27, 0x3D};
static u32 l_ids7[] = {4, 0x15, 0x19, 0x2A, 0x2B, 0x2D, 0x33};
static u32 l_ids8[] = {9, 0xC, 0x23, 0x24, 0x2C, 0x2E, 0x2F, 0x30};
static char l_name0[] = "F1_panel00";
static PanelData_c l_panel0 = {0, -1, l_ids0, l_count0, l_name0, 0};
static char l_name1[] = "F1_panel01";
static PanelData_c l_panel1 = {0, -1, l_ids1, l_count1, l_name1, 0};
static char l_name2[] = "F1_panel02";
static PanelData_c l_panel2 = {0, -1, l_ids2, l_count2, l_name2, 0};
static char l_name3[] = "F2_panel00";
static PanelData_c l_panel3 = {0, -1, l_ids3, l_count3, l_name3, 0};
static char l_name4[] = "F2_panel01";
static PanelData_c l_panel4 = {0, -1, l_ids4, l_count4, l_name4, 0};
static char l_name5[] = "F2_panel02";
static PanelData_c l_panel5 = {0, -1, l_ids5, l_count5, l_name5, 0};
static char l_name6[] = "F2_panel03";
static PanelData_c l_panel6 = {0, -1, l_ids6, l_count6, l_name6, 0};
static char l_name7[] = "F3_panel00";
static PanelData_c l_panel7 = {0, -1, l_ids7, l_count7, l_name7, 0};
static char l_name8[] = "F3_panel01";
static PanelData_c l_panel8 = {0, -1, l_ids8, l_count8, l_name8, 0};
static ActorData_c l_actors[] = {
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, 0.0f},
     {0, -0x8000, 0},
     0,
     &l_panel0},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, 0.0f},
     {0, -0x4000, 0},
     0,
     &l_panel1},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel2},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel3},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel4},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel5},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, 0.0f},
     {0, 0x2000, 0},
     0,
     &l_panel6},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel7},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, 0.0f},
     {0, 0x4000, 0},
     0,
     &l_panel8},
};
static Scene3Entry_c l_scene3[] = {{4, (void *)0},
                                   {0x15, (void *)0},
                                   {0x16, (void *)0},
                                   {2, (void *)0},
                                   {0x13, (void *)0}};
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
    {{0x1E, 0x1E, 0x1E, 0x1E},
     {144.0f, 0.0f, 224.0f},
     0x4000,
     0,
     0x4A,
     3,
     0,
     0x4000},
};
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01C6, 0x01C4, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01020000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmMmIns = &dScData::l_scene.mData;
