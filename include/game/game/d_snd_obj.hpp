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

// Holds a SoundHaniwa.
class dSndHaniwa_c {
public:
    dSndHaniwa_c();  // 8001101C
    ~dSndHaniwa_c(); // 8001104C

    void setup(u32 kind, s8 startBar); // 800110A4
    f32 calc(const mVec3_c *pos);     // 800110AC
    void play();                       // 800110EC
    void stop();                       // 800110F0
    u8 getStartBar();                  // 800110F4
    int getWaitFrames();               // 800110FC

    /* 0x00 */ SoundHaniwa mObj;
};

// Thin actor-facing wrapper around the instrument used for the town tune.
class dSndObjInst_c {
public:
    dSndObjInst_c();  // 80011100
    ~dSndObjInst_c(); // 80011130

    void setup(int inst);                // 80011188
    void calc(const mVec3_c *pos);      // 8001118C
    void play();                         // 800111CC
    void stop();                         // 800111D0

    /* 0x00 */ SoundObjInst mObj;
};

// Furniture sound wrapper; the sound object sits at offset zero.
class dSndObjFurniture_c {
public:
    dSndObjFurniture_c();  // 800111D4
    ~dSndObjFurniture_c(); // 80011204

    void setup(u8 item, bool on); // 8001125C
    void setupSeq(const SoundObjFurniture::SeqData *data, const void *seq, bool on); // 80011260
    void update(const mVec3_c *pos, f32 frame); // 80011264
    void turnOn();                    // 800112B4
    void turnOff();                   // 800112B8
    int getType();                    // 800112BC

    /* 0x00 */ SoundObjFurniture mObj;
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
    void holdSound(u32 id);                    // 80011F28

    /* 0x00 */ SoundObjPinwheel mObj;
}; // size 0x34
