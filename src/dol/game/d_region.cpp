// Console region and language helpers. .text 801068AC..8010693C.
// Names are inferred.
#include <game/game/d_region.hpp>
#include <revolution/SC/scapi.h>

// 801068AC: the console region (this build is North America)
int getRegion() {
    return CONSOLE_REGION_NA;
}

// 801068B4
int getLanguage() {
    switch (SCGetLanguage()) {
    case SC_LANG_EN:
        return LANGUAGE_US;
    case SC_LANG_SP:
        return LANGUAGE_MX;
    case SC_LANG_FR:
        return LANGUAGE_QC;
    default:
        return LANGUAGE_US;
    }
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
