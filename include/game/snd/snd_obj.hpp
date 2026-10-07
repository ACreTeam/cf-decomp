#pragma once
#include <types.h>

// The sound library's positional sound objects (DOL, not split yet, around 801D5548..801DF45C).
// Class names are from the RTTI:
//   SoundObjBase
//     SoundObj<N>        N 4-byte entries at 0x2C (with ctor/dtor; sound handles?)
//       SoundObjSimple
//       SoundObjInsect
//       SoundHaniwa
// Only what is used so far; the layouts are opaque.

class SoundObjBase {
public:
    SoundObjBase();          // 801DC544
    virtual ~SoundObjBase(); // 801DC5B4

    /* 0x04 */ u8 _04[0x2C - 0x04];
}; // size 0x2C

template <int N>
class SoundObj : public SoundObjBase {
public:
    virtual ~SoundObj(); // SoundObj<1>: 80011FF4 (weak copy kept in the DOL)

    /* 0x2C */ u8 _2C[N * 4];
};

class SoundObjSimple : public SoundObj<1> {
public:
    SoundObjSimple();          // 801DD270
    virtual ~SoundObjSimple(); // 801DD2E0
}; // size 0x30

class SoundObjInsect : public SoundObj<1> {
public:
    SoundObjInsect();          // 801DCFC4
    virtual ~SoundObjInsect(); // 801DD034
};

class SoundHaniwa : public SoundObj<1> {
public:
    SoundHaniwa();          // 801D5548
    virtual ~SoundHaniwa(); // 801D5648
};
