#pragma once

// The sky actor (RTTI dSky_c, profile 0xAA). Source: src/d_skyNP/d_sky.cpp (not decompiled yet).

#include <game/framework/f_base.hpp>
#include <nw4r/math.h>

class dSky_c : public fBase_c {
public:
    virtual void setFireworksBrightness(f32 brightness); // vtable 0x4C

    u8 _pad[0x568 - sizeof(fBase_c)];
    /* 0x568 */ nw4r::math::VEC3 mPos;
};

extern dSky_c *lbl_8074E830; // 8074E830 (defined in d_play_util.cpp)
