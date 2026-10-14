#pragma once
// eggDrawGX.o (not decompiled): only what other TUs use. Names are unknown.

#include <types.h>

namespace EGG { class Heap; }

extern "C" {
void fn_80446348(EGG::Heap *heap); // 80446348: initialises the GX draw helpers (texture, identity matrix; NULL = current heap)
}
