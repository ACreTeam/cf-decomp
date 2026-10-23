#pragma once

#include <types.h>

namespace textinput {
namespace keyboard {
namespace cellphonetype {

static const int KEY_COUNT = 12;

struct PaneNameToCharCode {
    /* 0x00 */ const char *szPaneName;
    /* 0x04 */ wchar_t szPaneDisp[8];
    /* 0x14 */ wchar_t wc[16];
}; // size 0x34

struct KeySet {
    /* 0x00 */ const PaneNameToCharCode *pPaneNameToCharCode;
    /* 0x04 */ wchar_t szKeySetName[17];
    /* 0x26 */ u16 uNum;
    /* 0x28 */ u16 uType;
}; // size 0x2C

extern const KeySet csKeySetUSAbc;
extern const KeySet csKeySetUSABC;
extern const KeySet csKeySetUSabc;
extern const KeySet csKeySetUKAbc;
extern const KeySet csKeySetUKABC;
extern const KeySet csKeySetUKabc;
extern const KeySet csKeySetDEAbc;
extern const KeySet csKeySetDEABC;
extern const KeySet csKeySetDEabc;
extern const KeySet csKeySetITAbc;
extern const KeySet csKeySetITABC;
extern const KeySet csKeySetITabc;
extern const KeySet csKeySetNLAbc;
extern const KeySet csKeySetNLABC;
extern const KeySet csKeySetNLabc;
extern const KeySet csKeySetSPAbc;
extern const KeySet csKeySetSPABC;
extern const KeySet csKeySetSPabc;
extern const KeySet csKeySetFRAbc;
extern const KeySet csKeySetFRABC;
extern const KeySet csKeySetFRabc;
extern const KeySet csKeySetHiragana;
extern const KeySet csKeySetKatakana;
extern const KeySet csKeySetABCJP;
extern const KeySet csKeySetabcJP;
extern const KeySet csKeySetNumber;

extern const wchar_t *csSpaceZero;
extern const wchar_t *csSpace[2];

} // namespace cellphonetype
} // namespace keyboard
} // namespace textinput
