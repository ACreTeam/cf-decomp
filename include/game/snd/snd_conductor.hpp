#pragma once

// dol/sound/SoundConductor.cpp (801D42AC..801D5390, not decompiled). Class names from the RTTI
// ("SoundConductor", vtable 80500FFC; "SoundConductorKK", vtable 80500FD0). Only what is used so
// far; layouts are opaque, methods keep their fn_ names.

#include <types.h>

class SoundConductor {
public:
    SoundConductor();  // 801D42AC
    ~SoundConductor(); // 801D4344 (not virtual: not in the vtable)

    virtual void fn_801D4648(); // 801D4648

    /* 0x04 */ u8 _04[0x1C - 0x04];
}; // size 0x1C

class SoundConductorKK : public SoundConductor {
public:
    SoundConductorKK();  // 801D4938
    ~SoundConductorKK(); // 801D499C

    virtual void fn_801D4648(); // 801D4A48 (the override)

    /* 0x1C */ u8 _1C[0x38 - 0x1C];
}; // size 0x38
