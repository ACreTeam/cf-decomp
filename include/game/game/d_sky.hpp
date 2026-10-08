#pragma once

// The sky actor (RTTI dSky_c). Its code is in the d_skyNP REL, which is not split yet.

#include <game/game/d_base.hpp>
#include <nw4r/math.h>

class dSky_c : public dBase_c {
public:
    virtual void setFireworksBrightness(f32 brightness); // vtable 0x4C

    u8 _pad[0x568 - sizeof(dBase_c)];
    /* 0x568 */ nw4r::math::VEC3 mPos;
};

extern dSky_c *lbl_8074E830; // 8074E830 (defined in d_play_util.cpp)
