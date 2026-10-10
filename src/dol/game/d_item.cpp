#include <game/game/d_region.hpp>
#include <game/sLib/s_crc.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/revolution/OS/OSCache.h>
#include <lib/revolution/OS/OSError.h>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <cstdio>
#include <game/game/d_player_mgr.hpp>

// First pass: every function in the TU is written for equivalence; matching
// work has not started. External callees whose owners are unrecovered keep
// their address names (see the extern "C" block).

#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_catalog.hpp>

using namespace dItem;

extern "C" {
void __register_global_object(void *object, void *dtor, void *node);


// Loads entry `index` of a message group into a word (TU near 8016AC58).
void fn_8016AE68(dScript::Word_c *word, u16 index, const char *group);


// Item debug TU (empty in release).
void *fn_800C59EC();
void fn_800C59E0(void *debug);
void fn_800C59E4(void *debug);
void fn_800C59E8(void *debug, Item item);
}


// 804E91E8: furniture function names (debug). Unreferenced in the release build.
const char *dItem::sFtrFuncNames[0x98] = {
    "なし", "ソ\ファ", "引き出し型収納", "扉型収納", "ブーブークッション", "電気スイッチ", "蛍光灯", "ろうそく", "イス", "ベッド", "いぬごや",
    "マスターソ\ード", "アーウィン", "トライフォース", "ちゃいろのドラムかん", "みどりのドラムかん", "あかいドラムかん", "キケンなドラムかん", "きいろいドラムかん",
    "だんしようトイレ", "ピクミン", "おきあがりこぼし", "ちょきんばこ", "ゴング", "コイン", "どかん", "ファイアフラワー", "はた", "ハテナブロック",
    "キノコ", "ノコノコのこうら", "キラーほうだい", "ピンボールだい", "サンドバッグ", "ボウリングのピン", "トーテムポール・アラ", "トーテムポール・オヤ",
    "トーテムポール・サテ", "トーテムポール・マア", "ちきゅうぎ", "１ＵＰキノコ", "スピードバッグ", "きゅうゆき", "ておしぐるま", "イス型トイレ",
    "マッサージいす", "じっけんイス", "じっけんだい", "そらとぶえんばん", "バーベキューグリル", "ハムスターのかご", "たいまつ", "とりかご", "たきび",
    "キャンプファイア", "いろり", "ダルマストーブ", "ストーブ", "だんろ", "プール", "ファイアバー", "ふるいミシン", "スター", "メリーゴーランド",
    "メトロイド", "アップライトゲームき", "しばかりき", "マーライオン", "てつどうもけい", "せんぷうき", "スプリンクラー", "にそうしきせんたくき",
    "ぜんじどうせんたくき", "ミキサー", "ろてんぶろ", "すごそうなキカイ", "しょうべんこぞう", "クッキングヒーター", "キッチンのシンク", "ドラムしきせんたくき",
    "でんしレンジ", "ミツバチ", "スズメバチ", "アブラゼミ", "ミンミンゼミ", "ヒグラシ", "カ", "ハエ", "れいぞうこ", "たからばこ", "ミイラのひつぎ",
    "きんこ", "ひっしょうダルマ", "ファッションケース", "ダルマ", "ミニダルマ", "にんじゃとう", "パソ\コン", "木のイス", "通常時計", "振り子時計",
    "鳩時計", "ボンボン時計", "テレビ", "テレビデオ", "エフゼロ", "ビックリばこ", "アナログレコードプレイヤー", "オルゴール", "ししおどし", "みずのみドリ",
    "アメリカンクラッカー", "ごみばこ", "マトリョーシカ", "トースター", "キャンディマシン", "レジスター", "よくわからないキカイ", "しりょくそくていき",
    "ゆりかご", "ボウリングリターン", "メトロノーム", "むぎばたけ", "ちくおんき", "テープレコーダー", "レトロなステレオ", "サイコロコンポ", "ラジカセ",
    "CDラジカセ", "ダブルラジカセ", "ロボコンポ", "うばぐるま", "ロボタンス", "ロッカー", "ロイヤル系ベッド", "ショウリョウバッタ", "オケラ",
    "ツクツクホウシ", "ローラー付き家具", "ゆぶね", "ロケット", "しんじつのくち", "かきごおりき", "ヨッシーのタマゴ", "バースデーケーキ", "うらないテレフォン",
    "スノードーム", "しろいにほんとう", "はりこのとら", "ひばち", "マウンテンバイク", "クーラーバック",
};

// 804E9768: field object model name per BITM fgobj (getFgObjName, alt)
static const char *sFgObjNamesAlt[0x5A] = {
    "fg_apple", "fg_wall", "fg_carpet", "fg_cloth", "fg_cap", "fg_glass", "fg_apple", "fg_orange",
    "fg_pear", "fg_peach", "fg_cherry", "fg_coconut", "fg_moneybag", "fg_music", "fg_fossil",
    "fg_paper", "fg_leaf", "fg_cage", "fg_present", "fg_seedpitfall", "fg_seed", "fg_seedling",
    "fg_seedling_c", "fg_medicine", "fg_kabu", "fg_r_kabu", "fg_timer", "fg_haniwa", "fg_paperbag",
    "fg_umbrella", "fg_wig", "fg_letter", "fg_conch", "fg_bivalve", "fg_coral", "fg_tire", "fg_can",
    "fg_boot", "fg_paint_rd", "fg_paint_bl", "fg_paint_ye", "fg_paint_gr", "fg_paint_pi",
    "fg_paint_or", "fg_paint_lb", "fg_paint_yg", "fg_paint_vi", "fg_paint_br", "fg_paint_wt",
    "fg_paint_bk", "fg_kabu_x", "fg_bottle_mail", "fg_coin", "fg_key", "fg_soldout", "mush0",
    "mush1", "mush2", "mush3", "mush4", "fg_cake", "fg_chocolate", "fg_candy", "fg_egg", "fg_bone",
    "fg_card", "fg_gd_card", "fg_grace_soldout", "fg_lamp", "fg_ufo_parts", "fg_axe", "fg_S_axe",
    "fg_G_axe", "fg_jyoro", "fg_S_jyoro", "fg_G_jyoro", "fg_net", "fg_S_net", "fg_G_net", "fg_pole",
    "fg_S_pole", "fg_G_pole", "fg_scoop", "fg_S_scoop", "fg_G_scoop", "fg_sling", "fg_S_sling",
    "fg_G_sling", "fg_easter_ticket", "fg_present_gr",
};

// 804E98D0: field object model name per BITM fgobj (getFgObjName)
static const char *sFgObjNames[0x5A] = {
    "fg_apple", "fg_wall", "fg_carpet", "fg_cloth", "fg_cap", "fg_glass", "fg_apple", "fg_orange",
    "fg_pear", "fg_peach", "fg_cherry", "fg_coconut", "fg_moneybag", "fg_music", "fg_fossil",
    "fg_paper", "fg_leaf", "fg_cage", "fg_present", "fg_seedpitfall", "fg_seed", "fg_seedling",
    "fg_seedling_c", "fg_medicine", "fg_kabu", "fg_r_kabu", "fg_timer", "fg_haniwa", "fg_paperbag",
    "fg_umbrella", "fg_wig", "fg_letter", "fg_conch", "fg_bivalve", "fg_coral", "fg_tire", "fg_can",
    "fg_boot", "fg_paint_rd", "fg_paint_bl", "fg_paint_ye", "fg_paint_gr", "fg_paint_pi",
    "fg_paint_or", "fg_paint_lb", "fg_paint_yg", "fg_paint_vi", "fg_paint_br", "fg_paint_wt",
    "fg_paint_bk", "fg_kabu_x", "fg_bottle_mail", "fg_coin", "fg_key", "fg_soldout", "mush0",
    "mush1", "mush2", "mush3", "mush4", "fg_cake", "fg_chocolate", "fg_candy", "fg_egg", "fg_bone",
    "fg_card", "fg_gd_card", "fg_grace_soldout", "fg_lamp", "fg_ufo_parts", "fg_axe", "fg_S_axe",
    "fg_G_axe", "fg_jyoro", "fg_S_jyoro", "fg_G_jyoro", "fg_net", "fg_S_net", "fg_G_net", "fg_pole",
    "fg_S_pole", "fg_G_pole", "fg_scoop", "fg_S_scoop", "fg_G_scoop", "fg_sling", "fg_S_sling",
    "fg_G_sling", "fg_easter_ticket", "fg_present_gr",
};

// 804E9C80: kind names (getKindName); also the item/Res archive prefixes
static const char *sKindNames[KIND_COUNT] = {
    "Money", "Wall", "Carpet", "Ftr", "Cloth", "Cap", "Acc", "Insect", "Fish", "Paper",
    "BeforeFossil", "Fossil", "Haniwa", "Picture", "Music", "Fruit", "Seed", "FakePictureBefore",
    "FakePictureAfter", "OrgCloth", "OrgUmb", "OrgCap", "OrgWC", "OrgEasel", "TaCloth", "Seedling",
    "Kabu", "BadKabu", "Rkabu", "RkabuSeed", "Paint", "Shell", "SoldOut", "BottleBefore",
    "BottleAfter", "PitfallSeed", "Pbox", "Hand_Letter", "Paper_Bag", "Medicine", "Dust", "None",
    "Dummy", "EggFakeBefore", "EggFakeAfter", "EggBingoBefore", "EggBingoAfter", "KnifeAndFork",
    "Candy", "Hanabi", "Chocolate", "Cracker", "CatalogOnly", "Key", "Mushroom", "Timer",
    "Umbrella", "Flower", "Fishingrod", "SilverFishingrod", "GoldFishingrod", "Scoop",
    "SilverScoop", "GoldScoop", "Axe", "SilverAxe", "GoldAxe", "Watering", "SilverWatering",
    "GoldWatering", "Net", "SilverNet", "GoldNet", "Pachinko", "GoldPachinko", "SilverPachinko",
    "Balloon", "Syabon", "Windmill", "CreditCard", "Lamp", "UfoParts", "DsnDataPlayer", "DsnDataTa",
    "DsnDataFlag", "DsnDataSeiichi", "MushFtr",
};

// 804EA128: source group names, per From. Unreferenced in the release build.
const char *dItem::sFromNames[FROM_COUNT] = {
    "GROUP_ABC", "GROUP_ABC_AVERAGE", "GROUP_ABC_ALL", "GROUP_A", "GROUP_B", "GROUP_C",
    "EYE_CATCHER", "FOX", "TAILOR", "PRESENT", "HANIWA", "FLOWER", "INSECT", "FISH",
    "NORMAL_FOSSIL", "NICE_FOSSIL", "SP_PRESENT", "SNOW", "JONNY", "FORTUNE", "RAKKO", "PICTURE",
    "ORIGINAL", "NOUSE", "DONGURI", "LOST", "WARASIBE", "SAVING", "FISHING", "FISHING_SP",
    "BUGCATCHING", "GARDENING", "GRACE", "ROLAN", "SHELL", "BED_DEFAULT", "FOX_PICTURE", "FORGED",
    "HAPPY_ROOM", "POINT_CHANGE", "POINT_PRESENT", "AFTER_FORGED", "LIMITED1", "LIMITED2", "NONE",
    "JINGLE", "SETSUBUN", "HINA", "KODOMO", "TSUKIMI_JP", "TSUKIMI_EU", "TSUKIMI_KR", "GROUNDHOG",
    "EARTH_DAY", "LABOR_DAY", "COLUMBUS_DAY", "HARVESTMOON", "MIDSUMMER", "ST_NICHOLAS_DAY",
    "MIDWINTER", "OLD_NEWYEAR", "PLANTING_DAY", "MASTERS_DAY", "TANABATA", "NEWYEAR", "COUNTDOWN",
    "HARVESTFESTIVAL", "HALLOWEEN", "FIREWORKS", "EASTER", "GRACE_SPR", "GRACE_SUM", "GRACE_AUT",
    "GRACE_WIN", "CARNIVAL", "MUSIC_GOKIGEN", "MUSIC_FUKIGEN", "MUSIC_MATTARI", "MUSIC_BLUE",
    "MUSIC_UNKNOWN", "MUSIC_SECRET", "APRIL_FOOL", "LIMITED3", "LIMITED4", "BALLOON",
    "GRACE_SPR_FASHION", "GRACE_SUM_FASHION", "GRACE_AUT_FASHION", "GRACE_WIN_FASHION",
    "BALLOON_MAN", "MUSHROOM", "MUSIC_HAZURE",
};

static wchar_t sNone[] = L"\x306A\x3057"; // "なし" (none); lives in .sdata, so not const

// 800C0F80
static u32 getDlSlot(u16 index) {
    return index - DL_ITEM_FIRST;
}

// 800C0F88
static int getTownRegion() {
    switch (dSaveData_c::getTown()->_0735C2 & 0xF) {
    case 1:
        return 2;
    case 2:
        return 3;
    case 3:
        return 4;
    default:
        return 1;
    }
}

// 800C0FF4
void setItemName(dScript::Word_c *name, Item item) {
    dScript::Inflect_c *inflect;
    name->clear();
    const BITM *bitm = infoBank_c::get()->getBITM(item);
    if (bitm != NULL) {
        inflect = &name->mInflect;
        name->set(bitm->getName(), 0);
        int def = bitm->getDefArticle();
        inflect->setDefArticle(def);
        int indef = bitm->getIndefArticle();
        inflect->setIndefArticle(indef);
        inflect->setGender(bitm->getGender());
    }
}

// 800C10B0
void setFurnitureName(dScript::Word_c *name, u32 index) {
    name->clear();
    if (index < 0x84) {
        fn_8016AE68(name, index + 1, "sys_STRING/STR_Furniture");
    }
}

void setFashionName(dScript::Word_c *name, u32 fashion);

template< typename T >
static inline T ClampEnum(T v, T max) {
    T ret = 0;

    if (v < max) {
        ret = v;
    }

    return ret;
}

// 800C1108
void setItemFashionName(dScript::Word_c *name, Item item) {
    const BITM *bitm = infoBank_c::get()->getBITM(item);
    if (bitm != NULL) {
        setFashionName(name, bitm->getFashion());
    }
}

// 800C116C
void setFashionName(dScript::Word_c *name, u32 fashion) {
    name->clear();
    if (fashion == 0) {
        memcpy(name->getBuffer(), sNone, sizeof(sNone));
    } else if (fashion < 0x39) {
        fn_8016AE68(name, fashion, "sys_STRING/STR_Fashion");
    }
}

// 800C11EC
void setColorName(dScript::Word_c *name, u32 color) {
    name->clear();
    if (color < 0xF) {
        fn_8016AE68(name, color + 1, "sys_STRING/STR_Color");
    }
}

// 800C1244
void setLookName(dScript::Word_c *name, int look) {
    name->clear();
    if ((u32)look < 0xB && look != 0xA) {
        fn_8016AE68(name, look + 1, "sys_STRING/STR_Look");
    }
}

// 800C12A4
void setQ4LookName(dScript::Word_c *name, int look) {
    name->clear();
    if (look >= 0xA) {
        memcpy(name->getBuffer(), sNone, sizeof(sNone));
    } else if (static_cast<u32>(look) < 0xB) {
        fn_8016AE68(name, look + 1, "sys_STRING/STR_Q04_Look");
    }
}

// 800C1328
void setSeriesName(dScript::Word_c *name, u32 series) {
    name->clear();
    Series *entry = infoBank_c::get()->getSeries(series);
    if (entry != NULL) {
        fn_8016AE68(name, (s8)entry->mId, "sys_STRING/STR_Q05_Series");
    } else {
        memcpy(name->getBuffer(), sNone, sizeof(sNone));
    }
}

// 800C13B4
void setQ5PartName(dScript::Word_c *name, u32 part) {
    name->clear();
    if (part < 0xB) {
        fn_8016AE68(name, part, "sys_STRING/STR_Q05_Part");
    } else {
        memcpy(name->getBuffer(), sNone, sizeof(sNone));
    }
}

// 800C142C
u16 indexTable_c::getKindFirst(int kind) const {
    if (kind == KIND_NONE) {
        return 0;
    }
    if (static_cast<u32>(kind) < KIND_COUNT) {
        return mKindFirst[kind];
    }
    return mKindFirst[0];
}

// 800C145C
u16 indexTable_c::getKindLast(int kind) const {
    if (kind == KIND_NONE) {
        return ITEM_COUNT - 1;
    }
    if (static_cast<u32>(kind) < KIND_COUNT) {
        return mKindLast[kind];
    }
    return mKindLast[0];
}

// 800C148C
int indexTable_c::getIndexInKind(u16 index) const {
    if (index < ITEM_COUNT) {
        const BITM *bitm = infoBank_c::get()->getBITM(index);
        if (bitm != NULL) {
            dItem::Item item(getKindFirst(bitm->getKind()));
            return index - item.mId;
        }
    }
    return 0;
}

// 800C150C
u16 indexTable_c::getIndex(u16 baseId) const {
    if (baseId < BASE_ID_COUNT) {
        u16 index = mIndex[baseId];
        if (index < DL_ITEM_END) {
            return index;
        }
    }
    return INDEX_NONE;
}

// 800C1530
void indexTable_c::build() {
    for (u16 *p = mIndex; p < mIndex + BASE_ID_COUNT; p++) {
        *p = INDEX_NONE;
    }

    infoBank_c *bank = infoBank_c::get();
    u32 i;
    for (i = 0; i < ITEM_COUNT; i++) {
        const BITM *bitm = bank->getBITM(static_cast<u16>(i));
        if (bitm != NULL) {
            u16 baseId = static_cast<u32>(bitm->m_baseId);
            if (baseId < BASE_ID_COUNT) {
                mIndex[baseId] = i;
            }
        }
    }

    for (i = 0; i < KIND_COUNT; i++) {
        mKindFirst[i] = INDEX_NONE;
        mKindLast[i] = INDEX_NONE;
    }

    for (i = 0; i < ITEM_COUNT; i++) {
        const BITM *bitm = bank->getBITM(static_cast<u16>(i));
        if (bitm != NULL) {
            int kind = bitm->getKind();
            if (mKindFirst[kind] == INDEX_NONE) {
                mKindFirst[kind] = i;
            }
            mKindLast[kind] = i;
        }
    }
}

// 800C1790
void indexTable_c::addDlItems() {
    const dSaveDLItemList_c *dl = dSaveDLItemList_c::get();
    for (u16 *p = mIndex; p < mIndex + BASE_ID_COUNT; p++) {
        if (*p >= DL_ITEM_FIRST && *p < DL_ITEM_END) {
            *p = INDEX_NONE;
        }
    }

    for (u32 slot = 0; slot < DL_ITEM_COUNT; slot++) {
        const BITM *bitm = dl->getAt(slot)->getValidBITM();
        if (bitm != NULL) {
            u16 baseId = static_cast<u32>(bitm->m_baseId);
            if (baseId < BASE_ID_COUNT) {
                mIndex[baseId] = slot + DL_ITEM_FIRST;
            }
        }
    }

    fn_800C59E4(fn_800C59EC());
}

Item makeItemFromBaseId(u16 baseId);

// 800C1864
void indexTable_c::setDlItem(u16 baseId, u16 slot) {
    if (baseId < BASE_ID_COUNT && slot < DL_ITEM_COUNT) {
        mIndex[baseId] = slot + DL_ITEM_FIRST;
        Item item = makeItemFromBaseId(baseId);
        fn_800C59E8(fn_800C59EC(), item);
    }
}

// 800C18B8
static int getBullfestMsg(const NpcMsgBullfest *msg, int index) {
    switch (index) {
    case 0:
        return static_cast<s8>(msg->mMsg[0]);
    case 1:
        return static_cast<s8>(msg->mMsg[1]);
    case 2:
        return static_cast<s8>(msg->mMsg[2]);
    case 3:
        return static_cast<s8>(msg->mMsg[3]);
    case 4:
        return static_cast<s8>(msg->mMsg[4]);
    case 5:
        return static_cast<s8>(msg->mMsg[5]);
    default:
        return 0;
    }
}

// 800C193C
static int getNpcMsgA(const NpcMsg *msg, int index) {
    switch (index) {
    case 0:
        if (msg->mA0) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 1:
        if (msg->mA1) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 2:
        if (msg->mA2) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 3:
        if (msg->mA3) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 4:
        if (msg->mA4) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 5:
        if (msg->mA5) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    }
    return 0;
}

// 800C1A08
static int getNpcMsgB(const NpcMsg *msg, int index) {
    switch (index) {
    case 0:
        if (msg->mB0) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 1:
        if (msg->mB1) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 2:
        if (msg->mB2) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 3:
        if (msg->mB3) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 4:
        if (msg->mB4) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    case 5:
        if (msg->mB5) {
            return static_cast<s8>(msg->mMsg);
        }
        break;
    }
    return 0;
}

// 800C1AD4
bool BITM::isAxe() const {
    int kind = getKind();

    return kind == KIND_AXE || kind == KIND_SILVER_AXE || kind == KIND_GOLD_AXE;
}

// 800C1B0C
bool BITM::isNet() const {
    int kind = getKind();

    return kind == KIND_NET || kind == KIND_SILVER_NET || kind == KIND_GOLD_NET;
}

// 800C1B44
bool BITM::isFishingrod() const {
    int kind = getKind();

    return kind == KIND_FISHINGROD || kind == KIND_SILVER_FISHINGROD || kind == KIND_GOLD_FISHINGROD;
}

// 800C1B7C
bool BITM::isWatering() const {
    int kind = getKind();

    return kind == KIND_WATERING || kind == KIND_SILVER_WATERING || kind == KIND_GOLD_WATERING;
}

// 800C1BB4
int BITM::getNewOld() const {
    if (m_ftrGenreA0) {
        return 1;
    }
    int result = 0;
    if (m_ftrGenreA1) {
        result = 2;
    }
    return result;
}

// 800C1BE0
int BITM::getAdultKiddy() const {
    if (m_ftrGenreB0) {
        return 1;
    }
    int result = 0;
    if (m_ftrGenreB1) {
        result = 2;
    }
    return result;
}

// 800C1C08
BOOL BITM::isNotForSale() const {
    KindInfo *info = infoBank_c::get()->getKindInfo(getKind());
    if (info != NULL && info->mCheckNoPurchase) {
        return m_noPurchase;
    }
    return FALSE;
}

// 800C1C80
u16 BITM::getVersion() {
    return BITM_VERSION;
}

// 800C1C88
BOOL BITM::isValid() const {
    s16 version = static_cast<s16>(m_version);
    return static_cast<u16>(version) == BITM_VERSION;
}

// 800C1CA0. Nothing in the DOL calls it, yet the original link kept it.
#pragma force_active on
const char *getDevelopOnlyLabel() {
    static const char *label = "DEVELOP ONLY";
    return label;
}
#pragma force_active reset

// .rodata order: sKindAvailable, sFromNameIndex, sFtrFuncTypes, then the catalog tables.
// 80471B40
static const u8 sKindAvailable[0x80] = {
    1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
};

// 80471BC0: name index per From (getFromNameIndex)
static const u16 sFromNameIndex[FROM_COUNT] = {
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x97, 0x19C, 0x3, 0x3E8, 0x33C, 0x5,
    0x3, 0x3, 0x12C, 0x3E8, 0x457, 0x378, 0x19C, 0x53, 0x607, 0xC80, 0x3, 0x0,
    0x19C, 0x500, 0x145, 0x390, 0x320, 0x320, 0x320, 0x320, 0x3, 0x3, 0x3, 0x0,
    0x97, 0x0, 0x457, 0x457, 0x457, 0x0, 0x97, 0x97, 0x0, 0x4C8, 0xCA, 0x12F,
    0x1F9, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96,
    0x96, 0x96, 0x96, 0x4D, 0x65, 0x141, 0x258, 0x258, 0x96, 0x258, 0xFA, 0xFA,
    0xFA, 0xFA, 0x258, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x96, 0x97, 0x97,
    0x320, 0xFA, 0xFA, 0xFA, 0xFA, 0x58, 0x19C, 0x0,
};

// 80471C78: furniture function -> type (convFtrFunc)
static const u16 sFtrFuncTypes[0x41] = {
    0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x32, 0x32, 0x40,
    0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x2A, 0x2A, 0x2A, 0x2B, 0x2B, 0x2B, 0x2B,
    0x2B, 0x2C, 0x2C, 0x2C, 0x2D, 0x2D, 0x2D, 0x2E, 0x2F, 0x30, 0x30, 0x31, 0x3B,
    0x34, 0x35, 0x36, 0x3C, 0x3D, 0x43, 0x38, 0x37, 0x39, 0x3A, 0x42, 0x41, 0x33,
    0x3E, 0x3F, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x44, 0x45, 0x42, 0x46, 0x2C,
};

// 800C1CA8
int convFtrFunc(u32 func) {
    if (func < 0x41) {
        return sFtrFuncTypes[func];
    }
    return 0x21;
}

// 800C1CCC
int BITM::getFtrFuncType() const {
    int func = static_cast<u32>(m_ftrFunc) < 0x41 ? m_ftrFunc : 1;
    if (func != 0) {
        return convFtrFunc(func);
    }
    return convFtrFunc(1);
}

// 800C1CF8
const wchar_t *BITM::getName() const {
    switch (getLanguage()) {
    case LANG_US:
        return m_nameUs;
    case LANG_QC:
        return m_nameQc;
    case LANG_MX:
        return m_nameMx;
    case LANG_KR:
        return m_nameKr;
    case LANG_EN:
        return m_nameEn;
    case LANG_ES:
        return m_nameEs;
    case LANG_FR:
        return m_nameFr;
    case LANG_IT:
        return m_nameIt;
    case LANG_DE:
        return m_nameDe;
    default:
        return m_nameJp;
    }
}

// 800C1D90
int BITM::getDefArticle() const {
    switch (getLanguage()) {
    case LANG_US:
        return m_defUs;
    case LANG_QC:
        return m_defQc;
    case LANG_MX:
        return m_defMx;
    case LANG_KR:
        return 0;
    case LANG_EN:
        return m_defEn;
    case LANG_ES:
        return m_defEs;
    case LANG_FR:
        return m_defFr;
    case LANG_IT:
        return m_defIt;
    case LANG_DE:
        // The game reads the indefinite nibble here as well.
        return m_indefDe;
    default:
        return 0;
    }
}

// 800C1E48
int BITM::getIndefArticle() const {
    switch (getLanguage()) {
    case LANG_US:
        return m_indefUs;
    case LANG_QC:
        return m_indefQc;
    case LANG_MX:
        return m_indefMx;
    case LANG_KR:
        return 0;
    case LANG_EN:
        return m_indefEn;
    case LANG_ES:
        return m_indefEs;
    case LANG_FR:
        return m_indefFr;
    case LANG_IT:
        return m_indefIt;
    case LANG_DE:
        return m_indefDe;
    default:
        return 0;
    }
}

// 800C1F00
int BITM::getGender() const {
    switch (getLanguage()) {
    case LANG_US:
        return 0;
    case LANG_QC:
        return m_genderQc;
    case LANG_MX:
        return m_genderMx;
    case LANG_KR:
        return 0;
    case LANG_EN:
        return 0;
    case LANG_ES:
        return m_genderEs;
    case LANG_FR:
        return m_genderFr;
    case LANG_IT:
        return m_genderIt;
    case LANG_DE:
        return m_genderDe;
    default:
        return 0;
    }
}

// 800C1FB0
#define BITM_IS_FTR_KIND(kind) (kind == KIND_FTR || kind == KIND_CLOTH || kind == KIND_CAP || kind == KIND_ACC || kind == KIND_FOSSIL || kind == KIND_HANIWA || kind == KIND_PICTURE || kind == KIND_FAKE_PICTURE_BEFORE || kind == KIND_FAKE_PICTURE_AFTER)
BOOL BITM::isFurnitureLike() const {
    int kind = getKind();

    return BITM_IS_FTR_KIND(kind) != FALSE || kind == KIND_UMBRELLA;
}

// 800C2008
BOOL BITM::isAvailableInRegion() const {
    u32 value = 0;
    u32 region = static_cast<s8>(m_region);
    if (region < 5) {
        value = region;
    }
    if (value == 0) {
        return TRUE;
    }
    return getTownRegion() == value;
}

// 800C2068
int BITM::getSeriesGroup() const {
    u32 series = 0;
    if (static_cast<u32>(m_series) < SERIES_COUNT) {
        series = m_series;
    }
    Series *entry = infoBank_c::get()->getSeries(series);
    if (entry != NULL) {
        s8 group = entry->mGroup;
        return static_cast<u32>(group) < 4 ? group : 3;
    }
    return 3;
}

// 800C20DC
BOOL BITM::canAddToCatalog() const {
    int kind = getKind();
    BOOL ok = FALSE;
    switch (kind) {
    case KIND_WALL:
    case KIND_CARPET:
    case KIND_FTR:
    case KIND_CLOTH:
    case KIND_CAP:
    case KIND_ACC:
    case KIND_PAPER:
    case KIND_UMBRELLA:
        ok = TRUE;
        break;
    }
    if (ok) {
        return m_noPurchase == 0;
    }
    return FALSE;
}

// 800C214C
int BITM::getKindFlag6() const {
    KindInfo *info = infoBank_c::get()->getKindInfo(getKind());
    if (info != NULL) {
        return info->mFlag6;
    }
    return 0;
}

// 800C21AC
int BITM::getKindFlag5() const {
    KindInfo *info = infoBank_c::get()->getKindInfo(getKind());
    if (info != NULL) {
        return info->mFlag5;
    }
    return 0;
}

// 800C220C
int BITM::getKindFlag4() const {
    KindInfo *info = infoBank_c::get()->getKindInfo(getKind());
    if (info != NULL) {
        return info->mFlag4;
    }
    return 0;
}

const char *getKindName(u32 kind);

// 800C226C
const char *BITM::getKindName() const {
    return ::getKindName(getKind());
}

// 800C2288
BOOL isFurnitureKind(int kind) {
    switch (kind) {
    case KIND_FTR:
    case KIND_FOSSIL:
    case KIND_HANIWA:
    case KIND_PICTURE:
    case KIND_FAKE_PICTURE_BEFORE:
    case KIND_FAKE_PICTURE_AFTER:
        return TRUE;
    }
    return FALSE;
}

// 800C22CC
const char *BITM::getFgObjName(BOOL alt) const {
    u32 fgobj = m_fgobj;
    if (fgobj < 0x5A) {
        if (alt) {
            return sFgObjNamesAlt[fgobj];
        }
        return sFgObjNames[fgobj];
    }
    return sFgObjNamesAlt[0];
}

// 800C2318
u16 BITM::getFromNameIndex() const {
    int raw = m_from;
    u32 from = static_cast<u32>(raw) < FROM_COUNT ? raw : FROM_NONE;
    if (from < FROM_COUNT) {
        return sFromNameIndex[from];
    }
    return 0;
}

// 800C2354
int BITM::resolveColor(int color, BOOL usePlayer) const {
    if (color == 0 && getKind() == KIND_CAP) {
        if (usePlayer) {
            u8 *player = (u8 *)dPlayerMgr_c::getCurrentPlayerRaw();
            if (player != NULL) {
            switch (player[0x83EC]) {
            case 0:
                return 7;
            case 1:
                return 9;
            case 2:
                return 2;
            case 3:
                return 5;
            case 4:
                return 1;
            case 5:
                return 4;
            case 6:
                return 0xA;
            case 7:
                return 6;
            }
            }
        } else {
            return 9;
        }
    }
    return color;
}

// Static data, in the target's .bss/.sbss and __sinit order.
static indexTable_c s_indexTable;

// Global at 8074E490. Its constructor runs the (empty) debug registration.
class debugFlag_c {
public:
    debugFlag_c() {
        mEnabled = 0;
        fn_800C59E0(fn_800C59EC());
    }

    u8 mEnabled;
};

debugFlag_c s_debugFlag; // global: the item debug TU after this one reads it

// 800C242C
dsnPlttLoader_c *dsnPlttLoader_c::get() {
    static dsnPlttLoader_c s_loader;
    return &s_loader;
}

// 800C249C
void dsnPlttLoader_c::onLoaded() {
    infoBank_c::get()->setPalettes(nw4r::g3d::ResFile(mpData));
}

static infoBank_c s_infoBank;

// 800C24CC
infoBank_c *infoBank_c::get() {
    return &s_infoBank;
}

// 800C24D8
infoBank_c::infoBank_c() {
    mDlLoaded = 0;
    mpItems = NULL;
    mpSeries = NULL;
    mpNpcMsg = NULL;
    mNpcMsgCount = 0;
    mpNpcMsgBullfest = NULL;
    mNpcMsgBullfestCount = 0;
    mpKindInfo = NULL;
    for (int i = 0; i < 16; i++) {
        mpPalettes[i] = NULL;
    }
}

// 800C257C
BOOL infoBank_c::load(void *heap) {
    return dDvd::bank_c::load("/Item/item.arc", heap, 0);
}

// 800C2590
void infoBank_c::onLoaded() {
    u32 size;
    ARCInitHandle(mpData, &mHandle);
    mArcReady = 1;
    mpItems = reinterpret_cast<BITM *>(static_cast<u8 *>(getFile("item/item.bin", &size)) + 0x20);
    mpSeries = reinterpret_cast<Series *>(static_cast<u8 *>(getFile("item/series.bin", &size)) + 0x20);
    u32 *npcMsg = static_cast<u32 *>(getFile("item/npcMsg.bin", &size));
    mpNpcMsg = reinterpret_cast<NpcMsg *>(reinterpret_cast<u8 *>(npcMsg) + 0x20);
    mNpcMsgCount = *npcMsg;
    u32 *bullfest = static_cast<u32 *>(getFile("item/npcMsgBullfest.bin", &size));
    mpNpcMsgBullfest = reinterpret_cast<NpcMsgBullfest *>(reinterpret_cast<u8 *>(bullfest) + 0x20);
    mNpcMsgBullfestCount = *bullfest;
    mpKindInfo = reinterpret_cast<KindInfo *>(static_cast<u8 *>(getFile("item/kind.bin", &size)) + 0x20);
    s_indexTable.build();
}

// 800C2670
void infoBank_c::loadDlItems() {
    mDlLoaded = 1;
    s_indexTable.addDlItems();
}

// 800C2684
void infoBank_c::setDlItem(u16 baseId, u16 slot) {
    s_indexTable.setDlItem(baseId, slot);
}

// 800C2690
void infoBank_c::setPalettes(nw4r::g3d::ResFile file) {
    nw4r::g3d::ResFile res = file;
    for (u32 i = 0; i < 16; i++) {
        mpPalettes[i] = res.GetResPltt(i).GetPlttData();
    }
}

// 800C2704
const BITM *infoBank_c::getBITM(u16 index) const {
    if (mpItems != NULL) {
        if (index < ITEM_COUNT) {
            BITM *bitm = &mpItems[index];
            if (bitm->isValid()) {
                return bitm;
            }
            return mpItems;
        } else if (index < DL_ITEM_END) {
            dSaveDLItemList_c *dl = dSaveDLItemList_c::getRaw();
            dSaveDLItem_c *block = dl->getAt(getDlSlot(index));
            if (block != NULL) {
                const BITM *bitm = block->getValidBITM();
                if (bitm != NULL) {
                    if (bitm->isValid()) {
                        return bitm;
                    }
                    return mpItems;
                }
            }
            return NULL;
        }
    }
    return NULL;
}

// 800C27E0
const BITM *infoBank_c::getBITM(Item item) const {
    u16 index = getIndexFromItemId(item.mId);
    if (index < DL_ITEM_END) {
        const BITM *bitm = getBITM(index);
        if (bitm != NULL) {
            return bitm;
        }
        if (isRealItemId(item.mId)) {
            return mpItems;
        }
    }
    return NULL;
}

// 800C2870
Series *infoBank_c::getSeries(u32 series) {
    if (series < SERIES_COUNT) {
        return &mpSeries[series];
    }
    return mpSeries;
}

// 800C2890
NpcMsg *infoBank_c::getNpcMsg(u16 index) {
    if (mpNpcMsg != NULL && index < mNpcMsgCount) {
        return &mpNpcMsg[index];
    }
    return NULL;
}

// 800C28C0
NpcMsgBullfest *infoBank_c::getNpcMsgBullfest(u16 index) {
    if (mpNpcMsgBullfest != NULL && index < mNpcMsgBullfestCount) {
        return &mpNpcMsgBullfest[index];
    }
    return NULL;
}

// 800C28EC
KindInfo *infoBank_c::getKindInfo(u32 kind) {
    if (mpKindInfo != NULL && kind < KIND_COUNT) {
        return &mpKindInfo[kind];
    }
    return NULL;
}

// 800C2910
u16 infoBank_c::getIndexFromBaseId(u16 baseId) const {
    return s_indexTable.getIndex(baseId);
}

// 800C291C
u16 infoBank_c::getBaseId(u16 index) const {
    const BITM *bitm = getBITM(index);
    if (bitm != NULL) {
        int raw = bitm->m_baseId;
        u16 baseId = raw;
        if (baseId != INDEX_NONE) {
            if (baseId < BASE_ID_COUNT) {
                return baseId;
            }
            return getBaseId(ITEM_IDX_DUMMY);
        }
        return getBaseId(ITEM_IDX_DUMMY);
    }
    return getBaseId(ITEM_IDX_DUMMY);
}

// 800C2998
u16 infoBank_c::getItemId(u16 index) {
    return 0x9000 + (getBaseId(index) << 2);
}

// 800C29C8
u16 infoBank_c::getItemIdFromIndex(int index) {
    return getItemId(index);
}

// 800C29D0
u16 infoBank_c::getBaseIdFromItemId(u16 id) const {
    if (isRealItemId(id)) {
        return (id - 0x9000) >> 2;
    }
    return getBaseId(ITEM_IDX_DUMMY);
}

// 800C2A10
BOOL infoBank_c::isBuiltinBaseId(u16 baseId) {
    return getIndexFromBaseId(baseId) < DL_ITEM_FIRST;
}

// 800C2A3C
BOOL infoBank_c::isBuiltinItemId(u16 id) {
    if (isRealItemId(id)) {
        return isBuiltinBaseId(getBaseIdFromItemId(id));
    }
    return FALSE;
}

// 800C2AA8
u16 infoBank_c::getIndexFromItemId(u16 id) const {
    if (isRealItemId(id)) {
        u32 index = getIndexFromBaseId(getBaseIdFromItemId(id));
        if (index < DL_ITEM_END) {
            return index;
        } else {
            getCurrentScene();
            return INDEX_NONE;
        }
    }
    return INDEX_NONE;
}

static seeker_c s_seeker;

// 800C2B34
seeker_c *seeker_c::get() {
    return &s_seeker;
}

// 800C2B40
void seeker_c::add(u16 index) {
    if (index < DL_ITEM_END) {
        u32 byte = (index >> 3) % sizeof(mBits);
        mBits[byte] |= 1 << (index & 7);
    }
}

// 800C2B80
BOOL seeker_c::contains(u16 index) const {
    if (index < DL_ITEM_END) {
        u32 byte = (index / 8) % sizeof(mBits);
        return (mBits[byte] >> (index & 7)) & 1;
    }
    return FALSE;
}

// 800C2BC0
void seeker_c::searchItem(const Item &item, int flags, candCB_c *cb) {
    const BITM *bitm = infoBank_c::get()->getBITM(item);
    if (bitm != NULL) {
        search(bitm->getKind(), flags, cb);
    } else {
        memset(this, 0, sizeof(seeker_c));
    }
}

// 800C2C58
// Inlined into search(): accepts every item when there is no callback.
static inline BOOL checkCand(seeker_c::candCB_c *cb, infoBank_c *bank, const BITM *bitm, u16 index) {
    BOOL ok = TRUE;
    if (cb != NULL) {
        Item item(bank->getItemId(index));
        if (!cb->check(bitm, &item)) {
            ok = FALSE;
        }
    }
    return ok;
}

void seeker_c::search(int kind, int flags, candCB_c *cb) {
    BOOL any;
    memset(this, 0, sizeof(seeker_c));
    infoBank_c *bank = infoBank_c::get();

    if (flags & 2) {
        const u16 first = s_indexTable.getKindFirst(kind);
        const u16 last = s_indexTable.getKindLast(kind);
        if (first != INDEX_NONE && last != INDEX_NONE) {
            any = flags & 1;
            for (u16 i = first; i <= last; i++) {
                const BITM *bitm = bank->getBITM(i);
                if (bitm == NULL) {
                    continue;
                }
                if (!any && !bitm->isAvailableInRegion()) {
                    continue;
                }
                if (checkCand(cb, bank, bitm, i)) {
                    add(i);
                    mCount++;
                }
            }
        }
    }

    if (flags & 4) {
        dSaveDLItemList_c::getRaw();
        any = flags & 1;
        for (u32 j = DL_ITEM_FIRST; j < DL_ITEM_END; j++) {
            const BITM *bitm = bank->getBITM(static_cast<u16>(j));
            if (bitm == NULL) {
                continue;
            }
            if (kind != KIND_NONE && kind != bitm->getKind()) {
                continue;
            }
            if (!any && !bitm->isAvailableInRegion()) {
                continue;
            }
            if (checkCand(cb, bank, bitm, j)) {
                add(j);
                mCount++;
            }
        }
    }
}

// 800C2E6C
Item seeker_c::getNth(u32 n) const {
    if (n < mCount) {
        u32 found = 0;
        for (u32 i = 0; i < DL_ITEM_END; i++) {
            if (!contains(i)) {
                continue;
            }
            if (found == n) {
                Item item(infoBank_c::get()->getItemId(i));
                if (item.getKind() == 9) {
                    item = item.getVariant(3);
                }
                return item;
            }
            found++;
        }
    }
    return Item();
}

// 800C2F34
Item seeker_c::getRandom() const {
    if (mCount != 0) {
        return getNth(cM::rndInt(mCount));
    }
    return Item();
}

// 800C2F9C
int seeker_c::find(const Item &item) const {
    Item key = item.withVariant(0);
    int found = 0;
    for (u32 i = 0; i < DL_ITEM_END; i++) {
        if (!contains(i)) {
            continue;
        }
        if (infoBank_c::get()->getItemId(i) == key.getId()) {
            return found;
        }
        found++;
    }
    return -1;
}

// 800C303C
int seeker_c::findLike(Item item) {
    searchItem(item, 2, NULL);
    return find(item);
}

// 800C3088
int seeker_c::findInSeries(Item item) {
    if (isRealItemId(item.mId)) {
        const BITM *bitm = infoBank_c::get()->getBITM(Item(item.mId));
        if (bitm != NULL) {
            u32 series = 0;
            if (static_cast<u32>(bitm->m_series) < SERIES_COUNT) {
                series = bitm->m_series;
            }
            int kind = bitm->getKind();
            seriesCandCB_c cb(series, Item(), FALSE);
            search(kind, 7, &cb);
            int index = find(item);
            if (index >= 0) {
                return index;
            }
        }
    }
    return 0;
}

// 800C3180
u32 seeker_c::searchSeries(int series, int kind) {
    seriesCandCB_c cb(series, Item(), FALSE);
    search(kind, 7, &cb);
    return mCount;
}

// 800C31E8
Item seeker_c::getNthInSeries(u32 n, int series, int kind) {
    seriesCandCB_c cb(series, Item(), FALSE);
    search(kind, 7, &cb);
    return getNth(n);
}

// 800C3278
int seeker_c::findFossil(Item item) {
    fossilCandCB_c cb(item);
    get()->searchItem(item, 6, &cb);
    return find(item);
}

// 800C32E0
u32 seeker_c::searchFossil(int fossil) {
    fossilCandCB_c cb(fossil);
    get()->search(KIND_FOSSIL, 6, &cb);
    return mCount;
}

// 800C3330
Item seeker_c::getRandomFossil(int fossil) {
    fossilCandCB_c cb(fossil);
    get()->search(KIND_FOSSIL, 6, &cb);
    return getRandom();
}

// 800C3394
clothCandCB_c::clothCandCB_c(Item item, int style, int prevStyle, BOOL excludeNoSale) {
    set(item, style, prevStyle, excludeNoSale);
}

// 800C33E8
void clothCandCB_c::set(Item item, int style, int prevStyle, BOOL excludeNoSale) {
    mStyle = style;
    mExclude = item;
    mPrevStyle = prevStyle;
    mExcludeNoSale = excludeNoSale;
}

// 800C3400
BOOL clothCandCB_c::check(const BITM *bitm, Item *item) const {
    int style = 0;
    u32 raw = bitm->m_style;
    if (raw < 0xB) {
        style = raw;
    }
    if (item->isSame(mExclude)) {
        return FALSE;
    }
    if (mExcludeNoSale && bitm->isNotForSale()) {
        return FALSE;
    }
    if (mPrevStyle == style && style != 0xA) {
        return FALSE;
    }
    if (mStyle == 0xA || mStyle == style) {
        return TRUE;
    }
    return FALSE;
}

static inline int getFrom(const BITM *bitm) {
    int raw = bitm->m_from;
    return static_cast<u32>(raw) < FROM_COUNT ? raw : FROM_NONE;
}

// 800C34D0
BOOL musicCandCB_c::check(const BITM *bitm, Item *item) const {
    static seeker_c s_seeker;
    int index = s_seeker.findLike(*item);
    if (index != -1) {
        BOOL match = FALSE;
        if (mExcludeFrom != FROM_COUNT && mExcludeFrom == getFrom(bitm)) {
            return FALSE;
        }
        for (int i = 0; i < 5; i++) {
            if (mFrom[i] != FROM_COUNT && mFrom[i] == getFrom(bitm)) {
                match = TRUE;
                break;
            }
        }
        if (match && static_cast<u32>(index) < 0x4E) {
            if (mpOwned != NULL) {
                return !((mpOwned[index >> 3] >> (index & 7)) & 1);
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 800C36DC
BOOL fromCandCB_c::check(const BITM *bitm, Item *item) const {
    if (mFrom == FROM_COUNT || mFrom == getFrom(bitm)) {
        return TRUE;
    }
    return FALSE;
}

// 800C3718
void fossilCandCB_c::set(Item item) {
    const BITM *bitm = infoBank_c::get()->getBITM(item);
    if (bitm != NULL) {
        u32 value = 0;
        u32 fossil = bitm->m_fossil;
        if (fossil < 0x1C) {
            value = fossil;
        }
        mFossil = value;
    } else {
        mFossil = 0;
    }
}

// 800C3784
BOOL fossilCandCB_c::check(const BITM *bitm, Item *item) const {
    if (mFossil == 0) {
        if (bitm->getKind() == KIND_FOSSIL) {
            return TRUE;
        }
        return FALSE;
    }
    if (mFossil == bitm->getFossil()) {
        return TRUE;
    }
    return FALSE;
}

// 800C37F0
BOOL colorCandCB_c::check(const BITM *bitm, Item *item) const {
    u32 colorA = bitm->m_ftrColorA;
    int color;
    if (colorA < 0xF) {
        color = bitm->resolveColor(colorA, TRUE);
    } else {
        color = bitm->resolveColor(0, TRUE);
    }
    BOOL match = mColor == color;
    if (!match) {
        s8 colorB = bitm->m_ftrColorB;
        if (static_cast<u32>(colorB) < 0xF) {
            color = bitm->resolveColor(colorB, TRUE);
        } else {
            color = bitm->resolveColor(0, TRUE);
        }
        match = mColor == color;
    }
    return match;
}

// 800C38AC
BOOL seriesCandCB_c::check(const BITM *bitm, Item *item) const {
    if (mExclude.mId != ITEM_ID_NONE && item->isSame(mExclude)) {
        return FALSE;
    }
    if (mExcludeNoSale && bitm->isNotForSale()) {
        return FALSE;
    }
    return mSeries == bitm->getSeries();
}

// 800C3954
BOOL sizeCandCB_c::check(const BITM *bitm, Item *item) const {
    return mSize == bitm->getFtrSize();
}

// 800C3984
BOOL ftrSeCandCB_c::check(const BITM *bitm, Item *item) const {
    return mSfx == bitm->m_ftrSfx;
}

// 800C399C
BOOL newOldCandCB_c::check(const BITM *bitm, Item *item) const {
    return bitm->getNewOld() == mValue;
}

// 800C39DC
BOOL adultKiddyCandCB_c::check(const BITM *bitm, Item *item) const {
    return bitm->getAdultKiddy() == mValue;
}


// 800C3A1C
BOOL categoryQ5CandCB_c::check(const BITM *bitm, Item *item) const {
    return getCategoryQ5(*item) == mCategory;
}

static resList_c s_resList;

// 800C3A5C
void resLoader_c::link(u16 index) {
    if (mIndex == INDEX_NONE) {
        mIndex = index;
        nw4r::ut::List_Append(&s_resList.mList, this);
        if (index < ITEM_COUNT) {
            s_resList.mState[index] = 1;
        }
    }
}

// 800C3AC4
void resLoader_c::unlink() {
    if (isLinked()) {
        if (mIndex < ITEM_COUNT) {
            s_resList.mState[mIndex] = 0;
        }
        mIndex = INDEX_NONE;
        nw4r::ut::List_Remove(&s_resList.mList, this);
    }
}

// 800C3B10
void resLoader_c::reset() {
    mpBITM = NULL;
    mFromArchive = 1;
    mItemId = ITEM_ID_NONE;
    mSize = -1;
    mIndex = INDEX_NONE;
}

// 800C3B40
resLoader_c::resLoader_c() {
    reset();
}

// 800C3B84
resLoader_c::~resLoader_c() {
    unlink();
}

// 800C3BFC
BOOL resLoader_c::loadFromMemory(const void *src, u32 size, void *heap) {
    // EGG::Heap::alloc(size, 0x20)
    struct heap_c {
        virtual void v0();
        virtual void v1();
        virtual void v2();
        virtual void *alloc(u32 size, int align);
    };
    if (src != NULL) {
        mpData = static_cast<heap_c *>(heap)->alloc(size, 0x20);
        if (mpData != NULL) {
            mpHeap = heap;
            memcpy(mpData, src, size);
            mSize = size;
            DCFlushRange(mpData, size);
            onLoaded();
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// 800C3CC0
BOOL resLoader_c::loadDesign(void *design, void *heap, s16 width, s16 height) {
    void *tex = ((dDesign_c *)design)->getTexture();
    void *pltt = ((dDesign_c *)design)->getPalette();
    return loadTexture(tex, pltt, heap, width, height);
}

// 800C3D30
BOOL resLoader_c::loadDesignInPlace(void *design, void *heap, s16 width, s16 height) {
    void *tex = ((dDesign_c *)design)->getTexture();
    void *pltt = ((dDesign_c *)design)->getPalette();
    return bindTexture(tex, pltt, heap, width, height);
}

// 800C3DA0
BOOL resLoader_c::loadTexture(const void *tex, const void *pltt, void *heap, s16 width, s16 height) {
    if (mpData != NULL) {
        return TRUE;
    }
    Item item(0x372);
    if (loadItem(item, heap)) {
        nw4r::g3d::ResFile file(mpData);
        if (tex != NULL) {
            nw4r::g3d::ResTex resTex = file.GetResTex(0);
            void *data = const_cast<void *>(resTex.GetTexData());
            u32 size = (resTex.GetWidth() * resTex.GetHeight()) >> 1;
            memcpy(data, tex, size);
            if (width != -1) {
                resTex.ref().width = width;
            }
            if (height != -1) {
                resTex.ref().height = height;
            }
            DCStoreRangeNoSync(data, size);
            resTex.EndEdit();
        }
        if (pltt != NULL) {
            nw4r::g3d::ResPltt resPltt = file.GetResPltt(0);
            u16 *data = resPltt.GetPlttData();
            memcpy(data, pltt, 0x20);
            DCStoreRangeNoSync(data, 0x20);
            resPltt.DCStore(false);
        }
        return TRUE;
    }
    return FALSE;
}

// 800C3F1C
BOOL resLoader_c::bindTexture(void *tex, void *pltt, void *heap, s16 width, s16 height) {
    if (mpData != NULL) {
        return TRUE;
    }
    Item item(0x372);
    if (loadItem(item, heap)) {
        nw4r::g3d::ResFile file(mpData);
        if (tex != NULL) {
            nw4r::g3d::ResTex resTex = file.GetResTex(0);
            if (width != -1) {
                resTex.ref().width = width;
            }
            if (height != -1) {
                resTex.ref().height = height;
            }
            resTex.ref().toTexData = static_cast<u8 *>(tex) - reinterpret_cast<u8 *>(&resTex.ref());
            DCStoreRangeNoSync(tex, (resTex.GetWidth() * resTex.GetHeight()) >> 1);
            resTex.EndEdit();
        }
        if (pltt != NULL) {
            nw4r::g3d::ResPltt resPltt = file.GetResPltt(0);
            resPltt.ref().toPlttData = static_cast<u8 *>(pltt) - reinterpret_cast<u8 *>(&resPltt.ref());
            DCStoreRangeNoSync(pltt, 0x20);
            resPltt.DCStore(false);
        }
        return TRUE;
    }
    return FALSE;
}

// 800C4054
BOOL resLoader_c::loadItem(Item item, void *heap) {
    return loadIndex(infoBank_c::get()->getIndexFromItemId(item.mId), heap);
}

// 800C40B4
BOOL resLoader_c::loadIndex(u16 index, void *heap) {
    if (mpData != NULL) {
        return mpData != NULL;
    }
    if (mpBITM == NULL) {
        infoBank_c *bank = infoBank_c::get();
        mItemId = bank->getItemId(index);
        mpBITM = bank->getBITM(index);
    }
    if (mpBITM != NULL) {
        if (!mpBITM->m_hasRes) {
            return TRUE;
        }
    } else {
        return TRUE;
    }

    if (index < ITEM_COUNT) {
        if (mFromArchive) {
            char path[50];
            snprintf(path, sizeof(path), "item/Res/%s%d.brres", mpBITM->getKindName(), s_indexTable.getIndexInKind(index));
            u32 size;
            void *src = infoBank_c::get()->getFile(path, &size);
            if (loadFromMemory(src, size, heap)) {
                return TRUE;
            }
            mFromArchive = 0;
        }

        if (mIndex == INDEX_NONE) {
            int state = index < ITEM_COUNT ? s_resList.mState[index] : 0;
            if (state == 1) {
                return FALSE;
            }
            if (state == 2) {
                resLoader_c *other = static_cast<resLoader_c *>(nw4r::ut::List_GetNext(&s_resList.mList, NULL));
                while (other != NULL) {
                    resLoader_c *next = static_cast<resLoader_c *>(nw4r::ut::List_GetNext(&s_resList.mList, other));
                    void *data = other->getData();
                    if (index == other->mIndex && data != NULL) {
                        s32 size = other->getDataSize();
                        if (data != NULL && loadFromMemory(data, size, heap)) {
                            return TRUE;
                        }
                        return FALSE;
                    }
                    other = next;
                }
                return FALSE;
            }
        }

        char path[50];
        snprintf(path, sizeof(path), "/Item/%s/%s%d.brres", mpBITM->getKindName(), mpBITM->getKindName(),
                 s_indexTable.getIndexInKind(index));
        link(index);
        return load(path, heap, 0);
    }

    const dSaveDLItemList_c *dl = dSaveDLItemList_c::get();
    u32 slot = getDlSlot(index);
    if (slot < DL_ITEM_COUNT) {
        dSaveDLItem_c *block = dl->getAt(slot);
        if (block != NULL) {
            mpData = block->loadArchive((EGG::Heap *)heap);
            if (mpData != NULL) {
                mpHeap = heap;
                mSize = block->getArchiveSize();
                onLoaded();
            }
        }
        return mpData != NULL;
    }
    return loadIndex(ITEM_IDX_DUMMY, heap);
}

// Not in the DOL: nothing calls it, so the linker dead-strips it. Its body is unknown; only its
// error string ("furniture SE error") survives, unreferenced, in .data at 804EA4A8.
void reportFtrSeError() {
    OSReport("家具SEエラー");
}

// 800C43B8
bool resLoader_c::release() {
    if (unload(FALSE)) {
        unlink();
        reset();
        return TRUE;
    }
    return FALSE;
}

// 800C440C
void resLoader_c::onLoaded() {
    dDvd::brresBank_c::onLoaded();
    mpBITM = NULL;
    if (mIndex < ITEM_COUNT) {
        s_resList.mState[mIndex] = 2;
    }
}

// 800C4488
s32 resLoader_c::getDataSize() {
    return mSize >= 0 ? mSize : getSize();
}

// 800C44C0
void resLoader_c::replaceTexture(void *tex, void *pltt, u32 texIdx, u32 plttIdx, u16 width, u16 height) {
    nw4r::g3d::ResFile file(mpData);
    if (!file.IsValid()) {
        return;
    }
    if (tex != NULL && texIdx < file.GetResTexNumEntries()) {
        nw4r::g3d::ResTex resTex = file.GetResTex(texIdx);
        resTex.ref().toTexData = static_cast<u8 *>(tex) - reinterpret_cast<u8 *>(&resTex.ref());
        resTex.ref().width = width;
        resTex.ref().height = height;
        DCStoreRangeNoSync(tex, static_cast<u32>(width * height) >> 1);
        resTex.EndEdit();
    }
    if (pltt != NULL && plttIdx < file.GetResPlttNumEntries()) {
        nw4r::g3d::ResPltt resPltt = file.GetResPltt(plttIdx);
        resPltt.ref().toPlttData = static_cast<u8 *>(pltt) - reinterpret_cast<u8 *>(&resPltt.ref());
        DCStoreRangeNoSync(pltt, 0x20);
        resPltt.DCStore(false);
    }
}

// 800C45B8
makeSendData_c::makeSendData_c(u32 count, u8 *exclude) {
    mCount = count;
    mpExclude = exclude;
}

BOOL isDlItemMarked(const u8 *mask, u16 id);

// 800C45D0
u32 makeSendData_c::countSendable() const {
    dSaveDLItemList_c *dl = dSaveDLItemList_c::getRaw();
    u32 count = 0;
    for (u32 i = 0; i < mCount; i++) {
        const Item item = getItem(i);
        BOOL ok = item.isValid() && dl->find(item);
        if (ok && (mpExclude == NULL || !isDlItemMarked(mpExclude, item.mId))) {
            count++;
        }
    }
    return count;
}

// 800C4698
void *makeSendData_c::getNthSendable(u32 n) const {
    dSaveDLItemList_c *dl = dSaveDLItemList_c::getRaw();
    u32 found = 0;
    for (u32 i = 0; i < mCount; i++) {
        Item item = getItem(i);
        void *block = dl->find(item);
        if (block != NULL && (mpExclude == NULL || !isDlItemMarked(mpExclude, item.mId))) {
            if (n == found) {
                return block;
            }
            found++;
        }
    }
    return NULL;
}

// 800C4760
Item makePlEquipSendData_c::getItem(u32 index) const {
    if (mpPlayer != NULL) {
        Item items[2] = {mpPlayer->mEquipment.mHat, mpPlayer->mEquipment.mAcc};
        if (index < 2) {
            return items[index];
        }
    }
    return Item();
}

// 800C47B0
u32 countPlEquipSendable() {
    makePlEquipSendData_c data(dPlayerMgr_c::getCurrentPlayerRaw());
    return data.countSendable();
}

// 800C4804
void *getPlEquipSendable(u32 n) {
    makePlEquipSendData_c data(dPlayerMgr_c::getCurrentPlayerRaw());
    return data.getNthSendable(n);
}

// 800C4868
BOOL hasDlItem(void *block) {
    Item item = dSaveDLItemList_c::get()->add((dSaveDLItem_c *)block, 0);
    return item.mId != ITEM_ID_NONE;
}

// 800C48C8
Item makeToCstmSendData_c::getItem(u32 index) const {
    return dSaveDLItemList_c::getRaw()->getItemAt(index);
}

// 800C4910
u32 countToCstmSendable(u8 *exclude) {
    makeToCstmSendData_c data(exclude);
    return data.countSendable();
}

// 800C4950
void *getToCstmSendable(u32 n, u8 *exclude) {
    makeToCstmSendData_c data(exclude);
    return data.getNthSendable(n);
}

// 800C49A0
BOOL isDlBlockUsed(void *block) {
    return hasDlItem(block);
}

// 800C49A4
BOOL markDlItem(u8 *mask, u16 id) {
    Item item;
    item = id;
    s32 slot = dSaveDLItemList_c::getRaw()->getSlot(&item);
    if (slot >= 0) {
        mask[(slot >> 3) & 0x1F] |= 1 << (slot & 7);
        return TRUE;
    }
    return FALSE;
}

// 800C4A0C
BOOL isDlItemMarked(const u8 *mask, u16 id) {
    Item item;
    item = id;
    s32 slot = dSaveDLItemList_c::getRaw()->getSlot(&item);
    if (slot >= 0) {
        return (mask[(slot >> 3) & 0x1F] >> (slot & 7)) & 1;
    }
    return FALSE;
}

// 800C4A68
void buildDlItemMask(u8 *mask) {
    memset(mask, 0, 0x20);
    dSaveDLItemList_c *dl = dSaveDLItemList_c::getRaw();
    for (u32 slot = 0; slot < DL_ITEM_COUNT; slot++) {
        Item item = dl->getItemAt(slot);
        if (item.mId != ITEM_ID_NONE) {
            markDlItem(mask, item.mId);
        }
    }
}

// 800C4AE8
dlBlockList_c::dlBlockList_c(u8 *blocks, u32 count) {
    mpBlocks = blocks;
    mCount = count;
}

// 800C4AF4
BOOL dlBlockList_c::add(void *block) {
    if (block != NULL) {
        const BITM *bitm = ((dSaveDLItem_c *)block)->getBITM();
        for (u8 *p = mpBlocks; p < mpBlocks + (mCount << 13); p += 0x2000) {
            if (((dSaveDLItem_c *)p)->isBITM()) {
                const BITM *otherBitm = ((dSaveDLItem_c *)p)->getBITM();
                int rawOther = otherBitm->m_baseId;
                u16 other = rawOther;
                int rawMine = bitm->m_baseId;
                u16 mine = rawMine;
                if (mine == other) {
                    return TRUE;
                }
            } else {
                ((dSaveDLItem_c *)p)->copy((dSaveDLItem_c *)block);
                return TRUE;
            }
        }
        return FALSE;
    }
    return FALSE;
}

// 800C4BC8
BOOL dlBlockList_c::addItem(Item item) {
    if (isRealItemId(item.mId)) {
        const dSaveDLItemList_c *dl = dSaveDLItemList_c::get();
        void *block = dl->find(item);
        if (block != NULL && add(block)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800C4C60
void dlBlockList_c::clear() {
    if (mpBlocks != NULL && mCount != 0) {
        memset(mpBlocks, 0, mCount << 13);
    }
}

// 800C4C8C
int dlBlockList_c::addFromPlayer(dPrivateData_c *player) {
    int count = 0;
    clear();
    Item *pocket = player->mPockets;
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++, pocket++) {
        if (addItem(*pocket)) {
            count++;
        }
    }
    for (int i = 0; i < PLAYER_MAIL_COUNT; i++) {
        if (addItem(player->mLetters[i].getPresent())) {
            count++;
        }
    }
    if (addItem(player->mEquipment.mHat)) {
        count++;
    }
    if (addItem(player->mEquipment.mAcc)) {
        count++;
    }
    if (addItem(player->mEquipment.mHeld)) {
        count++;
    }
    return count;
}

// 800C4DA0
int dlBlockList_c::countValid() {
    int count = 0;
    dSaveDLItemList_c *dl = dSaveDLItemList_c::get();
    for (u8 *p = mpBlocks; p < mpBlocks + (mCount << 13); p += 0x2000) {
        if (((dSaveDLItem_c *)p)->isUsed()) {
            Item item = dl->add((dSaveDLItem_c *)p, 0);
            if (item.mId != ITEM_ID_NONE) {
                count++;
            }
        }
    }
    return count;
}

// 800C4E48
int dlBlockList_c::countOwned(dPrivateData_c *player) {
    int count = 0;
    dSaveDLItemList_c::get();
    for (u8 *p = mpBlocks; p < mpBlocks + (mCount << 13); p += 0x2000) {
        if (((dSaveDLItem_c *)p)->isUsed()) {
            Item item = ((dSaveDLItem_c *)p)->getItem();
            if (item.mId != ITEM_ID_NONE && player->pickUp(&item, TRUE)) {
                count++;
            }
        }
    }
    return count;
}

// 800C4F00
int addPlayerDlItems(u8 *blocks, dPrivateData_c *player) {
    dlBlockList_c list(blocks, 0x1C);
    return list.addFromPlayer(player);
}

// 800C4F44
int countPlayerDlItems(u8 *blocks) {
    dlBlockList_c list(blocks, 0x1C);
    return list.countValid();
}

int countOwnedDlItems(u8 *blocks, dPrivateData_c *player);

// 800C4F78
int countTownDlItems(u8 *blocks, dPrivateData_c *player) {
    dlBlockList_c list(blocks, 0xF);
    int count = list.countValid();
    countOwnedDlItems(blocks, player);
    return count;
}

// 800C4FE0
int countOwnedDlItems(u8 *blocks, dPrivateData_c *player) {
    dlBlockList_c list(blocks, 0xF);
    return list.countOwned(player);
}

// 800C5024
Item makeItemFromBaseId(u16 baseId) {
    if (baseId < BASE_ID_COUNT) {
        return Item(static_cast<u16>(0x9000 + (baseId << 2)));
    }
    return Item();
}

// 800C5050
static void addCatalogList(const u16 *table, u32 count, const u8 *mask, dCatalog_c *catalog) {
    const u16 *entry = table;
    for (u32 i = 0; i < count; i++) {
        u32 byte = i >> 3;
        u32 bit = i & 7;
        if (*entry != INDEX_NONE) {
            Item item = makeItemFromBaseId(*entry);
            if (item.mId != ITEM_ID_NONE && ((mask[byte] >> bit) & 1)) {
                const BITM *bitm = infoBank_c::get()->getBITM(Item(item.mId));
                if (bitm != NULL && bitm->canAddToCatalog()) {
                    catalog->registerItem(item.mId, FALSE);
                }
            }
        }
        entry++;
    }
}

// 800C5110
static void addCatalogRecords(const u16 *table, u32 count, const u8 *mask, dCatalog_c *catalog) {
    u32 i;
    const u16 *entry = table + 3;
    u32 n = 3;
    i = 0;
    for (; n < count; n += 4) {
        u32 byte = i >> 3;
        u32 bit = i & 7;
        if (*entry != INDEX_NONE) {
            Item item = makeItemFromBaseId(*entry);
            if (item.mId != ITEM_ID_NONE && ((mask[byte] >> bit) & 1)) {
                const BITM *bitm = infoBank_c::get()->getBITM(Item(item.mId));
                if (bitm != NULL && bitm->canAddToCatalog()) {
                    catalog->registerItem(item.mId, FALSE);
                }
            }
        }
        i++;
        entry += 4;
    }
}

// 80471CFC
static const u16 sCatalogFurniture[0x6E9] = {
    0x9C4, 0x9C5, 0x9C6, 0x9C7, 0x9C8, 0x9C9, 0x9CA, 0x9CB, 0x9CC, 0x9CD,
    0x9CE, 0x9CF, 0x9D0, 0x9D1, 0x9D2, 0x9D3, 0x9D4, 0x9D5, 0x9D6, 0x9D7,
    0x9D8, 0x9D9, 0x9DA, 0x9DB, 0x9DC, 0x9DD, 0x9DE, 0x9DF, 0x9E0, 0x9E1,
    0x9E2, 0x9E3, 0x9E4, 0x9E5, 0x9E6, 0x9E7, 0x9E8, 0x9E9, 0x9EA, 0x9EB,
    0x9EC, 0x9ED, 0x9EE, 0x9EF, 0x9F0, 0x9F1, 0x9F2, 0x9F3, 0x9F4, 0x9F5,
    0x9F6, 0x9F7, 0x9F8, 0x9F9, 0x9FA, 0x9FB, 0x9FC, 0x9FD, 0x9FE, 0x9FF,
    0xA00, 0xA01, 0xA02, 0xA03, 0xA04, 0xA05, 0xA06, 0xA07, 0xA08, 0xA09,
    0xA0A, 0xA0B, 0xA0C, 0xA0D, 0xA0E, 0xA0F, 0xA10, 0xA11, 0xA12, 0xA13,
    0xA14, 0xA15, 0xA16, 0xA17, 0xA18, 0xA19, 0xA1A, 0xA1B, 0xA1C, 0xA1D,
    0xA1E, 0xA1F, 0xA20, 0xA21, 0xA22, 0xA23, 0xA24, 0xA25, 0xA26, 0xA27,
    0xA28, 0xA29, 0xA2A, 0xA2B, 0xA2C, 0xA2D, 0xA2E, 0xA2F, 0xA30, 0xA31,
    0xA32, 0xA33, 0xA34, 0xA35, 0xA36, 0xA37, 0xA38, 0xA39, 0xA3A, 0xA3B,
    0xA3C, 0xA3D, 0xA3E, 0xA3F, 0xA40, 0xA41, 0xA42, 0xA43, 0xA44, 0xA45,
    0xA46, 0xA47, 0xA48, 0xA49, 0xA4A, 0xA4B, 0xA4C, 0xA4D, 0xA4E, 0xA4F,
    0xAAA, 0xAAB, 0xAAC, 0xAAD, 0xAAE, 0xAAF, 0xAB0, 0xAB1, 0xAB2, 0xAB3,
    0xAB4, 0xAB5, 0xAB6, 0xAB7, 0xAB8, 0xAB9, 0xABA, 0xABB, 0xABC, 0xABD,
    0xABE, 0xABF, 0xAC0, 0xAC1, 0xAC2, 0xAC3, 0xAC4, 0xAC5, 0xAC6, 0xAC7,
    0xAC8, 0xAC9, 0xACA, 0xACB, 0xACC, 0xACD, 0xACE, 0xACF, 0xAD0, 0xAD1,
    0xAD2, 0xAD3, 0xAD4, 0xAD5, 0xAD6, 0xAD7, 0xAD8, 0xAD9, 0xADA, 0xADB,
    0xADC, 0xADD, 0xADE, 0xADF, 0xAE0, 0xAE1, 0xAE2, 0xAE3, 0xAE4, 0xAE5,
    0xAE6, 0xAE7, 0xFFFF, 0xAE8, 0xAE9, 0xAEA, 0xAEB, 0xAEC, 0xAED, 0xAEE,
    0xAEF, 0xAF0, 0xAF1, 0xAF2, 0xAF3, 0xAF4, 0xAF5, 0xAF6, 0xAF7, 0xAF8,
    0xAF9, 0xAFA, 0xAFB, 0xAFC, 0xAFD, 0xAFE, 0xAFF, 0xB00, 0xB01, 0xB02,
    0xB03, 0xB04, 0xB05, 0xB06, 0xB07, 0xB08, 0xB09, 0xB0A, 0xB18, 0xB14,
    0xB15, 0xB16, 0xB17, 0xB20, 0xB21, 0xB22, 0xB23, 0xB24, 0xB28, 0xB29,
    0xB2A, 0xB2B, 0xB2C, 0xB2D, 0xB2E, 0xB2F, 0xB30, 0xB31, 0xB32, 0xB33,
    0xB34, 0xB35, 0xB36, 0xB37, 0xB38, 0xB39, 0xB3A, 0xB3B, 0xB3C, 0xB3D,
    0xB3E, 0xB3F, 0xB40, 0xB41, 0xB42, 0xB43, 0xB44, 0xB45, 0xB46, 0xB47,
    0xB48, 0xB49, 0xB4A, 0xB58, 0xB59, 0xB5A, 0xB5B, 0xB56, 0xB57, 0xB4B,
    0xB4C, 0xB5C, 0xB5D, 0xC60, 0xC61, 0xC62, 0xC63, 0xB65, 0xB66, 0xB67,
    0xB68, 0xB69, 0xB6A, 0xB6B, 0xB6C, 0xB6D, 0xB6E, 0xB6F, 0xB70, 0xB71,
    0xB72, 0xB73, 0xB74, 0xB75, 0xB76, 0xB77, 0xB78, 0xB79, 0xB7A, 0xB7B,
    0xB7C, 0xB7D, 0xB7E, 0xB7F, 0xB80, 0xB99, 0xB9A, 0xB9B, 0xB9C, 0xB9D,
    0xB9E, 0xB9F, 0xBA0, 0xBA1, 0xBA2, 0xBA3, 0xBA4, 0xBA5, 0xBA7, 0xBA8,
    0xBA9, 0xBAA, 0xBAB, 0xBAC, 0xBAD, 0xBAE, 0xBAF, 0xBB0, 0xBB1, 0xBB2,
    0xBB3, 0xBB4, 0xBB5, 0xBB6, 0xBB7, 0xBB9, 0xBBA, 0xBBC, 0xBBB, 0xBBD,
    0xBBE, 0xBBF, 0xBC0, 0xBC1, 0xBC2, 0xBC3, 0xBC6, 0xBC7, 0xBC8, 0xBC9,
    0xBCA, 0xBCB, 0xBCC, 0xBCD, 0xBCE, 0xBCF, 0xBD0, 0xBD1, 0xBD3, 0xBD4,
    0xBD5, 0xBD6, 0xBD7, 0xBD8, 0xBD9, 0xBDA, 0xBDB, 0xB81, 0xB82, 0xB83,
    0xB84, 0xB85, 0xB86, 0xB87, 0xB88, 0xB89, 0xB8A, 0xB8B, 0xB8C, 0xB8D,
    0xB8E, 0xB8F, 0xB90, 0xB91, 0xB92, 0xB93, 0xB94, 0xB95, 0xB96, 0xB97,
    0xB98, 0xBDC, 0xBDD, 0xBDE, 0xBDF, 0xBE0, 0xBE1, 0xBE2, 0xBE3, 0xBE5,
    0xBE7, 0xBE8, 0xBE9, 0xBEA, 0xBE4, 0xBEB, 0xBEC, 0xBED, 0xBEE, 0xBEF,
    0xBF0, 0xBF1, 0xBF2, 0xBF3, 0xBF4, 0xBF5, 0xBF6, 0xBF7, 0xBF8, 0xBF9,
    0xBFA, 0xBFB, 0xBFC, 0xBFD, 0xBFE, 0xBFF, 0xC00, 0xC01, 0xC71, 0xC02,
    0xC03, 0xC04, 0xC05, 0xC06, 0xC07, 0xC08, 0xC12, 0xC0D, 0xC0E, 0xC0A,
    0xC10, 0xC13, 0xC14, 0xC09, 0xC0B, 0xC0C, 0xC0F, 0xC11, 0xC15, 0xC16,
    0xC17, 0xC18, 0xC19, 0xC1A, 0xC1B, 0xC1C, 0xC1D, 0xC1E, 0xC1F, 0xC20,
    0xC23, 0xC24, 0xC25, 0xC26, 0xFFFF, 0xC27, 0xC28, 0xC69, 0xC6A, 0xC29,
    0xC2A, 0xBD2, 0xC2B, 0xC2D, 0xB4D, 0xC2F, 0xC30, 0xC31, 0xC34, 0xC35,
    0xC36, 0xC37, 0xC38, 0xC70, 0xC39, 0xC3A, 0xC3B, 0xC3C, 0xC3D, 0xFFFF,
    0xC3E, 0xC40, 0xC6D, 0xC41, 0xC42, 0xC43, 0xC3F, 0xC44, 0xC64, 0xC65,
    0xC49, 0xC48, 0xC47, 0xC45, 0xC46, 0xC6B, 0xC6C, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xC6E, 0xC6F, 0xC86, 0xC88, 0xFFFF, 0xC8A, 0xC8B, 0xC8C, 0xC8D,
    0xC8E, 0xC89, 0xFFFF, 0xFFFF, 0xFFFF, 0xC4A, 0xC4B, 0xC4C, 0xC4D, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x546,
    0x547, 0x548, 0x549, 0x54A, 0x54B, 0x54C, 0x54D, 0x54E, 0x54F, 0x550,
    0x551, 0x552, 0x553, 0x554, 0x555, 0x556, 0x557, 0x558, 0x55A, 0x55B,
    0x55C, 0x55D, 0x55E, 0x55F, 0x560, 0x561, 0x562, 0x563, 0x564, 0x565,
    0x566, 0x567, 0x568, 0x569, 0x56A, 0x56B, 0x56C, 0x56D, 0x56E, 0x56F,
    0x570, 0x571, 0x572, 0x573, 0x574, 0x575, 0x576, 0x577, 0x578, 0x579,
    0x57A, 0x57B, 0x57C, 0x57D, 0x57E, 0x57F, 0x580, 0x583, 0x584, 0x585,
    0x586, 0x587, 0x588, 0x589, 0x58A, 0x58B, 0x58D, 0x58E, 0x58F, 0x590,
    0x591, 0x592, 0x593, 0x594, 0x595, 0x596, 0x597, 0x598, 0x599, 0x59A,
    0x59B, 0x59C, 0x59D, 0x59E, 0x59F, 0x5A0, 0x5A1, 0x5A2, 0x5A6, 0x5A7,
    0x5A8, 0x5A9, 0x5AA, 0x5AB, 0x5AC, 0x5AD, 0x5AE, 0x5AF, 0x5B0, 0x5B1,
    0x5B2, 0x5B7, 0x5B8, 0x5B9, 0x5BA, 0x5BB, 0x5BC, 0x5BD, 0x5BE, 0x5BF,
    0x5C0, 0x5C1, 0x5C5, 0x5C6, 0x5C7, 0x5C8, 0x5C9, 0x5CA, 0x5CB, 0x5CC,
    0x5CD, 0x5CE, 0x5CF, 0x5D0, 0x5D1, 0x5D2, 0x5D3, 0x5D4, 0x5D5, 0x5D6,
    0x5D8, 0x5D9, 0x5DA, 0x5DB, 0x5DC, 0x5DD, 0x5DE, 0x5DF, 0x5E0, 0x5E1,
    0x5E2, 0x5E3, 0x5E4, 0x5E5, 0x5E6, 0x5E7, 0x5E8, 0x5E9, 0x5EA, 0x5EB,
    0x5EC, 0x5ED, 0x5EE, 0x5EF, 0x5F0, 0x5F1, 0x5F2, 0x5F3, 0x5F4, 0x5F5,
    0x5F6, 0x5F9, 0x5FA, 0x5FB, 0x5FE, 0x5FF, 0x600, 0x601, 0x602, 0x603,
    0x604, 0x605, 0x606, 0x607, 0x608, 0x609, 0x60A, 0x60B, 0x60C, 0x60D,
    0x60E, 0x60F, 0x610, 0x611, 0x614, 0x615, 0x616, 0x617, 0x618, 0x619,
    0x61A, 0x61B, 0x61C, 0x61D, 0x61E, 0x61F, 0x620, 0x621, 0x622, 0x624,
    0x627, 0x628, 0x629, 0x62A, 0x62B, 0x62C, 0x62D, 0x62E, 0x62F, 0x630,
    0x631, 0x632, 0x633, 0x634, 0x635, 0x636, 0x637, 0x638, 0x639, 0x63A,
    0x63B, 0x63C, 0x63D, 0x63E, 0x63F, 0x640, 0x641, 0x642, 0x643, 0x644,
    0x645, 0x646, 0x648, 0x649, 0x64A, 0x64B, 0x64C, 0x64D, 0x64E, 0x64F,
    0x654, 0x655, 0x656, 0x657, 0x658, 0x659, 0x65A, 0x65B, 0x65D, 0x65E,
    0x661, 0x662, 0x664, 0x665, 0x678, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x6A4, 0x6A5, 0x6A6, 0x6A7, 0x6A8,
    0x6A9, 0x6AA, 0x6AB, 0x6AC, 0x6AD, 0x6AE, 0x6AF, 0x6B0, 0x6B1, 0x6B2,
    0x6B3, 0x6B4, 0x6B5, 0x6B6, 0x6B7, 0x6B8, 0x6B9, 0x6BA, 0x6BB, 0x6BC,
    0x6BD, 0x6BE, 0x6BF, 0x6C0, 0x6C1, 0x6C2, 0x6C3, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0x708, 0x709, 0x70A, 0x70B, 0x70C, 0x70D, 0x70E, 0x70F, 0x711,
    0x712, 0x713, 0x714, 0x715, 0x716, 0x717, 0x718, 0x719, 0x71A, 0x71B,
    0x71C, 0x71D, 0x71E, 0x71F, 0x720, 0x723, 0x726, 0x727, 0x72A, 0x72B,
    0x72D, 0x72E, 0x72F, 0x730, 0x731, 0x732, 0x733, 0x734, 0x735, 0x736,
    0x737, 0x738, 0x739, 0x73A, 0x73B, 0x73C, 0x73D, 0x73E, 0x73F, 0x747,
    0x748, 0x749, 0x74A, 0x74B, 0x74C, 0x74D, 0x74E, 0x74F, 0x750, 0x751,
    0x752, 0x753, 0x754, 0x755, 0x756, 0x76D, 0x76E, 0x76F, 0x77E, 0x770,
    0x771, 0x772, 0x773, 0x774, 0x775, 0x776, 0x777, 0x778, 0x779, 0x77A,
    0x77B, 0x780, 0x788, 0x789, 0x78A, 0x78B, 0x78C, 0x78D, 0x78E, 0x78F,
    0x790, 0x791, 0x79E, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x7D0, 0x7D1, 0x7D2,
    0x7D3, 0x7D4, 0x7D5, 0x7D6, 0x7D7, 0x7D8, 0x7D9, 0x7DA, 0x7DB, 0x7DE,
    0x7DF, 0x7E0, 0x7E1, 0x7E2, 0x7E5, 0x7E6, 0x7E7, 0x7E8, 0x7E9, 0x7EA,
    0x7EB, 0x7EC, 0x7ED, 0x7EE, 0x7EF, 0x7F0, 0x7F1, 0x7F3, 0x7F4, 0x7F5,
    0x7F7, 0xFFFF, 0x7F8, 0x7F9, 0x7FA, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

// 80472AD0
static const u16 sCatalogList0[0x44] = {
    0x3E8, 0x3E9, 0x3EA, 0x3EB, 0x3EC, 0x3ED, 0x3EE, 0x3EF, 0x3F0, 0x3F1,
    0x3F2, 0x3F3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x403, 0x404,
    0xFFFF, 0x406, 0x409, 0x40A, 0x40B, 0x40D, 0x40F, 0x410, 0x411, 0x412,
    0x413, 0x414, 0x415, 0x416, 0xFFFF, 0x41D, 0x41E, 0x420, 0x421, 0x422,
    0xFFFF, 0x425, 0x427, 0x428, 0x429, 0x42A, 0x42B, 0x42C, 0x42D, 0xFFFF,
    0x42F, 0x430, 0xFFFF, 0x432, 0xFFFF, 0x433, 0xFFFF, 0xFFFF, 0xFFFF, 0x437,
    0x439, 0x43A, 0x43C, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

// 80472B58
static const u16 sCatalogList1[0x44] = {
    0x4B0, 0x4B1, 0x4B2, 0x4B3, 0x4B4, 0x4B5, 0x4B6, 0x4B7, 0x4B8, 0x4B9,
    0x4BA, 0x4BB, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x4CB, 0x4CC,
    0xFFFF, 0x4CE, 0x4D1, 0x4D2, 0x4D3, 0x4D4, 0x4D6, 0x4D7, 0x4D9, 0x4DA,
    0x4DB, 0x4DC, 0x4DD, 0x4DE, 0xFFFF, 0x4E3, 0xFFFF, 0x4E6, 0xFFFF, 0x4EB,
    0x4EC, 0x4ED, 0x4EE, 0x4EF, 0x4F0, 0x4F1, 0x4F2, 0x4F3, 0x4F4, 0x4F5,
    0x4F6, 0x4F7, 0x4F8, 0x4F9, 0xFFFF, 0xFFFF, 0x4FB, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0x501, 0x502, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

// 80472BE0
static const u16 sCatalogRecords[0x100] = {
    0xFFFF, 0xFFFF, 0xFFFF, 0x190, 0xFFFF, 0xFFFF, 0xFFFF, 0x191,
    0xFFFF, 0xFFFF, 0xFFFF, 0x192, 0xFFFF, 0xFFFF, 0xFFFF, 0x193,
    0xFFFF, 0xFFFF, 0xFFFF, 0x194, 0xFFFF, 0xFFFF, 0xFFFF, 0x195,
    0xFFFF, 0xFFFF, 0xFFFF, 0x196, 0xFFFF, 0xFFFF, 0xFFFF, 0x197,
    0xFFFF, 0xFFFF, 0xFFFF, 0x198, 0xFFFF, 0xFFFF, 0xFFFF, 0x199,
    0xFFFF, 0xFFFF, 0xFFFF, 0x19A, 0xFFFF, 0xFFFF, 0xFFFF, 0x19B,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x19D,
    0xFFFF, 0xFFFF, 0xFFFF, 0x19E, 0xFFFF, 0xFFFF, 0xFFFF, 0x19F,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1A2, 0xFFFF, 0xFFFF, 0xFFFF, 0x1A3,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1A4, 0xFFFF, 0xFFFF, 0xFFFF, 0x1A5,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1A6, 0xFFFF, 0xFFFF, 0xFFFF, 0x1A7,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1AA, 0xFFFF, 0xFFFF, 0xFFFF, 0x1AB,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1B0, 0xFFFF, 0xFFFF, 0xFFFF, 0x1B1,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1B2, 0xFFFF, 0xFFFF, 0xFFFF, 0x1B3,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1B4, 0xFFFF, 0xFFFF, 0xFFFF, 0x1B5,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1B6, 0xFFFF, 0xFFFF, 0xFFFF, 0x1B7,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1B8, 0xFFFF, 0xFFFF, 0xFFFF, 0x1B9,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1BA, 0xFFFF, 0xFFFF, 0xFFFF, 0x1BB,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1BC, 0xFFFF, 0xFFFF, 0xFFFF, 0x1BD,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1BE, 0xFFFF, 0xFFFF, 0xFFFF, 0x1BF,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1C0, 0xFFFF, 0xFFFF, 0xFFFF, 0x1C1,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1C2, 0xFFFF, 0xFFFF, 0xFFFF, 0x1C3,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1C4, 0xFFFF, 0xFFFF, 0xFFFF, 0x1C5,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1C6, 0xFFFF, 0xFFFF, 0xFFFF, 0x1C7,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1C8, 0xFFFF, 0xFFFF, 0xFFFF, 0x1C9,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1CA, 0xFFFF, 0xFFFF, 0xFFFF, 0x1CB,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1CC, 0xFFFF, 0xFFFF, 0xFFFF, 0x1CD,
    0xFFFF, 0xFFFF, 0xFFFF, 0x1CE, 0xFFFF, 0xFFFF, 0xFFFF, 0x1CF,
};

// 800C51D8
void addCatalogItems(void *addr, dCatalog_c *catalog) {
    u8 *mask = (u8 *)addr;
    u8 *extra = mask + 0x100;
    u8 *list1 = extra + 9;
    u8 *records = list1 + 0x12;
    addCatalogList(sCatalogList0, 0x44, extra, catalog);
    addCatalogList(sCatalogList1, 0x44, list1, catalog);
    addCatalogList(sCatalogFurniture, 0x6E9, mask, catalog);
    addCatalogRecords(sCatalogRecords, 0x100, records, catalog);
}

// 800C5278
extern "C" void fn_800C5278(void *a, void *b) {
    sCrc::calcCRC32(a, (ulong)b, 0x04201018, -1);
}

// 800C5288
const char *getKindName(u32 kind) {
    if (kind < KIND_COUNT) {
        return sKindNames[kind];
    }
    return sKindNames[0];
}

// 800C52B0
BOOL isKindAvailable(u32 kind) {
    if (kind < 0x7A) {
        return sKindAvailable[kind] != 0;
    }
    return TRUE;
}

// 800C52DC
int getCategoryQ5(const Item &item) {
    const BITM *bitm = infoBank_c::get()->getBITM(item);
    if (bitm != NULL) {
        switch (bitm->getFtrPartA()) {
        case 2:
        case 3:
            return 3;
        case 4:
            return 2;
        case 5:
            return 1;
        }

        s8 partB = bitm->m_ftrPartB;
        switch (static_cast<u32>(partB) < FTR_PART_B_COUNT ? partB : FTR_PART_B_ART) {
        case FTR_PART_B_INSTRUMENT:
            return 5;
        case FTR_PART_B_PLANT:
            return 6;
        case FTR_PART_B_DOLL:
            return 0xA;
        }

        if (bitm->getFtrLamp() != FTR_LAMP_NONE) {
            return 4;
        }

        s8 func = bitm->m_ftrFunc;
        switch (static_cast<u32>(func) < 0x41 ? func : 1) {
        case 27:
        case 28:
        case 29:
        case 64:
            return 7;
        case 30:
        case 31:
        case 32:
            return 8;
        case 33:
        case 34:
            return 9;
        }
    }
    return 0;
}

// 800C5454
int getNpcMsgFlagged(const Item &item, int index, BOOL first) {
    const BITM *bitm = infoBank_c::get()->getBITM(item);
    if (bitm != NULL) {
        NpcMsg *msg = infoBank_c::get()->getNpcMsg(static_cast<int>(bitm->m_ftrKind));
        if (msg != NULL) {
            if (first) {
                return getNpcMsgA(msg, index);
            }
            return getNpcMsgB(msg, index);
        }
    }
    return 0;
}

// 800C54EC
int getNpcMsgBullfest(const Item &item, int index) {
    const BITM *bitm = infoBank_c::get()->getBITM(item);
    if (bitm != NULL) {
        NpcMsgBullfest *msg = infoBank_c::get()->getNpcMsgBullfest(static_cast<int>(bitm->_E));
        if (msg != NULL) {
            return getBullfestMsg(msg, index);
        }
    }
    return 0;
}

// 800C5564
infoBank_c::~infoBank_c() {}

class resListInit_c {
public:
    resListInit_c() {
        memset(s_resList.mState, 0, sizeof(s_resList)); // clears 0xC bytes past the end
        resLoader_c tmp;
        nw4r::ut::List_Init(&s_resList.mList, reinterpret_cast<u8 *>(&tmp.mLink) - reinterpret_cast<u8 *>(&tmp));
    }
};
static resListInit_c s_resListInit;

// The base candCB_c::check and the series/fossil callback constructors sit after the
// static initializer and the d_dvd.hpp weak functions, so they come from an included
// file (with -sym on, MWCC gives an included file's functions their own .text section).
