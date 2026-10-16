// The scene data of RM_MM_PIC1 (dScene::dSceneData_c sScDataRmMmPic1; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dScene::dSceneBaseData_c l_base[] = {{fProf::SV_DEMO, 0}};
static u32 l_id0 = 1;
static u32 l_id1 = 4;
static u32 l_id2 = 6;
static u32 l_id3 = 8;
static u32 l_id4 = 0x16;
static ColorByte_c l_palette0 = {{0x52, 0x52, 0x94, 0xFF}, 0xA5};
static u32 l_setting0[] = {0xFFEFCEFF, 0xDAAB0800}; // scene setting kind 9
static u16 l_resources[] = {fProf::A_NAMEPLATE_MUSEUM, fProf::A_RMOBJ_PIC};
static PlayerData_c l_player = {{512.0f, 0.0f, 352.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {fProf::A_RMOBJ_PIC, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, 0},
};
static char l_name0[] = "panel_art_velazq";
static PanelData_c l_panel0 = {2, -1, &l_id0, 1, l_name0, 0};
static char l_name1[] = "panel_art_famous";
static PanelData_c l_panel1 = {2, -1, &l_id1, 1, l_name1, 0};
static char l_name2[] = "panel_art_rembra";
static PanelData_c l_panel2 = {2, -1, &l_id2, 1, l_name2, 0};
static char l_name3[] = "panel_art_goodlooking";
static PanelData_c l_panel3 = {2, -1, &l_id3, 1, l_name3, 0};
static char l_name4[] = "panel_art_admirable";
static PanelData_c l_panel4 = {2, -1, &l_id4, 1, l_name4, 0};
static ActorData_c l_actors1[] = {
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, -13.0f},
     {0, 0, 0},
     0,
     &l_panel0},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, 19.0f},
     {0, 0, 0},
     0,
     &l_panel1},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, -13.0f},
     {0, 0, 0},
     0,
     &l_panel2},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, -13.0f},
     {0, 0, 0},
     0,
     &l_panel3},
    {fProf::A_NAMEPLATE_MUSEUM,
     0,
     {0.0f, 0.0f, -13.0f},
     {0, 0, 0},
     0,
     &l_panel4},
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
    {{0x23, 0x23, 0x23, 0x23}, {768.0f, 0.0f, 144.0f}, 0, 0, 0x4A, 3, 0, 0},
};
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01CE, 0x01D0, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x02010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmMmPic1 = &dScData::l_scene.mData;
