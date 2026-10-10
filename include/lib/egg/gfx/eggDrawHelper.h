#pragma once

// EGG::Graphics (lib/egg/gfx/eggDrawHelper.cpp, .text 80445D18..80445F80). Names from Big Brain Academy's EGG
// link maps (eggDrawHelper.o): CF's linker kept only setupGX (used by d_shadow) and the static mColor.

#include <lib/egg/gfx/eggColor.h>

namespace EGG {

class Graphics {
public:
    static void setupGX(); // 80445D18: GX state for untextured, vertex-coloured drawing

    static Color mColor; // 8074FC80 (white)
};

} // namespace EGG
