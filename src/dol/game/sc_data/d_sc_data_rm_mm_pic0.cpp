// The scene data of RM_MM_PIC0 (dScene::dSceneData_c sScDataRmMmPic0; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static u32 l_id15;
static dScene::dSceneBaseData_c l_base[] = {{0x00A3, 0}};
static u32 l_id0 = 0x11;
static u32 l_id1 = 2;
static u32 l_id2 = 5;
static u32 l_id3 = 0xB;
static u32 l_id4 = 9;
static u32 l_id5 = 7;
static u32 l_id6 = 3;
static u32 l_id7 = 0xE;
static u32 l_id8 = 0x10;
static u32 l_id9 = 0x12;
static u32 l_id10 = 0xD;
static u32 l_id11 = 0xA;
static u32 l_id12 = 0x14;
static u32 l_id13 = 0xF;
static u32 l_id14 = 0x13;
static u32 l_id16 = 0x15;
static u32 l_id17 = 0xC;
static ColorByte_c l_palette0 = {{0x63, 0x63, 0xA5, 0xFF}, 0x7B};
static u32 l_setting0[] = {0xFFEFCEFF, 0xDAAB0800}; // scene setting kind 9
static u16 l_resources[] = {0x00F5, 0x0015};
static PlayerData_c l_player = {{256.0f, 0.0f, 256.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {0x0015, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, 0},
};
static char l_name0[] = "panel_art_valuable";
static PanelData_c l_panel0 = {2, -1, &l_id0, 1, l_name0, 0};
static char l_name1[] = "panel_art_becomecalm";
static PanelData_c l_panel1 = {2, -1, &l_id1, 1, l_name1, 0};
static char l_name2[] = "panel_art_fantastic";
static PanelData_c l_panel2 = {2, -1, &l_id2, 1, l_name2, 0};
static char l_name3[] = "panel_art_goya";
static PanelData_c l_panel3 = {2, -1, &l_id3, 1, l_name3, 0};
static char l_name4[] = "panel_art_intimacy";
static PanelData_c l_panel4 = {2, -1, &l_id4, 1, l_name4, 0};
static char l_name5[] = "panel_art_manet";
static PanelData_c l_panel5 = {2, -1, &l_id5, 1, l_name5, 0};
static char l_name6[] = "panel_art_bravely";
static PanelData_c l_panel6 = {2, -1, &l_id6, 1, l_name6, 0};
static char l_name7[] = "panel_art_stylish";
static PanelData_c l_panel7 = {2, -1, &l_id7, 1, l_name7, 0};
static char l_name8[] = "panel_art_unspeakable";
static PanelData_c l_panel8 = {2, -1, &l_id8, 1, l_name8, 0};
static char l_name9[] = "panel_art_wonderful";
static PanelData_c l_panel9 = {2, -1, &l_id9, 1, l_name9, 0};
static char l_name10[] = "panel_art_splendid";
static PanelData_c l_panel10 = {2, -1, &l_id10, 1, l_name10, 0};
static char l_name11[] = "panel_art_matchless";
static PanelData_c l_panel11 = {2, -1, &l_id11, 1, l_name11, 0};
static char l_name12[] = "panel_art_fine";
static PanelData_c l_panel12 = {2, -1, &l_id12, 1, l_name12, 0};
static char l_name13[] = "panel_art_lively";
static PanelData_c l_panel13 = {2, -1, &l_id13, 1, l_name13, 0};
static char l_name14[] = "panel_art_powerful";
static PanelData_c l_panel14 = {2, -1, &l_id14, 1, l_name14, 0};
static char l_name15[] = "panel_art_iki";
static PanelData_c l_panel15 = {2, -1, &l_id15, 1, l_name15, 0};
static char l_name16[] = "panel_art_feminin";
static PanelData_c l_panel16 = {2, -1, &l_id16, 1, l_name16, 0};
static char l_name17[] = "panel_art_interesting";
static PanelData_c l_panel17 = {2, -1, &l_id17, 1, l_name17, 0};
static ActorData_c l_actors1[] = {
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel0},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel1},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel2},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel3},
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
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel14},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel15},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel16},
    {0x00F5, 0, {0.0f, 0.0f, -13.0f}, {0, 0, 0}, 0, &l_panel17},
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
    {{0x1E, 0x1E, 0x1E, 0x1E}, {368.0f, 0.0f, 352.0f}, -0x4000, 0, 0x4A, 3, 0, -0x4000},
    {{0x24, 0x24, 0x24, 0x24}, {512.0f, 0.0f, 464.0f}, -0x8000, 0, 0x4A, 3, 0, -0x8000},
};
static dScene::dSceneExitList_c l_exitList = {(dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01C8, 0x01CA, 0x01CC, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x03010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmMmPic0 = &dScData::l_scene.mData;
