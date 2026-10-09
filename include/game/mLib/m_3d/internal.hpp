#pragma once
#include <game/mLib/m_allocator.hpp>
#include <lib/egg/gfxe/eggLightManager.h>
#include <lib/egg/gfxe/eggFogManager.h>

namespace m3d {
    namespace internal {
        extern mAllocator_c *l_allocator_p;
        extern nw4r::g3d::ScnRoot *l_scnRoot_p;
        extern EGG::LightManager *l_lightMgr_p;
        extern EGG::FogManager *l_fogMgr_p;
    }
}
