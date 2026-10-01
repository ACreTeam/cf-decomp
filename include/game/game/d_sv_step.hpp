#pragma once

#include <game/game/d_state_step.hpp>

// RTTI confirms this wrapper adds no storage or virtual slots beyond its
// destructor. Save-specific nonvirtual helper interfaces remain unrecovered.
template <class Owner, class Step>
class dSvStep_c : public dState::step_c<Owner, Step> {
public:
    dSvStep_c() {}
    virtual ~dSvStep_c() {}
}; // sizeof = 0x30
