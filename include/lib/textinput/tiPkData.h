#pragma once

#include <types.h>

namespace textinput {
namespace keyboard {
namespace pctype {

static const int KEY_COUNT = 50;

struct PaneNameToCharCodeWithModeAscii {
    /* 0x00 */ const char *szPaneName;
    /* 0x04 */ wchar_t wc[4];
}; // size 0xC

extern const PaneNameToCharCodeWithModeAscii csJPKeyboard[KEY_COUNT];
extern const PaneNameToCharCodeWithModeAscii csUSKeyboard[KEY_COUNT];
extern const PaneNameToCharCodeWithModeAscii csUKKeyboard[KEY_COUNT];
extern const PaneNameToCharCodeWithModeAscii csFRKeyboard[KEY_COUNT];
extern const PaneNameToCharCodeWithModeAscii csDEKeyboard[KEY_COUNT];
extern const PaneNameToCharCodeWithModeAscii csITKeyboard[KEY_COUNT];
extern const PaneNameToCharCodeWithModeAscii csESKeyboard[KEY_COUNT];
extern const PaneNameToCharCodeWithModeAscii csNLKeyboard[KEY_COUNT];

} // namespace pctype
} // namespace keyboard
} // namespace textinput
