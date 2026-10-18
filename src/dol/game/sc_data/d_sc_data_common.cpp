// The scene data lists shared by the d_sc_data_* scenes (dScData). A data-only
// TU: .data 804F4BB0..804F4D80, .sdata 8074B5F0..8074B5F8, .sdata2
// 807513A8..807513B8. See include/game/game/d_sc_data.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_sc_data.hpp>

namespace dScData {

dScene::dSceneBaseData_c g_commonBase[] = {
    {fProf::EVENT_MNG, 0},    {fProf::CAMERA, 0},      {fProf::OBJC, 0},
    {fProf::ORG_OBJ, 0},      {fProf::SHADOW, 0},      {fProf::MENU_MNG, 0},
    {fProf::OBJ_KEYBOARD, 0}, {fProf::OBJ_CURSOR, 0},  {fProf::OBJ_VOICE, 0},
    {fProf::AC_UITAG, 0},     {fProf::CLOCK, 0},       {fProf::OBJ_TELOP, 0},
    {fProf::MONEY, 0},        {fProf::OBJ_BAL, 0},     {fProf::OBJ_TUTORIAL, 0},
    {fProf::OBJ_CHAT, 0},     {fProf::MSG, 0},         {fProf::WEATHER, 0},
    {fProf::FI_MDL_MGR, 0},   {fProf::PLAYER_MNGR, 0}, {fProf::CD, 0},
};
const u16 g_commonBaseNum = ARRAY_SIZE(g_commonBase);

u16 g_commonRes[] = {fProf::A_PLAYER, fProf::ORG_OBJ};
const u16 g_commonResNum = ARRAY_SIZE(g_commonRes);

dScene::dSceneBaseData_c g_fieldBase[] = {
    {fProf::BGO_DRAW, 0},      {fProf::STR_MANAGER, 0}, {fProf::FG_DRAW, 1},
    {fProf::FGOBJ_MANAGER, 0}, {fProf::ENVIRONMENT, 0}, {fProf::SKY, 0},
    {fProf::INSECT_FIELD, 0},  {fProf::FISH_FIELD, 0},  {fProf::HAND_ITEM, 0},
    {fProf::A_GROWUP_MSG, 0},
};
const u16 g_fieldBaseNum = ARRAY_SIZE(g_fieldBase);

u16 g_fieldRes[] = {
    fProf::FG_DRAW,
    fProf::FGOBJ_MANAGER,
    fProf::BGO_DRAW,
    fProf::STR_MANAGER,
    fProf::A_STR,
    fProf::STR_PL,
    fProf::A_STR_CDBBS,
    fProf::A_BBS_BIRD,
    fProf::STR_BUS,
    fProf::SKY,
    fProf::SKY_AURORA,
    fProf::SKY_RAINBOW,
    fProf::INSECT_FIELD,
    fProf::FISH_FIELD,
    fProf::FLAG,
    fProf::COUNTDOWN_FLAG,
    fProf::A_NAMEPLATE,
    fProf::A_GROWUP_MSG,
    fProf::A_UFO,
    fProf::BALLOON,
    fProf::BALLOON_MNG,
};
const u16 g_fieldResNum = ARRAY_SIZE(g_fieldRes);

dScene::dSceneBaseData_c g_roomBase[] = {
    {fProf::BGI_DRAW, 0},    {fProf::FG_DRAW, 0}, {fProf::FGOBJ_MANAGER, 0},
    {fProf::ENVIRONMENT, 1}, {fProf::SHOP_FG, 0}, {fProf::FTR_MANAGER, 0},
    {fProf::HAND_ITEM, 0},
};
const u16 g_roomBaseNum = ARRAY_SIZE(g_roomBase);

u16 g_roomRes[] = {fProf::FG_DRAW,       fProf::FGOBJ_MANAGER, fProf::BGI_DRAW,
                   fProf::SHOP_FG,       fProf::FTR_MANAGER,   fProf::A_FTR,
                   fProf::FTR_LP, fProf::A_SIGN_TAG};
const u16 g_roomResNum = ARRAY_SIZE(g_roomRes);

dScene::dSceneBaseData_c g_townBase[] = {
    {fProf::BGT_DRAW, 0},      {fProf::FG_DRAW, 0},
    {fProf::FGOBJ_MANAGER, 0}, {fProf::STR_MANAGER, 0},
    {fProf::ENVIRONMENT, 0},   {fProf::SKY, 0},
    {fProf::HAND_ITEM, 0},     {fProf::A_GROWUP_MSG, 0},
};
const u16 g_townBaseNum = ARRAY_SIZE(g_townBase);

u16 g_townRes[] = {
    fProf::FG_DRAW,          fProf::FGOBJ_MANAGER,  fProf::SKY,
    fProf::SKY_AURORA,       fProf::SKY_RAINBOW,    fProf::BGT_DRAW,
    fProf::A_RMOBJ_ATM,      fProf::STR_MANAGER,    fProf::A_STR,
    fProf::STR_BUS, fProf::A_STR_TOWN_112, fProf::A_SIGN_TAG,
    fProf::A_GROWUP_MSG,
};
const u16 g_townResNum = ARRAY_SIZE(g_townRes);

} // namespace dScData
