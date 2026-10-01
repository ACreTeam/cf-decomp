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

/// @brief Returns the REGION_e for the console's SCGetLanguage() setting.
/// On RUUE01: SC_LANG_EN -> REGION_US, SC_LANG_SP -> REGION_MX,
/// SC_LANG_FR -> REGION_QC, anything else -> REGION_US.
extern "C" int fn_801068B4(); // 801068B4
