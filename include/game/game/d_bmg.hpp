#pragma once

#include <types.h>

// 8015790C: BMG-aware UTF-16 length, including embedded message-tag payloads.
// The helper belongs to the BMG/script layer; its TU is not split here.
extern "C" int BMG_DAT_GetStringLength_wchar_t(const u16 *text, u32 maxSize,
                                              int breakOnNewLine);
