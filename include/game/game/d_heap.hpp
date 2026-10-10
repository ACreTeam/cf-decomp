#pragma once

// d_heap (src/dol/game/d_heap.cpp, .text 800B4DA0..800B6350): the game heaps. Each create function makes one
// heap in parent and keeps it in its dHeap::*_p pointer (names from the heap name strings).

#include <types.h>
#include <lib/egg/core/eggExpHeap.h>
#include <lib/egg/core/eggFrmHeap.h>

namespace dHeap {

extern EGG::FrmHeap *hmnAnmHeap_p; // 人型アニメヒープ
extern EGG::FrmHeap *hmnHeadHeap_p; // 人型頭ヒープ
extern EGG::FrmHeap *hmnPaletteHeap_p; // 人型パレットヒープ
extern EGG::FrmHeap *hmnFaceTexHeap_p; // 人型顔テクスチャヒープ
extern EGG::FrmHeap *hmnBodyHeap_p; // 人型体モデルヒープ
extern EGG::FrmHeap *hmnFaceAnmHeap_p; // 人型顔テクスチャアニメヒープ
extern EGG::FrmHeap *msgHeap_p; // メッセージヒープ
extern EGG::FrmHeap *scriptHeap_p; // 文字列ヒープ
extern EGG::FrmHeap *saveHeap_p; // セーブヒープ
extern EGG::FrmHeap *fontHeap_p; // フォントヒープ
extern EGG::FrmHeap *bannerHeap_p; // セーブバナー
extern EGG::FrmHeap *bgAlwaysHeap_p; // 地形常駐ヒープ
extern EGG::FrmHeap *ftrAlwaysHeap_p; // 家具常駐ヒープ
extern EGG::FrmHeap *itemInfoHeap_p; // アイテム情報ヒープ
extern EGG::FrmHeap *fgDataHeap_p; // FGデータヒープ
extern EGG::FrmHeap *fieldHeap_p; // フィールド情報ヒープ
extern EGG::FrmHeap *fgHeap_p; // ＦＧヒープ
extern EGG::FrmHeap *fgAlwaysHeap_p; // ＦＧ常駐ヒープ
extern EGG::FrmHeap *strHeap_p; // 建物ヒープ
extern EGG::FrmHeap *strResHeap_p; // 建物リソースヒープ
extern EGG::FrmHeap *strColHeap_p; // 建物コリジョンヒープ
extern EGG::FrmHeap *dsnPlttHeap_p; // デザインパレットヒープ
extern EGG::ExpHeap *menuDylHeap_p; // メニュー用DynamicLinkヒープ
extern EGG::FrmHeap *npcModelHeap_p; // NPCモデルヒープ
extern EGG::FrmHeap *npcNmlPackHeap_p; // 通常 NPC常駐設定テーブルヒープ
extern EGG::FrmHeap *UkiHeap_p; // 浮き管理ヒープ
extern EGG::FrmHeap *EnvLightHeap_p; // 環境ライトヒープ
extern EGG::FrmHeap *nandHeap_p; // NANDライブラリ用ヒープ
extern EGG::FrmHeap *ngwordHeap_p; // NGWORD用ヒープ
extern EGG::FrmHeap *prcWorkHeap_p; // PRCライブラリ用ヒープ
extern EGG::FrmHeap *prcHeap_p; // PRCスレッド用ヒープ
extern EGG::ExpHeap *netSoHeap_p; // ソケットライブラリ用ヒープ
extern EGG::ExpHeap *netDwcHeap_p; // DWCライブラリ用ヒープ
extern EGG::ExpHeap *netWc24Heap_p; // NWC24ライブラリ用ヒープ
extern EGG::ExpHeap *localHeap_p; // ステージ固有ヒープ
extern EGG::FrmHeap *soundHeap_p; // サウンドヒープ
extern EGG::FrmHeap *netGameHeap_p; // 通信ゲーム処理用ヒープ
extern EGG::FrmHeap *photoHeap_p; // 画像変換処理用ヒープ
extern EGG::FrmHeap *faceResHeap_p; // 似顔絵リソース用ヒープ
extern EGG::FrmHeap *hbmHeap_p; // ホームボタンメニュー用ヒープ
extern EGG::FrmHeap *blackWaitHeap_p; //  黒画面待ちアイコン用ヒープ
extern EGG::ExpHeap *dvdErrHeap_p; // ＤＶＤエラー表示用ヒープ
extern EGG::ExpHeap *rumbleHeap_p; // 振動マネージャ用ヒープ
extern EGG::ExpHeap *playerHeap_p; // プレイヤー用ヒープ
extern EGG::ExpHeap *hmnToolHeap_p; // 人型道具モデル生成用ヒープ
extern EGG::ExpHeap *effectResHeap_p; // エフェクトリソース用ヒープ
extern EGG::ExpHeap *effectMngHeap_p; // エフェクト管理用ヒープ
extern EGG::ExpHeap *effectCallBackHeap_p; // エフェクトコールバック用ヒープ
extern EGG::ExpHeap *growUpHeap_p; // ゲーム開始時の成長処理用ヒープ
extern EGG::FrmHeap *npcPermitMngHeap_p; // NPC権限管理用ヒープ

void report(const char *func, const char *name, u32 size); // 800B4DA0: empty
EGG::FrmHeap *createHmnAnmHeap(EGG::Heap *parent); // 800B4DA4: 人型アニメヒープ
EGG::FrmHeap *createHmnHeadHeap(EGG::Heap *parent); // 800B4E18: 人型頭ヒープ
EGG::FrmHeap *createHmnPaletteHeap(EGG::Heap *parent); // 800B4E8C: 人型パレットヒープ
EGG::FrmHeap *createHmnFaceTexHeap(EGG::Heap *parent); // 800B4F00: 人型顔テクスチャヒープ
EGG::FrmHeap *createHmnBodyHeap(EGG::Heap *parent); // 800B4F74: 人型体モデルヒープ
EGG::FrmHeap *createHmnFaceAnmHeap(EGG::Heap *parent); // 800B4FE8: 人型顔テクスチャアニメヒープ
EGG::FrmHeap *createMsgHeap(EGG::Heap *parent); // 800B505C: メッセージヒープ
EGG::FrmHeap *createScriptHeap(EGG::Heap *parent); // 800B50C4: 文字列ヒープ
EGG::FrmHeap *createSaveHeap(EGG::Heap *parent); // 800B5138: セーブヒープ
EGG::FrmHeap *createFontHeap(EGG::Heap *parent); // 800B51AC: フォントヒープ
EGG::FrmHeap *createBannerHeap(EGG::Heap *parent); // 800B5220: セーブバナー
EGG::FrmHeap *createBgAlwaysHeap(EGG::Heap *parent); // 800B5294: 地形常駐ヒープ
EGG::FrmHeap *createFtrAlwaysHeap(EGG::Heap *parent); // 800B52FC: 家具常駐ヒープ
EGG::FrmHeap *createItemInfoHeap(EGG::Heap *parent); // 800B5364: アイテム情報ヒープ
EGG::FrmHeap *createFgDataHeap(EGG::Heap *parent); // 800B53CC: FGデータヒープ
EGG::FrmHeap *createFieldHeap(EGG::Heap *parent); // 800B5440: フィールド情報ヒープ
EGG::FrmHeap *createFgHeap(EGG::Heap *parent); // 800B54B4: ＦＧヒープ
EGG::FrmHeap *createFgAlwaysHeap(EGG::Heap *parent); // 800B5528: ＦＧ常駐ヒープ
EGG::FrmHeap *createStrHeap(EGG::Heap *parent); // 800B5590: 建物ヒープ
void destroyStrHeap(); // 800B55F8
EGG::FrmHeap *createStrResHeap(EGG::Heap *parent); // 800B5624: 建物リソースヒープ
EGG::FrmHeap *createStrColHeap(EGG::Heap *parent); // 800B568C: 建物コリジョンヒープ
EGG::FrmHeap *createDsnPlttHeap(EGG::Heap *parent); // 800B56F4: デザインパレットヒープ
EGG::ExpHeap *createMenuDylHeap(EGG::Heap *parent); // 800B5750: メニュー用DynamicLinkヒープ
EGG::FrmHeap *createNpcModelHeap(EGG::Heap *parent); // 800B57C4: NPCモデルヒープ
EGG::FrmHeap *createNpcNmlPackHeap(EGG::Heap *parent); // 800B5838: 通常 NPC常駐設定テーブルヒープ
EGG::FrmHeap *createUkiHeap(EGG::Heap *parent); // 800B58A0: 浮き管理ヒープ
EGG::FrmHeap *createEnvLightHeap(EGG::Heap *parent); // 800B5914: 環境ライトヒープ
EGG::FrmHeap *createNandHeap(EGG::Heap *parent); // 800B5970: NANDライブラリ用ヒープ
EGG::FrmHeap *createNgwordHeap(EGG::Heap *parent); // 800B59E4: NGWORD用ヒープ
EGG::FrmHeap *createPrcWorkHeap(EGG::Heap *parent); // 800B5A58: PRCライブラリ用ヒープ
EGG::FrmHeap *createPrcHeap(EGG::Heap *parent); // 800B5ACC: PRCスレッド用ヒープ
EGG::ExpHeap *createNetSoHeap(EGG::Heap *parent); // 800B5B40: ソケットライブラリ用ヒープ
EGG::ExpHeap *createNetDwcHeap(EGG::Heap *parent); // 800B5BB8: DWCライブラリ用ヒープ
EGG::ExpHeap *createNetWc24Heap(EGG::Heap *parent); // 800B5C30: NWC24ライブラリ用ヒープ
EGG::ExpHeap *createLocalHeap(EGG::Heap *parent); // 800B5CA8: ステージ固有ヒープ
EGG::FrmHeap *createSoundHeap(EGG::Heap *parent); // 800B5D10: サウンドヒープ
EGG::FrmHeap *createNetGameHeap(EGG::Heap *parent); // 800B5D84: 通信ゲーム処理用ヒープ
EGG::FrmHeap *createPhotoHeap(EGG::Heap *parent); // 800B5DF4: 画像変換処理用ヒープ
EGG::FrmHeap *createFaceResHeap(EGG::Heap *parent); // 800B5E50: 似顔絵リソース用ヒープ
EGG::FrmHeap *createHbmHeap(EGG::Heap *parent); // 800B5EAC: ホームボタンメニュー用ヒープ
EGG::FrmHeap *createBlackWaitHeap(EGG::Heap *parent); // 800B5F08:  黒画面待ちアイコン用ヒープ
EGG::ExpHeap *createDvdErrHeap(EGG::Heap *parent); // 800B5F58: ＤＶＤエラー表示用ヒープ
EGG::ExpHeap *createRumbleHeap(EGG::Heap *parent); // 800B5FB4: 振動マネージャ用ヒープ
EGG::ExpHeap *createPlayerHeap(EGG::Heap *parent); // 800B6010: プレイヤー用ヒープ
EGG::ExpHeap *createHmnToolHeap(EGG::Heap *parent); // 800B6088: 人型道具モデル生成用ヒープ
EGG::ExpHeap *createEffectResHeap(EGG::Heap *parent); // 800B6100: エフェクトリソース用ヒープ
EGG::ExpHeap *createEffectMngHeap(EGG::Heap *parent); // 800B6178: エフェクト管理用ヒープ
EGG::ExpHeap *createEffectCallBackHeap(EGG::Heap *parent); // 800B61F0: エフェクトコールバック用ヒープ
EGG::ExpHeap *createGrowUpHeap(EGG::Heap *parent); // 800B6270: ゲーム開始時の成長処理用ヒープ
EGG::FrmHeap *createNpcPermitMngHeap(EGG::Heap *parent); // 800B62DC: NPC権限管理用ヒープ

} // namespace dHeap
