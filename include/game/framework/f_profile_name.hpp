#ifndef GAME_FRAMEWORK_F_PROFILE_NAME_HPP
#define GAME_FRAMEWORK_F_PROFILE_NAME_HPP

#include <types.h>

// City Folk (RUUE01_00), in d_profileNP table order. Names are inferred from
// exported profile symbols or their owning modules. A numeric suffix identifies
// an otherwise unidentified variant within a module. FIELD and SHADOW are
// identified by their factory functions in the DOL.
namespace fProf {

enum PROFILE_NAME_e {
    PROFILE_NULL = 0xFFFF,
    ROOT = 0x000,         // g_profile_ROOT
    S_STAGE = 0x001,      // g_profile_S_STAGE__7dSStage
    OBJ_CURSOR = 0x002,   // g_profile_OBJ_CURSOR
    AC_UITAG = 0x003,     // g_profile_AC_UITAG__8dAcUitag
    MSG = 0x004,          // g_profile_MSG
    CLOCK = 0x005,        // g_profile_CLOCK
    MONEY = 0x006,        // g_profile_MONEY
    OBJ_KEYBOARD = 0x007, // g_profile_OBJ_KEYBOARD
    A_GROWUP_MSG = 0x008, // g_profile_A_GROWUP_MSG__11dAGrowupMsg
    A_PLAYER = 0x009,     // g_profile_A_PLAYER__8dAPlayer
    PLAYER_MNGR = 0x00A,  // g_profile_PLAYER_MNGR
    CD = 0x00B,           // g_profile_CD
    ORG_OBJ = 0x00C,      // g_profile_ORG_OBJ__7dOrgObj
    A_RMOBJ_ATM = 0x00D,  // g_profile_A_RMOBJ_ATM__10dARmobjAtm
    A_RMOBJ_BARRICADE =
        0x00E,              // g_profile_A_RMOBJ_BARRICADE__16dARmobjBarricade
    A_RMOBJ_BUS = 0x00F,    // g_profile_A_RMOBJ_BUS__10dARmobjBus
    A_RMOBJ_CAFE = 0x010,   // g_profile_A_RMOBJ_CAFE__11dARmobjCafe
    A_RMOBJ_CHECKP = 0x011, // g_profile_A_RMOBJ_CHECKP__13dARmobjCheckp
    A_RMOBJ_FOSSIL = 0x012, // g_profile_A_RMOBJ_FOSSIL__13dARmobjFossil
    A_RMOBJ_GRACE = 0x013,  // g_profile_A_RMOBJ_GRACE__12dARmobjGrace
    A_RMOBJ_PERM_MACHINE =
        0x014,           // g_profile_A_RMOBJ_PERM_MACHINE__18dARmobjPermMachine
    A_RMOBJ_PIC = 0x015, // g_profile_A_RMOBJ_PIC__10dARmobjPic
    A_RMOBJ_POINT = 0x016, // g_profile_A_RMOBJ_POINT__12dARmobjPoint
    A_RMOBJ_RECYCLEBOX =
        0x017,              // g_profile_A_RMOBJ_RECYCLEBOX__17dARmobjRecyclebox
    A_RMOBJ_RESET = 0x018,  // g_profile_A_RMOBJ_RESET__12dARmobjReset
    A_RMOBJ_SAMPLE = 0x019, // g_profile_A_RMOBJ_SAMPLE__13dARmobjSample
    A_RMOBJ_SEWING_MACHINE =
        0x01A, // g_profile_A_RMOBJ_SEWING_MACHINE__20dARmobjSewingMachine
    A_RMOBJ_TEL = 0x01B,     // g_profile_A_RMOBJ_TEL__10dARmobjTel
    A_RMOBJ_THEATER = 0x01C, // g_profile_A_RMOBJ_THEATER__14dARmobjTheater
    A_RMOBJ_MIRRORBALL =
        0x01D, // g_profile_A_RMOBJ_MIRRORBALL__17dARmobjMirrorball
    A_TICKET_COUNTER = 0x01E, // g_profile_A_TICKET_COUNTER__15dATicketCounter
    COCKROACH_MANAGER =
        0x01F,             // g_profile_COCKROACH_MANAGER__17dCockroachManager
    A_COCKROACH = 0x020,   // g_profile_A_COCKROACH__11dACockroach
    A_FTR = 0x021,         // g_profile_A_FTR__5dAFtr
    FTR_LP = 0x022, // g_profile_FTR_LP__8dAFtrAct
    FTR_TRG = 0x023, // g_profile_FTR_TRG__8dAFtrAct
    FTR_SW = 0x024, // g_profile_FTR_SW__8dAFtrAct
    FTR_SW2 = 0x025, // g_profile_FTR_SW2__8dAFtrAct
    FTR_FLP = 0x026, // g_profile_FTR_FLP__8dAFtrAct
    FTR2_W = 0x027, // g_profile_FTR2_W__8dAFtrAct
    FTR_FSW = 0x028, // g_profile_FTR_FSW__8dAFtrAct
    FTR_CHAIR = 0x029, // g_profile_FTR_CHAIR__8dAFtrAct
    FTR_BED = 0x02A, // g_profile_FTR_BED__8dAFtrAct
    FTR_CHEST = 0x02B, // g_profile_FTR_CHEST__8dAFtrAct
    FTR_CLOCK = 0x02C, // g_profile_FTR_CLOCK__8dAFtrAct
    FTR_AUDIO = 0x02D, // g_profile_FTR_AUDIO__8dAFtrAct
    FTR_TV = 0x02E, // g_profile_FTR_TV__8dAFtrAct
    FTR_TV_F = 0x02F, // g_profile_FTR_TV_F__8dAFtrAct
    FTR_PCART = 0x030, // g_profile_FTR_PCART__8dAFtrAct
    FTR_HNW = 0x031, // g_profile_FTR_HNW__8dAFtrAct
    FTR_FAW = 0x032, // g_profile_FTR_FAW__8dAFtrAct
    FTR_KILLER = 0x033, // g_profile_FTR_KILLER__8dAFtrAct
    FTR_CLOTH = 0x034, // g_profile_FTR_CLOTH__8dAFtrAct
    FTR_MYOBJ = 0x035, // g_profile_FTR_MYOBJ__8dAFtrAct
    FTR_TAOBJ = 0x036, // g_profile_FTR_TAOBJ__8dAFtrAct
    FTR_CMPS = 0x037, // g_profile_FTR_CMPS__8dAFtrAct
    FTR_INSTRUMENT = 0x038, // g_profile_FTR_INSTRUMENT__8dAFtrAct
    FTR_BK = 0x039, // g_profile_FTR_BK__8dAFtrAct
    FTR_METRO = 0x03A, // g_profile_FTR_METRO__8dAFtrAct
    FTR_CATALOG_CW = 0x03B, // g_profile_FTR_CATALOG_CW__8dAFtrAct
    FTR_ACC = 0x03C, // g_profile_FTR_ACC__8dAFtrAct
    FTR_UMB = 0x03D, // g_profile_FTR_UMB__8dAFtrAct
    FTR_BALLOON = 0x03E, // g_profile_FTR_BALLOON__8dAFtrAct
    FTR_WINDMILL = 0x03F, // g_profile_FTR_WINDMILL__8dAFtrAct
    FTR_MONITOR = 0x040, // g_profile_FTR_MONITOR__8dAFtrAct
    FTR_CICADA = 0x041, // g_profile_FTR_CICADA__8dAFtrAct
    FTR_TEL = 0x042, // g_profile_FTR_TEL__8dAFtrAct
    FTR_FLOWER = 0x043, // g_profile_FTR_FLOWER__8dAFtrAct
    FTR_ATM = 0x044, // g_profile_FTR_ATM__8dAFtrAct
    FTR_BARLEY = 0x045, // g_profile_FTR_BARLEY__8dAFtrAct
    FTR_ROLLER = 0x046, // g_profile_FTR_ROLLER__8dAFtrAct
    FTR_MANAGER = 0x047,   // g_profile_FTR_MANAGER__11dFtrManager
    A_SAMPLE = 0x048,      // g_profile_A_SAMPLE__8dASample
    A_SAMPLE2 = 0x049,     // g_profile_A_SAMPLE2__9dASample2
    A_SAMPLE3 = 0x04A,     // g_profile_A_SAMPLE3__9dASample3
    SNOWMAN = 0x04B,       // g_profile_SNOWMAN
    A_NPC_OUT = 0x04C,     // g_profile_A_NPC_OUT__8dANpcOut
    A_NPC_IN = 0x04D,      // g_profile_A_NPC_IN__7dANpcIn
    A_NPC_VISIT_NPC_HOUSE =
        0x04E, // g_profile_A_NPC_VISIT_NPC_HOUSE__18dANpcVisitNpcHouse
    A_NPC_VISIT_PLAYER_HOUSE =
        0x04F, // g_profile_A_NPC_VISIT_PLAYER_HOUSE__21dANpcVisitPlayerHouse
    A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET =
        0x050, // g_profile_A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET__31dANpcVisitPlayerHouseFleamarket
    A_NPC_TOWN_OUT = 0x051,   // g_profile_A_NPC_TOWN_OUT__12dANpcTownOut
    A_NPC_TOWN_GRACE = 0x052, // g_profile_A_NPC_TOWN_GRACE__14dANpcTownGrace
    A_NPC_TOWN_THEATER =
        0x053,               // g_profile_A_NPC_TOWN_THEATER__16dANpcTownTheater
    A_NPC_CAFE = 0x054,      // g_profile_A_NPC_CAFE__9dANpcCafe
    A_NPC_SP_SAMPLE = 0x055, // g_profile_A_NPC_SP_SAMPLE__13dANpcSpSample
    A_NPC_SP_PLAYER_CREATE =
        0x056, // g_profile_A_NPC_SP_PLAYER_CREATE__19dANpcSpPlayerCreate
    A_NPC_SP_PLAYER_SELECT =
        0x057, // g_profile_A_NPC_SP_PLAYER_SELECT__19dANpcSpPlayerSelect
    A_NPC_SP_RESETSAN = 0x058, // g_profile_A_NPC_SP_RESETSAN__15dANpcSpResetsan
    A_NPC_SP_CAFE_GUEST =
        0x059, // g_profile_A_NPC_SP_CAFE_GUEST__16dANpcSpCafeGuest
    A_NPC_SP_PERIKO_05A = 0x05A, // g_profile_NPC_SP_PERIKO__13dANpcSpPeriko
    A_NPC_SP_PERIKO_05B = 0x05B, // g_profile_NPC_SP_PERIMI__13dANpcSpPeriko
    A_NPC_SP_SONCHO = 0x05C,     // g_profile_A_NPC_SP_SONCHO__13dANpcSpSoncho
    A_NPC_SP_OP_TANUKICHI =
        0x05D, // g_profile_A_NPC_SP_OP_TANUKICHI__18dANpcSpOpTanukichi
    A_NPC_SP_TANUKICHI_05E =
        0x05E, // g_profile_NPC_SP_TANUKICHI__16dANpcSpTanukichi
    A_NPC_SP_TANUKICHI_05F =
        0x05F, // g_profile_NPC_SP_MAMEKICHI__16dANpcSpTanukichi
    A_NPC_SP_TANUKICHI_060 =
        0x060,                 // g_profile_NPC_SP_TUBUKICHI__16dANpcSpTanukichi
    A_NPC_SP_KABURIBA = 0x061, // g_profile_A_NPC_SP_KABURIBA__15dANpcSpKaburiba
    A_NPC_SP_TUNEKICHI =
        0x062, // g_profile_A_NPC_SP_TUNEKICHI__16dANpcSpTunekichi
    A_NPC_SP_CATHERINE =
        0x063,               // g_profile_A_NPC_SP_CATHERINE__16dANpcSpCatherine
    A_NPC_SP_ASAMI = 0x064,  // g_profile_A_NPC_SP_ASAMI__12dANpcSpAsami
    A_NPC_SP_KINUYO = 0x065, // g_profile_A_NPC_SP_KINUYO__13dANpcSpKinuyo
    A_NPC_SP_JIJYO = 0x066,  // g_profile_A_NPC_SP_JIJYO__12dANpcSpJijyo
    A_NPC_SP_CAFE = 0x067,   // g_profile_A_NPC_SP_CAFE__11dANpcSpCafe
    A_NPC_SP_FUTA = 0x068,   // g_profile_A_NPC_SP_FUTA__11dANpcSpFuta
    A_NPC_SP_FUKO = 0x069,   // g_profile_A_NPC_SP_FUKO__11dANpcSpFuko
    A_NPC_SP_SEIICHI = 0x06A,  // g_profile_A_NPC_SP_SEIICHI__14dANpcSpSeiichi
    A_NPC_SP_RORAN = 0x06B,    // g_profile_A_NPC_SP_RORAN__12dANpcSpRoran
    A_NPC_SP_KAPPEI = 0x06C,   // g_profile_A_NPC_SP_KAPPEI__13dANpcSpKappei
    A_NPC_SP_TOTAKEKE = 0x06D, // g_profile_A_NPC_SP_TOTAKEKE__15dANpcSpTotakeke
    A_NPC_SP_JONNY = 0x06E,    // g_profile_A_NPC_SP_JONNY__12dANpcSpJonny
    A_NPC_SP_RACCO = 0x06F,    // g_profile_A_NPC_SP_RACCO__12dANpcSpRacco
    A_NPC_SP_PERIO = 0x070,    // g_profile_A_NPC_SP_PERIO__12dANpcSpPerio
    A_NPC_SP_FORTUNE = 0x071,  // g_profile_A_NPC_SP_FORTUNE__14dANpcSpFortune
    A_NPC_SP_SISYO = 0x072,    // g_profile_A_NPC_SP_SISYO__12dANpcSpSisyo
    A_NPC_SP_HONMA = 0x073,    // g_profile_A_NPC_SP_HONMA__12dANpcSpHonma
    A_NPC_SP_GRACE = 0x074,    // g_profile_A_NPC_SP_GRACE__12dANpcSpGrace
    A_NPC_SP_GATEKEEPER_A =
        0x075, // g_profile_A_NPC_SP_GATEKEEPER_A__18dANpcSpGatekeeperA
    A_NPC_SP_GATEKEEPER_B =
        0x076, // g_profile_A_NPC_SP_GATEKEEPER_B__18dANpcSpGatekeeperB
    A_NPC_SP_JINGLE = 0x077,   // g_profile_A_NPC_SP_JINGLE__13dANpcSpJingle
    A_NPC_SP_FRANKLIN = 0x078, // g_profile_A_NPC_SP_FRANKLIN__15dANpcSpFranklin
    A_NPC_SP_UOMASA = 0x079,   // g_profile_A_NPC_SP_UOMASA__13dANpcSpUomasa
    A_NPC_SP_PUMPKING = 0x07A, // g_profile_A_NPC_SP_PUMPKING__15dANpcSpPumpking
    A_NPC_SP_KUTSUMIGAKI =
        0x07B, // g_profile_A_NPC_SP_KUTSUMIGAKI__18dANpcSpKutsumigaki
    A_NPC_SP_EVENT_SONCHO_07C =
        0x07C, // g_profile_NPC_SP_EV_SONCHO_CD__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_07D =
        0x07D, // g_profile_NPC_SP_EV_SONCHO_NY__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_07E =
        0x07E, // g_profile_NPC_SP_EV_SONCHO_HF__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_07F =
        0x07F, // g_profile_NPC_SP_EV_SONCHO_HT__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_080 =
        0x080, // g_profile_NPC_SP_EV_SONCHO01__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_081 =
        0x081, // g_profile_NPC_SP_EV_SONCHO02__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_082 =
        0x082, // g_profile_NPC_SP_EV_SONCHO03__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_083 =
        0x083, // g_profile_NPC_SP_EV_SONCHO04__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_084 =
        0x084, // g_profile_NPC_SP_EV_SONCHO05__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_085 =
        0x085, // g_profile_NPC_SP_EV_SONCHO06__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_086 =
        0x086, // g_profile_NPC_SP_EV_SONCHO07__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_087 =
        0x087, // g_profile_NPC_SP_EV_SONCHO08__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_088 =
        0x088, // g_profile_NPC_SP_EV_SONCHO09__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_089 =
        0x089, // g_profile_NPC_SP_EV_SONCHO10__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_08A =
        0x08A, // g_profile_NPC_SP_EV_SONCHO11__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_08B =
        0x08B, // g_profile_NPC_SP_EV_SONCHO12__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_08C =
        0x08C, // g_profile_NPC_SP_EV_SONCHO13__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_08D =
        0x08D, // g_profile_NPC_SP_EV_SONCHO14__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_08E =
        0x08E, // g_profile_NPC_SP_EV_SONCHO15__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_08F =
        0x08F, // g_profile_NPC_SP_EV_SONCHO16__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_090 =
        0x090, // g_profile_NPC_SP_EV_SONCHO17__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_091 =
        0x091, // g_profile_NPC_SP_EV_SONCHO18__18dANpcSpEventSoncho
    A_NPC_SP_EVENT_SONCHO_092 =
        0x092, // g_profile_NPC_SP_EV_SONCHO19__18dANpcSpEventSoncho
    A_NPC_SP_CEREMONY_SONCHO =
        0x093, // g_profile_A_NPC_SP_CEREMONY_SONCHO__21dANpcSpCeremonySoncho
    A_NPC_SP_GOLDITEM_SONCHO =
        0x094, // g_profile_A_NPC_SP_GOLDITEM_SONCHO__21dANpcSpGolditemSoncho
    A_NPC_SP_PYONTARO = 0x095, // g_profile_A_NPC_SP_PYONTARO__15dANpcSpPyontaro
    A_NPC_SP_GODDESS = 0x096,  // g_profile_A_NPC_SP_GODDESS__14dANpcSpGoddess
    A_NPC_SP_BUG = 0x097,      // g_profile_A_NPC_SP_BUG__10dANpcSpBug
    A_NPC_SP_HANIWA = 0x098,   // g_profile_A_NPC_SP_HANIWA__13dANpcSpHaniwa
    A_NPC_SP_DEMO_RESETSAN_099 =
        0x099, // g_profile_NPC_SP_DEMO_RESETSAN__19dANpcSpDemoResetsan
    A_NPC_SP_DEMO_RESETSAN_09A =
        0x09A, // g_profile_NPC_SP_DEMO_RACKETSAN__19dANpcSpDemoResetsan
    A_NPC_SP_DANCER = 0x09B, // g_profile_A_NPC_SP_DANCER__13dANpcSpDancer
    A_NPC_SP_MISSING_CHILD =
        0x09C, // g_profile_A_NPC_SP_MISSING_CHILD__19dANpcSpMissingChild
    A_NPC_SP_MISSING_MOTHER =
        0x09D, // g_profile_A_NPC_SP_MISSING_MOTHER__20dANpcSpMissingMother
    A_NPC_SP_MYSTERYCAT =
        0x09E, // g_profile_A_NPC_SP_MYSTERYCAT__17dANpcSpMysterycat
    A_NPC_SP_YUUTARO = 0x09F, // g_profile_A_NPC_SP_YUUTARO__14dANpcSpYuutaro
    A_NPC_SP_YUUTARO_LOFT =
        0x0A0, // g_profile_A_NPC_SP_YUUTARO_LOFT__18dANpcSpYuutaroLoft
    A_NPC_SP_FUUSEN = 0x0A1, // g_profile_A_NPC_SP_FUUSEN__13dANpcSpFuusen
    S_SELECT = 0x0A2,        // g_profile_S_SELECT__8dSSelect
    SV_DEMO = 0x0A3,         // g_profile_SV_DEMO
    SV_MGR = 0x0A4,          // g_profile_SV_MGR
    TITLE_DEMO = 0x0A5,      // g_profile_TITLE_DEMO__10dTitleDemo
    BOOT = 0x0A6,            // g_profile_BOOT
    CAMERA = 0x0A7,          // g_profile_CAMERA
    FIELD = 0x0A8,           // g_profile_FIELD__6dField
    WEATHER = 0x0A9,         // g_profile_WEATHER
    SKY = 0x0AA,             // g_profile_SKY
    SKY_AURORA = 0x0AB,      // g_profile_SKY_AURORA
    SKY_RAINBOW = 0x0AC,     // g_profile_SKY_RAINBOW
    NPC_MNG = 0x0AD,         // g_profile_NPC_MNG
    HAND_ITEM = 0x0AE,       // g_profile_HAND_ITEM
    FI_MDL_MGR = 0x0AF,      // g_profile_FI_MDL_MGR
    FISH_MUSEUM = 0x0B0,     // g_profile_FISH_MUSEUM__11dFishMuseum
    INSECT_MUSEUM = 0x0B1,   // g_profile_INSECT_MUSEUM__13dInsectMuseum
    FISH_FIELD = 0x0B2,      // g_profile_FISH_FIELD
    INSECT_FIELD = 0x0B3,    // g_profile_INSECT_FIELD__12dInsectField
    A_LAYOUT = 0x0B4,        // g_profile_A_LAYOUT__8dALayout
    MENU_MNG = 0x0B5,        // g_profile_MENU_MNG
    MENU_ITEM00 = 0x0B6,     // g_profile_MENU_ITEM00__11dMenuItem00
    MENU_ITEM01 = 0x0B7,     // g_profile_MENU_ITEM01__11dMenuItem01
    MENU_ITEM02 = 0x0B8,     // g_profile_MENU_ITEM02__11dMenuItem02
    MENU_ITEM03 = 0x0B9,     // g_profile_MENU_ITEM03__11dMenuItem03
    MENU_ITEM04 = 0x0BA,     // g_profile_MENU_ITEM04__11dMenuItem04
    MENU_ITEM05 = 0x0BB,     // g_profile_MENU_ITEM05__11dMenuItem05
    MENU_ITEM06 = 0x0BC,     // g_profile_MENU_ITEM06__11dMenuItem06
    MENU_FRIEND = 0x0BD,     // g_profile_MENU_FRIEND__11dMenuFriend
    MENU_FRIEND01 = 0x0BE,   // g_profile_MENU_FRIEND01__13dMenuFriend01
    MENU_FRIEND02 = 0x0BF,   // g_profile_MENU_FRIEND02__13dMenuFriend02
    MENU_FRIEND03 = 0x0C0,   // g_profile_MENU_FRIEND03__13dMenuFriend03
    MENU_FRIEND04 = 0x0C1,   // g_profile_MENU_FRIEND04__13dMenuFriend04
    MENU_FRIEND05 = 0x0C2,   // g_profile_MENU_FRIEND05__13dMenuFriend05
    MENU_FRIEND06 = 0x0C3,   // g_profile_MENU_FRIEND06__13dMenuFriend06
    MENU_FRIEND07 = 0x0C4,   // g_profile_MENU_FRIEND07__13dMenuFriend07
    MENU_FRIEND08 = 0x0C5,   // g_profile_MENU_FRIEND08__13dMenuFriend08
    MENU_INSC = 0x0C6,       // g_profile_MENU_INSC__9dMenuInsc
    MENU_CHAT = 0x0C7,       // g_profile_MENU_CHAT__9dMenuChat
    MENU_DESI00 = 0x0C8,     // g_profile_MENU_DESI00__11dMenuDesi00
    MENU_EDIT = 0x0C9,       // g_profile_MENU_EDIT__9dMenuEdit
    MENU_PAPER = 0x0CA,      // g_profile_MENU_PAPER__10dMenuPaper
    MENU_KEYTEST = 0x0CB,    // g_profile_MENU_KEYTEST__12dMenuKeytest
    MENU_HAN = 0x0CC,        // g_profile_MENU_HAN__8dMenuHan
    MENU_MAP00 = 0x0CD,      // g_profile_MENU_MAP00__10dMenuMap00
    MENU_MAP01 = 0x0CE,      // g_profile_MENU_MAP01__10dMenuMap01
    MENU_MAP02 = 0x0CF,      // g_profile_MENU_MAP02__10dMenuMap02
    MENU_MAP03 = 0x0D0,      // g_profile_MENU_MAP03__10dMenuMap03
    MENU_PHOTO = 0x0D1,      // g_profile_MENU_PHOTO__10dMenuPhoto
    MENU_PHOTO01 = 0x0D2,    // g_profile_MENU_PHOTO01__12dMenuPhoto01
    MENU_CATALOG = 0x0D3,    // g_profile_MENU_CATALOG__12dMenuCatalog
    MENU_NUMBER00 = 0x0D4,   // g_profile_MENU_NUMBER00__13dMenuNumber00
    MENU_MUSEUM = 0x0D5,     // g_profile_MENU_MUSEUM__11dMenuMuseum
    MENU_TIME00 = 0x0D6,     // g_profile_MENU_TIME00__11dMenuTime00
    MENU_WRITE = 0x0D7,      // g_profile_MENU_WRITE__10dMenuWrite
    MENU_BANK = 0x0D8,       // g_profile_MENU_BANK__9dMenuBank
    MENU_KABU = 0x0D9,       // g_profile_MENU_KABU__9dMenuKabu
    MENU_MSLCT = 0x0DA,      // g_profile_MENU_MSLCT__10dMenuMslct
    MENU_MII = 0x0DB,        // g_profile_MENU_MII__8dMenuMii
    MENU_DESI01 = 0x0DC,     // g_profile_MENU_DESI01__11dMenuDesi01
    MENU_DESI02 = 0x0DD,     // g_profile_MENU_DESI02__11dMenuDesi02
    MENU_DESI03 = 0x0DE,     // g_profile_MENU_DESI03__11dMenuDesi03
    MENU_MUSIC = 0x0DF,      // g_profile_MENU_MUSIC__10dMenuMusic
    MENU_POST00 = 0x0E0,     // g_profile_MENU_POST00__11dMenuPost00
    MENU_POST01 = 0x0E1,     // g_profile_MENU_POST01__11dMenuPost01
    MENU_NOTICE = 0x0E2,     // g_profile_MENU_NOTICE__11dMenuNotice
    MENU_NOTICE00 = 0x0E3,   // g_profile_MENU_NOTICE00__13dMenuNotice00
    MENU_NOTICE01 = 0x0E4,   // g_profile_MENU_NOTICE01__13dMenuNotice01
    MENU_MELODY = 0x0E5,     // g_profile_MENU_MELODY__11dMenuMelody
    MENU_DEBUG00 = 0x0E6,    // g_profile__12dMenuDebug00
    MENU_STAR00 = 0x0E7,     // g_profile_MENU_STAR00__11dMenuStar00
    MENU_STAR01 = 0x0E8,     // g_profile_MENU_STAR01__11dMenuStar01
    MENU_POINT = 0x0E9,      // g_profile_MENU_POINT__10dMenuPoint
    MENU_HANIWA = 0x0EA,     // g_profile_MENU_HANIWA__11dMenuHaniwa
    A_MDL = 0x0EB,           // g_profile_A_MDL__5dAMdl
    BGT_DRAW = 0x0EC,        // g_profile_BGT_DRAW__8dBgtDraw
    BGO_DRAW = 0x0ED,        // g_profile_BGO_DRAW__8dBgoDraw
    BGI_DRAW = 0x0EE,        // g_profile_BGI_DRAW__8dBgiDraw
    FG_DRAW = 0x0EF,         // g_profile_FG_DRAW__7dFgDraw
    FGOBJ_MANAGER = 0x0F0,   // g_profile_FGOBJ_MANAGER__13dFgobjManager
    SHOP_FG = 0x0F1,         // g_profile_SHOP_FG__7dShopFg
    BG_DEBUG = 0x0F2,        // g_profile_BG_DEBUG
    SHADOW = 0x0F3,          // g_profile_SHADOW__7dShadow
    A_NAMEPLATE = 0x0F4,     // g_profile_A_NAMEPLATE__11dANameplate
    A_NAMEPLATE_MUSEUM =
        0x0F5,             // g_profile_A_NAMEPLATE_MUSEUM__17dANameplateMuseum
    A_SIGN_TAG = 0x0F6,    // g_profile_A_SIGN_TAG__9dASignTag
    OBJ_BAL = 0x0F7,       // g_profile_OBJ_BAL
    OBJ_VOICE = 0x0F8,     // g_profile_OBJ_VOICE
    OBJ_STAFFROLL = 0x0F9, // g_profile_OBJ_STAFFROLL__13dObjStaffroll
    A_STR = 0x0FA,         // g_profile_A_STR__5dAStr
    A_STR_FIELD_0FB = 0x0FB,  // g_profile_STR_PL__10dAStrField
    A_STR_FIELD_0FC = 0x0FC,  // g_profile_STR_POST__10dAStrField
    A_STR_FIELD_0FD = 0x0FD,  // g_profile_STR_NPC__10dAStrField
    A_STR_FIELD_0FE = 0x0FE,  // g_profile_STR_OFFICE__10dAStrField
    A_STR_FIELD_0FF = 0x0FF,  // g_profile_STR_SHOP__10dAStrField
    A_STR_FIELD_100 = 0x100,  // g_profile_STR_MUSEUM__10dAStrField
    A_STR_FIELD_101 = 0x101,  // g_profile_STR_RESERVE__10dAStrField
    A_STR_FIELD_102 = 0x102,  // g_profile_STR_CHKP__10dAStrField
    A_STR_FIELD_103 = 0x103,  // g_profile_STR_TAILOR__10dAStrField
    A_STR_CDBBS = 0x104,      // g_profile_A_STR_CDBBS__10dAStrCdbbs
    A_STR_FIELD_105 = 0x105,  // g_profile_STR_BBS__10dAStrField
    A_STR_FIELD_106 = 0x106,  // g_profile_STR_LIGHT_HS__10dAStrField
    A_STR_FIELD_107 = 0x107,  // g_profile_STR_WINDMILL__10dAStrField
    A_STR_FIELD_108 = 0x108,  // g_profile_STR_TENT__10dAStrField
    A_STR_FIELD_109 = 0x109,  // g_profile_STR_HARVEST__10dAStrField
    A_STR_FIELD_10A = 0x10A,  // g_profile_STR_FOUNTAIN__10dAStrField
    A_STR_FIELD_10B = 0x10B,  // g_profile_STR_UFO__10dAStrField
    A_STR_COMMON_10C = 0x10C, // g_profile_STR_BUS__11dAStrCommon
    A_STR_COMMON_10D = 0x10D, // g_profile_STR_BUS_STOP__11dAStrCommon
    A_STR_TOWN_10E = 0x10E,   // g_profile_STR_TOWN__9dAStrTown
    A_STR_TOWN_10F = 0x10F,   // g_profile_STR_ATM__9dAStrTown
    A_STR_TOWN_110 = 0x110,   // g_profile_STR_BARBER__9dAStrTown
    A_STR_TOWN_111 = 0x111,   // g_profile_STR_HAPPY__9dAStrTown
    A_STR_TOWN_112 = 0x112,   // g_profile_STR_BROKER__9dAStrTown
    A_STR_TOWN_113 = 0x113,   // g_profile_STR_FORTUNE__9dAStrTown
    A_STR_TOWN_114 = 0x114,   // g_profile_STR_GRACE__9dAStrTown
    A_STR_TOWN_115 = 0x115,   // g_profile_STR_RESET__9dAStrTown
    A_STR_TOWN_116 = 0x116,   // g_profile_STR_LAMP__9dAStrTown
    A_STR_TOWN_117 = 0x117,   // g_profile_STR_AUCTION__9dAStrTown
    A_STR_TOWN_118 = 0x118,   // g_profile_STR_THEATER__9dAStrTown
    A_STR_TOWN_119 = 0x119,   // g_profile_STR_TW_FOUNTAIN__9dAStrTown
    A_STR_TOWN_11A = 0x11A,   // g_profile_STR_ORGAN__9dAStrTown
    A_STR_TOWN_11B = 0x11B,   // g_profile_STR_CHEST__9dAStrTown
    A_STR_FIELD_11C = 0x11C,  // g_profile_STR_CURNIVAL__10dAStrField
    FLAG = 0x11D,             // g_profile_FLAG__5dFlag
    COUNTDOWN_FLAG = 0x11E,   // g_profile_COUNTDOWN_FLAG__14dCountdownFlag
    ORGAN_BALLOON = 0x11F,    // g_profile_ORGAN_BALLOON__13dOrganBalloon
    BALLOON_MNG = 0x120,    // g_profile_BALLOON_MNG__9dABalloon
    BALLOON = 0x121,    // g_profile_BALLOON__9dABalloon
    A_UFO = 0x122,            // g_profile_A_UFO__5dAUfo
    STR_MANAGER = 0x123,      // g_profile_STR_MANAGER__11dStrManager
    A_BBS_BIRD = 0x124,       // g_profile_A_BBS_BIRD__9dABbsBird
    OBJ_CHAT = 0x125,         // g_profile_OBJ_CHAT
    OBJ_TUTORIAL = 0x126,     // g_profile_OBJ_TUTORIAL
    OBJ_TELOP = 0x127,        // g_profile_OBJ_TELOP
    EVENT_MNG = 0x128,        // g_profile_EVENT_MNG
    DEMO = 0x129,             // g_profile_DEMO
    OBJC = 0x12A,             // g_profile_OBJC
    ENVIRONMENT = 0x12B,      // g_profile_ENVIRONMENT
    PROFILE_COUNT = 0x12C,
    INVALID = PROFILE_COUNT,
};

// Draw priorities read from the corresponding eight-byte profile records.
namespace DRAW_ORDER {
enum DRAW_ORDER_e {
    ROOT = 0x12B,
    S_STAGE = 0x12A,
    OBJ_CURSOR = 0x126,
    AC_UITAG = 0x123,
    MSG = 0x0E3,
    CLOCK = 0x124,
    MONEY = 0x125,
    OBJ_KEYBOARD = 0x11F,
    A_GROWUP_MSG = 0x00D,
    A_PLAYER = 0x083,
    PLAYER_MNGR = 0x082,
    CD = 0x004,
    ORG_OBJ = 0x045,
    A_RMOBJ_ATM = 0x046,
    A_RMOBJ_BARRICADE = 0x047,
    A_RMOBJ_BUS = 0x048,
    A_RMOBJ_CAFE = 0x049,
    A_RMOBJ_CHECKP = 0x04A,
    A_RMOBJ_FOSSIL = 0x04B,
    A_RMOBJ_GRACE = 0x04C,
    A_RMOBJ_PERM_MACHINE = 0x04D,
    A_RMOBJ_PIC = 0x04E,
    A_RMOBJ_POINT = 0x04F,
    A_RMOBJ_RECYCLEBOX = 0x050,
    A_RMOBJ_RESET = 0x051,
    A_RMOBJ_SAMPLE = 0x052,
    A_RMOBJ_SEWING_MACHINE = 0x053,
    A_RMOBJ_TEL = 0x054,
    A_RMOBJ_THEATER = 0x055,
    A_RMOBJ_MIRRORBALL = 0x056,
    A_TICKET_COUNTER = 0x057,
    COCKROACH_MANAGER = 0x059,
    A_COCKROACH = 0x05A,
    A_FTR = 0x05B,
    FTR_LP = 0x05C,
    FTR_TRG = 0x05D,
    FTR_SW = 0x05E,
    FTR_SW2 = 0x05F,
    FTR_FLP = 0x060,
    FTR2_W = 0x061,
    FTR_FSW = 0x062,
    FTR_CHAIR = 0x063,
    FTR_BED = 0x065,
    FTR_CHEST = 0x064,
    FTR_CLOCK = 0x066,
    FTR_AUDIO = 0x067,
    FTR_TV = 0x068,
    FTR_TV_F = 0x069,
    FTR_PCART = 0x06A,
    FTR_HNW = 0x06B,
    FTR_FAW = 0x06C,
    FTR_KILLER = 0x06D,
    FTR_CLOTH = 0x06E,
    FTR_MYOBJ = 0x06F,
    FTR_TAOBJ = 0x070,
    FTR_CMPS = 0x071,
    FTR_INSTRUMENT = 0x072,
    FTR_BK = 0x073,
    FTR_METRO = 0x074,
    FTR_CATALOG_CW = 0x075,
    FTR_ACC = 0x076,
    FTR_UMB = 0x077,
    FTR_BALLOON = 0x078,
    FTR_WINDMILL = 0x079,
    FTR_MONITOR = 0x07A,
    FTR_CICADA = 0x07B,
    FTR_TEL = 0x07C,
    FTR_FLOWER = 0x07D,
    FTR_ATM = 0x07E,
    FTR_BARLEY = 0x07F,
    FTR_ROLLER = 0x080,
    FTR_MANAGER = 0x058,
    A_SAMPLE = 0x084,
    A_SAMPLE2 = 0x085,
    A_SAMPLE3 = 0x086,
    SNOWMAN = 0x087,
    A_NPC_OUT = 0x088,
    A_NPC_IN = 0x089,
    A_NPC_VISIT_NPC_HOUSE = 0x08A,
    A_NPC_VISIT_PLAYER_HOUSE = 0x08B,
    A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET = 0x08C,
    A_NPC_TOWN_OUT = 0x08D,
    A_NPC_TOWN_GRACE = 0x08E,
    A_NPC_TOWN_THEATER = 0x08F,
    A_NPC_CAFE = 0x090,
    A_NPC_SP_SAMPLE = 0x091,
    A_NPC_SP_PLAYER_CREATE = 0x093,
    A_NPC_SP_PLAYER_SELECT = 0x092,
    A_NPC_SP_RESETSAN = 0x094,
    A_NPC_SP_CAFE_GUEST = 0x095,
    A_NPC_SP_PERIKO_05A = 0x096,
    A_NPC_SP_PERIKO_05B = 0x097,
    A_NPC_SP_SONCHO = 0x098,
    A_NPC_SP_OP_TANUKICHI = 0x099,
    A_NPC_SP_TANUKICHI_05E = 0x09A,
    A_NPC_SP_TANUKICHI_05F = 0x09B,
    A_NPC_SP_TANUKICHI_060 = 0x09C,
    A_NPC_SP_KABURIBA = 0x09D,
    A_NPC_SP_TUNEKICHI = 0x09E,
    A_NPC_SP_CATHERINE = 0x09F,
    A_NPC_SP_ASAMI = 0x0A0,
    A_NPC_SP_KINUYO = 0x0A1,
    A_NPC_SP_JIJYO = 0x0A2,
    A_NPC_SP_CAFE = 0x0A3,
    A_NPC_SP_FUTA = 0x0A5,
    A_NPC_SP_FUKO = 0x0A6,
    A_NPC_SP_SEIICHI = 0x0A9,
    A_NPC_SP_RORAN = 0x0AA,
    A_NPC_SP_KAPPEI = 0x0A8,
    A_NPC_SP_TOTAKEKE = 0x0A7,
    A_NPC_SP_JONNY = 0x0AB,
    A_NPC_SP_RACCO = 0x0AC,
    A_NPC_SP_PERIO = 0x0AD,
    A_NPC_SP_FORTUNE = 0x0B4,
    A_NPC_SP_SISYO = 0x0B5,
    A_NPC_SP_HONMA = 0x0B6,
    A_NPC_SP_GRACE = 0x0B1,
    A_NPC_SP_GATEKEEPER_A = 0x0B2,
    A_NPC_SP_GATEKEEPER_B = 0x0B3,
    A_NPC_SP_JINGLE = 0x0AE,
    A_NPC_SP_FRANKLIN = 0x0AF,
    A_NPC_SP_UOMASA = 0x0B0,
    A_NPC_SP_PUMPKING = 0x0A4,
    A_NPC_SP_KUTSUMIGAKI = 0x0B7,
    A_NPC_SP_EVENT_SONCHO_07C = 0x0B8,
    A_NPC_SP_EVENT_SONCHO_07D = 0x0B9,
    A_NPC_SP_EVENT_SONCHO_07E = 0x0BA,
    A_NPC_SP_EVENT_SONCHO_07F = 0x0BB,
    A_NPC_SP_EVENT_SONCHO_080 = 0x0BC,
    A_NPC_SP_EVENT_SONCHO_081 = 0x0BD,
    A_NPC_SP_EVENT_SONCHO_082 = 0x0BE,
    A_NPC_SP_EVENT_SONCHO_083 = 0x0BF,
    A_NPC_SP_EVENT_SONCHO_084 = 0x0C0,
    A_NPC_SP_EVENT_SONCHO_085 = 0x0C1,
    A_NPC_SP_EVENT_SONCHO_086 = 0x0C2,
    A_NPC_SP_EVENT_SONCHO_087 = 0x0C3,
    A_NPC_SP_EVENT_SONCHO_088 = 0x0C4,
    A_NPC_SP_EVENT_SONCHO_089 = 0x0C5,
    A_NPC_SP_EVENT_SONCHO_08A = 0x0C6,
    A_NPC_SP_EVENT_SONCHO_08B = 0x0C7,
    A_NPC_SP_EVENT_SONCHO_08C = 0x0C8,
    A_NPC_SP_EVENT_SONCHO_08D = 0x0C9,
    A_NPC_SP_EVENT_SONCHO_08E = 0x0CA,
    A_NPC_SP_EVENT_SONCHO_08F = 0x0CB,
    A_NPC_SP_EVENT_SONCHO_090 = 0x0CC,
    A_NPC_SP_EVENT_SONCHO_091 = 0x0CD,
    A_NPC_SP_EVENT_SONCHO_092 = 0x0CE,
    A_NPC_SP_CEREMONY_SONCHO = 0x0CF,
    A_NPC_SP_GOLDITEM_SONCHO = 0x0D0,
    A_NPC_SP_PYONTARO = 0x0D1,
    A_NPC_SP_GODDESS = 0x0D2,
    A_NPC_SP_BUG = 0x0D3,
    A_NPC_SP_HANIWA = 0x0D4,
    A_NPC_SP_DEMO_RESETSAN_099 = 0x0D5,
    A_NPC_SP_DEMO_RESETSAN_09A = 0x0D6,
    A_NPC_SP_DANCER = 0x0D7,
    A_NPC_SP_MISSING_CHILD = 0x0D8,
    A_NPC_SP_MISSING_MOTHER = 0x0D9,
    A_NPC_SP_MYSTERYCAT = 0x0DA,
    A_NPC_SP_YUUTARO = 0x0DB,
    A_NPC_SP_YUUTARO_LOFT = 0x0DC,
    A_NPC_SP_FUUSEN = 0x0DD,
    S_SELECT = 0x0DF,
    SV_DEMO = 0x0E0,
    SV_MGR = 0x0E1,
    TITLE_DEMO = 0x0E2,
    BOOT = 0x0DE,
    CAMERA = 0x0E4,
    FIELD = 0x00C,
    WEATHER = 0x081,
    SKY = 0x005,
    SKY_AURORA = 0x006,
    SKY_RAINBOW = 0x007,
    NPC_MNG = 0x00E,
    HAND_ITEM = 0x00F,
    FI_MDL_MGR = 0x010,
    FISH_MUSEUM = 0x011,
    INSECT_MUSEUM = 0x012,
    FISH_FIELD = 0x013,
    INSECT_FIELD = 0x014,
    A_LAYOUT = 0x0E5,
    MENU_MNG = 0x0E6,
    MENU_ITEM00 = 0x0E9,
    MENU_ITEM01 = 0x0EA,
    MENU_ITEM02 = 0x0EB,
    MENU_ITEM03 = 0x0EC,
    MENU_ITEM04 = 0x0ED,
    MENU_ITEM05 = 0x0EE,
    MENU_ITEM06 = 0x0EF,
    MENU_FRIEND = 0x0F0,
    MENU_FRIEND01 = 0x0F1,
    MENU_FRIEND02 = 0x0F2,
    MENU_FRIEND03 = 0x0F3,
    MENU_FRIEND04 = 0x0F4,
    MENU_FRIEND05 = 0x0F5,
    MENU_FRIEND06 = 0x0F6,
    MENU_FRIEND07 = 0x0F7,
    MENU_FRIEND08 = 0x0F8,
    MENU_INSC = 0x0F9,
    MENU_CHAT = 0x0FA,
    MENU_DESI00 = 0x0FB,
    MENU_EDIT = 0x0FC,
    MENU_PAPER = 0x0FD,
    MENU_KEYTEST = 0x0FE,
    MENU_HAN = 0x0FF,
    MENU_MAP00 = 0x100,
    MENU_MAP01 = 0x101,
    MENU_MAP02 = 0x102,
    MENU_MAP03 = 0x103,
    MENU_PHOTO = 0x104,
    MENU_PHOTO01 = 0x105,
    MENU_CATALOG = 0x106,
    MENU_NUMBER00 = 0x108,
    MENU_MUSEUM = 0x107,
    MENU_TIME00 = 0x109,
    MENU_WRITE = 0x10A,
    MENU_BANK = 0x10B,
    MENU_KABU = 0x10C,
    MENU_MSLCT = 0x10D,
    MENU_MII = 0x10E,
    MENU_DESI01 = 0x10F,
    MENU_DESI02 = 0x110,
    MENU_DESI03 = 0x111,
    MENU_MUSIC = 0x112,
    MENU_POST00 = 0x113,
    MENU_POST01 = 0x114,
    MENU_NOTICE = 0x115,
    MENU_NOTICE00 = 0x116,
    MENU_NOTICE01 = 0x117,
    MENU_MELODY = 0x118,
    MENU_DEBUG00 = 0x119,
    MENU_STAR00 = 0x11A,
    MENU_STAR01 = 0x11B,
    MENU_POINT = 0x11C,
    MENU_HANIWA = 0x11D,
    A_MDL = 0x11E,
    BGT_DRAW = 0x003,
    BGO_DRAW = 0x001,
    BGI_DRAW = 0x002,
    FG_DRAW = 0x009,
    FGOBJ_MANAGER = 0x00A,
    SHOP_FG = 0x008,
    BG_DEBUG = 0x00B,
    SHADOW = 0x043,
    A_NAMEPLATE = 0x015,
    A_NAMEPLATE_MUSEUM = 0x016,
    A_SIGN_TAG = 0x017,
    OBJ_BAL = 0x122,
    OBJ_VOICE = 0x127,
    OBJ_STAFFROLL = 0x128,
    A_STR = 0x019,
    A_STR_FIELD_0FB = 0x01A,
    A_STR_FIELD_0FC = 0x01B,
    A_STR_FIELD_0FD = 0x01C,
    A_STR_FIELD_0FE = 0x01D,
    A_STR_FIELD_0FF = 0x01F,
    A_STR_FIELD_100 = 0x020,
    A_STR_FIELD_101 = 0x01E,
    A_STR_FIELD_102 = 0x021,
    A_STR_FIELD_103 = 0x022,
    A_STR_CDBBS = 0x023,
    A_STR_FIELD_105 = 0x024,
    A_STR_FIELD_106 = 0x025,
    A_STR_FIELD_107 = 0x026,
    A_STR_FIELD_108 = 0x027,
    A_STR_FIELD_109 = 0x028,
    A_STR_FIELD_10A = 0x029,
    A_STR_FIELD_10B = 0x037,
    A_STR_COMMON_10C = 0x02A,
    A_STR_COMMON_10D = 0x02B,
    A_STR_TOWN_10E = 0x02C,
    A_STR_TOWN_10F = 0x02D,
    A_STR_TOWN_110 = 0x02E,
    A_STR_TOWN_111 = 0x02F,
    A_STR_TOWN_112 = 0x030,
    A_STR_TOWN_113 = 0x031,
    A_STR_TOWN_114 = 0x032,
    A_STR_TOWN_115 = 0x033,
    A_STR_TOWN_116 = 0x034,
    A_STR_TOWN_117 = 0x035,
    A_STR_TOWN_118 = 0x036,
    A_STR_TOWN_119 = 0x038,
    A_STR_TOWN_11A = 0x039,
    A_STR_TOWN_11B = 0x03A,
    A_STR_FIELD_11C = 0x03B,
    FLAG = 0x03C,
    COUNTDOWN_FLAG = 0x03D,
    ORGAN_BALLOON = 0x03E,
    BALLOON_MNG = 0x03F,
    BALLOON = 0x040,
    A_UFO = 0x041,
    STR_MANAGER = 0x018,
    A_BBS_BIRD = 0x042,
    OBJ_CHAT = 0x120,
    OBJ_TUTORIAL = 0x121,
    OBJ_TELOP = 0x129,
    EVENT_MNG = 0x0E7,
    DEMO = 0x000,
    OBJC = 0x044,
    ENVIRONMENT = 0x0E8,
};
} // namespace DRAW_ORDER

} // namespace fProf

#endif
