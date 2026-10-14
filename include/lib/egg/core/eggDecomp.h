#pragma once
// eggDecomp.o (not decompiled): only what other TUs use. Names are unknown.

#include <types.h>

namespace EGG { class Heap; }

extern "C" {
void fn_8043EFD4(EGG::Heap *heap); // 8043EFD4: allocates the 0x9008-byte work buffer once (NULL = current heap); d_sys
}
