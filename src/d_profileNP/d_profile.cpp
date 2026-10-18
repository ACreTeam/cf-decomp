#include <game/game/d_profile.hpp>

// Profiles exported by the DOL and RELs. Names of newly identified exports
// follow their system namespace and profile enum; multi-profile variants use
// the concrete class identified by each factory's final fBase vtable.
extern fProf::fBaseProfile_c g_profile_ROOT;
extern fProf::fBaseProfile_c g_profile_OBJ_CURSOR;
extern fProf::fBaseProfile_c g_profile_MSG;
extern fProf::fBaseProfile_c g_profile_CLOCK;
extern fProf::fBaseProfile_c g_profile_MONEY;
extern fProf::fBaseProfile_c g_profile_OBJ_KEYBOARD;
extern fProf::fBaseProfile_c g_profile_PLAYER_MNGR;
extern fProf::fBaseProfile_c g_profile_CD;
extern fProf::fBaseProfile_c g_profile_SNOWMAN;
extern fProf::fBaseProfile_c g_profile_SV_DEMO;
extern fProf::fBaseProfile_c g_profile_SV_MGR;
extern fProf::fBaseProfile_c g_profile_BOOT;
extern fProf::fBaseProfile_c g_profile_CAMERA;
extern fProf::fBaseProfile_c g_profile_WEATHER;
extern fProf::fBaseProfile_c g_profile_SKY;
extern fProf::fBaseProfile_c g_profile_NPC_MNG;
extern fProf::fBaseProfile_c g_profile_HAND_ITEM;
extern fProf::fBaseProfile_c g_profile_FI_MDL_MGR;
extern fProf::fBaseProfile_c g_profile_FISH_FIELD;
extern fProf::fBaseProfile_c g_profile_MENU_MNG;
extern fProf::fBaseProfile_c g_profile_BG_DEBUG;
extern fProf::fBaseProfile_c g_profile_OBJ_BAL;
extern fProf::fBaseProfile_c g_profile_OBJ_VOICE;
extern fProf::fBaseProfile_c g_profile_OBJ_CHAT;
extern fProf::fBaseProfile_c g_profile_OBJ_TUTORIAL;
extern fProf::fBaseProfile_c g_profile_OBJ_TELOP;
extern fProf::fBaseProfile_c g_profile_EVENT_MNG;
extern fProf::fBaseProfile_c g_profile_DEMO;
extern fProf::fBaseProfile_c g_profile_OBJC;
extern fProf::fBaseProfile_c g_profile_ENVIRONMENT;

namespace dSStage {
extern fProf::fBaseProfile_c g_profile_S_STAGE;
} // namespace dSStage

namespace dAcUitag {
extern fProf::fBaseProfile_c g_profile_AC_UITAG;
} // namespace dAcUitag

namespace dAGrowupMsg {
extern fProf::fBaseProfile_c g_profile_A_GROWUP_MSG;
} // namespace dAGrowupMsg

namespace dAPlayer {
extern fProf::fBaseProfile_c g_profile_A_PLAYER;
} // namespace dAPlayer

namespace dOrgObj {
extern fProf::fBaseProfile_c g_profile_ORG_OBJ;
} // namespace dOrgObj

namespace dARmobjAtm {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_ATM;
} // namespace dARmobjAtm

namespace dARmobjBarricade {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_BARRICADE;
} // namespace dARmobjBarricade

namespace dARmobjBus {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_BUS;
} // namespace dARmobjBus

namespace dARmobjCafe {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_CAFE;
} // namespace dARmobjCafe

namespace dARmobjCheckp {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_CHECKP;
} // namespace dARmobjCheckp

namespace dARmobjFossil {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_FOSSIL;
} // namespace dARmobjFossil

namespace dARmobjGrace {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_GRACE;
} // namespace dARmobjGrace

namespace dARmobjPermMachine {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_PERM_MACHINE;
} // namespace dARmobjPermMachine

namespace dARmobjPic {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_PIC;
} // namespace dARmobjPic

namespace dARmobjPoint {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_POINT;
} // namespace dARmobjPoint

namespace dARmobjRecyclebox {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_RECYCLEBOX;
} // namespace dARmobjRecyclebox

namespace dARmobjReset {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_RESET;
} // namespace dARmobjReset

namespace dARmobjSample {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_SAMPLE;
} // namespace dARmobjSample

namespace dARmobjSewingMachine {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_SEWING_MACHINE;
} // namespace dARmobjSewingMachine

namespace dARmobjTel {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_TEL;
} // namespace dARmobjTel

namespace dARmobjTheater {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_THEATER;
} // namespace dARmobjTheater

namespace dARmobjMirrorball {
extern fProf::fBaseProfile_c g_profile_A_RMOBJ_MIRRORBALL;
} // namespace dARmobjMirrorball

namespace dATicketCounter {
extern fProf::fBaseProfile_c g_profile_A_TICKET_COUNTER;
} // namespace dATicketCounter

namespace dCockroachManager {
extern fProf::fBaseProfile_c g_profile_COCKROACH_MANAGER;
} // namespace dCockroachManager

namespace dACockroach {
extern fProf::fBaseProfile_c g_profile_A_COCKROACH;
} // namespace dACockroach

namespace dAFtr {
extern fProf::fBaseProfile_c g_profile_A_FTR;
} // namespace dAFtr

namespace dAFtrAct {
extern fProf::fBaseProfile_c g_profile_FTR_LP;
extern fProf::fBaseProfile_c g_profile_FTR_TRG;
extern fProf::fBaseProfile_c g_profile_FTR_SW;
extern fProf::fBaseProfile_c g_profile_FTR_SW2;
extern fProf::fBaseProfile_c g_profile_FTR_FLP;
extern fProf::fBaseProfile_c g_profile_FTR2_W;
extern fProf::fBaseProfile_c g_profile_FTR_FSW;
extern fProf::fBaseProfile_c g_profile_FTR_CHAIR;
extern fProf::fBaseProfile_c g_profile_FTR_BED;
extern fProf::fBaseProfile_c g_profile_FTR_CHEST;
extern fProf::fBaseProfile_c g_profile_FTR_CLOCK;
extern fProf::fBaseProfile_c g_profile_FTR_AUDIO;
extern fProf::fBaseProfile_c g_profile_FTR_TV;
extern fProf::fBaseProfile_c g_profile_FTR_TV_F;
extern fProf::fBaseProfile_c g_profile_FTR_PCART;
extern fProf::fBaseProfile_c g_profile_FTR_HNW;
extern fProf::fBaseProfile_c g_profile_FTR_FAW;
extern fProf::fBaseProfile_c g_profile_FTR_KILLER;
extern fProf::fBaseProfile_c g_profile_FTR_CLOTH;
extern fProf::fBaseProfile_c g_profile_FTR_MYOBJ;
extern fProf::fBaseProfile_c g_profile_FTR_TAOBJ;
extern fProf::fBaseProfile_c g_profile_FTR_CMPS;
extern fProf::fBaseProfile_c g_profile_FTR_INSTRUMENT;
extern fProf::fBaseProfile_c g_profile_FTR_BK;
extern fProf::fBaseProfile_c g_profile_FTR_METRO;
extern fProf::fBaseProfile_c g_profile_FTR_CATALOG_CW;
extern fProf::fBaseProfile_c g_profile_FTR_ACC;
extern fProf::fBaseProfile_c g_profile_FTR_UMB;
extern fProf::fBaseProfile_c g_profile_FTR_BALLOON;
extern fProf::fBaseProfile_c g_profile_FTR_WINDMILL;
extern fProf::fBaseProfile_c g_profile_FTR_MONITOR;
extern fProf::fBaseProfile_c g_profile_FTR_CICADA;
extern fProf::fBaseProfile_c g_profile_FTR_TEL;
extern fProf::fBaseProfile_c g_profile_FTR_FLOWER;
extern fProf::fBaseProfile_c g_profile_FTR_ATM;
extern fProf::fBaseProfile_c g_profile_FTR_BARLEY;
extern fProf::fBaseProfile_c g_profile_FTR_ROLLER;
} // namespace dAFtrAct

namespace dFtrManager {
extern fProf::fBaseProfile_c g_profile_FTR_MANAGER;
} // namespace dFtrManager

namespace dASample {
extern fProf::fBaseProfile_c g_profile_A_SAMPLE;
} // namespace dASample

namespace dASample2 {
extern fProf::fBaseProfile_c g_profile_A_SAMPLE2;
} // namespace dASample2

namespace dASample3 {
extern fProf::fBaseProfile_c g_profile_A_SAMPLE3;
} // namespace dASample3

namespace dANpcOut {
extern fProf::fBaseProfile_c g_profile_A_NPC_OUT;
} // namespace dANpcOut

namespace dANpcIn {
extern fProf::fBaseProfile_c g_profile_A_NPC_IN;
} // namespace dANpcIn

namespace dANpcVisitNpcHouse {
extern fProf::fBaseProfile_c g_profile_A_NPC_VISIT_NPC_HOUSE;
} // namespace dANpcVisitNpcHouse

namespace dANpcVisitPlayerHouse {
extern fProf::fBaseProfile_c g_profile_A_NPC_VISIT_PLAYER_HOUSE;
} // namespace dANpcVisitPlayerHouse

namespace dANpcVisitPlayerHouseFleamarket {
extern fProf::fBaseProfile_c g_profile_A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET;
} // namespace dANpcVisitPlayerHouseFleamarket

namespace dANpcTownOut {
extern fProf::fBaseProfile_c g_profile_A_NPC_TOWN_OUT;
} // namespace dANpcTownOut

namespace dANpcTownGrace {
extern fProf::fBaseProfile_c g_profile_A_NPC_TOWN_GRACE;
} // namespace dANpcTownGrace

namespace dANpcTownTheater {
extern fProf::fBaseProfile_c g_profile_A_NPC_TOWN_THEATER;
} // namespace dANpcTownTheater

namespace dANpcCafe {
extern fProf::fBaseProfile_c g_profile_A_NPC_CAFE;
} // namespace dANpcCafe

namespace dANpcSpSample {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_SAMPLE;
} // namespace dANpcSpSample

namespace dANpcSpPlayerCreate {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_PLAYER_CREATE;
} // namespace dANpcSpPlayerCreate

namespace dANpcSpPlayerSelect {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_PLAYER_SELECT;
} // namespace dANpcSpPlayerSelect

namespace dANpcSpResetsan {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_RESETSAN;
} // namespace dANpcSpResetsan

namespace dANpcSpCafeGuest {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_CAFE_GUEST;
} // namespace dANpcSpCafeGuest

namespace dANpcSpPeriko {
extern fProf::fBaseProfile_c g_profile_NPC_SP_PERIKO;
extern fProf::fBaseProfile_c g_profile_NPC_SP_PERIMI;
} // namespace dANpcSpPeriko

namespace dANpcSpSoncho {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_SONCHO;
} // namespace dANpcSpSoncho

namespace dANpcSpOpTanukichi {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_OP_TANUKICHI;
} // namespace dANpcSpOpTanukichi

namespace dANpcSpTanukichi {
extern fProf::fBaseProfile_c g_profile_NPC_SP_TANUKICHI;
extern fProf::fBaseProfile_c g_profile_NPC_SP_MAMEKICHI;
extern fProf::fBaseProfile_c g_profile_NPC_SP_TUBUKICHI;
} // namespace dANpcSpTanukichi

namespace dANpcSpKaburiba {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_KABURIBA;
} // namespace dANpcSpKaburiba

namespace dANpcSpTunekichi {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_TUNEKICHI;
} // namespace dANpcSpTunekichi

namespace dANpcSpCatherine {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_CATHERINE;
} // namespace dANpcSpCatherine

namespace dANpcSpAsami {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_ASAMI;
} // namespace dANpcSpAsami

namespace dANpcSpKinuyo {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_KINUYO;
} // namespace dANpcSpKinuyo

namespace dANpcSpJijyo {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_JIJYO;
} // namespace dANpcSpJijyo

namespace dANpcSpCafe {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_CAFE;
} // namespace dANpcSpCafe

namespace dANpcSpFuta {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_FUTA;
} // namespace dANpcSpFuta

namespace dANpcSpFuko {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_FUKO;
} // namespace dANpcSpFuko

namespace dANpcSpSeiichi {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_SEIICHI;
} // namespace dANpcSpSeiichi

namespace dANpcSpRoran {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_RORAN;
} // namespace dANpcSpRoran

namespace dANpcSpKappei {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_KAPPEI;
} // namespace dANpcSpKappei

namespace dANpcSpTotakeke {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_TOTAKEKE;
} // namespace dANpcSpTotakeke

namespace dANpcSpJonny {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_JONNY;
} // namespace dANpcSpJonny

namespace dANpcSpRacco {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_RACCO;
} // namespace dANpcSpRacco

namespace dANpcSpPerio {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_PERIO;
} // namespace dANpcSpPerio

namespace dANpcSpFortune {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_FORTUNE;
} // namespace dANpcSpFortune

namespace dANpcSpSisyo {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_SISYO;
} // namespace dANpcSpSisyo

namespace dANpcSpHonma {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_HONMA;
} // namespace dANpcSpHonma

namespace dANpcSpGrace {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_GRACE;
} // namespace dANpcSpGrace

namespace dANpcSpGatekeeperA {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_GATEKEEPER_A;
} // namespace dANpcSpGatekeeperA

namespace dANpcSpGatekeeperB {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_GATEKEEPER_B;
} // namespace dANpcSpGatekeeperB

namespace dANpcSpJingle {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_JINGLE;
} // namespace dANpcSpJingle

namespace dANpcSpFranklin {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_FRANKLIN;
} // namespace dANpcSpFranklin

namespace dANpcSpUomasa {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_UOMASA;
} // namespace dANpcSpUomasa

namespace dANpcSpPumpking {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_PUMPKING;
} // namespace dANpcSpPumpking

namespace dANpcSpKutsumigaki {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_KUTSUMIGAKI;
} // namespace dANpcSpKutsumigaki

namespace dANpcSpEventSoncho {
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO_CD;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO_NY;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO_HF;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO_HT;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO01;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO02;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO03;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO04;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO05;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO06;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO07;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO08;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO09;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO10;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO11;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO12;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO13;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO14;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO15;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO16;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO17;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO18;
extern fProf::fBaseProfile_c g_profile_NPC_SP_EV_SONCHO19;
} // namespace dANpcSpEventSoncho

namespace dANpcSpCeremonySoncho {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_CEREMONY_SONCHO;
} // namespace dANpcSpCeremonySoncho

namespace dANpcSpGolditemSoncho {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_GOLDITEM_SONCHO;
} // namespace dANpcSpGolditemSoncho

namespace dANpcSpPyontaro {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_PYONTARO;
} // namespace dANpcSpPyontaro

namespace dANpcSpGoddess {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_GODDESS;
} // namespace dANpcSpGoddess

namespace dANpcSpBug {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_BUG;
} // namespace dANpcSpBug

namespace dANpcSpHaniwa {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_HANIWA;
} // namespace dANpcSpHaniwa

namespace dANpcSpDemoResetsan {
extern fProf::fBaseProfile_c g_profile_NPC_SP_DEMO_RESETSAN;
extern fProf::fBaseProfile_c g_profile_NPC_SP_DEMO_RACKETSAN;
} // namespace dANpcSpDemoResetsan

namespace dANpcSpDancer {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_DANCER;
} // namespace dANpcSpDancer

namespace dANpcSpMissingChild {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_MISSING_CHILD;
} // namespace dANpcSpMissingChild

namespace dANpcSpMissingMother {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_MISSING_MOTHER;
} // namespace dANpcSpMissingMother

namespace dANpcSpMysterycat {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_MYSTERYCAT;
} // namespace dANpcSpMysterycat

namespace dANpcSpYuutaro {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_YUUTARO;
} // namespace dANpcSpYuutaro

namespace dANpcSpYuutaroLoft {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_YUUTARO_LOFT;
} // namespace dANpcSpYuutaroLoft

namespace dANpcSpFuusen {
extern fProf::fBaseProfile_c g_profile_A_NPC_SP_FUUSEN;
} // namespace dANpcSpFuusen

namespace dSSelect {
extern fProf::fBaseProfile_c g_profile_S_SELECT;
} // namespace dSSelect

namespace dTitleDemo {
extern fProf::fBaseProfile_c g_profile_TITLE_DEMO;
} // namespace dTitleDemo

namespace dField {
extern fProf::fBaseProfile_c g_profile_FIELD;
} // namespace dField

extern fProf::fBaseProfile_c g_profile_SKY_AURORA;

extern fProf::fBaseProfile_c g_profile_SKY_RAINBOW;

namespace dFishMuseum {
extern fProf::fBaseProfile_c g_profile_FISH_MUSEUM;
} // namespace dFishMuseum

namespace dInsectMuseum {
extern fProf::fBaseProfile_c g_profile_INSECT_MUSEUM;
} // namespace dInsectMuseum

namespace dInsectField {
extern fProf::fBaseProfile_c g_profile_INSECT_FIELD;
} // namespace dInsectField

namespace dALayout {
extern fProf::fBaseProfile_c g_profile_A_LAYOUT;
} // namespace dALayout

namespace dMenuItem00 {
extern fProf::fBaseProfile_c g_profile_MENU_ITEM00;
} // namespace dMenuItem00

namespace dMenuItem01 {
extern fProf::fBaseProfile_c g_profile_MENU_ITEM01;
} // namespace dMenuItem01

namespace dMenuItem02 {
extern fProf::fBaseProfile_c g_profile_MENU_ITEM02;
} // namespace dMenuItem02

namespace dMenuItem03 {
extern fProf::fBaseProfile_c g_profile_MENU_ITEM03;
} // namespace dMenuItem03

namespace dMenuItem04 {
extern fProf::fBaseProfile_c g_profile_MENU_ITEM04;
} // namespace dMenuItem04

namespace dMenuItem05 {
extern fProf::fBaseProfile_c g_profile_MENU_ITEM05;
} // namespace dMenuItem05

namespace dMenuItem06 {
extern fProf::fBaseProfile_c g_profile_MENU_ITEM06;
} // namespace dMenuItem06

namespace dMenuFriend {
extern fProf::fBaseProfile_c g_profile_MENU_FRIEND;
} // namespace dMenuFriend

namespace dMenuFriend01 {
extern fProf::fBaseProfile_c g_profile_MENU_FRIEND01;
} // namespace dMenuFriend01

namespace dMenuFriend02 {
extern fProf::fBaseProfile_c g_profile_MENU_FRIEND02;
} // namespace dMenuFriend02

namespace dMenuFriend03 {
extern fProf::fBaseProfile_c g_profile_MENU_FRIEND03;
} // namespace dMenuFriend03

namespace dMenuFriend04 {
extern fProf::fBaseProfile_c g_profile_MENU_FRIEND04;
} // namespace dMenuFriend04

namespace dMenuFriend05 {
extern fProf::fBaseProfile_c g_profile_MENU_FRIEND05;
} // namespace dMenuFriend05

namespace dMenuFriend06 {
extern fProf::fBaseProfile_c g_profile_MENU_FRIEND06;
} // namespace dMenuFriend06

namespace dMenuFriend07 {
extern fProf::fBaseProfile_c g_profile_MENU_FRIEND07;
} // namespace dMenuFriend07

namespace dMenuFriend08 {
extern fProf::fBaseProfile_c g_profile_MENU_FRIEND08;
} // namespace dMenuFriend08

namespace dMenuInsc {
extern fProf::fBaseProfile_c g_profile_MENU_INSC;
} // namespace dMenuInsc

namespace dMenuChat {
extern fProf::fBaseProfile_c g_profile_MENU_CHAT;
} // namespace dMenuChat

namespace dMenuDesi00 {
extern fProf::fBaseProfile_c g_profile_MENU_DESI00;
} // namespace dMenuDesi00

namespace dMenuEdit {
extern fProf::fBaseProfile_c g_profile_MENU_EDIT;
} // namespace dMenuEdit

namespace dMenuPaper {
extern fProf::fBaseProfile_c g_profile_MENU_PAPER;
} // namespace dMenuPaper

namespace dMenuKeytest {
extern fProf::fBaseProfile_c g_profile_MENU_KEYTEST;
} // namespace dMenuKeytest

namespace dMenuHan {
extern fProf::fBaseProfile_c g_profile_MENU_HAN;
} // namespace dMenuHan

namespace dMenuMap00 {
extern fProf::fBaseProfile_c g_profile_MENU_MAP00;
} // namespace dMenuMap00

namespace dMenuMap01 {
extern fProf::fBaseProfile_c g_profile_MENU_MAP01;
} // namespace dMenuMap01

namespace dMenuMap02 {
extern fProf::fBaseProfile_c g_profile_MENU_MAP02;
} // namespace dMenuMap02

namespace dMenuMap03 {
extern fProf::fBaseProfile_c g_profile_MENU_MAP03;
} // namespace dMenuMap03

namespace dMenuPhoto {
extern fProf::fBaseProfile_c g_profile_MENU_PHOTO;
} // namespace dMenuPhoto

namespace dMenuPhoto01 {
extern fProf::fBaseProfile_c g_profile_MENU_PHOTO01;
} // namespace dMenuPhoto01

namespace dMenuCatalog {
extern fProf::fBaseProfile_c g_profile_MENU_CATALOG;
} // namespace dMenuCatalog

namespace dMenuNumber00 {
extern fProf::fBaseProfile_c g_profile_MENU_NUMBER00;
} // namespace dMenuNumber00

namespace dMenuMuseum {
extern fProf::fBaseProfile_c g_profile_MENU_MUSEUM;
} // namespace dMenuMuseum

namespace dMenuTime00 {
extern fProf::fBaseProfile_c g_profile_MENU_TIME00;
} // namespace dMenuTime00

namespace dMenuWrite {
extern fProf::fBaseProfile_c g_profile_MENU_WRITE;
} // namespace dMenuWrite

namespace dMenuBank {
extern fProf::fBaseProfile_c g_profile_MENU_BANK;
} // namespace dMenuBank

namespace dMenuKabu {
extern fProf::fBaseProfile_c g_profile_MENU_KABU;
} // namespace dMenuKabu

namespace dMenuMslct {
extern fProf::fBaseProfile_c g_profile_MENU_MSLCT;
} // namespace dMenuMslct

namespace dMenuMii {
extern fProf::fBaseProfile_c g_profile_MENU_MII;
} // namespace dMenuMii

namespace dMenuDesi01 {
extern fProf::fBaseProfile_c g_profile_MENU_DESI01;
} // namespace dMenuDesi01

namespace dMenuDesi02 {
extern fProf::fBaseProfile_c g_profile_MENU_DESI02;
} // namespace dMenuDesi02

namespace dMenuDesi03 {
extern fProf::fBaseProfile_c g_profile_MENU_DESI03;
} // namespace dMenuDesi03

namespace dMenuMusic {
extern fProf::fBaseProfile_c g_profile_MENU_MUSIC;
} // namespace dMenuMusic

namespace dMenuPost00 {
extern fProf::fBaseProfile_c g_profile_MENU_POST00;
} // namespace dMenuPost00

namespace dMenuPost01 {
extern fProf::fBaseProfile_c g_profile_MENU_POST01;
} // namespace dMenuPost01

namespace dMenuNotice {
extern fProf::fBaseProfile_c g_profile_MENU_NOTICE;
} // namespace dMenuNotice

namespace dMenuNotice00 {
extern fProf::fBaseProfile_c g_profile_MENU_NOTICE00;
} // namespace dMenuNotice00

namespace dMenuNotice01 {
extern fProf::fBaseProfile_c g_profile_MENU_NOTICE01;
} // namespace dMenuNotice01

namespace dMenuMelody {
extern fProf::fBaseProfile_c g_profile_MENU_MELODY;
} // namespace dMenuMelody

namespace dMenuDebug00 {
extern fProf::fBaseProfile_c g_profile;
} // namespace dMenuDebug00

namespace dMenuStar00 {
extern fProf::fBaseProfile_c g_profile_MENU_STAR00;
} // namespace dMenuStar00

namespace dMenuStar01 {
extern fProf::fBaseProfile_c g_profile_MENU_STAR01;
} // namespace dMenuStar01

namespace dMenuPoint {
extern fProf::fBaseProfile_c g_profile_MENU_POINT;
} // namespace dMenuPoint

namespace dMenuHaniwa {
extern fProf::fBaseProfile_c g_profile_MENU_HANIWA;
} // namespace dMenuHaniwa

namespace dAMdl {
extern fProf::fBaseProfile_c g_profile_A_MDL;
} // namespace dAMdl

namespace dBgtDraw {
extern fProf::fBaseProfile_c g_profile_BGT_DRAW;
} // namespace dBgtDraw

namespace dBgoDraw {
extern fProf::fBaseProfile_c g_profile_BGO_DRAW;
} // namespace dBgoDraw

namespace dBgiDraw {
extern fProf::fBaseProfile_c g_profile_BGI_DRAW;
} // namespace dBgiDraw

namespace dFgDraw {
extern fProf::fBaseProfile_c g_profile_FG_DRAW;
} // namespace dFgDraw

namespace dFgobjManager {
extern fProf::fBaseProfile_c g_profile_FGOBJ_MANAGER;
} // namespace dFgobjManager

namespace dShopFg {
extern fProf::fBaseProfile_c g_profile_SHOP_FG;
} // namespace dShopFg

namespace dShadow {
extern fProf::fBaseProfile_c g_profile_SHADOW;
} // namespace dShadow

namespace dANameplate {
extern fProf::fBaseProfile_c g_profile_A_NAMEPLATE;
} // namespace dANameplate

namespace dANameplateMuseum {
extern fProf::fBaseProfile_c g_profile_A_NAMEPLATE_MUSEUM;
} // namespace dANameplateMuseum

namespace dASignTag {
extern fProf::fBaseProfile_c g_profile_A_SIGN_TAG;
} // namespace dASignTag

namespace dObjStaffroll {
extern fProf::fBaseProfile_c g_profile_OBJ_STAFFROLL;
} // namespace dObjStaffroll

namespace dAStr {
extern fProf::fBaseProfile_c g_profile_A_STR;
} // namespace dAStr

namespace dAStrField {
extern fProf::fBaseProfile_c g_profile_STR_PL;
extern fProf::fBaseProfile_c g_profile_STR_POST;
extern fProf::fBaseProfile_c g_profile_STR_NPC;
extern fProf::fBaseProfile_c g_profile_STR_OFFICE;
extern fProf::fBaseProfile_c g_profile_STR_SHOP;
extern fProf::fBaseProfile_c g_profile_STR_MUSEUM;
extern fProf::fBaseProfile_c g_profile_STR_RESERVE;
extern fProf::fBaseProfile_c g_profile_STR_CHKP;
extern fProf::fBaseProfile_c g_profile_STR_TAILOR;
extern fProf::fBaseProfile_c g_profile_STR_BBS;
extern fProf::fBaseProfile_c g_profile_STR_LIGHT_HS;
extern fProf::fBaseProfile_c g_profile_STR_WINDMILL;
extern fProf::fBaseProfile_c g_profile_STR_TENT;
extern fProf::fBaseProfile_c g_profile_STR_HARVEST;
extern fProf::fBaseProfile_c g_profile_STR_FOUNTAIN;
extern fProf::fBaseProfile_c g_profile_STR_UFO;
extern fProf::fBaseProfile_c g_profile_STR_CURNIVAL;
} // namespace dAStrField

namespace dAStrCdbbs {
extern fProf::fBaseProfile_c g_profile_A_STR_CDBBS;
} // namespace dAStrCdbbs

namespace dAStrCommon {
extern fProf::fBaseProfile_c g_profile_STR_BUS;
extern fProf::fBaseProfile_c g_profile_STR_BUS_STOP;
} // namespace dAStrCommon

namespace dAStrTown {
extern fProf::fBaseProfile_c g_profile_STR_TOWN;
extern fProf::fBaseProfile_c g_profile_STR_ATM;
extern fProf::fBaseProfile_c g_profile_STR_BARBER;
extern fProf::fBaseProfile_c g_profile_STR_HAPPY;
extern fProf::fBaseProfile_c g_profile_STR_BROKER;
extern fProf::fBaseProfile_c g_profile_STR_FORTUNE;
extern fProf::fBaseProfile_c g_profile_STR_GRACE;
extern fProf::fBaseProfile_c g_profile_STR_RESET;
extern fProf::fBaseProfile_c g_profile_STR_LAMP;
extern fProf::fBaseProfile_c g_profile_STR_AUCTION;
extern fProf::fBaseProfile_c g_profile_STR_THEATER;
extern fProf::fBaseProfile_c g_profile_STR_TW_FOUNTAIN;
extern fProf::fBaseProfile_c g_profile_STR_ORGAN;
extern fProf::fBaseProfile_c g_profile_STR_CHEST;
} // namespace dAStrTown

namespace dFlag {
extern fProf::fBaseProfile_c g_profile_FLAG;
} // namespace dFlag

namespace dCountdownFlag {
extern fProf::fBaseProfile_c g_profile_COUNTDOWN_FLAG;
} // namespace dCountdownFlag

namespace dOrganBalloon {
extern fProf::fBaseProfile_c g_profile_ORGAN_BALLOON;
} // namespace dOrganBalloon

namespace dABalloon {
extern fProf::fBaseProfile_c g_profile_BALLOON_MNG;
extern fProf::fBaseProfile_c g_profile_BALLOON;
} // namespace dABalloon

namespace dAUfo {
extern fProf::fBaseProfile_c g_profile_A_UFO;
} // namespace dAUfo

namespace dStrManager {
extern fProf::fBaseProfile_c g_profile_STR_MANAGER;
} // namespace dStrManager

namespace dABbsBird {
extern fProf::fBaseProfile_c g_profile_A_BBS_BIRD;
} // namespace dABbsBird

namespace dProfile {

// Profile-number order from the original REL (fProf::PROFILE_NAME_e).
static const fProfListMg_c::fProfilePtr_c l_profiles[fProf::PROFILE_COUNT] = {
    // fProf::ROOT
    {&g_profile_ROOT},
    // fProf::S_STAGE
    {&dSStage::g_profile_S_STAGE},
    // fProf::OBJ_CURSOR
    {&g_profile_OBJ_CURSOR},
    // fProf::AC_UITAG
    {&dAcUitag::g_profile_AC_UITAG},
    // fProf::MSG
    {&g_profile_MSG},
    // fProf::CLOCK
    {&g_profile_CLOCK},
    // fProf::MONEY
    {&g_profile_MONEY},
    // fProf::OBJ_KEYBOARD
    {&g_profile_OBJ_KEYBOARD},
    // fProf::A_GROWUP_MSG
    {&dAGrowupMsg::g_profile_A_GROWUP_MSG},
    // fProf::A_PLAYER
    {&dAPlayer::g_profile_A_PLAYER},
    // fProf::PLAYER_MNGR
    {&g_profile_PLAYER_MNGR},
    // fProf::CD
    {&g_profile_CD},
    // fProf::ORG_OBJ
    {&dOrgObj::g_profile_ORG_OBJ},
    // fProf::A_RMOBJ_ATM
    {&dARmobjAtm::g_profile_A_RMOBJ_ATM},
    // fProf::A_RMOBJ_BARRICADE
    {&dARmobjBarricade::g_profile_A_RMOBJ_BARRICADE},
    // fProf::A_RMOBJ_BUS
    {&dARmobjBus::g_profile_A_RMOBJ_BUS},
    // fProf::A_RMOBJ_CAFE
    {&dARmobjCafe::g_profile_A_RMOBJ_CAFE},
    // fProf::A_RMOBJ_CHECKP
    {&dARmobjCheckp::g_profile_A_RMOBJ_CHECKP},
    // fProf::A_RMOBJ_FOSSIL
    {&dARmobjFossil::g_profile_A_RMOBJ_FOSSIL},
    // fProf::A_RMOBJ_GRACE
    {&dARmobjGrace::g_profile_A_RMOBJ_GRACE},
    // fProf::A_RMOBJ_PERM_MACHINE
    {&dARmobjPermMachine::g_profile_A_RMOBJ_PERM_MACHINE},
    // fProf::A_RMOBJ_PIC
    {&dARmobjPic::g_profile_A_RMOBJ_PIC},
    // fProf::A_RMOBJ_POINT
    {&dARmobjPoint::g_profile_A_RMOBJ_POINT},
    // fProf::A_RMOBJ_RECYCLEBOX
    {&dARmobjRecyclebox::g_profile_A_RMOBJ_RECYCLEBOX},
    // fProf::A_RMOBJ_RESET
    {&dARmobjReset::g_profile_A_RMOBJ_RESET},
    // fProf::A_RMOBJ_SAMPLE
    {&dARmobjSample::g_profile_A_RMOBJ_SAMPLE},
    // fProf::A_RMOBJ_SEWING_MACHINE
    {&dARmobjSewingMachine::g_profile_A_RMOBJ_SEWING_MACHINE},
    // fProf::A_RMOBJ_TEL
    {&dARmobjTel::g_profile_A_RMOBJ_TEL},
    // fProf::A_RMOBJ_THEATER
    {&dARmobjTheater::g_profile_A_RMOBJ_THEATER},
    // fProf::A_RMOBJ_MIRRORBALL
    {&dARmobjMirrorball::g_profile_A_RMOBJ_MIRRORBALL},
    // fProf::A_TICKET_COUNTER
    {&dATicketCounter::g_profile_A_TICKET_COUNTER},
    // fProf::COCKROACH_MANAGER
    {&dCockroachManager::g_profile_COCKROACH_MANAGER},
    // fProf::A_COCKROACH
    {&dACockroach::g_profile_A_COCKROACH},
    // fProf::A_FTR
    {&dAFtr::g_profile_A_FTR},
    // fProf::FTR_LP
    {&dAFtrAct::g_profile_FTR_LP},
    // fProf::FTR_TRG
    {&dAFtrAct::g_profile_FTR_TRG},
    // fProf::FTR_SW
    {&dAFtrAct::g_profile_FTR_SW},
    // fProf::FTR_SW2
    {&dAFtrAct::g_profile_FTR_SW2},
    // fProf::FTR_FLP
    {&dAFtrAct::g_profile_FTR_FLP},
    // fProf::FTR2_W
    {&dAFtrAct::g_profile_FTR2_W},
    // fProf::FTR_FSW
    {&dAFtrAct::g_profile_FTR_FSW},
    // fProf::FTR_CHAIR
    {&dAFtrAct::g_profile_FTR_CHAIR},
    // fProf::FTR_BED
    {&dAFtrAct::g_profile_FTR_BED},
    // fProf::FTR_CHEST
    {&dAFtrAct::g_profile_FTR_CHEST},
    // fProf::FTR_CLOCK
    {&dAFtrAct::g_profile_FTR_CLOCK},
    // fProf::FTR_AUDIO
    {&dAFtrAct::g_profile_FTR_AUDIO},
    // fProf::FTR_TV
    {&dAFtrAct::g_profile_FTR_TV},
    // fProf::FTR_TV_F
    {&dAFtrAct::g_profile_FTR_TV_F},
    // fProf::FTR_PCART
    {&dAFtrAct::g_profile_FTR_PCART},
    // fProf::FTR_HNW
    {&dAFtrAct::g_profile_FTR_HNW},
    // fProf::FTR_FAW
    {&dAFtrAct::g_profile_FTR_FAW},
    // fProf::FTR_KILLER
    {&dAFtrAct::g_profile_FTR_KILLER},
    // fProf::FTR_CLOTH
    {&dAFtrAct::g_profile_FTR_CLOTH},
    // fProf::FTR_MYOBJ
    {&dAFtrAct::g_profile_FTR_MYOBJ},
    // fProf::FTR_TAOBJ
    {&dAFtrAct::g_profile_FTR_TAOBJ},
    // fProf::FTR_CMPS
    {&dAFtrAct::g_profile_FTR_CMPS},
    // fProf::FTR_INSTRUMENT
    {&dAFtrAct::g_profile_FTR_INSTRUMENT},
    // fProf::FTR_BK
    {&dAFtrAct::g_profile_FTR_BK},
    // fProf::FTR_METRO
    {&dAFtrAct::g_profile_FTR_METRO},
    // fProf::FTR_CATALOG_CW
    {&dAFtrAct::g_profile_FTR_CATALOG_CW},
    // fProf::FTR_ACC
    {&dAFtrAct::g_profile_FTR_ACC},
    // fProf::FTR_UMB
    {&dAFtrAct::g_profile_FTR_UMB},
    // fProf::FTR_BALLOON
    {&dAFtrAct::g_profile_FTR_BALLOON},
    // fProf::FTR_WINDMILL
    {&dAFtrAct::g_profile_FTR_WINDMILL},
    // fProf::FTR_MONITOR
    {&dAFtrAct::g_profile_FTR_MONITOR},
    // fProf::FTR_CICADA
    {&dAFtrAct::g_profile_FTR_CICADA},
    // fProf::FTR_TEL
    {&dAFtrAct::g_profile_FTR_TEL},
    // fProf::FTR_FLOWER
    {&dAFtrAct::g_profile_FTR_FLOWER},
    // fProf::FTR_ATM
    {&dAFtrAct::g_profile_FTR_ATM},
    // fProf::FTR_BARLEY
    {&dAFtrAct::g_profile_FTR_BARLEY},
    // fProf::FTR_ROLLER
    {&dAFtrAct::g_profile_FTR_ROLLER},
    // fProf::FTR_MANAGER
    {&dFtrManager::g_profile_FTR_MANAGER},
    // fProf::A_SAMPLE
    {&dASample::g_profile_A_SAMPLE},
    // fProf::A_SAMPLE2
    {&dASample2::g_profile_A_SAMPLE2},
    // fProf::A_SAMPLE3
    {&dASample3::g_profile_A_SAMPLE3},
    // fProf::SNOWMAN
    {&g_profile_SNOWMAN},
    // fProf::A_NPC_OUT
    {&dANpcOut::g_profile_A_NPC_OUT},
    // fProf::A_NPC_IN
    {&dANpcIn::g_profile_A_NPC_IN},
    // fProf::A_NPC_VISIT_NPC_HOUSE
    {&dANpcVisitNpcHouse::g_profile_A_NPC_VISIT_NPC_HOUSE},
    // fProf::A_NPC_VISIT_PLAYER_HOUSE
    {&dANpcVisitPlayerHouse::g_profile_A_NPC_VISIT_PLAYER_HOUSE},
    // fProf::A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET
    {&dANpcVisitPlayerHouseFleamarket::
         g_profile_A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET},
    // fProf::A_NPC_TOWN_OUT
    {&dANpcTownOut::g_profile_A_NPC_TOWN_OUT},
    // fProf::A_NPC_TOWN_GRACE
    {&dANpcTownGrace::g_profile_A_NPC_TOWN_GRACE},
    // fProf::A_NPC_TOWN_THEATER
    {&dANpcTownTheater::g_profile_A_NPC_TOWN_THEATER},
    // fProf::A_NPC_CAFE
    {&dANpcCafe::g_profile_A_NPC_CAFE},
    // fProf::A_NPC_SP_SAMPLE
    {&dANpcSpSample::g_profile_A_NPC_SP_SAMPLE},
    // fProf::A_NPC_SP_PLAYER_CREATE
    {&dANpcSpPlayerCreate::g_profile_A_NPC_SP_PLAYER_CREATE},
    // fProf::A_NPC_SP_PLAYER_SELECT
    {&dANpcSpPlayerSelect::g_profile_A_NPC_SP_PLAYER_SELECT},
    // fProf::A_NPC_SP_RESETSAN
    {&dANpcSpResetsan::g_profile_A_NPC_SP_RESETSAN},
    // fProf::A_NPC_SP_CAFE_GUEST
    {&dANpcSpCafeGuest::g_profile_A_NPC_SP_CAFE_GUEST},
    // fProf::A_NPC_SP_PERIKO_05A
    {&dANpcSpPeriko::g_profile_NPC_SP_PERIKO},
    // fProf::A_NPC_SP_PERIKO_05B
    {&dANpcSpPeriko::g_profile_NPC_SP_PERIMI},
    // fProf::A_NPC_SP_SONCHO
    {&dANpcSpSoncho::g_profile_A_NPC_SP_SONCHO},
    // fProf::A_NPC_SP_OP_TANUKICHI
    {&dANpcSpOpTanukichi::g_profile_A_NPC_SP_OP_TANUKICHI},
    // fProf::A_NPC_SP_TANUKICHI_05E
    {&dANpcSpTanukichi::g_profile_NPC_SP_TANUKICHI},
    // fProf::A_NPC_SP_TANUKICHI_05F
    {&dANpcSpTanukichi::g_profile_NPC_SP_MAMEKICHI},
    // fProf::A_NPC_SP_TANUKICHI_060
    {&dANpcSpTanukichi::g_profile_NPC_SP_TUBUKICHI},
    // fProf::A_NPC_SP_KABURIBA
    {&dANpcSpKaburiba::g_profile_A_NPC_SP_KABURIBA},
    // fProf::A_NPC_SP_TUNEKICHI
    {&dANpcSpTunekichi::g_profile_A_NPC_SP_TUNEKICHI},
    // fProf::A_NPC_SP_CATHERINE
    {&dANpcSpCatherine::g_profile_A_NPC_SP_CATHERINE},
    // fProf::A_NPC_SP_ASAMI
    {&dANpcSpAsami::g_profile_A_NPC_SP_ASAMI},
    // fProf::A_NPC_SP_KINUYO
    {&dANpcSpKinuyo::g_profile_A_NPC_SP_KINUYO},
    // fProf::A_NPC_SP_JIJYO
    {&dANpcSpJijyo::g_profile_A_NPC_SP_JIJYO},
    // fProf::A_NPC_SP_CAFE
    {&dANpcSpCafe::g_profile_A_NPC_SP_CAFE},
    // fProf::A_NPC_SP_FUTA
    {&dANpcSpFuta::g_profile_A_NPC_SP_FUTA},
    // fProf::A_NPC_SP_FUKO
    {&dANpcSpFuko::g_profile_A_NPC_SP_FUKO},
    // fProf::A_NPC_SP_SEIICHI
    {&dANpcSpSeiichi::g_profile_A_NPC_SP_SEIICHI},
    // fProf::A_NPC_SP_RORAN
    {&dANpcSpRoran::g_profile_A_NPC_SP_RORAN},
    // fProf::A_NPC_SP_KAPPEI
    {&dANpcSpKappei::g_profile_A_NPC_SP_KAPPEI},
    // fProf::A_NPC_SP_TOTAKEKE
    {&dANpcSpTotakeke::g_profile_A_NPC_SP_TOTAKEKE},
    // fProf::A_NPC_SP_JONNY
    {&dANpcSpJonny::g_profile_A_NPC_SP_JONNY},
    // fProf::A_NPC_SP_RACCO
    {&dANpcSpRacco::g_profile_A_NPC_SP_RACCO},
    // fProf::A_NPC_SP_PERIO
    {&dANpcSpPerio::g_profile_A_NPC_SP_PERIO},
    // fProf::A_NPC_SP_FORTUNE
    {&dANpcSpFortune::g_profile_A_NPC_SP_FORTUNE},
    // fProf::A_NPC_SP_SISYO
    {&dANpcSpSisyo::g_profile_A_NPC_SP_SISYO},
    // fProf::A_NPC_SP_HONMA
    {&dANpcSpHonma::g_profile_A_NPC_SP_HONMA},
    // fProf::A_NPC_SP_GRACE
    {&dANpcSpGrace::g_profile_A_NPC_SP_GRACE},
    // fProf::A_NPC_SP_GATEKEEPER_A
    {&dANpcSpGatekeeperA::g_profile_A_NPC_SP_GATEKEEPER_A},
    // fProf::A_NPC_SP_GATEKEEPER_B
    {&dANpcSpGatekeeperB::g_profile_A_NPC_SP_GATEKEEPER_B},
    // fProf::A_NPC_SP_JINGLE
    {&dANpcSpJingle::g_profile_A_NPC_SP_JINGLE},
    // fProf::A_NPC_SP_FRANKLIN
    {&dANpcSpFranklin::g_profile_A_NPC_SP_FRANKLIN},
    // fProf::A_NPC_SP_UOMASA
    {&dANpcSpUomasa::g_profile_A_NPC_SP_UOMASA},
    // fProf::A_NPC_SP_PUMPKING
    {&dANpcSpPumpking::g_profile_A_NPC_SP_PUMPKING},
    // fProf::A_NPC_SP_KUTSUMIGAKI
    {&dANpcSpKutsumigaki::g_profile_A_NPC_SP_KUTSUMIGAKI},
    // fProf::A_NPC_SP_EVENT_SONCHO_07C
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO_CD},
    // fProf::A_NPC_SP_EVENT_SONCHO_07D
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO_NY},
    // fProf::A_NPC_SP_EVENT_SONCHO_07E
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO_HF},
    // fProf::A_NPC_SP_EVENT_SONCHO_07F
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO_HT},
    // fProf::A_NPC_SP_EVENT_SONCHO_080
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO01},
    // fProf::A_NPC_SP_EVENT_SONCHO_081
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO02},
    // fProf::A_NPC_SP_EVENT_SONCHO_082
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO03},
    // fProf::A_NPC_SP_EVENT_SONCHO_083
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO04},
    // fProf::A_NPC_SP_EVENT_SONCHO_084
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO05},
    // fProf::A_NPC_SP_EVENT_SONCHO_085
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO06},
    // fProf::A_NPC_SP_EVENT_SONCHO_086
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO07},
    // fProf::A_NPC_SP_EVENT_SONCHO_087
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO08},
    // fProf::A_NPC_SP_EVENT_SONCHO_088
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO09},
    // fProf::A_NPC_SP_EVENT_SONCHO_089
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO10},
    // fProf::A_NPC_SP_EVENT_SONCHO_08A
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO11},
    // fProf::A_NPC_SP_EVENT_SONCHO_08B
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO12},
    // fProf::A_NPC_SP_EVENT_SONCHO_08C
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO13},
    // fProf::A_NPC_SP_EVENT_SONCHO_08D
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO14},
    // fProf::A_NPC_SP_EVENT_SONCHO_08E
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO15},
    // fProf::A_NPC_SP_EVENT_SONCHO_08F
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO16},
    // fProf::A_NPC_SP_EVENT_SONCHO_090
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO17},
    // fProf::A_NPC_SP_EVENT_SONCHO_091
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO18},
    // fProf::A_NPC_SP_EVENT_SONCHO_092
    {&dANpcSpEventSoncho::g_profile_NPC_SP_EV_SONCHO19},
    // fProf::A_NPC_SP_CEREMONY_SONCHO
    {&dANpcSpCeremonySoncho::g_profile_A_NPC_SP_CEREMONY_SONCHO},
    // fProf::A_NPC_SP_GOLDITEM_SONCHO
    {&dANpcSpGolditemSoncho::g_profile_A_NPC_SP_GOLDITEM_SONCHO},
    // fProf::A_NPC_SP_PYONTARO
    {&dANpcSpPyontaro::g_profile_A_NPC_SP_PYONTARO},
    // fProf::A_NPC_SP_GODDESS
    {&dANpcSpGoddess::g_profile_A_NPC_SP_GODDESS},
    // fProf::A_NPC_SP_BUG
    {&dANpcSpBug::g_profile_A_NPC_SP_BUG},
    // fProf::A_NPC_SP_HANIWA
    {&dANpcSpHaniwa::g_profile_A_NPC_SP_HANIWA},
    // fProf::A_NPC_SP_DEMO_RESETSAN_099
    {&dANpcSpDemoResetsan::g_profile_NPC_SP_DEMO_RESETSAN},
    // fProf::A_NPC_SP_DEMO_RESETSAN_09A
    {&dANpcSpDemoResetsan::g_profile_NPC_SP_DEMO_RACKETSAN},
    // fProf::A_NPC_SP_DANCER
    {&dANpcSpDancer::g_profile_A_NPC_SP_DANCER},
    // fProf::A_NPC_SP_MISSING_CHILD
    {&dANpcSpMissingChild::g_profile_A_NPC_SP_MISSING_CHILD},
    // fProf::A_NPC_SP_MISSING_MOTHER
    {&dANpcSpMissingMother::g_profile_A_NPC_SP_MISSING_MOTHER},
    // fProf::A_NPC_SP_MYSTERYCAT
    {&dANpcSpMysterycat::g_profile_A_NPC_SP_MYSTERYCAT},
    // fProf::A_NPC_SP_YUUTARO
    {&dANpcSpYuutaro::g_profile_A_NPC_SP_YUUTARO},
    // fProf::A_NPC_SP_YUUTARO_LOFT
    {&dANpcSpYuutaroLoft::g_profile_A_NPC_SP_YUUTARO_LOFT},
    // fProf::A_NPC_SP_FUUSEN
    {&dANpcSpFuusen::g_profile_A_NPC_SP_FUUSEN},
    // fProf::S_SELECT
    {&dSSelect::g_profile_S_SELECT},
    // fProf::SV_DEMO
    {&g_profile_SV_DEMO},
    // fProf::SV_MGR
    {&g_profile_SV_MGR},
    // fProf::TITLE_DEMO
    {&dTitleDemo::g_profile_TITLE_DEMO},
    // fProf::BOOT
    {&g_profile_BOOT},
    // fProf::CAMERA
    {&g_profile_CAMERA},
    // fProf::FIELD
    {&dField::g_profile_FIELD},
    // fProf::WEATHER
    {&g_profile_WEATHER},
    // fProf::SKY
    {&g_profile_SKY},
    // fProf::SKY_AURORA
    {&g_profile_SKY_AURORA},
    // fProf::SKY_RAINBOW
    {&g_profile_SKY_RAINBOW},
    // fProf::NPC_MNG
    {&g_profile_NPC_MNG},
    // fProf::HAND_ITEM
    {&g_profile_HAND_ITEM},
    // fProf::FI_MDL_MGR
    {&g_profile_FI_MDL_MGR},
    // fProf::FISH_MUSEUM
    {&dFishMuseum::g_profile_FISH_MUSEUM},
    // fProf::INSECT_MUSEUM
    {&dInsectMuseum::g_profile_INSECT_MUSEUM},
    // fProf::FISH_FIELD
    {&g_profile_FISH_FIELD},
    // fProf::INSECT_FIELD
    {&dInsectField::g_profile_INSECT_FIELD},
    // fProf::A_LAYOUT
    {&dALayout::g_profile_A_LAYOUT},
    // fProf::MENU_MNG
    {&g_profile_MENU_MNG},
    // fProf::MENU_ITEM00
    {&dMenuItem00::g_profile_MENU_ITEM00},
    // fProf::MENU_ITEM01
    {&dMenuItem01::g_profile_MENU_ITEM01},
    // fProf::MENU_ITEM02
    {&dMenuItem02::g_profile_MENU_ITEM02},
    // fProf::MENU_ITEM03
    {&dMenuItem03::g_profile_MENU_ITEM03},
    // fProf::MENU_ITEM04
    {&dMenuItem04::g_profile_MENU_ITEM04},
    // fProf::MENU_ITEM05
    {&dMenuItem05::g_profile_MENU_ITEM05},
    // fProf::MENU_ITEM06
    {&dMenuItem06::g_profile_MENU_ITEM06},
    // fProf::MENU_FRIEND
    {&dMenuFriend::g_profile_MENU_FRIEND},
    // fProf::MENU_FRIEND01
    {&dMenuFriend01::g_profile_MENU_FRIEND01},
    // fProf::MENU_FRIEND02
    {&dMenuFriend02::g_profile_MENU_FRIEND02},
    // fProf::MENU_FRIEND03
    {&dMenuFriend03::g_profile_MENU_FRIEND03},
    // fProf::MENU_FRIEND04
    {&dMenuFriend04::g_profile_MENU_FRIEND04},
    // fProf::MENU_FRIEND05
    {&dMenuFriend05::g_profile_MENU_FRIEND05},
    // fProf::MENU_FRIEND06
    {&dMenuFriend06::g_profile_MENU_FRIEND06},
    // fProf::MENU_FRIEND07
    {&dMenuFriend07::g_profile_MENU_FRIEND07},
    // fProf::MENU_FRIEND08
    {&dMenuFriend08::g_profile_MENU_FRIEND08},
    // fProf::MENU_INSC
    {&dMenuInsc::g_profile_MENU_INSC},
    // fProf::MENU_CHAT
    {&dMenuChat::g_profile_MENU_CHAT},
    // fProf::MENU_DESI00
    {&dMenuDesi00::g_profile_MENU_DESI00},
    // fProf::MENU_EDIT
    {&dMenuEdit::g_profile_MENU_EDIT},
    // fProf::MENU_PAPER
    {&dMenuPaper::g_profile_MENU_PAPER},
    // fProf::MENU_KEYTEST
    {&dMenuKeytest::g_profile_MENU_KEYTEST},
    // fProf::MENU_HAN
    {&dMenuHan::g_profile_MENU_HAN},
    // fProf::MENU_MAP00
    {&dMenuMap00::g_profile_MENU_MAP00},
    // fProf::MENU_MAP01
    {&dMenuMap01::g_profile_MENU_MAP01},
    // fProf::MENU_MAP02
    {&dMenuMap02::g_profile_MENU_MAP02},
    // fProf::MENU_MAP03
    {&dMenuMap03::g_profile_MENU_MAP03},
    // fProf::MENU_PHOTO
    {&dMenuPhoto::g_profile_MENU_PHOTO},
    // fProf::MENU_PHOTO01
    {&dMenuPhoto01::g_profile_MENU_PHOTO01},
    // fProf::MENU_CATALOG
    {&dMenuCatalog::g_profile_MENU_CATALOG},
    // fProf::MENU_NUMBER00
    {&dMenuNumber00::g_profile_MENU_NUMBER00},
    // fProf::MENU_MUSEUM
    {&dMenuMuseum::g_profile_MENU_MUSEUM},
    // fProf::MENU_TIME00
    {&dMenuTime00::g_profile_MENU_TIME00},
    // fProf::MENU_WRITE
    {&dMenuWrite::g_profile_MENU_WRITE},
    // fProf::MENU_BANK
    {&dMenuBank::g_profile_MENU_BANK},
    // fProf::MENU_KABU
    {&dMenuKabu::g_profile_MENU_KABU},
    // fProf::MENU_MSLCT
    {&dMenuMslct::g_profile_MENU_MSLCT},
    // fProf::MENU_MII
    {&dMenuMii::g_profile_MENU_MII},
    // fProf::MENU_DESI01
    {&dMenuDesi01::g_profile_MENU_DESI01},
    // fProf::MENU_DESI02
    {&dMenuDesi02::g_profile_MENU_DESI02},
    // fProf::MENU_DESI03
    {&dMenuDesi03::g_profile_MENU_DESI03},
    // fProf::MENU_MUSIC
    {&dMenuMusic::g_profile_MENU_MUSIC},
    // fProf::MENU_POST00
    {&dMenuPost00::g_profile_MENU_POST00},
    // fProf::MENU_POST01
    {&dMenuPost01::g_profile_MENU_POST01},
    // fProf::MENU_NOTICE
    {&dMenuNotice::g_profile_MENU_NOTICE},
    // fProf::MENU_NOTICE00
    {&dMenuNotice00::g_profile_MENU_NOTICE00},
    // fProf::MENU_NOTICE01
    {&dMenuNotice01::g_profile_MENU_NOTICE01},
    // fProf::MENU_MELODY
    {&dMenuMelody::g_profile_MENU_MELODY},
    // fProf::MENU_DEBUG00
    {&dMenuDebug00::g_profile},
    // fProf::MENU_STAR00
    {&dMenuStar00::g_profile_MENU_STAR00},
    // fProf::MENU_STAR01
    {&dMenuStar01::g_profile_MENU_STAR01},
    // fProf::MENU_POINT
    {&dMenuPoint::g_profile_MENU_POINT},
    // fProf::MENU_HANIWA
    {&dMenuHaniwa::g_profile_MENU_HANIWA},
    // fProf::A_MDL
    {&dAMdl::g_profile_A_MDL},
    // fProf::BGT_DRAW
    {&dBgtDraw::g_profile_BGT_DRAW},
    // fProf::BGO_DRAW
    {&dBgoDraw::g_profile_BGO_DRAW},
    // fProf::BGI_DRAW
    {&dBgiDraw::g_profile_BGI_DRAW},
    // fProf::FG_DRAW
    {&dFgDraw::g_profile_FG_DRAW},
    // fProf::FGOBJ_MANAGER
    {&dFgobjManager::g_profile_FGOBJ_MANAGER},
    // fProf::SHOP_FG
    {&dShopFg::g_profile_SHOP_FG},
    // fProf::BG_DEBUG
    {&g_profile_BG_DEBUG},
    // fProf::SHADOW
    {&dShadow::g_profile_SHADOW},
    // fProf::A_NAMEPLATE
    {&dANameplate::g_profile_A_NAMEPLATE},
    // fProf::A_NAMEPLATE_MUSEUM
    {&dANameplateMuseum::g_profile_A_NAMEPLATE_MUSEUM},
    // fProf::A_SIGN_TAG
    {&dASignTag::g_profile_A_SIGN_TAG},
    // fProf::OBJ_BAL
    {&g_profile_OBJ_BAL},
    // fProf::OBJ_VOICE
    {&g_profile_OBJ_VOICE},
    // fProf::OBJ_STAFFROLL
    {&dObjStaffroll::g_profile_OBJ_STAFFROLL},
    // fProf::A_STR
    {&dAStr::g_profile_A_STR},
    // fProf::A_STR_FIELD_0FB
    {&dAStrField::g_profile_STR_PL},
    // fProf::A_STR_FIELD_0FC
    {&dAStrField::g_profile_STR_POST},
    // fProf::A_STR_FIELD_0FD
    {&dAStrField::g_profile_STR_NPC},
    // fProf::A_STR_FIELD_0FE
    {&dAStrField::g_profile_STR_OFFICE},
    // fProf::A_STR_FIELD_0FF
    {&dAStrField::g_profile_STR_SHOP},
    // fProf::A_STR_FIELD_100
    {&dAStrField::g_profile_STR_MUSEUM},
    // fProf::A_STR_FIELD_101
    {&dAStrField::g_profile_STR_RESERVE},
    // fProf::A_STR_FIELD_102
    {&dAStrField::g_profile_STR_CHKP},
    // fProf::A_STR_FIELD_103
    {&dAStrField::g_profile_STR_TAILOR},
    // fProf::A_STR_CDBBS
    {&dAStrCdbbs::g_profile_A_STR_CDBBS},
    // fProf::A_STR_FIELD_105
    {&dAStrField::g_profile_STR_BBS},
    // fProf::A_STR_FIELD_106
    {&dAStrField::g_profile_STR_LIGHT_HS},
    // fProf::A_STR_FIELD_107
    {&dAStrField::g_profile_STR_WINDMILL},
    // fProf::A_STR_FIELD_108
    {&dAStrField::g_profile_STR_TENT},
    // fProf::A_STR_FIELD_109
    {&dAStrField::g_profile_STR_HARVEST},
    // fProf::A_STR_FIELD_10A
    {&dAStrField::g_profile_STR_FOUNTAIN},
    // fProf::A_STR_FIELD_10B
    {&dAStrField::g_profile_STR_UFO},
    // fProf::STR_BUS
    {&dAStrCommon::g_profile_STR_BUS},
    // fProf::STR_BUS_STOP
    {&dAStrCommon::g_profile_STR_BUS_STOP},
    // fProf::A_STR_TOWN_10E
    {&dAStrTown::g_profile_STR_TOWN},
    // fProf::A_STR_TOWN_10F
    {&dAStrTown::g_profile_STR_ATM},
    // fProf::A_STR_TOWN_110
    {&dAStrTown::g_profile_STR_BARBER},
    // fProf::A_STR_TOWN_111
    {&dAStrTown::g_profile_STR_HAPPY},
    // fProf::A_STR_TOWN_112
    {&dAStrTown::g_profile_STR_BROKER},
    // fProf::A_STR_TOWN_113
    {&dAStrTown::g_profile_STR_FORTUNE},
    // fProf::A_STR_TOWN_114
    {&dAStrTown::g_profile_STR_GRACE},
    // fProf::A_STR_TOWN_115
    {&dAStrTown::g_profile_STR_RESET},
    // fProf::A_STR_TOWN_116
    {&dAStrTown::g_profile_STR_LAMP},
    // fProf::A_STR_TOWN_117
    {&dAStrTown::g_profile_STR_AUCTION},
    // fProf::A_STR_TOWN_118
    {&dAStrTown::g_profile_STR_THEATER},
    // fProf::A_STR_TOWN_119
    {&dAStrTown::g_profile_STR_TW_FOUNTAIN},
    // fProf::A_STR_TOWN_11A
    {&dAStrTown::g_profile_STR_ORGAN},
    // fProf::A_STR_TOWN_11B
    {&dAStrTown::g_profile_STR_CHEST},
    // fProf::A_STR_FIELD_11C
    {&dAStrField::g_profile_STR_CURNIVAL},
    // fProf::FLAG
    {&dFlag::g_profile_FLAG},
    // fProf::COUNTDOWN_FLAG
    {&dCountdownFlag::g_profile_COUNTDOWN_FLAG},
    // fProf::ORGAN_BALLOON
    {&dOrganBalloon::g_profile_ORGAN_BALLOON},
    // fProf::BALLOON_MNG
    {&dABalloon::g_profile_BALLOON_MNG},
    // fProf::BALLOON
    {&dABalloon::g_profile_BALLOON},
    // fProf::A_UFO
    {&dAUfo::g_profile_A_UFO},
    // fProf::STR_MANAGER
    {&dStrManager::g_profile_STR_MANAGER},
    // fProf::A_BBS_BIRD
    {&dABbsBird::g_profile_A_BBS_BIRD},
    // fProf::OBJ_CHAT
    {&g_profile_OBJ_CHAT},
    // fProf::OBJ_TUTORIAL
    {&g_profile_OBJ_TUTORIAL},
    // fProf::OBJ_TELOP
    {&g_profile_OBJ_TELOP},
    // fProf::EVENT_MNG
    {&g_profile_EVENT_MNG},
    // fProf::DEMO
    {&g_profile_DEMO},
    // fProf::OBJC
    {&g_profile_OBJC},
    // fProf::ENVIRONMENT
    {&g_profile_ENVIRONMENT},
};

// .text:0x5C
void install() {
    fProfListMg_c::m_data_p = &l_profiles;
}

// .text:0x70
void remove() {
    fProfListMg_c::m_data_p = NULL;
}

} // namespace dProfile
