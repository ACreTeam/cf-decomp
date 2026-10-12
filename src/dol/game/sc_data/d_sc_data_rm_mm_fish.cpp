// The scene data of RM_MM_FISH (dSceneData_c sScDataRmMmFish; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static ColorByte_c l_palette0 = {{0x31, 0x63, 0x84, 0xFF}, 0x7B};
static u32 l_setting0[] = {0x8CCEEFFF, 0xE5560000}; // scene setting kind 9
static u16 l_resources[] = {0x00B0, 0x00F5};
static PlayerData_c l_player = {{256.0f, 0.0f, 464.0f}, {0, -0x8000, 0}, 0, 8};
static dSceneBaseData_c l_base[] = {{0x00A3, 0}, {0x00B0, 0}};
static u32 l_ids3[] = {0, 1, 2, 3, 4, 5, 6, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x17, 0x18, 0x19, 0x1A, 0x1B};
static u32 l_ids4[] = {0xA, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x1C, 0x20, 0x21, 0x22, 0x23, 0x24};
static u32 l_ids0[] = {0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
static u32 l_ids1[] = {7, 8, 9, 0x1D, 0x1E, 0x1F};
static u32 l_ids2[] = {0x25, 0x26, 0x27, 0x28, 0x29};
static char l_name0[] = "left_sea_panel";
static PanelData_c l_panel0 = {1, -1, l_ids0, 0x16, l_name0, 0};
static char l_name1[] = "right_sea_panel";
static PanelData_c l_panel1 = {1, -1, l_ids0, 0x16, l_name1, 0};
static char l_name2[] = "left_cylinder_suisou_panel";
static PanelData_c l_panel2 = {1, -1, l_ids1, 6, l_name2, 0};
static char l_name3[] = "right_cylinder_suisou_panel";
static PanelData_c l_panel3 = {1, -1, l_ids2, 5, l_name3, 0};
static char l_name4[] = "left_suisou_panel";
static PanelData_c l_panel4 = {1, -1, l_ids3, 0x12, l_name4, 0};
static char l_name5[] = "right_suisou_panel";
static PanelData_c l_panel5 = {1, -1, l_ids4, 0xD, l_name5, 0};
static ActorData_c l_actors[] = {
    {0x00F5, 0, {0.0f, 0.0f, -12.0f}, {0, 0, 0}, 0, &l_panel0},
    {0x00F5, 0, {0.0f, 0.0f, -12.0f}, {0, 0, 0}, 0, &l_panel1},
    {0x00F5, 0, {-12.0f, 0.0f, 0.0f}, {0, 0x4000, 0}, 0, &l_panel2},
    {0x00F5, 0, {12.0f, 0.0f, 0.0f}, {0, -0x4000, 0}, 0, &l_panel3},
    {0x00F5, 0, {0.0f, 0.0f, -12.0f}, {0, 0, 0}, 0, &l_panel4},
    {0x00F5, 0, {0.0f, 0.0f, -12.0f}, {0, 0, 0}, 0, &l_panel5},
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
    {SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x1E, 0x1E, 0x1E, 0x1E}, {320.0f, 0.0f, 112.0f}, 0, 0, 0x4A, 3, 0, 0},
};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01C0, 0x01C2, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x02010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataRmMmFish = &dScData::l_scene.mData;
