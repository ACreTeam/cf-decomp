// The scene data of RM_MM_FOSSIL1 (dSceneData_c sScDataRmMmFossil1; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static u32 l_id3;
static u32 l_ids1[] = {0x10, 0x11};
static u32 l_ids2[] = {0x1E, 0x1F};
static u32 l_id0 = 6;
static u32 l_id1 = 7;
static u32 l_id2 = 4;
static u32 l_id4 = 3;
static u32 l_id5 = 1;
static u32 l_id6 = 9;
static u32 l_id7 = 8;
static u32 l_id8 = 5;
static u32 l_id9 = 2;
static ColorByte_c l_palette0 = {{0x63, 0x63, 0xA5, 0xFF}, 0x7B};
static u32 l_setting0[] = {0xE7D6B5FF, 0xDAAB0000}; // scene setting kind 9
static u16 l_resources[] = {0x0012, 0x00F5};
static PlayerData_c l_player = {{256.0f, 0.0f, 256.0f}, {0, -0x8000, 0}, 0, 8};
static dSceneBaseData_c l_base[] = {{0x00A3, 0}, {0x00B0, 0}};
static ActorData_c l_actors[] = {
    {0x0012, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, 0},
};
static u32 l_ids0[] = {0x26, 0x27, 0x28, 0x29};
static u32 l_ids3[] = {0x30, 0x31, 0x32};
static char l_name0[] = "panel_B_00";
static PanelData_c l_panel0 = {3, 0x1D, l_ids0, 4, l_name0, 0};
static char l_name1[] = "panel_B_01";
static PanelData_c l_panel1 = {3, 0x13, l_ids1, 2, l_name1, 0};
static char l_name2[] = "panel_B_02";
static PanelData_c l_panel2 = {3, 0x1B, l_ids2, 2, l_name2, 0};
static char l_name3[] = "panel_B_03";
static PanelData_c l_panel3 = {3, 0x19, l_ids3, 3, l_name3, 0x1000000};
static char l_name4[] = "panel_B_04";
static PanelData_c l_panel4 = {3, -1, &l_id0, 1, l_name4, 0};
static char l_name5[] = "panel_B_05";
static PanelData_c l_panel5 = {3, -1, &l_id1, 1, l_name5, 0};
static char l_name6[] = "panel_B_06";
static PanelData_c l_panel6 = {3, -1, &l_id2, 1, l_name6, 0};
static char l_name7[] = "panel_B_07";
static PanelData_c l_panel7 = {3, -1, &l_id3, 1, l_name7, 0};
static char l_name8[] = "panel_B_08";
static PanelData_c l_panel8 = {3, -1, &l_id4, 1, l_name8, 0};
static char l_name9[] = "panel_B_09";
static PanelData_c l_panel9 = {3, -1, &l_id5, 1, l_name9, 0};
static char l_name10[] = "panel_B_010";
static PanelData_c l_panel10 = {3, -1, &l_id6, 1, l_name10, 0};
static char l_name11[] = "panel_B_011";
static PanelData_c l_panel11 = {3, -1, &l_id7, 1, l_name11, 0};
static char l_name12[] = "panel_B_012";
static PanelData_c l_panel12 = {3, -1, &l_id8, 1, l_name12, 0};
static char l_name13[] = "panel_B_013";
static PanelData_c l_panel13 = {3, -1, &l_id9, 1, l_name13, 0};
static ActorData_c l_actors1[] = {
    {0x00F5, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel0},
    {0x00F5, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel1},
    {0x00F5, 0, {0.0f, 0.0f, 0.0f}, {0, -0x4000, 0}, 0, &l_panel2},
    {0x00F5, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, &l_panel3},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel4},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel5},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel6},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel7},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel8},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel9},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel10},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel11},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel12},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel13},
};
static Scene3Entry_c l_scene3[] = {{5, &l_palette0}, {9, l_setting0}};
static dSceneDataEntry_c l_entries[] = {
    {SCENE_DATA_PLAYER, 1, &l_player},
    {SCENE_DATA_3, ARRAY_SIZE(l_scene3), l_scene3},
    {SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {SCENE_DATA_BASE, dScData::g_roomBaseNum, dScData::g_roomBase},
    {SCENE_DATA_RES, dScData::g_roomResNum, dScData::g_roomRes},
    {SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
    {SCENE_DATA_ACTOR, ARRAY_SIZE(l_actors), l_actors},
    {SCENE_DATA_ACTOR, ARRAY_SIZE(l_actors1), l_actors1},
    {SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x21, 0x21, 0x21, 0x21}, {480.0f, 0.0f, 176.0f}, 0, 0, 0x4A, 3, 0, 0},
    {{0x21, 0x21, 0x21, 0x21}, {288.0f, 0.0f, 176.0f}, 0, 0, 0x4A, 3, 0, 0},
};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01BC, 0x01BE, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x02010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataRmMmFossil1 = &dScData::l_scene.mData;
