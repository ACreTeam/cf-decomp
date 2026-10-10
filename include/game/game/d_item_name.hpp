#pragma once

// Name words of item series (d_hr). Kept apart from d_item.hpp: d_hr's RTTI/vtable order needs this class
// completed after dSvMdlRm_c::searchCB_c (d_model_room.hpp, which pulls d_item.hpp in) and its weak dtor
// in a section of its own after d_home.hpp's.

#include <game/game/d_item.hpp>

namespace dItem {

class nameSeries_c : public dString::Word_c {
public:
    virtual ~nameSeries_c() {} // weak 800B4D48 (d_hr)
};

} // namespace dItem
