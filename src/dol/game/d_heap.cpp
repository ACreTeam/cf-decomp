// The game heaps. .text 800B4DA0..800B6350. See include/game/game/d_heap.hpp.
#include <game/game/d_heap.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/game/d_banner.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_fg_data.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_font.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_hmn_body_mng.hpp>
#include <game/game/d_hmn_face_anm_mng.hpp>
#include <game/game/d_hmn_face_tex_mng.hpp>
#include <game/game/d_hmn_head_mng.hpp>
#include <game/game/d_hmn_palette_mng.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_ds_download.hpp>
#include <game/game/d_net_wc24.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_mdl_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_prc_mng.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_uki.hpp>
#include <game/snd/snd_system.hpp>

EGG::FrmHeap *dHeap::hmnAnmHeap_p;
EGG::FrmHeap *dHeap::hmnHeadHeap_p;
EGG::FrmHeap *dHeap::hmnPaletteHeap_p;
EGG::FrmHeap *dHeap::hmnFaceTexHeap_p;
EGG::FrmHeap *dHeap::hmnBodyHeap_p;
EGG::FrmHeap *dHeap::hmnFaceAnmHeap_p;
EGG::FrmHeap *dHeap::msgHeap_p;
EGG::FrmHeap *dHeap::scriptHeap_p;
EGG::FrmHeap *dHeap::saveHeap_p;
EGG::FrmHeap *dHeap::fontHeap_p;
EGG::FrmHeap *dHeap::bannerHeap_p;
EGG::FrmHeap *dHeap::bgAlwaysHeap_p;
EGG::FrmHeap *dHeap::ftrAlwaysHeap_p;
EGG::FrmHeap *dHeap::itemInfoHeap_p;
EGG::FrmHeap *dHeap::fgDataHeap_p;
EGG::FrmHeap *dHeap::fieldHeap_p;
EGG::FrmHeap *dHeap::fgHeap_p;
EGG::FrmHeap *dHeap::fgAlwaysHeap_p;
EGG::FrmHeap *dHeap::strHeap_p;
EGG::FrmHeap *dHeap::strResHeap_p;
EGG::FrmHeap *dHeap::strColHeap_p;
EGG::FrmHeap *dHeap::dsnPlttHeap_p;
EGG::ExpHeap *dHeap::menuDylHeap_p;
EGG::FrmHeap *dHeap::npcModelHeap_p;
EGG::FrmHeap *dHeap::npcNmlPackHeap_p;
EGG::FrmHeap *dHeap::UkiHeap_p;
EGG::FrmHeap *dHeap::EnvLightHeap_p;
EGG::FrmHeap *dHeap::nandHeap_p;
EGG::FrmHeap *dHeap::ngwordHeap_p;
EGG::FrmHeap *dHeap::prcWorkHeap_p;
EGG::FrmHeap *dHeap::prcHeap_p;
EGG::ExpHeap *dHeap::netSoHeap_p;
EGG::ExpHeap *dHeap::netDwcHeap_p;
EGG::ExpHeap *dHeap::netWc24Heap_p;
EGG::ExpHeap *dHeap::localHeap_p;
EGG::FrmHeap *dHeap::soundHeap_p;
EGG::FrmHeap *dHeap::netGameHeap_p;
EGG::FrmHeap *dHeap::photoHeap_p;
EGG::FrmHeap *dHeap::faceResHeap_p;
EGG::FrmHeap *dHeap::hbmHeap_p;
EGG::FrmHeap *dHeap::blackWaitHeap_p;
EGG::ExpHeap *dHeap::dvdErrHeap_p;
EGG::ExpHeap *dHeap::rumbleHeap_p;
EGG::ExpHeap *dHeap::playerHeap_p;
EGG::ExpHeap *dHeap::hmnToolHeap_p;
EGG::ExpHeap *dHeap::effectResHeap_p;
EGG::ExpHeap *dHeap::effectMngHeap_p;
EGG::ExpHeap *dHeap::effectCallBackHeap_p;
EGG::ExpHeap *dHeap::growUpHeap_p;
EGG::FrmHeap *dHeap::npcPermitMngHeap_p;

// 800B4DA0: a stripped debug report of a created heap (empty in the release build).
void dHeap::report(const char *func, const char *name, u32 size) {}

// 800B4DA4
EGG::FrmHeap *dHeap::createHmnAnmHeap(EGG::Heap *parent) {
    u32 size = ROUND_UP(dHmnAnm_c::getHeapSize(), 0x20);
    hmnAnmHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::hmnAnmHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "人型アニメヒープ", size);
    return hmnAnmHeap_p;
}

// 800B4E18
EGG::FrmHeap *dHeap::createHmnHeadHeap(EGG::Heap *parent) {
    u32 size = dHmnHeadMng_c::getWorkSize();
    hmnHeadHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::hmnHeadHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "人型頭ヒープ", size);
    return hmnHeadHeap_p;
}

// 800B4E8C
EGG::FrmHeap *dHeap::createHmnPaletteHeap(EGG::Heap *parent) {
    u32 size = dHmnPaletteMng_c::getWorkSize();
    hmnPaletteHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::hmnPaletteHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "人型パレットヒープ", size);
    return hmnPaletteHeap_p;
}

// 800B4F00
EGG::FrmHeap *dHeap::createHmnFaceTexHeap(EGG::Heap *parent) {
    u32 size = dHmnFaceTexMng_c::getWorkSize();
    hmnFaceTexHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::hmnFaceTexHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "人型顔テクスチャヒープ", size);
    return hmnFaceTexHeap_p;
}

// 800B4F74
EGG::FrmHeap *dHeap::createHmnBodyHeap(EGG::Heap *parent) {
    u32 size = dHmnBodyMng_c::getWorkSize();
    hmnBodyHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::hmnBodyHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "人型体モデルヒープ", size);
    return hmnBodyHeap_p;
}

// 800B4FE8
EGG::FrmHeap *dHeap::createHmnFaceAnmHeap(EGG::Heap *parent) {
    u32 size = dHmnFaceAnmMng_c::getWorkSize();
    hmnFaceAnmHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::hmnFaceAnmHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "人型顔テクスチャアニメヒープ", size);
    return hmnFaceAnmHeap_p;
}

// 800B505C
EGG::FrmHeap *dHeap::createMsgHeap(EGG::Heap *parent) {
    u32 size = 0x32000;
    msgHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::msgHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "メッセージヒープ", size);
    return msgHeap_p;
}

// 800B50C4
EGG::FrmHeap *dHeap::createScriptHeap(EGG::Heap *parent) {
    u32 size = dScript::getBankHeapSize();
    scriptHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::scriptHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "文字列ヒープ", size);
    return scriptHeap_p;
}

// 800B5138
EGG::FrmHeap *dHeap::createSaveHeap(EGG::Heap *parent) {
    u32 size = dSaveData_c::getSize();
    saveHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::saveHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "セーブヒープ", size);
    return saveHeap_p;
}

// 800B51AC
EGG::FrmHeap *dHeap::createFontHeap(EGG::Heap *parent) {
    u32 size = fn_800A7678();
    fontHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::fontHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "フォントヒープ", size);
    return fontHeap_p;
}

// 800B5220
EGG::FrmHeap *dHeap::createBannerHeap(EGG::Heap *parent) {
    u32 size = ROUND_UP(fn_80065244(), 0x20);
    bannerHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::bannerHeap_p : セーブバナー", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "セーブバナー", size);
    return bannerHeap_p;
}

// 800B5294
EGG::FrmHeap *dHeap::createBgAlwaysHeap(EGG::Heap *parent) {
    u32 size = 0x9E080;
    bgAlwaysHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::bgAlwaysHeap_p : 地形常駐ヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "地形常駐ヒープ", size);
    return bgAlwaysHeap_p;
}

// 800B52FC
EGG::FrmHeap *dHeap::createFtrAlwaysHeap(EGG::Heap *parent) {
    u32 size = 0x103C0;
    ftrAlwaysHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::ftrAlwaysHeap_p : 家具常駐", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "家具常駐ヒープ", size);
    return ftrAlwaysHeap_p;
}

// 800B5364
EGG::FrmHeap *dHeap::createItemInfoHeap(EGG::Heap *parent) {
    u32 size = 0x12EFC0;
    itemInfoHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::itemInfoHeap_p : アイテムパラメータ情報", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "アイテム情報ヒープ", size);
    return itemInfoHeap_p;
}

// 800B53CC
EGG::FrmHeap *dHeap::createFgDataHeap(EGG::Heap *parent) {
    u32 size = ROUND_UP(dFgData_c::getFileSize(), 0x20);
    fgDataHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::fgDataHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "FGデータヒープ", size);
    return fgDataHeap_p;
}

// 800B5440
EGG::FrmHeap *dHeap::createFieldHeap(EGG::Heap *parent) {
    u32 size = fn_8019112C();
    fieldHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::fieldHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "フィールド情報ヒープ", size);
    return fieldHeap_p;
}

// 800B54B4
EGG::FrmHeap *dHeap::createFgHeap(EGG::Heap *parent) {
    u32 size = ROUND_UP(fgMngProc_getFgHeapSize(), 0x20);
    fgHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::fgHeap_p : FG描画ヒープ (リソース、g3d)", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "ＦＧヒープ", size);
    return fgHeap_p;
}

// 800B5528
EGG::FrmHeap *dHeap::createFgAlwaysHeap(EGG::Heap *parent) {
    u32 size = 0xAEB80;
    fgAlwaysHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::fgAlwaysHeap_p : FG描画常駐リソースヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "ＦＧ常駐ヒープ", size);
    return fgAlwaysHeap_p;
}

// 800B5590
EGG::FrmHeap *dHeap::createStrHeap(EGG::Heap *parent) {
    u32 size = 0x4B000;
    strHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::strHeap_p : 建物ヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "建物ヒープ", size);
    return strHeap_p;
}

// 800B55F8
void dHeap::destroyStrHeap() {
    mHeap::destroyFrmHeap(strHeap_p);
    strHeap_p = NULL;
}

// 800B5624
EGG::FrmHeap *dHeap::createStrResHeap(EGG::Heap *parent) {
    u32 size = 0x2E6680;
    strResHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::strResHeap_p : 建物リソースヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "建物リソースヒープ", size);
    return strResHeap_p;
}

// 800B568C
EGG::FrmHeap *dHeap::createStrColHeap(EGG::Heap *parent) {
    u32 size = 0x1C740;
    strColHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::strColHeap_p : 建物コリジョンヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "建物コリジョンヒープ", size);
    return strColHeap_p;
}

// 800B56F4: the heap name string repeats createStrColHeap's description (建物コリジョンヒープ).
EGG::FrmHeap *dHeap::createDsnPlttHeap(EGG::Heap *parent) {
    u32 size = 0x880;
    dsnPlttHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::dsnPlttHeap_p : 建物コリジョンヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "デザインパレットヒープ", size);
    return dsnPlttHeap_p;
}

// 800B5750
EGG::ExpHeap *dHeap::createMenuDylHeap(EGG::Heap *parent) {
    u32 size = 0x32000;
    menuDylHeap_p = EGG::ExpHeap::create(size, parent, 0);
    menuDylHeap_p->setName("dHeap::menuDylHeap_p");
    menuDylHeap_p->setAllocMode(0);
    report(__FUNCTION__, "メニュー用DynamicLinkヒープ", size);
    return menuDylHeap_p;
}

// 800B57C4
EGG::FrmHeap *dHeap::createNpcModelHeap(EGG::Heap *parent) {
    u32 size = fn_800F9D5C();
    npcModelHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::npcModelHeap_p : NPCモデルヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "NPCモデルヒープ", size);
    return npcModelHeap_p;
}

// 800B5838
EGG::FrmHeap *dHeap::createNpcNmlPackHeap(EGG::Heap *parent) {
    u32 size = 0x14EE0;
    npcNmlPackHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::npcNmlPackHeap_p : 通常 NPC常駐設定テーブルヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "通常 NPC常駐設定テーブルヒープ", size);
    return npcNmlPackHeap_p;
}

// 800B58A0
EGG::FrmHeap *dHeap::createUkiHeap(EGG::Heap *parent) {
    u32 size = fn_801710DC();
    UkiHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::UkiHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "浮き管理ヒープ", size);
    return UkiHeap_p;
}

// 800B5914
EGG::FrmHeap *dHeap::createEnvLightHeap(EGG::Heap *parent) {
    u32 size = 0x800;
    EnvLightHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::EnvLightHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "環境ライトヒープ", size);
    return EnvLightHeap_p;
}

// 800B5970
EGG::FrmHeap *dHeap::createNandHeap(EGG::Heap *parent) {
    u32 size = fn_800D2840();
    nandHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::nandHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "NANDライブラリ用ヒープ", size);
    return nandHeap_p;
}

// 800B59E4
EGG::FrmHeap *dHeap::createNgwordHeap(EGG::Heap *parent) {
    u32 size = dPrcMng_c::getNgwordHeapSize();
    ngwordHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::ngwordHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "NGWORD用ヒープ", size);
    return ngwordHeap_p;
}

// 800B5A58
EGG::FrmHeap *dHeap::createPrcWorkHeap(EGG::Heap *parent) {
    u32 size = dPrcMng_c::getWorkHeapSize();
    prcWorkHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::prcWorkHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "PRCライブラリ用ヒープ", size);
    return prcWorkHeap_p;
}

// 800B5ACC
EGG::FrmHeap *dHeap::createPrcHeap(EGG::Heap *parent) {
    u32 size = dPrcMng_c::getStackSize();
    prcHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::prcHeap_p", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "PRCスレッド用ヒープ", size);
    return prcHeap_p;
}

// 800B5B40
EGG::ExpHeap *dHeap::createNetSoHeap(EGG::Heap *parent) {
    u32 size = fn_800E4888();
    netSoHeap_p = EGG::ExpHeap::create(size, parent, 0);
    netSoHeap_p->setName("ソケットライブラリ用ヒープ");
    netSoHeap_p->setAllocMode(0);
    report(__FUNCTION__, "ソケットライブラリ用ヒープ", size);
    return netSoHeap_p;
}

// 800B5BB8
EGG::ExpHeap *dHeap::createNetDwcHeap(EGG::Heap *parent) {
    u32 size = fn_800E4894();
    netDwcHeap_p = EGG::ExpHeap::create(size, parent, 0);
    netDwcHeap_p->setName("DWCライブラリ用ヒープ");
    netDwcHeap_p->setAllocMode(0);
    report(__FUNCTION__, "DWCライブラリ用ヒープ", size);
    return netDwcHeap_p;
}

// 800B5C30
EGG::ExpHeap *dHeap::createNetWc24Heap(EGG::Heap *parent) {
    u32 size = fn_800E94A8();
    netWc24Heap_p = EGG::ExpHeap::create(size, parent, 0);
    netWc24Heap_p->setName("NWC24ライブラリ用ヒープ");
    netWc24Heap_p->setAllocMode(0);
    report(__FUNCTION__, "NWC24ライブラリ用ヒープ", size);
    return netWc24Heap_p;
}

// 800B5CA8
EGG::ExpHeap *dHeap::createLocalHeap(EGG::Heap *parent) {
    u32 size = 0x4C0000;
    localHeap_p = EGG::ExpHeap::create(size, parent, 4);
    localHeap_p->setName("dHeap::localHeap_p");
    localHeap_p->setAllocMode(0);
    report(__FUNCTION__, "ステージ固有ヒープ", size);
    return localHeap_p;
}

// 800B5D10
EGG::FrmHeap *dHeap::createSoundHeap(EGG::Heap *parent) {
    u32 size = fn_801CE584();
    soundHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::soundHeap_p : サウンドヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "サウンドヒープ", size);
    return soundHeap_p;
}

// 800B5D84
EGG::FrmHeap *dHeap::createNetGameHeap(EGG::Heap *parent) {
    u32 size = mHeap::frmHeapCost(fn_800DCB68(), 4);
    netGameHeap_p = EGG::FrmHeap::create(size, parent, 0);
    netGameHeap_p->setName("通信ゲーム処理用ヒープ");
    report(__FUNCTION__, "通信ゲーム処理用ヒープ", size);
    return netGameHeap_p;
}

// 800B5DF4
EGG::FrmHeap *dHeap::createPhotoHeap(EGG::Heap *parent) {
    u32 size = 0x17B4A0;
    photoHeap_p = EGG::FrmHeap::create(size, parent, 0);
    photoHeap_p->setName("画像変換処理用ヒープ");
    report(__FUNCTION__, "画像変換処理用ヒープ", size);
    return photoHeap_p;
}

// 800B5E50
EGG::FrmHeap *dHeap::createFaceResHeap(EGG::Heap *parent) {
    u32 size = 0xF50C0;
    faceResHeap_p = EGG::FrmHeap::create(size, parent, 0);
    faceResHeap_p->setName("似顔絵リソース用ヒープ");
    report(__FUNCTION__, "似顔絵リソース用ヒープ", size);
    return faceResHeap_p;
}

// 800B5EAC
EGG::FrmHeap *dHeap::createHbmHeap(EGG::Heap *parent) {
    u32 size = 0x16D0D8;
    hbmHeap_p = EGG::FrmHeap::create(size, parent, 0);
    hbmHeap_p->setName("ホームボタンメニュー用ヒープ");
    report(__FUNCTION__, "ホームボタンメニュー用ヒープ", size);
    return hbmHeap_p;
}

// 800B5F08
EGG::FrmHeap *dHeap::createBlackWaitHeap(EGG::Heap *parent) {
    u32 size = 0x5B18;
    blackWaitHeap_p = EGG::FrmHeap::create(size, parent, 0);
    blackWaitHeap_p->setName(" 黒画面待ちアイコン用ヒープ");
    report(__FUNCTION__, " 黒画面待ちアイコン用ヒープ", size);
    return blackWaitHeap_p;
}

// 800B5F58
EGG::ExpHeap *dHeap::createDvdErrHeap(EGG::Heap *parent) {
    u32 size = 0x19000;
    dvdErrHeap_p = EGG::ExpHeap::create(size, parent, 0);
    dvdErrHeap_p->setName("ＤＶＤエラー表示用ヒープ");
    report(__FUNCTION__, "ＤＶＤエラー表示用ヒープ", size);
    return dvdErrHeap_p;
}

// 800B5FB4
EGG::ExpHeap *dHeap::createRumbleHeap(EGG::Heap *parent) {
    u32 size = 0x4B000;
    rumbleHeap_p = EGG::ExpHeap::create(size, parent, 0);
    rumbleHeap_p->setName("振動マネージャ用ヒープ");
    report(__FUNCTION__, "振動マネージャ用ヒープ", size);
    return rumbleHeap_p;
}

// 800B6010
EGG::ExpHeap *dHeap::createPlayerHeap(EGG::Heap *parent) {
    u32 size = fn_801015F4();
    playerHeap_p = EGG::ExpHeap::create(size, parent, 0);
    playerHeap_p->setName("プレイヤー用ヒープ");
    playerHeap_p->setAllocMode(0);
    report(__FUNCTION__, "プレイヤー用ヒープ", size);
    return playerHeap_p;
}

// 800B6088
EGG::ExpHeap *dHeap::createHmnToolHeap(EGG::Heap *parent) {
    u32 size = dHmnToolMng_c::getHeapSize();
    hmnToolHeap_p = EGG::ExpHeap::create(size, parent, 0);
    hmnToolHeap_p->setName("人型道具モデル生成用ヒープ");
    hmnToolHeap_p->setAllocMode(0);
    report(__FUNCTION__, "人型道具モデル生成用ヒープ", size);
    return hmnToolHeap_p;
}

// 800B6100
EGG::ExpHeap *dHeap::createEffectResHeap(EGG::Heap *parent) {
    u32 size = fn_800887DC();
    effectResHeap_p = EGG::ExpHeap::create(size, parent, 0);
    effectResHeap_p->setName("エフェクトリソース用ヒープ");
    effectResHeap_p->setAllocMode(0);
    report(__FUNCTION__, "エフェクトリソース用ヒープ", size);
    return effectResHeap_p;
}

// 800B6178
EGG::ExpHeap *dHeap::createEffectMngHeap(EGG::Heap *parent) {
    u32 size = fn_800887E8();
    effectMngHeap_p = EGG::ExpHeap::create(size, parent, 0);
    effectMngHeap_p->setName("エフェクト管理用ヒープ");
    effectMngHeap_p->setAllocMode(0);
    report(__FUNCTION__, "エフェクト管理用ヒープ", size);
    return effectMngHeap_p;
}

// 800B61F0: the heap is named like createEffectMngHeap's (エフェクト管理用ヒープ) in the original.
EGG::ExpHeap *dHeap::createEffectCallBackHeap(EGG::Heap *parent) {
    u32 size = fn_800887F4();
    effectCallBackHeap_p = EGG::ExpHeap::create(size, parent, 0);
    effectCallBackHeap_p->setName("エフェクト管理用ヒープ");
    effectCallBackHeap_p->setAllocMode(0);
    report(__FUNCTION__, "エフェクトコールバック用ヒープ", size);
    return effectCallBackHeap_p;
}

// 800B6270
EGG::ExpHeap *dHeap::createGrowUpHeap(EGG::Heap *parent) {
    u32 size = fgMngProc_getGrowUpHeapSize();
    growUpHeap_p = EGG::ExpHeap::create(size, parent, 0);
    growUpHeap_p->setAllocMode(0);
    report(__FUNCTION__, "ゲーム開始時の成長処理用ヒープ", size);
    return growUpHeap_p;
}

// 800B62DC
EGG::FrmHeap *dHeap::createNpcPermitMngHeap(EGG::Heap *parent) {
    u32 size = fn_800F5370();
    npcPermitMngHeap_p = mHeap::createFrmHeap(size, parent, "dHeap::npcPermitMngHeap_p : NPC権限管理用ヒープ", 0x20, mHeap::OPT_NONE);
    report(__FUNCTION__, "NPC権限管理用ヒープ", size);
    return npcPermitMngHeap_p;
}

#if MUST_MATCH
#include <lib/nw4r/snd/snd_SoundThread.h>

namespace dHeap {
// Fake, matching-only stand-in for an unknown stripped function. This emits the SoundFrameCallback
// vtable and RTTI name into the live string pool. The earlier dead weak copy in d_bgm.cpp makes the
// linker discard this vtable's relocations, leaving the target's 0x14 zero bytes followed by its name.
// This function is also stripped. See notes/d_heap.txt.
void fakeSoundFrameCallbackForHeapTail() {
    nw4r::snd::detail::SoundThread::SoundFrameCallback callback;
}
} // namespace dHeap
#endif
