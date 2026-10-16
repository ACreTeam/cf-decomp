// The scene data of FIELD (dScene::dSceneData_c sScDataField; see
// include/game/game/d_sc_data.hpp).
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

static PlayerData_c l_player = {{544.0f, 0.0f, 544.0f}, {0, 0, 0}, 0, 8};
static dScene::dSceneBaseData_c l_base[] = {{fProf::SV_DEMO, 0},
                                            {fProf::NPC_MNG, 0}};
static u16 l_resources[] = {fProf::A_NPC_OUT,
                            fProf::A_NPC_SP_OP_TANUKICHI,
                            fProf::A_NPC_SP_RESETSAN,
                            fProf::A_NPC_SP_PUMPKING,
                            fProf::A_NPC_SP_KABURIBA,
                            fProf::A_NPC_SP_RACCO,
                            fProf::A_NPC_SP_PERIO,
                            fProf::A_NPC_SP_SEIICHI,
                            fProf::A_NPC_SP_RORAN,
                            fProf::A_NPC_SP_JONNY,
                            fProf::A_NPC_SP_JINGLE,
                            fProf::A_NPC_SP_FRANKLIN,
                            fProf::A_NPC_SP_UOMASA,
                            fProf::A_NPC_SP_PYONTARO,
                            fProf::A_NPC_SP_GODDESS,
                            fProf::A_NPC_SP_DANCER,
                            fProf::A_NPC_SP_BUG,
                            fProf::A_NPC_SP_EVENT_SONCHO_080,
                            fProf::A_NPC_SP_EVENT_SONCHO_081,
                            fProf::A_NPC_SP_EVENT_SONCHO_082,
                            fProf::A_NPC_SP_EVENT_SONCHO_083,
                            fProf::A_NPC_SP_EVENT_SONCHO_084,
                            fProf::A_NPC_SP_EVENT_SONCHO_085,
                            fProf::A_NPC_SP_EVENT_SONCHO_086,
                            fProf::A_NPC_SP_EVENT_SONCHO_087,
                            fProf::A_NPC_SP_EVENT_SONCHO_088,
                            fProf::A_NPC_SP_EVENT_SONCHO_089,
                            fProf::A_NPC_SP_EVENT_SONCHO_08A,
                            fProf::A_NPC_SP_EVENT_SONCHO_08B,
                            fProf::A_NPC_SP_EVENT_SONCHO_08C,
                            fProf::A_NPC_SP_EVENT_SONCHO_08D,
                            fProf::A_NPC_SP_EVENT_SONCHO_08E,
                            fProf::A_NPC_SP_EVENT_SONCHO_08F,
                            fProf::A_NPC_SP_EVENT_SONCHO_090,
                            fProf::A_NPC_SP_EVENT_SONCHO_091,
                            fProf::A_NPC_SP_EVENT_SONCHO_092,
                            fProf::A_NPC_SP_CEREMONY_SONCHO,
                            fProf::A_NPC_SP_EVENT_SONCHO_07C,
                            fProf::A_NPC_SP_EVENT_SONCHO_07D,
                            fProf::A_NPC_SP_EVENT_SONCHO_07E,
                            fProf::A_NPC_SP_EVENT_SONCHO_07F,
                            fProf::A_NPC_SP_GOLDITEM_SONCHO,
                            fProf::A_NPC_SP_MISSING_MOTHER,
                            fProf::A_NPC_SP_MISSING_CHILD,
                            fProf::A_NPC_SP_MYSTERYCAT,
                            fProf::A_NPC_SP_YUUTARO};
static dScene::dSceneDataEntry_c l_entries[] = {
    {dScene::SCENE_DATA_PLAYER, 1, &l_player},
    {dScene::SCENE_DATA_BASE, dScData::g_commonBaseNum, dScData::g_commonBase},
    {dScene::SCENE_DATA_RES, dScData::g_commonResNum, dScData::g_commonRes},
    {dScene::SCENE_DATA_BASE, dScData::g_fieldBaseNum, dScData::g_fieldBase},
    {dScene::SCENE_DATA_RES, dScData::g_fieldResNum, dScData::g_fieldRes},
    {dScene::SCENE_DATA_BASE, ARRAY_SIZE(l_base), l_base},
    {dScene::SCENE_DATA_RES, ARRAY_SIZE(l_resources), l_resources},
};
static ExitData_c l_exits[] = {
    {{0x4B, 0x4B, 0x4B, 0x4B}, {256.0f, 0.0f, 464.0f}, 0, 0, 0xB, 4, 0, 0},
};
static dScene::dSceneExitList_c l_exitList = {
    (dScene::dSceneExitData_c *)l_exits, ARRAY_SIZE(l_exits)};
static SceneData_c l_scene = {{ARRAY_SIZE(l_entries), l_entries, &l_exitList},
                              NULL};

} // namespace dScData

dScene::dSceneData_c *sScDataField = &dScData::l_scene.mData;
