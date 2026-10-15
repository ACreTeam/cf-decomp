#pragma once
#include <lib/egg/util/eggEffect.hpp>

namespace EGG {

// RTTI: EGG::ExOneEmitter. Only the interface used by mEf is declared here.
class ExOneEmitter {
public:
    ExOneEmitter(Effect *effect); // 8044D9A4
    virtual ~ExOneEmitter(); // 8044D9F4
    void create(); // 8044DA34
    void setPos(const nw4r::math::VEC3 *pos); // 8044DAF0
    void update(); // 8044DB8C

private:
    u8 _04[0x24];
}; // 0x28

} // namespace EGG
