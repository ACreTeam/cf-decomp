#include <lib/textinput/tiCellPhone.h>
#include <lib/textinput/tiPcKeyboard.h>
#include <lib/textinput/tiManager.h>
#include <lib/textinput/tiHKBManager.h>
#include <lib/textinput/tiLanguageIndependentData.h>
#include <lib/textinput/tiLayoutGather.h>
#include <lib/textinput/tiUtil.h>
#include <new>
#include <string.h>

namespace textinput {
namespace keyboard {
namespace pctype {

struct AnimationFile {
    /* 0x00 */ u32 id;
    /* 0x04 */ char fileName[0x40];
}; // size 0x44

struct PaneToAnimation {
    /* 0x00 */ u32 type;
    /* 0x04 */ const char *paneName;
    /* 0x08 */ u32 count;
    /* 0x0C */ const char *forceAddName;
    /* 0x10 */ const AnimationFile *pAnims[12];
}; // size 0x40

struct TranslateModeArg {
    /* 0x0 */ u32 mode;
}; // size 0x4

struct BoolArg {
    /* 0x0 */ bool flag;
}; // size 0x1

const wchar_t *csszPredictLanguage[] = {
    L"", L"", L"Eng", L"Fra", L"Esp", L"EN", L"DE", L"FR", L"ES", L"IT", L"NL",
};

static const PaneNameToCharCodeWithMode csGridKeyboard[GRID_KEY_COUNT] = {
    {"P_Gkey_00", {0x00E0, L'!', 0, 0}},
    {"P_Gkey_01", {0x00E1, L'?', 0x3042, 0x30A2}},
    {"P_Gkey_02", {0x00E2, L'&', 0x304B, 0x30AB}},
    {"P_Gkey_03", {0x00E4, 0x2033, 0x3055, 0x30B5}},
    {"P_Gkey_04", {0x00E8, 0x0027, 0x305F, 0x30BF}},
    {"P_Gkey_05", {0x00E9, 0xFF5E, 0x306A, 0x30CA}},
    {"P_Gkey_06", {0x00EA, L':', 0x306F, 0x30CF}},
    {"P_Gkey_07", {0x00EB, L';', 0x307E, 0x30DE}},
    {"P_Gkey_08", {0x00EC, L'@', 0x3084, 0x30E4}},
    {"P_Gkey_09", {0x00ED, L'~', 0x3089, 0x30E9}},
    {"P_Gkey_10", {0x00EE, L'_', 0x308F, 0x30EF}},
    {"P_Gkey_11", {0x00E0, L'+', 0, 0}},
    {"P_Gkey_12", {0x00E1, L'-', 0x3044, 0x30A4}},
    {"P_Gkey_13", {0x00E2, L'*', 0x304D, 0x30AD}},
    {"P_Gkey_14", {0x00E4, L'/', 0x3057, 0x30B7}},
    {"P_Gkey_15", {0x00E8, 0x00D7, 0x3061, 0x30C1}},
    {"P_Gkey_16", {0x00E9, 0x00F7, 0x306B, 0x30CB}},
    {"P_Gkey_17", {0x00EA, L'=', 0x3072, 0x30D2}},
    {"P_Gkey_18", {0x00EB, 0x2192, 0x307F, 0x30DF}},
    {"P_Gkey_19", {0x00EC, 0x2190, 0x3086, 0x30E6}},
    {"P_Gkey_20", {0x00ED, 0x2191, 0x308A, 0x30EA}},
    {"P_Gkey_21", {0x00EE, 0x2193, 0x3092, 0x30F2}},
    {"P_Gkey_22", {0x00F1, 0x300C, 0, 0}},
    {"P_Gkey_23", {0x00DF, 0x300D, 0x3046, 0x30A6}},
    {"P_Gkey_24", {0x00C0, 0x201C, 0x304F, 0x30AF}},
    {"P_Gkey_25", {0x00C1, 0x201D, 0x3059, 0x30B9}},
    {"P_Gkey_26", {0x00C2, L'(', 0x3064, 0x30C4}},
    {"P_Gkey_27", {0x00C4, L')', 0x306C, 0x30CC}},
    {"P_Gkey_28", {0x00C8, L'<', 0x3075, 0x30D5}},
    {"P_Gkey_29", {0x00C9, L'>', 0x3080, 0x30E0}},
    {"P_Gkey_30", {0x00CA, L'{', 0x3088, 0x30E8}},
    {"P_Gkey_31", {0x00CB, L'}', 0x308B, 0x30EB}},
    {"P_Gkey_32", {0x00CC, 0x2022, 0x3093, 0x30F3}},
    {"P_Gkey_33", {0x00CD, L'%', 0, 0}},
    {"P_Gkey_34", {0x00CE, 0x203B, 0x3048, 0x30A8}},
    {"P_Gkey_35", {0x00CF, 0x3012, 0x3051, 0x30B1}},
    {"P_Gkey_36", {0x00D2, L'#', 0x305B, 0x30BB}},
    {"P_Gkey_37", {0x00D3, 0x266D, 0x3066, 0x30C6}},
    {"P_Gkey_38", {0x00D4, 0x266A, 0x306D, 0x30CD}},
    {"P_Gkey_39", {0x00D6, 0x00B1, 0x3078, 0x30D8}},
    {"P_Gkey_40", {0x0152, L'$', 0x3081, 0x30E1}},
    {"P_Gkey_41", {0x00D9, 0x00A2, 0xFF01, 0xFF01}},
    {"P_Gkey_42", {0x00DA, 0x00A3, 0x308C, 0x30EC}},
    {"P_Gkey_43", {0x00DB, 0x205C, 0x3001, 0x3001}},
    {"P_Gkey_44", {0x00DC, L'^', 0, 0}},
    {"P_Gkey_45", {0x00C7, 0x00B0, 0x304A, 0x30AA}},
    {"P_Gkey_46", {0x00D1, 0xFF5C, 0x3053, 0x30B3}},
    {"P_Gkey_47", {0x00A1, 0xFF0F, 0x305D, 0x30BD}},
    {"P_Gkey_48", {0x00BF, 0xFF3C, 0x3068, 0x30C8}},
    {"P_Gkey_49", {0x00AC, 0x221E, 0x306E, 0x30CE}},
    {"P_Gkey_50", {0x00A2, 0x2234, 0x307B, 0x30DB}},
    {"P_Gkey_51", {0x00A3, 0x2026, 0x3082, 0x30E2}},
    {"P_Gkey_52", {0x00A7, 0x2122, 0xFF1F, 0xFF1F}},
    {"P_Gkey_53", {0, 0x00A9, 0x308D, 0x30ED}},
    {"P_Gkey_54", {0, 0x00AE, 0x3002, 0x3002}},
    {"P_Gkey_55", {0x00A2, 0x2234, 0x30FC, 0x30FC}},
};

static const PaneNameToControlKey csPaneNameToControlKey[CONTROL_KEY_COUNT] = {
    {"P_key_LF", CK_LineFeed},
    {"P_key_DELETE", CK_BackSpace},
    {"P_key_CAPS", CK_Caps},
    {"P_key_SHIFT", CK_Shift},
    {"P_key_SPACE", CK_Space},
    {"P_Gkey_LF", CK_LineFeed},
    {"P_Gkey_DELETE", CK_BackSpace},
    {"P_Gkey_SPACE", CK_Space},
    {"W_USEU_prdc_lang", CK_PredictLanguage},
    {"W_USEU_Chng_sign", CK_SignWindow},
    {"W_JP_Chng_ABC", CK_ABC},
    {"W_JP_Chng_KANA", CK_Kana},
    {"W_JP_Chng_sign", CK_SignWindow},
    {"P_Mode_roma_hira", CK_RomaHiragana},
    {"P_Mode_roma_kata", CK_RomaKatakana},
    {"P_Mode_direct", CK_Direct},
    {"P_hiragana", CK_Hiragana},
    {"P_katakana", CK_Katakana},
    {"P_Gkey_komoji", CK_Small},
    {"P_Gkey_handaku", CK_Handaku},
    {"P_Gkey_dakuten", CK_Dakuten},
};

static const VisiblePanes csJPNAsciiVisiblePanes = {
    13,
    23,
    {
        "N_modeSelect_all",
        "P_Mode_direct",
        "P_Mode_roma_hira",
        "P_Mode_roma_kata",
        "N_KeyChange_JP",
        "W_JP_Chng_ABC",
        "W_JP_Chng_KANA",
        "W_JP_Chng_sign",
        "T_JP_Chng_ABC",
        "T_JP_Chng_KANA",
        "T_JP_Chng_sign",
        "T_key_CAPS",
        "T_key_SHIFT",
    },
    {
        "P_key_41",         "T_key_41",         "P_key_42",
        "T_key_42",         "P_key_43",         "T_key_43",
        "N_KeyChange_USEU", "P_Gkey_00",        "P_Gkey_11",
        "P_Gkey_22",        "P_Gkey_33",        "P_Gkey_44",
        "T_Gkey_00",        "T_Gkey_11",        "T_Gkey_22",
        "T_Gkey_33",        "T_Gkey_44",        "W_USEU_prdc_lang",
        "W_USEU_Chng_sign", "T_USEU_prdc_lang", "T_USEU_Chng_sign",
        "P_SHIFTMark",      "P_CAPSMark",
    },
};

static const VisiblePanes csUSAsciiVisiblePanes = {
    17,
    19,
    {
        "N_KeyChange_USEU",
        "P_Gkey_00",
        "P_Gkey_11",
        "P_Gkey_22",
        "P_Gkey_33",
        "P_Gkey_44",
        "T_Gkey_00",
        "T_Gkey_11",
        "T_Gkey_22",
        "T_Gkey_33",
        "T_Gkey_44",
        "W_USEU_prdc_lang",
        "W_USEU_Chng_sign",
        "T_USEU_prdc_lang",
        "T_USEU_Chng_sign",
        "T_key_CAPS",
        "T_key_SHIFT",
    },
    {
        "P_key_41",         "T_key_41",       "P_key_42",
        "T_key_42",         "P_key_43",       "T_key_43",
        "N_modeSelect_all", "P_Mode_direct",  "P_Mode_roma_hira",
        "P_Mode_roma_kata", "N_KeyChange_JP", "W_JP_Chng_ABC",
        "W_JP_Chng_KANA",   "W_JP_Chng_sign", "T_JP_Chng_ABC",
        "T_JP_Chng_KANA",   "T_JP_Chng_sign", "P_SHIFTMark",
        "P_CAPSMark",
    },
};

static const VisiblePanes csEngAsciiVisiblePanesUK = {
    21,
    15,
    {
        "P_key_42",         "T_key_42",         "P_key_43",
        "T_key_43",         "N_KeyChange_USEU", "P_Gkey_00",
        "P_Gkey_11",        "P_Gkey_22",        "P_Gkey_33",
        "P_Gkey_44",        "T_Gkey_00",        "T_Gkey_11",
        "T_Gkey_22",        "T_Gkey_33",        "T_Gkey_44",
        "W_USEU_prdc_lang", "W_USEU_Chng_sign", "T_USEU_prdc_lang",
        "T_USEU_Chng_sign", "T_key_CAPS",       "T_key_SHIFT",
    },
    {
        "P_key_41",
        "T_key_41",
        "N_modeSelect_all",
        "P_Mode_direct",
        "P_Mode_roma_hira",
        "P_Mode_roma_kata",
        "N_KeyChange_JP",
        "W_JP_Chng_ABC",
        "W_JP_Chng_KANA",
        "W_JP_Chng_sign",
        "T_JP_Chng_ABC",
        "T_JP_Chng_KANA",
        "T_JP_Chng_sign",
        "P_SHIFTMark",
        "P_CAPSMark",
    },
};

static const VisiblePanes csEngAsciiVisiblePanesSP = {
    21,
    15,
    {
        "P_key_42",         "T_key_42",         "P_key_43",
        "T_key_43",         "N_KeyChange_USEU", "P_Gkey_00",
        "P_Gkey_11",        "P_Gkey_22",        "P_Gkey_33",
        "P_Gkey_44",        "T_Gkey_00",        "T_Gkey_11",
        "T_Gkey_22",        "T_Gkey_33",        "T_Gkey_44",
        "W_USEU_prdc_lang", "W_USEU_Chng_sign", "T_USEU_prdc_lang",
        "T_USEU_Chng_sign", "P_SHIFTMark",      "P_CAPSMark",
    },
    {
        "P_key_41",
        "T_key_41",
        "N_modeSelect_all",
        "P_Mode_direct",
        "P_Mode_roma_hira",
        "P_Mode_roma_kata",
        "N_KeyChange_JP",
        "W_JP_Chng_ABC",
        "W_JP_Chng_KANA",
        "W_JP_Chng_sign",
        "T_JP_Chng_ABC",
        "T_JP_Chng_KANA",
        "T_JP_Chng_sign",
        "T_key_CAPS",
        "T_key_SHIFT",
    },
};

static const VisiblePanes csAsciiVisiblePanes = {
    23,
    13,
    {
        "P_key_41",         "T_key_41",         "P_key_42",
        "T_key_42",         "P_key_43",         "T_key_43",
        "N_KeyChange_USEU", "P_Gkey_00",        "P_Gkey_11",
        "P_Gkey_22",        "P_Gkey_33",        "P_Gkey_44",
        "T_Gkey_00",        "T_Gkey_11",        "T_Gkey_22",
        "T_Gkey_33",        "T_Gkey_44",        "W_USEU_prdc_lang",
        "W_USEU_Chng_sign", "T_USEU_prdc_lang", "T_USEU_Chng_sign",
        "P_SHIFTMark",      "P_CAPSMark",
    },
    {
        "N_modeSelect_all",
        "P_Mode_direct",
        "P_Mode_roma_hira",
        "P_Mode_roma_kata",
        "N_KeyChange_JP",
        "W_JP_Chng_ABC",
        "W_JP_Chng_KANA",
        "W_JP_Chng_sign",
        "T_JP_Chng_ABC",
        "T_JP_Chng_KANA",
        "T_JP_Chng_sign",
        "T_key_CAPS",
        "T_key_SHIFT",
    },
};

static const VisiblePanes csAsciiVisiblePanesNL = {
    19,
    17,
    {
        "P_key_43",         "T_key_43",         "N_KeyChange_USEU",
        "P_Gkey_00",        "P_Gkey_11",        "P_Gkey_22",
        "P_Gkey_33",        "P_Gkey_44",        "T_Gkey_00",
        "T_Gkey_11",        "T_Gkey_22",        "T_Gkey_33",
        "T_Gkey_44",        "W_USEU_prdc_lang", "W_USEU_Chng_sign",
        "T_USEU_prdc_lang", "T_USEU_Chng_sign", "T_key_CAPS",
        "T_key_SHIFT",
    },
    {
        "P_key_41",
        "T_key_41",
        "P_key_42",
        "T_key_42",
        "N_modeSelect_all",
        "P_Mode_direct",
        "P_Mode_roma_hira",
        "P_Mode_roma_kata",
        "N_KeyChange_JP",
        "W_JP_Chng_ABC",
        "W_JP_Chng_KANA",
        "W_JP_Chng_sign",
        "T_JP_Chng_ABC",
        "T_JP_Chng_KANA",
        "T_JP_Chng_sign",
        "P_SHIFTMark",
        "P_CAPSMark",
    },
};

static const CharCodeToAblaut csCharCodeToAblautSP[ABLAUT_COUNT] = {
    {L'a', {0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4}},
    {0, {0, L'i', 0x00EC, 0x00ED, 0x00EE}},
    {0, {0x00EF, 0, 0, L'u', 0x00F9}},
    {0x00FA, {0x00FB, 0, 0x00FC, 0, 0}},
    {L'e', {0x00E8, 0x00E9, 0x00EA, 0, 0x00EB}},
    {0, {0, L'o', 0x00F2, 0x00F3, 0x00F4}},
    {0x00F5, {0x00F6, 0, 0, L'A', 0x00C0}},
    {0x00C1, {0x00C2, 0x00C3, 0x00C4, 0, 0}},
    {L'I', {0x00CC, 0x00CD, 0x00CE, 0, 0x00CF}},
    {0, {0, L'U', 0x00D9, 0x00DA, 0x00DB}},
    {0, {0x00DC, 0, 0, L'E', 0x00C8}},
    {0x00C9, {0x00CA, 0, 0x00CB, 0, 0}},
    {L'O', {0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6}},
    {0, {0, L'y', 0, 0x00FD, 0}},
    {0, {0x00FF, 0, 0, L'Y', 0}},
    {0x00DD, {0, 0, 0x0178, 0, 0}},
};

static const CharCodeToAblaut csCharCodeToAblautNL[ABLAUT_COUNT] = {
    {L'a', {0x00E0, 0, 0x00E2, 0x00E3, 0}},
    {0x00E1, {0x00E4, L'i', 0x00EC, 0, 0x00EE}},
    {0, {0, 0x00ED, 0x00EF, L'u', 0x00F9}},
    {0, {0x00FB, 0, 0, 0x00FA, 0x00FC}},
    {L'e', {0x00E8, 0, 0x00EA, 0, 0}},
    {0x00E9, {0x00EB, L'o', 0x00F2, 0, 0x00F4}},
    {0x00F5, {0, 0x00F3, 0x00F6, L'A', 0x00C0}},
    {0, {0x00C2, 0x00C3, 0, 0x00C1, 0x00C4}},
    {L'I', {0x00CC, 0, 0x00CE, 0, 0}},
    {0x00CD, {0x00CF, L'U', 0x00D9, 0, 0x00DB}},
    {0, {0, 0x00DA, 0x00DC, L'E', 0x00C8}},
    {0, {0x00CA, 0, 0, 0x00C9, 0x00CB}},
    {L'O', {0x00D2, 0, 0x00D4, 0x00D5, 0}},
    {0x00D3, {0x00D6, L'y', 0, 0, 0}},
    {0, {0, 0x00FD, 0x00FF, L'Y', 0}},
    {0, {0, 0, 0, 0x00DD, 0x0178}},
};

static const CharCodeToAblaut csCharCodeToAblautDE[ABLAUT_COUNT] = {
    {L'a', {0x00E0, 0x00E1, 0, 0, 0}}, {0, {0, L'i', 0x00EC, 0x00ED, 0}},
    {0, {0, 0, 0, L'u', 0x00F9}},      {0x00FA, {0, 0, 0, 0, 0}},
    {L'e', {0x00E8, 0x00E9, 0, 0, 0}}, {0, {0, L'o', 0x00F2, 0x00F3, 0}},
    {0, {0, 0, 0, L'A', 0x00C0}},      {0x00C1, {0, 0, 0, 0, 0}},
    {L'I', {0x00CC, 0x00CD, 0, 0, 0}}, {0, {0, L'U', 0x00D9, 0x00DA, 0}},
    {0, {0, 0, 0, L'E', 0x00C8}},      {0x00C9, {0, 0, 0, 0, 0}},
    {L'O', {0x00D2, 0x00D3, 0, 0, 0}}, {0, {0, L'y', 0, 0x00FD, 0}},
    {0, {0, 0, 0, L'Y', 0}},           {0x00DD, {0, 0, 0, 0, 0}},
};

static const CharCodeToAblaut csCharCodeToAblautFR[ABLAUT_COUNT] = {
    {L'a', {0, 0, 0x00E2, 0, 0x00E4}}, {0, {0, L'i', 0, 0, 0x00EE}},
    {0, {0x00EF, 0, 0, L'u', 0}},      {0, {0x00FB, 0, 0x00FC, 0, 0}},
    {L'e', {0, 0, 0x00EA, 0, 0x00EB}}, {0, {0, L'o', 0, 0, 0x00F4}},
    {0, {0x00F6, 0, 0, L'A', 0}},      {0, {0x00C2, 0, 0x00C4, 0, 0}},
    {L'I', {0, 0, 0x00CE, 0, 0x00CF}}, {0, {0, L'U', 0, 0, 0x00DB}},
    {0, {0x00DC, 0, 0, L'E', 0}},      {0, {0x00CA, 0, 0x00CB, 0, 0}},
    {L'O', {0, 0, 0x00D4, 0, 0x00D6}}, {0, {0, L'y', 0, 0, 0}},
    {0, {0x00FF, 0, 0, L'Y', 0}},      {0, {0, 0, 0x0178, 0, 0}},
};

static const LanguageDependency csLanguageDependencyData[LanguageEnd] = {
    {csUSKeyboard, csGridKeyboard, csPaneNameToControlKey,
     &csJPNAsciiVisiblePanes, NULL},
    {csUSKeyboard, csGridKeyboard, csPaneNameToControlKey,
     &csUSAsciiVisiblePanes, NULL},
    {csUKKeyboard, csGridKeyboard, csPaneNameToControlKey,
     &csEngAsciiVisiblePanesUK, NULL},
    {csFRKeyboard, csGridKeyboard, csPaneNameToControlKey, &csAsciiVisiblePanes,
     csCharCodeToAblautFR},
    {csDEKeyboard, csGridKeyboard, csPaneNameToControlKey, &csAsciiVisiblePanes,
     csCharCodeToAblautDE},
    {csITKeyboard, csGridKeyboard, csPaneNameToControlKey, &csAsciiVisiblePanes,
     NULL},
    {csESKeyboard, csGridKeyboard, csPaneNameToControlKey,
     &csEngAsciiVisiblePanesSP, csCharCodeToAblautSP},
    {csNLKeyboard, csGridKeyboard, csPaneNameToControlKey,
     &csAsciiVisiblePanesNL, csCharCodeToAblautNL},
};

static const LanguageDependency csJapanKanaInput = {
    csJPKeyboard, csGridKeyboard, csPaneNameToControlKey,
    &csJPNAsciiVisiblePanes, NULL};

static const AnimationFile csAninationFile[ANIMATION_FILE_COUNT] = {
    {0, "fs_VK_ascii_keytop_a_normal.brlan"},
    {1, "fs_VK_ascii_keytop_a_Focus-IN.brlan"},
    {2, "fs_VK_ascii_keytop_a_Focus-OUT.brlan"},
    {3, "fs_VK_ascii_keytop_a_Roll_over.brlan"},
    {4, "fs_VK_ascii_keytop_a_Pushed.brlan"},
    {5, "fs_VK_ascii_keytop_a_toggle-ON.brlan"},
    {6, "fs_VK_ascii_keytop_a_toggle-OFF.brlan"},
    {7, "fs_VK_ascii_keytop_a_Target_ON.brlan"},
    {8, "fs_VK_ascii_keytop_a_toggleON_Focus-IN.brlan"},
    {9, "fs_VK_ascii_keytop_a_toggleON_Focus-OUT.brlan"},
    {10, "fs_VK_ascii_keytop_a_normal_toggle-ON.brlan"},
    {11, "fs_VK_ascii_keytop_a_toggleON_Pushed.brlan"},
    {12, "fs_VK_ascii_keytop_a_active-ON.brlan"},
    {13, "fs_VK_ascii_keytop_a_active-OFF.brlan"},
    {14, "fs_VK_ascii_keytop_a_active_normal.brlan"},
    {15, "fs_VK_ascii_keytop_a_not_active_normal.brlan"},
};

static const char *COMMON_KEY_ANIM = "P_key_00";

static const PaneToAnimation csPaneToAnimation[PANE_ANIMATION_COUNT] = {
    {0,
     "P_key_00",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_01",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_02",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_03",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_04",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_05",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_06",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_07",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_08",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_09",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_10",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_11",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_12",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_13",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_15",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_16",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_17",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_18",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_14",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_19",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_20",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_21",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_22",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_23",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_24",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_25",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_26",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_27",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_28",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_29",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_30",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_31",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_32",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_33",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_34",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_35",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_36",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_37",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_38",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_39",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_40",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_41",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_42",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_43",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_44",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_45",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_46",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_47",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_48",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_49",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_DELETE",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_LF",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_key_SPACE",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {1,
     "P_key_SHIFT",
     12,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[7], &csAninationFile[8],
      &csAninationFile[9], &csAninationFile[10], &csAninationFile[11]}},
    {1,
     "P_key_CAPS",
     12,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[7], &csAninationFile[8],
      &csAninationFile[9], &csAninationFile[10], &csAninationFile[11]}},
    {0,
     "P_Gkey_00",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_01",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_02",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_03",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_04",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_05",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_06",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_07",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_08",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_09",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_10",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_11",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_12",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_13",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_14",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_15",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_16",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_17",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_18",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_19",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_20",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_21",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_22",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_23",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_24",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_25",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_26",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_27",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_28",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_29",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_30",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_31",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_32",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_33",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_34",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_35",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_36",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_37",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_38",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_39",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_40",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_41",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_42",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_43",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_44",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_45",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_46",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_47",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_48",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_49",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_50",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_51",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_52",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_53",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_54",
     5,
     COMMON_KEY_ANIM,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_55",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_DELETE",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_LF",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "P_Gkey_SPACE",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "W_USEU_prdc_lang",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {0,
     "W_USEU_Chng_sign",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {2,
     "W_JP_Chng_ABC",
     7,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[7]}},
    {2,
     "W_JP_Chng_KANA",
     7,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[7]}},
    {0,
     "W_JP_Chng_sign",
     5,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5]}},
    {2,
     "P_hiragana",
     7,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[7]}},
    {2,
     "P_katakana",
     7,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[7]}},
    {3,
     "P_Gkey_dakuten",
     11,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[12], &csAninationFile[13],
      &csAninationFile[14], &csAninationFile[15]}},
    {3,
     "P_Gkey_handaku",
     11,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[12], &csAninationFile[13],
      &csAninationFile[14], &csAninationFile[15]}},
    {3,
     "P_Gkey_komoji",
     11,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[12], &csAninationFile[13],
      &csAninationFile[14], &csAninationFile[15]}},
    {2,
     "P_Mode_roma_hira",
     7,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[7]}},
    {2,
     "P_Mode_roma_kata",
     7,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[7]}},
    {2,
     "P_Mode_direct",
     7,
     NULL,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3], &csAninationFile[4], &csAninationFile[5],
      &csAninationFile[6], &csAninationFile[7]}},
};

void Base::create(MEMAllocator *allocator) {
    mpAllocator = allocator;
}

void Base::init() {
    mState.bInputReceived = false;
    mState.pLanguageData = &csLanguageDependencyData[getLanguage()];
    mKeyState.init();
    TranslateModeArg arg;
    arg.mode = mKeyState.getTranslateMode();
    sendCommand(0x12, &arg);
}

void Base::inputCharCode(wchar_t ch) {
    sendInputWChar(ch, false);
}

void Base::onKey(u32 key, void *data) {
    wchar_t ch = getWCCode(static_cast<char *>(data));
    if (key == 4) {
        if (ch == 0) {
            switch (getControlKey(static_cast<char *>(data))) {
            case CK_BackSpace:
                sendCommand(1, NULL);
                break;
            case CK_LineFeed:
                sendCommand(7, NULL);
                sendCommand(0x27, NULL);
                break;
            case CK_Space:
                if (mpManager->getInputForm()->canConvert()) {
                    sendCommand(0x28, NULL);
                } else {
                    sendInputWChar(L' ', false);
                }
                break;
            case CK_ABC:
                setABC(true);
                break;
            case CK_Kana:
                setABC(false);
                break;
            case CK_SignWindow:
                goSignInputMode();
                break;
            case CK_PredictLanguage:
                changePredictLanguage();
                break;
            case CK_0E:
                changeAIUInputMode(IM_04);
                break;
            case CK_Direct: {
                TranslateModeArg arg = {TM_Direct};
                sendCommand(0x12, &arg);
                setTranslateMode(TM_Direct);
                break;
            }
            case CK_RomaHiragana: {
                TranslateModeArg arg = {TM_Hiragana};
                sendCommand(0x12, &arg);
                setTranslateMode(TM_Hiragana);
                break;
            }
            case CK_RomaKatakana: {
                TranslateModeArg arg = {TM_Katakana};
                sendCommand(0x12, &arg);
                setTranslateMode(TM_Katakana);
                break;
            }
            case CK_Hiragana:
                mKeyState.setKanaMode(0);
                break;
            case CK_Katakana:
                mKeyState.setKanaMode(1);
                break;
            case CK_Dakuten:
                if (mKeyState.mInputMode & 0x20) {
                    sendCommand(0x19, NULL);
                }
                break;
            case CK_Handaku:
                if (mKeyState.mInputMode & 0x40) {
                    sendCommand(0x1A, NULL);
                }
                break;
            case CK_Small:
                if (mKeyState.mInputMode & 0x80) {
                    sendCommand(0x1C, NULL);
                }
                break;
            }
        } else {
            inputCharCode(ch);
        }
    }
}

void Base::goSignInputMode() {
}

void Base::changePredictLanguage() {
}

void Base::updateFromReceiver(u32 command, void *data) {
    if (command == 0x1F) {
        return;
    }
    CommandReceiver::ChangePredictMode predictMode = {0, 0};
    sendCommand(0x1F, &predictMode);
    if ((command != 0x19 && command != 0x1A && command != 0x1C &&
         command != 0x18 && command != 0x14 && command != 0x13 &&
         command != 0x23) ||
        command == 0x24) {
        mKeyState.setInputMode(0);
    }
    if (command == 0x24) {
        mState.bInputReceived = true;
    }
}

void Base::onActive() {
    TranslateModeArg arg;
    arg.mode = mKeyState.getTranslateMode();
    sendCommand(0x12, &arg);
    sendCommand(0x27, NULL);
    if (mKeyState.mInputType == 0) {
        switch (mKeyState.getTranslateMode()) {
            {
                BoolArg kana;
            case TM_Hiragana:
                kana.flag = true;
                goto sendABC;
            case TM_Katakana:
                kana.flag = false;
            sendABC:
                sendCommand(0x13, &kana);
                break;
            }
        }
    } else {
        switch (mKeyState.getAIUInputMode()) {
            {
                BoolArg kana;
            case IM_06:
                kana.flag = true;
                goto sendAIU;
            case IM_07:
                kana.flag = false;
            sendAIU:
                sendCommand(0x13, &kana);
                break;
            }
        }
    }
    updateFixMode();
}

void Base::onClose() {
}

wchar_t Base::getWCCode(char *paneName) {
    return mKeyState.getWCCode(paneName);
}

inline ControlKey Base::KeyState::getControlKey(char *paneName) {
    for (u16 i = 0; i < CONTROL_KEY_COUNT; i++) {
        if (util::strcmp(mpLanguageData->pControlKeys[i].szPaneName,
                         paneName)) {
            return mpLanguageData->pControlKeys[i].code;
        }
    }
    return CK_None;
}

ControlKey Base::getControlKey(char *paneName) {
    return mKeyState.getControlKey(paneName);
}

void Base::changeABCInputMode(InputMode mode) {
    switch (mode) {
    case IM_00:
        mKeyState.setABCFlag(0);
        break;
    case IM_01:
        mKeyState.setABCFlag(0x40);
        break;
    case IM_02:
        mKeyState.setABCFlag(0x80);
        break;
    case IM_03:
        mKeyState.setABCFlag(0xC0);
        break;
    }
}

void Base::changeAIUInputMode(InputMode mode) {
    switch (mode) {
    case IM_06:
        mKeyState.setKanaMode(0);
        break;
    case IM_07:
        mKeyState.setKanaMode(1);
        break;
    }
}

void Base::sendInputWChar(wchar_t ch, bool direct) {
    mState.bInputReceived = false;
    u32 flags = 0;
    if (mKeyState.muFlag & 0x40) {
        flags |= 2;
    }
    if (mKeyState.muFlag & 0x80) {
        flags |= 1;
    }
    inputform::Base::CharInput input = {L'\0', 0, false, true};
    input.ch = ch;
    input.flags = flags;
    sendCommand(0, &input);
    if (mKeyState.muFlag & 0x80) {
        LayoutGather &gather = LayoutGather::Singleton::getInstance();
        if (!gather.isHoldingShift()) {
            mKeyState.clearFlag(0x80);
        }
    }
    if (!mState.bInputReceived) {
        u32 mode = 0;
        if (ch != util::KBD_ConvertDakuten(ch)) {
            mode |= 0x20;
        }
        if (ch != util::KBD_ConvertHandaku(ch)) {
            mode |= 0x40;
        }
        if (ch != util::KBD_ConvertSmall(ch)) {
            mode |= 0x80;
        }
        mKeyState.setInputMode(mode);
    }
}

void Base::setLanguage(Language language) {
    meLanguage = language;
    mKeyState.meLanguage = language;
    mKeyState.refresh_();
}

void Base::updateFixMode() {
    if ((mpManager->getToolBar() != NULL &&
         !mpManager->getToolBar()->isQwerty()) ||
        getLanguage() != JP) {
        return;
    }
    BoolArg fix;
    fix.flag = false;
    if (isABC() && mKeyState.muTranslateMode == 0) {
        fix.flag = true;
    } else {
        fix.flag = false;
    }
    sendCommand(0x14, &fix);
}

void Base::setABC(bool abc) {
    mKeyState.setKana(!abc);
    sendCommand(0x29, NULL);
}

void Base::setTranslateMode(TranslateMode mode) {
    u32 translateMode;
    switch (mode) {
    case TM_Direct:
        translateMode = 0;
        break;
    case TM_Hiragana:
        translateMode = 1;
        break;
    case TM_Katakana:
        translateMode = 2;
        break;
    }
    if (mKeyState.getTranslateBits() == translateMode) {
        return;
    }
    u32 prevMode = mKeyState.getTranslateBits();
    mKeyState.setTranslateMode(translateMode);
    TranslateModeArg arg;
    arg.mode = mode;
    sendCommand(0x12, &arg);
    mState.pLanguageData = &csJapanKanaInput;
    sendCommand(0x29, NULL);
}

void LayoutByNW4R::setABC(bool abc) {
    changeAnimationAllToNormal();
    if (abc != isABC()) {
        if (abc) {
            searchAnmPane("W_JP_Chng_ABC")->changeAnimation(4);
            searchAnmPane("W_JP_Chng_KANA")->changeAnimation(6);
        } else {
            searchAnmPane("W_JP_Chng_ABC")->changeAnimation(6);
            searchAnmPane("W_JP_Chng_KANA")->changeAnimation(4);
        }
    }
    mKeyState.setKana(!abc);
    sendCommand(0x29, NULL);
}

void Base::KeyState::refresh_() {
    BoolArg fixMode;
    BoolArg kanaMode;
    if ((meLanguage != JP || mpBase->mbKanaLock) && muTranslateMode != 0) {
        muTranslateMode = 0;
        refresh_();
    }
    if (muTranslateMode == 0) {
        mpLanguageData = &csLanguageDependencyData[meLanguage];
    } else {
        mpLanguageData = &csJapanKanaInput;
    }
    if (mInputType == 0) {
        switch (muTranslateMode) {
        case 0:
            fixMode.flag = true;
            mpBase->sendCommand(0x14, &fixMode);
            break;
        case 1:
            fixMode.flag = false;
            kanaMode.flag = true;
            mpBase->sendCommand(0x14, &fixMode);
            mpBase->sendCommand(0x13, &kanaMode);
            break;
        case 2:
            fixMode.flag = false;
            kanaMode.flag = false;
            mpBase->sendCommand(0x14, &fixMode);
            mpBase->sendCommand(0x13, &kanaMode);
            break;
        }
    } else {
        switch (getKanaMode()) {
        case 0:
            fixMode.flag = false;
            kanaMode.flag = true;
            mpBase->sendCommand(0x14, &fixMode);
            mpBase->sendCommand(0x13, &kanaMode);
            break;
        case 1:
            fixMode.flag = false;
            kanaMode.flag = false;
            mpBase->sendCommand(0x14, &fixMode);
            mpBase->sendCommand(0x13, &kanaMode);
            break;
        }
    }
    if (mpBase != NULL) {
        mpBase->refreshState();
    }
}

void Base::refreshState() {
}

void Base::KeyState::refreshText(nw4r::lyt::Pane *pane) {
    for (u16 i = 0; i < KEY_COUNT; i++) {
        char textName[0x14];
        util::replaceChar(textName, 0x11,
                          mpLanguageData->pKeyboard[i].szPaneName, 0, 'T');
        nw4r::lyt::TextBox *textBox =
            nw4r::ut::DynamicCast<nw4r::lyt::TextBox *>(
                pane->FindPaneByName(textName, true));
        if (textBox != NULL) {
            wchar_t str[2];
            str[0] = getWCCode(i);
            str[1] = 0;
            textBox->SetString(str, 0);
        }
    }
    for (u16 i = 0; i < GRID_KEY_COUNT; i++) {
        char textName[0x14];
        util::replaceChar(textName, 0x11,
                          mpLanguageData->pGridKeyboard[i].szPaneName, 0, 'T');
        nw4r::lyt::TextBox *textBox =
            nw4r::ut::DynamicCast<nw4r::lyt::TextBox *>(
                pane->FindPaneByName(textName, true));
        if (textBox != NULL) {
            wchar_t str[2];
            str[0] =
                mpLanguageData->pGridKeyboard[i].wc[getKanaMode() == 0 ? 2 : 3];
            str[1] = 0;
            textBox->SetString(str, 0);
        }
    }
}

void Base::KeyState::setABCFlag(u32 flag) {
    if ((muFlag & ~0xF) == flag) {
        return;
    }
    muFlag = (muFlag & 0xF) | (flag & ~0xF);
    if (mpBase != NULL) {
        mpBase->refreshState();
    }
}

void Base::KeyState::setInputMode(u32 mode) {
    if ((mInputMode & ~0xF) == mode) {
        return;
    }
    mInputMode = (mInputMode & 0xF) | (mode & ~0xF);
    refresh_();
}

wchar_t Base::KeyState::getWCCode(u32 key) {
    u32 mode = 0;
    if (muFlag & 0x40) {
        mode |= 1;
    }
    if (muFlag & 0x80) {
        mode |= 2;
    }
    wchar_t wc = mpLanguageData->pKeyboard[key].wc[mode];
    if (mInputType == 0 && muTranslateMode == 0) {
        if (wc == 0x3001) {
            wc = L',';
        } else if (wc == 0x3002) {
            wc = L'.';
        } else if (wc == 0x30FC) {
            wc = L'-';
        } else if (wc == 0x300C) {
            wc = L'[';
        } else if (wc == 0x300D) {
            wc = L']';
        }
    }
    return wc;
}

wchar_t Base::KeyState::getWCCode(char *paneName) {
    wchar_t wc = 0;
    for (u16 i = 0; i < KEY_COUNT; i++) {
        if (util::strcmp(mpLanguageData->pKeyboard[i].szPaneName, paneName)) {
            wc = getWCCode(i);
        }
    }
    for (u16 i = 0; i < GRID_KEY_COUNT; i++) {
        if (util::strcmp(mpLanguageData->pGridKeyboard[i].szPaneName,
                         paneName)) {
            u32 idx = 0;
            if (getKanaMode() == 0) {
                idx = 2;
            } else {
                idx = 3;
            }
            wc = mpLanguageData->pGridKeyboard[i].wc[idx];
        }
    }
    return wc;
}

void Base::setLangKeyActive(bool active) {
    mbKanaLock = active;
    if (getLanguage() == JP) {
        if (active) {
            setABC(true);
            changeABCInputMode(IM_00);
        } else {
            setABC(false);
            changeAIUInputMode(IM_06);
        }
    }
}

LayoutByNW4R::~LayoutByNW4R() {
    mpEventHandler->~EventHandler();
    MEMFreeToAllocator(mpAllocator, mpEventHandler);
    for (nw4rmanager::AnmPane *pane = static_cast<nw4rmanager::AnmPane *>(
             nw4r::ut::List_GetNext(&mAnmPanes, NULL));
         pane != NULL; pane = static_cast<nw4rmanager::AnmPane *>(
                           nw4r::ut::List_GetNext(&mAnmPanes, NULL))) {
        nw4r::ut::List_Remove(&mAnmPanes, pane);
        pane->destroy(mpAllocator);
    }
}

inline EventHandler::~EventHandler() {
}

void LayoutByNW4R::create(MEMAllocator *allocator) {
    mpAllocator = allocator;
    mpEventHandler =
        new (MEMAllocFromAllocator(allocator, sizeof(EventHandler)))
            EventHandler(this);
    nw4rmanager::Layout::createWithEventHandler(allocator, mpEventHandler);
    mpPaneManager->setAllComponentTriggerTarget(false);
    mpPaneManager->setAllBoundingBoxComponentTriggerTarget(true);
    createAnmPane_(allocator);
    mShiftButton.Create(this, "P_key_SHIFT", "B_key_SHIFT");
    mCapsButton.Create(this, "P_key_CAPS", "B_key_CAPS");
    mShiftButton.setTriggerTarget(true);
    mCapsButton.setTriggerTarget(true);
    init();
    changeABCInputMode(IM_00);
}

void LayoutByNW4R::createAnmPane_(MEMAllocator *allocator) {
    const char *forceAddName;
    for (u16 i = 0; i < PANE_ANIMATION_COUNT; i++) {
        const PaneToAnimation &info = csPaneToAnimation[i];
        AnmPane *anmPane = NULL;
        switch (info.type) {
        case 0:
            anmPane = new (
                MEMAllocFromAllocator(allocator, sizeof(NormalButtonAnmPane)))
                NormalButtonAnmPane(getPane(info.paneName));
            break;
        case 1:
            anmPane =
                new (MEMAllocFromAllocator(allocator, sizeof(ShiftCapsAnmPane)))
                    ShiftCapsAnmPane(getPane(info.paneName));
            break;
        case 2:
            anmPane = new (
                MEMAllocFromAllocator(allocator, sizeof(ToggleButtonAnmPane)))
                ToggleButtonAnmPane(getPane(info.paneName));
            break;
        case 3:
            anmPane = new (
                MEMAllocFromAllocator(allocator, sizeof(OnOffButtonAnmPane)))
                OnOffButtonAnmPane(getPane(info.paneName));
            break;
        }
        nw4r::ut::List_Append(&mAnmPanes, anmPane);
        forceAddName = info.forceAddName;
        for (u16 j = 0; j < info.count; j++) {
            void *res = mpMultiArcResourceAccessor->GetResource(
                0, info.pAnims[j]->fileName, NULL);
            AnimTransformPane *anim = reinterpret_cast<AnimTransformPane *>(
                getLayout()->CreateAnimTransform(res,
                                                 mpMultiArcResourceAccessor));
            if (forceAddName == NULL) {
                anmPane->addAnimation(allocator, info.pAnims[j]->id, anim,
                                      false, true);
            } else {
                anmPane->forceAddAnimation(allocator, info.pAnims[j]->id, anim,
                                           forceAddName, false, true);
            }
        }
    }
}

void LayoutByNW4R::init() {
    Base::init();
    setLineFeedButton(true);
    setPredictLanguageButton(true);
    setSignWindowButton(true);
    searchAnmPane("P_key_SHIFT")->changeAnimation(0);
    searchAnmPane("P_key_CAPS")->changeAnimation(0);
    const VisiblePanes *visiblePanes = mState.pLanguageData->pVisiblePanes;
    for (u16 i = 0; i < visiblePanes->uVisibleNum; i++) {
        setVisible(visiblePanes->szVisiblePanes[i], true);
    }
    for (u16 i = 0; i < visiblePanes->uInvisibleNum; i++) {
        setVisible(visiblePanes->szInvisiblePanes[i], false);
    }
    if (getLanguage() == JP) {
        setString("T_JP_Chng_sign",
                  langindependent::cLanguageIndependentString
                      [langindependent::LANG_STRID_SIGN_WINDOW][getLanguage()]);
    } else {
        setString("T_USEU_Chng_sign",
                  langindependent::cLanguageIndependentString
                      [langindependent::LANG_STRID_SIGN_WINDOW][getLanguage()]);
    }
    setString("T_key_SPACE",
              langindependent::cLanguageIndependentString
                  [langindependent::LANG_STRID_SPACE][getLanguage()]);
    setLangKeyActive(false);
    CommandReceiver::ChangePredictMode predictMode = {0, 0};
    sendCommand(0x1F, &predictMode);
    updatePredictLanguage(&predictMode);
    mpLayout->Animate(0);
    mpLayout->CalculateMtx(mDrawInfo);
    setVisible("T_key_SPACE", true);
    setVisible("T_Gkey_SPACE", true);
    setVisible("P_key_HENKAN", false);
    setVisible("P_Gkey_HENKAN", false);
}

void LayoutByNW4R::initLayout() {
    if (getLanguage() == JP) {
        if (isABC()) {
            searchAnmPane("W_JP_Chng_ABC")->changeAnimation(5);
            searchAnmPane("W_JP_Chng_KANA")->changeAnimation(0);
        } else {
            searchAnmPane("W_JP_Chng_ABC")->changeAnimation(0);
            searchAnmPane("W_JP_Chng_KANA")->changeAnimation(5);
        }
        if (mKeyState.getKanaMode() == 0) {
            searchAnmPane("P_hiragana")->changeAnimation(5);
            searchAnmPane("P_katakana")->changeAnimation(0);
        } else {
            searchAnmPane("P_hiragana")->changeAnimation(0);
            searchAnmPane("P_katakana")->changeAnimation(5);
        }
        if (mKeyState.mInputMode & 0x20) {
            searchAnmPane("P_Gkey_dakuten")->changeAnimation(14);
        } else {
            searchAnmPane("P_Gkey_dakuten")->changeAnimation(15);
        }
        if (mKeyState.mInputMode & 0x40) {
            searchAnmPane("P_Gkey_handaku")->changeAnimation(14);
        } else {
            searchAnmPane("P_Gkey_handaku")->changeAnimation(15);
        }
        if (mKeyState.mInputMode & 0x80) {
            searchAnmPane("P_Gkey_komoji")->changeAnimation(14);
        } else {
            searchAnmPane("P_Gkey_komoji")->changeAnimation(15);
        }
        switch (mKeyState.muTranslateMode) {
        case 0:
            searchAnmPane("P_Mode_roma_hira")->changeAnimation(0);
            searchAnmPane("P_Mode_roma_kata")->changeAnimation(0);
            searchAnmPane("P_Mode_direct")->changeAnimation(5);
            break;
        case 1:
            searchAnmPane("P_Mode_roma_hira")->changeAnimation(5);
            searchAnmPane("P_Mode_roma_kata")->changeAnimation(0);
            searchAnmPane("P_Mode_direct")->changeAnimation(0);
            break;
        case 2:
            searchAnmPane("P_Mode_roma_hira")->changeAnimation(0);
            searchAnmPane("P_Mode_roma_kata")->changeAnimation(5);
            searchAnmPane("P_Mode_direct")->changeAnimation(0);
            break;
        }
    }
    mpLayout->Animate(0);
    mpLayout->CalculateMtx(mDrawInfo);
}

void LayoutByNW4R::calc() {
    nw4rmanager::Layout::calc();
    if (mpManager->getInputForm()->canConvert()) {
        setVisible("T_key_SPACE", false);
        setVisible("T_Gkey_SPACE", false);
        setVisible("P_key_HENKAN", true);
        setVisible("P_Gkey_HENKAN", true);
    } else {
        setVisible("T_key_SPACE", true);
        setVisible("T_Gkey_SPACE", true);
        setVisible("P_key_HENKAN", false);
        setVisible("P_Gkey_HENKAN", false);
    }
}

void LayoutByNW4R::draw() {
    nw4rmanager::Layout::draw();
}

void LayoutByNW4R::inputCharCode(wchar_t ch) {
    sendInputWChar(ch, false);
}

void LayoutByNW4R::onPressedCaps() {
    LayoutGather &gather = LayoutGather::Singleton::getInstance();
    if (!gather.isHoldingShift()) {
        mKeyState.clearFlag(0x80);
        mShiftButton.release();
    }
    gather.changeCapsLock(!gather.isCapsLock());
    mKeyState.flipFlag(0x40);
    mCapsButton.setPressed(mKeyState.mbCaps);
    mpEventObserver->onSE(sound::SE_KETAI_MODE_SWITCHING);
}

void LayoutByNW4R::onPressedShift(bool se) {
    setShiftFlag(true);
    mShiftButton.setPressed(true);
    if (se) {
        mpEventObserver->onSE(sound::SE_KETAI_MODE_SWITCHING);
    }
}

void LayoutByNW4R::onReleasedShift() {
    setShiftFlag(false);
    mShiftButton.release();
}

void LayoutByNW4R::onKey(u32 key, void *data) {
    getWCCode(static_cast<char *>(data));
    if (key == 4 && getWCCode(static_cast<char *>(data)) == 0) {
        switch (getControlKey(static_cast<char *>(data))) {
        case CK_Caps:
            onPressedCaps();
            break;
        case CK_Shift: {
            LayoutGather &gather = LayoutGather::Singleton::getInstance();
            mKeyState.clearFlag(0x40);
            mCapsButton.release();
            if (!gather.isHoldingShift()) {
                mKeyState.flipFlag(0x80);
            }
            mShiftButton.setPressed(mKeyState.mbShift);
            mpEventObserver->onSE(sound::SE_KETAI_MODE_SWITCHING);
            break;
        }
        case CK_LineFeed:
            mpEventObserver->onSE(sound::SE_CHAR_DECIDE);
            break;
        case CK_ABC:
            if (!isABC()) {
                mpEventObserver->onSE(sound::SE_KETAI_MODE_SWITCHING);
                searchAnmPane("W_JP_Chng_KANA")->changeAnimation(6);
            }
            break;
        case CK_Kana:
            if (isABC()) {
                mpEventObserver->onSE(sound::SE_KETAI_MODE_SWITCHING);
                searchAnmPane("W_JP_Chng_ABC")->changeAnimation(6);
            }
            break;
        case CK_Hiragana:
            if (mKeyState.getKanaMode() == 1) {
                mpEventObserver->onSE(sound::SE_KETAI_MODE_SWITCHING);
                searchAnmPane("P_katakana")->changeAnimation(6);
            }
            break;
        case CK_Katakana:
            if (mKeyState.getKanaMode() == 0) {
                mpEventObserver->onSE(sound::SE_KETAI_MODE_SWITCHING);
                searchAnmPane("P_hiragana")->changeAnimation(6);
            }
            break;
        case CK_Direct:
            if (mKeyState.muTranslateMode == 2) {
                searchAnmPane("P_Mode_roma_kata")->changeAnimation(6);
            }
            if (mKeyState.muTranslateMode == 1) {
                searchAnmPane("P_Mode_roma_hira")->changeAnimation(6);
            }
            break;
        case CK_RomaHiragana:
            if (mKeyState.muTranslateMode == 0) {
                searchAnmPane("P_Mode_direct")->changeAnimation(6);
            }
            if (mKeyState.muTranslateMode == 2) {
                searchAnmPane("P_Mode_roma_kata")->changeAnimation(6);
            }
            break;
        case CK_RomaKatakana:
            if (mKeyState.muTranslateMode == 0) {
                searchAnmPane("P_Mode_direct")->changeAnimation(6);
            }
            if (mKeyState.muTranslateMode == 1) {
                searchAnmPane("P_Mode_roma_hira")->changeAnimation(6);
            }
            break;
        }
    }
    Base::onKey(key, data);
}

void LayoutByNW4R::setLanguage(Language language) {
    meLanguage = language;
    mKeyState.meLanguage = language;
    mKeyState.refresh_();
    init();
}

void LayoutByNW4R::updateFromReceiver(u32 command, void *data) {
    Base::updateFromReceiver(command, data);
    updateDakuten();
    switch (command) {
    case 0x1D:
        updatePredictLanguage(
            static_cast<CommandReceiver::ChangePredictMode *>(data));
        break;
    }
}

void LayoutByNW4R::onActive() {
    Base::onActive();
    nw4rmanager::Layout::init();
    if (mKeyState.mInputMode & 0x20) {
        searchAnmPane("P_Gkey_dakuten")->changeAnimation(14);
    } else {
        searchAnmPane("P_Gkey_dakuten")->changeAnimation(15);
    }
    if (mKeyState.mInputMode & 0x40) {
        searchAnmPane("P_Gkey_handaku")->changeAnimation(14);
    } else {
        searchAnmPane("P_Gkey_handaku")->changeAnimation(15);
    }
    if (mKeyState.mInputMode & 0x80) {
        searchAnmPane("P_Gkey_komoji")->changeAnimation(14);
    } else {
        searchAnmPane("P_Gkey_komoji")->changeAnimation(15);
    }
}

void LayoutByNW4R::onClose() {
    initPaneLastDrawReceived();
    cancelStateFocusIn();
}

void LayoutByNW4R::throwReleaseForAll() {
    for (AnmPane *anmPane =
             static_cast<AnmPane *>(nw4r::ut::List_GetNext(&mAnmPanes, NULL));
         anmPane != NULL; anmPane = static_cast<AnmPane *>(
                              nw4r::ut::List_GetNext(&mAnmPanes, anmPane))) {
        if (anmPane->getKeyType() == 0) {
            anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_2);
        }
    }
}

void LayoutByNW4R::cancelStateFocusIn() {
    for (AnmPane *anmPane =
             static_cast<AnmPane *>(nw4r::ut::List_GetNext(&mAnmPanes, NULL));
         anmPane != NULL; anmPane = static_cast<AnmPane *>(
                              nw4r::ut::List_GetNext(&mAnmPanes, anmPane))) {
        switch (anmPane->getKeyType()) {
        case 0:
            switch (anmPane->getState()) {
            case 1:
            case 3:
            case 4:
                anmPane->changeAnimation(2);
                break;
            case 16:
                anmPane->changeAnimation(0);
                break;
            }
            break;
        case 1:
            switch (anmPane->getState()) {
            case 1:
            case 3:
            case 11:
                anmPane->changeAnimation(2);
                break;
            case 4:
            case 5:
            case 8:
                anmPane->changeAnimation(9);
                break;
            }
            break;
        case 2:
            switch (anmPane->getState()) {
            case 1:
            case 3:
                anmPane->changeAnimation(2);
                break;
            case 4:
                anmPane->changeAnimation(5);
                break;
            }
            break;
        case 3:
            switch (anmPane->getState()) {
            case 1:
            case 3:
            case 4:
                anmPane->changeAnimation(2);
                break;
            }
            break;
        }
    }
}

void LayoutByNW4R::goSignInputMode() {
    mpSignWindow->open(this, false);
    throwReleaseForAll();
}

void LayoutByNW4R::changePredictLanguage() {
    throwReleaseForAll();
    CommandReceiver::ChangePredictMode predictMode = {0, 0};
    sendCommand(0x1F, &predictMode);
    mpPredictLanguageDialog->open(
        static_cast<inputform::Base::PredictMode>(predictMode.mode), this);
}

void LayoutByNW4R::updatePredictLanguage(
    CommandReceiver::ChangePredictMode *mode) {
    if (getLanguage() != JP) {
        if (!mode->on) {
            setVisible("P_prdc_ON", false);
            setVisible("P_prdc_OFF", true);
        } else {
            setVisible("P_prdc_ON", true);
            setVisible("P_prdc_OFF", false);
        }
        nw4r::lyt::TextBox *textBox =
            static_cast<nw4r::lyt::TextBox *>(getPane("T_USEU_prdc_lang"));
        textBox->SetString(csszPredictLanguage[mode->mode], 0);
    }
}

void LayoutByNW4R::sendInputWChar(wchar_t ch, bool direct) {
    bool shift = false;
    nw4r::lyt::Pane *pane = NULL;
    if (mKeyState.muFlag & 0x80) {
        pane = mAnmPaneFifo.getLast();
        shift = true;
    }
    Base::sendInputWChar(ch, direct);
    if (shift && pane != NULL) {
        setPaneLastDrawReceived(pane);
    }
    updateDakuten();
}

void LayoutByNW4R::updateDakuten() {
    if (mKeyState.mInputMode & 0x20) {
        searchAnmPane("P_Gkey_dakuten")->onAnmEvent(nw4rmanager::AnmPane::PE_6);
    } else {
        searchAnmPane("P_Gkey_dakuten")->onAnmEvent(nw4rmanager::AnmPane::PE_7);
    }
    if (mKeyState.mInputMode & 0x40) {
        searchAnmPane("P_Gkey_handaku")->onAnmEvent(nw4rmanager::AnmPane::PE_6);
    } else {
        searchAnmPane("P_Gkey_handaku")->onAnmEvent(nw4rmanager::AnmPane::PE_7);
    }
    if (mKeyState.mInputMode & 0x80) {
        searchAnmPane("P_Gkey_komoji")->onAnmEvent(nw4rmanager::AnmPane::PE_6);
    } else {
        searchAnmPane("P_Gkey_komoji")->onAnmEvent(nw4rmanager::AnmPane::PE_7);
    }
}

void LayoutByNW4R::setLineFeedButton(bool enable) {
    mbEnableLineFeed = enable;
    setVisible("P_key_LF", mbEnableLineFeed);
    setVisible("P_Gkey_LF", mbEnableLineFeed);
}

void LayoutByNW4R::setPredictLanguageButton(bool enable) {
    setVisible("W_USEU_prdc_lang", false);
}

void LayoutByNW4R::setSignWindowButton(bool enable) {
    if (!enable) {
        setVisible("W_USEU_Chng_sign", enable);
        setVisible("W_JP_Chng_sign", enable);
    } else {
        setVisible("W_USEU_Chng_sign", false);
        setVisible("W_JP_Chng_sign", false);
        if (getLanguage() == JP) {
            setVisible("W_JP_Chng_sign", true);
        } else {
            setVisible("W_USEU_Chng_sign", true);
        }
    }
}

void LayoutByNW4R::setLangKeyActive(bool active) {
    Base::setLangKeyActive(active);
    if (getLanguage() == JP) {
        initLayout();
        setVisible("W_JP_Chng_ABC", !active);
        setVisible("W_JP_Chng_KANA", !active);
        setVisible("P_Mode_roma_hira", !active);
        setVisible("P_Mode_roma_kata", !active);
        setVisible("P_Mode_direct", !active);
        setVisible("P_romajiBox", !active);
    }
}

void LayoutByNW4R::setInputModeJP(bool abc, u32 abcMode, u32 aiuMode) {
    setABC(abc);
    mKeyState.setTranslateMode(abcMode);
    mKeyState.setKanaMode(aiuMode);
    switch (abcMode) {
    case 0:
        mState.pLanguageData = &csLanguageDependencyData[getLanguage()];
        break;
    case 1:
        mState.pLanguageData = &csJapanKanaInput;
        break;
    case 2:
        mState.pLanguageData = &csJapanKanaInput;
        break;
    }
    initLayout();
    sendCommand(0x29, NULL);
}

void LayoutByNW4R::setTranslateMode(TranslateMode mode) {
    u32 prevMode = mKeyState.muTranslateMode;
    Base::setTranslateMode(mode);
    if (prevMode != mKeyState.muTranslateMode) {
        mpEventObserver->onSE(sound::SE_KETAI_MODE_SWITCHING);
        switch (prevMode) {
        case 0:
            searchAnmPane("P_Mode_direct")->changeAnimation(6);
            break;
        case 1:
            searchAnmPane("P_Mode_roma_hira")->changeAnimation(6);
            break;
        case 2:
            searchAnmPane("P_Mode_roma_kata")->changeAnimation(6);
            break;
        }
        switch (mode) {
        case TM_Direct:
            searchAnmPane("P_Mode_direct")
                ->onAnmEvent(nw4rmanager::AnmPane::PE_0);
            break;
        case TM_Hiragana:
            searchAnmPane("P_Mode_roma_hira")
                ->onAnmEvent(nw4rmanager::AnmPane::PE_0);
            break;
        case TM_Katakana:
            searchAnmPane("P_Mode_roma_kata")
                ->onAnmEvent(nw4rmanager::AnmPane::PE_0);
            break;
        }
    }
}

bool LayoutByNW4R::updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                               u32 release, void *data) {
    bool result =
        nw4rmanager::Layout::updateInput(chan, x, y, trig, hold, release, data);
    LayoutGather &gather = LayoutGather::Singleton::getInstance();
    bool holding = gather.isHoldingShift();
    if (hold & 0x400) {
        gather.setPressedShiftB(true);
        input::HKBManager::sInstance.SetForceModifierState(2, 2);
        if (!holding) {
            onPressedShift(isABC());
        }
    } else {
        gather.setPressedShiftB(false);
        input::HKBManager::sInstance.SetForceModifierState(0, 0);
        if (holding && !gather.isHoldingShift()) {
            onReleasedShift();
        }
    }
    return result;
}

bool LayoutByNW4R::updateInput(input::HKBManager &hkbManager) {
    input::HKBManager::KeySet keySet = hkbManager.GetTriggeredKeySet();
    for (; keySet.IsValid(); keySet = keySet.GetNext()) {
        nw4rmanager::AnmPane *anmPane = NULL;
        u8 key = keySet.GetKey();
        wchar_t wc = keySet.GetWChar();
        switch (key) {
        case 0x2C:
            break;
        case 0x28:
        case 0x58:
            if (!(hkbManager.GetModifierState() & 4)) {
                anmPane = searchAnmPane("P_key_LF");
            }
            break;
        default:
            anmPane =
                searchAnmPane(inputform::DeadKeyStream::ToIndependentClass(wc));
            break;
        }
        if (anmPane != NULL) {
            anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_0);
        }
    }
    for (keySet = hkbManager.GetRepeatedKeySet(); keySet.IsValid();
         keySet = keySet.GetNext()) {
        nw4rmanager::AnmPane *anmPane = NULL;
        u8 key = keySet.GetKey();
        keySet.GetWChar();
        switch (key) {
        case 0x2A:
            if (hkbManager.GetModifierState() & 4) {
                break;
            }
        case 0x4C:
            anmPane = searchAnmPane("P_key_DELETE");
            break;
        case 0x2C:
            anmPane = searchAnmPane("P_key_SPACE");
            break;
        }
        if (anmPane != NULL) {
            anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_0);
        }
    }
    return false;
}

void LayoutByNW4R::changeAnimationAllToNormal() {
    for (AnmPane *anmPane =
             static_cast<AnmPane *>(nw4r::ut::List_GetNext(&mAnmPanes, NULL));
         anmPane != NULL; anmPane = static_cast<AnmPane *>(
                              nw4r::ut::List_GetNext(&mAnmPanes, anmPane))) {
        if (anmPane->getKeyType() == 0) {
            anmPane->changeAnimation(0);
        }
    }
    if (mKeyState.getKanaMode() == 0) {
        searchAnmPane("P_hiragana")->changeAnimation(5);
        searchAnmPane("P_katakana")->changeAnimation(0);
    } else {
        searchAnmPane("P_hiragana")->changeAnimation(0);
        searchAnmPane("P_katakana")->changeAnimation(5);
    }
}

void LayoutByNW4R::refreshState() {
    bool visible;
    if (isABC()) {
        visible = isVisible("N_VK_grid", &visible) ? visible != false : false;
        setVisible("N_VK_grid", false);
        setVisible("N_VK_grd_Bnd_ALL", false);
        setVisible("N_VK_ascii", true);
        setVisible("N_VK_asc_Bnd_ALL", true);
    } else {
        visible = isVisible("N_VK_grid", &visible) ? visible != true : false;
        setVisible("N_VK_grid", true);
        setVisible("N_VK_grd_Bnd_ALL", true);
        setVisible("N_VK_ascii", false);
        setVisible("N_VK_asc_Bnd_ALL", false);
    }
    mKeyState.refreshText(mpLayout->GetRootPane());
    if (getLanguage() == JP) {
        if (mKeyState.getKanaMode() == 0) {
            static_cast<nw4r::lyt::TextBox *>(getPane("T_JP_Chng_KANA"))
                ->SetString(L"\x3042\x3044\x3046", 0);
        } else if (mKeyState.getKanaMode() == 1) {
            static_cast<nw4r::lyt::TextBox *>(getPane("T_JP_Chng_KANA"))
                ->SetString(L"\x30A2\x30A4\x30A6", 0);
        }
    }
    if (!isShiftOn()) {
        mShiftButton.release();
    }
    if (!isCapsOn()) {
        mCapsButton.release();
        LayoutGather::Singleton::getInstance().changeCapsLock(0);
    }
    if (visible) {
        initPaneLastDrawReceived();
    }
}

bool Base::isShiftOn() const {
    return mKeyState.mbShift;
}

void LayoutByNW4R::onEvent(UIObj *object, u32 event, void *data) {
    if (event == 0) {
        mpEventObserver->onSE((sound::SE)(u32)data);
    }
}

void EventHandler::onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                             Input *input) {
    const char *paneName = paneComponent->getPane()->GetName();
    if (util::strcmp("B_key_SHIFT", paneName) ||
        util::strcmp("B_key_CAPS", paneName)) {
        return;
    }
    if (paneName[0] == 'B') {
        char anmName[0x11];
        util::replaceChar(anmName, sizeof(anmName), paneName, 0, 'P');
        if (strncmp(paneName + 5, "Chng", 4) == 0 ||
            strncmp(paneName + 7, "Chng", 4) == 0 ||
            strncmp(paneName + 7, "prdc", 4) == 0) {
            util::replaceChar(anmName, sizeof(anmName), paneName, 0, 'W');
        }
        nw4rmanager::AnmPane *anmPane = mpLayout->searchAnmPane(anmName);
        switch (event) {
        case gui::EventHandler::ON_TRIG:
            if (input->trig & 0x800) {
                if (anmPane != NULL) {
                    anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_0);
                }
                mpLayout->onKey(4, anmName);
            }
            break;
        case gui::EventHandler::ON_LEFT:
            if (anmPane != NULL) {
                anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_2);
            }
            mpLayout->onKey(1, anmName);
            break;
        case gui::EventHandler::ON_POINT:
            if (anmPane != NULL) {
                mpEventObserver->onSE(sound::SE_SELECT);
                mpLayout->setPaneLastDrawReceived(anmPane->getPane());
                anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_1);
            }
            break;
        }
        if (event == gui::EventHandler::ON_MOVE && (input->hold & 0x800) &&
            !(input->trig & 0x800)) {
            if (util::strcmp("P_key_DELETE", anmName) ||
                util::strcmp("P_Gkey_DELETE", anmName) ||
                util::strcmp("P_key_SPACE", anmName) ||
                util::strcmp("P_Gkey_SPACE", anmName)) {
                if (paneComponent->isDragging(input->ctrlNo)) {
                    u32 duration =
                        mpLayout->getFlightDuration(input->ctrlNo, paneName);
                    if (duration >= 30 && duration % 9 == 0) {
                        anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_0);
                        mpLayout->onKey(4, anmName);
                    }
                }
            }
        }
    }
}

void AnmPane::changeAnimation(u32 id) {
    meState = id;
    nw4rmanager::AnmPane::changeAnimation(id == 16 ? 2 : id);
}

void NormalButtonAnmPane::onAnmEvent(AnmPaneEvent paneEvent) {
    switch (meState) {
    case 0:
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        if (paneEvent == PE_0) {
            changeAnimation(16);
        }
        break;
    case 1:
        if (paneEvent == PE_4) {
            changeAnimation(3);
        }
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        if (paneEvent == PE_0) {
            changeAnimation(4);
        }
        break;
    case 3:
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        if (paneEvent == PE_0) {
            changeAnimation(4);
        }
        break;
    case 2:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        if (paneEvent == PE_0) {
            changeAnimation(16);
        }
        break;
    case 4:
        if (paneEvent == PE_4) {
            changeAnimation(3);
        }
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        if (paneEvent == PE_0) {
            changeAnimation(4);
        }
        break;
    case 16:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        if (paneEvent == PE_0) {
            changeAnimation(16);
        }
        break;
    }
}

bool ShiftCapsAnmPane::isFocused() const {
    switch (meState) {
    case 0:
    case 2:
    case 6:
    case 9:
    case 10:
        return false;
    }
    return true;
}

void ShiftCapsAnmPane::onAnmEvent(AnmPaneEvent paneEvent) {
    if (paneEvent == PE_10) {
        mbOn = true;
        if (!isFocused()) {
            changeAnimation(9);
        } else {
            changeAnimation(4);
        }
    } else if (paneEvent == PE_11) {
        mbOn = false;
        if (!isFocused()) {
            changeAnimation(2);
        } else {
            changeAnimation(11);
        }
    } else if (paneEvent == PE_5) {
        mbOn = false;
        if (meState != 9) {
            if (meState == 10) {
                changeAnimation(6);
            } else if (meState == 8 || meState == 4 || meState == 5) {
                changeAnimation(3);
            }
        }
    } else if (!mbOn) {
        switch (meState) {
        case 9:
            if (paneEvent == PE_4) {
                changeAnimation(0);
            }
            break;
        case 6:
            if (paneEvent == PE_4) {
                changeAnimation(0);
            }
        case 0:
            if (paneEvent == PE_1) {
                changeAnimation(1);
            }
            break;
        case 1:
            if (paneEvent == PE_4) {
                changeAnimation(3);
            }
            if (paneEvent == PE_2) {
                changeAnimation(2);
            }
            break;
        case 3:
            if (paneEvent == PE_2) {
                changeAnimation(2);
            }
            break;
        case 2:
            if (paneEvent == PE_4) {
                changeAnimation(0);
            }
            if (paneEvent == PE_1) {
                changeAnimation(1);
            }
            break;
        case 11:
            if (paneEvent == PE_4) {
                changeAnimation(3);
            }
            if (paneEvent == PE_2) {
                changeAnimation(2);
            }
            break;
        }
    } else {
        switch (meState) {
        case 6:
            if (paneEvent == PE_4) {
                changeAnimation(0);
            }
        case 0:
            if (paneEvent == PE_1) {
                changeAnimation(1);
            }
            if (paneEvent == PE_0) {
                changeAnimation(9);
            }
            break;
        case 10:
            if (paneEvent == PE_1) {
                changeAnimation(8);
            }
            break;
        case 8:
            if (paneEvent == PE_4) {
                changeAnimation(5);
            }
            if (paneEvent == PE_2) {
                changeAnimation(9);
            }
            break;
        case 5:
            if (paneEvent == PE_2) {
                changeAnimation(9);
            }
            break;
        case 9:
            if (paneEvent == PE_4) {
                changeAnimation(10);
            }
            if (paneEvent == PE_1) {
                changeAnimation(8);
            }
            break;
        case 4:
            if (paneEvent == PE_4) {
                changeAnimation(5);
            }
            if (paneEvent == PE_2) {
                changeAnimation(9);
            }
            break;
        case 11:
            break;
        }
    }
}

void ToggleButtonAnmPane::onAnmEvent(AnmPaneEvent paneEvent) {
    if (paneEvent == PE_0 && meState != 5 && meState != 4) {
        changeAnimation(4);
    }
    switch (meState) {
    case 0:
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        break;
    case 1:
        if (paneEvent == PE_4) {
            changeAnimation(3);
        }
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        break;
    case 3:
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        break;
    case 2:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        break;
    case 4:
        if (paneEvent == PE_4) {
            changeAnimation(5);
        }
        break;
    case 6:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        break;
    }
}

void OnOffButtonAnmPane::onAnmEvent(AnmPaneEvent paneEvent) {
    if (paneEvent == PE_0 && meState != 15 && meState != 2 && meState != 13) {
        changeAnimation(4);
    } else if (paneEvent == PE_6 && meState != 14 && meState != 4 &&
               meState != 3 && meState != 12 && meState != 1 && meState != 2) {
        changeAnimation(12);
    } else if (paneEvent == PE_7 && meState != 15) {
        changeAnimation(13);
    } else {
        switch (meState) {
        case 12:
            if (paneEvent == PE_4) {
                changeAnimation(14);
            }
            if (paneEvent == PE_1) {
                changeAnimation(1);
            }
            break;
        case 13:
            if (paneEvent == PE_4) {
                changeAnimation(15);
            }
            break;
        case 14:
            if (paneEvent == PE_1) {
                changeAnimation(1);
            }
            break;
        case 1:
            if (paneEvent == PE_4) {
                changeAnimation(3);
            }
            if (paneEvent == PE_2) {
                changeAnimation(2);
            }
            break;
        case 3:
            if (paneEvent == PE_2) {
                changeAnimation(2);
            }
            break;
        case 2:
            if (paneEvent == PE_4) {
                changeAnimation(14);
            }
            if (paneEvent == PE_1) {
                changeAnimation(1);
            }
            break;
        case 4:
            if (paneEvent == PE_4) {
                changeAnimation(3);
            }
            if (paneEvent == PE_2) {
                changeAnimation(2);
            }
            break;
        }
    }
}

UIModifierButton::UIModifierButton(u32 id, LayoutByNW4R *layout,
                                   Listener *listener)
    : UIObj(id, layout, listener), mpPaneComponent(NULL), mpButton(NULL),
      mpAnmPane(NULL), mbPressed(false) {
}

inline void UIModifierButton::Create(nw4rmanager::Layout *layout,
                                     const char *paneName,
                                     const char *buttonName) {
    gui::PaneManager *paneManager = layout->getPaneManager();
    mpPaneComponent = paneManager->searchPaneComponent(paneName);
    mpButton = paneManager->searchPaneComponent(buttonName);
    mpAnmPane = layout->searchAnmPane(paneName);
    mpButton->setListener(this);
}

inline void UIModifierButton::setTriggerTarget(bool enable) {
    mpButton->setTriggerTarget(enable);
}

inline void UIModifierButton::release() {
    if (mbPressed) {
        mbPressed = false;
        mpAnmPane->onAnmEvent(nw4rmanager::AnmPane::PE_5);
    }
}

inline void UIModifierButton::setPressed(bool pressed) {
    mbPressed = pressed;
    if (mbPressed) {
        mpAnmPane->onAnmEvent(nw4rmanager::AnmPane::PE_10);
    } else {
        mpAnmPane->onAnmEvent(nw4rmanager::AnmPane::PE_11);
    }
}

void UIModifierButton::onGUIEvent(gui::PaneComponent &component, u32 event,
                                  nw4rmanager::TiEventHandler::Input *input) {
    const char *paneName = "P_key_CAPS";
    if (muID == 1) {
        paneName = "P_key_SHIFT";
    }
    switch (event) {
    case gui::EventHandler::ON_TRIG:
        if (input->trig & 0x800) {
            mpLayout->onKey(4, (void *)paneName);
        }
        break;
    case gui::EventHandler::ON_LEFT:
        mpAnmPane->onAnmEvent(nw4rmanager::AnmPane::PE_2);
        mpLayout->onKey(1, (void *)paneName);
        break;
    case gui::EventHandler::ON_POINT:
        if (mpListener != NULL) {
            mpListener->onEvent(this, 0, (void *)sound::SE_SELECT);
        }
        mpLayout->setPaneLastDrawReceived(mpAnmPane->getPane());
        mpAnmPane->onAnmEvent(nw4rmanager::AnmPane::PE_1);
        break;
    }
}

} // namespace pctype
} // namespace keyboard
} // namespace textinput
