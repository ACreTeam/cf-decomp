// The scene data of RM_MM_FOSSIL0 (dScene::dSceneData_c sScDataRmMmFossil0; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static u32 l_ids5[] = {0x33, 0x34};
static u32 l_ids6[] = {0x35, 0x36};
static ColorByte_c l_palette0 = {{0x63, 0x63, 0xA5, 0xFF}, 0x7B};
static u32 l_setting0[] = {0xE7D6B5FF, 0xDAAB0000}; // scene setting kind 9
static u16 l_resources[] = {fProf::A_RMOBJ_FOSSIL, fProf::A_NAMEPLATE_MUSEUM};
static PlayerData_c l_player = {{256.0f, 0.0f, 256.0f}, {0, -0x8000, 0}, 0, 8};
static dScene::dSceneBaseData_c l_base[] = {{fProf::SV_DEMO, 0},
                                            {fProf::FISH_MUSEUM, 0}};
static ActorData_c l_actors[] = {
    {fProf::A_RMOBJ_FOSSIL, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, 0},
};
static u32 l_ids0[] = {0x15, 0x16, 0x17};
static u32 l_ids1[] = {0x2D, 0x2E, 0x2F};
static u32 l_ids2[] = {0x23, 0x24, 0x25};
static u32 l_ids3[] = {0x1B, 0x1C, 0x1D};
static u32 l_ids4[] = {0x37, 0x38, 0x39};
static u32 l_ids7[] = {0x2A, 0x2B, 0x2C};
static u32 l_ids8[] = {0xD, 0xE, 0xF};
static u32 l_ids9[] = {0x20, 0x21, 0x22};
static u32 l_ids10[] = {0x12, 0x13, 0x14};
static u32 l_ids11[] = {0xA, 0xB, 0xC};
static u32 l_ids12[] = {0x18, 0x19, 0x1A};
static char l_name0[] = "panel_A_00";
static PanelData_c l_panel0 = {3, 0xD, l_ids0, 3, l_name0, 0};
static char l_name1[] = "panel_A_01";
static PanelData_c l_panel1 = {3, 0x1F, l_ids1, 3, l_name1, 0};
static char l_name2[] = "panel_A_02";
static PanelData_c l_panel2 = {3, 0x17, l_ids2, 3, l_name2, 0};
static char l_name3[] = "panel_A_03";
static PanelData_c l_panel3 = {3, 0x11, l_ids3, 3, l_name3, 0};
static char l_name4[] = "panel_A_04";
static PanelData_c l_panel4 = {3, 0x27, l_ids4, 3, l_name4, 0};
static char l_name5[] = "panel_A_05";
static PanelData_c l_panel5 = {3, 0x2B, l_ids5, 2, l_name5, 0x1000000};
static char l_name6[] = "panel_A_06";
static PanelData_c l_panel6 = {3, 0x29, l_ids6, 2, l_name6, 0};
static char l_name7[] = "panel_A_07";
static PanelData_c l_panel7 = {3, 0x21, l_ids7, 3, l_name7, 0x1000000};
static char l_name8[] = "panel_A_08";
static PanelData_c l_panel8 = {3, 0x25, l_ids8, 3, l_name8, 0};
static char l_name9[] = "panel_A_09";
static PanelData_c l_panel9 = {3, 0x15, l_ids9, 3, l_name9, 0};
static char l_name10[] = "panel_A_10";
static PanelData_c l_panel10 = {3, 0xB, l_ids10, 3, l_name10, 0};
static char l_name11[] = "panel_A_11";
static PanelData_c l_panel11 = {3, 0x23, l_ids11, 3, l_name11, 0};
static char l_name12[] = "panel_A_12";
static PanelData_c l_panel12 = {3, 0xF, l_ids12, 3, l_name12, 0};
static ActorData_c l_actors1[] = {
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel0},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel1},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel2},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel3},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel4},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel5},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, 0.0f},
     {0, -0x4000, 0},
     0,
     &l_panel6},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel7},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel8},
    {fProf::A_NAMEPLATE_MUSEUM, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel9},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, 0.0f},
     {0, 0, 0},
     0,
     &l_panel10},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, 0.0f},
     {0, 0, 0},
     0,
     &l_panel11},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, 0.0f},
     {0, 0, 0},
     0,
     &l_panel12},
};
static Scene3Entry_c l_scene3[] = {{5, &l_palette0}, {9, l_setting0}};
static dScene::dSceneDataEntry_c l_entries[] = {
    {dScene::SCENE_DATA_PLAYER, 1, &l_player},
    {dScene::SCENE_DATA_3, ARRAY_SIZE(l_scene3), l_scene3},
    {dScene::SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {dScene::SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {dScene::SCENE_DATA_BASE, dScData::g_roomBaseNum, dScData::g_roomBase},
    {dScene::SCENE_DATA_RES, dScData::g_roomResNum, dScData::g_roomRes},
    {dScene::SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
    {dScene::SCENE_DATA_ACTOR, ARRAY_SIZE(l_actors), l_actors},
    {dScene::SCENE_DATA_ACTOR, ARRAY_SIZE(l_actors1), l_actors1},
    {dScene::SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x1E, 0x1E, 0x1E, 0x1E},
     {368.0f, 0.0f, 224.0f},
     -0x4000,
     0,
     0x4A,
     3,
     0,
     -0x4000},
    {{0x22, 0x22, 0x22, 0x22},
     {480.0f, 0.0f, 464.0f},
     -0x8000,
     0,
     0x4A,
     3,
     0,
     -0x8000},
    {{0x22, 0x22, 0x22, 0x22},
     {288.0f, 0.0f, 464.0f},
     -0x8000,
     0,
     0x4A,
     3,
     0,
     -0x8000},
};
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01B4, 0x01B8, 0x01B6, 0x01BA};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x02020000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmMmFossil0 = &dScData::l_scene.mData;
