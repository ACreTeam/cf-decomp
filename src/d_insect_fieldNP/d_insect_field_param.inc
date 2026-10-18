// The per-type factories and parameters (dInsectType_e order), for the
// manager. Included at the end of d_insect_field.cpp: the factories are
// inline (only their addresses are taken), so they are emitted with this
// file's .text after every other weak function (.text 0x26374..0x27040).

namespace dInsectField {

// 0x26374
inline dInsFldBase_c *createMonsiro(EGG::Heap *heap) {
    return new (heap) dInsFldButterfly_c();
}

// 0x263A8
inline dInsFldBase_c *createMonki(EGG::Heap *heap) {
    return new (heap) dInsFldButterfly_c();
}

// 0x263DC
inline dInsFldBase_c *createAgeha(EGG::Heap *heap) {
    return new (heap) dInsFldButterfly_c();
}

// 0x26410
inline dInsFldBase_c *createKarasu(EGG::Heap *heap) {
    return new (heap) dInsFldButterfly_c();
}

// 0x26444
inline dInsFldBase_c *createOokaba(EGG::Heap *heap) {
    return new (heap) dInsFldButterfly_c();
}

// 0x26478
inline dInsFldBase_c *createMorpho(EGG::Heap *heap) {
    return new (heap) dInsFldButterfly_c();
}

// 0x264AC
inline dInsFldBase_c *createMiiro(EGG::Heap *heap) {
    return new (heap) dInsFldButterfly_c();
}

// 0x264E0
inline dInsFldBase_c *createAkaeri(EGG::Heap *heap) {
    return new (heap) dInsFldButterfly_c();
}

// 0x26514
inline dInsFldBase_c *createAlex(EGG::Heap *heap) {
    return new (heap) dInsFldButterfly_c();
}

// 0x26548
inline dInsFldBase_c *createGa(EGG::Heap *heap) {
    return new (heap) dInsFldMoth_c();
}

// 0x2657C
inline dInsFldBase_c *createYmmga(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x265B0
inline dInsFldBase_c *createMitsu(EGG::Heap *heap) {
    return new (heap) dInsFldBee_c();
}

// 0x265E4
inline dInsFldBase_c *createSuzume(EGG::Heap *heap) {
    return new (heap) dInsFldHornet_c();
}

// 0x26618
inline dInsFldBase_c *createSyouryou(EGG::Heap *heap) {
    return new (heap) dInsFldGrasshopper_c();
}

// 0x2664C
inline dInsFldBase_c *createTonosama(EGG::Heap *heap) {
    return new (heap) dInsFldGrasshopper_c();
}

// 0x26680
inline dInsFldBase_c *createKama(EGG::Heap *heap) {
    return new (heap) dInsFldFlower_c();
}

// 0x266B4
inline dInsFldBase_c *createHanakama(EGG::Heap *heap) {
    return new (heap) dInsFldFlower_c();
}

// 0x266E8
inline dInsFldBase_c *createAbura(EGG::Heap *heap) {
    return new (heap) dInsFldCicada_c();
}

// 0x2671C
inline dInsFldBase_c *createMinmin(EGG::Heap *heap) {
    return new (heap) dInsFldCicada_c();
}

// 0x26750
inline dInsFldBase_c *createTsuku(EGG::Heap *heap) {
    return new (heap) dInsFldCicada_c();
}

// 0x26784
inline dInsFldBase_c *createHigu(EGG::Heap *heap) {
    return new (heap) dInsFldCicada_c();
}

// 0x267B8
inline dInsFldBase_c *createBiwa(EGG::Heap *heap) {
    return new (heap) dInsFldCicada_c();
}

// 0x267EC
inline dInsFldBase_c *createAkane(EGG::Heap *heap) {
    return new (heap) dInsFldDragonfly_c();
}

// 0x26820
inline dInsFldBase_c *createGinyan(EGG::Heap *heap) {
    return new (heap) dInsFldDragonfly_c();
}

// 0x26854
inline dInsFldBase_c *createOniyan(EGG::Heap *heap) {
    return new (heap) dInsFldDragonfly_c();
}

// 0x26888
inline dInsFldBase_c *createMukashi(EGG::Heap *heap) {
    return new (heap) dInsFldDragonfly_c();
}

// 0x268BC
inline dInsFldBase_c *createAri(EGG::Heap *heap) {
    return new (heap) dInsFldAnt_c();
}

// 0x268F0
inline dInsFldBase_c *createAmenbo(EGG::Heap *heap) {
    return new (heap) dInsFldStrider_c();
}

// 0x26924
inline dInsFldBase_c *createGengorou(EGG::Heap *heap) {
    return new (heap) dInsFldDivingBeetle_c();
}

// 0x26958
inline dInsFldBase_c *createKata(EGG::Heap *heap) {
    return new (heap) dInsFldFlower_c();
}

// 0x2698C
inline dInsFldBase_c *createKoorogi(EGG::Heap *heap) {
    return new (heap) dInsFldGrasshopper_c();
}

// 0x269C0
inline dInsFldBase_c *createSuzu(EGG::Heap *heap) {
    return new (heap) dInsFldGrasshopper_c();
}

// 0x269F4
inline dInsFldBase_c *createKirigiri(EGG::Heap *heap) {
    return new (heap) dInsFldGrasshopper_c();
}

// 0x26A28
inline dInsFldBase_c *createOkera(EGG::Heap *heap) {
    return new (heap) dInsFldMoleCricket_c();
}

// 0x26A5C
inline dInsFldBase_c *createKonoha(EGG::Heap *heap) {
    return new (heap) dInsFldLeaf_c();
}

// 0x26A90
inline dInsFldBase_c *createNana(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26AC4
inline dInsFldBase_c *createMino(EGG::Heap *heap) {
    return new (heap) dInsFldSpider_c();
}

// 0x26AF8
inline dInsFldBase_c *createTentou(EGG::Heap *heap) {
    return new (heap) dInsFldFlower_c();
}

// 0x26B2C
inline dInsFldBase_c *createViolin(EGG::Heap *heap) {
    return new (heap) dInsFldStump_c();
}

// 0x26B60
inline dInsFldBase_c *createKamikiri(EGG::Heap *heap) {
    return new (heap) dInsFldStump_c();
}

// 0x26B94
inline dInsFldBase_c *createHunkoro(EGG::Heap *heap) {
    return new (heap) dInsFldTumbleBug_c();
}

// 0x26BC8
inline dInsFldBase_c *createHotaru(EGG::Heap *heap) {
    return new (heap) dInsFldFirefly_c();
}

// 0x26BFC
inline dInsFldBase_c *createKogane(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26C30
inline dInsFldBase_c *createPlakoga(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26C64
inline dInsFldBase_c *createTama(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26C98
inline dInsFldBase_c *createMiyakwa(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26CCC
inline dInsFldBase_c *createNokokwa(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26D00
inline dInsFldBase_c *createOokwa(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26D34
inline dInsFldBase_c *createNijikwa(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26D68
inline dInsFldBase_c *createHosokwa(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26D9C
inline dInsFldBase_c *createOugon(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26DD0
inline dInsFldBase_c *createKbt(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26E04
inline dInsFldBase_c *createKokbt(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26E38
inline dInsFldBase_c *createZokbt(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26E6C
inline dInsFldBase_c *createHelkbt(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26EA0
inline dInsFldBase_c *createGorias(EGG::Heap *heap) {
    return new (heap) dInsFldBeetle_c();
}

// 0x26ED4
inline dInsFldBase_c *createNomi(EGG::Heap *heap) {
    return new (heap) dInsFldFlea_c();
}

// 0x26F08
inline dInsFldBase_c *createDango(EGG::Heap *heap) {
    return new (heap) dInsFldPillBug_c();
}

// 0x26F3C
inline dInsFldBase_c *createKa(EGG::Heap *heap) {
    return new (heap) dInsFldMosquito_c();
}

// 0x26F70
inline dInsFldBase_c *createHae(EGG::Heap *heap) {
    return new (heap) dInsFldBee_c();
}

// 0x26FA4
inline dInsFldBase_c *createMukade(EGG::Heap *heap) {
    return new (heap) dInsFldPillBug_c();
}

// 0x26FD8
inline dInsFldBase_c *createKumo(EGG::Heap *heap) {
    return new (heap) dInsFldSpider_c();
}

// 0x2700C
inline dInsFldBase_c *createTaran(EGG::Heap *heap) {
    return new (heap) dInsFldTarantula_c();
}

// 0x27040
inline dInsFldBase_c *createSasori(EGG::Heap *heap) {
    return new (heap) dInsFldTarantula_c();
}

// .data 0x1C7C: the per-type parameters (dInsectType_e order).
static const char l_monsiroName[] = "ins_monsiro";
static const char l_monkiName[] = "ins_monki";
static const char l_agehaName[] = "ins_ageha";
static const char l_karasuName[] = "ins_karasu";
static const char l_ookabaName[] = "ins_ookaba";
static const char l_morphoName[] = "ins_morpho";
static const char l_miiroName[] = "ins_miiro";
static const char l_akaeriName[] = "ins_akaeri";
static const char l_alexName[] = "ins_alex";
static const char l_gaName[] = "ins_ga";
static const char l_ymmgaName[] = "ins_ymmga";
static const char l_mitsuName[] = "ins_mitsu";
static const char l_suzumeName[] = "ins_suzume";
static const char l_syouryouName[] = "ins_syouryou";
static const char l_tonosamaName[] = "ins_tonosama";
static const char l_kamaName[] = "ins_kama";
static const char l_hanakamaName[] = "ins_hanakama";
static const char l_aburaName[] = "ins_abura";
static const char l_minminName[] = "ins_minmin";
static const char l_tsukuName[] = "ins_tsuku";
static const char l_higuName[] = "ins_higu";
static const char l_biwaName[] = "ins_biwa";
static const char l_akaneName[] = "ins_akane";
static const char l_ginyanName[] = "ins_ginyan";
static const char l_oniyanName[] = "ins_oniyan";
static const char l_mukashiName[] = "ins_mukashi";
static const char l_ariName[] = "ins_ari";
static const char l_amenboName[] = "ins_amenbo";
static const char l_gengorouName[] = "ins_gengorou";
static const char l_kataName[] = "ins_kata";
static const char l_koorogiName[] = "ins_koorogi";
static const char l_suzuName[] = "ins_suzu";
static const char l_kirigiriName[] = "ins_kirigiri";
static const char l_okeraName[] = "ins_okera";
static const char l_konohaName[] = "ins_konoha";
static const char l_nanaName[] = "ins_nana";
static const char l_minoName[] = "ins_mino";
static const char l_tentouName[] = "ins_tentou";
static const char l_violinName[] = "ins_violin";
static const char l_kamikiriName[] = "ins_kamikiri";
static const char l_hunkoroName[] = "ins_hunkoro";
static const char l_hotaruName[] = "ins_hotaru";
static const char l_koganeName[] = "ins_kogane";
static const char l_plakogaName[] = "ins_plakoga";
static const char l_tamaName[] = "ins_tama";
static const char l_miyakwaName[] = "ins_miyakwa";
static const char l_nokokwaName[] = "ins_nokokwa";
static const char l_ookwaName[] = "ins_ookwa";
static const char l_nijikwaName[] = "ins_nijikwa";
static const char l_hosokwaName[] = "ins_hosokwa";
static const char l_ougonName[] = "ins_ougon";
static const char l_kbtName[] = "ins_kbt";
static const char l_kokbtName[] = "ins_kokbt";
static const char l_zokbtName[] = "ins_zokbt";
static const char l_helkbtName[] = "ins_helkbt";
static const char l_goriasName[] = "ins_gorias";
static const char l_nomiName[] = "ins_nomi";
static const char l_dangoName[] = "ins_dango";
static const char l_kaName[] = "ins_ka";
static const char l_haeName[] = "ins_hae";
static const char l_mukadeName[] = "ins_mukade";
static const char l_kumoName[] = "ins_kumo";
static const char l_taranName[] = "ins_taran";
static const char l_sasoriName[] = "ins_sasori";

#ifdef MUST_MATCH
// .rodata 0x11A0..0x1260: 48 words nothing references, after the names. Folded
// function-local static consts (3.0f, 1.3f, -0.08f, then int/float groups such
// as {.., -30.0f, 5.0f} and {-500.0f, 5.0f} that look like getGroundMovePos
// arguments) whose functions are unknown; kept for the .rodata layout.
extern const u32 lUnusedRodataTail[48];
const u32 lUnusedRodataTail[48] = {
    0x40400000, 0x3FA66666, 0xBDA3D70A, 0x00000002, 0x00000006, 0xC1F00000,
    0x40A00000, 0x00000001, 0x00000002, 0x42480000, 0x02000000, 0xC1F00000,
    0x40A00000, 0x00000002, 0x00000002, 0x40B00000, 0x00000001, 0xC1F00000,
    0x40A00000, 0x00000002, 0x42333333, 0x424CCCCD, 0x00000001, 0xC1F00000,
    0x40A00000, 0x00000001, 0xC1F00000, 0x40A00000, 0x00000001, 0x00000001,
    0x00000001, 0x00000001, 0xC3FA0000, 0x40A00000, 0x00000001, 0x00000002,
    0x00000001, 0x00000004, 0x40400000, 0x00000005, 0x00000004, 0x00000004,
    0x00190000, 0x00000005, 0x40400000, 0x00000010, 0x00000010, 0x00000009,
};
#endif

param_c l_monsiro = {createMonsiro, l_monsiroName, 150, 0, 80.0f, 5.0f, 6.0f};
param_c l_monki = {createMonki, l_monkiName, 150, 0, 80.0f, 5.0f, 6.0f};
param_c l_ageha = {createAgeha, l_agehaName, 120, 0, 100.0f, 7.0f, 8.0f};
param_c l_karasu = {createKarasu, l_karasuName, 120, 0, 100.0f, 7.0f, 8.0f};
param_c l_ookaba = {createOokaba, l_ookabaName, 120, 0, 80.0f, 5.0f, 7.0f};
param_c l_morpho = {createMorpho, l_morphoName, 80, 0, 100.0f, 6.0f, 9.0f};
param_c l_miiro = {createMiiro, l_miiroName, 120, 0, 100.0f, 5.0f, 8.0f};
param_c l_akaeri = {createAkaeri, l_akaeriName, 120, 0, 150.0f, 8.0f, 8.0f};
param_c l_alex = {createAlex, l_alexName, 80, 0, 220.0f, 12.0f, 13.0f};
param_c l_ga = {createGa, l_gaName, 400, 0, 50.0f, 5.0f, 6.5f};
param_c l_ymmga = {createYmmga, l_ymmgaName, 80, 0, 120.0f, 10.0f, 12.0f};
param_c l_mitsu = {createMitsu, l_mitsuName, 200, 0, 50.0f, 3.0f, 3.0f};
param_c l_suzume = {createSuzume, l_suzumeName, 0, 0, 0.0f, 15.0f, 10.0f};
param_c l_syouryou = {createSyouryou, l_syouryouName, 100, 0,
                      80.0f,          5.0f,           5.0f};
param_c l_tonosama = {createTonosama, l_tonosamaName, 70, 0, 80.0f, 4.0f, 6.0f};
param_c l_kama = {createKama, l_kamaName, 20, 400, 80.0f, 5.0f, 4.0f};
param_c l_hanakama = {createHanakama, l_hanakamaName, 20,  300,
                      80.0f,          4.0f,           3.0f};
param_c l_abura = {createAbura, l_aburaName, 40, 200, 60.0f, 8.0f, 5.0f};
param_c l_minmin = {createMinmin, l_minminName, 40, 200, 60.0f, 8.0f, 5.0f};
param_c l_tsuku = {createTsuku, l_tsukuName, 40, 200, 60.0f, 8.0f, 5.0f};
param_c l_higu = {createHigu, l_higuName, 40, 200, 60.0f, 8.0f, 5.0f};
param_c l_biwa = {createBiwa, l_biwaName, 30, 80, 120.0f, 9.0f, 7.5f};
param_c l_akane = {createAkane, l_akaneName, 100, 0, 80.0f, 4.0f, 5.0f};
param_c l_ginyan = {createGinyan, l_ginyanName, 100, 0, 80.0f, 5.0f, 6.0f};
param_c l_oniyan = {createOniyan, l_oniyanName, 100, 0, 120.0f, 5.0f, 8.0f};
param_c l_mukashi = {createMukashi, l_mukashiName, 100,  0,
                     200.0f,        10.0f,         11.0f};
param_c l_ari = {createAri, l_ariName, 0, 0, 0.0f, 6.0f, 0.0f};
param_c l_amenbo = {createAmenbo, l_amenboName, 0, 0, 0.0f, 6.0f, 0.0f};
param_c l_gengorou = {createGengorou, l_gengorouName, 0, 0, 0.0f, 8.0f, 0.0f};
param_c l_kata = {createKata, l_kataName, 100, 400, 40.0f, 5.0f, 4.0f};
param_c l_koorogi = {createKoorogi, l_koorogiName, 30, 100, 80.0f, 4.0f, 3.5f};
param_c l_suzu = {createSuzu, l_suzuName, 30, 100, 80.0f, 4.0f, 4.0f};
param_c l_kirigiri = {createKirigiri, l_kirigiriName, 30,  100,
                      80.0f,          4.0f,           5.0f};
param_c l_okera = {createOkera, l_okeraName, 0, 0, 0.0f, 5.0f, 5.0f};
param_c l_konoha = {createKonoha, l_konohaName, 5, 0, 50.0f, 7.0f, 7.0f};
param_c l_nana = {createNana, l_nanaName, 200, 0, 100.0f, 8.0f, 4.0f};
param_c l_mino = {createMino, l_minoName, 0, 0, 0.0f, 4.0f, 0.0f};
param_c l_tentou = {createTentou, l_tentouName, 400, 0, 40.0f, 3.0f, 4.5f};
param_c l_violin = {createViolin, l_violinName, 200, 0, 100.0f, 5.0f, 4.0f};
param_c l_kamikiri = {createKamikiri, l_kamikiriName, 200, 0,
                      100.0f,         4.0f,           4.0f};
param_c l_hunkoro = {createHunkoro, l_hunkoroName, 150, 0, 100.0f, 8.0f, 6.0f};
param_c l_hotaru = {createHotaru, l_hotaruName, 70, 0, 80.0f, 4.0f, 7.0f};
param_c l_kogane = {createKogane, l_koganeName, 200, 0, 60.0f, 7.0f, 5.0f};
param_c l_plakoga = {createPlakoga, l_plakogaName, 80, 0, 90.0f, 7.0f, 5.0f};
param_c l_tama = {createTama, l_tamaName, 150, 0, 90.0f, 7.0f, 4.5f};
param_c l_miyakwa = {createMiyakwa, l_miyakwaName, 200, 0, 60.0f, 8.0f, 6.5f};
param_c l_nokokwa = {createNokokwa, l_nokokwaName, 150, 0, 90.0f, 8.0f, 6.5f};
param_c l_ookwa = {createOokwa, l_ookwaName, 100, 0, 90.0f, 8.0f, 7.0f};
param_c l_nijikwa = {createNijikwa, l_nijikwaName, 100, 0, 90.0f, 8.0f, 6.0f};
param_c l_hosokwa = {createHosokwa, l_hosokwaName, 100, 0, 90.0f, 8.0f, 7.5f};
param_c l_ougon = {createOugon, l_ougonName, 80, 0, 90.0f, 8.0f, 7.0f};
param_c l_kbt = {createKbt, l_kbtName, 150, 0, 90.0f, 8.0f, 7.0f};
param_c l_kokbt = {createKokbt, l_kokbtName, 100, 0, 90.0f, 9.0f, 8.0f};
param_c l_zokbt = {createZokbt, l_zokbtName, 100, 0, 90.0f, 9.0f, 7.5f};
param_c l_helkbt = {createHelkbt, l_helkbtName, 80, 0, 90.0f, 9.0f, 8.0f};
param_c l_gorias = {createGorias, l_goriasName, 120, 0, 90.0f, 9.0f, 8.0f};
param_c l_nomi = {createNomi, l_nomiName, 0, 0, 0.0f, 5.0f, 0.0f};
param_c l_dango = {createDango, l_dangoName, 50, 0, 64.0f, 3.0f, 3.0f};
param_c l_ka = {createKa, l_kaName, 0, 0, 0.0f, 2.0f, 0.0f};
param_c l_hae = {createHae, l_haeName, 100, 0, 100.0f, 2.0f, 3.0f};
param_c l_mukade = {createMukade, l_mukadeName, 70, 0, 100.0f, 3.0f, 3.0f};
param_c l_kumo = {createKumo, l_kumoName, 0, 0, 0.0f, 4.0f, 0.0f};
param_c l_taran = {createTaran, l_taranName, 40, 80, 120.0f, 6.0f, 10.0f};
param_c l_sasori = {createSasori, l_sasoriName, 40, 80, 120.0f, 7.0f, 10.0f};

// .data 0x2380
const param_c *sParamTable[INSECT_NUM] = {
    &l_monsiro, &l_monki,    &l_ageha,    &l_karasu,   &l_ookaba,   &l_morpho,
    &l_miiro,   &l_akaeri,   &l_alex,     &l_ga,       &l_ymmga,    &l_mitsu,
    &l_suzume,  &l_syouryou, &l_tonosama, &l_kama,     &l_hanakama, &l_abura,
    &l_minmin,  &l_tsuku,    &l_higu,     &l_biwa,     &l_akane,    &l_ginyan,
    &l_oniyan,  &l_mukashi,  &l_ari,      &l_amenbo,   &l_gengorou, &l_kata,
    &l_koorogi, &l_suzu,     &l_kirigiri, &l_okera,    &l_konoha,   &l_nana,
    &l_mino,    &l_tentou,   &l_violin,   &l_kamikiri, &l_hunkoro,  &l_hotaru,
    &l_kogane,  &l_plakoga,  &l_tama,     &l_miyakwa,  &l_nokokwa,  &l_ookwa,
    &l_nijikwa, &l_hosokwa,  &l_ougon,    &l_kbt,      &l_kokbt,    &l_zokbt,
    &l_helkbt,  &l_gorias,   &l_nomi,     &l_dango,    &l_ka,       &l_hae,
    &l_mukade,  &l_kumo,     &l_taran,    &l_sasori,
};

} // namespace dInsectField
