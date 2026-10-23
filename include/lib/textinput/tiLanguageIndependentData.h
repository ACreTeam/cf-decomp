#pragma once

#include <lib/textinput/tiTextInputBase.h>

namespace textinput {
namespace langindependent {

enum {
    LANG_STRID_OK,
    LANG_STRID_CANCEL,
    LANG_STRID_LANG_DICT,
    LANG_STRID_NEXT,
    LANG_STRID_PREV,
    LANG_STRID_A,
    LANG_STRID_CLOSE,
    LANG_STRID_SIGN_WINDOW,
    LANG_STRID_SPACE,
    LANG_STRID_BACK,
    LANG_STRID_MAX,
};

extern const wchar_t *cLanguageIndependentString[LANG_STRID_MAX][LanguageEnd];

} // namespace langindependent
} // namespace textinput
