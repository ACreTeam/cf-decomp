// The profiles' REL modules (dDyl). .text 80086C48..80087064, .rodata
// 8046DAD8..8046E340, .data 804DE8C8..804DF520, .sdata 80749E40..80749E78,
// .sbss 8074E2A0..8074E2C0. See include/game/game/d_dylink.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_dylink.hpp>
#include <game/cLib/c_dylink.hpp>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_dvd.hpp>
#include <game/mLib/m_heap.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <lib/revolution/OS/OSLink.h>
#include <string.h>

namespace dDyl {

const DynamicNameTable_c *DynamicNameTable;
int DynamicNameTableNum;
DynamicModuleControlBase **DMC;
int DMC_num;
EGG::FrmHeap *cCc_frmHeap;
BOOL Initialized;
mDvd_callback_c *cDyl_DVD;

// 8046DAD8: the profiles in REL modules.
static const DynamicNameTable_c l_dylinkTable[] = {
    {fProf::S_STAGE, "d_s_stage"},
    {fProf::A_GROWUP_MSG, "d_a_growup_msg"},
    {fProf::A_PLAYER, "d_a_player"},
    {fProf::ORG_OBJ, "d_org_obj"},
    {fProf::A_RMOBJ_ATM, "d_a_rmobj_atm"},
    {fProf::A_RMOBJ_BARRICADE, "d_a_rmobj_barricade"},
    {fProf::A_RMOBJ_BUS, "d_a_rmobj_bus"},
    {fProf::A_RMOBJ_CAFE, "d_a_rmobj_cafe"},
    {fProf::A_RMOBJ_CHECKP, "d_a_rmobj_checkp"},
    {fProf::A_RMOBJ_FOSSIL, "d_a_rmobj_fossil"},
    {fProf::A_RMOBJ_GRACE, "d_a_rmobj_grace"},
    {fProf::A_RMOBJ_PERM_MACHINE, "d_a_rmobj_perm_machine"},
    {fProf::A_RMOBJ_PIC, "d_a_rmobj_pic"},
    {fProf::A_RMOBJ_POINT, "d_a_rmobj_point"},
    {fProf::A_RMOBJ_RECYCLEBOX, "d_a_rmobj_recyclebox"},
    {fProf::A_RMOBJ_RESET, "d_a_rmobj_reset"},
    {fProf::A_RMOBJ_SAMPLE, "d_a_rmobj_sample"},
    {fProf::A_RMOBJ_SEWING_MACHINE, "d_a_rmobj_sewing_machine"},
    {fProf::A_RMOBJ_TEL, "d_a_rmobj_tel"},
    {fProf::A_RMOBJ_THEATER, "d_a_rmobj_theater"},
    {fProf::A_RMOBJ_MIRRORBALL, "d_a_rmobj_mirrorball"},
    {fProf::A_TICKET_COUNTER, "d_a_ticket_counter"},
    {fProf::COCKROACH_MANAGER, "d_cockroach_manager"},
    {fProf::A_COCKROACH, "d_a_cockroach"},
    {fProf::A_FTR, "d_a_ftr"},
    {fProf::FTR_LP, "d_a_ftr_act"},
    {fProf::FTR_TRG, "d_a_ftr_act"},
    {fProf::FTR_SW, "d_a_ftr_act"},
    {fProf::FTR_SW2, "d_a_ftr_act"},
    {fProf::FTR_FLP, "d_a_ftr_act"},
    {fProf::FTR2_W, "d_a_ftr_act"},
    {fProf::FTR_FSW, "d_a_ftr_act"},
    {fProf::FTR_CHAIR, "d_a_ftr_act"},
    {fProf::FTR_BED, "d_a_ftr_act"},
    {fProf::FTR_CHEST, "d_a_ftr_act"},
    {fProf::FTR_CLOCK, "d_a_ftr_act"},
    {fProf::FTR_AUDIO, "d_a_ftr_act"},
    {fProf::FTR_TV, "d_a_ftr_act"},
    {fProf::FTR_TV_F, "d_a_ftr_act"},
    {fProf::FTR_PCART, "d_a_ftr_act"},
    {fProf::FTR_HNW, "d_a_ftr_act"},
    {fProf::FTR_FAW, "d_a_ftr_act"},
    {fProf::FTR_KILLER, "d_a_ftr_act"},
    {fProf::FTR_CLOTH, "d_a_ftr_act"},
    {fProf::FTR_MYOBJ, "d_a_ftr_act"},
    {fProf::FTR_TAOBJ, "d_a_ftr_act"},
    {fProf::FTR_CMPS, "d_a_ftr_act"},
    {fProf::FTR_INSTRUMENT, "d_a_ftr_act"},
    {fProf::FTR_BK, "d_a_ftr_act"},
    {fProf::FTR_METRO, "d_a_ftr_act"},
    {fProf::FTR_CATALOG_CW, "d_a_ftr_act"},
    {fProf::FTR_ACC, "d_a_ftr_act"},
    {fProf::FTR_UMB, "d_a_ftr_act"},
    {fProf::FTR_BALLOON, "d_a_ftr_act"},
    {fProf::FTR_WINDMILL, "d_a_ftr_act"},
    {fProf::FTR_MONITOR, "d_a_ftr_act"},
    {fProf::FTR_CICADA, "d_a_ftr_act"},
    {fProf::FTR_TEL, "d_a_ftr_act"},
    {fProf::FTR_FLOWER, "d_a_ftr_act"},
    {fProf::FTR_ATM, "d_a_ftr_act"},
    {fProf::FTR_BARLEY, "d_a_ftr_act"},
    {fProf::FTR_ROLLER, "d_a_ftr_act"},
    {fProf::FTR_MANAGER, "d_ftr_manager"},
    {fProf::A_SAMPLE, "d_a_sample"},
    {fProf::A_SAMPLE2, "d_a_sample2"},
    {fProf::A_SAMPLE3, "d_a_sample3"},
    {fProf::A_NPC_OUT, "d_a_npc_out"},
    {fProf::A_NPC_IN, "d_a_npc_in"},
    {fProf::A_NPC_VISIT_NPC_HOUSE, "d_a_npc_visit_npc_house"},
    {fProf::A_NPC_VISIT_PLAYER_HOUSE, "d_a_npc_visit_player_house"},
    {fProf::A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET,
     "d_a_npc_visit_player_house_fleamarket"},
    {fProf::A_NPC_TOWN_OUT, "d_a_npc_town_out"},
    {fProf::A_NPC_TOWN_GRACE, "d_a_npc_town_grace"},
    {fProf::A_NPC_TOWN_THEATER, "d_a_npc_town_theater"},
    {fProf::A_NPC_CAFE, "d_a_npc_cafe"},
    {fProf::A_NPC_SP_SAMPLE, "d_a_npc_sp_sample"},
    {fProf::A_NPC_SP_PLAYER_CREATE, "d_a_npc_sp_player_create"},
    {fProf::A_NPC_SP_PLAYER_SELECT, "d_a_npc_sp_player_select"},
    {fProf::A_NPC_SP_RESETSAN, "d_a_npc_sp_resetsan"},
    {fProf::A_NPC_SP_CAFE_GUEST, "d_a_npc_sp_cafe_guest"},
    {fProf::A_NPC_SP_PERIKO_05A, "d_a_npc_sp_periko"},
    {fProf::A_NPC_SP_PERIKO_05B, "d_a_npc_sp_periko"},
    {fProf::A_NPC_SP_SONCHO, "d_a_npc_sp_soncho"},
    {fProf::A_NPC_SP_OP_TANUKICHI, "d_a_npc_sp_op_tanukichi"},
    {fProf::A_NPC_SP_TANUKICHI_05E, "d_a_npc_sp_tanukichi"},
    {fProf::A_NPC_SP_TANUKICHI_05F, "d_a_npc_sp_tanukichi"},
    {fProf::A_NPC_SP_TANUKICHI_060, "d_a_npc_sp_tanukichi"},
    {fProf::A_NPC_SP_KABURIBA, "d_a_npc_sp_kaburiba"},
    {fProf::A_NPC_SP_TUNEKICHI, "d_a_npc_sp_tunekichi"},
    {fProf::A_NPC_SP_CATHERINE, "d_a_npc_sp_catherine"},
    {fProf::A_NPC_SP_ASAMI, "d_a_npc_sp_asami"},
    {fProf::A_NPC_SP_KINUYO, "d_a_npc_sp_kinuyo"},
    {fProf::A_NPC_SP_JIJYO, "d_a_npc_sp_jijyo"},
    {fProf::A_NPC_SP_CAFE, "d_a_npc_sp_cafe"},
    {fProf::A_NPC_SP_FUTA, "d_a_npc_sp_futa"},
    {fProf::A_NPC_SP_FUKO, "d_a_npc_sp_fuko"},
    {fProf::A_NPC_SP_SEIICHI, "d_a_npc_sp_seiichi"},
    {fProf::A_NPC_SP_RORAN, "d_a_npc_sp_roran"},
    {fProf::A_NPC_SP_KAPPEI, "d_a_npc_sp_kappei"},
    {fProf::A_NPC_SP_TOTAKEKE, "d_a_npc_sp_totakeke"},
    {fProf::A_NPC_SP_JONNY, "d_a_npc_sp_jonny"},
    {fProf::A_NPC_SP_RACCO, "d_a_npc_sp_racco"},
    {fProf::A_NPC_SP_PERIO, "d_a_npc_sp_perio"},
    {fProf::A_NPC_SP_FORTUNE, "d_a_npc_sp_fortune"},
    {fProf::A_NPC_SP_SISYO, "d_a_npc_sp_sisyo"},
    {fProf::A_NPC_SP_HONMA, "d_a_npc_sp_honma"},
    {fProf::A_NPC_SP_GRACE, "d_a_npc_sp_grace"},
    {fProf::A_NPC_SP_GATEKEEPER_A, "d_a_npc_sp_gatekeeper_a"},
    {fProf::A_NPC_SP_GATEKEEPER_B, "d_a_npc_sp_gatekeeper_b"},
    {fProf::A_NPC_SP_JINGLE, "d_a_npc_sp_jingle"},
    {fProf::A_NPC_SP_FRANKLIN, "d_a_npc_sp_franklin"},
    {fProf::A_NPC_SP_UOMASA, "d_a_npc_sp_uomasa"},
    {fProf::A_NPC_SP_PUMPKING, "d_a_npc_sp_pumpking"},
    {fProf::A_NPC_SP_KUTSUMIGAKI, "d_a_npc_sp_kutsumigaki"},
    {fProf::A_NPC_SP_EVENT_SONCHO_07C, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_07D, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_07E, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_07F, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_080, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_081, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_082, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_083, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_084, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_085, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_086, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_087, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_088, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_089, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_08A, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_08B, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_08C, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_08D, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_08E, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_08F, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_090, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_091, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_EVENT_SONCHO_092, "d_a_npc_sp_event_soncho"},
    {fProf::A_NPC_SP_CEREMONY_SONCHO, "d_a_npc_sp_ceremony_soncho"},
    {fProf::A_NPC_SP_GOLDITEM_SONCHO, "d_a_npc_sp_golditem_soncho"},
    {fProf::A_NPC_SP_PYONTARO, "d_a_npc_sp_pyontaro"},
    {fProf::A_NPC_SP_GODDESS, "d_a_npc_sp_goddess"},
    {fProf::A_NPC_SP_BUG, "d_a_npc_sp_bug"},
    {fProf::A_NPC_SP_HANIWA, "d_a_npc_sp_haniwa"},
    {fProf::A_NPC_SP_DEMO_RESETSAN_099, "d_a_npc_sp_demo_resetsan"},
    {fProf::A_NPC_SP_DEMO_RESETSAN_09A, "d_a_npc_sp_demo_resetsan"},
    {fProf::A_NPC_SP_DANCER, "d_a_npc_sp_dancer"},
    {fProf::A_NPC_SP_MISSING_CHILD, "d_a_npc_sp_missing_child"},
    {fProf::A_NPC_SP_MISSING_MOTHER, "d_a_npc_sp_missing_mother"},
    {fProf::A_NPC_SP_MYSTERYCAT, "d_a_npc_sp_mysterycat"},
    {fProf::A_NPC_SP_YUUTARO, "d_a_npc_sp_yuutaro"},
    {fProf::A_NPC_SP_YUUTARO_LOFT, "d_a_npc_sp_yuutaro_loft"},
    {fProf::A_NPC_SP_FUUSEN, "d_a_npc_sp_fuusen"},
    {fProf::S_SELECT, "d_s_select"},
    {fProf::TITLE_DEMO, "d_title_demo"},
    {fProf::SKY, "d_sky"},
    {fProf::SKY_AURORA, "d_sky_aurora"},
    {fProf::SKY_RAINBOW, "d_sky_rainbow"},
    {fProf::FISH_MUSEUM, "d_fish_museum"},
    {fProf::INSECT_MUSEUM, "d_insect_museum"},
    {fProf::FISH_FIELD, "d_fish_field"},
    {fProf::INSECT_FIELD, "d_insect_field"},
    {fProf::A_LAYOUT, "d_a_layout"},
    {fProf::MENU_ITEM00, "d_menu_item00"},
    {fProf::MENU_ITEM01, "d_menu_item01"},
    {fProf::MENU_ITEM02, "d_menu_item02"},
    {fProf::MENU_ITEM03, "d_menu_item03"},
    {fProf::MENU_ITEM04, "d_menu_item04"},
    {fProf::MENU_ITEM05, "d_menu_item05"},
    {fProf::MENU_ITEM06, "d_menu_item06"},
    {fProf::MENU_FRIEND, "d_menu_friend"},
    {fProf::MENU_FRIEND01, "d_menu_friend01"},
    {fProf::MENU_FRIEND02, "d_menu_friend02"},
    {fProf::MENU_FRIEND03, "d_menu_friend03"},
    {fProf::MENU_FRIEND04, "d_menu_friend04"},
    {fProf::MENU_FRIEND05, "d_menu_friend05"},
    {fProf::MENU_FRIEND06, "d_menu_friend06"},
    {fProf::MENU_FRIEND07, "d_menu_friend07"},
    {fProf::MENU_FRIEND08, "d_menu_friend08"},
    {fProf::MENU_INSC, "d_menu_insc"},
    {fProf::MENU_CHAT, "d_menu_chat"},
    {fProf::MENU_DESI00, "d_menu_desi00"},
    {fProf::MENU_EDIT, "d_menu_edit"},
    {fProf::MENU_PAPER, "d_menu_paper"},
    {fProf::MENU_KEYTEST, "d_menu_keytest"},
    {fProf::MENU_HAN, "d_menu_han"},
    {fProf::MENU_MAP00, "d_menu_map00"},
    {fProf::MENU_MAP01, "d_menu_map01"},
    {fProf::MENU_MAP02, "d_menu_map02"},
    {fProf::MENU_MAP03, "d_menu_map03"},
    {fProf::MENU_PHOTO, "d_menu_photo"},
    {fProf::MENU_PHOTO01, "d_menu_photo01"},
    {fProf::MENU_CATALOG, "d_menu_catalog"},
    {fProf::MENU_NUMBER00, "d_menu_number00"},
    {fProf::MENU_MUSEUM, "d_menu_museum"},
    {fProf::MENU_TIME00, "d_menu_time00"},
    {fProf::MENU_WRITE, "d_menu_write"},
    {fProf::MENU_BANK, "d_menu_bank"},
    {fProf::MENU_KABU, "d_menu_kabu"},
    {fProf::MENU_MSLCT, "d_menu_mslct"},
    {fProf::MENU_MII, "d_menu_mii"},
    {fProf::MENU_DESI01, "d_menu_desi01"},
    {fProf::MENU_DESI02, "d_menu_desi02"},
    {fProf::MENU_DESI03, "d_menu_desi03"},
    {fProf::MENU_MUSIC, "d_menu_music"},
    {fProf::MENU_POST00, "d_menu_post00"},
    {fProf::MENU_POST01, "d_menu_post01"},
    {fProf::MENU_NOTICE, "d_menu_notice"},
    {fProf::MENU_NOTICE00, "d_menu_notice00"},
    {fProf::MENU_NOTICE01, "d_menu_notice01"},
    {fProf::MENU_MELODY, "d_menu_melody"},
    {fProf::MENU_DEBUG00, "d_menu_debug00"},
    {fProf::MENU_STAR00, "d_menu_star00"},
    {fProf::MENU_STAR01, "d_menu_star01"},
    {fProf::MENU_POINT, "d_menu_point"},
    {fProf::MENU_HANIWA, "d_menu_haniwa"},
    {fProf::A_MDL, "d_a_mdl"},
    {fProf::BGT_DRAW, "d_bgt_draw"},
    {fProf::BGO_DRAW, "d_bgo_draw"},
    {fProf::BGI_DRAW, "d_bgi_draw"},
    {fProf::FG_DRAW, "d_fg_draw"},
    {fProf::FGOBJ_MANAGER, "d_fgobj_manager"},
    {fProf::SHOP_FG, "d_shop_fg"},
    {fProf::A_NAMEPLATE, "d_a_nameplate"},
    {fProf::A_NAMEPLATE_MUSEUM, "d_a_nameplate_museum"},
    {fProf::A_SIGN_TAG, "d_a_sign_tag"},
    {fProf::A_STR, "d_a_str"},
    {fProf::A_STR_FIELD_0FB, "d_a_str_field"},
    {fProf::A_STR_FIELD_0FC, "d_a_str_field"},
    {fProf::A_STR_FIELD_0FD, "d_a_str_field"},
    {fProf::A_STR_FIELD_0FE, "d_a_str_field"},
    {fProf::A_STR_FIELD_0FF, "d_a_str_field"},
    {fProf::A_STR_FIELD_100, "d_a_str_field"},
    {fProf::A_STR_FIELD_101, "d_a_str_field"},
    {fProf::A_STR_FIELD_102, "d_a_str_field"},
    {fProf::A_STR_FIELD_103, "d_a_str_field"},
    {fProf::A_STR_CDBBS, "d_a_str_cdbbs"},
    {fProf::A_STR_FIELD_105, "d_a_str_field"},
    {fProf::A_STR_FIELD_106, "d_a_str_field"},
    {fProf::A_STR_FIELD_107, "d_a_str_field"},
    {fProf::A_STR_FIELD_108, "d_a_str_field"},
    {fProf::A_STR_FIELD_109, "d_a_str_field"},
    {fProf::A_STR_FIELD_10A, "d_a_str_field"},
    {fProf::A_STR_FIELD_10B, "d_a_str_field"},
    {fProf::A_STR_COMMON_10C, "d_a_str_common"},
    {fProf::A_STR_COMMON_10D, "d_a_str_common"},
    {fProf::A_STR_TOWN_10E, "d_a_str_town"},
    {fProf::A_STR_TOWN_10F, "d_a_str_town"},
    {fProf::A_STR_TOWN_110, "d_a_str_town"},
    {fProf::A_STR_TOWN_111, "d_a_str_town"},
    {fProf::A_STR_TOWN_112, "d_a_str_town"},
    {fProf::A_STR_TOWN_113, "d_a_str_town"},
    {fProf::A_STR_TOWN_114, "d_a_str_town"},
    {fProf::A_STR_TOWN_115, "d_a_str_town"},
    {fProf::A_STR_TOWN_116, "d_a_str_town"},
    {fProf::A_STR_TOWN_117, "d_a_str_town"},
    {fProf::A_STR_TOWN_118, "d_a_str_town"},
    {fProf::A_STR_TOWN_119, "d_a_str_town"},
    {fProf::A_STR_TOWN_11A, "d_a_str_town"},
    {fProf::A_STR_TOWN_11B, "d_a_str_town"},
    {fProf::A_STR_FIELD_11C, "d_a_str_field"},
    {fProf::FLAG, "d_flag"},
    {fProf::COUNTDOWN_FLAG, "d_countdown_flag"},
    {fProf::ORGAN_BALLOON, "d_organ_balloon"},
    {fProf::A_BALLOON_120, "d_balloon_manager"},
    {fProf::A_BALLOON_121, "d_a_balloon"},
    {fProf::A_UFO, "d_a_ufo"},
    {fProf::STR_MANAGER, "d_str_manager"},
    {fProf::A_BBS_BIRD, "d_a_bbs_bird"},
    {fProf::PROFILE_COUNT, NULL},
};

// 80086C48
BOOL initDMC(int profNum, const DynamicNameTable_c *table, int tableNum,
             EGG::Heap *heap) {
    cCc_frmHeap = mHeap::createFrmHeap(
        profNum * 16 + tableNum * sizeof(DynamicModuleControl), heap,
        "ダイナミックリンク制御用ヒープ(dDyl::cCc_frmHeap)", 0x20,
        mHeap::OPT_NONE);
    EGG::Heap *prevHeap = mHeap::setCurrentHeap(cCc_frmHeap);
    DMC_num = profNum;
    DMC = new DynamicModuleControlBase *[profNum];
    DynamicNameTable = table;
    DynamicNameTableNum = tableNum;
    for (int i = 0; i < DynamicNameTableNum; i++) {
        const DynamicNameTable_c *entry = &DynamicNameTable[i];
        if (entry->mName != NULL) {
            for (int j = 0; j < DMC_num; j++) {
                if (DMC[j] != NULL &&
                    strcmp(entry->mName, DMC[j]->getModuleName()) == 0) {
                    DMC[entry->mProfName] = DMC[j];
                    break;
                }
            }
            if (DMC[entry->mProfName] == NULL) {
                DMC[entry->mProfName] =
                    new DynamicModuleControl(entry->mName, NULL);
            }
        }
    }
    cCc_frmHeap->adjust();
    mHeap::setCurrentHeap(prevHeap);
    return TRUE;
}

// 80086DD0
BOOL isLinked(ProfileName profName) {
    DynamicModuleControlBase *dmc = DMC[profName];
    if (dmc != NULL) {
        return dmc->isLinked();
    }
    return TRUE;
}

// 80086E00
BOOL unlink(ProfileName profName) {
    DynamicModuleControlBase *dmc = DMC[profName];
    if (dmc != NULL) {
        return dmc->unlink();
    }
    return FALSE;
}

// 80086E20
int linkASync(ProfileName profName) {
    if (!Initialized) {
        return fBase_c::NOT_READY;
    }
    DynamicModuleControlBase *dmc = DMC[profName];
    if (dmc != NULL) {
        if (dmc->load_async()) {
            if (dmc->link()) {
                return fBase_c::SUCCEEDED;
            }
            return fBase_c::FAILED;
        }
        return fBase_c::NOT_READY;
    }
    return fBase_c::SUCCEEDED;
}

// 80086EA4
void *initCallback(void *arg) {
    DynamicModuleControl::initialize(mHeap::g_dylinkHeap);
    DynamicModuleControl::sModulesDir = "/rels";
    if (DynamicModuleControl::sDvdFile->openFilename("/RVFOREST1NP.str")) {
        size_t size = DynamicModuleControl::sDvdFile->getFileSize();
        if (size != 0) {
            size_t bufSize = ROUND_UP(size, 32);
            void *buf = DynamicModuleControl::sDylinkHeap->alloc(bufSize, 0x20);
            DynamicModuleControl::sDvdFile->readData(buf, bufSize, 0);
            OSSetStringTable(buf);
        }
        DynamicModuleControl::sDvdFile->close();
    }
    DynamicModuleControl profile("d_profile", NULL);
    profile.link();
    Initialized = TRUE;
    return (void *)TRUE;
}

// 80086FB8
void initAsync() {
    initDMC(PROFILE_NUM, l_dylinkTable, ARRAY_SIZE(l_dylinkTable),
            mHeap::g_dylinkHeap);
    cDyl_DVD = mDvd_callback_c::create(initCallback, mHeap::g_dylinkHeap);
}

// 80087000
BOOL initAsyncIsDone() {
    if (cDyl_DVD == NULL) {
        return TRUE;
    }
    if (cDyl_DVD->mDone == TRUE) {
        cDyl_DVD->destroy();
        cDyl_DVD = NULL;
        return TRUE;
    }
    return FALSE;
}

} // namespace dDyl
