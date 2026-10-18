#pragma once

// The insect factories of dInsectMuseum::sInsectDataTable.

#include <game/game/d_insect_museum.hpp>

namespace dInsectMuseum {

// Type gives each insect type its own instance, even when types share a class.
template <class T, int Type>
dInsMusBase_c *createInsect(EGG::Heap *heap) {
    return new (heap) T();
}

} // namespace dInsectMuseum
