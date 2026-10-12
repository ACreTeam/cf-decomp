// The profiles' REL modules (dDyl). .text 80086C48..80087064, .rodata 8046DAD8..8046E340, .data 804DE8C8..804DF520,
// .sdata 80749E40..80749E78, .sbss 8074E2A0..8074E2C0. See include/game/game/d_dylink.hpp.
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
    {0x001, "d_s_stage"},
    {0x008, "d_a_growup_msg"},
    {0x009, "d_a_player"},
    {0x00C, "d_org_obj"},
    {0x00D, "d_a_rmobj_atm"},
    {0x00E, "d_a_rmobj_barricade"},
    {0x00F, "d_a_rmobj_bus"},
    {0x010, "d_a_rmobj_cafe"},
    {0x011, "d_a_rmobj_checkp"},
    {0x012, "d_a_rmobj_fossil"},
    {0x013, "d_a_rmobj_grace"},
    {0x014, "d_a_rmobj_perm_machine"},
    {0x015, "d_a_rmobj_pic"},
    {0x016, "d_a_rmobj_point"},
    {0x017, "d_a_rmobj_recyclebox"},
    {0x018, "d_a_rmobj_reset"},
    {0x019, "d_a_rmobj_sample"},
    {0x01A, "d_a_rmobj_sewing_machine"},
    {0x01B, "d_a_rmobj_tel"},
    {0x01C, "d_a_rmobj_theater"},
    {0x01D, "d_a_rmobj_mirrorball"},
    {0x01E, "d_a_ticket_counter"},
    {0x01F, "d_cockroach_manager"},
    {0x020, "d_a_cockroach"},
    {0x021, "d_a_ftr"},
    {0x022, "d_a_ftr_act"},
    {0x023, "d_a_ftr_act"},
    {0x024, "d_a_ftr_act"},
    {0x025, "d_a_ftr_act"},
    {0x026, "d_a_ftr_act"},
    {0x027, "d_a_ftr_act"},
    {0x028, "d_a_ftr_act"},
    {0x029, "d_a_ftr_act"},
    {0x02A, "d_a_ftr_act"},
    {0x02B, "d_a_ftr_act"},
    {0x02C, "d_a_ftr_act"},
    {0x02D, "d_a_ftr_act"},
    {0x02E, "d_a_ftr_act"},
    {0x02F, "d_a_ftr_act"},
    {0x030, "d_a_ftr_act"},
    {0x031, "d_a_ftr_act"},
    {0x032, "d_a_ftr_act"},
    {0x033, "d_a_ftr_act"},
    {0x034, "d_a_ftr_act"},
    {0x035, "d_a_ftr_act"},
    {0x036, "d_a_ftr_act"},
    {0x037, "d_a_ftr_act"},
    {0x038, "d_a_ftr_act"},
    {0x039, "d_a_ftr_act"},
    {0x03A, "d_a_ftr_act"},
    {0x03B, "d_a_ftr_act"},
    {0x03C, "d_a_ftr_act"},
    {0x03D, "d_a_ftr_act"},
    {0x03E, "d_a_ftr_act"},
    {0x03F, "d_a_ftr_act"},
    {0x040, "d_a_ftr_act"},
    {0x041, "d_a_ftr_act"},
    {0x042, "d_a_ftr_act"},
    {0x043, "d_a_ftr_act"},
    {0x044, "d_a_ftr_act"},
    {0x045, "d_a_ftr_act"},
    {0x046, "d_a_ftr_act"},
    {0x047, "d_ftr_manager"},
    {0x048, "d_a_sample"},
    {0x049, "d_a_sample2"},
    {0x04A, "d_a_sample3"},
    {0x04C, "d_a_npc_out"},
    {0x04D, "d_a_npc_in"},
    {0x04E, "d_a_npc_visit_npc_house"},
    {0x04F, "d_a_npc_visit_player_house"},
    {0x050, "d_a_npc_visit_player_house_fleamarket"},
    {0x051, "d_a_npc_town_out"},
    {0x052, "d_a_npc_town_grace"},
    {0x053, "d_a_npc_town_theater"},
    {0x054, "d_a_npc_cafe"},
    {0x055, "d_a_npc_sp_sample"},
    {0x056, "d_a_npc_sp_player_create"},
    {0x057, "d_a_npc_sp_player_select"},
    {0x058, "d_a_npc_sp_resetsan"},
    {0x059, "d_a_npc_sp_cafe_guest"},
    {0x05A, "d_a_npc_sp_periko"},
    {0x05B, "d_a_npc_sp_periko"},
    {0x05C, "d_a_npc_sp_soncho"},
    {0x05D, "d_a_npc_sp_op_tanukichi"},
    {0x05E, "d_a_npc_sp_tanukichi"},
    {0x05F, "d_a_npc_sp_tanukichi"},
    {0x060, "d_a_npc_sp_tanukichi"},
    {0x061, "d_a_npc_sp_kaburiba"},
    {0x062, "d_a_npc_sp_tunekichi"},
    {0x063, "d_a_npc_sp_catherine"},
    {0x064, "d_a_npc_sp_asami"},
    {0x065, "d_a_npc_sp_kinuyo"},
    {0x066, "d_a_npc_sp_jijyo"},
    {0x067, "d_a_npc_sp_cafe"},
    {0x068, "d_a_npc_sp_futa"},
    {0x069, "d_a_npc_sp_fuko"},
    {0x06A, "d_a_npc_sp_seiichi"},
    {0x06B, "d_a_npc_sp_roran"},
    {0x06C, "d_a_npc_sp_kappei"},
    {0x06D, "d_a_npc_sp_totakeke"},
    {0x06E, "d_a_npc_sp_jonny"},
    {0x06F, "d_a_npc_sp_racco"},
    {0x070, "d_a_npc_sp_perio"},
    {0x071, "d_a_npc_sp_fortune"},
    {0x072, "d_a_npc_sp_sisyo"},
    {0x073, "d_a_npc_sp_honma"},
    {0x074, "d_a_npc_sp_grace"},
    {0x075, "d_a_npc_sp_gatekeeper_a"},
    {0x076, "d_a_npc_sp_gatekeeper_b"},
    {0x077, "d_a_npc_sp_jingle"},
    {0x078, "d_a_npc_sp_franklin"},
    {0x079, "d_a_npc_sp_uomasa"},
    {0x07A, "d_a_npc_sp_pumpking"},
    {0x07B, "d_a_npc_sp_kutsumigaki"},
    {0x07C, "d_a_npc_sp_event_soncho"},
    {0x07D, "d_a_npc_sp_event_soncho"},
    {0x07E, "d_a_npc_sp_event_soncho"},
    {0x07F, "d_a_npc_sp_event_soncho"},
    {0x080, "d_a_npc_sp_event_soncho"},
    {0x081, "d_a_npc_sp_event_soncho"},
    {0x082, "d_a_npc_sp_event_soncho"},
    {0x083, "d_a_npc_sp_event_soncho"},
    {0x084, "d_a_npc_sp_event_soncho"},
    {0x085, "d_a_npc_sp_event_soncho"},
    {0x086, "d_a_npc_sp_event_soncho"},
    {0x087, "d_a_npc_sp_event_soncho"},
    {0x088, "d_a_npc_sp_event_soncho"},
    {0x089, "d_a_npc_sp_event_soncho"},
    {0x08A, "d_a_npc_sp_event_soncho"},
    {0x08B, "d_a_npc_sp_event_soncho"},
    {0x08C, "d_a_npc_sp_event_soncho"},
    {0x08D, "d_a_npc_sp_event_soncho"},
    {0x08E, "d_a_npc_sp_event_soncho"},
    {0x08F, "d_a_npc_sp_event_soncho"},
    {0x090, "d_a_npc_sp_event_soncho"},
    {0x091, "d_a_npc_sp_event_soncho"},
    {0x092, "d_a_npc_sp_event_soncho"},
    {0x093, "d_a_npc_sp_ceremony_soncho"},
    {0x094, "d_a_npc_sp_golditem_soncho"},
    {0x095, "d_a_npc_sp_pyontaro"},
    {0x096, "d_a_npc_sp_goddess"},
    {0x097, "d_a_npc_sp_bug"},
    {0x098, "d_a_npc_sp_haniwa"},
    {0x099, "d_a_npc_sp_demo_resetsan"},
    {0x09A, "d_a_npc_sp_demo_resetsan"},
    {0x09B, "d_a_npc_sp_dancer"},
    {0x09C, "d_a_npc_sp_missing_child"},
    {0x09D, "d_a_npc_sp_missing_mother"},
    {0x09E, "d_a_npc_sp_mysterycat"},
    {0x09F, "d_a_npc_sp_yuutaro"},
    {0x0A0, "d_a_npc_sp_yuutaro_loft"},
    {0x0A1, "d_a_npc_sp_fuusen"},
    {0x0A2, "d_s_select"},
    {0x0A5, "d_title_demo"},
    {0x0AA, "d_sky"},
    {0x0AB, "d_sky_aurora"},
    {0x0AC, "d_sky_rainbow"},
    {0x0B0, "d_fish_museum"},
    {0x0B1, "d_insect_museum"},
    {0x0B2, "d_fish_field"},
    {0x0B3, "d_insect_field"},
    {0x0B4, "d_a_layout"},
    {0x0B6, "d_menu_item00"},
    {0x0B7, "d_menu_item01"},
    {0x0B8, "d_menu_item02"},
    {0x0B9, "d_menu_item03"},
    {0x0BA, "d_menu_item04"},
    {0x0BB, "d_menu_item05"},
    {0x0BC, "d_menu_item06"},
    {0x0BD, "d_menu_friend"},
    {0x0BE, "d_menu_friend01"},
    {0x0BF, "d_menu_friend02"},
    {0x0C0, "d_menu_friend03"},
    {0x0C1, "d_menu_friend04"},
    {0x0C2, "d_menu_friend05"},
    {0x0C3, "d_menu_friend06"},
    {0x0C4, "d_menu_friend07"},
    {0x0C5, "d_menu_friend08"},
    {0x0C6, "d_menu_insc"},
    {0x0C7, "d_menu_chat"},
    {0x0C8, "d_menu_desi00"},
    {0x0C9, "d_menu_edit"},
    {0x0CA, "d_menu_paper"},
    {0x0CB, "d_menu_keytest"},
    {0x0CC, "d_menu_han"},
    {0x0CD, "d_menu_map00"},
    {0x0CE, "d_menu_map01"},
    {0x0CF, "d_menu_map02"},
    {0x0D0, "d_menu_map03"},
    {0x0D1, "d_menu_photo"},
    {0x0D2, "d_menu_photo01"},
    {0x0D3, "d_menu_catalog"},
    {0x0D4, "d_menu_number00"},
    {0x0D5, "d_menu_museum"},
    {0x0D6, "d_menu_time00"},
    {0x0D7, "d_menu_write"},
    {0x0D8, "d_menu_bank"},
    {0x0D9, "d_menu_kabu"},
    {0x0DA, "d_menu_mslct"},
    {0x0DB, "d_menu_mii"},
    {0x0DC, "d_menu_desi01"},
    {0x0DD, "d_menu_desi02"},
    {0x0DE, "d_menu_desi03"},
    {0x0DF, "d_menu_music"},
    {0x0E0, "d_menu_post00"},
    {0x0E1, "d_menu_post01"},
    {0x0E2, "d_menu_notice"},
    {0x0E3, "d_menu_notice00"},
    {0x0E4, "d_menu_notice01"},
    {0x0E5, "d_menu_melody"},
    {0x0E6, "d_menu_debug00"},
    {0x0E7, "d_menu_star00"},
    {0x0E8, "d_menu_star01"},
    {0x0E9, "d_menu_point"},
    {0x0EA, "d_menu_haniwa"},
    {0x0EB, "d_a_mdl"},
    {0x0EC, "d_bgt_draw"},
    {0x0ED, "d_bgo_draw"},
    {0x0EE, "d_bgi_draw"},
    {0x0EF, "d_fg_draw"},
    {0x0F0, "d_fgobj_manager"},
    {0x0F1, "d_shop_fg"},
    {0x0F4, "d_a_nameplate"},
    {0x0F5, "d_a_nameplate_museum"},
    {0x0F6, "d_a_sign_tag"},
    {0x0FA, "d_a_str"},
    {0x0FB, "d_a_str_field"},
    {0x0FC, "d_a_str_field"},
    {0x0FD, "d_a_str_field"},
    {0x0FE, "d_a_str_field"},
    {0x0FF, "d_a_str_field"},
    {0x100, "d_a_str_field"},
    {0x101, "d_a_str_field"},
    {0x102, "d_a_str_field"},
    {0x103, "d_a_str_field"},
    {0x104, "d_a_str_cdbbs"},
    {0x105, "d_a_str_field"},
    {0x106, "d_a_str_field"},
    {0x107, "d_a_str_field"},
    {0x108, "d_a_str_field"},
    {0x109, "d_a_str_field"},
    {0x10A, "d_a_str_field"},
    {0x10B, "d_a_str_field"},
    {0x10C, "d_a_str_common"},
    {0x10D, "d_a_str_common"},
    {0x10E, "d_a_str_town"},
    {0x10F, "d_a_str_town"},
    {0x110, "d_a_str_town"},
    {0x111, "d_a_str_town"},
    {0x112, "d_a_str_town"},
    {0x113, "d_a_str_town"},
    {0x114, "d_a_str_town"},
    {0x115, "d_a_str_town"},
    {0x116, "d_a_str_town"},
    {0x117, "d_a_str_town"},
    {0x118, "d_a_str_town"},
    {0x119, "d_a_str_town"},
    {0x11A, "d_a_str_town"},
    {0x11B, "d_a_str_town"},
    {0x11C, "d_a_str_field"},
    {0x11D, "d_flag"},
    {0x11E, "d_countdown_flag"},
    {0x11F, "d_organ_balloon"},
    {0x120, "d_balloon_manager"},
    {0x121, "d_a_balloon"},
    {0x122, "d_a_ufo"},
    {0x123, "d_str_manager"},
    {0x124, "d_a_bbs_bird"},
    {0x12C, NULL},
};

// 80086C48
BOOL initDMC(int profNum, const DynamicNameTable_c *table, int tableNum, EGG::Heap *heap) {
    cCc_frmHeap = mHeap::createFrmHeap(profNum * 16 + tableNum * sizeof(DynamicModuleControl),
                                       heap, "ダイナミックリンク制御用ヒープ(dDyl::cCc_frmHeap)", 0x20, mHeap::OPT_NONE);
    EGG::Heap *prevHeap = mHeap::setCurrentHeap(cCc_frmHeap);
    DMC_num = profNum;
    DMC = new DynamicModuleControlBase *[profNum];
    DynamicNameTable = table;
    DynamicNameTableNum = tableNum;
    for (int i = 0; i < DynamicNameTableNum; i++) {
        const DynamicNameTable_c *entry = &DynamicNameTable[i];
        if (entry->mName != NULL) {
            for (int j = 0; j < DMC_num; j++) {
                if (DMC[j] != NULL && strcmp(entry->mName, DMC[j]->getModuleName()) == 0) {
                    DMC[entry->mProfName] = DMC[j];
                    break;
                }
            }
            if (DMC[entry->mProfName] == NULL) {
                DMC[entry->mProfName] = new DynamicModuleControl(entry->mName, NULL);
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
    initDMC(PROFILE_NUM, l_dylinkTable, ARRAY_SIZE(l_dylinkTable), mHeap::g_dylinkHeap);
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
