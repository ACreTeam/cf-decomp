#pragma once

#include <types.h>

namespace textinput {
namespace keyboard {
namespace signwindow {

static const int KEY_COUNT = 20;

struct PaneNameToCharCodeInSignMode {
    /* 0x00 */ const char *szPaneName;
    /* 0x04 */ const char *szNextPaneName;
    /* 0x08 */ wchar_t wc[16];
}; // size 0x28

extern const PaneNameToCharCodeInSignMode csSignKeyJP[KEY_COUNT];
extern const PaneNameToCharCodeInSignMode csSignKeyUS[KEY_COUNT];
extern const PaneNameToCharCodeInSignMode csSignKeyUK[KEY_COUNT];
extern const PaneNameToCharCodeInSignMode csSignKeyDE[KEY_COUNT];
extern const PaneNameToCharCodeInSignMode csSignKeyFR[KEY_COUNT];
extern const PaneNameToCharCodeInSignMode csSignKeyIT[KEY_COUNT];
extern const PaneNameToCharCodeInSignMode csSignKeySP[KEY_COUNT];
extern const PaneNameToCharCodeInSignMode csSignKeyNL[KEY_COUNT];

} // namespace signwindow
} // namespace keyboard
} // namespace textinput
