#pragma once

#include <nw4r/g3d.h>
#include <egg/gfxe/eggFog.h>

namespace EGG {

class FogManager {
public:
    FogManager(unsigned short);
    virtual void vf_8(); ///< @unofficial
    virtual void vf_c(); ///< @unofficial
    virtual void vf_10(); ///< @unofficial
    virtual void vf_14(); ///< @unofficial
    virtual ~FogManager() {}

    void Calc();
    void CopyToG3D(nw4r::g3d::ScnRoot *) const;

    u8 state;
    char pad[7];
    Fog **mFog; // 0x0C

    void DoneDraw() { state &= 0xfb; }
};

} // namespace EGG
