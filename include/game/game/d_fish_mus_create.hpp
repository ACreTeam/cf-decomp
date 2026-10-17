#pragma once

// The fish factories of dFishMuseum::sFishCreateData.

#include <game/game/d_fish_museum.hpp>

namespace dFishMuseum {

// One instance per species (Kind = dFishMusKind_e): species that share a class still get their own factory.
template <class T, int Kind>
dFishMusBase_c *createFish(EGG::Heap *heap) {
    return new (heap) T();
}

} // namespace dFishMuseum
