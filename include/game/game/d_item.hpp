#pragma once

#include <types.h>
#include <cstring>
#include <game/game/d_dvd.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_string.hpp>
#include <lib/nw4r/ut/ut_list.h>
#include <nw4r/g3d/res/g3d_resfile.h>

// Item definition tables loaded from /Item/item.arc.
// RTTI names place these under the dItem namespace.
namespace dItem {

enum {
    // Entries stored in item/item.bin; lower item indices address the table.
    ITEM_COUNT = 0xA15,
    // Indices [DL_ITEM_FIRST, DL_ITEM_END) resolve through the save's
    // downloadable-item blocks instead of item.bin.
    DL_ITEM_FIRST = 0xA15,
    DL_ITEM_END = 0xB15,
    DL_ITEM_COUNT = DL_ITEM_END - DL_ITEM_FIRST,

    BASE_ID_COUNT = 0x1000,

    BITM_VERSION = 0x1701,

    // Item IDs: categories 9..12 (bits 12-15) encode 0x9000 + (baseId << 2).
    // ITEM_ID_NONE is declared with Item in d_fg_item.hpp.
    INDEX_NONE = 0xFFFF,
};

enum Language {
    LANG_JP,
    LANG_US,
    LANG_MX,
    LANG_QC,
    LANG_EN,
    LANG_ES,
    LANG_FR,
    LANG_IT,
    LANG_DE,
    LANG_KR,
};

// Item kinds (BITM::m_kind). Names follow the game's resource-name table
// (804E9C80, read by getKindName).
enum Kind {
    KIND_MONEY, // 0x00
    KIND_WALL,
    KIND_CARPET,
    KIND_FTR,
    KIND_CLOTH,
    KIND_CAP,
    KIND_ACC,
    KIND_INSECT,
    KIND_FISH,
    KIND_PAPER,
    KIND_BEFORE_FOSSIL,
    KIND_FOSSIL,
    KIND_HANIWA,
    KIND_PICTURE,
    KIND_MUSIC,
    KIND_FRUIT,
    KIND_SEED,
    KIND_FAKE_PICTURE_BEFORE,
    KIND_FAKE_PICTURE_AFTER,
    KIND_ORG_CLOTH,
    KIND_ORG_UMB,
    KIND_ORG_CAP,
    KIND_ORG_WC,
    KIND_ORG_EASEL,
    KIND_TA_CLOTH,
    KIND_SEEDLING,
    KIND_KABU,
    KIND_BAD_KABU,
    KIND_RKABU,
    KIND_RKABU_SEED,
    KIND_PAINT,
    KIND_SHELL,
    KIND_SOLD_OUT,
    KIND_BOTTLE_BEFORE,
    KIND_BOTTLE_AFTER,
    KIND_PITFALL_SEED,
    KIND_PBOX,
    KIND_HAND_LETTER,
    KIND_PAPER_BAG,
    KIND_MEDICINE,
    KIND_DUST,
    KIND_NONE, // 0x29
    KIND_DUMMY,
    KIND_EGG_FAKE_BEFORE,
    KIND_EGG_FAKE_AFTER,
    KIND_EGG_BINGO_BEFORE,
    KIND_EGG_BINGO_AFTER,
    KIND_KNIFE_AND_FORK,
    KIND_CANDY,
    KIND_HANABI,
    KIND_CHOCOLATE,
    KIND_CRACKER,
    KIND_CATALOG_ONLY,
    KIND_KEY,
    KIND_MUSHROOM,
    KIND_TIMER,
    KIND_UMBRELLA,
    KIND_FLOWER,
    KIND_FISHINGROD,
    KIND_SILVER_FISHINGROD,
    KIND_GOLD_FISHINGROD,
    KIND_SCOOP,
    KIND_SILVER_SCOOP,
    KIND_GOLD_SCOOP,
    KIND_AXE,
    KIND_SILVER_AXE,
    KIND_GOLD_AXE,
    KIND_WATERING,
    KIND_SILVER_WATERING,
    KIND_GOLD_WATERING,
    KIND_NET,
    KIND_SILVER_NET,
    KIND_GOLD_NET,
    KIND_PACHINKO,
    KIND_GOLD_PACHINKO,
    KIND_SILVER_PACHINKO,
    KIND_BALLOON,
    KIND_SYABON,
    KIND_WINDMILL,
    KIND_CREDIT_CARD,
    KIND_LAMP,
    KIND_UFO_PARTS,
    KIND_DSN_DATA_PLAYER,
    KIND_DSN_DATA_TA,
    KIND_DSN_DATA_FLAG,
    KIND_DSN_DATA_SEIICHI,
    KIND_MUSH_FTR,

    KIND_COUNT, // 0x57
};

// Furniture footprint (BITM::m_ftrSize).
enum FtrSize {
    FTR_SIZE_1x1, // 0x00
    FTR_SIZE_1x2,
    FTR_SIZE_2x2,

    FTR_SIZE_COUNT, // 0x03
};

enum FtrFunc {
    FTR_FUNC_NONE = 0,                        // None / なし
    FTR_FUNC_SOFA,                            // Sofa / ソファ
    FTR_FUNC_DRAWER_STORAGE,                  // Drawer storage / 引き出し型収納
    FTR_FUNC_CABINET_STORAGE,                 // Storage with doors / 扉型収納
    FTR_FUNC_WHOOPEE_CUSHION,                  // Whoopee cushion / ブーブークッション
    FTR_FUNC_LIGHT_SWITCH,                    // Light switch / 電気スイッチ
    FTR_FUNC_FLUORESCENT_LIGHT,               // Fluorescent light / 蛍光灯
    FTR_FUNC_CANDLE,                          // Candle / ろうそく
    FTR_FUNC_CHAIR,                           // Chair / イス
    FTR_FUNC_BED,                             // Bed / ベッド
    FTR_FUNC_DOGHOUSE,                        // Doghouse / いぬごや
    FTR_FUNC_MASTER_SWORD,                    // Master Sword / マスターソード
    FTR_FUNC_ARWING,                          // Arwing / アーウィン
    FTR_FUNC_TRIFORCE,                        // Triforce / トライフォース
    FTR_FUNC_BROWN_DRUM,                      // Brown drum / ちゃいろのドラムかん
    FTR_FUNC_GREEN_DRUM,                      // Green drum / みどりのドラムかん
    FTR_FUNC_RED_DRUM,                        // Red drum / あかいドラムかん
    FTR_FUNC_HAZARDOUS_DRUM,                  // Hazardous drum / キケンなドラムかん
    FTR_FUNC_YELLOW_DRUM,                     // Yellow drum / きいろいドラムかん
    FTR_FUNC_URINAL,                          // Urinal / だんしようトイレ
    FTR_FUNC_PIKMIN,                          // Pikmin / ピクミン
    FTR_FUNC_ROLY_POLY_TOY,                   // Roly-poly toy / おきあがりこぼし
    FTR_FUNC_PIGGY_BANK,                      // Piggy bank / ちょきんばこ
    FTR_FUNC_GONG,                            // Gong / ゴング
    FTR_FUNC_COIN,                            // Coin / コイン
    FTR_FUNC_PIPE,                            // Pipe / どかん
    FTR_FUNC_FIRE_FLOWER,                     // Fire Flower / ファイアフラワー
    FTR_FUNC_FLAG,                            // Flag / はた
    FTR_FUNC_QUESTION_BLOCK,                  // Question Block / ハテナブロック
    FTR_FUNC_MUSHROOM,                        // Mushroom / キノコ
    FTR_FUNC_KOOPA_TROOPA_SHELL,               // Koopa Troopa shell / ノコノコのこうら
    FTR_FUNC_BULLET_BILL_CANNON,               // Bullet Bill cannon / キラーほうだい
    FTR_FUNC_PINBALL_MACHINE,                 // Pinball machine / ピンボールだい
    FTR_FUNC_PUNCHING_BAG,                    // Punching bag / サンドバッグ
    FTR_FUNC_BOWLING_PIN,                     // Bowling pin / ボウリングのピン
    FTR_FUNC_BEAR_POLE,                       // Bear pole / トーテムポール・アラ
    FTR_FUNC_EAGLE_POLE,                      // Eagle pole / トーテムポール・オヤ
    FTR_FUNC_FROG_WOMAN_POLE,                 // Frog woman pole / トーテムポール・サテ
    FTR_FUNC_RAVEN_POLE,                      // Raven pole / トーテムポール・マア
    FTR_FUNC_GLOBE,                           // Globe / ちきゅうぎ
    FTR_FUNC_1UP_MUSHROOM,                    // 1-Up Mushroom / １ＵＰキノコ
    FTR_FUNC_SPEED_BAG,                       // Speed bag / スピードバッグ
    FTR_FUNC_GAS_PUMP,                        // Gas pump / きゅうゆき
    FTR_FUNC_WHEELBARROW,                     // Wheelbarrow / ておしぐるま
    FTR_FUNC_WESTERN_TOILET,                  // Western-style toilet / イス型トイレ
    FTR_FUNC_MASSAGE_CHAIR,                   // Massage chair / マッサージいす
    FTR_FUNC_LAB_CHAIR,                       // Lab chair / じっけんイス
    FTR_FUNC_LAB_BENCH,                       // Lab bench / じっけんだい
    FTR_FUNC_FLYING_SAUCER,                   // Flying saucer / そらとぶえんばん
    FTR_FUNC_BARBECUE_GRILL,                  // Barbecue grill / バーベキューグリル
    FTR_FUNC_HAMSTER_CAGE,                    // Hamster cage / ハムスターのかご
    FTR_FUNC_TORCH,                           // Torch / たいまつ
    FTR_FUNC_BIRDCAGE,                        // Birdcage / とりかご
    FTR_FUNC_BONFIRE,                         // Bonfire / たきび
    FTR_FUNC_CAMPFIRE,                        // Campfire / キャンプファイア
    FTR_FUNC_SUNKEN_HEARTH,                   // Sunken hearth / いろり
    FTR_FUNC_POTBELLY_STOVE,                  // Potbelly stove / ダルマストーブ
    FTR_FUNC_SPACE_HEATER,                    // Space heater / ストーブ
    FTR_FUNC_FIREPLACE,                       // Fireplace / だんろ
    FTR_FUNC_POOL,                            // Pool / プール
    FTR_FUNC_FIRE_BAR,                        // Fire Bar / ファイアバー
    FTR_FUNC_OLD_SEWING_MACHINE,              // Old sewing machine / ふるいミシン
    FTR_FUNC_STAR,                            // Star / スター
    FTR_FUNC_MERRY_GO_ROUND,                  // Merry-go-round / メリーゴーランド
    FTR_FUNC_METROID,                         // Metroid / メトロイド
    FTR_FUNC_UPRIGHT_ARCADE_MACHINE,          // Upright arcade machine / アップライトゲームき
    FTR_FUNC_LAWN_MOWER,                      // Lawn mower / しばかりき
    FTR_FUNC_MERLION,                         // Merlion / マーライオン
    FTR_FUNC_MODEL_RAILWAY,                   // Model railway / てつどうもけい
    FTR_FUNC_FAN,                             // Fan / せんぷうき
    FTR_FUNC_SPRINKLER,                       // Sprinkler / スプリンクラー
    FTR_FUNC_TWIN_TUB_WASHER,                 // Twin-tub washing machine / にそうしきせんたくき
    FTR_FUNC_AUTOMATIC_WASHER,                // Fully automatic washing machine / ぜんじどうせんたくき
    FTR_FUNC_BLENDER,                         // Blender / ミキサー
    FTR_FUNC_OUTDOOR_BATH,                    // Outdoor bath / ろてんぶろ
    FTR_FUNC_AMAZING_MACHINE,                 // Amazing machine / すごそうなキカイ
    FTR_FUNC_PEEING_BOY_STATUE,               // Peeing boy statue / しょうべんこぞう
    FTR_FUNC_COOKTOP,                         // Cooktop / クッキングヒーター
    FTR_FUNC_KITCHEN_SINK,                    // Kitchen sink / キッチンのシンク
    FTR_FUNC_DRUM_WASHER,                     // Drum-type washing machine / ドラムしきせんたくき
    FTR_FUNC_MICROWAVE,                       // Microwave / でんしレンジ
    FTR_FUNC_HONEYBEE,                        // Honeybee / ミツバチ
    FTR_FUNC_HORNET,                          // Hornet / スズメバチ
    FTR_FUNC_BROWN_CICADA,                    // Brown cicada / アブラゼミ
    FTR_FUNC_ROBUST_CICADA,                   // Robust cicada / ミンミンゼミ
    FTR_FUNC_EVENING_CICADA,                  // Evening cicada / ヒグラシ
    FTR_FUNC_MOSQUITO,                        // Mosquito / カ
    FTR_FUNC_FLY,                             // Fly / ハエ
    FTR_FUNC_REFRIGERATOR,                    // Refrigerator / れいぞうこ
    FTR_FUNC_TREASURE_CHEST,                  // Treasure chest / たからばこ
    FTR_FUNC_MUMMY_COFFIN,                    // Mummy's coffin / ミイラのひつぎ
    FTR_FUNC_SAFE,                            // Safe / きんこ
    FTR_FUNC_VICTORY_DARUMA,                  // Victory daruma / ひっしょうダルマ
    FTR_FUNC_CLOTHING_CASE,                   // Clothing case / ファッションケース
    FTR_FUNC_DARUMA,                          // Daruma / ダルマ
    FTR_FUNC_MINI_DARUMA,                     // Mini daruma / ミニダルマ
    FTR_FUNC_NINJA_SWORD,                     // Ninja sword / にんじゃとう
    FTR_FUNC_COMPUTER,                        // Computer / パソコン
    FTR_FUNC_WOODEN_CHAIR,                    // Wooden chair / 木のイス
    FTR_FUNC_STANDARD_CLOCK,                 // Standard clock / 通常時計
    FTR_FUNC_PENDULUM_CLOCK,                 // Pendulum clock / 振り子時計
    FTR_FUNC_CUCKOO_CLOCK,                   // Cuckoo clock / 鳩時計
    FTR_FUNC_CHIMING_CLOCK,                  // Chiming clock / ボンボン時計
    FTR_FUNC_TV,                             // Television / テレビ
    FTR_FUNC_TV_VCR_COMBO,                    // TV/VCR combo / テレビデオ
    FTR_FUNC_F_ZERO,                         // F-Zero / エフゼロ
    FTR_FUNC_JACK_IN_THE_BOX,                 // Jack-in-the-box / ビックリばこ
    FTR_FUNC_VINYL_RECORD_PLAYER,             // Vinyl record player / アナログレコードプレイヤー
    FTR_FUNC_MUSIC_BOX,                       // Music box / オルゴール
    FTR_FUNC_DEER_SCARE,                      // Deer scare / ししおどし
    FTR_FUNC_DRINKING_BIRD,                   // Drinking bird / みずのみドリ
    FTR_FUNC_CLACKER_TOY,                     // Clacker toy / アメリカンクラッカー
    FTR_FUNC_TRASH_CAN,                       // Trash can / ごみばこ
    FTR_FUNC_MATRYOSHKA,                      // Matryoshka nesting doll / マトリョーシカ
    FTR_FUNC_TOASTER,                         // Toaster / トースター
    FTR_FUNC_CANDY_MACHINE,                   // Candy machine / キャンディマシン
    FTR_FUNC_CASH_REGISTER,                   // Cash register / レジスター
    FTR_FUNC_UNKNOWN_MACHINE,                // Unknown machine / よくわからないキカイ
    FTR_FUNC_VISION_TESTER,                   // Vision tester / しりょくそくていき
    FTR_FUNC_CRADLE,                          // Cradle / ゆりかご
    FTR_FUNC_BOWLING_BALL_RETURN,             // Bowling ball return / ボウリングリターン
    FTR_FUNC_METRONOME,                       // Metronome / メトロノーム
    FTR_FUNC_WHEAT_FIELD,                     // Wheat field / むぎばたけ
    FTR_FUNC_GRAMOPHONE,                      // Gramophone / ちくおんき
    FTR_FUNC_TAPE_RECORDER,                   // Tape recorder / テープレコーダー
    FTR_FUNC_RETRO_STEREO,                    // Retro stereo / レトロなステレオ
    FTR_FUNC_DICE_STEREO,                     // Dice stereo / サイコロコンポ
    FTR_FUNC_RADIO_CASSETTE_PLAYER,           // Radio cassette player / ラジカセ
    FTR_FUNC_CD_RADIO_CASSETTE_PLAYER,        // CD radio cassette player / CDラジカセ
    FTR_FUNC_DUAL_CASSETTE_BOOMBOX,           // Dual-cassette boombox / ダブルラジカセ
    FTR_FUNC_ROBO_STEREO,                     // Robo stereo / ロボコンポ
    FTR_FUNC_BABY_CARRIAGE,                   // Baby carriage / うばぐるま
    FTR_FUNC_ROBO_DRESSER,                    // Robo dresser / ロボタンス
    FTR_FUNC_LOCKER,                          // Locker / ロッカー
    FTR_FUNC_REGAL_STYLE_BED,                 // Regal-style bed / ロイヤル系ベッド
    FTR_FUNC_LONG_LOCUST,                     // Long locust / ショウリョウバッタ
    FTR_FUNC_MOLE_CRICKET,                    // Mole cricket / オケラ
    FTR_FUNC_WALKER_CICADA,                   // Walker cicada / ツクツクホウシ
    FTR_FUNC_WHEELED_FURNITURE,               // Furniture with casters / ローラー付き家具
    FTR_FUNC_BATHTUB,                         // Bathtub / ゆぶね
    FTR_FUNC_ROCKET,                          // Rocket / ロケット
    FTR_FUNC_MOUTH_OF_TRUTH,                  // Mouth of Truth / しんじつのくち
    FTR_FUNC_SHAVED_ICE_MAKER,                // Shaved ice maker / かきごおりき
    FTR_FUNC_YOSHI_EGG,                       // Yoshi's egg / ヨッシーのタマゴ
    FTR_FUNC_BIRTHDAY_CAKE,                   // Birthday cake / バースデーケーキ
    FTR_FUNC_FORTUNE_TELLING_PHONE,           // Fortune-telling phone / うらないテレフォン
    FTR_FUNC_SNOW_GLOBE,                      // Snow globe / スノードーム
    FTR_FUNC_WHITE_KATANA,                    // White katana / しろいにほんとう
    FTR_FUNC_PAPIER_MACHE_TIGER,               // Papier-mache tiger / はりこのとら
    FTR_FUNC_HIBACHI,                         // Hibachi / ひばち
    FTR_FUNC_MOUNTAIN_BIKE,                   // Mountain bike / マウンテンバイク
    FTR_FUNC_COOLER_BAG,                      // Cooler bag / クーラーバック

    FTR_FUNC_NUM
};

// Fashion style of clothing (BITM::m_fashion). Names and values follow sys_STRING/STR_Fashion,
// whose entry N is style N (setFashionName); 0 is none.
enum Fashion {
    FASHION_NONE, // 0x00
    FASHION_EXPLORER,        // 0x01 explorer
    FASHION_PIRATE,          // 0x02 pirate
    FASHION_FIREFIGHTER,     // 0x03 firefighter
    FASHION_ASTRONAUT,       // 0x04 astronaut
    FASHION_NINJA,           // 0x05 ninja
    FASHION_REGAL,           // 0x06 regal
    FASHION_WRESTLER,        // 0x07 wrestler
    FASHION_MAGICAL,         // 0x08 magical
    FASHION_GRADUATE,        // 0x09 graduate
    FASHION_POLICE,          // 0x0A police
    FASHION_RACER,           // 0x0B racer
    FASHION_WESTERN,         // 0x0C western
    FASHION_JESTER,          // 0x0D jester
    FASHION_REGGAE,          // 0x0E reggae
    FASHION_ZEN,             // 0x0F zen
    FASHION_BUNNY,           // 0x10 bunny
    FASHION_MUMMY,           // 0x11 mummy
    FASHION_BRIDAL,          // 0x12 bridal
    FASHION_ALPINE,          // 0x13 Alpine
    FASHION_NORDIC,          // 0x14 Nordic
    FASHION_ORNATE,          // 0x15 ornate
    FASHION_CAPTAIN,         // 0x16 captain
    FASHION_SAILOR,          // 0x17 sailor
    FASHION_COMBAT,          // 0x18 combat
    FASHION_CAVALIER,        // 0x19 cavalier
    FASHION_DOCTOR,          // 0x1A doctor
    FASHION_MASQUERADE,      // 0x1B masquerade
    FASHION_KNIGHT,          // 0x1C knight
    FASHION_WARRIOR,         // 0x1D warrior
    FASHION_VIKING,          // 0x1E Viking
    FASHION_CHEF,            // 0x1F chef
    FASHION_LABORER,         // 0x20 laborer
    FASHION_COOK,            // 0x21 cook
    FASHION_KAPPN,           // 0x22 Kapp'n
    FASHION_DETECTIVE,       // 0x23 detective
    FASHION_CYCLIST,         // 0x24 cyclist
    FASHION_PERSIAN,         // 0x25 Persian
    FASHION_NATIVE,          // 0x26 native
    FASHION_EGYPTIAN,        // 0x27 Egyptian
    FASHION_RED_HERO,        // 0x28 red hero
    FASHION_SAMURAI,         // 0x29 samurai
    FASHION_FROG,            // 0x2A frog
    FASHION_BEAR,            // 0x2B bear
    FASHION_CAT,             // 0x2C cat
    FASHION_SPA,             // 0x2D spa
    FASHION_TANGERINE,       // 0x2E tangerine
    FASHION_KIWI,            // 0x2F kiwi
    FASHION_WATERMELON,      // 0x30 watermelon
    FASHION_STRAWBERRY,      // 0x31 strawberry
    FASHION_GRAPE,           // 0x32 grape
    FASHION_MELON,           // 0x33 melon
    FASHION_LINK,            // 0x34 Link
    FASHION_SAMUS,           // 0x35 Samus
    FASHION_GREEN_HERO,      // 0x36 green hero
    FASHION_BLUE_HERO,       // 0x37 blue hero
    FASHION_HEROINE,         // 0x38 heroine

    FASHION_COUNT, // 0x39
};

// Furniture part A (BITM::m_ftrPartA); 0 is none (getCategoryQ5).
enum FtrPartA {
    FTR_PART_A_NONE, // 0x00

    FTR_PART_A_COUNT = 6,
};

// Furniture lamp type (BITM::m_ftrLamp); 0 is none (getCategoryQ5).
enum FtrLamp {
    FTR_LAMP_NONE, // 0x00

    FTR_LAMP_COUNT = 3,
};

// Fossil set (BITM::m_fossil); 0 is none.
enum Fossil {
    FOSSIL_NONE, // 0x00

    FOSSIL_COUNT = 0x1C,
};

// Furniture series (BITM::m_series), 0..SERIES_COUNT-1; series.bin entry N is series N.
// English names come from sys_STRING/STR_Furniture (entry N + 1, setFurnitureName) where it has
// one. The game has no series name for the set series (0x2B-0x5F), so those are named after the
// US names of their items in item.bin (BITM::m_nameUs, listed in the comment). The reserved slots
// are translated from series.bin. The comment gives series.bin's group (getSeriesGroup) and
// Japanese name.
enum SeriesId {
    SERIES_EXOTIC,               // 0x00 basic: Exotic / アジア
    SERIES_LOVELY,               // 0x01 basic: Lovely / ラブリー
    SERIES_CLASSIC,              // 0x02 basic: Classic / シック
    SERIES_RANCH,                // 0x03 basic: Ranch / カントリー
    SERIES_CABANA,               // 0x04 basic: Cabana / リゾート
    SERIES_BLUE,                 // 0x05 basic: Blue / あおいろ
    SERIES_MODERN,               // 0x06 basic: Modern / モノクロ
    SERIES_REGAL,                // 0x07 basic: Regal / ロイヤル
    SERIES_GREEN,                // 0x08 basic: Green / みどり
    SERIES_CABIN,                // 0x09 basic: Cabin / ログ
    SERIES_KIDDIE,               // 0x0A basic: Kiddie / カラフル
    SERIES_ROBO,                 // 0x0B basic: Robo / ロボ
    SERIES_SNOWMAN,              // 0x0C basic: Snowman / ゆきだるま
    SERIES_MUSHROOM,             // 0x0D basic: Mushroom / きのこ
    SERIES_FESTIVE,              // 0x0E basic: Festive / クリスマス
    SERIES_HARVEST,              // 0x0F basic: Harvest / ハーベスト
    SERIES_SPOOKY,               // 0x10 basic: Spooky / ハロウィン
    SERIES_EGG,                  // 0x11 basic: Egg / たまご
    SERIES_PRINCESS,             // 0x12 basic: Princess / プリンセス
    SERIES_GORGEOUS,             // 0x13 basic: Gorgeous / ゴージャス
    SERIES_SWEETS,               // 0x14 basic: Sweets / お菓子
    SERIES_GRACIE,               // 0x15 basic: Gracie / グレース
    SERIES_FESTIVALE,            // 0x16 basic: Festivale / カーニバル
    SERIES_BASIC_SPARE_1,        // 0x17 basic: 基本予備１
    SERIES_BASIC_SPARE_2,        // 0x18 basic: 基本予備２
    SERIES_BASIC_SPARE_3,        // 0x19 basic: 基本予備３
    SERIES_BASIC_SPARE_4,        // 0x1A basic: 基本予備４
    SERIES_BASIC_SPARE_5,        // 0x1B basic: 基本予備５
    SERIES_WESTERN,              // 0x1C theme: Western / ウェスタン
    SERIES_SPACE,                // 0x1D theme: Space / うちゅう
    SERIES_CONSTRUCTION,         // 0x1E theme: Construction / こうじ
    SERIES_BOXING,               // 0x1F theme: Boxing / プロレス
    SERIES_MOSSY_GARDEN,         // 0x20 theme: Mossy Garden / ガーデン
    SERIES_NURSERY,              // 0x21 theme: Nursery / ベビー
    SERIES_MARIO,                // 0x22 theme: Mario / マリオ
    SERIES_PIRATE,               // 0x23 theme: Pirate / かいぞく
    SERIES_MAD_SCIENTIST,        // 0x24 theme: Mad Scientist / サイエンティスト
    SERIES_BATH,                 // 0x25 theme: Bath / おふろ
    SERIES_THEME_SPARE_1,        // 0x26 theme: テーマ予備１
    SERIES_THEME_SPARE_2,        // 0x27 theme: テーマ予備２
    SERIES_THEME_SPARE_3,        // 0x28 theme: テーマ予備３
    SERIES_THEME_SPARE_4,        // 0x29 theme: テーマ予備４
    SERIES_THEME_SPARE_5,        // 0x2A theme: テーマ予備５
    SERIES_WHITE_CHESS,          // 0x2B set:   しろいチェス (white bishop, white king, white knight, white pawn, ...)
    SERIES_BLACK_CHESS,          // 0x2C set:   くろいチェス (black bishop, black king, black knight, black pawn, ...)
    SERIES_DRUM_CAN,             // 0x2D set:   ドラムカン (brown drum, green drum, haz-mat barrel, oil drum, ...)
    SERIES_CLASSROOM,            // 0x2E set:   がっこう (chalk board, classroom floor, classroom wall, cubby hole, ...)
    SERIES_HOSPITAL,             // 0x2F set:   びょういん (hospital bed, hospital screen, IV drip, scale, ...)
    SERIES_WRITING,              // 0x30 set:   べんきょう (globe, writing chair, writing desk)
    SERIES_OFFICE,               // 0x31 set:   じむ (office chair, office desk, office locker)
    SERIES_CAFE,                 // 0x32 set:   カフェ (cash register, checkout counter, coffee maker, jukebox, ...)
    SERIES_DHARMA,               // 0x33 set:   だるま (dharma, giant dharma, mini-dharma)
    SERIES_BEAR,                 // 0x34 set:   くま (Baby bear, Mama bear, Papa bear)
    SERIES_PANDA,                // 0x35 set:   パンダ (Baby panda, Mama panda, Papa panda)
    SERIES_CACTUS,               // 0x36 set:   サボテン (cactus, round cactus, tall cactus)
    SERIES_GOLF_BAG,             // 0x37 set:   ゴルフ (blue golf bag, green golf bag, white golf bag)
    SERIES_RED,                  // 0x38 set:   あか (red armchair, red sofa)
    SERIES_TOTEM_POLE,           // 0x39 set:   トーテム (bear pole, eagle pole, frog-woman pole, raven pole)
    SERIES_LAVA_LAMP,            // 0x3A set:   ラバランプ (blue lava lamp, green lava lamp, purple lava lamp)
    SERIES_JAPANESE,             // 0x3B set:   わふう (hearth, hibachi, low screen, lucky frog, ...)
    SERIES_YARD,                 // 0x3C set:   にわ (deer scare, tall lantern)
    SERIES_NINTENDO,             // 0x3D set:   にんてんどう (Arwing, banana, Blue Falcon, kart, ...)
    SERIES_VASE,                 // 0x3E set:   つぼ (blue vase, red vase, tea vase)
    SERIES_LUCKY_CAT,            // 0x3F set:   まねき (lefty lucky cat, lucky black cat, lucky cat, lucky gold cat)
    SERIES_CITRUS,               // 0x40 set:   かんきつ (grapefruit table, lemon table, lime chair, orange chair)
    SERIES_PEAR,                 // 0x41 set:   ヨウナシ (pear dresser, pear wardrobe)
    SERIES_WATERMELON,           // 0x42 set:   スイカ (melon chair, watermelon chair, watermelon table)
    SERIES_APPLE,                // 0x43 set:   リンゴ (apple clock, apple TV)
    SERIES_FROGGY,               // 0x44 set:   カエル (froggy chair, lily-pad table)
    SERIES_PINE,                 // 0x45 set:   パインざい (pine chair, pine table)
    SERIES_TULIP,                // 0x46 set:   あかいおはな (tulip chair, tulip table)
    SERIES_IRIS,                 // 0x47 set:   しろいおはな (iris chair, iris table)
    SERIES_DAFFODIL,             // 0x48 set:   きいろのはな (daffodil chair, daffodil table)
    SERIES_DRUM,                 // 0x49 set:   ドラム (conga drum, djimbe drum, timpano drum)
    SERIES_STRINGS,              // 0x4A set:   げんがく (bass, cello, violin)
    SERIES_GUITAR,               // 0x4B set:   ギター (country guitar, folk guitar, metal guitar, rock guitar)
    SERIES_PINE_BONSAI,          // 0x4C set:   まつ (mugho bonsai, pine bonsai, ponderosa bonsai)
    SERIES_BONSAI,               // 0x4D set:   ぼんさい (azalea bonsai, hawthorn bonsai, holly bonsai, jasmine bonsai, ...)
    SERIES_HOUSEPLANT,           // 0x4E set:   かんよう (aloe, bromeliaceae, caladium, coconut palm, ...)
    SERIES_T_REX,                // 0x4F set:   ティラノ (T. rex skull, T. rex tail, T. rex torso)
    SERIES_TRICERA,              // 0x50 set:   トリケラ (tricera skull, tricera tail, tricera torso)
    SERIES_MAMMOTH,              // 0x51 set:   マンモス (mammoth skull, mammoth torso)
    SERIES_ANKYLO,               // 0x52 set:   アンキロ (ankylo skull, ankylo tail, ankylo torso)
    SERIES_APATO,                // 0x53 set:   アパト (apato skull, apato tail, apato torso)
    SERIES_DIMETRODON,           // 0x54 set:   ディメトロ (dimetrodon skull, dimetrodon tail, dimetrodon torso)
    SERIES_IGUANODON,            // 0x55 set:   イグアノ (iguanodon skull, iguanodon tail, iguanodon torso)
    SERIES_SABERTOOTH,           // 0x56 set:   タイガー (sabertooth skull, sabertooth torso)
    SERIES_PACHY,                // 0x57 set:   パキケファロ (pachy skull, pachy tail, pachy torso)
    SERIES_PARASAUR,             // 0x58 set:   パラサウロ (parasaur skull, parasaur tail, parasaur torso)
    SERIES_SEISMO,               // 0x59 set:   セイスモ (seismo chest, seismo hip, seismo skull, seismo tail)
    SERIES_PLESIO,               // 0x5A set:   フタバ (plesio neck, plesio skull, plesio torso)
    SERIES_STEGO,                // 0x5B set:   ステゴ (stego skull, stego tail, stego torso)
    SERIES_PTERA,                // 0x5C set:   プテラノドン (ptera left wing, ptera right wing, ptera skull)
    SERIES_ICTHYO,               // 0x5D set:   イクチオ (icthyo skull, icthyo torso)
    SERIES_RAPTOR,               // 0x5E set:   ラプター (raptor skull, raptor torso)
    SERIES_STYRACO,              // 0x5F set:   スティラコ (styraco skull, styraco tail, styraco torso)
    SERIES_SET_SPARE_1,          // 0x60 set:   セット予備１
    SERIES_SET_SPARE_2,          // 0x61 set:   セット予備２
    SERIES_SET_SPARE_3,          // 0x62 set:   セット予備３
    SERIES_SET_SPARE_4,          // 0x63 set:   セット予備４
    SERIES_SET_SPARE_5,          // 0x64 set:   セット予備５
    SERIES_OTHER,                // 0x65 other: Other / そのた
    SERIES_EXTRA_BASIC_1,        // 0x66 basic: 追加基本０１
    SERIES_EXTRA_BASIC_2,        // 0x67 basic: 追加基本０２
    SERIES_EXTRA_BASIC_3,        // 0x68 basic: 追加基本０３
    SERIES_EXTRA_BASIC_4,        // 0x69 basic: 追加基本０４
    SERIES_EXTRA_BASIC_5,        // 0x6A basic: 追加基本０５
    SERIES_EXTRA_BASIC_6,        // 0x6B basic: 追加基本０６
    SERIES_EXTRA_BASIC_7,        // 0x6C basic: 追加基本０７
    SERIES_EXTRA_BASIC_8,        // 0x6D basic: 追加基本０８
    SERIES_EXTRA_BASIC_9,        // 0x6E basic: 追加基本０９
    SERIES_EXTRA_BASIC_10,       // 0x6F basic: 追加基本１０
    SERIES_EXTRA_THEME_1,        // 0x70 theme: 追加テーマ０１
    SERIES_EXTRA_THEME_2,        // 0x71 theme: 追加テーマ０２
    SERIES_EXTRA_THEME_3,        // 0x72 theme: 追加テーマ０３
    SERIES_EXTRA_THEME_4,        // 0x73 theme: 追加テーマ０４
    SERIES_EXTRA_THEME_5,        // 0x74 theme: 追加テーマ０５
    SERIES_EXTRA_THEME_6,        // 0x75 theme: 追加テーマ０６
    SERIES_EXTRA_THEME_7,        // 0x76 theme: 追加テーマ０７
    SERIES_EXTRA_THEME_8,        // 0x77 theme: 追加テーマ０８
    SERIES_EXTRA_THEME_9,        // 0x78 theme: 追加テーマ０９
    SERIES_EXTRA_THEME_10,       // 0x79 theme: 追加テーマ１０
    SERIES_EXTRA_SET_1,          // 0x7A set:   追加セット０１
    SERIES_EXTRA_SET_2,          // 0x7B set:   追加セット０２
    SERIES_EXTRA_SET_3,          // 0x7C set:   追加セット０３
    SERIES_EXTRA_SET_4,          // 0x7D set:   追加セット０４
    SERIES_EXTRA_SET_5,          // 0x7E set:   追加セット０５
    SERIES_EXTRA_SET_6,          // 0x7F set:   追加セット０６
    SERIES_EXTRA_SET_7,          // 0x80 set:   追加セット０７
    SERIES_EXTRA_SET_8,          // 0x81 set:   追加セット０８
    SERIES_EXTRA_SET_9,          // 0x82 set:   追加セット０９
    SERIES_EXTRA_SET_10,         // 0x83 set:   追加セット１０

    SERIES_COUNT
};

// Item source groups (BITM::m_from). Names follow the game's own string table
// (804EA114, owned by the preceding TU).
enum From {
    FROM_GROUP_ABC, // 0x00
    FROM_GROUP_ABC_AVERAGE,
    FROM_GROUP_ABC_ALL,
    FROM_GROUP_A,
    FROM_GROUP_B,
    FROM_GROUP_C,
    FROM_EYE_CATCHER,
    FROM_FOX,
    FROM_TAILOR,
    FROM_PRESENT,
    FROM_HANIWA,
    FROM_FLOWER,
    FROM_INSECT,
    FROM_FISH,
    FROM_NORMAL_FOSSIL,
    FROM_NICE_FOSSIL,
    FROM_SP_PRESENT,
    FROM_SNOW,
    FROM_JONNY,
    FROM_FORTUNE,
    FROM_RAKKO,
    FROM_PICTURE,
    FROM_ORIGINAL,
    FROM_NOUSE,
    FROM_DONGURI,
    FROM_LOST,
    FROM_WARASIBE,
    FROM_SAVING,
    FROM_FISHING,
    FROM_FISHING_SP,
    FROM_BUGCATCHING,
    FROM_GARDENING,
    FROM_GRACE,
    FROM_ROLAN,
    FROM_SHELL,
    FROM_BED_DEFAULT,
    FROM_FOX_PICTURE,
    FROM_FORGED,
    FROM_HAPPY_ROOM,
    FROM_POINT_CHANGE,
    FROM_POINT_PRESENT,
    FROM_AFTER_FORGED,
    FROM_LIMITED1,
    FROM_LIMITED2,
    FROM_NONE, // 0x2C
    FROM_JINGLE,
    FROM_SETSUBUN,
    FROM_HINA,
    FROM_KODOMO,
    FROM_TSUKIMI_JP,
    FROM_TSUKIMI_EU,
    FROM_TSUKIMI_KR,
    FROM_GROUNDHOG,
    FROM_EARTH_DAY,
    FROM_LABOR_DAY,
    FROM_COLUMBUS_DAY,
    FROM_HARVESTMOON,
    FROM_MIDSUMMER,
    FROM_ST_NICHOLAS_DAY,
    FROM_MIDWINTER,
    FROM_OLD_NEWYEAR,
    FROM_PLANTING_DAY,
    FROM_MASTERS_DAY,
    FROM_TANABATA,
    FROM_NEWYEAR,
    FROM_COUNTDOWN,
    FROM_HARVESTFESTIVAL,
    FROM_HALLOWEEN,
    FROM_FIREWORKS,
    FROM_EASTER,
    FROM_GRACE_SPR,
    FROM_GRACE_SUM,
    FROM_GRACE_AUT,
    FROM_GRACE_WIN,
    FROM_CARNIVAL,
    FROM_MUSIC_GOKIGEN,
    FROM_MUSIC_FUKIGEN,
    FROM_MUSIC_MATTARI,
    FROM_MUSIC_BLUE,
    FROM_MUSIC_UNKNOWN,
    FROM_MUSIC_SECRET,
    FROM_APRIL_FOOL,
    FROM_LIMITED3,
    FROM_LIMITED4,
    FROM_BALLOON,
    FROM_GRACE_SPR_FASHION,
    FROM_GRACE_SUM_FASHION,
    FROM_GRACE_AUT_FASHION,
    FROM_GRACE_WIN_FASHION,
    FROM_BALLOON_MAN,
    FROM_MUSHROOM,
    FROM_MUSIC_HAZURE,

    FROM_COUNT, // 0x5C
};

// Debug name tables, unreferenced in the release build.
extern const char *sFtrFuncNames[0x98]; // 804E91E8: furniture function names
extern const char *sFromNames[FROM_COUNT]; // 804EA128: source group names

// item/item.bin header; the BITM entries follow it.
struct BinHeader {
    u32 mCount; // 0x00: ITEM_COUNT
    u32 mEntrySize; // 0x04: sizeof(BITM)
    u8 _08[0x18];
}; // sizeof = 0x20

// Range check used by the BITM field accessors: an out-of-range value
// falls back to a default.
inline int clampField(int value, u32 count, int def) {
    return static_cast<u32>(value) < count ? value : def;
}

// One item definition. Field order and names follow community research;
// offsets are verified against item.bin and the dItem accessors.
struct BITM {
    // Normal or silver tool of a type; gold tools are excluded.
    bool isAxe() const; // 800C1AD4
    bool isNet() const; // 800C1B0C
    bool isFishingrod() const; // 800C1B44
    bool isWatering() const; // 800C1B7C
    int getNewOld() const; // 800C1BB4
    int getAdultKiddy() const; // 800C1BE0
    BOOL isNotForSale() const; // 800C1C08
    static u16 getVersion(); // 800C1C80
    BOOL isValid() const; // 800C1C88
    int getFtrFuncType() const; // 800C1CCC
    const wchar_t *getName() const; // 800C1CF8
    int getDefArticle() const; // 800C1D90
    int getIndefArticle() const; // 800C1E48
    int getGender() const; // 800C1F00
    BOOL isFurnitureLike() const; // 800C1FB0
    BOOL isAvailableInRegion() const; // 800C2008
    int getSeriesGroup() const; // 800C2068
    BOOL canAddToCatalog() const; // 800C20DC
    int getKindFlag6() const; // 800C214C
    int getKindFlag5() const; // 800C21AC
    int getKindFlag4() const; // 800C220C
    const char *getKindName() const; // 800C226C
    const char *getFgObjName(BOOL alt) const; // 800C22CC
    u16 getFromNameIndex() const; // 800C2318
    int resolveColor(int color, BOOL usePlayer) const; // 800C2354

    int getKind() const {
        int kind = m_kind;
        return static_cast<u32>(kind) < KIND_COUNT ? kind : KIND_NONE;
    }

    int getFtrSize() const {
        int size = static_cast<s8>(m_ftrSize);
        return static_cast<u32>(size) < FTR_SIZE_COUNT ? static_cast<FtrSize>(size) : FTR_SIZE_1x1;
    }

    int getFtrPartA() const {
        int part = m_ftrPartA;
        return static_cast<u32>(part) < FTR_PART_A_COUNT ? static_cast<FtrPartA>(part) : FTR_PART_A_NONE;
    }

    int getFtrLamp() const {
        int lamp = static_cast<s8>(m_ftrLamp);
        return static_cast<u32>(lamp) < FTR_LAMP_COUNT ? static_cast<FtrLamp>(lamp) : FTR_LAMP_NONE;
    }

    int getFossil() const {
        int fossil = m_fossil;
        return static_cast<u32>(fossil) < FOSSIL_COUNT ? static_cast<Fossil>(fossil) : FOSSIL_NONE;
    }

    int getSeries() const {
        int series = m_series;
        return static_cast<u32>(series) < SERIES_COUNT ? static_cast<SeriesId>(series) : SERIES_EXOTIC;
    }

    int getFashion() const {
        int fashion = m_fashion;
        return static_cast<u32>(fashion) < FASHION_COUNT ? static_cast<Fashion>(fashion) : FASHION_NONE;
    }

    u32 m_magic; // 0x000: 'BITM'
    s32 m_price; // 0x004
    s16 m_baseId; // 0x008
    u16 m_icon; // 0x00A
    s16 m_ftrKind; // 0x00C
    s16 _E; // 0x00E
    s16 m_version; // 0x010
    wchar_t m_nameJp[17]; // 0x012
    wchar_t m_nameUs[17]; // 0x034
    wchar_t m_nameMx[17]; // 0x056
    wchar_t m_nameQc[17]; // 0x078
    wchar_t m_nameEn[17]; // 0x09A
    wchar_t m_nameDe[17]; // 0x0BC
    wchar_t m_nameIt[17]; // 0x0DE
    wchar_t m_nameEs[17]; // 0x100
    wchar_t m_nameFr[17]; // 0x122
    wchar_t m_nameKr[17]; // 0x144
    s8 m_kind; // 0x166
    u8 m_addItem; // 0x167
    s8 m_fgobj; // 0x168
    s8 m_series; // 0x169
    s8 m_from; // 0x16A
    u8 m_carpetSfx; // 0x16B
    s8 m_fashion; // 0x16C
    u8 m_designCreatorName; // 0x16D
    u8 m_designCreatorFrom; // 0x16E
    u8 m_catalogScale; // 0x16F
    u8 _170; // 0x170
    s8 m_ftrFunc; // 0x171
    u8 m_ftrInstr; // 0x172
    u8 m_ftrSfx; // 0x173
    s8 m_fossil; // 0x174
    u8 m_ftrHeight; // 0x175
    // MWCC allocates u8 bit-fields from the most significant bit.
    u8 m_region : 4; // 0x176
    u8 m_season : 4;
    u8 m_hideBone : 4; // 0x177
    u8 m_style : 4;
    u8 m_textPltt : 4; // 0x178
    u8 m_designPltt : 4;
    u8 m_ftrSize : 4; // 0x179
    u8 m_ftrColorA : 4;
    u8 m_ftrColorB : 4; // 0x17A
    u8 m_ftrPartA : 4;
    u8 m_ftrPartB : 4; // 0x17B
    u8 m_defUs : 4;
    u8 m_indefUs : 4; // 0x17C
    u8 m_defMx : 4;
    u8 m_indefMx : 4; // 0x17D
    u8 m_genderMx : 4;
    u8 m_defQc : 4; // 0x17E
    u8 m_indefQc : 4;
    u8 m_genderQc : 4; // 0x17F
    u8 m_defEn : 4;
    u8 m_indefEn : 4; // 0x180
    u8 m_defDe : 4;
    u8 m_indefDe : 4; // 0x181
    u8 m_genderDe : 4;
    u8 m_defIt : 4; // 0x182
    u8 m_indefIt : 4;
    u8 m_genderIt : 4; // 0x183
    u8 m_defEs : 4;
    u8 m_indefEs : 4; // 0x184
    u8 m_genderEs : 4;
    u8 m_defFr : 4; // 0x185
    u8 m_indefFr : 4;
    u8 m_genderFr : 4; // 0x186
    u8 m_edibility : 2;
    u8 m_ftrTable : 2;
    u8 m_ftrLamp : 2; // 0x187
    u8 m_hasRes : 1;
    u8 m_noPurchase : 1;
    u8 _187_3 : 1;
    u8 m_proDesign : 1;
    u8 m_catalogStore : 1;
    // The genre pairs straddle bytes and are read as separate bits
    // (0x187 bit 0 / 0x188 bit 7, 0x188 bits 6 and 5).
    u8 m_ftrGenreA0 : 1;
    u8 m_ftrGenreA1 : 1; // 0x188
    u8 m_ftrGenreB0 : 1;
    u8 m_ftrGenreB1 : 1;
    u8 m_hraWallCheck : 1;
    u8 m_hraLuckyBonus : 1;
    u8 _188_2 : 1;
    u8 m_ftrPassThru : 1;
    u8 _188_0 : 1;
    u8 _189_7 : 1; // 0x189
}; // sizeof = 0x18C

#ifdef __MWERKS__ // wchar_t is 16-bit on MWCC
typedef char BitmSizeCheck[sizeof(BITM) == 0x18C ? 1 : -1];
#endif
typedef char BinHeaderSizeCheck[sizeof(BinHeader) == 0x20 ? 1 : -1];

// Base-ID to item-index map plus per-kind index ranges.
// Global at 8059AD98; built from item.bin and the downloadable items.
class indexTable_c {
public:
    u16 getKindFirst(int kind) const; // 800C142C
    u16 getKindLast(int kind) const; // 800C145C
    int getIndexInKind(u16 index) const; // 800C148C
    u16 getIndex(u16 baseId) const; // 800C150C
    void build(); // 800C1530
    void addDlItems(); // 800C1790
    void setDlItem(u16 baseId, u16 slot); // 800C1864

    u16 mIndex[BASE_ID_COUNT]; // 0x0000
    u16 mKindFirst[KIND_COUNT]; // 0x2000
    u16 mKindLast[KIND_COUNT]; // 0x20AE
}; // sizeof = 0x215C

// kind.bin entry (one byte per kind). MWCC allocates from the top bit.
struct KindInfo {
    u8 mCheckNoPurchase : 1; // bit 7: BITM::m_noPurchase applies
    u8 mFlag6 : 1;
    u8 mFlag5 : 1;
    u8 mFlag4 : 1;
    u8 _low : 4;
};

// series.bin entry.
struct Series {
    s8 mId; // 0x00
    u8 _01[0x21];
    u8 mGroup : 4; // 0x22
    u8 _22_lo : 4;
}; // sizeof = 0x23

// npcMsg.bin entry (3 bytes) and npcMsgBullfest.bin entry (6 bytes).
struct NpcMsg {
    u8 mMsg; // 0x00 (read as s8)
    // Per-slot enables for the two message sets (A = first, B = second).
    u8 mA0 : 1; // 0x01
    u8 mB0 : 1;
    u8 mA1 : 1;
    u8 mB1 : 1;
    u8 mA2 : 1;
    u8 mB2 : 1;
    u8 mA3 : 1;
    u8 mB3 : 1;
    u8 mA4 : 1; // 0x02
    u8 mB4 : 1;
    u8 mA5 : 1;
    u8 mB5 : 1;
    u8 _02_lo : 4;
};
struct NpcMsgBullfest {
    u8 mMsg[6];
};

// MWCC emits vtables in reverse class-definition order, so the order of the
// classes from here to makeToCstmSendData_c sets d_item's .data layout.
class resLoader_c : public dDvd::brresBank_c {
public:
    resLoader_c(); // 800C3B40
    virtual ~resLoader_c(); // 800C3B84
    virtual void onLoaded(); // 800C440C

    bool isLinked() const { return mIndex != INDEX_NONE; }
    void link(u16 index); // 800C3A5C
    void unlink(); // 800C3AC4
    void reset(); // 800C3B10
    BOOL loadFromMemory(const void *src, u32 size, void *heap); // 800C3BFC
    BOOL loadDesign(void *design, void *heap, s16 width, s16 height); // 800C3CC0
    BOOL loadDesignInPlace(void *design, void *heap, s16 width, s16 height); // 800C3D30
    BOOL loadTexture(const void *tex, const void *pltt, void *heap, s16 width, s16 height); // 800C3DA0
    BOOL bindTexture(void *tex, void *pltt, void *heap, s16 width, s16 height); // 800C3F1C
    BOOL loadItem(Item item, void *heap); // 800C4054
    BOOL loadIndex(u16 index, void *heap); // 800C40B4
    bool release(); // 800C43B8
    s32 getDataSize(); // 800C4488
    void replaceTexture(void *tex, void *pltt, u32 texIdx, u32 plttIdx, u16 width, u16 height); // 800C44C0

    nw4r::ut::Link mLink; // 0x58
    u16 mIndex; // 0x60
    const BITM *mpBITM; // 0x64
    u16 mItemId; // 0x68 (a raw id: the constructor leaves it unset)
    u8 mFromArchive; // 0x6A
    s32 mSize; // 0x6C
}; // sizeof = 0x70

class dsnPlttLoader_c : public dDvd::brresBank_c {
public:
    // The implicit destructor is emitted at 800C5674.
    virtual void onLoaded(); // 800C249C

    static dsnPlttLoader_c *get(); // 800C242C
}; // sizeof = 0x58

class infoBank_c : public dDvd::arcBank_c {
public:
    infoBank_c(); // 800C24D8
    virtual ~infoBank_c(); // 800C5564
    virtual void onLoaded(); // 800C2590

    static infoBank_c *get(); // 800C24CC
    BOOL load(void *heap); // 800C257C
    void loadDlItems(); // 800C2670
    void setDlItem(u16 baseId, u16 slot); // 800C2684
    void setPalettes(nw4r::g3d::ResFile file); // 800C2690
    const BITM *getBITM(u16 index) const; // 800C2704
    const BITM *getBITM(Item item) const; // 800C27E0
    Series *getSeries(u32 series); // 800C2870
    NpcMsg *getNpcMsg(u16 index); // 800C2890
    NpcMsgBullfest *getNpcMsgBullfest(u16 index); // 800C28C0
    KindInfo *getKindInfo(u32 kind); // 800C28EC
    u16 getIndexFromBaseId(u16 baseId) const; // 800C2910
    u16 getBaseId(u16 index) const; // 800C291C
    u16 getItemId(u16 index); // 800C2998
    u16 getItemIdFromIndex(int index); // 800C29C8
    u16 getBaseIdFromItemId(u16 id) const; // 800C29D0
    BOOL isBuiltinBaseId(u16 baseId); // 800C2A10
    BOOL isBuiltinItemId(u16 id); // 800C2A3C
    u16 getIndexFromItemId(u16 id) const; // 800C2AA8

    u8 mDlLoaded; // 0x74
    BITM *mpItems; // 0x78
    Series *mpSeries; // 0x7C
    NpcMsg *mpNpcMsg; // 0x80
    u32 mNpcMsgCount; // 0x84
    NpcMsgBullfest *mpNpcMsgBullfest; // 0x88
    u32 mNpcMsgBullfestCount; // 0x8C
    KindInfo *mpKindInfo; // 0x90
    void *mpPalettes[16]; // 0x94
}; // sizeof = 0xD4

// Candidate set over item indices 0..DL_ITEM_END.
class seeker_c {
public:
    // Filters candidates during search.
    class candCB_c {
    public:
        virtual BOOL check(const BITM *bitm, Item *item) const { return TRUE; } // 800C56D4
    };

    seeker_c() { memset(this, 0, sizeof(seeker_c)); }

    static seeker_c *get(); // 800C2B34
    void add(u16 index); // 800C2B40
    BOOL contains(u16 index) const; // 800C2B80
    void searchItem(const Item &item, int flags, candCB_c *cb); // 800C2BC0
    void search(int kind, int flags, candCB_c *cb); // 800C2C58
    Item getNth(u32 n) const; // 800C2E6C
    Item getRandom() const; // 800C2F34
    int find(const Item &item) const; // 800C2F9C
    int findLike(Item item); // 800C303C
    int findInSeries(Item item); // 800C3088
    u32 searchSeries(int series, int kind); // 800C3180
    Item getNthInSeries(u32 n, int series, int kind); // 800C31E8
    int findFossil(Item item); // 800C3278
    u32 searchFossil(int fossil); // 800C32E0
    Item getRandomFossil(int fossil); // 800C3330

    inline int findFromId(u16 id) {
        Item item(id);
        return findLike(item);
    }

    u8 mBits[(DL_ITEM_END + 7) / 8]; // 0x000
    u32 mCount; // 0x164
}; // sizeof = 0x168

class clothCandCB_c : public seeker_c::candCB_c {
public:
    clothCandCB_c(Item item, int style, int prevStyle, BOOL excludeNoSale); // 800C3394
    void set(Item item, int style, int prevStyle, BOOL excludeNoSale); // 800C33E8
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C3400

    Item mExclude; // 0x04
    int mStyle; // 0x08
    int mPrevStyle; // 0x0C
    u8 mExcludeNoSale; // 0x10
};

class fromCandCB_c : public seeker_c::candCB_c {
public:
    fromCandCB_c(int from) : mFrom(from) {}
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C36DC

    int mFrom; // 0x04
};

class musicCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C34D0

    int mExcludeFrom; // 0x04
    int mFrom[5]; // 0x08
    u8 *mpOwned; // 0x1C
};

class fossilCandCB_c : public seeker_c::candCB_c {
public:
    fossilCandCB_c(int fossil) : mFossil(fossil) {}
    fossilCandCB_c(Item item) { set(item); } // 800C5744
    void set(Item item); // 800C3718
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C3784

    int mFossil; // 0x04
};

class colorCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C37F0

    int mColor; // 0x04
};

class seriesCandCB_c : public seeker_c::candCB_c {
public:
    seriesCandCB_c(int series, Item exclude, BOOL excludeNoSale) { set(series, exclude, excludeNoSale); } // 800C56DC
    void set(int series, Item exclude, BOOL excludeNoSale) { // 800C5730
        mSeries = series;
        mExclude = exclude;
        mExcludeNoSale = excludeNoSale;
    }
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C38AC

    int mSeries; // 0x04
    Item mExclude; // 0x08
    u8 mExcludeNoSale; // 0x0A
};

class sizeCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C3954

    int mSize; // 0x04
};

class ftrSeCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C3984

    u32 mSfx; // 0x04
};

class newOldCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C399C

    int mValue; // 0x04
};

class adultKiddyCandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C39DC

    int mValue; // 0x04
};

class categoryQ5CandCB_c : public seeker_c::candCB_c {
public:
    virtual BOOL check(const BITM *bitm, Item *item) const; // 800C3A1C

    int mCategory; // 0x04
};

// Shared state for loaded resLoader_c objects. Global at 8059D308.
struct resList_c {
    nw4r::ut::List mList; // 0x00
    u8 mState[ITEM_COUNT]; // 0x0C: per-index load state (1 = loading, 2 = loaded)
}; // sizeof = 0xA24

} // namespace dItem

class dPrivateData_c;

// Enumerates items to send over the network. Global classes: their RTTI names carry no dItem::.
class makeSendData_c {
public:
    // Declared before the first virtual, so the vtable pointer lands at 0x08.
    u32 mCount; // 0x00
    u8 *mpExclude; // 0x04

    makeSendData_c(u32 count, u8 *exclude); // 800C45B8
    u32 countSendable() const; // 800C45D0
    void *getNthSendable(u32 n) const; // 800C4698

    virtual dItem::Item getItem(u32 index) const = 0;
};

class makePlEquipSendData_c : public makeSendData_c {
public:
    makePlEquipSendData_c(dPrivateData_c *player) : makeSendData_c(2, NULL), mpPlayer(player) {}
    virtual dItem::Item getItem(u32 index) const; // 800C4760

    dPrivateData_c *mpPlayer; // 0x0C
};

class makeToCstmSendData_c : public makeSendData_c {
public:
    makeToCstmSendData_c(u8 *exclude) : makeSendData_c(dItem::DL_ITEM_COUNT, exclude) {}
    virtual dItem::Item getItem(u32 index) const; // 800C48C8
};

class dCatalog_c;

namespace dItem {

// View over downloadable-item blocks in the save (0x2000 bytes each).
struct dlBlockList_c {
    dlBlockList_c(u8 *blocks, u32 count); // 800C4AE8
    BOOL add(void *block); // 800C4AF4
    BOOL addItem(Item item); // 800C4BC8
    void clear(); // 800C4C60
    int addFromPlayer(dPrivateData_c *player); // 800C4C8C
    int countValid(); // 800C4DA0
    int countOwned(dPrivateData_c *player); // 800C4E48

    u8 *mpBlocks; // 0x00
    u32 mCount; // 0x04
};

// Item name holder: a word with room for one BITM name (17 characters).
// Vtable 8049E4EC; its functions live in the TU around 8000C14C.
class name_c : public dScript::Word_c {
public:
    virtual ~name_c(); // 8000DDF4
    virtual u32 getBufferSize(); // 8000DE54: returns sizeof(mBuffer)
    virtual wchar_t *getBuffer(); // 8000DE4C

    wchar_t mBuffer[17]; // 0x24
}; // sizeof = 0x48

// Category-specific names (RTTI-only so far). Each adds nothing but its
// own destructor on top of dString::Word_c.
class nameFashion_c : public dString::Word_c {
public:
    virtual ~nameFashion_c(); // 80036274
};
class nameLook_c : public dString::Word_c {
public:
    virtual ~nameLook_c(); // 800362CC
};
class nameLookQ4_c : public dString::Word_c {
public:
    virtual ~nameLookQ4_c(); // 8003616C
};
class nameSeries_c : public dString::Word_c {
public:
    virtual ~nameSeries_c(); // 800B4D48
};
class nameSeriesQ5_c : public dString::Word_c {
public:
    virtual ~nameSeriesQ5_c(); // 8003621C
};
class nameCategoryQ5_c : public dString::Word_c {
public:
    virtual ~nameCategoryQ5_c(); // 800361C4
};

static inline int Item_getIdxInKind(const Item &item) {
    return seeker_c::get()->findLike(item);
}

static inline const dItem::BITM *getBITM(u16 id) {
    Item item(id);
    return infoBank_c::get()->getBITM(item);
}

} // namespace dItem
