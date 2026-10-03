#pragma once

#include <types.h>

/// @brief Game region/language codes. Order matches the BITM name tables
/// (m_nameJp, m_nameUs, ...) and dItem::Language.
/// Stored in dLandID_c::mRegion; cleared land IDs use LANGUAGE_e (0xA).
/// @unofficial
enum LANGUAGE_e {
    LANGUAGE_JP, // Japanese
    LANGUAGE_US, // American English
    LANGUAGE_MX, // Mexican/Latino Spanish
    LANGUAGE_QC, // French Canadian
    LANGUAGE_EN, // British English
    LANGUAGE_ES, // Spanish
    LANGUAGE_FR, // French
    LANGUAGE_IT, // Italian
    LANGUAGE_DE, // German
    LANGUAGE_KR, // Korean

    LANGUAGE_NUM
};

enum REGION_e {
    REGION_JP, // Japan
    REGION_NA, // North America
    REGION_UK, // United Kingdom
    REGION_SP, // Spain
    REGION_FR, // France
    REGION_IT, // Italy
    REGION_DE, // Germany
    REGION_KR, // South Korea

    REGION_NUM
};

enum CONSOLE_REGION_e {
    CONSOLE_REGION_JP,
    CONSOLE_REGION_NA,
    CONSOLE_REGION_EU,
    CONSOLE_REGION_KR,

    CONSOLE_REGION_NUM,
};

// Source: src/dol/game/d_region.cpp (.text 801068AC..8010693C). Names are inferred.

/// @brief Console region; always 1 (North America) on RUUE01.
int getRegion(); // 801068AC

/// @brief Returns the LANGUAGE_e for the console's SCGetLanguage() setting.
/// On RUUE01: SC_LANG_SP -> LANGUAGE_MX, SC_LANG_FR -> LANGUAGE_QC,
/// anything else -> LANGUAGE_US.
int getLanguage(); // 801068B4

/// @brief Area for a LANGUAGE_e: 0 Japan, 1 America, 2 Europe, 3 Korea, 4 if out of range.
int getLanguageArea(int language); // 80106918
