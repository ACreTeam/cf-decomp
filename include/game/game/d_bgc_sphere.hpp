#pragma once
#include <game/game/d_bgc.hpp>

// dBGC::sphere_c, in a header of its own: d_bgc.cpp (-sym on) emits its inline dtor (8006C5E0) after
// poly_c's (8006C5A0) although its vtable comes first, so its inlines were in a separate file.
// Source: src/dol/game/d_bgc.cpp. See notes/d_bgc.txt.

namespace dBGC {

// A sphere. RTTI dBGC::sphere_c, vtable 804A5508.
class sphere_c {
public:
    /* 0x00 */ mVec3_c mCenter;
    /* 0x0C */ f32 mRadius;
    /* 0x10 */ // vtable

    sphere_c();            // 8006C3B8
    virtual ~sphere_c() {} // 8006C5E0

    BOOL crossLine(mVec3_c *out, const line_c &line) const; // 8006C3D0
}; // size 0x14

} // namespace dBGC
