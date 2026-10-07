#pragma once
#include <types.h>
#include <game/snd/snd_obj.hpp>

class mVec3_c;

// Game-side holders of the sound library's objects (DOL TU d_snd_obj.cpp, not split yet, around
// 80010E18..80011130). There is one holder per SoundObj class, each holding the object at offset 0.
// They have no RTTI: the class names are inferred from the class they hold.
// Users: dFishFldShadow_c, daSnowMan_c, dHandItem_c.

// Holds a SoundObjSimple.
class dSndObjSimple_c {
public:
    dSndObjSimple_c();  // 80010F1C
    ~dSndObjSimple_c(); // 80010F4C

    void setPos(const mVec3_c *pos); // 80010FA4: converts the position, then sets it on mObj
    void startSound(u32 id);         // 80010FF8
    void holdSound(u32 id);          // 8001100C

    /* 0x00 */ SoundObjSimple mObj;
}; // size 0x30

// Holds a SoundObjInsect (layout not needed yet).
class dSndObjInsect_c {
public:
    dSndObjInsect_c();  // 80010E18
    ~dSndObjInsect_c(); // 80010E48
};

// Holds a SoundHaniwa (layout not needed yet).
class dSndHaniwa_c {
public:
    dSndHaniwa_c();  // 8001101C
    ~dSndHaniwa_c(); // 8001104C
};
