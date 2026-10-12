// The scene data of DM_PL_SEL (dSceneData_c sScDataDmPlSel; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static ColorByte_c l_palette0 = {{0x31, 0x42, 0x63, 0xFF}, 0x00};
static u16 l_resources[] = {0x0057};
static u16 l_exitInfoData[] = {0x0184, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static ActorData_c l_actors[] = {
    {0x0057, 0, {256.0f, 0.0f, 432.0f}, {0, 0, 0}, 0, (void *)0x8029},
};
static u32 l_setting0[] ALIGN(8) = {0xFFEF63FF, 0x43800000, 0x42A00000, 0x43F50000, 0xE0000000, 0x3FA00000, 0x3AA3D70A, 0x3903126F, 0x41F00000, 0x00000002}; // scene setting kind 0xC
static Scene3Entry_c l_scene3[] = {{5, &l_palette0}, {0xC, l_setting0}};
static dSceneDataEntry_c l_entries[] = {
    {SCENE_DATA_3, ARRAY_SIZE(l_scene3), l_scene3},
    {SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {SCENE_DATA_BASE, dScData::g_roomBaseNum, dScData::g_roomBase},
    {SCENE_DATA_RES, dScData::g_roomResNum, dScData::g_roomRes},
    {SCENE_DATA_ACTOR, ARRAY_SIZE(l_actors), l_actors},
    {SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x46, 0x46, 0x46, 0x46}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x47, 0x47, 0x47, 0x47}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x48, 0x48, 0x48, 0x48}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x49, 0x49, 0x49, 0x49}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
    {{0x4A, 0x4A, 0x4A, 0x4A}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
};
static dSceneExitList_c l_exitList = {(dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataDmPlSel = &dScData::l_scene.mData;
