// The scene data of RM_MM_CAFE (dScene::dSceneData_c sScDataRmMmCafe; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static ColorByte_c l_palette0 = {{0x63, 0x63, 0xA5, 0xFF}, 0x7B};
static PlayerData_c l_player = {{384.0f, 0.0f, 416.0f}, {0, -0x4000, 0}, 0, 8};
static dScene::dSceneBaseData_c l_base[] = {
    {fProf::SV_DEMO, 0}, {fProf::OBJ_STAFFROLL, 0}, {fProf::NPC_MNG, 0}};
static ActorData_c l_actors[] = {
    {fProf::A_RMOBJ_CAFE, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, 0},
    {fProf::A_NPC_SP_TOTAKEKE,
     0,
     {240.0f, 0.0f, 272.0f},
     {0, 0, 0},
     0,
     (void *)0x8007},
    {fProf::A_NPC_SP_CAFE,
     0,
     {408.0f, 0.0f, 304.0f},
     {0, -0x4000, 0},
     0,
     (void *)0x8017},
    {fProf::A_NPC_SP_CAFE_GUEST,
     0,
     {336.0f, 0.0f, 272.0f},
     {0, 0x4000, 0},
     0,
     (void *)0x802A},
};
static u32 l_setting0[] = {0xEF9C7BFF, 0xE5560800, 0x52297BFF, 0x01000000,
                           0x3C360B61, 0x3C360B61}; // scene setting kind 0xA
static u32 l_setting1[] = {0xEFFF20FF, 0x43800000, 0x42BC0000, 0x43AC0000,
                           0xDC720000, 0x3F400000, 0x00000000, 0x389D4952,
                           0x41F00000, 0x00000002, 0x00000000, 0x3C360B61,
                           0x3C360B61}; // scene setting kind 0xD
static Scene3Entry_c l_scene3[] = {
    {5, &l_palette0}, {0xA, l_setting0}, {0xD, l_setting1}};
static u16 l_resources[] = {fProf::A_RMOBJ_CAFE,  fProf::A_RMOBJ_MIRRORBALL,
                            fProf::A_NPC_CAFE,    fProf::A_NPC_SP_TOTAKEKE,
                            fProf::A_NPC_SP_CAFE, fProf::A_NPC_SP_CAFE_GUEST};
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
     {144.0f, 0.0f, 352.0f},
     0x4000,
     0,
     0x81,
     3,
     0,
     0x4000},
};
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static u16 l_exitInfoData[] = {0x01D4, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              &l_exitInfo};

} // namespace dScData

dScene::dSceneData_c *sScDataRmMmCafe = &dScData::l_scene.mData;
