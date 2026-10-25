// Console region and language helpers. .text 801068AC..8010693C.
// Names are inferred.
#include <game/game/d_region.hpp>
#include <revolution/sc.h>

// 801068AC: the console region (this build is North America)
int getRegion() {
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    return CONSOLE_REGION_NA;
#elif VERSION == VERSION_RUUP01_00 || VERSION == VERSION_RUUP01_01
    return CONSOLE_REGION_EU;
#elif VERSION == VERSION_RUUJ01_01 || VERSION == VERSION_RUUJ01_02
    return CONSOLE_REGION_JP;
#else
    return CONSOLE_REGION_KR;
#endif
}

// 801068B4
int getLanguage() {
#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    switch (SCGetLanguage()) {
    case SC_LANG_ENGLISH:
        return LANGUAGE_US;
    case SC_LANG_SPANISH:
        return LANGUAGE_MX;
    case SC_LANG_FRENCH:
        return LANGUAGE_QC;
    default:
        return LANGUAGE_US;
    }
#elif VERSION == VERSION_RUUP01_00 || VERSION == VERSION_RUUP01_01
    switch (SCGetLanguage()) {
    case SC_LANG_ENGLISH:
        return LANGUAGE_EN;
    case SC_LANG_SPANISH:
        return LANGUAGE_ES;
    case SC_LANG_FRENCH:
        return LANGUAGE_FR;
    case SC_LANG_ITALIAN:
        return LANGUAGE_IT;
    case SC_LANG_GERMAN:
        return LANGUAGE_DE;
    case SC_LANG_DUTCH:
        return LANGUAGE_EN;
    default:
        return LANGUAGE_EN;
    }
#elif VERSION == VERSION_RUUJ01_01 || VERSION == VERSION_RUUJ01_02
    switch (SCGetLanguage()) {
    case SC_LANG_JAPANESE:
        return LANGUAGE_JP;
    default:
        return LANGUAGE_JP;
    }
#else
    return LANGUAGE_KR;
#endif
}

// 80475C08: language -> area (0 Japan, 1 America, 2 Europe, 3 Korea)
static const int lbl_80475C08[LANGUAGE_NUM] = {0, 1, 1, 1, 2, 2, 2, 2, 2, 3};

// 80106918
int getLanguageArea(int language) {
    if (language < LANGUAGE_NUM) {
        return lbl_80475C08[language];
    }
    return 4;
}
