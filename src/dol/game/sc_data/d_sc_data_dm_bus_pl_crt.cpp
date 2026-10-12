// The scene data of DM_BUS_PL_CRT (dSceneData_c sScDataDmBusPlCrt; see include/game/game/d_sc_data.hpp).
#include <game/game/d_sc_data.hpp>

namespace dScData {

static dSceneBaseData_c l_base[] = {{0x00AA, 0}};
static u32 l_setting0[] = {0x404850FF, 0x6B000000}; // scene setting kind 8
static u32 l_setting1[] = {0x906040FF, 0xF71D2000}; // scene setting kind 0x11
static u16 l_resources[] = {0x00AA, 0x0056, 0x006C, 0x000F};
static u16 l_exitInfoData[] = {0x019A, 0x0000};
static ExitInfo_c l_exitInfo = {l_exitInfoData, 0x01010000};
static PlayerData_c l_player = {{288.0f, 0.0f, 544.0f}, {0, -0x8000, 0}, 0, 8};
static ActorData_c l_actors[] = {
    {0x0056, 0, {235.0f, 0.0f, 426.0f}, {0, -0x8000, 0}, 0, (void *)0x800C},
    {0x006C, 0, {220.0f, 0.0f, 309.0f}, {0, -0x8000, 0}, 0, (void *)0x8014},
    {0x000F, 0, {0.0f, 0.0f, 0.0f}, {0, 0, 0}, 0, 0},
};
static u32 l_setting2[] = {0xFFEFBDFF, 0x43880000, 0x42400000, 0x43A80000, 0xC0000000, 0x3F800000, 0x3A83126F, 0x38D1B717, 0x42B40000, 0x00000000, 0x00000000, 0x3DCCCCCD, 0x3DCCCCCD}; // scene setting kind 0xD
static u32 l_setting3[] = {0x906000FF, 0x43700000, 0x42500000, 0x00000000, 0xC0000000, 0x3F800000, 0x3C23D70A, 0x3983126F, 0x42B40000, 0x00000000, 0x43700000, 0x42500000, 0x44480000, 0x41000000}; // scene setting kind 0x12
static Scene3Entry_c l_scene3[] = {{8, l_setting0}, {0x11, l_setting1}, {0xD, l_setting2}, {0x12, l_setting3}};
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
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, NULL}, &l_exitInfo};

} // namespace dScData

dSceneData_c *sScDataDmBusPlCrt = &dScData::l_scene.mData;
