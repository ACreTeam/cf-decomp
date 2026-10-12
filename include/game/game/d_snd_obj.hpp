#pragma once
#include <types.h>
#include <game/snd/snd_obj.hpp>

class mVec3_c;

// Game-side holders of the sound library's objects (in DOL TU d_audio_obj.cpp, 80010E18..80011F28: the
// data order puts all of 800107AC..800120A8 in one TU, see d_audio_obj.hpp). There is one holder per
// SoundObj class, each holding the object at offset 0; setPos converts the position with getSoundPos
// first.
// They have no RTTI: the class names are inferred from the class they hold.
// Users: dFishFldShadow_c, daSnowMan_c, dHandItem_c.

// Holds a SoundObjInsect.
class dSndObjInsect_c {
public:
    dSndObjInsect_c();  // 80010E18
    ~dSndObjInsect_c(); // 80010E48

    void setPos(const mVec3_c *pos); // 80010EA0
    void startSound(u32 id);         // 80010EF4
    void holdSound(u32 id);          // 80010F08
    bool isSounding();               // 80010F18

    /* 0x00 */ SoundObjInsect mObj;
}; // size 0x30

// Holds a SoundObjSimple.
class dSndObjSimple_c {
public:
    dSndObjSimple_c();  // 80010F1C
    ~dSndObjSimple_c(); // 80010F4C

    void setPos(const mVec3_c *pos); // 80010FA4
    void startSound(u32 id);         // 80010FF8
    void holdSound(u32 id);          // 8001100C

    /* 0x00 */ SoundObjSimple mObj;
}; // size 0x30

// Holds a SoundHaniwa (layout not needed yet).
class dSndHaniwa_c {
public:
    dSndHaniwa_c();  // 8001101C
    ~dSndHaniwa_c(); // 8001104C
};

// Holds a SoundObjPond.
class dSndObjPond_c {
public:
    dSndObjPond_c();  // 800112C0
    ~dSndObjPond_c(); // 800112F0

    void setPos(const mVec3_c *pos); // 80011348
    void holdSound(u32 id);          // 80011394

    /* 0x00 */ SoundObjPond mObj;
}; // size 0x30

// Holds a SoundObjFall.
class dSndObjFall_c {
public:
    dSndObjFall_c();  // 800113A4
    ~dSndObjFall_c(); // 800113D4

    void setPos(const mVec3_c *pos); // 8001142C
    void holdSound(u32 id);          // 80011478

    /* 0x00 */ SoundObjFall mObj;
}; // size 0x30

// Holds a SoundObjRiver.
class dSndObjRiver_c {
public:
    dSndObjRiver_c();  // 80011488
    ~dSndObjRiver_c(); // 800114B8

    void setPos(const mVec3_c *pos); // 80011510

    /* 0x00 */ SoundObjRiver mObj;
}; // size 0x34

// Holds a SoundObjPinwheel.
class dSndObjPinwheel_c {
public:
    dSndObjPinwheel_c();  // 80011D7C
    ~dSndObjPinwheel_c(); // 80011E3C

    void update(const mVec3_c *pos, f32 speed); // 80011ED0: no-op without a position

    /* 0x00 */ SoundObjPinwheel mObj;
}; // size 0x34
